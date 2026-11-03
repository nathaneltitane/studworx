#pragma once

#include "sw_bricklink.h"

class swBrickLinkExportDialog : public QDialog
{
	Q_OBJECT

public:
	explicit swBrickLinkExportDialog(QWidget* Parent);

	swBrickLinkOptions GetOptions() const;

public slots:
	void accept() override;

protected slots:
	void BrowseExceptions();
	void LoadExceptions();
	void AddException();
	void RemoveExceptions();
	void UpdateExceptionButtons();

protected:
	swBrickLinkExceptions GetExceptions() const;
	void SetExceptions(const swBrickLinkExceptions& Exceptions);

	QComboBox* mConditionCombo;
	QLineEdit* mWantedListEdit;
	QCheckBox* mNotifyCheck;
	QLineEdit* mExceptionsEdit;
	QTableWidget* mExceptionsTable;
	QPushButton* mRemoveButton;
};

QString swChooseBrickLinkMappingFile(QWidget* Parent, const QString& FileName);
bool swUpdateBrickLinkMappingFile(QWidget* Parent, const QString& FileName, bool ShowSummary, const QString& ApiKey = QString());
std::optional<swBrickLinkOptions> swRequestBrickLinkExport(QWidget* Parent);
void swShowBrickLinkExportSummary(QWidget* Parent, const swBrickLinkExport& Export);
