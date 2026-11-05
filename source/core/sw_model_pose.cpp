#include "sw_global.h"
#include "sw_model.h"
#include "sw_model_history.h"
#include "sw_object_part.h"
#include "sw_project.h"
#include "sw_window_main.h"
#include "sw_library_shadow.h"
#include "sw_data_part.h"
#include "sw_view.h"

static swMatrix33 swRotationFromVector(const swVector3& Rotation)
{
	const float Angle = swLength(Rotation);

	if (Angle < 0.00001f)
		return swMatrix33Identity();

	return swMatrix33FromAxisAngle(Rotation / Angle, Angle * SW_DEGREES_TO_RADIANS);
}

static swVector3 swVectorFromRotation(const swMatrix33& Rotation)
{
	const swVector4 AxisAngle = swMatrix44ToAxisAngle(swMatrix44(Rotation, swVector3(0.0f, 0.0f, 0.0f)));

	return swVector3(AxisAngle[0], AxisAngle[1], AxisAngle[2]) * (AxisAngle[3] * SW_RADIANS_TO_DEGREES);
}

// use to list project models
static std::vector<swModel*> swGetPoseModels(const Project* Project, swModel* Model)
{
	std::vector<swModel*> Models;

	if (Project)
		for (const std::unique_ptr<swModel>& ProjectModel : Project->GetModels())
			Models.push_back(ProjectModel.get());

	if (std::find(Models.begin(), Models.end(), Model) == Models.end())
		Models.push_back(Model);

	return Models;
}

swPose swModel::GetCurrentPose(const QString& Name) const
{
	swPose Pose;
	Pose.Name = Name;

	for (swModel* Model : swGetPoseModels(mProject, const_cast<swModel*>(this)))
		for (size_t Index = 0; Index < Model->mParts.size(); Index++)
			if (Model->mParts[Index]->mJoint.Type != swJointType::None && Model->mParts[Index]->mPartData->IsModel())
				Pose.Joints.push_back({ Model->GetFileName(), static_cast<int>(Index), Model->mParts[Index]->mJoint.Rotation });

	return Pose;
}

// use to pose joints between two poses
void swModel::ApplyPoseBlend(const swPose& From, const swPose& To, float Fraction)
{
	const std::vector<swModel*> Models = swGetPoseModels(mProject, this);

	for (const swPoseJoint& Joint : From.Joints)
	{
		const auto ModelIt = std::find_if(Models.begin(), Models.end(), [&Joint](const swModel* Model) { return Model->GetFileName() == Joint.Model; });

		if (ModelIt == Models.end() || Joint.Index < 0 || Joint.Index >= static_cast<int>((*ModelIt)->mParts.size()))
			continue;

		swPart* Part = (*ModelIt)->mParts[Joint.Index].get();

		if (Part->mJoint.Type == swJointType::None)
			continue;

		swVector3 Rotation = Joint.Rotation;
		const auto ToIt = std::find_if(To.Joints.begin(), To.Joints.end(), [&Joint](const swPoseJoint& Other) { return Other.Model == Joint.Model && Other.Index == Joint.Index; });

		if (ToIt != To.Joints.end())
			Rotation = swBlendRotation(Joint.Rotation, ToIt->Rotation, Fraction, Part->mJoint.Type == swJointType::Hinge ? &Part->mJoint.Axis : nullptr);

		Part->SetJointRotation(Rotation, (*ModelIt)->mCurrentStep);
	}

	std::vector<swModel*> UpdatedModels;

	for (swModel* Model : Models)
		Model->UpdatePartData(UpdatedModels);
}

void swModel::RecallPoseAction(const swPose& Pose)
{
	BeginPoseEdit();
	ApplyPoseBlend(Pose, Pose, 0.0f);
	EndPoseEdit(tr("Recall Pose"), true);
}

// use to record edits in every model
void swModel::BeginPoseEdit()
{
	BeginHistorySequence();
	BeginEditHistory();
	mPoseEdits.clear();

	for (swModel* Model : swGetPoseModels(mProject, this))
	{
		if (Model == this)
			continue;

		std::unique_ptr<swModelHistoryEdit> Edit = std::make_unique<swModelHistoryEdit>(swModelHistoryEditMergeKey{ swModelHistoryEditMerge::None, swObjectPropertyId::Count, Model->mCurrentStep, false });
		Edit->SaveStartState(Model);
		mPoseEdits.emplace_back(Model, std::move(Edit));
	}
}

void swModel::EndPoseEdit(const QString& Description, bool Accept)
{
	if (!Accept)
	{
		for (const auto& [Model, Edit] : mPoseEdits)
			Edit->LoadStartState(Model);

		mPoseEdits.clear();
		RevertHistorySequence();
	}
	else
	{
		EndEditHistory();

		for (auto& [Model, Edit] : mPoseEdits)
		{
			Edit->SaveEndState(Model);

			if (Edit->StateChanged())
			{
				Model->CalculateStep(Model->mCurrentStep);
				mHistorySequence.emplace_back(std::make_unique<swModelHistorySubModel>(Model->GetFileName(), std::move(Edit)));
			}
		}

		mPoseEdits.clear();
	}

	std::vector<swModel*> UpdatedModels;

	for (swModel* Model : swGetPoseModels(mProject, this))
		Model->UpdatePartData(UpdatedModels);

	if (Accept)
		EndHistorySequence(Description);

	if (gMainWindow)
		gMainWindow->UpdateSelectedObjects(false);

	UpdateAllViews();
}

// use to find the parts under a ray
bool swModel::PoseRayTest(const swVector3& Start, const swVector3& End, float& Distance, std::vector<std::pair<swModel*, swPart*>>& Chain, std::vector<std::pair<swModel*, swPart*>>& Hit)
{
	bool Found = false;

	for (const std::unique_ptr<swPart>& Part : mParts)
	{
		if (!(Chain.empty() ? Part->IsSelected() || Part->IsVisible(mCurrentStep) : Part->IsVisibleInSubModel()))
			continue;

		const swMatrix44 InverseWorld = swMatrix44AffineInverse(Part->mModelWorld);
		const swVector3 LocalStart = swMultiply31(Start, InverseWorld);
		const swVector3 LocalEnd = swMultiply31(End, InverseWorld);

		if (Part->mPartData->IsModel())
		{
			Chain.emplace_back(this, Part.get());
			Found |= Part->mPartData->GetModel()->PoseRayTest(LocalStart, LocalEnd, Distance, Chain, Hit);
			Chain.pop_back();
		}
		else
		{
			swPartRayTest PartRayTest;

			if (Part->MinIntersectDist(LocalStart, LocalEnd, Distance, PartRayTest))
			{
				Hit = Chain;
				Hit.emplace_back(this, Part.get());
				Found = true;
			}
		}
	}

	return Found;
}

// use to find the one selected submodel
swPart* swModel::GetJointPart() const
{
	swPart* Found = nullptr;

	for (const std::unique_ptr<swPart>& Part : mParts)
	{
		if (!Part->IsSelected() || !Part->mPartData->IsModel())
			continue;

		if (Found)
			return nullptr;

		Found = Part.get();
	}

	return Found;
}

void swModel::StartJointPick()
{
	mJointPick = GetJointPart() != nullptr;
}

bool swModel::PoseToolClicked(const swVector3& Start, const swVector3& End, const swVector3& Right, const swVector3& Up)
{
	float Distance = FLT_MAX;
	std::vector<std::pair<swModel*, swPart*>> Chain, Hit;

	mPoseJoint = nullptr;

	if (!PoseRayTest(Start, End, Distance, Chain, Hit) || Hit.size() < 2)
		return false;

	// world of the part at each depth
	const auto GetWorld = [&Hit](size_t Count, size_t First)
	{
		swMatrix44 World = swMatrix44Identity();

		for (size_t Index = First; Index < Count; Index++)
			World = swMultiply(Hit[Index].second->mModelWorld, World);

		return World;
	};

	if (mJointPick)
	{
		swPart* Target = GetJointPart();
		mJointPick = false;

		if (!Target || Hit[0].second != Target)
			return false;

		const swMatrix44 InverseWorld = swMatrix44AffineInverse(Target->mModelWorld);
		const swVector3 LocalStart = swMultiply31(Start, InverseWorld);
		const swVector3 Direction = swMultiply31(End, InverseWorld) - LocalStart;
		const swMatrix44 Local = GetWorld(Hit.size(), 1);
		const swPart* Leaf = Hit.back().second;
		swPartJoint Joint;
		float BestDistance = FLT_MAX;

		// the connection nearest the click
		for (const swConnectionCylinder& Cylinder : swShadowLibrary::GetCylinders(Leaf->mPartData->mFileName))
		{
			if (Cylinder.Kind == swConnectionKind::Cylinder && (Cylinder.Sections.empty() || Cylinder.Sections[0].Shape != swConnectionShape::Round))
				continue;

			const swMatrix44 Frame = swMultiply(swMatrix44LDrawToStudWorx(Cylinder.Transform), Local);
			const float Away = swLength(swCross(Frame.GetTranslation() - LocalStart, Direction)) / swLength(Direction);

			if (Away < BestDistance)
			{
				BestDistance = Away;
				Joint.Type = Cylinder.Kind == swConnectionKind::Ball ? swJointType::Ball : swJointType::Hinge;
				Joint.Pivot = Frame.GetTranslation();
				Joint.Axis = swNormalize(swMultiply30(swVector3(0.0f, 0.0f, 1.0f), Frame));
			}
		}

		// parts without shadow data pivot at their origin
		if (Joint.Type == swJointType::None)
		{
			Joint.Type = swJointType::Hinge;
			Joint.Pivot = Local.GetTranslation();
			Joint.Axis = swVector3(1.0f, 0.0f, 0.0f);
		}

		BeginHistorySequence();
		BeginEditHistory();
		Target->mJoint = Joint;
		EndEditHistory();
		EndHistorySequence(tr("Set Joint"));

		if (gMainWindow)
			gMainWindow->UpdateSelectedObjects(false);

		return false;
	}

	// innermost submodel with a joint
	for (size_t Index = Hit.size() - 1; Index-- > 0;)
	{
		swPart* Joint = Hit[Index].second;

		if (Joint->mJoint.Type == swJointType::None)
			continue;

		mPoseJoint = Joint;
		mPoseStartRotation = Joint->mJoint.Rotation;
		mPoseWorldRotation = swMatrix33(swMultiply(Joint->mModelWorld, GetWorld(Index, 0)));
		mPoseRight = Right;
		mPoseUp = Up;

		BeginPoseEdit();

		return true;
	}

	return false;
}

void swModel::UpdatePoseTool(float DeltaX, float DeltaY)
{
	if (!mPoseJoint)
		return;

	// angles follow the snap
	const auto Snap = [this](float Angle) { return SnapRotation(swVector3(Angle, Angle, Angle))[0]; };
	const swPartJoint& Joint = mPoseJoint->mJoint;
	swVector3 Rotation;

	if (Joint.Type == swJointType::Hinge)
	{
		const swVector3 Axis = swNormalize(Joint.Axis);

		Rotation = Axis * (swDot(mPoseStartRotation, Axis) + Snap(DeltaX * 0.5f));
	}
	else
	{
		const swMatrix33 Turn = swMultiply(swMatrix33FromAxisAngle(mPoseUp, Snap(DeltaX * 0.5f) * SW_DEGREES_TO_RADIANS), swMatrix33FromAxisAngle(mPoseRight, Snap(DeltaY * 0.5f) * SW_DEGREES_TO_RADIANS));

		// world turn in child terms
		Rotation = swVectorFromRotation(swMultiply(mPoseWorldRotation, swMultiply(Turn, swMultiply(swMatrix33AffineInverse(mPoseWorldRotation), swRotationFromVector(mPoseStartRotation)))));
	}

	mPoseJoint->SetJointRotation(Rotation, mCurrentStep);
	UpdateAllViews();
}

void swModel::EndPoseTool(bool Accept)
{
	if (!mPoseJoint)
		return;

	mPoseJoint = nullptr;
	EndPoseEdit(tr("Pose"), Accept);
}

void swModel::SetJointAxisAction(swPart* Part, int Axis)
{
	BeginHistorySequence();
	BeginEditHistory();

	// rest pose before the new axis
	Part->SetJointRotation(swVector3(0.0f, 0.0f, 0.0f), mCurrentStep);
	Part->mJoint.Axis = swVector3(Axis == 0 ? 1.0f : 0.0f, Axis == 1 ? 1.0f : 0.0f, Axis == 2 ? 1.0f : 0.0f);

	EndEditHistory();
	EndHistorySequence(tr("Joint Axis"));
}

void swModel::ClearJointAction(swPart* Part)
{
	BeginHistorySequence();
	BeginEditHistory();

	Part->SetJointRotation(swVector3(0.0f, 0.0f, 0.0f), mCurrentStep);
	Part->mJoint = swPartJoint();

	EndEditHistory();
	EndHistorySequence(tr("Clear Joint"));
}
