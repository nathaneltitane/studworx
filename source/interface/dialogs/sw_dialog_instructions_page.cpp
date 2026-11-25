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
#include <QPicture>
#include "sw_application.h"
#include "sw_window_main.h"

// freed pictures go back to the system

#if defined(__GLIBC__)
#include <malloc.h>
#endif

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
		Key = { TextItem->GetElement(), TextItem->GetModel(), 0 };
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

// use to list the pointer tip spots inside an element in quarters
static std::array<QPointF, 25> swGetPointerGrid()
{
	std::array<QPointF, 25> Fractions;

	for (int Index = 0; Index < 25; Index++)
		Fractions[Index] = QPointF((Index % 5) / 4.0, (Index / 5) / 4.0);

	return Fractions;
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

	// moves go anywhere, past the page
	if (!Scene || !Scene->mouseGrabberItem())
		return Value;

	return Parent ? Parent->mapFromScene(ScenePosition) : ScenePosition;
}

// drag and snap movable elements
constexpr QGraphicsItem::GraphicsItemFlags SW_MOVABLE_ITEM_FLAGS = QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemSendsGeometryChanges;

// use to get the text alignment of an element
static bool swGetElementAlignment(const swInstructions* Instructions, const swInstructionsElementKey& Key, int& Alignment, int& VerticalAlignment)
{
	swInstructionsPropertyType Type;
	swModel* Model = std::get<1>(Key);
	swStep Step = std::get<2>(Key);

	switch (std::get<0>(Key))
	{
	case swInstructionsPlacementElement::StepNumber:
		Type = Step == 0 ? swInstructionsPropertyType::BillOfMaterialsTitleAlignment : swInstructionsPropertyType::StepNumberAlignment;
		break;

	case swInstructionsPlacementElement::PageNumber:
		Type = swInstructionsPropertyType::PageNumberAlignment;
		break;

	case swInstructionsPlacementElement::BillOfMaterialsPartCount:
	case swInstructionsPlacementElement::PagePartCount:
	case swInstructionsPlacementElement::SubmodelPreviewPartCount:
		Type = swInstructionsPropertyType::PartCountAlignment;
		Step = 0;
		break;

	case swInstructionsPlacementElement::Notice:
	{
		// a notice can keep its own
		const swInstructionsNotice* Notice = Instructions->FindNotice(static_cast<int>(Step));

		if (Notice && Notice->Alignment >= 0)
		{
			Alignment = Notice->Alignment;
			VerticalAlignment = Notice->VerticalAlignment;
			return true;
		}

		Type = swInstructionsPropertyType::NoticeAlignment;
		Model = nullptr;
		Step = 0;
		break;
	}

	default:
	{
		const swInstructionsPublicationElement* Publication = swFindPublicationElement(std::get<0>(Key));

		if (!Publication)
			return false;

		Type = swGetPublicationType(*Publication, swInstructionsPublicationField::Alignment);
		break;
	}
	}

	Alignment = Instructions->GetIntProperty(Type, Model, Step);
	VerticalAlignment = Instructions->GetIntProperty(static_cast<swInstructionsPropertyType>(static_cast<int>(Type) + 1), Model, Step);

	return true;
}

// use to turn watermarks in place
static void swRotateWatermark(QGraphicsItem* Item, const swInstructions* Instructions)
{
	Item->setTransformOriginPoint(Item->boundingRect().center());
	Item->setRotation(Instructions->GetIntProperty(swInstructionsPropertyType::WatermarkRotation, nullptr, 0));
}

// use to place an element
static void swPlaceElement(QGraphicsItem* Item, const swInstructions* Instructions, const QPointF& Position)
{
	swInstructionsElementKey Key;
	swGetElementKey(Item, Key);

	// text boxes take their size and alignment
	int Alignment = 0;
	int VerticalAlignment = 1;
	swInstructionsBoxBase* Box = dynamic_cast<swInstructionsBoxBase*>(Item);

	if (Box && swGetElementAlignment(Instructions, Key, Alignment, VerticalAlignment))
		Box->SetBoxLayout(Instructions->GetElementSize(Key), Alignment, VerticalAlignment);
	else if (Box && std::get<0>(Key) == swInstructionsPlacementElement::Image && Instructions->GetElementSize(Key).isValid())
		Box->SetBoxSize(Item->boundingRect().size());

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

// renders kept until the model changes

struct swStepImageRender
{
	QImage Image;
	std::vector<swInstructionsStepImageItem::swPartRegion> PartRegions;
};

struct swPartsListRender
{
	QImage Image;
	std::vector<QRect> CountRects;
};

static QCache<QByteArray, swStepImageRender> gStepImageCache(SW_INSTRUCTIONS_STEP_IMAGE_CACHE);
static QCache<QByteArray, swPartsListRender> gPartsListCache(SW_INSTRUCTIONS_PARTS_LIST_CACHE);

// use to free every kept render

void swClearInstructionsImageCache()
{
	gStepImageCache.clear();
	gPartsListCache.clear();
}

// use to write the model part of a render key

static void swWriteInstructionsImageKey(QDataStream& Stream, swModel* Model, swStep Step)
{
	Stream << reinterpret_cast<quintptr>(Model) << Step;

	// any model edit changes its revision

	if (gApplication && gApplication->mProject)
		for (const std::unique_ptr<swModel>& ProjectModel : gApplication->mProject->GetModels())
			Stream << reinterpret_cast<quintptr>(ProjectModel.get()) << ProjectModel->GetCurrentHistoryRevision();
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

	const int ModelLength = mInstructions->GetIntProperty(SizeType, mModel, mStep);
	const qreal ModelSize = ModelLength > 0 ? static_cast<qreal>(ModelLength) / qMax(qMax(mWidth, mHeight), 1) : 1.0;
	const int Width = qMax(qRound(CellWidth * ModelSize), 1);
	const int Height = qMax(qRound(CellHeight * ModelSize), 1);

	// cover and previews show the finished model

	const bool Finished = mCoverImage || mSubmodelPreview || mFinalModel;
	const bool FadeSteps = !Finished && mInstructions->GetBoolProperty(swInstructionsPropertyType::FadePreviousSteps, mModel, mStep);
	const QColor FadeColor = mInstructions->GetColorProperty(swInstructionsPropertyType::FadeColor, mModel, mStep);
	const bool HighlightNewParts = !Finished && mInstructions->GetBoolProperty(swInstructionsPropertyType::HighlightNewParts, mModel, mStep);
	const QColor HighlightColor = mInstructions->GetColorProperty(swInstructionsPropertyType::HighlightColor, mModel, mStep);
	const int HighlightWidth = mInstructions->GetIntProperty(swInstructionsPropertyType::HighlightWidth, mModel, mStep);
	const int Projection = mInstructions->GetIntProperty(swInstructionsPropertyType::ModelProjection, mModel, mStep);
	const bool LockHomeView = mInstructions->GetBoolProperty(swInstructionsPropertyType::LockHomeView, mModel, mStep);
	const qreal Inset = mBoxStyle.GetInset();

	// every input of the render

	QByteArray Key;
	QDataStream KeyStream(&Key, QIODevice::WriteOnly);
	swWriteInstructionsImageKey(KeyStream, mModel, mStep);
	KeyStream << BackgroundColor << Scale << CellWidth << CellHeight << Width << Height << Finished << FadeSteps << FadeColor << HighlightNewParts << HighlightColor << HighlightWidth << Projection << LockHomeView << Inset;

	// a locked view ignores the main camera

	if (!LockHomeView)
	{
		const swCamera* Camera = View.GetCamera();

		KeyStream.writeRawData(reinterpret_cast<const char*>(&Camera->mWorldView), sizeof(Camera->mWorldView));
		KeyStream << Camera->m_fovy << Camera->m_zNear << Camera->m_zFar << static_cast<int>(Camera->GetProjection());
	}

	// a stored render skips the drawing

	if (const swStepImageRender* Render = gStepImageCache.object(Key))
	{
		mPartRegions = Render->PartRegions;
		setPixmap(QPixmap::fromImage(Render->Image));
		return;
	}

	View.SetBackgroundColor(SW_RGBA(BackgroundColor.red(), BackgroundColor.green(), BackgroundColor.blue(), 0));
	View.SetSize(Width, Height);
	View.SetLineWidthScale(Scale);
	View.SetFadeStepsOverride(FadeSteps, swRGBAFromQColor(FadeColor));
	View.SetHighlightNewPartsOverride(HighlightNewParts, swRGBAFromQColor(HighlightColor), HighlightWidth);

	// work on a copy of the main camera

	View.SetCamera(View.GetCamera(), true);
	View.GetCamera()->SetProjection(static_cast<swCameraProjection>(Projection));

	// a locked view starts from home

	if (LockHomeView)
		View.SetViewpoint(swViewpoint::Home);

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

	// cost is the picture kilobytes

	gStepImageCache.insert(Key, new swStepImageRender{ Image, mPartRegions }, static_cast<int>(Image.sizeInBytes() / 1024) + 1);

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
	SetBoxStyle(mInstructions->GetBoxStyle(mStep ? swInstructionsBoxElement::StepNumber : swInstructionsBoxElement::BillOfMaterialsTitle, mModel, mStep));
}

QVariant swInstructionsStepNumberItem::itemChange(GraphicsItemChange Change, const QVariant& Value)
{
	if (Change == QGraphicsItem::ItemPositionChange)
		return swSnapItemPosition(this, mInstructions, Value);

	return QGraphicsSimpleTextItem::itemChange(Change, Value);
}

void swInstructionsStepNumberItem::Update()
{
	bool Visible = mInstructions->GetBoolProperty(mStep ? swInstructionsPropertyType::ShowStepNumber : swInstructionsPropertyType::ShowBillOfMaterialsTitle, mModel, mStep);
	setVisible(Visible);

	// step 0 is the bill of materials title
	setFont(mInstructions->GetFontProperty(mStep ? swInstructionsPropertyType::StepNumberFont : swInstructionsPropertyType::BillOfMaterialsTitleFont, mModel, mStep));
	setBrush(QBrush(mInstructions->GetColorProperty(mStep ? swInstructionsPropertyType::StepNumberColor : swInstructionsPropertyType::BillOfMaterialsTitleColor, mModel, mStep)));
	setText(mStep ? QString::number(mStep) : mInstructions->GetSourcedText(swInstructionsPropertyType::BillOfMaterialsTitleText, swInstructionsPropertyType::BillOfMaterialsTitleFile, QCoreApplication::translate("swInstructionsDialog", "Parts List")));
}

// use to get the page number text
static QString swGetPageNumberText(const swInstructions* Instructions, size_t PageIndex)
{
	const QLocale Locale;
	const QString PageNumber = swFormatIntegerLocalized(static_cast<qlonglong>(PageIndex + 1), Locale);

	if (static_cast<swInstructionsPageNumberFormat>(Instructions->GetIntProperty(swInstructionsPropertyType::PageNumberFormat, nullptr, 0)) == swInstructionsPageNumberFormat::NumberOfPages)
		return QCoreApplication::translate("swInstructionsDialog", "%1 / %2").arg(PageNumber, swFormatIntegerLocalized(static_cast<qlonglong>(Instructions->mPages.size()), Locale));

	return PageNumber;
}

swInstructionsPageNumberItem::swInstructionsPageNumberItem(QGraphicsItem* Parent, swInstructions* Instructions, size_t PageIndex)
	: swInstructionsBoxItem(Parent), mInstructions(Instructions)
{
	setFlags(SW_MOVABLE_ITEM_FLAGS);
	setZValue(1.0);
	SetBoxStyle(mInstructions->GetBoxStyle(swInstructionsBoxElement::PageNumber, nullptr, 0));

	setFont(mInstructions->GetFontProperty(swInstructionsPropertyType::PageNumberFont, nullptr, 0));
	setBrush(QBrush(mInstructions->GetColorProperty(swInstructionsPropertyType::PageNumberColor, nullptr, 0)));
	setText(swGetPageNumberText(mInstructions, PageIndex));
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
	// only the bill of materials count has a box
	if (Element == swInstructionsPlacementElement::BillOfMaterialsPartCount)
		SetBoxStyle(mInstructions->GetBoxStyle(swInstructionsBoxElement::BillOfMaterialsPartCount, mModel, 0));

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

	SetBoxStyle(mInstructions->GetBoxStyle(swInstructionsBoxElement::PartsList, mModel, mStep));
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

	// the box paints the background in one piece
	QColor BackgroundColor = mInstructions->GetColorProperty(mBillOfMaterials ? swInstructionsPropertyType::BillOfMaterialsListBackgroundColor : swInstructionsPropertyType::PLIBackgroundColor, mModel, mStep);
	BackgroundColor.setAlpha(0);
	QFont Font = mInstructions->GetFontProperty(mBillOfMaterials ? swInstructionsPropertyType::BillOfMaterialsFont : swInstructionsPropertyType::PLIFont, mModel, mStep);
	QColor TextColor = mInstructions->GetColorProperty(mBillOfMaterials ? swInstructionsPropertyType::BillOfMaterialsTextColor : swInstructionsPropertyType::PLITextColor, mModel, mStep);
	const bool ShowCounts = mInstructions->GetBoolProperty(mBillOfMaterials ? swInstructionsPropertyType::BillOfMaterialsShowCounts : swInstructionsPropertyType::PLIShowCounts, mModel, mStep);
	const bool ShowPartIds = mBillOfMaterials && mInstructions->GetBoolProperty(swInstructionsPropertyType::BillOfMaterialsShowPartIds, mModel, mStep);
	const swPartsListSort Sort = static_cast<swPartsListSort>(mInstructions->GetIntProperty(mBillOfMaterials ? swInstructionsPropertyType::BillOfMaterialsSortOrder : swInstructionsPropertyType::PLISortOrder, mModel, mStep));
	const swPartsListLayout Layout = static_cast<swPartsListLayout>(mInstructions->GetIntProperty(mBillOfMaterials ? swInstructionsPropertyType::BillOfMaterialsLayout : swInstructionsPropertyType::PLILayout, mModel, mStep));
	const swPartsListAlignment Alignment = static_cast<swPartsListAlignment>(mInstructions->GetIntProperty(mBillOfMaterials ? swInstructionsPropertyType::BillOfMaterialsAlignment : swInstructionsPropertyType::PLIAlignment, mModel, mStep));
	const int ThumbnailSize = mInstructions->GetIntProperty(mBillOfMaterials ? swInstructionsPropertyType::BillOfMaterialsThumbnailSize : swInstructionsPropertyType::PLIThumbnailSize, mModel, mStep);
	const int CountPlacement = mInstructions->GetIntProperty(mBillOfMaterials ? swInstructionsPropertyType::BillOfMaterialsCountPlacement : swInstructionsPropertyType::PLICountPlacement, mModel, mStep);
	const float Scale = swGetRenderScale(mInstructions);
	std::vector<QRect> CountRects;

	SetBoxStyle(mInstructions->GetBoxStyle(mBillOfMaterials ? swInstructionsBoxElement::BillOfMaterialsList : swInstructionsBoxElement::PartsList, mModel, mStep));
	mClipContent = true;

	const swInstructionsBoxStyle CountBox = mInstructions->GetBoxStyle(mBillOfMaterials ? swInstructionsBoxElement::BillOfMaterialsQuantity : swInstructionsBoxElement::PartsListQuantity, mModel, mStep);

	// every input of the parts list

	const swStep ListStep = mBillOfMaterials ? 0 : mStep;
	QByteArray Key;
	QDataStream KeyStream(&Key, QIODevice::WriteOnly);
	swWriteInstructionsImageKey(KeyStream, mModel, ListStep);
	KeyStream << mMaxWidth << mMaxHeight << ThumbnailSize << BackgroundColor << Font.toString() << TextColor << ShowCounts << ShowPartIds << static_cast<int>(Sort) << static_cast<int>(Layout) << static_cast<int>(Alignment) << CountPlacement << Scale;
	KeyStream << CountBox.Margin << CountBox.Padding << CountBox.BorderWidth << CountBox.BorderColor << CountBox.BackgroundColor << CountBox.CornerRadius;

	// a stored list skips the drawing

	const swPartsListRender* Render = gPartsListCache.object(Key);
	swResult<QImage> PartsImage = Render ? swResult<QImage>(Render->Image) : mModel->GetPartsListImage(mMaxWidth, ThumbnailSize, ListStep, swRGBAFromQColor(BackgroundColor), Font, TextColor, false, ShowCounts, Sort, Layout, Alignment, mMaxHeight, Scale, ShowPartIds, CountPlacement, &CountRects, &CountBox);

	if (Render)
		CountRects = Render->CountRects;
	else if (PartsImage)
		gPartsListCache.insert(Key, new swPartsListRender{ PartsImage.value(), CountRects }, static_cast<int>(PartsImage.value().sizeInBytes() / 1024) + 1);

	// quantities rebuild with the image
	int SelectedCount = -1;
	int CountIndex = 0;

	for (QGraphicsItem* Child : childItems())
	{
		if (!dynamic_cast<swInstructionsPartsListCountItem*>(Child))
			continue;

		if (Child->isSelected())
			SelectedCount = CountIndex;

		CountIndex++;
		delete Child;
	}

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

	for (size_t Index = 0; Index < CountRects.size(); Index++)
	{
		const QRect& CountRect = CountRects[Index];
		swInstructionsPartsListCountItem* CountItem = new swInstructionsPartsListCountItem(this, QRectF(CountRect.x() / Scale + Inset, CountRect.y() / Scale + Inset, CountRect.width() / Scale, CountRect.height() / Scale), mModel, mBillOfMaterials ? 0 : mStep);

		// the selected quantity stays selected

		if (static_cast<int>(Index) == SelectedCount)
			CountItem->setSelected(true);
	}
}

swInstructionsCoverTextItem::swInstructionsCoverTextItem(QGraphicsItem* Parent, swInstructions* Instructions, swModel* Model, swInstructionsPlacementElement Element)
	: swInstructionsBoxItem(Parent), mInstructions(Instructions), mModel(Model), mElement(Element)
{
	setFlags(SW_MOVABLE_ITEM_FLAGS);
	setZValue(1.0);
	const swInstructionsPublicationElement* Publication = swFindPublicationElement(Element);
	SetBoxStyle(mInstructions->GetBoxStyle(Publication->Box, mModel, 0));
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
	mBend = mPointer.Bend;
	mReach = mPointer.Reach;
	mBendRadius = mInstructions->GetIntProperty(swInstructionsPropertyType::PointerBendRadius, nullptr, 0);
	mTip = static_cast<swInstructionsPointerTip>(Tip);
	mColor = mInstructions->GetColorProperty(swInstructionsPropertyType::PointerColor, nullptr, 0);

	// body follows the element border
	const swInstructionsBoxBase* Box = dynamic_cast<const swInstructionsBoxBase*>(mSource);
	const int Width = mInstructions->GetIntProperty(swInstructionsPropertyType::PointerWidth, nullptr, 0);

	mSourceBorderWidth = Box ? Box->GetBorderWidth() : 0.0;
	mSourceRadius = Box ? Box->GetBoxStyle().CornerRadius : 0.0;
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

		if (mPointer.TargetAnchor.x() >= 0.0)
			TipPosition = TargetRect.topLeft() + QPointF(TargetRect.width() * mPointer.TargetAnchor.x(), TargetRect.height() * mPointer.TargetAnchor.y());
		else if (TargetRect.contains(Center))
			TipPosition = TargetRect.center();
		else
			TipPosition = QPointF(qBound(TargetRect.left(), Center.x(), TargetRect.right()), qBound(TargetRect.top(), Center.y(), TargetRect.bottom()));
	}

	// leave the middle of the border
	const qreal BorderHalf = mSourceBorderWidth / 2.0;
	const QRectF AnchorRect = SourceRect.adjusted(BorderHalf, BorderHalf, -BorderHalf, -BorderHalf);

	// chosen or nearest anchor whose lead stays off the element
	prepareGeometryChange();
	mTipPosition = TipPosition;
	const int Anchor = GetClearAnchor(SourceRect, mPointer.Anchor, TipPosition);

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

	const QPointF Direction = GetTipDirection();
	const QPointF Normal(-Direction.y(), Direction.x());

	// wedge narrows to the tip
	if (mStyle == swInstructionsPointerStyle::Wedge)
		Path.addPath(GetWedgePath(SourceRect, BorderWidth));

	if (mTip == swInstructionsPointerTip::Dot)
		Path.addEllipse(mTipPosition, mTipSize / 2.0, mTipSize / 2.0);
	else if (mTip == swInstructionsPointerTip::HollowDot && mStyle != swInstructionsPointerStyle::Wedge)
	{
		// ring with an empty middle
		const qreal InnerRadius = qMax(0.0, mTipSize / 2.0 - mWidth);
		QPainterPath Outer, Inner;
		Outer.addEllipse(mTipPosition, mTipSize / 2.0, mTipSize / 2.0);
		Inner.addEllipse(mTipPosition, InnerRadius, InnerRadius);
		Path.addPath(Outer.subtracted(Inner));
	}
	else if (mStyle != swInstructionsPointerStyle::Wedge)
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

// use to route the lead from the base to the tip
QPolygonF swInstructionsPointerItem::GetLeadPoints(const QPointF& Base, int Anchor) const
{
	const bool Horizontal = IsLeadHorizontal(Base, Anchor);

	if (mStyle == swInstructionsPointerStyle::UShape)
	{
		// out past the base and the tip, then back in
		const QPointF Signs[SW_POINTER_ANCHOR_COUNT] = { { -1.0, -1.0 }, { 0.0, -1.0 }, { 1.0, -1.0 }, { 1.0, 0.0 }, { 1.0, 1.0 }, { 0.0, 1.0 }, { -1.0, 1.0 }, { -1.0, 0.0 } };
		const QPointF Sign = Signs[qBound(0, Anchor, SW_POINTER_ANCHOR_COUNT - 1)];
		const QPointF Normal = Horizontal ? QPointF(Sign.x(), 0.0) : QPointF(0.0, Sign.y());
		const QPointF Delta = mTipPosition - Base;
		const qreal Clearance = qMax(0.0, QPointF::dotProduct(Delta, Normal)) + mWidth;
		const qreal Reach = mReach >= 0.0 ? qMax(Clearance, mReach) : Clearance + qMax(mTipSize, 12.0);
		const QPointF Out = Base + Normal * Reach;

		return QPolygonF(QVector<QPointF>{ Base, Out, mTipPosition + Normal * (Reach - QPointF::dotProduct(Delta, Normal)), mTipPosition });
	}

	if (mStyle != swInstructionsPointerStyle::LShape && mStyle != swInstructionsPointerStyle::ZShape)
		return QPolygonF(QVector<QPointF>{ Base, mTipPosition });

	if (mStyle == swInstructionsPointerStyle::LShape)
		return QPolygonF(QVector<QPointF>{ Base, Horizontal ? QPointF(mTipPosition.x(), Base.y()) : QPointF(Base.x(), mTipPosition.y()), mTipPosition });

	const QPointF Middle = Base + (mTipPosition - Base) * mBend;

	if (Horizontal)
		return QPolygonF(QVector<QPointF>{ Base, QPointF(Middle.x(), Base.y()), QPointF(Middle.x(), mTipPosition.y()), mTipPosition });

	return QPolygonF(QVector<QPointF>{ Base, QPointF(Base.x(), Middle.y()), QPointF(mTipPosition.x(), Middle.y()), mTipPosition });
}

// use to leave side anchors straight out, corners follow the longer way
bool swInstructionsPointerItem::IsLeadHorizontal(const QPointF& Base, int Anchor) const
{
	const QPointF Delta = mTipPosition - Base;

	return Anchor == 3 || Anchor == 7 || (Anchor % 2 == 0 && qAbs(Delta.x()) >= qAbs(Delta.y()));
}

// use to test a lead segment against the inside of its element
static bool swIsSegmentInRect(const QPointF& Start, const QPointF& End, const QRectF& Rect)
{
	if (!Rect.isValid())
		return false;

	// clip the segment to each side
	const QPointF Delta = End - Start;
	const qreal Directions[4] = { -Delta.x(), Delta.x(), -Delta.y(), Delta.y() };
	const qreal Distances[4] = { Start.x() - Rect.left(), Rect.right() - Start.x(), Start.y() - Rect.top(), Rect.bottom() - Start.y() };
	qreal Enter = 0.0;
	qreal Leave = 1.0;

	for (int Side = 0; Side < 4; Side++)
	{
		if (Directions[Side] == 0.0)
		{
			if (Distances[Side] < 0.0)
				return false;

			continue;
		}

		const qreal Fraction = Distances[Side] / Directions[Side];

		if (Directions[Side] < 0.0)
			Enter = qMax(Enter, Fraction);
		else
			Leave = qMin(Leave, Fraction);

		if (Enter > Leave)
			return false;
	}

	return true;
}

// use to keep the lead off its own element, nearest the reference first
int swInstructionsPointerItem::GetClearAnchor(const QRectF& SourceRect, int Preferred, const QPointF& Reference) const
{
	const qreal BorderHalf = mSourceBorderWidth / 2.0;
	const QRectF AnchorRect = SourceRect.adjusted(BorderHalf, BorderHalf, -BorderHalf, -BorderHalf);
	const qreal Inset = mSourceBorderWidth + 1.0;
	const QRectF InnerRect = SourceRect.adjusted(Inset, Inset, -Inset, -Inset);
	std::array<int, SW_POINTER_ANCHOR_COUNT> Anchors;

	for (int Index = 0; Index < SW_POINTER_ANCHOR_COUNT; Index++)
		Anchors[Index] = Index;

	std::stable_sort(Anchors.begin(), Anchors.end(), [&](int First, int Second)
	{
		return QLineF(swGetPointerAnchor(AnchorRect, First), Reference).length() < QLineF(swGetPointerAnchor(AnchorRect, Second), Reference).length();
	});

	if (Preferred >= 0 && Preferred < SW_POINTER_ANCHOR_COUNT)
		std::rotate(Anchors.begin(), std::find(Anchors.begin(), Anchors.end(), Preferred), std::find(Anchors.begin(), Anchors.end(), Preferred) + 1);

	for (int Anchor : Anchors)
	{
		const QPolygonF Points = GetLeadPoints(swGetPointerAnchor(AnchorRect, Anchor), Anchor);
		bool Clear = true;

		for (int Index = 1; Index < Points.size() && Clear; Index++)
			Clear = !swIsSegmentInRect(Points[Index - 1], Points[Index], InnerRect);

		if (Clear)
			return Anchor;
	}

	return Anchors[0];
}

bool swInstructionsPointerItem::IsBendHorizontal() const
{
	return IsLeadHorizontal(mBasePosition);
}

// use to find the Z middle or U outer segment in parent coordinates
bool swInstructionsPointerItem::GetBendLine(bool& Horizontal, qreal& Coordinate) const
{
	if (mStyle != swInstructionsPointerStyle::ZShape && mStyle != swInstructionsPointerStyle::UShape)
		return false;

	const QPolygonF Points = GetLeadPoints(mBasePosition);
	Horizontal = IsLeadHorizontal(mBasePosition);
	Coordinate = Horizontal ? Points[1].x() : Points[1].y();

	return true;
}

bool swInstructionsPointerItem::IsBendAt(const QPointF& Position) const
{
	if (mLocked || (mStyle != swInstructionsPointerStyle::ZShape && mStyle != swInstructionsPointerStyle::UShape))
		return false;

	const QPolygonF Points = GetLeadPoints(mBasePosition);

	return QLineF(Position, (Points[1] + Points[2]) / 2.0).length() <= 8.0;
}

// use to round the lead bends
static QPainterPath swGetRoundedLead(const QPolygonF& Points, qreal Radius)
{
	QPainterPath Path(Points[0]);

	for (int Index = 1; Index < Points.size() - 1; Index++)
	{
		const QLineF In(Points[Index - 1], Points[Index]);
		const QLineF Out(Points[Index], Points[Index + 1]);
		const qreal CornerRadius = qMin(Radius, qMin(In.length(), Out.length()) / 2.0);

		if (CornerRadius < 0.5)
		{
			Path.lineTo(Points[Index]);
			continue;
		}

		// cubic quarter circle
		const QPointF InDirection = (In.p2() - In.p1()) / In.length();
		const QPointF OutDirection = (Out.p2() - Out.p1()) / Out.length();
		const QPointF Start = Points[Index] - InDirection * CornerRadius;
		const QPointF End = Points[Index] + OutDirection * CornerRadius;
		const qreal Handle = CornerRadius * 0.5523;

		Path.lineTo(Start);
		Path.cubicTo(Start + InDirection * Handle, End - OutDirection * Handle, End);
	}

	Path.lineTo(Points.back());

	return Path;
}

// use to turn the tip along the last lead segment
QPointF swInstructionsPointerItem::GetTipDirection() const
{
	const QPolygonF Points = GetLeadPoints(mBasePosition);

	for (int Index = Points.size() - 1; Index > 0; Index--)
	{
		const QLineF Segment(Points[Index - 1], Points[Index]);

		if (Segment.length() >= 0.5)
			return (Segment.p2() - Segment.p1()) / Segment.length();
	}

	return QPointF(1.0, 0.0);
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
		// follow the rounded box inside
		const qreal InnerRadius = qMax(0.0, mSourceRadius - BorderWidth);
		QPainterPath InnerPath;
		InnerPath.addRoundedRect(InnerRect, InnerRadius, InnerRadius);
		Path = Path.subtracted(InnerPath);
	}

	return Path;
}

QRectF swInstructionsPointerItem::boundingRect() const
{
	const qreal Extent = qMax(mTipSize, mWidth) + mSourceBorderWidth + mWidth + 8.0;

	// the U shape reaches past the base and tip
	return GetLeadPoints(mBasePosition).boundingRect().united(QRectF(mBasePosition, mTipPosition).normalized()).adjusted(-Extent, -Extent, Extent, Extent);
}

QPainterPath swInstructionsPointerItem::shape() const
{
	QPainterPath Path;
	Path.addPolygon(GetLeadPoints(mBasePosition));

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

	if (mStyle != swInstructionsPointerStyle::Wedge && Line.length() >= 0.5)
	{
		// start the line inside the box
		const qreal BorderHalf = BorderWidth / 2.0;
		const QPointF Base = swGetPointerAnchor(SourceRect.adjusted(BorderHalf, BorderHalf, -BorderHalf, -BorderHalf), mAnchor);
		const QTransform& Transform = Painter->worldTransform();
		const qreal Scale = qAbs(Transform.m11());
		qreal Width = mWidth == mSourceBorderWidth ? BorderWidth : mWidth;
		QPolygonF Points = GetLeadPoints(Base);

		// whole pixel lines like borders

		if (!Transform.isRotating() && Transform.isInvertible() && Scale > 0.0)
		{
			const qreal DeviceWidth = qMax<qreal>(qRound(Width * Scale), 1.0);
			const qreal Offset = static_cast<int>(DeviceWidth) % 2 ? 0.5 : 0.0;
			const QTransform Inverted = Transform.inverted();

			Width = DeviceWidth / Scale;

			for (QPointF& Point : Points)
			{
				const QPointF Device = Transform.map(Point);
				Point = Inverted.map(QPointF(qRound(Device.x() - Offset) + Offset, qRound(Device.y() - Offset) + Offset));
			}
		}

		const QLineF FirstSegment(Points[0], Points[1]);

		if (FirstSegment.length() >= 0.5)
			Points[0] -= (FirstSegment.p2() - FirstSegment.p1()) / FirstSegment.length() * (BorderWidth + Width);

		// stop the line at the arrow
		const QLineF LastSegment(Points[Points.size() - 2], Points.back());

		if (mTip == swInstructionsPointerTip::Arrow && LastSegment.length() > mTipSize * 0.5)
			Points.back() -= GetTipDirection() * (mTipSize * 0.5);

		// lines end on the ring
		if (mTip == swInstructionsPointerTip::HollowDot && LastSegment.length() > mTipSize * 0.5)
			Points.back() -= GetTipDirection() * qMax(0.0, mTipSize / 2.0 - mWidth / 2.0);

		// sharp bends
		QPen Pen(mColor, Width);
		Pen.setCapStyle(Qt::FlatCap);
		Pen.setJoinStyle(Qt::MiterJoin);
		Painter->setPen(Pen);
		Painter->setBrush(Qt::NoBrush);
		Painter->drawPath(swGetRoundedLead(Points, mBendRadius));
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

		// bend handle
		if (mStyle == swInstructionsPointerStyle::ZShape || mStyle == swInstructionsPointerStyle::UShape)
		{
			const QPolygonF Points = GetLeadPoints(mBasePosition);
			Painter->drawEllipse((Points[1] + Points[2]) / 2.0, 4.0, 4.0);
		}
	}

	Painter->restore();
}

void swInstructionsPointerItem::mousePressEvent(QGraphicsSceneMouseEvent* Event)
{
	QGraphicsItem::mousePressEvent(Event);

	// drag the tip to a new target
	if (Event->button() == Qt::LeftButton && IsTipAt(Event->pos()))
	{
		mDraggingTip = true;
		Event->accept();
	}
	else if (Event->button() == Qt::LeftButton && IsBaseAt(Event->pos()))
	{
		mDraggingBase = true;
		Event->accept();
	}
	else if (Event->button() == Qt::LeftButton && IsBendAt(Event->pos()))
	{
		mDraggingBend = true;
		Event->accept();
	}
}

void swInstructionsPointerItem::mouseMoveEvent(QGraphicsSceneMouseEvent* Event)
{
	if (mDraggingBase)
	{
		// nearest anchor, the tip stays
		const qreal BorderHalf = mSourceBorderWidth / 2.0;
		const QRectF AnchorRect = mSourceRect.adjusted(BorderHalf, BorderHalf, -BorderHalf, -BorderHalf);
		mAnchor = GetClearAnchor(mSourceRect, -1, mapToItem(parentItem(), Event->pos()));

		prepareGeometryChange();
		mBasePosition = swGetPointerAnchor(AnchorRect, mAnchor);
		update();
		return;
	}

	if (mDraggingBend)
	{
		const bool Horizontal = IsLeadHorizontal(mBasePosition);
		const qreal Start = Horizontal ? mBasePosition.x() : mBasePosition.y();
		const qreal Span = (Horizontal ? mTipPosition.x() : mTipPosition.y()) - Start;
		qreal Coordinate = Horizontal ? Event->pos().x() : Event->pos().y();

		// line up with the other bends
		const QGraphicsView* View = scene()->views().value(0);
		qreal SnapLength = 10.0 / (View ? qMax(0.01, View->transform().m11()) : 1.0);
		const qreal Pointer = Coordinate;

		for (const QGraphicsItem* Item : parentItem()->childItems())
		{
			const swInstructionsPointerItem* Other = dynamic_cast<const swInstructionsPointerItem*>(Item);
			bool OtherHorizontal = false;
			qreal OtherCoordinate = 0.0;

			if (!Other || Other == this || !Other->GetBendLine(OtherHorizontal, OtherCoordinate) || OtherHorizontal != Horizontal || qAbs(OtherCoordinate - Pointer) > SnapLength)
				continue;

			SnapLength = qAbs(OtherCoordinate - Pointer);
			Coordinate = OtherCoordinate;
		}

		prepareGeometryChange();

		// the U reach runs from the base along its side
		if (mStyle == swInstructionsPointerStyle::UShape)
		{
			const QPolygonF Points = GetLeadPoints(mBasePosition);
			const qreal Outward = (Horizontal ? Points[1].x() : Points[1].y()) >= Start ? 1.0 : -1.0;
			mReach = qMax(0.0, (Coordinate - Start) * Outward);
		}
		else if (qAbs(Span) >= 0.5)
			mBend = qBound(0.0, (Coordinate - Start) / Span, 1.0);

		update();
		return;
	}

	if (!mDraggingTip)
	{
		QGraphicsItem::mouseMoveEvent(Event);
		return;
	}

	mTipPosition = Event->pos();
	mSnapItem = nullptr;
	mSnapped = false;

	// tip snaps to the grid of the element under it
	const QGraphicsView* View = scene()->views().value(0);
	const qreal SnapDistance = 10.0 / (View ? qMax(0.01, View->transform().m11()) : 1.0);
	const QPointF Position = mapToItem(parentItem(), Event->pos());
	const QRectF SearchRect(Event->scenePos() - QPointF(SnapDistance, SnapDistance), QSizeF(2.0 * SnapDistance, 2.0 * SnapDistance));

	for (const QGraphicsItem* Item : scene()->items(SearchRect))
	{
		swInstructionsElementKey Key;
		QRectF Rect;

		if (Item == mSource || dynamic_cast<const swInstructionsPointerItem*>(Item) || !swGetElementKey(Item, Key) || !swGetBorderRect(Item, parentItem(), Rect))
			continue;

		if (!Rect.adjusted(-SnapDistance, -SnapDistance, SnapDistance, SnapDistance).contains(Position))
			continue;

		mSnapItem = Item;
		mSnapRect = Rect;
		qreal SnapLength = SnapDistance;

		for (const QPointF& Fraction : swGetPointerGrid())
		{
			const QPointF Point = Rect.topLeft() + QPointF(Rect.width() * Fraction.x(), Rect.height() * Fraction.y());
			const qreal Length = QLineF(Point, Position).length();

			if (Length > SnapLength)
				continue;

			SnapLength = Length;
			mSnapFraction = Fraction;
			mSnapped = true;
			mTipPosition = mapFromItem(parentItem(), Point);
		}

		break;
	}

	UpdateGeometry();
	scene()->update();
}

void swInstructionsPointerItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* Event)
{
	QGraphicsItem::mouseReleaseEvent(Event);

	if (mDraggingBase)
	{
		mDraggingBase = false;

		// rebuild after the release
		swInstructions* Instructions = mInstructions;
		const int Id = mPointer.Id;
		const int Anchor = mAnchor;

		QTimer::singleShot(0, Instructions, [Instructions, Id, Anchor]()
		{
			Instructions->SetPointerAnchor(Id, Anchor);
		});

		return;
	}

	if (mDraggingBend)
	{
		mDraggingBend = false;

		// rebuild after the release
		swInstructions* Instructions = mInstructions;
		const int Id = mPointer.Id;
		const qreal Bend = mBend;
		const qreal Reach = mReach;
		const bool UShape = mStyle == swInstructionsPointerStyle::UShape;

		QTimer::singleShot(0, Instructions, [Instructions, Id, Bend, Reach, UShape]()
		{
			if (UShape)
				Instructions->SetPointerReach(Id, Reach);
			else
				Instructions->SetPointerBend(Id, Bend);
		});

		return;
	}

	if (!mDraggingTip)
		return;

	mDraggingTip = false;
	scene()->update();

	// point at the topmost target
	swInstructionsPointerTarget TargetType = swInstructionsPointerTarget::Point;
	swInstructionsElementKey TargetElement = { swInstructionsPlacementElement::None, nullptr, 0 };
	int TargetPart = -1;
	QPointF TargetAnchor(-1.0, -1.0);
	QPointF TipPosition = mapToItem(parentItem(), Event->pos());
	const QList<QGraphicsItem*> Items = scene()->items(Event->scenePos());

	// grid spots win over parts
	if (mSnapItem && mSnapped && swGetElementKey(mSnapItem, TargetElement))
	{
		TargetType = swInstructionsPointerTarget::Element;
		TargetAnchor = mSnapFraction;
		TipPosition = mapToItem(parentItem(), mTipPosition);
	}

	for (const QGraphicsItem* Item : Items)
	{
		if (TargetType != swInstructionsPointerTarget::Point)
			break;

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

		// the tip stays where it was dropped
		QRectF TargetRect;

		if (swGetBorderRect(Item, parentItem(), TargetRect) && TargetRect.width() > 0.0 && TargetRect.height() > 0.0)
			TargetAnchor = QPointF(qBound(0.0, (TipPosition.x() - TargetRect.left()) / TargetRect.width(), 1.0), qBound(0.0, (TipPosition.y() - TargetRect.top()) / TargetRect.height(), 1.0));

		break;
	}

	// rebuild after the release
	swInstructions* Instructions = mInstructions;
	const int Id = mPointer.Id;

	mSnapItem = nullptr;

	QTimer::singleShot(0, Instructions, [Instructions, Id, TargetType, TargetElement, TargetPart, TipPosition, TargetAnchor]()
	{
		Instructions->SetPointerTarget(Id, TargetType, TargetElement, TargetPart, TipPosition, TargetAnchor);
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

	// models follow the turned view

	if (gMainWindow)
	{
		QTimer* ViewTimer = new QTimer(this);
		ViewTimer->setSingleShot(true);
		ViewTimer->setInterval(250);

		connect(gMainWindow, &swMainWindow::ViewOrientationChanged, ViewTimer, qOverload<>(&QTimer::start));
		connect(ViewTimer, &QTimer::timeout, this, [this]()
		{
			if (!mInstructions->GetBoolProperty(swInstructionsPropertyType::LockHomeView, nullptr, 0))
				StepSettingsChanged(nullptr, 0);
		});
	}
	connect(Scene, &QGraphicsScene::selectionChanged, this, &swInstructionsPageWidget::SelectionChanged);
	connect(mPlacementWidget, &swInstructionsPlacementWidget::AlignSelection, this, &swInstructionsPageWidget::AlignSelection);
	connect(mPlacementWidget, &swInstructionsPlacementWidget::PlaceElement, this, &swInstructionsPageWidget::PlaceElement);
	connect(mPlacementWidget, &swInstructionsPlacementWidget::NudgeSelection, this, &swInstructionsPageWidget::NudgeSelection);

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

// use to free the unowned page scene

swInstructionsPageWidget::~swInstructionsPageWidget()
{
	QGraphicsScene* Scene = scene();

	// no selection signals while freeing

	disconnect(Scene, nullptr, this, nullptr);
	setScene(nullptr);
	delete Scene;

	// closed instructions keep nothing

	swClearInstructionsImageCache();

	// return the freed pages memory

#if defined(__GLIBC__)
	malloc_trim(0);
#endif
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

	// panels change after the click so nothing shifts under it
	if (QApplication::mouseButtons() & Qt::LeftButton)
	{
		mSelectionPending = true;
		return;
	}

	mSelectionPending = false;

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

// use to get a band property of a band
static swInstructionsPropertyType swGetBandType(swInstructionsPropertyType First, int Band)
{
	return static_cast<swInstructionsPropertyType>(static_cast<int>(First) + Band);
}

// use to get a band rectangle from the paper edge, empty when hidden
static QRectF swGetBandRect(const swInstructions* Instructions, size_t PageIndex, int Band)
{
	const swInstructionsPageType Type = Instructions->mPages[PageIndex].Type;

	if ((Type != swInstructionsPageType::Steps && Type != swInstructionsPageType::Blank) || !Instructions->GetPageBoolProperty(PageIndex, swGetBandType(swInstructionsPropertyType::ShowHeaderBand, Band)))
		return QRectF();

	const qreal Size = Instructions->GetPageIntProperty(PageIndex, swGetBandType(swInstructionsPropertyType::HeaderBandSize, Band));
	const qreal Width = Instructions->mPageSetup.Width;
	const qreal Height = Instructions->mPageSetup.Height;

	switch (Band)
	{
	case 0:
		return QRectF(0.0, 0.0, Width, Size);

	case 1:
		return QRectF(0.0, Height - Size, Width, Size);

	case 2:
		return QRectF(0.0, 0.0, Size, Height);
	}

	return QRectF(Width - Size, 0.0, Size, Height);
}

// use to get the band area inside its margin, border and padding
static QRectF swGetBandContentRect(const swInstructions* Instructions, size_t PageIndex, int Band)
{
	const QRectF BandRect = swGetBandRect(Instructions, PageIndex, Band);

	if (BandRect.isNull())
		return BandRect;

	const qreal Inset = Instructions->GetPageBoxStyle(PageIndex, static_cast<swInstructionsBoxElement>(static_cast<int>(swInstructionsBoxElement::HeaderBand) + Band)).GetInset();

	return BandRect.adjusted(Inset, Inset, -Inset, -Inset);
}

// use to get the page area bands leave free
static QRectF swGetContentRect(const swInstructions* Instructions, size_t PageIndex, const QRectF& MarginsRect)
{
	QRectF Rect = MarginsRect;

	for (int Band = 0; Band < 4; Band++)
	{
		const QRectF BandRect = swGetBandRect(Instructions, PageIndex, Band);

		if (BandRect.isNull() || !Instructions->GetPageBoolProperty(PageIndex, swGetBandType(swInstructionsPropertyType::HeaderBandReserve, Band)))
			continue;

		// only a band past the margin pushes content
		if (Band == 0)
			Rect.setTop(qMax(Rect.top(), BandRect.bottom()));
		else if (Band == 1)
			Rect.setBottom(qMin(Rect.bottom(), BandRect.top()));
		else if (Band == 2)
			Rect.setLeft(qMax(Rect.left(), BandRect.right()));
		else
			Rect.setRight(qMin(Rect.right(), BandRect.left()));
	}

	return Rect;
}

void swInstructionsPageWidget::PlaceElement(int Placement)
{
	const QList<QGraphicsItem*> SelectedItems = scene()->selectedItems();
	swInstructionsElementKey Key;

	if (SelectedItems.size() != 1 || !mPage || !swGetElementKey(SelectedItems.front(), Key) || mInstructions->IsElementLocked(Key))
		return;

	// page cells, outer ones on the paper edge

	const swInstructionsPageScope PageScope(mInstructions, static_cast<size_t>(mPage - mInstructions->mPages.data()));
	const swInstructionsPageSetup& PageSetup = mInstructions->mPageSetup;
	const QRectF MarginsRect(PageSetup.MarginLeft, PageSetup.MarginTop, PageSetup.Width - PageSetup.MarginLeft - PageSetup.MarginRight, PageSetup.Height - PageSetup.MarginTop - PageSetup.MarginBottom);
	QGraphicsItem* Item = SelectedItems.front();
	const QRectF ItemRect = Item->mapRectToParent(Item->boundingRect());
	const int Id = static_cast<int>(std::get<2>(Key));

	// a pointer tip goes free on the cell point
	if (std::get<0>(Key) == swInstructionsPlacementElement::Pointer)
	{
		mInstructions->SetPointerTarget(Id, swInstructionsPointerTarget::Point, swInstructionsElementKey(), -1, swGetPagePlacementPosition(mInstructions, MarginsRect, QSizeF(), Placement), QPointF(-1.0, -1.0));
		return;
	}

	if (std::get<0>(Key) != swInstructionsPlacementElement::Notice)
		return;
	const swInstructionsNotice* Notice = mInstructions->FindNotice(Id);
	const size_t PageIndex = static_cast<size_t>(mPage - mInstructions->mPages.data());
	const int Band = Notice ? static_cast<int>(Notice->Pages) - static_cast<int>(swInstructionsImagePages::Header) : -1;

	// band notices stay in their band
	if (Band >= 0 && Band < 4)
	{
		const QRectF BandRect = swGetBandContentRect(mInstructions, PageIndex, Band);

		if (!BandRect.isNull())
			mInstructions->SetNoticePosition(Id, Item->pos() + swGetPlacementPosition(BandRect, ItemRect.size(), swGetValidPlacement(Placement, true)) - ItemRect.topLeft() - BandRect.topLeft());

		return;
	}

	const QPointF Distance = swGetPagePlacementPosition(mInstructions, MarginsRect, ItemRect.size(), Placement) - ItemRect.topLeft();

	mInstructions->SetNoticePosition(Id, Item->pos() + Distance);
}

// use to fill the page codes of a text
static QString swFillPageCodes(const swInstructions* Instructions, size_t PageIndex, QString Text)
{
	return Instructions->FillModelCodes(Text.replace(QLatin1String("%page"), swGetPageNumberText(Instructions, PageIndex)));
}

// use to add a step area and its elements
static void swAddStepArea(swInstructions* Instructions, const swInstructionsStep& Step, QGraphicsItem* ParentItem, const QRectF& CellRect, const QMarginsF& DividerMargins)
{
	// the step area is a box holding its elements
	const swInstructionsBoxStyle AreaStyle = Instructions->GetBoxStyle(swInstructionsBoxElement::StepArea, Step.Model, Step.Step);
	const qreal AreaInset = AreaStyle.GetInset();
	swInstructionsFrameItem* StepAreaItem = new swInstructionsFrameItem(QRectF(QPointF(0.0, 0.0), CellRect.size()), AreaStyle, ParentItem);
	StepAreaItem->setPos(CellRect.topLeft());
	StepAreaItem->setData(SW_OBJECT_DATA, static_cast<int>(swInstructionsPageObject::StepArea));

	swInstructionsStepImageItem* StepImageItem = new swInstructionsStepImageItem(StepAreaItem, Instructions, Step.Model, Step.Step);
	const qreal ImageInset = StepImageItem->GetBoxStyle().GetInset();

	swInstructionsStepNumberItem* StepNumberItem = new swInstructionsStepNumberItem(StepAreaItem, Instructions, Step.Model, Step.Step);
	StepNumberItem->Update();

	// place step elements in the split area
	QRectF AreaRect(QPointF(AreaInset, AreaInset), CellRect.size() - QSizeF(2.0 * AreaInset, 2.0 * AreaInset));
	AreaRect = AreaRect.marginsRemoved(DividerMargins);

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

	// an anchored pair sits in the area as one block
	QRectF NumberRect(swGetPlacementPosition(AreaRect, NumberSize, NumberPlacement), NumberSize);
	QRectF PartsRect(swGetPlacementPosition(AreaRect, PartsSize, PartsPlacement), PartsSize);

	if (NumberAnchor == swInstructionsPlacementAnchor::PartsList || PartsAnchor == swInstructionsPlacementAnchor::StepNumber)
	{
		const bool NumberFollows = NumberAnchor == swInstructionsPlacementAnchor::PartsList;
		const QRectF BaseRect(QPointF(), NumberFollows ? PartsSize : NumberSize);
		const QRectF FollowRect(swGetPlacementPosition(BaseRect, NumberFollows ? NumberSize : PartsSize, NumberFollows ? NumberPlacement : PartsPlacement), NumberFollows ? NumberSize : PartsSize);
		const QRectF BlockRect = BaseRect.united(FollowRect);
		const QPointF Shift = swGetPlacementPosition(AreaRect, BlockRect.size(), NumberFollows ? PartsPlacement : NumberPlacement) - BlockRect.topLeft();

		(NumberFollows ? PartsRect : NumberRect) = BaseRect.translated(Shift);
		(NumberFollows ? NumberRect : PartsRect) = FollowRect.translated(Shift);
	}

	// the parts strip keeps the model out

	QRectF ImageSizeRect(QPointF(AreaInset, AreaInset), CellRect.size() - QSizeF(2.0 * AreaInset, 2.0 * AreaInset));
	QRectF ImagePlaceRect = AreaRect;

	if (PartsImageItem->isVisible() && Instructions->GetBoolProperty(swInstructionsPropertyType::PLIReserveArea, Step.Model, Step.Step))
	{
		// the number joins when beside the list
		const bool NumberJoins = StepNumberItem->isVisible() && (NumberAnchor == swInstructionsPlacementAnchor::PartsList || PartsAnchor == swInstructionsPlacementAnchor::StepNumber);
		const QRectF StripRect = NumberJoins ? PartsRect.united(NumberRect) : PartsRect;

		// the side follows the area placement
		const int AreaPlacement = PartsAnchor == swInstructionsPlacementAnchor::Area ? PartsPlacement : NumberPlacement;
		const int Row = AreaPlacement / SW_PLACEMENT_GRID_SIZE;
		const int Column = AreaPlacement % SW_PLACEMENT_GRID_SIZE;

		if (Row == 1)
			ImagePlaceRect.setTop(qMax(ImagePlaceRect.top(), StripRect.bottom()));
		else if (Row == 3)
			ImagePlaceRect.setBottom(qMin(ImagePlaceRect.bottom(), StripRect.top()));
		else if (Column == 1)
			ImagePlaceRect.setLeft(qMax(ImagePlaceRect.left(), StripRect.right()));
		else if (Column == 3)
			ImagePlaceRect.setRight(qMin(ImagePlaceRect.right(), StripRect.left()));

		ImageSizeRect = ImageSizeRect.intersected(ImagePlaceRect);
	}

	new swInstructionsGuideItem(AreaRect, StepAreaItem);

	if (ImagePlaceRect != AreaRect)
		new swInstructionsGuideItem(ImagePlaceRect, StepAreaItem);

	StepImageItem->SetImageSize(qMax(static_cast<int>(ImageSizeRect.width() - 2.0 * ImageInset), 1), qMax(static_cast<int>(ImageSizeRect.height() - 2.0 * ImageInset), 1));
	StepImageItem->Update();

	const int ImagePlacement = swGetValidPlacement(Instructions->GetIntProperty(swInstructionsPropertyType::StepImagePlacement, Step.Model, Step.Step), true);
	swPlaceElement(StepImageItem, Instructions, swGetPlacementPosition(ImagePlaceRect, StepImageItem->boundingRect().size(), ImagePlacement));

	// place moved and locked elements first
	if (NumberAnchor == swInstructionsPlacementAnchor::Area)
		swPlaceElement(StepNumberItem, Instructions, NumberRect.topLeft());

	if (PartsAnchor == swInstructionsPlacementAnchor::Area)
		swPlaceElement(PartsImageItem, Instructions, PartsRect.topLeft());

	if (NumberAnchor == swInstructionsPlacementAnchor::PartsList)
		swPlaceElement(StepNumberItem, Instructions, swGetPlacementPosition(QRectF(PartsImageItem->pos(), PartsSize), NumberSize, NumberPlacement));

	if (PartsAnchor == swInstructionsPlacementAnchor::StepNumber)
		swPlaceElement(PartsImageItem, Instructions, swGetPlacementPosition(QRectF(StepNumberItem->pos(), NumberSize), PartsSize, PartsPlacement));
}

// use to add the page elements
static void swAddPageContent(swInstructions* Instructions, const swInstructionsPage* Page, QGraphicsScene* Scene, QGraphicsRectItem* PageItem, const QRectF& MarginsRect)
{
	if (Page->Type == swInstructionsPageType::Cover || Page->Type == swInstructionsPageType::BackCover)
	{
		const bool BackCover = Page->Type == swInstructionsPageType::BackCover;
		swModel* Model = Page->Model;

		struct swCoverText
		{
			swInstructionsCoverTextItem* Item;
			int Placement;
			QSizeF Size;
		};

		std::vector<swCoverText> CoverTexts;

		auto AddCoverText = [&](const QString& Text, const QFont& Font, const QColor& Color, swInstructionsPlacementElement Element, swInstructionsPropertyType PlacementType)
		{
			const int Placement = qBound(0, Instructions->GetIntProperty(PlacementType, Model, 0), SW_PLACEMENT_COUNT - 1);

			// wrapped lines follow the alignment property
			swInstructionsCoverTextItem* Item = new swInstructionsCoverTextItem(PageItem, Instructions, Model, Element);
			Item->setFont(Font);
			Item->setDefaultTextColor(Color);
			Item->setPlainText(Text);

			const qreal TextWidth = MarginsRect.width() - 2.0 * Item->GetBoxStyle().GetInset();

			if (Item->document()->idealWidth() > TextWidth)
				Item->setTextWidth(TextWidth);

			CoverTexts.push_back({ Item, Placement, Item->boundingRect().size() });
		};

		// each cover has its own elements
		for (const swInstructionsPublicationElement& Publication : swGetPublicationElements())
		{
			if (!Instructions->GetBoolProperty(Publication.Show, Model, 0) || Publication.Back != BackCover)
				continue;

			const QString Text = Instructions->FillModelCodes(Instructions->GetTypedText(swGetPublicationType(Publication, swInstructionsPublicationField::Text), Instructions->GetModelFieldText(Model, Publication.Element))).trimmed();

			if (!Text.isEmpty())
				AddCoverText(Text, Instructions->GetFontProperty(swGetPublicationType(Publication, swInstructionsPublicationField::Font), Model, 0), Instructions->GetColorProperty(swGetPublicationType(Publication, swInstructionsPublicationField::Color), Model, 0), Publication.Element, swGetPublicationType(Publication, swInstructionsPublicationField::Placement));
		}

		// each text sits on its own cell, so toggling one never moves another
		qreal TopHeight = 0.0;
		qreal BottomHeight = 0.0;

		for (const swCoverText& CoverText : CoverTexts)
		{
			swPlaceElement(CoverText.Item, Instructions, swGetPagePlacementPosition(Instructions, MarginsRect, CoverText.Size, CoverText.Placement));

			// the model image leaves room for the inner rows
			const int Row = CoverText.Placement / SW_PLACEMENT_GRID_SIZE;

			if (Row == 1)
				TopHeight = qMax(TopHeight, CoverText.Size.height());
			else if (Row == SW_PLACEMENT_GRID_SIZE - 2)
				BottomHeight = qMax(BottomHeight, CoverText.Size.height());
		}

		if (BackCover)
			return;

		// place the finished model on the cover
		const QRectF ImageRect = MarginsRect.adjusted(0.0, TopHeight, 0.0, -BottomHeight);
		const swStep LastStep = Model->GetLastStep();

		const qreal ImageInset = Instructions->GetBoxStyle(swInstructionsBoxElement::CoverImage, Model, LastStep).GetInset();

		if (ImageRect.height() > 2.0 * ImageInset + 1.0 && Instructions->GetBoolProperty(swInstructionsPropertyType::ShowCoverImage, Model, LastStep))
		{
			Scene->addRect(ImageRect, QPen(Qt::NoPen), QBrush(Instructions->GetColorProperty(swInstructionsPropertyType::StepBackgroundColor, Model, LastStep)))->setParentItem(PageItem);

			swInstructionsStepImageItem* ModelImageItem = new swInstructionsStepImageItem(PageItem, Instructions, Model, LastStep);
			ModelImageItem->SetCoverImage();
			ModelImageItem->SetImageSize(qMax(static_cast<int>(ImageRect.width() - 2.0 * ImageInset), 1), static_cast<int>(ImageRect.height() - 2.0 * ImageInset));
			ModelImageItem->Update();

			const int Placement = swGetValidPlacement(Instructions->GetIntProperty(swInstructionsPropertyType::CoverImagePlacement, Model, LastStep), true);
			swPlaceElement(ModelImageItem, Instructions, swGetPlacementPosition(ImageRect, ModelImageItem->boundingRect().size(), Placement));
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
			const int TitlePlacement = swGetValidPlacement(Instructions->GetIntProperty(swInstructionsPropertyType::BillOfMaterialsTitlePlacement, Page->Model, 0), true);
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
		const qreal Inset = 2.0 * Instructions->GetBoxStyle(swInstructionsBoxElement::BillOfMaterialsList, Page->Model, 0).GetInset();
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

				swInstructionsDividerItem* DividerItem = new swInstructionsDividerItem(Rect, Rounded, PageItem);
				DividerItem->setPen(Qt::NoPen);
				DividerItem->setBrush(DividerBrush);
				DividerItem->setData(SW_OBJECT_DATA, static_cast<int>(swInstructionsPageObject::Divider));
			};

			// round ends stay inside the margins

			const qreal Inset = Instructions->GetIntProperty(swInstructionsPropertyType::StepDividerMargin, FirstStep.Model, FirstStep.Step) + (Rounded ? DividerWidth / 2.0 : 0.0);

			DividerInset = DividerWidth / 2.0 + Instructions->GetIntProperty(swInstructionsPropertyType::StepDividerPadding, FirstStep.Model, FirstStep.Step);

			const bool Horizontal = Instructions->GetBoolProperty(swInstructionsPropertyType::StepDividerHorizontal, FirstStep.Model, FirstStep.Step);
			const bool Vertical = Instructions->GetBoolProperty(swInstructionsPropertyType::StepDividerVertical, FirstStep.Model, FirstStep.Step);
			const bool FinalModelDividers = Instructions->GetBoolProperty(swInstructionsPropertyType::StepDividerFinalModel, FirstStep.Model, FirstStep.Step);

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

					// the final model border already splits

					if (!FinalModelDividers && (Step.FinalModel || Other.FinalModel))
						continue;

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
			// the finished main model has its own box
			const bool Completion = Step.Model == Instructions->GetMainModel();
			const swInstructionsBoxStyle FinalStyle = Instructions->GetBoxStyle(Completion ? swInstructionsBoxElement::Completion : swInstructionsBoxElement::FinalModel, Step.Model, Step.Step);
			const QRectF FinalRect = CellRect.adjusted(FinalStyle.GetInset(), FinalStyle.GetInset(), -FinalStyle.GetInset(), -FinalStyle.GetInset());

			new swInstructionsGuideItem(FinalRect, PageItem);

			swInstructionsFrameItem* FinalFrameItem = new swInstructionsFrameItem(CellRect, FinalStyle, PageItem);
			FinalFrameItem->setZValue(-1.0);
			FinalFrameItem->setData(SW_OBJECT_DATA, static_cast<int>(Completion ? swInstructionsPageObject::Completion : swInstructionsPageObject::FinalModel));

			swInstructionsStepImageItem* FinalImageItem = new swInstructionsStepImageItem(PageItem, Instructions, Step.Model, Step.Step);
			const qreal FinalInset = FinalImageItem->GetBoxStyle().GetInset();

			FinalImageItem->SetFinalModel();
			FinalImageItem->SetImageSize(qMax(static_cast<int>(FinalRect.width() - 2.0 * FinalInset), 1), qMax(static_cast<int>(FinalRect.height() - 2.0 * FinalInset), 1));
			FinalImageItem->Update();
			FinalImageItem->setPos(FinalRect.center() - FinalImageItem->boundingRect().center());

			continue;
		}

		// dividers inset the inner edges
		const QMarginsF DividerMargins(Step.Rect.left() > 0.0001 ? DividerInset : 0.0, Step.Rect.top() > 0.0001 ? DividerInset : 0.0, Step.Rect.right() < 0.9999 ? DividerInset : 0.0, Step.Rect.bottom() < 0.9999 ? DividerInset : 0.0);

		swAddStepArea(Instructions, Step, PageItem, CellRect, DividerMargins);
	}
}

// use to add notices and pointers
static void swAddPageAnnotations(swInstructions* Instructions, size_t PageIndex, QGraphicsScene* Scene, QGraphicsRectItem* PageItem, const QRectF& MarginsRect, const QRectF& ContentRect)
{
	const swInstructionsPageKey PageKey = Instructions->GetPageElementsKey(PageIndex);
	const swInstructionsPageType PageType = Instructions->mPages[PageIndex].Type;
	const bool BandPage = PageType == swInstructionsPageType::Steps || PageType == swInstructionsPageType::Blank;

	for (const swInstructionsNotice& Notice : Instructions->GetNotices())
	{
		// band notices follow their band
		const int Band = static_cast<int>(Notice.Pages) - static_cast<int>(swInstructionsImagePages::Header);
		const QRectF BandRect = BandPage && Band >= 0 && Band < 4 ? swGetBandContentRect(Instructions, PageIndex, Band) : QRectF();
		bool Shown = BandPage && !BandRect.isNull();

		if (Notice.Pages == swInstructionsImagePages::None)
			Shown = Notice.PageKey == PageKey;
		else if (Notice.Pages == swInstructionsImagePages::Watermark)
			Shown = BandPage && Instructions->GetPageBoolProperty(PageIndex, swInstructionsPropertyType::ShowWatermark);
		else if (Notice.Pages == swInstructionsImagePages::Front)
			Shown = PageType == swInstructionsPageType::Cover;
		else if (Notice.Pages == swInstructionsImagePages::Back)
			Shown = PageType == swInstructionsPageType::BackCover;

		if (!Shown)
			continue;

		swInstructionsNoticeItem* NoticeItem = new swInstructionsNoticeItem(PageItem, Instructions, Notice);
		NoticeItem->setPlainText(swFillPageCodes(Instructions, PageIndex, NoticeItem->toPlainText()));

		// watermarks wrap in their font

		if (Notice.Pages == swInstructionsImagePages::Watermark)
		{
			NoticeItem->setFont(Instructions->GetFontProperty(swInstructionsPropertyType::WatermarkFont, nullptr, 0));
			NoticeItem->setDefaultTextColor(Instructions->GetColorProperty(swInstructionsPropertyType::WatermarkTextColor, nullptr, 0));
		}

		// wrap long notices at half width
		const qreal TextWidth = BandRect.isNull() ? ContentRect.width() / 2.0 : BandRect.width();

		if (NoticeItem->document()->idealWidth() > TextWidth)
			NoticeItem->setTextWidth(TextWidth);

		if (Notice.Pages == swInstructionsImagePages::Watermark)
		{
			NoticeItem->SetBoxStyle(Instructions->GetPageBoxStyle(PageIndex, swInstructionsBoxElement::Watermark));
			NoticeItem->setZValue(-1.0);
			swRotateWatermark(NoticeItem, Instructions);
		}

		swPlaceElement(NoticeItem, Instructions, BandRect.topLeft() + Notice.Position);
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

		PointerItem->SetLocked(Instructions->mPageState.LockedPositions);
	}
}

swInstructionsImageItem::swInstructionsImageItem(QGraphicsItem* Parent, swInstructions* Instructions, const swInstructionsImage& Image)
	: swInstructionsBoxItem(Parent), mInstructions(Instructions), mId(Image.Id)
{
	setFlags(SW_MOVABLE_ITEM_FLAGS);

	// under the texts, over the model

	setZValue(0.5);
	setTransformationMode(Qt::SmoothTransformation);

	// a missing file shows a framed gap to find it
	const qreal Scale = swGetRenderScale(mInstructions);
	const qreal Width = mInstructions->mPageSetup.Width * Image.Size / 100.0;
	QImageReader Reader(mInstructions->GetFilePath(Image.File));
	const QSize FileSize = Reader.size();


	if (FileSize.isValid() && FileSize.width() > 0)
		Reader.setScaledSize(QSize(qRound(Width * Scale), qMax(qRound(Width * Scale * FileSize.height() / FileSize.width()), 1)));

	QImage Picture = Reader.read();
	SetBoxStyle(mInstructions->GetBoxStyle(swInstructionsBoxElement::Image, nullptr, 0));

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
static void swAddPageImages(swInstructions* Instructions, const swInstructionsPage* Page, size_t PageIndex, QGraphicsRectItem* PageItem, const QRectF& MarginsRect)
{
	const swInstructionsImagePages Shown = Page->Type == swInstructionsPageType::Cover ? swInstructionsImagePages::Front : swInstructionsImagePages::Back;
	const bool BandPage = Page->Type == swInstructionsPageType::Steps || Page->Type == swInstructionsPageType::Blank;

	if (!BandPage && Page->Type != swInstructionsPageType::Cover && Page->Type != swInstructionsPageType::BackCover)
		return;

	for (const swInstructionsImage& Image : Instructions->GetImages())
	{
		// bands and the watermark show on step and blank pages
		if (BandPage)
		{
			const int Band = static_cast<int>(Image.Pages) - static_cast<int>(swInstructionsImagePages::Header);
			const bool IsWatermark = Image.Pages == swInstructionsImagePages::Watermark;
			const QRectF BandRect = Band >= 0 && Band < 4 ? swGetBandContentRect(Instructions, PageIndex, Band) : QRectF();

			if (BandRect.isNull() && (!IsWatermark || !Instructions->GetPageBoolProperty(PageIndex, swInstructionsPropertyType::ShowWatermark)))
				continue;

			swInstructionsImageItem* ImageItem = new swInstructionsImageItem(PageItem, Instructions, Image);

			if (IsWatermark)
			{
				ImageItem->setOpacity(Instructions->GetPageIntProperty(PageIndex, swInstructionsPropertyType::WatermarkOpacity) / 100.0);
				ImageItem->SetBoxStyle(Instructions->GetPageBoxStyle(PageIndex, swInstructionsBoxElement::Watermark));
				swRotateWatermark(ImageItem, Instructions);
				ImageItem->setZValue(-1.0);
				swPlaceElement(ImageItem, Instructions, swGetPagePlacementPosition(Instructions, MarginsRect, ImageItem->boundingRect().size(), Image.Placement));
			}
			else
				swPlaceElement(ImageItem, Instructions, swGetPlacementPosition(BandRect, ImageItem->boundingRect().size(), swGetValidPlacement(Image.Placement, true)));

			continue;
		}

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

	QRectF MarginsRect(PageSetup.MarginLeft, PageSetup.MarginTop, PageSetup.Width - PageSetup.MarginLeft - PageSetup.MarginRight, PageSetup.Height - PageSetup.MarginTop - PageSetup.MarginBottom);

	// bands keep the full margins, content leaves their space
	const QRectF ContentRect = swGetContentRect(Instructions, PageIndex, MarginsRect);

	for (int Band = 0; Band < 4; Band++)
	{
		const QRectF BandRect = swGetBandRect(Instructions, PageIndex, Band);

		if (BandRect.isNull())
			continue;

		swInstructionsFrameItem* BandItem = new swInstructionsFrameItem(BandRect, Instructions->GetPageBoxStyle(PageIndex, static_cast<swInstructionsBoxElement>(static_cast<int>(swInstructionsBoxElement::HeaderBand) + Band)), PageItem);
		BandItem->setZValue(-2.0);
		BandItem->setData(SW_BAND_DATA, static_cast<int>(swInstructionsImagePages::Header) + Band);
		BandItem->setFlag(QGraphicsItem::ItemIsSelectable);
	}

	// number step and materials pages
	const bool PageHasContent = (Page->Type == swInstructionsPageType::Steps && !Page->Steps.empty()) || Page->Type == swInstructionsPageType::BillOfMaterials || Instructions->GetBoolProperty(swInstructionsPropertyType::ShowPageNumberAllPages, nullptr, 0);

	if (PageHasContent && Instructions->GetBoolProperty(swInstructionsPropertyType::ShowPageNumber, nullptr, 0))
	{
		swInstructionsPageNumberItem* PageNumberItem = new swInstructionsPageNumberItem(PageItem, Instructions, PageIndex);

		const int Placement = Instructions->GetIntProperty(swInstructionsPropertyType::PageNumberPlacement, nullptr, 0);
		swPlaceElement(PageNumberItem, Instructions, swGetPagePlacementPosition(Instructions, ContentRect, PageNumberItem->boundingRect().size(), Placement));
	}

	new swInstructionsGuideItem(ContentRect, PageItem);

	if (Page->Type != swInstructionsPageType::Blank)
		swAddPageContent(Instructions, Page, Scene, PageItem, ContentRect);

	if (Page->Type == swInstructionsPageType::Steps)
		swAddPageCounts(Instructions, Page, PageItem, ContentRect);

	swAddPageImages(Instructions, Page, PageIndex, PageItem, MarginsRect);

	swAddPageAnnotations(Instructions, PageIndex, Scene, PageItem, MarginsRect, ContentRect);
}

// use to draw one page into an image
QImage swRenderInstructionsPage(swInstructions* Instructions, size_t PageIndex, const QSize& Size)
{
	QImage Image(Size, QImage::Format_ARGB32_Premultiplied);
	Image.fill(Instructions->GetPageBackgroundColor(PageIndex));

	QGraphicsScene Scene;
	swUpdateInstructionsPageScene(Instructions, &Instructions->mPages[PageIndex], &Scene);

	QPainter Painter(&Image);
	Painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);
	Scene.render(&Painter, QRectF(QPointF(), QSizeF(Image.size())), Scene.sceneRect(), Qt::KeepAspectRatio);

	return Image;
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

// use to gather element boxes and layout areas
static void swCollectBoundaries(const QGraphicsScene* Scene, const QTransform& Offset, std::vector<swInstructionsBoundaryBox>& Boxes, std::vector<QPolygonF>& LayoutEdges)
{
	for (const QGraphicsItem* Item : Scene->items(Qt::AscendingOrder))
	{
		if (!Item->isVisible())
			continue;

		const QTransform Transform = Item->sceneTransform() * Offset;

		if (const swInstructionsGuideItem* GuideItem = dynamic_cast<const swInstructionsGuideItem*>(Item))
			LayoutEdges.push_back(Transform.map(QPolygonF(GuideItem->GetRect())));
		else if (const swInstructionsDividerItem* DividerItem = dynamic_cast<const swInstructionsDividerItem*>(Item))
			LayoutEdges.push_back(Transform.map(QPolygonF(DividerItem->rect())));
		else if (const swInstructionsFrameItem* FrameItem = dynamic_cast<const swInstructionsFrameItem*>(Item))
			Boxes.push_back({ Transform, FrameItem->rect(), FrameItem->GetBoxStyle() });
		else if (const swInstructionsBoxBase* BoxItem = dynamic_cast<const swInstructionsBoxBase*>(Item))
			if (!Item->boundingRect().isEmpty())
				Boxes.push_back({ Transform, Item->boundingRect(), BoxItem->GetBoxStyle() });
	}
}

// a drawn copy of the paired page, clicked to edit it

class swInstructionsSpreadItem : public QGraphicsItem
{
public:
	swInstructionsSpreadItem(const QPicture& Picture, const QRectF& Rect, int Row)
		: mPicture(Picture), mRect(Rect), mRow(Row)
	{
	}

	int GetRow() const
	{
		return mRow;
	}

	QRectF boundingRect() const override
	{
		return mRect;
	}

	void paint(QPainter* Painter, const QStyleOptionGraphicsItem* Option, QWidget* Widget) override
	{
		Q_UNUSED(Option);
		Q_UNUSED(Widget);

		Painter->drawPicture(mRect.topLeft(), mPicture);
	}

protected:
	QPicture mPicture;
	QRectF mRect;
	int mRow;
};

void swInstructionsPageWidget::SetPageSpread(swInstructionsPageSpread PageSpread)
{
	if (mPageSpread == PageSpread)
		return;

	mPageSpread = PageSpread;

	if (mPage)
		SetCurrentPage(mPage);
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
	mPairBoxes.clear();
	mPairLayoutEdges.clear();
	mPage = Page;
	mRebuildingPage = true;
	swUpdateInstructionsPageScene(mInstructions, Page, Scene);

	// pairs are 1 and 2, 3 and 4, the edited page stays at the origin

	const size_t PageIndex = Page ? static_cast<size_t>(Page - mInstructions->mPages.data()) : 0;
	const size_t PairIndex = PageIndex ^ 1;

	if (Page && mPageSpread != swInstructionsPageSpread::Single && PairIndex < mInstructions->mPages.size())
	{
		QGraphicsScene PairScene;
		swUpdateInstructionsPageScene(mInstructions, &mInstructions->mPages[PairIndex], &PairScene);

		const QRectF PageRect = Scene->sceneRect();
		const bool SideBySide = mPageSpread == swInstructionsPageSpread::SideBySide;
		const qreal Gap = qMin(PageRect.width(), PageRect.height()) * 0.04;
		const qreal Direction = PairIndex > PageIndex ? 1.0 : -1.0;
		const QRectF PairRect = PageRect.translated(SideBySide ? Direction * (PageRect.width() + Gap) : 0.0, SideBySide ? 0.0 : Direction * (PageRect.height() + Gap));

		QPicture Picture;
		QPainter Painter(&Picture);
		PairScene.render(&Painter, QRectF(QPointF(0.0, 0.0), PageRect.size()), PairScene.sceneRect());
		Painter.end();

		Scene->addItem(new swInstructionsSpreadItem(Picture, PairRect, static_cast<int>(PairIndex)));
		swCollectBoundaries(&PairScene, QTransform::fromTranslate(PairRect.left(), PairRect.top()), mPairBoxes, mPairLayoutEdges);
		Scene->setSceneRect(PageRect.united(PairRect));
	}

	FitPage();

	// select a new pointer alone
	if (!mPendingSelection.empty())
	{
		SelectedKeys = std::set<swInstructionsElementKey>(mPendingSelection.begin(), mPendingSelection.end());
		mPendingSelection.clear();
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

// use to find a resize edge under the pointer
int swInstructionsPageWidget::FindResizeHandle(const QPoint& Position, QGraphicsItem** HandleItem) const
{
	// locked pages keep every box
	if (!mPage || mInstructions->IsPageLocked(static_cast<size_t>(mPage - mInstructions->mPages.data())))
		return 0;

	for (QGraphicsItem* Item : scene()->selectedItems())
	{
		const swInstructionsBoxBase* Box = dynamic_cast<const swInstructionsBoxBase*>(Item);

		if (!Box || !Box->GetBoxSize().isValid())
			continue;

		// edges grab within a few pixels
		const QRect ViewRect = mapFromScene(Item->sceneBoundingRect()).boundingRect();
		const int Reach = 6;

		if (!ViewRect.adjusted(-Reach, -Reach, Reach, Reach).contains(Position))
			continue;

		const int Handle = (qAbs(Position.x() - ViewRect.right()) <= Reach ? 1 : 0) | (qAbs(Position.y() - ViewRect.bottom()) <= Reach ? 2 : 0) | (qAbs(Position.x() - ViewRect.left()) <= Reach ? 4 : 0) | (qAbs(Position.y() - ViewRect.top()) <= Reach ? 8 : 0);

		if (!Handle)
			continue;

		if (HandleItem)
			*HandleItem = Item;

		return Handle;
	}

	return 0;
}

void swInstructionsPageWidget::mousePressEvent(QMouseEvent* Event)
{
	// the paired page becomes the edited one

	if (const swInstructionsSpreadItem* SpreadItem = dynamic_cast<const swInstructionsSpreadItem*>(itemAt(Event->pos())))
	{
		mDragStarted = false;
		mResizeItem = nullptr;
		emit PageClicked(SpreadItem->GetRow());
		Event->accept();
		return;
	}

	mPressPosition = Event->pos();
	mDragStarted = false;

	// a sized box handle starts a resize
	mResizeItem = nullptr;
	mResizeHandle = Event->button() == Qt::LeftButton ? FindResizeHandle(Event->pos(), &mResizeItem) : 0;

	if (mResizeHandle)
	{
		mResizeStartPosition = mResizeItem->pos();
		mResizeStartRect = mResizeItem->sceneBoundingRect();
		mDragItems.clear();
		Event->accept();
		return;
	}

	mResizeItem = nullptr;

	QGraphicsView::mousePressEvent(Event);

	// the selection moves with a drag
	mDragItems.clear();

	if (Event->button() != Qt::LeftButton)
		return;

	const QList<QGraphicsItem*> SelectedItems = scene()->selectedItems();

	// plain objects and blank spots show their properties

	if (SelectedItems.isEmpty() && mPage)
	{
		const QRect HitRect(Event->pos() - QPoint(4, 4), QSize(9, 9));
		std::optional<swInstructionsPageObject> Object;

		// thin dividers win over the areas they cross

		for (const QGraphicsItem* Item : items(HitRect))
		{
			if (!Item->data(SW_OBJECT_DATA).isValid())
				continue;

			const swInstructionsPageObject ItemObject = static_cast<swInstructionsPageObject>(Item->data(SW_OBJECT_DATA).toInt());

			if (!Object || ItemObject == swInstructionsPageObject::Divider)
				Object = ItemObject;

			if (ItemObject == swInstructionsPageObject::Divider)
				break;
		}

		mPropertiesWidget->PageClicked(mPage->Type, Object);
	}

	for (QGraphicsItem* Item : SelectedItems)
		if (Item->flags() & QGraphicsItem::ItemIsMovable)
			mDragItems.emplace_back(Item, Item->pos());
}

void swInstructionsPageWidget::mouseMoveEvent(QMouseEvent* Event)
{
	// a click only drags once moved
	if ((Event->buttons() & Qt::LeftButton) && !mDragStarted)
	{
		if ((Event->pos() - mPressPosition).manhattanLength() < QApplication::startDragDistance())
			return;

		mDragStarted = true;

		// the grabbed thing sets the cursor
		const int Handle = FindResizeHandle(mPressPosition, nullptr);
		Qt::CursorShape Cursor = Qt::ArrowCursor;

		if (Handle == 3 || Handle == 12)
			Cursor = Qt::SizeFDiagCursor;
		else if (Handle == 6 || Handle == 9)
			Cursor = Qt::SizeBDiagCursor;
		else if (Handle == 1 || Handle == 4)
			Cursor = Qt::SizeHorCursor;
		else if (Handle == 2 || Handle == 8)
			Cursor = Qt::SizeVerCursor;
		else if (mPage && !mInstructions->IsPageLocked(static_cast<size_t>(mPage - mInstructions->mPages.data())))
		{
			// movable elements show the move pointer
			for (QGraphicsItem* Item : items(mPressPosition))
			{
				const swInstructionsPointerItem* PointerItem = dynamic_cast<const swInstructionsPointerItem*>(Item);

				const QPointF ItemPosition = Item->mapFromScene(mapToScene(mPressPosition));

				if (PointerItem && PointerItem->IsBendAt(ItemPosition))
				{
					Cursor = PointerItem->IsBendHorizontal() ? Qt::SizeHorCursor : Qt::SizeVerCursor;
					break;
				}

				if ((Item->flags() & QGraphicsItem::ItemIsMovable) || (PointerItem && (PointerItem->IsTipAt(ItemPosition) || PointerItem->IsBaseAt(ItemPosition))))
				{
					Cursor = Qt::SizeAllCursor;
					break;
				}
			}
		}

		viewport()->setCursor(Cursor);
	}
	else if (!(Event->buttons() & Qt::LeftButton))
		viewport()->setCursor(Qt::ArrowCursor);

	if (mResizeItem)
	{
		// grabbed edges follow the pointer
		swInstructionsBoxBase* Box = dynamic_cast<swInstructionsBoxBase*>(mResizeItem);
		const QPointF Pointer = mapToScene(Event->pos());
		const qreal MinimumSize = 2.0 * Box->GetBoxStyle().GetInset() + 4.0;
		QRectF Rect = mResizeStartRect;

		if (mResizeHandle & 1)
			Rect.setRight(qMax(Rect.left() + MinimumSize, Pointer.x()));

		if (mResizeHandle & 2)
			Rect.setBottom(qMax(Rect.top() + MinimumSize, Pointer.y()));

		if (mResizeHandle & 4)
			Rect.setLeft(qMin(Rect.right() - MinimumSize, Pointer.x()));

		if (mResizeHandle & 8)
			Rect.setTop(qMin(Rect.bottom() - MinimumSize, Pointer.y()));

		// images keep their shape
		if (dynamic_cast<swInstructionsImageItem*>(mResizeItem) && mResizeStartRect.width() > 0.0)
		{
			const qreal Aspect = mResizeStartRect.height() / mResizeStartRect.width();
			const QSizeF Size = (mResizeHandle & 5) ? QSizeF(Rect.width(), Rect.width() * Aspect) : QSizeF(Rect.height() / Aspect, Rect.height());

			if (mResizeHandle & 4)
				Rect.setLeft(Rect.right() - Size.width());
			else
				Rect.setRight(Rect.left() + Size.width());

			if (mResizeHandle & 8)
				Rect.setTop(Rect.bottom() - Size.height());
			else
				Rect.setBottom(Rect.top() + Size.height());
		}

		Box->SetBoxSize(Rect.size());
		mResizeItem->setPos(mResizeStartPosition + Rect.topLeft() - mResizeStartRect.topLeft());
		return;
	}

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

	// built in elements hide and reset
	std::map<swInstructionsPlacementElement, swInstructionsPropertyType> ShowTypes = {
		{ swInstructionsPlacementElement::StepNumber, swInstructionsPropertyType::ShowStepNumber },
		{ swInstructionsPlacementElement::PartsList, swInstructionsPropertyType::ShowStepPLI },
		{ swInstructionsPlacementElement::PartsListCount, swInstructionsPropertyType::PLIShowCounts },
		{ swInstructionsPlacementElement::CoverImage, swInstructionsPropertyType::ShowCoverImage },
		{ swInstructionsPlacementElement::BillOfMaterials, swInstructionsPropertyType::ShowBillOfMaterials },
		{ swInstructionsPlacementElement::PageNumber, swInstructionsPropertyType::ShowPageNumber },
		{ swInstructionsPlacementElement::BillOfMaterialsPartCount, swInstructionsPropertyType::ShowBillOfMaterialsPartCount },
		{ swInstructionsPlacementElement::PagePartCount, swInstructionsPropertyType::ShowPagePartCount },
		{ swInstructionsPlacementElement::SubmodelPreview, swInstructionsPropertyType::ShowSubmodelPreview },
		{ swInstructionsPlacementElement::SubmodelPreviewPartCount, swInstructionsPropertyType::ShowSubmodelPreview }
	};

	for (const swInstructionsPublicationElement& Publication : swGetPublicationElements())
		ShowTypes[Publication.Element] = Publication.Show;

	for (const swInstructionsElementKey& Key : Keys)
	{
		const std::map<swInstructionsPlacementElement, swInstructionsPropertyType>::const_iterator Show = ShowTypes.find(std::get<0>(Key));

		if (Show != ShowTypes.end())
		{
			swInstructionsPropertyType ShowType = Show->second;

			// the bill of materials hides its own
			if (std::get<2>(Key) == 0 && ShowType == swInstructionsPropertyType::ShowStepNumber)
				ShowType = swInstructionsPropertyType::ShowBillOfMaterialsTitle;
			else if (std::get<2>(Key) == 0 && ShowType == swInstructionsPropertyType::PLIShowCounts)
				ShowType = swInstructionsPropertyType::BillOfMaterialsShowCounts;

			mInstructions->ResetElementOffset(Key);
			mInstructions->SetElementSize(Key, QSizeF());
			mInstructions->SetDefaultBool(ShowType, false);
		}
		else if (std::get<0>(Key) == swInstructionsPlacementElement::Pointer)
			mInstructions->RemovePointer(static_cast<int>(std::get<2>(Key)));
		else if (std::get<0>(Key) == swInstructionsPlacementElement::Notice)
			mInstructions->RemoveNotice(static_cast<int>(std::get<2>(Key)));
		else if (std::get<0>(Key) == swInstructionsPlacementElement::Image)
			mInstructions->RemoveImage(static_cast<int>(std::get<2>(Key)));
	}
}

// use to run through the pages
void swInstructionsPageWidget::wheelEvent(QWheelEvent* Event)
{
	// fine wheels add up to one notch

	mWheelDelta += Event->angleDelta().y();

	while (qAbs(mWheelDelta) >= 120)
	{
		const int Direction = mWheelDelta > 0 ? -1 : 1;

		mWheelDelta += Direction * 120;
		emit PageScrolled(Direction);
	}

	Event->accept();
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

	// a band under the cursor takes new notices and images
	const swInstructionsPageType PageType = mPage->Type;
	const bool BandPage = PageType == swInstructionsPageType::Steps || PageType == swInstructionsPageType::Blank;
	swInstructionsImagePages BandPages = swInstructionsImagePages::None;
	QRectF BandRect;

	for (QGraphicsItem* Item : Items)
	{
		if (Item->data(SW_BAND_DATA).isValid())
		{
			BandPages = static_cast<swInstructionsImagePages>(Item->data(SW_BAND_DATA).toInt());
			BandRect = Item->sceneBoundingRect();
			break;
		}
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

		// values follow the menu order
		auto AddChoiceMenu = [this, &Menu](const QString& Title, const QStringList& Names, const QList<int>& Values, int Current, std::function<void(int)> SetChoice) -> QMenu*
		{
			QMenu* ChoiceMenu = Menu.addMenu(Title);
			QActionGroup* ChoiceGroup = new QActionGroup(ChoiceMenu);

			for (int Index = -1; Index < Names.size(); Index++)
			{
				const int Choice = Index < 0 ? -1 : Values[Index];
				QAction* Action = ChoiceMenu->addAction(Index < 0 ? tr("Default") : Names[Index]);
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

		AddChoiceMenu(tr("Style"), { tr("Line"), tr("L Shape"), tr("Z Shape"), tr("U Shape"), tr("Callout Wedge") }, { 0, 2, 3, 4, 1 }, PointerStyle, [this, Id, PointerTip](int Style)
		{
			mInstructions->SetPointerStyle(Id, Style, PointerTip);
		});

		QMenu* TipMenu = AddChoiceMenu(tr("Tip"), { tr("Arrow"), tr("Dot"), tr("Hollow Dot") }, { 0, 1, 2 }, PointerTip, [this, Id, PointerStyle](int Tip)
		{
			mInstructions->SetPointerStyle(Id, PointerStyle, Tip);
		});

		// wedges have no tip
		const int ShownStyle = PointerStyle >= 0 ? PointerStyle : mInstructions->GetIntProperty(swInstructionsPropertyType::PointerStyle, nullptr, 0);
		TipMenu->setEnabled(ShownStyle != static_cast<int>(swInstructionsPointerStyle::Wedge));

		// only Z and U shapes have a bend to reset
		QAction* ResetBendAction = Menu.addAction(tr("Reset Bend"));
		ResetBendAction->setEnabled((ShownStyle == static_cast<int>(swInstructionsPointerStyle::ZShape) && Pointer->Bend != 0.5) || (ShownStyle == static_cast<int>(swInstructionsPointerStyle::UShape) && Pointer->Reach >= 0.0));

		connect(ResetBendAction, &QAction::triggered, this, [this, Id]()
		{
			mInstructions->SetPointerBend(Id, 0.5);
			mInstructions->SetPointerReach(Id, -1.0);
		});

		Menu.addSeparator();
		AddClipboardActions(Menu, true);

		connect(Menu.addAction(tr("Remove Pointer")), &QAction::triggered, this, [this, Id]()
		{
			mInstructions->RemovePointer(Id);
		});
	}
	else
	{
		// add items only on empty space
		if (!ElementItem)
		{
			connect(Menu.addAction(tr("Add Notice...")), &QAction::triggered, this, [this, PageIndex, ScenePosition, BandPages, BandRect]()
			{
				QString Text;
				QString File;

				if (swEditNoticeText(this, Text, File, true))
					mInstructions->AddNotice(PageIndex, ScenePosition - BandRect.topLeft(), Text, File, BandPages);
			});
		}

		if (!ElementItem && BandPage && BandPages == swInstructionsImagePages::None)
		{
			connect(Menu.addAction(tr("Add Watermark Text...")), &QAction::triggered, this, [this, PageIndex, ScenePosition]()
			{
				QString Text;
				QString File;

				if (swEditNoticeText(this, Text, File, true))
					mInstructions->AddNotice(PageIndex, ScenePosition, Text, File, swInstructionsImagePages::Watermark);
			});
		}

		// covers carry linked images, like a logo
		const QString ImageFilter = tr("Images (*.png *.jpg *.jpeg *.svg *.bmp *.gif *.webp);;All Files (*)");

		if (!ElementItem && (PageType == swInstructionsPageType::Cover || PageType == swInstructionsPageType::BackCover))
		{
			connect(Menu.addAction(tr("Add Image...")), &QAction::triggered, this, [this, PageType, ImageFilter]()
			{
				const QString FileName = QFileDialog::getOpenFileName(this, tr("Add Image"), QString(), ImageFilter);

				if (!FileName.isEmpty())
					SelectElement({ swInstructionsPlacementElement::Image, nullptr, static_cast<swStep>(mInstructions->AddImage(PageType, FileName)) });
			});
		}
		else if (!ElementItem && BandPage)
		{
			// images show in the band or as the watermark
			const swInstructionsImagePages ImagePages = BandPages == swInstructionsImagePages::None ? swInstructionsImagePages::Watermark : BandPages;

			connect(Menu.addAction(BandPages == swInstructionsImagePages::None ? tr("Add Watermark Image...") : tr("Add Image...")), &QAction::triggered, this, [this, PageType, ImageFilter, ImagePages]()
			{
				const QString FileName = QFileDialog::getOpenFileName(this, tr("Add Image"), QString(), ImageFilter);

				if (FileName.isEmpty())
					return;

				const int ImageId = mInstructions->AddImage(PageType, FileName);
				swInstructionsImage Changed = *mInstructions->FindImage(ImageId);
				Changed.Pages = ImagePages;
				mInstructions->SetImage(Changed);
				SelectElement({ swInstructionsPlacementElement::Image, nullptr, static_cast<swStep>(ImageId) });
			});
		}

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

		// copies and remove sit under add
		AddClipboardActions(Menu, Element == swInstructionsPlacementElement::Notice || Element == swInstructionsPlacementElement::Image);

		if (ElementItem)
		{
			const QString RemoveName = Element == swInstructionsPlacementElement::Notice ? tr("Remove Notice") : Element == swInstructionsPlacementElement::Image ? tr("Remove Image") : tr("Remove Element");

			connect(Menu.addAction(RemoveName), &QAction::triggered, this, &swInstructionsPageWidget::RemoveSelectedElements);
		}

		const swInstructionsImage* Image = Element == swInstructionsPlacementElement::Image ? mInstructions->FindImage(Id) : nullptr;
		const swInstructionsImage Current = Image ? *Image : swInstructionsImage();

		// where it shows and aligns
		Menu.addSeparator();

		if (Element == swInstructionsPlacementElement::Notice)
		{
			const swInstructionsNotice* Notice = mInstructions->FindNotice(Id);

			if (Notice && BandPage)
			{
				QMenu* NoticePagesMenu = Menu.addMenu(tr("Show On"));
				QActionGroup* NoticePagesGroup = new QActionGroup(NoticePagesMenu);
				const swInstructionsImagePages CurrentPages = Notice->Pages;
				const QPointF PagePosition = ElementItem->pos();
				const std::pair<swInstructionsImagePages, QString> NoticePagesNames[] =
				{
					{ swInstructionsImagePages::None, tr("This Page") },
					{ swInstructionsImagePages::Header, tr("Header Band") },
					{ swInstructionsImagePages::Footer, tr("Footer Band") },
					{ swInstructionsImagePages::Left, tr("Left Band") },
					{ swInstructionsImagePages::Right, tr("Right Band") },
					{ swInstructionsImagePages::Watermark, tr("Watermark") }
				};

				for (const std::pair<swInstructionsImagePages, QString>& PagesName : NoticePagesNames)
				{
					if (PagesName.first == swInstructionsImagePages::Header)
						NoticePagesMenu->addSeparator();

					QAction* Action = NoticePagesMenu->addAction(PagesName.second);
					Action->setCheckable(true);
					Action->setChecked(CurrentPages == PagesName.first);
					NoticePagesGroup->addAction(Action);

					const swInstructionsImagePages Pages = PagesName.first;

					connect(Action, &QAction::triggered, this, [this, Id, Pages, PageIndex, PagePosition]()
					{
						// bands start at their corner
						const bool InBand = Pages != swInstructionsImagePages::None && Pages != swInstructionsImagePages::Watermark;
						mInstructions->SetNoticePages(Id, Pages, PageIndex, InBand ? QPointF() : PagePosition);
					});
				}
			}

			// a notice can align on its own
			if (const swInstructionsNotice* Notice = mInstructions->FindNotice(Id))
			{
				QMenu* AlignMenu = Menu.addMenu(tr("Align"));
				const bool Sized = mInstructions->GetElementSize(Key).isValid();
				int Alignment = 0;
				int VerticalAlignment = 1;
				swGetElementAlignment(mInstructions, Key, Alignment, VerticalAlignment);

				QAction* DefaultAction = AlignMenu->addAction(tr("Default"));
				DefaultAction->setCheckable(true);
				DefaultAction->setChecked(Notice->Alignment < 0);

				connect(DefaultAction, &QAction::triggered, this, [this, Id]()
				{
					mInstructions->SetNoticeAlignment(Id, -1, -1);
				});

				AlignMenu->addSeparator();

				const QStringList AlignNames = { tr("Left"), tr("Center"), tr("Right"), tr("Top"), tr("Middle"), tr("Bottom") };

				for (int Choice = 0; Choice < 6; Choice++)
				{
					if (Choice == 3)
						AlignMenu->addSeparator();

					const bool Vertical = Choice >= 3;
					QAction* Action = AlignMenu->addAction(AlignNames[Choice]);
					Action->setCheckable(true);
					Action->setChecked(Notice->Alignment >= 0 && (Vertical ? VerticalAlignment == Choice - 3 : Alignment == Choice));
					Action->setEnabled(!Vertical || Sized);

					connect(Action, &QAction::triggered, this, [this, Id, Vertical, Choice, Alignment, VerticalAlignment]()
					{
						mInstructions->SetNoticeAlignment(Id, Vertical ? Alignment : Choice, Vertical ? Choice - 3 : VerticalAlignment);
					});
				}
			}
		}

		if (Image)
		{
			QMenu* PagesMenu = Menu.addMenu(tr("Show On"));
			QActionGroup* PagesGroup = new QActionGroup(PagesMenu);
			const std::pair<swInstructionsImagePages, QString> PagesNames[] =
			{
				{ swInstructionsImagePages::Front, tr("Front Cover") },
				{ swInstructionsImagePages::Back, tr("Back Cover") },
				{ swInstructionsImagePages::Both, tr("Both Covers") },
				{ swInstructionsImagePages::Header, tr("Header Band") },
				{ swInstructionsImagePages::Footer, tr("Footer Band") },
				{ swInstructionsImagePages::Left, tr("Left Band") },
				{ swInstructionsImagePages::Right, tr("Right Band") },
				{ swInstructionsImagePages::Watermark, tr("Watermark") }
			};

			for (const std::pair<swInstructionsImagePages, QString>& PagesName : PagesNames)
			{
				if (PagesName.first == swInstructionsImagePages::Header)
					PagesMenu->addSeparator();

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
		}

		// edits come last
		Menu.addSeparator();

		if (Element == swInstructionsPlacementElement::Notice)
		{
			connect(Menu.addAction(tr("Edit Notice...")), &QAction::triggered, this, [this, Id]()
			{
				const swInstructionsNotice* Notice = mInstructions->FindNotice(Id);
				QString Text = Notice ? Notice->Text : QString();
				QString File = Notice ? Notice->File : QString();

				if (Notice && swEditNoticeText(this, Text, File, false))
					mInstructions->SetNoticeText(Id, Text, File);
			});
		}

		if (Image)
		{
			connect(Menu.addAction(tr("Change Image...")), &QAction::triggered, this, [this, Current, ImageFilter]()
			{
				const QString FileName = QFileDialog::getOpenFileName(this, tr("Change Image"), mInstructions->GetFilePath(Current.File), ImageFilter);
				swInstructionsImage Changed = Current;
				Changed.File = FileName;

				if (!FileName.isEmpty())
					mInstructions->SetImage(Changed);
			});

			// size edits live in properties
			QAction* SizeAction = Menu.addAction(tr("Edit Size"));
			SizeAction->setCheckable(true);
			SizeAction->setChecked(mInstructions->GetElementSize(Key).isValid());
			const QSizeF CurrentSize = ElementItem->boundingRect().size();

			connect(SizeAction, &QAction::toggled, this, [this, Key, Current, CurrentSize](bool Checked)
			{
				// off returns the default width
				if (!Checked)
				{
					swInstructionsImage Changed = Current;
					Changed.Size = swInstructionsImage().Size;
					mInstructions->SetImage(Changed);
				}

				mInstructions->SetElementSize(Key, Checked ? CurrentSize : QSizeF());
			});
		}
	}

	// text boxes keep a size when asked
	int Alignment = 0;
	int VerticalAlignment = 1;

	if (dynamic_cast<swInstructionsBoxBase*>(ElementItem) && swGetElementAlignment(mInstructions, Key, Alignment, VerticalAlignment))
	{
		QAction* SizeAction = Menu.addAction(tr("Edit Size"));
		SizeAction->setCheckable(true);
		SizeAction->setChecked(mInstructions->GetElementSize(Key).isValid());
		const QSizeF CurrentSize = ElementItem->boundingRect().size();

		connect(SizeAction, &QAction::toggled, this, [this, Key, CurrentSize](bool Checked)
		{
			mInstructions->SetElementSize(Key, Checked ? CurrentSize : QSizeF());
		});
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
	mPendingSelection = { Key };
}

// use to list copy, paste and duplicate
void swInstructionsPageWidget::AddClipboardActions(QMenu& Menu, bool Copyable)
{
	const std::pair<QString, int> Commands[] =
	{
		{ tr("Copy"), SW_EDIT_COPY },
		{ tr("Paste"), SW_EDIT_PASTE },
		{ tr("Duplicate"), SW_PART_DUPLICATE }
	};

	// locked pages take no copies

	const bool Editable = mPage && !mInstructions->IsPageLocked(static_cast<size_t>(mPage - mInstructions->mPages.data()));

	for (const auto& [Name, CommandId] : Commands)
	{
		QAction* Action = Menu.addAction(Name);

		// the main window keys show
		if (gMainWindow)
			Action->setShortcut(gMainWindow->mActions[CommandId]->shortcut());

		Action->setEnabled(CommandId == SW_EDIT_COPY ? Copyable : Editable && (CommandId == SW_EDIT_PASTE ? CanPaste() : Copyable));

		if (CommandId == SW_EDIT_COPY)
			connect(Action, &QAction::triggered, this, &swInstructionsPageWidget::CopySelectedElements);
		else if (CommandId == SW_EDIT_PASTE)
			connect(Action, &QAction::triggered, this, &swInstructionsPageWidget::PasteElements);
		else
			connect(Action, &QAction::triggered, this, &swInstructionsPageWidget::DuplicateSelectedElements);
	}

	Menu.addSeparator();
}

// use to copy the notices, images and pointers
void swInstructionsPageWidget::CopySelectedElements()
{
	std::vector<swInstructionsElementKey> Keys;

	for (QGraphicsItem* Item : scene()->selectedItems())
	{
		swInstructionsElementKey Key;

		if (swGetElementKey(Item, Key))
			Keys.push_back(Key);
	}

	swInstructionsClipboard Clipboard = mInstructions->CopyElements(Keys);

	// other elements keep the old copy
	if (Clipboard.IsEmpty())
		return;

	mClipboard = std::move(Clipboard);
	mPasteCount = 0;

	// pastes start from the copied page

	mCopyPageIndex = mPage ? static_cast<size_t>(mPage - mInstructions->mPages.data()) : 0;
	mPastePageIndex = mCopyPageIndex;
}

void swInstructionsPageWidget::PasteElements()
{
	if (!CanPaste())
		return;

	const size_t PageIndex = static_cast<size_t>(mPage - mInstructions->mPages.data());

	if (mInstructions->IsPageLocked(PageIndex))
		return;

	// shared images and notices show on many pages

	const bool Shared = !mClipboard.Images.empty() || std::any_of(mClipboard.Notices.begin(), mClipboard.Notices.end(), [](const swInstructionsNotice& Notice)
	{
		return Notice.Pages != swInstructionsImagePages::None;
	});

	// other pages start in place, each paste steps one grid further

	if (PageIndex != mPastePageIndex && !Shared)
	{
		mPastePageIndex = PageIndex;
		mPasteCount = PageIndex == mCopyPageIndex ? 1 : 0;
	}
	else
	{
		mPastePageIndex = PageIndex;
		mPasteCount++;
	}

	const qreal Step = mInstructions->GetGridSnapSize() * mPasteCount;

	// the copies are selected once rebuilt
	scene()->clearSelection();

	{
		const QSignalBlocker Blocker(mInstructions);
		mPendingSelection = mInstructions->PasteElements(mClipboard, PageIndex, QPointF(Step, Step));
	}

	if (!mPendingSelection.empty())
		emit mInstructions->PageLayoutChanged();
}

void swInstructionsPageWidget::DuplicateSelectedElements()
{
	if (!mPage)
		return;

	const size_t PageIndex = static_cast<size_t>(mPage - mInstructions->mPages.data());
	std::vector<swInstructionsElementKey> Keys;

	if (mInstructions->IsPageLocked(PageIndex))
		return;

	for (QGraphicsItem* Item : scene()->selectedItems())
	{
		swInstructionsElementKey Key;

		if (swGetElementKey(Item, Key))
			Keys.push_back(Key);
	}

	const swInstructionsClipboard Clipboard = mInstructions->CopyElements(Keys);
	const qreal Step = mInstructions->GetGridSnapSize();

	if (Clipboard.IsEmpty())
		return;

	// the copies are selected once rebuilt
	scene()->clearSelection();

	{
		const QSignalBlocker Blocker(mInstructions);
		mPendingSelection = mInstructions->PasteElements(Clipboard, PageIndex, QPointF(Step, Step));
	}

	if (!mPendingSelection.empty())
		emit mInstructions->PageLayoutChanged();
}

// use to move the selection by grid steps
void swInstructionsPageWidget::NudgeSelection(int DirectionX, int DirectionY, bool FullUnit)
{
	if (!mPage || mInstructions->IsPageLocked(static_cast<size_t>(mPage - mInstructions->mPages.data())))
		return;

	const qreal Step = FullUnit ? mInstructions->GetGridSize() : mInstructions->GetGridSnapSize();
	const QPointF Distance(DirectionX * Step, DirectionY * Step);
	std::map<swInstructionsElementKey, QPointF> Offsets;
	bool Moved = false;

	// one redraw for the whole selection
	{
		const QSignalBlocker Blocker(mInstructions);

		for (QGraphicsItem* Item : scene()->selectedItems())
		{
			swInstructionsElementKey Key;

			if (!swGetElementKey(Item, Key))
				continue;

			if (mInstructions->IsElementLocked(Key))
				mInstructions->LockElement(Key, mInstructions->GetLockedPosition(Key) + Distance);
			else if (Item->flags() & QGraphicsItem::ItemIsMovable)
				Offsets[Key] = mInstructions->GetElementOffset(Key) + Distance;
			else
				continue;

			Moved = true;
		}

		mInstructions->SetElementOffsets(Offsets);
	}

	if (Moved)
		emit mInstructions->PageLayoutChanged();
}

// use to match the main window copy keys
bool swInstructionsPageWidget::RunClipboardKey(QKeyEvent* Event, bool Run)
{
	if (!gMainWindow)
		return false;

	const QKeySequence Pressed(static_cast<int>(Event->modifiers() & ~Qt::KeypadModifier) | Event->key());
	const std::pair<int, void (swInstructionsPageWidget::*)()> Commands[] =
	{
		{ SW_EDIT_COPY, &swInstructionsPageWidget::CopySelectedElements },
		{ SW_EDIT_PASTE, &swInstructionsPageWidget::PasteElements },
		{ SW_PART_DUPLICATE, &swInstructionsPageWidget::DuplicateSelectedElements }
	};

	for (const auto& [CommandId, Command] : Commands)
	{
		if (gMainWindow->mActions[CommandId]->shortcut() != Pressed)
			continue;

		if (Run)
			(this->*Command)();

		return true;
	}

	return false;
}

// the page takes its keys before the main window
bool swInstructionsPageWidget::event(QEvent* Event)
{
	if (Event->type() == QEvent::ShortcutOverride && RunClipboardKey(static_cast<QKeyEvent*>(Event), false))
	{
		Event->accept();
		return true;
	}

	return QGraphicsView::event(Event);
}

// use to nudge with the number pad
void swInstructionsPageWidget::keyPressEvent(QKeyEvent* Event)
{
	if (RunClipboardKey(Event, true))
	{
		Event->accept();
		return;
	}

	static const std::map<int, QPoint> PadDirections =
	{
		{ Qt::Key_1, QPoint(-1, 1) }, { Qt::Key_2, QPoint(0, 1) }, { Qt::Key_3, QPoint(1, 1) },
		{ Qt::Key_4, QPoint(-1, 0) }, { Qt::Key_6, QPoint(1, 0) },
		{ Qt::Key_7, QPoint(-1, -1) }, { Qt::Key_8, QPoint(0, -1) }, { Qt::Key_9, QPoint(1, -1) },
		{ Qt::Key_End, QPoint(-1, 1) }, { Qt::Key_Down, QPoint(0, 1) }, { Qt::Key_PageDown, QPoint(1, 1) },
		{ Qt::Key_Left, QPoint(-1, 0) }, { Qt::Key_Right, QPoint(1, 0) },
		{ Qt::Key_Home, QPoint(-1, -1) }, { Qt::Key_Up, QPoint(0, -1) }, { Qt::Key_PageUp, QPoint(1, -1) }
	};

	const std::map<int, QPoint>::const_iterator Direction = PadDirections.find(Event->key());

	if ((Event->modifiers() & Qt::KeypadModifier) && Direction != PadDirections.end() && !scene()->selectedItems().isEmpty())
	{
		// shift turns pad digits into navigation keys
		const bool FullUnit = (Event->modifiers() & Qt::ShiftModifier) || Event->key() < Qt::Key_0 || Event->key() > Qt::Key_9;

		NudgeSelection(Direction->second.x(), Direction->second.y(), FullUnit);
		Event->accept();
		return;
	}

	QGraphicsView::keyPressEvent(Event);
}

void swInstructionsPageWidget::mouseReleaseEvent(QMouseEvent* Event)
{
	viewport()->setCursor(Qt::ArrowCursor);

	// fit once the click is done
	if (mFitPending)
	{
		mFitPending = false;
		QTimer::singleShot(0, this, &swInstructionsPageWidget::FitPage);
	}

	if (mSelectionPending)
		QTimer::singleShot(0, this, &swInstructionsPageWidget::SelectionChanged);

	// clicks reopen the selection groups

	if (Event->button() == Qt::LeftButton && !mDragStarted && !mResizeItem)
	{
		QTimer::singleShot(0, this, [this]()
		{
			const QList<QGraphicsItem*> SelectedItems = scene()->selectedItems();

			if (!SelectedItems.isEmpty())
				mPropertiesWidget->SelectionChanged(SelectedItems.first(), true);
		});
	}

	if (mResizeItem)
	{
		// keep the size, left and top edges move it
		swInstructionsElementKey Key;
		const QSizeF Size = dynamic_cast<swInstructionsBoxBase*>(mResizeItem)->GetBoxSize();
		const QPointF Distance = mResizeItem->pos() - mResizeStartPosition;
		const bool HasKey = swGetElementKey(mResizeItem, Key);
		const qreal Inset = dynamic_cast<swInstructionsBoxBase*>(mResizeItem)->GetBoxStyle().GetInset();

		mResizeItem = nullptr;
		mResizeHandle = 0;

		if (HasKey)
			QTimer::singleShot(0, this, [this, Key, Size, Distance, Inset]()
			{
				{
					const QSignalBlocker Blocker(mInstructions);
					mInstructions->SetElementSize(Key, Size);

					// images take the new width
					const swInstructionsImage* Image = std::get<0>(Key) == swInstructionsPlacementElement::Image ? mInstructions->FindImage(static_cast<int>(std::get<2>(Key))) : nullptr;

					if (Image)
					{
						swInstructionsImage Changed = *Image;
						Changed.Size = qBound(1.0, (Size.width() - 2.0 * Inset) * 100.0 / mInstructions->mPageSetup.Width, 500.0);
						mInstructions->SetImage(Changed);
					}

					if (!Distance.isNull() && mInstructions->IsElementLocked(Key))
						mInstructions->LockElement(Key, mInstructions->GetLockedPosition(Key) + Distance);
					else if (!Distance.isNull())
						mInstructions->SetElementOffsets({ { Key, mInstructions->GetElementOffset(Key) + Distance } });
				}

				emit mInstructions->PageLayoutChanged();
			});

		return;
	}

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
	DrawBoundaries(Painter);
	DrawPointerGrid(Painter);
}

// use to outline the edges on both pages
void swInstructionsPageWidget::DrawBoundaries(QPainter* Painter)
{
	const swInstructionsBoundaries Boundaries = mInstructions->GetElementBoundaries();
	const bool ShowLayoutAreas = mInstructions->IsLayoutAreasShown();

	if (Boundaries == swInstructionsBoundaries::Off && !ShowLayoutAreas)
		return;

	std::vector<swInstructionsBoundaryBox> Boxes = mPairBoxes;
	std::vector<QPolygonF> LayoutEdges = mPairLayoutEdges;
	swCollectBoundaries(scene(), QTransform(), Boxes, LayoutEdges);

	Painter->save();
	Painter->setBrush(Qt::NoBrush);

	if (ShowLayoutAreas)
	{
		QPen LayoutPen(mInstructions->GetLayoutAreaColor(), 1.0);
		LayoutPen.setCosmetic(true);
		Painter->setPen(LayoutPen);

		for (const QPolygonF& Edge : LayoutEdges)
			Painter->drawPolygon(Edge);
	}

	QPen ElementPen(mInstructions->GetBoundaryEdgeColor(), 1.0);
	ElementPen.setCosmetic(true);
	QPen DashedPen = ElementPen;
	DashedPen.setStyle(Qt::DashLine);
	QPen BorderPen(mInstructions->GetBoundaryBorderColor(), 1.0);
	BorderPen.setCosmetic(true);

	for (const swInstructionsBoundaryBox& Box : Boxes)
	{
		if (Boundaries == swInstructionsBoundaries::Off)
			break;

		// box model rectangles from the outside in
		const swInstructionsBoxStyle& BoxStyle = Box.Style;
		const QRectF BorderRect = Box.Rect.adjusted(BoxStyle.Margin, BoxStyle.Margin, -BoxStyle.Margin, -BoxStyle.Margin);
		const QRectF PaddingRect = BorderRect.adjusted(BoxStyle.BorderWidth, BoxStyle.BorderWidth, -BoxStyle.BorderWidth, -BoxStyle.BorderWidth);
		const QRectF ContentRect = PaddingRect.adjusted(BoxStyle.Padding, BoxStyle.Padding, -BoxStyle.Padding, -BoxStyle.Padding);

		Painter->setPen(ElementPen);
		Painter->drawPolygon(Box.Transform.map(QPolygonF(Box.Rect)));

		if (Boundaries == swInstructionsBoundaries::MarginPadding)
		{
			Painter->setPen(DashedPen);
			Painter->drawPolygon(Box.Transform.map(QPolygonF(ContentRect)));
		}
		else if (Boundaries == swInstructionsBoundaries::EveryEdge)
		{
			Painter->drawPolygon(Box.Transform.map(QPolygonF(ContentRect)));
			Painter->setPen(BorderPen);

			for (const QRectF& EdgeRect : { BorderRect, PaddingRect })
				Painter->drawPolygon(Box.Transform.map(QPolygonF(EdgeRect)));
		}
	}

	Painter->restore();
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

// use to show the tip spots of the element under a dragged pointer tip
void swInstructionsPageWidget::DrawPointerGrid(QPainter* Painter)
{
	const swInstructionsPointerItem* PointerItem = dynamic_cast<const swInstructionsPointerItem*>(scene()->mouseGrabberItem());
	QRectF Rect;
	QPointF SnapFraction;
	bool Snapped = false;

	if (!PointerItem || !PointerItem->GetSnapGrid(Rect, SnapFraction, Snapped))
		return;

	// markers keep their size at any zoom
	const QTransform Transform = PointerItem->parentItem()->sceneTransform() * Painter->transform();
	const qreal Size = qMax(5.0, fontMetrics().height() / 3.0);
	QColor Color = mInstructions->GetElementAnchorColor();
	Color.setAlpha(255);

	Painter->save();
	Painter->resetTransform();
	Painter->setRenderHint(QPainter::Antialiasing, true);
	Painter->setPen(QPen(Qt::white, 1.0));

	for (const QPointF& Fraction : swGetPointerGrid())
	{
		const QPointF Point = Transform.map(Rect.topLeft() + QPointF(Rect.width() * Fraction.x(), Rect.height() * Fraction.y()));
		const qreal MarkerSize = Snapped && Fraction == SnapFraction ? Size * 1.6 : Size;

		Painter->setBrush(Snapped && Fraction == SnapFraction ? Color : Color.lighter(150));
		Painter->drawEllipse(Point, MarkerSize / 2.0, MarkerSize / 2.0);
	}

	Painter->restore();
}

void swInstructionsPageWidget::resizeEvent(QResizeEvent* Event)
{
	QGraphicsView::resizeEvent(Event);

	// a held click keeps the page still
	if (QApplication::mouseButtons() & Qt::LeftButton)
		mFitPending = true;
	else
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
