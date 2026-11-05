#include "sw_global.h"
#include "sw_widget_poses.h"
#include "sw_window_main.h"
#include "sw_project.h"
#include "sw_object_part.h"

swPosesWidget::swPosesWidget(QWidget* Parent)
	: QWidget(Parent)
{
	QVBoxLayout* Layout = new QVBoxLayout(this);

	mList = new QListWidget(this);
	mList->setToolTip(tr("Saved poses, played in this order by the Poses motion"));
	mList->setWhatsThis(tr("<p><b>Poses</b></p><p>The poses saved in the model, in the order the Poses motion of the image dialog plays them. Pick one to recall, rename or delete it.</p>"));
	Layout->addWidget(mList);

	QHBoxLayout* Buttons = new QHBoxLayout();
	struct swPoseButton
	{
		QString Text;
		QString ToolTip;
		QString WhatsThis;
		void (swPosesWidget::*Slot)();
	};

	const swPoseButton Entries[] =
	{
		{ tr("Save"), tr("Save the joint angles as a pose"), tr("<p><b>Save</b></p><p>Store the angles of every joint in the model under a name. Saving a name that already exists replaces that pose.</p>"), &swPosesWidget::SaveClicked },
		{ tr("Recall"), tr("Put the joints back in the picked pose"), tr("<p><b>Recall</b></p><p>Turn every joint back to the angles of the picked pose, in one undo step.</p>"), &swPosesWidget::RecallClicked },
		{ tr("Rename"), tr("Rename the picked pose"), tr("<p><b>Rename</b></p><p>Give the picked pose a new name.</p>"), &swPosesWidget::RenameClicked },
		{ tr("Delete"), tr("Delete the picked pose"), tr("<p><b>Delete</b></p><p>Remove the picked pose from the model. The joints keep their angles.</p>"), &swPosesWidget::DeleteClicked }
	};

	for (const swPoseButton& Entry : Entries)
	{
		QToolButton* Button = new QToolButton(this);
		Button->setText(Entry.Text);
		Button->setToolTip(Entry.ToolTip);
		Button->setWhatsThis(Entry.WhatsThis);
		connect(Button, &QToolButton::clicked, this, Entry.Slot);
		Buttons->addWidget(Button);
	}

	Layout->addLayout(Buttons);

	// joint of the selected submodel
	mJointLabel = new QLabel(this);
	Layout->addWidget(mJointLabel);

	QHBoxLayout* JointRow = new QHBoxLayout();
	mAxisCombo = new QComboBox(this);
	mAxisCombo->addItems({ tr("Axis X"), tr("Axis Y"), tr("Axis Z") });
	mAxisCombo->setToolTip(tr("Hinge axis, for parts without shadow data"));
	mAxisCombo->setWhatsThis(tr("<p><b>Axis</b></p><p>The axis a hinge turns about, in the coordinates of the submodel. Use it for parts without shadow data, which get a hinge on their origin.</p>"));
	connect(mAxisCombo, QOverload<int>::of(&QComboBox::activated), this, &swPosesWidget::AxisChanged);
	JointRow->addWidget(mAxisCombo);

	mClearButton = new QToolButton(this);
	mClearButton->setText(tr("Clear Joint"));
	mClearButton->setToolTip(tr("Remove the joint of the selected submodel"));
	mClearButton->setWhatsThis(tr("<p><b>Clear Joint</b></p><p>Remove the joint of the selected submodel and put it back in its placed position. This can be undone.</p>"));
	connect(mClearButton, &QToolButton::clicked, this, &swPosesWidget::ClearClicked);
	JointRow->addWidget(mClearButton);
	Layout->addLayout(JointRow);
	Layout->addStretch(1);

	UpdateJoint();
}

swModel* swPosesWidget::GetMainModel() const
{
	swModel* Model = gMainWindow ? gMainWindow->GetActiveModel() : nullptr;

	return Model && Model->GetProject() ? Model->GetProject()->GetMainModel() : nullptr;
}

void swPosesWidget::UpdateList()
{
	mList->clear();

	if (swModel* Main = GetMainModel())
		for (const swPose& Pose : Main->GetPoses())
			mList->addItem(Pose.Name);
}

// use to refresh the list and joint
void swPosesWidget::UpdateJoint()
{
	UpdateList();

	swModel* Model = gMainWindow ? gMainWindow->GetActiveModel() : nullptr;
	swPart* Part = Model ? Model->GetJointPart() : nullptr;
	const swPartJoint* Joint = Part ? &Part->mJoint : nullptr;
	const bool HasJoint = Joint && Joint->Type != swJointType::None;

	mJointLabel->setText(!Part ? tr("Select one submodel for a joint") : !HasJoint ? tr("No joint") : tr("%1, angle %2").arg(Joint->Type == swJointType::Hinge ? tr("Hinge") : tr("Ball")).arg(swLength(Joint->Rotation), 0, 'f', 1));
	mAxisCombo->setEnabled(HasJoint && Joint->Type == swJointType::Hinge);
	mClearButton->setEnabled(HasJoint);

	if (HasJoint)
	{
		int Axis = 0;

		for (int Index = 1; Index < 3; Index++)
			if (fabsf(Joint->Axis[Index]) > fabsf(Joint->Axis[Axis]))
				Axis = Index;

		mAxisCombo->setCurrentIndex(Axis);
	}
}

void swPosesWidget::SaveClicked()
{
	swModel* Main = GetMainModel();

	if (!Main)
		return;

	bool Ok;
	const QString Name = QInputDialog::getText(this, tr("Save Pose"), tr("Name"), QLineEdit::Normal, tr("Pose %1").arg(Main->GetPoses().size() + 1), &Ok).trimmed();

	if (!Ok || Name.isEmpty())
		return;

	std::vector<swPose> Poses = Main->GetPoses();
	const swPose Pose = Main->GetCurrentPose(Name);
	const auto Existing = std::find_if(Poses.begin(), Poses.end(), [&Name](const swPose& Other) { return Other.Name == Name; });

	if (Existing != Poses.end())
		*Existing = Pose;
	else
		Poses.push_back(Pose);

	Main->SetPoses(Poses);
	Main->GetProject()->MarkAsModified();
	UpdateList();
}

void swPosesWidget::RecallClicked()
{
	swModel* Main = GetMainModel();
	const int Row = mList->currentRow();

	if (Main && Row >= 0 && Row < static_cast<int>(Main->GetPoses().size()))
		Main->RecallPoseAction(Main->GetPoses()[Row]);
}

void swPosesWidget::RenameClicked()
{
	swModel* Main = GetMainModel();
	const int Row = mList->currentRow();

	if (!Main || Row < 0)
		return;

	bool Ok;
	const QString Name = QInputDialog::getText(this, tr("Rename Pose"), tr("Name"), QLineEdit::Normal, mList->currentItem()->text(), &Ok).trimmed();

	if (!Ok || Name.isEmpty())
		return;

	std::vector<swPose> Poses = Main->GetPoses();
	Poses[Row].Name = Name;
	Main->SetPoses(Poses);
	Main->GetProject()->MarkAsModified();
	UpdateList();
}

void swPosesWidget::DeleteClicked()
{
	swModel* Main = GetMainModel();
	const int Row = mList->currentRow();

	if (!Main || Row < 0)
		return;

	std::vector<swPose> Poses = Main->GetPoses();
	Poses.erase(Poses.begin() + Row);
	Main->SetPoses(Poses);
	Main->GetProject()->MarkAsModified();
	UpdateList();
}

void swPosesWidget::AxisChanged(int Index)
{
	swModel* Model = gMainWindow->GetActiveModel();
	swPart* Part = Model ? Model->GetJointPart() : nullptr;

	if (Part)
		Model->SetJointAxisAction(Part, Index);

	UpdateJoint();
}

void swPosesWidget::ClearClicked()
{
	swModel* Model = gMainWindow->GetActiveModel();
	swPart* Part = Model ? Model->GetJointPart() : nullptr;

	if (Part)
		Model->ClearJointAction(Part);

	UpdateJoint();
}
