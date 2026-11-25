#pragma once

#include "sw_instructions.h"
#include "sw_widget_spin_box.h"

class swCollapsibleWidgetButton;

// Length arrows step to whole increments.
class swInstructionsLengthSpinBox : public swRangeDoubleSpinBox
{
public:
	explicit swInstructionsLengthSpinBox(QWidget* Parent)
		: swRangeDoubleSpinBox(Parent)
	{
		// Typed values apply after a pause.
		mApplyTimer.setSingleShot(true);
		mApplyTimer.setInterval(750);
		connect(&mApplyTimer, &QTimer::timeout, this, &QAbstractSpinBox::interpretText);
		connect(lineEdit(), &QLineEdit::textEdited, &mApplyTimer, qOverload<>(&QTimer::start));
	}

	void stepBy(int Steps) override
	{
		const double Step = singleStep();
		const double Increments = value() / Step;
		const double Base = Steps > 0 ? std::floor(Increments + 1e-6) : std::ceil(Increments - 1e-6);

		setValue((Base + Steps) * Step);
	}

protected:
	QTimer mApplyTimer;
};

// Shows the units of a length.
class swInstructionsUnitButton : public QToolButton
{
	Q_OBJECT

public:
	swInstructionsUnitButton(QWidget* Parent, const QString& Name);

	swInstructionsPageUnits GetUnits(const swInstructions* Instructions) const;
	void UpdateUnits(const swInstructions* Instructions);

signals:
	void UnitsChanged();

protected:
	void ShowUnitsMenu();

	std::optional<swInstructionsPageUnits> mUnits;
};

enum class swInstructionsPageObject : int;

class swInstructionsPropertiesWidget : public QDockWidget
{
	Q_OBJECT

public:
	swInstructionsPropertiesWidget(QWidget* Parent, swInstructions* Instructions);

	void AddOptionsAction(QAction* Action);
	void SelectionChanged(QGraphicsItem* FocusItem, bool Clicked = false);
	void PageClicked(swInstructionsPageType PageType, std::optional<swInstructionsPageObject> Object = std::nullopt);
	void UpdateNoticeSize();
	void UpdateImageSize();
	void SetCurrentPage(int PageIndex);

protected:
	enum class CategoryIndex
	{
		Document,
		Cover,
		Publication,
		Page,
		Header,
		Footer,
		LeftBand,
		RightBand,
		Watermark,
		BillOfMaterials,
		PageLayout,
		Step,
		PartsList,
		Divider,
		Notice,
		Pointer,
		Box,
		Image,
		Count
	};

	enum class EditorType
	{
		Bool,
		Length,
		Percent,
		Choice,
		Color,
		Font,
		Text
	};

	enum class FocusType
	{
		None,
		StepImage,
		StepNumber,
		PartsList,
		PageNumber,
		Publication,
		CoverImage,
		BillOfMaterialsPartCount,
		PagePartCount,
		SubmodelPreview,
		BillOfMaterials,
		Notice,
		Pointer,
		Bands,
		BandContent,
		Watermark,
		WatermarkNotice,
		Image,
		PageImage
	};

	struct CategoryWidgets
	{
		swCollapsibleWidgetButton* Button = nullptr;
		QWidget* Rows = nullptr;
	};

	struct EditorWidgets
	{
		QWidget* Widget = nullptr;
		QLabel* Label = nullptr;
		swInstructionsUnitButton* UnitButton = nullptr;
		QDoubleSpinBox* SizeSpinBox = nullptr;
		QToolButton* BrowseButton = nullptr;
		QToolButton* SymbolButton = nullptr;
		QToolButton* FitButton = nullptr;
		EditorType Type = EditorType::Bool;
		QString Name;
		int Minimum = 0;
		int Maximum = 0;
		double PageFraction = 0.0;
	};

	void ShowPageSetupDialog();
	void CreateWidgets();
	void AddCategory(CategoryIndex Index, const QString& Title, const QString& WhatsThis);
	QWidget* AddRow(CategoryIndex Index, const std::vector<QWidget*>& Editors);
	QLabel* AddLabeledRow(CategoryIndex Index, const QString& Name, QWidget* Editor);
	QLabel* AddEditorRow(CategoryIndex Index, swInstructionsPropertyType Type);
	void RegisterEditor(swInstructionsPropertyType Type, EditorType Editor, QWidget* Widget, const QString& Name, const QString& ToolTip);
	QToolButton* CreateToggleEditor(swInstructionsPropertyType Type, const QString& Name, const QString& ToolTip);
	QDoubleSpinBox* CreateLengthEditor(swInstructionsPropertyType Type, const QString& Name, const QString& ToolTip, int Minimum, int Maximum, double PageFraction = 0.0);
	double GetEditorMaximum(const EditorWidgets& Editor) const;
	QSpinBox* CreatePercentEditor(swInstructionsPropertyType Type, const QString& Name, const QString& ToolTip, int Minimum, int Maximum);
	QComboBox* CreateChoiceEditor(swInstructionsPropertyType Type, const QString& Name, const QString& ToolTip, const QStringList& Choices, const QList<int>& Values = {});
	QToolButton* CreateApplyAllButton(const QString& ToolTip);
	QToolButton* CreateColorEditor(swInstructionsPropertyType Type, const QString& Name, const QString& ToolTip);
	QToolButton* CreateFontEditor(swInstructionsPropertyType Type, const QString& Name, const QString& ToolTip);
	QLineEdit* CreateTextEditor(swInstructionsPropertyType Type, const QString& Name, const QString& ToolTip, bool File);
	void ColorButtonClicked(swInstructionsPropertyType Type);
	void FontButtonClicked(swInstructionsPropertyType Type);
	void SetCategoriesExpanded(bool Expanded);
	int GetIntValue(swInstructionsPropertyType Type) const;
	swInstructionsPageUnits GetEditorUnits(swInstructionsPropertyType Type) const;
	QColor GetColorValue(swInstructionsPropertyType Type) const;
	void SetIntValue(swInstructionsPropertyType Type, int Value);
	void SetColorValue(swInstructionsPropertyType Type, const QColor& Color);
	void SetThumbnailPageScope(bool PageOnly);
	void DividerPresetClicked(int Preset);
	void SetBoxElement(swInstructionsBoxElement Element);
	void SetPublicationCover(bool Back);

	void UpdateWidget();
	void UpdateEditors();
	void UpdatePageLayout();
	void UpdateCategories();
	void FitWidth();
	swInstructionsPageSettings GetPageLayoutSettings() const;
	swInstructionsPageType GetPageType() const;

	swInstructions* mInstructions = nullptr;
	QWidget* mOptionsRow = nullptr;
	FocusType mFocus = FocusType::None;
	int mPageIndex = 0;
	swModel* mModel = nullptr;
	swStep mStep = 1;

	std::array<CategoryWidgets, static_cast<int>(CategoryIndex::Count)> mCategoryWidgets = {};
	std::array<EditorWidgets, static_cast<int>(swInstructionsPropertyType::Count)> mEditors;
	QSpinBox* mRowsSpinBox = nullptr;
	QSpinBox* mColumnsSpinBox = nullptr;
	QComboBox* mStepFlowComboBox = nullptr;
	QToolButton* mFinalModelButton = nullptr;
	QToolButton* mThumbnailPageButton = nullptr;
	QToolButton* mBackgroundPageButton = nullptr;
	QToolButton* mBandsPageButtons[4] = {};
	QDoubleSpinBox* mNoticeSizeSpinBoxes[2] = {};
	QLabel* mNoticeSizeLabels[2] = {};
	swInstructionsUnitButton* mNoticeSizeUnitButtons[2] = {};
	std::optional<swInstructionsElementKey> mNoticeKey;
	QSizeF mNoticeItemSize;
	QDoubleSpinBox* mImageSizeSpinBoxes[2] = {};
	QLabel* mImageSizeLabels[2] = {};
	swInstructionsUnitButton* mImageSizeUnitButtons[2] = {};
	std::optional<swInstructionsElementKey> mImageKey;
	qreal mImageAspect = 1.0;
	QToolButton* mBandsApplyAllButtons[4] = {};
	QToolButton* mWatermarkPageButton = nullptr;
	QToolButton* mThumbnailApplyAllButton = nullptr;
	QToolButton* mModelSizePageButton = nullptr;
	QToolButton* mModelSizeApplyAllButton = nullptr;
	std::vector<QToolButton*> mDividerPresetButtons;
	std::vector<QLabel*> mRowLabels;
	QComboBox* mBoxElementComboBox = nullptr;
	swInstructionsBoxElement mBoxElement = swInstructionsBoxElement::StepImage;
	std::array<std::vector<QWidget*>, static_cast<int>(swInstructionsBoxElement::Count)> mBoxRows;
	std::array<std::vector<QWidget*>, 2> mPublicationRows;
	QComboBox* mPublicationCoverComboBox = nullptr;
	QVBoxLayout* mCategoriesLayout = nullptr;
	QScrollArea* mScrollArea = nullptr;
};

// Shows a length in page units.
void swSetupLengthSpinBox(QDoubleSpinBox* SpinBox, swInstructionsPageUnits Units, double Minimum, double Maximum);
