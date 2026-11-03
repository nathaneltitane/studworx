#pragma once

class swEditorTextEdit;

// text editor with options, find and replace
class swEditorWidget : public QWidget
{
	Q_OBJECT

public:
	swEditorWidget(QWidget* Parent, QSyntaxHighlighter* Highlighter);

	swEditorTextEdit* GetTextEdit() const
	{
		return mTextEdit;
	}

	void UpdateOptions();

signals:
	void SnapRequested();

protected:
	bool eventFilter(QObject* Object, QEvent* Event) override;
	void SetHighlightEnabled(bool Enabled);
	void SetNumbersEnabled(bool Enabled);
	void SetWrapEnabled(bool Enabled);
	void SetMarkersEnabled(bool Enabled);
	void SetFindVisible(bool Visible);
	void FindMatch(bool Backward);
	void ReplaceClicked();
	void ReplaceAllClicked();
	void UpdateMatches();
	QTextDocument::FindFlags GetFindFlags() const;

	QSyntaxHighlighter* mHighlighter = nullptr;
	swEditorTextEdit* mTextEdit = nullptr;
	QAction* mFindAction = nullptr;
	QAction* mHighlightAction = nullptr;
	QAction* mNumbersAction = nullptr;
	QAction* mWrapAction = nullptr;
	QAction* mMarkersAction = nullptr;
	QWidget* mFindWidget = nullptr;
	QLineEdit* mFindEdit = nullptr;
	QLineEdit* mReplaceEdit = nullptr;
	QLabel* mMatchLabel = nullptr;
	bool mMatchCase = false;
	bool mWholeWords = false;
};
