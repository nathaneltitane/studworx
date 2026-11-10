#pragma once

#include "sw_object_property.h"

class swKeyframesView;

struct swKeyframesTrack
{
	swObjectPropertyId PropertyId;
	QString Name;
	bool Core;
	std::vector<swStep> Steps;
};

struct swKeyframesObject
{
	swObject* Object;
	QString Name;
	bool Selected;
	std::vector<swKeyframesTrack> Tracks;
};

struct swKeyframesRow
{
	int ObjectIndex;
	int TrackIndex;
};

class swKeyframesWidget : public QWidget
{
	Q_OBJECT

public:
	swKeyframesWidget(QWidget* Parent);

	void Update();
	void AddKeysClicked();
	void RemoveKeysClicked();
	void PlayClicked();
	void StopClicked();

private slots:
	void ShowAllToggled(bool ShowAll);
	void PlayStep();

private:
	void SetSelectedKeys(bool KeyFrame);

	void SetPlaying(bool Playing);

	swKeyframesView* mView;
	QToolButton* mPlayButton;
	QTimer* mPlayTimer;
	swStep mPlayStart = 0;
	QToolButton* mSelectedButton;
	QToolButton* mAllButton;
};

class swKeyframesView : public QAbstractScrollArea
{
	Q_OBJECT

public:
	swKeyframesView(QWidget* Parent);

	void Update();
	void SetShowAll(bool ShowAll);
	void SetCurrentStep(swStep Step);

	bool GetShowAll() const
	{
		return mShowAll;
	}

	const std::vector<swKeyframesObject>& GetObjects() const
	{
		return mObjects;
	}

	static bool HasStep(const std::vector<swStep>& Steps, swStep Step);

	QSize sizeHint() const override
	{
		return QSize(mNameWidth + 20 * mColumnWidth, 8 * mRowHeight);
	}

protected:
	void paintEvent(QPaintEvent* Event) override;
	void resizeEvent(QResizeEvent* Event) override;
	void mousePressEvent(QMouseEvent* Event) override;
	void mouseMoveEvent(QMouseEvent* Event) override;
	void mouseReleaseEvent(QMouseEvent* Event) override;
	void mouseDoubleClickEvent(QMouseEvent* Event) override;
	void contextMenuEvent(QContextMenuEvent* Event) override;

	void UpdateRows();
	void UpdateScrollBars();
	bool HitTest(const QPoint& Position, int* RowIndex, swStep* Step) const;
	bool RowHasKey(int RowIndex, swStep Step) const;
	std::vector<const swKeyframesTrack*> GetRowTracks(int RowIndex, bool CoreOnly) const;
	void EditRow(int RowIndex, swStep RemoveStep, swStep AddStep, const QString& Description);
	void RemoveRowKeys(int RowIndex);

	std::vector<swKeyframesObject> mObjects;
	std::vector<swKeyframesRow> mRows;
	QSet<swObject*> mExpanded;
	swStep mCurrentStep = 1;
	swStep mStepCount = 1;
	bool mShowAll = false;

	int mRowHeight = 20;
	int mColumnWidth = 24;
	int mNameWidth = 180;

	int mDragRow = -1;
	swStep mDragStep = 0;
	swStep mDragTarget = 0;
};
