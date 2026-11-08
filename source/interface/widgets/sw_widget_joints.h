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
	void UpdateClearButton();
	void TreeMenuRequested(const QPoint& Position);

private:
	void UpdateTree();
	void UpdateList();
	swModel* GetPoseModel() const;
	void SelectJoint(const QString& ModelName, int Index, int Joint);
	swPart* GetTreeJoint(swModel** Model, size_t* Joint) const;
	bool eventFilter(QObject* Object, QEvent* Event) override;

	QTreeWidget* mTree;
	QLabel* mPoseLabel;
	QListWidget* mList;
	QLabel* mJointLabel;
	QComboBox* mAxisCombo;
	QToolButton* mClearButton;
};
