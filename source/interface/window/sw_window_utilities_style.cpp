#include "sw_global.h"
#include "sw_window_utilities.h"

void swInterfaceStyle::drawControl(ControlElement Element, const QStyleOption* Option, QPainter* Painter, const QWidget* Widget) const
{
	const QStyleOptionComboBox* ComboBoxOption = qstyleoption_cast<const QStyleOptionComboBox*>(Option);

	if (Element != CE_ComboBoxLabel || !ComboBoxOption || ComboBoxOption->editable)
	{
		QProxyStyle::drawControl(Element, Option, Painter, Widget);
		return;
	}

	// center value and icon
	const QRect EditRect = subControlRect(CC_ComboBox, ComboBoxOption, SC_ComboBoxEditField, Widget).adjusted(1, 0, -1, 0);
	const QSize IconSize = ComboBoxOption->currentIcon.isNull() ? QSize() : ComboBoxOption->iconSize;
	const int Spacing = IconSize.isEmpty() ? 0 : pixelMetric(PM_ButtonIconSize, Option, Widget) / 4;
	const QString Text = ComboBoxOption->fontMetrics.elidedText(ComboBoxOption->currentText, Qt::ElideRight, EditRect.width() - IconSize.width() - Spacing);
	const int Width = IconSize.width() + Spacing + ComboBoxOption->fontMetrics.horizontalAdvance(Text);
	const int Left = EditRect.left() + qMax((EditRect.width() - Width) / 2, 0);

	Painter->save();
	Painter->setClipRect(EditRect);

	if (!IconSize.isEmpty())
	{
		const QIcon::Mode Mode = (ComboBoxOption->state & State_Enabled) ? QIcon::Normal : QIcon::Disabled;
		const QRect IconRect(Left, EditRect.top() + (EditRect.height() - IconSize.height()) / 2, IconSize.width(), IconSize.height());
		ComboBoxOption->currentIcon.paint(Painter, IconRect, Qt::AlignCenter, Mode);
	}

	const QRect TextRect(Left + IconSize.width() + Spacing, EditRect.top(), EditRect.right() - Left - IconSize.width() - Spacing + 1, EditRect.height());
	drawItemText(Painter, TextRect, Qt::AlignLeft | Qt::AlignVCenter, ComboBoxOption->palette, ComboBoxOption->state & State_Enabled, Text);

	Painter->restore();
}

int swInterfaceStyle::pixelMetric(PixelMetric Metric, const QStyleOption* Option, const QWidget* Widget) const
{
	// tab scroll arrows are as wide as panel buttons
	if (Metric == PM_TabBarScrollButtonWidth)
		return swGetPanelButtonSize().width() + 2;

	return QProxyStyle::pixelMetric(Metric, Option, Widget);
}

QRect swInterfaceStyle::subControlRect(ComplexControl Control, const QStyleOptionComplex* Option, SubControl Element, const QWidget* Widget) const
{
	QRect Rect = QProxyStyle::subControlRect(Control, Option, Element, Widget);

	// some fonts draw past the title rect

	if (Control == CC_GroupBox && Element == SC_GroupBoxLabel)
		Rect.setRight(Rect.right() + Option->fontMetrics.horizontalAdvance(QLatin1Char('M')));

	return Rect;
}

QSize swInterfaceStyle::sizeFromContents(ContentsType Type, const QStyleOption* Option, const QSize& Size, const QWidget* Widget) const
{
	QSize Result = QProxyStyle::sizeFromContents(Type, Option, Size, Widget);

	if (Type != CT_LineEdit && Type != CT_SpinBox && Type != CT_ComboBox)
		return Result;

	// spin box measures its field
	if (Type == CT_LineEdit && Widget && qobject_cast<const QAbstractSpinBox*>(Widget->parentWidget()))
		return Result;

	// inputs as tall as buttons
	QStyleOptionButton ButtonOption;
	ButtonOption.QStyleOption::operator=(*Option);
	ButtonOption.text = QLatin1String("X");

	const QSize TextSize(ButtonOption.fontMetrics.horizontalAdvance(ButtonOption.text), ButtonOption.fontMetrics.height());
	const int ButtonHeight = QProxyStyle::sizeFromContents(CT_PushButton, &ButtonOption, TextSize, nullptr).height();

	Result.setHeight(qMax(Result.height(), ButtonHeight));

	return Result;
}

void swInterfaceStyle::drawPrimitive(PrimitiveElement Element, const QStyleOption* Option, QPainter* Painter, const QWidget* Widget) const
{
	// custom theme draws its own tree branches, system styles keep theirs
	if (Element == PE_IndicatorBranch && mCustom)
	{
		drawBranch(Option, Painter);
		return;
	}

	// list check boxes stand out from the list

	if (Element == PE_IndicatorItemViewItemCheck && mCustom)
	{
		QStyleOption CheckOption(*Option);
		CheckOption.palette.setColor(QPalette::Base, Option->palette.color(QPalette::Window));
		QProxyStyle::drawPrimitive(Element, &CheckOption, Painter, Widget);
		return;
	}

	if (Element != PE_IndicatorTabClose)
	{
		QProxyStyle::drawPrimitive(Element, Option, Painter, Widget);
		return;
	}

	const bool Enabled = Option->state & State_Enabled;

	// shade behind the cross on hover and press
	if (Enabled && (Option->state & (State_MouseOver | State_Sunken)))
	{
		QColor Color = Option->palette.color(QPalette::WindowText);
		Color.setAlpha((Option->state & State_Sunken) ? 40 : 30);

		Painter->save();
		Painter->setRenderHint(QPainter::Antialiasing);
		Painter->setPen(Qt::NoPen);
		Painter->setBrush(Color);
		Painter->drawRoundedRect(Option->rect, 3, 3);
		Painter->restore();
	}

	swCreatePaintedIcon(swPaintedIcon::Close).paint(Painter, Option->rect.adjusted(2, 2, -2, -2), Qt::AlignCenter, Enabled ? QIcon::Normal : QIcon::Disabled);
}

// use to draw tree lines, and chevrons on parent rows
void swInterfaceStyle::drawBranch(const QStyleOption* Option, QPainter* Painter) const
{
	const QRect& Rect = Option->rect;
	const QPoint Center = Rect.center();

	QColor Color = Option->palette.color(QPalette::WindowText);
	Color.setAlpha(60);

	Painter->save();

	if (Option->state & State_Children)
	{
		const qreal Size = 3.0;
		const QPointF Middle = QRectF(Rect).center();
		QPainterPath Path;

		if (Option->state & State_Open)
		{
			Path.moveTo(Middle + QPointF(-Size, -Size / 2.0));
			Path.lineTo(Middle + QPointF(0.0, Size / 2.0));
			Path.lineTo(Middle + QPointF(Size, -Size / 2.0));
		}
		else
		{
			const qreal Direction = Option->direction == Qt::RightToLeft ? -1.0 : 1.0;

			Path.moveTo(Middle + QPointF(-Size / 2.0 * Direction, -Size));
			Path.lineTo(Middle + QPointF(Size / 2.0 * Direction, 0.0));
			Path.lineTo(Middle + QPointF(-Size / 2.0 * Direction, Size));
		}

		Painter->setRenderHint(QPainter::Antialiasing);
		Painter->strokePath(Path, QPen(Color, 2.0, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
	}
	else
	{
		Painter->setPen(Color);

		// rows with following siblings carry the line through
		if (Option->state & State_Sibling)
			Painter->drawLine(Center.x(), Rect.top(), Center.x(), Rect.bottom());

		if (Option->state & State_Item)
		{
			if (!(Option->state & State_Sibling))
				Painter->drawLine(Center.x(), Rect.top(), Center.x(), Center.y());

			if (Option->direction == Qt::RightToLeft)
				Painter->drawLine(Rect.left(), Center.y(), Center.x(), Center.y());
			else
				Painter->drawLine(Center.x(), Center.y(), Rect.right(), Center.y());
		}
	}

	Painter->restore();
}

QIcon swInterfaceStyle::standardIcon(StandardPixmap Pixmap, const QStyleOption* Option, const QWidget* Widget) const
{
	switch (Pixmap)
	{
	case SP_TitleBarCloseButton:
	case SP_DockWidgetCloseButton:
		return swCreatePaintedIcon(swPaintedIcon::Close);

	case SP_TitleBarNormalButton:
		return swCreatePaintedIcon(swPaintedIcon::Undock);

	default:
		break;
	}

	return QProxyStyle::standardIcon(Pixmap, Option, Widget);
}
