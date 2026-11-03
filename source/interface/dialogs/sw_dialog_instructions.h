#pragma once

#include "sw_instructions.h"

class swInstructionsGridWidget;
class swInstructionsEditorWidget;
class swInstructionsPageListWidget;
class swInstructionsPageWidget;
class swInstructionsPlacementWidget;
class swInstructionsPrintWidget;
class swInstructionsPropertiesWidget;
class swRangeSpinBox;

class swInstructionsDialog : public QMainWindow
{
	Q_OBJECT

public:
	swInstructionsDialog(QWidget* Parent, Project* Project);

	void SaveLayout();
	void UpdateOptions();

public slots:
	void ModelChanged();

public:
	void SetProject(Project* Project);

protected slots:
	void PageSetupChanged();
	void CurrentThumbnailChanged(int Index);

protected:
	bool eventFilter(QObject* Object, QEvent* Event) override;
	void closeEvent(QCloseEvent* Event) override;
	void UpdatePageList();
	void SnapPageListDock();
	void SnapEditorDock();
	QWidget* CreatePageBar();
	void ShowPage(int Row);
	int FindPage(swInstructionsPageType Type) const;
	void UpdatePageBar();
	swModel* GetEditableStepModel(int Row) const;
	bool IsEmptyStepPage(int Row) const;
	void InsertStepPage();
	void RemovePage();

	Project* mProject = nullptr;
	bool mUpdatePending = false;
	bool mPageListSnapPending = false;
	bool mEditorSnapPending = false;
	QTimer* mEditorSnapTimer = nullptr;

	int mCurrentPageNumber;
	swInstructions* mInstructions;

	swInstructionsPageWidget* mPageWidget = nullptr;
	swInstructionsPageListWidget* mPageListWidget = nullptr;
	swInstructionsPropertiesWidget* mPropertiesWidget = nullptr;
	swInstructionsPlacementWidget* mPlacementWidget = nullptr;
	swInstructionsGridWidget* mGridWidget = nullptr;
	swInstructionsPrintWidget* mPrintWidget = nullptr;
	swInstructionsEditorWidget* mEditorWidget = nullptr;

	QToolButton* mFirstButton = nullptr;
	QToolButton* mPreviousButton = nullptr;
	QToolButton* mCoverButton = nullptr;
	QToolButton* mStepPageButton = nullptr;
	QToolButton* mBlankPageButton = nullptr;
	QToolButton* mRemovePageButton = nullptr;
	QToolButton* mBillOfMaterialsButton = nullptr;
	QToolButton* mNextButton = nullptr;
	QToolButton* mLastButton = nullptr;
	swRangeSpinBox* mPageSpinBox = nullptr;
};
