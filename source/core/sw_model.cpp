#include "sw_global.h"
#include "sw_model.h"
#include "sw_model_history.h"
#include "sw_object_part.h"
#include "sw_object_camera.h"
#include "sw_object_light.h"
#include "sw_group.h"
#include "sw_project.h"
#include "sw_window_main.h"
#include "sw_profile.h"
#include "sw_library.h"
#include "sw_scene.h"
#include "sw_track_train.h"
#include "sw_file.h"
#include "sw_data_part.h"
#include "sw_view.h"
#include "sw_builder_minifigure.h"
#include "sw_dialog_array.h"
#include "sw_dialog_group.h"
#include "sw_dialog_groups_edit.h"
#include "sw_dialog_properties.h"
#include "sw_window_utilities.h"
#include "sw_lxf.h"
#include "sw_widget_preview.h"
#include "sw_string.h"
#include <locale.h>

swModel::swModel(const QString& FileName, Project* Project, bool Preview)
	: mProject(Project), mIsPreview(Preview)
{
	mProperties.mModelName = FileName;
	mProperties.mFileName = FileName;
	mProperties.LoadDefaults();

	mActive = false;
	mCurrentStep = 1;
}

void swModel::SetProject(Project* Project)
{
	if (mProject && mProject != Project)
	{
		const QString SourceFileName = mProject->GetFileName();
		const QString SourceDirectory = SourceFileName.isEmpty() ? QString() : QFileInfo(SourceFileName).absolutePath();

		PreserveAssetSourceDirectory(SourceDirectory, false);
	}

	mProject = Project;
}

void swModel::PreserveAssetSourceDirectory(const QString& Directory, bool PreferCurrentDirectory)
{
	if (!Directory.isEmpty())
	{
		const QString CleanDirectory = QDir::cleanPath(Directory);
		const std::vector<QString>::iterator NewEnd = std::remove(mAssetSourceDirectories.begin(), mAssetSourceDirectories.end(), CleanDirectory);

		mAssetSourceDirectories.erase(NewEnd, mAssetSourceDirectories.end());

		if (mPreferCurrentAssetDirectory)
			mAssetSourceDirectories.insert(mAssetSourceDirectories.begin(), CleanDirectory);
		else
			mAssetSourceDirectories.push_back(CleanDirectory);
	}

	mPreferCurrentAssetDirectory = PreferCurrentDirectory;
}

swModelAssetSourceState swModel::GetAssetSourceState() const
{
	return { mAssetSourceDirectories, mPreferCurrentAssetDirectory };
}

void swModel::RestoreAssetSourceState(const swModelAssetSourceState& State)
{
	mAssetSourceDirectories = State.Directories;
	mPreferCurrentAssetDirectory = State.PreferCurrentDirectory;
}

std::vector<QString> swModel::GetAssetSearchDirectories() const
{
	std::vector<QString> Directories;
	QString CurrentDirectory;

	if (mProject && !mProject->GetFileName().isEmpty())
		CurrentDirectory = QFileInfo(mProject->GetFileName()).absolutePath();

	if (mPreferCurrentAssetDirectory && !CurrentDirectory.isEmpty())
		Directories.push_back(CurrentDirectory);

	for (const QString& Directory : mAssetSourceDirectories)
	{
		if (mPreferCurrentAssetDirectory && Directory == CurrentDirectory)
			continue;

		Directories.push_back(Directory);
	}

	if (!mPreferCurrentAssetDirectory && !CurrentDirectory.isEmpty() &&
		std::find(mAssetSourceDirectories.begin(), mAssetSourceDirectories.end(), CurrentDirectory) == mAssetSourceDirectories.end())
		Directories.push_back(CurrentDirectory);

	return Directories;
}

swModel::~swModel()
{
	mPreviewInsertPartInfo.clear();

	if (mPartData)
	{
		if (mPartData->GetModel() == this)
		{
			if (!mIsPreview && gMainWindow && gMainWindow->GetInsertPartData() == mPartData)
				gMainWindow->SetInsertPartData(nullptr);

			// keep the name while placements use it
			swGetPartsLibrary()->InvalidatePartDataLoad(mPartData.Get());
			mPartData->DetachContainer();
			mPartData->SetFailed(tr("Model '%1' was deleted.").arg(QString::fromLatin1(mPartData->mFileName)));
			swGetPartsLibrary()->NotifyAssetStatusChanged();
		}

		mPartData.Reset();
	}

	DeleteModel();
}

bool swModel::GetPartWorldMatrix(swPart* Part, swMatrix44& ParentWorldMatrix) const
{
	for (const std::unique_ptr<swPart>& ModelPart : mParts)
	{
		if (ModelPart.get() == Part)
		{
			ParentWorldMatrix = swMultiply(ModelPart->mModelWorld, ParentWorldMatrix);
			return true;
		}

		const swPartData* PartData = ModelPart->mPartData.Get();

		if (PartData->IsModel())
		{
			swMatrix44 WorldMatrix = swMultiply(ModelPart->mModelWorld, ParentWorldMatrix);

			if (PartData->GetPartWorldMatrix(Part, WorldMatrix))
			{
				ParentWorldMatrix = WorldMatrix;
				return true;
			}
		}
	}

	return false;
}

bool swModel::IncludesModel(const swModel* Model) const
{
	std::unordered_set<const swModel*> Visited;
	const std::function<bool(const swModel*)> Visit = [Model, &Visited, &Visit](const swModel* Current)
	{
		if (Current == Model)
			return true;

		if (!Current || !Visited.insert(Current).second)
			return false;

		for (const std::unique_ptr<swPart>& Part : Current->mParts)
		{
			const swPartData* PartData = Part->mPartData.Get();

			if (PartData && PartData->IsModel() && Visit(PartData->GetModel()))
				return true;
		}

		return false;
	};

	return Visit(this);
}

void swModel::DeleteModel()
{
	if (gMainWindow)
	{
		std::vector<swView*> Views = swView::GetModelViews(this);

		// todo avoid the camera view pointer reset
		for (swView* View : Views)
		{
			swCamera* ViewCamera = View->GetCamera();

			if (!ViewCamera->IsSimple())
			{
				for (const std::unique_ptr<swCamera>& Camera : mCameras)
				{
					if (Camera.get() == ViewCamera)
					{
						View->SetCamera(ViewCamera, true);
						break;
					}
				}
			}
		}
	}

	mParts.clear();
	mCameras.clear();
	mLights.clear();
	mGroups.clear();
	mFileLines.clear();
	mHasDirectGeometry = false;
}

void swModel::CreatePartData(Project* Project)
{
	swPartsLibrary* Library = swGetPartsLibrary();
	bool Reused;
	mPartData = Library->AcquireModelPartData(mProperties.mFileName.toLatin1().constData(), Project, Reused);
	mPartData->SetModel(this);

	if (Reused)
	{
		// a reused name can make a cycle
		mParts.erase(std::remove_if(mParts.begin(), mParts.end(), [this](const std::unique_ptr<swPart>& Part)
		{
			return Part->mPartData && Part->mPartData->IncludesModel(this);
		}), mParts.end());
	}

	// the model owns the part data reference
	if (!Project->DefersModelMeshRequests())
	{
		if (mIsPreview)
			Library->RequestPartDataLoad(mPartData.Get(), swLoadPriority::Normal);
		else
			Library->WaitForPartData(mPartData);
	}

	if (Reused)
	{
		UpdateAllViews();

		if (!Project->DefersModelMeshRequests())
			RefreshPreview();
	}
}

bool swModel::RebindPartData(swPartData* Previous, swPartData* Replacement)
{
	bool Changed = false;

	mParts.erase(std::remove_if(mParts.begin(), mParts.end(), [this, Previous, Replacement, &Changed](const std::unique_ptr<swPart>& Part)
	{
		if (Part->mPartData != Previous)
			return false;

		Changed = true;

		if (Replacement->IncludesModel(this))
			return true;

		const QString PartId = Part->GetID();
		Part->SetPartData(swPartDataReference(Replacement), PartId, false, false);

		return false;
	}), mParts.end());

	return Changed;
}

void swModel::QueueMeshBuild()
{
	mPartData->SetModel(this);
	swGetPartsLibrary()->RebuildModelPartData(mPartData.Get());
}

void swModel::UpdateAllViews() const
{
	swView::UpdateProjectViews(mProject);
}

void swModel::RefreshPreview() const
{
	if (mIsPreview || !mProject)
		return;

	if (gMainWindow && gMainWindow->GetPreviewWidget())
		gMainWindow->GetPreviewWidget()->RefreshModel(this);
}

std::vector<swPartDataReference> swModel::GetRequiredPartData() const
{
	std::vector<swPartDataReference> RequiredPartData;
	std::unordered_set<const swModel*> Visited;

	const std::function<void(const swModel*)> Collect = [&RequiredPartData, &Visited, &Collect](const swModel* Model)
	{
		if (!Model || !Visited.insert(Model).second)
			return;

		if (Model->mPartData)
			RequiredPartData.push_back(Model->mPartData);

		for (const std::unique_ptr<swPart>& Part : Model->mParts)
		{
			const swPartDataReference& PartData = Part->mPartData;

			if (PartData->IsModel())
				Collect(PartData->GetModel());
			else if (PartData->IsProject())
				Collect(PartData->GetProject()->GetMainModel());
			else
				RequiredPartData.push_back(PartData);
		}
	};

	Collect(this);

	std::sort(RequiredPartData.begin(), RequiredPartData.end());
	RequiredPartData.erase(std::unique(RequiredPartData.begin(), RequiredPartData.end()), RequiredPartData.end());

	return RequiredPartData;
}

std::vector<swPart*> swModel::GetRequiredSynthParts() const
{
	std::vector<swPart*> Required;
	std::unordered_set<const swModel*> Visited;

	const std::function<void(const swModel*)> Collect = [&Required, &Visited, &Collect](const swModel* Model)
	{
		if (!Model || !Visited.insert(Model).second)
			return;

		for (const std::unique_ptr<swPart>& Part : Model->mParts)
		{
			swPartData* PartData = Part->mPartData.Get();

			if (PartData->IsModel())
				Collect(PartData->GetModel());
			else if (PartData->IsProject())
				Collect(PartData->GetProject()->GetMainModel());
			else if (PartData->GetSynthInfo())
				Required.push_back(Part.get());
		}
	};

	Collect(this);

	return Required;
}

void swModel::WaitForAssets() const
{
	const std::vector<swPartDataReference> RequiredPartData = GetRequiredPartData();
	swPartsLibrary* Library = swGetPartsLibrary();

	swBlockingProgress Progress(tr("Loading"), tr("Loading parts..."), false);

	Library->WaitForPartData(RequiredPartData, [&Progress](int Finished, int Total)
	{
		Progress.SetProgress(Finished, Total);
	});

	Library->WaitForSynthMeshes(GetRequiredSynthParts());

	// temporary models have no container part
	if (mPartData)
	{
		std::vector<swModel*> UpdatedModels;
		const_cast<swModel*>(this)->UpdatePartData(UpdatedModels);
	}
}

swResult<void> swModel::EnsureAssetsReady() const
{
	WaitForAssets();

	const std::vector<swPartDataReference> RequiredPartData = GetRequiredPartData();
	swPartsLibrary* Library = swGetPartsLibrary();

	for (const swPartDataReference& RequiredData : RequiredPartData)
	{
		if (RequiredData->mState == swAssetState::Loaded)
			continue;

		const QString Error = Library->GetPartDataLoadError(RequiredData.Get());

		if (!Error.isEmpty())
			return swUnexpected(Error);

		return swUnexpected(tr("Could not load all required parts."));
	}

	const std::vector<swPart*> GeneratedParts = GetRequiredSynthParts();

	for (const swPart* Part : GeneratedParts)
		if (!Part->HasGeneratedMesh())
		{
			QString Error = Library->GetSynthMeshError(Part);

			if (Error.isEmpty())
				Error = tr("Could not generate geometry for part '%1'.").arg(QString::fromLatin1(Part->mPartData->mFileName));

			return swUnexpected(Error);
		}

	return swResult<void>();
}

void swModel::UpdatePartData(std::vector<swModel*>& UpdatedModels)
{
	if (std::find(UpdatedModels.begin(), UpdatedModels.end(), this) != UpdatedModels.end())
		return;

	mPartData->SetModel(this);
	UpdatedModels.push_back(this);

	const swMesh* Mesh = mPartData->GetDisplayMesh();

	if (mParts.empty() && !Mesh)
	{
		mPartData->SetBoundingBox(swVector3(0.0f, 0.0f, 0.0f), swVector3(0.0f, 0.0f, 0.0f));
		return;
	}

	swVector3 Min(FLT_MAX, FLT_MAX, FLT_MAX), Max(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (const std::unique_ptr<swPart>& Part : mParts)
	{
		if (Part->IsVisibleInSubModel())
		{
			Part->mPartData->UpdateBoundingBox(UpdatedModels);
			Part->CompareBoundingBox(Min, Max);
		}
	}

	if (Mesh)
	{
		Min = swMinimum(Min, Mesh->mBoundingBox.Min);
		Max = swMaximum(Max, Mesh->mBoundingBox.Max);
	}

	mPartData->SetBoundingBox(Min, Max);
}

void swModel::SetActive(bool Active)
{
	CalculateStep(Active ? mCurrentStep : SW_STEP_MAXIMUM);
	mActive = Active;

	if (Active)
		UpdateControlPointSeats();
}

void swModel::CalculateStep(swStep Step)
{
	for (const std::unique_ptr<swPart>& Part : mParts)
		Part->UpdatePosition(Step);

	for (std::unique_ptr<swCamera>& Camera : mCameras)
		Camera->UpdatePosition(Step);

	for (const std::unique_ptr<swLight>& Light : mLights)
		Light->UpdatePosition(Step);
}

void swModel::SetCurrentStep(swStep Step)
{
	const bool StepChanged = Step != mCurrentStep;
	mCurrentStep = Step;
	CalculateStep(Step);
	UpdateControlPointSeats();

	if (gMainWindow)
	{
		gMainWindow->UpdateTimeline(false, false);
		gMainWindow->UpdateSelectedObjects(true);
	}

	// only step changes turn the view
	if (StepChanged && swGetPreferences().mFollowStepViews && !mIsPreview)
		for (swView* View : swView::GetModelViews(this))
			View->ApplyStepView();

	UpdateAllViews();

	if (gMainWindow)
		gMainWindow->UpdateCurrentStep();
}

void swModel::ShowFirstStep()
{
	if (mCurrentStep == 1)
		return;

	SetCurrentStep(1);
}

void swModel::ShowLastStep()
{
	const swStep LastStep = GetLastStep();

	if (mCurrentStep == LastStep)
		return;

	SetCurrentStep(LastStep);
}

void swModel::ShowPreviousStep()
{
	if (mCurrentStep == 1)
		return;

	SetCurrentStep(mCurrentStep - 1);
}

void swModel::ShowNextStep()
{
	if (mCurrentStep == SW_STEP_MAXIMUM)
		return;

	SetCurrentStep(mCurrentStep + 1);
}

swStep swModel::GetLastStep() const
{
	swStep Step = 1;

	for (const std::unique_ptr<swPart>& Part : mParts)
		Step = swMaximum(Step, Part->GetLastStep());

	for (const std::unique_ptr<swCamera>& Camera : mCameras)
		Step = swMaximum(Step, Camera->GetLastStep());

	for (const std::unique_ptr<swLight>& Light : mLights)
		Step = swMaximum(Step, Light->GetLastStep());

	return Step;
}

void swModel::InsertStep(swStep Step)
{
	for (const std::unique_ptr<swPart>& Part : mParts)
		Part->InsertTime(Step, 1);

	for (std::unique_ptr<swCamera>& Camera : mCameras)
		Camera->InsertTime(Step, 1);

	for (const std::unique_ptr<swLight>& Light : mLights)
		Light->InsertTime(Step, 1);
}

void swModel::SetStepView(swStep Step, const std::optional<swStepView>& StepView)
{
	swModelProperties Properties = mProperties;

	if (StepView)
		Properties.mStepViews[Step] = *StepView;
	else
		Properties.mStepViews.erase(Step);

	if (Properties == mProperties)
		return;

	BeginHistorySequence();
	SetModelProperties(Properties);
	EndHistorySequence(tr("Set Step View"));

	SetCurrentStep(mCurrentStep);
}

std::optional<swStepView> swModel::GetEffectiveStepView(swStep Step) const
{
	std::optional<swStepView> Effective;

	// a step view lasts until changed
	for (const std::pair<const swStep, swStepView>& StepView : mProperties.mStepViews)
	{
		if (StepView.first > Step)
			break;

		switch (StepView.second.Mode)
		{
		case swStepViewMode::End:
			Effective.reset();
			break;

		case swStepViewMode::Relative:
		case swStepViewMode::Absolute:
			Effective = StepView.second;
			break;

		case swStepViewMode::Add:
			if (Effective)
				Effective->Angles += StepView.second.Angles;
			else
				Effective = swStepView{ swStepViewMode::Relative, StepView.second.Angles };
			break;
		}
	}

	return Effective;
}

void swModel::ShiftStepViews(swStep Step, int Offset)
{
	swModelProperties Properties = mProperties;
	std::map<swStep, swStepView> StepViews;

	// step views follow their steps
	for (const std::pair<const swStep, swStepView>& StepView : mProperties.mStepViews)
	{
		if (StepView.first < Step)
			StepViews[StepView.first] = StepView.second;
		else if (Offset > 0)
			StepViews[StepView.first + Offset] = StepView.second;
		else if (StepView.first > Step)
			StepViews[StepView.first + Offset] = StepView.second;
	}

	Properties.mStepViews = std::move(StepViews);
	SetModelProperties(Properties);
}

void swModel::InsertStepAction(swStep Step)
{
	BeginHistorySequence();
	BeginEditHistory();

	InsertStep(Step);

	EndEditHistory();
	ShiftStepViews(Step, 1);
	EndHistorySequence(tr("Insert Step"));

	SetCurrentStep(mCurrentStep);
}

void swModel::RemoveStepAction(swStep Step)
{
	BeginHistorySequence();
	BeginEditHistory();

	for (const std::unique_ptr<swPart>& Part : mParts)
		Part->RemoveTime(Step, 1);

	for (std::unique_ptr<swCamera>& Camera : mCameras)
		Camera->RemoveTime(Step, 1);

	for (const std::unique_ptr<swLight>& Light : mLights)
		Light->RemoveTime(Step, 1);

	EndEditHistory();
	ShiftStepViews(Step, -1);
	EndHistorySequence(tr("Delete Step"));

	SetCurrentStep(mCurrentStep);
}

void swModel::GetPartsList(int DefaultColorIndex, bool ScanSubModels, bool AddSubModels, swPartsList& PartsList) const
{
	for (const std::unique_ptr<swPart>& Part : mParts)
	{
		if (!Part->IsVisibleInSubModel())
			continue;

		int ColorIndex = Part->GetColorIndex();

		if (ColorIndex == gDefaultColor)
			ColorIndex = DefaultColorIndex;

		Part->mPartData->GetPartsList(ColorIndex, ScanSubModels, AddSubModels, PartsList);
	}
}

void swModel::GetPartsListForStep(swStep Step, int DefaultColorIndex, swPartsList& PartsList, bool Cumulative) const
{
	for (const std::unique_ptr<swPart>& Part : mParts)
	{
		if (Cumulative ? Part->GetStepShow() > Step : Part->GetStepShow() != Step || Part->IsHidden())
			continue;

		int ColorIndex = Part->GetColorIndex();

		if (ColorIndex == gDefaultColor)
			ColorIndex = DefaultColorIndex;

		Part->mPartData->GetPartsList(ColorIndex, false, true, PartsList);
	}
}

void swModel::GetModelParts(const swMatrix44& WorldMatrix, int DefaultColorIndex, std::vector<swModelPartsEntry>& ModelParts) const
{
	if (mHasDirectGeometry && mPartData->GetMesh())
		ModelParts.emplace_back(swModelPartsEntry{ WorldMatrix, mPartData.Get(), mPartData->GetMesh(), DefaultColorIndex });

	for (const std::unique_ptr<swPart>& Part : mParts)
		Part->GetModelParts(WorldMatrix, DefaultColorIndex, ModelParts);
}

void swModel::ShowPropertiesDialog()
{
	swPropertiesDialogOptions Options;

	Options.Properties = mProperties;
	Options.BoundingBox = GetAllPartsBoundingBox();

	GetPartsList(gDefaultColor, true, false, Options.PartsList);

	swPropertiesDialog Dialog(gMainWindow, &Options);
	if (Dialog.exec() != QDialog::Accepted)
		return;

	if (mProperties == Options.Properties)
		return;

	BeginHistorySequence();
	SetModelProperties(Options.Properties);
	EndHistorySequence(tr("Change Model Properties"));
}

void swModel::ShowArrayDialog()
{
	swVector3 Center;

	if (!GetPartFocusOrSelectionCenter(Center))
	{
		QMessageBox::information(gMainWindow, tr("Part Array"), tr("No parts selected."));
		return;
	}

	swArrayDialog Dialog(gMainWindow);

	if (Dialog.exec() != QDialog::Accepted)
		return;

	if (Dialog.mCounts[0] * Dialog.mCounts[1] * Dialog.mCounts[2] < 2)
	{
		QMessageBox::information(gMainWindow, tr("Part Array"), tr("Array only has 1 element or less. No parts added."));
		return;
	}

	BeginHistorySequence();
	BeginEditHistory();

	std::vector<swObject*> NewParts;

	for (int Step1 = 0; Step1 < Dialog.mCounts[0]; Step1++)
	{
		for (int Step2 = 0; Step2 < Dialog.mCounts[1]; Step2++)
		{
			for (int Step3 = (Step1 == 0 && Step2 == 0) ? 1 : 0; Step3 < Dialog.mCounts[2]; Step3++)
			{
				swMatrix44 ModelWorld;
				swVector3 Position;

				swVector3 RotationAngles = Dialog.mRotations[0] * Step1 + Dialog.mRotations[1] * Step2 + Dialog.mRotations[2] * Step3;
				const swVector3 Offset = Dialog.mOffsets[0] * Step1 + Dialog.mOffsets[1] * Step2 + Dialog.mOffsets[2] * Step3;

				for (const std::unique_ptr<swPart>& Part : mParts)
				{
					if (!Part->IsSelected())
						continue;

					ModelWorld = Part->mModelWorld;

					ModelWorld.r[3] -= swVector4(Center, 0.0f);
					ModelWorld = swMultiply(ModelWorld, swMatrix44RotationX(RotationAngles[0] * SW_DEGREES_TO_RADIANS));
					ModelWorld = swMultiply(ModelWorld, swMatrix44RotationY(RotationAngles[1] * SW_DEGREES_TO_RADIANS));
					ModelWorld = swMultiply(ModelWorld, swMatrix44RotationZ(RotationAngles[2] * SW_DEGREES_TO_RADIANS));
					ModelWorld.r[3] += swVector4(Center, 0.0f);

					Position = swVector3(ModelWorld.r[3].x, ModelWorld.r[3].y, ModelWorld.r[3].z);
					ModelWorld.SetTranslation(Position + Offset);

					swPart* NewPart = new swPart(nullptr);
					NewPart->SetPartData(Part->mPartData, Part->GetID(), true, true);
					NewPart->Initialize(ModelWorld, mCurrentStep);
					NewPart->SetColorIndex(Part->GetColorIndex());

					NewParts.emplace_back(NewPart);
				}
			}
		}
	}

	for (size_t PartIdx = 0; PartIdx < NewParts.size(); PartIdx++)
	{
		swPart* Part = (swPart*)NewParts[PartIdx];
		Part->UpdatePosition(mCurrentStep);
		AddPart(Part);
	}

	EndEditHistory();

	AddToSelection(NewParts);

	EndHistorySequence(tr("Part Array"));

	gMainWindow->UpdateTimeline(false, false);
}

// use to find the part that stays in place, the first and last selected follow the model order
swPart* swModel::GetExplodeAnchor(swExplodeAnchor Anchor) const
{
	if (Anchor == swExplodeAnchor::Focused)
	{
		swObject* Focus = GetFocusObject();
		return Focus && Focus->IsPart() ? static_cast<swPart*>(Focus) : nullptr;
	}

	// shortcut: first and last selected follow the model order, track the click order if users expect that
	swPart* Found = nullptr;

	for (const std::unique_ptr<swPart>& Part : mParts)
	{
		if (!Part->IsSelected())
			continue;

		Found = Part.get();

		if (Anchor == swExplodeAnchor::First)
			break;
	}

	return Found;
}

// use to flatten the submodels and spread the parts away from the anchor, one undo puts it all back
void swModel::Explode(const swExplodeSettings& Settings)
{
	// a new spread starts again from the model the last one flattened
	if (IsExplodeFlattened())
		CollapseExplodeAction();

	swPart* Anchor = GetExplodeAnchor(Settings.Anchor);

	if (!Anchor)
		return;

	const bool HasSubModels = std::any_of(mParts.begin(), mParts.end(), [&Settings](const std::unique_ptr<swPart>& Part)
	{
		return Part->mPartData->IsModel() && (!Settings.SelectedOnly || Part->IsSelected());
	});

	if (HasSubModels)
	{
		BeginHistorySequence();
		BeginEditHistory();

		std::vector<swObject*> Selection;

		FlattenSubModels(Settings.SelectedOnly, Anchor, Selection);
		EndEditHistory();

		SetSelectionAndFocus(Selection, Settings.Anchor == swExplodeAnchor::Focused ? Anchor : nullptr, SW_PART_SECTION_POSITION, swSelectionMode::Single);

		EndHistorySequence(tr("Explode"));

		mExplodeRevision = mUndoHistory.empty() ? 0 : mUndoHistory.front()->Revision;

		gMainWindow->UpdateTimeline(false, false);
		gMainWindow->UpdateInUseCategory();

		if (!Anchor)
			return;
	}

	for (const std::unique_ptr<swPart>& Part : mParts)
		Part->mExplodeOffset = swVector3(0.0f, 0.0f, 0.0f);

	mExplodeLines.clear();

	// with selected parts only a group moves as one part
	const auto GetUnit = [&Settings](const swPart* Part) -> const void*
	{
		swGroup* Group = Part->GetGroup();

		if (Settings.SelectedOnly && Group)
			return Group->GetTopGroup();

		return Part;
	};

	const swVector3 Scale = Settings.Spread - swVector3(1.0f, 1.0f, 1.0f);

	struct swExplodePart
	{
		swPart* Part;
		swVector3 Center;
		swBoundingBox Box;
		swConnectionShapes Shapes;
		swVector3 Offset;
		swVector3 Start;
		swVector3 End;
		const void* Unit;
		bool Line;
		bool Reached;
	};

	std::vector<swExplodePart> ExplodeParts;
	std::vector<size_t> Queue;

	for (const std::unique_ptr<swPart>& Part : mParts)
	{
		if (Part.get() != Anchor && !Part->IsVisible(mCurrentStep))
			continue;

		const swBoundingBox& PartBox = Part->GetBoundingBox();
		swExplodePart ExplodePart = { Part.get(), swMultiply31((PartBox.Min + PartBox.Max) * 0.5f, Part->mModelWorld), { swVector3(FLT_MAX, FLT_MAX, FLT_MAX), swVector3(-FLT_MAX, -FLT_MAX, -FLT_MAX) }, {}, swVector3(0.0f, 0.0f, 0.0f), swVector3(0.0f, 0.0f, 0.0f), swVector3(0.0f, 0.0f, 0.0f), GetUnit(Part.get()), false, false };

		AddPartShapes(Part.get(), swMatrix44Identity(), ExplodePart.Shapes);

		for (const auto& [Cylinders, Transform] : ExplodePart.Shapes)
			swConnectivity::AddToBoundingBox(*Cylinders, Transform, ExplodePart.Box);

		ExplodeParts.push_back(std::move(ExplodePart));
	}

	const void* AnchorUnit = GetUnit(Anchor);
	const swBoundingBox& AnchorBox = Anchor->GetBoundingBox();
	const swVector3 AnchorCenter = swMultiply31((AnchorBox.Min + AnchorBox.Max) * 0.5f, Anchor->mModelWorld);

	// parts that stay put keep a zero offset so the parts hanging on them start from where they sit
	const auto StaysPut = [&Settings, AnchorUnit](const swExplodePart& ExplodePart)
	{
		return ExplodePart.Unit == AnchorUnit || (Settings.SelectedOnly && !ExplodePart.Part->IsSelected());
	};

	// the rest of a part's group moves with it
	const auto ReachUnit = [&ExplodeParts, &Queue, &StaysPut](const void* Unit, const swVector3& Offset)
	{
		for (size_t PartIndex = 0; PartIndex < ExplodeParts.size(); PartIndex++)
		{
			swExplodePart& Member = ExplodeParts[PartIndex];

			if (Member.Reached || Member.Unit != Unit)
				continue;

			Member.Offset = StaysPut(Member) ? swVector3(0.0f, 0.0f, 0.0f) : Offset;
			Member.Reached = true;
			Queue.push_back(PartIndex);
		}
	};

	// the world axis closest to a direction keeps moves and lines orthogonal
	const auto SnapAxis = [](const swVector3& Direction)
	{
		int Axis = 0;

		for (int Index = 1; Index < 3; Index++)
			if (fabsf(Direction[Index]) > fabsf(Direction[Axis]))
				Axis = Index;

		swVector3 Snapped(0.0f, 0.0f, 0.0f);
		Snapped[Axis] = Direction[Axis] < 0.0f ? -1.0f : 1.0f;
		return Snapped;
	};

	// move a part out from its parent along one axis, the line runs through Point on that axis
	const auto MoveFrom = [&Scale, &ReachUnit](const swExplodePart& From, swExplodePart& To, const swVector3& Axis, const swVector3& Point)
	{
		const swVector3 Move = Axis * swDot(To.Center - From.Center, Axis) * Scale;

		To.Start = Point + From.Offset;
		To.End = Point + From.Offset + Move;
		To.Line = true;
		ReachUnit(To.Unit, From.Offset + Move);
	};

	ReachUnit(AnchorUnit, swVector3(0.0f, 0.0f, 0.0f));

	// only compare shapes of nearby parts
	const auto BoxesMeet = [](const swBoundingBox& a, const swBoundingBox& b)
	{
		const float Margin = 1.0f;

		for (int Axis = 0; Axis < 3; Axis++)
			if (a.Min[Axis] > b.Max[Axis] + Margin || b.Min[Axis] > a.Max[Axis] + Margin)
				return false;

		return true;
	};

	// slide each part out along the connection to the part it hangs on
	size_t QueueIndex = 0;

	const auto Spread = [&]()
	{
		for (; QueueIndex < Queue.size(); QueueIndex++)
		{
			const swExplodePart& From = ExplodeParts[Queue[QueueIndex]];

			for (size_t PartIndex = 0; PartIndex < ExplodeParts.size(); PartIndex++)
			{
				swExplodePart& To = ExplodeParts[PartIndex];
				swVector3 Axis;
				swVector3 Point;

				if (To.Reached || To.Shapes.empty() || !BoxesMeet(From.Box, To.Box) || !swConnectivity::GetConnectionAxis(From.Shapes, To.Shapes, Axis, Point))
					continue;

				// centers level on the connection axis give no direction, so use the nearest world axis
				if (fabsf(swDot(To.Center - From.Center, Axis)) < 1.0f)
					Axis = SnapAxis(To.Center - From.Center);

				MoveFrom(From, To, Axis, Point);
			}
		}
	};

	Spread();

	// parts without a connection hang on the nearest placed part, closest to the anchor first
	std::vector<size_t> Loose;

	for (size_t PartIndex = 0; PartIndex < ExplodeParts.size(); PartIndex++)
		if (!ExplodeParts[PartIndex].Reached)
			Loose.push_back(PartIndex);

	std::sort(Loose.begin(), Loose.end(), [&ExplodeParts, &AnchorCenter](size_t a, size_t b)
	{
		return swLengthSquared(ExplodeParts[a].Center - AnchorCenter) < swLengthSquared(ExplodeParts[b].Center - AnchorCenter);
	});

	for (size_t LooseIndex : Loose)
	{
		swExplodePart& To = ExplodeParts[LooseIndex];

		if (To.Reached)
			continue;

		const swExplodePart* From = nullptr;

		for (const swExplodePart& Placed : ExplodeParts)
			if (Placed.Reached && (!From || swLengthSquared(Placed.Center - To.Center) < swLengthSquared(From->Center - To.Center)))
				From = &Placed;

		MoveFrom(*From, To, SnapAxis(To.Center - From->Center), To.Center);
		Spread();
	}

	for (swExplodePart& ExplodePart : ExplodeParts)
	{
		if (StaysPut(ExplodePart))
			continue;

		ExplodePart.Part->mExplodeOffset = ExplodePart.Offset;

		if (Settings.ShowLines && ExplodePart.Line)
		{
			mExplodeLines.push_back(ExplodePart.Start);
			mExplodeLines.push_back(ExplodePart.End);
		}
	}

	for (const std::unique_ptr<swPart>& Part : mParts)
		Part->UpdatePosition(mCurrentStep);

	mExploded = true;

	gMainWindow->UpdateSelectedObjects(true);
	UpdateAllViews();
}

// use to save the exploded model to a separate file, the open project keeps its file and positions
bool swModel::SaveExplodedCopy(const swExplodeSettings& Settings)
{
	Explode(Settings);

	if (!mExploded)
		return false;

	Project* ActiveProject = swGetActiveProject();
	const QString ProjectFileName = ActiveProject->GetFileName();
	const QString DefaultSuffix = ActiveProject->IsMultiPart() ? QLatin1String("mpd") : QLatin1String("ldr");
	const QFileInfo ProjectInfo(ProjectFileName.isEmpty() ? QDir(swGetProfileString(SW_PROFILE_PROJECTS_PATH)).absoluteFilePath(ActiveProject->GetTitle()) : ProjectFileName);
	const QString Suffix = ProjectInfo.suffix().isEmpty() ? DefaultSuffix : ProjectInfo.suffix();
	const QString CopyName = ProjectInfo.absoluteDir().absoluteFilePath(ProjectInfo.completeBaseName() + QLatin1String("-exploded.") + Suffix);

	QString FileName = QFileDialog::getSaveFileName(gMainWindow, tr("Save Exploded Copy"), CopyName, tr("LDraw Models (*.%1);;All Files (*)").arg(DefaultSuffix));

	if (FileName.isEmpty())
		return false;

	if (QFileInfo(FileName).suffix().isEmpty())
		FileName += QLatin1Char('.') + DefaultSuffix;

	if (!ProjectFileName.isEmpty() && QFileInfo(FileName).absoluteFilePath() == QFileInfo(ProjectFileName).absoluteFilePath())
	{
		QMessageBox::warning(gMainWindow, tr("Save Exploded Copy"), tr("The exploded copy needs a name other than the open file."));
		return false;
	}

	// the saved place drops the explode offset, so fold it into the transform while writing
	std::vector<std::pair<swPart*, swVector3>> Offsets;

	for (const std::unique_ptr<swPart>& Part : mParts)
	{
		Offsets.emplace_back(Part.get(), Part->mExplodeOffset);
		Part->mExplodeOffset = swVector3(0.0f, 0.0f, 0.0f);
	}

	// shortcut: parts with position keyframes keep their unexploded keys, write exploded keys if animated models need it
	const swResult<void> Result = ActiveProject->SaveCopy(FileName);

	for (const auto& [Part, Offset] : Offsets)
		Part->mExplodeOffset = Offset;

	if (!Result)
	{
		QMessageBox::warning(gMainWindow, tr("Save Exploded Copy"), Result.error());
		return false;
	}

	return true;
}

// use to tell whether the last edit is the flatten of the spread, selecting parts since does not count
bool swModel::IsExplodeFlattened() const
{
	return mExplodeRevision && GetCurrentHistoryRevision() == mExplodeRevision;
}

// use to put the spread back, with the submodels it flattened when no edit came after
void swModel::CollapseExplodeAction()
{
	if (!IsExplodeFlattened())
	{
		CollapseExplode();
		return;
	}

	while (!mUndoHistory.empty() && mUndoHistory.front()->Revision != mExplodeRevision)
		UndoAction();

	UndoAction();
}

void swModel::CollapseExplode()
{
	if (!mExploded)
		return;

	mExploded = false;
	mExplodeLines.clear();

	for (const std::unique_ptr<swPart>& Part : mParts)
	{
		Part->mExplodeOffset = swVector3(0.0f, 0.0f, 0.0f);
		Part->UpdatePosition(mCurrentStep);
	}

	if (gMainWindow)
		gMainWindow->UpdateSelectedObjects(true);

	UpdateAllViews();
}

void swModel::AddMinifigure(const swMinifigure& Minifigure)
{
	gMainWindow->GetActiveView()->MakeCurrent();

	BeginHistorySequence();
	BeginEditHistory();

	swGroup* Group = AddGroup(tr("Minifigure "), nullptr);
	std::vector<swObject*> Parts;

	Parts.reserve(SW_MINIFIGURE_ITEM_COUNT);

	for (int PartIndex = 0; PartIndex < SW_MINIFIGURE_ITEM_COUNT; PartIndex++)
	{
		if (!Minifigure.PartData[PartIndex])
			continue;

		swPart* Part = new swPart(Minifigure.PartData[PartIndex]);

		Part->Initialize(Minifigure.Matrices[PartIndex], mCurrentStep);
		Part->SetColorIndex(Minifigure.ColorIndices[PartIndex]);
		Part->SetGroup(Group);
		AddPart(Part);
		Part->UpdatePosition(mCurrentStep);

		Parts.emplace_back(Part);
	}

	EndEditHistory();

	SetSelectionAndFocus(Parts, nullptr, 0, swSelectionMode::Single);

	EndHistorySequence(tr("Add Minifigure"));

	gMainWindow->UpdateTimeline(false, false);
}

void swModel::SetMinifigure(const swMinifigure& Minifigure)
{
	DeleteModel();

	std::vector<swObject*> Parts;
	Parts.reserve(SW_MINIFIGURE_ITEM_COUNT);

	for (int PartIdx = 0; PartIdx < SW_MINIFIGURE_ITEM_COUNT; PartIdx++)
	{
		if (!Minifigure.PartData[PartIdx])
			continue;

		swPart* Part = new swPart(Minifigure.PartData[PartIdx]);

		Part->Initialize(Minifigure.Matrices[PartIdx], 1);
		Part->SetColorIndex(Minifigure.ColorIndices[PartIdx]);
		AddPart(Part);
		Part->UpdatePosition(1);

		Parts.emplace_back(Part);
	}
}

void swModel::SetPreviewPartData(swPartData* PartData, int ColorIndex)
{
	DeleteModel();

	swPart* Part = new swPart(PartData);

	Part->Initialize(swMatrix44Identity(), 1);
	Part->SetColorIndex(ColorIndex);
	AddPart(Part);
	Part->UpdatePosition(1);

	mCurrentStep = SW_STEP_MAXIMUM;
	CalculateStep(SW_STEP_MAXIMUM);
}

void swModel::UpdateInterface()
{
	if (!gMainWindow)
		return;

	gMainWindow->UpdateTimeline(true, false);
	gMainWindow->UpdateUndoRedo(!mUndoHistory.empty() ? mUndoHistory[0]->Description : nullptr, !mRedoHistory.empty() ? mRedoHistory[0]->Description : nullptr);
	gMainWindow->UpdatePaste(!gApplication->mClipboard.isEmpty());
	gMainWindow->UpdateCategories();
	gMainWindow->UpdateTitle();
	gMainWindow->SetTool(gMainWindow->GetTool());

	gMainWindow->UpdateSelectedObjects(true);
	gMainWindow->SetTransformType(gMainWindow->GetTransformType());
	gMainWindow->UpdateLockSnap();
	gMainWindow->UpdateSnap();
	gMainWindow->UpdateModels();
	gMainWindow->UpdateShadingMode();
	gMainWindow->UpdateCurrentStep();
	gMainWindow->UpdateSelectionMode();
}
