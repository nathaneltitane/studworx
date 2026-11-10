#include "sw_global.h"
#include "sw_widget_joints.h"
#include "sw_window_main.h"
#include "sw_project.h"
#include "sw_object_part.h"
#include "sw_data_part.h"
#include "sw_view.h"

swJointsWidget::swJointsWidget(QWidget* Parent)
	: QWidget(Parent)
{
	// panel overview

	setWhatsThis(tr("<p><b>Joints</b></p><p>A tree lists every model with joints, under it each jointed submodel by line index, and under that each of its joints with type and angle. Click a joint to select its submodel and make it the joint the panel and the rotate gizmo work on, right click it to move, rotate or delete it, or right click a submodel to edit or delete all its joints. Click a model to show its poses in the Poses panel.</p><p>Below the tree the panel shows the joint picked on the selected submodel with its axis. A joint inside a submodel opens it in place, and a bar over the view points to the picked joint.</p>"));

	QVBoxLayout* Layout = new QVBoxLayout(this);

	// joints of every model
	mTree = new QTreeWidget(this);
	mTree->setColumnCount(3);
	mTree->setHeaderLabels({ tr("Joint"), tr("Type"), tr("Angle") });
	mTree->setRootIsDecorated(true);
	mTree->setToolTip(tr("Joints of the model and its submodels"));
	connect(mTree, &QTreeWidget::itemClicked, this, &swJointsWidget::TreeItemClicked);
	connect(mTree, &QTreeWidget::currentItemChanged, this, &swJointsWidget::UpdateClearButton);
	mTree->installEventFilter(this);
	mTree->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(mTree, &QTreeWidget::customContextMenuRequested, this, &swJointsWidget::TreeMenuRequested);
	Layout->addWidget(mTree, 2);

	// joint of the selected submodel
	mJointLabel = new QLabel(this);
	Layout->addWidget(mJointLabel);

	QHBoxLayout* JointRow = new QHBoxLayout();
	mAxisCombo = new QComboBox(this);
	mAxisCombo->addItems({ tr("X Axis"), tr("Y Axis"), tr("Z Axis") });
	mAxisCombo->setToolTip(tr("Hinge axis, for parts without shadow data"));
	mAxisCombo->setWhatsThis(tr("<p><b>Axis</b></p><p>The axis a hinge turns about, in the coordinates of the submodel. Use it for parts without shadow data, which get a hinge on their origin.</p>"));
	connect(mAxisCombo, QOverload<int>::of(&QComboBox::activated), this, &swJointsWidget::AxisChanged);
	JointRow->addWidget(mAxisCombo);

	mClearButton = new QToolButton(this);
	mClearButton->setText(tr("Delete Joint"));
	mClearButton->setToolTip(tr("Delete the joint picked in the list (Delete)"));
	mClearButton->setWhatsThis(tr("<p><b>Delete Joint</b></p><p>Delete the joint picked in the joints list and put that joint back at rest. The Delete key does the same in the list. This can be undone.</p>"));
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
					ModelItem->setExpanded(true);

					if (Model->GetFileName() == PreviousModel && PreviousIndex < 0)
						Current = ModelItem;
				}

				QTreeWidgetItem* PartItem = new QTreeWidgetItem(ModelItem, { QString("%1 %2").arg(Part->mPartData->GetModel()->GetFileName()).arg(Index + 1, 3, 10, QLatin1Char('0')) });
				PartItem->setData(0, Qt::UserRole, Model->GetFileName());
				PartItem->setData(0, Qt::UserRole + 1, static_cast<int>(Index));
				PartItem->setData(0, Qt::UserRole + 2, -1);
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
		QTreeWidgetItem* Empty = new QTreeWidgetItem(mTree, { tr("None: Part > Joint > Set Joint") });
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

	emit PoseModelChanged();

	if (Index < 0)
		return;

	// select later, the tree rebuilds on selection
	QTimer::singleShot(0, this, [this, ModelName, Index, Joint]()
	{
		SelectJoint(ModelName, Index, Joint);
	});
}

void swJointsWidget::SelectJoint(const QString& ModelName, int Index, int Joint)
{
	swModel* ActiveModel = gMainWindow ? gMainWindow->GetActiveModel() : nullptr;
	Project* CurrentProject = ActiveModel ? ActiveModel->GetProject() : nullptr;
	swModel* Model = CurrentProject ? CurrentProject->GetModel(ModelName) : nullptr;

	if (!Model || Index >= static_cast<int>(Model->GetParts().size()))
		return;

	// open it in place, tab switch when unreachable
	swView* View = gMainWindow->GetActiveView();

	if (!View || !View->SetJointModelActive(Model))
		CurrentProject->SetActiveModel(Model, true);

	swPart* Part = Model->GetParts()[Index].get();
	Model->SetJointIndex(Part, static_cast<size_t>(Joint));
	Model->SetSelectionAndFocusAction({ Part }, Part, SW_PART_SECTION_POSITION, swSelectionMode::Single);
	Model->SetJointFocus();
	UpdateJoint();

	// same selection, another joint
	swView::UpdateAllViews();

	// keys stay on the list
	mTree->setFocus();
}

void swJointsWidget::TreeMenuRequested(const QPoint& Position)
{
	const QTreeWidgetItem* Item = mTree->itemAt(Position);
	const int Index = Item ? Item->data(0, Qt::UserRole + 1).toInt() : -1;
	const int Joint = Item ? Item->data(0, Qt::UserRole + 2).toInt() : -1;

	if (Index < 0)
		return;

	const QString ModelName = Item->data(0, Qt::UserRole).toString();
	const QPoint MenuPosition = mTree->viewport()->mapToGlobal(Position);
	QMenu Menu(this);

	// submodel rows work on all its joints
	if (Joint < 0)
	{
		QAction* EditAction = Menu.addAction(tr("Edit Joints"));
		Menu.addSeparator();
		Menu.addAction(tr("Delete All Joints"));
		QAction* Chosen = Menu.exec(MenuPosition);

		if (!Chosen)
			return;

		SelectJoint(ModelName, Index, 0);

		if (Chosen == EditAction)
		{
			gMainWindow->mActions[SW_EDIT_ACTION_JOINTS]->trigger();
			return;
		}

		swModel* Model = gMainWindow->GetActiveModel();
		swPart* Part = Model ? Model->GetJointPart() : nullptr;

		if (Part)
			Model->ClearJointsAction(Part);

		UpdateJoint();
		swView::UpdateAllViews();
		return;
	}

	QAction* MoveAction = Menu.addAction(tr("Move Joint"));
	Menu.addAction(tr("Rotate Joint"));
	Menu.addSeparator();
	QAction* DeleteAction = Menu.addAction(tr("Delete Joint"));
	QAction* Chosen = Menu.exec(MenuPosition);

	if (!Chosen)
		return;

	SelectJoint(ModelName, Index, Joint);

	if (Chosen == DeleteAction)
		ClearClicked();
	else
		gMainWindow->mActions[Chosen == MoveAction ? SW_PART_JOINT_MOVE : SW_PART_JOINT_ROTATE]->trigger();
}

// use to refresh the tree, poses and joint
void swJointsWidget::UpdateJoint()
{
	UpdateTree();
	emit PoseModelChanged();

	swModel* Model = gMainWindow ? gMainWindow->GetActiveModel() : nullptr;
	swPart* Part = Model ? Model->GetJointPart() : nullptr;
	const swPartJoint* Joint = Part && Part->HasJoints() ? &Part->mJoints[Model->GetJointIndex()] : nullptr;
	const bool HasJoint = Joint != nullptr;

	mJointLabel->setText(!Part ? tr("None: select a jointed submodel") : !HasJoint ? tr("No joint") : tr("Joint %1, %2, angle %3").arg(Model->GetJointIndex() + 1, 3, 10, QLatin1Char('0')).arg(Joint->Type == swJointType::Hinge ? tr("Hinge") : tr("Ball")).arg(swLength(Joint->Rotation), 0, 'f', 1));
	mAxisCombo->setEnabled(HasJoint && Joint->Type == swJointType::Hinge);
	UpdateClearButton();

	if (HasJoint)
	{
		int Axis = 0;

		for (int Index = 1; Index < 3; Index++)
			if (fabsf(Joint->Axis[Index]) > fabsf(Joint->Axis[Axis]))
				Axis = Index;

		mAxisCombo->setCurrentIndex(Axis);
	}
}

void swJointsWidget::AxisChanged(int Index)
{
	swModel* Model = gMainWindow->GetActiveModel();
	swPart* Part = Model ? Model->GetJointPart() : nullptr;

	if (Part)
		Model->SetJointAxisAction(Part, Index);

	UpdateJoint();
}

// use to find the joint picked in the tree
swPart* swJointsWidget::GetTreeJoint(swModel** Model, size_t* Joint) const
{
	swModel* ActiveModel = gMainWindow ? gMainWindow->GetActiveModel() : nullptr;
	Project* CurrentProject = ActiveModel ? ActiveModel->GetProject() : nullptr;
	const QTreeWidgetItem* Item = mTree->currentItem();

	if (!Item || !CurrentProject || Item->data(0, Qt::UserRole + 2).toInt() < 0)
		return nullptr;

	swModel* JointModel = CurrentProject->GetModel(Item->data(0, Qt::UserRole).toString());
	const int Index = Item->data(0, Qt::UserRole + 1).toInt();

	if (!JointModel || Index < 0 || Index >= static_cast<int>(JointModel->GetParts().size()))
		return nullptr;

	*Model = JointModel;
	*Joint = static_cast<size_t>(Item->data(0, Qt::UserRole + 2).toInt());

	return JointModel->GetParts()[Index].get();
}

void swJointsWidget::UpdateClearButton()
{
	swModel* Model;
	size_t Joint;

	mClearButton->setEnabled(GetTreeJoint(&Model, &Joint) != nullptr);
}

void swJointsWidget::ClearClicked()
{
	swModel* Model;
	size_t Joint;
	swPart* Part = GetTreeJoint(&Model, &Joint);

	if (!Part)
		return;

	Model->ClearJointAction(Part, Joint);
	UpdateJoint();
	swView::UpdateAllViews();
}

bool swJointsWidget::eventFilter(QObject* Object, QEvent* Event)
{
	// the delete key goes to the list, not the part delete
	if (Object == mTree && (Event->type() == QEvent::ShortcutOverride || Event->type() == QEvent::KeyPress) && static_cast<QKeyEvent*>(Event)->key() == Qt::Key_Delete && mClearButton->isEnabled())
	{
		if (Event->type() == QEvent::KeyPress)
			ClearClicked();

		Event->accept();
		return true;
	}

	return QWidget::eventFilter(Object, Event);
}

swPosesWidget::swPosesWidget(swJointsWidget* JointsWidget, QWidget* Parent)
	: QWidget(Parent), mJointsWidget(JointsWidget)
{
	// panel overview

	setWhatsThis(tr("<p><b>Poses</b></p><p>The poses saved in the model picked in the Joints panel, or in the current model, each holding the angles of that model's joints only. Save stores the joint angles as a named pose, Recall puts them back in one undo step, and Rename and Delete change the picked pose. Poses are saved with the model, and the Joints motion of the image dialog plays the poses of the main model in this order.</p>"));

	QVBoxLayout* Layout = new QVBoxLayout(this);

	// poses under a header like the joints
	mList = new QTreeWidget(this);
	mList->setHeaderLabels({ tr("Pose") });
	mList->setRootIsDecorated(false);
	mList->setToolTip(tr("Saved poses of this model, played in this order by the Joints motion"));
	Layout->addWidget(mList, 1);

	QHBoxLayout* Buttons = new QHBoxLayout();
	struct swPosesButton
	{
		QString Text;
		QString ToolTip;
		QString WhatsThis;
		void (swPosesWidget::*Slot)();
	};

	const swPosesButton Entries[] =
	{
		{ tr("Save"), tr("Save the joint angles of this model as a pose"), tr("<p><b>Save</b></p><p>Store the angles of the joints in this model under a name. Saving a name that already exists replaces that pose.</p>"), &swPosesWidget::SaveClicked },
		{ tr("Recall"), tr("Put the joints of this model back in the picked pose"), tr("<p><b>Recall</b></p><p>Turn the joints of this model back to the angles of the picked pose, in one undo step.</p>"), &swPosesWidget::RecallClicked },
		{ tr("Rename"), tr("Rename the picked pose"), tr("<p><b>Rename</b></p><p>Give the picked pose a new name.</p>"), &swPosesWidget::RenameClicked },
		{ tr("Delete"), tr("Delete the picked pose"), tr("<p><b>Delete</b></p><p>Remove the picked pose from the model. The joints keep their angles.</p>"), &swPosesWidget::DeleteClicked }
	};

	for (const swPosesButton& Entry : Entries)
	{
		QToolButton* Button = new QToolButton(this);
		Button->setText(Entry.Text);
		Button->setToolTip(Entry.ToolTip);
		Button->setWhatsThis(Entry.WhatsThis);
		connect(Button, &QToolButton::clicked, this, Entry.Slot);
		Buttons->addWidget(Button);
	}

	Layout->addLayout(Buttons);

	connect(mJointsWidget, &swJointsWidget::PoseModelChanged, this, &swPosesWidget::UpdateList);
	UpdateList();
}

void swPosesWidget::UpdateList()
{
	const int Row = mList->indexOfTopLevelItem(mList->currentItem());
	swModel* Model = mJointsWidget->GetPoseModel();

	mList->clear();

	if (Model)
		for (const swPose& Pose : Model->GetPoses())
			new QTreeWidgetItem(mList, { Pose.Name });

	if (Row < mList->topLevelItemCount())
		mList->setCurrentItem(mList->topLevelItem(Row));
}

void swPosesWidget::SaveClicked()
{
	swModel* Owner = mJointsWidget->GetPoseModel();

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

void swPosesWidget::RecallClicked()
{
	swModel* Owner = mJointsWidget->GetPoseModel();
	const int Row = mList->indexOfTopLevelItem(mList->currentItem());

	if (Owner && Row >= 0 && Row < static_cast<int>(Owner->GetPoses().size()))
		gMainWindow->GetActiveModel()->RecallPoseAction(Owner, Owner->GetPoses()[Row]);
}

void swPosesWidget::RenameClicked()
{
	swModel* Owner = mJointsWidget->GetPoseModel();
	const int Row = mList->indexOfTopLevelItem(mList->currentItem());

	if (!Owner || Row < 0)
		return;

	bool Ok;
	const QString Name = QInputDialog::getText(this, tr("Rename Pose"), tr("Name"), QLineEdit::Normal, mList->currentItem()->text(0), &Ok).trimmed();

	if (!Ok || Name.isEmpty())
		return;

	std::vector<swPose> Poses = Owner->GetPoses();
	Poses[Row].Name = Name;
	Owner->SetPoses(Poses);
	Owner->GetProject()->MarkAsModified();
	UpdateList();
}

void swPosesWidget::DeleteClicked()
{
	swModel* Owner = mJointsWidget->GetPoseModel();
	const int Row = mList->indexOfTopLevelItem(mList->currentItem());

	if (!Owner || Row < 0)
		return;

	std::vector<swPose> Poses = Owner->GetPoses();
	Poses.erase(Poses.begin() + Row);
	Owner->SetPoses(Poses);
	Owner->GetProject()->MarkAsModified();
	UpdateList();
}
