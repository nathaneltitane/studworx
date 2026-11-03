#pragma once

#include "sw_project.h"

namespace Ui {
class swHTMLDialog;
}

class swHTMLDialog : public QDialog
{
	Q_OBJECT

public:
	explicit swHTMLDialog(QWidget* Parent, swHTMLExportOptions* Options);
	virtual ~swHTMLDialog();

public slots:
	void accept() override;

protected:
	void showEvent(QShowEvent* Event) override;

private slots:
	void OutputFolderBrowseClicked();

private:
	swHTMLExportOptions* mOptions;
	QColor mBackgroundColor;
	std::array<QColor, SW_HTML_OVERRIDE_COLOR_COUNT> mOverrideColors;
	QList<QWidget*> mOverrideColorWidgets;
	Ui::swHTMLDialog* ui;
};

