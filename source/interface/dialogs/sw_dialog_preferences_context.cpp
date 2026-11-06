#include "sw_global.h"
#include "sw_dialog_preferences.h"
#include "ui_sw_dialog_preferences.h"
#include "sw_commands.h"

enum
{
	SW_CONTEXT_ENTRY_ROLE = Qt::UserRole + 1
};

// commands are copied out, never moved
class swContextCommandTree : public QTreeWidget
{
public:
	using QTreeWidget::QTreeWidget;

protected:
	Qt::DropActions supportedDropActions() const override
	{
		return Qt::CopyAction;
	}
};

// text, flags and font from the layout entry
static void swSetContextMenuItem(QTreeWidgetItem* Item, const QString& Entry)
{
	Qt::ItemFlags Flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled;
	QFont Font = Item->font(0);
	Font.setBold(false);
	Font.setItalic(false);

	Item->setData(0, SW_CONTEXT_ENTRY_ROLE, Entry);

	if (Entry == QLatin1String("-"))
		Item->setText(0, QString(16, QChar(0x2500)));
	else if (Entry.startsWith(QLatin1Char('>')))
	{
		Item->setText(0, qApp->translate("ContextMenu", Entry.mid(1).toUtf8().constData()));
		Font.setBold(true);
		Flags |= Qt::ItemIsDropEnabled;
	}
	else if (Entry.startsWith(QLatin1Char('@')))
	{
		Item->setText(0, qApp->translate("ContextMenu", Entry.mid(1).toUtf8().constData()) + QString::fromUtf8(" ▸"));
		Font.setItalic(true);
	}
	else
	{
		const int CommandIndex = swFindCommand(Entry);

		if (CommandIndex >= 0)
		{
			Item->setText(0, qApp->translate("Menu", gCommands[CommandIndex].MenuName).remove('&').remove(QLatin1String("...")));
			Item->setToolTip(0, qApp->translate("Status", gCommands[CommandIndex].StatusText));
		}
		else
			Item->setText(0, Entry);
	}

	Item->setFont(0, Font);
	Item->setFlags(Flags);
}

// redo flags after a drop, which gives default ones
static void swFixContextMenuItems(QTreeWidgetItem* Parent)
{
	for (int ChildIndex = 0; ChildIndex < Parent->childCount(); ChildIndex++)
	{
		QTreeWidgetItem* Child = Parent->child(ChildIndex);
		swSetContextMenuItem(Child, Child->data(0, SW_CONTEXT_ENTRY_ROLE).toString());
		swFixContextMenuItems(Child);
	}
}

void swPreferencesDialog::SetContextMenuTree(const QStringList& Layout)
{
	mContextMenuTree->clear();
	std::vector<QTreeWidgetItem*> Parents = { mContextMenuTree->invisibleRootItem() };

	for (const QString& Line : Layout)
	{
		int Depth = 0;

		while (Depth < Line.size() && Line[Depth] == QLatin1Char('\t'))
			Depth++;

		const QString Entry = Line.mid(Depth);

		if (Entry.isEmpty())
			continue;

		Depth = std::min<int>(Depth, static_cast<int>(Parents.size()) - 1);
		Parents.resize(Depth + 1);

		QTreeWidgetItem* Item = new QTreeWidgetItem(Parents.back());
		swSetContextMenuItem(Item, Entry);

		if (Entry.startsWith(QLatin1Char('>')))
			Parents.push_back(Item);
	}

	mContextMenuTree->expandAll();
}

QStringList swPreferencesDialog::GetContextMenuLayout() const
{
	QStringList Layout;

	const std::function<void(QTreeWidgetItem*, int)> AddChildren = [&](QTreeWidgetItem* Parent, int Depth)
	{
		for (int ChildIndex = 0; ChildIndex < Parent->childCount(); ChildIndex++)
		{
			QTreeWidgetItem* Child = Parent->child(ChildIndex);
			const QString Entry = Child->data(0, SW_CONTEXT_ENTRY_ROLE).toString();

			Layout.append(QString(Depth, QLatin1Char('\t')) + Entry);

			if (Entry.startsWith(QLatin1Char('>')))
				AddChildren(Child, Depth + 1);
		}
	};

	AddChildren(mContextMenuTree->invisibleRootItem(), 0);

	// the default layout is kept empty to follow new versions

	return Layout == swGetDefaultContextMenu() ? QStringList() : Layout;
}

void swPreferencesDialog::CreateContextMenuTab()
{
	QWidget* Tab = new QWidget(ui->tabWidget);
	QGridLayout* TabLayout = new QGridLayout(Tab);

	// commands on the left

	QGroupBox* CommandGroup = new QGroupBox(tr("Commands"), Tab);
	QVBoxLayout* CommandLayout = new QVBoxLayout(CommandGroup);
	QLineEdit* FilterEdit = new QLineEdit(CommandGroup);
	FilterEdit->setPlaceholderText(tr("Filter Commands"));
	FilterEdit->setToolTip(tr("Filter commands by name."));
	FilterEdit->setClearButtonEnabled(true);
	CommandLayout->addWidget(FilterEdit);

	mContextCommandTree = new swContextCommandTree(CommandGroup);
	mContextCommandTree->setHeaderHidden(true);
	mContextCommandTree->setDragDropMode(QAbstractItemView::DragOnly);
	mContextCommandTree->setToolTip(tr("Choose a command, then Add, or drag it into the context menu."));
	CommandLayout->addWidget(mContextCommandTree);

	QMap<QString, QTreeWidgetItem*> Sections;

	const auto GetSection = [this, &Sections](const QString& Name)
	{
		QTreeWidgetItem*& Section = Sections[Name];

		if (!Section)
		{
			Section = new QTreeWidgetItem(mContextCommandTree, QStringList(Name));
			QFont Font = Section->font(0);
			Font.setBold(true);
			Section->setFont(0, Font);
			Section->setFlags(Qt::ItemIsEnabled);
		}

		return Section;
	};

	GetSection(tr("Menus"));

	for (const QString& Builtin : swGetContextMenuBuiltins())
		swSetContextMenuItem(new QTreeWidgetItem(Sections[tr("Menus")]), QLatin1Char('@') + Builtin);

	for (int CommandIndex = 0; CommandIndex < SW_COMMAND_COUNT; CommandIndex++)
	{
		const QString ID = QLatin1String(gCommands[CommandIndex].ID);

		if (!ID.isEmpty() && gCommands[CommandIndex].MenuName[0])
			swSetContextMenuItem(new QTreeWidgetItem(GetSection(ID.section(QLatin1Char('.'), 0, 0))), ID);
	}

	// context menu on the right

	QGroupBox* MenuGroup = new QGroupBox(tr("Context Menu"), Tab);
	QVBoxLayout* MenuLayout = new QVBoxLayout(MenuGroup);
	mContextMenuTree = new QTreeWidget(MenuGroup);
	mContextMenuTree->setHeaderHidden(true);
	mContextMenuTree->setDragDropMode(QAbstractItemView::DragDrop);
	mContextMenuTree->setDefaultDropAction(Qt::MoveAction);
	mContextMenuTree->setToolTip(tr("The right-click menu of the model view. Drag entries to reorder them or into a submenu. Control point commands only show for flexible parts."));
	MenuLayout->addWidget(mContextMenuTree);

	QToolButton* AddButton = new QToolButton(Tab);
	AddButton->setArrowType(Qt::RightArrow);
	AddButton->setToolTip(tr("Add the chosen command below the selected entry, or into the selected submenu."));

	QHBoxLayout* LeftButtons = new QHBoxLayout;
	QPushButton* ResetButton = new QPushButton(tr("Reset..."), Tab);
	ResetButton->setToolTip(tr("Restore the default context menu after confirmation."));
	LeftButtons->addWidget(ResetButton);
	LeftButtons->addStretch();

	QHBoxLayout* RightButtons = new QHBoxLayout;
	QPushButton* UpButton = new QPushButton(tr("Up"), Tab);
	QPushButton* DownButton = new QPushButton(tr("Down"), Tab);
	QPushButton* RemoveButton = new QPushButton(tr("Remove"), Tab);
	QPushButton* SeparatorButton = new QPushButton(tr("Separator"), Tab);
	QPushButton* SubmenuButton = new QPushButton(tr("Submenu..."), Tab);
	QPushButton* RenameButton = new QPushButton(tr("Rename..."), Tab);
	UpButton->setToolTip(tr("Move the selected entry up."));
	DownButton->setToolTip(tr("Move the selected entry down."));
	RemoveButton->setToolTip(tr("Remove the selected entry and anything inside it."));
	SeparatorButton->setToolTip(tr("Add a separator line below the selected entry."));
	SubmenuButton->setToolTip(tr("Add a named submenu below the selected entry."));
	RenameButton->setToolTip(tr("Rename the selected submenu."));
	RightButtons->addStretch();

	for (QPushButton* Button : { UpButton, DownButton, RemoveButton, SeparatorButton, SubmenuButton, RenameButton })
		RightButtons->addWidget(Button);

	TabLayout->addWidget(CommandGroup, 0, 0);
	TabLayout->addWidget(AddButton, 0, 1);
	TabLayout->addWidget(MenuGroup, 0, 2);
	TabLayout->addLayout(LeftButtons, 1, 0);
	TabLayout->addLayout(RightButtons, 1, 2);

	ui->tabWidget->insertTab(ui->tabWidget->indexOf(ui->tabKeyboard), Tab, tr("Context Menu"));

	const QStringList& Custom = mOptions->Preferences.mContextMenu;
	SetContextMenuTree(Custom.isEmpty() ? swGetDefaultContextMenu() : Custom);

	// insert below the selection, or into a selected submenu

	const auto InsertEntry = [this](const QString& Entry)
	{
		QTreeWidgetItem* Current = mContextMenuTree->currentItem();
		QTreeWidgetItem* Item = new QTreeWidgetItem;
		swSetContextMenuItem(Item, Entry);

		if (!Current)
			mContextMenuTree->addTopLevelItem(Item);
		else if (Current->data(0, SW_CONTEXT_ENTRY_ROLE).toString().startsWith(QLatin1Char('>')))
		{
			Current->addChild(Item);
			Current->setExpanded(true);
		}
		else
		{
			QTreeWidgetItem* Parent = Current->parent() ? Current->parent() : mContextMenuTree->invisibleRootItem();
			Parent->insertChild(Parent->indexOfChild(Current) + 1, Item);
		}

		mContextMenuTree->setCurrentItem(Item);
	};

	const auto AddCommand = [this, InsertEntry]()
	{
		const QTreeWidgetItem* Current = mContextCommandTree->currentItem();
		const QString Entry = Current ? Current->data(0, SW_CONTEXT_ENTRY_ROLE).toString() : QString();

		if (!Entry.isEmpty())
			InsertEntry(Entry);
	};

	const auto MoveEntry = [this](int Offset)
	{
		QTreeWidgetItem* Current = mContextMenuTree->currentItem();

		if (!Current)
			return;

		QTreeWidgetItem* Parent = Current->parent() ? Current->parent() : mContextMenuTree->invisibleRootItem();
		const int Index = Parent->indexOfChild(Current) + Offset;

		if (Index < 0 || Index >= Parent->childCount())
			return;

		const bool Expanded = Current->isExpanded();
		Parent->takeChild(Index - Offset);
		Parent->insertChild(Index, Current);
		Current->setExpanded(Expanded);
		mContextMenuTree->setCurrentItem(Current);
	};

	connect(AddButton, &QToolButton::clicked, this, AddCommand);
	connect(mContextCommandTree, &QTreeWidget::itemDoubleClicked, this, AddCommand);
	connect(UpButton, &QPushButton::clicked, this, [MoveEntry]() { MoveEntry(-1); });
	connect(DownButton, &QPushButton::clicked, this, [MoveEntry]() { MoveEntry(1); });
	connect(SeparatorButton, &QPushButton::clicked, this, [InsertEntry]() { InsertEntry(QLatin1String("-")); });

	connect(RemoveButton, &QPushButton::clicked, this, [this]()
	{
		delete mContextMenuTree->currentItem();
	});

	connect(SubmenuButton, &QPushButton::clicked, this, [this, InsertEntry]()
	{
		const QString Name = QInputDialog::getText(this, tr("Submenu"), tr("Name:")).trimmed();

		if (!Name.isEmpty())
			InsertEntry(QLatin1Char('>') + Name);
	});

	connect(RenameButton, &QPushButton::clicked, this, [this]()
	{
		QTreeWidgetItem* Current = mContextMenuTree->currentItem();

		if (!Current || !Current->data(0, SW_CONTEXT_ENTRY_ROLE).toString().startsWith(QLatin1Char('>')))
			return;

		const QString Name = QInputDialog::getText(this, tr("Rename Submenu"), tr("Name:"), QLineEdit::Normal, Current->text(0)).trimmed();

		if (!Name.isEmpty())
			swSetContextMenuItem(Current, QLatin1Char('>') + Name);
	});

	connect(ResetButton, &QPushButton::clicked, this, [this]()
	{
		if (QMessageBox::question(this, tr("Reset Context Menu"), tr("Are you sure you want to load the default context menu?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
			SetContextMenuTree(swGetDefaultContextMenu());
	});

	connect(FilterEdit, &QLineEdit::textEdited, this, [this](const QString& Text)
	{
		for (int SectionIndex = 0; SectionIndex < mContextCommandTree->topLevelItemCount(); SectionIndex++)
		{
			QTreeWidgetItem* Section = mContextCommandTree->topLevelItem(SectionIndex);
			bool AnyVisible = false;

			for (int ChildIndex = 0; ChildIndex < Section->childCount(); ChildIndex++)
			{
				QTreeWidgetItem* Child = Section->child(ChildIndex);
				const bool Visible = Child->text(0).contains(Text, Qt::CaseInsensitive);
				Child->setHidden(!Visible);
				AnyVisible |= Visible;
			}

			Section->setHidden(!AnyVisible);
			Section->setExpanded(!Text.isEmpty());
		}
	});

	connect(mContextMenuTree->model(), &QAbstractItemModel::rowsInserted, this, [this]()
	{
		QTimer::singleShot(0, this, [this]() { swFixContextMenuItems(mContextMenuTree->invisibleRootItem()); });
	});
}
