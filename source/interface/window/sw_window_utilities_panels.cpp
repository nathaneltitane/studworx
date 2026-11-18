#include "sw_global.h"
#include "sw_window_utilities.h"
#include "sw_shortcuts.h"

QSize swGetPanelButtonSize()
{
	// size buttons after the gear button
	static QSize ButtonSize;

	if (!ButtonSize.isValid())
	{
		QToolBar ToolBar;
		QToolButton* Button = new QToolButton(&ToolBar);

		Button->setIcon(swCreatePaintedIcon(swPaintedIcon::Gear));
		Button->setAutoRaise(true);
		ToolBar.addWidget(Button);

		ButtonSize = Button->sizeHint();
	}

	return ButtonSize;
}

QSize swGetBrowseButtonSize()
{
	// narrowest text button is browse
	static QSize ButtonSize;

	if (!ButtonSize.isValid())
	{
		QToolButton Button;

		Button.setText(QLatin1String("..."));
		ButtonSize = Button.sizeHint();
	}

	return ButtonSize;
}

QColor swGetButtonOutlineColor(const QPalette& Palette)
{
	// same outline the style gives buttons
	const QColor Back = Palette.color(QPalette::Button);
	const QColor Fore = Palette.color(QPalette::ButtonText);

	return QColor::fromRgbF(Back.redF() * 0.7 + Fore.redF() * 0.3, Back.greenF() * 0.7 + Fore.greenF() * 0.3, Back.blueF() * 0.7 + Fore.blueF() * 0.3);
}

void swSetupPanelButton(QToolButton* Button)
{
	// outline panel buttons
	const QSize ButtonSize = swGetPanelButtonSize();

	Button->setAutoRaise(false);
	Button->setFocusPolicy(Qt::NoFocus);
	Button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	if (Button->toolButtonStyle() == Qt::ToolButtonIconOnly)
		Button->setFixedSize(ButtonSize);
	else
	{
		Button->setMinimumWidth(0);
		Button->setMaximumWidth(QWIDGETSIZE_MAX);
		Button->setFixedHeight(ButtonSize.height());
	}
}

// use to draw tab scroll arrows like panel buttons
class swTabScrollFilter : public QObject
{
public:
	using QObject::QObject;

	bool eventFilter(QObject* Object, QEvent* Event) override
	{
		if (Event->type() != QEvent::Paint)
			return false;

		QToolButton* Button = static_cast<QToolButton*>(Object);
		// framed like the gear, level with the tabs, 2px gap on the left
		const int Height = swGetPanelButtonSize().height();
		const QRect Area(0, (Button->height() - Height + 1) / 2, Button->width(), Height);
		const QRectF Frame = QRectF(Area).adjusted(2.5, 1.5, -0.5, -1.5);
		const QPalette& Palette = Button->palette();

		QPainter Painter(Button);
		// right arrow overlaps the left one, keep its edge
		Painter.fillRect(Button->property("swTabScroll").toBool() ? Area : Area.adjusted(2, 0, 0, 0), Palette.color(QPalette::Window));
		Painter.setRenderHint(QPainter::Antialiasing);

		QColor Shade = Palette.color(QPalette::WindowText);
		Shade.setAlpha(Button->isDown() ? 40 : 30);

		Painter.setPen(swGetButtonOutlineColor(Palette));
		Painter.setBrush(Button->isEnabled() && (Button->isDown() || Button->underMouse()) ? QBrush(Shade) : QBrush(Palette.color(QPalette::Button)));
		Painter.drawRoundedRect(Frame, 3, 3);

		const QRect IconRect = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, Button->iconSize(), Frame.toAlignedRect());
		Button->icon().paint(&Painter, IconRect, Qt::AlignCenter, Button->isEnabled() ? QIcon::Normal : QIcon::Disabled);

		return true;
	}
};

void swSetupTabScrollButtons(QTabBar* TabBar)
{
	// scroll arrows match the gear button
	const QList<QToolButton*> Buttons = TabBar->findChildren<QToolButton*>(QString(), Qt::FindDirectChildrenOnly);

	for (QToolButton* Button : Buttons)
	{
		if (Button->property("swTabScroll").isValid() || Button->arrowType() == Qt::NoArrow)
			continue;

		const bool Left = Button->arrowType() == Qt::LeftArrow;

		Button->setProperty("swTabScroll", Left);
		Button->setArrowType(Qt::NoArrow);
		Button->setIcon(swCreatePaintedIcon(Left ? swPaintedIcon::ArrowLeft : swPaintedIcon::ArrowRight));
		Button->setAutoRaise(false);
		Button->setFocusPolicy(Qt::NoFocus);
		Button->setAttribute(Qt::WA_Hover);
		Button->setToolTip(Left ? QCoreApplication::translate("swMainWindow", "Scroll the model tabs left.") : QCoreApplication::translate("swMainWindow", "Scroll the model tabs right."));
		Button->setAccessibleName(Left ? QCoreApplication::translate("swMainWindow", "Scroll Tabs Left") : QCoreApplication::translate("swMainWindow", "Scroll Tabs Right"));
		Button->setWhatsThis(Left ? QCoreApplication::translate("swMainWindow", "<p><b>Scroll Tabs Left</b></p><p>Show the model tabs hidden on the left when they do not all fit.</p>") : QCoreApplication::translate("swMainWindow", "<p><b>Scroll Tabs Right</b></p><p>Show the model tabs hidden on the right when they do not all fit.</p>"));
		Button->installEventFilter(new swTabScrollFilter(Button));
	}
}

// use as the right end of a tab, divided from the title
class swTabCloseButton : public QToolButton
{
public:
	using QToolButton::QToolButton;

	// square filling the tab inside its outline
	QSize sizeHint() const override
	{
		// tab bar adds its vertical space around the button
		const int Height = swGetPanelButtonSize().height() - 4;

		return QSize(Height + 2 + Gap, Height);
	}

	// gap pads the label, not clickable
	static constexpr int Gap = 6;

	bool hitButton(const QPoint& Position) const override
	{
		return Position.x() > Gap && rect().contains(Position);
	}

protected:
	// shortcut: tab bar places the button 2px left and 1px up of the tab inside, offset measured on stylesheet tabs
	void moveEvent(QMoveEvent* Event) override
	{
		QToolButton::moveEvent(Event);

		if (pos() == mPlaced)
			return;

		mPlaced = pos() + QPoint(2, 1);
		move(mPlaced);
	}

	void paintEvent(QPaintEvent*) override
	{
		QPainter Painter(this);

		// shade on hover and press
		if (isDown() || underMouse())
		{
			QColor Color = palette().color(QPalette::WindowText);
			Color.setAlpha(isDown() ? 40 : 30);

			Painter.setRenderHint(QPainter::Antialiasing);
			Painter.setPen(Qt::NoPen);
			Painter.setBrush(Color);
			Painter.drawRoundedRect(QRectF(rect()).adjusted(Gap + 1, 0, 0, 0), 2, 2);
			Painter.setRenderHint(QPainter::Antialiasing, false);
		}

		// divider shares the tab outline
		Painter.setPen(swGetButtonOutlineColor(palette()));
		Painter.drawLine(Gap, 0, Gap, height() - 1);

		const QRect IconRect = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, iconSize(), rect().adjusted(Gap + 1, 0, 0, 0));
		icon().paint(&Painter, IconRect, Qt::AlignCenter, isEnabled() ? QIcon::Normal : QIcon::Disabled);
	}

	QPoint mPlaced;
};

QToolButton* swCreateTabCloseButton(QWidget* Parent)
{
	QToolButton* Button = new swTabCloseButton(Parent);

	Button->setIcon(swCreatePaintedIcon(swPaintedIcon::Close));
	Button->setFocusPolicy(Qt::NoFocus);
	Button->setAttribute(Qt::WA_Hover);

	return Button;
}

void swSetupPanelEdit(QWidget* Edit)
{
	// text boxes as tall as buttons
	Edit->setFixedHeight(swGetPanelButtonSize().height());
}

// use to select the value on click
class swSpinBoxSelectFilter : public QObject
{
public:
	using QObject::QObject;

	bool eventFilter(QObject* Object, QEvent* Event) override
	{
		QLineEdit* LineEdit = qobject_cast<QLineEdit*>(Object);

		if (LineEdit && Event->type() == QEvent::MouseButtonRelease && static_cast<QMouseEvent*>(Event)->button() == Qt::LeftButton && !LineEdit->hasSelectedText())
		{
			// keep a dragged selection
			QAbstractSpinBox* SpinBox = static_cast<QAbstractSpinBox*>(parent());
			SpinBox->selectAll();

			QTimer::singleShot(0, LineEdit, [LineEdit, SpinBox]()
			{
				if (!LineEdit->hasSelectedText())
					SpinBox->selectAll();
			});
		}

		return QObject::eventFilter(Object, Event);
	}
};

void swSetupSpinBoxSelection(QAbstractSpinBox* SpinBox)
{
	// clicks go to the inner text
	if (SpinBox->property("SelectOnClick").toBool())
		return;

	SpinBox->setProperty("SelectOnClick", true);
	swSpinBoxSelectFilter* Filter = new swSpinBoxSelectFilter(SpinBox);

	if (QLineEdit* LineEdit = SpinBox->findChild<QLineEdit*>(QString(), Qt::FindDirectChildrenOnly))
		LineEdit->installEventFilter(Filter);
}

void swSetupPanelSpinBox(QAbstractSpinBox* SpinBox)
{
	swSetupSpinBoxSelection(SpinBox);

	// equal stretch for equal widths
	QSizePolicy SizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	SizePolicy.setHorizontalStretch(1);
	SpinBox->setSizePolicy(SizePolicy);
	swUpdatePanelSpinBoxWidth(SpinBox);
	swSetupPanelEdit(SpinBox);
}

void swUpdatePanelSpinBoxWidth(QAbstractSpinBox* SpinBox)
{
	// fit the largest value
	QString Prefix, Suffix, Value = QLatin1String("-000");

	if (const QDoubleSpinBox* DoubleSpinBox = qobject_cast<const QDoubleSpinBox*>(SpinBox))
	{
		Prefix = DoubleSpinBox->prefix();
		Suffix = DoubleSpinBox->suffix();
		Value = DoubleSpinBox->textFromValue(DoubleSpinBox->maximum());
	}
	else if (const QSpinBox* IntegerSpinBox = qobject_cast<const QSpinBox*>(SpinBox))
	{
		Prefix = IntegerSpinBox->prefix();
		Suffix = IntegerSpinBox->suffix();
		Value = QString::number(IntegerSpinBox->maximum());
	}

	if (Value.size() < 4)
		Value = QLatin1String("-000");

	// size unbounded ranges for typical values
	const bool Unbounded = Value.size() > 8;

	if (Unbounded)
		Value = QLatin1String("-0000.00");

	QStyleOptionSpinBox Option;
	Option.initFrom(SpinBox);
	Option.rect = QRect(QPoint(0, 0), SpinBox->sizeHint());
	Option.subControls = QStyle::SC_SpinBoxUp | QStyle::SC_SpinBoxDown | QStyle::SC_SpinBoxFrame | QStyle::SC_SpinBoxEditField;
	Option.frame = SpinBox->hasFrame();

	const QRect EditRect = SpinBox->style()->subControlRect(QStyle::CC_SpinBox, &Option, QStyle::SC_SpinBoxEditField, SpinBox);
	const int Decoration = Option.rect.width() - EditRect.width();

	// keep the larger width
	const int TextWidth = SpinBox->fontMetrics().horizontalAdvance(Prefix + Value + Suffix) + Decoration;
	const int Width = Unbounded ? TextWidth : qMax(TextWidth, SpinBox->sizeHint().width());
	SpinBox->setMinimumWidth(qMax(Width, SpinBox->property("swColumnWidth").toInt()));
}

int swGetLayoutSpacing(const QWidget* Widget, Qt::Orientation Orientation)
{
	// standard layout spacing
	QStyle* Style = Widget->style();
	const int Spacing = Style->pixelMetric(Orientation == Qt::Vertical ? QStyle::PM_LayoutVerticalSpacing : QStyle::PM_LayoutHorizontalSpacing, nullptr, Widget);

	return Spacing >= 0 ? Spacing : Style->layoutSpacing(QSizePolicy::DefaultType, QSizePolicy::DefaultType, Orientation, nullptr, Widget);
}

QWidget* swCreatePanelRow(QWidget* Parent)
{
	// panel layout gives the margins
	QWidget* Row = new QWidget(Parent);
	QHBoxLayout* Layout = new QHBoxLayout(Row);
	Layout->setContentsMargins(0, 0, 0, 0);

	return Row;
}

void swAddPanelRowWidget(QWidget* Row, QWidget* Widget)
{
	Row->layout()->addWidget(Widget);
}

void swAddPanelRowStretch(QWidget* Row)
{
	static_cast<QHBoxLayout*>(Row->layout())->addStretch(1);
}

QToolButton* swAddPanelActionButton(QWidget* Row, QAction* Action)
{
	QToolButton* Button = new QToolButton(Row);

	Button->setDefaultAction(Action);
	swAddPanelRowWidget(Row, Button);
	swSetupPanelButton(Button);

	return Button;
}

QToolButton* swAddPanelTextButton(QWidget* Row, const QString& Name, const QString& ToolTip)
{
	QToolButton* Button = new QToolButton(Row);

	Button->setText(Name);
	Button->setToolButtonStyle(Qt::ToolButtonTextOnly);
	Button->setToolTip(ToolTip);
	Button->setWhatsThis(ToolTip);
	Button->setAccessibleName(Name);
	swSetupPanelButton(Button);
	Button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	static_cast<QHBoxLayout*>(Row->layout())->addWidget(Button, 1);

	return Button;
}

QToolButton* swAddPanelOptionsButton(QWidget* Row, QMenu* Menu, const QString& ToolTip, const QString& AccessibleName)
{
	QToolButton* OptionsButton = new QToolButton(Row);

	OptionsButton->setIcon(swCreatePaintedIcon(swPaintedIcon::Gear));
	OptionsButton->setToolTip(ToolTip);
	OptionsButton->setAccessibleName(AccessibleName);
	OptionsButton->setPopupMode(QToolButton::InstantPopup);
	OptionsButton->setMenu(Menu);
	swAddPanelRowWidget(Row, OptionsButton);
	swSetupPanelButton(OptionsButton);

	return OptionsButton;
}

void swSetupWidgetHelp(QWidget* Root)
{
	const auto ShowHelp = [Root](QWidget* Widget)
	{
		while (Widget && Widget->whatsThis().isEmpty() && Widget != Root)
			Widget = Widget->parentWidget();

		if (Widget && !Widget->whatsThis().isEmpty())
			QWhatsThis::showText(Widget->mapToGlobal(QPoint(0, Widget->height())), Widget->whatsThis(), Widget);
	};

	QShortcut* HelpShortcut = new QShortcut(QKeySequence(Qt::Key_F1), Root);
	HelpShortcut->setContext(Qt::WidgetWithChildrenShortcut);

	const auto HelpActivated = [Root, ShowHelp]()
	{
		QWidget* Focus = QApplication::focusWidget();

		// popups share the shortcut scope
		ShowHelp(Focus ? Focus : Root);
	};

	QObject::connect(HelpShortcut, &QShortcut::activated, Root, HelpActivated);

	// choosers overlap the dock scope
	QObject::connect(HelpShortcut, &QShortcut::activatedAmbiguously, Root, HelpActivated);
}

// use to give dialogs the title bar what's this button, f1 and the what's this shortcut
void swSetupDialogHelp(QDialog* Dialog)
{
	Dialog->setWindowFlag(Qt::WindowContextHelpButtonHint, true);

	swSetupWidgetHelp(Dialog);

	// modal dialogs block the main window shortcut
	const QKeySequence WhatsThisKey(gKeyboardShortcuts.mShortcuts[SW_HELP_WHATS_THIS]);

	if (WhatsThisKey.isEmpty())
		return;

	QShortcut* WhatsThisShortcut = new QShortcut(WhatsThisKey, Dialog);
	WhatsThisShortcut->setContext(Qt::WindowShortcut);

	QObject::connect(WhatsThisShortcut, &QShortcut::activated, Dialog, []()
	{
		QWhatsThis::enterWhatsThisMode();
	});
}
