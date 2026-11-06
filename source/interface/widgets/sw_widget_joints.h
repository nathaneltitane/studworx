#pragma once

#include "sw_model.h"

class swJointsWidget : public QWidget
{
	Q_OBJECT

public:
	swJointsWidget(QWidget* Parent);

	void UpdateJoint();

private slots:
	void SaveClicked();
	void RecallClicked();
	void RenameClicked();
	void DeleteClicked();
	void AxisChanged(int Index);
	void ClearClicked();
	void TreeItemClicked(QTreeWidgetItem* Item);

private:
	void UpdateTree();
	void UpdateList();
	swModel* GetPoseModel() const;

	QTreeWidget* mTree;
	QLabel* mPoseLabel;
	QListWidget* mList;
	QLabel* mJointLabel;
	QComboBox* mAxisCombo;
	QToolButton* mClearButton;
};
