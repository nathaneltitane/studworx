#include "sw_global.h"
#include "sw_dialog_color_edge.h"
#include "sw_window_utilities.h"
#include "sw_application.h"

swAutomateEdgeColorDialog::swAutomateEdgeColorDialog(QWidget* Parent, bool ShowHighContrastDialog, const swPreferences& Preferences)
	:QDialog(Parent)
{
	mStudCylinderColorEnabled = Preferences.mStudCylinderColorEnabled;
	mStudCylinderColor = Preferences.mStudCylinderColor;
	mPartEdgeColorEnabled = Preferences.mPartEdgeColorEnabled;
	mPartEdgeColor = Preferences.mPartEdgeColor;
	mBlackEdgeColorEnabled = Preferences.mBlackEdgeColorEnabled;
	mBlackEdgeColor = Preferences.mBlackEdgeColor;
	mDarkEdgeColorEnabled = Preferences.mDarkEdgeColorEnabled;
	mDarkEdgeColor = Preferences.mDarkEdgeColor;
	mPartEdgeContrast = Preferences.mPartEdgeContrast;
	mPartColorValueLDIndex = Preferences.mPartColorValueLDIndex;
	mAutomateEdgeColor = Preferences.mAutomateEdgeColor;

	setWindowTitle(tr("Edge Colors"));
	setWhatsThis(ShowHighContrastDialog ?
		tr("<p><b>High Contrast Edge Colors</b></p><p>Choose custom stud and edge colors for high contrast stud styles. Enable an override with its checkbox, then choose its color. Light to Dark sets the threshold for dark part edge colors. Automated edge colors take precedence over the part edge overrides.</p><p>Choose OK to return changes to Preferences, then choose OK in Preferences to keep them. Cancel discards changes in this dialog. Reset restores an individual control's default.</p>") :
		tr("<p><b>Automated Edge Colors</b></p><p>Adjust the contrast and saturation used to calculate part edge colors. Both controls range from 0.00 to 1.00. Reset restores a control to 0.50.</p><p>Choose OK to return changes to Preferences, then choose OK in Preferences to keep them. Cancel discards changes in this dialog.</p>"));
	QVBoxLayout* MainLayout = new QVBoxLayout(this);

	QGroupBox* EdgeSettingsBox = new QGroupBox(tr("Edge Colors"), this);
	MainLayout->addWidget(EdgeSettingsBox);
	mEdgeSettingsLayout = new QGridLayout(EdgeSettingsBox);

	// use to build a slider row with its value and reset button
	auto AddSliderRow = [this](int Row, const QString& Name, float Value, const QString& ToolTip, const QString& Help, const QString& ResetToolTip, QSlider*& Slider, QLabel*& ValueLabel, QToolButton*& ResetButton)
	{
		QLabel* NameLabel = new QLabel(Name, this);
		ValueLabel = new QLabel(this);
		ValueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		Slider = new QSlider(Qt::Horizontal, this);
		Slider->setRange(0, 100);
		Slider->setValue(Value * 100);
		Slider->setToolTip(ToolTip);
		Slider->setWhatsThis(Help);
		Slider->setAccessibleName(Name);
		NameLabel->setBuddy(Slider);
		NameLabel->setToolTip(ToolTip);
		NameLabel->setWhatsThis(Help);
		connect(Slider, &QSlider::valueChanged, this, &swAutomateEdgeColorDialog::SliderValueChanged);

		ResetButton = new QToolButton(this);
		ResetButton->setText(tr("Reset"));
		ResetButton->setToolTip(ResetToolTip);
		ResetButton->setWhatsThis(Help);
		connect(ResetButton, &QToolButton::clicked, this, &swAutomateEdgeColorDialog::ResetSliderButtonClicked);

		mEdgeSettingsLayout->addWidget(NameLabel, Row, 0);
		mEdgeSettingsLayout->addWidget(Slider, Row, 1);
		mEdgeSettingsLayout->addWidget(ValueLabel, Row, 2);
		mEdgeSettingsLayout->addWidget(ResetButton, Row, 3);

		mValueLabels.append(ValueLabel);
		mResetButtons.append(ResetButton);
		emit Slider->valueChanged(Slider->value());
	};

	int LDIndexRow = 0;
	PartEdgeContrast = nullptr;
	PartEdgeContrastSlider = nullptr;
	ResetPartEdgeContrastButton = nullptr;

	if (!ShowHighContrastDialog)
	{
		LDIndexRow = 1;

		AddSliderRow(0, tr("Contrast"), mPartEdgeContrast,
			tr("Set edge contrast from 0.00 to 1.00 (default 0.50)."),
			tr("<p><b>Edge Contrast</b></p><p>Adjust the contrast used by automated edge colors to distinguish edges from part surfaces. The range is 0.00 to 1.00; Reset restores 0.50.</p>"),
			tr("Restore edge contrast to 0.50."),
			PartEdgeContrastSlider, PartEdgeContrast, ResetPartEdgeContrastButton);
	}

	AddSliderRow(LDIndexRow, ShowHighContrastDialog ? tr("Light to Dark") : tr("Saturation"), mPartColorValueLDIndex,
		ShowHighContrastDialog ? tr("Set the light to dark threshold from 0.00 to 1.00 (default 0.50).") : tr("Set edge color saturation from 0.00 to 1.00 (default 0.50)."),
		ShowHighContrastDialog ?
			tr("<p><b>Light to Dark</b></p><p>Classify part colors by brightness to choose which parts receive the dark part edge override. Colors below the threshold are treated as dark. A separate enabled black part override takes priority for black parts. The range is 0.00 to 1.00; Reset restores 0.50.</p>") :
			tr("<p><b>Edge Saturation</b></p><p>Adjust the tint or shade used to calculate automated edge colors from part colors. The range is 0.00 to 1.00; Reset restores 0.50.</p>"),
		ShowHighContrastDialog ? tr("Restore the light to dark threshold to 0.50.") : tr("Restore edge color saturation to 0.50."),
		PartColorValueLDIndexSlider, PartColorValueLDIndex, ResetPartColorValueLDIndexButton);

	// slider takes the width
	mEdgeSettingsLayout->setColumnStretch(1, 1);

	QGroupBox* HighContrastColorBox = new QGroupBox(tr("High Contrast"), this);
	HighContrastColorBox->setVisible(ShowHighContrastDialog);
	MainLayout->addWidget(HighContrastColorBox);
	mHighContrastColorLayout = new QGridLayout(HighContrastColorBox);

	// use to build a color override row with its swatch and reset button
	auto AddColorRow = [this](int Row, const QString& Name, bool Enabled, quint32 Color, const QString& Help, QCheckBox*& CheckBox, QToolButton*& ColorButton, QToolButton*& ResetButton)
	{
		CheckBox = new QCheckBox(Name, this);
		CheckBox->setChecked(Enabled);
		CheckBox->setToolTip(tr("Enable the custom %1 color.").arg(Name));
		CheckBox->setWhatsThis(Help);
		connect(CheckBox, &QCheckBox::clicked, this, &swAutomateEdgeColorDialog::ColorCheckBoxClicked);

		ColorButton = new QToolButton(this);
		ColorButton->setEnabled(Enabled);
		ColorButton->setAccessibleName(Name);
		ColorButton->setWhatsThis(Help);
		ColorButton->setSizePolicy(QSizePolicy::Expanding, ColorButton->sizePolicy().verticalPolicy());
		SetColorButton(ColorButton, Color);
		connect(ColorButton, &QToolButton::clicked, this, &swAutomateEdgeColorDialog::ColorButtonClicked);

		ResetButton = new QToolButton(this);
		ResetButton->setText(tr("Reset"));
		ResetButton->setEnabled(Enabled);
		ResetButton->setToolTip(tr("Restore the default %1 color.").arg(Name));
		ResetButton->setWhatsThis(Help);
		connect(ResetButton, &QToolButton::clicked, this, &swAutomateEdgeColorDialog::ResetColorButtonClicked);

		mHighContrastColorLayout->addWidget(CheckBox, Row, 0);
		mHighContrastColorLayout->addWidget(ColorButton, Row, 1, 1, 2);
		mHighContrastColorLayout->addWidget(ResetButton, Row, 3);

		mResetButtons.append(ResetButton);
	};

	AddColorRow(0, tr("Stud Cylinder"), mStudCylinderColorEnabled, mStudCylinderColor,
		tr("Enable a custom color for stud cylinders in high contrast stud styles, then use the color button to choose it."),
		StudCylinderColorEnabledBox, StudCylinderColorButton, ResetStudCylinderColorButton);
	AddColorRow(1, tr("Edge"), mPartEdgeColorEnabled, mPartEdgeColor,
		tr("Enable a custom edge color for parts not covered by the black or dark overrides. Automated edge colors take precedence."),
		PartEdgeColorEnabledBox, PartEdgeColorButton, ResetPartEdgeColorButton);
	AddColorRow(2, tr("Black Edge"), mBlackEdgeColorEnabled, mBlackEdgeColor,
		tr("Enable a custom edge color for black parts. This override takes priority over the dark part and general edge colors. Automated edge colors take precedence."),
		BlackEdgeColorEnabledBox, BlackEdgeColorButton, ResetBlackEdgeColorButton);
	AddColorRow(3, tr("Dark Edge"), mDarkEdgeColorEnabled, mDarkEdgeColor,
		tr("Enable a custom edge color for parts below the light to dark threshold, except black parts with their own enabled override. Automated edge colors take precedence."),
		DarkEdgeColorEnabledBox, DarkEdgeColorButton, ResetDarkEdgeColorButton);

	mHighContrastColorLayout->setColumnStretch(1, 1);

	// keep the groups compact in the minimum dialog size
	MainLayout->addStretch(1);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	MainLayout->addWidget(buttonBox);
	swSetupDialogHelp(this);
	QObject::connect(buttonBox, &QDialogButtonBox::accepted, this, &swAutomateEdgeColorDialog::accept);
	QObject::connect(buttonBox, &QDialogButtonBox::rejected, this, &swAutomateEdgeColorDialog::reject);
}

// use to show a color on its swatch button
void swAutomateEdgeColorDialog::SetColorButton(QToolButton* Button, quint32 Color)
{
	const QColor ButtonColor(SW_RGBA_RED(Color), SW_RGBA_GREEN(Color), SW_RGBA_BLUE(Color));
	const int Size = Button->fontMetrics().height();
	QPixmap Pixmap(Size * 4, Size);

	Pixmap.fill(ButtonColor);
	Button->setIconSize(Pixmap.size());
	Button->setIcon(Pixmap);
	Button->setToolTip(ButtonColor.name().toUpper());
}

// use to share the label column and trailing widths once shown
void swAutomateEdgeColorDialog::showEvent(QShowEvent* Event)
{
	QDialog::showEvent(Event);

	const int Spacing = swGetLayoutSpacing(this, Qt::Horizontal);
	int LabelWidth = 0;

	for (QGridLayout* Layout : { mEdgeSettingsLayout, mHighContrastColorLayout })
		for (int Row = 0; Row < Layout->rowCount(); Row++)
			if (QLayoutItem* Item = Layout->itemAtPosition(Row, 0))
				LabelWidth = qMax(LabelWidth, Item->sizeHint().width());

	int ValueWidth = 0;

	for (QLabel* Label : mValueLabels)
		ValueWidth = qMax(ValueWidth, Label->fontMetrics().horizontalAdvance(swFormatDoubleLocalized(1.0, locale(), 'f', 2)));

	int ResetWidth = 0;

	for (QToolButton* Button : mResetButtons)
		ResetWidth = qMax(ResetWidth, Button->sizeHint().width());

	for (QGridLayout* Layout : { mEdgeSettingsLayout, mHighContrastColorLayout })
	{
		Layout->setColumnMinimumWidth(0, LabelWidth + Spacing * 2);
		Layout->setColumnMinimumWidth(2, ValueWidth);
		Layout->setColumnMinimumWidth(3, ResetWidth);
	}

	for (QToolButton* Button : mResetButtons)
		Button->setMinimumWidth(ResetWidth);
}

void swAutomateEdgeColorDialog::ColorCheckBoxClicked()
{
	QObject* CheckBox = sender();
	if (CheckBox == StudCylinderColorEnabledBox)
	{
		mStudCylinderColorEnabled = StudCylinderColorEnabledBox->isChecked();
		StudCylinderColorButton->setEnabled(mStudCylinderColorEnabled);
		ResetStudCylinderColorButton->setEnabled(mStudCylinderColorEnabled);
	}
	else if (CheckBox == PartEdgeColorEnabledBox)
	{
		mPartEdgeColorEnabled = PartEdgeColorEnabledBox->isChecked();
		PartEdgeColorButton->setEnabled(mPartEdgeColorEnabled);
		ResetPartEdgeColorButton->setEnabled(mPartEdgeColorEnabled);
	}
	else if (CheckBox == BlackEdgeColorEnabledBox)
	{
		mBlackEdgeColorEnabled = BlackEdgeColorEnabledBox->isChecked();
		BlackEdgeColorButton->setEnabled(mBlackEdgeColorEnabled);
		ResetBlackEdgeColorButton->setEnabled(mBlackEdgeColorEnabled);
	}
	else if (CheckBox == DarkEdgeColorEnabledBox)
	{
		mDarkEdgeColorEnabled = DarkEdgeColorEnabledBox->isChecked();
		DarkEdgeColorButton->setEnabled(mDarkEdgeColorEnabled);
		ResetDarkEdgeColorButton->setEnabled(mDarkEdgeColorEnabled);
	}
}

void swAutomateEdgeColorDialog::SliderValueChanged(int Value)
{
	if (sender() == PartEdgeContrastSlider)
	{
		mPartEdgeContrast = Value * 0.01f;
		PartEdgeContrast->setText(swFormatDoubleLocalized(mPartEdgeContrast, locale(), 'f', 2));
	}
	else if (sender() == PartColorValueLDIndexSlider)
	{
		mPartColorValueLDIndex = Value * 0.01f;
		PartColorValueLDIndex->setText(swFormatDoubleLocalized(mPartColorValueLDIndex, locale(), 'f', 2));
	}
}

void swAutomateEdgeColorDialog::ColorButtonClicked()
{
	QObject* Button = sender();
	QString Title;
	quint32* Color = nullptr;
	QColorDialog::ColorDialogOptions DialogOptions;

	if (Button == StudCylinderColorButton)
	{
		Title = tr("Select Stud Cylinder Color");
		Color = &mStudCylinderColor;
	}
	else if (Button == PartEdgeColorButton)
	{
		Title = tr("Select Part Edge Color");
		Color = &mPartEdgeColor;
	}
	else if (Button == BlackEdgeColorButton)
	{
		if (mAutomateEdgeColor)
		{
			QMessageBox msgBox;
			msgBox.setWindowTitle(tr("Select Black Edge Color"));
			msgBox.setText(tr("Automate edge color appears to be enabled.<br>Black parts edge color will not be accessible.<br>Do you want to continue?"));
			if (msgBox.exec() != QMessageBox::Accepted)
				return;
		}
		Title = tr("Select Black Edge Color");
		Color = &mBlackEdgeColor;
	}
	else if (Button == DarkEdgeColorButton)
	{
		if (mAutomateEdgeColor)
		{
			QMessageBox msgBox;
			msgBox.setWindowTitle(tr("Select Dark Edge Color"));
			msgBox.setText(tr("Automate edge color appears to be enabled.<br>Dark parts edge color will not be accessible.<br>Do you want to continue?"));
			if (msgBox.exec() != QMessageBox::Accepted)
				return;
		}
		Title = tr("Select Dark Edge Color");
		Color = &mDarkEdgeColor;
	}
	else
		return;

	QColor OldColor = QColor(SW_RGBA_RED(*Color), SW_RGBA_GREEN(*Color), SW_RGBA_BLUE(*Color), SW_RGBA_ALPHA(*Color));
	QColor NewColor = QColorDialog::getColor(OldColor, this, Title, DialogOptions);

	if (NewColor == OldColor || !NewColor.isValid())
		return;

	*Color = SW_RGBA(NewColor.red(), NewColor.green(), NewColor.blue(), NewColor.alpha());

	SetColorButton(static_cast<QToolButton*>(Button), *Color);
}

void swAutomateEdgeColorDialog::ResetSliderButtonClicked()
{
	if (sender() == ResetPartEdgeContrastButton)
	{
		PartEdgeContrastSlider->setValue(0.5f * 100);
	}
	else if (sender() == ResetPartColorValueLDIndexButton)
	{
		PartColorValueLDIndexSlider->setValue(0.5f * 100);
	}
}

void swAutomateEdgeColorDialog::ResetColorButtonClicked()
{
	quint32* Color = nullptr;

	if (sender() == ResetStudCylinderColorButton)
	{
		Color = &mStudCylinderColor;
		*Color = SW_RGBA(27, 42, 52, 255);
		SetColorButton(StudCylinderColorButton, *Color);
	}
	else if (sender() == ResetPartEdgeColorButton)
	{
		Color = &mPartEdgeColor;
		*Color = SW_RGBA(0, 0, 0, 255);
		SetColorButton(PartEdgeColorButton, *Color);
	}
	else if (sender() == ResetBlackEdgeColorButton)
	{
		Color = &mBlackEdgeColor;
		*Color = SW_RGBA(255, 255, 255, 255);
		SetColorButton(BlackEdgeColorButton, *Color);
	}
	else if (sender() == ResetDarkEdgeColorButton)
	{
		Color = &mDarkEdgeColor;
		*Color = SW_RGBA(27, 42, 52, 255);
		SetColorButton(DarkEdgeColorButton, *Color);
	}
}
