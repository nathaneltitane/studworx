#pragma once

#include "sw_model.h"

class swJointsWidget : public QWidget
{
	Q_OBJECT

public:
	swJointsWidget(QWidget* Parent);

	void UpdateJoint();
	swModel* GetPoseModel() const;

signals:
	void PoseModelChanged();

private slots:
	void AxisChanged(int Index);
	void ClearClicked();
	void TreeItemClicked(QTreeWidgetItem* Item);
	void UpdateClearButton();
	void TreeMenuRequested(const QPoint& Position);

private:
	void UpdateTree();
	void SelectJoint(const QString& ModelName, int Index, int Joint);
	swPart* GetTreeJoint(swModel** Model, size_t* Joint) const;
	bool eventFilter(QObject* Object, QEvent* Event) override;

	QTreeWidget* mTree;
	QLabel* mJointLabel;
	QComboBox* mAxisCombo;
	QToolButton* mClearButton;
};

class swPosesWidget : public QWidget
{
	Q_OBJECT

public:
	swPosesWidget(swJointsWidget* JointsWidget, QWidget* Parent);

	void UpdateList();

private slots:
	void SaveClicked();
	void RecallClicked();
	void RenameClicked();
	void DeleteClicked();

private:
	swJointsWidget* mJointsWidget;
	QTreeWidget* mList;
};
