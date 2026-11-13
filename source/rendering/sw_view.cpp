#include "sw_global.h"
#include "sw_view.h"
#include "sw_view_widget.h"
#include "sw_window_main.h"
#include "sw_object_camera.h"
#include "sw_texture.h"
#include "sw_object_part.h"
#include "sw_data_part.h"
#include "sw_track_train.h"
#include "sw_scene.h"
#include "sw_view_manipulator.h"
#include "sw_view_cube.h"
#include "sw_widget_find_replace.h"
#include "sw_library.h"
#include "sw_widget_selection_part_popup.h"
#include "sw_application.h"

swFindReplaceParameters swView::mFindReplaceParams;
QPointer<swFindReplaceWidget> swView::mFindWidget;

swView* swView::mLastFocusedView;
std::vector<swView*> swView::mViews;
swCameraProjection swView::mLastProjection = swCameraProjection::Perspective;

swView::swView(swViewType ViewType, swModel* Model)
	: mViewType(ViewType), mScene(new swScene()), mModel(Model)
{
	mContext = new swContext();
	mViews.push_back(this);

	mViewManipulator = std::unique_ptr<swViewManipulator>(new swViewManipulator(this));
	mViewCube = std::unique_ptr<swViewCube>(new swViewCube(this));
	memset(mGridSettings, 0, sizeof(mGridSettings));

	mDragState = swDragState::None;
	mTrackToolFromOverlay = false;

	swView* ActiveView = gMainWindow ? gMainWindow->GetActiveView() : nullptr;

	if (ActiveView)
		SetCamera(ActiveView->mCamera, false);
	else
	{
		mCamera = new swCamera(true);
		mCamera->SetViewpoint(swViewpoint::Home);
	}

	// use the last chosen projection
	if (mViewType == swViewType::View && mCamera->IsSimple())
		mCamera->SetProjection(mLastProjection);

	LoadCutaway();
}

swView::~swView()
{
	DestroyOutlineFramebuffers();

	mContext->DestroyVertexBuffer(mGridBuffer);

	if (gMainWindow && mViewType == swViewType::View)
		gMainWindow->RemoveView(this);

	if (mCamera && mCamera->IsSimple())
		delete mCamera;

	mViews.erase(std::find(mViews.begin(), mViews.end(), this));

	if (mLastFocusedView == this)
		mLastFocusedView = nullptr;

	delete mContext;
}

// splits keep the original view top left
bool swView::IsMainView() const
{
	swModelTabWidget* TabWidget = gMainWindow ? gMainWindow->GetTabForView(this) : nullptr;

	QWidget* MainWidget = TabWidget ? TabWidget->GetAnyViewWidget() : nullptr;

	return !MainWidget || MainWidget == mWidget;
}

std::vector<swView*> swView::GetModelViews(const swModel* Model)
{
	std::vector<swView*> Views;

	for (swView* View : mViews)
		if (View->GetModel() == Model)
			Views.push_back(View);

	return Views;
}

void swView::UpdateProjectViews(const Project* Project)
{
	for (swView* View : mViews)
	{
		const swModel* ViewModel = View->GetActiveModel();

		if (ViewModel && ViewModel->GetProject() == Project)
			View->RedrawWithMarkers();
	}
}

void swView::UpdateAllViews()
{
	for (swView* View : mViews)
		View->RedrawWithMarkers();
}

// use to keep markers on moved parts
void swView::RedrawWithMarkers()
{
	// shown markers follow the parts they mark

	if (!mConnectionMarkers.empty())
		UpdateConnectionMarkers();

	Redraw();
}

// use to refit views once loaded
void swView::UpdateFittedViews()
{
	for (swView* View : mViews)
	{
		if (!View->IsFitted())
			continue;

		View->ZoomExtents(View->mZoomExtentsMode, View->mZoomExtentsFarPlane);
		View->Redraw();
	}
}

// use to keep edited views where they are
void swView::ClearFittedViews(const swModel* Model)
{
	for (swView* View : mViews)
		if (View->GetActiveModel() == Model)
			View->mZoomExtentsCamera = nullptr;
}

void swView::MakeCurrent()
{
	mContext->MakeCurrent();
}

void swView::Redraw()
{
	// a plain selection keeps the editing
	if (mJointReturn && mModel)
	{
		if (GetActiveModel()->IsJointFocused())
			mJointSeen = true;
		else if (mJointSeen)
			mJointReturn = false;
	}

	if (mWidget)
	{
		mWidget->UpdateModelEditingPath();
		mWidget->UpdateJointPath();
		mWidget->update();
	}
}

void swView::SetOffscreenContext()
{
	mContext->SetOffscreenContext();
}

void swView::SetFocus(bool Focus)
{
	if (Focus)
	{
		if (mLastFocusedView != this)
		{
			// close only the other bar later
			if (mFindWidget && !mWidget->isAncestorOf(mFindWidget))
			{
				mFindWidget->hide();
				mFindWidget->deleteLater();
				mFindWidget = nullptr;
			}

			mLastFocusedView = this;
		}

		emit FocusReceived();
	}
}

void swView::SetMousePosition(int MouseX, int MouseY)
{
	mMouseX = MouseX;
	mMouseY = MouseY;
}

void swView::SetMouseModifiers(Qt::KeyboardModifiers MouseModifiers)
{
	mMouseModifiers = MouseModifiers;
}

swModel* swView::GetActiveModel() const
{
	return !mActiveSubmodelInstance ? mModel : mActiveSubmodelInstance->mPartData->GetModel();
}

std::vector<swModel*> swView::GetModelEditingPath() const
{
	std::vector<swModel*> Models = { mModel };

	for (const swPart* Part : mSubmodelEditingPath)
		Models.push_back(Part->mPartData->GetModel());

	return Models;
}

void swView::RestoreModelEditingContext()
{
	mModel->SetActive(true);
	mActiveSubmodelTransform = swMatrix44Identity();

	for (swPart* Part : mSubmodelEditingPath)
	{
		mActiveSubmodelTransform = swMultiply(Part->mModelWorld, mActiveSubmodelTransform);
		Part->mPartData->GetModel()->SetActive(true);
	}
}

void swView::SetSubmodelActive(size_t Depth)
{
	if (Depth > mSubmodelEditingPath.size())
		return;

	StopTracking(true);

	// another model in place keeps the camera
	if (Depth != mSubmodelEditingPath.size())
		mZoomExtentsCamera = nullptr;

	const std::vector<swModel*> EditingModels = GetModelEditingPath();

	for (size_t Index = EditingModels.size(); Index > Depth + 1; Index--)
		EditingModels[Index - 1]->SetActive(false);

	std::vector<swModel*> UpdatedModels;
	for (size_t Index = EditingModels.size(); Index > Depth + 1; Index--)
		EditingModels[Index - 1]->UpdatePartData(UpdatedModels);

	mSubmodelEditingPath.resize(Depth);
	mActiveSubmodelInstance = nullptr;

	if (!mSubmodelEditingPath.empty())
		mActiveSubmodelInstance = mSubmodelEditingPath.back();

	RestoreModelEditingContext();
	GetActiveModel()->UpdateInterface();
}

void swView::SetTopSubmodelActive()
{
	SetSubmodelActive(0);
}

void swView::SetSelectedSubmodelActive()
{
	swModel* ActiveModel = GetActiveModel();
	swObject* Object = ActiveModel->GetFocusObject();

	if (!Object || !Object->IsPart())
		return;

	swPart* Part = (swPart*)Object;

	if (!Part->mPartData->IsModel())
		return;

	// keep ancestor models at their steps
	mZoomExtentsCamera = nullptr;
	mSubmodelEditingPath.push_back(Part);
	mActiveSubmodelInstance = Part;

	ActiveModel = mActiveSubmodelInstance->mPartData->GetModel();
	ActiveModel->SetTemporaryStep(ActiveModel->GetLastStep());
	RestoreModelEditingContext();

	RemoveCamera();

	GetActiveModel()->UpdateInterface();
}

// use to edit a chain of submodels in place
void swView::SetSubmodelEditingPath(const std::vector<swPart*>& Path)
{
	SetSubmodelActive(0);

	// stop where an instance is gone
	swModel* Parent = mModel;

	for (swPart* Part : Path)
	{
		const std::vector<std::unique_ptr<swPart>>& Parts = Parent->GetParts();

		if (std::none_of(Parts.begin(), Parts.end(), [Part](const std::unique_ptr<swPart>& Entry) { return Entry.get() == Part; }) || !Part->mPartData->IsModel())
			break;

		mSubmodelEditingPath.push_back(Part);
		Parent = Part->mPartData->GetModel();
		Parent->SetTemporaryStep(Parent->GetLastStep());
	}

	mActiveSubmodelInstance = mSubmodelEditingPath.empty() ? nullptr : mSubmodelEditingPath.back();

	if (mActiveSubmodelInstance)
		mZoomExtentsCamera = nullptr;

	RestoreModelEditingContext();
	RemoveCamera();
	GetActiveModel()->UpdateInterface();
}

// joints edit their model in place
bool swView::SetJointModelActive(swModel* Target)
{
	swModel* ActiveModel = GetActiveModel();

	if (!ActiveModel->IsJointFocused())
	{
		mJointReturnPath = mSubmodelEditingPath;
		mJointReturn = true;
	}

	// focus is set after the switch
	mJointSeen = false;

	if (ActiveModel == Target)
		return true;

	if (ActiveModel->IsJointFocused())
		ActiveModel->EndJointFocus();

	// first instance of the target
	std::vector<swPart*> Path;
	std::function<bool(swModel*)> FindPath = [&](swModel* Model)
	{
		if (Model == Target)
			return true;

		for (const std::unique_ptr<swPart>& Part : Model->GetParts())
		{
			if (!Part->mPartData->IsModel())
				continue;

			Path.push_back(Part.get());

			if (FindPath(Part->mPartData->GetModel()))
				return true;

			Path.pop_back();
		}

		return false;
	};

	if (!FindPath(mModel))
		return false;

	SetSubmodelEditingPath(Path);
	return true;
}

// set joint from the menu picks on the submodel under the pointer
bool swView::OpenSetJoint()
{
	if (mContextPath.empty())
		return false;

	swModel* ActiveModel = GetActiveModel();

	if (!ActiveModel->IsJointFocused())
	{
		mJointReturnPath = mSubmodelEditingPath;
		mJointReturn = true;
	}

	mJointSeen = false;
	std::vector<swPart*> Path = mSubmodelEditingPath;
	Path.insert(Path.end(), mContextPath.begin(), mContextPath.end() - 1);
	swPart* Holder = mContextPath.back();
	mContextPath.clear();

	if (ActiveModel->IsJointFocused())
		ActiveModel->EndJointFocus();

	if (Path != mSubmodelEditingPath)
	{
		SetSubmodelEditingPath(Path);
		ActiveModel = GetActiveModel();
	}

	ActiveModel->SetSelectionAndFocusAction({ Holder }, Holder, SW_PART_SECTION_POSITION, swSelectionMode::Single);
	gMainWindow->SetTool(swTool::Joints);
	ActiveModel->SetJointPick(true);
	ActiveModel->UpdateInterface();
	UpdateAllViews();
	return true;
}

// set joint ends in joint mode on the new joint
void swView::FocusSetJoint(const std::vector<swPart*>& Parts)
{
	swModel* ActiveModel = GetActiveModel();
	std::vector<swPart*> Path = mSubmodelEditingPath;
	Path.insert(Path.end(), Parts.begin(), Parts.end() - 1);

	// a set joint from the menu already keeps its way back
	if (!ActiveModel->IsJointFocused() && !mJointReturn)
	{
		mJointReturnPath = mSubmodelEditingPath;
		mJointReturn = true;
	}

	if (Path != mSubmodelEditingPath)
	{
		if (ActiveModel->IsJointFocused())
			ActiveModel->EndJointFocus();

		SetSubmodelEditingPath(Path);
		ActiveModel = GetActiveModel();
	}

	swPart* Holder = Parts.back();
	ActiveModel->SetSelectionAndFocusAction({ Holder }, Holder, SW_PART_SECTION_POSITION, swSelectionMode::Single);
	ActiveModel->SetJointFocus();
	mJointSeen = true;
	ActiveModel->UpdateInterface();
	UpdateAllViews();
}

// back to where the joint started
void swView::EndJointFocus()
{
	// the redraw from ending clears the flag
	const bool Return = mJointReturn;
	mJointReturn = false;
	GetActiveModel()->EndJointFocus();

	if (Return)
		SetSubmodelEditingPath(mJointReturnPath);

	gMainWindow->SetTool(swTool::Select);
}

// select the entry of a model on the joint path
void swView::SetJointPathDepth(size_t Index)
{
	// the joint entry itself stays as it is
	if (Index > mSubmodelEditingPath.size())
		return;

	// a model button stands for its own entry, the main model has none
	const size_t Depth = Index ? Index - 1 : 0;
	swModel* ActiveModel = GetActiveModel();
	swPart* Part = Index ? mSubmodelEditingPath[Depth] : nullptr;
	const bool Joint = Part && Part->HasJoints();
	const bool Return = mJointReturn;
	mJointReturn = false;
	ActiveModel->EndJointFocus();
	SetSubmodelActive(Depth);

	if (!Part)
		return gMainWindow->SetTool(swTool::Select);

	ActiveModel = GetActiveModel();
	ActiveModel->SetSelectionAndFocusAction({ Part }, Part, SW_PART_SECTION_POSITION, swSelectionMode::Single);

	if (!Joint)
		return gMainWindow->SetTool(swTool::Select);

	// keep the joint mode on its joint
	ActiveModel->SetJointIndex(Part, 0);
	ActiveModel->SetJointFocus();
	mJointReturn = Return;
	mJointSeen = true;
	ActiveModel->UpdateInterface();
	UpdateAllViews();
}

void swView::CreateResources(swContext* Context)
{
	Q_UNUSED(Context);
}

void swView::DestroyResources(swContext* Context)
{
	Q_UNUSED(Context);
}

void swView::RemoveCamera()
{
	if (mCamera && mCamera->IsSimple())
		return;

	swCamera* Camera = mCamera;
	mCamera = new swCamera(true);

	if (Camera)
		mCamera->CopyPosition(Camera);
	else
		mCamera->SetViewpoint(swViewpoint::Home);

	Redraw();
}

void swView::ShowTrainTrackPopup()
{
	if (mViewType != swViewType::View)
		return;

	swModel* ActiveModel = GetActiveModel();
	swObject* Focus = ActiveModel->GetFocusObject();

	if (!Focus || !Focus->IsPart())
		return;

	swPart* Part = (swPart*)Focus;
	const swTrainTrackInformation* TrainTrackInfo = Part->mPartData->GetTrainTrackInfo();

	if (!TrainTrackInfo)
		return;

	if (mTrackToolSection < SW_PART_SECTION_TRAIN_TRACK_CONNECTION_FIRST)
		return;

	int ConnectionIndex = mTrackToolSection - SW_PART_SECTION_TRAIN_TRACK_CONNECTION_FIRST;
	const swTrainTrackConnectionType& ConnectionType = TrainTrackInfo->GetConnections()[ConnectionIndex].Type;
	std::vector<swPartData*> TrainTrackParts = swGetPartsLibrary()->GetVisibleTrainTrackParts(ConnectionType);

	auto PartSortFunc = [](const swPartData* a, const swPartData* b)
	{
		if (!a)
			return true;

		if (!b)
			return false;

		return strcmp(a->m_strDescription, b->m_strDescription) < 0;
	};

	std::sort(TrainTrackParts.begin(), TrainTrackParts.end(), PartSortFunc);

	std::vector<std::pair<swPartData*, std::string>> Parts;

	Parts.reserve(TrainTrackParts.size());

	for (swPartData* PartData : TrainTrackParts)
		Parts.emplace_back(PartData, std::string());

	std::optional<swPartData*> Result = swShowPartSelectionPopup(nullptr, Parts, gMainWindow->GetInsertColorIndex(), mWidget, QCursor::pos());
	swPartData* PartData = Result.has_value() ? Result.value() : nullptr;

	if (PartData)
	{
		Part = ActiveModel->AddPart(PartData, mTrackToolSection);

		if (Part)
			ActiveModel->UpdateTrainTrackConnections(Part, false);
	}

	mWidget->UpdateMousePosition();
}

// fill a menu from the context menu layout
static void swAddContextMenuEntries(QMenu* Menu, const QStringList& Layout, int& LineIndex, int Depth, bool FlexPart)
{
	QAction** Actions = gMainWindow->mActions;

	while (LineIndex < Layout.size())
	{
		const QString& Line = Layout[LineIndex];
		int LineDepth = 0;

		while (LineDepth < Line.size() && Line[LineDepth] == QLatin1Char('\t'))
			LineDepth++;

		if (LineDepth < Depth)
			return;

		const QString Entry = Line.mid(LineDepth);
		LineIndex++;

		if (LineDepth > Depth || Entry.isEmpty())
			continue;

		if (Entry == QLatin1String("-"))
			Menu->addSeparator();
		else if (Entry.startsWith(QLatin1Char('>')))
		{
			QMenu* SubMenu = Menu->addMenu(qApp->translate("ContextMenu", Entry.mid(1).toUtf8().constData()));
			swAddContextMenuEntries(SubMenu, Layout, LineIndex, Depth + 1, FlexPart);

			// hide submenus left empty

			const QList<QAction*> SubActions = SubMenu->actions();

			if (std::all_of(SubActions.begin(), SubActions.end(), [](const QAction* Action) { return Action->isSeparator(); }))
				Menu->removeAction(SubMenu->menuAction());
		}
		else if (Entry.startsWith(QLatin1Char('@')))
		{
			QMenu* Builtin = gMainWindow->findChild<QMenu*>(Entry.mid(1));

			if (Builtin)
				Menu->addMenu(Builtin);
		}
		else
		{
			const int CommandIndex = swFindCommand(Entry);

			// control points only on flex parts

			if (CommandIndex < 0 || (!FlexPart && CommandIndex >= SW_PART_CONTROL_POINT_INSERT && CommandIndex <= SW_PART_CONTROL_POINT_ALIGN_Z))
				continue;

			Menu->addAction(Actions[CommandIndex]);
		}
	}
}

void swView::ShowContextMenu(bool AllEntries) const
{
	if (mViewType != swViewType::View)
		return;

	const swModel* ActiveModel = GetActiveModel();
	const swPart* FocusPart = ActiveModel ? dynamic_cast<const swPart*>(ActiveModel->GetFocusObject()) : nullptr;
	const bool FlexPart = AllEntries || (FocusPart && FocusPart->mPartData && FocusPart->mPartData->GetSynthInfo());
	const QStringList& Custom = swGetPreferences().mContextMenu;
	const QStringList Layout = Custom.isEmpty() ? swGetDefaultContextMenu() : Custom;

	QMenu* Popup = new QMenu();
	int LineIndex = 0;

	swAddContextMenuEntries(Popup, Layout, LineIndex, 0, FlexPart);

	Popup->exec(QCursor::pos());
	delete Popup;
}

bool swView::CloseFindReplaceDialog()
{
	if (mFindWidget && (mWidget->hasFocus() || mFindWidget->focusWidget()))
	{
		delete mFindWidget;
		mFindWidget = nullptr;

		return true;
	}

	return false;
}

void swView::ShowFindReplaceWidget(bool Replace)
{
	delete mFindWidget;
	mFindReplaceParams = swFindReplaceParameters();
	mFindWidget = new swFindReplaceWidget(mWidget, GetActiveModel(), Replace);
	mWidget->SetFindReplaceWidget(mFindWidget);
}
