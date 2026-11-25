#include "sw_global.h"
#include "sw_dialog_instructions.h"
#include "sw_dialog_instructions_editor.h"
#include "sw_dialog_instructions_page.h"
#include "sw_dialog_instructions_placement.h"
#include "sw_dialog_instructions_print.h"
#include "sw_dialog_instructions_properties.h"
#include "sw_project.h"
#include "sw_model.h"
#include "sw_object_part.h"
#include "sw_data_part.h"
#include "sw_window_utilities.h"
#include "sw_profile.h"
#include "sw_window_main.h"
#include "sw_widget_spin_box.h"

swInstructionsDialog::swInstructionsDialog(QWidget* Parent, Project* Project)
	: QMainWindow(Parent), mProject(Project)
{
	setWindowModified(mProject->IsModified());
	setWindowTitle(isWindowModified() ? tr("Instructions") + QLatin1String(" *") : tr("Instructions"));

	mInstructions = mProject->GetInstructions();

	mPropertiesWidget = new swInstructionsPropertiesWidget(this, mInstructions);
	mPropertiesWidget->setObjectName("InstructionsProperties");
	addDockWidget(Qt::RightDockWidgetArea, mPropertiesWidget);

	mPlacementWidget = new swInstructionsPlacementWidget(this, mInstructions);
	mPlacementWidget->setObjectName("InstructionsPlacement");
	addDockWidget(Qt::RightDockWidgetArea, mPlacementWidget);
	splitDockWidget(mPropertiesWidget, mPlacementWidget, Qt::Vertical);

	mGridWidget = new swInstructionsGridWidget(this, mInstructions);
	mGridWidget->setObjectName("InstructionsGrid");
	addDockWidget(Qt::RightDockWidgetArea, mGridWidget);
	splitDockWidget(mPlacementWidget, mGridWidget, Qt::Vertical);

	mPageWidget = new swInstructionsPageWidget(this, mInstructions, mPropertiesWidget, mPlacementWidget);

	mPageListWidget = new swInstructionsPageListWidget(this, mInstructions);
	mPageListWidget->setObjectName("InstructionsPageList");
	addDockWidget(Qt::LeftDockWidgetArea, mPageListWidget);

	// page bar sits under the page
	QWidget* PageArea = new QWidget(this);
	QVBoxLayout* PageAreaLayout = new QVBoxLayout(PageArea);
	PageAreaLayout->setContentsMargins(0, 0, 0, 0);

	// the bar margins space it evenly
	PageAreaLayout->setSpacing(0);
	PageAreaLayout->addWidget(mPageWidget, 1);
	PageAreaLayout->addWidget(CreatePageBar());
	setCentralWidget(PageArea);

	// step shortcuts turn pages, kept off the editor panel
	const std::pair<int, QToolButton*> PageCommands[] =
	{
		{ SW_VIEW_TIME_FIRST, mFirstButton },
		{ SW_VIEW_TIME_PREVIOUS, mPreviousButton },
		{ SW_VIEW_TIME_NEXT, mNextButton },
		{ SW_VIEW_TIME_LAST, mLastButton }
	};

	for (const auto& [CommandId, Button] : PageCommands)
	{
		QAction* StepAction = gMainWindow->mActions[CommandId];

		for (QWidget* Target : { PageArea, static_cast<QWidget*>(mPageListWidget) })
		{
			QShortcut* Shortcut = new QShortcut(StepAction->shortcut(), Target, nullptr, nullptr, Qt::WidgetWithChildrenShortcut);
			connect(Shortcut, &QShortcut::activated, Button, &QToolButton::click);
			connect(StepAction, &QAction::changed, Shortcut, [Shortcut, StepAction]() { Shortcut->setKey(StepAction->shortcut()); });
		}
	}

	// undo keys follow the main window, text boxes keep their own

	const std::pair<int, QToolButton*> HistoryCommands[] =
	{
		{ SW_EDIT_UNDO, mUndoButton },
		{ SW_EDIT_REDO, mRedoButton }
	};

	for (const auto& [CommandId, Button] : HistoryCommands)
	{
		QAction* HistoryAction = gMainWindow->mActions[CommandId];
		QShortcut* Shortcut = new QShortcut(HistoryAction->shortcut(), this, nullptr, nullptr, Qt::WindowShortcut);
		connect(Shortcut, &QShortcut::activated, Button, &QToolButton::click);
		connect(HistoryAction, &QAction::changed, Shortcut, [Shortcut, HistoryAction]() { Shortcut->setKey(HistoryAction->shortcut()); });
	}

	// delete key follows the main window

	QAction* DeleteAction = gMainWindow->mActions[SW_PART_DELETE];
	const std::pair<QWidget*, std::function<void()>> DeleteTargets[] =
	{
		{ PageArea, [this]() { mPageWidget->RemoveSelectedElements(); } },
		{ mPageListWidget, [this]() { RemovePage(); } }
	};

	for (const auto& [Target, Remove] : DeleteTargets)
	{
		QShortcut* Shortcut = new QShortcut(DeleteAction->shortcut(), Target, nullptr, nullptr, Qt::WidgetWithChildrenShortcut);
		connect(Shortcut, &QShortcut::activated, this, Remove);
		connect(DeleteAction, &QAction::changed, Shortcut, [Shortcut, DeleteAction]() { Shortcut->setKey(DeleteAction->shortcut()); });
	}

	// paste key reaches the page from the page list

	QAction* PasteAction = gMainWindow->mActions[SW_EDIT_PASTE];
	QShortcut* PasteShortcut = new QShortcut(PasteAction->shortcut(), mPageListWidget, nullptr, nullptr, Qt::WidgetWithChildrenShortcut);
	connect(PasteShortcut, &QShortcut::activated, mPageWidget, &swInstructionsPageWidget::PasteElements);
	connect(PasteAction, &QAction::changed, PasteShortcut, [PasteShortcut, PasteAction]() { PasteShortcut->setKey(PasteAction->shortcut()); });

	// print panel opens below the grid
	mPrintWidget = new swInstructionsPrintWidget(this, mInstructions);
	mPrintWidget->setObjectName("InstructionsPrint");
	addDockWidget(Qt::RightDockWidgetArea, mPrintWidget);
	splitDockWidget(mGridWidget, mPrintWidget, Qt::Vertical);
	mPrintWidget->hide();

	// print button toggles the print panel
	QAction* PrintAction = mPrintWidget->toggleViewAction();
	PrintAction->setToolTip(tr("Show or hide the Print panel to print the instruction pages"));

	// html export draws these pages
	QAction* ExportAction = new QAction(tr("Export HTML"), this);
	ExportAction->setToolTip(tr("Export the instruction pages as HTML pages and images"));
	connect(ExportAction, &QAction::triggered, this, [this]() { gMainWindow->ShowHTMLDialog(this); });

	// editor panel opens below the page
	mEditorWidget = new swInstructionsEditorWidget(this, mProject, mInstructions);
	mEditorWidget->setObjectName("InstructionsEditor");
	addDockWidget(Qt::BottomDockWidgetArea, mEditorWidget);
	mEditorWidget->hide();

	// save key saves the model, the editor applies first

	QAction* SaveAction = gMainWindow->mActions[SW_FILE_SAVE];
	QShortcut* SaveShortcut = new QShortcut(SaveAction->shortcut(), this, nullptr, nullptr, Qt::WindowShortcut);
	connect(SaveShortcut, &QShortcut::activated, this, [this, SaveAction]()
	{
		if (mEditorWidget->isAncestorOf(QApplication::focusWidget()))
			mEditorWidget->ApplyText(true);
		else
			SaveAction->trigger();
	});
	connect(SaveAction, &QAction::changed, SaveShortcut, [SaveShortcut, SaveAction]() { SaveShortcut->setKey(SaveAction->shortcut()); });

	QAction* EditorAction = mEditorWidget->toggleViewAction();
	EditorAction->setShortcut(QKeySequence(tr("Ctrl+Shift+E")));
	addAction(EditorAction);
	EditorAction->setToolTip(tr("Show or hide the Editor panel to inspect and edit the text of the model file"));

	// editor, export and print read left to right
	mPropertiesWidget->AddOptionsAction(EditorAction);
	mPropertiesWidget->AddOptionsAction(ExportAction);
	mPropertiesWidget->AddOptionsAction(PrintAction);

	connect(mPageListWidget->mThumbnailsWidget, &QListWidget::currentRowChanged, this, &swInstructionsDialog::CurrentThumbnailChanged);
	UpdatePageList();

	// pages panel fits its longest name
	mPageListWidget->installEventFilter(this);
	installEventFilter(this);
	connect(mPageListWidget, &QDockWidget::dockLocationChanged, this, [this]()
	{
		QTimer::singleShot(0, this, &swInstructionsDialog::SnapPageListDock);
	});
	QTimer::singleShot(0, this, &swInstructionsDialog::SnapPageListDock);

	// editor panel shows whole lines
	mEditorSnapTimer = new QTimer(this);
	mEditorSnapTimer->setSingleShot(true);
	mEditorSnapTimer->setInterval(100);
	connect(mEditorSnapTimer, &QTimer::timeout, this, &swInstructionsDialog::SnapEditorDock);

	mEditorWidget->installEventFilter(this);
	connect(mEditorWidget, &swInstructionsEditorWidget::SnapRequested, mEditorSnapTimer, QOverload<>::of(&QTimer::start));
	connect(mEditorWidget, &QDockWidget::visibilityChanged, mEditorSnapTimer, QOverload<>::of(&QTimer::start));
	connect(mEditorWidget, &QDockWidget::topLevelChanged, mEditorSnapTimer, QOverload<>::of(&QTimer::start));
	connect(mEditorWidget, &QDockWidget::dockLocationChanged, mEditorSnapTimer, QOverload<>::of(&QTimer::start));

	connect(mInstructions, &swInstructions::PageSetupChanged, this, &swInstructionsDialog::PageSetupChanged);
	connect(mInstructions, &swInstructions::PageLayoutChanged, this, &swInstructionsDialog::PageSetupChanged);

	// restore the panel layout
	QSettings Settings;
	Settings.beginGroup("InstructionsWindow");
	restoreState(Settings.value("State").toByteArray());
	Settings.endGroup();

	// panels follow the main window lock
	for (QDockWidget* DockWidget : findChildren<QDockWidget*>(QString(), Qt::FindDirectChildrenOnly))
		swSetDockWidgetLocked(DockWidget, swIsInterfaceLocked());
}

// use to save the panel layout
void swInstructionsDialog::SaveLayout()
{
	QSettings Settings;
	Settings.beginGroup("InstructionsWindow");
	Settings.setValue("State", saveState());
	Settings.endGroup();
}

// use to follow the preferences options
void swInstructionsDialog::UpdateOptions()
{
	mInstructions->SetShowMargins(swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_SHOW_MARGINS) != 0);
	mInstructions->SetShowElementMargins(swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_SHOW_ELEMENT_MARGINS) != 0);
	mInstructions->SetShowElementPadding(swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_SHOW_ELEMENT_PADDING) != 0);
	mInstructions->SetShowElementAnchors(swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_SHOW_ELEMENT_ANCHORS) != 0);
	mInstructions->SetShowRulers(swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_SHOW_RULERS) != 0);
	mInstructions->UpdateColors();
	mEditorWidget->UpdateOptions();
}

// use to mark unsaved changes, kde style
void swInstructionsDialog::changeEvent(QEvent* Event)
{
	if (Event->type() == QEvent::ModifiedChange)
		setWindowTitle(isWindowModified() ? tr("Instructions") + QLatin1String(" *") : tr("Instructions"));

	QMainWindow::changeEvent(Event);
}

void swInstructionsDialog::closeEvent(QCloseEvent* Event)
{
	SaveLayout();

	QMainWindow::closeEvent(Event);
}

void swInstructionsDialog::UpdatePageList()
{
	QListWidget* ThumbnailsWidget = mPageListWidget->mThumbnailsWidget;
	const int CurrentRow = qMax(ThumbnailsWidget->currentRow(), 0);

	// rebuild the page list
	QSignalBlocker Blocker(ThumbnailsWidget);
	ThumbnailsWidget->clear();

	for (size_t PageNumber = 0; PageNumber < mInstructions->mPages.size(); PageNumber++)
	{
		QString Label = tr("Page %1").arg(swFormatIntegerLocalized(PageNumber + 1, locale()));

		switch (mInstructions->mPages[PageNumber].Type)
		{
		case swInstructionsPageType::Cover:
			Label = tr("%1 - Front Cover").arg(Label);
			break;

		case swInstructionsPageType::BillOfMaterials:
			Label = tr("%1 - Bill of Materials").arg(Label);
			break;

		case swInstructionsPageType::Blank:
			Label = tr("%1 - Blank").arg(Label);
			break;

		case swInstructionsPageType::BackCover:
			Label = tr("%1 - Back Cover").arg(Label);
			break;

		case swInstructionsPageType::Steps:
		{
			const swModel* TitleModel = nullptr;
			const bool Assembly = mInstructions->GetPageTitleModel(PageNumber, TitleModel);

			if (!TitleModel)
				break;

			auto GetTitle = [](const swModel* Model)
			{
				const swModelProperties& Properties = Model->GetProperties();
				QString Title = Properties.mDescription.trimmed();

				if (Title.isEmpty())
					Title = Properties.mModelName.trimmed();

				if (Title.isEmpty())
					Title = QFileInfo(Properties.mFileName).completeBaseName();

				return Title;
			};

			if (mInstructions->IsCompletionPage(PageNumber))
				Label = tr("%1 - Completion - %2").arg(Label, GetTitle(mInstructions->GetMainModel()));
			else
				Label = Assembly ? tr("%1 - Assembly - %2").arg(Label, GetTitle(TitleModel)) : tr("%1 - %2").arg(Label, GetTitle(TitleModel));

			break;
		}
		}

		ThumbnailsWidget->addItem(Label);
	}

	const int Row = qMin(CurrentRow, ThumbnailsWidget->count() - 1);
	ThumbnailsWidget->setCurrentRow(Row);
	CurrentThumbnailChanged(Row);

	QTimer::singleShot(0, this, &swInstructionsDialog::SnapPageListDock);
}

bool swInstructionsDialog::eventFilter(QObject* Object, QEvent* Event)
{
	// snap the pages panel back on release
	if (Object == mPageListWidget && Event->type() == QEvent::Resize && QApplication::mouseButtons() != Qt::NoButton)
		mPageListSnapPending = true;
	else if (Event->type() == QEvent::MouseButtonRelease && mPageListSnapPending)
	{
		mPageListSnapPending = false;
		QTimer::singleShot(0, this, &swInstructionsDialog::SnapPageListDock);
	}

	// snap the editor panel after a drag or a window resize
	if (Object == mEditorWidget && Event->type() == QEvent::Resize)
	{
		if (QApplication::mouseButtons() != Qt::NoButton && !mEditorWidget->isFloating())
			mEditorSnapPending = true;
		else
			mEditorSnapTimer->start();
	}
	else if (Event->type() == QEvent::MouseButtonRelease && mEditorSnapPending)
	{
		mEditorSnapPending = false;
		mEditorSnapTimer->start();
	}

	return QMainWindow::eventFilter(Object, Event);
}

void swInstructionsDialog::SnapPageListDock()
{
	if (mPageListWidget->isFloating())
		return;

	const Qt::DockWidgetArea Area = dockWidgetArea(mPageListWidget);

	if (Area != Qt::LeftDockWidgetArea && Area != Qt::RightDockWidgetArea)
		return;

	// other panels keep their width
	const QList<QDockWidget*> DockWidgets = findChildren<QDockWidget*>(QString(), Qt::FindDirectChildrenOnly);

	for (QDockWidget* DockWidget : DockWidgets)
		if (DockWidget != mPageListWidget && !DockWidget->isHidden() && !DockWidget->isFloating() && dockWidgetArea(DockWidget) == Area)
			return;

	// measure every page name
	QListWidget* ThumbnailsWidget = mPageListWidget->mThumbnailsWidget;
	const int TextWidth = ThumbnailsWidget->sizeHintForColumn(0);

	if (TextWidth <= 0)
		return;

	const int Delta = TextWidth - ThumbnailsWidget->viewport()->width();

	if (!Delta)
		return;

	resizeDocks({ mPageListWidget }, { mPageListWidget->width() + Delta }, Qt::Horizontal);
}

void swInstructionsDialog::SnapEditorDock()
{
	mEditorSnapPending = false;

	if (mEditorWidget->isHidden())
		return;

	const int Delta = mEditorWidget->GetSnapDelta();

	if (!Delta)
		return;

	if (mEditorWidget->isFloating())
	{
		mEditorWidget->resize(mEditorWidget->width(), mEditorWidget->height() + Delta);
		return;
	}

	const Qt::DockWidgetArea Area = dockWidgetArea(mEditorWidget);

	if (Area != Qt::TopDockWidgetArea && Area != Qt::BottomDockWidgetArea)
		return;

	resizeDocks({ mEditorWidget }, { mEditorWidget->height() + Delta }, Qt::Vertical);
}

void swInstructionsDialog::ModelChanged()
{
	// rebuild the pages once per event
	if (mUpdatePending)
		return;

	mUpdatePending = true;

	QTimer::singleShot(0, this, [this]()
	{
		mUpdatePending = false;
		mInstructions->Update();
	});

	mEditorWidget->ScheduleUpdate();
}

void swInstructionsDialog::SetProject(Project* Project)
{
	mProject = Project;

	// hand the instructions to the project
	mPageWidget->scene()->clearSelection();
	mEditorWidget->SetProject(Project);
}

void swInstructionsDialog::PageSetupChanged()
{
	UpdatePageList();
}

void swInstructionsDialog::CurrentThumbnailChanged(int Index)
{
	UpdatePageBar();

	if (Index < 0 || Index >= static_cast<int>(mInstructions->mPages.size()))
	{
		mPageWidget->SetCurrentPage(nullptr);
		mPlacementWidget->SetCurrentPage(-1);
		return;
	}

	const swInstructionsPage* Page = &mInstructions->mPages[Index];

	mPageWidget->SetCurrentPage(Page);
	mPropertiesWidget->SetCurrentPage(Index);
	mPlacementWidget->SetCurrentPage(Index);
}

// use to build the page buttons and the page box
// use to step pages down to go forward, like the wheel on the page
class swPageSpinBox : public swRangeSpinBox
{
public:
	using swRangeSpinBox::swRangeSpinBox;

	void stepBy(int Steps) override
	{
		swRangeSpinBox::stepBy(-Steps);
	}

protected:
	StepEnabled stepEnabled() const override
	{
		const StepEnabled Enabled = swRangeSpinBox::stepEnabled();
		StepEnabled Swapped = StepNone;

		if (Enabled & StepUpEnabled)
			Swapped |= StepDownEnabled;

		if (Enabled & StepDownEnabled)
			Swapped |= StepUpEnabled;

		return Swapped;
	}
};

QWidget* swInstructionsDialog::CreatePageBar()
{
	QWidget* PageBar = new QWidget(this);
	QHBoxLayout* PageBarLayout = new QHBoxLayout(PageBar);
	PageBarLayout->addStretch(1);

	// arrows point away from the page names
	auto CreateButton = [this, PageBar, PageBarLayout](const QString& Text, const QString& ToolTip, const QIcon& Icon, bool IconAfterText, auto Clicked)
	{
		QToolButton* Button = new QToolButton(PageBar);
		Button->setText(Text);
		Button->setToolTip(ToolTip);
		Button->setIcon(Icon);
		Button->setToolButtonStyle(Icon.isNull() ? Qt::ToolButtonTextOnly : Qt::ToolButtonTextBesideIcon);
		Button->setLayoutDirection(IconAfterText ? Qt::RightToLeft : Qt::LeftToRight);
		Button->setWhatsThis(tr("<p><b>Page Bar</b></p><p>Go to the first, previous, next or last page, or straight to the cover or the bill of materials, greyed out while those pages are turned off. Step Page inserts an empty step before the current page, Blank Page inserts a blank page before it and Remove Page removes a blank page or a page of empty steps. The box on the right shows the current page, type a number to go there. The First, Previous, Next and Last step shortcuts turn the pages too while the page or the Pages panel has focus.</p>"));
		connect(Button, &QToolButton::clicked, this, Clicked);
		PageBarLayout->addWidget(Button);
		return Button;
	};

	QListWidget* ThumbnailsWidget = mPageListWidget->mThumbnailsWidget;

	// undo and redo take back instruction changes

	const QString HistoryWhatsThis = tr("<p><b>Undo and Redo</b></p><p>Undo takes back your last change to the instructions, Redo puts it back. Quick changes to the same setting count as one change. The main window Undo and Redo shortcuts work here too.</p>");

	mUndoButton = CreateButton(tr("&Undo"), tr("Take back the last change to the instructions"), gMainWindow->mActions[SW_EDIT_UNDO]->icon(), false, [this]() { mInstructions->Undo(); });
	mUndoButton->setWhatsThis(HistoryWhatsThis);
	mRedoButton = CreateButton(tr("Re&do"), tr("Put back the last change that was taken back"), gMainWindow->mActions[SW_EDIT_REDO]->icon(), false, [this]() { mInstructions->Redo(); });
	mRedoButton->setWhatsThis(HistoryWhatsThis);

	auto UpdateHistoryButtons = [this]()
	{
		mUndoButton->setEnabled(mInstructions->CanUndo());
		mRedoButton->setEnabled(mInstructions->CanRedo());
	};

	connect(mInstructions, &swInstructions::HistoryChanged, this, UpdateHistoryButtons);
	UpdateHistoryButtons();

	mFirstButton = CreateButton(tr("&First"), tr("Go to the first page"), swCreatePaintedIcon(swPaintedIcon::TimeFirst), false, [this]() { ShowPage(0); });
	mPreviousButton = CreateButton(tr("&Previous"), tr("Go to the previous page"), swCreatePaintedIcon(swPaintedIcon::TimePrevious), false, [this, ThumbnailsWidget]() { ShowPage(ThumbnailsWidget->currentRow() / mPageStep * mPageStep - mPageStep); });
	mCoverButton = CreateButton(tr("Fr&ont Cover"), tr("Go to the front cover"), QIcon(), false, [this]() { ShowPage(FindPage(swInstructionsPageType::Cover)); });
	mStepPageButton = CreateButton(tr("&Step Page"), tr("Insert an empty step before the current page"), QIcon(), false, [this]() { InsertStepPage(); });
	mBlankPageButton = CreateButton(tr("Bl&ank Page"), tr("Insert a blank page before the current page"), QIcon(), false, [this, ThumbnailsWidget]() { mInstructions->InsertBlankPage(ThumbnailsWidget->currentRow()); });
	mRemovePageButton = CreateButton(tr("&Remove Page"), tr("Remove the current page when it is blank or holds only empty steps"), QIcon(), false, [this]() { RemovePage(); });
	mBillOfMaterialsButton = CreateButton(tr("&Bill of Materials"), tr("Go to the bill of materials page"), QIcon(), false, [this]() { ShowPage(FindPage(swInstructionsPageType::BillOfMaterials)); });
	mBackCoverButton = CreateButton(tr("Bac&k Cover"), tr("Go to the back cover"), QIcon(), false, [this]() { ShowPage(FindPage(swInstructionsPageType::BackCover)); });
	mNextButton = CreateButton(tr("&Next"), tr("Go to the next page"), swCreatePaintedIcon(swPaintedIcon::TimeNext), true, [this, ThumbnailsWidget]() { ShowPage(qMin(ThumbnailsWidget->currentRow() / mPageStep * mPageStep + mPageStep, ThumbnailsWidget->count() - 1)); });
	mLastButton = CreateButton(tr("&Last"), tr("Go to the last page"), swCreatePaintedIcon(swPaintedIcon::TimeLast), true, [this, ThumbnailsWidget]() { ShowPage(ThumbnailsWidget->count() - 1); });

	PageBarLayout->addStretch(1);

	// two pages side by side or stacked, one at a time

	const QString SpreadWhatsThis = tr("<p><b>Two Pages</b></p><p>Show two pages at once, pages 1 and 2, 3 and 4 and so on, side by side or one above the other. Only one of the two can be on, click it again for a single page. You edit the current page, click the other page to edit it.</p>");
	QToolButton* SideBySideButton = new QToolButton(PageBar);
	QToolButton* StackedButton = new QToolButton(PageBar);

	SideBySideButton->setIcon(swCreatePaintedIcon(swPaintedIcon::PageSideBySide));
	SideBySideButton->setToolTip(tr("Show two pages side by side"));
	SideBySideButton->setAccessibleName(tr("Side by Side"));
	StackedButton->setIcon(swCreatePaintedIcon(swPaintedIcon::PageStacked));
	StackedButton->setToolTip(tr("Show two pages one above the other"));
	StackedButton->setAccessibleName(tr("Stacked"));

	const swInstructionsPageSpread StoredSpread = static_cast<swInstructionsPageSpread>(std::clamp(swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_PAGE_SPREAD), 0, 2));

	for (QToolButton* Button : { SideBySideButton, StackedButton })
	{
		Button->setCheckable(true);
		Button->setWhatsThis(SpreadWhatsThis);
		Button->setChecked(StoredSpread == (Button == SideBySideButton ? swInstructionsPageSpread::SideBySide : swInstructionsPageSpread::Stacked));
		PageBarLayout->addWidget(Button);

		connect(Button, &QToolButton::toggled, this, [this, Button, SideBySideButton, StackedButton](bool Checked)
		{
			QToolButton* OtherButton = Button == SideBySideButton ? StackedButton : SideBySideButton;

			if (Checked)
			{
				QSignalBlocker Blocker(OtherButton);
				OtherButton->setChecked(false);
			}

			const swInstructionsPageSpread PageSpread = SideBySideButton->isChecked() ? swInstructionsPageSpread::SideBySide : StackedButton->isChecked() ? swInstructionsPageSpread::Stacked : swInstructionsPageSpread::Single;

			swSetProfileInteger(SW_PROFILE_INSTRUCTIONS_PAGE_SPREAD, static_cast<int>(PageSpread));
			mPageWidget->SetPageSpread(PageSpread);
			mPageStep = PageSpread == swInstructionsPageSpread::Single ? 1 : 2;
			UpdatePageBar();
		});
	}

	mPageWidget->SetPageSpread(StoredSpread);
	mPageStep = StoredSpread == swInstructionsPageSpread::Single ? 1 : 2;
	connect(mPageWidget, &swInstructionsPageWidget::PageClicked, this, &swInstructionsDialog::ShowPage);

	// the wheel turns pages like the buttons

	connect(mPageWidget, &swInstructionsPageWidget::PageScrolled, this, [this](int Direction)
	{
		(Direction > 0 ? mNextButton : mPreviousButton)->click();
	});

	// current page out of all pages, like the print range
	mPageSpinBox = new swPageSpinBox(PageBar);
	mPageSpinBox->setToolTip(tr("Current page out of all pages, type a number to go there"));
	mPageSpinBox->setWhatsThis(mFirstButton->whatsThis());
	mPageSpinBox->setKeyboardTracking(false);
	connect(mPageSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int Page) { ShowPage(Page - 1); });
	PageBarLayout->addWidget(mPageSpinBox);

	// square buttons as tall as the page box

	const int SpreadSize = qMax(mPageSpinBox->sizeHint().height(), mFirstButton->sizeHint().height());

	for (QToolButton* Button : { SideBySideButton, StackedButton })
		Button->setFixedSize(SpreadSize, SpreadSize);

	return PageBar;
}

// use to get the model whose steps fill a page, only the active model since step edits follow the Steps panel
swModel* swInstructionsDialog::GetEditableStepModel(int Row) const
{
	if (Row < 0 || Row >= static_cast<int>(mInstructions->mPages.size()))
		return nullptr;

	const swInstructionsPage& Page = mInstructions->mPages[Row];

	if (Page.Type != swInstructionsPageType::Steps || Page.Steps.empty() || Page.Steps.front().Model != mProject->GetActiveModel())
		return nullptr;

	return Page.Steps.front().Model;
}

// use to tell a step page holding no parts
bool swInstructionsDialog::IsEmptyStepPage(int Row) const
{
	swModel* Model = GetEditableStepModel(Row);

	if (!Model)
		return false;

	for (const swInstructionsStep& Step : mInstructions->mPages[Row].Steps)
		for (const std::unique_ptr<swPart>& Part : Model->GetParts())
			if (Part->GetStepShow() == Step.Step)
				return false;

	return true;
}

// use to insert an empty step before the first step of the page
void swInstructionsDialog::InsertStepPage()
{
	const int Row = mPageListWidget->mThumbnailsWidget->currentRow();
	swModel* Model = GetEditableStepModel(Row);

	if (Model)
		Model->InsertStepAction(mInstructions->mPages[Row].Steps.front().Step);
}

// use to remove a blank page or the empty steps of a page
void swInstructionsDialog::RemovePage()
{
	const int Row = mPageListWidget->mThumbnailsWidget->currentRow();

	// locked pages refuse removal
	if (Row < 0 || Row >= static_cast<int>(mInstructions->mPages.size()) || mInstructions->IsPageLocked(Row))
		return;

	if (mInstructions->mPages[Row].Type == swInstructionsPageType::Blank)
	{
		mInstructions->RemoveBlankPage(Row);
		return;
	}

	// the final model page hides
	const std::vector<swInstructionsStep>& PageSteps = mInstructions->mPages[Row].Steps;

	if (!PageSteps.empty() && std::all_of(PageSteps.begin(), PageSteps.end(), [](const swInstructionsStep& Step) { return Step.FinalModel; }) && PageSteps.front().Model == mInstructions->GetMainModel())
	{
		mInstructions->SetDefaultBool(swInstructionsPropertyType::ShowFinalModelPage, false);
		return;
	}

	if (!IsEmptyStepPage(Row))
		return;

	swModel* Model = GetEditableStepModel(Row);
	std::vector<swStep> Steps;

	for (const swInstructionsStep& Step : mInstructions->mPages[Row].Steps)
		if (!Step.FinalModel)
			Steps.push_back(Step.Step);

	// last step first so the earlier steps keep their numbers
	for (auto StepIt = Steps.rbegin(); StepIt != Steps.rend(); ++StepIt)
		Model->RemoveStepAction(*StepIt);
}


// use to select a page through the pages panel
void swInstructionsDialog::ShowPage(int Row)
{
	QListWidget* ThumbnailsWidget = mPageListWidget->mThumbnailsWidget;

	if (Row >= 0 && Row < ThumbnailsWidget->count())
		ThumbnailsWidget->setCurrentRow(Row);
}

// use to find the first page of a type, -1 when there is none
int swInstructionsDialog::FindPage(swInstructionsPageType Type) const
{
	for (size_t PageNumber = 0; PageNumber < mInstructions->mPages.size(); PageNumber++)
		if (mInstructions->mPages[PageNumber].Type == Type)
			return static_cast<int>(PageNumber);

	return -1;
}

// use to grey out the buttons with nowhere to go
void swInstructionsDialog::UpdatePageBar()
{
	if (!mFirstButton)
		return;

	const int Row = mPageListWidget->mThumbnailsWidget->currentRow();
	const int LastRow = mPageListWidget->mThumbnailsWidget->count() - 1;

	mFirstButton->setEnabled(Row > 0);
	mPreviousButton->setEnabled(Row > 0);
	mNextButton->setEnabled(Row >= 0 && Row < LastRow);
	mLastButton->setEnabled(Row >= 0 && Row < LastRow);
	mCoverButton->setEnabled(FindPage(swInstructionsPageType::Cover) >= 0);
	mStepPageButton->setEnabled(GetEditableStepModel(Row) != nullptr);
	mBlankPageButton->setEnabled(Row >= 0 && mInstructions->CanInsertBlankPage(Row));
	mRemovePageButton->setEnabled(Row >= 0 && Row <= LastRow && !mInstructions->IsPageLocked(Row) && (mInstructions->mPages[Row].Type == swInstructionsPageType::Blank || IsEmptyStepPage(Row)));

	QSignalBlocker Blocker(mPageSpinBox);
	mPageSpinBox->setRange(1, qMax(LastRow + 1, 1));
	mPageSpinBox->SetReference();

	// two pages step by pairs

	mPageSpinBox->setSingleStep(mPageStep);
	mPageSpinBox->setValue(Row / mPageStep * mPageStep + 1);
	mBillOfMaterialsButton->setEnabled(FindPage(swInstructionsPageType::BillOfMaterials) >= 0);
	mBackCoverButton->setEnabled(FindPage(swInstructionsPageType::BackCover) >= 0);
}
