#pragma once

#include "sw_model.h"

namespace Ui {
class swExplodeWidget;
}

class swExplodeWidget : public QWidget
{
	Q_OBJECT

public:
	swExplodeWidget(QWidget* Parent);
	virtual ~swExplodeWidget();

	void UpdateAnchors();

private slots:
	void UpdateUniform();
	void SettingsChanged();

private:
	swExplodeSettings GetSettings() const;

	Ui::swExplodeWidget* ui;
	QToolButton* mExplodeButton;
	QToolButton* mCollapseButton;
	QToolButton* mSaveCopyButton;
};
