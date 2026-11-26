#include "sw_global.h"
#include "sw_dialog_instructions_properties.h"
#include "sw_dialog_instructions_page.h"
#include "sw_dialog_setup_page.h"
#include "sw_project.h"
#include "sw_model.h"
#include "sw_widget_collapsible.h"
#include "sw_window_utilities.h"

// use to show a length in page units
void swSetupLengthSpinBox(QDoubleSpinBox* SpinBox, swInstructionsPageUnits Units, double Minimum, double Maximum)
{
	const double Scale = swGetPageUnitScale(Units);
	int Decimals = 0;
	double Step = 1.0;

	switch (Units)
	{
	case swInstructionsPageUnits::Pixels:
		break;

	case swInstructionsPageUnits::Millimeters:
		Decimals = 1;
		break;

	case swInstructionsPageUnits::Centimeters:
		Decimals = 2;
		Step = 0.1;
		break;

	case swInstructionsPageUnits::Inches:
		Decimals = 3;
		Step = 0.125;
		break;
	}

	// short ranges step by one point
	if (Step * 10.0 > (Maximum - Minimum) / Scale)
		Step = 1.0 / Scale;

	// skip unchanged settings
	if (SpinBox->decimals() != Decimals)
		SpinBox->setDecimals(Decimals);

	if (!qFuzzyCompare(SpinBox->singleStep(), Step))
		SpinBox->setSingleStep(Step);

	if (!qFuzzyCompare(SpinBox->minimum() + 1.0, Minimum / Scale + 1.0) || !qFuzzyCompare(SpinBox->maximum() + 1.0, Maximum / Scale + 1.0))
		SpinBox->setRange(Minimum / Scale, Maximum / Scale);

	// maximum follows the value
	const QString Suffix = QString(" / %1").arg(SpinBox->textFromValue(SpinBox->maximum()));

	if (SpinBox->suffix() != Suffix)
	{
		SpinBox->setSuffix(Suffix);
		swUpdatePanelSpinBoxWidth(SpinBox);
	}
}

swInstructionsUnitButton::swInstructionsUnitButton(QWidget* Parent, const QString& Name)
	: QToolButton(Parent)
{
	setToolTip(tr("Units of the value, press to show it in other units than the page units."));
	setWhatsThis(toolTip());
	setAccessibleName(tr("%1 Units").arg(Name));
	setToolButtonStyle(Qt::ToolButtonTextOnly);
	swSetupPanelButton(this);

	// square button like icon buttons
	setFixedWidth(height());

	connect(this, &QToolButton::clicked, this, &swInstructionsUnitButton::ShowUnitsMenu);
}

swInstructionsPageUnits swInstructionsUnitButton::GetUnits(const swInstructions* Instructions) const
{
	// pick the units of a value
	const std::optional<swInstructionsPageUnits> DocumentUnits = Instructions->GetDocumentUnits();

	if (DocumentUnits)
		return *DocumentUnits;

	return mUnits ? *mUnits : Instructions->GetPageUnits();
}

void swInstructionsUnitButton::UpdateUnits(const swInstructions* Instructions)
{
	setText(swGetPageUnitSuffix(GetUnits(Instructions)));
	setEnabled(!Instructions->GetDocumentUnits());
}

void swInstructionsUnitButton::ShowUnitsMenu()
{
	QMenu Menu(this);
	QActionGroup* Group = new QActionGroup(&Menu);

	QAction* PageAction = Menu.addAction(tr("Page Units"));
	PageAction->setCheckable(true);
	PageAction->setChecked(!mUnits);
	Group->addAction(PageAction);

	Menu.addSeparator();

	const std::pair<swInstructionsPageUnits, QString> UnitNames[] =
	{
		{ swInstructionsPageUnits::Pixels, tr("Pixels") },
		{ swInstructionsPageUnits::Millimeters, tr("Millimeters") },
		{ swInstructionsPageUnits::Centimeters, tr("Centimeters") },
		{ swInstructionsPageUnits::Inches, tr("Inches") }
	};

	for (const std::pair<swInstructionsPageUnits, QString>& UnitName : UnitNames)
	{
		QAction* Action = Menu.addAction(UnitName.second);
		Action->setCheckable(true);
		Action->setChecked(mUnits && *mUnits == UnitName.first);
		Action->setData(static_cast<int>(UnitName.first));
		Group->addAction(Action);
	}

	QAction* Chosen = Menu.exec(mapToGlobal(rect().bottomLeft()));

	if (!Chosen)
		return;

	if (Chosen == PageAction)
		mUnits.reset();
	else
		mUnits = static_cast<swInstructionsPageUnits>(Chosen->data().toInt());

	emit UnitsChanged();
}


swInstructionsPropertiesWidget::swInstructionsPropertiesWidget(QWidget* Parent, swInstructions* Instructions)
	: QDockWidget(Parent), mInstructions(Instructions)
{
	setWindowTitle(tr("Properties"));
	setWhatsThis(tr("<p><b>Properties</b></p><p>Edit how the instruction pages look. Every section starts folded, select an element on the page to open its section and fold the others. Right-click the page to add notices and pointers. Changes apply to all pages.</p>"));
	swSetupWidgetHelp(this);

	QWidget* CentralWidget = new QWidget(this);
	setWidget(CentralWidget);

	QVBoxLayout* Layout = new QVBoxLayout(CentralWidget);

	mOptionsRow = swCreatePanelRow(CentralWidget);
	swAddPanelRowStretch(mOptionsRow);
	Layout->addWidget(mOptionsRow);

	QMenu* OptionsMenu = new QMenu(this);
	swAddPanelOptionsButton(mOptionsRow, OptionsMenu, tr("Properties options"), tr("Properties Options"));

	QAction* PageSetupAction = OptionsMenu->addAction(tr("Page Setup..."));
	PageSetupAction->setToolTip(tr("Choose the page size, orientation and margins"));
	connect(PageSetupAction, &QAction::triggered, this, &swInstructionsPropertiesWidget::ShowPageSetupDialog);

	QAction* DefaultPageSetupAction = OptionsMenu->addAction(tr("Default Page Setup"));
	DefaultPageSetupAction->setToolTip(tr("Load the default page size, orientation, margins, grid and units of the preferences"));
	connect(DefaultPageSetupAction, &QAction::triggered, mInstructions, &swInstructions::ResetPageSetup);

	QAction* ShowMarginsAction = OptionsMenu->addAction(tr("Show Margins"));
	ShowMarginsAction->setToolTip(tr("Outline the page margins on screen, they are never printed"));
	ShowMarginsAction->setCheckable(true);
	ShowMarginsAction->setChecked(mInstructions->IsMarginsShown());
	connect(ShowMarginsAction, &QAction::toggled, mInstructions, &swInstructions::SetShowMargins);

	// inspect the element boxes on screen
	using swShownGetter = bool (swInstructions::*)() const;
	using swShownSetter = void (swInstructions::*)(bool);

	struct swShowOption
	{
		QString Text;
		QString ToolTip;
		swShownGetter Getter;
		swShownSetter Setter;
	};

	const swShowOption ShowOptions[] =
	{
		{ tr("Show Rulers"), tr("Show rulers in the page units along the page, their arrows follow the pointer"), &swInstructions::IsRulersShown, &swInstructions::SetShowRulers },
		{ tr("Show Element Margins"), tr("Fill the margins around each element on screen, they are never printed"), &swInstructions::IsElementMarginsShown, &swInstructions::SetShowElementMargins },
		{ tr("Show Element Padding"), tr("Fill the padding inside the border of each element on screen, it is never printed"), &swInstructions::IsElementPaddingShown, &swInstructions::SetShowElementPadding },
		{ tr("Show Element Anchors"), tr("Mark the corners, edge centers and center of each element on screen, they are never printed"), &swInstructions::IsElementAnchorsShown, &swInstructions::SetShowElementAnchors }
	};

	std::vector<std::pair<QAction*, swShownGetter>> ShowActions;

	for (const swShowOption& ShowOption : ShowOptions)
	{
		QAction* Action = OptionsMenu->addAction(ShowOption.Text);
		Action->setToolTip(ShowOption.ToolTip);
		Action->setCheckable(true);
		Action->setChecked((mInstructions->*ShowOption.Getter)());

		const swShownSetter Setter = ShowOption.Setter;

		connect(Action, &QAction::toggled, mInstructions, [this, Setter](bool Checked)
		{
			(mInstructions->*Setter)(Checked);
		});

		ShowActions.emplace_back(Action, ShowOption.Getter);
	}

	// thin lines on screen only
	QMenu* BoundariesMenu = OptionsMenu->addMenu(tr("Show Boundaries"));
	QActionGroup* BoundariesGroup = new QActionGroup(BoundariesMenu);

	const std::pair<swInstructionsBoundaries, QString> BoundaryNames[] =
	{
		{ swInstructionsBoundaries::Off, tr("No Element Edges") },
		{ swInstructionsBoundaries::OuterEdge, tr("Outer Edge") },
		{ swInstructionsBoundaries::MarginPadding, tr("Margin and Padding") },
		{ swInstructionsBoundaries::EveryEdge, tr("Every Edge") }
	};

	for (const std::pair<swInstructionsBoundaries, QString>& BoundaryName : BoundaryNames)
	{
		QAction* Action = BoundariesMenu->addAction(BoundaryName.second);
		Action->setCheckable(true);
		Action->setData(static_cast<int>(BoundaryName.first));
		BoundariesGroup->addAction(Action);
	}

	BoundariesMenu->actions().value(1)->setToolTip(tr("One red line around each element, margin included"));
	BoundariesMenu->actions().value(2)->setToolTip(tr("A red line at the margin edge and a dashed one at the padding edge of each element"));
	BoundariesMenu->actions().value(3)->setToolTip(tr("Red lines at the margin, border and padding edges of each element"));

	connect(BoundariesGroup, &QActionGroup::triggered, mInstructions, [this](QAction* Action)
	{
		mInstructions->SetElementBoundaries(static_cast<swInstructionsBoundaries>(Action->data().toInt()));
	});

	BoundariesMenu->addSeparator();

	QAction* LayoutAreasAction = BoundariesMenu->addAction(tr("Layout Areas"));
	LayoutAreasAction->setToolTip(tr("Blue lines around the content area, step areas, model areas, parts strips and dividers"));
	LayoutAreasAction->setCheckable(true);
	connect(LayoutAreasAction, &QAction::toggled, mInstructions, &swInstructions::SetShowLayoutAreas);

	connect(BoundariesMenu, &QMenu::aboutToShow, this, [this, BoundariesGroup, LayoutAreasAction]()
	{
		QSignalBlocker Blocker(LayoutAreasAction);
		LayoutAreasAction->setChecked(mInstructions->IsLayoutAreasShown());

		for (QAction* Action : BoundariesGroup->actions())
			Action->setChecked(Action->data().toInt() == static_cast<int>(mInstructions->GetElementBoundaries()));
	});

	OptionsMenu->addSeparator();

	// document units replace value units
	QAction* DocumentUnitsAction = OptionsMenu->addAction(tr("Use Document Units"));
	DocumentUnitsAction->setToolTip(tr("Show every value in the document units, the unit buttons of the values are disabled"));
	DocumentUnitsAction->setCheckable(true);
	connect(DocumentUnitsAction, &QAction::toggled, this, [this](bool Checked)
	{
		mInstructions->SetDefaultBool(swInstructionsPropertyType::UseDocumentUnits, Checked);
	});

	QMenu* DocumentUnitsMenu = OptionsMenu->addMenu(tr("Document Units"));
	QActionGroup* DocumentUnitsGroup = new QActionGroup(DocumentUnitsMenu);

	const std::pair<swInstructionsPageUnits, QString> UnitNames[] =
	{
		{ swInstructionsPageUnits::Pixels, tr("Pixels") },
		{ swInstructionsPageUnits::Millimeters, tr("Millimeters") },
		{ swInstructionsPageUnits::Centimeters, tr("Centimeters") },
		{ swInstructionsPageUnits::Inches, tr("Inches") }
	};

	for (const std::pair<swInstructionsPageUnits, QString>& UnitName : UnitNames)
	{
		QAction* Action = DocumentUnitsMenu->addAction(UnitName.second);
		Action->setCheckable(true);
		Action->setData(static_cast<int>(UnitName.first));
		DocumentUnitsGroup->addAction(Action);
	}

	connect(DocumentUnitsGroup, &QActionGroup::triggered, this, [this](QAction* Action)
	{
		mInstructions->SetDefaultInt(swInstructionsPropertyType::DocumentUnits, Action->data().toInt());
	});

	connect(OptionsMenu, &QMenu::aboutToShow, this, [this, ShowMarginsAction, ShowActions, DocumentUnitsAction, DocumentUnitsMenu, DocumentUnitsGroup]()
	{
		const bool UseDocumentUnits = mInstructions->GetBoolProperty(swInstructionsPropertyType::UseDocumentUnits, nullptr, 0);
		const int Units = mInstructions->GetIntProperty(swInstructionsPropertyType::DocumentUnits, nullptr, 0);
		QSignalBlocker Blocker(DocumentUnitsAction);
		QSignalBlocker MarginsBlocker(ShowMarginsAction);

		ShowMarginsAction->setChecked(mInstructions->IsMarginsShown());

		for (const std::pair<QAction*, swShownGetter>& ShowAction : ShowActions)
		{
			QSignalBlocker ShowBlocker(ShowAction.first);
			ShowAction.first->setChecked((mInstructions->*ShowAction.second)());
		}

		DocumentUnitsAction->setChecked(UseDocumentUnits);
		DocumentUnitsMenu->setEnabled(UseDocumentUnits);

		for (QAction* Action : DocumentUnitsGroup->actions())
			Action->setChecked(Action->data().toInt() == Units);
	});

	connect(DocumentUnitsAction, &QAction::toggled, DocumentUnitsMenu, &QMenu::setEnabled);

	OptionsMenu->addSeparator();

	QAction* ExpandAction = OptionsMenu->addAction(tr("Expand All Sections"));
	ExpandAction->setToolTip(tr("Show the settings of every section"));
	connect(ExpandAction, &QAction::triggered, this, [this]()
	{
		SetCategoriesExpanded(true);
	});

	QAction* CollapseAction = OptionsMenu->addAction(tr("Collapse All Sections"));
	CollapseAction->setToolTip(tr("Hide the settings of every section"));
	connect(CollapseAction, &QAction::triggered, this, [this]()
	{
		SetCategoriesExpanded(false);
	});

	OptionsMenu->addSeparator();

	// templates carry the look to other models
	QAction* TemplateCodesAction = OptionsMenu->addAction(tr("Save Template with Codes"));
	TemplateCodesAction->setToolTip(tr("Save Template turns the model's own title, author, web address and other details into codes like %title, and its image paths into paths from the model folder, so the template fits any model"));
	TemplateCodesAction->setCheckable(true);
	TemplateCodesAction->setChecked(swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_TEMPLATE_CODES));
	connect(TemplateCodesAction, &QAction::toggled, this, [](bool Checked) { swSetProfileInteger(SW_PROFILE_INSTRUCTIONS_TEMPLATE_CODES, Checked); });

	QAction* SaveTemplateAction = OptionsMenu->addAction(tr("Save Template..."));
	SaveTemplateAction->setToolTip(tr("Save everything laid out in these instructions to a model file that any model can apply"));
	connect(SaveTemplateAction, &QAction::triggered, this, [this]()
	{
		const QString FileName = QFileDialog::getSaveFileName(this, tr("Save Template"), QString(), tr("LDraw Files (*.ldr);;All Files (*)"));

		if (!FileName.isEmpty() && !mInstructions->SaveTemplate(FileName, swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_TEMPLATE_CODES)))
			QMessageBox::warning(this, tr("Save Template"), tr("Could not save the template '%1'.").arg(QDir::toNativeSeparators(FileName)));
	});

	QAction* ApplyTemplateAction = OptionsMenu->addAction(tr("Apply Template..."));
	ApplyTemplateAction->setToolTip(tr("Apply everything laid out in a template or in another model to these instructions"));
	connect(ApplyTemplateAction, &QAction::triggered, this, [this]()
	{
		const QString FileName = QFileDialog::getOpenFileName(this, tr("Apply Template"), QString(), tr("LDraw Files (*.ldr *.dat *.mpd);;All Files (*)"));

		if (!FileName.isEmpty() && !mInstructions->ApplyTemplate(FileName))
			QMessageBox::warning(this, tr("Apply Template"), tr("The file '%1' holds no instruction settings.").arg(QDir::toNativeSeparators(FileName)));
	});

	QAction* ResetAction = OptionsMenu->addAction(tr("Reset Properties"));
	ResetAction->setToolTip(tr("Restore the default look of the instructions"));
	connect(ResetAction, &QAction::triggered, mInstructions, &swInstructions::ResetProperties);

	// scroll tall sections
	mScrollArea = new QScrollArea(CentralWidget);
	mScrollArea->setWidgetResizable(true);
	mScrollArea->setFrameShape(QFrame::NoFrame);
	mScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	mScrollArea->setProperty("swKeepWidth", true);
	Layout->addWidget(mScrollArea);

	QWidget* CategoriesWidget = new QWidget(mScrollArea);
	mCategoriesLayout = new QVBoxLayout(CategoriesWidget);
	mCategoriesLayout->setContentsMargins(0, 0, 0, 0);
	mScrollArea->setWidget(CategoriesWidget);

	CreateWidgets();
	FitWidth();
	SetBoxElement(mBoxElement);
	mCategoriesLayout->addStretch(1);

	connect(mInstructions, &swInstructions::StepSettingsChanged, this, &swInstructionsPropertiesWidget::UpdateEditors);
	connect(mInstructions, &swInstructions::UnitsChanged, this, &swInstructionsPropertiesWidget::UpdateEditors);
	connect(mInstructions, &swInstructions::PageLayoutChanged, this, &swInstructionsPropertiesWidget::UpdateWidget);
	connect(mInstructions, &swInstructions::PageSetupChanged, this, &swInstructionsPropertiesWidget::UpdateWidget);

	UpdateWidget();
}

void swInstructionsPropertiesWidget::ShowPageSetupDialog()
{
	swInstructionsPageSetup PageSetup = mInstructions->mPageSetup;
	swPageSetupDialog Dialog(this, &PageSetup);

	if (Dialog.exec() != QDialog::Accepted)
		return;

	mInstructions->SetPageSetup(PageSetup);
}

// split page presets
// the button is the page, only its dividers show
class swDividerPresetButton : public QToolButton
{
public:
	swDividerPresetButton(int Rows, int Columns, QWidget* Parent)
		: QToolButton(Parent), mRows(Rows), mColumns(Columns)
	{
	}

protected:
	void paintEvent(QPaintEvent* Event) override
	{
		QToolButton::paintEvent(Event);

		QPainter Painter(this);
		const QRectF PageRect = QRectF(rect()).adjusted(1.0, 1.0, -1.0, -1.0);

		// line ends keep clear of the border

		const qreal Gap = 5.0;
		QColor LineColor = palette().color(QPalette::WindowText);

		if (!isEnabled())
			LineColor.setAlpha(110);

		Painter.setPen(QPen(LineColor, 2.0, Qt::SolidLine, Qt::FlatCap));

		// a single step shows a circle

		if (mRows == 1 && mColumns == 1)
		{
			const qreal Radius = PageRect.height() / 2.0 - Gap;

			Painter.setRenderHint(QPainter::Antialiasing);
			Painter.drawEllipse(PageRect.center(), Radius, Radius);
		}

		for (int Row = 1; Row < mRows; Row++)
		{
			const qreal Y = qRound(PageRect.top() + PageRect.height() * Row / mRows);
			Painter.drawLine(QPointF(PageRect.left() + Gap, Y), QPointF(PageRect.right() - Gap, Y));
		}

		for (int Column = 1; Column < mColumns; Column++)
		{
			const qreal X = qRound(PageRect.left() + PageRect.width() * Column / mColumns);
			Painter.drawLine(QPointF(X, PageRect.top() + Gap), QPointF(X, PageRect.bottom() - Gap));
		}
	}

	int mRows;
	int mColumns;
};

struct swDividerPreset
{
	int Rows;
	int Columns;
	const char* Name;
	const char* ToolTip;
};

static const swDividerPreset swDividerPresets[] =
{
	{ 1, 1, QT_TRANSLATE_NOOP("swInstructionsPropertiesWidget", "Full Page"), QT_TRANSLATE_NOOP("swInstructionsPropertiesWidget", "Give the page a single step, without dividers.") },
	{ 2, 1, QT_TRANSLATE_NOOP("swInstructionsPropertiesWidget", "Horizontal Halves"), QT_TRANSLATE_NOOP("swInstructionsPropertiesWidget", "Split the page in two rows with a horizontal divider.") },
	{ 3, 1, QT_TRANSLATE_NOOP("swInstructionsPropertiesWidget", "Horizontal Thirds"), QT_TRANSLATE_NOOP("swInstructionsPropertiesWidget", "Split the page in three rows with horizontal dividers.") },
	{ 4, 1, QT_TRANSLATE_NOOP("swInstructionsPropertiesWidget", "Horizontal Quarters"), QT_TRANSLATE_NOOP("swInstructionsPropertiesWidget", "Split the page in four rows with horizontal dividers.") },
	{ 1, 2, QT_TRANSLATE_NOOP("swInstructionsPropertiesWidget", "Vertical Halves"), QT_TRANSLATE_NOOP("swInstructionsPropertiesWidget", "Split the page in two columns with a vertical divider.") },
	{ 1, 3, QT_TRANSLATE_NOOP("swInstructionsPropertiesWidget", "Vertical Thirds"), QT_TRANSLATE_NOOP("swInstructionsPropertiesWidget", "Split the page in three columns with vertical dividers.") },
	{ 1, 4, QT_TRANSLATE_NOOP("swInstructionsPropertiesWidget", "Vertical Quarters"), QT_TRANSLATE_NOOP("swInstructionsPropertiesWidget", "Split the page in four columns with vertical dividers.") },
	{ 2, 2, QT_TRANSLATE_NOOP("swInstructionsPropertiesWidget", "Quarters"), QT_TRANSLATE_NOOP("swInstructionsPropertiesWidget", "Split the page in two rows and two columns with dividers both ways.") }
};

void swInstructionsPropertiesWidget::CreateWidgets()
{
	AddCategory(CategoryIndex::Document, tr("Document"), tr("<p><b>Document</b></p><p>One typeface and one font size for all text of the instructions, each usable on its own, and the projection of the step and cover models. The instructions settings, notices and pointers are saved in the model file, the page setup and grid in the preferences.</p>"));

	AddRow(CategoryIndex::Document, {
		CreateToggleEditor(swInstructionsPropertyType::UseDocumentFont, tr("Use Document Font"), tr("Use the document typeface for all text in the instructions, each text keeps its size.")),
		CreateToggleEditor(swInstructionsPropertyType::UseDocumentFontSize, tr("Use Document Font Size"), tr("Use the document font size for all text in the instructions, each text keeps its typeface."))
	});

	CreateFontEditor(swInstructionsPropertyType::DocumentFont, tr("Document Font"), tr("Typeface and size of all text while the document font or its size is used."));
	AddEditorRow(CategoryIndex::Document, swInstructionsPropertyType::DocumentFont);

	CreateChoiceEditor(swInstructionsPropertyType::ModelProjection, tr("Model Projection"), tr("Show the step and cover models in perspective or orthographic, whatever the model view shows."), { tr("Perspective"), tr("Orthographic") });
	AddEditorRow(CategoryIndex::Document, swInstructionsPropertyType::ModelProjection);

	AddRow(CategoryIndex::Document, {
		CreateToggleEditor(swInstructionsPropertyType::LockHomeView, tr("Lock to Home View"), tr("Show every model from the home view instead of the model view, a step with its own step view keeps it."))
	});

	AddRow(CategoryIndex::Document, {
		CreateToggleEditor(swInstructionsPropertyType::FadePreviousSteps, tr("Fade Previous Steps"), tr("Tint the parts added in earlier steps with the fade color."))
	});

	CreateColorEditor(swInstructionsPropertyType::FadeColor, tr("Fade Color"), tr("Color tinting the parts added in earlier steps, kept between 85 and 95 percent opaque."));
	AddEditorRow(CategoryIndex::Document, swInstructionsPropertyType::FadeColor);

	AddRow(CategoryIndex::Document, {
		CreateToggleEditor(swInstructionsPropertyType::HighlightNewParts, tr("Highlight New Parts"), tr("Outline the parts added in each step with the highlight color."))
	});

	CreateColorEditor(swInstructionsPropertyType::HighlightColor, tr("Highlight Color"), tr("Color and opacity of the outline around the parts added in each step."));
	AddEditorRow(CategoryIndex::Document, swInstructionsPropertyType::HighlightColor);
	CreateLengthEditor(swInstructionsPropertyType::HighlightWidth, tr("Highlight Width"), tr("Thickness of the outline around the parts added in each step."), 1, 72, 0.05);
	AddEditorRow(CategoryIndex::Document, swInstructionsPropertyType::HighlightWidth);

	AddCategory(CategoryIndex::Cover, tr("Cover Pages"), tr("<p><b>Cover Pages</b></p><p>The front cover shows the finished model. The final model page shows the finished model after the last step. The back cover closes the instructions and starts blank. Right click a cover to add an image, like a logo.</p>"));

	AddRow(CategoryIndex::Cover, {
		CreateToggleEditor(swInstructionsPropertyType::ShowCoverPage, tr("Show Front Cover"), tr("Start the instructions with a page showing the model title and the finished model.")),
		CreateToggleEditor(swInstructionsPropertyType::ShowBackCover, tr("Show Back Cover"), tr("End the instructions with a back cover, blank until you add images."))
	});

	AddRow(CategoryIndex::Cover, {
		CreateToggleEditor(swInstructionsPropertyType::ShowFinalModelPage, tr("Show Final Model Page"), tr("Close the steps with a page showing the finished model on its own, left out while the last page already shows it."))
	});

	AddRow(CategoryIndex::Cover, {
		CreateToggleEditor(swInstructionsPropertyType::ShowCoverBlankPage, tr("Blank Page After Cover"), tr("Follow the cover with a blank page, like the inside of a printed cover."))
	});


	AddRow(CategoryIndex::Cover, {
		CreateToggleEditor(swInstructionsPropertyType::ShowCoverImage, tr("Show Model Image"), tr("Show the finished model on the cover, unchecked leaves room for your own image."))
	});

	CreateTextEditor(swInstructionsPropertyType::PartCountText, tr("Part Count Text"), tr("Text of the part counts, %1 stands for the number of parts, empty for the default text."), false);
	AddEditorRow(CategoryIndex::Cover, swInstructionsPropertyType::PartCountText);
	CreateTextEditor(swInstructionsPropertyType::PartCountFile, tr("Part Count File"), tr("Text file holding the part count text, used instead of the typed text, relative to the model file."), true);
	AddEditorRow(CategoryIndex::Cover, swInstructionsPropertyType::PartCountFile);
	CreateFontEditor(swInstructionsPropertyType::PartCountFont, tr("Part Count Font"), tr("Font of the part count on the bill of materials, where it takes the size of the Parts List title."));
	AddEditorRow(CategoryIndex::Cover, swInstructionsPropertyType::PartCountFont);
	CreateColorEditor(swInstructionsPropertyType::PartCountColor, tr("Part Count Color"), tr("Color of the part count on the bill of materials."));
	AddEditorRow(CategoryIndex::Cover, swInstructionsPropertyType::PartCountColor);
	CreateChoiceEditor(swInstructionsPropertyType::PartCountAlignment, tr("Part Count Alignment"), tr("Line up the part count on the bill of materials left, centered or right inside its box."), { tr("Left"), tr("Center"), tr("Right") });
	AddEditorRow(CategoryIndex::Cover, swInstructionsPropertyType::PartCountAlignment);
	CreateChoiceEditor(swInstructionsPropertyType::PartCountVerticalAlignment, tr("Part Count Vertical Alignment"), tr("Line up the part count on the bill of materials at the top, middle or bottom of a box with Edit Size."), { tr("Top"), tr("Middle"), tr("Bottom") });
	AddEditorRow(CategoryIndex::Cover, swInstructionsPropertyType::PartCountVerticalAlignment);

	AddCategory(CategoryIndex::Publication, tr("Publication"), tr("<p><b>Publication</b></p><p>The title, author and publication details of the model, each shown on the front or back cover. Empty texts show the model properties of the same name, edited in Model Properties. Texts can use %title, %author, %id, %version, %date, %url, %contact, %copyright, %notice, %size, %weight and %comments.</p>"));

	// the editors below follow the chosen cover
	mPublicationCoverComboBox = new QComboBox(this);
	mPublicationCoverComboBox->addItems({ tr("Front Cover"), tr("Back Cover") });
	mPublicationCoverComboBox->setToolTip(tr("Choose the cover whose elements are edited."));
	mPublicationCoverComboBox->setWhatsThis(mPublicationCoverComboBox->toolTip());
	mPublicationCoverComboBox->setAccessibleName(tr("Cover"));
	swSetupPanelEdit(mPublicationCoverComboBox);
	AddLabeledRow(CategoryIndex::Publication, tr("Cover"), mPublicationCoverComboBox);

	connect(mPublicationCoverComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int Index)
	{
		SetPublicationCover(Index == 1);
	});

	// one group of editors per element and cover
	for (const swInstructionsPublicationElement& Publication : swGetPublicationElements())
	{
		const QString Name = swGetPublicationName(Publication);
		const QString Kind = swGetPublicationKindName(Publication);
		const auto GetType = [&Publication](swInstructionsPublicationField Field)
		{
			return swGetPublicationType(Publication, Field);
		};

		std::vector<QWidget*>& Rows = mPublicationRows[Publication.Back ? 1 : 0];

		Rows.push_back(AddRow(CategoryIndex::Publication, {
			CreateToggleEditor(Publication.Show, tr("Show %1").arg(Kind), tr("Show the %1 of the model on the cover.").arg(Kind))
		}));

		CreateTextEditor(GetType(swInstructionsPublicationField::Text), Kind, tr("%1 on the cover, empty for the one of the model properties.").arg(Kind), false);
		Rows.push_back(AddEditorRow(CategoryIndex::Publication, GetType(swInstructionsPublicationField::Text))->parentWidget());
		CreateFontEditor(GetType(swInstructionsPublicationField::Font), tr("%1 Font").arg(Kind), tr("Font of the %1.").arg(Kind));
		Rows.push_back(AddEditorRow(CategoryIndex::Publication, GetType(swInstructionsPublicationField::Font))->parentWidget());
		CreateColorEditor(GetType(swInstructionsPublicationField::Color), tr("%1 Color").arg(Kind), tr("Color of the %1.").arg(Kind));
		Rows.push_back(AddEditorRow(CategoryIndex::Publication, GetType(swInstructionsPublicationField::Color))->parentWidget());
		CreateChoiceEditor(GetType(swInstructionsPublicationField::Alignment), tr("%1 Alignment").arg(Kind), tr("Line up the %1 left, centered or right inside its box.").arg(Kind), { tr("Left"), tr("Center"), tr("Right") });
		Rows.push_back(AddEditorRow(CategoryIndex::Publication, GetType(swInstructionsPublicationField::Alignment))->parentWidget());
		CreateChoiceEditor(GetType(swInstructionsPublicationField::VerticalAlignment), tr("%1 Vertical Alignment").arg(Kind), tr("Line up the %1 at the top, middle or bottom of a box with Edit Size.").arg(Kind), { tr("Top"), tr("Middle"), tr("Bottom") });
		Rows.push_back(AddEditorRow(CategoryIndex::Publication, GetType(swInstructionsPublicationField::VerticalAlignment))->parentWidget());
	}

	SetPublicationCover(false);

	AddCategory(CategoryIndex::Page, tr("Page"), tr("<p><b>Page</b></p><p>The background and the number of the step and blank pages. The background applies to all these pages, or only to the current page when it has its own, the cover and the bill of materials have their own backgrounds here too. The cover and blank pages are not numbered unless all pages are. Drag the number or use the Placement panel to move it on all pages.</p>"));

	CreateColorEditor(swInstructionsPropertyType::CoverBackgroundColor, tr("Cover Background"), tr("Color filling the whole cover page, margins included."));
	AddEditorRow(CategoryIndex::Page, swInstructionsPropertyType::CoverBackgroundColor);
	CreateColorEditor(swInstructionsPropertyType::BlankPageBackgroundColor, tr("Blank Page Background"), tr("Color filling the whole of every blank page, margins included."));
	AddEditorRow(CategoryIndex::Page, swInstructionsPropertyType::BlankPageBackgroundColor);
	CreateColorEditor(swInstructionsPropertyType::SubmodelBackgroundColor, tr("Submodel Background"), tr("Color filling the whole page of submodel steps, margins included."));
	AddEditorRow(CategoryIndex::Page, swInstructionsPropertyType::SubmodelBackgroundColor);
	CreateColorEditor(swInstructionsPropertyType::AssemblyBackgroundColor, tr("Assembly Background"), tr("Color filling the whole page where two or more models come together, margins included, clear to follow the page."));
	AddEditorRow(CategoryIndex::Page, swInstructionsPropertyType::AssemblyBackgroundColor);
	CreateColorEditor(swInstructionsPropertyType::PageBackgroundColor, tr("Page Background"), tr("Color filling the whole page of main model steps, margins included."));
	AddEditorRow(CategoryIndex::Page, swInstructionsPropertyType::PageBackgroundColor);
	CreateColorEditor(swInstructionsPropertyType::CompletionBackgroundColor, tr("Completion Background"), tr("Color behind the finished main model, filling its own page or only its box on a steps page, clear to follow the page, a Completion box background goes first."));
	AddEditorRow(CategoryIndex::Page, swInstructionsPropertyType::CompletionBackgroundColor);
	CreateColorEditor(swInstructionsPropertyType::BillOfMaterialsBackgroundColor, tr("BOM Background"), tr("Color filling the whole bill of materials page, margins included."));
	AddEditorRow(CategoryIndex::Page, swInstructionsPropertyType::BillOfMaterialsBackgroundColor);

	// background for all or current page
	mBackgroundPageButton = new QToolButton(this);
	mBackgroundPageButton->setText(tr("Background on This Page Only"));
	mBackgroundPageButton->setCheckable(true);
	mBackgroundPageButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
	mBackgroundPageButton->setToolTip(tr("Give this page its own background, unchecked it follows all pages."));
	mBackgroundPageButton->setWhatsThis(mBackgroundPageButton->toolTip());
	mBackgroundPageButton->setAccessibleName(mBackgroundPageButton->text());
	swSetupPanelButton(mBackgroundPageButton);

	connect(mBackgroundPageButton, &QToolButton::toggled, this, [this](bool PageOnly)
	{
		mInstructions->SetPageScope(mPageIndex, swInstructionsPropertyType::PageBackgroundColor, PageOnly);
	});

	AddRow(CategoryIndex::Page, { mBackgroundPageButton });

	QToolButton* BackgroundApplyAllButton = CreateApplyAllButton(tr("Use the background of this page on every page."));

	connect(BackgroundApplyAllButton, &QToolButton::clicked, this, [this]()
	{
		mInstructions->ApplyPageToAllPages(mPageIndex, swInstructionsPropertyType::PageBackgroundColor);
	});

	AddRow(CategoryIndex::Page, { BackgroundApplyAllButton });

	AddRow(CategoryIndex::Page, {
		CreateToggleEditor(swInstructionsPropertyType::ShowPageNumber, tr("Show Page Number"), tr("Show the number of each page except the cover and blank pages.")),
		CreateToggleEditor(swInstructionsPropertyType::ShowPageNumberAllPages, tr("Number All Pages"), tr("Number every page, the cover and blank pages included."))
	});

	// choices follow the number formats
	CreateChoiceEditor(swInstructionsPropertyType::PageNumberFormat, tr("Page Number Format"), tr("Show the page number alone or with the page count."), { tr("Page Number"), tr("Page Number / Page Count") });
	AddEditorRow(CategoryIndex::Page, swInstructionsPropertyType::PageNumberFormat);
	CreateFontEditor(swInstructionsPropertyType::PageNumberFont, tr("Page Number Font"), tr("Font of the page numbers."));
	AddEditorRow(CategoryIndex::Page, swInstructionsPropertyType::PageNumberFont);
	CreateColorEditor(swInstructionsPropertyType::PageNumberColor, tr("Page Number Color"), tr("Color of the page numbers."));
	AddEditorRow(CategoryIndex::Page, swInstructionsPropertyType::PageNumberColor);
	CreateChoiceEditor(swInstructionsPropertyType::PageNumberAlignment, tr("Page Number Alignment"), tr("Line up the page numbers left, centered or right inside its box."), { tr("Left"), tr("Center"), tr("Right") });
	AddEditorRow(CategoryIndex::Page, swInstructionsPropertyType::PageNumberAlignment);
	CreateChoiceEditor(swInstructionsPropertyType::PageNumberVerticalAlignment, tr("Page Number Vertical Alignment"), tr("Line up the page numbers at the top, middle or bottom of a box with Edit Size."), { tr("Top"), tr("Middle"), tr("Bottom") });
	AddEditorRow(CategoryIndex::Page, swInstructionsPropertyType::PageNumberVerticalAlignment);

	AddRow(CategoryIndex::Page, {
		CreateToggleEditor(swInstructionsPropertyType::ShowPagePartCount, tr("Show Part Count"), tr("Show the number of new parts of the steps on each step page."))
	});

	CreateFontEditor(swInstructionsPropertyType::PagePartCountFont, tr("Page Part Count Font"), tr("Font of the part counts of the step pages and submodel previews."));
	AddEditorRow(CategoryIndex::Page, swInstructionsPropertyType::PagePartCountFont);
	CreateColorEditor(swInstructionsPropertyType::PagePartCountColor, tr("Page Part Count Color"), tr("Color of the part counts of the step pages and submodel previews."));
	AddEditorRow(CategoryIndex::Page, swInstructionsPropertyType::PagePartCountColor);

	AddRow(CategoryIndex::Page, {
		CreateToggleEditor(swInstructionsPropertyType::ShowSubmodelPreview, tr("Show Submodel Preview"), tr("Show the finished submodel with its part count on the first page of each submodel."))
	});

	AddCategory(CategoryIndex::Header, tr("Header"), tr("<p><b>Header</b></p><p>A strip along the top paper edge of the step and blank pages. Right click the band on the page to add text or images, they repeat on every page. Texts can use %page, %title, %author, %id, %version, %date, %url, %contact, %copyright, %notice, %size, %weight and %comments, filled from the page and the model properties. The band can reserve its space so the page content stays clear of it. Its box, border and background are set in the Box group.</p>"));
	AddCategory(CategoryIndex::Footer, tr("Footer"), tr("<p><b>Footer</b></p><p>A strip along the bottom paper edge of the step and blank pages. Right click the band on the page to add text or images, they repeat on every page. Texts can use %page, %title, %author, %id, %version, %date, %url, %contact, %copyright, %notice, %size, %weight and %comments, filled from the page and the model properties. The band can reserve its space so the page content stays clear of it. Its box, border and background are set in the Box group.</p>"));
	AddCategory(CategoryIndex::LeftBand, tr("Left Band"), tr("<p><b>Left Band</b></p><p>A strip along the left paper edge of the step and blank pages. Right click a band on the page to add text or images, they repeat on every page. Texts can use %page, %title, %author, %id, %version, %date, %url, %contact, %copyright, %notice, %size, %weight and %comments, filled from the page and the model properties. A band can reserve its space so the page content stays clear of it. Its box, border and background are set in the Box group.</p>"));
	AddCategory(CategoryIndex::RightBand, tr("Right Band"), tr("<p><b>Right Band</b></p><p>A strip along the right paper edge of the step and blank pages. Right click the band on the page to add text or images, they repeat on every page. Texts can use %page, %title, %author, %id, %version, %date, %url, %contact, %copyright, %notice, %size, %weight and %comments, filled from the page and the model properties. The band can reserve its space so the page content stays clear of it. Its box, border and background are set in the Box group.</p>"));

	const QString BandNames[4] = { tr("Header"), tr("Footer"), tr("Left"), tr("Right") };

	for (int BandIndex = 0; BandIndex < 4; BandIndex++)
	{
		const CategoryIndex Category = static_cast<CategoryIndex>(static_cast<int>(CategoryIndex::Header) + BandIndex);
		const QString& BandName = BandNames[BandIndex];
		const auto BandType = [BandIndex](int Kind) { return static_cast<swInstructionsPropertyType>(static_cast<int>(swInstructionsPropertyType::ShowHeaderBand) + Kind * 4 + BandIndex); };

		AddRow(Category, {
			CreateToggleEditor(BandType(0), tr("Show %1 Band").arg(BandName), tr("Show the %1 band on the step and blank pages.").arg(BandName.toLower()))
		});

		CreateLengthEditor(BandType(1), tr("%1 Size").arg(BandName), tr("Thickness of the %1 band from the paper edge.").arg(BandName.toLower()), 1, 576, 0.5);
		AddEditorRow(Category, BandType(1));

		AddRow(Category, {
			CreateToggleEditor(BandType(3), tr("Reserve %1 Space").arg(BandName), tr("Keep the page content clear of the %1 band, unchecked lets the content run under it.").arg(BandName.toLower()))
		});

		// band for all or current page
		const int Section = BandIndex;
		const auto SectionTypes = [BandType, BandIndex]()
		{
			std::vector<swInstructionsPropertyType> Types;

			for (int Kind = 0; Kind < 4; Kind++)
				Types.push_back(BandType(Kind));

			// the band box follows its page scope
			const swInstructionsBoxTypes BoxTypes = swGetBoxTypes(static_cast<swInstructionsBoxElement>(static_cast<int>(swInstructionsBoxElement::HeaderBand) + BandIndex));

			for (swInstructionsPropertyType Type : { BoxTypes.Margin, BoxTypes.Padding, BoxTypes.BorderWidth, BoxTypes.CornerRadius, BoxTypes.BorderColor })
				Types.push_back(Type);

			return Types;
		};

		QToolButton* PageButton = new QToolButton(this);
		PageButton->setText(BandIndex < 2 ? tr("%1 on This Page Only").arg(BandName) : tr("%1 Band on This Page Only").arg(BandName));
		PageButton->setCheckable(true);
		PageButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
		PageButton->setToolTip(tr("Keep this band on this page only and clear it from the other pages, unchecked it follows all pages."));
		PageButton->setWhatsThis(PageButton->toolTip());
		PageButton->setAccessibleName(PageButton->text());
		swSetupPanelButton(PageButton);
		mBandsPageButtons[Section] = PageButton;

		connect(PageButton, &QToolButton::toggled, this, [this, SectionTypes](bool PageOnly)
		{
			// one redraw for all band properties
			{
				const QSignalBlocker Blocker(mInstructions);
				const std::vector<swInstructionsPropertyType> Types = SectionTypes();

				for (swInstructionsPropertyType Type : Types)
					mInstructions->SetPageScope(mPageIndex, Type, PageOnly);

				// the other pages lose this band
				if (PageOnly)
					mInstructions->SetDefaultBool(Types[0], false);
			}

			emit mInstructions->PageLayoutChanged();
		});

		AddRow(Category, { PageButton });

		QToolButton* ApplyAllButton = CreateApplyAllButton(tr("Use this band of this page on every page."));
		mBandsApplyAllButtons[Section] = ApplyAllButton;

		connect(ApplyAllButton, &QToolButton::clicked, this, [this, SectionTypes]()
		{
			// one redraw for all band properties
			{
				const QSignalBlocker Blocker(mInstructions);

				for (swInstructionsPropertyType Type : SectionTypes())
					mInstructions->ApplyPageToAllPages(mPageIndex, Type);
			}

			emit mInstructions->PageLayoutChanged();
		});

		AddRow(Category, { ApplyAllButton });
	}

	AddCategory(CategoryIndex::Watermark, tr("Watermark"), tr("<p><b>Watermark</b></p><p>Text or an image behind the page content, right click the page to add one. The text color alpha sets how strongly text shows, Image Opacity how strongly an image shows, its box, border and background are set in the Box group.</p>"));

	CreateFontEditor(swInstructionsPropertyType::WatermarkFont, tr("Watermark Font"), tr("Font of the watermark text."));
	AddEditorRow(CategoryIndex::Watermark, swInstructionsPropertyType::WatermarkFont);
	CreateColorEditor(swInstructionsPropertyType::WatermarkTextColor, tr("Watermark Text Color"), tr("Color of the watermark text, its alpha sets how strongly the text shows."));
	AddEditorRow(CategoryIndex::Watermark, swInstructionsPropertyType::WatermarkTextColor);
	QSpinBox* RotationSpinBox = CreatePercentEditor(swInstructionsPropertyType::WatermarkRotation, tr("Watermark Rotation"), tr("Turn every watermark around its center, in degrees."), -180, 180);
	RotationSpinBox->setSingleStep(15);
	mEditors[static_cast<int>(swInstructionsPropertyType::WatermarkRotation)].SymbolButton->setText(QStringLiteral("°"));
	AddEditorRow(CategoryIndex::Watermark, swInstructionsPropertyType::WatermarkRotation);
	CreatePercentEditor(swInstructionsPropertyType::WatermarkOpacity, tr("Image Opacity"), tr("How strongly the watermark image shows, 100 percent is solid."), 1, 100);
	AddEditorRow(CategoryIndex::Watermark, swInstructionsPropertyType::WatermarkOpacity);

	mWatermarkPageButton = new QToolButton(this);
	mWatermarkPageButton->setText(tr("Hide Watermark on This Page"));
	mWatermarkPageButton->setCheckable(true);
	mWatermarkPageButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
	mWatermarkPageButton->setToolTip(tr("Hide the watermark on this page only."));
	mWatermarkPageButton->setWhatsThis(mWatermarkPageButton->toolTip());
	mWatermarkPageButton->setAccessibleName(mWatermarkPageButton->text());
	swSetupPanelButton(mWatermarkPageButton);

	connect(mWatermarkPageButton, &QToolButton::toggled, this, [this](bool Hide)
	{
		mInstructions->SetPageScope(mPageIndex, swInstructionsPropertyType::ShowWatermark, Hide);

		if (Hide)
			mInstructions->SetPageInt(mPageIndex, swInstructionsPropertyType::ShowWatermark, 0);
	});

	AddRow(CategoryIndex::Watermark, { mWatermarkPageButton });

	AddCategory(CategoryIndex::BillOfMaterials, tr("Bill of Materials"), tr("<p><b>Bill of Materials</b></p><p>The last page, listing every part of the model under a Parts List title. Its title, list, quantities and part count have their own settings here and their own boxes in the Box group, so they never change the steps.</p>"));

	AddRow(CategoryIndex::BillOfMaterials, {
		CreateToggleEditor(swInstructionsPropertyType::ShowBillOfMaterials, tr("Show Bill of Materials"), tr("End the instructions with a page listing every part of the model."))
	});

	AddRow(CategoryIndex::BillOfMaterials, {
		CreateToggleEditor(swInstructionsPropertyType::ShowBillOfMaterialsTitle, tr("Show BOM Title"), tr("Show the Parts List title above the bill of materials.")),
		CreateToggleEditor(swInstructionsPropertyType::BillOfMaterialsShowCounts, tr("Show Part Counts"), tr("Show how many of each part the bill of materials needs."))
	});

	AddRow(CategoryIndex::BillOfMaterials, {
		CreateToggleEditor(swInstructionsPropertyType::BillOfMaterialsShowPartIds, tr("Show Part IDs"), tr("Print the part number under each part of the bill of materials, for easy reference."))
	});

	AddRow(CategoryIndex::BillOfMaterials, {
		CreateToggleEditor(swInstructionsPropertyType::ShowBillOfMaterialsPartCount, tr("Show Part Count"), tr("Show the number of parts of the whole model under the title."))
	});

	AddRow(CategoryIndex::BillOfMaterials, {
		CreateToggleEditor(swInstructionsPropertyType::ShowBillOfMaterialsBlankPage, tr("Blank Page Before Bill of Materials"), tr("Precede the bill of materials with a blank page."))
	});

	CreateTextEditor(swInstructionsPropertyType::BillOfMaterialsTitleText, tr("BOM Title"), tr("Title above the bill of materials, empty for Parts List."), false);
	AddEditorRow(CategoryIndex::BillOfMaterials, swInstructionsPropertyType::BillOfMaterialsTitleText);
	CreateTextEditor(swInstructionsPropertyType::BillOfMaterialsTitleFile, tr("BOM Title File"), tr("Text file holding the bill of materials title, used instead of the typed title, relative to the model file."), true);
	AddEditorRow(CategoryIndex::BillOfMaterials, swInstructionsPropertyType::BillOfMaterialsTitleFile);
	CreateFontEditor(swInstructionsPropertyType::BillOfMaterialsTitleFont, tr("BOM Title Font"), tr("Font of the Parts List title above the bill of materials."));
	AddEditorRow(CategoryIndex::BillOfMaterials, swInstructionsPropertyType::BillOfMaterialsTitleFont);
	CreateColorEditor(swInstructionsPropertyType::BillOfMaterialsTitleColor, tr("BOM Title Color"), tr("Color of the Parts List title above the bill of materials."));
	AddEditorRow(CategoryIndex::BillOfMaterials, swInstructionsPropertyType::BillOfMaterialsTitleColor);
	CreateChoiceEditor(swInstructionsPropertyType::BillOfMaterialsTitleAlignment, tr("BOM Title Alignment"), tr("Line up the Parts List title left, centered or right inside its box."), { tr("Left"), tr("Center"), tr("Right") });
	AddEditorRow(CategoryIndex::BillOfMaterials, swInstructionsPropertyType::BillOfMaterialsTitleAlignment);
	CreateChoiceEditor(swInstructionsPropertyType::BillOfMaterialsTitleVerticalAlignment, tr("BOM Title Vertical Alignment"), tr("Line up the Parts List title at the top, middle or bottom of a box with Edit Size."), { tr("Top"), tr("Middle"), tr("Bottom") });
	AddEditorRow(CategoryIndex::BillOfMaterials, swInstructionsPropertyType::BillOfMaterialsTitleVerticalAlignment);

	// choices follow the sort order
	CreateChoiceEditor(swInstructionsPropertyType::BillOfMaterialsSortOrder, tr("BOM List Order"), tr("Order of the parts in the bill of materials."), { tr("Sort by Color"), tr("Sort by Name"), tr("Sort by Part ID") });
	AddEditorRow(CategoryIndex::BillOfMaterials, swInstructionsPropertyType::BillOfMaterialsSortOrder);

	// choices follow layout and alignment
	CreateChoiceEditor(swInstructionsPropertyType::BillOfMaterialsLayout, tr("BOM List Layout"), tr("Line the parts of the bill of materials up in rows or in columns."), { tr("Rows"), tr("Columns") });
	AddEditorRow(CategoryIndex::BillOfMaterials, swInstructionsPropertyType::BillOfMaterialsLayout);
	CreateChoiceEditor(swInstructionsPropertyType::BillOfMaterialsAlignment, tr("BOM Line Reference"), tr("Thumbnail edge the parts of the bill of materials line up on."), { tr("Top"), tr("Middle"), tr("Bottom") });
	AddEditorRow(CategoryIndex::BillOfMaterials, swInstructionsPropertyType::BillOfMaterialsAlignment);
	CreateFontEditor(swInstructionsPropertyType::BillOfMaterialsFont, tr("BOM List Font"), tr("Font of the part counts and part IDs of the bill of materials."));
	AddEditorRow(CategoryIndex::BillOfMaterials, swInstructionsPropertyType::BillOfMaterialsFont);
	CreateColorEditor(swInstructionsPropertyType::BillOfMaterialsTextColor, tr("BOM List Text Color"), tr("Color of the part counts and part IDs of the bill of materials."));
	AddEditorRow(CategoryIndex::BillOfMaterials, swInstructionsPropertyType::BillOfMaterialsTextColor);
	CreateLengthEditor(swInstructionsPropertyType::BillOfMaterialsThumbnailSize, tr("BOM Thumbnail Size"), tr("Size each part of the bill of materials is drawn at, larger thumbnails show larger parts."), 32, 2048);
	AddEditorRow(CategoryIndex::BillOfMaterials, swInstructionsPropertyType::BillOfMaterialsThumbnailSize);

	AddCategory(CategoryIndex::PageLayout, tr("Steps per Page"), tr("<p><b>Steps per Page</b></p><p>The rows and columns of steps on the current page, the following pages flow on from it. The presets split the page in halves, thirds or quarters and show the step dividers. Step Flow fills the page row by row or column by column, and the last step stretches into any empty cells. Show Final Model puts the finished model in the room left after the last step of a model, a full last page already ends on it, and the separate final model page is then left out. The final model has its own box, with margin, padding, border, corner radius and background, set in the Box group, its background takes the Page Background color until you pick your own.</p>"));

	std::vector<QWidget*> PresetButtons;

	for (int Preset = 0; Preset < static_cast<int>(std::size(swDividerPresets)); Preset++)
	{
		const swDividerPreset& DividerPreset = swDividerPresets[Preset];
		const QString Name = tr(DividerPreset.Name);

		QToolButton* Button = new swDividerPresetButton(DividerPreset.Rows, DividerPreset.Columns, this);
		Button->setCheckable(true);
		Button->setToolTip(tr(DividerPreset.ToolTip));
		Button->setWhatsThis(Button->toolTip());
		Button->setAccessibleName(Name);
		swSetupPanelButton(Button);

		connect(Button, &QToolButton::clicked, this, [this, Preset]()
		{
			DividerPresetClicked(Preset);
		});

		mDividerPresetButtons.push_back(Button);
		PresetButtons.push_back(Button);
	}

	AddRow(CategoryIndex::PageLayout, PresetButtons);

	auto CreatePageLayoutSpinBox = [this](const QString& Name, const QString& ToolTip)
	{
		swRangeSpinBox* SpinBox = new swRangeSpinBox(this);
		SpinBox->setRange(1, 10);
		SpinBox->SetReference();
		SpinBox->setToolTip(ToolTip);
		SpinBox->setWhatsThis(ToolTip);
		SpinBox->setAccessibleName(Name);
		swSetupPanelSpinBox(SpinBox);

		connect(SpinBox, qOverload<int>(&QSpinBox::valueChanged), this, [this]()
		{
			mInstructions->SetPageSettings(mPageIndex, GetPageLayoutSettings());
		});

		return SpinBox;
	};

	mRowsSpinBox = CreatePageLayoutSpinBox(tr("Rows"), tr("Number of step rows on this page."));
	mColumnsSpinBox = CreatePageLayoutSpinBox(tr("Columns"), tr("Number of step columns on this page."));

	// steps fill across or down the page

	mStepFlowComboBox = new QComboBox(this);
	mStepFlowComboBox->addItems({ tr("Horizontal"), tr("Vertical") });
	mStepFlowComboBox->setToolTip(tr("Fill this page with steps row by row, or column by column."));
	mStepFlowComboBox->setWhatsThis(mStepFlowComboBox->toolTip());
	mStepFlowComboBox->setAccessibleName(tr("Step Flow"));
	swSetupPanelEdit(mStepFlowComboBox);

	connect(mStepFlowComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, [this]()
	{
		mInstructions->SetPageSettings(mPageIndex, GetPageLayoutSettings());
	});

	// the finished model fills the room left

	mFinalModelButton = new QToolButton(this);
	mFinalModelButton->setText(tr("Show Final Model"));
	mFinalModelButton->setToolTip(tr("Show the finished model in the room left on the page after the last step of a model, in place of the final model page."));
	mFinalModelButton->setWhatsThis(mFinalModelButton->toolTip());
	mFinalModelButton->setCheckable(true);
	mFinalModelButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
	swSetupPanelButton(mFinalModelButton);

	connect(mFinalModelButton, &QToolButton::toggled, this, [this]()
	{
		mInstructions->SetPageSettings(mPageIndex, GetPageLayoutSettings());
	});

	QToolButton* ApplyAllButton = CreateApplyAllButton(tr("Use this page layout on every page."));

	connect(ApplyAllButton, &QToolButton::clicked, this, [this]()
	{
		mInstructions->SetDefaultPageSettings(GetPageLayoutSettings());
	});

	AddLabeledRow(CategoryIndex::PageLayout, tr("Rows"), mRowsSpinBox);
	AddLabeledRow(CategoryIndex::PageLayout, tr("Columns"), mColumnsSpinBox);
	AddLabeledRow(CategoryIndex::PageLayout, tr("Step Flow"), mStepFlowComboBox);
	AddRow(CategoryIndex::PageLayout, { mFinalModelButton });
	AddRow(CategoryIndex::PageLayout, { ApplyAllButton });

	AddCategory(CategoryIndex::Step, tr("Step Number"), tr("<p><b>Step Number</b></p><p>The step number and the area of each step, which has its own box in the Box group.</p>"));

	AddRow(CategoryIndex::Step, {
		CreateToggleEditor(swInstructionsPropertyType::ShowStepNumber, tr("Show Step Number"), tr("Show the number of each step in its top left corner."))
	});

	CreateFontEditor(swInstructionsPropertyType::StepNumberFont, tr("Step Number Font"), tr("Font of the step numbers."));
	AddEditorRow(CategoryIndex::Step, swInstructionsPropertyType::StepNumberFont);
	CreateColorEditor(swInstructionsPropertyType::StepNumberColor, tr("Step Number Color"), tr("Color of the step numbers."));
	AddEditorRow(CategoryIndex::Step, swInstructionsPropertyType::StepNumberColor);
	CreateChoiceEditor(swInstructionsPropertyType::StepNumberAlignment, tr("Step Number Alignment"), tr("Line up the step numbers left, centered or right inside its box."), { tr("Left"), tr("Center"), tr("Right") });
	AddEditorRow(CategoryIndex::Step, swInstructionsPropertyType::StepNumberAlignment);
	CreateChoiceEditor(swInstructionsPropertyType::StepNumberVerticalAlignment, tr("Step Number Vertical Alignment"), tr("Line up the step numbers at the top, middle or bottom of a box with Edit Size."), { tr("Top"), tr("Middle"), tr("Bottom") });
	AddEditorRow(CategoryIndex::Step, swInstructionsPropertyType::StepNumberVerticalAlignment);

	AddCategory(CategoryIndex::PartsList, tr("Parts List"), tr("<p><b>Parts List</b></p><p>The list of parts added in each step. Reserve Parts Area keeps a strip of the step for it, on the side where it is placed, like a band, and the model fits in the rest. The step number shares the strip when it sits beside the list.</p>"));

	AddRow(CategoryIndex::PartsList, {
		CreateToggleEditor(swInstructionsPropertyType::ShowStepPLI, tr("Show Parts List"), tr("Show the parts added in each step next to its number.")),
		CreateToggleEditor(swInstructionsPropertyType::PLIShowCounts, tr("Show Part Counts"), tr("Show how many of each part are needed."))
	});

	AddRow(CategoryIndex::PartsList, {
		CreateToggleEditor(swInstructionsPropertyType::PLIReserveArea, tr("Reserve Parts Area"), tr("Keep a strip of the step for the parts list and the step number beside it, on the side it is placed, the model fits in the rest and never runs under them."))
	});

	// choices follow the sort order
	CreateChoiceEditor(swInstructionsPropertyType::PLISortOrder, tr("Parts List Order"), tr("Order of the parts in the list."), { tr("Sort by Color"), tr("Sort by Name"), tr("Sort by Part ID") });
	AddEditorRow(CategoryIndex::PartsList, swInstructionsPropertyType::PLISortOrder);

	// choices follow layout and alignment
	CreateChoiceEditor(swInstructionsPropertyType::PLILayout, tr("Parts List Layout"), tr("Line the parts up in rows or in columns."), { tr("Rows"), tr("Columns") });
	AddEditorRow(CategoryIndex::PartsList, swInstructionsPropertyType::PLILayout);
	CreateChoiceEditor(swInstructionsPropertyType::PLIAlignment, tr("Line Reference"), tr("Thumbnail edge the parts line up on."), { tr("Top"), tr("Middle"), tr("Bottom") });
	AddEditorRow(CategoryIndex::PartsList, swInstructionsPropertyType::PLIAlignment);
	CreateFontEditor(swInstructionsPropertyType::PLIFont, tr("Parts List Font"), tr("Font of the part counts."));
	AddEditorRow(CategoryIndex::PartsList, swInstructionsPropertyType::PLIFont);
	CreateColorEditor(swInstructionsPropertyType::PLITextColor, tr("Parts List Text Color"), tr("Color of the part counts."));
	AddEditorRow(CategoryIndex::PartsList, swInstructionsPropertyType::PLITextColor);
	CreateLengthEditor(swInstructionsPropertyType::PLIThumbnailSize, tr("Thumbnail Size"), tr("Size each part is drawn at, larger thumbnails show larger parts."), 32, 2048);
	AddEditorRow(CategoryIndex::PartsList, swInstructionsPropertyType::PLIThumbnailSize);

	// thumbnails for all or current page
	mThumbnailPageButton = new QToolButton(this);
	mThumbnailPageButton->setText(tr("Thumbnails on This Page Only"));
	mThumbnailPageButton->setCheckable(true);
	mThumbnailPageButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
	mThumbnailPageButton->setToolTip(tr("Give this page its own thumbnail size, unchecked it follows all pages."));
	mThumbnailPageButton->setWhatsThis(mThumbnailPageButton->toolTip());
	mThumbnailPageButton->setAccessibleName(mThumbnailPageButton->text());
	swSetupPanelButton(mThumbnailPageButton);

	connect(mThumbnailPageButton, &QToolButton::toggled, this, &swInstructionsPropertiesWidget::SetThumbnailPageScope);

	AddRow(CategoryIndex::PartsList, { mThumbnailPageButton });

	mThumbnailApplyAllButton = CreateApplyAllButton(tr("Use the thumbnail size of this page on every page."));

	connect(mThumbnailApplyAllButton, &QToolButton::clicked, this, [this]()
	{
		mInstructions->ApplyPageToAllPages(mPageIndex, swInstructionsPropertyType::PLIThumbnailSize);
	});

	AddRow(CategoryIndex::PartsList, { mThumbnailApplyAllButton });

	AddCategory(CategoryIndex::Divider, tr("Divider"), tr("<p><b>Divider</b></p><p>The lines between neighboring steps. Horizontal and Vertical hide the lines in one direction, for example where page colors already split the page, At Final Model hides the lines along the final model. The margin keeps the line ends away from the page margins, the padding keeps the steps away from the lines.</p>"));

	AddRow(CategoryIndex::Divider, {
		CreateToggleEditor(swInstructionsPropertyType::ShowStepDividers, tr("Show Step Dividers"), tr("Draw lines between neighboring steps.")),
		CreateToggleEditor(swInstructionsPropertyType::StepDividerRounded, tr("Rounded Ends"), tr("Round the line ends, off draws flat ends."))
	});

	AddRow(CategoryIndex::Divider, {
		CreateToggleEditor(swInstructionsPropertyType::StepDividerHorizontal, tr("Horizontal"), tr("Draw the lines between steps above and below each other.")),
		CreateToggleEditor(swInstructionsPropertyType::StepDividerVertical, tr("Vertical"), tr("Draw the lines between steps side by side."))
	});

	AddRow(CategoryIndex::Divider, {
		CreateToggleEditor(swInstructionsPropertyType::StepDividerFinalModel, tr("At Final Model"), tr("Draw the lines along the final model, off leaves them out where its box already splits the page."))
	});

	CreateLengthEditor(swInstructionsPropertyType::StepDividerMargin, tr("Divider Margin"), tr("Space between the line ends and the page margins."), 0, 144, 0.25);
	AddEditorRow(CategoryIndex::Divider, swInstructionsPropertyType::StepDividerMargin);
	CreateLengthEditor(swInstructionsPropertyType::StepDividerPadding, tr("Divider Padding"), tr("Space between the lines and the steps on each side."), 0, 144, 0.25);
	AddEditorRow(CategoryIndex::Divider, swInstructionsPropertyType::StepDividerPadding);
	CreateLengthEditor(swInstructionsPropertyType::StepDividerWidth, tr("Divider Width"), tr("Width of the lines between steps."), 1, 72, 0.1);
	AddEditorRow(CategoryIndex::Divider, swInstructionsPropertyType::StepDividerWidth);
	CreateColorEditor(swInstructionsPropertyType::StepDividerColor, tr("Divider Color"), tr("Color of the lines between steps."));
	AddEditorRow(CategoryIndex::Divider, swInstructionsPropertyType::StepDividerColor);

	AddCategory(CategoryIndex::Notice, tr("Notice"), tr("<p><b>Notice</b></p><p>Free text written on a page to point out something. Right-click the page and choose Add Notice to write one, double-click it to edit its text. Drag it, lock it in place or give it pointers like any other element.</p>"));

	CreateFontEditor(swInstructionsPropertyType::NoticeFont, tr("Notice Font"), tr("Font of the notices."));
	AddEditorRow(CategoryIndex::Notice, swInstructionsPropertyType::NoticeFont);
	CreateColorEditor(swInstructionsPropertyType::NoticeTextColor, tr("Notice Text Color"), tr("Color of the notice text."));
	AddEditorRow(CategoryIndex::Notice, swInstructionsPropertyType::NoticeTextColor);
	CreateChoiceEditor(swInstructionsPropertyType::NoticeAlignment, tr("Notice Alignment"), tr("Line up notice text left, centered or right, a notice can keep its own from its Align menu."), { tr("Left"), tr("Center"), tr("Right") });
	AddEditorRow(CategoryIndex::Notice, swInstructionsPropertyType::NoticeAlignment);
	CreateChoiceEditor(swInstructionsPropertyType::NoticeVerticalAlignment, tr("Notice Vertical Alignment"), tr("Line up notice text at the top, middle or bottom of a box with Edit Size."), { tr("Top"), tr("Middle"), tr("Bottom") });
	AddEditorRow(CategoryIndex::Notice, swInstructionsPropertyType::NoticeVerticalAlignment);

	// the selected notice box, live with edit size
	const QString SizeNames[2] = { tr("Notice Width"), tr("Notice Height") };

	for (int Axis = 0; Axis < 2; Axis++)
	{
		QDoubleSpinBox* SpinBox = new swInstructionsLengthSpinBox(this);
		SpinBox->setToolTip(tr("Size of the selected notice box, check Edit Size in its right click menu to change it."));
		SpinBox->setWhatsThis(SpinBox->toolTip());
		SpinBox->setAccessibleName(SizeNames[Axis]);
		SpinBox->setKeyboardTracking(false);
		swSetupPanelSpinBox(SpinBox);
		mNoticeSizeSpinBoxes[Axis] = SpinBox;
		mNoticeSizeLabels[Axis] = AddLabeledRow(CategoryIndex::Notice, SizeNames[Axis], SpinBox);

		// unit button before the input
		swInstructionsUnitButton* UnitButton = new swInstructionsUnitButton(SpinBox->parentWidget(), SizeNames[Axis]);
		static_cast<QHBoxLayout*>(SpinBox->parentWidget()->layout())->insertWidget(1, UnitButton);
		mNoticeSizeUnitButtons[Axis] = UnitButton;
		connect(UnitButton, &swInstructionsUnitButton::UnitsChanged, this, &swInstructionsPropertiesWidget::UpdateNoticeSize);

		connect(SpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this, Axis](double Value)
		{
			if (!mNoticeKey)
				return;

			QSizeF Size = mInstructions->GetElementSize(*mNoticeKey);

			if (!Size.isValid())
				return;

			const qreal Length = Value * swGetPageUnitScale(mNoticeSizeUnitButtons[Axis]->GetUnits(mInstructions));

			if (Axis == 0)
				Size.setWidth(Length);
			else
				Size.setHeight(Length);

			mInstructions->SetElementSize(*mNoticeKey, Size);
		});
	}

	AddCategory(CategoryIndex::Image, tr("Image"), tr("<p><b>Image</b></p><p>A picture of your own on a page, such as a logo. Right-click the page and choose Add Image to place one. Check Edit Size in its right click menu to change its size here, the height follows the width to keep its shape.</p>"));

	// the selected image, live with edit size
	const QString ImageSizeNames[2] = { tr("Image Width"), tr("Image Height") };

	for (int Axis = 0; Axis < 2; Axis++)
	{
		QDoubleSpinBox* SpinBox = new swInstructionsLengthSpinBox(this);
		SpinBox->setToolTip(tr("Size of the selected image, check Edit Size in its right click menu to change it, the other side follows to keep its shape."));
		SpinBox->setWhatsThis(SpinBox->toolTip());
		SpinBox->setAccessibleName(ImageSizeNames[Axis]);
		SpinBox->setKeyboardTracking(false);
		swSetupPanelSpinBox(SpinBox);
		mImageSizeSpinBoxes[Axis] = SpinBox;
		mImageSizeLabels[Axis] = AddLabeledRow(CategoryIndex::Image, ImageSizeNames[Axis], SpinBox);

		// unit button before the input
		swInstructionsUnitButton* UnitButton = new swInstructionsUnitButton(SpinBox->parentWidget(), ImageSizeNames[Axis]);
		static_cast<QHBoxLayout*>(SpinBox->parentWidget()->layout())->insertWidget(1, UnitButton);
		mImageSizeUnitButtons[Axis] = UnitButton;
		connect(UnitButton, &swInstructionsUnitButton::UnitsChanged, this, &swInstructionsPropertiesWidget::UpdateImageSize);

		connect(SpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this, Axis](double Value)
		{
			const swInstructionsImage* Image = mImageKey ? mInstructions->FindImage(static_cast<int>(std::get<2>(*mImageKey))) : nullptr;

			if (!Image || !mInstructions->GetElementSize(*mImageKey).isValid())
				return;

			// width drives, height keeps the shape
			const qreal Length = Value * swGetPageUnitScale(mImageSizeUnitButtons[Axis]->GetUnits(mInstructions));
			const qreal PageWidth = mInstructions->mPageSetup.Width;
			const qreal Width = qBound(PageWidth / 100.0, Axis == 0 ? Length : Length / mImageAspect, PageWidth * 5.0);
			const swInstructionsElementKey Key = *mImageKey;
			swInstructionsImage Changed = *Image;
			Changed.Size = Width * 100.0 / PageWidth;
			mInstructions->SetImage(Changed);
			mInstructions->SetElementSize(Key, QSizeF(Width, Width * mImageAspect));
		});
	}

	AddCategory(CategoryIndex::Box, tr("Box"), tr("<p><b>Box</b></p><p>The space and border around each element. The margin keeps other elements away outside the border, the padding keeps the border away from the content and the background fills inside the border. The final model, the completion box of the finished main model, step areas, bands and watermark are boxes too. Pointers leave the border. The step and cover models also set their size here, fitted to their box until a size is set. Choose the element to edit, selecting an element on the page chooses it.</p>"));

	// choices follow the box elements
	mBoxElementComboBox = new QComboBox(this);
	// every element with a box
	const std::vector<std::pair<swInstructionsBoxElement, QString>> BoxNames = {
		{ swInstructionsBoxElement::StepImage, tr("Step Model") }, { swInstructionsBoxElement::StepNumber, tr("Step Number") }, { swInstructionsBoxElement::PartsList, tr("Parts List") },
		{ swInstructionsBoxElement::PageNumber, tr("Page Number") }, { swInstructionsBoxElement::CoverImage, tr("Cover Model") }, { swInstructionsBoxElement::Notice, tr("Notice") },
		{ swInstructionsBoxElement::SubmodelPreview, tr("Submodel Preview") }, { swInstructionsBoxElement::Image, tr("Image") }, { swInstructionsBoxElement::PartsListQuantity, tr("Parts List Quantity") },
		{ swInstructionsBoxElement::FinalModel, tr("Final Model") }, { swInstructionsBoxElement::Completion, tr("Completion") }, { swInstructionsBoxElement::StepArea, tr("Step Area") },
		{ swInstructionsBoxElement::HeaderBand, tr("Header") }, { swInstructionsBoxElement::FooterBand, tr("Footer") }, { swInstructionsBoxElement::LeftBand, tr("Left Band") },
		{ swInstructionsBoxElement::RightBand, tr("Right Band") }, { swInstructionsBoxElement::Watermark, tr("Watermark") }, { swInstructionsBoxElement::BillOfMaterialsTitle, tr("BOM Title") },
		{ swInstructionsBoxElement::BillOfMaterialsList, tr("BOM Parts List") }, { swInstructionsBoxElement::BillOfMaterialsQuantity, tr("BOM Quantity") }, { swInstructionsBoxElement::BillOfMaterialsPartCount, tr("BOM Part Count") }
	};

	for (const std::pair<swInstructionsBoxElement, QString>& BoxName : BoxNames)
		mBoxElementComboBox->addItem(BoxName.second, static_cast<int>(BoxName.first));

	for (const swInstructionsPublicationElement& Publication : swGetPublicationElements())
		mBoxElementComboBox->addItem(swGetPublicationName(Publication), static_cast<int>(Publication.Box));

	mBoxElementComboBox->setToolTip(tr("Choose the element whose box is edited."));
	mBoxElementComboBox->setWhatsThis(mBoxElementComboBox->toolTip());
	mBoxElementComboBox->setAccessibleName(tr("Element"));
	swSetupPanelEdit(mBoxElementComboBox);
	AddLabeledRow(CategoryIndex::Box, tr("Element"), mBoxElementComboBox);

	connect(mBoxElementComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int Index)
	{
		if (Index >= 0)
			SetBoxElement(static_cast<swInstructionsBoxElement>(mBoxElementComboBox->itemData(Index).toInt()));
	});

	for (int ElementIndex = 0; ElementIndex < static_cast<int>(swInstructionsBoxElement::Count); ElementIndex++)
	{
		const swInstructionsBoxTypes BoxTypes = swGetBoxTypes(static_cast<swInstructionsBoxElement>(ElementIndex));
		std::vector<QWidget*>& Rows = mBoxRows[ElementIndex];

		// model sizes follow their boxes, fitted until set

		auto AddModelSizeRows = [this, &Rows](swInstructionsPropertyType Type, const QString& ToolTip)
		{
			QDoubleSpinBox* SpinBox = CreateLengthEditor(Type, tr("Model Size"), ToolTip, 0, 2880, 1.0);
			SpinBox->setSpecialValueText(tr("Fit"));
			Rows.push_back(AddEditorRow(CategoryIndex::Box, Type)->parentWidget());

			QToolButton* FitButton = new QToolButton(this);
			FitButton->setText(tr("Fit Model to Box"));
			FitButton->setCheckable(true);
			FitButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
			FitButton->setToolTip(tr("Size the model to fill its box, uncheck to set its size."));
			FitButton->setWhatsThis(FitButton->toolTip());
			FitButton->setAccessibleName(FitButton->text());
			swSetupPanelButton(FitButton);
			mEditors[static_cast<int>(Type)].FitButton = FitButton;

			connect(FitButton, &QToolButton::toggled, this, [this, Type](bool Fit)
			{
				SetIntValue(Type, Fit ? 0 : qRound(GetEditorMaximum(mEditors[static_cast<int>(Type)]) / 2.0));
			});

			Rows.push_back(AddRow(CategoryIndex::Box, { FitButton }));
		};

		CreateLengthEditor(BoxTypes.Margin, tr("Margin"), tr("Space outside the border."), 0, 288, 0.5);
		Rows.push_back(AddEditorRow(CategoryIndex::Box, BoxTypes.Margin)->parentWidget());
		CreateLengthEditor(BoxTypes.Padding, tr("Padding"), tr("Space between the border and the content, filled with the background."), 0, 288, 0.5);
		Rows.push_back(AddEditorRow(CategoryIndex::Box, BoxTypes.Padding)->parentWidget());
		CreateLengthEditor(BoxTypes.BorderWidth, tr("Border Width"), tr("Width of the line around the element, 0 draws no line."), 0, 72, 0.1);
		Rows.push_back(AddEditorRow(CategoryIndex::Box, BoxTypes.BorderWidth)->parentWidget());
		CreateLengthEditor(BoxTypes.CornerRadius, tr("Corner Radius"), tr("Roundness of the box corners, 0 keeps them square."), 0, 72, 0.5);
		Rows.push_back(AddEditorRow(CategoryIndex::Box, BoxTypes.CornerRadius)->parentWidget());
		CreateColorEditor(BoxTypes.BorderColor, tr("Border Color"), tr("Color of the line around the element."));
		Rows.push_back(AddEditorRow(CategoryIndex::Box, BoxTypes.BorderColor)->parentWidget());
		CreateColorEditor(BoxTypes.BackgroundColor, tr("Background"), tr("Color filling the whole element, margin and padding included, transparent shows what lies behind."));
		Rows.push_back(AddEditorRow(CategoryIndex::Box, BoxTypes.BackgroundColor)->parentWidget());

		if (ElementIndex == static_cast<int>(swInstructionsBoxElement::StepImage))
		{
			AddModelSizeRows(swInstructionsPropertyType::StepImageSize, tr("Longest side of the step model, larger sizes are trimmed to its space."));

			mModelSizePageButton = new QToolButton(this);
			mModelSizePageButton->setText(tr("Model Size on This Page Only"));
			mModelSizePageButton->setCheckable(true);
			mModelSizePageButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
			mModelSizePageButton->setToolTip(tr("Give this page its own model size, unchecked it follows all pages."));
			mModelSizePageButton->setWhatsThis(mModelSizePageButton->toolTip());
			mModelSizePageButton->setAccessibleName(mModelSizePageButton->text());
			swSetupPanelButton(mModelSizePageButton);

			connect(mModelSizePageButton, &QToolButton::toggled, this, [this](bool PageOnly)
			{
				mInstructions->SetPageScope(mPageIndex, swInstructionsPropertyType::StepImageSize, PageOnly);
			});

			Rows.push_back(AddRow(CategoryIndex::Box, { mModelSizePageButton }));

			mModelSizeApplyAllButton = CreateApplyAllButton(tr("Use the model size of this page on every page."));

			connect(mModelSizeApplyAllButton, &QToolButton::clicked, this, [this]()
			{
				mInstructions->ApplyPageToAllPages(mPageIndex, swInstructionsPropertyType::StepImageSize);
			});

			Rows.push_back(AddRow(CategoryIndex::Box, { mModelSizeApplyAllButton }));
		}
		else if (ElementIndex == static_cast<int>(swInstructionsBoxElement::SubmodelPreview))
		{
			AddModelSizeRows(swInstructionsPropertyType::SubmodelPreviewSize, tr("Longest side of the submodel preview, larger sizes are trimmed to a quarter of the page."));
		}
		else if (ElementIndex == static_cast<int>(swInstructionsBoxElement::CoverImage))
		{
			AddModelSizeRows(swInstructionsPropertyType::CoverImageSize, tr("Longest side of the cover model, larger sizes are trimmed to its space."));
		}
	}

	AddCategory(CategoryIndex::Pointer, tr("Pointer"), tr("<p><b>Pointer</b></p><p>A pointer leaves the border of an element to point at a part, a step or a notice. Right-click an element and choose Add Pointer, then drag its tip onto a part of a step model, another element or any point of the page, the tip follows what it points at. Right-click a pointer to choose the anchor it leaves from, or its own style and tip.</p>"));

	// choices follow pointer styles and tips
	CreateChoiceEditor(swInstructionsPropertyType::PointerStyle, tr("Pointer Style"), tr("Draw pointers as a line, an L shape with one bend, a Z shape with two bends, a U shape going around or a callout wedge narrowing to the tip."), { tr("Line"), tr("L Shape"), tr("Z Shape"), tr("U Shape"), tr("Callout Wedge") }, { 0, 2, 3, 4, 1 });
	AddEditorRow(CategoryIndex::Pointer, swInstructionsPropertyType::PointerStyle);
	CreateChoiceEditor(swInstructionsPropertyType::PointerTip, tr("Pointer Tip"), tr("End line pointers with an arrow, a dot or a hollow dot, callout wedges have no tip."), { tr("Arrow"), tr("Dot"), tr("Hollow Dot") });
	AddEditorRow(CategoryIndex::Pointer, swInstructionsPropertyType::PointerTip);
	CreateColorEditor(swInstructionsPropertyType::PointerColor, tr("Pointer Color"), tr("Color of the pointers."));
	AddEditorRow(CategoryIndex::Pointer, swInstructionsPropertyType::PointerColor);

	// lowest pointer values
	CreateLengthEditor(swInstructionsPropertyType::PointerWidth, tr("Pointer Width"), tr("Width of the pointer lines, Border follows the border of the element."), 0, 72, 0.1)->setSpecialValueText(tr("Border"));
	AddEditorRow(CategoryIndex::Pointer, swInstructionsPropertyType::PointerWidth);
	CreateLengthEditor(swInstructionsPropertyType::PointerTipSize, tr("Tip Size"), tr("Size of the arrows and dots, and width of the wedges at the element, never less than three times the pointer width."), 0, 288, 0.5)->setSpecialValueText(tr("Automatic"));
	AddEditorRow(CategoryIndex::Pointer, swInstructionsPropertyType::PointerTipSize);
	CreateLengthEditor(swInstructionsPropertyType::PointerBendRadius, tr("Bend Radius"), tr("Roundness of the L, Z and U shape bends, 0 keeps them sharp."), 0, 72, 0.5);
	AddEditorRow(CategoryIndex::Pointer, swInstructionsPropertyType::PointerBendRadius);

	// labels share one width
	int LabelWidth = 0;

	for (const QLabel* Label : mRowLabels)
		LabelWidth = qMax(LabelWidth, Label->sizeHint().width());

	for (QLabel* Label : mRowLabels)
		Label->setFixedWidth(LabelWidth);
}

void swInstructionsPropertiesWidget::AddCategory(CategoryIndex Index, const QString& Title, const QString& WhatsThis)
{
	CategoryWidgets& Category = mCategoryWidgets[static_cast<int>(Index)];

	Category.Button = new swCollapsibleWidgetButton(Title, this);
	Category.Button->SetExpanded(false);
	Category.Button->setToolTip(tr("Expand or collapse the %1 properties.").arg(Title));
	Category.Button->setWhatsThis(WhatsThis);
	mCategoriesLayout->addWidget(Category.Button);

	Category.Rows = new QWidget(this);
	Category.Rows->setWhatsThis(WhatsThis);
	QVBoxLayout* RowsLayout = new QVBoxLayout(Category.Rows);
	RowsLayout->setContentsMargins(0, 0, 0, 0);
	mCategoriesLayout->addWidget(Category.Rows);

	connect(Category.Button, &swCollapsibleWidgetButton::StateChanged, this, &swInstructionsPropertiesWidget::UpdateCategories);
}

QWidget* swInstructionsPropertiesWidget::AddRow(CategoryIndex Index, const std::vector<QWidget*>& Editors)
{
	QWidget* Rows = mCategoryWidgets[static_cast<int>(Index)].Rows;
	QWidget* Row = swCreatePanelRow(Rows);
	Rows->layout()->addWidget(Row);

	// editors share the row width
	for (QWidget* Editor : Editors)
	{
		Editor->setParent(Row);
		Editor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

		// spin boxes fit their largest value
		if (!qobject_cast<QAbstractSpinBox*>(Editor))
			Editor->setMinimumWidth(0);
		static_cast<QHBoxLayout*>(Row->layout())->addWidget(Editor, 1);
	}

	return Row;
}

QLabel* swInstructionsPropertiesWidget::AddLabeledRow(CategoryIndex Index, const QString& Name, QWidget* Editor)
{
	// name beside the input
	QWidget* Row = AddRow(Index, { Editor });
	QLabel* Label = new QLabel(Name, Row);
	Label->setBuddy(Editor);
	static_cast<QHBoxLayout*>(Row->layout())->insertWidget(0, Label);
	mRowLabels.push_back(Label);

	return Label;
}

QLabel* swInstructionsPropertiesWidget::AddEditorRow(CategoryIndex Index, swInstructionsPropertyType Type)
{
	EditorWidgets& Editors = mEditors[static_cast<int>(Type)];

	// unit and font size before the input
	std::vector<QWidget*> RowEditors;

	if (Editors.SizeSpinBox)
		RowEditors.push_back(Editors.SizeSpinBox);

	RowEditors.push_back(Editors.Widget);

	if (Editors.BrowseButton)
		RowEditors.push_back(Editors.BrowseButton);

	QWidget* Row = AddRow(Index, RowEditors);
	QHBoxLayout* RowLayout = static_cast<QHBoxLayout*>(Row->layout());

	Editors.Label = new QLabel(Editors.Name, Row);
	Editors.Label->setBuddy(Editors.SizeSpinBox ? Editors.SizeSpinBox : Editors.Widget);
	RowLayout->insertWidget(0, Editors.Label);
	mRowLabels.push_back(Editors.Label);

	if (Editors.UnitButton)
	{
		Editors.UnitButton->setParent(Row);
		RowLayout->insertWidget(1, Editors.UnitButton);
	}

	if (Editors.SymbolButton)
	{
		Editors.SymbolButton->setParent(Row);
		RowLayout->insertWidget(1, Editors.SymbolButton);
	}

	return Editors.Label;
}

void swInstructionsPropertiesWidget::RegisterEditor(swInstructionsPropertyType Type, EditorType Editor, QWidget* Widget, const QString& Name, const QString& ToolTip)
{
	Widget->setToolTip(ToolTip);
	Widget->setWhatsThis(ToolTip);
	Widget->setAccessibleName(Name);

	EditorWidgets& Editors = mEditors[static_cast<int>(Type)];
	Editors.Widget = Widget;
	Editors.Type = Editor;
	Editors.Name = Name;
}

QToolButton* swInstructionsPropertiesWidget::CreateToggleEditor(swInstructionsPropertyType Type, const QString& Name, const QString& ToolTip)
{
	QToolButton* Button = new QToolButton(this);
	RegisterEditor(Type, EditorType::Bool, Button, Name, ToolTip);
	Button->setText(Name);
	Button->setCheckable(true);
	Button->setToolButtonStyle(Qt::ToolButtonTextOnly);
	swSetupPanelButton(Button);

	connect(Button, &QToolButton::toggled, this, [this, Type](bool Checked)
	{
		if (mInstructions->HasPageProperty(mPageIndex, Type))
			mInstructions->SetPageInt(mPageIndex, Type, Checked);
		else
			mInstructions->SetDefaultBool(Type, Checked);
	});

	return Button;
}

// use to get the largest value of an editor
double swInstructionsPropertiesWidget::GetEditorMaximum(const EditorWidgets& Editor) const
{
	if (Editor.PageFraction <= 0.0)
		return Editor.Maximum;

	const swInstructionsPageSetup& PageSetup = mInstructions->mPageSetup;

	return qMax<double>(Editor.Minimum + 1, qFloor(qMin(PageSetup.Width, PageSetup.Height) * Editor.PageFraction));
}

QDoubleSpinBox* swInstructionsPropertiesWidget::CreateLengthEditor(swInstructionsPropertyType Type, const QString& Name, const QString& ToolTip, int Minimum, int Maximum, double PageFraction)
{
	// lengths in points shown in page units
	QDoubleSpinBox* SpinBox = new swInstructionsLengthSpinBox(this);
	RegisterEditor(Type, EditorType::Length, SpinBox, Name, ToolTip);

	EditorWidgets& Editors = mEditors[static_cast<int>(Type)];
	Editors.Minimum = Minimum;
	Editors.Maximum = Maximum;
	Editors.PageFraction = PageFraction;

	Editors.UnitButton = new swInstructionsUnitButton(this, Name);
	swSetupLengthSpinBox(SpinBox, mInstructions->GetPageUnits(), Minimum, GetEditorMaximum(Editors));

	// apply and rebuild on typing
	SpinBox->setKeyboardTracking(false);
	swSetupPanelSpinBox(SpinBox);

	connect(SpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this, Type](double Value)
	{
		SetIntValue(Type, qRound(Value * swGetPageUnitScale(GetEditorUnits(Type))));
	});

	connect(Editors.UnitButton, &swInstructionsUnitButton::UnitsChanged, this, &swInstructionsPropertiesWidget::UpdateEditors);

	return SpinBox;
}

QSpinBox* swInstructionsPropertiesWidget::CreatePercentEditor(swInstructionsPropertyType Type, const QString& Name, const QString& ToolTip, int Minimum, int Maximum)
{
	swRangeSpinBox* SpinBox = new swRangeSpinBox(this);
	RegisterEditor(Type, EditorType::Percent, SpinBox, Name, ToolTip);

	SpinBox->setRange(Minimum, Maximum);
	SpinBox->setSingleStep(5);
	SpinBox->SetReference();
	SpinBox->setKeyboardTracking(false);
	swSetupPanelSpinBox(SpinBox);

	// percent sign beside like units
	QToolButton* SymbolButton = new QToolButton(this);
	SymbolButton->setText(QStringLiteral("%"));
	SymbolButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
	SymbolButton->setFocusPolicy(Qt::NoFocus);
	SymbolButton->setAttribute(Qt::WA_TransparentForMouseEvents);
	swSetupPanelButton(SymbolButton);
	SymbolButton->setFixedWidth(SymbolButton->height());
	mEditors[static_cast<int>(Type)].SymbolButton = SymbolButton;

	connect(SpinBox, qOverload<int>(&QSpinBox::valueChanged), this, [this, Type](int Value)
	{
		SetIntValue(Type, Value);
	});

	return SpinBox;
}

QComboBox* swInstructionsPropertiesWidget::CreateChoiceEditor(swInstructionsPropertyType Type, const QString& Name, const QString& ToolTip, const QStringList& Choices, const QList<int>& Values)
{
	QComboBox* ComboBox = new QComboBox(this);
	RegisterEditor(Type, EditorType::Choice, ComboBox, Name, ToolTip);

	// choices in property value order unless values are given
	for (int Index = 0; Index < Choices.size(); Index++)
		ComboBox->addItem(Choices[Index], Index < Values.size() ? Values[Index] : Index);

	swSetupPanelEdit(ComboBox);

	connect(ComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, [this, Type, ComboBox](int Index)
	{
		if (Index >= 0)
			mInstructions->SetDefaultInt(Type, ComboBox->itemData(Index).toInt());
	});

	return ComboBox;
}

// use to create a button giving every page a value of this page
QToolButton* swInstructionsPropertiesWidget::CreateApplyAllButton(const QString& ToolTip)
{
	QToolButton* Button = new QToolButton(this);
	Button->setText(tr("Apply to All Pages"));
	Button->setToolTip(ToolTip);
	Button->setWhatsThis(ToolTip);
	Button->setToolButtonStyle(Qt::ToolButtonTextOnly);
	swSetupPanelButton(Button);

	return Button;
}

QToolButton* swInstructionsPropertiesWidget::CreateColorEditor(swInstructionsPropertyType Type, const QString& Name, const QString& ToolTip)
{
	QToolButton* Button = new QToolButton(this);
	RegisterEditor(Type, EditorType::Color, Button, Name, ToolTip);
	Button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	swSetupPanelButton(Button);

	connect(Button, &QToolButton::clicked, this, [this, Type]()
	{
		ColorButtonClicked(Type);
	});

	return Button;
}

// a set path turns browse into reset
static void swUpdateBrowseButton(QToolButton* BrowseButton, const QString& FileName)
{
	const bool HasFile = !FileName.trimmed().isEmpty();

	BrowseButton->setText(HasFile ? QString() : QLatin1String("..."));
	BrowseButton->setIcon(HasFile ? BrowseButton->style()->standardIcon(QStyle::SP_LineEditClearButton) : QIcon());
	BrowseButton->setToolTip(HasFile ? QCoreApplication::translate("swInstructionsPropertiesWidget", "Remove the text file and go back to the typed or automatic text.") : QCoreApplication::translate("swInstructionsPropertiesWidget", "Choose the text file."));
	BrowseButton->setWhatsThis(BrowseButton->toolTip());
}

// empty text keeps the automatic text
QLineEdit* swInstructionsPropertiesWidget::CreateTextEditor(swInstructionsPropertyType Type, const QString& Name, const QString& ToolTip, bool File)
{
	QLineEdit* LineEdit = new QLineEdit(this);
	RegisterEditor(Type, EditorType::Text, LineEdit, Name, ToolTip);
	LineEdit->setPlaceholderText(File ? tr("No File") : tr("Automatic"));
	LineEdit->setClearButtonEnabled(!File);
	swSetupPanelEdit(LineEdit);

	connect(LineEdit, &QLineEdit::editingFinished, this, [this, Type, LineEdit, File]()
	{
		mInstructions->SetDefaultText(Type, File ? LineEdit->text().trimmed() : LineEdit->text());
	});

	if (!File)
		return LineEdit;

	EditorWidgets& Editors = mEditors[static_cast<int>(Type)];
	Editors.BrowseButton = new QToolButton(this);
	Editors.BrowseButton->setAccessibleName(tr("Choose %1").arg(Name));
	swSetupPanelButton(Editors.BrowseButton);
	swUpdateBrowseButton(Editors.BrowseButton, LineEdit->text());

	connect(LineEdit, &QLineEdit::textChanged, Editors.BrowseButton, [BrowseButton = Editors.BrowseButton](const QString& Text)
	{
		swUpdateBrowseButton(BrowseButton, Text);
	});

	connect(Editors.BrowseButton, &QToolButton::clicked, this, [this, Type, LineEdit, Name]()
	{
		if (!LineEdit->text().trimmed().isEmpty())
		{
			mInstructions->SetDefaultText(Type, QString());
			return;
		}

		const QString FileName = QFileDialog::getOpenFileName(this, Name, LineEdit->text(), tr("Text Files (*.txt);;All Files (*)"));

		if (!FileName.isEmpty())
			mInstructions->SetDefaultText(Type, FileName);
	});

	return LineEdit;
}

QToolButton* swInstructionsPropertiesWidget::CreateFontEditor(swInstructionsPropertyType Type, const QString& Name, const QString& ToolTip)
{
	// size in page units
	QToolButton* Button = new QToolButton(this);
	RegisterEditor(Type, EditorType::Font, Button, Name, ToolTip);
	Button->setToolButtonStyle(Qt::ToolButtonTextOnly);
	swSetupPanelButton(Button);

	EditorWidgets& Editors = mEditors[static_cast<int>(Type)];
	Editors.Minimum = 4;
	Editors.Maximum = 288;
	Editors.UnitButton = new swInstructionsUnitButton(this, Name);
	Editors.SizeSpinBox = new swInstructionsLengthSpinBox(this);
	swSetupLengthSpinBox(Editors.SizeSpinBox, mInstructions->GetPageUnits(), Editors.Minimum, Editors.Maximum);
	Editors.SizeSpinBox->setKeyboardTracking(false);
	Editors.SizeSpinBox->setToolTip(tr("Size of the font in the page units."));
	Editors.SizeSpinBox->setWhatsThis(Editors.SizeSpinBox->toolTip());
	Editors.SizeSpinBox->setAccessibleName(tr("%1 Size").arg(Name));
	swSetupPanelSpinBox(Editors.SizeSpinBox);

	connect(Button, &QToolButton::clicked, this, [this, Type]()
	{
		FontButtonClicked(Type);
	});

	connect(Editors.SizeSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this, Type](double Size)
	{
		QFont Font = mInstructions->GetFontProperty(Type, mModel, mStep, false);
		Font.setPointSizeF(Size * swGetPageUnitScale(GetEditorUnits(Type)));
		mInstructions->SetDefaultFont(Type, Font);
	});

	connect(Editors.UnitButton, &swInstructionsUnitButton::UnitsChanged, this, &swInstructionsPropertiesWidget::UpdateEditors);

	return Button;
}

void swInstructionsPropertiesWidget::ColorButtonClicked(swInstructionsPropertyType Type)
{
	QToolButton* Button = qobject_cast<QToolButton*>(mEditors[static_cast<int>(Type)].Widget);

	// edit the alpha of backgrounds
	QColor SelectedColor = swPickColor(Button, GetColorValue(Type));

	// keep faded parts visible
	if (SelectedColor.isValid() && Type == swInstructionsPropertyType::FadeColor)
		SelectedColor = swQColorFromRGBA(swClampFadeColorAlpha(swRGBAFromQColor(SelectedColor)));

	if (SelectedColor.isValid())
		SetColorValue(Type, SelectedColor);
}

void swInstructionsPropertiesWidget::FontButtonClicked(swInstructionsPropertyType Type)
{
	QFontDialog Dialog(mInstructions->GetFontProperty(Type, mModel, mStep, false), this);
	Dialog.setWindowTitle(mEditors[static_cast<int>(Type)].Name);

	// a quarter of the screen, centered

	if (const QScreen* Screen = screen())
	{
		const QRect ScreenRect = Screen->availableGeometry();
		Dialog.resize(ScreenRect.size() / 2);
		Dialog.move(ScreenRect.center() - QPoint(ScreenRect.width() / 4, ScreenRect.height() / 4));
	}

	if (Dialog.exec() == QDialog::Accepted)
		mInstructions->SetDefaultFont(Type, Dialog.selectedFont());
}

void swInstructionsPropertiesWidget::SetCategoriesExpanded(bool Expanded)
{
	for (CategoryWidgets& Category : mCategoryWidgets)
		Category.Button->SetExpanded(Expanded);

	UpdateCategories();
}

void swInstructionsPropertiesWidget::AddOptionsAction(QAction* Action)
{
	// actions fill the row beside options
	QHBoxLayout* RowLayout = static_cast<QHBoxLayout*>(mOptionsRow->layout());
	QToolButton* Button = new QToolButton(mOptionsRow);

	Button->setDefaultAction(Action);
	Button->setToolButtonStyle(Qt::ToolButtonTextOnly);
	Button->setWhatsThis(Action->toolTip());
	swSetupPanelButton(Button);
	Button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	if (RowLayout->itemAt(0)->spacerItem())
		delete RowLayout->takeAt(0);

	RowLayout->insertWidget(RowLayout->count() - 1, Button, 1);
}

void swInstructionsPropertiesWidget::SelectionChanged(QGraphicsItem* FocusItem, bool Clicked)
{
	// remember the notice for its size
	mNoticeKey.reset();
	mNoticeItemSize = QSizeF();
	swInstructionsElementKey NoticeKey;

	if (dynamic_cast<swInstructionsNoticeItem*>(FocusItem) && swGetElementKey(FocusItem, NoticeKey))
	{
		mNoticeKey = NoticeKey;
		mNoticeItemSize = FocusItem->boundingRect().size();
	}

	UpdateNoticeSize();

	// remember the image and its shape
	mImageKey.reset();
	swInstructionsElementKey ImageKey;

	if (dynamic_cast<swInstructionsImageItem*>(FocusItem) && swGetElementKey(FocusItem, ImageKey))
	{
		mImageKey = ImageKey;
		const QSizeF ItemSize = FocusItem->boundingRect().size();
		mImageAspect = ItemSize.width() > 0.0 ? ItemSize.height() / ItemSize.width() : 1.0;
	}

	UpdateImageSize();

	// quantities edit their parts list
	const bool QuantityFocus = FocusItem && dynamic_cast<swInstructionsPartsListCountItem*>(FocusItem);

	if (QuantityFocus)
		FocusItem = FocusItem->parentItem();

	// keep item kind and step only
	FocusType Focus = FocusType::None;
	const swInstructionsPublicationElement* FocusPublication = nullptr;
	swModel* Model = nullptr;
	swStep Step = 1;

	if (swInstructionsStepImageItem* ImageItem = dynamic_cast<swInstructionsStepImageItem*>(FocusItem))
	{
		Focus = FocusType::StepImage;
		Model = ImageItem->GetModel();
		Step = ImageItem->GetStep();
	}
	else if (swInstructionsStepNumberItem* NumberItem = dynamic_cast<swInstructionsStepNumberItem*>(FocusItem))
	{
		Focus = FocusType::StepNumber;
		Model = NumberItem->GetModel();
		Step = NumberItem->GetStep();
	}
	else if (swInstructionsPartsListItem* PartsItem = dynamic_cast<swInstructionsPartsListItem*>(FocusItem))
	{
		Focus = FocusType::PartsList;
		Model = PartsItem->GetModel();
		Step = PartsItem->GetStep();
	}
	else if (dynamic_cast<swInstructionsPageNumberItem*>(FocusItem))
		Focus = FocusType::PageNumber;
	else if (swInstructionsPartCountItem* CountItem = dynamic_cast<swInstructionsPartCountItem*>(FocusItem))
	{
		const swInstructionsPlacementElement CountElement = CountItem->GetElement();

		if (CountElement == swInstructionsPlacementElement::BillOfMaterialsPartCount)
			Focus = FocusType::BillOfMaterialsPartCount;
		else if (CountElement == swInstructionsPlacementElement::PagePartCount || CountElement == swInstructionsPlacementElement::SubmodelPreviewPartCount)
			Focus = FocusType::PagePartCount;

		Model = CountItem->GetModel();
		Step = 0;
	}
	else if (swInstructionsCoverTextItem* TextItem = dynamic_cast<swInstructionsCoverTextItem*>(FocusItem))
	{
		FocusPublication = swFindPublicationElement(TextItem->GetElement());
		Focus = FocusType::Publication;
		Model = TextItem->GetModel();
		Step = 0;
	}
	else if (dynamic_cast<swInstructionsNoticeItem*>(FocusItem))
		Focus = FocusType::Notice;
	else if (dynamic_cast<swInstructionsPointerItem*>(FocusItem))
		Focus = FocusType::Pointer;
	else if (dynamic_cast<swInstructionsImageItem*>(FocusItem))
		Focus = FocusType::Image;
	else if (FocusItem && FocusItem->data(SW_BAND_DATA).isValid())
		Focus = FocusType::Bands;

	// each band side opens its group
	swInstructionsImagePages BandPages = FocusItem && FocusItem->data(SW_BAND_DATA).isValid() ? static_cast<swInstructionsImagePages>(FocusItem->data(SW_BAND_DATA).toInt()) : swInstructionsImagePages::Header;

	// band and watermark content opens their group
	swInstructionsElementKey Key;

	if ((Focus == FocusType::Notice || Focus == FocusType::Image) && swGetElementKey(FocusItem, Key))
	{
		const int Id = static_cast<int>(std::get<2>(Key));
		const swInstructionsNotice* Notice = Focus == FocusType::Notice ? mInstructions->FindNotice(Id) : nullptr;
		const swInstructionsImage* Image = Focus == FocusType::Image ? mInstructions->FindImage(Id) : nullptr;
		const swInstructionsImagePages Pages = Notice ? Notice->Pages : Image ? Image->Pages : swInstructionsImagePages::None;

		if (Pages == swInstructionsImagePages::Watermark)
			Focus = Notice ? FocusType::WatermarkNotice : FocusType::Watermark;
		else if (Pages >= swInstructionsImagePages::Header)
		{
			Focus = FocusType::BandContent;
			BandPages = Pages;
		}
		else if (Image && Pages == swInstructionsImagePages::None)
			Focus = FocusType::PageImage;
	}

	// cover and materials own settings
	if (Focus == FocusType::StepImage && dynamic_cast<swInstructionsStepImageItem*>(FocusItem)->IsCoverImage())
		Focus = FocusType::CoverImage;
	else if (Focus == FocusType::StepImage && dynamic_cast<swInstructionsStepImageItem*>(FocusItem)->IsSubmodelPreview())
		Focus = FocusType::SubmodelPreview;
	else if (Focus == FocusType::PartsList && dynamic_cast<swInstructionsPartsListItem*>(FocusItem)->IsBillOfMaterials())
		Focus = FocusType::BillOfMaterials;

	// box section edits the selection
	switch (Focus)
	{
	case FocusType::StepImage:
		SetBoxElement(swInstructionsBoxElement::StepImage);
		break;

	case FocusType::StepNumber:
		SetBoxElement(Step ? swInstructionsBoxElement::StepNumber : swInstructionsBoxElement::BillOfMaterialsTitle);
		break;

	case FocusType::PartsList:
		SetBoxElement(QuantityFocus ? swInstructionsBoxElement::PartsListQuantity : swInstructionsBoxElement::PartsList);
		break;

	case FocusType::BillOfMaterials:
		SetBoxElement(QuantityFocus ? swInstructionsBoxElement::BillOfMaterialsQuantity : swInstructionsBoxElement::BillOfMaterialsList);
		break;

	case FocusType::PageNumber:
		SetBoxElement(swInstructionsBoxElement::PageNumber);
		break;

	case FocusType::Publication:
		SetBoxElement(FocusPublication->Box);
		SetPublicationCover(FocusPublication->Back);
		break;

	case FocusType::CoverImage:
		SetBoxElement(swInstructionsBoxElement::CoverImage);
		break;

	case FocusType::PagePartCount:
		break;

	case FocusType::BillOfMaterialsPartCount:
		SetBoxElement(swInstructionsBoxElement::BillOfMaterialsPartCount);
		break;

	case FocusType::SubmodelPreview:
		SetBoxElement(swInstructionsBoxElement::SubmodelPreview);
		break;

	case FocusType::Notice:
		SetBoxElement(swInstructionsBoxElement::Notice);
		break;

	case FocusType::WatermarkNotice:
		SetBoxElement(swInstructionsBoxElement::Watermark);
		break;

	case FocusType::BandContent:
		if (dynamic_cast<swInstructionsNoticeItem*>(FocusItem))
			SetBoxElement(swInstructionsBoxElement::Notice);
		else if (dynamic_cast<swInstructionsImageItem*>(FocusItem))
			SetBoxElement(swInstructionsBoxElement::Image);
		break;

	case FocusType::Watermark:
		if (dynamic_cast<swInstructionsImageItem*>(FocusItem))
			SetBoxElement(swInstructionsBoxElement::Watermark);
		break;

	case FocusType::Image:
	case FocusType::PageImage:
		if (dynamic_cast<swInstructionsImageItem*>(FocusItem))
			SetBoxElement(swInstructionsBoxElement::Image);
		break;

	case FocusType::Bands:
		SetBoxElement(static_cast<swInstructionsBoxElement>(static_cast<int>(swInstructionsBoxElement::HeaderBand) + static_cast<int>(BandPages) - static_cast<int>(swInstructionsImagePages::Header)));
		break;

	case FocusType::Pointer:
	case FocusType::None:
		break;
	}

	if (Focus == mFocus && Model == mModel && Step == mStep && !Clicked)
		return;

	// every click opens its own group
	if ((Focus != mFocus || Clicked) && Focus != FocusType::None)
	{
		CategoryIndex Open = CategoryIndex::Box;
		CategoryIndex OpenAlso = CategoryIndex::Box;

		switch (Focus)
		{
		case FocusType::StepNumber:
			Open = Step ? CategoryIndex::Step : CategoryIndex::BillOfMaterials;
			break;

		case FocusType::PartsList:
			Open = CategoryIndex::PartsList;
			break;

		case FocusType::BillOfMaterials:
			Open = CategoryIndex::BillOfMaterials;
			break;

		case FocusType::BillOfMaterialsPartCount:
			Open = CategoryIndex::BillOfMaterials;
			break;

		case FocusType::PageNumber:
		case FocusType::PagePartCount:
		case FocusType::SubmodelPreview:
			Open = CategoryIndex::Page;
			break;

		case FocusType::Publication:
			Open = CategoryIndex::Publication;
			break;

		case FocusType::CoverImage:
			Open = CategoryIndex::Cover;
			break;

		case FocusType::Notice:
			Open = CategoryIndex::Notice;
			break;

		case FocusType::Pointer:
			Open = CategoryIndex::Pointer;
			break;

		case FocusType::Bands:
			Open = BandPages == swInstructionsImagePages::Left ? CategoryIndex::LeftBand : BandPages == swInstructionsImagePages::Right ? CategoryIndex::RightBand : BandPages == swInstructionsImagePages::Footer ? CategoryIndex::Footer : CategoryIndex::Header;
			OpenAlso = CategoryIndex::Box;
			break;

		case FocusType::BandContent:
			Open = BandPages == swInstructionsImagePages::Left ? CategoryIndex::LeftBand : BandPages == swInstructionsImagePages::Right ? CategoryIndex::RightBand : BandPages == swInstructionsImagePages::Footer ? CategoryIndex::Footer : CategoryIndex::Header;
			OpenAlso = dynamic_cast<swInstructionsNoticeItem*>(FocusItem) ? CategoryIndex::Notice : dynamic_cast<swInstructionsImageItem*>(FocusItem) ? CategoryIndex::Image : Open;
			break;

		case FocusType::Watermark:
			Open = CategoryIndex::Watermark;
			break;

		case FocusType::WatermarkNotice:
			Open = CategoryIndex::Watermark;
			break;

		case FocusType::Image:
		case FocusType::PageImage:
			Open = CategoryIndex::Image;
			break;

		case FocusType::StepImage:
		case FocusType::None:
			break;
		}

		// boxed elements also open the box section

		const bool OpenBox = Focus != FocusType::Pointer;

		for (int Index = 0; Index < static_cast<int>(CategoryIndex::Count); Index++)
			mCategoryWidgets[Index].Button->SetExpanded(Index == static_cast<int>(Open) || Index == static_cast<int>(OpenAlso) || (OpenBox && Index == static_cast<int>(CategoryIndex::Box)));
	}

	mFocus = Focus;
	mModel = Model;
	mStep = Step;

	UpdateWidget();
}

void swInstructionsPropertiesWidget::PageClicked(swInstructionsPageType PageType, std::optional<swInstructionsPageObject> Object)
{
	// a blank spot opens its page groups

	CategoryIndex Open = CategoryIndex::Page;
	CategoryIndex OpenAlso = CategoryIndex::Page;

	switch (PageType)
	{
	case swInstructionsPageType::Steps:
		OpenAlso = CategoryIndex::PageLayout;
		break;

	case swInstructionsPageType::Cover:
	case swInstructionsPageType::BackCover:
		Open = CategoryIndex::Cover;
		break;

	case swInstructionsPageType::BillOfMaterials:
		Open = CategoryIndex::BillOfMaterials;
		break;

	case swInstructionsPageType::Blank:
		break;
	}

	// plain objects open their own group

	if (Object == swInstructionsPageObject::Divider)
		Open = OpenAlso = CategoryIndex::Divider;
	else if (Object == swInstructionsPageObject::StepArea)
	{
		Open = CategoryIndex::Step;
		OpenAlso = CategoryIndex::Box;
		SetBoxElement(swInstructionsBoxElement::StepArea);
	}
	else if (Object == swInstructionsPageObject::FinalModel || Object == swInstructionsPageObject::Completion)
	{
		Open = CategoryIndex::PageLayout;
		OpenAlso = CategoryIndex::Box;
		SetBoxElement(Object == swInstructionsPageObject::Completion ? swInstructionsBoxElement::Completion : swInstructionsBoxElement::FinalModel);
	}

	for (int Index = 0; Index < static_cast<int>(CategoryIndex::Count); Index++)
		mCategoryWidgets[Index].Button->SetExpanded(Index == static_cast<int>(Open) || Index == static_cast<int>(OpenAlso));

	UpdateCategories();
}

void swInstructionsPropertiesWidget::SetBoxElement(swInstructionsBoxElement Element)
{
	mBoxElement = Element;

	{
		QSignalBlocker Blocker(mBoxElementComboBox);
		mBoxElementComboBox->setCurrentIndex(mBoxElementComboBox->findData(static_cast<int>(Element)));
	}

	// show the chosen element rows
	for (int ElementIndex = 0; ElementIndex < static_cast<int>(mBoxRows.size()); ElementIndex++)
		for (QWidget* Row : mBoxRows[ElementIndex])
			Row->setVisible(ElementIndex == static_cast<int>(Element));
}

void swInstructionsPropertiesWidget::SetPublicationCover(bool Back)
{
	{
		QSignalBlocker Blocker(mPublicationCoverComboBox);
		mPublicationCoverComboBox->setCurrentIndex(Back ? 1 : 0);
	}

	// show the chosen cover rows
	for (int Cover = 0; Cover < 2; Cover++)
		for (QWidget* Row : mPublicationRows[Cover])
			Row->setVisible((Cover == 1) == Back);
}

void swInstructionsPropertiesWidget::SetCurrentPage(int PageIndex)
{
	mPageIndex = PageIndex;

	// show the current page
	UpdateEditors();
	UpdatePageLayout();
	UpdateCategories();
}

// use to show the selected notice size
void swInstructionsPropertiesWidget::UpdateNoticeSize()
{
	const QSizeF SetSize = mNoticeKey ? mInstructions->GetElementSize(*mNoticeKey) : QSizeF();
	const QSizeF Size = SetSize.isValid() ? SetSize : mNoticeItemSize;
	const qreal Lengths[2] = { Size.width(), Size.height() };
	const qreal PageLengths[2] = { mInstructions->mPageSetup.Width, mInstructions->mPageSetup.Height };

	for (int Axis = 0; Axis < 2; Axis++)
	{
		QDoubleSpinBox* SpinBox = mNoticeSizeSpinBoxes[Axis];
		const QSignalBlocker Blocker(SpinBox);
		const swInstructionsPageUnits Units = mNoticeSizeUnitButtons[Axis]->GetUnits(mInstructions);
		mNoticeSizeUnitButtons[Axis]->UpdateUnits(mInstructions);
		swSetupLengthSpinBox(SpinBox, Units, 1.0, PageLengths[Axis] * 5.0);
		SpinBox->setValue(qMax(0.0, Lengths[Axis]) / swGetPageUnitScale(Units));

		// automatic sizes show greyed
		SpinBox->setEnabled(SetSize.isValid());
		mNoticeSizeLabels[Axis]->setEnabled(SetSize.isValid());
	}
}

// use to show the selected image size
void swInstructionsPropertiesWidget::UpdateImageSize()
{
	const swInstructionsImage* Image = mImageKey ? mInstructions->FindImage(static_cast<int>(std::get<2>(*mImageKey))) : nullptr;
	const bool Edited = Image && mInstructions->GetElementSize(*mImageKey).isValid();
	const qreal PageWidth = mInstructions->mPageSetup.Width;
	const qreal Width = PageWidth * (Image ? Image->Size : swInstructionsImage().Size) / 100.0;
	const qreal Lengths[2] = { Width, Width * mImageAspect };

	for (int Axis = 0; Axis < 2; Axis++)
	{
		QDoubleSpinBox* SpinBox = mImageSizeSpinBoxes[Axis];
		const QSignalBlocker Blocker(SpinBox);
		const swInstructionsPageUnits Units = mImageSizeUnitButtons[Axis]->GetUnits(mInstructions);
		mImageSizeUnitButtons[Axis]->UpdateUnits(mInstructions);
		swSetupLengthSpinBox(SpinBox, Units, 1.0, PageWidth * 5.0 * (Axis == 0 ? 1.0 : mImageAspect));
		SpinBox->setValue(Lengths[Axis] / swGetPageUnitScale(Units));

		// default sizes show greyed
		SpinBox->setEnabled(Edited);
		mImageSizeLabels[Axis]->setEnabled(Edited);
	}
}

void swInstructionsPropertiesWidget::UpdateWidget()
{
	UpdateEditors();
	UpdateNoticeSize();
	UpdateImageSize();
	UpdatePageLayout();
	UpdateCategories();
}

void swInstructionsPropertiesWidget::UpdateEditors()
{
	for (int TypeIndex = 0; TypeIndex < static_cast<int>(swInstructionsPropertyType::Count); TypeIndex++)
	{
		const swInstructionsPropertyType Type = static_cast<swInstructionsPropertyType>(TypeIndex);
		const EditorWidgets& Editor = mEditors[TypeIndex];

		if (!Editor.Widget)
			continue;

		QSignalBlocker Blocker(Editor.Widget);

		switch (Editor.Type)
		{
		case EditorType::Bool:
			qobject_cast<QToolButton*>(Editor.Widget)->setChecked(swInstructions::IsPageProperty(Type) ? mInstructions->GetPageBoolProperty(mPageIndex, Type) : mInstructions->GetBoolProperty(Type, mModel, mStep));
			break;

		case EditorType::Length:
			{
				// units follow the page setup
				QDoubleSpinBox* SpinBox = qobject_cast<QDoubleSpinBox*>(Editor.Widget);
				const swInstructionsPageUnits Units = GetEditorUnits(Type);
				QSignalBlocker SpinBoxBlocker(SpinBox);

				swSetupLengthSpinBox(SpinBox, Units, Editor.Minimum, GetEditorMaximum(Editor));
				Editor.UnitButton->UpdateUnits(mInstructions);

				const double Value = GetIntValue(Type) / swGetPageUnitScale(Units);

				if (qRound(SpinBox->value() * swGetPageUnitScale(Units)) != GetIntValue(Type))
					SpinBox->setValue(Value);

				// fitted models grey their size

				if (Editor.FitButton)
				{
					const bool Fit = GetIntValue(Type) == 0;
					const QSignalBlocker FitBlocker(Editor.FitButton);
					Editor.FitButton->setChecked(Fit);
					SpinBox->setEnabled(!Fit);
					Editor.Label->setEnabled(!Fit);
					Editor.UnitButton->setEnabled(!Fit && !mInstructions->GetDocumentUnits());
				}
			}
			break;

		case EditorType::Percent:
			qobject_cast<QSpinBox*>(Editor.Widget)->setValue(GetIntValue(Type));
			break;

		case EditorType::Choice:
			{
				QComboBox* ComboBox = qobject_cast<QComboBox*>(Editor.Widget);
				ComboBox->setCurrentIndex(ComboBox->findData(mInstructions->GetIntProperty(Type, mModel, mStep)));
			}
			break;

		case EditorType::Color:
			{
				QToolButton* Button = qobject_cast<QToolButton*>(Editor.Widget);
				swUpdateColorButton(Button, GetColorValue(Type));
			}
			break;

		case EditorType::Font:
			{
				const QFont Font = mInstructions->GetFontProperty(Type, mModel, mStep);
				QSignalBlocker SizeBlocker(Editor.SizeSpinBox);

				const swInstructionsPageUnits Units = GetEditorUnits(Type);

				qobject_cast<QToolButton*>(Editor.Widget)->setText(Font.family());
				swSetupLengthSpinBox(Editor.SizeSpinBox, Units, Editor.Minimum, Editor.Maximum);
				Editor.UnitButton->UpdateUnits(mInstructions);
				if (!qFuzzyCompare(Editor.SizeSpinBox->value() * swGetPageUnitScale(Units), Font.pointSizeF()))
					Editor.SizeSpinBox->setValue(Font.pointSizeF() / swGetPageUnitScale(Units));
			}
			break;

		case EditorType::Text:
			{
				QLineEdit* LineEdit = qobject_cast<QLineEdit*>(Editor.Widget);
				QSignalBlocker LineEditBlocker(LineEdit);

				LineEdit->setText(mInstructions->GetTextProperty(Type));

				if (Editor.BrowseButton)
					swUpdateBrowseButton(Editor.BrowseButton, LineEdit->text());
			}
			break;
		}
	}

	// disable settings without effect
	auto GetBool = [this](swInstructionsPropertyType Type)
	{
		return mInstructions->GetBoolProperty(Type, mModel, mStep);
	};

	const bool UseDocumentFont = GetBool(swInstructionsPropertyType::UseDocumentFont);
	const bool UseDocumentFontSize = GetBool(swInstructionsPropertyType::UseDocumentFontSize);

	auto SetEnabled = [this, UseDocumentFont, UseDocumentFontSize](swInstructionsPropertyType Type, bool Enabled)
	{
		const EditorWidgets& Editors = mEditors[static_cast<int>(Type)];
		bool WidgetEnabled = Enabled;
		bool SizeEnabled = Enabled;

		// the document font greys out what it replaces
		if (Editors.Type == EditorType::Font)
		{
			const bool DocumentFont = Type == swInstructionsPropertyType::DocumentFont;
			WidgetEnabled = Enabled && UseDocumentFont == DocumentFont;
			SizeEnabled = Enabled && UseDocumentFontSize == DocumentFont;
		}

		Editors.Widget->setEnabled(WidgetEnabled);

		if (Editors.Label)
			Editors.Label->setEnabled(WidgetEnabled || SizeEnabled);

		if (Editors.UnitButton)
			Editors.UnitButton->setEnabled(SizeEnabled && !mInstructions->GetDocumentUnits());

		if (Editors.SizeSpinBox)
			Editors.SizeSpinBox->setEnabled(SizeEnabled);

		if (Editors.BrowseButton)
			Editors.BrowseButton->setEnabled(Enabled);

		if (Editors.SymbolButton)
			Editors.SymbolButton->setEnabled(WidgetEnabled);
	};

	const bool ShowStepNumber = GetBool(swInstructionsPropertyType::ShowStepNumber);
	const bool ShowStepDividers = GetBool(swInstructionsPropertyType::ShowStepDividers);

	SetEnabled(swInstructionsPropertyType::DocumentFont, true);
	SetEnabled(swInstructionsPropertyType::PointerTip, mInstructions->GetIntProperty(swInstructionsPropertyType::PointerStyle, mModel, mStep) != static_cast<int>(swInstructionsPointerStyle::Wedge));
	const bool ShowCoverPage = GetBool(swInstructionsPropertyType::ShowCoverPage);

	SetEnabled(swInstructionsPropertyType::ShowCoverBlankPage, ShowCoverPage);

	const bool ShowBackCover = GetBool(swInstructionsPropertyType::ShowBackCover);

	// publication texts follow their cover
	for (const swInstructionsPublicationElement& Publication : swGetPublicationElements())
	{
		const bool CoverShown = Publication.Back ? ShowBackCover : ShowCoverPage;
		const bool ShowText = CoverShown && GetBool(Publication.Show);

		SetEnabled(Publication.Show, ShowCoverPage || ShowBackCover);

		for (swInstructionsPublicationField Field : { swInstructionsPublicationField::Text, swInstructionsPublicationField::Font, swInstructionsPublicationField::Color, swInstructionsPublicationField::Alignment, swInstructionsPublicationField::VerticalAlignment })
			SetEnabled(swGetPublicationType(Publication, Field), ShowText);
	}

	SetEnabled(swInstructionsPropertyType::CoverBackgroundColor, ShowCoverPage);

	const bool ShowBillOfMaterials = GetBool(swInstructionsPropertyType::ShowBillOfMaterials);

	// materials share the count text
	const bool ShowPartCount = (ShowBillOfMaterials && GetBool(swInstructionsPropertyType::ShowBillOfMaterialsPartCount));
	const bool ShowPageCount = GetBool(swInstructionsPropertyType::ShowPagePartCount);

	SetEnabled(swInstructionsPropertyType::ShowBillOfMaterialsPartCount, ShowBillOfMaterials);
	SetEnabled(swInstructionsPropertyType::PartCountText, ShowPartCount || ShowPageCount || GetBool(swInstructionsPropertyType::ShowSubmodelPreview));
	SetEnabled(swInstructionsPropertyType::PartCountFile, ShowPartCount || ShowPageCount || GetBool(swInstructionsPropertyType::ShowSubmodelPreview));
	SetEnabled(swInstructionsPropertyType::PartCountFont, ShowPartCount);
	SetEnabled(swInstructionsPropertyType::PartCountColor, ShowPartCount);
	SetEnabled(swInstructionsPropertyType::PartCountAlignment, ShowPartCount);
	SetEnabled(swInstructionsPropertyType::PartCountVerticalAlignment, ShowPartCount);
	SetEnabled(swInstructionsPropertyType::PagePartCountFont, ShowPageCount || GetBool(swInstructionsPropertyType::ShowSubmodelPreview));
	SetEnabled(swInstructionsPropertyType::PagePartCountColor, ShowPageCount || GetBool(swInstructionsPropertyType::ShowSubmodelPreview));

	SetEnabled(swInstructionsPropertyType::ShowBillOfMaterialsBlankPage, ShowBillOfMaterials);
	SetEnabled(swInstructionsPropertyType::BillOfMaterialsShowPartIds, ShowBillOfMaterials);
	SetEnabled(swInstructionsPropertyType::BillOfMaterialsBackgroundColor, ShowBillOfMaterials);
	const bool ShowBillOfMaterialsTitle = ShowBillOfMaterials && GetBool(swInstructionsPropertyType::ShowBillOfMaterialsTitle);

	SetEnabled(swInstructionsPropertyType::ShowBillOfMaterialsTitle, ShowBillOfMaterials);
	SetEnabled(swInstructionsPropertyType::BillOfMaterialsTitleFont, ShowBillOfMaterialsTitle);
	SetEnabled(swInstructionsPropertyType::BillOfMaterialsTitleColor, ShowBillOfMaterialsTitle);
	SetEnabled(swInstructionsPropertyType::BillOfMaterialsTitleAlignment, ShowBillOfMaterialsTitle);
	SetEnabled(swInstructionsPropertyType::BillOfMaterialsTitleVerticalAlignment, ShowBillOfMaterialsTitle);
	SetEnabled(swInstructionsPropertyType::BillOfMaterialsTitleText, ShowBillOfMaterialsTitle);
	SetEnabled(swInstructionsPropertyType::BillOfMaterialsTitleFile, ShowBillOfMaterialsTitle);
	SetEnabled(swInstructionsPropertyType::BillOfMaterialsShowCounts, ShowBillOfMaterials);
	SetEnabled(swInstructionsPropertyType::BillOfMaterialsSortOrder, ShowBillOfMaterials);
	SetEnabled(swInstructionsPropertyType::BillOfMaterialsLayout, ShowBillOfMaterials);
	SetEnabled(swInstructionsPropertyType::BillOfMaterialsAlignment, ShowBillOfMaterials);
	SetEnabled(swInstructionsPropertyType::BillOfMaterialsFont, ShowBillOfMaterials);
	SetEnabled(swInstructionsPropertyType::BillOfMaterialsTextColor, ShowBillOfMaterials);
	SetEnabled(swInstructionsPropertyType::BillOfMaterialsThumbnailSize, ShowBillOfMaterials);
	SetEnabled(swInstructionsPropertyType::StepNumberFont, ShowStepNumber);
	SetEnabled(swInstructionsPropertyType::StepNumberColor, ShowStepNumber);
	SetEnabled(swInstructionsPropertyType::StepNumberAlignment, ShowStepNumber);
	SetEnabled(swInstructionsPropertyType::StepNumberVerticalAlignment, ShowStepNumber);
	SetEnabled(swInstructionsPropertyType::PLIFont, true);

	// rows on horizontal edges, columns vertical
	QComboBox* AlignmentComboBox = qobject_cast<QComboBox*>(mEditors[static_cast<int>(swInstructionsPropertyType::PLIAlignment)].Widget);
	const bool Columns = mInstructions->GetIntProperty(swInstructionsPropertyType::PLILayout, mModel, mStep) == static_cast<int>(swPartsListLayout::Columns);
	const QStringList AlignmentNames = Columns ? QStringList{ tr("Left"), tr("Center"), tr("Right") } : QStringList{ tr("Top"), tr("Middle"), tr("Bottom") };

	for (int Index = 0; Index < AlignmentNames.size(); Index++)
		AlignmentComboBox->setItemText(Index, AlignmentNames[Index]);

	// the bill of materials names its own
	QComboBox* BillAlignmentComboBox = qobject_cast<QComboBox*>(mEditors[static_cast<int>(swInstructionsPropertyType::BillOfMaterialsAlignment)].Widget);
	const bool BillColumns = mInstructions->GetIntProperty(swInstructionsPropertyType::BillOfMaterialsLayout, mModel, mStep) == static_cast<int>(swPartsListLayout::Columns);
	const QStringList BillAlignmentNames = BillColumns ? QStringList{ tr("Left"), tr("Center"), tr("Right") } : QStringList{ tr("Top"), tr("Middle"), tr("Bottom") };

	for (int Index = 0; Index < BillAlignmentNames.size(); Index++)
		BillAlignmentComboBox->setItemText(Index, BillAlignmentNames[Index]);
	SetEnabled(swInstructionsPropertyType::NoticeFont, true);
	SetEnabled(swInstructionsPropertyType::WatermarkFont, true);

	// image opacity needs a watermark image

	const std::vector<swInstructionsImage>& Images = mInstructions->GetImages();
	SetEnabled(swInstructionsPropertyType::WatermarkOpacity, std::any_of(Images.begin(), Images.end(), [](const swInstructionsImage& Image) { return Image.Pages == swInstructionsImagePages::Watermark; }));
	SetEnabled(swInstructionsPropertyType::StepDividerColor, ShowStepDividers);
	SetEnabled(swInstructionsPropertyType::PLIReserveArea, GetBool(swInstructionsPropertyType::ShowStepPLI));

	const bool ShowPageNumber = GetBool(swInstructionsPropertyType::ShowPageNumber);

	SetEnabled(swInstructionsPropertyType::ShowPageNumberAllPages, ShowPageNumber);
	SetEnabled(swInstructionsPropertyType::PageNumberFormat, ShowPageNumber);
	SetEnabled(swInstructionsPropertyType::PageNumberFont, ShowPageNumber);
	SetEnabled(swInstructionsPropertyType::PageNumberColor, ShowPageNumber);
	SetEnabled(swInstructionsPropertyType::PageNumberAlignment, ShowPageNumber);
	SetEnabled(swInstructionsPropertyType::PageNumberVerticalAlignment, ShowPageNumber);
	SetEnabled(swInstructionsPropertyType::StepDividerWidth, ShowStepDividers);
	SetEnabled(swInstructionsPropertyType::StepDividerMargin, ShowStepDividers);
	SetEnabled(swInstructionsPropertyType::StepDividerPadding, ShowStepDividers);
	SetEnabled(swInstructionsPropertyType::StepDividerRounded, ShowStepDividers);
	SetEnabled(swInstructionsPropertyType::StepDividerHorizontal, ShowStepDividers);
	SetEnabled(swInstructionsPropertyType::StepDividerVertical, ShowStepDividers);
	SetEnabled(swInstructionsPropertyType::StepDividerFinalModel, ShowStepDividers);

	const swInstructionsPageType PageType = GetPageType();
	QSignalBlocker Blocker(mThumbnailPageButton);

	mThumbnailPageButton->setEnabled(PageType == swInstructionsPageType::Steps || PageType == swInstructionsPageType::BillOfMaterials);
	mThumbnailApplyAllButton->setEnabled(mThumbnailPageButton->isEnabled());
	mThumbnailPageButton->setChecked(mInstructions->HasPageProperty(mPageIndex, swInstructionsPropertyType::PLIThumbnailSize));

	QSignalBlocker ModelSizeBlocker(mModelSizePageButton);

	mModelSizePageButton->setEnabled(PageType == swInstructionsPageType::Steps);
	mModelSizeApplyAllButton->setEnabled(mModelSizePageButton->isEnabled());
	mModelSizePageButton->setChecked(mInstructions->HasPageProperty(mPageIndex, swInstructionsPropertyType::StepImageSize));

	QSignalBlocker BackgroundBlocker(mBackgroundPageButton);

	mBackgroundPageButton->setChecked(mInstructions->HasPageProperty(mPageIndex, swInstructionsPropertyType::PageBackgroundColor));

	const bool BandsPage = PageType == swInstructionsPageType::Steps || PageType == swInstructionsPageType::Blank;

	const swInstructionsPropertyType SectionShow[4] = { swInstructionsPropertyType::ShowHeaderBand, swInstructionsPropertyType::ShowFooterBand, swInstructionsPropertyType::ShowLeftBand, swInstructionsPropertyType::ShowRightBand };

	for (int Section = 0; Section < 4; Section++)
	{
		QSignalBlocker BandsBlocker(mBandsPageButtons[Section]);

		// hidden bands grey their settings

		const bool ShowBand = mInstructions->GetPageBoolProperty(mPageIndex, SectionShow[Section]);
		const bool PageOnly = mInstructions->HasPageProperty(mPageIndex, SectionShow[Section]);

		// the color moved to the box group
		for (int Kind = 1; Kind < 4; Kind += 2)
			SetEnabled(static_cast<swInstructionsPropertyType>(static_cast<int>(SectionShow[0]) + Kind * 4 + Section), ShowBand);

		mBandsPageButtons[Section]->setEnabled(BandsPage && (ShowBand || PageOnly));
		mBandsApplyAllButtons[Section]->setEnabled(BandsPage);
		mBandsPageButtons[Section]->setChecked(PageOnly);
	}

	QSignalBlocker WatermarkBlocker(mWatermarkPageButton);

	mWatermarkPageButton->setChecked(mInstructions->HasPageProperty(mPageIndex, swInstructionsPropertyType::ShowWatermark));

	FitWidth();
}

void swInstructionsPropertiesWidget::FitWidth()
{
	// spin boxes take their widest value first
	ensurePolished();

	for (QAbstractSpinBox* SpinBox : mScrollArea->widget()->findChildren<QAbstractSpinBox*>())
		swUpdatePanelSpinBoxWidth(SpinBox);

	// widest row of every section, shown or folded
	int Width = 0;

	for (const CategoryWidgets& Category : mCategoryWidgets)
	{
		Width = qMax(Width, Category.Button->minimumSizeHint().width());

		for (const QWidget* Row : Category.Rows->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly))
			Width = qMax(Width, Row->minimumSizeHint().width());
	}

	// only grows so opening sections never shifts the page
	const int PanelWidth = Width + mScrollArea->verticalScrollBar()->sizeHint().width() + 2 * mScrollArea->frameWidth();

	if (PanelWidth > mScrollArea->minimumWidth())
		mScrollArea->setFixedWidth(PanelWidth);
}

swInstructionsPageUnits swInstructionsPropertiesWidget::GetEditorUnits(swInstructionsPropertyType Type) const
{
	return mEditors[static_cast<int>(Type)].UnitButton->GetUnits(mInstructions);
}

int swInstructionsPropertiesWidget::GetIntValue(swInstructionsPropertyType Type) const
{
	// show the current page properties
	if (swInstructions::IsPageProperty(Type))
		return mInstructions->GetPageIntProperty(mPageIndex, Type);

	return mInstructions->GetIntProperty(Type, mModel, mStep);
}

QColor swInstructionsPropertiesWidget::GetColorValue(swInstructionsPropertyType Type) const
{
	// submodel pages keep their own row

	if (Type == swInstructionsPropertyType::PageBackgroundColor && !mInstructions->HasPageProperty(mPageIndex, Type))
		return mInstructions->GetColorProperty(Type, nullptr, 0);

	// a clear final model takes the page background
	if (Type == swInstructionsPropertyType::FinalModelBackgroundColor && mInstructions->GetPageColorProperty(mPageIndex, Type).alpha() == 0)
		return mInstructions->GetBoxStyle(swInstructionsBoxElement::FinalModel, mModel, mStep).BackgroundColor;

	// a clear completion takes its page color
	if (Type == swInstructionsPropertyType::CompletionBoxBackgroundColor && mInstructions->GetColorProperty(Type, nullptr, 0).alpha() == 0)
		return mInstructions->GetBoxStyle(swInstructionsBoxElement::Completion, nullptr, 0).BackgroundColor;

	// a clear assembly or completion takes the page background
	if ((Type == swInstructionsPropertyType::AssemblyBackgroundColor || Type == swInstructionsPropertyType::CompletionBackgroundColor) && mInstructions->GetColorProperty(Type, nullptr, 0).alpha() == 0)
		return mInstructions->GetColorProperty(swInstructionsPropertyType::PageBackgroundColor, nullptr, 0);

	if (swInstructions::IsPageProperty(Type))
		return mInstructions->GetPageColorProperty(mPageIndex, Type);

	return mInstructions->GetColorProperty(Type, mModel, mStep);
}

void swInstructionsPropertiesWidget::SetIntValue(swInstructionsPropertyType Type, int Value)
{
	if (mInstructions->HasPageProperty(mPageIndex, Type))
		mInstructions->SetPageInt(mPageIndex, Type, Value);
	else
		mInstructions->SetDefaultInt(Type, Value);
}

void swInstructionsPropertiesWidget::SetColorValue(swInstructionsPropertyType Type, const QColor& Color)
{
	if (mInstructions->HasPageProperty(mPageIndex, Type))
		mInstructions->SetPageColor(mPageIndex, Type, Color);
	else
		mInstructions->SetDefaultColor(Type, Color);
}

void swInstructionsPropertiesWidget::SetThumbnailPageScope(bool PageOnly)
{
	mInstructions->SetPageScope(mPageIndex, swInstructionsPropertyType::PLIThumbnailSize, PageOnly);
}

void swInstructionsPropertiesWidget::DividerPresetClicked(int Preset)
{
	const swDividerPreset& DividerPreset = swDividerPresets[Preset];

	mInstructions->SetPageDividerPreset(mPageIndex, DividerPreset.Rows, DividerPreset.Columns);

	// uncheck the current preset
	UpdatePageLayout();
}

void swInstructionsPropertiesWidget::UpdatePageLayout()
{
	const swInstructionsPageSettings PageSettings = mInstructions->GetPageSettings(mPageIndex);

	QSignalBlocker RowsBlocker(mRowsSpinBox);
	QSignalBlocker ColumnsBlocker(mColumnsSpinBox);
	QSignalBlocker FlowBlocker(mStepFlowComboBox);
	QSignalBlocker FinalModelBlocker(mFinalModelButton);

	mRowsSpinBox->setValue(PageSettings.Rows);
	mColumnsSpinBox->setValue(PageSettings.Columns);
	mStepFlowComboBox->setCurrentIndex(static_cast<int>(PageSettings.Direction));
	mFinalModelButton->setChecked(PageSettings.FinalModel);

	// only pages ending a model have room for it
	bool HasLastStep = false;

	if (mPageIndex >= 0 && mPageIndex < static_cast<int>(mInstructions->mPages.size()))
	{
		for (const swInstructionsStep& Step : mInstructions->mPages[mPageIndex].Steps)
		{
			if (Step.FinalModel || Step.Step == Step.Model->GetLastStep())
			{
				HasLastStep = true;
			}
		}
	}

	mFinalModelButton->setEnabled(HasLastStep);

	// check the matching preset
	for (int Preset = 0; Preset < static_cast<int>(mDividerPresetButtons.size()); Preset++)
		mDividerPresetButtons[Preset]->setChecked(swDividerPresets[Preset].Rows == PageSettings.Rows && swDividerPresets[Preset].Columns == PageSettings.Columns);
}

swInstructionsPageSettings swInstructionsPropertiesWidget::GetPageLayoutSettings() const
{
	swInstructionsPageSettings PageSettings;

	PageSettings.Rows = mRowsSpinBox->value();
	PageSettings.Columns = mColumnsSpinBox->value();
	PageSettings.Direction = static_cast<swInstructionsDirection>(mStepFlowComboBox->currentIndex());
	PageSettings.FinalModel = mFinalModelButton->isChecked();

	return PageSettings;
}

swInstructionsPageType swInstructionsPropertiesWidget::GetPageType() const
{
	if (mPageIndex < 0 || mPageIndex >= static_cast<int>(mInstructions->mPages.size()))
		return swInstructionsPageType::Steps;

	return mInstructions->mPages[mPageIndex].Type;
}

void swInstructionsPropertiesWidget::UpdateCategories()
{
	// frozen pages keep their copy
	const bool Frozen = mPageIndex >= 0 && mInstructions->IsPageFrozen(static_cast<size_t>(mPageIndex));

	// fold arrow alone shows rows
	for (CategoryWidgets& Category : mCategoryWidgets)
	{
		Category.Rows->setVisible(Category.Button->IsExpanded());
		Category.Rows->setEnabled(!Frozen);
	}
}
