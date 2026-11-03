#pragma once

class Project;
class swInstructions;
class swLDrawHighlighter;
class swEditorTextEdit;
class swEditorWidget;

// The editor panel shows the saved text.
class swInstructionsEditorWidget : public QDockWidget
{
	Q_OBJECT

public:
	swInstructionsEditorWidget(QWidget* Parent, Project* Project, swInstructions* Instructions);

	void SetProject(Project* Project);
	void ScheduleUpdate();
	void UpdateOptions();

	int GetTextPosition() const;
	int GetTextScroll() const;
	void SetTextPosition(int Position, int Scroll);
	int GetSnapDelta() const;

signals:
	void SnapRequested();

protected:
	void UpdateText();
	void UpdateButtons();
	void ApplyClicked();
	void ApplyText(bool Save);
	bool ValidateText();
	void RevertClicked();

	Project* mProject = nullptr;
	swInstructions* mInstructions = nullptr;
	bool mUpdatePending = false;

	swLDrawHighlighter* mHighlighter = nullptr;
	swEditorWidget* mEditor = nullptr;
	swEditorTextEdit* mTextEdit = nullptr;
	QToolButton* mApplyButton = nullptr;
	QToolButton* mRevertButton = nullptr;
};
