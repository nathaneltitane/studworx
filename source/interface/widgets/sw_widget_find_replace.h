#pragma once

class swElidableToolButton;

class swFindReplaceWidget : public QWidget
{
	Q_OBJECT

public:
	swFindReplaceWidget(QWidget* Parent, swModel* Model, bool Replace);

protected slots:
	void FindColorIndexChanged(int ColorIndex);
	void FindButtonClicked();
	void ReplaceColorIndexChanged(int ColorIndex);
	void ReplaceButtonClicked();

protected:
	swElidableToolButton* mFindPartButton = nullptr;
	std::vector<std::pair<swPartData*, std::string>> mFindParts;
	std::vector<std::pair<int, bool>> mFindColors;
	swElidableToolButton* mReplacePartButton = nullptr;
};
