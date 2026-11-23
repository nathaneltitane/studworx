#include "sw_global.h"
#include "sw_window_utilities.h"

// use to fit a scroll area to its rows
static void swFitScrollAreaWidth(QScrollArea* ScrollArea)
{
	// panels sizing themselves opt out
	if (ScrollArea->widget() && !ScrollArea->property("swKeepWidth").toBool())
		ScrollArea->setMinimumWidth(ScrollArea->widget()->minimumSizeHint().width() + ScrollArea->verticalScrollBar()->sizeHint().width() + 2 * ScrollArea->frameWidth());
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
			else if (Tries == 2)
			{
				// remapping clears stale manager state
				DockWidget->hide();
				DockWidget->show();
				DockWidget->resize(DockWidget->width(), Height);
			}
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

		// docked panels scroll when squeezed
		for (QScrollArea* ScrollArea : DockWidget->findChildren<QScrollArea*>())
			ScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

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
				// floating size panels and strips stretch
				if (AreaDockWidget->isFloating() || MainWindow->dockWidgetArea(AreaDockWidget) != Area || swIsViewDock(AreaDockWidget) || AreaDockWidget->property("swFloatingSize").toSize().isValid() || AreaDockWidget->property("swStrip").toBool())
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

	bool Cut = false;

	if (const QScreen* Screen = DockWidget->screen())
	{
		const int ScreenHeight = Screen->availableGeometry().height() - DockWidget->frameGeometry().height() + DockWidget->height();

		// scroll a panel cut by the screen
		if (Height > ScreenHeight)
		{
			Height = ScreenHeight;
			Cut = true;

			for (const QScrollArea* ScrollArea : ScrollAreas)
				if (ScrollArea->widget() && ScrollArea->isVisibleTo(DockWidget))
					Width += qMax(ScrollArea->widget()->minimumSizeHint().width() + ScrollArea->verticalScrollBar()->sizeHint().width() + 2 * ScrollArea->frameWidth() - ScrollArea->minimumSizeHint().expandedTo(ScrollArea->minimumSize()).width(), 0);
		}

		if (MaximumHeight < QWIDGETSIZE_MAX)
			MaximumHeight = qMax(qMin(MaximumHeight, ScreenHeight), Height);
	}

	// a passing scroll bar reflows the contents
	for (QScrollArea* ScrollArea : ScrollAreas)
		ScrollArea->setVerticalScrollBarPolicy(Cut ? Qt::ScrollBarAsNeeded : Qt::ScrollBarAlwaysOff);

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

// use to fold a docked panel to its title or open it back
static void swToggleDockFolded(QDockWidget* DockWidget)
{
	QMainWindow* MainWindow = qobject_cast<QMainWindow*>(DockWidget->parentWidget());

	if (!MainWindow || DockWidget->isFloating())
		return;

	// the panels stacked in its column
	const Qt::DockWidgetArea Area = MainWindow->dockWidgetArea(DockWidget);
	const int Across = DockWidget->geometry().center().x();
	QList<QDockWidget*> Line;

	for (QDockWidget* Other : MainWindow->findChildren<QDockWidget*>(QString(), Qt::FindDirectChildrenOnly))
		if (Other->isVisible() && !Other->isFloating() && MainWindow->dockWidgetArea(Other) == Area && Across >= Other->geometry().left() && Across <= Other->geometry().right())
			Line.append(Other);

	std::sort(Line.begin(), Line.end(), [](QDockWidget* First, QDockWidget* Second) { return First->y() < Second->y(); });

	const int Folded = DockWidget->minimumSizeHint().height();
	const int Size = DockWidget->height();
	const bool Fold = Size > Folded;
	int Target = Folded;

	if (Fold)
		DockWidget->setProperty("swDockOpenHeight", Size);
	else
	{
		QWidget* Content = DockWidget->widget();
		const int Remembered = DockWidget->property("swDockOpenHeight").toInt();
		Target = Remembered > Folded ? Remembered : Folded + (Content ? Content->sizeHint().height() : 0);
		Target = qMin(Target, DockWidget->maximumHeight());
	}

	// the view takes or gives first, then the far panels inward
	QWidget* CentralWidget = MainWindow->centralWidget();
	const bool ViewInLine = Area == Qt::TopDockWidgetArea || Area == Qt::BottomDockWidgetArea;
	const int Index = Line.indexOf(DockWidget);
	QList<int> Heights;

	for (QDockWidget* Other : Line)
		Heights.append(Other->height());

	int Rest = qAbs(Target - Size);

	if (ViewInLine)
		Rest -= Fold ? Rest : qMin(Rest, CentralWidget ? qMax(0, CentralWidget->height() - CentralWidget->minimumSizeHint().height()) : 0);

	// folding drops the title onto the panels below, the open panel above grows
	QList<int> Order;

	if (Fold)
	{
		for (int Other = Index - 1; Other >= 0; Other--)
			Order.append(Other);

		for (int Other = Index + 1; Other < Line.size(); Other++)
			Order.append(Other);
	}
	else
	{
		// stretching panels give first, largest first, panels fitting their contents last
		for (int Other = 0; Other < Line.size(); Other++)
			if (Other != Index)
				Order.append(Other);

		const auto Fits = [&Line](int Other)
		{
			QWidget* Content = Line[Other]->widget();
			return Content && QWidgetItem(Content).maximumSize().height() < QWIDGETSIZE_MAX;
		};

		std::stable_sort(Order.begin(), Order.end(), [&Fits, &Heights](int First, int Second)
		{
			const bool FirstFits = Fits(First);
			const bool SecondFits = Fits(Second);
			return FirstFits != SecondFits ? SecondFits : (!FirstFits && Heights[First] > Heights[Second]);
		});
	}

	// folded panels stay folded unless nothing else takes the space
	for (int Pass = 0; Pass < 2 && Rest > 0; Pass++)
	{
		for (int Other : Order)
		{
			if (Rest <= 0)
				break;

			const int Minimum = Line[Other]->minimumSizeHint().height();

			if (Fold && Pass == 0 && Heights[Other] <= Minimum)
				continue;

			const int Room = Fold ? Line[Other]->maximumHeight() - Heights[Other] : Heights[Other] - Minimum;
			const int Move = qMin(Rest, qMax(0, Room));
			Heights[Other] += Fold ? Move : -Move;
			Rest -= Move;
		}
	}

	Heights[Index] = Fold ? Size - (qAbs(Target - Size) - Rest) : Size + (qAbs(Target - Size) - Rest);
	MainWindow->resizeDocks(Line, Heights, Qt::Vertical);
}

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

		QToolButton* FoldButton = new swDockTitleButton(swPaintedIcon::ArrowDown, this);
		Layout->addWidget(FoldButton);

		QToolButton* FloatButton = new swDockTitleButton(swPaintedIcon::Undock, this);
		FloatButton->setToolTip(QDockWidget::tr("Float"));
		Layout->addWidget(FloatButton);

		QToolButton* CloseButton = new swDockTitleButton(swPaintedIcon::Close, this);
		CloseButton->setToolTip(QDockWidget::tr("Close"));
		Layout->addWidget(CloseButton);

		connect(FoldButton, &QToolButton::clicked, DockWidget, [DockWidget]() { swToggleDockFolded(DockWidget); });
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

		mFoldButton = FoldButton;
		UpdateFoldButton();
		connect(DockWidget, &QDockWidget::topLevelChanged, this, [this]() { UpdateFoldButton(); });
		DockWidget->installEventFilter(this);
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
	// the chevron follows the panel height
	void UpdateFoldButton()
	{
		const bool Folded = mDockWidget->height() <= mDockWidget->minimumSizeHint().height();
		mFoldButton->setVisible(!mDockWidget->isFloating());
		mFoldButton->setIcon(swCreatePaintedIcon(Folded ? swPaintedIcon::ArrowUp : swPaintedIcon::ArrowDown));
		mFoldButton->setToolTip(Folded ? QDockWidget::tr("Expand") : QDockWidget::tr("Collapse"));
	}

	bool eventFilter(QObject* Object, QEvent* Event) override
	{
		if (Object == mDockWidget && Event->type() == QEvent::Resize)
			UpdateFoldButton();

		return QWidget::eventFilter(Object, Event);
	}

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
	QToolButton* mFoldButton = nullptr;
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
