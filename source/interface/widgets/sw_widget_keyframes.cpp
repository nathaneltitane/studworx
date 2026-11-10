#include "sw_global.h"
#include "sw_widget_keyframes.h"
#include "sw_window_main.h"
#include "sw_model.h"
#include "sw_object_part.h"
#include "sw_object_camera.h"
#include "sw_object_light.h"

swKeyframesWidget::swKeyframesWidget(QWidget* Parent)
	: QWidget(Parent)
{
	// panel overview

	setWhatsThis(tr("<p><b>Keyframes</b></p><p>A timeline of the animation keys with one column per step and one row per object, expandable to its properties. A diamond marks a key and a line joins the keys of one property. Click a step to go to it, double-click a cell to add or remove a key, drag a diamond to move the key to another step and right-click for more. Values are edited in the Properties panel.</p><p>Selected lists the selected objects, All every object with keys. The full and hollow diamonds add or remove the keys of the selected objects at the current step, Add Keys makes edits create keys, and the playback buttons play through the steps to show the keyed motion.</p>"));

	QVBoxLayout* Layout = new QVBoxLayout(this);
	QHBoxLayout* Buttons = new QHBoxLayout();

	// key buttons for the selection

	QToolButton* AddButton = new QToolButton(this);
	AddButton->setText(QString::fromUtf8("◆"));
	AddButton->setToolTip(tr("Add keys to the selected objects at the current step"));
	AddButton->setWhatsThis(tr("<p><b>Set Keys</b></p><p>Add position and rotation keys to the selected objects at the current step, holding their values at that step.</p>"));
	connect(AddButton, &QToolButton::clicked, this, &swKeyframesWidget::AddKeysClicked);
	Buttons->addWidget(AddButton);

	QToolButton* RemoveButton = new QToolButton(this);
	RemoveButton->setText(QString::fromUtf8("◇"));
	RemoveButton->setToolTip(tr("Remove the keys of the selected objects at the current step"));
	RemoveButton->setWhatsThis(tr("<p><b>Remove Keys</b></p><p>Remove every key the selected objects have at the current step.</p>"));
	connect(RemoveButton, &QToolButton::clicked, this, &swKeyframesWidget::RemoveKeysClicked);
	Buttons->addWidget(RemoveButton);

	QToolButton* AddKeysButton = new QToolButton(this);
	AddKeysButton->setDefaultAction(gMainWindow->mActions[SW_VIEW_TIME_ADD_KEYS]);
	AddKeysButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
	Buttons->addWidget(AddKeysButton);

	// playback through the steps

	QToolButton* RewindButton = new QToolButton(this);
	RewindButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));
	RewindButton->setToolTip(tr("Go to the first step"));
	RewindButton->setWhatsThis(tr("<p><b>Rewind</b></p><p>Go to the first step of the model.</p>"));
	connect(RewindButton, &QToolButton::clicked, this, [this]() { SetPlaying(false); mView->SetCurrentStep(1); });
	Buttons->addWidget(RewindButton);

	mPlayButton = new QToolButton(this);
	mPlayButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
	mPlayButton->setToolTip(tr("Play or pause the steps"));
	mPlayButton->setWhatsThis(tr("<p><b>Play</b></p><p>Step through the model one step at a time to the last step, showing the keyed motion. Click again to pause.</p>"));
	connect(mPlayButton, &QToolButton::clicked, this, &swKeyframesWidget::PlayClicked);
	Buttons->addWidget(mPlayButton);

	QToolButton* StopButton = new QToolButton(this);
	StopButton->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
	StopButton->setToolTip(tr("Stop and go back to the step playing started from"));
	StopButton->setWhatsThis(tr("<p><b>Stop</b></p><p>Stop playing through the steps and go back to the step playing started from.</p>"));
	connect(StopButton, &QToolButton::clicked, this, &swKeyframesWidget::StopClicked);
	Buttons->addWidget(StopButton);

	QToolButton* ForwardButton = new QToolButton(this);
	ForwardButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));
	ForwardButton->setToolTip(tr("Go to the last step"));
	ForwardButton->setWhatsThis(tr("<p><b>Forward</b></p><p>Go to the last step of the model.</p>"));
	connect(ForwardButton, &QToolButton::clicked, this, [this]()
	{
		SetPlaying(false);
		swModel* Model = gMainWindow->GetActiveModel();

		if (Model)
			mView->SetCurrentStep(Model->GetLastStep());
	});
	Buttons->addWidget(ForwardButton);

	mPlayTimer = new QTimer(this);
	mPlayTimer->setInterval(500);
	connect(mPlayTimer, &QTimer::timeout, this, &swKeyframesWidget::PlayStep);

	Buttons->addStretch(1);

	// selected or all objects

	mSelectedButton = new QToolButton(this);
	mSelectedButton->setText(tr("Selected"));
	mSelectedButton->setCheckable(true);
	mSelectedButton->setToolTip(tr("Show the keys of the selected objects"));
	mSelectedButton->setWhatsThis(tr("<p><b>Selected</b></p><p>List only the selected objects in the timeline, with or without keys.</p>"));
	Buttons->addWidget(mSelectedButton);

	mAllButton = new QToolButton(this);
	mAllButton->setText(tr("All"));
	mAllButton->setCheckable(true);
	mAllButton->setToolTip(tr("Show the keys of every object that has keys"));
	mAllButton->setWhatsThis(tr("<p><b>All</b></p><p>List the selected objects and every other object of the model that has keys.</p>"));
	Buttons->addWidget(mAllButton);

	QButtonGroup* ShowGroup = new QButtonGroup(this);
	ShowGroup->addButton(mSelectedButton);
	ShowGroup->addButton(mAllButton);

	Layout->addLayout(Buttons);

	mView = new swKeyframesView(this);
	Layout->addWidget(mView, 1);

	const bool ShowAll = QSettings().value("Keyframes/ShowAll", false).toBool();
	mAllButton->setChecked(ShowAll);
	mSelectedButton->setChecked(!ShowAll);
	mView->SetShowAll(ShowAll);
	connect(mAllButton, &QToolButton::toggled, this, &swKeyframesWidget::ShowAllToggled);
}

void swKeyframesWidget::Update()
{
	mView->Update();
}

void swKeyframesWidget::ShowAllToggled(bool ShowAll)
{
	QSettings().setValue("Keyframes/ShowAll", ShowAll);
	mView->SetShowAll(ShowAll);
}

void swKeyframesWidget::SetPlaying(bool Playing)
{
	if (Playing)
		mPlayTimer->start();
	else
		mPlayTimer->stop();

	mPlayButton->setIcon(style()->standardIcon(Playing ? QStyle::SP_MediaPause : QStyle::SP_MediaPlay));
}

void swKeyframesWidget::PlayClicked()
{
	swModel* Model = gMainWindow->GetActiveModel();

	if (!Model || mPlayTimer->isActive())
	{
		SetPlaying(false);
		return;
	}

	mPlayStart = Model->GetCurrentStep();

	// from the end plays from the start

	if (mPlayStart >= Model->GetLastStep())
		mView->SetCurrentStep(1);

	SetPlaying(true);
}

void swKeyframesWidget::StopClicked()
{
	SetPlaying(false);

	if (mPlayStart)
		mView->SetCurrentStep(mPlayStart);

	mPlayStart = 0;
}

void swKeyframesWidget::PlayStep()
{
	swModel* Model = gMainWindow->GetActiveModel();

	if (!Model || Model->GetCurrentStep() >= Model->GetLastStep())
	{
		SetPlaying(false);
		return;
	}

	mView->SetCurrentStep(Model->GetCurrentStep() + 1);
}

void swKeyframesWidget::AddKeysClicked()
{
	SetSelectedKeys(true);
}

void swKeyframesWidget::RemoveKeysClicked()
{
	SetSelectedKeys(false);
}

void swKeyframesWidget::SetSelectedKeys(bool KeyFrame)
{
	swModel* Model = gMainWindow->GetActiveModel();

	if (!Model)
		return;

	// shortcuts work with the panel hidden

	mView->Update();

	const swStep Step = Model->GetCurrentStep();
	std::vector<swKeyFrameEdit> Edits;

	for (const swKeyframesObject& Object : mView->GetObjects())
	{
		if (!Object.Selected)
			continue;

		for (const swKeyframesTrack& Track : Object.Tracks)
			if (KeyFrame ? Track.Core : swKeyframesView::HasStep(Track.Steps, Step))
				Edits.push_back({ Object.Object, Track.PropertyId, KeyFrame ? 0 : Step, KeyFrame ? Step : 0 });
	}

	Model->EditKeyFrames(Edits, KeyFrame ? tr("Add Keyframe") : tr("Remove Keyframe"));
}

swKeyframesView::swKeyframesView(QWidget* Parent)
	: QAbstractScrollArea(Parent)
{
	setMouseTracking(false);
	setToolTip(tr("Keys of each object per step: click a step to go there, double-click to add or remove a key, drag a key to move it"));

	const int TextHeight = fontMetrics().height();
	mRowHeight = TextHeight + 6;
	mColumnWidth = qMax(mRowHeight + 4, fontMetrics().horizontalAdvance(QLatin1String("000")) + 8);
	mNameWidth = fontMetrics().horizontalAdvance(QLatin1Char('M')) * 18;

	horizontalScrollBar()->setSingleStep(mColumnWidth);
	verticalScrollBar()->setSingleStep(mRowHeight);
}

bool swKeyframesView::HasStep(const std::vector<swStep>& Steps, swStep Step)
{
	return std::binary_search(Steps.begin(), Steps.end(), Step);
}

void swKeyframesView::SetShowAll(bool ShowAll)
{
	mShowAll = ShowAll;
	Update();
}

void swKeyframesView::Update()
{
	mObjects.clear();

	swModel* Model = gMainWindow ? gMainWindow->GetActiveModel() : nullptr;

	if (!Model)
	{
		UpdateRows();
		return;
	}

	mCurrentStep = Model->GetCurrentStep();
	const swStep LastStep = qMax(Model->GetLastStep(), mCurrentStep);

	// one column past the end

	mStepCount = LastStep + 1;

	const auto AddObject = [this](swObject* Object, std::initializer_list<swKeyframesTrack> Tracks)
	{
		swKeyframesObject Entry{ Object, Object->GetName(), Object->IsSelected(), {} };
		bool HasKeys = false;

		for (swKeyframesTrack Track : Tracks)
		{
			for (swStep Step = 1; Step <= mStepCount; Step++)
				if (Object->HasKeyFrame(Track.PropertyId, Step))
					Track.Steps.push_back(Step);

			HasKeys |= !Track.Steps.empty();

			if (Track.Core || !Track.Steps.empty())
				Entry.Tracks.push_back(Track);
		}

		if (Entry.Selected || (mShowAll && HasKeys))
			mObjects.push_back(std::move(Entry));
	};

	const QString Position = tr("Position");
	const QString Rotation = tr("Rotation");

	for (const std::unique_ptr<swCamera>& Camera : Model->GetCameras())
		AddObject(Camera.get(), {
			{ swObjectPropertyId::CameraPositionX, Position, true, {} },
			{ swObjectPropertyId::CameraTargetX, tr("Target"), true, {} },
			{ swObjectPropertyId::CameraUpX, tr("Up"), true, {} } });

	for (const std::unique_ptr<swLight>& Light : Model->GetLights())
		AddObject(Light.get(), {
			{ swObjectPropertyId::ObjectPositionX, Position, true, {} },
			{ swObjectPropertyId::ObjectRotationX, Rotation, true, {} },
			{ swObjectPropertyId::LightColor, tr("Color"), true, {} },
			{ swObjectPropertyId::LightBlenderPower, tr("Power"), false, {} },
			{ swObjectPropertyId::LightPOVRayPower, tr("POV-Ray Power"), false, {} },
			{ swObjectPropertyId::LightPOVRayFadeDistance, tr("Fade Distance"), false, {} },
			{ swObjectPropertyId::LightPOVRayFadePower, tr("Fade Power"), false, {} },
			{ swObjectPropertyId::LightBlenderRadius, tr("Radius"), false, {} },
			{ swObjectPropertyId::LightBlenderAngle, tr("Angle"), false, {} },
			{ swObjectPropertyId::LightAreaSizeX, tr("Area Width"), false, {} },
			{ swObjectPropertyId::LightAreaSizeY, tr("Area Height"), false, {} },
			{ swObjectPropertyId::LightSpotConeAngle, tr("Spot Cone"), false, {} },
			{ swObjectPropertyId::LightSpotPenumbraAngle, tr("Spot Penumbra"), false, {} },
			{ swObjectPropertyId::LightPOVRaySpotTightness, tr("Spot Tightness"), false, {} },
			{ swObjectPropertyId::LightPOVRayAreaGridX, tr("Area Grid X"), false, {} },
			{ swObjectPropertyId::LightPOVRayAreaGridY, tr("Area Grid Y"), false, {} } });

	for (const std::unique_ptr<swPart>& Part : Model->GetParts())
		AddObject(Part.get(), {
			{ swObjectPropertyId::ObjectPositionX, Position, true, {} },
			{ swObjectPropertyId::ObjectRotationX, Rotation, true, {} } });

	UpdateRows();
}

void swKeyframesView::UpdateRows()
{
	mRows.clear();

	for (int ObjectIndex = 0; ObjectIndex < static_cast<int>(mObjects.size()); ObjectIndex++)
	{
		mRows.push_back({ ObjectIndex, -1 });

		if (mExpanded.contains(mObjects[ObjectIndex].Object))
			for (int TrackIndex = 0; TrackIndex < static_cast<int>(mObjects[ObjectIndex].Tracks.size()); TrackIndex++)
				mRows.push_back({ ObjectIndex, TrackIndex });
	}

	UpdateScrollBars();
	viewport()->update();
}

void swKeyframesView::UpdateScrollBars()
{
	const QSize Size = viewport()->size();
	const int GridWidth = static_cast<int>(mStepCount) * mColumnWidth;
	const int GridHeight = static_cast<int>(mRows.size()) * mRowHeight;

	horizontalScrollBar()->setRange(0, qMax(0, GridWidth - (Size.width() - mNameWidth)));
	horizontalScrollBar()->setPageStep(qMax(mColumnWidth, Size.width() - mNameWidth));
	verticalScrollBar()->setRange(0, qMax(0, GridHeight - (Size.height() - mRowHeight)));
	verticalScrollBar()->setPageStep(qMax(mRowHeight, Size.height() - mRowHeight));
}

void swKeyframesView::resizeEvent(QResizeEvent* Event)
{
	QAbstractScrollArea::resizeEvent(Event);
	UpdateScrollBars();
}

bool swKeyframesView::HitTest(const QPoint& Position, int* RowIndex, swStep* Step) const
{
	*RowIndex = Position.y() < mRowHeight ? -1 : (Position.y() - mRowHeight + verticalScrollBar()->value()) / mRowHeight;
	*Step = 0;

	if (*RowIndex >= static_cast<int>(mRows.size()))
		*RowIndex = -2;

	if (Position.x() >= mNameWidth)
		*Step = (Position.x() - mNameWidth + horizontalScrollBar()->value()) / mColumnWidth + 1;

	return *RowIndex != -2;
}

std::vector<const swKeyframesTrack*> swKeyframesView::GetRowTracks(int RowIndex, bool CoreOnly) const
{
	std::vector<const swKeyframesTrack*> Tracks;

	if (RowIndex < 0 || RowIndex >= static_cast<int>(mRows.size()))
		return Tracks;

	const swKeyframesRow& Row = mRows[RowIndex];
	const swKeyframesObject& Object = mObjects[Row.ObjectIndex];

	if (Row.TrackIndex >= 0)
		Tracks.push_back(&Object.Tracks[Row.TrackIndex]);
	else
		for (const swKeyframesTrack& Track : Object.Tracks)
			if (!CoreOnly || Track.Core)
				Tracks.push_back(&Track);

	return Tracks;
}

bool swKeyframesView::RowHasKey(int RowIndex, swStep Step) const
{
	for (const swKeyframesTrack* Track : GetRowTracks(RowIndex, false))
		if (HasStep(Track->Steps, Step))
			return true;

	return false;
}

void swKeyframesView::EditRow(int RowIndex, swStep RemoveStep, swStep AddStep, const QString& Description)
{
	swModel* Model = gMainWindow->GetActiveModel();

	if (!Model || RowIndex < 0)
		return;

	swObject* Object = mObjects[mRows[RowIndex].ObjectIndex].Object;
	std::vector<swKeyFrameEdit> Edits;

	// adding on an object row keys its core

	for (const swKeyframesTrack* Track : GetRowTracks(RowIndex, !RemoveStep))
		Edits.push_back({ Object, Track->PropertyId, RemoveStep, AddStep });

	Model->EditKeyFrames(Edits, Description);
}

void swKeyframesView::RemoveRowKeys(int RowIndex)
{
	swModel* Model = gMainWindow->GetActiveModel();

	if (!Model || RowIndex < 0)
		return;

	swObject* Object = mObjects[mRows[RowIndex].ObjectIndex].Object;
	std::vector<swKeyFrameEdit> Edits;

	for (const swKeyframesTrack* Track : GetRowTracks(RowIndex, false))
		for (swStep Step : Track->Steps)
			Edits.push_back({ Object, Track->PropertyId, Step, 0 });

	Model->EditKeyFrames(Edits, tr("Remove Keyframes"));
}

void swKeyframesView::SetCurrentStep(swStep Step)
{
	swModel* Model = gMainWindow->GetActiveModel();

	if (Model && Step && Step != Model->GetCurrentStep())
	{
		Model->SetCurrentStep(Step);
		gMainWindow->UpdateCurrentStep();
		Model->UpdateAllViews();
	}
}

void swKeyframesView::mousePressEvent(QMouseEvent* Event)
{
	if (Event->button() != Qt::LeftButton)
		return;

	int RowIndex;
	swStep Step;

	if (!HitTest(Event->pos(), &RowIndex, &Step))
		return;

	// name column folds objects

	if (!Step)
	{
		if (RowIndex >= 0 && mRows[RowIndex].TrackIndex < 0)
		{
			swObject* Object = mObjects[mRows[RowIndex].ObjectIndex].Object;

			if (mExpanded.contains(Object))
				mExpanded.remove(Object);
			else
				mExpanded.insert(Object);

			UpdateRows();
		}

		return;
	}

	if (RowIndex >= 0 && RowHasKey(RowIndex, Step))
	{
		mDragRow = RowIndex;
		mDragStep = Step;
		mDragTarget = Step;
	}
	else
		SetCurrentStep(Step);
}

void swKeyframesView::mouseMoveEvent(QMouseEvent* Event)
{
	if (mDragRow < 0)
		return;

	const int X = Event->pos().x() - mNameWidth + horizontalScrollBar()->value();
	const swStep Target = static_cast<swStep>(qMax(0, X) / mColumnWidth + 1);

	if (Target != mDragTarget)
	{
		mDragTarget = Target;
		viewport()->update();
	}
}

void swKeyframesView::mouseReleaseEvent(QMouseEvent* Event)
{
	if (Event->button() != Qt::LeftButton || mDragRow < 0)
		return;

	const int RowIndex = mDragRow;
	const swStep From = mDragStep;
	const swStep To = mDragTarget;

	mDragRow = -1;
	viewport()->update();

	if (To != From)
		EditRow(RowIndex, From, To, tr("Move Keyframe"));
	else
		SetCurrentStep(From);
}

void swKeyframesView::mouseDoubleClickEvent(QMouseEvent* Event)
{
	int RowIndex;
	swStep Step;

	if (Event->button() != Qt::LeftButton || !HitTest(Event->pos(), &RowIndex, &Step) || RowIndex < 0 || !Step)
		return;

	mDragRow = -1;

	if (RowHasKey(RowIndex, Step))
		EditRow(RowIndex, Step, 0, tr("Remove Keyframe"));
	else
		EditRow(RowIndex, 0, Step, tr("Add Keyframe"));
}

void swKeyframesView::contextMenuEvent(QContextMenuEvent* Event)
{
	int RowIndex;
	swStep Step;

	if (!HitTest(Event->pos(), &RowIndex, &Step) || RowIndex < 0)
		return;

	QMenu Menu(this);
	QAction* AddAction = Menu.addAction(tr("Add Key"));
	QAction* RemoveAction = Menu.addAction(tr("Remove Key"));
	Menu.addSeparator();
	QAction* RemoveAllAction = Menu.addAction(tr("Remove All Keys"));

	const bool HasKey = Step && RowHasKey(RowIndex, Step);
	bool HasAnyKey = false;

	for (const swKeyframesTrack* Track : GetRowTracks(RowIndex, false))
		HasAnyKey |= !Track->Steps.empty();

	AddAction->setEnabled(Step && !HasKey);
	RemoveAction->setEnabled(HasKey);
	RemoveAllAction->setEnabled(HasAnyKey);

	QAction* Action = Menu.exec(Event->globalPos());

	if (Action == AddAction)
		EditRow(RowIndex, 0, Step, tr("Add Keyframe"));
	else if (Action == RemoveAction)
		EditRow(RowIndex, Step, 0, tr("Remove Keyframe"));
	else if (Action == RemoveAllAction)
		RemoveRowKeys(RowIndex);
}

void swKeyframesView::paintEvent(QPaintEvent* Event)
{
	Q_UNUSED(Event);

	QPainter Painter(viewport());
	const QPalette& Palette = palette();
	const QRect Rect = viewport()->rect();
	const int ScrollX = horizontalScrollBar()->value();
	const int ScrollY = verticalScrollBar()->value();
	const int Diamond = mRowHeight / 3;

	Painter.fillRect(Rect, Palette.base());

	const QColor GridColor = Palette.color(QPalette::Mid);
	const QColor KeyColor = Palette.color(QPalette::Text);
	const QColor HighlightColor = Palette.color(QPalette::Highlight);

	const auto StepX = [&](swStep Step)
	{
		return mNameWidth + static_cast<int>(Step - 1) * mColumnWidth - ScrollX + mColumnWidth / 2;
	};

	const auto DrawDiamond = [&](int X, int Y, int Size, const QColor& Color)
	{
		const QPoint Points[4] = { QPoint(X, Y - Size), QPoint(X + Size, Y), QPoint(X, Y + Size), QPoint(X - Size, Y) };
		Painter.setPen(Qt::NoPen);
		Painter.setBrush(Color);
		Painter.drawConvexPolygon(Points, 4);
	};

	Painter.setRenderHint(QPainter::Antialiasing, true);

	// rows below the header

	const int FirstRow = ScrollY / mRowHeight;
	const int LastRow = qMin(static_cast<int>(mRows.size()) - 1, (ScrollY + Rect.height()) / mRowHeight);
	const QRect GridClip(mNameWidth, mRowHeight, Rect.width() - mNameWidth, Rect.height() - mRowHeight);

	for (int RowIndex = FirstRow; RowIndex <= LastRow; RowIndex++)
	{
		const swKeyframesRow& Row = mRows[RowIndex];
		const swKeyframesObject& Object = mObjects[Row.ObjectIndex];
		const int Y = mRowHeight + RowIndex * mRowHeight - ScrollY;
		const int CenterY = Y + mRowHeight / 2;

		if (Row.TrackIndex < 0)
			Painter.fillRect(QRect(0, Y, Rect.width(), mRowHeight), Palette.alternateBase());

		// name column

		Painter.setClipRect(QRect(0, mRowHeight, mNameWidth, Rect.height()));
		QFont Font = font();
		Font.setBold(Row.TrackIndex < 0 && Object.Selected);
		Painter.setFont(Font);
		Painter.setPen(KeyColor);

		QString Name;

		if (Row.TrackIndex < 0)
			Name = QString::fromUtf8(mExpanded.contains(Object.Object) ? "▾ " : "▸ ") + Object.Name;
		else
			Name = QLatin1String("      ") + Object.Tracks[Row.TrackIndex].Name;

		Painter.drawText(QRect(4, Y, mNameWidth - 8, mRowHeight), Qt::AlignVCenter | Qt::AlignLeft, Painter.fontMetrics().elidedText(Name, Qt::ElideRight, mNameWidth - 8));

		// keys joined per property

		Painter.setClipRect(GridClip);

		std::vector<swStep> Steps;

		for (const swKeyframesTrack* Track : GetRowTracks(RowIndex, false))
		{
			if (Row.TrackIndex >= 0 && Track->Steps.size() > 1)
			{
				Painter.setPen(QPen(GridColor, 2));
				Painter.drawLine(StepX(Track->Steps.front()), CenterY, StepX(Track->Steps.back()), CenterY);
			}

			Steps.insert(Steps.end(), Track->Steps.begin(), Track->Steps.end());
		}

		const bool Dragging = RowIndex == mDragRow && mDragTarget != mDragStep;

		for (swStep Step : Steps)
			DrawDiamond(StepX(Step), CenterY, Row.TrackIndex < 0 ? Diamond - 1 : Diamond, (Dragging && Step == mDragStep) ? GridColor : KeyColor);

		if (Dragging)
			DrawDiamond(StepX(mDragTarget), CenterY, Diamond, HighlightColor);
	}

	// step header

	Painter.setClipping(false);
	Painter.fillRect(QRect(0, 0, Rect.width(), mRowHeight), Palette.button());
	Painter.setPen(Palette.color(QPalette::ButtonText));
	Painter.setFont(font());
	Painter.drawText(QRect(4, 0, mNameWidth - 8, mRowHeight), Qt::AlignVCenter | Qt::AlignLeft, tr("Step"));
	Painter.setClipRect(QRect(mNameWidth, 0, Rect.width() - mNameWidth, mRowHeight));

	const swStep FirstStep = static_cast<swStep>(ScrollX / mColumnWidth + 1);
	const swStep LastStep = qMin(mStepCount, static_cast<swStep>((ScrollX + Rect.width() - mNameWidth) / mColumnWidth + 1));

	for (swStep Step = FirstStep; Step <= LastStep; Step++)
	{
		const int X = StepX(Step) - mColumnWidth / 2;
		Painter.drawText(QRect(X, 0, mColumnWidth, mRowHeight), Qt::AlignCenter, QString::number(Step));
	}

	// current step line

	Painter.setClipRect(QRect(mNameWidth, 0, Rect.width() - mNameWidth, Rect.height()));
	Painter.setPen(QPen(HighlightColor, 2));
	Painter.drawLine(StepX(mCurrentStep), 0, StepX(mCurrentStep), Rect.height());

	Painter.setClipping(false);
	Painter.setPen(GridColor);
	Painter.drawLine(mNameWidth, 0, mNameWidth, Rect.height());
	Painter.drawLine(0, mRowHeight, Rect.width(), mRowHeight);
}
