#pragma once

#include "sw_bricklink.h"

class swBrickLinkExportDialog : public QDialog
{
	Q_OBJECT

public:
	explicit swBrickLinkExportDialog(QWidget* Parent);

	swBrickLinkOptions GetOptions() const;

protected:
	QComboBox* mConditionCombo;
	QLineEdit* mWantedListEdit;
	QCheckBox* mNotifyCheck;
};

QString swChooseBrickLinkMappingFile(QWidget* Parent, const QString& FileName);
bool swUpdateBrickLinkMappingFile(QWidget* Parent, const QString& FileName, bool ShowSummary, const QString& ApiKey = QString());
std::optional<swBrickLinkOptions> swRequestBrickLinkExport(QWidget* Parent);
void swShowBrickLinkExportSummary(QWidget* Parent, const swBrickLinkExport& Export);
