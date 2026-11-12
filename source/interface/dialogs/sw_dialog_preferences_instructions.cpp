#include "sw_global.h"
#include "sw_dialog_preferences.h"
#include "ui_sw_dialog_preferences.h"
#include "sw_window_utilities.h"
#include "sw_dialog_setup_page.h"
#include "sw_dialog_instructions_properties.h"

void swInstructionsPreferences::Load()
{
	PageSetup = { swGetProfileFloat(SW_PROFILE_INSTRUCTIONS_DEFAULT_PAGE_WIDTH), swGetProfileFloat(SW_PROFILE_INSTRUCTIONS_DEFAULT_PAGE_HEIGHT), swGetProfileFloat(SW_PROFILE_INSTRUCTIONS_DEFAULT_MARGIN_LEFT), swGetProfileFloat(SW_PROFILE_INSTRUCTIONS_DEFAULT_MARGIN_RIGHT), swGetProfileFloat(SW_PROFILE_INSTRUCTIONS_DEFAULT_MARGIN_TOP), swGetProfileFloat(SW_PROFILE_INSTRUCTIONS_DEFAULT_MARGIN_BOTTOM) };
	PageUnits = swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_DEFAULT_PAGE_UNITS);
	MarginUnits = swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_DEFAULT_MARGIN_UNITS);
	ShowGrid = swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_DEFAULT_SHOW_GRID) != 0;
	SnapToGrid = swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_DEFAULT_SNAP_TO_GRID) != 0;
	GridSize = swGetProfileFloat(SW_PROFILE_INSTRUCTIONS_DEFAULT_GRID_SIZE);
	GridDivisions = swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_DEFAULT_GRID_DIVISIONS);
	GridWithinMargins = swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_DEFAULT_GRID_WITHIN_MARGINS) != 0;
	NewModelDefaults = swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_NEW_MODEL_DEFAULTS) != 0;
	ShowMargins = swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_SHOW_MARGINS) != 0;
	ShowRulers = swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_SHOW_RULERS) != 0;
	ShowElementMargins = swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_SHOW_ELEMENT_MARGINS) != 0;
	ShowElementPadding = swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_SHOW_ELEMENT_PADDING) != 0;
	ShowElementAnchors = swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_SHOW_ELEMENT_ANCHORS) != 0;
	ElementMarginColor = swGetProfileUnsignedInteger(SW_PROFILE_INSTRUCTIONS_ELEMENT_MARGIN_COLOR);
	ElementPaddingColor = swGetProfileUnsignedInteger(SW_PROFILE_INSTRUCTIONS_ELEMENT_PADDING_COLOR);
	ElementAnchorColor = swGetProfileUnsignedInteger(SW_PROFILE_INSTRUCTIONS_ELEMENT_ANCHOR_COLOR);
	RulerPointerColor = swGetProfileUnsignedInteger(SW_PROFILE_INSTRUCTIONS_RULER_POINTER_COLOR);
	GridPrimaryColor = swQColorFromRGBA(swGetProfileUnsignedInteger(SW_PROFILE_INSTRUCTIONS_GRID_PRIMARY_COLOR));
	GridSecondaryColor = swQColorFromRGBA(swGetProfileUnsignedInteger(SW_PROFILE_INSTRUCTIONS_GRID_SECONDARY_COLOR));
	MarginColor = swQColorFromRGBA(swGetProfileUnsignedInteger(SW_PROFILE_INSTRUCTIONS_MARGIN_COLOR));
	EditorHighlight = swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_EDITOR_HIGHLIGHT) != 0;
	EditorNumbers = swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_EDITOR_NUMBERS) != 0;
	EditorWrap = swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_EDITOR_WRAP) != 0;
	EditorMarkers = swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_EDITOR_MARKERS) != 0;
}

void swInstructionsPreferences::Save() const
{
	swSetProfileFloat(SW_PROFILE_INSTRUCTIONS_DEFAULT_PAGE_WIDTH, PageSetup.Width);
	swSetProfileFloat(SW_PROFILE_INSTRUCTIONS_DEFAULT_PAGE_HEIGHT, PageSetup.Height);
	swSetProfileFloat(SW_PROFILE_INSTRUCTIONS_DEFAULT_MARGIN_LEFT, PageSetup.MarginLeft);
	swSetProfileFloat(SW_PROFILE_INSTRUCTIONS_DEFAULT_MARGIN_RIGHT, PageSetup.MarginRight);
	swSetProfileFloat(SW_PROFILE_INSTRUCTIONS_DEFAULT_MARGIN_TOP, PageSetup.MarginTop);
	swSetProfileFloat(SW_PROFILE_INSTRUCTIONS_DEFAULT_MARGIN_BOTTOM, PageSetup.MarginBottom);
	swSetProfileInteger(SW_PROFILE_INSTRUCTIONS_DEFAULT_PAGE_UNITS, PageUnits);
	swSetProfileInteger(SW_PROFILE_INSTRUCTIONS_DEFAULT_MARGIN_UNITS, MarginUnits);
	swSetProfileInteger(SW_PROFILE_INSTRUCTIONS_DEFAULT_SHOW_GRID, ShowGrid);
	swSetProfileInteger(SW_PROFILE_INSTRUCTIONS_DEFAULT_SNAP_TO_GRID, SnapToGrid);
	swSetProfileFloat(SW_PROFILE_INSTRUCTIONS_DEFAULT_GRID_SIZE, GridSize);
	swSetProfileInteger(SW_PROFILE_INSTRUCTIONS_DEFAULT_GRID_DIVISIONS, GridDivisions);
	swSetProfileInteger(SW_PROFILE_INSTRUCTIONS_DEFAULT_GRID_WITHIN_MARGINS, GridWithinMargins);
	swSetProfileInteger(SW_PROFILE_INSTRUCTIONS_NEW_MODEL_DEFAULTS, NewModelDefaults);
	swSetProfileInteger(SW_PROFILE_INSTRUCTIONS_SHOW_MARGINS, ShowMargins);
	swSetProfileInteger(SW_PROFILE_INSTRUCTIONS_SHOW_RULERS, ShowRulers);
	swSetProfileInteger(SW_PROFILE_INSTRUCTIONS_SHOW_ELEMENT_MARGINS, ShowElementMargins);
	swSetProfileInteger(SW_PROFILE_INSTRUCTIONS_SHOW_ELEMENT_PADDING, ShowElementPadding);
	swSetProfileInteger(SW_PROFILE_INSTRUCTIONS_SHOW_ELEMENT_ANCHORS, ShowElementAnchors);
	swSetProfileUnsignedInteger(SW_PROFILE_INSTRUCTIONS_ELEMENT_MARGIN_COLOR, ElementMarginColor);
	swSetProfileUnsignedInteger(SW_PROFILE_INSTRUCTIONS_ELEMENT_PADDING_COLOR, ElementPaddingColor);
	swSetProfileUnsignedInteger(SW_PROFILE_INSTRUCTIONS_ELEMENT_ANCHOR_COLOR, ElementAnchorColor);
	swSetProfileUnsignedInteger(SW_PROFILE_INSTRUCTIONS_RULER_POINTER_COLOR, RulerPointerColor);
	swSetProfileUnsignedInteger(SW_PROFILE_INSTRUCTIONS_GRID_PRIMARY_COLOR, swRGBAFromQColor(GridPrimaryColor));
	swSetProfileUnsignedInteger(SW_PROFILE_INSTRUCTIONS_GRID_SECONDARY_COLOR, swRGBAFromQColor(GridSecondaryColor));
	swSetProfileUnsignedInteger(SW_PROFILE_INSTRUCTIONS_MARGIN_COLOR, swRGBAFromQColor(MarginColor));
	swSetProfileInteger(SW_PROFILE_INSTRUCTIONS_EDITOR_HIGHLIGHT, EditorHighlight);
	swSetProfileInteger(SW_PROFILE_INSTRUCTIONS_EDITOR_NUMBERS, EditorNumbers);
	swSetProfileInteger(SW_PROFILE_INSTRUCTIONS_EDITOR_WRAP, EditorWrap);
	swSetProfileInteger(SW_PROFILE_INSTRUCTIONS_EDITOR_MARKERS, EditorMarkers);
}

// use to build the instructions tab
void swPreferencesDialog::CreateInstructionsTab()
{
	swInstructionsPreferences& Instructions = mOptions->Instructions;

	QWidget* Tab = new QWidget(ui->tabWidget);
	Tab->setWhatsThis(tr("<p><b>Instructions Preferences</b></p><p>Set the default page setup and grid that new models and the Default Page Setup option of the Properties panel load, and the options of the Instructions window.</p>"));
	QFormLayout* TabLayout = new QFormLayout(Tab);
	TabLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

	// label left of the checkbox
	auto CreateCheckBox = [](QFormLayout* Layout, QWidget* Parent, const QString& Text, const QString& ToolTip, bool Checked)
	{
		QCheckBox* CheckBox = new QCheckBox(Parent);
		CheckBox->setAccessibleName(Text);
		CheckBox->setToolTip(ToolTip);
		CheckBox->setChecked(Checked);

		QLabel* Label = new QLabel(Text, Parent);
		Label->setToolTip(ToolTip);
		Label->setBuddy(CheckBox);
		Layout->addRow(Label, CheckBox);

		return CheckBox;
	};

	QGroupBox* PageGroup = new QGroupBox(tr("Default Page Setup"), Tab);
	QFormLayout* PageLayout = new QFormLayout(PageGroup);
	TabLayout->addRow(PageGroup);

	QHBoxLayout* PageRowLayout = new QHBoxLayout();

	// button shows the default page setup
	mInstructionsPageButton = new QPushButton(PageGroup);
	mInstructionsPageButton->setToolTip(tr("Choose the default page size, orientation, margins and their units."));
	mInstructionsPageButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	PageRowLayout->addWidget(mInstructionsPageButton, 1);

	QPushButton* PageResetButton = new QPushButton(tr("Reset"), PageGroup);
	PageResetButton->setToolTip(tr("Bring back the built-in default page setup and grid, and the page margin and grid line colors of the Interface page."));
	PageRowLayout->addWidget(PageResetButton);

	QLabel* PageLabel = new QLabel(tr("Page"), PageGroup);
	PageLabel->setToolTip(tr("Page size, orientation and margins of new instructions."));
	PageLayout->addRow(PageLabel, PageRowLayout);

	mInstructionsNewModelDefaults = CreateCheckBox(PageLayout, PageGroup, tr("Load on New Models"), tr("Start every new model with the default page setup and grid."), Instructions.NewModelDefaults);

	QGroupBox* GridGroup = new QGroupBox(tr("Default Grid"), Tab);
	QFormLayout* GridLayout = new QFormLayout(GridGroup);
	TabLayout->addRow(GridGroup);

	mInstructionsShowGrid = CreateCheckBox(GridLayout, GridGroup, tr("Show Grid"), tr("Show the grid on the pages of new instructions, it is never printed."), Instructions.ShowGrid);
	mInstructionsSnapToGrid = CreateCheckBox(GridLayout, GridGroup, tr("Snap to Grid"), tr("Snap dragged elements to the grid of new instructions."), Instructions.SnapToGrid);

	mInstructionsGridSize = new swInstructionsLengthSpinBox(GridGroup);
	mInstructionsGridSize->setToolTip(tr("Main unit of the grid drawn in the Page Grid Primary Lines color, in the margin units of the default page setup."));
	mInstructionsGridSize->setAccessibleName(tr("Grid Size"));
	QLabel* GridSizeLabel = new QLabel(tr("Grid Size"), GridGroup);
	GridSizeLabel->setBuddy(mInstructionsGridSize);
	GridSizeLabel->setToolTip(mInstructionsGridSize->toolTip());
	GridLayout->addRow(GridSizeLabel, mInstructionsGridSize);

	mInstructionsGridDivisions = new swRangeSpinBox(GridGroup);
	mInstructionsGridDivisions->setRange(1, SW_GRID_DIVISIONS_MAXIMUM);
	mInstructionsGridDivisions->setPrefix(QLatin1String("1/"));
	mInstructionsGridDivisions->setValue(Instructions.GridDivisions);
	mInstructionsGridDivisions->setToolTip(tr("Split each grid unit into fractions drawn in the Page Grid Secondary Lines color, dragged elements snap to the fractions."));
	mInstructionsGridDivisions->setAccessibleName(tr("Divisions"));
	QLabel* GridDivisionsLabel = new QLabel(tr("Divisions"), GridGroup);
	GridDivisionsLabel->setBuddy(mInstructionsGridDivisions);
	GridDivisionsLabel->setToolTip(mInstructionsGridDivisions->toolTip());
	GridLayout->addRow(GridDivisionsLabel, mInstructionsGridDivisions);

	mInstructionsGridScope = new QComboBox(GridGroup);
	mInstructionsGridScope->addItems({ tr("Margins"), tr("Page") });
	mInstructionsGridScope->setToolTip(tr("Lay the grid out within the margins or across the whole page."));
	mInstructionsGridScope->setAccessibleName(tr("Grid Area"));
	mInstructionsGridScope->setCurrentIndex(Instructions.GridWithinMargins ? 0 : 1);
	QLabel* GridScopeLabel = new QLabel(tr("Grid Area"), GridGroup);
	GridScopeLabel->setBuddy(mInstructionsGridScope);
	GridScopeLabel->setToolTip(mInstructionsGridScope->toolTip());
	GridLayout->addRow(GridScopeLabel, mInstructionsGridScope);

	QGroupBox* WindowGroup = new QGroupBox(tr("Instructions Page"), Tab);
	QFormLayout* WindowLayout = new QFormLayout(WindowGroup);
	TabLayout->addRow(WindowGroup);

	mInstructionsShowMargins = CreateCheckBox(WindowLayout, WindowGroup, tr("Show Margins"), tr("Outline the page margins on screen, they are never printed."), Instructions.ShowMargins);
	mInstructionsShowRulers = CreateCheckBox(WindowLayout, WindowGroup, tr("Show Rulers"), tr("Show rulers in the page units along the page, their arrows follow the pointer."), Instructions.ShowRulers);
	mInstructionsShowElementMargins = CreateCheckBox(WindowLayout, WindowGroup, tr("Show Element Margins"), tr("Fill the margins around each element on screen with the element margin color, they are never printed."), Instructions.ShowElementMargins);
	mInstructionsShowElementPadding = CreateCheckBox(WindowLayout, WindowGroup, tr("Show Element Padding"), tr("Fill the padding inside the border of each element on screen with the element padding color, it is never printed."), Instructions.ShowElementPadding);
	mInstructionsShowElementAnchors = CreateCheckBox(WindowLayout, WindowGroup, tr("Show Element Anchors"), tr("Mark the corners, edge centers and center of each element on screen, they are never printed."), Instructions.ShowElementAnchors);

	// line colors live on the interface page
	auto CreateColorButton = [this](const QString& Name, const QString& ToolTip, QColor* Color)
	{
		QToolButton* Button = new QToolButton(ui->InstructionsColorsGroup);
		Button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
		Button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
		Button->setToolTip(ToolTip);
		Button->setAccessibleName(Name);
		swUpdateColorButton(Button, *Color);

		QLabel* Label = new QLabel(Name, ui->InstructionsColorsGroup);
		Label->setBuddy(Button);
		Label->setToolTip(ToolTip);
		const int Row = ui->InstructionsColorsLayout->rowCount();
		ui->InstructionsColorsLayout->addWidget(Label, Row, 0);
		ui->InstructionsColorsLayout->addWidget(Button, Row, 1);

		connect(Button, &QToolButton::clicked, this, [Button, Color]()
		{
			const QColor SelectedColor = swPickColor(Button, *Color);

			if (!SelectedColor.isValid())
				return;

			*Color = SelectedColor;
			swUpdateColorButton(Button, SelectedColor);
		});

		return Button;
	};

	mInstructionsMarginColor = CreateColorButton(tr("Page Margins"), tr("Color of the dashed outline of the page margins."), &Instructions.MarginColor);
	mInstructionsGridPrimaryColor = CreateColorButton(tr("Page Grid Primary Lines"), tr("Color of the grid lines at every grid unit."), &Instructions.GridPrimaryColor);
	mInstructionsGridSecondaryColor = CreateColorButton(tr("Page Grid Secondary Lines"), tr("Color of the grid lines at the fractions of a grid unit."), &Instructions.GridSecondaryColor);

	QGroupBox* EditorGroup = new QGroupBox(tr("Editor"), Tab);
	QFormLayout* EditorLayout = new QFormLayout(EditorGroup);
	TabLayout->addRow(EditorGroup);

	mInstructionsEditorHighlight = CreateCheckBox(EditorLayout, EditorGroup, tr("Syntax Highlighting"), tr("Color the commands of the model file in the Editor panel."), Instructions.EditorHighlight);
	mInstructionsEditorNumbers = CreateCheckBox(EditorLayout, EditorGroup, tr("Line Numbers"), tr("Number the lines of the model file in the Editor panel."), Instructions.EditorNumbers);
	mInstructionsEditorWrap = CreateCheckBox(EditorLayout, EditorGroup, tr("Wrap Lines"), tr("Wrap long lines of the model file in the Editor panel."), Instructions.EditorWrap);
	mInstructionsEditorMarkers = CreateCheckBox(EditorLayout, EditorGroup, tr("Whitespace Markers"), tr("Mark tabs, spaces and line ends in the Editor panel."), Instructions.EditorMarkers);

	ui->tabWidget->insertTab(ui->tabWidget->indexOf(ui->tabCategories), Tab, tr("Instructions"));

	connect(mInstructionsPageButton, &QPushButton::clicked, this, [this]()
	{
		swInstructionsPreferences& Instructions = mOptions->Instructions;

		// store units right away
		const int PageUnits = swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_DEFAULT_PAGE_UNITS);
		const int MarginUnits = swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_DEFAULT_MARGIN_UNITS);
		swSetProfileInteger(SW_PROFILE_INSTRUCTIONS_DEFAULT_PAGE_UNITS, Instructions.PageUnits);
		swSetProfileInteger(SW_PROFILE_INSTRUCTIONS_DEFAULT_MARGIN_UNITS, Instructions.MarginUnits);

		Instructions.GridSize = static_cast<float>(mInstructionsGridSize->value()) * swGetPageUnitScale(static_cast<swInstructionsPageUnits>(Instructions.MarginUnits));

		swPageSetupDialog Dialog(this, &Instructions.PageSetup, true);

		if (Dialog.exec() == QDialog::Accepted)
		{
			Instructions.PageUnits = swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_DEFAULT_PAGE_UNITS);
			Instructions.MarginUnits = swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_DEFAULT_MARGIN_UNITS);
		}

		swSetProfileInteger(SW_PROFILE_INSTRUCTIONS_DEFAULT_PAGE_UNITS, PageUnits);
		swSetProfileInteger(SW_PROFILE_INSTRUCTIONS_DEFAULT_MARGIN_UNITS, MarginUnits);

		UpdateInstructionsPage();
	});

	connect(PageResetButton, &QPushButton::clicked, this, [this]()
	{
		swInstructionsPreferences& Instructions = mOptions->Instructions;

		Instructions.PageSetup = { swGetProfileDefaultFloat(SW_PROFILE_INSTRUCTIONS_DEFAULT_PAGE_WIDTH), swGetProfileDefaultFloat(SW_PROFILE_INSTRUCTIONS_DEFAULT_PAGE_HEIGHT), swGetProfileDefaultFloat(SW_PROFILE_INSTRUCTIONS_DEFAULT_MARGIN_LEFT), swGetProfileDefaultFloat(SW_PROFILE_INSTRUCTIONS_DEFAULT_MARGIN_RIGHT), swGetProfileDefaultFloat(SW_PROFILE_INSTRUCTIONS_DEFAULT_MARGIN_TOP), swGetProfileDefaultFloat(SW_PROFILE_INSTRUCTIONS_DEFAULT_MARGIN_BOTTOM) };
		Instructions.PageUnits = swGetProfileDefaultInteger(SW_PROFILE_INSTRUCTIONS_DEFAULT_PAGE_UNITS);
		Instructions.MarginUnits = swGetProfileDefaultInteger(SW_PROFILE_INSTRUCTIONS_DEFAULT_MARGIN_UNITS);
		Instructions.GridSize = swGetProfileDefaultFloat(SW_PROFILE_INSTRUCTIONS_DEFAULT_GRID_SIZE);

		mInstructionsShowGrid->setChecked(swGetProfileDefaultInteger(SW_PROFILE_INSTRUCTIONS_DEFAULT_SHOW_GRID) != 0);
		mInstructionsSnapToGrid->setChecked(swGetProfileDefaultInteger(SW_PROFILE_INSTRUCTIONS_DEFAULT_SNAP_TO_GRID) != 0);
		mInstructionsGridScope->setCurrentIndex(swGetProfileDefaultInteger(SW_PROFILE_INSTRUCTIONS_DEFAULT_GRID_WITHIN_MARGINS) ? 0 : 1);
		mInstructionsGridDivisions->setValue(swGetProfileDefaultInteger(SW_PROFILE_INSTRUCTIONS_DEFAULT_GRID_DIVISIONS));

		Instructions.GridPrimaryColor = swQColorFromRGBA(swGetProfileDefaultUnsignedInteger(SW_PROFILE_INSTRUCTIONS_GRID_PRIMARY_COLOR));
		Instructions.GridSecondaryColor = swQColorFromRGBA(swGetProfileDefaultUnsignedInteger(SW_PROFILE_INSTRUCTIONS_GRID_SECONDARY_COLOR));
		Instructions.MarginColor = swQColorFromRGBA(swGetProfileDefaultUnsignedInteger(SW_PROFILE_INSTRUCTIONS_MARGIN_COLOR));
		swUpdateColorButton(mInstructionsGridPrimaryColor, Instructions.GridPrimaryColor);
		swUpdateColorButton(mInstructionsGridSecondaryColor, Instructions.GridSecondaryColor);
		swUpdateColorButton(mInstructionsMarginColor, Instructions.MarginColor);

		UpdateInstructionsPage();
	});

	UpdateInstructionsPage();
}

// use to show the default page units
void swPreferencesDialog::UpdateInstructionsPage()
{
	const swInstructionsPreferences& Instructions = mOptions->Instructions;
	const swInstructionsPageSetup& PageSetup = Instructions.PageSetup;
	const swInstructionsPageUnits PageUnits = static_cast<swInstructionsPageUnits>(qBound(0, Instructions.PageUnits, static_cast<int>(swInstructionsPageUnits::Inches)));
	const swInstructionsPageUnits MarginUnits = static_cast<swInstructionsPageUnits>(qBound(0, Instructions.MarginUnits, static_cast<int>(swInstructionsPageUnits::Inches)));

	auto FormatLength = [this](float Points, swInstructionsPageUnits Units)
	{
		return locale().toString(Points / swGetPageUnitScale(Units), 'g', 6);
	};

	const QString Orientation = PageSetup.Width > PageSetup.Height ? tr("Landscape") : tr("Portrait");
	QString Margins = FormatLength(PageSetup.MarginLeft, MarginUnits);

	if (PageSetup.MarginRight != PageSetup.MarginLeft || PageSetup.MarginTop != PageSetup.MarginLeft || PageSetup.MarginBottom != PageSetup.MarginLeft)
		Margins = tr("%1, %2, %3, %4").arg(Margins, FormatLength(PageSetup.MarginRight, MarginUnits), FormatLength(PageSetup.MarginTop, MarginUnits), FormatLength(PageSetup.MarginBottom, MarginUnits));

	mInstructionsPageButton->setText(tr("%1 x %2 %3 %4, margins %5 %6...").arg(FormatLength(PageSetup.Width, PageUnits), FormatLength(PageSetup.Height, PageUnits), swGetPageUnitSuffix(PageUnits), Orientation, Margins, swGetPageUnitSuffix(MarginUnits)));

	QSignalBlocker Blocker(mInstructionsGridSize);
	swSetupLengthSpinBox(mInstructionsGridSize, MarginUnits, SW_GRID_SIZE_MINIMUM, SW_GRID_SIZE_MAXIMUM);
	mInstructionsGridSize->setValue(Instructions.GridSize / swGetPageUnitScale(MarginUnits));
}
