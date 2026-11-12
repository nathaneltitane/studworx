#include "sw_global.h"
#include "sw_model.h"
#include "sw_model_history.h"
#include "sw_object_part.h"
#include "sw_project.h"
#include "sw_window_main.h"
#include "sw_library_shadow.h"
#include "sw_data_part.h"
#include "sw_view.h"
#include "sw_scene.h"
#include "sw_application.h"

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
static std::vector<swModel*> swGetJointsModels(const Project* Project, swModel* Model)
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

	for (size_t Index = 0; Index < mParts.size(); Index++)
		if (mParts[Index]->mPartData->IsModel())
			for (size_t JointIndex = 0; JointIndex < mParts[Index]->mJoints.size(); JointIndex++)
				Pose.Joints.push_back({ GetFileName(), static_cast<int>(Index), mParts[Index]->mJoints[JointIndex].Rotation, static_cast<int>(JointIndex) });

	return Pose;
}

// use to move old project wide poses into their models
void swModel::DistributePoses(const std::vector<std::unique_ptr<swModel>>& Models)
{
	for (const std::unique_ptr<swModel>& Owner : Models)
	{
		std::vector<swPose> Poses;

		for (swPose& Pose : Owner->mPoses)
		{
			std::vector<swJointAngle> OwnJoints;

			for (const swJointAngle& Joint : Pose.Joints)
			{
				if (Joint.Model.compare(Owner->GetFileName(), Qt::CaseInsensitive) == 0)
				{
					OwnJoints.push_back(Joint);
					continue;
				}

				const auto Target = std::find_if(Models.begin(), Models.end(), [&Joint](const std::unique_ptr<swModel>& Model) { return Model->GetFileName().compare(Joint.Model, Qt::CaseInsensitive) == 0; });

				if (Target == Models.end())
					continue;

				std::vector<swPose>& TargetPoses = Target->get() == Owner.get() ? Poses : (*Target)->mPoses;
				auto TargetPose = std::find_if(TargetPoses.begin(), TargetPoses.end(), [&Pose](const swPose& Other) { return Other.Name == Pose.Name; });

				if (TargetPose == TargetPoses.end())
				{
					TargetPoses.push_back({ Pose.Name, {} });
					TargetPose = TargetPoses.end() - 1;
				}

				TargetPose->Joints.push_back(Joint);
			}

			if (!OwnJoints.empty())
				Poses.push_back({ Pose.Name, OwnJoints });
		}

		Owner->mPoses = Poses;
	}
}

// use to read joints and old pose lines
void swModel::ReadPoseLine(const QString& Line)
{
	static const QRegularExpression PoseExpression(QStringLiteral("^\\s*\"([^\"]*)\"\\s+\"([^\"]*)\"\\s+(\\d+)\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)(?:\\s+(\\d+))?"));
	const QRegularExpressionMatch Match = PoseExpression.match(Line);

	if (!Match.hasMatch())
		return;

	auto Pose = std::find_if(mPoses.begin(), mPoses.end(), [&Match](const swPose& Other) { return Other.Name == Match.captured(1); });

	if (Pose == mPoses.end())
	{
		mPoses.emplace_back();
		mPoses.back().Name = Match.captured(1);
		Pose = mPoses.end() - 1;
	}

	// ldraw to internal axes
	Pose->Joints.push_back({ Match.captured(2), Match.captured(3).toInt(), swVector3(Match.captured(4).toFloat(), Match.captured(6).toFloat(), -Match.captured(5).toFloat()), Match.captured(7).toInt() });
}

// use to turn joints between two poses
void swModel::ApplyPoseBlend(const swPose& From, const swPose& To, float Fraction)
{
	for (const swJointAngle& Joint : From.Joints)
	{
		if (Joint.Index < 0 || Joint.Index >= static_cast<int>(mParts.size()))
			continue;

		swPart* Part = mParts[Joint.Index].get();

		if (Joint.Joint < 0 || Joint.Joint >= static_cast<int>(Part->mJoints.size()))
			continue;

		const swPartJoint& PartJoint = Part->mJoints[Joint.Joint];
		swVector3 Rotation = Joint.Rotation;
		const auto ToIt = std::find_if(To.Joints.begin(), To.Joints.end(), [&Joint](const swJointAngle& Other) { return Other.Index == Joint.Index && Other.Joint == Joint.Joint; });

		if (ToIt != To.Joints.end())
			Rotation = swBlendRotation(Joint.Rotation, ToIt->Rotation, Fraction, PartJoint.Type == swJointType::Hinge ? &PartJoint.Axis : nullptr);

		Part->SetJointRotation(Joint.Joint, Rotation, mCurrentStep);
	}

	std::vector<swModel*> UpdatedModels;

	for (swModel* Model : swGetJointsModels(mProject, this))
		Model->UpdatePartData(UpdatedModels);
}

// use to recall a pose of any model
void swModel::RecallPoseAction(swModel* Owner, const swPose& Pose)
{
	BeginJointsEdit();
	Owner->ApplyPoseBlend(Pose, Pose, 0.0f);
	EndJointsEdit(tr("Recall Pose"), true);
}

// use to record edits in every model
void swModel::BeginJointsEdit()
{
	BeginHistorySequence();
	BeginEditHistory();
	mJointsEdits.clear();

	for (swModel* Model : swGetJointsModels(mProject, this))
	{
		if (Model == this)
			continue;

		std::unique_ptr<swModelHistoryEdit> Edit = std::make_unique<swModelHistoryEdit>(swModelHistoryEditMergeKey{ swModelHistoryEditMerge::None, swObjectPropertyId::Count, Model->mCurrentStep, false });
		Edit->SaveStartState(Model);
		mJointsEdits.emplace_back(Model, std::move(Edit));
	}
}

void swModel::EndJointsEdit(const QString& Description, bool Accept)
{
	if (!Accept)
	{
		for (const auto& [Model, Edit] : mJointsEdits)
			Edit->LoadStartState(Model);

		mJointsEdits.clear();
		RevertHistorySequence();
	}
	else
	{
		EndEditHistory();

		for (auto& [Model, Edit] : mJointsEdits)
		{
			Edit->SaveEndState(Model);

			if (Edit->StateChanged())
			{
				Model->CalculateStep(Model->mCurrentStep);
				mHistorySequence.emplace_back(std::make_unique<swModelHistorySubModel>(Model->GetFileName(), std::move(Edit)));
			}
		}

		mJointsEdits.clear();
	}

	std::vector<swModel*> UpdatedModels;

	for (swModel* Model : swGetJointsModels(mProject, this))
		Model->UpdatePartData(UpdatedModels);

	if (Accept)
		EndHistorySequence(Description);

	if (gMainWindow)
		gMainWindow->UpdateSelectedObjects(false);

	UpdateAllViews();
}

// use to find the parts under a ray
bool swModel::JointsRayTest(const swVector3& Start, const swVector3& End, float& Distance, std::vector<std::pair<swModel*, swPart*>>& Chain, std::vector<std::pair<swModel*, swPart*>>& Hit)
{
	bool Found = false;

	// picking sees through other submodels
	const swPart* PickPart = mJointPick && Chain.empty() ? GetJointPart() : nullptr;

	for (const std::unique_ptr<swPart>& Part : mParts)
	{
		if (!(Chain.empty() ? Part->IsSelected() || Part->IsVisible(mCurrentStep) : Part->IsVisibleInSubModel()))
			continue;

		if (PickPart && Part.get() != PickPart)
			continue;

		const swMatrix44 InverseWorld = swMatrix44AffineInverse(Part->mModelWorld);
		const swVector3 LocalStart = swMultiply31(Start, InverseWorld);
		const swVector3 LocalEnd = swMultiply31(End, InverseWorld);

		if (Part->mPartData->IsModel())
		{
			Chain.emplace_back(this, Part.get());
			Found |= Part->mPartData->GetModel()->JointsRayTest(LocalStart, LocalEnd, Distance, Chain, Hit);
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

// use to pick one joint of the joint part
size_t swModel::GetJointIndex() const
{
	const swPart* Part = GetJointPart();

	return Part && Part == mJointIndexPart && !Part->mJoints.empty() ? std::min(mJointIndex, Part->mJoints.size() - 1) : 0;
}

void swModel::SetJointPick(bool Pick)
{
	mJointPick = Pick && GetJointPart() != nullptr;
	mJointHover.clear();
}

// use to outline the part under the pointer
bool swModel::UpdateJointHover(const swVector3& Start, const swVector3& End)
{
	float Distance = FLT_MAX;
	std::vector<std::pair<swModel*, swPart*>> Chain, Hit;
	std::vector<const swPart*> Hover;

	if (mJointPick && JointsRayTest(Start, End, Distance, Chain, Hit) && Hit.size() >= 2 && Hit[0].second == GetJointPart())
		for (const std::pair<swModel*, swPart*>& Link : Hit)
			Hover.push_back(Link.second);

	if (Hover == mJointHover)
		return false;

	mJointHover = Hover;

	return true;
}

// use to find the part holding a pivot
static bool swFindJointLeaf(const swModel* Model, const swMatrix44& Parent, const swVector3& Pivot, std::vector<const swPart*>& Chain)
{
	for (const std::unique_ptr<swPart>& Part : Model->GetParts())
	{
		const swMatrix44 Local = swMultiply(Part->mModelWorld, Parent);
		Chain.push_back(Part.get());

		if (Part->mPartData->IsModel())
		{
			if (swFindJointLeaf(Part->mPartData->GetModel(), Local, Pivot, Chain))
				return true;
		}
		else
		{
			// parts without shadow data pivot at their origin
			bool Found = swLength(Local.GetTranslation() - Pivot) < 0.5f;

			for (const swConnectionCylinder& Cylinder : swShadowLibrary::GetCylinders(Part->mPartData->mFileName))
				Found = Found || swLength(swMultiply(swMatrix44LDrawToStudWorx(Cylinder.Transform), Local).GetTranslation() - Pivot) < 0.5f;

			if (Found)
				return true;
		}

		Chain.pop_back();
	}

	return false;
}

// use to drop a chain left by an edit
static bool swIsChainLinked(const std::vector<const swPart*>& Chain)
{
	for (size_t Index = 1; Index < Chain.size(); Index++)
	{
		const auto& Data = Chain[Index - 1]->mPartData;
		const auto IsLink = [&](const std::unique_ptr<swPart>& Part) { return Part.get() == Chain[Index]; };

		if (!Data->IsModel() || std::none_of(Data->GetModel()->GetParts().begin(), Data->GetModel()->GetParts().end(), IsLink))
			return false;
	}

	return true;
}

// use to add the last part of a chain
static void swAddChainMeshes(swScene* Scene, const std::vector<const swPart*>& Chain, const swMatrix44& Transform, swRenderMeshState State)
{
	swMatrix44 World = swMatrix44Identity();
	int ColorIndex = gDefaultColor;

	// color 16 takes the color above
	for (size_t Index = Chain.size(); Index-- > 0;)
	{
		World = swMultiply(World, Chain[Index]->mModelWorld);

		if (ColorIndex == gDefaultColor)
			ColorIndex = Chain[Index]->GetColorIndex();
	}

	Chain.back()->mPartData->AddRenderMeshes(Scene, swMultiply(World, Transform), ColorIndex, State, false);
}

// use to outline the submodel under the pointer
void swModel::AddHoverOutline(swScene* Scene, const swMatrix44& Transform, std::vector<const swPart*> Chain) const
{
	// a joint goes on the holder of the hovered part
	if (mJointPick)
	{
		Chain = mJointHover;

		if (!Chain.empty())
			Chain.pop_back();
	}

	const auto IsPart = [&](const std::unique_ptr<swPart>& Part) { return !Chain.empty() && Part.get() == Chain[0]; };

	if (Chain.empty() || !Chain.back()->mPartData->IsModel() || std::none_of(mParts.begin(), mParts.end(), IsPart) || !swIsChainLinked(Chain))
		return;

	Scene->SetAddHover(true);
	swAddChainMeshes(Scene, Chain, Transform, swRenderMeshState::Default);
	Scene->SetAddHover(false);
}

// use to outline the hovered or joint part
void swModel::AddJointHighlight(swScene* Scene, const swMatrix44& Transform) const
{
	const swPart* Target = GetJointPart();
	std::vector<const swPart*> Chain;

	if (!Target)
		return;

	if (mJointPick)
	{
		Chain = mJointHover;

		if (Chain.empty() || Chain[0] != Target || !swIsChainLinked(Chain))
			return;
	}
	else
	{
		if (!Target->HasJoints() || !Target->mPartData->IsModel())
			return;

		const size_t JointIndex = GetJointIndex();
		const swVector3 Pivot = swMultiply31(Target->mJoints[JointIndex].Pivot, swMatrix44AffineInverse(Target->GetJointsMatrix(JointIndex)));

		Chain.push_back(Target);

		if (!swFindJointLeaf(Target->mPartData->GetModel(), swMatrix44Identity(), Pivot, Chain))
			return;
	}

	swAddChainMeshes(Scene, Chain, Transform, swRenderMeshState::Highlighted);
}

// submodels down to the one holding the part under the ray
std::vector<swPart*> swModel::GetSubmodelPath(const swVector3& Start, const swVector3& End)
{
	float Distance = FLT_MAX;
	std::vector<std::pair<swModel*, swPart*>> Chain, Hit;
	std::vector<swPart*> Path;

	if (JointsRayTest(Start, End, Distance, Chain, Hit))
		for (size_t Index = 0; Index + 1 < Hit.size(); Index++)
			Path.push_back(Hit[Index].second);

	return Path;
}

bool swModel::JointsToolClicked(const swVector3& Start, const swVector3& End, const swVector3& Right, const swVector3& Up)
{
	float Distance = FLT_MAX;
	std::vector<std::pair<swModel*, swPart*>> Chain, Hit;

	mJointsJoint = nullptr;

	if (!JointsRayTest(Start, End, Distance, Chain, Hit) || Hit.size() < 2)
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
		mJointHover.clear();

		if (!Target || Hit[0].second != Target)
			return false;

		// the joint goes on the submodel holding the clicked part
		const size_t Owner = Hit.size() - 2;
		swPart* Holder = Hit[Owner].second;
		const swMatrix44 InverseWorld = swMatrix44AffineInverse(GetWorld(Owner + 1, 0));
		const swVector3 LocalStart = swMultiply31(Start, InverseWorld);
		const swVector3 Direction = swMultiply31(End, InverseWorld) - LocalStart;
		const swMatrix44 Local = GetWorld(Hit.size(), Owner + 1);
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

		BeginJointsEdit();
		Hit[Owner].first->SetJointIndex(Holder, Holder->SetJoint(Joint, mCurrentStep));
		EndJointsEdit(tr("Set Joint"), true);

		// the view opens the new joint in joint mode
		mJointSetPath.clear();

		for (size_t Index = 0; Index <= Owner; Index++)
			mJointSetPath.push_back(Hit[Index].second);

		if (gMainWindow)
			gMainWindow->UpdateSelectedObjects(false);

		return false;
	}

	// innermost submodel with a joint
	for (size_t Index = Hit.size() - 1; Index-- > 0;)
	{
		swPart* Joint = Hit[Index].second;

		if (!Joint->HasJoints())
			continue;

		// the joint nearest the click turns
		const swMatrix44 Parent = GetWorld(Index, 0);
		const swVector3 Direction = End - Start;
		size_t JointIndex = 0;
		float BestDistance = FLT_MAX;

		for (size_t Candidate = 0; Candidate < Joint->mJoints.size(); Candidate++)
		{
			const swVector3 Pivot = swMultiply31(swMultiply31(Joint->mJoints[Candidate].Pivot, Joint->GetJointFrame(Candidate)), Parent);
			const float Away = swLength(swCross(Pivot - Start, Direction)) / swLength(Direction);

			if (Away < BestDistance)
			{
				BestDistance = Away;
				JointIndex = Candidate;
			}
		}

		mJointsJoint = Joint;
		mJointsJointIndex = JointIndex;
		mJointsStartRotation = Joint->mJoints[JointIndex].Rotation;
		mJointsWorldRotation = swMatrix33(swMultiply(Joint->GetJointFrame(JointIndex), Parent));

		SetJointIndex(Joint, JointIndex);
		mJointsRight = Right;
		mJointsUp = Up;

		BeginJointsEdit();

		return true;
	}

	return false;
}

void swModel::UpdateJointsTool(float DeltaX, float DeltaY)
{
	if (!mJointsJoint)
		return;

	// angles follow the snap
	const auto Snap = [this](float Angle) { return SnapRotation(swVector3(Angle, Angle, Angle))[0]; };
	const swPartJoint& Joint = mJointsJoint->mJoints[mJointsJointIndex];
	swVector3 Rotation;

	if (Joint.Type == swJointType::Hinge)
	{
		const swVector3 Axis = swNormalize(Joint.Axis);

		Rotation = Axis * (swDot(mJointsStartRotation, Axis) + Snap(DeltaX * 0.5f));
	}
	else
	{
		const swMatrix33 Turn = swMultiply(swMatrix33FromAxisAngle(mJointsUp, Snap(DeltaX * 0.5f) * SW_DEGREES_TO_RADIANS), swMatrix33FromAxisAngle(mJointsRight, Snap(DeltaY * 0.5f) * SW_DEGREES_TO_RADIANS));

		// world turn in child terms
		Rotation = swVectorFromRotation(swMultiply(mJointsWorldRotation, swMultiply(Turn, swMultiply(swMatrix33AffineInverse(mJointsWorldRotation), swRotationFromVector(mJointsStartRotation)))));
	}

	mJointsJoint->SetJointRotation(mJointsJointIndex, Rotation, mCurrentStep);
	UpdateAllViews();
}

void swModel::EndJointsTool(bool Accept)
{
	if (!mJointsJoint)
		return;

	mJointsJoint = nullptr;
	EndJointsEdit(tr("Turn Joint"), Accept);
}

void swModel::SetJointAxisAction(swPart* Part, int Axis)
{
	BeginHistorySequence();
	BeginEditHistory();

	const size_t JointIndex = Part == GetJointPart() ? GetJointIndex() : 0;

	// rest angles before the new axis
	Part->SetJointRotation(JointIndex, swVector3(0.0f, 0.0f, 0.0f), mCurrentStep);

	if (JointIndex < Part->mJoints.size())
		Part->mJoints[JointIndex].Axis = swVector3(Axis == 0 ? 1.0f : 0.0f, Axis == 1 ? 1.0f : 0.0f, Axis == 2 ? 1.0f : 0.0f);

	EndEditHistory();
	EndHistorySequence(tr("Joint Axis"));
}

void swModel::AddJointAction(swPart* Part, const swPartJoint& Joint)
{
	if (!Part || Joint.Type == swJointType::None)
		return;

	BeginJointsEdit();
	SetJointIndex(Part, Part->SetJoint(Joint, mCurrentStep));
	EndJointsEdit(tr("Set Joint"), true);
}

void swModel::ClearJointAction(swPart* Part, size_t Index)
{
	BeginHistorySequence();
	BeginEditHistory();

	Part->RemoveJoint(Index, mCurrentStep);

	EndEditHistory();
	EndHistorySequence(tr("Delete Joint"));
}

void swModel::ClearJointsAction(swPart* Part)
{
	BeginHistorySequence();
	BeginEditHistory();

	// last first keeps the indices
	for (size_t Index = Part->mJoints.size(); Index > 0; Index--)
		Part->RemoveJoint(Index - 1, mCurrentStep);

	EndEditHistory();
	EndHistorySequence(tr("Delete All Joints"));
}

// rotate commands turn this joint
void swModel::SetJointRotate(bool Rotate)
{
	swPart* Part = Rotate ? GetJointPart() : nullptr;

	mJointRotate = Part && Part->HasJoints() ? Part : nullptr;
}

// a new selection goes back to the pivot point
void swModel::UpdateJointRotate()
{
	const swPart* Part = GetJointPart();

	if (mJointRotate && mJointRotate != Part)
		mJointRotate = nullptr;

	if (mJointFocus && mJointFocus != Part)
		mJointFocus = nullptr;
}

// joints picked in the list
void swModel::SetJointFocus()
{
	swPart* Part = GetJointPart();

	mJointFocus = Part && Part->HasJoints() ? Part : nullptr;
}

// back to plain selection
void swModel::EndJointFocus()
{
	mJointFocus = nullptr;
	mJointRotate = nullptr;
	ClearSelectionAction();
}

swPart* swModel::GetRotateJoint() const
{
	swPart* Part = GetJointPart();

	if (!Part || !Part->HasJoints())
		return nullptr;

	return Part == mJointRotate || swGetPreferences().mJointRotation ? Part : nullptr;
}

// use to turn a joint by a model space rotation
bool swModel::RotateJoint(swPart* Part, const swMatrix33& WorldRotation)
{
	const size_t JointIndex = Part == GetJointPart() ? GetJointIndex() : 0;
	const swPartJoint& Joint = Part->mJoints[JointIndex];
	const swMatrix33 PartRotation(Part->GetJointFrame(JointIndex));
	const swMatrix33 Turn = swMultiply(PartRotation, swMultiply(WorldRotation, swMatrix33AffineInverse(PartRotation)));
	swVector3 Rotation;

	if (Joint.Type == swJointType::Hinge)
	{
		const swVector3 Axis = swNormalize(Joint.Axis);
		const swVector3 TurnVector = swVectorFromRotation(Turn);
		const float Angle = swLength(TurnVector);
		const float Along = swDot(TurnVector, Axis);

		// a turn across the hinge does nothing
		if (Angle < 0.001f || fabsf(Along) < Angle * 0.1f)
			return false;

		Rotation = Axis * (swDot(Joint.Rotation, Axis) + (Along > 0.0f ? Angle : -Angle));
	}
	else
		Rotation = swVectorFromRotation(swMultiply(Turn, swRotationFromVector(Joint.Rotation)));

	Part->SetJointRotation(JointIndex, Rotation, mCurrentStep);

	return true;
}
