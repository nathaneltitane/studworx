#include "sw_global.h"
#include "sw_window_utilities.h"

// use to draw the section chevron
QIcon swCreateDisclosureIcon(bool Expanded, const QPalette& Palette)
{
	QPixmap Pixmap(16, 16);
	Pixmap.fill(Qt::transparent);

	QStyleOption Option;
	Option.rect = Pixmap.rect();
	Option.palette = Palette;
	Option.state = QStyle::State_Enabled;

	QPainter Painter(&Pixmap);
	QApplication::style()->drawPrimitive(Expanded ? QStyle::PE_IndicatorArrowDown : QStyle::PE_IndicatorArrowRight, &Option, &Painter);
	Painter.end();

	return QIcon(Pixmap);
}

// use to show a color swatch
QIcon swCreateColorIcon(const QColor& Color, int Size)
{
	QPixmap Pixmap(Size, Size);
	QPainter Painter(&Pixmap);

	const int CellSize = qMax(Size / 4, 2);

	for (int Y = 0; Y < Size; Y += CellSize)
		for (int X = 0; X < Size; X += CellSize)
			Painter.fillRect(X, Y, CellSize, CellSize, ((X + Y) / CellSize) % 2 ? QColor(204, 204, 204) : QColor(255, 255, 255));

	Painter.fillRect(Pixmap.rect(), Color);
	Painter.setPen(QColor(128, 128, 128));
	Painter.drawRect(Pixmap.rect().adjusted(0, 0, -1, -1));
	Painter.end();

	return QIcon(Pixmap);
}

// use to show a color button
void swUpdateColorButton(QToolButton* Button, const QColor& Color)
{
	Button->setIcon(swCreateColorIcon(Color, Button->iconSize().height()));
	Button->setText(Color.alpha() ? Color.name().toUpper() : QCoreApplication::translate("swInstructionsPropertiesWidget", "Transparent"));
}

// use to pick a color with alpha
QColor swPickColor(QToolButton* Button, const QColor& Color)
{
	QColor SelectedColor;

	QMenu Menu(Button);
	QWidgetAction* Action = new QWidgetAction(&Menu);
	swColorDialogPopup* Popup = new swColorDialogPopup(Color, true, &Menu);
	Action->setDefaultWidget(Popup);
	Menu.addAction(Action);

	QObject::connect(Popup, &swColorDialogPopup::ColorSelected, &Menu, [&SelectedColor](QColor PickedColor)
	{
		SelectedColor = PickedColor;
	});

	Menu.exec(Button->mapToGlobal(Button->rect().bottomLeft()));

	return SelectedColor;
}

// use to color icons with the text
static QColor swGetInterfaceIconColor(QIcon::Mode Mode)
{
	const QPalette Palette = QApplication::palette();

	switch (Mode)
	{
	case QIcon::Disabled:
		return Palette.color(QPalette::Disabled, QPalette::WindowText);

	case QIcon::Selected:
		return Palette.color(QPalette::Active, QPalette::HighlightedText);

	case QIcon::Normal:
	case QIcon::Active:
		break;
	}

	return Palette.color(QPalette::Active, QPalette::WindowText);
}

// use to lay out the options gear
static QPainterPath swCreateGearPath(const QSizeF& Size)
{
	const qreal Extent = qMin(Size.width(), Size.height());
	const qreal Scale = Extent / 32.0;
	const bool Small = Extent <= 16.0;
	const int ToothCount = Small ? 6 : 8;
	const qreal RadiusOuter = 14.5 * Scale;
	const qreal RadiusRoot = 10.5 * Scale;
	const qreal RadiusHole = (Small ? 5.5 : 4.5) * Scale;
	const qreal AngleTip = Small ? 0.26 : 0.19;
	const qreal AngleBase = Small ? 0.40 : 0.30;
	const QPointF Center(Size.width() / 2.0, Size.height() / 2.0);

	QPolygonF Outline;

	for (int Tooth = 0; Tooth < ToothCount; Tooth++)
	{
		const qreal Angle = Tooth * 2.0 * M_PI / ToothCount;

		for (const QPointF& Corner : { QPointF(RadiusRoot, -AngleBase), QPointF(RadiusOuter, -AngleTip), QPointF(RadiusOuter, AngleTip), QPointF(RadiusRoot, AngleBase) })
			Outline << Center + QPointF(qCos(Angle + Corner.y()) * Corner.x(), qSin(Angle + Corner.y()) * Corner.x());
	}

	QPainterPath Path;
	Path.addPolygon(Outline);
	Path.closeSubpath();
	Path.addEllipse(Center, RadiusHole, RadiusHole);
	Path.setFillRule(Qt::OddEvenFill);

	return Path;
}

// use to lay out the open folder
static QPainterPath swCreateFolderPath(const QSizeF& Size)
{
	const qreal Extent = qMin(Size.width(), Size.height());
	const qreal Unit = Extent / 16.0;
	const QPointF Origin((Size.width() - Extent) / 2.0, (Size.height() - Extent) / 2.0);
	const auto Snap = [Unit](qreal Value) { return qreal(qRound(Value * Unit)); };
	const qreal Radius = qRound(Unit) * 0.5;
	const qreal Left = Snap(1);
	const qreal Right = Snap(15);
	const qreal TabTop = Snap(2);
	const qreal TabRight = Snap(6);
	const qreal BackTop = Snap(3);
	const qreal GapTop = Snap(5);
	const qreal FrontTop = qMax(Snap(6), GapTop + 1.0);
	const qreal Bottom = Snap(14);

	// gap between tab and front
	QPainterPath Tab;
	Tab.addRoundedRect(QRectF(Left, TabTop, TabRight - Left, GapTop - TabTop), Radius, Radius);

	QPainterPath Back;
	Back.addRoundedRect(QRectF(Left, BackTop, Right - Left, GapTop - BackTop), Radius, Radius);

	QPainterPath Front;
	Front.addRoundedRect(QRectF(Left, FrontTop, Right - Left, Bottom - FrontTop), Radius, Radius);

	return Tab.united(Back).united(Front).translated(Origin);
}

// use to build an open line
static QPainterPath swCreateIconLine(std::initializer_list<QPointF> Points, bool Closed = false)
{
	QPainterPath Path;
	QPolygonF Polygon(Points);

	Path.addPolygon(Polygon);

	if (Closed)
		Path.closeSubpath();

	return Path;
}

static QPainterPath swCreateIconCircle(qreal X, qreal Y, qreal Radius)
{
	QPainterPath Path;
	Path.addEllipse(QPointF(X, Y), Radius, Radius);

	return Path;
}

static QPainterPath swCreateIconRect(qreal Left, qreal Top, qreal Right, qreal Bottom, qreal Radius = 0.0)
{
	QPainterPath Path;
	Path.addRoundedRect(QRectF(Left, Top, Right - Left, Bottom - Top), Radius, Radius);

	return Path;
}

// use to point an open arrow head
static QPainterPath swCreateIconArrowHead(const QPointF& Tip, qreal Direction, qreal Length = 4.0)
{
	const qreal Back = qDegreesToRadians(Direction + 180.0);
	const qreal Spread = qDegreesToRadians(45.0);

	return swCreateIconLine({ Tip + QPointF(qCos(Back - Spread), qSin(Back - Spread)) * Length, Tip, Tip + QPointF(qCos(Back + Spread), qSin(Back + Spread)) * Length });
}

static QPainterPath swCreateIconArrow(const QPointF& Start, const QPointF& End, qreal Length = 4.0)
{
	const QPointF Delta = End - Start;
	QPainterPath Path = swCreateIconLine({ Start, End });

	Path.addPath(swCreateIconArrowHead(End, qRadiansToDegrees(qAtan2(Delta.y(), Delta.x())), Length));

	return Path;
}

// use to sweep an elliptic arc
static QPainterPath swCreateIconArc(qreal X, qreal Y, qreal RadiusX, qreal RadiusY, qreal Start, qreal Sweep, qreal Tilt = 0.0, qreal HeadLength = 0.0)
{
	QPainterPath Path;
	const QRectF Rect(-RadiusX, -RadiusY, RadiusX * 2.0, RadiusY * 2.0);

	Path.arcMoveTo(Rect, -Start);
	Path.arcTo(Rect, -Start, -Sweep);

	if (HeadLength > 0.0)
	{
		const qreal End = qDegreesToRadians(Start + Sweep);
		const QPointF Tip(qCos(End) * RadiusX, qSin(End) * RadiusY);
		const QPointF Tangent = QPointF(-qSin(End) * RadiusX, qCos(End) * RadiusY) * (Sweep < 0.0 ? -1.0 : 1.0);

		Path.addPath(swCreateIconArrowHead(Tip, qRadiansToDegrees(qAtan2(Tangent.y(), Tangent.x())), HeadLength));
	}

	QTransform Transform;
	Transform.translate(X, Y);
	Transform.rotate(Tilt);

	return Transform.map(Path);
}

// use to draw a painted icon shape
class swIconPainter
{
public:
	swIconPainter(QPainter* Painter, const QColor& Color)
		: mPainter(Painter), mColor(Color)
	{
	}

	void Stroke(const QPainterPath& Path, qreal Width = 2.0)
	{
		mPainter->strokePath(Path, QPen(mColor, Width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	}

	// round joins on filled shapes
	void Fill(const QPainterPath& Path)
	{
		mPainter->fillPath(Path, mColor);
		Stroke(Path, 1.0);
	}

	void Erase(const QPainterPath& Path, qreal Width = 0.0)
	{
		const QPainter::CompositionMode Mode = mPainter->compositionMode();

		mPainter->setCompositionMode(QPainter::CompositionMode_Clear);

		if (Width > 0.0)
			mPainter->strokePath(Path, QPen(Qt::black, Width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
		else
			mPainter->fillPath(Path, Qt::black);

		mPainter->setCompositionMode(Mode);
	}

protected:
	QPainter* mPainter;
	QColor mColor;
};

// use to draw a centered painted icon
static void swDrawPaintedIcon(QPainter* Painter, swPaintedIcon Icon, const QSizeF& Size, const QColor& Color)
{
	Painter->save();
	Painter->setRenderHint(QPainter::Antialiasing);

	if (Icon == swPaintedIcon::Gear || Icon == swPaintedIcon::Folder)
	{
		Painter->fillPath(Icon == swPaintedIcon::Gear ? swCreateGearPath(Size) : swCreateFolderPath(Size), Color);
		Painter->restore();

		return;
	}

	const qreal Extent = qMin(Size.width(), Size.height());

	Painter->translate((Size.width() - Extent) / 2.0, (Size.height() - Extent) / 2.0);
	Painter->scale(Extent / 24.0, Extent / 24.0);

	swIconPainter Draw(Painter, Color);

	switch (Icon)
	{
	case swPaintedIcon::Gear:
	case swPaintedIcon::Folder:
	case swPaintedIcon::Count:
		break;

	case swPaintedIcon::FileNew:
		Draw.Stroke(swCreateIconLine({ { 14, 3 }, { 6, 3 }, { 6, 21 }, { 19, 21 }, { 19, 8 }, { 14, 3 }, { 14, 8 }, { 19, 8 } }));
		Draw.Stroke(swCreateIconLine({ { 12.5, 11 }, { 12.5, 17 } }));
		Draw.Stroke(swCreateIconLine({ { 9.5, 14 }, { 15.5, 14 } }));
		break;

	case swPaintedIcon::FileSave:
		Draw.Stroke(swCreateIconLine({ { 4, 4 }, { 16, 4 }, { 20, 8 }, { 20, 20 }, { 4, 20 } }, true));
		Draw.Stroke(swCreateIconLine({ { 8, 4 }, { 8, 9 }, { 15, 9 }, { 15, 4 } }));
		Draw.Stroke(swCreateIconLine({ { 7, 20 }, { 7, 14 }, { 17, 14 }, { 17, 20 } }));
		break;

	case swPaintedIcon::EditUndo:
	case swPaintedIcon::EditRedo:
	{
		QPainterPath Path = swCreateIconLine({ { 9, 4 }, { 4, 9 }, { 9, 14 } });
		Path.addPath(swCreateIconLine({ { 4, 9 }, { 14.5, 9 } }));
		Path.addPath(swCreateIconArc(14.5, 14.5, 5.5, 5.5, -90, 180));
		Path.addPath(swCreateIconLine({ { 14.5, 20 }, { 10, 20 } }));

		// redo mirrors undo
		if (Icon == swPaintedIcon::EditRedo)
			Path = QTransform(-1, 0, 0, 1, 24, 0).map(Path);

		Draw.Stroke(Path);
		break;
	}

	case swPaintedIcon::ActionInsert:
		Draw.Fill(swCreateIconCircle(12, 12, 10));
		Draw.Erase(swCreateIconLine({ { 12, 7 }, { 12, 17 } }), 2.5);
		Draw.Erase(swCreateIconLine({ { 7, 12 }, { 17, 12 } }), 2.5);
		break;

	case swPaintedIcon::ActionSelect:
		Draw.Fill(swCreateIconLine({ { 5, 4 }, { 20, 11 }, { 13, 13 }, { 11, 20 } }, true));
		Draw.Stroke(swCreateIconLine({ { 5, 4 }, { 20, 11 }, { 13, 13 }, { 11, 20 } }, true));
		break;

	case swPaintedIcon::ActionMove:
		Draw.Stroke(swCreateIconArrow({ 9, 15 }, { 9, 3 }));
		Draw.Stroke(swCreateIconArrow({ 9, 15 }, { 21, 15 }));
		Draw.Stroke(swCreateIconArrow({ 9, 15 }, { 3, 21 }, 3.5));
		break;

	case swPaintedIcon::ActionRotate:
		Draw.Stroke(swCreateIconArc(12, 12, 8.5, 8.5, -60, 300, 0, 4));
		Draw.Fill(swCreateIconCircle(12, 12, 2));
		break;

	case swPaintedIcon::ActionErase:
		Draw.Stroke(swCreateIconLine({ { 3, 6 }, { 21, 6 } }));
		Draw.Stroke(swCreateIconLine({ { 9, 6 }, { 9, 3 }, { 15, 3 }, { 15, 6 } }));
		Draw.Stroke(swCreateIconLine({ { 5, 6 }, { 6, 21 }, { 18, 21 }, { 19, 6 } }));
		Draw.Stroke(swCreateIconLine({ { 10, 10 }, { 10, 17 } }));
		Draw.Stroke(swCreateIconLine({ { 14, 10 }, { 14, 17 } }));
		break;

	case swPaintedIcon::ActionPaint:
		Draw.Fill(swCreateIconRect(10, 2, 14, 9, 2));
		Draw.Fill(swCreateIconRect(5, 10, 19, 21, 1));
		Draw.Erase(swCreateIconLine({ { 5, 13 }, { 19, 13 } }), 1.5);
		Draw.Erase(swCreateIconLine({ { 9.5, 17 }, { 9.5, 23 } }), 1.5);
		Draw.Erase(swCreateIconLine({ { 14.5, 17 }, { 14.5, 23 } }), 1.5);
		break;

	case swPaintedIcon::ActionJoints:
		Draw.Stroke(swCreateIconLine({ { 5, 20 }, { 12, 12 }, { 19, 14 } }), 2.5);
		Draw.Fill(swCreateIconCircle(12, 12, 3));
		Draw.Fill(swCreateIconCircle(5, 20, 2));
		Draw.Fill(swCreateIconCircle(19, 14, 2));
		break;

	case swPaintedIcon::ActionPickerColor:
		Draw.Stroke(swCreateIconLine({ { 13, 11 }, { 5, 19 } }));
		Draw.Fill(swCreateIconCircle(4, 20, 1.5));
		Draw.Stroke(swCreateIconLine({ { 9.5, 8.5 }, { 15.5, 14.5 } }), 2.5);
		Draw.Stroke(swCreateIconLine({ { 14, 10 }, { 18, 6 } }), 6);
		break;

	case swPaintedIcon::ActionZoom:
		Draw.Stroke(swCreateIconCircle(10, 10, 7));
		Draw.Stroke(swCreateIconLine({ { 15.5, 15.5 }, { 21, 21 } }), 3);
		Draw.Stroke(swCreateIconLine({ { 10, 7 }, { 10, 13 } }));
		Draw.Stroke(swCreateIconLine({ { 7, 10 }, { 13, 10 } }));
		break;

	case swPaintedIcon::ActionZoomRegion:
		Draw.Stroke(swCreateIconLine({ { 3, 8 }, { 3, 3 }, { 8, 3 } }));
		Draw.Stroke(swCreateIconLine({ { 16, 3 }, { 21, 3 }, { 21, 8 } }));
		Draw.Stroke(swCreateIconLine({ { 3, 16 }, { 3, 21 }, { 8, 21 } }));
		Draw.Stroke(swCreateIconCircle(12, 12, 4.5));
		Draw.Stroke(swCreateIconLine({ { 15.5, 15.5 }, { 21, 21 } }), 3);
		break;

	case swPaintedIcon::ActionPan:
	{
		QPainterPath Fingers;
		Fingers.addPath(swCreateIconLine({ { 7.5, 13 }, { 7.5, 7 } }));
		Fingers.addPath(swCreateIconLine({ { 11, 11 }, { 11, 4 } }));
		Fingers.addPath(swCreateIconLine({ { 14.5, 11 }, { 14.5, 5 } }));
		Fingers.addPath(swCreateIconLine({ { 18, 12 }, { 18, 8 } }));
		Fingers.addPath(swCreateIconLine({ { 7.5, 17 }, { 4, 13 } }));
		Draw.Stroke(Fingers, 3);

		QPainterPath Palm;
		Palm.moveTo(6, 13);

		// palm follows the little finger
		Palm.lineTo(19, 11);
		Palm.lineTo(19, 18);
		Palm.cubicTo(19, 20, 18, 21, 16, 21);
		Palm.lineTo(12, 21);
		Palm.cubicTo(9.5, 21, 8, 19.5, 6, 17);
		Palm.closeSubpath();
		Draw.Fill(Palm);
		break;
	}

	case swPaintedIcon::ActionCamera:
		Draw.Stroke(swCreateIconRect(3, 10, 15, 20, 1.5));
		Draw.Stroke(swCreateIconLine({ { 15, 13.5 }, { 21, 10.5 }, { 21, 19.5 }, { 15, 16.5 } }, true));
		Draw.Stroke(swCreateIconCircle(6.5, 6, 2.5));
		Draw.Stroke(swCreateIconCircle(12.5, 6, 2.5));
		break;

	case swPaintedIcon::ActionViewRotate:
		Draw.Stroke(swCreateIconArc(12, 12, 10, 4.5, 20, 300, -35, 3.5));
		Draw.Fill(swCreateIconCircle(12, 12, 3));
		break;

	case swPaintedIcon::ActionRoll:
		Draw.Stroke(swCreateIconArc(12, 12, 8.5, 8.5, 200, 150, 0, 3.5));
		Draw.Stroke(swCreateIconArc(12, 12, 8.5, 8.5, 20, 150, 0, 3.5));
		break;

	case swPaintedIcon::LightPoint:
	{
		QPainterPath Bulb;
		Bulb.moveTo(9, 16);
		Bulb.cubicTo(9, 14, 5.5, 13, 5.5, 9.5);
		Bulb.arcTo(QRectF(5.5, 3, 13, 13), 180, -180);
		Bulb.cubicTo(18.5, 13, 15, 14, 15, 16);
		Bulb.closeSubpath();
		Draw.Stroke(Bulb);
		Draw.Stroke(swCreateIconLine({ { 9, 19 }, { 15, 19 } }));
		Draw.Stroke(swCreateIconLine({ { 10, 22 }, { 14, 22 } }));
		break;
	}

	case swPaintedIcon::LightSpot:
		Draw.Fill(swCreateIconRect(10, 2, 14, 5, 0.5));
		Draw.Stroke(swCreateIconLine({ { 10, 5 }, { 5, 13 }, { 19, 13 }, { 14, 5 } }, true));
		Draw.Stroke(swCreateIconLine({ { 12, 16 }, { 12, 21 } }));
		Draw.Stroke(swCreateIconLine({ { 7.5, 16 }, { 5.5, 20 } }));
		Draw.Stroke(swCreateIconLine({ { 16.5, 16 }, { 18.5, 20 } }));
		break;

	case swPaintedIcon::LightDirectional:
		Draw.Stroke(swCreateIconCircle(12, 12, 4));

		for (int Ray = 0; Ray < 8; Ray++)
		{
			const qreal Angle = qDegreesToRadians(Ray * 45.0);
			const QPointF Direction(qCos(Angle), qSin(Angle));

			Draw.Stroke(swCreateIconLine({ QPointF(12, 12) + Direction * 7.5, QPointF(12, 12) + Direction * 9.5 }));
		}
		break;

	case swPaintedIcon::LightArea:
		Draw.Stroke(swCreateIconRect(3, 3, 21, 10, 1));
		Draw.Stroke(swCreateIconLine({ { 7, 14 }, { 6, 20 } }));
		Draw.Stroke(swCreateIconLine({ { 12, 14 }, { 12, 21 } }));
		Draw.Stroke(swCreateIconLine({ { 17, 14 }, { 18, 20 } }));
		break;

	case swPaintedIcon::TimeFirst:
		Draw.Stroke(swCreateIconLine({ { 5, 5 }, { 5, 19 } }), 2.5);
		Draw.Fill(swCreateIconLine({ { 19, 5 }, { 19, 19 }, { 9, 12 } }, true));
		break;

	case swPaintedIcon::TimePrevious:
		Draw.Fill(swCreateIconLine({ { 15, 5 }, { 15, 19 }, { 5, 12 } }, true));
		Draw.Stroke(swCreateIconLine({ { 19, 5 }, { 19, 19 } }), 2.5);
		break;

	case swPaintedIcon::TimeNext:
		Draw.Stroke(swCreateIconLine({ { 5, 5 }, { 5, 19 } }), 2.5);
		Draw.Fill(swCreateIconLine({ { 9, 5 }, { 9, 19 }, { 19, 12 } }, true));
		break;

	case swPaintedIcon::TimeLast:
		Draw.Fill(swCreateIconLine({ { 5, 5 }, { 5, 19 }, { 15, 12 } }, true));
		Draw.Stroke(swCreateIconLine({ { 19, 5 }, { 19, 19 } }), 2.5);
		break;

	case swPaintedIcon::TimePlay:
		Draw.Fill(swCreateIconLine({ { 7, 5 }, { 7, 19 }, { 18, 12 } }, true));
		break;

	case swPaintedIcon::TimePause:
		Draw.Stroke(swCreateIconLine({ { 8, 5 }, { 8, 19 } }), 2.5);
		Draw.Stroke(swCreateIconLine({ { 16, 5 }, { 16, 19 } }), 2.5);
		break;

	case swPaintedIcon::TimeStop:
		Draw.Fill(swCreateIconRect(6, 6, 18, 18, 1));
		break;

	case swPaintedIcon::TimeKeysAdd:
		Draw.Stroke(swCreateIconLine({ { 10, 8 }, { 16, 14 }, { 10, 20 }, { 4, 14 } }, true));
		Draw.Stroke(swCreateIconLine({ { 18, 2 }, { 18, 9 } }));
		Draw.Stroke(swCreateIconLine({ { 14.5, 5.5 }, { 21.5, 5.5 } }));
		break;

	case swPaintedIcon::TimeKeysSet:
		Draw.Fill(swCreateIconLine({ { 12, 4 }, { 20, 12 }, { 12, 20 }, { 4, 12 } }, true));
		break;

	case swPaintedIcon::TimeKeysRemove:
		Draw.Stroke(swCreateIconLine({ { 12, 5 }, { 19, 12 }, { 12, 19 }, { 5, 12 } }, true));
		break;

	case swPaintedIcon::TransformTranslationAbsolute:
		Draw.Stroke(swCreateIconLine({ { 4, 3 }, { 4, 20 }, { 21, 20 } }));
		Draw.Stroke(swCreateIconArrow({ 4, 20 }, { 17, 7 }));
		break;

	case swPaintedIcon::TransformTranslationRelative:
		Draw.Stroke(swCreateIconRect(3, 15, 9, 21, 1));
		Draw.Stroke(swCreateIconArrow({ 9, 15 }, { 20, 4 }));
		break;

	case swPaintedIcon::TransformRotationAbsolute:
		Draw.Stroke(swCreateIconLine({ { 4, 3 }, { 4, 20 }, { 21, 20 } }));
		Draw.Stroke(swCreateIconArc(4, 20, 13, 13, 0, -80, 0, 3.5));
		break;

	case swPaintedIcon::TransformRotationRelative:
		Draw.Stroke(QTransform().translate(9, 15).rotate(-20).map(swCreateIconRect(-4, -4, 4, 4, 1)));
		Draw.Stroke(swCreateIconArc(9, 15, 11, 11, 15, -100, 0, 3.5));
		break;

	case swPaintedIcon::SnapMove:
		Draw.Fill(swCreateIconRect(9, 5, 15, 11, 1));
		Draw.Stroke(swCreateIconLine({ { 3, 16 }, { 21, 16 } }));
		Draw.Stroke(swCreateIconLine({ { 4, 16 }, { 4, 21 } }));
		Draw.Stroke(swCreateIconLine({ { 12, 16 }, { 12, 21 } }));
		Draw.Stroke(swCreateIconLine({ { 20, 16 }, { 20, 21 } }));
		Draw.Stroke(swCreateIconLine({ { 8, 16 }, { 8, 19 } }));
		Draw.Stroke(swCreateIconLine({ { 16, 16 }, { 16, 19 } }));
		break;

	case swPaintedIcon::SnapAngle:
		Draw.Stroke(swCreateIconLine({ { 4, 3 }, { 4, 20 }, { 21, 20 } }));

		for (int Step = 0; Step < 3; Step++)
			Draw.Stroke(swCreateIconArc(4, 20, 12, 12, -90 + Step * 33, 20));
		break;

	case swPaintedIcon::StatusWarning:
		Draw.Fill(swCreateIconLine({ { 12, 3 }, { 22, 20.5 }, { 2, 20.5 } }, true));
		Draw.Stroke(swCreateIconLine({ { 12, 3 }, { 22, 20.5 }, { 2, 20.5 } }, true));
		Draw.Erase(swCreateIconLine({ { 12, 9 }, { 12, 13.5 } }), 2.25);
		Draw.Erase(swCreateIconCircle(12, 17, 1.25));
		break;

	case swPaintedIcon::FilterCase:
		Draw.Stroke(swCreateIconLine({ { 3, 5 }, { 13, 5 } }));
		Draw.Stroke(swCreateIconLine({ { 8, 5 }, { 8, 20 } }));
		Draw.Stroke(swCreateIconLine({ { 14, 11 }, { 21, 11 } }));
		Draw.Stroke(swCreateIconLine({ { 17, 8 }, { 17, 18 }, { 18, 20 }, { 20, 20 } }));
		break;

	case swPaintedIcon::LibraryArchive:
		Draw.Stroke(swCreateIconRect(3, 4, 21, 9, 1));
		Draw.Stroke(swCreateIconLine({ { 5, 9 }, { 5, 20 }, { 19, 20 }, { 19, 9 } }));
		Draw.Stroke(swCreateIconLine({ { 10, 13 }, { 14, 13 } }));
		break;

	case swPaintedIcon::RenderPlaceholder:
		Draw.Stroke(swCreateIconRect(3, 4, 21, 20, 2));
		Draw.Fill(swCreateIconCircle(8, 7.5, 1.5));

		// hills end inside the frame
		Draw.Stroke(swCreateIconLine({ { 4, 15 }, { 8, 11 }, { 13, 16 }, { 16, 13 }, { 20, 17 } }));
		break;

	case swPaintedIcon::ThumbnailFlexible:
	{
		QPainterPath Wave;
		Wave.moveTo(3, 12);
		Wave.cubicTo(6, 2, 9, 2, 12, 12);
		Wave.cubicTo(15, 22, 18, 22, 21, 12);
		Draw.Stroke(Wave);
		break;
	}

	case swPaintedIcon::ThumbnailTrackTrain:
		Draw.Stroke(swCreateIconLine({ { 8, 2 }, { 8, 22 } }));
		Draw.Stroke(swCreateIconLine({ { 16, 2 }, { 16, 22 } }));
		Draw.Stroke(swCreateIconLine({ { 4, 6 }, { 20, 6 } }));
		Draw.Stroke(swCreateIconLine({ { 4, 12 }, { 20, 12 } }));
		Draw.Stroke(swCreateIconLine({ { 4, 18 }, { 20, 18 } }));
		break;

	case swPaintedIcon::ThumbnailColors:
		Draw.Fill(swCreateIconCircle(12, 12, 9));
		Draw.Erase(swCreateIconCircle(8, 8.5, 1.75));
		Draw.Erase(swCreateIconCircle(13, 6.5, 1.75));
		Draw.Erase(swCreateIconCircle(17, 10, 1.75));
		Draw.Erase(swCreateIconCircle(13.5, 16, 2.5));
		break;

	case swPaintedIcon::CursorBrick:
		Draw.Stroke(swCreateIconLine({ { 12, 3 }, { 20, 7.5 }, { 20, 16.5 }, { 12, 21 }, { 4, 16.5 }, { 4, 7.5 } }, true));
		Draw.Stroke(swCreateIconLine({ { 4, 7.5 }, { 12, 12 }, { 20, 7.5 } }));
		Draw.Stroke(swCreateIconLine({ { 12, 12 }, { 12, 21 } }));
		break;

	case swPaintedIcon::CursorMove:
		Draw.Stroke(swCreateIconArrow({ 12, 12 }, { 12, 3 }));
		Draw.Stroke(swCreateIconArrow({ 12, 12 }, { 21, 12 }));
		Draw.Stroke(swCreateIconArrow({ 12, 12 }, { 12, 21 }));
		Draw.Stroke(swCreateIconArrow({ 12, 12 }, { 3, 12 }));
		break;

	case swPaintedIcon::CursorMoveAxisX:
	case swPaintedIcon::CursorMoveAxisY:
	case swPaintedIcon::CursorMoveAxisZ:
	{
		// axes follow the gizmo
		const qreal Angle = Icon == swPaintedIcon::CursorMoveAxisX ? 18.0 : Icon == swPaintedIcon::CursorMoveAxisY ? -18.0 : -90.0;
		const QPointF Direction = QPointF(qCos(qDegreesToRadians(Angle)), qSin(qDegreesToRadians(Angle))) * 9.0;

		Draw.Stroke(swCreateIconArrow({ 12, 12 }, QPointF(12, 12) + Direction));
		Draw.Stroke(swCreateIconArrow({ 12, 12 }, QPointF(12, 12) - Direction));
		Draw.Fill(swCreateIconCircle(12, 12, 2));
		break;
	}

	case swPaintedIcon::CursorRotateAxisX:
	case swPaintedIcon::CursorRotateAxisY:
	case swPaintedIcon::CursorRotateAxisZ:
	{
		// each ring stands across its axis
		const qreal Tilt = Icon == swPaintedIcon::CursorRotateAxisX ? -72.0 : Icon == swPaintedIcon::CursorRotateAxisY ? 72.0 : 0.0;

		Draw.Stroke(swCreateIconArc(12, 12, 9.5, 4, 30, 300, Tilt, 3.5));
		Draw.Fill(swCreateIconCircle(12, 12, 2));
		break;
	}

	case swPaintedIcon::CursorRotateX:
	case swPaintedIcon::CursorRotateY:
	{
		QPainterPath Path = swCreateIconArc(12, 24, 13, 13, -140, 100, 0, 3.5);
		Path.addPath(swCreateIconArc(12, 24, 13, 13, -40, -100, 0, 3.5));
		Path.translate(0, -1);

		// vertical turn is the horizontal on its side
		if (Icon == swPaintedIcon::CursorRotateY)
			Path = QTransform(0, 1, 1, 0, 0, 0).map(Path);

		Draw.Stroke(Path);
		break;
	}

	case swPaintedIcon::BadgeAdd:
		Draw.Stroke(swCreateIconLine({ { 12, 4 }, { 12, 20 } }), 4);
		Draw.Stroke(swCreateIconLine({ { 4, 12 }, { 20, 12 } }), 4);
		break;

	case swPaintedIcon::BadgeRemove:
		Draw.Stroke(swCreateIconLine({ { 4, 12 }, { 20, 12 } }), 4);
		break;

	case swPaintedIcon::Close:
		Draw.Stroke(swCreateIconLine({ { 6, 6 }, { 18, 18 } }));
		Draw.Stroke(swCreateIconLine({ { 18, 6 }, { 6, 18 } }));
		break;

	case swPaintedIcon::ArrowLeft:
		Draw.Stroke(swCreateIconLine({ { 16, 4 }, { 7, 12 }, { 16, 20 } }));
		break;

	case swPaintedIcon::ArrowRight:
		Draw.Stroke(swCreateIconLine({ { 8, 4 }, { 17, 12 }, { 8, 20 } }));
		break;

	case swPaintedIcon::ArrowUp:
		Draw.Stroke(swCreateIconLine({ { 4, 16 }, { 12, 7 }, { 20, 16 } }));
		break;

	case swPaintedIcon::ArrowDown:
		Draw.Stroke(swCreateIconLine({ { 4, 8 }, { 12, 17 }, { 20, 8 } }));
		break;

	case swPaintedIcon::Undock:
		Draw.Stroke(swCreateIconRect(4, 9, 15, 20, 1));
		Draw.Stroke(swCreateIconLine({ { 9, 9 }, { 9, 4 }, { 20, 4 }, { 20, 15 }, { 15, 15 } }));
		break;

	case swPaintedIcon::Refresh:
		Draw.Stroke(swCreateIconArc(12, 12, 8, 8, -30, 290, 0, 4.5));
		break;

	case swPaintedIcon::Download:
		Draw.Stroke(swCreateIconLine({ { 12, 4 }, { 12, 15 } }));
		Draw.Stroke(swCreateIconLine({ { 7, 10 }, { 12, 15 }, { 17, 10 } }));
		Draw.Stroke(swCreateIconLine({ { 4, 16 }, { 4, 20 }, { 20, 20 }, { 20, 16 } }));
		break;

	case swPaintedIcon::Lock:
		Draw.Stroke(swCreateIconLine({ { 8, 11 }, { 8, 7.5 }, { 9.25, 4.75 }, { 12, 3.5 }, { 14.75, 4.75 }, { 16, 7.5 }, { 16, 11 } }));
		Draw.Fill(swCreateIconRect(5, 11, 19, 21, 1.5));
		Draw.Erase(swCreateIconCircle(12, 15.25, 1.5));
		Draw.Erase(swCreateIconLine({ { 12, 15.5 }, { 12, 18 } }), 1.75);
		break;

	case swPaintedIcon::Snowflake:
		Draw.Stroke(swCreateIconLine({ { 12, 3 }, { 12, 21 } }));
		Draw.Stroke(swCreateIconLine({ { 19.79, 7.5 }, { 4.21, 16.5 } }));
		Draw.Stroke(swCreateIconLine({ { 19.79, 16.5 }, { 4.21, 7.5 } }));
		Draw.Stroke(swCreateIconLine({ { 19.66, 10.03 }, { 16.76, 9.25 }, { 17.54, 6.35 } }));
		Draw.Stroke(swCreateIconLine({ { 14.12, 4.38 }, { 12, 6.5 }, { 9.88, 4.38 } }));
		Draw.Stroke(swCreateIconLine({ { 6.46, 6.35 }, { 7.24, 9.25 }, { 4.34, 10.03 } }));
		Draw.Stroke(swCreateIconLine({ { 4.34, 13.97 }, { 7.24, 14.75 }, { 6.46, 17.65 } }));
		Draw.Stroke(swCreateIconLine({ { 9.88, 19.62 }, { 12, 17.5 }, { 14.12, 19.62 } }));
		Draw.Stroke(swCreateIconLine({ { 17.54, 17.65 }, { 16.76, 14.75 }, { 19.66, 13.97 } }));
		break;
	}

	Painter->restore();
}

// use to draw the painted icons
class swPaintedIconEngine : public QIconEngine
{
public:
	explicit swPaintedIconEngine(swPaintedIcon Icon)
		: mIcon(Icon)
	{
	}

	void paint(QPainter* Painter, const QRect& Rect, QIcon::Mode Mode, QIcon::State State) override
	{
		const qreal PixelRatio = Painter->device() ? Painter->device()->devicePixelRatioF() : 1.0;
		QPixmap Pixmap = pixmap(Rect.size() * PixelRatio, Mode, State);

		Pixmap.setDevicePixelRatio(PixelRatio);
		Painter->drawPixmap(Rect, Pixmap);
	}

	QPixmap pixmap(const QSize& Size, QIcon::Mode Mode, QIcon::State State) override
	{
		Q_UNUSED(State);

		const QColor Color = swGetInterfaceIconColor(Mode);
		const QString CacheKey = QString::fromLatin1("swPaintedIcon:%1:%2x%3:%4").arg(static_cast<int>(mIcon)).arg(Size.width()).arg(Size.height()).arg(Color.rgba(), 0, 16);
		QPixmap Pixmap;

		if (QPixmapCache::find(CacheKey, &Pixmap))
			return Pixmap;

		Pixmap = QPixmap(Size);
		Pixmap.fill(Qt::transparent);

		QPainter Painter(&Pixmap);
		swDrawPaintedIcon(&Painter, mIcon, Size, Color);
		Painter.end();

		QPixmapCache::insert(CacheKey, Pixmap);

		return Pixmap;
	}

	QIconEngine* clone() const override
	{
		return new swPaintedIconEngine(mIcon);
	}

protected:
	swPaintedIcon mIcon;
};

QIcon swCreatePaintedIcon(swPaintedIcon Icon)
{
	return QIcon(new swPaintedIconEngine(Icon));
}

// use to paint an icon into an image
QImage swCreatePaintedIconImage(swPaintedIcon Icon, int Size, const QColor& Color)
{
	QImage Image(Size, Size, QImage::Format_ARGB32_Premultiplied);
	Image.fill(Qt::transparent);

	QPainter Painter(&Image);
	swDrawPaintedIcon(&Painter, Icon, QSizeF(Size, Size), Color);
	Painter.end();

	return Image;
}
