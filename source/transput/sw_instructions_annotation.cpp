#include "sw_global.h"
#include "sw_instructions.h"

const swInstructionsNotice* swInstructions::FindNotice(int Id) const
{
	for (const swInstructionsNotice& Notice : mNotices)
		if (Notice.Id == Id)
			return &Notice;

	return nullptr;
}

int swInstructions::AddNotice(size_t PageIndex, const QPointF& Position, const QString& Text, const QString& File, swInstructionsImagePages Pages)
{
	swInstructionsNotice Notice;

	Notice.Id = mNextNoticeId++;
	Notice.PageKey = GetPageElementsKey(PageIndex);
	Notice.Pages = Pages;
	Notice.Position = Position;
	Notice.Text = Text;
	Notice.File = File;

	mNotices.push_back(Notice);

	emit PageLayoutChanged();

	return Notice.Id;
}

void swInstructions::SetNoticeText(int Id, const QString& Text, const QString& File)
{
	for (swInstructionsNotice& Notice : mNotices)
	{
		if (Notice.Id != Id || (Notice.Text == Text && Notice.File == File))
			continue;

		Notice.Text = Text;
		Notice.File = File;

		emit PageLayoutChanged();
		return;
	}
}

void swInstructions::SetNoticePosition(int Id, const QPointF& Position)
{
	const swInstructionsElementKey Key = { swInstructionsPlacementElement::Notice, nullptr, static_cast<swStep>(Id) };

	for (swInstructionsNotice& Notice : mNotices)
	{
		if (Notice.Id != Id)
			continue;

		// the placed spot replaces the moved one

		const bool OffsetRemoved = mElementOffsets.erase(Key) > 0;

		if (Notice.Position == Position && !OffsetRemoved)
			return;

		Notice.Position = Position;

		emit PageLayoutChanged();
		return;
	}
}

void swInstructions::SetNoticePages(int Id, swInstructionsImagePages Pages, size_t PageIndex, const QPointF& Position)
{
	for (swInstructionsNotice& Notice : mNotices)
	{
		if (Notice.Id != Id)
			continue;

		Notice.Pages = Pages;
		Notice.PageKey = Pages == swInstructionsImagePages::None ? GetPageElementsKey(PageIndex) : swInstructionsPageKey();
		Notice.Position = Position;

		mElementOffsets.erase({ swInstructionsPlacementElement::Notice, nullptr, static_cast<swStep>(Id) });

		emit PageLayoutChanged();
		return;
	}
}

void swInstructions::SetNoticeAlignment(int Id, int Alignment, int VerticalAlignment)
{
	for (swInstructionsNotice& Notice : mNotices)
	{
		if (Notice.Id != Id)
			continue;

		Notice.Alignment = Alignment;
		Notice.VerticalAlignment = VerticalAlignment;

		emit PageLayoutChanged();
		return;
	}
}

void swInstructions::RemoveNotice(int Id)
{
	const swInstructionsElementKey Key = { swInstructionsPlacementElement::Notice, nullptr, static_cast<swStep>(Id) };
	const size_t NoticeCount = mNotices.size();

	mNotices.erase(std::remove_if(mNotices.begin(), mNotices.end(), [Id](const swInstructionsNotice& Notice)
	{
		return Notice.Id == Id;
	}), mNotices.end());

	if (mNotices.size() == NoticeCount)
		return;

	RemoveElementReferences(Key);

	emit PageLayoutChanged();
}

// use to drop the pointers, offset and lock of a removed element
void swInstructions::RemoveElementReferences(const swInstructionsElementKey& Key)
{
	mPointers.erase(std::remove_if(mPointers.begin(), mPointers.end(), [&Key](const swInstructionsPointer& Pointer)
	{
		return Pointer.Source == Key;
	}), mPointers.end());

	for (swInstructionsPointer& Pointer : mPointers)
		if (Pointer.TargetType == swInstructionsPointerTarget::Element && Pointer.TargetElement == Key)
		{
			Pointer.TargetType = swInstructionsPointerTarget::Point;
			Pointer.TargetAnchor = QPointF(-1.0, -1.0);
		}

	mElementOffsets.erase(Key);
	mElementSizes.erase(Key);
	mLockedPositions.erase(Key);
}

const swInstructionsImage* swInstructions::FindImage(int Id) const
{
	for (const swInstructionsImage& Image : mImages)
		if (Image.Id == Id)
			return &Image;

	return nullptr;
}

int swInstructions::AddImage(swInstructionsPageType PageType, const QString& File)
{
	swInstructionsImage Image;

	Image.Id = mNextImageId++;
	Image.Pages = PageType == swInstructionsPageType::BackCover ? swInstructionsImagePages::Back : swInstructionsImagePages::Front;
	Image.File = File;

	mImages.push_back(Image);

	emit PageLayoutChanged();

	return Image.Id;
}

void swInstructions::SetImage(const swInstructionsImage& Image)
{
	for (swInstructionsImage& Current : mImages)
	{
		if (Current.Id != Image.Id)
			continue;

		// a changed template image is written to the model
		swInstructionsImage Changed = Image;
		Changed.Template = false;

		if (Current.Pages == Changed.Pages && Current.Placement == Changed.Placement && Current.Size == Changed.Size && Current.File == Changed.File && !Current.Template)
			return;

		// a placed image drops its moved position
		if (Current.Placement != Changed.Placement)
			mElementOffsets.erase({ swInstructionsPlacementElement::Image, nullptr, static_cast<swStep>(Image.Id) });

		Current = Changed;

		emit PageLayoutChanged();
		return;
	}
}

void swInstructions::RemoveImage(int Id)
{
	std::vector<swInstructionsImage>::iterator ImageIt = std::find_if(mImages.begin(), mImages.end(), [Id](const swInstructionsImage& Image)
	{
		return Image.Id == Id;
	});

	if (ImageIt == mImages.end())
		return;

	// template images stay hidden in the model
	if (ImageIt->Template)
	{
		ImageIt->Pages = swInstructionsImagePages::None;
		ImageIt->Template = false;
	}
	else
		mImages.erase(ImageIt);

	RemoveElementReferences({ swInstructionsPlacementElement::Image, nullptr, static_cast<swStep>(Id) });

	emit PageLayoutChanged();
}

const swInstructionsPointer* swInstructions::FindPointer(int Id) const
{
	for (const swInstructionsPointer& Pointer : mPointers)
		if (Pointer.Id == Id)
			return &Pointer;

	return nullptr;
}

int swInstructions::AddPointer(size_t PageIndex, const swInstructionsElementKey& Source, const QPointF& TipPosition)
{
	swInstructionsPointer Pointer;

	Pointer.Id = mNextPointerId++;
	Pointer.PageKey = GetPageElementsKey(PageIndex);
	Pointer.Source = Source;
	Pointer.TargetElement = { swInstructionsPlacementElement::None, nullptr, 0 };
	Pointer.TipPosition = TipPosition;

	mPointers.push_back(Pointer);

	emit PageLayoutChanged();

	return Pointer.Id;
}

void swInstructions::SetPointerTarget(int Id, swInstructionsPointerTarget TargetType, const swInstructionsElementKey& TargetElement, int TargetPart, const QPointF& TipPosition, const QPointF& TargetAnchor)
{
	for (swInstructionsPointer& Pointer : mPointers)
	{
		if (Pointer.Id != Id)
			continue;

		Pointer.TargetType = TargetType;
		Pointer.TargetElement = TargetElement;
		Pointer.TargetPart = TargetPart;
		Pointer.TipPosition = TipPosition;
		Pointer.TargetAnchor = TargetAnchor;

		emit PageLayoutChanged();
		return;
	}
}

void swInstructions::SetPointerBend(int Id, qreal Bend)
{
	for (swInstructionsPointer& Pointer : mPointers)
	{
		if (Pointer.Id != Id || Pointer.Bend == Bend)
			continue;

		Pointer.Bend = qBound(0.0, Bend, 1.0);

		emit PageLayoutChanged();
		return;
	}
}

void swInstructions::SetPointerReach(int Id, qreal Reach)
{
	for (swInstructionsPointer& Pointer : mPointers)
	{
		if (Pointer.Id != Id || Pointer.Reach == Reach)
			continue;

		Pointer.Reach = Reach < 0.0 ? -1.0 : Reach;

		emit PageLayoutChanged();
		return;
	}
}

void swInstructions::SetPointerAnchor(int Id, int Anchor)
{
	for (swInstructionsPointer& Pointer : mPointers)
	{
		if (Pointer.Id != Id || Pointer.Anchor == Anchor)
			continue;

		Pointer.Anchor = Anchor;

		emit PageLayoutChanged();
		return;
	}
}

void swInstructions::SetPointerStyle(int Id, int Style, int Tip)
{
	for (swInstructionsPointer& Pointer : mPointers)
	{
		if (Pointer.Id != Id || (Pointer.Style == Style && Pointer.Tip == Tip))
			continue;

		Pointer.Style = Style;
		Pointer.Tip = Tip;

		emit PageLayoutChanged();
		return;
	}
}

void swInstructions::RemovePointer(int Id)
{
	const size_t PointerCount = mPointers.size();

	mPointers.erase(std::remove_if(mPointers.begin(), mPointers.end(), [Id](const swInstructionsPointer& Pointer)
	{
		return Pointer.Id == Id;
	}), mPointers.end());

	if (mPointers.size() != PointerCount)
		emit PageLayoutChanged();
}

swInstructionsClipboard swInstructions::CopyElements(const std::vector<swInstructionsElementKey>& Keys) const
{
	swInstructionsClipboard Clipboard;

	for (const swInstructionsElementKey& Key : Keys)
	{
		const int Id = static_cast<int>(std::get<2>(Key));
		const swInstructionsPlacementElement Element = std::get<0>(Key);

		if (const swInstructionsNotice* Notice = Element == swInstructionsPlacementElement::Notice ? FindNotice(Id) : nullptr)
			Clipboard.Notices.push_back(*Notice);
		else if (const swInstructionsImage* Image = Element == swInstructionsPlacementElement::Image ? FindImage(Id) : nullptr)
			Clipboard.Images.push_back(*Image);
		else if (const swInstructionsPointer* Pointer = Element == swInstructionsPlacementElement::Pointer ? FindPointer(Id) : nullptr)
			Clipboard.Pointers.push_back(*Pointer);
		else
			continue;

		// sizes and moves come along

		const std::map<swInstructionsElementKey, QSizeF>::const_iterator SizeIt = mElementSizes.find(Key);

		if (SizeIt != mElementSizes.end())
			Clipboard.Sizes[Key] = SizeIt->second;

		const std::map<swInstructionsElementKey, QPointF>::const_iterator OffsetIt = mElementOffsets.find(Key);

		if (OffsetIt != mElementOffsets.end())
			Clipboard.Offsets[Key] = OffsetIt->second;
	}

	return Clipboard;
}

std::vector<swInstructionsElementKey> swInstructions::PasteElements(const swInstructionsClipboard& Clipboard, size_t PageIndex, const QPointF& Shift)
{
	std::vector<swInstructionsElementKey> PastedKeys;
	const swInstructionsPageKey PageKey = GetPageElementsKey(PageIndex);

	// the copy takes the size and the shifted move
	auto AddKey = [this, &Clipboard, &PastedKeys](swInstructionsPlacementElement Element, int OldId, int NewId, const QPointF& OffsetShift)
	{
		const swInstructionsElementKey OldKey = { Element, nullptr, static_cast<swStep>(OldId) };
		const swInstructionsElementKey NewKey = { Element, nullptr, static_cast<swStep>(NewId) };
		const std::map<swInstructionsElementKey, QSizeF>::const_iterator SizeIt = Clipboard.Sizes.find(OldKey);
		const std::map<swInstructionsElementKey, QPointF>::const_iterator OffsetIt = Clipboard.Offsets.find(OldKey);
		const QPointF Offset = (OffsetIt != Clipboard.Offsets.end() ? OffsetIt->second : QPointF()) + OffsetShift;

		if (SizeIt != Clipboard.Sizes.end())
			mElementSizes[NewKey] = SizeIt->second;

		if (!Offset.isNull())
			mElementOffsets[NewKey] = Offset;

		PastedKeys.push_back(NewKey);
	};

	for (swInstructionsNotice Notice : Clipboard.Notices)
	{
		const int OldId = Notice.Id;

		// page notices land on this page
		Notice.Id = mNextNoticeId++;
		Notice.Position += Shift;

		if (Notice.Pages == swInstructionsImagePages::None)
			Notice.PageKey = PageKey;

		mNotices.push_back(Notice);
		AddKey(swInstructionsPlacementElement::Notice, OldId, Notice.Id, QPointF());
	}

	for (swInstructionsImage Image : Clipboard.Images)
	{
		const int OldId = Image.Id;

		// images sit on a cell, the move shifts them
		Image.Id = mNextImageId++;
		Image.Template = false;

		mImages.push_back(Image);
		AddKey(swInstructionsPlacementElement::Image, OldId, Image.Id, Shift);
	}

	for (swInstructionsPointer Pointer : Clipboard.Pointers)
	{
		// pointers start at an element of their page
		if (Pointer.PageKey != PageKey)
			continue;

		const int OldId = Pointer.Id;

		Pointer.Id = mNextPointerId++;
		Pointer.TipPosition += Shift;

		mPointers.push_back(Pointer);
		AddKey(swInstructionsPlacementElement::Pointer, OldId, Pointer.Id, QPointF());
	}

	if (!PastedKeys.empty())
		emit PageLayoutChanged();

	return PastedKeys;
}
