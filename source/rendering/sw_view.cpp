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
			View->Redraw();
	}
}

void swView::UpdateAllViews()
{
	for (swView* View : mViews)
		View->Redraw();
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

void swView::MakeCurrent()
{
	mContext->MakeCurrent();
}

void swView::Redraw()
{
	if (mWidget)
	{
		mWidget->UpdateModelEditingPath();
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
	mSubmodelEditingPath.push_back(Part);
	mActiveSubmodelInstance = Part;

	ActiveModel = mActiveSubmodelInstance->mPartData->GetModel();
	ActiveModel->SetTemporaryStep(ActiveModel->GetLastStep());
	RestoreModelEditingContext();

	RemoveCamera();

	GetActiveModel()->UpdateInterface();
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

void swView::ShowContextMenu() const
{
	if (mViewType != swViewType::View)
		return;

	const swModel* ActiveModel = GetActiveModel();
	const swPart* FocusPart = ActiveModel ? dynamic_cast<const swPart*>(ActiveModel->GetFocusObject()) : nullptr;
	const bool FlexPart = FocusPart && FocusPart->mPartData && FocusPart->mPartData->GetSynthInfo();
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
