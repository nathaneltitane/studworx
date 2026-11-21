#include "sw_global.h"
#include "sw_dialog_instructions_page.h"
#include "sw_dialog_instructions_placement.h"
#include "sw_dialog_instructions_properties.h"
#include "sw_project.h"
#include "sw_model.h"
#include "sw_object_part.h"
#include "sw_object_camera.h"
#include "sw_view.h"
#include "sw_window_utilities.h"

// use to identify a page element
bool swGetElementKey(const QGraphicsItem* Item, swInstructionsElementKey& Key)
{
	if (const swInstructionsStepImageItem* ImageItem = dynamic_cast<const swInstructionsStepImageItem*>(Item))
	{
		// the final model is no element of its own

		if (ImageItem->IsFinalModel())
		{
			Key = { swInstructionsPlacementElement::None, nullptr, 0 };
			return false;
		}


		swInstructionsPlacementElement Element = ImageItem->IsCoverImage() ? swInstructionsPlacementElement::CoverImage : swInstructionsPlacementElement::StepImage;

		if (ImageItem->IsSubmodelPreview())
			Element = swInstructionsPlacementElement::SubmodelPreview;

		Key = { Element, ImageItem->GetModel(), ImageItem->GetStep() };
	}
	else if (const swInstructionsStepNumberItem* NumberItem = dynamic_cast<const swInstructionsStepNumberItem*>(Item))
		Key = { swInstructionsPlacementElement::StepNumber, NumberItem->GetModel(), NumberItem->GetStep() };
	else if (const swInstructionsPartsListCountItem* QuantityItem = dynamic_cast<const swInstructionsPartsListCountItem*>(Item))
		Key = { swInstructionsPlacementElement::PartsListCount, QuantityItem->GetModel(), QuantityItem->GetStep() };
	else if (const swInstructionsPartsListItem* PartsItem = dynamic_cast<const swInstructionsPartsListItem*>(Item))
	{
		const swInstructionsPlacementElement Element = PartsItem->IsBillOfMaterials() ? swInstructionsPlacementElement::BillOfMaterials : swInstructionsPlacementElement::PartsList;
		Key = { Element, PartsItem->GetModel(), PartsItem->GetStep() };
	}
	else if (const swInstructionsCoverTextItem* TextItem = dynamic_cast<const swInstructionsCoverTextItem*>(Item))
	{
		const swInstructionsPlacementElement Element = TextItem->IsAuthor() ? swInstructionsPlacementElement::CoverAuthor : swInstructionsPlacementElement::CoverTitle;
		Key = { Element, TextItem->GetModel(), 0 };
	}
	else if (dynamic_cast<const swInstructionsPageNumberItem*>(Item))
		Key = { swInstructionsPlacementElement::PageNumber, nullptr, 0 };
	else if (const swInstructionsPartCountItem* CountItem = dynamic_cast<const swInstructionsPartCountItem*>(Item))
		Key = { CountItem->GetElement(), CountItem->GetModel(), 0 };
	else if (const swInstructionsNoticeItem* NoticeItem = dynamic_cast<const swInstructionsNoticeItem*>(Item))
		Key = { swInstructionsPlacementElement::Notice, nullptr, static_cast<swStep>(NoticeItem->GetId()) };
	else if (const swInstructionsImageItem* LogoItem = dynamic_cast<const swInstructionsImageItem*>(Item))
		Key = { swInstructionsPlacementElement::Image, nullptr, static_cast<swStep>(LogoItem->GetId()) };
	else if (const swInstructionsPointerItem* PointerItem = dynamic_cast<const swInstructionsPointerItem*>(Item))
		Key = { swInstructionsPlacementElement::Pointer, nullptr, static_cast<swStep>(PointerItem->GetId()) };
	else
	{
		Key = { swInstructionsPlacementElement::None, nullptr, 0 };
		return false;
	}

	return true;
}

// use to get the corners, edge centers and center of a rectangle
static std::array<QPointF, 9> swGetElementAnchors(const QRectF& Rect)
{
	const QPointF Center = Rect.center();

	return
	{
		Rect.topLeft(), QPointF(Center.x(), Rect.top()), Rect.topRight(),
		QPointF(Rect.left(), Center.y()), Center, QPointF(Rect.right(), Center.y()),
		Rect.bottomLeft(), QPointF(Center.x(), Rect.bottom()), Rect.bottomRight()
	};
}

// use to snap a dragged element to the anchors of the page area and the other elements
static QPointF swSnapAnchorPosition(const QGraphicsItem* Item, const swInstructions* Instructions, const QPointF& ScenePosition)
{
	const swInstructionsBoxBase* Box = dynamic_cast<const swInstructionsBoxBase*>(Item);
	const QGraphicsScene* Scene = Item->scene();

	if (!Box || !Scene || Scene->views().isEmpty())
		return ScenePosition;

	// snap within a few pixels at any zoom
	const qreal Scale = qMax(Scene->views().front()->transform().m11(), 0.0001);
	const qreal Tolerance = 8.0 / Scale;

	// the dragged anchors at the new position
	const QPointF Offset = ScenePosition - Item->scenePos();
	const std::array<QPointF, 9> ItemAnchors = swGetElementAnchors(Item->mapRectToScene(Box->GetBorderRect()).translated(Offset));

	std::vector<QPointF> TargetAnchors;
	const swInstructionsPageSetup& PageSetup = Instructions->mPageSetup;
	const QRectF AreaRect(QPointF(PageSetup.MarginLeft, PageSetup.MarginTop), QPointF(PageSetup.Width - PageSetup.MarginRight, PageSetup.Height - PageSetup.MarginBottom));

	for (const QPointF& Anchor : swGetElementAnchors(AreaRect))
		TargetAnchors.push_back(Anchor);

	// elements moving along are no targets
	const QList<QGraphicsItem*> Items = Scene->items();

	for (const QGraphicsItem* Other : Items)
	{
		const swInstructionsBoxBase* OtherBox = dynamic_cast<const swInstructionsBoxBase*>(Other);

		if (!OtherBox || Other == Item || !Other->isVisible() || Other->isSelected() || Item->isAncestorOf(Other) || Other->isAncestorOf(Item))
			continue;

		for (const QPointF& Anchor : swGetElementAnchors(Other->mapRectToScene(OtherBox->GetBorderRect())))
			TargetAnchors.push_back(Anchor);
	}

	// nearest anchor on each axis
	qreal DistanceX = Tolerance;
	qreal DistanceY = Tolerance;
	QPointF Delta;

	for (const QPointF& Target : TargetAnchors)
	{
		for (const QPointF& Anchor : ItemAnchors)
		{
			const qreal OffsetX = Target.x() - Anchor.x();
			const qreal OffsetY = Target.y() - Anchor.y();

			if (qAbs(OffsetX) < DistanceX)
			{
				DistanceX = qAbs(OffsetX);
				Delta.setX(OffsetX);
			}

			if (qAbs(OffsetY) < DistanceY)
			{
				DistanceY = qAbs(OffsetY);
				Delta.setY(OffsetY);
			}
		}
	}

	return ScenePosition + Delta;
}

// use to snap a dragged element
static QVariant swSnapItemPosition(const QGraphicsItem* Item, const swInstructions* Instructions, const QVariant& Value)
{
	const QGraphicsScene* Scene = Item->scene();
	const QGraphicsItem* Parent = Item->parentItem();
	const QPointF Position = Value.toPointF();
	QPointF ScenePosition = Parent ? Parent->mapToScene(Position) : Position;

	// only dragging snaps, to the anchors or to the grid
	if (Scene && Scene->mouseGrabberItem())
	{
		if (Instructions->IsSnapToAnchors())
			ScenePosition = swSnapAnchorPosition(Item, Instructions, ScenePosition);
		else if (Instructions->IsSnapToGrid())
		{
			const QPointF Origin = Instructions->GetGridRect().topLeft();
			const qreal GridSize = Instructions->GetGridSnapSize();

			ScenePosition = Origin + QPointF(qRound((ScenePosition.x() - Origin.x()) / GridSize) * GridSize, qRound((ScenePosition.y() - Origin.y()) / GridSize) * GridSize);
		}
	}

	// keep large elements at the top left margin
	const swInstructionsPageSetup& PageSetup = Instructions->mPageSetup;
	const QRectF ItemRect = Item->boundingRect();
	QRectF BoundsRect(QPointF(PageSetup.MarginLeft, PageSetup.MarginTop), QPointF(PageSetup.Width - PageSetup.MarginRight, PageSetup.Height - PageSetup.MarginBottom));

	// page edge placements reach past the margins
	const QVariant PlacedPosition = Item->data(SW_PLACEMENT_POSITION_DATA);

	if (PlacedPosition.isValid())
		BoundsRect = BoundsRect.united(ItemRect.translated(Parent ? Parent->mapToScene(PlacedPosition.toPointF()) : PlacedPosition.toPointF()));

	const QPointF MarginsMin = BoundsRect.topLeft() - ItemRect.topLeft();
	const QPointF MarginsMax = BoundsRect.bottomRight() - ItemRect.bottomRight();

	ScenePosition.setX(qMax(MarginsMin.x(), qMin(ScenePosition.x(), MarginsMax.x())));
	ScenePosition.setY(qMax(MarginsMin.y(), qMin(ScenePosition.y(), MarginsMax.y())));

	return Parent ? Parent->mapFromScene(ScenePosition) : ScenePosition;
}

// drag and snap movable elements
constexpr QGraphicsItem::GraphicsItemFlags SW_MOVABLE_ITEM_FLAGS = QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemSendsGeometryChanges;

// use to place an element
static void swPlaceElement(QGraphicsItem* Item, const swInstructions* Instructions, const QPointF& Position)
{
	swInstructionsElementKey Key;
	swGetElementKey(Item, Key);

	Item->setData(SW_PLACEMENT_POSITION_DATA, Position);

	if (Instructions->IsElementLocked(Key))
	{
		Item->setFlag(QGraphicsItem::ItemIsMovable, false);
		Item->setPos(Instructions->GetLockedPosition(Key));
	}
	else
		Item->setPos(Position + Instructions->GetElementOffset(Key));
}

// use to get pixels per page point
static qreal swGetRenderScale(const swInstructions* Instructions)
{
	const swInstructionsPageSetup& PageSetup = Instructions->mPageSetup;

	return qMax(1.0, SW_INSTRUCTIONS_RENDER_SIZE / qMax<qreal>(qMax(PageSetup.Width, PageSetup.Height), 1.0));
}

// use to crop an image to its pixels
static QImage swCropImage(const QImage& Image, QPoint& Offset)
{
	Offset = QPoint();

	const QImage Source = Image.convertToFormat(QImage::Format_ARGB32);
	int Left = Source.width(), Top = Source.height(), Right = -1, Bottom = -1;

	for (int Y = 0; Y < Source.height(); Y++)
	{
		const QRgb* Line = reinterpret_cast<const QRgb*>(Source.constScanLine(Y));

		for (int X = 0; X < Source.width(); X++)
		{
			if (!qAlpha(Line[X]))
				continue;

			Left = qMin(Left, X);
			Right = qMax(Right, X);
			Top = qMin(Top, Y);
			Bottom = qMax(Bottom, Y);
		}
	}

	if (Right < 0)
		return QImage();

	Offset = QPoint(Left, Top);

	return Source.copy(Left, Top, Right - Left + 1, Bottom - Top + 1);
}

// use to place an item on a grid cell
static QPointF swGetPlacementPosition(const QRectF& Anchor, const QSizeF& Size, int Placement)
{
	auto GetOffset = [](qreal Start, qreal Length, qreal ItemLength, int Cell)
	{
		switch (Cell)
		{
		case 0:
			return Start - ItemLength;

		case 1:
			return Start;

		case 2:
			return Start + (Length - ItemLength) / 2.0;

		case 3:
			return Start + Length - ItemLength;

		default:
			return Start + Length;
		}
	};

	const int Row = Placement / SW_PLACEMENT_GRID_SIZE;
	const int Column = Placement % SW_PLACEMENT_GRID_SIZE;

	return QPointF(GetOffset(Anchor.left(), Anchor.width(), Size.width(), Column), GetOffset(Anchor.top(), Anchor.height(), Size.height(), Row));
}

// use to place a page element, outer cells on the paper edge
static QPointF swGetPagePlacementPosition(const swInstructions* Instructions, const QRectF& MarginsRect, const QSizeF& Size, int Placement)
{
	const swInstructionsPageSetup& PageSetup = Instructions->mPageSetup;
	const int Last = SW_PLACEMENT_GRID_SIZE - 1;
	const int Row = qBound(0, Placement / SW_PLACEMENT_GRID_SIZE, Last);
	const int Column = qBound(0, Placement % SW_PLACEMENT_GRID_SIZE, Last);
	QPointF Position = swGetPlacementPosition(MarginsRect, Size, swGetValidPlacement(Placement, true));

	if (Column == 0)
		Position.setX(0.0);
	else if (Column == Last)
		Position.setX(PageSetup.Width - Size.width());

	if (Row == 0)
		Position.setY(0.0);
	else if (Row == Last)
		Position.setY(PageSetup.Height - Size.height());

	return Position;
}

swInstructionsStepImageItem::swInstructionsStepImageItem(QGraphicsItem* Parent, swInstructions* Instructions, swModel* Model, swStep Step)
	: swInstructionsBoxItem(Parent), mInstructions(Instructions), mModel(Model), mStep(Step)
{
	setFlags(SW_MOVABLE_ITEM_FLAGS);
	SetBoxStyle(mInstructions->GetBoxStyle(swInstructionsBoxElement::StepImage, mModel, mStep));
}

void swInstructionsStepImageItem::SetCoverImage()
{
	mCoverImage = true;
	SetBoxStyle(mInstructions->GetBoxStyle(swInstructionsBoxElement::CoverImage, mModel, mStep));
}

void swInstructionsStepImageItem::SetSubmodelPreview()
{
	mSubmodelPreview = true;
	SetBoxStyle(mInstructions->GetBoxStyle(swInstructionsBoxElement::SubmodelPreview, mModel, mStep));
}

void swInstructionsStepImageItem::SetFinalModel()
{
	mFinalModel = true;
	setFlags(QGraphicsItem::GraphicsItemFlags());
}

QVariant swInstructionsStepImageItem::itemChange(GraphicsItemChange Change, const QVariant& Value)
{
	if (Change == QGraphicsItem::ItemPositionChange)
		return swSnapItemPosition(this, mInstructions, Value);

	return QGraphicsPixmapItem::itemChange(Change, Value);
}

void swInstructionsStepImageItem::Update()
{
	swView View(swViewType::View, mModel);

	View.SetOffscreenContext();
	View.MakeCurrent();

	// step background behind the image
	const QColor BackgroundColor = mInstructions->GetColorProperty(swInstructionsPropertyType::StepBackgroundColor, mModel, mStep);
	View.SetBackgroundColor(SW_RGBA(BackgroundColor.red(), BackgroundColor.green(), BackgroundColor.blue(), 0));

	// render large and draw at page size
	const qreal Scale = swGetRenderScale(mInstructions);
	const int CellWidth = qRound(mWidth * Scale);
	const int CellHeight = qRound(mHeight * Scale);

	// model size scales the fitted model

	swInstructionsPropertyType SizeType = swInstructionsPropertyType::StepImageSize;

	if (mCoverImage)
		SizeType = swInstructionsPropertyType::CoverImageSize;
	else if (mSubmodelPreview)
		SizeType = swInstructionsPropertyType::SubmodelPreviewSize;

	const qreal ModelSize = mInstructions->GetIntProperty(SizeType, mModel, mStep) / 100.0;
	const int Width = qMax(qRound(CellWidth * ModelSize), 1);
	const int Height = qMax(qRound(CellHeight * ModelSize), 1);
	View.SetSize(Width, Height);
	View.SetLineWidthScale(Scale);

	// cover and previews show the finished model
	const bool Finished = mCoverImage || mSubmodelPreview || mFinalModel;
	const bool FadeSteps = !Finished && mInstructions->GetBoolProperty(swInstructionsPropertyType::FadePreviousSteps, mModel, mStep);
	const QColor FadeColor = mInstructions->GetColorProperty(swInstructionsPropertyType::FadeColor, mModel, mStep);
	View.SetFadeStepsOverride(FadeSteps, swRGBAFromQColor(FadeColor));

	const bool HighlightNewParts = !Finished && mInstructions->GetBoolProperty(swInstructionsPropertyType::HighlightNewParts, mModel, mStep);
	const QColor HighlightColor = mInstructions->GetColorProperty(swInstructionsPropertyType::HighlightColor, mModel, mStep);
	View.SetHighlightNewPartsOverride(HighlightNewParts, swRGBAFromQColor(HighlightColor), mInstructions->GetIntProperty(swInstructionsPropertyType::HighlightWidth, mModel, mStep));

	// work on a copy of the main camera
	View.SetCamera(View.GetCamera(), true);
	View.GetCamera()->SetProjection(static_cast<swCameraProjection>(mInstructions->GetIntProperty(swInstructionsPropertyType::ModelProjection, mModel, mStep)));

	// show the step rotation view
	View.ApplyStepView(mStep);

	// frame the complete model
	View.ZoomExtentsAtStep(mModel->GetLastStep(), swGeometryBoundsMode::IncludeFallback, false);

	std::vector<QImage> Images = View.GetStepImages(mStep, mStep);
	mPartRegions.clear();

	if (Images.empty())
	{
		setPixmap(QPixmap());
		return;
	}

	QPoint CropOffset;
	QImage Image = swCropImage(Images.front(), CropOffset);

	// larger models keep to their cell

	if (Image.width() > CellWidth || Image.height() > CellHeight)
	{
		const QPoint Trim(qMax((Image.width() - CellWidth) / 2, 0), qMax((Image.height() - CellHeight) / 2, 0));

		Image = Image.copy(QRect(Trim, QSize(qMin(Image.width(), CellWidth), qMin(Image.height(), CellHeight))));
		CropOffset += Trim;
	}

	Image.setDevicePixelRatio(Scale);
	setPixmap(QPixmap::fromImage(Image));

	// project the step parts for pointers
	View.SetSize(Width, Height);

	const swStep CurrentStep = mModel->GetCurrentStep();
	const qreal Inset = mBoxStyle.GetInset();
	const QPointF Origin(Inset - CropOffset.x() / Scale, Inset - CropOffset.y() / Scale);
	const std::vector<std::unique_ptr<swPart>>& Parts = mModel->GetParts();

	mModel->SetTemporaryStep(mStep);

	for (size_t PartIndex = 0; PartIndex < Parts.size(); PartIndex++)
	{
		const swPart* Part = Parts[PartIndex].get();

		if (Part->IsHidden() || !Part->IsVisible(mStep))
			continue;

		swVector3 Min(FLT_MAX, FLT_MAX, FLT_MAX);
		swVector3 Max(-FLT_MAX, -FLT_MAX, -FLT_MAX);
		Part->CompareBoundingBox(Min, Max);

		swVector3 Corners[8];
		swGetBoxCorners(Min, Max, Corners);

		QRectF Rect;

		for (const swVector3& Corner : Corners)
		{
			const swVector3 Point = View.ProjectPoint(Corner);
			const QPointF ImagePoint = Origin + QPointF(Point.x, Height - Point.y) / Scale;

			Rect = Rect.isNull() ? QRectF(ImagePoint, QSizeF(0.0, 0.0)) : Rect.united(QRectF(ImagePoint, QSizeF(0.0, 0.0)));
		}

		Rect = Rect.normalized();

		const swVector3 Center = View.ProjectPoint((Min + Max) * 0.5f);
		mPartRegions.push_back({ static_cast<int>(PartIndex), Rect, Origin + QPointF(Center.x, Height - Center.y) / Scale });
	}

	mModel->SetTemporaryStep(CurrentStep);

	if (!mModel->IsActive())
		mModel->CalculateStep(SW_STEP_MAXIMUM);
}

bool swInstructionsStepImageItem::FindPart(const QPointF& Point, int& PartIndex) const
{
	// the smallest part under the point wins
	const swPartRegion* Found = nullptr;
	qreal FoundArea = 0.0;

	for (const swPartRegion& Region : mPartRegions)
	{
		if (!Region.Rect.contains(Point))
			continue;

		const qreal Area = Region.Rect.width() * Region.Rect.height();

		if (!Found || Area < FoundArea)
		{
			Found = &Region;
			FoundArea = Area;
		}
	}

	if (!Found)
	{
		qreal FoundDistance = 24.0;

		for (const swPartRegion& Region : mPartRegions)
		{
			const qreal Distance = QLineF(Point, Region.Center).length();

			if (Distance < FoundDistance)
			{
				Found = &Region;
				FoundDistance = Distance;
			}
		}
	}

	if (!Found)
		return false;

	PartIndex = Found->PartIndex;

	return true;
}

bool swInstructionsStepImageItem::GetPartPoint(int PartIndex, QPointF& Point) const
{
	for (const swPartRegion& Region : mPartRegions)
	{
		if (Region.PartIndex != PartIndex)
			continue;

		Point = Region.Center;

		return true;
	}

	return false;
}

swInstructionsStepNumberItem::swInstructionsStepNumberItem(QGraphicsItem* Parent, swInstructions* Instructions, swModel* Model, swStep Step)
	: swInstructionsBoxItem(Parent), mInstructions(Instructions), mModel(Model), mStep(Step)
{
	setFlags(SW_MOVABLE_ITEM_FLAGS);
	SetBoxStyle(mInstructions->GetBoxStyle(swInstructionsBoxElement::StepNumber, mModel, mStep));
}

QVariant swInstructionsStepNumberItem::itemChange(GraphicsItemChange Change, const QVariant& Value)
{
	if (Change == QGraphicsItem::ItemPositionChange)
		return swSnapItemPosition(this, mInstructions, Value);

	return QGraphicsSimpleTextItem::itemChange(Change, Value);
}

void swInstructionsStepNumberItem::Update()
{
	bool Visible = mInstructions->GetBoolProperty(swInstructionsPropertyType::ShowStepNumber, mModel, mStep);
	setVisible(Visible);

	// step 0 is the bill of materials title
	setFont(mInstructions->GetFontProperty(mStep ? swInstructionsPropertyType::StepNumberFont : swInstructionsPropertyType::BillOfMaterialsTitleFont, mModel, mStep));
	setBrush(QBrush(mInstructions->GetColorProperty(mStep ? swInstructionsPropertyType::StepNumberColor : swInstructionsPropertyType::BillOfMaterialsTitleColor, mModel, mStep)));
	setText(mStep ? QString::number(mStep) : mInstructions->GetSourcedText(swInstructionsPropertyType::BillOfMaterialsTitleText, swInstructionsPropertyType::BillOfMaterialsTitleFile, QCoreApplication::translate("swInstructionsDialog", "Parts List")));
}

swInstructionsPageNumberItem::swInstructionsPageNumberItem(QGraphicsItem* Parent, swInstructions* Instructions, size_t PageIndex)
	: swInstructionsBoxItem(Parent), mInstructions(Instructions)
{
	setFlags(SW_MOVABLE_ITEM_FLAGS);
	setZValue(1.0);
	SetBoxStyle(mInstructions->GetBoxStyle(swInstructionsBoxElement::PageNumber, nullptr, 0));

	const QLocale Locale;
	const QString PageNumber = swFormatIntegerLocalized(static_cast<qlonglong>(PageIndex + 1), Locale);
	const swInstructionsPageNumberFormat Format = static_cast<swInstructionsPageNumberFormat>(mInstructions->GetIntProperty(swInstructionsPropertyType::PageNumberFormat, nullptr, 0));

	setFont(mInstructions->GetFontProperty(swInstructionsPropertyType::PageNumberFont, nullptr, 0));
	setBrush(QBrush(mInstructions->GetColorProperty(swInstructionsPropertyType::PageNumberColor, nullptr, 0)));

	if (Format == swInstructionsPageNumberFormat::NumberOfPages)
		setText(QCoreApplication::translate("swInstructionsDialog", "%1 / %2").arg(PageNumber, swFormatIntegerLocalized(static_cast<qlonglong>(mInstructions->mPages.size()), Locale)));
	else
		setText(PageNumber);
}

QVariant swInstructionsPageNumberItem::itemChange(GraphicsItemChange Change, const QVariant& Value)
{
	if (Change == QGraphicsItem::ItemPositionChange)
		return swSnapItemPosition(this, mInstructions, Value);

	return QGraphicsSimpleTextItem::itemChange(Change, Value);
}

swInstructionsPartCountItem::swInstructionsPartCountItem(QGraphicsItem* Parent, swInstructions* Instructions, swInstructionsPlacementElement Element, swModel* Model, int Count)
	: swInstructionsBoxItem(Parent), mInstructions(Instructions), mElement(Element), mModel(Model)
{
	setFlags(SW_MOVABLE_ITEM_FLAGS);
	setZValue(1.0);
	SetBoxStyle(mInstructions->GetBoxStyle(swInstructionsBoxElement::PartCount, mModel, 0));

	// steps use the page number look
	const bool PageCount = mElement == swInstructionsPlacementElement::PagePartCount || mElement == swInstructionsPlacementElement::SubmodelPreviewPartCount;

	QFont CountFont = mInstructions->GetFontProperty(PageCount ? swInstructionsPropertyType::PagePartCountFont : swInstructionsPropertyType::PartCountFont, mModel, 0);

	// bill of materials count matches the title size
	if (mElement == swInstructionsPlacementElement::BillOfMaterialsPartCount)
		CountFont.setPointSizeF(mInstructions->GetFontProperty(swInstructionsPropertyType::BillOfMaterialsTitleFont, mModel, 0).pointSizeF());

	setFont(CountFont);
	setBrush(QBrush(mInstructions->GetColorProperty(PageCount ? swInstructionsPropertyType::PagePartCountColor : swInstructionsPropertyType::PartCountColor, mModel, 0)));
	setText(mInstructions->GetPartCountText(Count));
}

QVariant swInstructionsPartCountItem::itemChange(GraphicsItemChange Change, const QVariant& Value)
{
	if (Change == QGraphicsItem::ItemPositionChange)
		return swSnapItemPosition(this, mInstructions, Value);

	return QGraphicsSimpleTextItem::itemChange(Change, Value);
}

swInstructionsPartsListItem::swInstructionsPartsListItem(QGraphicsItem* Parent, swInstructions* Instructions, swModel* Model, swStep Step)
	: swInstructionsBoxItem(Parent), mInstructions(Instructions), mModel(Model), mStep(Step)
{
	setFlags(SW_MOVABLE_ITEM_FLAGS);

	// parts image on the background color
	SetBoxStyle(mInstructions->GetBoxStyle(swInstructionsBoxElement::PartsList, mModel, mStep));
	SetContentBackground(true);
}

QVariant swInstructionsPartsListItem::itemChange(GraphicsItemChange Change, const QVariant& Value)
{
	if (Change == QGraphicsItem::ItemPositionChange)
		return swSnapItemPosition(this, mInstructions, Value);

	return QGraphicsPixmapItem::itemChange(Change, Value);
}

void swInstructionsPartsListItem::Update()
{
	// parts list settings for the materials page
	bool Visible = mBillOfMaterials || mInstructions->GetBoolProperty(swInstructionsPropertyType::ShowStepPLI, mModel, mStep);
	setVisible(Visible);

	if (!Visible)
		return;

	QColor BackgroundColor = mInstructions->GetColorProperty(swInstructionsPropertyType::PLIBackgroundColor, mModel, mStep);
	QFont Font = mInstructions->GetFontProperty(swInstructionsPropertyType::PLIFont, mModel, mStep);
	QColor TextColor = mInstructions->GetColorProperty(swInstructionsPropertyType::PLITextColor, mModel, mStep);
	const bool ShowCounts = mInstructions->GetBoolProperty(swInstructionsPropertyType::PLIShowCounts, mModel, mStep);
	const bool ShowPartIds = mBillOfMaterials && mInstructions->GetBoolProperty(swInstructionsPropertyType::BillOfMaterialsShowPartIds, mModel, mStep);
	const swPartsListSort Sort = static_cast<swPartsListSort>(mInstructions->GetIntProperty(swInstructionsPropertyType::PLISortOrder, mModel, mStep));
	const swPartsListLayout Layout = static_cast<swPartsListLayout>(mInstructions->GetIntProperty(swInstructionsPropertyType::PLILayout, mModel, mStep));
	const swPartsListAlignment Alignment = static_cast<swPartsListAlignment>(mInstructions->GetIntProperty(swInstructionsPropertyType::PLIAlignment, mModel, mStep));
	const int ThumbnailSize = mInstructions->GetIntProperty(swInstructionsPropertyType::PLIThumbnailSize, mModel, mStep);
	const QColor ThumbnailBackgroundColor = mInstructions->GetColorProperty(swInstructionsPropertyType::PLIThumbnailBackgroundColor, mModel, mStep);
	const int CountPlacement = mInstructions->GetIntProperty(swInstructionsPropertyType::PLICountPlacement, mModel, mStep);
	const float Scale = swGetRenderScale(mInstructions);
	std::vector<QRect> CountRects;

	const swResult<QImage> PartsImage = mModel->GetPartsListImage(mMaxWidth, ThumbnailSize, mBillOfMaterials ? 0 : mStep, swRGBAFromQColor(BackgroundColor), swRGBAFromQColor(ThumbnailBackgroundColor), Font, TextColor, false, ShowCounts, Sort, Layout, Alignment, mMaxHeight, Scale, ShowPartIds, CountPlacement, &CountRects);

	// quantities rebuild with the image
	for (QGraphicsItem* Child : childItems())
		if (dynamic_cast<swInstructionsPartsListCountItem*>(Child))
			delete Child;

	if (!PartsImage)
	{
		setPixmap(QPixmap());

		QWidget* Parent = scene() && !scene()->views().isEmpty() ? scene()->views().front() : nullptr;
		QMessageBox::warning(Parent, QObject::tr("Create Parts List"), PartsImage.error());

		return;
	}

	setPixmap(QPixmap::fromImage(PartsImage.value()));

	// quantities sit over their drawn text
	const qreal Inset = GetBoxStyle().GetInset();

	for (const QRect& CountRect : CountRects)
		new swInstructionsPartsListCountItem(this, QRectF(CountRect.x() / Scale + Inset, CountRect.y() / Scale + Inset, CountRect.width() / Scale, CountRect.height() / Scale), mModel, mBillOfMaterials ? 0 : mStep);
}

swInstructionsCoverTextItem::swInstructionsCoverTextItem(QGraphicsItem* Parent, swInstructions* Instructions, swModel* Model, bool Author)
	: swInstructionsBoxItem(Parent), mInstructions(Instructions), mModel(Model), mAuthor(Author)
{
	setFlags(SW_MOVABLE_ITEM_FLAGS);
	setZValue(1.0);
	SetBoxStyle(mInstructions->GetBoxStyle(Author ? swInstructionsBoxElement::CoverAuthor : swInstructionsBoxElement::CoverTitle, mModel, 0));
}

QVariant swInstructionsCoverTextItem::itemChange(GraphicsItemChange Change, const QVariant& Value)
{
	if (Change == QGraphicsItem::ItemPositionChange)
		return swSnapItemPosition(this, mInstructions, Value);

	return QGraphicsTextItem::itemChange(Change, Value);
}

// typed text, or a text file that wins over it
bool swEditNoticeText(QWidget* Parent, QString& Text, QString& File, bool NewNotice)
{
	QDialog Dialog(Parent);
	Dialog.setWindowTitle(NewNotice ? QCoreApplication::translate("swInstructionsDialog", "Add Notice") : QCoreApplication::translate("swInstructionsDialog", "Edit Notice"));

	QVBoxLayout* Layout = new QVBoxLayout(&Dialog);
	QLabel* TextLabel = new QLabel(QCoreApplication::translate("swInstructionsDialog", "Notice &Text"), &Dialog);
	QPlainTextEdit* TextEdit = new QPlainTextEdit(Text, &Dialog);
	TextLabel->setBuddy(TextEdit);
	Layout->addWidget(TextLabel);
	Layout->addWidget(TextEdit);

	QLabel* FileLabel = new QLabel(QCoreApplication::translate("swInstructionsDialog", "Text &File"), &Dialog);
	QHBoxLayout* FileLayout = new QHBoxLayout();
	QLineEdit* FileEdit = new QLineEdit(File, &Dialog);
	FileEdit->setToolTip(QCoreApplication::translate("swInstructionsDialog", "Text file used instead of the typed text, relative to the model file."));
	FileEdit->setWhatsThis(FileEdit->toolTip());
	FileLabel->setBuddy(FileEdit);

	QToolButton* BrowseButton = new QToolButton(&Dialog);
	BrowseButton->setAccessibleName(QCoreApplication::translate("swInstructionsDialog", "Choose Text File"));

	// a set path turns browse into reset

	auto UpdateBrowseButton = [BrowseButton](const QString& FileName)
	{
		const bool HasFile = !FileName.trimmed().isEmpty();

		BrowseButton->setText(HasFile ? QString() : QLatin1String("..."));
		BrowseButton->setIcon(HasFile ? BrowseButton->style()->standardIcon(QStyle::SP_LineEditClearButton) : QIcon());
		BrowseButton->setToolTip(HasFile ? QCoreApplication::translate("swInstructionsDialog", "Remove the text file and go back to the typed text.") : QCoreApplication::translate("swInstructionsDialog", "Choose the text file."));
	};

	UpdateBrowseButton(File);
	QObject::connect(FileEdit, &QLineEdit::textChanged, BrowseButton, UpdateBrowseButton);

	QObject::connect(BrowseButton, &QToolButton::clicked, &Dialog, [&Dialog, FileEdit]()
	{
		if (!FileEdit->text().trimmed().isEmpty())
		{
			FileEdit->clear();
			return;
		}

		const QString FileName = QFileDialog::getOpenFileName(&Dialog, QCoreApplication::translate("swInstructionsDialog", "Choose Text File"), FileEdit->text(), QCoreApplication::translate("swInstructionsDialog", "Text Files (*.txt);;All Files (*)"));

		if (!FileName.isEmpty())
			FileEdit->setText(FileName);
	});

	FileLayout->addWidget(FileEdit);
	FileLayout->addWidget(BrowseButton);
	Layout->addWidget(FileLabel);
	Layout->addLayout(FileLayout);

	QDialogButtonBox* ButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &Dialog);
	QObject::connect(ButtonBox, &QDialogButtonBox::accepted, &Dialog, &QDialog::accept);
	QObject::connect(ButtonBox, &QDialogButtonBox::rejected, &Dialog, &QDialog::reject);
	Layout->addWidget(ButtonBox);

	if (Dialog.exec() != QDialog::Accepted)
		return false;

	const QString NewText = TextEdit->toPlainText();
	const QString NewFile = FileEdit->text().trimmed();

	if (NewText.trimmed().isEmpty() && NewFile.isEmpty())
		return false;

	Text = NewText;
	File = NewFile;

	return true;
}

swInstructionsNoticeItem::swInstructionsNoticeItem(QGraphicsItem* Parent, swInstructions* Instructions, const swInstructionsNotice& Notice)
	: swInstructionsBoxItem(Parent), mInstructions(Instructions), mId(Notice.Id)
{
	setFlags(SW_MOVABLE_ITEM_FLAGS);
	setZValue(1.0);
	SetBoxStyle(mInstructions->GetBoxStyle(swInstructionsBoxElement::Notice, nullptr, 0));

	setFont(mInstructions->GetFontProperty(swInstructionsPropertyType::NoticeFont, nullptr, 0));
	setDefaultTextColor(mInstructions->GetColorProperty(swInstructionsPropertyType::NoticeTextColor, nullptr, 0));
	const QString FileText = mInstructions->GetFileText(Notice.File);
	setPlainText(FileText.isEmpty() ? Notice.Text : FileText);
}

QVariant swInstructionsNoticeItem::itemChange(GraphicsItemChange Change, const QVariant& Value)
{
	if (Change == QGraphicsItem::ItemPositionChange)
		return swSnapItemPosition(this, mInstructions, Value);

	return QGraphicsTextItem::itemChange(Change, Value);
}

void swInstructionsNoticeItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* Event)
{
	Event->accept();

	const swInstructionsNotice* Notice = mInstructions->FindNotice(mId);
	QWidget* Parent = scene() && !scene()->views().isEmpty() ? scene()->views().front() : nullptr;

	if (!Notice)
		return;

	// rebuild after the click
	const int Id = mId;
	swInstructions* Instructions = mInstructions;
	QString Text = Notice->Text;
	QString File = Notice->File;

	QTimer::singleShot(0, Instructions, [Instructions, Parent, Id, Text, File]() mutable
	{
		if (swEditNoticeText(Parent, Text, File, false))
			Instructions->SetNoticeText(Id, Text, File);
	});
}

QPointF swGetPointerAnchor(const QRectF& BorderRect, int Anchor)
{
	switch (Anchor)
	{
	case 0:
		return BorderRect.topLeft();

	case 1:
		return QPointF(BorderRect.center().x(), BorderRect.top());

	case 2:
		return BorderRect.topRight();

	case 3:
		return QPointF(BorderRect.right(), BorderRect.center().y());

	case 4:
		return BorderRect.bottomRight();

	case 5:
		return QPointF(BorderRect.center().x(), BorderRect.bottom());

	case 6:
		return BorderRect.bottomLeft();

	default:
		return QPointF(BorderRect.left(), BorderRect.center().y());
	}
}

// use to get an element border
static bool swGetBorderRect(const QGraphicsItem* Item, const QGraphicsItem* Parent, QRectF& BorderRect)
{
	const swInstructionsBoxBase* Box = dynamic_cast<const swInstructionsBoxBase*>(Item);

	if (!Box || !Item->isVisible())
		return false;

	BorderRect = Item->mapRectToItem(Parent, Box->GetBorderRect());

	return true;
}

swInstructionsPointerItem::swInstructionsPointerItem(QGraphicsItem* Parent, swInstructions* Instructions, const swInstructionsPointer& Pointer, QGraphicsItem* Source, QGraphicsItem* Target)
	: QGraphicsItem(Parent), mInstructions(Instructions), mPointer(Pointer), mSource(Source), mTarget(Target)
{
	setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
	setZValue(2.0);

	// pointers default to the pointer properties
	const int Style = mPointer.Style >= 0 ? mPointer.Style : mInstructions->GetIntProperty(swInstructionsPropertyType::PointerStyle, nullptr, 0);
	const int Tip = mPointer.Tip >= 0 ? mPointer.Tip : mInstructions->GetIntProperty(swInstructionsPropertyType::PointerTip, nullptr, 0);

	mStyle = static_cast<swInstructionsPointerStyle>(Style);
	mTip = static_cast<swInstructionsPointerTip>(Tip);
	mColor = mInstructions->GetColorProperty(swInstructionsPropertyType::PointerColor, nullptr, 0);

	// body follows the element border
	const swInstructionsBoxBase* Box = dynamic_cast<const swInstructionsBoxBase*>(mSource);
	const int Width = mInstructions->GetIntProperty(swInstructionsPropertyType::PointerWidth, nullptr, 0);

	mSourceBorderWidth = Box ? Box->GetBorderWidth() : 0.0;
	mWidth = Width > 0 ? Width : qMax(mSourceBorderWidth, 2.0);

	// the tip is at least three times the body
	mTipSize = qMax<qreal>(mInstructions->GetIntProperty(swInstructionsPropertyType::PointerTipSize, nullptr, 0), 3.0 * mWidth);

	UpdateGeometry();
}

void swInstructionsPointerItem::UpdateGeometry()
{
	QRectF SourceRect;

	if (!swGetBorderRect(mSource, parentItem(), SourceRect))
		return;

	QPointF TipPosition = mPointer.TipPosition;
	QRectF TargetRect;

	if (mDraggingTip)
		TipPosition = mTipPosition;
	else if (mPointer.TargetType == swInstructionsPointerTarget::Part && mTarget)
	{
		const swInstructionsStepImageItem* ImageItem = dynamic_cast<const swInstructionsStepImageItem*>(mTarget);
		QPointF PartPoint;

		if (ImageItem && ImageItem->GetPartPoint(mPointer.TargetPart, PartPoint))
			TipPosition = mTarget->mapToItem(parentItem(), PartPoint);
	}
	else if (mPointer.TargetType == swInstructionsPointerTarget::Element && swGetBorderRect(mTarget, parentItem(), TargetRect))
	{
		// tip touches the target border
		const QPointF Center = SourceRect.center();

		if (TargetRect.contains(Center))
			TipPosition = TargetRect.center();
		else
			TipPosition = QPointF(qBound(TargetRect.left(), Center.x(), TargetRect.right()), qBound(TargetRect.top(), Center.y(), TargetRect.bottom()));
	}

	// leave the middle of the border
	const qreal BorderHalf = mSourceBorderWidth / 2.0;
	const QRectF AnchorRect = SourceRect.adjusted(BorderHalf, BorderHalf, -BorderHalf, -BorderHalf);

	// automatic anchor nearest the tip
	int Anchor = mPointer.Anchor;

	if (Anchor < 0 || Anchor >= SW_POINTER_ANCHOR_COUNT)
	{
		qreal AnchorDistance = 0.0;

		for (int Index = 0; Index < SW_POINTER_ANCHOR_COUNT; Index++)
		{
			const qreal Distance = QLineF(swGetPointerAnchor(AnchorRect, Index), TipPosition).length();

			if (Index == 0 || Distance < AnchorDistance)
			{
				Anchor = Index;
				AnchorDistance = Distance;
			}
		}
	}

	prepareGeometryChange();
	mSourceRect = SourceRect;
	mAnchor = Anchor;
	mBasePosition = swGetPointerAnchor(AnchorRect, Anchor);
	mTipPosition = TipPosition;
	update();
}

QPainterPath swInstructionsPointerItem::GetPath(const QRectF& SourceRect, qreal BorderWidth) const
{
	QPainterPath Path;
	const QLineF Line(mBasePosition, mTipPosition);

	if (Line.length() < 0.5)
		return Path;

	const QPointF Direction = (mTipPosition - mBasePosition) / Line.length();
	const QPointF Normal(-Direction.y(), Direction.x());

	// wedge narrows to the tip
	if (mStyle == swInstructionsPointerStyle::Wedge)
		Path.addPath(GetWedgePath(SourceRect, BorderWidth));

	if (mTip == swInstructionsPointerTip::Dot)
		Path.addEllipse(mTipPosition, mTipSize / 2.0, mTipSize / 2.0);
	else if (mStyle == swInstructionsPointerStyle::Line)
	{
		const QPointF Back = mTipPosition - Direction * mTipSize;
		const qreal HalfWidth = mTipSize * 0.4;

		Path.moveTo(mTipPosition);
		Path.lineTo(Back + Normal * HalfWidth);
		Path.lineTo(Back - Normal * HalfWidth);
		Path.closeSubpath();
	}

	return Path;
}

// use to sit the wedge on the box edge
QPainterPath swInstructionsPointerItem::GetWedgePath(const QRectF& SourceRect, qreal BorderWidth) const
{
	const QRectF& Rect = SourceRect;
	const QPointF Corners[4] = { Rect.topLeft(), Rect.topRight(), Rect.bottomRight(), Rect.bottomLeft() };
	QPointF Start, End, Normal;

	if (mAnchor % 2 == 0)
	{
		// corner wedges run along the far side
		const QPointF Corner = Corners[mAnchor / 2];
		const qreal SignX = Corner.x() > Rect.center().x() ? 1.0 : -1.0;
		const qreal SignY = Corner.y() > Rect.center().y() ? 1.0 : -1.0;
		const bool Vertical = (mTipPosition.x() - Corner.x()) * SignX >= (mTipPosition.y() - Corner.y()) * SignY;
		const qreal Length = qMin(mTipSize, Vertical ? Rect.height() : Rect.width());

		Normal = Vertical ? QPointF(SignX, 0.0) : QPointF(0.0, SignY);
		Start = Corner;
		End = Corner - (Vertical ? QPointF(0.0, SignY) : QPointF(SignX, 0.0)) * Length;
	}
	else
	{
		// center side wedges
		const QPointF Middle = swGetPointerAnchor(Rect, mAnchor);
		const bool Vertical = mAnchor == 3 || mAnchor == 7;
		const qreal HalfLength = qMin(mTipSize, Vertical ? Rect.height() : Rect.width()) / 2.0;
		const QPointF Along = Vertical ? QPointF(0.0, HalfLength) : QPointF(HalfLength, 0.0);

		Normal = Vertical ? QPointF(mAnchor == 3 ? 1.0 : -1.0, 0.0) : QPointF(0.0, mAnchor == 1 ? -1.0 : 1.0);
		Start = Middle - Along;
		End = Middle + Along;
	}

	// base reaches through the border
	const QPointF Inset = Normal * BorderWidth;
	QPainterPath Path;

	Path.moveTo(Start);
	Path.lineTo(mTipPosition);
	Path.lineTo(End);
	Path.lineTo(End - Inset);
	Path.lineTo(Start - Inset);
	Path.closeSubpath();

	return Path;
}

// use to keep the pointer outside its box
QPainterPath swInstructionsPointerItem::GetClipPath(const QRectF& SourceRect, qreal BorderWidth) const
{
	const QRectF InnerRect = SourceRect.adjusted(BorderWidth, BorderWidth, -BorderWidth, -BorderWidth);
	QPainterPath Path;

	Path.addRect(boundingRect());

	if (InnerRect.isValid())
	{
		QPainterPath InnerPath;
		InnerPath.addRect(InnerRect);
		Path = Path.subtracted(InnerPath);
	}

	return Path;
}

QRectF swInstructionsPointerItem::boundingRect() const
{
	const qreal Extent = qMax(mTipSize, mWidth) + mSourceBorderWidth + mWidth + 8.0;

	return QRectF(mBasePosition, mTipPosition).normalized().adjusted(-Extent, -Extent, Extent, Extent);
}

QPainterPath swInstructionsPointerItem::shape() const
{
	QPainterPath Path;
	Path.moveTo(mBasePosition);
	Path.lineTo(mTipPosition);

	// easy to pick at any width
	QPainterPathStroker Stroker;
	Stroker.setWidth(qMax(mWidth, 8.0));

	QPainterPath Shape = Stroker.createStroke(Path);
	Shape.addPath(GetPath(mSourceRect, mSourceBorderWidth));
	Shape.addEllipse(mTipPosition, qMax(mTipSize, 10.0), qMax(mTipSize, 10.0));

	return Shape;
}

void swInstructionsPointerItem::paint(QPainter* Painter, const QStyleOptionGraphicsItem* Option, QWidget* Widget)
{
	Q_UNUSED(Widget);

	// meet the border on its pixels
	QRectF SourceRect = mSourceRect;
	qreal BorderWidth = mSourceBorderWidth;
	swSnapBorderRect(Painter->worldTransform(), SourceRect, BorderWidth);

	Painter->save();
	Painter->setRenderHint(QPainter::Antialiasing, true);
	Painter->setClipPath(GetClipPath(SourceRect, BorderWidth));

	const QLineF Line(mBasePosition, mTipPosition);

	if (mStyle == swInstructionsPointerStyle::Line && Line.length() >= 0.5)
	{
		// start the line inside the box
		const qreal BorderHalf = BorderWidth / 2.0;
		const QPointF Base = swGetPointerAnchor(SourceRect.adjusted(BorderHalf, BorderHalf, -BorderHalf, -BorderHalf), mAnchor);
		const QPointF Direction = (mTipPosition - Base) / QLineF(Base, mTipPosition).length();
		const qreal Width = mWidth == mSourceBorderWidth ? BorderWidth : mWidth;

		// stop the line at the arrow
		QPointF LineEnd = mTipPosition;

		if (mTip == swInstructionsPointerTip::Arrow && Line.length() > mTipSize)
			LineEnd = mTipPosition - Direction * (mTipSize * 0.5);

		QPen Pen(mColor, Width);
		Pen.setCapStyle(Qt::FlatCap);
		Painter->setPen(Pen);
		Painter->drawLine(Base - Direction * (BorderWidth + Width), LineEnd);
	}

	Painter->setPen(Qt::NoPen);
	Painter->setBrush(mColor);
	Painter->drawPath(GetPath(SourceRect, BorderWidth));
	Painter->setClipping(false);

	if (Option->state & QStyle::State_Selected)
	{
		// show anchor and tip handles
		QPen Pen(Option->palette.highlight(), 2.0);
		Pen.setCosmetic(true);

		Painter->setPen(Pen);
		Painter->setBrush(Option->palette.base());
		Painter->drawEllipse(mBasePosition, 4.0, 4.0);
		Painter->drawEllipse(mTipPosition, 5.0, 5.0);
	}

	Painter->restore();
}

void swInstructionsPointerItem::mousePressEvent(QGraphicsSceneMouseEvent* Event)
{
	QGraphicsItem::mousePressEvent(Event);

	// drag the tip to a new target
	if ((flags() & QGraphicsItem::ItemIsMovable) && Event->button() == Qt::LeftButton && QLineF(Event->pos(), mTipPosition).length() <= qMax(mTipSize, 10.0))
	{
		mDraggingTip = true;
		Event->accept();
	}
}

void swInstructionsPointerItem::mouseMoveEvent(QGraphicsSceneMouseEvent* Event)
{
	if (!mDraggingTip)
	{
		QGraphicsItem::mouseMoveEvent(Event);
		return;
	}

	mTipPosition = Event->pos();
	UpdateGeometry();
}

void swInstructionsPointerItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* Event)
{
	QGraphicsItem::mouseReleaseEvent(Event);

	if (!mDraggingTip)
		return;

	mDraggingTip = false;

	// point at the topmost target
	swInstructionsPointerTarget TargetType = swInstructionsPointerTarget::Point;
	swInstructionsElementKey TargetElement = { swInstructionsPlacementElement::None, nullptr, 0 };
	int TargetPart = -1;
	const QPointF TipPosition = mapToItem(parentItem(), Event->pos());
	const QList<QGraphicsItem*> Items = scene()->items(Event->scenePos());

	for (const QGraphicsItem* Item : Items)
	{
		swInstructionsElementKey Key;

		if (Item == mSource || dynamic_cast<const swInstructionsPointerItem*>(Item) || !swGetElementKey(Item, Key))
			continue;

		if (const swInstructionsStepImageItem* ImageItem = dynamic_cast<const swInstructionsStepImageItem*>(Item))
		{
			if (ImageItem->FindPart(ImageItem->mapFromScene(Event->scenePos()), TargetPart))
			{
				TargetType = swInstructionsPointerTarget::Part;
				TargetElement = Key;
				break;
			}
		}

		TargetType = swInstructionsPointerTarget::Element;
		TargetElement = Key;
		break;
	}

	// rebuild after the release
	swInstructions* Instructions = mInstructions;
	const int Id = mPointer.Id;

	QTimer::singleShot(0, Instructions, [Instructions, Id, TargetType, TargetElement, TargetPart, TipPosition]()
	{
		Instructions->SetPointerTarget(Id, TargetType, TargetElement, TargetPart, TipPosition);
	});
}

swInstructionsPageWidget::swInstructionsPageWidget(QWidget* Parent, swInstructions* Instructions, swInstructionsPropertiesWidget* PropertiesWidget, swInstructionsPlacementWidget* PlacementWidget)
	: QGraphicsView(Parent), mInstructions(Instructions), mPropertiesWidget(PropertiesWidget), mPlacementWidget(PlacementWidget)
{
	QGraphicsScene* Scene = new QGraphicsScene();
	setScene(Scene);
	setBackgroundBrush(palette().color(QPalette::Mid));

	// smooth lines and round ends
	setRenderHint(QPainter::Antialiasing, true);

	connect(mInstructions, &swInstructions::StepSettingsChanged, this, &swInstructionsPageWidget::StepSettingsChanged);
	connect(Scene, &QGraphicsScene::selectionChanged, this, &swInstructionsPageWidget::SelectionChanged);
	connect(mPlacementWidget, &swInstructionsPlacementWidget::AlignSelection, this, &swInstructionsPageWidget::AlignSelection);

	// the page always fits, scroll bars would toggle while it refits
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	// rulers follow the pointer
	viewport()->setMouseTracking(true);

	mHorizontalRuler = new swInstructionsRulerWidget(this, Qt::Horizontal);
	mVerticalRuler = new swInstructionsRulerWidget(this, Qt::Vertical);

	mRulerCorner = new QLabel(this);
	mRulerCorner->setAlignment(Qt::AlignCenter);
	mRulerCorner->setAutoFillBackground(true);
	mRulerCorner->setToolTip(tr("Units of the rulers, the page units or the document units when they are in use."));

	connect(mInstructions, &swInstructions::GridChanged, this, [this]()
	{
		UpdateRulers();
		viewport()->update();
	});

	connect(mInstructions, &swInstructions::UnitsChanged, this, &swInstructionsPageWidget::UpdateRulers);
	connect(mInstructions, &swInstructions::PageSetupChanged, this, &swInstructionsPageWidget::UpdateRulers);

	UpdateRulers();
}

void swInstructionsPageWidget::StepSettingsChanged(swModel* Model, swStep Step)
{
	QGraphicsScene* Scene = scene();

	// frozen pages keep their look
	const size_t PageIndex = mPage ? static_cast<size_t>(mPage - mInstructions->mPages.data()) : mInstructions->mPages.size();
	const swInstructionsPageScope PageScope(mInstructions, PageIndex);

	const QList<QGraphicsItem*> Items = Scene->items();

	for (QGraphicsItem* Item : Items)
	{
		swInstructionsStepImageItem* ImageItem = dynamic_cast<swInstructionsStepImageItem*>(Item);

		if (ImageItem)
		{
			if (!Model || (ImageItem->GetModel() == Model && ImageItem->GetStep() == Step))
				ImageItem->Update();

			continue;
		}

		swInstructionsStepNumberItem* NumberItem = dynamic_cast<swInstructionsStepNumberItem*>(Item);

		if (NumberItem)
		{
			if (!Model || (NumberItem->GetModel() == Model && NumberItem->GetStep() == Step))
				NumberItem->Update();

			continue;
		}

		swInstructionsPartsListItem* PartsItem = dynamic_cast<swInstructionsPartsListItem*>(Item);

		if (PartsItem)
		{
			if (!Model || (PartsItem->GetModel() == Model && PartsItem->GetStep() == Step))
				PartsItem->Update();

			continue;
		}
	}
}

void swInstructionsPageWidget::SelectionChanged()
{
	// reselect elements after a rebuild
	if (mRebuildingPage)
		return;

	QGraphicsItem* Focus = nullptr;
	const QList<QGraphicsItem*> SelectedItems = scene()->selectedItems();
	std::vector<swInstructionsElementKey> SelectedKeys;

	if (!SelectedItems.isEmpty())
		Focus = SelectedItems.first();

	for (const QGraphicsItem* Item : SelectedItems)
	{
		swInstructionsElementKey Key;

		if (swGetElementKey(Item, Key) && std::get<0>(Key) != swInstructionsPlacementElement::Pointer)
			SelectedKeys.push_back(Key);
	}

	mSelectionOrder.erase(std::remove_if(mSelectionOrder.begin(), mSelectionOrder.end(), [&SelectedKeys](const swInstructionsElementKey& Key)
	{
		return std::find(SelectedKeys.begin(), SelectedKeys.end(), Key) == SelectedKeys.end();
	}), mSelectionOrder.end());

	for (const swInstructionsElementKey& Key : SelectedKeys)
		if (std::find(mSelectionOrder.begin(), mSelectionOrder.end(), Key) == mSelectionOrder.end())
			mSelectionOrder.push_back(Key);

	mPropertiesWidget->SelectionChanged(Focus);
	mPlacementWidget->SelectionChanged(Focus, static_cast<int>(mSelectionOrder.size()));
}

// use to align the selected elements
void swInstructionsPageWidget::AlignSelection(int Placement, bool LastSelection)
{
	if (mSelectionOrder.size() < 2 || !mPage)
		return;

	const swInstructionsPageScope PageScope(mInstructions, static_cast<size_t>(mPage - mInstructions->mPages.data()));
	std::map<swInstructionsElementKey, QGraphicsItem*> ElementItems;
	const QList<QGraphicsItem*> SelectedItems = scene()->selectedItems();

	for (QGraphicsItem* Item : SelectedItems)
	{
		swInstructionsElementKey Key;

		if (swGetElementKey(Item, Key))
			ElementItems[Key] = Item;
	}

	const swInstructionsElementKey& ReferenceKey = LastSelection ? mSelectionOrder.back() : mSelectionOrder.front();
	const auto ReferenceIt = ElementItems.find(ReferenceKey);

	if (ReferenceIt == ElementItems.end())
		return;

	const QRectF ReferenceRect = ReferenceIt->second->sceneBoundingRect();
	std::map<swInstructionsElementKey, QPointF> Offsets;

	// locked elements keep their position
	for (const swInstructionsElementKey& Key : mSelectionOrder)
	{
		const auto ItemIt = ElementItems.find(Key);

		if (Key == ReferenceKey || ItemIt == ElementItems.end() || mInstructions->IsElementLocked(Key))
			continue;

		const QRectF ItemRect = ItemIt->second->sceneBoundingRect();
		const QPointF Distance = swGetPlacementPosition(ReferenceRect, ItemRect.size(), Placement) - ItemRect.topLeft();

		if (!Distance.isNull())
			Offsets[Key] = mInstructions->GetElementOffset(Key) + Distance;
	}

	mInstructions->SetElementOffsets(Offsets);
}

// use to add the page elements
static void swAddPageContent(swInstructions* Instructions, const swInstructionsPage* Page, QGraphicsScene* Scene, QGraphicsRectItem* PageItem, const QRectF& MarginsRect)
{
	if (Page->Type == swInstructionsPageType::Cover)
	{
		swModel* Model = Page->Model;
		const swModelProperties& Properties = Model->GetProperties();
		QString Title = Properties.mDescription.trimmed();

		if (Title.isEmpty())
			Title = Properties.mModelName.trimmed();

		if (Title.isEmpty())
			Title = QFileInfo(Properties.mFileName).completeBaseName();

		Title = Instructions->GetSourcedText(swInstructionsPropertyType::CoverTitleText, swInstructionsPropertyType::CoverTitleFile, Title);

		const QFont TitleFont = Instructions->GetFontProperty(swInstructionsPropertyType::CoverTitleFont, Model, 0);
		const QColor TextColor = Instructions->GetColorProperty(swInstructionsPropertyType::CoverTitleColor, Model, 0);
		const QColor AuthorColor = Instructions->GetColorProperty(swInstructionsPropertyType::CoverAuthorColor, Model, 0);

		struct swCoverText
		{
			swInstructionsCoverTextItem* Item;
			int Placement;
			QSizeF Size;
		};

		std::vector<swCoverText> CoverTexts;

		auto AddCoverText = [&](const QString& Text, const QFont& Font, bool Author, swInstructionsPropertyType PlacementType)
		{
			const int Placement = qBound(0, Instructions->GetIntProperty(PlacementType, Model, 0), SW_PLACEMENT_COUNT - 1);
			const int Column = Placement % SW_PLACEMENT_GRID_SIZE;

			// align wrapped text lines
			QTextOption TextOption;

			if (Column <= 1)
				TextOption.setAlignment(Qt::AlignLeft);
			else if (Column >= 3)
				TextOption.setAlignment(Qt::AlignRight);
			else
				TextOption.setAlignment(Qt::AlignHCenter);

			swInstructionsCoverTextItem* Item = new swInstructionsCoverTextItem(PageItem, Instructions, Model, Author);
			Item->setFont(Font);
			Item->setDefaultTextColor(Author ? AuthorColor : TextColor);
			Item->setPlainText(Text);
			Item->document()->setDefaultTextOption(TextOption);

			const qreal TextWidth = MarginsRect.width() - 2.0 * Item->GetBoxStyle().GetInset();

			if (Item->document()->idealWidth() > TextWidth)
				Item->setTextWidth(TextWidth);

			CoverTexts.push_back({ Item, Placement, Item->boundingRect().size() });
		};

		AddCoverText(Title, TitleFont, false, swInstructionsPropertyType::CoverTitlePlacement);

		const QString Author = Instructions->GetSourcedText(swInstructionsPropertyType::CoverAuthorText, swInstructionsPropertyType::CoverAuthorFile, Properties.mAuthor.trimmed());

		if (!Author.isEmpty() && Instructions->GetBoolProperty(swInstructionsPropertyType::ShowCoverAuthor, Model, 0))
		{
			AddCoverText(Author, Instructions->GetFontProperty(swInstructionsPropertyType::CoverAuthorFont, Model, 0), true, swInstructionsPropertyType::CoverAuthorPlacement);
		}

		// stack texts sharing a row
		qreal TopHeight = 0.0;
		qreal BottomHeight = 0.0;

		for (int Row = 0; Row < SW_PLACEMENT_GRID_SIZE; Row++)
		{
			qreal RowHeight = 0.0;

			for (const swCoverText& CoverText : CoverTexts)
				if (CoverText.Placement / SW_PLACEMENT_GRID_SIZE == Row)
					RowHeight += CoverText.Size.height();

			const qreal RowTop = swGetPagePlacementPosition(Instructions, MarginsRect, QSizeF(MarginsRect.width(), RowHeight), Row * SW_PLACEMENT_GRID_SIZE + 2).y();
			qreal TextTop = RowTop;

			for (const swCoverText& CoverText : CoverTexts)
			{
				if (CoverText.Placement / SW_PLACEMENT_GRID_SIZE != Row)
					continue;

				const QPointF Position = swGetPagePlacementPosition(Instructions, MarginsRect, CoverText.Size, CoverText.Placement);
				swPlaceElement(CoverText.Item, Instructions, QPointF(Position.x(), TextTop));
				TextTop += CoverText.Size.height();
			}

			if (Row == 1)
				TopHeight = RowHeight;
			else if (Row == SW_PLACEMENT_GRID_SIZE - 2)
				BottomHeight = RowHeight;
		}

		// place the finished model on the cover
		const QRectF ImageRect = MarginsRect.adjusted(0.0, TopHeight, 0.0, -BottomHeight);
		const swStep LastStep = Model->GetLastStep();

		const qreal ImageInset = Instructions->GetBoxStyle(swInstructionsBoxElement::CoverImage, Model, LastStep).GetInset();

		if (ImageRect.height() > 2.0 * ImageInset + 1.0)
		{
			Scene->addRect(ImageRect, QPen(Qt::NoPen), QBrush(Instructions->GetColorProperty(swInstructionsPropertyType::StepBackgroundColor, Model, LastStep)))->setParentItem(PageItem);

			swInstructionsStepImageItem* ModelImageItem = new swInstructionsStepImageItem(PageItem, Instructions, Model, LastStep);
			ModelImageItem->SetCoverImage();
			ModelImageItem->SetImageSize(qMax(static_cast<int>(ImageRect.width() - 2.0 * ImageInset), 1), static_cast<int>(ImageRect.height() - 2.0 * ImageInset));
			ModelImageItem->Update();

			const int Placement = swGetValidPlacement(Instructions->GetIntProperty(swInstructionsPropertyType::CoverImagePlacement, Model, LastStep), true);
			swPlaceElement(ModelImageItem, Instructions, swGetPlacementPosition(ImageRect, ModelImageItem->boundingRect().size(), Placement));
		}

		if (Instructions->GetBoolProperty(swInstructionsPropertyType::ShowCoverPartCount, Model, 0))
		{
			swInstructionsPartCountItem* CountItem = new swInstructionsPartCountItem(PageItem, Instructions, swInstructionsPlacementElement::CoverPartCount, Model, swInstructions::GetModelPartCount(Model));

			const int CountPlacement = Instructions->GetIntProperty(swInstructionsPropertyType::PartCountPlacement, Model, 0);
			swPlaceElement(CountItem, Instructions, swGetPagePlacementPosition(Instructions, MarginsRect, CountItem->boundingRect().size(), CountPlacement));
		}

		return;
	}

	if (Page->Type == swInstructionsPageType::BillOfMaterials)
	{
		swInstructionsStepNumberItem* TitleItem = new swInstructionsStepNumberItem(PageItem, Instructions, Page->Model, 0);
		TitleItem->Update();

		// the list fills the page above or below the title
		QRectF PartsRect = MarginsRect;
		QRectF TitleRect;

		if (TitleItem->isVisible())
		{
			const QSizeF TitleSize = TitleItem->boundingRect().size();
			const int TitlePlacement = swGetValidPlacement(Instructions->GetIntProperty(swInstructionsPropertyType::StepNumberPlacement, Page->Model, 0), true);
			const QPointF TitlePosition = swGetPlacementPosition(MarginsRect, TitleSize, TitlePlacement);
			swPlaceElement(TitleItem, Instructions, TitlePosition);
			TitleRect = QRectF(TitlePosition, TitleSize);

			if (TitlePosition.y() + TitleSize.height() / 2.0 < MarginsRect.center().y())
				PartsRect.setTop(TitlePosition.y() + TitleSize.height());
			else
				PartsRect.setBottom(TitlePosition.y());
		}

		// the part count goes beside or past the title
		if (Instructions->GetBoolProperty(swInstructionsPropertyType::ShowBillOfMaterialsPartCount, Page->Model, 0))
		{
			swInstructionsPartCountItem* CountItem = new swInstructionsPartCountItem(PageItem, Instructions, swInstructionsPlacementElement::BillOfMaterialsPartCount, Page->Model, swInstructions::GetModelPartCount(Page->Model));
			const QSizeF CountSize = CountItem->boundingRect().size();

			const int CountPlacement = Instructions->GetIntProperty(swInstructionsPropertyType::BillOfMaterialsPartCountPlacement, Page->Model, 0);
			QPointF CountPosition = swGetPagePlacementPosition(Instructions, MarginsRect, CountSize, CountPlacement);

			// beside the title when it fits, else past it
			if (QRectF(CountPosition, CountSize).intersects(TitleRect))
				CountPosition = swGetPlacementPosition(PartsRect, CountSize, swGetValidPlacement(CountPlacement, true));

			swPlaceElement(CountItem, Instructions, CountPosition);

			if (CountPosition.y() + CountSize.height() / 2.0 < PartsRect.center().y())
				PartsRect.setTop(qMax(PartsRect.top(), CountPosition.y() + CountSize.height()));
			else
				PartsRect.setBottom(qMin(PartsRect.bottom(), CountPosition.y()));
		}

		swInstructionsPartsListItem* PartsImageItem = new swInstructionsPartsListItem(PageItem, Instructions, Page->Model, 0);
		const qreal Inset = 2.0 * PartsImageItem->GetBoxStyle().GetInset();
		PartsImageItem->SetBillOfMaterials(qMax(static_cast<int>(PartsRect.width() - Inset), 16), qMax(static_cast<int>(PartsRect.height() - Inset), 16));
		PartsImageItem->Update();

		const int Placement = swGetValidPlacement(Instructions->GetIntProperty(swInstructionsPropertyType::BillOfMaterialsPlacement, Page->Model, 0), true);
		swPlaceElement(PartsImageItem, Instructions, swGetPlacementPosition(PartsRect, PartsImageItem->boundingRect().size(), Placement));

		return;
	}

	// draw dividers between steps
	qreal DividerInset = 0.0;

	if (!Page->Steps.empty())
	{
		const swInstructionsStep& FirstStep = Page->Steps.front();

		if (Instructions->GetBoolProperty(swInstructionsPropertyType::ShowStepDividers, FirstStep.Model, FirstStep.Step))
		{
			const int DividerWidth = Instructions->GetIntProperty(swInstructionsPropertyType::StepDividerWidth, FirstStep.Model, FirstStep.Step);
			const bool Rounded = Instructions->GetBoolProperty(swInstructionsPropertyType::StepDividerRounded, FirstStep.Model, FirstStep.Step);
			const QBrush DividerBrush(Instructions->GetColorProperty(swInstructionsPropertyType::StepDividerColor, FirstStep.Model, FirstStep.Step));

			// filled shapes keep thin round ends smooth
			auto AddDivider = [&](qreal X1, qreal Y1, qreal X2, qreal Y2)
			{
				const qreal HalfWidth = DividerWidth / 2.0;
				const qreal Extend = Rounded ? HalfWidth : 0.0;
				const QRectF Rect = X1 == X2 ? QRectF(QPointF(X1 - HalfWidth, Y1 - Extend), QPointF(X2 + HalfWidth, Y2 + Extend)) : QRectF(QPointF(X1 - Extend, Y1 - HalfWidth), QPointF(X2 + Extend, Y2 + HalfWidth));

				QPainterPath Path;
				Path.addRoundedRect(Rect, Rounded ? HalfWidth : 0.0, Rounded ? HalfWidth : 0.0);
				Scene->addPath(Path, Qt::NoPen, DividerBrush)->setParentItem(PageItem);
			};

			// round ends stay inside the margins

			const qreal Inset = Instructions->GetIntProperty(swInstructionsPropertyType::StepDividerMargin, FirstStep.Model, FirstStep.Step) + (Rounded ? DividerWidth / 2.0 : 0.0);

			DividerInset = DividerWidth / 2.0 + Instructions->GetIntProperty(swInstructionsPropertyType::StepDividerPadding, FirstStep.Model, FirstStep.Step);

			const bool Horizontal = Instructions->GetBoolProperty(swInstructionsPropertyType::StepDividerHorizontal, FirstStep.Model, FirstStep.Step);
			const bool Vertical = Instructions->GetBoolProperty(swInstructionsPropertyType::StepDividerVertical, FirstStep.Model, FirstStep.Step);

			auto IsNear = [](qreal Value1, qreal Value2)
			{
				return qAbs(Value1 - Value2) < 0.0001;
			};

			for (const swInstructionsStep& Step : Page->Steps)
			{
				const QRectF& Rect = Step.Rect;

				for (const swInstructionsStep& Other : Page->Steps)
				{
					const QRectF& OtherRect = Other.Rect;

					// draw along the shared part of touching edges
					const qreal Top = qMax(Rect.top(), OtherRect.top());
					const qreal Bottom = qMin(Rect.bottom(), OtherRect.bottom());
					const qreal Left = qMax(Rect.left(), OtherRect.left());
					const qreal Right = qMin(Rect.right(), OtherRect.right());

					if (Vertical && IsNear(OtherRect.left(), Rect.right()) && Bottom - Top > 0.0001)
					{
						const qreal X = MarginsRect.left() + MarginsRect.width() * Rect.right();
						AddDivider(X, MarginsRect.top() + MarginsRect.height() * Top + Inset, X, MarginsRect.top() + MarginsRect.height() * Bottom - Inset);
					}

					if (Horizontal && IsNear(OtherRect.top(), Rect.bottom()) && Right - Left > 0.0001)
					{
						const qreal Y = MarginsRect.top() + MarginsRect.height() * Rect.bottom();
						AddDivider(MarginsRect.left() + MarginsRect.width() * Left + Inset, Y, MarginsRect.left() + MarginsRect.width() * Right - Inset, Y);
					}
				}
			}
		}
	}

	for (const swInstructionsStep& Step : Page->Steps)
	{
		const QRectF CellRect(MarginsRect.left() + MarginsRect.width() * Step.Rect.x(), MarginsRect.top() + MarginsRect.height() * Step.Rect.y(), MarginsRect.width() * Step.Rect.width(), MarginsRect.height() * Step.Rect.height());

		// the finished model sits on the page alone

		if (Step.FinalModel)
		{
			// margin and border shrink the area

			const qreal FinalMargin = Instructions->GetIntProperty(swInstructionsPropertyType::FinalModelMargin, Step.Model, Step.Step);
			const qreal FinalBorder = Instructions->GetIntProperty(swInstructionsPropertyType::FinalModelBorderWidth, Step.Model, Step.Step);
			const QRectF FinalRect = CellRect.adjusted(FinalMargin, FinalMargin, -FinalMargin, -FinalMargin);

			// shortcut: uses the default page color, not a parent page override

			if (Page->Settings.FinalModelColor)
			{
				QGraphicsRectItem* FinalAreaItem = new QGraphicsRectItem(FinalRect, PageItem);
				FinalAreaItem->setPen(QPen(Qt::NoPen));
				FinalAreaItem->setBrush(QBrush(Instructions->GetColorProperty(swInstructionsPropertyType::PageBackgroundColor, nullptr, 0)));
				FinalAreaItem->setZValue(-1.0);
			}

			if (FinalBorder > 0.0)
			{
				const qreal Half = FinalBorder / 2.0;

				QGraphicsRectItem* FinalBorderItem = new QGraphicsRectItem(FinalRect.adjusted(Half, Half, -Half, -Half), PageItem);
				FinalBorderItem->setPen(QPen(Instructions->GetColorProperty(swInstructionsPropertyType::FinalModelBorderColor, Step.Model, Step.Step), FinalBorder, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
				FinalBorderItem->setBrush(Qt::NoBrush);
			}

			swInstructionsStepImageItem* FinalImageItem = new swInstructionsStepImageItem(PageItem, Instructions, Step.Model, Step.Step);
			const qreal FinalInset = FinalImageItem->GetBoxStyle().GetInset() + FinalBorder;

			FinalImageItem->SetFinalModel();
			FinalImageItem->SetImageSize(qMax(static_cast<int>(FinalRect.width() - 2.0 * FinalInset), 1), qMax(static_cast<int>(FinalRect.height() - 2.0 * FinalInset), 1));
			FinalImageItem->Update();
			FinalImageItem->setPos(FinalRect.center() - FinalImageItem->boundingRect().center());

			continue;
		}

		// the step area holds its elements
		QGraphicsRectItem* StepAreaItem = new QGraphicsRectItem(QRectF(QPointF(0.0, 0.0), CellRect.size()), PageItem);
		StepAreaItem->setPen(QPen(Qt::NoPen));
		StepAreaItem->setBrush(QBrush(Instructions->GetColorProperty(swInstructionsPropertyType::StepBackgroundColor, Step.Model, Step.Step)));
		StepAreaItem->setPos(CellRect.topLeft());

		swInstructionsStepImageItem* StepImageItem = new swInstructionsStepImageItem(StepAreaItem, Instructions, Step.Model, Step.Step);
		const qreal ImageInset = StepImageItem->GetBoxStyle().GetInset();
		StepImageItem->SetImageSize(qMax(static_cast<int>(CellRect.width() - 2.0 * ImageInset), 1), qMax(static_cast<int>(CellRect.height() - 2.0 * ImageInset), 1));
		StepImageItem->Update();

		swInstructionsStepNumberItem* StepNumberItem = new swInstructionsStepNumberItem(StepAreaItem, Instructions, Step.Model, Step.Step);
		StepNumberItem->Update();

		// place step elements in the split area
		QRectF AreaRect(QPointF(0.0, 0.0), CellRect.size());
		const qreal InsetLeft = Step.Rect.left() > 0.0001 ? DividerInset : 0.0;
		const qreal InsetTop = Step.Rect.top() > 0.0001 ? DividerInset : 0.0;
		const qreal InsetRight = Step.Rect.right() < 0.9999 ? DividerInset : 0.0;
		const qreal InsetBottom = Step.Rect.bottom() < 0.9999 ? DividerInset : 0.0;
		AreaRect.adjust(InsetLeft, InsetTop, -InsetRight, -InsetBottom);

		// wrap the parts list at its area
		swInstructionsPartsListItem* PartsImageItem = new swInstructionsPartsListItem(StepAreaItem, Instructions, Step.Model, Step.Step);
		const qreal Inset = 2.0 * PartsImageItem->GetBoxStyle().GetInset();
		PartsImageItem->SetMaxSize(qMax(static_cast<int>(AreaRect.width() - Inset), 16), qMax(static_cast<int>(AreaRect.height() - Inset), 16));
		PartsImageItem->Update();

		swInstructionsPlacementAnchor NumberAnchor = static_cast<swInstructionsPlacementAnchor>(Instructions->GetIntProperty(swInstructionsPropertyType::StepNumberAnchor, Step.Model, Step.Step));
		swInstructionsPlacementAnchor PartsAnchor = static_cast<swInstructionsPlacementAnchor>(Instructions->GetIntProperty(swInstructionsPropertyType::PLIAnchor, Step.Model, Step.Step));

		// fall back from hidden anchors
		if (NumberAnchor != swInstructionsPlacementAnchor::PartsList || !PartsImageItem->isVisible() || PartsAnchor == swInstructionsPlacementAnchor::StepNumber)
			NumberAnchor = swInstructionsPlacementAnchor::Area;

		if (PartsAnchor != swInstructionsPlacementAnchor::StepNumber || !StepNumberItem->isVisible())
			PartsAnchor = swInstructionsPlacementAnchor::Area;

		const int NumberPlacement = swGetValidPlacement(Instructions->GetIntProperty(swInstructionsPropertyType::StepNumberPlacement, Step.Model, Step.Step), NumberAnchor == swInstructionsPlacementAnchor::Area);
		const int PartsPlacement = swGetValidPlacement(Instructions->GetIntProperty(swInstructionsPropertyType::PLIPlacement, Step.Model, Step.Step), PartsAnchor == swInstructionsPlacementAnchor::Area);
		const QSizeF NumberSize = StepNumberItem->boundingRect().size();
		const QSizeF PartsSize = PartsImageItem->boundingRect().size();

		const int ImagePlacement = swGetValidPlacement(Instructions->GetIntProperty(swInstructionsPropertyType::StepImagePlacement, Step.Model, Step.Step), true);
		swPlaceElement(StepImageItem, Instructions, swGetPlacementPosition(AreaRect, StepImageItem->boundingRect().size(), ImagePlacement));

		// place moved and locked elements first
		if (NumberAnchor == swInstructionsPlacementAnchor::Area)
			swPlaceElement(StepNumberItem, Instructions, swGetPlacementPosition(AreaRect, NumberSize, NumberPlacement));

		if (PartsAnchor == swInstructionsPlacementAnchor::Area)
			swPlaceElement(PartsImageItem, Instructions, swGetPlacementPosition(AreaRect, PartsSize, PartsPlacement));

		if (NumberAnchor == swInstructionsPlacementAnchor::PartsList)
			swPlaceElement(StepNumberItem, Instructions, swGetPlacementPosition(QRectF(PartsImageItem->pos(), PartsSize), NumberSize, NumberPlacement));

		if (PartsAnchor == swInstructionsPlacementAnchor::StepNumber)
			swPlaceElement(PartsImageItem, Instructions, swGetPlacementPosition(QRectF(StepNumberItem->pos(), NumberSize), PartsSize, PartsPlacement));
	}
}

// use to add notices and pointers
static void swAddPageAnnotations(swInstructions* Instructions, size_t PageIndex, QGraphicsScene* Scene, QGraphicsRectItem* PageItem, const QRectF& MarginsRect)
{
	const swInstructionsPageKey PageKey = Instructions->GetPageElementsKey(PageIndex);

	for (const swInstructionsNotice& Notice : Instructions->GetNotices())
	{
		if (Notice.PageKey != PageKey)
			continue;

		swInstructionsNoticeItem* NoticeItem = new swInstructionsNoticeItem(PageItem, Instructions, Notice);
		const qreal TextWidth = MarginsRect.width() / 2.0;

		// wrap long notices at half width
		if (NoticeItem->document()->idealWidth() > TextWidth)
			NoticeItem->setTextWidth(TextWidth);

		swPlaceElement(NoticeItem, Instructions, Notice.Position);
	}

	std::map<swInstructionsElementKey, QGraphicsItem*> ElementItems;
	const QList<QGraphicsItem*> Items = Scene->items();

	for (QGraphicsItem* Item : Items)
	{
		swInstructionsElementKey Key;

		if (swGetElementKey(Item, Key))
			ElementItems[Key] = Item;
	}

	auto FindItem = [&ElementItems](const swInstructionsElementKey& Key) -> QGraphicsItem*
	{
		const auto ItemIt = ElementItems.find(Key);

		return ItemIt != ElementItems.end() && ItemIt->second->isVisible() ? ItemIt->second : nullptr;
	};

	// skip pointers of hidden elements
	for (const swInstructionsPointer& Pointer : Instructions->GetPointers())
	{
		if (Pointer.PageKey != PageKey)
			continue;

		QGraphicsItem* SourceItem = FindItem(Pointer.Source);

		if (!SourceItem)
			continue;

		// locked pages keep their pointer tips
		swInstructionsPointerItem* PointerItem = new swInstructionsPointerItem(PageItem, Instructions, Pointer, SourceItem, FindItem(Pointer.TargetElement));

		if (Instructions->mPageState.LockedPositions)
			PointerItem->setFlag(QGraphicsItem::ItemIsMovable, false);
	}
}

swInstructionsImageItem::swInstructionsImageItem(QGraphicsItem* Parent, swInstructions* Instructions, const swInstructionsImage& Image)
	: swInstructionsBoxItem(Parent), mInstructions(Instructions), mId(Image.Id)
{
	setFlags(SW_MOVABLE_ITEM_FLAGS);
	setZValue(1.0);
	setTransformationMode(Qt::SmoothTransformation);

	// a missing file shows a framed gap to find it
	const qreal Scale = swGetRenderScale(mInstructions);
	const qreal Width = mInstructions->mPageSetup.Width * Image.Size / 100.0;
	QImageReader Reader(mInstructions->GetFilePath(Image.File));
	const QSize FileSize = Reader.size();


	if (FileSize.isValid() && FileSize.width() > 0)
		Reader.setScaledSize(QSize(qRound(Width * Scale), qMax(qRound(Width * Scale * FileSize.height() / FileSize.width()), 1)));

	QImage Picture = Reader.read();

	if (Picture.isNull())
	{
		swInstructionsBoxStyle BoxStyle;
		BoxStyle.BorderWidth = 1.0;
		BoxStyle.BorderColor = Qt::gray;
		SetBoxStyle(BoxStyle);

		Picture = QImage(qRound(Width * Scale), qRound(Width * Scale / 2.0), QImage::Format_ARGB32_Premultiplied);
		Picture.fill(Qt::transparent);
	}

	Picture.setDevicePixelRatio(Scale);
	setPixmap(QPixmap::fromImage(Picture));
}

QVariant swInstructionsImageItem::itemChange(GraphicsItemChange Change, const QVariant& Value)
{
	if (Change == QGraphicsItem::ItemPositionChange)
		return swSnapItemPosition(this, mInstructions, Value);

	return QGraphicsPixmapItem::itemChange(Change, Value);
}

// use to add the images of a cover
static void swAddPageImages(swInstructions* Instructions, const swInstructionsPage* Page, QGraphicsRectItem* PageItem, const QRectF& MarginsRect)
{
	const swInstructionsImagePages Shown = Page->Type == swInstructionsPageType::Cover ? swInstructionsImagePages::Front : swInstructionsImagePages::Back;

	if (Page->Type != swInstructionsPageType::Cover && Page->Type != swInstructionsPageType::BackCover)
		return;

	for (const swInstructionsImage& Image : Instructions->GetImages())
	{
		if (Image.Pages != Shown && Image.Pages != swInstructionsImagePages::Both)
			continue;

		swInstructionsImageItem* ImageItem = new swInstructionsImageItem(PageItem, Instructions, Image);
		swPlaceElement(ImageItem, Instructions, swGetPagePlacementPosition(Instructions, MarginsRect, ImageItem->boundingRect().size(), Image.Placement));
	}
}

// use to add the step part count and the submodel preview
static void swAddPageCounts(swInstructions* Instructions, const swInstructionsPage* Page, QGraphicsRectItem* PageItem, const QRectF& MarginsRect)
{
	if (Page->Steps.empty())
		return;

	if (Instructions->GetBoolProperty(swInstructionsPropertyType::ShowPagePartCount, nullptr, 0))
	{
		const int Count = swInstructions::GetPageStepPartCount(*Page);

		if (Count > 0)
		{
			swInstructionsPartCountItem* CountItem = new swInstructionsPartCountItem(PageItem, Instructions, swInstructionsPlacementElement::PagePartCount, nullptr, Count);

			const int Placement = Instructions->GetIntProperty(swInstructionsPropertyType::PagePartCountPlacement, nullptr, 0);
			swPlaceElement(CountItem, Instructions, swGetPagePlacementPosition(Instructions, MarginsRect, CountItem->boundingRect().size(), Placement));
		}
	}

	// submodels show their finished model on their first page
	const swInstructionsStep& FirstStep = Page->Steps.front();
	const swModel* MainModel = Instructions->GetMainModel();

	if (FirstStep.FinalModel || FirstStep.Step != 1 || FirstStep.Model == MainModel || !Instructions->GetBoolProperty(swInstructionsPropertyType::ShowSubmodelPreview, nullptr, 0))
		return;

	swModel* Model = FirstStep.Model;
	const swStep LastStep = Model->GetLastStep();

	swInstructionsStepImageItem* PreviewItem = new swInstructionsStepImageItem(PageItem, Instructions, Model, LastStep);
	PreviewItem->SetSubmodelPreview();
	PreviewItem->setZValue(1.0);

	// a preview takes a quarter of the page
	const qreal PreviewInset = PreviewItem->GetBoxStyle().GetInset();
	PreviewItem->SetImageSize(qMax(static_cast<int>(MarginsRect.width() / 4.0 - 2.0 * PreviewInset), 1), qMax(static_cast<int>(MarginsRect.height() / 4.0 - 2.0 * PreviewInset), 1));
	PreviewItem->Update();

	// the part count sits around the preview
	swInstructionsPartCountItem* CountItem = new swInstructionsPartCountItem(PreviewItem, Instructions, swInstructionsPlacementElement::SubmodelPreviewPartCount, Model, swInstructions::GetModelPartCount(Model));
	const QRectF PreviewRect = PreviewItem->boundingRect();
	const QSizeF CountSize = CountItem->boundingRect().size();
	const int CountPlacement = Instructions->GetIntProperty(swInstructionsPropertyType::SubmodelPreviewPartCountPlacement, Model, LastStep);
	const QPointF CountPosition = swGetPlacementPosition(PreviewRect, CountSize, CountPlacement);

	// both together stay on the page
	const QRectF GroupRect = PreviewRect.united(QRectF(CountPosition, CountSize));
	const int PreviewPlacement = Instructions->GetIntProperty(swInstructionsPropertyType::SubmodelPreviewPlacement, Model, LastStep);
	swPlaceElement(PreviewItem, Instructions, swGetPagePlacementPosition(Instructions, MarginsRect, GroupRect.size(), PreviewPlacement) - GroupRect.topLeft());
	swPlaceElement(CountItem, Instructions, CountPosition);
}

void swUpdateInstructionsPageScene(swInstructions* Instructions, const swInstructionsPage* Page, QGraphicsScene* Scene)
{
	Scene->clear();

	if (!Page)
		return;

	const swInstructionsPageSetup& PageSetup = Instructions->mPageSetup;
	Scene->setSceneRect(0, 0, PageSetup.Width, PageSetup.Height);

	// fill the whole page background
	const size_t PageIndex = static_cast<size_t>(Page - Instructions->mPages.data());
	const swInstructionsPageScope PageScope(Instructions, PageIndex);
	const QColor BackgroundColor = Instructions->GetPageBackgroundColor(PageIndex);
	QGraphicsRectItem* PageItem = Scene->addRect(QRectF(0.0f, 0.0f, PageSetup.Width, PageSetup.Height), QPen(Qt::NoPen), QBrush(BackgroundColor));
	PageItem->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);

	QRectF MarginsRect(PageSetup.MarginLeft, PageSetup.MarginTop, PageSetup.Width - PageSetup.MarginLeft - PageSetup.MarginRight, PageSetup.Height - PageSetup.MarginTop - PageSetup.MarginBottom);

	// number step and materials pages
	const bool PageHasContent = (Page->Type == swInstructionsPageType::Steps && !Page->Steps.empty()) || Page->Type == swInstructionsPageType::BillOfMaterials || Instructions->GetBoolProperty(swInstructionsPropertyType::ShowPageNumberAllPages, nullptr, 0);

	if (PageHasContent && Instructions->GetBoolProperty(swInstructionsPropertyType::ShowPageNumber, nullptr, 0))
	{
		swInstructionsPageNumberItem* PageNumberItem = new swInstructionsPageNumberItem(PageItem, Instructions, PageIndex);

		const int Placement = Instructions->GetIntProperty(swInstructionsPropertyType::PageNumberPlacement, nullptr, 0);
		swPlaceElement(PageNumberItem, Instructions, swGetPagePlacementPosition(Instructions, MarginsRect, PageNumberItem->boundingRect().size(), Placement));
	}

	if (Page->Type != swInstructionsPageType::Blank && Page->Type != swInstructionsPageType::BackCover)
		swAddPageContent(Instructions, Page, Scene, PageItem, MarginsRect);

	if (Page->Type == swInstructionsPageType::Steps)
		swAddPageCounts(Instructions, Page, PageItem, MarginsRect);

	swAddPageImages(Instructions, Page, PageItem, MarginsRect);

	swAddPageAnnotations(Instructions, PageIndex, Scene, PageItem, MarginsRect);
}

// use to lock a page where it is drawn now
void swLockInstructionsPage(swInstructions* Instructions, size_t PageIndex)
{
	if (PageIndex >= Instructions->mPages.size())
		return;

	QGraphicsScene Scene;
	swUpdateInstructionsPageScene(Instructions, &Instructions->mPages[PageIndex], &Scene);

	// pointers follow their elements
	std::map<swInstructionsElementKey, QPointF> Positions;
	const QList<QGraphicsItem*> Items = Scene.items();

	for (const QGraphicsItem* Item : Items)
	{
		swInstructionsElementKey Key;

		if (swGetElementKey(Item, Key) && std::get<0>(Key) != swInstructionsPlacementElement::Pointer)
			Positions[Key] = Item->pos();
	}

	Instructions->LockPage(PageIndex, Positions);
}

void swInstructionsPageWidget::SetCurrentPage(const swInstructionsPage* Page)
{
	QGraphicsScene* Scene = scene();

	// keep the selection after a rebuild
	std::set<swInstructionsElementKey> SelectedKeys;
	const QList<QGraphicsItem*> SelectedItems = Scene->selectedItems();

	for (const QGraphicsItem* Item : SelectedItems)
	{
		swInstructionsElementKey Key;

		if (swGetElementKey(Item, Key))
			SelectedKeys.insert(Key);
	}

	mDragItems.clear();
	mPage = Page;
	mRebuildingPage = true;
	swUpdateInstructionsPageScene(mInstructions, Page, Scene);
	FitPage();

	// select a new pointer alone
	if (mPendingSelection)
	{
		SelectedKeys = { *mPendingSelection };
		mPendingSelection.reset();
	}

	const QList<QGraphicsItem*> Items = Scene->items();

	for (QGraphicsItem* Item : Items)
	{
		swInstructionsElementKey Key;

		if (swGetElementKey(Item, Key) && SelectedKeys.find(Key) != SelectedKeys.end())
			Item->setSelected(true);
	}

	mRebuildingPage = false;
	SelectionChanged();
}

void swInstructionsPageWidget::mousePressEvent(QMouseEvent* Event)
{
	QGraphicsView::mousePressEvent(Event);

	// the selection moves with a drag
	mDragItems.clear();

	if (Event->button() != Qt::LeftButton)
		return;

	const QList<QGraphicsItem*> SelectedItems = scene()->selectedItems();

	for (QGraphicsItem* Item : SelectedItems)
		if (Item->flags() & QGraphicsItem::ItemIsMovable)
			mDragItems.emplace_back(Item, Item->pos());
}

void swInstructionsPageWidget::mouseMoveEvent(QMouseEvent* Event)
{
	QGraphicsView::mouseMoveEvent(Event);

	mPointerPosition = mapToScene(Event->pos());

	if (mInstructions->IsRulersShown())
	{
		mHorizontalRuler->update();
		mVerticalRuler->update();
	}

	if (mDragItems.empty())
		return;

	// pointers follow dragged elements
	const QList<QGraphicsItem*> Items = scene()->items();

	for (QGraphicsItem* Item : Items)
		if (swInstructionsPointerItem* PointerItem = dynamic_cast<swInstructionsPointerItem*>(Item))
			PointerItem->UpdateGeometry();
}

void swInstructionsPageWidget::RemoveSelectedElements()
{
	std::vector<swInstructionsElementKey> Keys;

	for (QGraphicsItem* Item : scene()->selectedItems())
	{
		swInstructionsElementKey Key;

		if (swGetElementKey(Item, Key))
			Keys.push_back(Key);
	}

	// generated elements stay

	for (const swInstructionsElementKey& Key : Keys)
	{
		if (std::get<0>(Key) == swInstructionsPlacementElement::Pointer)
			mInstructions->RemovePointer(static_cast<int>(std::get<2>(Key)));
		else if (std::get<0>(Key) == swInstructionsPlacementElement::Notice)
			mInstructions->RemoveNotice(static_cast<int>(std::get<2>(Key)));
		else if (std::get<0>(Key) == swInstructionsPlacementElement::Image)
			mInstructions->RemoveImage(static_cast<int>(std::get<2>(Key)));
	}
}

void swInstructionsPageWidget::contextMenuEvent(QContextMenuEvent* Event)
{
	if (!mPage)
		return;

	const size_t PageIndex = static_cast<size_t>(mPage - mInstructions->mPages.data());
	const QPointF ScenePosition = mapToScene(Event->pos());
	QGraphicsItem* ElementItem = nullptr;
	swInstructionsElementKey Key = { swInstructionsPlacementElement::None, nullptr, 0 };

	// act on the topmost element
	const QList<QGraphicsItem*> Items = scene()->items(ScenePosition);

	for (QGraphicsItem* Item : Items)
	{
		if (swGetElementKey(Item, Key))
		{
			ElementItem = Item;
			break;
		}
	}

	if (ElementItem && !ElementItem->isSelected())
	{
		scene()->clearSelection();
		ElementItem->setSelected(true);
	}

	QMenu Menu(this);
	const swInstructionsPlacementElement Element = std::get<0>(Key);
	const int Id = static_cast<int>(std::get<2>(Key));

	if (Element == swInstructionsPlacementElement::Pointer)
	{
		const swInstructionsPointer* Pointer = mInstructions->FindPointer(Id);

		if (!Pointer)
			return;

		QMenu* AnchorMenu = Menu.addMenu(tr("Anchor"));
		const QStringList AnchorNames = { tr("Top Left"), tr("Top"), tr("Top Right"), tr("Right"), tr("Bottom Right"), tr("Bottom"), tr("Bottom Left"), tr("Left") };
		QActionGroup* AnchorGroup = new QActionGroup(AnchorMenu);

		for (int Anchor = SW_POINTER_ANCHOR_AUTO; Anchor < SW_POINTER_ANCHOR_COUNT; Anchor++)
		{
			QAction* Action = AnchorMenu->addAction(Anchor < 0 ? tr("Automatic") : AnchorNames[Anchor]);
			Action->setCheckable(true);
			Action->setChecked(Pointer->Anchor == Anchor);
			AnchorGroup->addAction(Action);

			if (Anchor < 0)
				AnchorMenu->addSeparator();

			connect(Action, &QAction::triggered, this, [this, Id, Anchor]()
			{
				mInstructions->SetPointerAnchor(Id, Anchor);
			});
		}

		const int PointerStyle = Pointer->Style;
		const int PointerTip = Pointer->Tip;

		auto AddChoiceMenu = [this, &Menu](const QString& Title, const QStringList& Names, int Current, std::function<void(int)> SetChoice) -> QMenu*
		{
			QMenu* ChoiceMenu = Menu.addMenu(Title);
			QActionGroup* ChoiceGroup = new QActionGroup(ChoiceMenu);

			for (int Choice = -1; Choice < Names.size(); Choice++)
			{
				QAction* Action = ChoiceMenu->addAction(Choice < 0 ? tr("Default") : Names[Choice]);
				Action->setCheckable(true);
				Action->setChecked(Current == Choice);
				ChoiceGroup->addAction(Action);

				if (Choice < 0)
					ChoiceMenu->addSeparator();

				connect(Action, &QAction::triggered, this, [SetChoice, Choice]()
				{
					SetChoice(Choice);
				});
			}

			return ChoiceMenu;
		};

		AddChoiceMenu(tr("Style"), { tr("Line"), tr("Callout Wedge") }, PointerStyle, [this, Id, PointerTip](int Style)
		{
			mInstructions->SetPointerStyle(Id, Style, PointerTip);
		});

		QMenu* TipMenu = AddChoiceMenu(tr("Tip"), { tr("Arrow"), tr("Dot") }, PointerTip, [this, Id, PointerStyle](int Tip)
		{
			mInstructions->SetPointerStyle(Id, PointerStyle, Tip);
		});

		// wedges have no tip
		TipMenu->setEnabled((PointerStyle >= 0 ? PointerStyle : mInstructions->GetIntProperty(swInstructionsPropertyType::PointerStyle, nullptr, 0)) != static_cast<int>(swInstructionsPointerStyle::Wedge));

		Menu.addSeparator();

		connect(Menu.addAction(tr("Remove Pointer")), &QAction::triggered, this, [this, Id]()
		{
			mInstructions->RemovePointer(Id);
		});
	}
	else
	{
		connect(Menu.addAction(tr("Add Notice...")), &QAction::triggered, this, [this, PageIndex, ScenePosition]()
		{
			QString Text;
			QString File;

			if (swEditNoticeText(this, Text, File, true))
				mInstructions->AddNotice(PageIndex, ScenePosition, Text, File);
		});

		if (ElementItem && dynamic_cast<swInstructionsBoxBase*>(ElementItem))
		{
			// new pointers leave sideways
			const QRectF BorderRect = ElementItem->mapRectToScene(dynamic_cast<swInstructionsBoxBase*>(ElementItem)->GetBorderRect());
			QPointF TipPosition(BorderRect.right() + 72.0, BorderRect.center().y());

			if (!sceneRect().contains(TipPosition))
				TipPosition.setX(BorderRect.left() - 72.0);

			connect(Menu.addAction(tr("Add Pointer")), &QAction::triggered, this, [this, PageIndex, Key, TipPosition]()
			{
				const int PointerId = mInstructions->AddPointer(PageIndex, Key, TipPosition);

				SelectElement({ swInstructionsPlacementElement::Pointer, nullptr, static_cast<swStep>(PointerId) });
			});
		}

		if (Element == swInstructionsPlacementElement::Notice)
		{
			Menu.addSeparator();

			connect(Menu.addAction(tr("Edit Notice...")), &QAction::triggered, this, [this, Id]()
			{
				const swInstructionsNotice* Notice = mInstructions->FindNotice(Id);
				QString Text = Notice ? Notice->Text : QString();
				QString File = Notice ? Notice->File : QString();

				if (Notice && swEditNoticeText(this, Text, File, false))
					mInstructions->SetNoticeText(Id, Text, File);
			});

			connect(Menu.addAction(tr("Remove Notice")), &QAction::triggered, this, [this, Id]()
			{
				mInstructions->RemoveNotice(Id);
			});
		}

		// covers carry linked images, like a logo
		const swInstructionsPageType PageType = mPage->Type;
		const QString ImageFilter = tr("Images (*.png *.jpg *.jpeg *.svg *.bmp *.gif *.webp);;All Files (*)");

		if (PageType == swInstructionsPageType::Cover || PageType == swInstructionsPageType::BackCover)
		{
			Menu.addSeparator();

			connect(Menu.addAction(tr("Add Image...")), &QAction::triggered, this, [this, PageType, ImageFilter]()
			{
				const QString FileName = QFileDialog::getOpenFileName(this, tr("Add Image"), QString(), ImageFilter);

				if (!FileName.isEmpty())
					SelectElement({ swInstructionsPlacementElement::Image, nullptr, static_cast<swStep>(mInstructions->AddImage(PageType, FileName)) });
			});
		}

		const swInstructionsImage* Image = Element == swInstructionsPlacementElement::Image ? mInstructions->FindImage(Id) : nullptr;

		if (Image)
		{
			const swInstructionsImage Current = *Image;

			connect(Menu.addAction(tr("Change Image...")), &QAction::triggered, this, [this, Current, ImageFilter]()
			{
				const QString FileName = QFileDialog::getOpenFileName(this, tr("Change Image"), mInstructions->GetFilePath(Current.File), ImageFilter);
				swInstructionsImage Changed = Current;
				Changed.File = FileName;

				if (!FileName.isEmpty())
					mInstructions->SetImage(Changed);
			});

			QMenu* PagesMenu = Menu.addMenu(tr("Show On"));
			QActionGroup* PagesGroup = new QActionGroup(PagesMenu);
			const std::pair<swInstructionsImagePages, QString> PagesNames[] =
			{
				{ swInstructionsImagePages::Front, tr("Front Cover") },
				{ swInstructionsImagePages::Back, tr("Back Cover") },
				{ swInstructionsImagePages::Both, tr("Both Covers") }
			};

			for (const std::pair<swInstructionsImagePages, QString>& PagesName : PagesNames)
			{
				QAction* Action = PagesMenu->addAction(PagesName.second);
				Action->setCheckable(true);
				Action->setChecked(Current.Pages == PagesName.first);
				PagesGroup->addAction(Action);

				const swInstructionsImagePages Pages = PagesName.first;

				connect(Action, &QAction::triggered, this, [this, Current, Pages]()
				{
					swInstructionsImage Changed = Current;
					Changed.Pages = Pages;
					mInstructions->SetImage(Changed);
				});
			}

			connect(Menu.addAction(tr("Size...")), &QAction::triggered, this, [this, Current]()
			{
				bool Accepted = false;
				const int Size = QInputDialog::getInt(this, tr("Image Size"), tr("Width in percent of the page width:"), Current.Size, 1, 100, 1, &Accepted);
				swInstructionsImage Changed = Current;
				Changed.Size = Size;

				if (Accepted)
					mInstructions->SetImage(Changed);
			});

			connect(Menu.addAction(tr("Remove Image")), &QAction::triggered, this, [this, Id]()
			{
				mInstructions->RemoveImage(Id);
			});
		}
	}

	Menu.exec(Event->globalPos());
}

void swInstructionsPageWidget::SelectElement(const swInstructionsElementKey& Key)
{
	const QList<QGraphicsItem*> Items = scene()->items();

	for (QGraphicsItem* Item : Items)
	{
		swInstructionsElementKey ItemKey;

		if (swGetElementKey(Item, ItemKey) && ItemKey == Key)
		{
			scene()->clearSelection();
			Item->setSelected(true);
			return;
		}
	}

	// select the element once rebuilt
	mPendingSelection = Key;
}

void swInstructionsPageWidget::mouseReleaseEvent(QMouseEvent* Event)
{
	QGraphicsView::mouseReleaseEvent(Event);

	std::map<swInstructionsElementKey, QPointF> Offsets;

	for (const std::pair<QGraphicsItem*, QPointF>& DragItem : mDragItems)
	{
		const QPointF Distance = DragItem.first->pos() - DragItem.second;
		swInstructionsElementKey Key;

		if (Distance.isNull() || !swGetElementKey(DragItem.first, Key))
			continue;

		Offsets[Key] = mInstructions->GetElementOffset(Key) + Distance;
	}

	mDragItems.clear();

	if (Offsets.empty())
		return;

	// rebuild after the release
	QTimer::singleShot(0, this, [this, Offsets]()
	{
		mInstructions->SetElementOffsets(Offsets);
	});
}

void swInstructionsPageWidget::drawForeground(QPainter* Painter, const QRectF& Rect)
{
	QGraphicsView::drawForeground(Painter, Rect);

	if (!mPage)
		return;

	// dash the margins on screen
	if (mInstructions->IsMarginsShown())
	{
		const swInstructionsPageSetup& PageSetup = mInstructions->mPageSetup;
		const QRectF MarginsRect(PageSetup.MarginLeft, PageSetup.MarginTop, PageSetup.Width - PageSetup.MarginLeft - PageSetup.MarginRight, PageSetup.Height - PageSetup.MarginTop - PageSetup.MarginBottom);

		QPen MarginPen(mInstructions->GetMarginColor(), 0.0, Qt::DashLine);
		MarginPen.setCosmetic(true);

		Painter->save();
		Painter->setPen(MarginPen);
		Painter->setBrush(Qt::NoBrush);
		Painter->drawRect(MarginsRect);
		Painter->restore();
	}

	if (mInstructions->IsGridShown())
		DrawGrid(Painter, Rect);

	DrawElementBoxes(Painter);
}

// use to draw the grid on screen
void swInstructionsPageWidget::DrawGrid(QPainter* Painter, const QRectF& Rect)
{
	const QRectF GridRect = mInstructions->GetGridRect();
	const QRectF PageRect = GridRect.intersected(Rect);
	const int Divisions = mInstructions->GetGridDivisions();
	const qreal SnapSize = mInstructions->GetGridSnapSize();

	if (PageRect.isEmpty())
		return;

	// count lines from the grid corner
	QVector<QLineF> PrimaryLines;
	QVector<QLineF> SecondaryLines;

	const int FirstColumn = static_cast<int>(std::ceil((PageRect.left() - GridRect.left()) / SnapSize - 0.001));
	const int LastColumn = static_cast<int>(std::floor((PageRect.right() - GridRect.left()) / SnapSize + 0.001));
	const int FirstRow = static_cast<int>(std::ceil((PageRect.top() - GridRect.top()) / SnapSize - 0.001));
	const int LastRow = static_cast<int>(std::floor((PageRect.bottom() - GridRect.top()) / SnapSize + 0.001));

	for (int Column = FirstColumn; Column <= LastColumn; Column++)
	{
		const qreal X = GridRect.left() + Column * SnapSize;
		(Column % Divisions ? SecondaryLines : PrimaryLines).append(QLineF(X, PageRect.top(), X, PageRect.bottom()));
	}

	for (int Row = FirstRow; Row <= LastRow; Row++)
	{
		const qreal Y = GridRect.top() + Row * SnapSize;
		(Row % Divisions ? SecondaryLines : PrimaryLines).append(QLineF(PageRect.left(), Y, PageRect.right(), Y));
	}

	QPen Pen(mInstructions->GetGridSecondaryColor(), 0.0);
	Pen.setCosmetic(true);

	Painter->save();
	Painter->setClipRect(PageRect);
	Painter->setPen(Pen);
	Painter->drawLines(SecondaryLines);
	Pen.setColor(mInstructions->GetGridPrimaryColor());
	Painter->setPen(Pen);
	Painter->drawLines(PrimaryLines);
	Painter->restore();
}

// use to inspect the element boxes like a web browser
void swInstructionsPageWidget::DrawElementBoxes(QPainter* Painter)
{
	const bool ShowMargins = mInstructions->IsElementMarginsShown();
	const bool ShowPadding = mInstructions->IsElementPaddingShown();
	const bool ShowAnchors = mInstructions->IsElementAnchorsShown();

	if (!ShowMargins && !ShowPadding && !ShowAnchors)
		return;

	const QColor MarginColor = mInstructions->GetElementMarginColor();
	const QColor PaddingColor = mInstructions->GetElementPaddingColor();
	const QColor AnchorColor = mInstructions->GetElementAnchorColor();
	const QTransform ViewTransform = Painter->transform();
	const QList<QGraphicsItem*> Items = scene()->items(Qt::AscendingOrder);
	std::vector<QPointF> AnchorPoints;

	Painter->save();
	Painter->setPen(Qt::NoPen);

	for (const QGraphicsItem* Item : Items)
	{
		const swInstructionsBoxBase* BoxItem = dynamic_cast<const swInstructionsBoxBase*>(Item);

		if (!BoxItem || !Item->isVisible())
			continue;

		const QRectF BoxRect = Item->boundingRect();

		if (BoxRect.isEmpty())
			continue;

		// box model rectangles from the outside in
		const swInstructionsBoxStyle& BoxStyle = BoxItem->GetBoxStyle();
		const QRectF BorderRect = BoxRect.adjusted(BoxStyle.Margin, BoxStyle.Margin, -BoxStyle.Margin, -BoxStyle.Margin);
		const QRectF PaddingRect = BorderRect.adjusted(BoxStyle.BorderWidth, BoxStyle.BorderWidth, -BoxStyle.BorderWidth, -BoxStyle.BorderWidth);
		const QRectF ContentRect = PaddingRect.adjusted(BoxStyle.Padding, BoxStyle.Padding, -BoxStyle.Padding, -BoxStyle.Padding);
		const QTransform ItemTransform = Item->sceneTransform();

		if (ShowMargins && BoxStyle.Margin > 0.0 && MarginColor.alpha())
		{
			QPainterPath MarginPath;
			MarginPath.setFillRule(Qt::OddEvenFill);
			MarginPath.addRect(BoxRect);
			MarginPath.addRect(BorderRect);

			Painter->fillPath(ItemTransform.map(MarginPath), MarginColor);
		}

		if (ShowPadding && BoxStyle.Padding > 0.0 && PaddingColor.alpha())
		{
			QPainterPath PaddingPath;
			PaddingPath.setFillRule(Qt::OddEvenFill);
			PaddingPath.addRect(PaddingRect);
			PaddingPath.addRect(ContentRect);

			Painter->fillPath(ItemTransform.map(PaddingPath), PaddingColor);
		}

		if (!ShowAnchors)
			continue;

		// corners, edge centers and center
		for (const QPointF& Point : swGetElementAnchors(BorderRect))
			AnchorPoints.push_back(ViewTransform.map(ItemTransform.map(Point)));
	}

	// markers keep their size at any zoom
	if (!AnchorPoints.empty() && AnchorColor.alpha())
	{
		const qreal Size = qMax(5.0, fontMetrics().height() / 3.0);
		QColor OutlineColor(Qt::white);
		OutlineColor.setAlpha(AnchorColor.alpha());

		Painter->resetTransform();
		Painter->setRenderHint(QPainter::Antialiasing, true);
		Painter->setPen(QPen(OutlineColor, 1.0));
		Painter->setBrush(AnchorColor);

		for (const QPointF& Point : AnchorPoints)
			Painter->drawRect(QRectF(Point.x() - Size / 2.0, Point.y() - Size / 2.0, Size, Size));
	}

	Painter->restore();
}

void swInstructionsPageWidget::resizeEvent(QResizeEvent* Event)
{
	QGraphicsView::resizeEvent(Event);
	FitPage();
	UpdateRulers();
}

void swInstructionsPageWidget::scrollContentsBy(int DeltaX, int DeltaY)
{
	QGraphicsView::scrollContentsBy(DeltaX, DeltaY);
	UpdateRulers();
}

bool swInstructionsPageWidget::viewportEvent(QEvent* Event)
{
	// the ruler arrows leave with the pointer
	if (Event->type() == QEvent::Leave && mPointerPosition)
	{
		mPointerPosition.reset();
		UpdateRulers();
	}

	return QGraphicsView::viewportEvent(Event);
}

// use to place the rulers around the page
void swInstructionsPageWidget::UpdateRulers()
{
	const bool ShowRulers = mInstructions->IsRulersShown();
	const int Thickness = ShowRulers ? swInstructionsRulerWidget::GetThickness(this) : 0;

	const QMargins Margins(Thickness, Thickness, 0, 0);

	// relayout only when the margins change, a relayout resizes and refits the page
	if (viewportMargins() != Margins)
		setViewportMargins(Margins);

	mHorizontalRuler->setVisible(ShowRulers);
	mVerticalRuler->setVisible(ShowRulers);
	mRulerCorner->setVisible(ShowRulers);

	if (!ShowRulers)
		return;

	const QRect ViewportRect = viewport()->geometry();

	mHorizontalRuler->setGeometry(ViewportRect.left(), ViewportRect.top() - Thickness, ViewportRect.width(), Thickness);
	mVerticalRuler->setGeometry(ViewportRect.left() - Thickness, ViewportRect.top(), Thickness, ViewportRect.height());
	mRulerCorner->setGeometry(ViewportRect.left() - Thickness, ViewportRect.top() - Thickness, Thickness, Thickness);
	mRulerCorner->setText(swGetPageUnitSuffix(mInstructions->GetRulerUnits()));

	mHorizontalRuler->update();
	mVerticalRuler->update();
}

void swInstructionsPageWidget::FitPage()
{
	// show the whole page
	if (mPage)
		fitInView(sceneRect().adjusted(-10.0, -10.0, 10.0, 10.0), Qt::KeepAspectRatio);

	UpdateRulers();
}

swInstructionsRulerWidget::swInstructionsRulerWidget(swInstructionsPageWidget* PageWidget, Qt::Orientation Orientation)
	: QWidget(PageWidget), mPageWidget(PageWidget), mOrientation(Orientation)
{
	setAutoFillBackground(false);

	if (Orientation == Qt::Horizontal)
	{
		setAccessibleName(tr("Horizontal Ruler"));
		setToolTip(tr("Horizontal position on the page in the page units, the arrow follows the pointer."));
	}
	else
	{
		setAccessibleName(tr("Vertical Ruler"));
		setToolTip(tr("Vertical position on the page in the page units, the arrow follows the pointer."));
	}
}

int swInstructionsRulerWidget::GetThickness(const QWidget* Widget)
{
	return Widget->fontMetrics().height() + 8;
}

// use to pick a labeled step at least the spacing apart
qreal swInstructionsRulerWidget::GetMajorStep(qreal PixelsPerUnit, qreal MinimumSpacing)
{
	const qreal Steps[] = { 0.1, 0.25, 0.5, 1.0, 2.0, 5.0, 10.0, 25.0, 50.0, 100.0, 250.0, 500.0, 1000.0, 2500.0, 5000.0, 10000.0 };

	for (const qreal Step : Steps)
		if (Step * PixelsPerUnit >= MinimumSpacing)
			return Step;

	return Steps[std::size(Steps) - 1];
}

// use to split a step into readable ticks
int swInstructionsRulerWidget::GetSubdivisions(swInstructionsPageUnits Units, qreal MajorSpacing)
{
	const std::vector<int> InchDivisions = { 16, 8, 4, 2 };
	const std::vector<int> MetricDivisions = { 10, 5, 2 };
	const std::vector<int>& Divisions = Units == swInstructionsPageUnits::Inches ? InchDivisions : MetricDivisions;

	for (const int Division : Divisions)
		if (MajorSpacing / Division >= 5.0)
			return Division;

	return 1;
}

void swInstructionsRulerWidget::paintEvent(QPaintEvent* Event)
{
	Q_UNUSED(Event);

	QPainter Painter(this);
	Painter.fillRect(rect(), palette().window());

	const bool Horizontal = mOrientation == Qt::Horizontal;
	const int Length = Horizontal ? width() : height();
	const int Thickness = Horizontal ? height() : width();
	const swInstructions* Instructions = mPageWidget->GetInstructions();
	const swInstructionsPageSetup& PageSetup = Instructions->mPageSetup;
	const QTransform ViewTransform = mPageWidget->viewportTransform();
	const qreal Scale = Horizontal ? ViewTransform.m11() : ViewTransform.m22();
	const qreal Offset = Horizontal ? ViewTransform.dx() : ViewTransform.dy();

	if (Scale <= 0.0)
		return;

	// page and margin spans
	auto MapLength = [Scale, Offset](qreal Points)
	{
		return Points * Scale + Offset;
	};

	auto FillSpan = [&Painter, Horizontal, Thickness](qreal Start, qreal End, const QBrush& Brush)
	{
		if (Horizontal)
			Painter.fillRect(QRectF(Start, 0.0, End - Start, Thickness), Brush);
		else
			Painter.fillRect(QRectF(0.0, Start, Thickness, End - Start), Brush);
	};

	const qreal PageLength = Horizontal ? PageSetup.Width : PageSetup.Height;
	const qreal MarginStart = Horizontal ? PageSetup.MarginLeft : PageSetup.MarginTop;
	const qreal MarginEnd = Horizontal ? PageSetup.MarginRight : PageSetup.MarginBottom;

	FillSpan(MapLength(0.0), MapLength(PageLength), palette().alternateBase());
	FillSpan(MapLength(MarginStart), MapLength(PageLength - MarginEnd), palette().base());

	// ticks in page units from the page corner
	const swInstructionsPageUnits Units = Instructions->GetRulerUnits();
	const qreal UnitScale = swGetPageUnitScale(Units);
	const qreal PixelsPerUnit = UnitScale * Scale;
	const qreal MinimumSpacing = fontMetrics().horizontalAdvance(QLatin1String("0000")) + 8.0;
	const qreal MajorStep = GetMajorStep(PixelsPerUnit, MinimumSpacing);
	const int Subdivisions = GetSubdivisions(Units, MajorStep * PixelsPerUnit);
	const qreal MinorStep = MajorStep / Subdivisions;
	const qreal StartUnits = (0.0 - Offset) / PixelsPerUnit;
	const qreal EndUnits = (Length - Offset) / PixelsPerUnit;
	const qint64 FirstTick = static_cast<qint64>(std::floor(StartUnits / MinorStep)) - 1;
	const qint64 LastTick = static_cast<qint64>(std::ceil(EndUnits / MinorStep)) + 1;

	QColor TickColor = palette().color(QPalette::WindowText);
	Painter.setPen(QPen(TickColor, 0.0));
	Painter.setFont(font());

	for (qint64 Tick = FirstTick; Tick <= LastTick; Tick++)
	{
		const qreal Position = std::round(Tick * MinorStep * PixelsPerUnit + Offset) + 0.5;
		const bool Major = Tick % Subdivisions == 0;
		const bool Half = Subdivisions % 2 == 0 && Tick % (Subdivisions / 2) == 0;
		qreal TickLength = Thickness * 0.2;

		if (Major)
			TickLength = Thickness;
		else if (Half)
			TickLength = Thickness * 0.4;

		if (Horizontal)
			Painter.drawLine(QPointF(Position, Thickness - TickLength), QPointF(Position, Thickness));
		else
			Painter.drawLine(QPointF(Thickness - TickLength, Position), QPointF(Thickness, Position));

		if (!Major)
			continue;

		const QString Label = locale().toString(Tick * MinorStep, 'g', 6);

		if (Horizontal)
		{
			Painter.drawText(QPointF(Position + 3.0, fontMetrics().ascent() + 1.0), Label);
		}
		else
		{
			Painter.save();
			Painter.translate(fontMetrics().ascent() + 1.0, Position - 3.0);
			Painter.rotate(-90.0);
			Painter.drawText(QPointF(0.0, 0.0), Label);
			Painter.restore();
		}
	}

	// edge facing the page
	Painter.setPen(QPen(palette().color(QPalette::Mid), 0.0));

	if (Horizontal)
		Painter.drawLine(QPointF(0.0, Thickness - 0.5), QPointF(Length, Thickness - 0.5));
	else
		Painter.drawLine(QPointF(Thickness - 0.5, 0.0), QPointF(Thickness - 0.5, Length));

	const std::optional<QPointF>& PointerPosition = mPageWidget->GetPointerPosition();

	if (!PointerPosition)
		return;

	// arrow pointing at the pointer
	const qreal PointerPixels = MapLength(Horizontal ? PointerPosition->x() : PointerPosition->y());
	const qreal ArrowSize = Thickness * 0.4;
	QPolygonF Arrow;

	if (Horizontal)
		Arrow << QPointF(PointerPixels, Thickness) << QPointF(PointerPixels - ArrowSize, Thickness - ArrowSize * 1.4) << QPointF(PointerPixels + ArrowSize, Thickness - ArrowSize * 1.4);
	else
		Arrow << QPointF(Thickness, PointerPixels) << QPointF(Thickness - ArrowSize * 1.4, PointerPixels - ArrowSize) << QPointF(Thickness - ArrowSize * 1.4, PointerPixels + ArrowSize);

	Painter.setRenderHint(QPainter::Antialiasing, true);
	const QColor PointerColor = Instructions->GetRulerPointerColor();
	QColor OutlineColor = PointerColor.lightnessF() > 0.5 ? QColor(Qt::black) : QColor(Qt::white);
	OutlineColor.setAlpha(PointerColor.alpha());

	Painter.setPen(QPen(OutlineColor, 1.0));
	Painter.setBrush(PointerColor);
	Painter.drawPolygon(Arrow);
}

// The lock and freeze toggles of each page.
class swInstructionsPageListDelegate : public QStyledItemDelegate
{
public:
	swInstructionsPageListDelegate(QObject* Parent, swInstructions* Instructions)
		: QStyledItemDelegate(Parent), mInstructions(Instructions)
	{
	}

	void paint(QPainter* Painter, const QStyleOptionViewItem& Option, const QModelIndex& Index) const override
	{
		QStyleOptionViewItem ItemOption = Option;
		initStyleOption(&ItemOption, Index);
		const QWidget* Widget = Option.widget;
		QStyle* Style = Widget ? Widget->style() : QApplication::style();

		// whole row first, then the toggles
		Style->drawPrimitive(QStyle::PE_PanelItemViewItem, &ItemOption, Painter, Widget);

		const size_t PageIndex = static_cast<size_t>(Index.row());
		const bool Selected = Option.state & QStyle::State_Selected;
		auto DrawToggle = [&](const QRect& Rect, swPaintedIcon Icon, bool Checked)
		{
			const QIcon::Mode Mode = !Checked ? QIcon::Disabled : Selected ? QIcon::Selected : QIcon::Normal;
			swCreatePaintedIcon(Icon).paint(Painter, Rect, Qt::AlignCenter, Mode);
		};

		DrawToggle(GetToggleRect(Option, 0), swPaintedIcon::Lock, mInstructions->IsPageLocked(PageIndex));
		DrawToggle(GetToggleRect(Option, 1), swPaintedIcon::Snowflake, mInstructions->IsPageFrozen(PageIndex));

		ItemOption.rect.setRight(GetVisibleRight(Option) - GetColumnWidth());
		ItemOption.state &= ~QStyle::State_HasFocus;
		QStyledItemDelegate::paint(Painter, ItemOption, Index);
	}

	QSize sizeHint(const QStyleOptionViewItem& Option, const QModelIndex& Index) const override
	{
		const QSize Size = QStyledItemDelegate::sizeHint(Option, Index);
		return QSize(Size.width() + GetColumnWidth(), qMax(Size.height(), GetIconSize() + 4));
	}

	bool editorEvent(QEvent* Event, QAbstractItemModel* Model, const QStyleOptionViewItem& Option, const QModelIndex& Index) override
	{
		if (Event->type() != QEvent::MouseButtonPress && Event->type() != QEvent::MouseButtonRelease && Event->type() != QEvent::MouseButtonDblClick)
			return QStyledItemDelegate::editorEvent(Event, Model, Option, Index);

		const QMouseEvent* MouseEvent = static_cast<QMouseEvent*>(Event);
		const int Toggle = FindToggle(Option, MouseEvent->pos());

		if (Toggle < 0 || MouseEvent->button() != Qt::LeftButton)
			return QStyledItemDelegate::editorEvent(Event, Model, Option, Index);

		// the list rebuilds after a toggle
		if (Event->type() == QEvent::MouseButtonRelease)
		{
			swInstructions* Instructions = mInstructions;
			const size_t PageIndex = static_cast<size_t>(Index.row());

			QTimer::singleShot(0, Instructions, [Instructions, PageIndex, Toggle]()
			{
				if (Toggle == 0)
				{
					if (Instructions->IsPageLocked(PageIndex))
						Instructions->UnlockPage(PageIndex);
					else
						swLockInstructionsPage(Instructions, PageIndex);
				}
				else if (Instructions->IsPageFrozen(PageIndex))
					Instructions->UnfreezePage(PageIndex);
				else
					Instructions->FreezePage(PageIndex);
			});
		}

		return true;
	}

	bool helpEvent(QHelpEvent* Event, QAbstractItemView* View, const QStyleOptionViewItem& Option, const QModelIndex& Index) override
	{
		const int Toggle = Event->type() == QEvent::ToolTip ? FindToggle(Option, Event->pos()) : -1;

		if (Toggle < 0)
			return QStyledItemDelegate::helpEvent(Event, View, Option, Index);

		const QString ToolTip = Toggle == 0
			? QCoreApplication::translate("swInstructionsPageListWidget", "Lock Page: keep every element where it is and refuse removal")
			: QCoreApplication::translate("swInstructionsPageListWidget", "Freeze Page: keep the page settings when changes are made for all pages");

		QToolTip::showText(Event->globalPos(), ToolTip, View);
		return true;
	}

protected:
	static int GetIconSize()
	{
		return QApplication::style()->pixelMetric(QStyle::PM_SmallIconSize);
	}

	static int GetColumnWidth()
	{
		return GetIconSize() * 2 + 12;
	}

	// rows can be wider than the list
	static int GetVisibleRight(const QStyleOptionViewItem& Option)
	{
		const QAbstractItemView* View = qobject_cast<const QAbstractItemView*>(Option.widget);
		return View ? qMin(Option.rect.right(), View->viewport()->rect().right()) : Option.rect.right();
	}

	// toggles sit at the right end
	static QRect GetToggleRect(const QStyleOptionViewItem& Option, int Toggle)
	{
		const int Size = GetIconSize();
		return QRect(GetVisibleRight(Option) - (2 - Toggle) * (Size + 4) + 1, Option.rect.center().y() - Size / 2, Size, Size);
	}

	static int FindToggle(const QStyleOptionViewItem& Option, const QPoint& Position)
	{
		// a little slack around each icon
		for (int Toggle = 0; Toggle < 2; Toggle++)
			if (GetToggleRect(Option, Toggle).adjusted(-2, -2, 2, 2).contains(Position))
				return Toggle;

		return -1;
	}

	swInstructions* mInstructions;
};

swInstructionsPageListWidget::swInstructionsPageListWidget(QWidget* Parent, swInstructions* Instructions)
	: QDockWidget(Parent), mInstructions(Instructions)
{
	setWindowTitle(tr("Pages"));
	setWhatsThis(tr("<p><b>Pages</b></p><p>Select a page to show it. Right-click a page to insert or remove blank pages around it.</p><p>Click the lock beside a page to keep its elements where they are. A locked page can't be removed.</p><p>Click the snowflake to freeze a page. It keeps its own copy of the settings, so changes for all pages skip it. Unfreeze it to follow the other pages again.</p>"));
	swSetupWidgetHelp(this);

	QWidget* CentralWidget = new QWidget(this);
	setWidget(CentralWidget);

	QVBoxLayout* Layout = new QVBoxLayout(CentralWidget);

	mThumbnailsWidget = new QListWidget(CentralWidget);
	mThumbnailsWidget->setAccessibleName(tr("Pages"));
	mThumbnailsWidget->setContextMenuPolicy(Qt::CustomContextMenu);
	mThumbnailsWidget->setItemDelegate(new swInstructionsPageListDelegate(mThumbnailsWidget, mInstructions));
	Layout->addWidget(mThumbnailsWidget);

	connect(mThumbnailsWidget, &QWidget::customContextMenuRequested, this, &swInstructionsPageListWidget::CustomMenuRequested);
}

void swInstructionsPageListWidget::CustomMenuRequested(const QPoint& Position)
{
	// right click makes the page current
	QListWidgetItem* ClickedItem = mThumbnailsWidget->itemAt(Position);

	if (ClickedItem)
		mThumbnailsWidget->setCurrentItem(ClickedItem);

	const size_t PageIndex = qMax(mThumbnailsWidget->currentRow(), 0);
	const bool IsBlank = PageIndex < mInstructions->mPages.size() && mInstructions->mPages[PageIndex].Type == swInstructionsPageType::Blank;

	QMenu Menu(this);

	QAction* InsertBeforeAction = Menu.addAction(tr("Insert Blank Page Before"));
	InsertBeforeAction->setEnabled(mInstructions->CanInsertBlankPage(PageIndex));

	connect(InsertBeforeAction, &QAction::triggered, this, [this, PageIndex]()
	{
		mInstructions->InsertBlankPage(PageIndex);
	});

	QAction* InsertAfterAction = Menu.addAction(tr("Insert Blank Page After"));
	InsertAfterAction->setEnabled(mInstructions->CanInsertBlankPage(PageIndex + 1));

	connect(InsertAfterAction, &QAction::triggered, this, [this, PageIndex]()
	{
		mInstructions->InsertBlankPage(PageIndex + 1);
	});

	QAction* RemoveAction = Menu.addAction(tr("Remove Blank Page"));
	RemoveAction->setEnabled(IsBlank && !mInstructions->IsPageLocked(PageIndex));

	connect(RemoveAction, &QAction::triggered, this, [this, PageIndex]()
	{
		mInstructions->RemoveBlankPage(PageIndex);
	});

	Menu.exec(mThumbnailsWidget->viewport()->mapToGlobal(Position));
}
