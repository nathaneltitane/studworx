#include "sw_global.h"
#include "sw_window_main.h"
#include "sw_view.h"
#include "sw_project.h"
#include "sw_dialog_instructions.h"
#include "sw_dialog_instructions_page.h"

// one line per command: path > item, tab, shortcut

static QString GetActionText(const QAction* Action)
{
	QString Text = Action->text();

	Text.replace(QLatin1String("&&"), QString(QChar(1)));
	Text.remove(QLatin1Char('&'));
	Text.replace(QChar(1), QLatin1Char('&'));
	Text.remove(QLatin1String("..."));

	return Text.trimmed();
}

// recent files, models and cameras name the user's own files

static bool IsListAction(const QAction* Action)
{
	QAction** Actions = gMainWindow->mActions;
	const std::pair<int, int> Ranges[] =
	{
		{ SW_FILE_RECENT_FIRST, SW_FILE_RECENT_LAST },
		{ SW_MODEL_FIRST, SW_MODEL_LAST },
		{ SW_VIEW_CAMERA_FIRST, SW_VIEW_CAMERA_LAST }
	};

	for (const std::pair<int, int>& Range : Ranges)
		for (int CommandIndex = Range.first; CommandIndex <= Range.second; CommandIndex++)
			if (Actions[CommandIndex] == Action)
				return true;

	return false;
}

static void WriteMenu(QStringList& Lines, const QString& Path, QMenu* Menu, bool Prepare)
{
	// fill menus built when opened

	if (Prepare)
		emit Menu->aboutToShow();

	for (QAction* Action : Menu->actions())
	{
		if (Action->isSeparator() || !Action->isVisible() || qobject_cast<QWidgetAction*>(Action) || IsListAction(Action))
			continue;

		const QString Text = GetActionText(Action);

		if (Text.isEmpty())
			continue;

		if (Action->menu())
			WriteMenu(Lines, Path + QLatin1String(" > ") + Text, Action->menu(), true);
		else
			Lines.append(Path + QLatin1String(" > ") + Text + QLatin1Char('\t') + Action->shortcut().toString(QKeySequence::PortableText));
	}
}

// records each popup menu as it opens, then closes it

class swMenuRecorder : public QObject
{
public:
	swMenuRecorder(QStringList& Lines)
		: mLines(Lines)
	{
	}

	// true when a menu opened
	bool Open(const QString& Path, const std::function<void()>& Trigger)
	{
		mPath = Path;
		Trigger();

		const bool Opened = mPath.isEmpty();
		mPath.clear();

		return Opened;
	}

protected:
	bool eventFilter(QObject* Object, QEvent* Event) override
	{
		QMenu* Menu = qobject_cast<QMenu*>(Object);

		if (Menu && Event->type() == QEvent::Show && !mPath.isEmpty())
		{
			WriteMenu(mLines, mPath, Menu, false);
			mPath.clear();
			QTimer::singleShot(0, Menu, &QMenu::close);
		}

		return false;
	}

	QStringList& mLines;
	QString mPath;
};

// scroll areas take the click on their viewport

static void SendRightClick(QWidget* Widget, const QPoint& Position)
{
	QAbstractScrollArea* ScrollArea = qobject_cast<QAbstractScrollArea*>(Widget);
	QWidget* Target = ScrollArea ? ScrollArea->viewport() : Widget;
	QContextMenuEvent Event(QContextMenuEvent::Mouse, Position, Target->mapToGlobal(Position));

	QCoreApplication::sendEvent(Target, &Event);
}

// where a widget sits, for the menu paths

static QString GetPlace(QWidget* Widget)
{
	for (QWidget* Parent = Widget; Parent; Parent = Parent->parentWidget())
	{
		if (QDockWidget* Dock = qobject_cast<QDockWidget*>(Parent))
			return Dock->windowTitle() + QLatin1String(" panel");

		if (qobject_cast<QTabBar*>(Parent))
			return QStringLiteral("a model tab");

		if (qobject_cast<QStatusBar*>(Parent))
			return QStringLiteral("the status bar");

		if (Parent->inherits("swFindReplaceWidget"))
			return QStringLiteral("the find bar");
	}

	return Widget->accessibleName();
}

// gear buttons and right click menus under one window

static void WriteWindowMenus(QStringList& Lines, swMenuRecorder& Recorder, QWidget* Window, const QString& Prefix)
{
	for (QToolButton* Button : Window->findChildren<QToolButton*>())
	{
		if (!Button->menu() || qobject_cast<QToolBar*>(Button->parentWidget()))
			continue;

		const QString Name = Button->accessibleName().isEmpty() ? Button->toolTip() : Button->accessibleName();
		const QString Place = GetPlace(Button);

		WriteMenu(Lines, Prefix + (Place == Name ? Name : Place + QLatin1String(" > ") + Name), Button->menu(), true);
	}

	for (QWidget* Widget : Window->findChildren<QWidget*>())
	{
		const QString Path = Prefix + QStringLiteral("Right click ") + GetPlace(Widget);

		if (Widget->contextMenuPolicy() == Qt::CustomContextMenu)
		{
			QTabBar* TabBar = qobject_cast<QTabBar*>(Widget);
			const QPoint Position = TabBar && TabBar->count() ? TabBar->tabRect(0).center() : Widget->rect().center();

			Recorder.Open(Path, [Widget, Position]() { emit Widget->customContextMenuRequested(Position); });
		}
		else if (Widget->inherits("swKeyframesView"))
		{
			// first row that holds a track, sized while its panel is hidden

			if (Widget->height() < 100)
				Widget->resize(800, 400);

			for (int Y = 0; Y < Widget->height(); Y += 4)
				if (Recorder.Open(Path, [Widget, Y]() { SendRightClick(Widget, QPoint(Widget->width() / 2, Y)); }))
					break;
		}
	}
}

// a spot where this element is the topmost one

static std::optional<QPointF> GetPickPosition(QGraphicsScene* Scene, QGraphicsItem* Item)
{
	const QRectF Rect = Item->sceneBoundingRect();

	for (int Row = 1; Row < 20; Row++)
	{
		for (int Column = 1; Column < 20; Column++)
		{
			const QPointF Position(Rect.left() + Rect.width() * Column / 20.0, Rect.top() + Rect.height() * Row / 20.0);

			for (QGraphicsItem* Top : Scene->items(Position))
			{
				swInstructionsElementKey Key;

				if (swGetElementKey(Top, Key))
				{
					if (Top == Item)
						return Position;

					break;
				}
			}
		}
	}

	return std::nullopt;
}

// page, element, notice and pointer menus of the instructions

static void WritePageMenus(QStringList& Lines, swMenuRecorder& Recorder, swInstructionsPageWidget* PageWidget, const QString& Prefix)
{
	swInstructions* Instructions = PageWidget->GetInstructions();

	if (Instructions->mPages.empty())
		return;

	PageWidget->resize(800, 1000);
	PageWidget->SetCurrentPage(&Instructions->mPages[0]);

	// a notice and a pointer, removed before leaving

	swInstructionsElementKey BoxKey = { swInstructionsPlacementElement::None, nullptr, 0 };
	QRectF BoxRect;

	for (QGraphicsItem* Item : PageWidget->scene()->items())
	{
		if (dynamic_cast<swInstructionsBoxBase*>(Item) && swGetElementKey(Item, BoxKey))
		{
			BoxRect = Item->sceneBoundingRect();
			break;
		}
	}

	const int NoticeId = Instructions->AddNotice(0, QPointF(10.0, 10.0), QStringLiteral("Notice"));
	const QPointF TipPosition = BoxRect.center() + QPointF(BoxRect.width(), 0.0);
	const int PointerId = std::get<0>(BoxKey) == swInstructionsPlacementElement::None ? -1 : Instructions->AddPointer(0, BoxKey, TipPosition);

	PageWidget->SetCurrentPage(&Instructions->mPages[0]);

	for (QGraphicsItem* Item : PageWidget->scene()->items())
	{
		swInstructionsElementKey Key;

		if (!swGetElementKey(Item, Key))
			continue;

		const swInstructionsPlacementElement Element = std::get<0>(Key);
		const QString Name = Element == swInstructionsPlacementElement::Pointer ? QStringLiteral("a pointer") : Element == swInstructionsPlacementElement::Notice ? QStringLiteral("a notice") : QStringLiteral("a page element");
		const std::optional<QPointF> PickPosition = GetPickPosition(PageWidget->scene(), Item);

		if (!PickPosition)
			continue;

		const QPoint Position = PageWidget->mapFromScene(*PickPosition);

		Recorder.Open(Prefix + QStringLiteral("Right click ") + Name, [PageWidget, Position]() { SendRightClick(PageWidget, Position); });
	}

	Instructions->RemoveNotice(NoticeId);

	if (PointerId >= 0)
		Instructions->RemovePointer(PointerId);
}

QString swMainWindow::GetMenuText()
{
	QStringList Lines;
	swMenuRecorder Recorder(Lines);
	qApp->installEventFilter(&Recorder);

	// menu bar

	for (QAction* Action : menuBar()->actions())
		if (Action->menu())
			WriteMenu(Lines, GetActionText(Action), Action->menu(), true);

	// toolbars and their drop downs

	for (QToolBar* ToolBar : findChildren<QToolBar*>())
	{
		const QString Path = ToolBar->windowTitle() + QLatin1String(" toolbar");

		for (QAction* Action : ToolBar->actions())
		{
			if (Action->isSeparator() || qobject_cast<QWidgetAction*>(Action))
				continue;

			if (Action->menu())
				WriteMenu(Lines, Path + QLatin1String(" > ") + GetActionText(Action), Action->menu(), true);
			else
				Lines.append(Path + QLatin1String(" > ") + GetActionText(Action) + QLatin1Char('\t') + Action->shortcut().toString(QKeySequence::PortableText));
		}
	}

	std::unique_ptr<QMenu> PopupMenu(createPopupMenu());

	if (PopupMenu)
		WriteMenu(Lines, QStringLiteral("Right click a toolbar or panel title"), PopupMenu.get(), false);

	// panels built when first shown

	emit mMinifigureBuilderToolBar->visibilityChanged(true);

	// two model tabs, for the tab menu

	Project* ActiveProject = swGetActiveProject();
	const std::vector<std::unique_ptr<swModel>>& Models = ActiveProject->GetModels();

	if (Models.size() > 1)
	{
		SetCurrentModelTab(Models[1].get(), false);
		SetCurrentModelTab(Models[0].get(), false);
	}

	// fuller menus with every part selected

	mActions[SW_EDIT_SELECT_ALL]->trigger();
	emit mKeyframesToolBar->visibilityChanged(true);

	swView* ActiveView = GetActiveView();

	if (ActiveView)
	{
		Recorder.Open(QStringLiteral("Right click the model view"), [ActiveView]() { ActiveView->ShowContextMenu(true); });
		ActiveView->ShowFindReplaceWidget(false);
	}

	WriteWindowMenus(Lines, Recorder, this, QString());

	// instructions window, never shown

	std::unique_ptr<swInstructionsDialog> Instructions(new swInstructionsDialog(this, ActiveProject));
	const QString Prefix = QStringLiteral("Instructions window > ");

	WriteWindowMenus(Lines, Recorder, Instructions.get(), Prefix);

	swInstructionsPageWidget* PageWidget = Instructions->findChild<swInstructionsPageWidget*>();

	if (PageWidget)
		WritePageMenus(Lines, Recorder, PageWidget, Prefix);

	// unit buttons are off while the document units apply

	for (QToolButton* Button : Instructions->findChildren<QToolButton*>())
	{
		if (Button->inherits("swInstructionsUnitButton"))
		{
			Button->setEnabled(true);
			Recorder.Open(Prefix + QStringLiteral("Unit button beside a value"), [Button]() { Button->click(); });
			break;
		}
	}

	Instructions.reset();
	qApp->removeEventFilter(&Recorder);

	// commands in no menu still take a shortcut in the preferences

	for (int CommandIndex = 0; CommandIndex < SW_COMMAND_COUNT; CommandIndex++)
	{
		QAction* Action = mActions[CommandIndex];

		if (!Action || IsListAction(Action))
			continue;

		const QString Text = GetActionText(Action);
		const QString Item = QLatin1String(" > ") + Text + QLatin1Char('\t');

		if (Text.isEmpty() || std::any_of(Lines.cbegin(), Lines.cend(), [&Item](const QString& Line) { return Line.contains(Item); }))
			continue;

		Lines.append(QStringLiteral("Preferences > Keyboard") + Item + Action->shortcut().toString(QKeySequence::PortableText));
	}

	Lines.removeDuplicates();

	return Lines.join(QLatin1Char('\n')) + QLatin1Char('\n');
}
