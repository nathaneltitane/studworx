#include "sw_global.h"
#include "sw_dialog_instructions_editor.h"
#include "sw_instructions.h"
#include "sw_application.h"
#include "sw_colors.h"
#include "sw_profile.h"
#include "sw_project.h"
#include "sw_window_main.h"
#include "sw_window_utilities.h"
#include "sw_widget_text_edit.h"
#include "sw_widget_editor.h"

// use to check ldraw lines and metas
static const QStringList& swGetLDrawMetaKeywords()
{
	static const QStringList Keywords = { "STEP", "FILE", "NOFILE", "BFC", "CLEAR", "PAUSE", "SAVE", "PRINT", "WRITE", "ROTSTEP", "Name:", "Author:" };
	return Keywords;
}

// use to capitalize the meta command
static QString swGetLDrawMetaLine(const QString& Line)
{
	const QRegularExpression Expression(QLatin1String("^(\\s*0\\s+)(\\S+)"));
	const QRegularExpressionMatch Match = Expression.match(Line);

	if (!Match.hasMatch())
		return Line;

	const QString Command = Match.captured(2);

	if (Command.endsWith(':') || Command.startsWith(QLatin1String("//")))
		return Line;

	if (!Command.startsWith('!') && !swGetLDrawMetaKeywords().contains(Command, Qt::CaseInsensitive))
		return Line;

	QString Result = Line;
	Result.replace(Match.capturedStart(2), Command.size(), Command.toUpper());
	return Result;
}

static QString swGetLDrawLineError(const QString& Line)
{
	const QStringList Tokens = Line.simplified().split(' ', Qt::SkipEmptyParts);

	if (Tokens.isEmpty())
		return QString();

	// parameter count of each line type
	static const int TypeTokenCounts[] = { 0, 15, 8, 11, 14, 14 };
	bool Valid = false;
	const int TypeNumber = Tokens[0].toInt(&Valid);

	if (!Valid || TypeNumber < 0 || TypeNumber > 5 || Tokens[0].size() != 1)
		return QObject::tr("Unknown line type '%1'.").arg(Tokens[0]);

	if (TypeNumber == 0)
		return QString();

	const int TokenCount = TypeTokenCounts[TypeNumber];

	// part file names may hold spaces
	if (Tokens.size() < TokenCount || (TypeNumber != 1 && Tokens.size() > TokenCount))
		return QObject::tr("A line of type %1 has %2 values instead of %3.").arg(TypeNumber).arg(Tokens.size()).arg(TokenCount);

	Tokens[1].toUInt(&Valid, 0);

	if (!Valid)
		return QObject::tr("Invalid color code '%1'.").arg(Tokens[1]);

	const int NumberEnd = TypeNumber == 1 ? TokenCount - 1 : TokenCount;

	for (int Index = 2; Index < NumberEnd; Index++)
	{
		Tokens[Index].toDouble(&Valid);

		if (!Valid)
			return QObject::tr("Invalid number '%1'.").arg(Tokens[Index]);
	}

	return QString();
}

// color ldraw lines by type
class swLDrawHighlighter : public QSyntaxHighlighter
{
public:
	explicit swLDrawHighlighter(QObject* Parent);

protected:
	void highlightBlock(const QString& Text) override;

	QTextCharFormat GetColorCodeFormat(const QString& Code) const;

	QTextCharFormat mCommentFormat;
	QTextCharFormat mKeywordFormat;
	QTextCharFormat mValueFormat;
	QTextCharFormat mNumberFormat;
	QTextCharFormat mTypeFormat;
	QTextCharFormat mPartFormat;
	QTextCharFormat mDataFormat;
	QTextCharFormat mErrorFormat;
};

swLDrawHighlighter::swLDrawHighlighter(QObject* Parent)
	: QSyntaxHighlighter(Parent)
{
	// tints for dark and light themes
	const bool DarkTheme = gApplication && gApplication->IsDarkTheme();

	mCommentFormat = swGetEditorTextFormat(swEditorTextFormat::Comment, DarkTheme);
	mKeywordFormat = swGetEditorTextFormat(swEditorTextFormat::Keyword, DarkTheme);
	mValueFormat = swGetEditorTextFormat(swEditorTextFormat::Value, DarkTheme);
	mNumberFormat = swGetEditorTextFormat(swEditorTextFormat::Number, DarkTheme);
	mTypeFormat = swGetEditorTextFormat(swEditorTextFormat::Type, DarkTheme);
	mPartFormat = swGetEditorTextFormat(swEditorTextFormat::Part, DarkTheme);
	mDataFormat = swGetEditorTextFormat(swEditorTextFormat::Data, DarkTheme);
	mErrorFormat = swGetEditorTextFormat(swEditorTextFormat::Error, DarkTheme);
}

QTextCharFormat swLDrawHighlighter::GetColorCodeFormat(const QString& Code) const
{
	bool Valid = false;
	const quint32 Value = Code.toUInt(&Valid, 0);

	if (!Valid)
		return mErrorFormat;

	QColor Color;

	// look up color codes without adding them
	if ((Value & 0xff000000) == 0x02000000)
		Color = QColor::fromRgb(Value & 0xffffff);
	else
	{
		for (const swColor& ListColor : gColorList)
		{
			if (ListColor.Code == Value)
			{
				Color = QColor::fromRgbF(ListColor.Value[0], ListColor.Value[1], ListColor.Value[2]);
				break;
			}
		}
	}

	if (!Color.isValid())
		return mNumberFormat;

	// keep colors readable on the background
	const int BaseLightness = QApplication::palette().color(QPalette::Base).lightness();

	if (qAbs(Color.lightness() - BaseLightness) < 80)
		Color = QColor::fromHsl(Color.hslHue(), Color.hslSaturation(), BaseLightness < 128 ? qMin(BaseLightness + 80, 255) : qMax(BaseLightness - 80, 0));

	QTextCharFormat Format;
	Format.setForeground(Color);
	Format.setFontWeight(QFont::Bold);

	return Format;
}

void swLDrawHighlighter::highlightBlock(const QString& Text)
{
	std::vector<std::pair<int, int>> Tokens;

	for (int Position = 0; Position < Text.size(); )
	{
		if (Text[Position].isSpace())
		{
			Position++;
			continue;
		}

		const int Start = Position;

		while (Position < Text.size() && !Text[Position].isSpace())
			Position++;

		Tokens.emplace_back(Start, Position - Start);
	}

	if (Tokens.empty())
		return;

	const auto GetToken = [&Text, &Tokens](size_t Index)
	{
		return Text.mid(Tokens[Index].first, Tokens[Index].second);
	};

	const auto SetTokenFormat = [this, &Tokens](size_t Index, const QTextCharFormat& Format)
	{
		setFormat(Tokens[Index].first, Tokens[Index].second, Format);
	};

	const auto SetRestFormat = [this, &Text, &Tokens](size_t Index, const QTextCharFormat& Format)
	{
		if (Index < Tokens.size())
			setFormat(Tokens[Index].first, Text.size() - Tokens[Index].first, Format);
	};

	const QString Type = GetToken(0);

	if (Type == QLatin1String("0"))
	{
		SetTokenFormat(0, mTypeFormat);

		if (Tokens.size() < 2)
			return;

		const QString Command = GetToken(1);

		// dim comments and embedded data
		if (Command.startsWith(QLatin1String("//")))
			SetRestFormat(1, mCommentFormat);
		else if (Command == QLatin1String("!:"))
		{
			SetTokenFormat(1, mKeywordFormat);
			SetRestFormat(2, mDataFormat);
		}
		else if (Command.startsWith('!') || swGetLDrawMetaKeywords().contains(Command, Qt::CaseInsensitive))
		{
			SetRestFormat(2, mValueFormat);
			SetTokenFormat(1, mKeywordFormat);

			// program metas name their command
			if (Command.startsWith('!') && Tokens.size() > 2 && GetToken(2) == GetToken(2).toUpper() && GetToken(2).at(0).isLetter())
				SetTokenFormat(2, mKeywordFormat);
		}
		else
			SetRestFormat(1, mCommentFormat);

		return;
	}

	// parameter count of each line type
	static const size_t TypeTokenCounts[] = { 0, 15, 8, 11, 14, 14 };
	bool Valid = false;
	const int TypeNumber = Type.toInt(&Valid);

	if (!Valid || TypeNumber < 1 || TypeNumber > 5 || Type.size() != 1)
	{
		SetRestFormat(0, mErrorFormat);
		return;
	}

	SetTokenFormat(0, mTypeFormat);

	const size_t TokenCount = TypeTokenCounts[TypeNumber];
	const size_t NumberEnd = TypeNumber == 1 ? TokenCount - 1 : TokenCount;

	if (Tokens.size() > 1)
		SetTokenFormat(1, GetColorCodeFormat(GetToken(1)));

	for (size_t Index = 2; Index < qMin(NumberEnd, Tokens.size()); Index++)
	{
		bool Number = false;
		GetToken(Index).toDouble(&Number);
		SetTokenFormat(Index, Number ? mNumberFormat : mErrorFormat);
	}

	if (TypeNumber == 1)
		SetRestFormat(NumberEnd, mPartFormat);
	else
		SetRestFormat(TokenCount, mErrorFormat);

	if (Tokens.size() < TokenCount)
		setFormat(0, 1, mErrorFormat);
}

swInstructionsEditorWidget::swInstructionsEditorWidget(QWidget* Parent, Project* Project, swInstructions* Instructions)
	: QDockWidget(Parent), mProject(Project), mInstructions(Instructions)
{
	setWindowTitle(tr("Editor"));
	setWhatsThis(tr("<p><b>Editor</b></p><p>The text of the model file as it is saved, its parts, steps and instructions. Edit it and apply to load the model from the text, the change is unsaved until the model is saved.</p>"));
	swSetupWidgetHelp(this);

	QWidget* CentralWidget = new QWidget(this);
	setWidget(CentralWidget);

	QVBoxLayout* Layout = new QVBoxLayout(CentralWidget);

	QWidget* ButtonRow = swCreatePanelRow(CentralWidget);

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

	mApplyButton = CreateButton(ButtonRow, tr("Apply"), tr("Check the syntax of the edited text and load the model from it, the change is unsaved until the model is saved."));
	mRevertButton = CreateButton(ButtonRow, tr("Revert"), tr("Discard the edits and show the text of the model again."));

	// shared editor with options, find and replace
	mHighlighter = new swLDrawHighlighter(this);
	mEditor = new swEditorWidget(CentralWidget, mHighlighter);
	mTextEdit = mEditor->GetTextEdit();
	mTextEdit->setToolTip(tr("Text of the model file."));
	mTextEdit->setAccessibleName(tr("Editor"));
	connect(mEditor, &swEditorWidget::SnapRequested, this, &swInstructionsEditorWidget::SnapRequested);

	// editor buttons below the text
	Layout->addWidget(mEditor, 1);
	Layout->addWidget(ButtonRow);

	connect(mApplyButton, &QToolButton::clicked, this, &swInstructionsEditorWidget::ApplyClicked);

	// save shortcut applies and saves
	QAction* SaveAction = new QAction(tr("Save"), this);
	SaveAction->setShortcuts(gMainWindow->mActions[SW_FILE_SAVE]->shortcuts());
	SaveAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	addAction(SaveAction);
	connect(SaveAction, &QAction::triggered, this, [this]() { ApplyText(true); });
	connect(mRevertButton, &QToolButton::clicked, this, &swInstructionsEditorWidget::RevertClicked);
	connect(mTextEdit->document(), &QTextDocument::modificationChanged, this, &swInstructionsEditorWidget::UpdateButtons);

	// follow the model while shown
	connect(this, &QDockWidget::visibilityChanged, this, [this](bool Visible)
	{
		if (Visible)
			ScheduleUpdate();
	});
	connect(mInstructions, &swInstructions::PageSetupChanged, this, &swInstructionsEditorWidget::ScheduleUpdate);
	connect(mInstructions, &swInstructions::PageLayoutChanged, this, &swInstructionsEditorWidget::ScheduleUpdate);
	connect(mInstructions, &swInstructions::StepSettingsChanged, this, &swInstructionsEditorWidget::ScheduleUpdate);

	UpdateButtons();
}

void swInstructionsEditorWidget::SetProject(Project* Project)
{
	mProject = Project;

	// the applied text is the project text
	mTextEdit->document()->setModified(false);
	ScheduleUpdate();
}

void swInstructionsEditorWidget::ScheduleUpdate()
{
	// write the text once per event
	if (mUpdatePending)
		return;

	mUpdatePending = true;

	QTimer::singleShot(0, this, [this]()
	{
		mUpdatePending = false;
		UpdateText();
	});
}

void swInstructionsEditorWidget::UpdateText()
{
	// revert shows the model again
	if (!isVisible() || mTextEdit->document()->isModified())
		return;

	QByteArray Bytes;
	QTextStream SaveStream(&Bytes, QIODevice::WriteOnly);

	if (!mProject->Save(SaveStream))
		return;

	SaveStream.flush();

	QTextStream ReadStream(&Bytes, QIODevice::ReadOnly);
	const QString Text = ReadStream.readAll();

	if (Text == mTextEdit->toPlainText())
		return;

	const int Position = GetTextPosition();
	const int Scroll = GetTextScroll();

	mTextEdit->setPlainText(Text);
	SetTextPosition(Position, Scroll);
}

void swInstructionsEditorWidget::UpdateButtons()
{
	const bool Modified = mTextEdit->document()->isModified();

	mApplyButton->setEnabled(Modified);
	mRevertButton->setEnabled(Modified);
}

int swInstructionsEditorWidget::GetTextPosition() const
{
	return mTextEdit->textCursor().position();
}

int swInstructionsEditorWidget::GetTextScroll() const
{
	return mTextEdit->verticalScrollBar()->value();
}

void swInstructionsEditorWidget::SetTextPosition(int Position, int Scroll)
{
	QTextCursor Cursor = mTextEdit->textCursor();
	Cursor.setPosition(qBound(0, Position, mTextEdit->document()->characterCount() - 1));
	mTextEdit->setTextCursor(Cursor);
	mTextEdit->verticalScrollBar()->setValue(Scroll);
}

void swInstructionsEditorWidget::ApplyClicked()
{
	ApplyText(false);
}

void swInstructionsEditorWidget::ApplyText(bool Save)
{
	// save an unedited text as is
	if (!mTextEdit->document()->isModified())
	{
		if (Save)
			gMainWindow->SaveProject(swGetActiveProject()->GetFileName());

		return;
	}

	if (!ValidateText())
		return;

	// read metas in capitals only
	QStringList Lines = mTextEdit->toPlainText().split('\n');

	for (QString& Line : Lines)
		Line = swGetLDrawMetaLine(Line);

	QByteArray Bytes;
	QTextStream Stream(&Bytes, QIODevice::WriteOnly);
	Stream << Lines.join('\n');
	Stream.flush();

	// load after this event
	QTimer::singleShot(0, gMainWindow, [Bytes, Save]()
	{
		if (gMainWindow->LoadProjectText(Bytes) && Save)
			gMainWindow->SaveProject(swGetActiveProject()->GetFileName());
	});
}

bool swInstructionsEditorWidget::ValidateText()
{
	QStringList Errors;
	int FirstErrorLine = -1;
	const QStringList Lines = mTextEdit->toPlainText().split('\n');

	for (int LineIndex = 0; LineIndex < Lines.size(); LineIndex++)
	{
		const QString Error = swGetLDrawLineError(Lines[LineIndex]);

		if (Error.isEmpty())
			continue;

		if (FirstErrorLine < 0)
			FirstErrorLine = LineIndex;

		Errors.append(tr("Line %1: %2").arg(LineIndex + 1).arg(Error));
	}

	if (Errors.isEmpty())
		return true;

	// select the first wrong line
	QTextCursor Cursor(mTextEdit->document()->findBlockByNumber(FirstErrorLine));
	Cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
	mTextEdit->setTextCursor(Cursor);
	mTextEdit->setFocus();

	const int ErrorCount = Errors.size();

	if (ErrorCount > 10)
	{
		Errors = Errors.mid(0, 10);
		Errors.append(tr("And %n more.", nullptr, ErrorCount - 10));
	}

	QMessageBox::warning(this, tr("Editor"), tr("The text was not applied. %n line(s) are not valid LDraw syntax:", nullptr, ErrorCount) + QLatin1String("\n\n") + Errors.join('\n'));

	return false;
}

// use to follow the preferences editor options
void swInstructionsEditorWidget::UpdateOptions()
{
	mEditor->UpdateOptions();
}

void swInstructionsEditorWidget::RevertClicked()
{
	mTextEdit->document()->setModified(false);
	UpdateText();
}

// use to fit whole lines in the text
int swInstructionsEditorWidget::GetSnapDelta() const
{
	const qreal LineHeight = mTextEdit->GetLineHeight();

	if (LineHeight <= 0.0)
		return 0;

	// lines start below the document margin
	const qreal Margin = mTextEdit->document()->documentMargin();
	const int ViewportHeight = mTextEdit->viewport()->height();
	int Lines = qMax(1, qRound((ViewportHeight - Margin) / LineHeight));
	int Delta = qCeil(Margin + Lines * LineHeight) - ViewportHeight;

	// round up when the panel cannot shrink
	if (height() + Delta < minimumSizeHint().height())
	{
		Lines++;
		Delta = qCeil(Margin + Lines * LineHeight) - ViewportHeight;
	}

	return Delta;
}
