#include "sw_global.h"
#include "sw_widget_keyframe.h"
#include "sw_window_utilities.h"

swKeyFrameWidget::swKeyFrameWidget(QWidget* Parent)
	: QCheckBox(Parent)
{
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	setFocusPolicy(Qt::NoFocus);
	setAttribute(Qt::WA_Hover);
	setCheckable(true);
}

QSize swKeyFrameWidget::sizeHint() const
{
	return swGetPanelButtonSize();
}

void swKeyFrameWidget::paintEvent(QPaintEvent* PaintEvent)
{
	Q_UNUSED(PaintEvent);

	QPainter Painter(this);

	// draw the keyframe button
	QStyleOptionToolButton Option;
	Option.initFrom(this);
	Option.subControls = QStyle::SC_ToolButton;
	Option.toolButtonStyle = Qt::ToolButtonIconOnly;
	Option.state |= QStyle::State_Raised;

	if (isDown())
		Option.state |= QStyle::State_Sunken;

	style()->drawComplexControl(QStyle::CC_ToolButton, &Option, &Painter, this);

	// small dot, filled when keyed

	Qt::CheckState State = checkState();
	Painter.setRenderHint(QPainter::Antialiasing);
	Painter.setPen(palette().color(QPalette::Text));

	if (State == Qt::Checked)
		Painter.setBrush(palette().color(QPalette::Text));
	else if (State == Qt::PartiallyChecked)
		Painter.setBrush(palette().color(QPalette::Mid));

	QRectF Rect(0, 0, 7, 7);
	Rect.moveCenter(QRectF(rect()).center());
	Painter.drawEllipse(Rect);
}
