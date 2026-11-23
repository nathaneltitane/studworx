#pragma once

#include "sw_window_utilities.h"

// Spin box that clamps once typed.
template<typename SpinBoxType>
class swRangeSpinBoxBase : public SpinBoxType
{
public:
	explicit swRangeSpinBoxBase(QWidget* Parent)
		: SpinBoxType(Parent)
	{
		swSetupSpinBoxSelection(this);
	}

	// Shows the maximum after the value.
	void SetReference(const QString& Unit = QString())
	{
		this->setSuffix(QString(" / %1%2").arg(this->textFromValue(this->maximum()), Unit));
	}

	QValidator::State validate(QString& Text, int& Position) const override
	{
		const QValidator::State State = SpinBoxType::validate(Text, Position);

		if (State != QValidator::Invalid)
			return State;

		bool Valid = false;
		GetTypedValue(Text, Valid);

		return Valid ? QValidator::Intermediate : QValidator::Invalid;
	}

	void fixup(QString& Text) const override
	{
		bool Valid = false;
		const double Value = GetTypedValue(Text, Valid);

		if (Valid)
			Text = this->prefix() + this->textFromValue(static_cast<decltype(this->value())>(qBound<double>(this->minimum(), Value, this->maximum()))) + this->suffix();
		else
			SpinBoxType::fixup(Text);
	}

protected:
	// Reads the typed number.
	double GetTypedValue(const QString& Text, bool& Valid) const
	{
		QString Number = Text;

		if (!this->prefix().isEmpty() && Number.startsWith(this->prefix()))
			Number.remove(0, this->prefix().size());

		if (!this->suffix().isEmpty() && Number.endsWith(this->suffix()))
			Number.chop(this->suffix().size());

		return this->locale().toDouble(Number.trimmed(), &Valid);
	}
};

class swRangeSpinBox : public swRangeSpinBoxBase<QSpinBox>
{
public:
	using swRangeSpinBoxBase::swRangeSpinBoxBase;
};

class swRangeDoubleSpinBox : public swRangeSpinBoxBase<QDoubleSpinBox>
{
public:
	using swRangeSpinBoxBase::swRangeSpinBoxBase;
};

// The unit beside a spin box, like page units.
class swSpinBoxUnitLabel : public QToolButton
{
public:
	swSpinBoxUnitLabel(QAbstractSpinBox* SpinBox, const QString& Unit)
		: QToolButton(SpinBox->parentWidget())
	{
		setText(Unit.trimmed());
		setAccessibleName(Unit.trimmed());
		setToolButtonStyle(Qt::ToolButtonTextOnly);
		swSetupPanelButton(this);
		setFixedHeight(SpinBox->sizeHint().height());

		// short units share one width
		const int TextWidth = sizeHint().width();
		setText(QStringLiteral("fps"));
		const int ShortWidth = sizeHint().width();
		setText(Unit.trimmed());
		setFixedWidth(qMax(height(), qMax(TextWidth, ShortWidth)));
		setAttribute(Qt::WA_TransparentForMouseEvents);
		setEnabled(SpinBox->isEnabled());
		SpinBox->installEventFilter(this);
	}

protected:
	bool eventFilter(QObject* Object, QEvent* Event) override
	{
		// follows the spin box state
		if (Event->type() == QEvent::EnabledChange)
			setEnabled(static_cast<QWidget*>(Object)->isEnabled());
		else if (Event->type() == QEvent::ShowToParent)
			show();
		else if (Event->type() == QEvent::HideToParent)
			hide();

		return false;
	}
};

// use to show a unit beside a placed spin box
inline swSpinBoxUnitLabel* swAddSpinBoxUnit(QAbstractSpinBox* SpinBox, const QString& Unit)
{
	QWidget* Parent = SpinBox->parentWidget();
	QLayout* Layout = Parent ? Parent->layout() : nullptr;

	if (!Layout)
		return nullptr;

	QWidget* Row = new QWidget(Parent);
	Row->setSizePolicy(SpinBox->sizePolicy());
	QLayoutItem* Item = Layout->replaceWidget(SpinBox, Row);

	if (!Item)
	{
		delete Row;
		return nullptr;
	}

	delete Item;

	QHBoxLayout* RowLayout = new QHBoxLayout(Row);
	RowLayout->setContentsMargins(0, 0, 0, 0);
	swSpinBoxUnitLabel* Label = new swSpinBoxUnitLabel(SpinBox, Unit);
	RowLayout->addWidget(Label);
	RowLayout->addWidget(SpinBox, 1);

	return Label;
}
