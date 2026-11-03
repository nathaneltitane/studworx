#pragma once

enum class swEditorTextFormat
{
	Comment,
	Keyword,
	Value,
	Number,
	Type,
	Part,
	Data,
	Error
};

// use to get the editor highlight of a text kind, tinted for dark and light themes
inline QTextCharFormat swGetEditorTextFormat(swEditorTextFormat Format, bool DarkTheme)
{
	struct swEditorTextStyle
	{
		QRgb Dark;
		QRgb Light;
		bool Bold;
		bool Italic;
	};

	static const swEditorTextStyle Styles[] = { { 0x8a95a3, 0x6a737d, false, true }, { 0x82aaff, 0x0033b3, true, false }, { 0xc3e88d, 0x067d17, false, false }, { 0xf78c6c, 0x1750eb, false, false }, { 0xffcb6b, 0x9e6a00, true, false }, { 0x89ddff, 0x00627a, true, false }, { 0x6a7380, 0x9aa0a6, false, false }, { 0xff5370, 0xd32f2f, false, false } };
	const swEditorTextStyle& Style = Styles[static_cast<int>(Format)];
	QTextCharFormat TextFormat;

	TextFormat.setForeground(QColor(DarkTheme ? Style.Dark : Style.Light));
	TextFormat.setFontItalic(Style.Italic);

	if (Style.Bold)
		TextFormat.setFontWeight(QFont::Bold);

	// errors get a wavy underline
	if (Format == swEditorTextFormat::Error)
	{
		TextFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
		TextFormat.setUnderlineColor(TextFormat.foreground().color());
	}

	return TextFormat;
}

// number lines and mark line ends
class swEditorTextEdit : public QPlainTextEdit
{
public:
	swEditorTextEdit(QWidget* Parent)
		: QPlainTextEdit(Parent)
	{
		mLineNumberArea = new QWidget(this);
		mLineNumberArea->installEventFilter(this);

		connect(this, &QPlainTextEdit::blockCountChanged, this, &swEditorTextEdit::UpdateLineNumberWidth);
		connect(this, &QPlainTextEdit::updateRequest, this, &swEditorTextEdit::UpdateLineNumberArea);
		connect(this, &QPlainTextEdit::cursorPositionChanged, mLineNumberArea, QOverload<>::of(&QWidget::update));
	}

	void SetLineMarkersVisible(bool Visible)
	{
		mLineMarkersVisible = Visible;
		viewport()->update();
	}

	void SetLineNumbersVisible(bool Visible)
	{
		mLineNumbersVisible = Visible;
		mLineNumberArea->setVisible(Visible);
		UpdateLineNumberWidth();
	}

	qreal GetLineHeight() const
	{
		// measure the laid out lines
		const QTextBlock Block = document()->firstBlock();
		const QTextLayout* Layout = Block.layout();

		if (Block.isValid() && Layout && Layout->lineCount())
			return blockBoundingRect(Block).height() / Layout->lineCount();

		return fontMetrics().lineSpacing();
	}

protected:
	int GetLineNumberWidth() const
	{
		if (!mLineNumbersVisible)
			return 0;

		const QFontMetrics Metrics(font());
		const int Digits = QString::number(qMax(blockCount(), 1)).size();

		return Metrics.horizontalAdvance(QLatin1Char('9')) * Digits + Metrics.horizontalAdvance(QLatin1Char(' ')) * 2;
	}

	void UpdateLineNumberWidth()
	{
		setViewportMargins(GetLineNumberWidth(), 0, 0, 0);
		ResizeLineNumberArea();
	}

	void UpdateLineNumberArea(const QRect& Rect, int ScrollY)
	{
		if (ScrollY)
			mLineNumberArea->scroll(0, ScrollY);
		else
			mLineNumberArea->update(0, Rect.y(), mLineNumberArea->width(), Rect.height());

		if (Rect.contains(viewport()->rect()))
			UpdateLineNumberWidth();
	}

	void ResizeLineNumberArea()
	{
		const QRect Rect = contentsRect();
		mLineNumberArea->setGeometry(Rect.left(), Rect.top(), GetLineNumberWidth(), Rect.height());
	}

	void resizeEvent(QResizeEvent* Event) override
	{
		QPlainTextEdit::resizeEvent(Event);
		ResizeLineNumberArea();
	}

	void changeEvent(QEvent* Event) override
	{
		QPlainTextEdit::changeEvent(Event);

		if (Event->type() == QEvent::FontChange)
			UpdateLineNumberWidth();
	}

	bool eventFilter(QObject* Object, QEvent* Event) override
	{
		if (Object == mLineNumberArea && Event->type() == QEvent::Paint)
		{
			PaintLineNumbers(static_cast<QPaintEvent*>(Event)->rect());
			return true;
		}

		return QPlainTextEdit::eventFilter(Object, Event);
	}

	void PaintLineNumbers(const QRect& Rect)
	{
		QPainter Painter(mLineNumberArea);
		Painter.fillRect(Rect, palette().color(QPalette::Window));
		Painter.setFont(font());

		// number the cursor line in text color
		const int CursorBlock = textCursor().blockNumber();
		const int Margin = QFontMetrics(font()).horizontalAdvance(QLatin1Char(' '));
		const int Width = mLineNumberArea->width() - Margin;
		const int Height = QFontMetrics(font()).height();
		const QPointF Offset = contentOffset();

		for (QTextBlock Block = firstVisibleBlock(); Block.isValid(); Block = Block.next())
		{
			const int Top = qRound(blockBoundingGeometry(Block).translated(Offset).top());

			if (Top > Rect.bottom())
				break;

			if (!Block.isVisible())
				continue;

			Painter.setPen(palette().color(Block.blockNumber() == CursorBlock ? QPalette::Text : QPalette::PlaceholderText));
			Painter.drawText(0, Top, Width, Height, Qt::AlignRight | Qt::AlignVCenter, QString::number(Block.blockNumber() + 1));
		}
	}

	void paintEvent(QPaintEvent* Event) override
	{
		QPlainTextEdit::paintEvent(Event);

		if (!mLineMarkersVisible)
			return;

		QPainter Painter(viewport());
		Painter.setFont(font());
		Painter.setPen(palette().color(QPalette::PlaceholderText));

		const QPointF Offset = contentOffset();
		const int Bottom = Event->rect().bottom();

		for (QTextBlock Block = firstVisibleBlock(); Block.isValid(); Block = Block.next())
		{
			const QRectF Geometry = blockBoundingGeometry(Block).translated(Offset);

			if (Geometry.top() > Bottom)
				break;

			const QTextLayout* Layout = Block.layout();

			// an empty last line has no end
			if (!Block.isVisible() || !Layout || !Layout->lineCount() || (!Block.next().isValid() && Block.length() <= 1))
				continue;

			// arrow after the last character
			const QTextLine Line = Layout->lineAt(Layout->lineCount() - 1);
			const qreal X = Geometry.left() + Line.cursorToX(Block.length() - 1);
			const qreal Y = Geometry.top() + Line.y() + Line.ascent();

			Painter.drawText(QPointF(X, Y), QString(QChar(0x21b5)));
		}
	}

	QWidget* mLineNumberArea = nullptr;
	bool mLineMarkersVisible = false;
	bool mLineNumbersVisible = false;
};
