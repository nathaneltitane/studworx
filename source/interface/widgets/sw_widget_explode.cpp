#include "sw_global.h"
#include "sw_widget_explode.h"
#include "ui_sw_widget_explode.h"
#include "sw_window_main.h"
#include "sw_window_utilities.h"
#include "sw_object_part.h"

swExplodeWidget::swExplodeWidget(QWidget* Parent)
	: QWidget(Parent), ui(new Ui::swExplodeWidget)
{
	ui->setupUi(this);

	connect(ui->uniform, &QCheckBox::toggled, this, &swExplodeWidget::UpdateUniform);
	connect(ui->uniformAxis, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &swExplodeWidget::UpdateUniform);

	for (QSpinBox* Spread : { ui->spreadX, ui->spreadY, ui->spreadZ })
	{
		connect(Spread, QOverload<int>::of(&QSpinBox::valueChanged), this, &swExplodeWidget::UpdateUniform);
		connect(Spread, QOverload<int>::of(&QSpinBox::valueChanged), this, &swExplodeWidget::SettingsChanged);
	}

	connect(ui->anchor, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &swExplodeWidget::SettingsChanged);
	connect(ui->selectedOnly, &QCheckBox::toggled, this, &swExplodeWidget::SettingsChanged);
	connect(ui->showLines, &QCheckBox::toggled, this, &swExplodeWidget::SettingsChanged);

	// explode buttons share one row and expand evenly
	QWidget* Buttons = new QWidget(this);
	QHBoxLayout* ButtonsLayout = new QHBoxLayout(Buttons);
	ButtonsLayout->setContentsMargins(0, 0, 0, 0);
	ui->verticalLayout->addWidget(Buttons);
	ui->verticalLayout->addStretch(1);

	mExplodeButton = new QToolButton(Buttons);
	mExplodeButton->setText(tr("Explode"));
	mExplodeButton->setToolTip(tr("Flatten the submodels and spread the parts away from the anchor"));
	mExplodeButton->setWhatsThis(tr("<p><b>Explode</b></p><p>Spread the parts on screen away from the anchor with the settings above. Submodels, and the submodels inside them, flatten into their parts first, so everything spreads in one undo step. While they are spread, changing a setting spreads them again.</p>"));

	mCollapseButton = new QToolButton(Buttons);
	mCollapseButton->setText(tr("Collapse"));
	mCollapseButton->setToolTip(tr("Put the exploded parts back"));
	mCollapseButton->setWhatsThis(tr("<p><b>Collapse</b></p><p>Put the exploded parts back where they were and the flattened submodels back together, like Undo. Any edit to the model puts the parts back too and keeps the submodels flattened.</p>"));

	mSaveCopyButton = new QToolButton(Buttons);
	mSaveCopyButton->setText(tr("Save Copy..."));
	mSaveCopyButton->setToolTip(tr("Save the exploded model to a separate file"));
	mSaveCopyButton->setWhatsThis(tr("<p><b>Save Copy</b></p><p>Explode the parts and save the whole project to a separate file, named after the open file with -exploded before the extension. The open file keeps its name, Collapse puts its submodels back, and the copy cannot replace it.</p>"));

	for (QToolButton* Button : { mExplodeButton, mCollapseButton, mSaveCopyButton })
	{
		Button->setToolButtonStyle(Qt::ToolButtonTextOnly);
		swSetupPanelButton(Button);
		Button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
		Button->setMinimumWidth(0);
		ButtonsLayout->addWidget(Button);
	}

	connect(mExplodeButton, &QToolButton::clicked, this, [this]()
	{
		swModel* ActiveModel = gMainWindow->GetActiveModel();

		if (ActiveModel)
			ActiveModel->Explode(GetSettings());
	});

	connect(mCollapseButton, &QToolButton::clicked, this, []()
	{
		swModel* ActiveModel = gMainWindow->GetActiveModel();

		if (ActiveModel)
			ActiveModel->CollapseExplodeAction();
	});

	connect(mSaveCopyButton, &QToolButton::clicked, this, [this]()
	{
		swModel* ActiveModel = gMainWindow->GetActiveModel();

		if (ActiveModel)
			ActiveModel->SaveExplodedCopy(GetSettings());
	});

	swSetupWidgetHelp(this);
	UpdateUniform();
	UpdateAnchors();
}

swExplodeWidget::~swExplodeWidget()
{
	delete ui;
}

// use to list the anchors the selection offers, keeping the chosen kind
void swExplodeWidget::UpdateAnchors()
{
	const QVariant Current = ui->anchor->currentData();
	swModel* ActiveModel = gMainWindow->GetActiveModel();
	const QString Names[] = { tr("Focused Part: %1"), tr("First Selected: %1"), tr("Last Selected: %1") };
	QSignalBlocker Blocker(ui->anchor);

	ui->anchor->clear();

	for (int AnchorIndex = 0; ActiveModel && AnchorIndex < static_cast<int>(swExplodeAnchor::Count); AnchorIndex++)
	{
		const swPart* Anchor = ActiveModel->GetExplodeAnchor(static_cast<swExplodeAnchor>(AnchorIndex));

		if (Anchor)
			ui->anchor->addItem(Names[AnchorIndex].arg(Anchor->GetName()), AnchorIndex);
	}

	const int Index = ui->anchor->findData(Current);
	ui->anchor->setCurrentIndex(Index < 0 ? 0 : Index);

	const bool HasAnchor = ui->anchor->count() > 0;

	ui->anchor->setPlaceholderText(tr("No part is selected"));
	mExplodeButton->setEnabled(HasAnchor);
	mSaveCopyButton->setEnabled(HasAnchor);
	mCollapseButton->setEnabled(ActiveModel && (ActiveModel->IsExploded() || ActiveModel->IsExplodeFlattened()));
}

// use to make the other axes follow the uniform axis
void swExplodeWidget::UpdateUniform()
{
	QSpinBox* const Spreads[] = { ui->spreadX, ui->spreadY, ui->spreadZ };
	const bool Uniform = ui->uniform->isChecked();
	const QSpinBox* Source = Spreads[ui->uniformAxis->currentIndex()];

	ui->uniformAxis->setEnabled(Uniform);

	for (QSpinBox* Spread : Spreads)
	{
		Spread->setEnabled(!Uniform || Spread == Source);

		if (Uniform && Spread != Source)
			Spread->setValue(Source->value());
	}
}

// use to spread the parts again while they are exploded
void swExplodeWidget::SettingsChanged()
{
	swModel* ActiveModel = gMainWindow->GetActiveModel();

	if (ActiveModel && ActiveModel->IsExploded())
		ActiveModel->Explode(GetSettings());
}

swExplodeSettings swExplodeWidget::GetSettings() const
{
	swExplodeSettings Settings;

	Settings.Anchor = static_cast<swExplodeAnchor>(ui->anchor->currentData().toInt());
	Settings.Spread = swVector3(ui->spreadX->value(), ui->spreadY->value(), ui->spreadZ->value()) / 100.0f;
	Settings.SelectedOnly = ui->selectedOnly->isChecked();
	Settings.ShowLines = ui->showLines->isChecked();

	return Settings;
}
