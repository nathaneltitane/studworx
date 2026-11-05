#pragma once

#include "sw_model.h"

class swPosesWidget : public QWidget
{
	Q_OBJECT

public:
	swPosesWidget(QWidget* Parent);

	void UpdateJoint();

private slots:
	void SaveClicked();
	void RecallClicked();
	void RenameClicked();
	void DeleteClicked();
	void AxisChanged(int Index);
	void ClearClicked();

private:
	void UpdateList();
	swModel* GetMainModel() const;

	QListWidget* mList;
	QLabel* mJointLabel;
	QComboBox* mAxisCombo;
	QToolButton* mClearButton;
};
