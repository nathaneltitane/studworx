#pragma once

#include "sw_manager_thumbnail.h"

struct swMissingPart
{
	swPartData* PartData = nullptr;
	swPartData* Replacement = nullptr;
	QString Error;
	QString Origin;
	std::map<QString, int> Models;
	int ColorIndex = 0;
};

class swMissingPartsDialog : public QDialog
{
	Q_OBJECT

public:
	swMissingPartsDialog(QWidget* Parent, std::vector<swMissingPart> MissingParts);
	~swMissingPartsDialog();

protected slots:
	void ThumbnailReady(swPartThumbnailId ThumbnailId, QPixmap Pixmap);
	void ReplaceChecked();
	void CopyList();
	void UpdateReplaceButton();

protected:
	QString GetUsageText(const swMissingPart& MissingPart) const;

	std::vector<swMissingPart> mMissingParts;
	std::vector<std::pair<swPartThumbnailId, QTableWidgetItem*>> mThumbnails;
	QTableWidget* mPartsTable;
	QCheckBox* mDontWarnCheck;
	QCheckBox* mSaveCheck;
	QPushButton* mReplaceButton;
};
