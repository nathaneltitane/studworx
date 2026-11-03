#include "sw_global.h"
#include "sw_window_utilities.h"
#include "sw_dialog_html.h"
#include "ui_sw_dialog_html.h"
#include "sw_project.h"

swHTMLDialog::swHTMLDialog(QWidget* Parent, swHTMLExportOptions* Options)
	: QDialog(Parent), ui(new Ui::swHTMLDialog)
{
    ui->setupUi(this);
	swSetupDialogHelp(this);

	connect(ui->outputFolderBrowse, &QPushButton::clicked, this, &swHTMLDialog::OutputFolderBrowseClicked);

	ui->stepWidth->setValidator(new QIntValidator(1, SW_MAXIMUM_IMAGE_DIMENSION, ui->stepWidth));
	ui->stepHeight->setValidator(new QIntValidator(1, SW_MAXIMUM_IMAGE_DIMENSION, ui->stepHeight));

	mOptions = Options;

	ui->outputFolder->setText(QDir::toNativeSeparators(mOptions->PathName));

	if (mOptions->CurrentOnly)
		ui->currentModelOnly->setChecked(true);
	else if (mOptions->SubModels)
		ui->currentModelSubmodels->setChecked(true);
	else
		ui->allModels->setChecked(true);

	ui->transparentImages->setChecked(mOptions->TransparentImages);
	ui->singlePage->setChecked(mOptions->SinglePage);
	ui->oneStepPerPage->setChecked(!mOptions->SinglePage);
	ui->indexPage->setChecked(mOptions->IndexPage);
	ui->stepWidth->setText(swFormatIntegerLocalized(mOptions->StepImagesWidth, ui->stepWidth->validator()->locale()));
	ui->stepHeight->setText(swFormatIntegerLocalized(mOptions->StepImagesHeight, ui->stepHeight->validator()->locale()));
	ui->partsAfterEachStep->setChecked(mOptions->PartsListStep);
	ui->partsAtTheEnd->setChecked(mOptions->PartsListEnd);
	ui->arrowPosition->setCurrentIndex(mOptions->ArrowPosition);
	ui->display->setCurrentIndex(mOptions->Display);

	// pixel sizes in the order of the display drop-down list, the instructions page keeps the typed size
	const QSize DisplaySizes[] = { QSize(), QSize(1920, 1080), QSize(1080, 1920), QSize(2048, 1536) };

	connect(ui->display, QOverload<int>::of(&QComboBox::activated), this, [this, DisplaySizes](int Index)
	{
		if (DisplaySizes[Index].isEmpty())
			return;

		ui->stepWidth->setText(swFormatIntegerLocalized(DisplaySizes[Index].width(), ui->stepWidth->validator()->locale()));
		ui->stepHeight->setText(swFormatIntegerLocalized(DisplaySizes[Index].height(), ui->stepHeight->validator()->locale()));
	});
	ui->overrideInstructions->setChecked(mOptions->OverrideInstructions);
	mBackgroundColor = mOptions->BackgroundColor;
	swUpdateColorButton(ui->backgroundColor, mBackgroundColor);

	connect(ui->backgroundColor, &QToolButton::clicked, this, [this]()
	{
		const QColor SelectedColor = swPickColor(ui->backgroundColor, mBackgroundColor);

		if (!SelectedColor.isValid())
			return;

		mBackgroundColor = SelectedColor;
		swUpdateColorButton(ui->backgroundColor, SelectedColor);
	});

	// rows in the order of the export override colors, half in each column
	const std::pair<QString, QString> OverrideColorRows[SW_HTML_OVERRIDE_COLOR_COUNT] =
	{
		{ tr("Page Background"), tr("Color filling the whole page, margins included.") },
		{ tr("Cover Background"), tr("Color filling the whole cover page, margins included.") },
		{ tr("Bill of Materials Background"), tr("Color filling the whole bill of materials page, margins included.") },
		{ tr("Cover Title"), tr("Color of the model title and author.") },
		{ tr("Thumbnail Background"), tr("Color behind each part and its count, transparent draws none.") },
		{ tr("Notice Text"), tr("Color of the notice text.") },
		{ tr("Fade"), tr("Color tinting the parts added in earlier steps, kept between 85 and 95 percent opaque.") },
		{ tr("Highlight"), tr("Color of the outline around the parts added in each step, kept between 85 and 95 percent opaque.") },
		{ tr("Step Background"), tr("Color filling the area of each step, transparent shows the page background.") },
		{ tr("Step Number"), tr("Color of the step numbers.") },
		{ tr("Page Number"), tr("Color of the page numbers.") },
		{ tr("Parts List Text"), tr("Color of the part counts.") },
		{ tr("Divider"), tr("Color of the lines between steps.") },
		{ tr("Pointer"), tr("Color of the pointers.") },
		{ tr("Box Border"), tr("Color of the line around every element box.") },
		{ tr("Box Background"), tr("Color filling every element box, transparent shows what lies behind.") }
	};

	QGridLayout* OverrideLayout = qobject_cast<QGridLayout*>(ui->OverrideGroup->layout());
	QWidget* PreviousWidget = ui->overrideInstructions;
	mOverrideColors = mOptions->OverrideColors;

	for (int Row = 0; Row < SW_HTML_OVERRIDE_COLOR_COUNT; Row++)
	{
		const auto& [Name, ToolTip] = OverrideColorRows[Row];
		QLabel* Label = new QLabel(Name, ui->OverrideGroup);
		QToolButton* Button = new QToolButton(ui->OverrideGroup);
		const int Column = Row / (SW_HTML_OVERRIDE_COLOR_COUNT / 2) * 2;
		const int GridRow = Row % (SW_HTML_OVERRIDE_COLOR_COUNT / 2);

		Label->setToolTip(ToolTip);
		Label->setBuddy(Button);
		Button->setToolTip(ToolTip);
		Button->setAccessibleName(Name);
		Button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
		Button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
		swUpdateColorButton(Button, mOverrideColors[Row]);

		connect(Button, &QToolButton::clicked, this, [this, Button, Row]()
		{
			const QColor SelectedColor = swPickColor(Button, mOverrideColors[Row]);

			if (!SelectedColor.isValid())
				return;

			mOverrideColors[Row] = SelectedColor;
			swUpdateColorButton(Button, SelectedColor);
		});

		OverrideLayout->addWidget(Label, GridRow, Column);
		OverrideLayout->addWidget(Button, GridRow, Column + 1);
		setTabOrder(PreviousWidget, Button);
		PreviousWidget = Button;
		mOverrideColorWidgets << Label << Button;
	}

	OverrideLayout->setColumnStretch(3, 1);

	// single page follows the instructions window unless overridden
	auto UpdatePageOptions = [this]()
	{
		const bool SinglePage = ui->singlePage->isChecked();
		const bool ExportOptionsUsed = !SinglePage || ui->overrideInstructions->isChecked();

		ui->indexPage->setEnabled(!SinglePage);
		ui->arrowPositionLabel->setEnabled(SinglePage);
		ui->arrowPosition->setEnabled(SinglePage);
		ui->displayLabel->setEnabled(SinglePage);
		ui->display->setEnabled(SinglePage);
		ui->overrideInstructions->setEnabled(SinglePage);

		for (QWidget* Widget : mOverrideColorWidgets)
			Widget->setEnabled(SinglePage && ui->overrideInstructions->isChecked());

		ui->transparentImages->setEnabled(ExportOptionsUsed);
		ui->backgroundColorLabel->setEnabled(ExportOptionsUsed && !ui->transparentImages->isChecked());
		ui->backgroundColor->setEnabled(ExportOptionsUsed && !ui->transparentImages->isChecked());
		ui->PartsGroup->setEnabled(ExportOptionsUsed);
	};

	connect(ui->singlePage, &QRadioButton::toggled, this, UpdatePageOptions);
	connect(ui->overrideInstructions, &QCheckBox::toggled, this, UpdatePageOptions);
	connect(ui->transparentImages, &QCheckBox::toggled, this, UpdatePageOptions);
	UpdatePageOptions();
}

// use to share the label column once shown
void swHTMLDialog::showEvent(QShowEvent* Event)
{
	QDialog::showEvent(Event);

	const int Spacing = swGetLayoutSpacing(this, Qt::Horizontal);
	const QList<QGridLayout*> Layouts = { qobject_cast<QGridLayout*>(ui->OutputGroup->layout()), qobject_cast<QGridLayout*>(ui->PagesGroup->layout()), qobject_cast<QGridLayout*>(ui->OverrideGroup->layout()), qobject_cast<QGridLayout*>(ui->ImagesGroup->layout()) };
	int LabelWidth = 0;

	for (QGridLayout* Layout : Layouts)
		for (int Row = 0; Row < Layout->rowCount(); Row++)
			if (QLayoutItem* Item = Layout->itemAtPosition(Row, 0); Item && qobject_cast<QLabel*>(Item->widget()))
				LabelWidth = qMax(LabelWidth, Item->sizeHint().width());

	for (QGridLayout* Layout : Layouts)
	{
		Layout->setColumnMinimumWidth(0, LabelWidth + Spacing * 2);
		Layout->setColumnStretch(1, 1);
	}

	// the second override column keeps the same label gap
	QGridLayout* OverrideLayout = qobject_cast<QGridLayout*>(ui->OverrideGroup->layout());
	int SecondLabelWidth = 0;

	for (int Row = 0; Row < OverrideLayout->rowCount(); Row++)
		if (QLayoutItem* Item = OverrideLayout->itemAtPosition(Row, 2); Item && qobject_cast<QLabel*>(Item->widget()))
			SecondLabelWidth = qMax(SecondLabelWidth, Item->sizeHint().width());

	OverrideLayout->setColumnMinimumWidth(2, SecondLabelWidth + Spacing * 2);
}

swHTMLDialog::~swHTMLDialog()
{
    delete ui;
}

void swHTMLDialog::accept()
{
	QString pathName = ui->outputFolder->text();

	if (pathName.isEmpty())
	{
		QMessageBox::information(this, tr("Export HTML"), tr("Output folder cannot be empty."));
		return;
	}

	bool WidthOk, HeightOk;
	const int Width = ui->stepWidth->validator()->locale().toInt(ui->stepWidth->text(), &WidthOk);
	const int Height = ui->stepHeight->validator()->locale().toInt(ui->stepHeight->text(), &HeightOk);

	if (!WidthOk || !ui->stepWidth->hasAcceptableInput())
	{
		QMessageBox::information(this, windowTitle(), tr("Please enter a width between 1 and 32768 pixels."));
		ui->stepWidth->setFocus();
		return;
	}

	if (!HeightOk || !ui->stepHeight->hasAcceptableInput())
	{
		QMessageBox::information(this, windowTitle(), tr("Please enter a height between 1 and 32768 pixels."));
		ui->stepHeight->setFocus();
		return;
	}

	mOptions->PathName = pathName;
	mOptions->SubModels = ui->currentModelSubmodels->isChecked();
	mOptions->CurrentOnly = ui->currentModelOnly->isChecked();
	mOptions->TransparentImages = ui->transparentImages->isChecked();
	mOptions->SinglePage = ui->singlePage->isChecked();
	mOptions->IndexPage = ui->indexPage->isChecked();
	mOptions->StepImagesWidth = Width;
	mOptions->StepImagesHeight = Height;
	mOptions->PartsListStep = ui->partsAfterEachStep->isChecked();
	mOptions->PartsListEnd = ui->partsAtTheEnd->isChecked();
	mOptions->ArrowPosition = ui->arrowPosition->currentIndex();
	mOptions->Display = ui->display->currentIndex();
	mOptions->OverrideInstructions = ui->overrideInstructions->isChecked();
	mOptions->BackgroundColor = mBackgroundColor;
	mOptions->OverrideColors = mOverrideColors;

	QDialog::accept();
}

void swHTMLDialog::OutputFolderBrowseClicked()
{
	QString result = QFileDialog::getExistingDirectory(this, tr("Select Output Folder"), ui->outputFolder->text());

	if (!result.isEmpty())
		ui->outputFolder->setText(QDir::toNativeSeparators(result));
}
