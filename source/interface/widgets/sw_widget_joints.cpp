#include "sw_global.h"
#include "sw_widget_joints.h"
#include "sw_window_main.h"
#include "sw_project.h"
#include "sw_object_part.h"
#include "sw_data_part.h"

swJointsWidget::swJointsWidget(QWidget* Parent)
	: QWidget(Parent)
{
	QVBoxLayout* Layout = new QVBoxLayout(this);

	// joints of every model
	mTree = new QTreeWidget(this);
	mTree->setColumnCount(3);
	mTree->setHeaderLabels({ tr("Joint"), tr("Type"), tr("Angle") });
	mTree->setRootIsDecorated(true);
	mTree->setToolTip(tr("Joints of the model and its submodels"));
	mTree->setWhatsThis(tr("<p><b>Joints</b></p><p>Every model with joints, under it each jointed submodel by line index, and under that each of its joints with type and angle. Click a joint to select its submodel and make it the joint the panel and the rotate gizmo work on. Click a model to show its poses.</p>"));
	connect(mTree, &QTreeWidget::itemClicked, this, &swJointsWidget::TreeItemClicked);
	Layout->addWidget(mTree, 2);

	mPoseLabel = new QLabel(this);
	Layout->addWidget(mPoseLabel);

	mList = new QListWidget(this);
	mList->setToolTip(tr("Saved poses of this model, played in this order by the Joints motion"));
	mList->setWhatsThis(tr("<p><b>Poses</b></p><p>The poses saved in the model picked above, each holding the angles of that model's joints only. The Joints motion of the image dialog plays the poses of the main model in this order. Pick one to recall, rename or delete it.</p>"));
	Layout->addWidget(mList, 1);

	QHBoxLayout* Buttons = new QHBoxLayout();
	struct swJointsButton
	{
		QString Text;
		QString ToolTip;
		QString WhatsThis;
		void (swJointsWidget::*Slot)();
	};

	const swJointsButton Entries[] =
	{
		{ tr("Save"), tr("Save the joint angles of this model as a pose"), tr("<p><b>Save</b></p><p>Store the angles of the joints in this model under a name. Saving a name that already exists replaces that pose.</p>"), &swJointsWidget::SaveClicked },
		{ tr("Recall"), tr("Put the joints of this model back in the picked pose"), tr("<p><b>Recall</b></p><p>Turn the joints of this model back to the angles of the picked pose, in one undo step.</p>"), &swJointsWidget::RecallClicked },
		{ tr("Rename"), tr("Rename the picked pose"), tr("<p><b>Rename</b></p><p>Give the picked pose a new name.</p>"), &swJointsWidget::RenameClicked },
		{ tr("Delete"), tr("Delete the picked pose"), tr("<p><b>Delete</b></p><p>Remove the picked pose from the model. The joints keep their angles.</p>"), &swJointsWidget::DeleteClicked }
	};

	for (const swJointsButton& Entry : Entries)
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
	connect(mAxisCombo, QOverload<int>::of(&QComboBox::activated), this, &swJointsWidget::AxisChanged);
	JointRow->addWidget(mAxisCombo);

	mClearButton = new QToolButton(this);
	mClearButton->setText(tr("Clear Joint"));
	mClearButton->setToolTip(tr("Remove the picked joint of the selected submodel"));
	mClearButton->setWhatsThis(tr("<p><b>Clear Joint</b></p><p>Remove the picked joint of the selected submodel and put that joint back at rest. This can be undone.</p>"));
	connect(mClearButton, &QToolButton::clicked, this, &swJointsWidget::ClearClicked);
	JointRow->addWidget(mClearButton);
	Layout->addLayout(JointRow);

	UpdateJoint();
}

// use to find the model of the poses
swModel* swJointsWidget::GetPoseModel() const
{
	swModel* Model = gMainWindow ? gMainWindow->GetActiveModel() : nullptr;
	const QTreeWidgetItem* Item = mTree->currentItem();
	Project* CurrentProject = Model ? Model->GetProject() : nullptr;

	if (Item && CurrentProject)
		if (swModel* Picked = CurrentProject->GetModel(Item->data(0, Qt::UserRole).toString()))
			return Picked;

	return Model;
}

void swJointsWidget::UpdateTree()
{
	swModel* ActiveModel = gMainWindow ? gMainWindow->GetActiveModel() : nullptr;
	Project* CurrentProject = ActiveModel ? ActiveModel->GetProject() : nullptr;
	const swPart* JointPart = ActiveModel ? ActiveModel->GetJointPart() : nullptr;
	const QTreeWidgetItem* Previous = mTree->currentItem();
	const QString PreviousModel = Previous ? Previous->data(0, Qt::UserRole).toString() : QString();
	const int PreviousIndex = Previous ? Previous->data(0, Qt::UserRole + 1).toInt() : -1;
	const int PreviousJoint = Previous ? Previous->data(0, Qt::UserRole + 2).toInt() : -1;
	const int JointIndex = static_cast<int>(ActiveModel ? ActiveModel->GetJointIndex() : 0);
	QTreeWidgetItem* Current = nullptr;

	mTree->clear();

	if (CurrentProject)
	{
		for (const std::unique_ptr<swModel>& Model : CurrentProject->GetModels())
		{
			QTreeWidgetItem* ModelItem = nullptr;
			const std::vector<std::unique_ptr<swPart>>& Parts = Model->GetParts();

			for (size_t Index = 0; Index < Parts.size(); Index++)
			{
				const swPart* Part = Parts[Index].get();

				if (!Part->HasJoints() || !Part->mPartData->IsModel())
					continue;

				if (!ModelItem)
				{
					ModelItem = new QTreeWidgetItem(mTree, { Model->GetFileName() });
					ModelItem->setData(0, Qt::UserRole, Model->GetFileName());
					ModelItem->setData(0, Qt::UserRole + 1, -1);
					ModelItem->setData(0, Qt::UserRole + 2, -1);
					ModelItem->setFirstColumnSpanned(true);
					ModelItem->setExpanded(true);

					if (Model->GetFileName() == PreviousModel && PreviousIndex < 0)
						Current = ModelItem;
				}

				QTreeWidgetItem* PartItem = new QTreeWidgetItem(ModelItem, { QString("%1 %2").arg(Part->mPartData->GetModel()->GetFileName()).arg(Index + 1, 3, 10, QLatin1Char('0')) });
				PartItem->setData(0, Qt::UserRole, Model->GetFileName());
				PartItem->setData(0, Qt::UserRole + 1, static_cast<int>(Index));
				PartItem->setData(0, Qt::UserRole + 2, -1);
				PartItem->setFirstColumnSpanned(true);
				PartItem->setExpanded(true);

				if (!JointPart && Model->GetFileName() == PreviousModel && static_cast<int>(Index) == PreviousIndex && PreviousJoint < 0)
					Current = PartItem;

				for (size_t Joint = 0; Joint < Part->mJoints.size(); Joint++)
				{
					const swPartJoint& PartJoint = Part->mJoints[Joint];
					const QString Type = PartJoint.Type == swJointType::Hinge ? tr("Hinge") : tr("Ball");
					QTreeWidgetItem* JointItem = new QTreeWidgetItem(PartItem, QStringList({ tr("Joint %1").arg(Joint + 1, 3, 10, QLatin1Char('0')), Type, QString("%1°").arg(swLength(PartJoint.Rotation), 0, 'f', 1) }));
					JointItem->setData(0, Qt::UserRole, Model->GetFileName());
					JointItem->setData(0, Qt::UserRole + 1, static_cast<int>(Index));
					JointItem->setData(0, Qt::UserRole + 2, static_cast<int>(Joint));

					// the selected joint wins over the previous pick
					if ((Part == JointPart && static_cast<int>(Joint) == JointIndex) || (!JointPart && Model->GetFileName() == PreviousModel && static_cast<int>(Index) == PreviousIndex && static_cast<int>(Joint) == PreviousJoint))
						Current = JointItem;
				}
			}
		}
	}

	if (!mTree->topLevelItemCount())
	{
		QTreeWidgetItem* Empty = new QTreeWidgetItem(mTree, { tr("No joints, use Part > Joint > Set") });
		Empty->setFlags(Qt::NoItemFlags);
		Empty->setFirstColumnSpanned(true);
	}

	mTree->resizeColumnToContents(0);

	if (Current)
		mTree->setCurrentItem(Current);
}

void swJointsWidget::TreeItemClicked(QTreeWidgetItem* Item)
{
	const QString ModelName = Item->data(0, Qt::UserRole).toString();
	const int Index = Item->data(0, Qt::UserRole + 1).toInt();
	const int Joint = std::max(Item->data(0, Qt::UserRole + 2).toInt(), 0);

	UpdateList();

	if (Index < 0)
		return;

	// select later, the tree rebuilds on selection
	QTimer::singleShot(0, this, [this, ModelName, Index, Joint]()
	{
		swModel* ActiveModel = gMainWindow ? gMainWindow->GetActiveModel() : nullptr;
		Project* CurrentProject = ActiveModel ? ActiveModel->GetProject() : nullptr;
		swModel* Model = CurrentProject ? CurrentProject->GetModel(ModelName) : nullptr;

		if (!Model || Index >= static_cast<int>(Model->GetParts().size()))
			return;

		if (Model != ActiveModel)
			CurrentProject->SetActiveModel(Model, true);

		swPart* Part = Model->GetParts()[Index].get();
		Model->SetJointIndex(Part, static_cast<size_t>(Joint));
		Model->SetSelectionAndFocusAction({ Part }, Part, SW_PART_SECTION_POSITION, swSelectionMode::Single);
		UpdateJoint();
	});
}

void swJointsWidget::UpdateList()
{
	const int Row = mList->currentRow();
	swModel* Model = GetPoseModel();

	mList->clear();
	mPoseLabel->setText(Model ? tr("Poses of %1").arg(Model->GetFileName()) : tr("Poses"));

	if (Model)
		for (const swPose& Pose : Model->GetPoses())
			mList->addItem(Pose.Name);

	if (Row < mList->count())
		mList->setCurrentRow(Row);
}

// use to refresh the tree, poses and joint
void swJointsWidget::UpdateJoint()
{
	UpdateTree();
	UpdateList();

	swModel* Model = gMainWindow ? gMainWindow->GetActiveModel() : nullptr;
	swPart* Part = Model ? Model->GetJointPart() : nullptr;
	const swPartJoint* Joint = Part && Part->HasJoints() ? &Part->mJoints[Model->GetJointIndex()] : nullptr;
	const bool HasJoint = Joint != nullptr;

	mJointLabel->setText(!Part ? tr("Select one submodel for a joint") : !HasJoint ? tr("No joint") : tr("Joint %1, %2, angle %3").arg(Model->GetJointIndex() + 1, 3, 10, QLatin1Char('0')).arg(Joint->Type == swJointType::Hinge ? tr("Hinge") : tr("Ball")).arg(swLength(Joint->Rotation), 0, 'f', 1));
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

void swJointsWidget::SaveClicked()
{
	swModel* Owner = GetPoseModel();

	if (!Owner)
		return;

	bool Ok;
	const QString Name = QInputDialog::getText(this, tr("Save Pose"), tr("Name"), QLineEdit::Normal, tr("Pose %1").arg(Owner->GetPoses().size() + 1), &Ok).trimmed();

	if (!Ok || Name.isEmpty())
		return;

	std::vector<swPose> Poses = Owner->GetPoses();
	const swPose Pose = Owner->GetCurrentPose(Name);
	const auto Existing = std::find_if(Poses.begin(), Poses.end(), [&Name](const swPose& Other) { return Other.Name == Name; });

	if (Existing != Poses.end())
		*Existing = Pose;
	else
		Poses.push_back(Pose);

	Owner->SetPoses(Poses);
	Owner->GetProject()->MarkAsModified();
	UpdateList();
}

void swJointsWidget::RecallClicked()
{
	swModel* Owner = GetPoseModel();
	const int Row = mList->currentRow();

	if (Owner && Row >= 0 && Row < static_cast<int>(Owner->GetPoses().size()))
		gMainWindow->GetActiveModel()->RecallPoseAction(Owner, Owner->GetPoses()[Row]);
}

void swJointsWidget::RenameClicked()
{
	swModel* Owner = GetPoseModel();
	const int Row = mList->currentRow();

	if (!Owner || Row < 0)
		return;

	bool Ok;
	const QString Name = QInputDialog::getText(this, tr("Rename Pose"), tr("Name"), QLineEdit::Normal, mList->currentItem()->text(), &Ok).trimmed();

	if (!Ok || Name.isEmpty())
		return;

	std::vector<swPose> Poses = Owner->GetPoses();
	Poses[Row].Name = Name;
	Owner->SetPoses(Poses);
	Owner->GetProject()->MarkAsModified();
	UpdateList();
}

void swJointsWidget::DeleteClicked()
{
	swModel* Owner = GetPoseModel();
	const int Row = mList->currentRow();

	if (!Owner || Row < 0)
		return;

	std::vector<swPose> Poses = Owner->GetPoses();
	Poses.erase(Poses.begin() + Row);
	Owner->SetPoses(Poses);
	Owner->GetProject()->MarkAsModified();
	UpdateList();
}

void swJointsWidget::AxisChanged(int Index)
{
	swModel* Model = gMainWindow->GetActiveModel();
	swPart* Part = Model ? Model->GetJointPart() : nullptr;

	if (Part)
		Model->SetJointAxisAction(Part, Index);

	UpdateJoint();
}

void swJointsWidget::ClearClicked()
{
	swModel* Model = gMainWindow->GetActiveModel();
	swPart* Part = Model ? Model->GetJointPart() : nullptr;

	if (Part)
		Model->ClearJointAction(Part);

	UpdateJoint();
}
