#include "sw_global.h"
#include "sw_window_utilities.h"
#include "sw_application.h"
#include "sw_library.h"
#include "sw_model.h"
#include "sw_data_part.h"
#include "sw_string.h"
#include "sw_profile.h"
#include "sw_shortcuts.h"
#include "sw_color_configuration.h"
#include <QPrintDialog>

#ifdef Q_OS_WIN
#include <windows.h>
#include <TlHelp32.h>
#endif

// use to read the rebrickable key, asked for once
QString swRequestRebrickableApiKey(QWidget* Parent, const QString& Purpose)
{
	const QString StoredKey = swGetProfileString(SW_PROFILE_REBRICKABLE_API_KEY);

	if (!StoredKey.isEmpty())
		return StoredKey;

	bool Ok = false;
	const QString Key = QInputDialog::getText(Parent, QCoreApplication::translate("swRebrickable", "Rebrickable API Key"), QCoreApplication::translate("swRebrickable", "%1\nEnter your Rebrickable API key, free from your account settings at rebrickable.com.\nIt can be modified under Preferences > General > Online Services.").arg(Purpose), QLineEdit::Normal, QString(), &Ok).trimmed();

	if (!Ok || Key.isEmpty())
		return QString();

	swSetProfileString(SW_PROFILE_REBRICKABLE_API_KEY, Key);

	return Key;
}

// use to pick an existing or a new color configuration file
QString swChooseColorConfigurationFile(QWidget* Parent, swColorSource Source, const QString& FileName, bool Existing)
{
	QString StartPath = FileName;

	// suggest the official name beside the parts library
	if (StartPath.isEmpty())
	{
		const QString LibraryPath = swGetPartsLibrary() ? swGetPartsLibrary()->GetLibraryPath() : swGetProfileString(SW_PROFILE_PARTS_LIBRARY);
		const QFileInfo LibraryInfo(LibraryPath);
		const QString LibraryFolder = LibraryInfo.isFile() ? LibraryInfo.absolutePath() : LibraryPath;

		StartPath = QDir(LibraryFolder).absoluteFilePath(swGetColorConfigurationOfficialName(Source));
	}

	const QString Title = Existing ? QCoreApplication::translate("swColorConfiguration", "Choose %1").arg(swGetColorConfigurationName(Source)) : QCoreApplication::translate("swColorConfiguration", "Save %1").arg(swGetColorConfigurationName(Source));
	const QString Filter = QCoreApplication::translate("swColorConfiguration", "LDraw Files (*.ldr);;All Files (*)");
	const QString Result = Existing ? QFileDialog::getOpenFileName(Parent, Title, StartPath, Filter) : QFileDialog::getSaveFileName(Parent, Title, StartPath, Filter);

	return Result.isEmpty() ? QString() : QDir::toNativeSeparators(Result);
}

// use to write the latest official file to a chosen name
bool swUpdateColorConfigurationFile(QWidget* Parent, swColorSource Source, const QString& FileName, bool ShowSummary)
{
	const QString Title = QCoreApplication::translate("swColorConfiguration", "Update %1").arg(swGetColorConfigurationName(Source));
	const swResult<void> Result = swUpdateColorConfiguration(Parent, Source, FileName);

	if (!Result)
	{
		QMessageBox::warning(Parent, Title, Result.error());
		return false;
	}

	if (ShowSummary)
		QMessageBox::information(Parent, Title, QCoreApplication::translate("swColorConfiguration", "Saved the latest %1 to '%2'.").arg(swGetColorConfigurationOfficialName(Source), FileName));

	return true;
}

// use to register and write a missing color configuration
QString swEnsureColorConfigurationFile(QWidget* Parent, swColorSource Source, const QString& FileName)
{
	if (swColorConfigurationExists(FileName))
		return FileName;

	const QString Name = swGetColorConfigurationName(Source);
	const QString Question = FileName.isEmpty() ?
		QCoreApplication::translate("swColorConfiguration", "No %1 file is set. It defines the LDraw colors and their edge lines.\n\nChoose where to save it and download the latest %2 now?").arg(Name, swGetColorConfigurationOfficialName(Source)) :
		QCoreApplication::translate("swColorConfiguration", "The %1 file '%2' was not found. It defines the LDraw colors and their edge lines.\n\nChoose where to save it and download the latest %3 now?").arg(Name, FileName, swGetColorConfigurationOfficialName(Source));

	if (QMessageBox::question(Parent, Name, Question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes) != QMessageBox::Yes)
		return QString();

	const QString ChosenName = swChooseColorConfigurationFile(Parent, Source, FileName, false);

	if (ChosenName.isEmpty() || !swUpdateColorConfigurationFile(Parent, Source, ChosenName, false))
		return QString();

	return ChosenName;
}

QPoint swGetPopupPosition(const QPoint& Position, const QSize& Size, const QPoint& ScreenPosition)
{
	QScreen* Screen = QGuiApplication::screenAt(ScreenPosition);

	if (!Screen)
		Screen = QGuiApplication::primaryScreen();

	if (!Screen)
		return Position;

	const QRect AvailableGeometry = Screen->availableGeometry();
	const int X = qBound(AvailableGeometry.left(), Position.x(), qMax(AvailableGeometry.left(), AvailableGeometry.right() - Size.width() + 1));
	const int Y = qBound(AvailableGeometry.top(), Position.y(), qMax(AvailableGeometry.top(), AvailableGeometry.bottom() - Size.height() + 1));

	return QPoint(X, Y);
}

// use to start form labels at the left edge and grow fields to the right edge
static void swAlignFormLayouts(QWidget* Window)
{
	for (QFormLayout* FormLayout : Window->findChildren<QFormLayout*>())
	{
		FormLayout->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		FormLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
	}
}

bool swWindowGeometryFilter::eventFilter(QObject* Object, QEvent* Event)
{
	if (Event->type() != QEvent::Show || !Object->isWidgetType())
		return false;

	QWidget* Window = static_cast<QWidget*>(Object);

	if (!Window->isWindow() || Window->property("swInitialGeometry").toBool())
		return false;

	// text editor prompts stay resizable
	const QInputDialog* InputDialog = qobject_cast<QInputDialog*>(Window);

	if (Window->property("swInputPrompt").toBool() || (InputDialog && !InputDialog->testOption(QInputDialog::UsePlainTextEditForTextInput)))
	{
		Window->setProperty("swInitialGeometry", true);
		swAlignFormLayouts(Window);

		// four by one up to 64 characters
		const QSize ContentSize = Window->sizeHint().expandedTo(Window->minimumSizeHint());
		const int ProportionalWidth = qMin(4 * ContentSize.height(), 64 * Window->fontMetrics().averageCharWidth());

		// widen until every field shows its whole text
		int TextOverflow = 0;

		for (const QLineEdit* LineEdit : Window->findChildren<QLineEdit*>())
		{
			const int TextWidth = LineEdit->fontMetrics().horizontalAdvance(LineEdit->text()) + 4 * LineEdit->fontMetrics().averageCharWidth();
			TextOverflow = qMax(TextOverflow, TextWidth - LineEdit->sizeHint().width());
		}

		int Width = qMax(ContentSize.width() + TextOverflow, ProportionalWidth);

		if (const QScreen* Screen = Window->screen())
			Width = qMin(Width, qMax(ContentSize.width(), Screen->availableGeometry().width() / 2));

		Window->setFixedSize(Width, ContentSize.height());

		return false;
	}

	// qt dialogs size themselves
	const bool IsDialog = qobject_cast<QDialog*>(Window) && !qobject_cast<QMessageBox*>(Window) && !qobject_cast<QInputDialog*>(Window) && !qobject_cast<QProgressDialog*>(Window) &&
		!qobject_cast<QFileDialog*>(Window) && !qobject_cast<QColorDialog*>(Window) && !qobject_cast<QFontDialog*>(Window) && !qobject_cast<QPrintDialog*>(Window);
	const bool IsMainWindow = qobject_cast<QMainWindow*>(Window);
	const bool IsSecondaryWindow = IsMainWindow && Window->parentWidget();

	if (!IsDialog && !IsMainWindow)
		return false;

	Window->setProperty("swInitialGeometry", true);
	swAlignFormLayouts(Window);

	QWidget* Parent = Window->parentWidget() ? Window->parentWidget()->window() : nullptr;
	QScreen* Screen = Parent ? Parent->screen() : Window->screen();

	if (!Screen)
		return false;

	// a quarter of the screen
	const QRect Available = Screen->availableGeometry();
	const QSize MinimumSize = QSize(Available.width() / 4, Available.height() / 4).expandedTo(Window->minimumSizeHint()).boundedTo(Available.size());

	// keep at least a quarter
	Window->setMinimumSize(MinimumSize);

	if (!IsDialog && !IsSecondaryWindow)
		return false;

	// set only the size
	QSize Size = MinimumSize.expandedTo(Window->sizeHint());
	Size = Size.boundedTo(Available.size());

	Window->resize(Size);

	return false;
}

// use to fit a scroll area to its rows
static void swFitScrollAreaWidth(QScrollArea* ScrollArea)
{
	if (ScrollArea->widget())
		ScrollArea->setMinimumWidth(ScrollArea->widget()->minimumSizeHint().width() + ScrollArea->verticalScrollBar()->sizeHint().width() + 2 * ScrollArea->frameWidth());
}

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

static bool swIsViewDock(const QDockWidget* DockWidget);

// use to bring floating panels back within their limits
// window managers refuse during their own resizes, so ask again for a while
static void swClampFloatingDock(QDockWidget* DockWidget)
{
	QTimer* Timer = DockWidget->findChild<QTimer*>("swClampTimer", Qt::FindDirectChildrenOnly);

	if (!Timer)
	{
		Timer = new QTimer(DockWidget);
		Timer->setObjectName("swClampTimer");
		Timer->setInterval(250);

		QObject::connect(Timer, &QTimer::timeout, DockWidget, [DockWidget, Timer]()
		{
			const int Tries = DockWidget->property("swClampTries").toInt() + 1;
			DockWidget->setProperty("swClampTries", Tries);

			// shortcut: gives up after ten seconds, a drag held longer stays tall until the next resize
			const int Height = qBound(DockWidget->minimumHeight(), DockWidget->height(), DockWidget->maximumHeight());

			if (!DockWidget->isFloating() || DockWidget->height() == Height || Tries > 40)
				Timer->stop();
			else
				DockWidget->resize(DockWidget->width(), Height);
		});
	}

	// refused answers come back as resizes, asking at once again would loop
	const int Height = qBound(DockWidget->minimumHeight(), DockWidget->height(), DockWidget->maximumHeight());

	if (Timer->isActive() || !DockWidget->isFloating() || DockWidget->height() == Height)
		return;

	DockWidget->setProperty("swClampTries", 0);
	DockWidget->resize(DockWidget->width(), Height);
	Timer->start();
}

// use to hold title-only panels when one leaves or the window grows
static void swKeepSqueezedDocks(QMainWindow* MainWindow, const QDockWidget* Leaving = nullptr)
{
	if (!MainWindow)
		return;

	QList<QDockWidget*> DockWidgets;
	QList<int> Heights;

	for (QDockWidget* DockWidget : MainWindow->findChildren<QDockWidget*>(QString(), Qt::FindDirectChildrenOnly))
	{
		if (DockWidget != Leaving && !DockWidget->isFloating() && DockWidget->isVisible() && DockWidget->height() <= DockWidget->minimumSizeHint().height())
		{
			DockWidgets.append(DockWidget);
			Heights.append(DockWidget->minimumSizeHint().height());
		}
	}

	if (!DockWidgets.isEmpty())
		QTimer::singleShot(0, MainWindow, [MainWindow, DockWidgets, Heights]() { MainWindow->resizeDocks(DockWidgets, Heights, Qt::Vertical); });
}

bool swInterfaceFitFilter::eventFilter(QObject* Object, QEvent* Event)
{
	if (!Object->isWidgetType())
		return false;

	const QEvent::Type Type = Event->type();

	if (Type != QEvent::Polish && Type != QEvent::Show && Type != QEvent::FontChange && Type != QEvent::StyleChange && Type != QEvent::LayoutRequest && Type != QEvent::Hide && Type != QEvent::Resize)
		return false;

	// window managers keep the limits from the start of a resize
	// panels still have their old heights before the window lays them out
	if (Type == QEvent::Resize)
	{
		if (QMainWindow* MainWindow = qobject_cast<QMainWindow*>(Object))
			swKeepSqueezedDocks(MainWindow);

		QDockWidget* DockWidget = qobject_cast<QDockWidget*>(Object);

		if (DockWidget && DockWidget->isFloating() && (DockWidget->height() > DockWidget->maximumHeight() || DockWidget->height() < DockWidget->minimumHeight()))
			QTimer::singleShot(0, DockWidget, [DockWidget]() { swClampFloatingDock(DockWidget); });

		return false;
	}

	// panels leaving the stack free no space for squeezed ones
	if (Type == QEvent::Hide)
	{
		QDockWidget* DockWidget = qobject_cast<QDockWidget*>(Object);

		if (DockWidget && !DockWidget->isFloating())
			swKeepSqueezedDocks(qobject_cast<QMainWindow*>(DockWidget->parentWidget()), DockWidget);

		return false;
	}

	QWidget* Widget = static_cast<QWidget*>(Object);

	// show the whole spin box value
	if (QAbstractSpinBox* SpinBox = qobject_cast<QAbstractSpinBox*>(Widget))
	{
		if (Type != QEvent::LayoutRequest)
			swUpdatePanelSpinBoxWidth(SpinBox);

		return false;
	}

	// never cut unscrolled rows
	if (Type == QEvent::LayoutRequest && !qobject_cast<QDockWidget*>(Widget))
	{
		QWidget* Viewport = Widget->parentWidget();
		QScrollArea* ScrollArea = Viewport ? qobject_cast<QScrollArea*>(Viewport->parentWidget()) : nullptr;

		if (ScrollArea && ScrollArea->widget() == Widget && ScrollArea->horizontalScrollBarPolicy() == Qt::ScrollBarAlwaysOff)
			swFitScrollAreaWidth(ScrollArea);

		return false;
	}

	if (QScrollArea* ScrollArea = qobject_cast<QScrollArea*>(Widget))
	{
		if (ScrollArea->horizontalScrollBarPolicy() == Qt::ScrollBarAlwaysOff)
			swFitScrollAreaWidth(ScrollArea);

		return false;
	}

	// panels side by side, stacked or tabbed
	if (QMainWindow* MainWindow = qobject_cast<QMainWindow*>(Widget))
	{
		if (Type == QEvent::Polish)
			MainWindow->setDockOptions(QMainWindow::AnimatedDocks | QMainWindow::AllowNestedDocks | QMainWindow::AllowTabbedDocks);

		return false;
	}

	// floating panels fit their contents
	if (QDockWidget* DockWidget = qobject_cast<QDockWidget*>(Widget))
	{
		// open at the contents size when first shown floating
		if (Type == QEvent::Show)
		{
			if (DockWidget->isFloating() && !DockWidget->property("swShownFloating").toBool())
				swFitFloatingDock(DockWidget, true);

			DockWidget->setProperty("swShownFloating", DockWidget->isFloating());

			return false;
		}

		// follow contents and widths that change
		if (Type == QEvent::LayoutRequest)
		{
			// capped contents shrinking free no space for squeezed panels
			if (!DockWidget->isFloating() && DockWidget->widget())
			{
				const int MaximumHeight = DockWidget->widget()->maximumHeight();

				if (MaximumHeight < DockWidget->property("swContentsHeight").toInt())
					swKeepSqueezedDocks(qobject_cast<QMainWindow*>(DockWidget->parentWidget()), DockWidget);

				DockWidget->setProperty("swContentsHeight", MaximumHeight);
			}

			// views size their scroll bars and lines from the panel, following them would loop
			if (DockWidget->isFloating() && !swIsViewDock(DockWidget) && !DockWidget->property("swFitQueued").toBool())
			{
				DockWidget->setProperty("swFitQueued", true);

				QTimer::singleShot(0, DockWidget, [DockWidget]()
				{
					DockWidget->setProperty("swFitQueued", false);
					swFitFloatingDock(DockWidget);
				});
			}

			return false;
		}

		if (Type != QEvent::Polish || DockWidget->property("swFitFloating").toBool())
			return false;

		DockWidget->setProperty("swFitFloating", true);

		// locked panels already carry an empty title bar
		if (!DockWidget->titleBarWidget())
			DockWidget->setTitleBarWidget(swCreateDockTitleBar(DockWidget));

		QObject::connect(DockWidget, &QDockWidget::topLevelChanged, DockWidget, [DockWidget](bool Floating)
		{
			if (Floating)
				swKeepSqueezedDocks(qobject_cast<QMainWindow*>(DockWidget->parentWidget()), DockWidget);

			QTimer::singleShot(0, DockWidget, [DockWidget]() { swFitFloatingDock(DockWidget, true); });
		});

		QObject::connect(DockWidget, &QDockWidget::dockLocationChanged, DockWidget, [DockWidget]()
		{
			QTimer::singleShot(0, DockWidget, [DockWidget]() { swFitFloatingDock(DockWidget); });
		});

		QTimer::singleShot(0, DockWidget, [DockWidget]() { swFitFloatingDock(DockWidget); });
	}

	return false;
}

// use to tell sizeless widgets
static bool swIsViewDock(const QDockWidget* DockWidget)
{
	const QList<QWidget*> Children = DockWidget->findChildren<QWidget*>();

	for (const QWidget* Child : Children)
	{
		// dropdown lists are separate popups
		if (Child->window() != DockWidget->window() || !Child->isVisibleTo(DockWidget))
			continue;

		const QAbstractScrollArea* View = qobject_cast<const QAbstractScrollArea*>(Child);

		if ((View && !qobject_cast<const QScrollArea*>(View)) || qobject_cast<const QOpenGLWidget*>(Child) || Child->inherits("QWindowContainer"))
			return true;

		// scrolling views like the terminal, not the bars of scroll areas
		if (qobject_cast<const QScrollBar*>(Child) && !qobject_cast<const QAbstractScrollArea*>(Child->parentWidget()) && !qobject_cast<const QAbstractScrollArea*>(Child->parentWidget()->parentWidget()))
			return true;
	}

	return false;
}

void swFitFloatingDock(QDockWidget* DockWidget, bool Open)
{
	if (!DockWidget->isFloating())
	{
		DockWidget->setMinimumSize(0, 0);
		DockWidget->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

		// a set minimum wins over the size hint, so docked panels squeeze to the title bar
		if (DockWidget->widget() && DockWidget->widget()->minimumHeight() == 0)
			DockWidget->widget()->setMinimumHeight(1);

		// keep the widest row width
		const QMainWindow* MainWindow = qobject_cast<const QMainWindow*>(DockWidget->parentWidget());

		if (!MainWindow)
			return;

		for (const Qt::DockWidgetArea Area : { Qt::TopDockWidgetArea, Qt::BottomDockWidgetArea })
		{
			std::vector<QDockWidget*> AreaDockWidgets;
			int Width = 0;

			for (QDockWidget* AreaDockWidget : MainWindow->findChildren<QDockWidget*>(QString(), Qt::FindDirectChildrenOnly))
			{
				// floating size panels stretch
				if (AreaDockWidget->isFloating() || MainWindow->dockWidgetArea(AreaDockWidget) != Area || swIsViewDock(AreaDockWidget) || AreaDockWidget->property("swFloatingSize").toSize().isValid())
					continue;

				AreaDockWidget->setMaximumWidth(QWIDGETSIZE_MAX);
				AreaDockWidgets.push_back(AreaDockWidget);
				Width = qMax(Width, AreaDockWidget->minimumSizeHint().width());
			}

			for (QDockWidget* AreaDockWidget : AreaDockWidgets)
				AreaDockWidget->setMaximumWidth(Width);
		}

		// marked views dock square
		if (Open && DockWidget->layout())
			DockWidget->layout()->activate();

		if (Open && DockWidget->widget() && DockWidget->widget()->layout())
			DockWidget->widget()->layout()->activate();

		for (const QWidget* View : DockWidget->findChildren<QWidget*>())
		{
			if (!Open || !View->property("swOpenSquare").toBool() || !View->isVisibleTo(DockWidget))
				continue;

			// panels left unsized share the difference, so the tallest one beside takes it
			QList<QDockWidget*> DockWidgets = { DockWidget };
			QList<int> Heights = { DockWidget->height() - View->height() + View->width() };
			int Tallest = -1;

			for (QDockWidget* Other : MainWindow->findChildren<QDockWidget*>(QString(), Qt::FindDirectChildrenOnly))
			{
				if (Other == DockWidget || Other->isFloating() || !Other->isVisible() || Other->x() != DockWidget->x() || Other->width() != DockWidget->width() || MainWindow->tabifiedDockWidgets(DockWidget).contains(Other))
					continue;

				if (Tallest < 0 || Other->height() > Heights[Tallest])
					Tallest = DockWidgets.size();

				DockWidgets.append(Other);
				Heights.append(Other->height());
			}

			if (Tallest > 0)
				Heights[Tallest] -= Heights[0] - DockWidget->height();

			const_cast<QMainWindow*>(MainWindow)->resizeDocks(DockWidgets, Heights, Qt::Vertical);
		}

		return;
	}

	QWidget* Contents = DockWidget->widget();

	if (!Contents)
		return;

	// floating panels open at their contents size
	if (Contents->minimumHeight() == 1)
		Contents->setMinimumHeight(0);

	// title and frame around the contents, centred contents leave equal gaps
	// contents taller than the panel are cut at the bottom instead
	if (DockWidget->layout())
		DockWidget->layout()->activate();

	const QRect ContentsRect = Contents->geometry();
	const int BottomGap = DockWidget->height() - ContentsRect.bottom() - 1;
	const int FrameHeight = ContentsRect.top() + ContentsRect.left() - qMax(BottomGap - ContentsRect.left(), 0);

	// least height with every scroll area unrolled
	// a set minimum height wins over the size hint
	const auto MinimumHeight = [](const QWidget* Widget)
	{
		if (Widget->minimumHeight() > 0)
			return Widget->minimumHeight();

		// panels that squeeze when docked still open at their layout size
		return qMax(Widget->layout() ? Widget->layout()->totalMinimumSize().height() : 0, Widget->minimumSizeHint().height());
	};
	int Height = FrameHeight + MinimumHeight(Contents);
	int Width = DockWidget->minimumSizeHint().width();
	const QList<QScrollArea*> ScrollAreas = DockWidget->findChildren<QScrollArea*>();

	// hidden lists take no room
	for (const QScrollArea* ScrollArea : ScrollAreas)
		if (ScrollArea->widget() && ScrollArea->isVisibleTo(DockWidget))
			Height += qMax(MinimumHeight(ScrollArea->widget()) + 2 * ScrollArea->frameWidth() - ScrollArea->minimumSizeHint().height(), 0);

	// capped contents hold their height, forms never stretch, views grow
	int MaximumHeight = QWIDGETSIZE_MAX;

	if (Contents->maximumHeight() < QWIDGETSIZE_MAX)
		MaximumHeight = qMax(FrameHeight + Contents->maximumHeight(), Height);
	else if (!swIsViewDock(DockWidget))
		MaximumHeight = Height;

	if (const QScreen* Screen = DockWidget->screen())
	{
		const int ScreenHeight = Screen->availableGeometry().height() - DockWidget->frameGeometry().height() + DockWidget->height();

		// scroll a panel cut by the screen
		if (Height > ScreenHeight)
		{
			Height = ScreenHeight;

			for (const QScrollArea* ScrollArea : ScrollAreas)
				if (ScrollArea->widget() && ScrollArea->isVisibleTo(DockWidget))
					Width += qMax(ScrollArea->widget()->minimumSizeHint().width() + ScrollArea->verticalScrollBar()->sizeHint().width() + 2 * ScrollArea->frameWidth() - ScrollArea->minimumSizeHint().expandedTo(ScrollArea->minimumSize()).width(), 0);
		}

		if (MaximumHeight < QWIDGETSIZE_MAX)
			MaximumHeight = qMax(qMin(MaximumHeight, ScreenHeight), Height);
	}

	DockWidget->setMaximumSize(QWIDGETSIZE_MAX, MaximumHeight);
	DockWidget->setMinimumSize(Width, Height);

	// follow the contents even without a window manager
	if (DockWidget->height() > MaximumHeight || DockWidget->height() < Height)
		QTimer::singleShot(0, DockWidget, [DockWidget]() { swClampFloatingDock(DockWidget); });

	if (!Open)
		return;

	// open at the least contents size
	// views keep their docked width, marked ones open square
	const int OpenWidth = swIsViewDock(DockWidget) ? qMax(Width, DockWidget->width()) : Width;
	int OpenHeight = Height;

	for (const QWidget* View : DockWidget->findChildren<QWidget*>())
		if (View->property("swOpenSquare").toBool() && View->isVisibleTo(DockWidget))
			OpenHeight = qMax(Height, DockWidget->height() - View->height() + OpenWidth - DockWidget->width() + View->width());

	if (const QScreen* Screen = DockWidget->screen())
		OpenHeight = qMin(OpenHeight, qMax(Screen->availableGeometry().height() - DockWidget->frameGeometry().height() + DockWidget->height(), Height));

	DockWidget->resize(OpenWidth, OpenHeight);
}

class swFitHeightFilter : public QObject
{
public:
	explicit swFitHeightFilter(QWidget* Widget)
		: QObject(Widget)
	{
	}

protected:
	bool eventFilter(QObject* Object, QEvent* Event) override
	{
		if (Event->type() == QEvent::LayoutRequest)
		{
			QWidget* Widget = static_cast<QWidget*>(Object);
			Widget->setFixedHeight(Widget->sizeHint().height());
		}

		return false;
	}
};

void swFitHeightToContents(QWidget* Widget)
{
	Widget->setFixedHeight(Widget->sizeHint().height());
	Widget->installEventFilter(new swFitHeightFilter(Widget));
}

class swCrampedScrollArea : public QScrollArea
{
public:
	explicit swCrampedScrollArea(QWidget* Parent)
		: QScrollArea(Parent)
	{
	}

	QSize sizeHint() const override
	{
		// full height of the contents
		return QSize(QScrollArea::sizeHint().width(), widget() ? widget()->minimumHeight() : 0);
	}

	QSize minimumSizeHint() const override
	{
		// the window may shrink below the contents
		return QSize(QScrollArea::minimumSizeHint().width(), 0);
	}

protected:
	bool eventFilter(QObject* Object, QEvent* Event) override
	{
		if (Object == widget() && Event->type() == QEvent::Resize)
			FitContents();

		return QScrollArea::eventFilter(Object, Event);
	}

	bool viewportEvent(QEvent* Event) override
	{
		// a stretched widget only reports a smaller height here
		if (Event->type() == QEvent::LayoutRequest && widget())
			FitContents();

		return QScrollArea::viewportEvent(Event);
	}

private:
	void FitContents()
	{
		// never taller than the contents
		setMaximumHeight(widget()->minimumHeight());
		updateGeometry();

		// floating panels follow the new height
		if (QDockWidget* DockWidget = qobject_cast<QDockWidget*>(window()))
			QCoreApplication::postEvent(DockWidget, new QEvent(QEvent::LayoutRequest));
	}
};

QScrollArea* swScrollWhenCramped(QWidget* Widget, QWidget* Parent)
{
	swCrampedScrollArea* ScrollArea = new swCrampedScrollArea(Parent);
	ScrollArea->setFrameShape(QFrame::NoFrame);
	ScrollArea->setWidgetResizable(true);
	ScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ScrollArea->setWidget(Widget);
	ScrollArea->setMaximumHeight(Widget->minimumHeight());

	return ScrollArea;
}

void swSetInputPrompt(QDialog* Dialog)
{
	Dialog->setProperty("swInputPrompt", true);
}

swBlockingProgress* swBlockingProgress::mActive = nullptr;

swBlockingProgress::swBlockingProgress(const QString& Title, const QString& Label, bool Cancelable)
	: mTitle(Title), mLabel(Label), mCancelable(Cancelable)
{
	// nested waits share one task
	mNested = mActive != nullptr;

	if (!mNested)
		mActive = this;

	mTimer.start();
}

swBlockingProgress::~swBlockingProgress()
{
	delete mDialog;

	if (mActive == this)
		mActive = nullptr;
}

void swBlockingProgress::SetProgress(int Value, int Maximum)
{
	if (mNested)
		return;

	if (!mDialog)
	{
		// no dialog for short tasks
		if (Value >= Maximum || mTimer.elapsed() < 400)
			return;

		mDialog = new QProgressDialog(QApplication::activeWindow());
		mDialog->setWindowTitle(mTitle);
		mDialog->setLabelText(mLabel);
		mDialog->setWindowModality(Qt::ApplicationModal);
		mDialog->setMinimumDuration(0);
		mDialog->setAutoClose(false);
		mDialog->setAutoReset(false);

		QProgressBar* Bar = new QProgressBar(mDialog);
		Bar->setFormat(tr("%v of %m"));
		mDialog->setBar(Bar);

		if (!mCancelable)
		{
			mDialog->setCancelButton(nullptr);
			mDialog->setWindowFlags(mDialog->windowFlags() & ~Qt::WindowCloseButtonHint);
		}

		mDialog->setRange(0, Maximum);
		mDialog->show();
		mUpdateTimer.invalidate();
	}

	// throttle the repaint updates
	if (mUpdateTimer.isValid() && mUpdateTimer.elapsed() < 50 && Value < Maximum)
		return;

	mUpdateTimer.start();
	mDialog->setMaximum(Maximum);
	mDialog->setValue(Value);
}

bool swBlockingProgress::IsCanceled() const
{
	return mDialog && mDialog->wasCanceled();
}

QColor swGetWarningTextColor()
{
	return QColor(208, 0, 0);
}

// use to keep warning text red while the row is selected
void swWarningItemDelegate::initStyleOption(QStyleOptionViewItem* Option, const QModelIndex& Index) const
{
	QStyledItemDelegate::initStyleOption(Option, Index);

	const QVariant Foreground = Index.data(Qt::ForegroundRole);

	if (Foreground.canConvert<QBrush>() && Foreground.value<QBrush>().color() == swGetWarningTextColor())
		Option->palette.setColor(QPalette::HighlightedText, swGetWarningTextColor());
}

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

// use to draw the title bar buttons of panels
class swDockTitleButton : public QToolButton
{
public:
	swDockTitleButton(swPaintedIcon Icon, QWidget* Parent)
		: QToolButton(Parent)
	{
		setIcon(swCreatePaintedIcon(Icon));
		setFixedSize(16, 16);
		setFocusPolicy(Qt::NoFocus);
		setAttribute(Qt::WA_Hover);
	}

protected:
	void paintEvent(QPaintEvent*) override
	{
		QPainter Painter(this);

		if (underMouse() || isDown())
		{
			QColor Color = palette().color(QPalette::WindowText);
			Color.setAlpha(isDown() ? 40 : 30);

			Painter.setRenderHint(QPainter::Antialiasing);
			Painter.setPen(Qt::NoPen);
			Painter.setBrush(Color);
			Painter.drawRoundedRect(QRectF(rect()), 3, 3);
		}

		icon().paint(&Painter, rect(), Qt::AlignCenter, isEnabled() ? QIcon::Normal : QIcon::Disabled);
	}
};

// use to give panels one title bar, docked and floating
// platform frames of floating panels ignore the color theme
class swDockTitleBar : public QWidget
{
public:
	swDockTitleBar(QDockWidget* DockWidget)
		: QWidget(DockWidget), mDockWidget(DockWidget)
	{
		QHBoxLayout* Layout = new QHBoxLayout(this);
		Layout->setContentsMargins(4, 4, 8, 4);
		Layout->setSpacing(2);
		Layout->addStretch(1);

		QToolButton* FloatButton = new swDockTitleButton(swPaintedIcon::Undock, this);
		FloatButton->setToolTip(QDockWidget::tr("Float"));
		Layout->addWidget(FloatButton);

		QToolButton* CloseButton = new swDockTitleButton(swPaintedIcon::Close, this);
		CloseButton->setToolTip(QDockWidget::tr("Close"));
		Layout->addWidget(CloseButton);

		connect(FloatButton, &QToolButton::clicked, DockWidget, [DockWidget]() { DockWidget->setFloating(!DockWidget->isFloating()); });
		connect(CloseButton, &QToolButton::clicked, DockWidget, &QDockWidget::close);
		connect(DockWidget, &QDockWidget::windowTitleChanged, this, QOverload<>::of(&QWidget::update));

		const auto UpdateButtons = [FloatButton, CloseButton](QDockWidget::DockWidgetFeatures Features)
		{
			FloatButton->setVisible(Features & QDockWidget::DockWidgetFloatable);
			CloseButton->setVisible(Features & QDockWidget::DockWidgetClosable);
		};

		UpdateButtons(DockWidget->features());
		connect(DockWidget, &QDockWidget::featuresChanged, this, UpdateButtons);
	}

	QSize sizeHint() const override
	{
		return QSize(0, qMax(fontMetrics().height() + 16, 28));
	}

	QSize minimumSizeHint() const override
	{
		return sizeHint();
	}

protected:
	void paintEvent(QPaintEvent*) override
	{
		QPainter Painter(this);
		int Right = width();

		for (const QWidget* Button : findChildren<QToolButton*>(QString(), Qt::FindDirectChildrenOnly))
			if (Button->isVisible())
				Right = qMin(Right, Button->x());

		const QRect TextRect = QRect(4, 0, Right - 4, height());
		const QString Title = fontMetrics().elidedText(mDockWidget->windowTitle(), Qt::ElideRight, TextRect.width());

		Painter.setPen(palette().color(QPalette::WindowText));
		Painter.drawText(TextRect, Qt::AlignCenter, Title);
	}

	QDockWidget* mDockWidget;
};

QWidget* swCreateDockTitleBar(QDockWidget* DockWidget)
{
	return new swDockTitleBar(DockWidget);
}

// use to lock a panel, fixed panels never close
void swSetDockWidgetLocked(QDockWidget* DockWidget, bool Locked)
{
	QDockWidget::DockWidgetFeatures Features = QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable;

	if (!DockWidget->property("swFixedPanel").toBool())
		Features |= QDockWidget::DockWidgetClosable;

	// empty title bar collapses the title
	QWidget* TitleBar = DockWidget->titleBarWidget();

	DockWidget->setTitleBarWidget(Locked ? new QWidget(DockWidget) : swCreateDockTitleBar(DockWidget));
	delete TitleBar;

	DockWidget->setFeatures(Locked ? QDockWidget::NoDockWidgetFeatures : Features);
}

bool swIsInterfaceLocked()
{
	return QSettings().value("MainWindow/LockInterface", false).toBool();
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

QString swFormatValue(float Value, int Precision)
{
	QString String = QString::number(Value, 'f', Precision);
	const qsizetype Dot = String.indexOf('.');

	if (Dot != -1)
	{
		while (String.endsWith('0'))
			String.chop(1);

		if (String.endsWith('.'))
			String.chop(1);
	}

	return String;
}

QString swFormatFloatLocalized(float Value, QLocale Locale)
{
	Locale.setNumberOptions(Locale.numberOptions() | QLocale::OmitGroupSeparator);
	QString DecimalPoint = Locale.decimalPoint();
	QString String = Locale.toString(Value, 'f', 4);

	if (String.indexOf(DecimalPoint) != -1)
	{
		while (String.endsWith(Locale.zeroDigit()))
			String.chop(1);

		if (String.endsWith(DecimalPoint))
			String.chop(1);
	}

	return String;
}

QString swFormatDoubleLocalized(double Value, QLocale Locale, char Format, int Precision)
{
	Locale.setNumberOptions(Locale.numberOptions() | QLocale::OmitGroupSeparator);
	return Locale.toString(Value, Format, Precision);
}

QString swFormatIntegerLocalized(qlonglong Value, QLocale Locale)
{
	Locale.setNumberOptions(Locale.numberOptions() | QLocale::OmitGroupSeparator);
	return Locale.toString(Value);
}

std::optional<float> swParseFloatLocalized(const QString& Value, QLocale Locale)
{
	bool Ok;
	const float Number = Locale.toFloat(Value, &Ok);

	if (!Ok || !std::isfinite(Number))
		return std::nullopt;

	return Number;
}

#ifdef Q_OS_WIN

int swTerminateChildProcess(QWidget* Parent, const qint64 Pid, const qint64 Ppid)
{
	DWORD PID        = DWORD(Pid);
	DWORD PPID       = DWORD(Ppid);
	HANDLE hSnapshot = INVALID_HANDLE_VALUE, hProcess = INVALID_HANDLE_VALUE;
	PROCESSENTRY32 PE32;

	if ((hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, PPID)) == INVALID_HANDLE_VALUE)
	{
		QMessageBox::warning(Parent, QObject::tr("Stop Blender Process"), QObject::tr("%1 failed: %2").arg("CreateToolhelp32Snapshot").arg(GetLastError()));
		return -1;
	}
	PE32.dwSize = sizeof(PROCESSENTRY32);
	if (Process32First(hSnapshot, &PE32) == FALSE)
	{
		QMessageBox::warning(Parent, QObject::tr("Stop Blender Process"), QObject::tr("%1 failed: %2").arg("Process32First").arg(GetLastError()));
		CloseHandle(hSnapshot);
		return -2;
	}
	do
	{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
		QRegularExpression ExeExpression("^(?:cmd\\.exe|conhost\\.exe|blender\\.exe)$", QRegularExpression::CaseInsensitiveOption);
#else
		QRegExp ExeExpression("^(?:cmd\\.exe|conhost\\.exe|blender\\.exe)$", Qt::CaseInsensitive);
#endif

		if (QString::fromWCharArray(PE32.szExeFile).contains(ExeExpression))
		{
			if ((PE32.th32ProcessID == PID && PE32.th32ParentProcessID == PPID) || PE32.th32ParentProcessID == PID)
			{
				if ((hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, PE32.th32ProcessID)) == INVALID_HANDLE_VALUE)
				{
					QMessageBox::warning(Parent, QObject::tr("Stop Blender Process"), QObject::tr("%1 failed: %2").arg("OpenProcess").arg(GetLastError()));
					return -3;
				}
				else
				{
					TerminateProcess(hProcess, 9);
					CloseHandle(hProcess);
				}
			}
		}
	}
	while (Process32Next(hSnapshot, &PE32));

	CloseHandle(hSnapshot);

	return 0;
}

bool swRunElevatedProcess(const LPCWSTR ExeName, const LPCWSTR Arguments, const LPCWSTR WorkingFolder)
{
	SHELLEXECUTEINFO ShellExecuteInfo = {};

	ShellExecuteInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShellExecuteInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShellExecuteInfo.hwnd = NULL;
	ShellExecuteInfo.lpVerb = L"runas";
	ShellExecuteInfo.lpFile = ExeName;
	ShellExecuteInfo.lpParameters = Arguments;
	ShellExecuteInfo.lpDirectory = WorkingFolder;
	ShellExecuteInfo.nShow = SW_SHOW;
	ShellExecuteInfo.hInstApp = NULL;

	if (!ShellExecuteEx(&ShellExecuteInfo) || !ShellExecuteInfo.hProcess)
		return false;

	if (WaitForSingleObject(ShellExecuteInfo.hProcess, INFINITE))
		return false;

	DWORD ExitCode;

	if (!GetExitCodeProcess(ShellExecuteInfo.hProcess, &ExitCode))
		return false;

	CloseHandle(ShellExecuteInfo.hProcess);

	return ExitCode == 0;
}

#endif

// resize columns except the stretching one
swTreeWidgetColumnStretcher::swTreeWidgetColumnStretcher(QTreeWidget* TreeWidget, int ColumnToStretch)
	: QObject(TreeWidget->header()), mColumnToStretch(ColumnToStretch), mInteractiveResize(false), mStretchWidth(0)
{
	parent()->installEventFilter(this);
	connect(TreeWidget->header(), &QHeaderView::sectionResized, this, &swTreeWidgetColumnStretcher::SectionResized);

	QHideEvent Fake;
	swTreeWidgetColumnStretcher::eventFilter(parent(), &Fake);
}

void swTreeWidgetColumnStretcher::SectionResized(int LogicalIndex, int OldSize, int NewSize)
{
	Q_UNUSED(OldSize)

	if (LogicalIndex == mColumnToStretch)
	{
		QHeaderView* HeaderView = qobject_cast<QHeaderView*>(parent());

		if (HeaderView->isVisible())
			mInteractiveResize = true;

		mStretchWidth = NewSize;
	}
}

bool swTreeWidgetColumnStretcher::eventFilter(QObject* Object, QEvent* Event)
{
	if (Object == parent())
	{
		QHeaderView* HeaderView = qobject_cast<QHeaderView*>(Object);

		if (Event->type() == QEvent::Show)
		{
			for (int i = 0; i < HeaderView->count(); ++i)
				HeaderView->setSectionResizeMode(i, QHeaderView::Interactive);

			mStretchWidth = HeaderView->sectionSize(mColumnToStretch);

		}
		else if (Event->type() == QEvent::Hide)
		{
			if (!mInteractiveResize)
				for (int i = 0; i < HeaderView->count(); ++i)
					HeaderView->setSectionResizeMode(i, i == mColumnToStretch ? QHeaderView::Stretch : QHeaderView::ResizeToContents);
		}
		else if (Event->type() == QEvent::Resize)
		{
			if (HeaderView->sectionResizeMode(mColumnToStretch) == QHeaderView::Interactive) {

				const int StretchWidth = HeaderView->isVisible() ? mStretchWidth : 32;

				HeaderView->resizeSection(mColumnToStretch, StretchWidth);
			}
		}
	}

	return false;
}

swPartIdStringModel::swPartIdStringModel(swModel* Model, QObject* Parent)
	: QAbstractListModel(Parent)
{
	swPartsLibrary* Library = swGetPartsLibrary();
	mSortedParts = Library->GetParts();

	mSortedParts.erase(std::remove_if(mSortedParts.begin(), mSortedParts.end(), [Model](swPartData* PartData)
	{
		return PartData->IsModel() && PartData->GetModel()->IncludesModel(Model);
	}), mSortedParts.end());

	auto PartCompare = [](swPartData* PartData1, swPartData* PartData2)
	{
		return strcmp(PartData1->m_strDescription, PartData2->m_strDescription) < 0;
	};

	std::sort(mSortedParts.begin(), mSortedParts.end(), PartCompare);
}

QModelIndex swPartIdStringModel::Index(swPartData* PartData) const
{
	for (size_t PartDataIndex = 0; PartDataIndex < mSortedParts.size(); PartDataIndex++)
		if (mSortedParts[PartDataIndex] == PartData)
			return index(static_cast<int>(PartDataIndex), 0);

	return QModelIndex();
}

std::vector<bool> swPartIdStringModel::GetFilteredRows(const QString& FilterText) const
{
	const std::string Text = FilterText.toStdString();
	std::vector<bool> FilteredRows(mSortedParts.size());

	for (size_t PartDataIndex = 0; PartDataIndex < mSortedParts.size(); PartDataIndex++)
	{
		const swPartData* PartData = mSortedParts[PartDataIndex];

		FilteredRows[PartDataIndex] = (swstrcasestr(PartData->m_strDescription, Text.c_str()) || swstrcasestr(PartData->mFileName, Text.c_str()));
	}

	return FilteredRows;
}

int swPartIdStringModel::rowCount(const QModelIndex& Parent) const
{
	Q_UNUSED(Parent);

	return static_cast<int>(mSortedParts.size());
}

QVariant swPartIdStringModel::data(const QModelIndex& Index, int Role) const
{
	if (Index.row() < static_cast<int>(mSortedParts.size()))
	{
		if (Role == Qt::DisplayRole)
			return QString::fromLatin1(mSortedParts[Index.row()]->m_strDescription);
		else if (Role == Qt::UserRole)
			return QVariant::fromValue(reinterpret_cast<void*>(mSortedParts[Index.row()]));
	}

	return QVariant();
}

swColorDialogPopup::swColorDialogPopup(const QColor& InitialColor, bool ShowAlphaChannel, QWidget* Parent)
	: QWidget(Parent)
{
	setAccessibleName(tr("Choose Color"));
	setWhatsThis(tr("<p><b>Choose Color</b></p><p>Pick a color using the swatches or enter its component values. Choose OK to use the selected color, or Cancel to keep the original color. Editing the controls alone does not apply the color.</p>"));

	QVBoxLayout* Layout = new QVBoxLayout(this);
	Layout->setContentsMargins(0, 0, 0, 0);

	mColorDialog = new QColorDialog(InitialColor, this);
	mColorDialog->setWindowFlags(Qt::Widget);
	mColorDialog->setOptions(QColorDialog::DontUseNativeDialog | QColorDialog::NoButtons);
	mColorDialog->setOption(QColorDialog::ShowAlphaChannel, ShowAlphaChannel);

	// transparent start picks opaque
	mTransparentStart = ShowAlphaChannel && InitialColor.alpha() == 0;

	QObject::connect(mColorDialog, &QColorDialog::currentColorChanged, this, [this](const QColor& Color)
	{
		if (!mTransparentStart || Color.alpha() != 0)
			return;

		mTransparentStart = false;

		QColor OpaqueColor(Color);
		OpaqueColor.setAlpha(255);
		mColorDialog->setCurrentColor(OpaqueColor);
	});

	Layout->addWidget(mColorDialog);
	mColorDialog->setAccessibleName(tr("Custom Color"));
	mColorDialog->setToolTip(tr("Choose a color or enter its component values."));

	QDialogButtonBox* ButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel, this);
	Layout->addWidget(ButtonBox);
	swSetupWidgetHelp(this);

	QObject::connect(ButtonBox, &QDialogButtonBox::accepted, this, &swColorDialogPopup::Accept);
	QObject::connect(ButtonBox, &QDialogButtonBox::rejected, this, &swColorDialogPopup::Reject);
}

void swColorDialogPopup::Accept()
{
	QColor Color = mColorDialog->currentColor();

	emit ColorSelected(Color);

	Close();
}

void swColorDialogPopup::Reject()
{
	Close();
}

void swColorDialogPopup::Close()
{
	QMenu* Menu = qobject_cast<QMenu*>(parent());

	if (Menu)
		Menu->close();
}

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

	case swPaintedIcon::Undock:
		Draw.Stroke(swCreateIconRect(4, 9, 15, 20, 1));
		Draw.Stroke(swCreateIconLine({ { 9, 9 }, { 9, 4 }, { 20, 4 }, { 20, 15 }, { 15, 15 } }));
		break;

	case swPaintedIcon::Refresh:
		Draw.Stroke(swCreateIconArc(12, 12, 8, 8, -30, 290, 0, 4.5));
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
