#pragma once

#include "sw_instructions.h"

class swInstructionsPropertiesWidget;
class swInstructionsPlacementWidget;

// Snaps a border to device pixels.
inline void swSnapBorderRect(const QTransform& Transform, QRectF& Rect, qreal& BorderWidth)
{
	const qreal Scale = qAbs(Transform.m11());

	if (Transform.isRotating() || !Transform.isInvertible() || Scale <= 0.0)
		return;

	const QRectF DeviceRect = Transform.mapRect(Rect);
	const QRectF SnappedRect(QPointF(qRound(DeviceRect.left()), qRound(DeviceRect.top())), QPointF(qRound(DeviceRect.right()), qRound(DeviceRect.bottom())));

	Rect = Transform.inverted().mapRect(SnappedRect);

	if (BorderWidth > 0.0)
		BorderWidth = qMax<qreal>(qRound(BorderWidth * Scale), 1.0) / Scale;
}

// A step divider snapped like box borders.
class swInstructionsDividerItem : public QGraphicsRectItem
{
public:
	swInstructionsDividerItem(const QRectF& Rect, bool Rounded, QGraphicsItem* Parent)
		: QGraphicsRectItem(Rect, Parent), mRounded(Rounded)
	{
	}

	void paint(QPainter* Painter, const QStyleOptionGraphicsItem* Option, QWidget* Widget) override
	{
		Q_UNUSED(Option);
		Q_UNUSED(Widget);

		QRectF LineRect = rect();
		const QTransform& Transform = Painter->worldTransform();

		// whole pixel thickness at any zoom

		if (!Transform.isRotating() && Transform.isInvertible())
		{
			const QRectF DeviceRect = Transform.mapRect(LineRect);
			const bool Vertical = LineRect.width() < LineRect.height();
			const qreal Thickness = qMax<qreal>(qRound(Vertical ? DeviceRect.width() : DeviceRect.height()), 1.0);
			QRectF SnappedRect;

			if (Vertical)
				SnappedRect = QRectF(qRound(DeviceRect.center().x() - Thickness / 2.0), qRound(DeviceRect.top()), Thickness, qRound(DeviceRect.bottom()) - qRound(DeviceRect.top()));
			else
				SnappedRect = QRectF(qRound(DeviceRect.left()), qRound(DeviceRect.center().y() - Thickness / 2.0), qRound(DeviceRect.right()) - qRound(DeviceRect.left()), Thickness);

			LineRect = Transform.inverted().mapRect(SnappedRect);
		}

		const qreal Radius = mRounded ? qMin(LineRect.width(), LineRect.height()) / 2.0 : 0.0;

		Painter->save();
		Painter->setRenderHint(QPainter::Antialiasing, true);
		Painter->setPen(Qt::NoPen);
		Painter->setBrush(brush());
		Painter->drawRoundedRect(LineRect, Radius, Radius);
		Painter->restore();
	}

protected:
	bool mRounded = false;
};

// Fills inside the border, rounded.
inline void swPaintBoxBackground(QPainter* Painter, const QRectF& BorderRect, const swInstructionsBoxStyle& BoxStyle)
{
	if (!BoxStyle.BackgroundColor.alpha())
		return;

	QPainterPath BackgroundPath;
	BackgroundPath.addRoundedRect(BorderRect, BoxStyle.CornerRadius, BoxStyle.CornerRadius);

	Painter->save();
	Painter->setRenderHint(QPainter::Antialiasing, true);
	Painter->fillPath(BackgroundPath, BoxStyle.BackgroundColor);
	Painter->restore();
}

// Draws the border with equal sides at any zoom.
inline void swPaintBoxBorder(QPainter* Painter, const QRectF& BorderRect, const swInstructionsBoxStyle& BoxStyle)
{
	if (BoxStyle.BorderWidth <= 0.0 || !BoxStyle.BorderColor.alpha())
		return;

	QRectF OuterRect = BorderRect;
	qreal BorderWidth = BoxStyle.BorderWidth;
	swSnapBorderRect(Painter->worldTransform(), OuterRect, BorderWidth);

	// one stroke keeps corners as thick as sides
	const qreal BorderHalf = BorderWidth / 2.0;
	const QRectF LineRect = OuterRect.adjusted(BorderHalf, BorderHalf, -BorderHalf, -BorderHalf);
	const qreal LineRadius = qMax(0.0, BoxStyle.CornerRadius - BorderHalf);
	QPen BorderPen(BoxStyle.BorderColor, BorderWidth);
	BorderPen.setJoinStyle(Qt::MiterJoin);

	Painter->save();
	Painter->setRenderHint(QPainter::Antialiasing, true);
	Painter->setPen(BorderPen);
	Painter->setBrush(Qt::NoBrush);
	Painter->drawRoundedRect(LineRect, LineRadius, LineRadius);
	Painter->restore();
}

// A box around an area, filled and framed.
// an element box outlined by boundaries
struct swInstructionsBoundaryBox
{
	QTransform Transform;
	QRectF Rect;
	swInstructionsBoxStyle Style;
};

// an unpainted layout area shown by boundaries

class swInstructionsGuideItem : public QGraphicsItem
{
public:
	swInstructionsGuideItem(const QRectF& Rect, QGraphicsItem* Parent)
		: QGraphicsItem(Parent), mRect(Rect)
	{
		setFlag(QGraphicsItem::ItemHasNoContents);
	}

	const QRectF& GetRect() const
	{
		return mRect;
	}

	QRectF boundingRect() const override
	{
		return QRectF();
	}

	void paint(QPainter* Painter, const QStyleOptionGraphicsItem* Option, QWidget* Widget) override
	{
		Q_UNUSED(Painter);
		Q_UNUSED(Option);
		Q_UNUSED(Widget);
	}

protected:
	QRectF mRect;
};

class swInstructionsFrameItem : public QGraphicsRectItem
{
public:
	swInstructionsFrameItem(const QRectF& Rect, const swInstructionsBoxStyle& BoxStyle, QGraphicsItem* Parent)
		: QGraphicsRectItem(Rect, Parent), mBoxStyle(BoxStyle)
	{
		setPen(Qt::NoPen);
	}

	void paint(QPainter* Painter, const QStyleOptionGraphicsItem* Option, QWidget* Widget) override
	{
		Q_UNUSED(Widget);

		const qreal Margin = mBoxStyle.Margin;
		const QRectF BorderRect = rect().adjusted(Margin, Margin, -Margin, -Margin);

		swPaintBoxBackground(Painter, BorderRect, mBoxStyle);
		swPaintBoxBorder(Painter, BorderRect, mBoxStyle);

		if (!(Option->state & QStyle::State_Selected))
			return;

		// a highlight visible on light pages
		QPen Pen(Option->palette.highlight(), 2.0, Qt::DashLine);
		Pen.setCosmetic(true);

		Painter->save();
		Painter->setPen(Pen);
		Painter->setBrush(Qt::NoBrush);
		Painter->drawRect(rect());
		Painter->restore();
	}

	const swInstructionsBoxStyle& GetBoxStyle() const
	{
		return mBoxStyle;
	}

protected:
	swInstructionsBoxStyle mBoxStyle;
};

// Pointers leave any boxed element.
class swInstructionsBoxBase
{
public:
	virtual ~swInstructionsBoxBase() = default;

	// The outer border edge.
	virtual QRectF GetBorderRect() const = 0;

	// The border width.
	virtual qreal GetBorderWidth() const = 0;

	// The margin, border and padding.
	virtual const swInstructionsBoxStyle& GetBoxStyle() const = 0;

	// A fixed outer size and the content alignment inside it.
	virtual void SetBoxLayout(const QSizeF& BoxSize, int Alignment, int VerticalAlignment) = 0;
	virtual void SetBoxSize(const QSizeF& BoxSize) = 0;
	virtual QSizeF GetBoxSize() const = 0;
};

// The box around page elements.
template <typename T>
class swInstructionsBoxItem : public T, public swInstructionsBoxBase
{
public:
	explicit swInstructionsBoxItem(QGraphicsItem* Parent)
		: T(Parent)
	{
	}

	void SetBoxStyle(const swInstructionsBoxStyle& BoxStyle)
	{
		T::prepareGeometryChange();
		mBoxStyle = BoxStyle;
	}

	const swInstructionsBoxStyle& GetBoxStyle() const override
	{
		return mBoxStyle;
	}

	void SetBoxLayout(const QSizeF& BoxSize, int Alignment, int VerticalAlignment) override
	{
		T::prepareGeometryChange();
		mBoxSize = BoxSize;
		mAlignment = qBound(0, Alignment, 2);
		mVerticalAlignment = qBound(0, VerticalAlignment, 2);

		// text lines follow the alignment
		if constexpr (std::is_base_of_v<QGraphicsTextItem, T>)
		{
			QTextOption TextOption = T::document()->defaultTextOption();
			TextOption.setAlignment(mAlignment == 1 ? Qt::AlignHCenter : mAlignment == 2 ? Qt::AlignRight : Qt::AlignLeft);
			T::document()->setDefaultTextOption(TextOption);

			if (mBoxSize.isValid())
				T::setTextWidth(qMax(1.0, mBoxSize.width() - 2.0 * mBoxStyle.GetInset()));
			else if (T::textWidth() < 0.0)
				T::setTextWidth(T::document()->idealWidth());
		}
	}

	void SetBoxSize(const QSizeF& BoxSize) override
	{
		SetBoxLayout(BoxSize, mAlignment, mVerticalAlignment);
	}

	QSizeF GetBoxSize() const override
	{
		return mBoxSize;
	}

	// Content corner inside the box.
	QPointF GetContentOffset() const
	{
		const qreal Inset = mBoxStyle.GetInset();

		if (!mBoxSize.isValid())
			return QPointF(Inset, Inset);

		const QSizeF ContentSize = T::boundingRect().size();
		const qreal FreeWidth = qMax(0.0, mBoxSize.width() - 2.0 * Inset - ContentSize.width());
		const qreal FreeHeight = qMax(0.0, mBoxSize.height() - 2.0 * Inset - ContentSize.height());

		return QPointF(Inset + FreeWidth * mAlignment / 2.0, Inset + FreeHeight * mVerticalAlignment / 2.0);
	}

	QRectF GetBorderRect() const override
	{
		const QRectF BoxRect = boundingRect();

		return BoxRect.adjusted(mBoxStyle.Margin, mBoxStyle.Margin, -mBoxStyle.Margin, -mBoxStyle.Margin);
	}

	qreal GetBorderWidth() const override
	{
		return mBoxStyle.BorderColor.alpha() ? mBoxStyle.BorderWidth : 0.0;
	}

	QRectF boundingRect() const override
	{
		const QRectF ContentRect = T::boundingRect();

		if (mBoxSize.isValid())
			return QRectF(QPointF(0.0, 0.0), mBoxSize);

		if (ContentRect.isEmpty())
			return ContentRect;

		const qreal Inset = mBoxStyle.GetInset();

		return ContentRect.adjusted(0.0, 0.0, 2.0 * Inset, 2.0 * Inset);
	}

	// only drawn parts take clicks, selected items their frame

	QPainterPath shape() const override
	{
		QPainterPath Path;

		if (T::isSelected())
			Path.addRect(boundingRect());
		else
		{
			UpdateHitMask();
			Path.addRegion(mHitRegion);
			Path.translate(boundingRect().topLeft());
		}

		return Path;
	}

	bool contains(const QPointF& Point) const override
	{
		if (T::isSelected())
			return boundingRect().contains(Point);

		UpdateHitMask();

		const QRectF BoxRect = boundingRect();
		const QPoint Pixel = (Point - BoxRect.topLeft()).toPoint();

		return mHitRegion.intersects(QRect(Pixel - QPoint(2, 2), QSize(5, 5)));
	}

	void paint(QPainter* Painter, const QStyleOptionGraphicsItem* Option, QWidget* Widget) override
	{
		const QRectF BoxRect = boundingRect();

		// a repaint renews the hit mask

		if (!mHitUpdating)
			mHitValid = false;

		if (BoxRect.isEmpty())
			return;

		// background fills inside the border
		swPaintBoxBackground(Painter, GetBorderRect(), mBoxStyle);

		// Content inside the padding.
		QStyleOptionGraphicsItem ContentOption(*Option);
		ContentOption.state &= ~(QStyle::State_Selected | QStyle::State_HasFocus);

		Painter->save();

		// rounded boxes trim their fill

		if (mClipContent && mBoxStyle.CornerRadius > 0.0)
		{
			const qreal Border = GetBorderWidth();
			const QRectF InnerRect = GetBorderRect().adjusted(Border, Border, -Border, -Border);
			const qreal InnerRadius = qMax(0.0, mBoxStyle.CornerRadius - Border);
			QPainterPath ClipPath;
			ClipPath.addRoundedRect(InnerRect, InnerRadius, InnerRadius);
			Painter->setClipPath(ClipPath, Qt::IntersectClip);
		}

		Painter->translate(GetContentOffset());
		T::paint(Painter, &ContentOption, Widget);
		Painter->restore();

		swPaintBoxBorder(Painter, GetBorderRect(), mBoxStyle);

		if (!(Option->state & QStyle::State_Selected))
			return;

		Painter->save();

		// A highlight visible on light pages.
		QPen Pen(Option->palette.highlight(), 2.0, Qt::DashLine);
		Pen.setCosmetic(true);

		Painter->setPen(Pen);
		Painter->setBrush(Qt::NoBrush);
		Painter->drawRect(BoxRect);

		// handles drag the width and height
		if (mBoxSize.isValid())
		{
			const qreal HandleSize = 8.0 / qMax(0.001, Painter->worldTransform().m11());
			const QPointF Handles[8] = { BoxRect.topLeft(), QPointF(BoxRect.center().x(), BoxRect.top()), BoxRect.topRight(), QPointF(BoxRect.right(), BoxRect.center().y()), BoxRect.bottomRight(), QPointF(BoxRect.center().x(), BoxRect.bottom()), BoxRect.bottomLeft(), QPointF(BoxRect.left(), BoxRect.center().y()) };

			Painter->setPen(Qt::NoPen);
			Painter->setBrush(Option->palette.highlight());

			for (const QPointF& Handle : Handles)
				Painter->drawRect(QRectF(Handle - QPointF(HandleSize / 2.0, HandleSize / 2.0), QSizeF(HandleSize, HandleSize)));
		}

		Painter->restore();
	}

protected:
	swInstructionsBoxStyle mBoxStyle;
	QSizeF mBoxSize;
	bool mClipContent = false;

	// the painted pixels in page units

	void UpdateHitMask() const
	{
		if (mHitValid)
			return;

		const QRectF BoxRect = boundingRect();
		QImage Mask(qMax(1, qCeil(BoxRect.width())), qMax(1, qCeil(BoxRect.height())), QImage::Format_ARGB32_Premultiplied);
		Mask.fill(Qt::transparent);

		if (!BoxRect.isEmpty())
		{
			QStyleOptionGraphicsItem Option;
			Option.exposedRect = BoxRect;
			QPainter Painter(&Mask);
			Painter.translate(-BoxRect.topLeft());
			mHitUpdating = true;
			const_cast<swInstructionsBoxItem*>(this)->paint(&Painter, &Option, nullptr);
			mHitUpdating = false;
		}

		// texts take clicks on their whole drawn area

		if constexpr (!std::is_same_v<T, QGraphicsPixmapItem>)
		{
			mHitRegion = QRegion(QRegion(QBitmap::fromImage(Mask.createAlphaMask())).boundingRect());
			mHitValid = true;
			return;
		}

		// images use coarse cells, quick to test

		constexpr int Cell = 8;
		QImage Cells((Mask.width() + Cell - 1) / Cell, (Mask.height() + Cell - 1) / Cell, QImage::Format_Alpha8);
		Cells.fill(0);

		for (int Y = 0; Y < Mask.height(); Y++)
		{
			const QRgb* Line = reinterpret_cast<const QRgb*>(Mask.constScanLine(Y));
			uchar* CellLine = Cells.scanLine(Y / Cell);

			for (int X = 0; X < Mask.width(); X++)
				if (qAlpha(Line[X]))
					CellLine[X / Cell] = 255;
		}

		mHitRegion = QTransform::fromScale(Cell, Cell).map(QRegion(QBitmap::fromImage(Cells.createAlphaMask())));
		mHitValid = true;
	}

	mutable QRegion mHitRegion;
	mutable bool mHitValid = false;
	mutable bool mHitUpdating = false;
	int mAlignment = 0;
	int mVerticalAlignment = 1;
};

class swInstructionsStepImageItem : public swInstructionsBoxItem<QGraphicsPixmapItem>
{
public:
	swInstructionsStepImageItem(QGraphicsItem* Parent, swInstructions* Instructions, swModel* Model, swStep Step);

	swModel* GetModel() const
	{
		return mModel;
	}

	swStep GetStep() const
	{
		return mStep;
	}

	void SetImageSize(int Width, int Height)
	{
		mWidth = Width;
		mHeight = Height;
	}

	void SetCoverImage();

	bool IsCoverImage() const
	{
		return mCoverImage;
	}

	// Draws the finished submodel with its part count below.
	void SetSubmodelPreview();

	bool IsSubmodelPreview() const
	{
		return mSubmodelPreview;
	}

	void SetFinalModel();

	bool IsFinalModel() const
	{
		return mFinalModel;
	}

	void Update();

	// Finds the part drawn at a point.
	bool FindPart(const QPointF& Point, int& PartIndex) const;
	bool GetPartPoint(int PartIndex, QPointF& Point) const;

protected:
	QVariant itemChange(GraphicsItemChange Change, const QVariant& Value) override;

	struct swPartRegion
	{
		int PartIndex;
		QRectF Rect;
		QPointF Center;
	};

	swInstructions* mInstructions = nullptr;
	swModel* mModel = nullptr;
	swStep mStep = 1;
	int mWidth = 1;
	int mHeight = 1;
	bool mCoverImage = false;
	bool mFinalModel = false;
	bool mSubmodelPreview = false;
	std::vector<swPartRegion> mPartRegions;
};

class swInstructionsStepNumberItem : public swInstructionsBoxItem<QGraphicsSimpleTextItem>
{
public:
	swInstructionsStepNumberItem(QGraphicsItem* Parent, swInstructions* Instructions, swModel* Model, swStep Step);

	swModel* GetModel() const
	{
		return mModel;
	}

	swStep GetStep() const
	{
		return mStep;
	}

	void Update();

protected:
	QVariant itemChange(GraphicsItemChange Change, const QVariant& Value) override;

	swInstructions* mInstructions = nullptr;
	swModel* mModel = nullptr;
	swStep mStep = 1;
};

// One page number item per page.
class swInstructionsPageNumberItem : public swInstructionsBoxItem<QGraphicsSimpleTextItem>
{
public:
	swInstructionsPageNumberItem(QGraphicsItem* Parent, swInstructions* Instructions, size_t PageIndex);

protected:
	QVariant itemChange(GraphicsItemChange Change, const QVariant& Value) override;

	swInstructions* mInstructions = nullptr;
};

// The part count of a model or of the steps of a page.
class swInstructionsPartCountItem : public swInstructionsBoxItem<QGraphicsSimpleTextItem>
{
public:
	swInstructionsPartCountItem(QGraphicsItem* Parent, swInstructions* Instructions, swInstructionsPlacementElement Element, swModel* Model, int Count);

	swInstructionsPlacementElement GetElement() const
	{
		return mElement;
	}

	swModel* GetModel() const
	{
		return mModel;
	}

protected:
	QVariant itemChange(GraphicsItemChange Change, const QVariant& Value) override;

	swInstructions* mInstructions = nullptr;
	swInstructionsPlacementElement mElement = swInstructionsPlacementElement::PagePartCount;
	swModel* mModel = nullptr;
};

// a parts list quantity, selectable to reach its placement
class swInstructionsPartsListCountItem : public QGraphicsRectItem
{
public:
	swInstructionsPartsListCountItem(QGraphicsItem* Parent, const QRectF& Rect, swModel* Model, swStep Step)
		: QGraphicsRectItem(Rect, Parent), mModel(Model), mStep(Step)
	{
		setPen(Qt::NoPen);
		setFlags(QGraphicsItem::ItemIsSelectable);
	}

	swModel* GetModel() const
	{
		return mModel;
	}

	swStep GetStep() const
	{
		return mStep;
	}

protected:
	swModel* mModel = nullptr;
	swStep mStep = 1;
};

class swInstructionsPartsListItem : public swInstructionsBoxItem<QGraphicsPixmapItem>
{
public:
	swInstructionsPartsListItem(QGraphicsItem* Parent, swInstructions* Instructions, swModel* Model, swStep Step);

	swModel* GetModel() const
	{
		return mModel;
	}

	swStep GetStep() const
	{
		return mStep;
	}

	void SetBillOfMaterials(int MaxWidth, int MaxHeight)
	{
		mBillOfMaterials = true;
		SetMaxSize(MaxWidth, MaxHeight);
	}

	void SetMaxSize(int MaxWidth, int MaxHeight)
	{
		mMaxWidth = MaxWidth;
		mMaxHeight = MaxHeight;
	}

	bool IsBillOfMaterials() const
	{
		return mBillOfMaterials;
	}

	void Update();

protected:
	QVariant itemChange(GraphicsItemChange Change, const QVariant& Value) override;

	swInstructions* mInstructions = nullptr;
	swModel* mModel = nullptr;
	swStep mStep = 1;
	bool mBillOfMaterials = false;
	int mMaxWidth = 300;
	int mMaxHeight = 300;
};

class swInstructionsCoverTextItem : public swInstructionsBoxItem<QGraphicsTextItem>
{
public:
	swInstructionsCoverTextItem(QGraphicsItem* Parent, swInstructions* Instructions, swModel* Model, swInstructionsPlacementElement Element);

	swModel* GetModel() const
	{
		return mModel;
	}

	swInstructionsPlacementElement GetElement() const
	{
		return mElement;
	}

protected:
	QVariant itemChange(GraphicsItemChange Change, const QVariant& Value) override;

	swInstructions* mInstructions = nullptr;
	swModel* mModel = nullptr;
	swInstructionsPlacementElement mElement = swInstructionsPlacementElement::CoverFrontTitle;
};

// A free text notice on a page.
class swInstructionsNoticeItem : public swInstructionsBoxItem<QGraphicsTextItem>
{
public:
	swInstructionsNoticeItem(QGraphicsItem* Parent, swInstructions* Instructions, const swInstructionsNotice& Notice);

	int GetId() const
	{
		return mId;
	}

protected:
	QVariant itemChange(GraphicsItemChange Change, const QVariant& Value) override;
	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* Event) override;

	swInstructions* mInstructions = nullptr;
	int mId = 0;
};

// A linked image on a cover, like a logo.
class swInstructionsImageItem : public swInstructionsBoxItem<QGraphicsPixmapItem>
{
public:
	swInstructionsImageItem(QGraphicsItem* Parent, swInstructions* Instructions, const swInstructionsImage& Image);

	int GetId() const
	{
		return mId;
	}

protected:
	QVariant itemChange(GraphicsItemChange Change, const QVariant& Value) override;

	swInstructions* mInstructions = nullptr;
	int mId = 0;
};

// A pointer from an element anchor.
class swInstructionsPointerItem : public QGraphicsItem
{
public:
	swInstructionsPointerItem(QGraphicsItem* Parent, swInstructions* Instructions, const swInstructionsPointer& Pointer, QGraphicsItem* Source, QGraphicsItem* Target);

	int GetId() const
	{
		return mPointer.Id;
	}

	void UpdateGeometry();

	// The tip drags to a new target.
	bool IsTipAt(const QPointF& Position) const
	{
		return !mLocked && QLineF(Position, mTipPosition).length() <= qMax(mTipSize, 10.0);
	}

	// The base drags to another anchor of its element.
	bool IsBaseAt(const QPointF& Position) const
	{
		return !mLocked && QLineF(Position, mBasePosition).length() <= 8.0;
	}

	// The Z middle and U outer segments drag sideways.
	bool IsBendAt(const QPointF& Position) const;
	bool IsBendHorizontal() const;
	bool GetBendLine(bool& Horizontal, qreal& Coordinate) const;

	void SetLocked(bool Locked)
	{
		mLocked = Locked;
	}

	// The element grid under a dragged tip.
	bool GetSnapGrid(QRectF& Rect, QPointF& Fraction, bool& Snapped) const
	{
		if (!mDraggingTip || !mSnapItem)
			return false;

		Rect = mSnapRect;
		Fraction = mSnapFraction;
		Snapped = mSnapped;
		return true;
	}

	QRectF boundingRect() const override;
	QPainterPath shape() const override;
	void paint(QPainter* Painter, const QStyleOptionGraphicsItem* Option, QWidget* Widget) override;

protected:
	void mousePressEvent(QGraphicsSceneMouseEvent* Event) override;
	void mouseMoveEvent(QGraphicsSceneMouseEvent* Event) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent* Event) override;
	QPainterPath GetPath(const QRectF& SourceRect, qreal BorderWidth) const;
	QPainterPath GetWedgePath(const QRectF& SourceRect, qreal BorderWidth) const;
	QPolygonF GetLeadPoints(const QPointF& Base) const
	{
		return GetLeadPoints(Base, mAnchor);
	}

	QPolygonF GetLeadPoints(const QPointF& Base, int Anchor) const;
	int GetClearAnchor(const QRectF& SourceRect, int Preferred, const QPointF& Reference) const;
	QPointF GetTipDirection() const;
	bool IsLeadHorizontal(const QPointF& Base) const
	{
		return IsLeadHorizontal(Base, mAnchor);
	}

	bool IsLeadHorizontal(const QPointF& Base, int Anchor) const;
	QPainterPath GetClipPath(const QRectF& SourceRect, qreal BorderWidth) const;

	swInstructions* mInstructions = nullptr;
	swInstructionsPointer mPointer;
	QGraphicsItem* mSource = nullptr;
	QGraphicsItem* mTarget = nullptr;
	swInstructionsPointerStyle mStyle = swInstructionsPointerStyle::Line;
	swInstructionsPointerTip mTip = swInstructionsPointerTip::Arrow;
	QColor mColor;
	qreal mWidth = 2.0;
	qreal mTipSize = 12.0;
	QPointF mBasePosition;
	QPointF mTipPosition;
	QRectF mSourceRect;
	int mAnchor = 0;
	qreal mSourceBorderWidth = 0.0;
	qreal mSourceRadius = 0.0;
	bool mLocked = false;
	bool mDraggingBend = false;
	bool mDraggingBase = false;
	qreal mBend = 0.5;
	qreal mReach = -1.0;
	qreal mBendRadius = 0.0;
	const QGraphicsItem* mSnapItem = nullptr;
	QRectF mSnapRect;
	QPointF mSnapFraction;
	bool mSnapped = false;
	bool mDraggingTip = false;
};

class swInstructionsPageWidget;

// Pages shown together, pairs start from the first page.
enum class swInstructionsPageSpread
{
	Single,
	SideBySide,
	Stacked
};

// Rulers along the page.
class swInstructionsRulerWidget : public QWidget
{
	Q_OBJECT

public:
	swInstructionsRulerWidget(swInstructionsPageWidget* PageWidget, Qt::Orientation Orientation);

	// The ruler width across.
	static int GetThickness(const QWidget* Widget);

protected:
	void paintEvent(QPaintEvent* Event) override;
	static qreal GetMajorStep(qreal PixelsPerUnit, qreal MinimumSpacing);
	static int GetSubdivisions(swInstructionsPageUnits Units, qreal MajorSpacing);

	swInstructionsPageWidget* mPageWidget;
	Qt::Orientation mOrientation;
};

class swInstructionsPageWidget : public QGraphicsView
{
	Q_OBJECT

public:
	swInstructionsPageWidget(QWidget* Parent, swInstructions* Instructions, swInstructionsPropertiesWidget* PropertiesWidget, swInstructionsPlacementWidget* PlacementWidget);

	void SetCurrentPage(const swInstructionsPage* Page);
	void SetPageSpread(swInstructionsPageSpread PageSpread);
	void RemoveSelectedElements();
	void CopySelectedElements();
	void PasteElements();
	void DuplicateSelectedElements();
	void AddClipboardActions(QMenu& Menu, bool Copyable);

	bool CanPaste() const
	{
		return !mClipboard.IsEmpty();
	}

	swInstructions* GetInstructions() const
	{
		return mInstructions;
	}

	// The pointer on the page.
	const std::optional<QPointF>& GetPointerPosition() const
	{
		return mPointerPosition;
	}

protected:
	void SelectElement(const swInstructionsElementKey& Key);
	void AlignSelection(int Placement, bool LastSelection);
	void PlaceElement(int Placement);
	void NudgeSelection(int DirectionX, int DirectionY, bool FullUnit);
	bool RunClipboardKey(QKeyEvent* Event, bool Run);
	bool event(QEvent* Event) override;
	void keyPressEvent(QKeyEvent* Event) override;
	void resizeEvent(QResizeEvent* Event) override;
	void mousePressEvent(QMouseEvent* Event) override;
	void mouseReleaseEvent(QMouseEvent* Event) override;
	void mouseMoveEvent(QMouseEvent* Event) override;
	void wheelEvent(QWheelEvent* Event) override;
	void contextMenuEvent(QContextMenuEvent* Event) override;
	void drawForeground(QPainter* Painter, const QRectF& Rect) override;
	void scrollContentsBy(int DeltaX, int DeltaY) override;
	bool viewportEvent(QEvent* Event) override;
	void DrawGrid(QPainter* Painter, const QRectF& Rect);
	void DrawElementBoxes(QPainter* Painter);
	void DrawBoundaries(QPainter* Painter);
	void DrawPointerGrid(QPainter* Painter);
	int FindResizeHandle(const QPoint& Position, QGraphicsItem** HandleItem) const;
	void UpdateRulers();
	void FitPage();

signals:
	void PageClicked(int Row);
	void PageScrolled(int Direction);

protected slots:
	void StepSettingsChanged(swModel* Model, swStep Step);
	void SelectionChanged();

protected:
	swInstructions* mInstructions;
	swInstructionsPropertiesWidget* mPropertiesWidget;
	swInstructionsPlacementWidget* mPlacementWidget;
	const swInstructionsPage* mPage = nullptr;
	swInstructionsPageSpread mPageSpread = swInstructionsPageSpread::Single;
	std::vector<swInstructionsBoundaryBox> mPairBoxes;
	std::vector<QPolygonF> mPairLayoutEdges;
	int mWheelDelta = 0;
	std::vector<std::pair<QGraphicsItem*, QPointF>> mDragItems;
	QGraphicsItem* mResizeItem = nullptr;
	int mResizeHandle = 0;
	QPointF mResizeStartPosition;
	QRectF mResizeStartRect;
	QPoint mPressPosition;
	bool mDragStarted = false;
	bool mFitPending = false;
	bool mSelectionPending = false;
	std::vector<swInstructionsElementKey> mPendingSelection;
	swInstructionsClipboard mClipboard;
	int mPasteCount = 0;
	std::vector<swInstructionsElementKey> mSelectionOrder;
	bool mRebuildingPage = false;
	swInstructionsRulerWidget* mHorizontalRuler = nullptr;
	swInstructionsRulerWidget* mVerticalRuler = nullptr;
	QLabel* mRulerCorner = nullptr;
	std::optional<QPointF> mPointerPosition;
};

class swInstructionsPageListWidget : public QDockWidget
{
	Q_OBJECT

public:
	swInstructionsPageListWidget(QWidget* Parent, swInstructions* Instructions);

	QListWidget* mThumbnailsWidget = nullptr;

protected:
	void CustomMenuRequested(const QPoint& Position);

	swInstructions* mInstructions;
};

// The position from the placement.
constexpr int SW_PLACEMENT_POSITION_DATA = 0;

// the band a background item draws
constexpr int SW_BAND_DATA = 1;

// the page object a plain item draws
constexpr int SW_OBJECT_DATA = 2;

enum class swInstructionsPageObject : int
{
	Divider,
	StepArea,
	FinalModel,
	Completion
};

// Identifies a page element.
bool swGetElementKey(const QGraphicsItem* Item, swInstructionsElementKey& Key);

// Gets the border anchors of an element.
QPointF swGetPointerAnchor(const QRectF& BorderRect, int Anchor);

// Edits the text of a notice.
bool swEditNoticeText(QWidget* Parent, QString& Text, QString& File, bool NewNotice);

// Fills a scene with page elements.
void swUpdateInstructionsPageScene(swInstructions* Instructions, const swInstructionsPage* Page, QGraphicsScene* Scene);
void swLockInstructionsPage(swInstructions* Instructions, size_t PageIndex);
QImage swRenderInstructionsPage(swInstructions* Instructions, size_t PageIndex, const QSize& Size);
