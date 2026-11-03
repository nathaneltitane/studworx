#include "sw_global.h"
#include "sw_widget_editor.h"
#include "sw_widget_text_edit.h"
#include "sw_profile.h"
#include "sw_window_utilities.h"

swEditorWidget::swEditorWidget(QWidget* Parent, QSyntaxHighlighter* Highlighter)
	: QWidget(Parent), mHighlighter(Highlighter)
{
	QVBoxLayout* Layout = new QVBoxLayout(this);
	Layout->setContentsMargins(0, 0, 0, 0);

	auto CreateButton = [](QWidget* Row, const QString& Name, const QString& ToolTip)
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
	};

	// options button at the top right
	QWidget* OptionsRow = swCreatePanelRow(this);
	swAddPanelRowStretch(OptionsRow);
	Layout->addWidget(OptionsRow);

	QMenu* OptionsMenu = new QMenu(OptionsRow);
	swAddPanelOptionsButton(OptionsRow, OptionsMenu, tr("Configure the editor."), tr("Editor Options"));

	mHighlightAction = OptionsMenu->addAction(tr("&Syntax Highlighting"));
	mHighlightAction->setCheckable(true);
	mHighlightAction->setChecked(swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_EDITOR_HIGHLIGHT) != 0);
	connect(mHighlightAction, &QAction::toggled, this, &swEditorWidget::SetHighlightEnabled);

	mNumbersAction = OptionsMenu->addAction(tr("Line &Numbers"));
	mNumbersAction->setCheckable(true);
	mNumbersAction->setChecked(swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_EDITOR_NUMBERS) != 0);
	connect(mNumbersAction, &QAction::toggled, this, &swEditorWidget::SetNumbersEnabled);

	mWrapAction = OptionsMenu->addAction(tr("&Wrap Lines"));
	mWrapAction->setCheckable(true);
	mWrapAction->setChecked(swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_EDITOR_WRAP) != 0);
	connect(mWrapAction, &QAction::toggled, this, &swEditorWidget::SetWrapEnabled);

	mMarkersAction = OptionsMenu->addAction(tr("Whitespace &Markers"));
	mMarkersAction->setCheckable(true);
	mMarkersAction->setChecked(swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_EDITOR_MARKERS) != 0);
	connect(mMarkersAction, &QAction::toggled, this, &swEditorWidget::SetMarkersEnabled);

	OptionsMenu->addSeparator();

	mFindAction = OptionsMenu->addAction(tr("&Find and Replace"));
	mFindAction->setCheckable(true);
	mFindAction->setShortcut(QKeySequence::Find);
	mFindAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	addAction(mFindAction);
	connect(mFindAction, &QAction::toggled, this, &swEditorWidget::SetFindVisible);

	QAction* MatchCaseAction = OptionsMenu->addAction(tr("Match &Case"));
	MatchCaseAction->setCheckable(true);
	connect(MatchCaseAction, &QAction::toggled, this, [this](bool Checked)
	{
		mMatchCase = Checked;
		UpdateMatches();
	});

	QAction* WholeWordsAction = OptionsMenu->addAction(tr("Wh&ole Words"));
	WholeWordsAction->setCheckable(true);
	connect(WholeWordsAction, &QAction::toggled, this, [this](bool Checked)
	{
		mWholeWords = Checked;
		UpdateMatches();
	});

	// find and replace rows below the text
	mFindWidget = new QWidget(this);
	QVBoxLayout* FindLayout = new QVBoxLayout(mFindWidget);
	FindLayout->setContentsMargins(0, 0, 0, 0);
	mFindWidget->hide();

	QWidget* FindRow = swCreatePanelRow(mFindWidget);
	FindLayout->addWidget(FindRow);

	mFindEdit = new QLineEdit(FindRow);
	mFindEdit->setPlaceholderText(tr("Find"));
	mFindEdit->setAccessibleName(tr("Find"));
	mFindEdit->setToolTip(tr("Text to find, enter finds the next match and shift enter the previous one."));
	mFindEdit->setClearButtonEnabled(true);
	swSetupPanelEdit(mFindEdit);
	static_cast<QHBoxLayout*>(FindRow->layout())->addWidget(mFindEdit, 2);

	QToolButton* PreviousButton = CreateButton(FindRow, tr("Previous"), tr("Select the previous match."));
	QToolButton* NextButton = CreateButton(FindRow, tr("Next"), tr("Select the next match."));

	QWidget* ReplaceRow = swCreatePanelRow(mFindWidget);
	FindLayout->addWidget(ReplaceRow);

	mReplaceEdit = new QLineEdit(ReplaceRow);
	mReplaceEdit->setPlaceholderText(tr("Replace"));
	mReplaceEdit->setAccessibleName(tr("Replace"));
	mReplaceEdit->setToolTip(tr("Text that replaces the matches, enter replaces the selected match."));
	swSetupPanelEdit(mReplaceEdit);
	static_cast<QHBoxLayout*>(ReplaceRow->layout())->addWidget(mReplaceEdit, 2);

	QToolButton* ReplaceButton = CreateButton(ReplaceRow, tr("Replace"), tr("Replace the selected match and select the next one."));
	QToolButton* ReplaceAllButton = CreateButton(ReplaceRow, tr("Replace All"), tr("Replace every match in the text."));

	mMatchLabel = new QLabel(FindRow);
	mMatchLabel->setMinimumWidth(mMatchLabel->fontMetrics().horizontalAdvance(tr("No Matches")));
	mMatchLabel->setAlignment(Qt::AlignCenter);
	static_cast<QHBoxLayout*>(FindRow->layout())->insertWidget(1, mMatchLabel);

	QLabel* ReplaceSpacer = new QLabel(ReplaceRow);
	ReplaceSpacer->setMinimumWidth(mMatchLabel->minimumWidth());
	static_cast<QHBoxLayout*>(ReplaceRow->layout())->insertWidget(1, ReplaceSpacer);

	connect(mFindEdit, &QLineEdit::textChanged, this, [this]()
	{
		// find from the selection start
		QTextCursor Cursor = mTextEdit->textCursor();
		Cursor.setPosition(Cursor.selectionStart());
		mTextEdit->setTextCursor(Cursor);
		FindMatch(false);
	});
	mFindEdit->installEventFilter(this);
	mReplaceEdit->installEventFilter(this);
	connect(PreviousButton, &QToolButton::clicked, this, [this]() { FindMatch(true); });
	connect(NextButton, &QToolButton::clicked, this, [this]() { FindMatch(false); });
	connect(ReplaceButton, &QToolButton::clicked, this, &swEditorWidget::ReplaceClicked);
	connect(ReplaceAllButton, &QToolButton::clicked, this, &swEditorWidget::ReplaceAllClicked);

	QShortcut* CloseFindShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), mFindWidget, nullptr, nullptr, Qt::WidgetWithChildrenShortcut);
	connect(CloseFindShortcut, &QShortcut::activated, mFindAction, [this]() { mFindAction->setChecked(false); });

	// fixed font keeps the columns
	mTextEdit = new swEditorTextEdit(this);
	mTextEdit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
	mTextEdit->setLineWrapMode(mWrapAction->isChecked() ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);

	// find and replace below the text
	Layout->addWidget(mTextEdit, 1);
	Layout->addWidget(mFindWidget);

	SetMarkersEnabled(mMarkersAction->isChecked());
	mTextEdit->SetLineNumbersVisible(mNumbersAction->isChecked());

	if (mHighlightAction->isChecked())
		mHighlighter->setDocument(mTextEdit->document());

	connect(mTextEdit, &QPlainTextEdit::textChanged, this, &swEditorWidget::UpdateMatches);
	connect(mTextEdit, &QPlainTextEdit::cursorPositionChanged, this, &swEditorWidget::UpdateMatches);
}

// use to find and replace on enter without pressing the default button of a dialog
bool swEditorWidget::eventFilter(QObject* Object, QEvent* Event)
{
	if (Event->type() != QEvent::KeyPress || (Object != mFindEdit && Object != mReplaceEdit))
		return QWidget::eventFilter(Object, Event);

	const QKeyEvent* KeyEvent = static_cast<QKeyEvent*>(Event);

	if (KeyEvent->key() != Qt::Key_Return && KeyEvent->key() != Qt::Key_Enter)
		return QWidget::eventFilter(Object, Event);

	if (Object == mFindEdit)
		FindMatch(KeyEvent->modifiers() & Qt::ShiftModifier);
	else
		ReplaceClicked();

	return true;
}

// use to follow the preferences editor options
void swEditorWidget::UpdateOptions()
{
	mHighlightAction->setChecked(swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_EDITOR_HIGHLIGHT) != 0);
	mNumbersAction->setChecked(swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_EDITOR_NUMBERS) != 0);
	mWrapAction->setChecked(swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_EDITOR_WRAP) != 0);
	mMarkersAction->setChecked(swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_EDITOR_MARKERS) != 0);
}

void swEditorWidget::SetHighlightEnabled(bool Enabled)
{
	swSetProfileInteger(SW_PROFILE_INSTRUCTIONS_EDITOR_HIGHLIGHT, Enabled);
	mHighlighter->setDocument(Enabled ? mTextEdit->document() : nullptr);
}

void swEditorWidget::SetNumbersEnabled(bool Enabled)
{
	swSetProfileInteger(SW_PROFILE_INSTRUCTIONS_EDITOR_NUMBERS, Enabled);
	mTextEdit->SetLineNumbersVisible(Enabled);
}

void swEditorWidget::SetWrapEnabled(bool Enabled)
{
	swSetProfileInteger(SW_PROFILE_INSTRUCTIONS_EDITOR_WRAP, Enabled);
	mTextEdit->setLineWrapMode(Enabled ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
	emit SnapRequested();
}

void swEditorWidget::SetMarkersEnabled(bool Enabled)
{
	swSetProfileInteger(SW_PROFILE_INSTRUCTIONS_EDITOR_MARKERS, Enabled);

	// show whitespace and line end markers
	QTextDocument* Document = mTextEdit->document();
	QTextOption Option = Document->defaultTextOption();
	Option.setFlags(Enabled ? Option.flags() | QTextOption::ShowTabsAndSpaces : Option.flags() & ~QTextOption::ShowTabsAndSpaces);
	Document->setDefaultTextOption(Option);
	mTextEdit->SetLineMarkersVisible(Enabled);
}

void swEditorWidget::SetFindVisible(bool Visible)
{
	QSignalBlocker Blocker(mFindAction);
	mFindAction->setChecked(Visible);
	mFindWidget->setVisible(Visible);
	emit SnapRequested();

	if (!Visible)
	{
		mTextEdit->setExtraSelections({});
		mTextEdit->setFocus();
		return;
	}

	// find a one line selection
	const QString Selection = mTextEdit->textCursor().selectedText();

	if (!Selection.isEmpty() && !Selection.contains(QChar::ParagraphSeparator))
		mFindEdit->setText(Selection);

	mFindEdit->setFocus();
	mFindEdit->selectAll();
	UpdateMatches();
}

QTextDocument::FindFlags swEditorWidget::GetFindFlags() const
{
	QTextDocument::FindFlags Flags;

	if (mMatchCase)
		Flags |= QTextDocument::FindCaseSensitively;

	if (mWholeWords)
		Flags |= QTextDocument::FindWholeWords;

	return Flags;
}

void swEditorWidget::FindMatch(bool Backward)
{
	const QString Text = mFindEdit->text();

	if (Text.isEmpty())
	{
		UpdateMatches();
		return;
	}

	QTextDocument* Document = mTextEdit->document();
	const QTextDocument::FindFlags Flags = GetFindFlags() | (Backward ? QTextDocument::FindBackward : QTextDocument::FindFlags());
	QTextCursor Cursor = Document->find(Text, mTextEdit->textCursor(), Flags);

	// wrap the search around
	if (Cursor.isNull())
		Cursor = Document->find(Text, Backward ? Document->characterCount() - 1 : 0, Flags);

	if (!Cursor.isNull())
		mTextEdit->setTextCursor(Cursor);

	UpdateMatches();
}

void swEditorWidget::ReplaceClicked()
{
	const QString Text = mFindEdit->text();
	QTextCursor Cursor = mTextEdit->textCursor();

	if (Text.isEmpty())
		return;

	// replace a matching selection only
	const QTextCursor Match = mTextEdit->document()->find(Text, Cursor.selectionStart(), GetFindFlags());

	if (Cursor.hasSelection() && Match.selectionStart() == Cursor.selectionStart() && Match.selectionEnd() == Cursor.selectionEnd())
	{
		Cursor.insertText(mReplaceEdit->text());
		mTextEdit->setTextCursor(Cursor);
	}

	FindMatch(false);
}

void swEditorWidget::ReplaceAllClicked()
{
	const QString Text = mFindEdit->text();

	if (Text.isEmpty())
		return;

	// undo each replacement
	QTextDocument* Document = mTextEdit->document();
	QTextCursor Cursor(Document);
	int Count = 0;

	Cursor.beginEditBlock();

	for (QTextCursor Match = Document->find(Text, 0, GetFindFlags()); !Match.isNull(); Match = Document->find(Text, Cursor.position(), GetFindFlags()))
	{
		Cursor.setPosition(Match.selectionStart());
		Cursor.setPosition(Match.selectionEnd(), QTextCursor::KeepAnchor);
		Cursor.insertText(mReplaceEdit->text());
		Count++;
	}

	Cursor.endEditBlock();

	mMatchLabel->setText(tr("%n Replaced", nullptr, Count));
}

void swEditorWidget::UpdateMatches()
{
	if (!mFindWidget->isVisible())
		return;

	const QString Text = mFindEdit->text();
	QList<QTextEdit::ExtraSelection> Selections;

	if (Text.isEmpty())
	{
		mMatchLabel->clear();
		mTextEdit->setExtraSelections(Selections);
		return;
	}

	// mark and count every match
	QTextDocument* Document = mTextEdit->document();
	const QTextCursor Current = mTextEdit->textCursor();
	QColor Color = palette().color(QPalette::Highlight);
	Color.setAlpha(80);
	int CurrentIndex = 0;

	for (QTextCursor Match = Document->find(Text, 0, GetFindFlags()); !Match.isNull(); Match = Document->find(Text, Match, GetFindFlags()))
	{
		QTextEdit::ExtraSelection Selection;
		Selection.cursor = Match;
		Selection.format.setBackground(Color);
		Selections.append(Selection);

		if (Match.selectionStart() == Current.selectionStart() && Match.selectionEnd() == Current.selectionEnd())
			CurrentIndex = Selections.size();
	}

	mTextEdit->setExtraSelections(Selections);

	if (Selections.isEmpty())
		mMatchLabel->setText(tr("No Matches"));
	else if (CurrentIndex)
		mMatchLabel->setText(tr("%1 of %2").arg(CurrentIndex).arg(Selections.size()));
	else
		mMatchLabel->setText(tr("%n Matches", nullptr, Selections.size()));
}
