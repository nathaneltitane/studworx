#pragma once

#include "sw_math.h"
#include "sw_profile.h"

enum class swInstructionsPageUnits
{
	Pixels,
	Millimeters,
	Centimeters,
	Inches
};

enum class swInstructionsBoundaries
{
	Off,
	OuterEdge,
	MarginPadding,
	EveryEdge
};

// Gets the points per page unit.
float swGetPageUnitScale(swInstructionsPageUnits Units);
QString swGetPageUnitSuffix(swInstructionsPageUnits Units);

struct swInstructionsPageSetup
{
	float Width;
	float Height;
	float MarginLeft;
	float MarginRight;
	float MarginTop;
	float MarginBottom;
};

enum class swInstructionsDirection
{
	Horizontal,
	Vertical
};

struct swInstructionsPageSettings
{
	int Rows = 1;
	int Columns = 1;
	swInstructionsDirection Direction = swInstructionsDirection::Horizontal;
	bool FinalModel = false;

	bool operator== (const swInstructionsPageSettings& Other) const
	{
		return Rows == Other.Rows && Columns == Other.Columns && Direction == Other.Direction && FinalModel == Other.FinalModel;
	}
};

enum class swInstructionsPropertyMode
{
	NotSet,
	Default,
	Model,
	StepForward,
	StepOnly
};

// Both covers show the same elements, each with its own set of properties.
enum class swInstructionsCoverElement
{
	Title,
	Author,
	ProductId,
	Version,
	PublicationDate,
	Url,
	Contact,
	Copyright,
	PublicationNotice,
	Comments,
	Measurements,
	Weight,
	Pieces
};

constexpr int SW_COVER_ELEMENT_COUNT = 13;
constexpr int SW_COVER_FIELD_COUNT = 13;
constexpr int SW_COVER_PROPERTY_COUNT = 2 * SW_COVER_ELEMENT_COUNT * SW_COVER_FIELD_COUNT;

enum class swInstructionsPropertyType
{
	ShowStepNumber,
	ShowStepPLI,
	StepNumberFont,
	StepNumberColor,
	StepBackgroundColor,
	PLIBackgroundColor,
	PLIFont,
	PLITextColor,
	PLIBorderColor,
	PLIBorderWidth,

	// Parts list spacing and alignment.
	PLIShowCounts,
	PLISortOrder,
	ShowStepDividers,
	StepDividerColor,
	StepDividerWidth,
	ShowBillOfMaterials,
	UseDocumentFont,
	UseDocumentFontSize,
	DocumentFont,
	ShowCoverPage,
	PageBackgroundColor,
	StepNumberPlacement,
	StepNumberAnchor,
	PLIPlacement,
	PLIAnchor,
	BillOfMaterialsPlacement,
	PLIThumbnailSize,
	PLIThumbnailBackgroundColor, // read as parts list background
	CoverImagePlacement,
	StepImagePlacement,
	CoverBackgroundColor,
	BillOfMaterialsBackgroundColor,
	ShowCoverBlankPage,
	ShowBillOfMaterialsBlankPage,
	UseDocumentUnits,
	DocumentUnits,
	ShowPageNumber,
	PageNumberFormat,
	PageNumberFont,
	PageNumberColor,
	PageNumberPlacement,
	ShowPageNumberAllPages,
	StepImageMargin,
	StepImagePadding,
	StepImageBorderWidth,
	StepImageBorderColor,
	StepImageBackgroundColor,
	StepNumberMargin,
	StepNumberPadding,
	StepNumberBorderWidth,
	StepNumberBorderColor,
	StepNumberBackgroundColor,
	PLIMargin,
	PLIPadding,
	PageNumberMargin,
	PageNumberPadding,
	PageNumberBorderWidth,
	PageNumberBorderColor,
	PageNumberBackgroundColor,
	CoverImageMargin,
	CoverImagePadding,
	CoverImageBorderWidth,
	CoverImageBorderColor,
	CoverImageBackgroundColor,
	NoticeFont,
	NoticeTextColor,
	NoticeMargin,
	NoticePadding,
	NoticeBorderWidth,
	NoticeBorderColor,
	NoticeBackgroundColor,
	PointerStyle,
	PointerTip,
	PointerColor,
	PointerWidth,
	PointerTipSize,
	PLILayout,
	PLIAlignment,
	ModelProjection,
	FadePreviousSteps,
	FadeColor,
	HighlightNewParts,
	HighlightColor,
	StepImageCornerRadius,
	StepNumberCornerRadius,
	PLICornerRadius,
	PageNumberCornerRadius,
	CoverImageCornerRadius,
	NoticeCornerRadius,
	BillOfMaterialsTitleFont,
	BillOfMaterialsTitleColor,
	SubmodelBackgroundColor,
	StepImageSize,
	CoverImageSize,
	BillOfMaterialsTitleText,
	BillOfMaterialsTitleFile,
	StepDividerMargin,
	StepDividerPadding,
	StepDividerRounded,
	StepDividerHorizontal,
	StepDividerVertical,
	HighlightWidth,
	FinalModelBorderWidth,
	FinalModelBorderColor,
	FinalModelMargin,
	BillOfMaterialsShowPartIds,
	ShowBillOfMaterialsPartCount,
	ShowPagePartCount,
	PartCountText,
	PartCountFile,
	PartCountFont,
	PartCountColor,
	PagePartCountFont,
	PagePartCountColor,
	BillOfMaterialsPartCountPlacement,
	PagePartCountPlacement,
	ShowSubmodelPreview,
	SubmodelPreviewSize,
	SubmodelPreviewPlacement,
	SubmodelPreviewMargin,
	SubmodelPreviewPadding,
	SubmodelPreviewBorderWidth,
	SubmodelPreviewBorderColor,
	SubmodelPreviewBackgroundColor,
	SubmodelPreviewCornerRadius,
	SubmodelPreviewPartCountPlacement,
	PLICountPlacement,
	ShowBackCover,
	ShowHeaderBand,
	ShowFooterBand,
	ShowLeftBand,
	ShowRightBand,
	HeaderBandSize,
	FooterBandSize,
	LeftBandSize,
	RightBandSize,
	HeaderBandColor,
	FooterBandColor,
	LeftBandColor,
	RightBandColor,
	HeaderBandReserve,
	FooterBandReserve,
	LeftBandReserve,
	RightBandReserve,
	ShowWatermark,
	WatermarkOpacity,

	// text alignment inside the box
	StepNumberAlignment,
	StepNumberVerticalAlignment,
	BillOfMaterialsTitleAlignment,
	BillOfMaterialsTitleVerticalAlignment,
	PageNumberAlignment,
	PageNumberVerticalAlignment,
	PartCountAlignment,
	PartCountVerticalAlignment,
	NoticeAlignment,
	NoticeVerticalAlignment,
	ShowCoverImage,
	ImageMargin,
	ImagePadding,
	ImageBorderWidth,
	ImageBorderColor,
	ImageBackgroundColor,
	ImageCornerRadius,
	PointerBendRadius,
	ShowFinalModelPage,
	BlankPageBackgroundColor,
	PartsListQuantityMargin,
	PartsListQuantityPadding,
	PartsListQuantityBorderWidth,
	PartsListQuantityBorderColor,
	PartsListQuantityBackgroundColor,
	PartsListQuantityCornerRadius,
	FinalModelPadding,
	FinalModelCornerRadius,
	StepMargin,
	StepPadding,
	StepBorderWidth,
	StepBorderColor,
	StepCornerRadius,
	FinalModelBackgroundColor,
	HeaderBandMargin,
	HeaderBandPadding,
	HeaderBandBorderWidth,
	HeaderBandBorderColor,
	HeaderBandCornerRadius,
	FooterBandMargin,
	FooterBandPadding,
	FooterBandBorderWidth,
	FooterBandBorderColor,
	FooterBandCornerRadius,
	LeftBandMargin,
	LeftBandPadding,
	LeftBandBorderWidth,
	LeftBandBorderColor,
	LeftBandCornerRadius,
	RightBandMargin,
	RightBandPadding,
	RightBandBorderWidth,
	RightBandBorderColor,
	RightBandCornerRadius,
	WatermarkMargin,
	WatermarkPadding,
	WatermarkBorderWidth,
	WatermarkBorderColor,
	WatermarkCornerRadius,
	WatermarkBackgroundColor,
	LockHomeView,
	WatermarkFont,
	WatermarkTextColor,
	WatermarkRotation,
	ShowBillOfMaterialsTitle,
	BillOfMaterialsTitlePlacement,
	BillOfMaterialsFont,
	BillOfMaterialsTextColor,
	BillOfMaterialsShowCounts,
	BillOfMaterialsSortOrder,
	BillOfMaterialsLayout,
	BillOfMaterialsAlignment,
	BillOfMaterialsThumbnailSize,
	BillOfMaterialsCountPlacement,
	BillOfMaterialsTitleMargin,
	BillOfMaterialsTitlePadding,
	BillOfMaterialsTitleBorderWidth,
	BillOfMaterialsTitleBorderColor,
	BillOfMaterialsTitleCornerRadius,
	BillOfMaterialsTitleBackgroundColor,
	BillOfMaterialsListMargin,
	BillOfMaterialsListPadding,
	BillOfMaterialsListBorderWidth,
	BillOfMaterialsListBorderColor,
	BillOfMaterialsListCornerRadius,
	BillOfMaterialsListBackgroundColor,
	BillOfMaterialsQuantityMargin,
	BillOfMaterialsQuantityPadding,
	BillOfMaterialsQuantityBorderWidth,
	BillOfMaterialsQuantityBorderColor,
	BillOfMaterialsQuantityCornerRadius,
	BillOfMaterialsQuantityBackgroundColor,
	BillOfMaterialsPartCountMargin,
	BillOfMaterialsPartCountPadding,
	BillOfMaterialsPartCountBorderWidth,
	BillOfMaterialsPartCountBorderColor,
	BillOfMaterialsPartCountCornerRadius,
	BillOfMaterialsPartCountBackgroundColor,
	AssemblyBackgroundColor,
	StepDividerFinalModel,
	CompletionBackgroundColor,
	CompletionMargin,
	CompletionPadding,
	CompletionBorderWidth,
	CompletionBorderColor,
	CompletionCornerRadius,
	CompletionBoxBackgroundColor,
	PLIReserveArea,
	CoverElements,
	Count = CoverElements + SW_COVER_PROPERTY_COUNT
};

// The page number with or without count.
enum class swInstructionsPageNumberFormat
{
	Number,
	NumberOfPages
};

// The box around every page element.
enum class swInstructionsBoxElement
{
	StepImage,
	StepNumber,
	PartsList,
	PageNumber,
	CoverImage,
	Notice,
	SubmodelPreview,
	Image,
	PartsListQuantity,
	FinalModel,
	StepArea,
	HeaderBand,
	FooterBand,
	LeftBand,
	RightBand,
	Watermark,
	BillOfMaterialsTitle,
	BillOfMaterialsList,
	BillOfMaterialsQuantity,
	BillOfMaterialsPartCount,
	Completion,
	CoverFrontTitle,
	CoverFrontAuthor,
	CoverFrontProductId,
	CoverFrontVersion,
	CoverFrontPublicationDate,
	CoverFrontUrl,
	CoverFrontContact,
	CoverFrontCopyright,
	CoverFrontPublicationNotice,
	CoverFrontComments,
	CoverFrontMeasurements,
	CoverFrontWeight,
	CoverFrontPieces,
	CoverBackTitle,
	CoverBackAuthor,
	CoverBackProductId,
	CoverBackVersion,
	CoverBackPublicationDate,
	CoverBackUrl,
	CoverBackContact,
	CoverBackCopyright,
	CoverBackPublicationNotice,
	CoverBackComments,
	CoverBackMeasurements,
	CoverBackWeight,
	CoverBackPieces,
	Count
};

struct swInstructionsBoxTypes
{
	swInstructionsPropertyType Margin;
	swInstructionsPropertyType Padding;
	swInstructionsPropertyType BorderWidth;
	swInstructionsPropertyType CornerRadius;
	swInstructionsPropertyType BorderColor;
	swInstructionsPropertyType BackgroundColor;
};

// Gets the box of an element.
swInstructionsBoxTypes swGetBoxTypes(swInstructionsBoxElement Element);

// Gets the step setting each bill of materials setting starts from.
const std::vector<std::pair<swInstructionsPropertyType, swInstructionsPropertyType>>& swGetBillOfMaterialsSources();

struct swInstructionsBoxStyle
{
	qreal Margin = 0.0;
	qreal Padding = 0.0;
	qreal BorderWidth = 0.0;
	QColor BorderColor = Qt::transparent;
	QColor BackgroundColor = Qt::transparent;
	qreal CornerRadius = 0.0;

	// The box padding.
	qreal GetInset() const
	{
		return Margin + BorderWidth + Padding;
	}

	// use to round the background outside the border by the margin
	qreal GetBackgroundRadius() const
	{
		return CornerRadius > 0.0 ? CornerRadius + Margin : 0.0;
	}
};

// A pointer line or callout.
enum class swInstructionsPointerStyle
{
	Line,
	Wedge,
	LShape,
	ZShape,
	UShape
};

enum class swInstructionsPointerTip
{
	Arrow,
	Dot,
	HollowDot
};

// What a step element is placed against.
enum class swInstructionsPlacementAnchor
{
	Area,
	StepNumber,
	PartsList
};

// Placement cells around the anchor.
constexpr int SW_PLACEMENT_GRID_SIZE = 5;
constexpr int SW_PLACEMENT_COUNT = SW_PLACEMENT_GRID_SIZE * SW_PLACEMENT_GRID_SIZE;

inline bool swIsInsidePlacement(int Placement)
{
	const int Row = Placement / SW_PLACEMENT_GRID_SIZE;
	const int Column = Placement % SW_PLACEMENT_GRID_SIZE;

	return Row > 0 && Row < SW_PLACEMENT_GRID_SIZE - 1 && Column > 0 && Column < SW_PLACEMENT_GRID_SIZE - 1;
}

// Moves a placement into an area.
inline int swGetValidPlacement(int Placement, bool Inside)
{
	const int Last = SW_PLACEMENT_GRID_SIZE - 1;
	int Row = qBound(0, Placement / SW_PLACEMENT_GRID_SIZE, Last);
	int Column = qBound(0, Placement % SW_PLACEMENT_GRID_SIZE, Last);

	if (Inside)
	{
		Row = qBound(1, Row, Last - 1);
		Column = qBound(1, Column, Last - 1);
	}
	else if (swIsInsidePlacement(Row * SW_PLACEMENT_GRID_SIZE + Column))
	{
		if (Column == 1)
			Column = 0;
		else if (Column == Last - 1)
			Column = Last;
		else if (Row == 1)
			Row = 0;
		else if (Row == Last - 1)
			Row = Last;
		else
		{
			// Center goes right of the element.
			Row = 1;
			Column = Last;
		}
	}

	return Row * SW_PLACEMENT_GRID_SIZE + Column;
}

// The selectable page elements.
enum class swInstructionsPlacementElement
{
	None,
	StepNumber,
	PartsList,
	BillOfMaterials,
	StepImage,
	CoverImage,
	PageNumber,
	Notice,
	Pointer,
	BillOfMaterialsPartCount,
	PagePartCount,
	SubmodelPreview,
	SubmodelPreviewPartCount,
	PartsListCount,
	Image,
	CoverFrontTitle,
	CoverFrontAuthor,
	CoverFrontProductId,
	CoverFrontVersion,
	CoverFrontPublicationDate,
	CoverFrontUrl,
	CoverFrontContact,
	CoverFrontCopyright,
	CoverFrontPublicationNotice,
	CoverFrontComments,
	CoverFrontMeasurements,
	CoverFrontWeight,
	CoverFrontPieces,
	CoverBackTitle,
	CoverBackAuthor,
	CoverBackProductId,
	CoverBackVersion,
	CoverBackPublicationDate,
	CoverBackUrl,
	CoverBackContact,
	CoverBackCopyright,
	CoverBackPublicationNotice,
	CoverBackComments,
	CoverBackMeasurements,
	CoverBackWeight,
	CoverBackPieces
};

// The publication texts share one set of properties.
enum class swInstructionsPublicationField
{
	Show,
	Text,
	Font,
	Color,
	Placement,
	Alignment,
	VerticalAlignment,
	Margin,
	Padding,
	BorderWidth,
	BorderColor,
	BackgroundColor,
	CornerRadius
};

struct swInstructionsPublicationElement
{
	swInstructionsPlacementElement Element;
	swInstructionsBoxElement Box;
	swInstructionsPropertyType Show;
	swInstructionsCoverElement Kind;
	bool Back;
};

// Gets the cover elements, front first.
const std::vector<swInstructionsPublicationElement>& swGetPublicationElements();

// Gets a property of a cover element.
inline swInstructionsPropertyType swGetPublicationType(const swInstructionsPublicationElement& Publication, swInstructionsPublicationField Field)
{
	return static_cast<swInstructionsPropertyType>(static_cast<int>(Publication.Show) + static_cast<int>(Field));
}

// Finds the cover element of a placement element.
const swInstructionsPublicationElement* swFindPublicationElement(swInstructionsPlacementElement Element);

// use to name a cover element, with and without its cover
QString swGetPublicationKindName(const swInstructionsPublicationElement& Publication);
QString swGetPublicationName(const swInstructionsPublicationElement& Publication);

// Moved elements keep their offset.
using swInstructionsElementKey = std::tuple<swInstructionsPlacementElement, swModel*, swStep>;

// The grid size in points.
constexpr float SW_GRID_SIZE_MINIMUM = 2.0f;
constexpr float SW_GRID_SIZE_MAXIMUM = 144.0f;
constexpr int SW_GRID_DIVISIONS_MAXIMUM = 64;

// The minimum render size of a page.
constexpr qreal SW_INSTRUCTIONS_RENDER_SIZE = 3300.0;

// The memory kept for rendered step pictures and parts lists, in kilobytes.
constexpr int SW_INSTRUCTIONS_STEP_IMAGE_CACHE = 192 * 1024;
constexpr int SW_INSTRUCTIONS_PARTS_LIST_CACHE = 32 * 1024;

struct swInstructionsProperty
{
	swInstructionsPropertyMode Mode = swInstructionsPropertyMode::NotSet;
	QVariant Value;
};

using swInstructionsProperties = std::array<swInstructionsProperty, static_cast<int>(swInstructionsPropertyType::Count)>;

struct swInstructionsStep
{
	QRectF Rect;
	swModel* Model;
	swStep Step;
	bool FinalModel = false;

	swInstructionsProperties Properties;
};

enum class swInstructionsPageType
{
	Steps,
	Cover,
	BillOfMaterials,
	Blank,
	BackCover
};

using swInstructionsStepKey = std::pair<swModel*, swStep>;

// The page notices and pointers belong to.
using swInstructionsPageKey = std::tuple<swInstructionsPageType, swModel*, swStep, int>;

// Where an image or notice shows: covers, bands or the watermark.
enum class swInstructionsImagePages
{
	None,
	Front,
	Back,
	Both,
	Header,
	Footer,
	Left,
	Right,
	Watermark
};

// A free text box on a page.
struct swInstructionsNotice
{
	int Id = 0;
	swInstructionsPageKey PageKey;

	// none is its own page, a band is relative to the band
	swInstructionsImagePages Pages = swInstructionsImagePages::None;
	QPointF Position;
	QString Text;
	QString File;

	// minus one follows the notice properties
	int Alignment = -1;
	int VerticalAlignment = -1;
};

// A picture linked by path, such as a logo.
struct swInstructionsImage
{
	int Id = 0;
	swInstructionsImagePages Pages = swInstructionsImagePages::Front;
	int Placement = 12;
	qreal Size = 25.0;
	QString File;

	// default template images are not written to the model
	bool Template = false;
};

// A pointer from an element anchor.
constexpr int SW_POINTER_ANCHOR_AUTO = -1;
constexpr int SW_POINTER_ANCHOR_COUNT = 8;

enum class swInstructionsPointerTarget
{
	Point,
	Element,
	Part
};

struct swInstructionsPointer
{
	int Id = 0;
	swInstructionsPageKey PageKey;
	swInstructionsElementKey Source;
	int Anchor = SW_POINTER_ANCHOR_AUTO;
	swInstructionsPointerTarget TargetType = swInstructionsPointerTarget::Point;
	swInstructionsElementKey TargetElement;
	int TargetPart = -1;
	QPointF TipPosition;

	// The tip spot as a fraction of the target box, negative when the tip finds the border.
	QPointF TargetAnchor = QPointF(-1.0, -1.0);

	// The Z shape bend as a fraction from the base to the tip.
	qreal Bend = 0.5;

	// How far the U shape reaches out, negative for automatic.
	qreal Reach = -1.0;

	// Pointers default to the pointer properties.
	int Style = -1;
	int Tip = -1;
};

// Copied annotations, each keyed by its old element key.
struct swInstructionsClipboard
{
	std::vector<swInstructionsNotice> Notices;
	std::vector<swInstructionsImage> Images;
	std::vector<swInstructionsPointer> Pointers;
	std::map<swInstructionsElementKey, QSizeF> Sizes;
	std::map<swInstructionsElementKey, QPointF> Offsets;

	bool IsEmpty() const
	{
		return Notices.empty() && Images.empty() && Pointers.empty();
	}
};

struct swInstructionsPage
{
	swInstructionsPageSettings Settings;
	std::vector<swInstructionsStep> Steps;
	swInstructionsPageType Type = swInstructionsPageType::Steps;
	swModel* Model = nullptr;
	swInstructionsStepKey BlankKey = { nullptr, 0 };
};

// The lock and freeze of the page being drawn.
struct swInstructionsPageState
{
	const swInstructionsProperties* FrozenProperties = nullptr;
	const std::map<swInstructionsElementKey, QPointF>* LockedPositions = nullptr;
};

struct swInstructionsModel
{
	std::vector<swInstructionsProperties> StepProperties;
};

// The instructions are main model metas.
void swRenameInstructionsModels(QStringList& CommandLines, const std::map<QString, QString>& Names);

class swInstructions : public QObject
{
	Q_OBJECT

public:
	swInstructions(Project* Project = nullptr);

	void SetProject(Project* Project);

	void SetDefaultPageSettings(const swInstructionsPageSettings& PageSettings);
	swInstructionsPageSettings GetPageSettings(size_t PageIndex) const;
	void SetPageSettings(size_t PageIndex, const swInstructionsPageSettings& PageSettings);
	void SetPageDividerPreset(size_t PageIndex, int Rows, int Columns);
	bool CanInsertBlankPage(size_t PageIndex) const;
	void InsertBlankPage(size_t PageIndex);
	void RemoveBlankPage(size_t PageIndex);
	void SetPageSetup(const swInstructionsPageSetup& PageSetup);
	void ResetPageSetup();
	static void LoadDefaultPageSetup();
	void Update();

	bool GetBoolProperty(swInstructionsPropertyType Type, swModel* Model, swStep Step) const;
	int GetIntProperty(swInstructionsPropertyType Type, swModel* Model, swStep Step) const;
	QColor GetColorProperty(swInstructionsPropertyType Type, swModel* Model, swStep Step) const;
	QFont GetFontProperty(swInstructionsPropertyType Type, swModel* Model, swStep Step, bool ApplyDocumentFont = true) const;

	void SetDefaultBool(swInstructionsPropertyType Type, bool Enabled);
	void SetDefaultInt(swInstructionsPropertyType Type, int Value);
	void SetDefaultColor(swInstructionsPropertyType Type, const QColor& Color);
	void SetDefaultFont(swInstructionsPropertyType Type, const QFont& Font);
	void SetDefaultText(swInstructionsPropertyType Type, const QString& Text);
	QString GetTextProperty(swInstructionsPropertyType Type) const;
	QString GetSourcedText(swInstructionsPropertyType TextType, swInstructionsPropertyType FileType, const QString& Automatic) const;
	QString GetTypedText(swInstructionsPropertyType TextType, const QString& Automatic) const;
	QString GetModelFieldText(const swModel* Model, swInstructionsPlacementElement Element) const;
	QString GetModelTitle(const swModel* Model) const;
	QString FillModelCodes(QString Text) const;

	// The model the instructions are made for.
	swModel* GetMainModel() const;

	// Counts the visible parts of a model with its submodels.
	static int GetModelPartCount(const swModel* Model);

	// Counts the new parts of every step on a page like the parts list.
	static int GetPageStepPartCount(const swInstructionsPage& Page);

	// Fills the part count text, from the file, the typed text or the default.
	QString GetPartCountText(int Count) const;
	QString GetFileText(const QString& FileName) const;
	void ResetProperties();

	// Properties set on a single page.
	static bool IsPageProperty(swInstructionsPropertyType Type);
	bool HasPageProperty(size_t PageIndex, swInstructionsPropertyType Type) const;
	void SetPageScope(size_t PageIndex, swInstructionsPropertyType Type, bool PageOnly);
	void ApplyPageToAllPages(size_t PageIndex, swInstructionsPropertyType Type);
	int GetPageIntProperty(size_t PageIndex, swInstructionsPropertyType Type) const;
	bool GetPageBoolProperty(size_t PageIndex, swInstructionsPropertyType Type) const;
	QColor GetPageColorProperty(size_t PageIndex, swInstructionsPropertyType Type) const;
	void SetPageInt(size_t PageIndex, swInstructionsPropertyType Type, int Value);
	void SetPageColor(size_t PageIndex, swInstructionsPropertyType Type, const QColor& Color);

	// Cover and parts list backgrounds.
	QColor GetPageBackgroundColor(size_t PageIndex) const;
	bool GetPageTitleModel(size_t PageIndex, const swModel*& TitleModel) const;
	bool IsCompletionPage(size_t PageIndex) const;

	QPointF GetElementOffset(const swInstructionsElementKey& Key) const;
	bool HasElementOffset(const swInstructionsElementKey& Key) const;
	void SetElementOffsets(const std::map<swInstructionsElementKey, QPointF>& Offsets, const std::map<swInstructionsElementKey, QSizeF>& Sizes = {});
	QSizeF GetPlacedSize(const swInstructionsElementKey& Key, const QSizeF& Size) const;
	void ResetElementOffset(const swInstructionsElementKey& Key);

	// An invalid size fits the content.
	QSizeF GetElementSize(const swInstructionsElementKey& Key) const;
	void SetElementSize(const swInstructionsElementKey& Key, const QSizeF& Size);
	void SetElementPlacement(swInstructionsPropertyType Type, int Placement, const swInstructionsElementKey& Key);

	// The box of an element.
	swInstructionsBoxStyle GetBoxStyle(swInstructionsBoxElement Element, swModel* Model, swStep Step) const;

	// Gets the box of a page element.
	swInstructionsBoxStyle GetPageBoxStyle(size_t PageIndex, swInstructionsBoxElement Element) const;

	// Notices and pointers stay on their page.
	swInstructionsPageKey GetPageElementsKey(size_t PageIndex) const;

	const std::vector<swInstructionsNotice>& GetNotices() const
	{
		return mNotices;
	}

	const swInstructionsNotice* FindNotice(int Id) const;
	int AddNotice(size_t PageIndex, const QPointF& Position, const QString& Text, const QString& File = QString(), swInstructionsImagePages Pages = swInstructionsImagePages::None);
	void SetNoticeText(int Id, const QString& Text, const QString& File);
	void SetNoticePosition(int Id, const QPointF& Position, const QSizeF& Size = QSizeF());
	void SetNoticePages(int Id, swInstructionsImagePages Pages, size_t PageIndex, const QPointF& Position);
	void SetNoticeAlignment(int Id, int Alignment, int VerticalAlignment);
	void RemoveNotice(int Id);

	const std::vector<swInstructionsPointer>& GetPointers() const
	{
		return mPointers;
	}

	// Images show on the front and back covers.
	const std::vector<swInstructionsImage>& GetImages() const
	{
		return mImages;
	}

	const swInstructionsImage* FindImage(int Id) const;
	int AddImage(swInstructionsPageType PageType, const QString& File);
	void SetImage(const swInstructionsImage& Image);
	void RemoveImage(int Id);
	QString GetFilePath(const QString& FileName) const;

	// Templates hold the document settings and images.
	bool SaveTemplate(const QString& FileName) const;
	bool ApplyTemplate(const QString& FileName);
	static QStringList ReadTemplateLines(const QString& FileName);

	const swInstructionsPointer* FindPointer(int Id) const;
	int AddPointer(size_t PageIndex, const swInstructionsElementKey& Source, const QPointF& TipPosition);
	void SetPointerTarget(int Id, swInstructionsPointerTarget TargetType, const swInstructionsElementKey& TargetElement, int TargetPart, const QPointF& TipPosition, const QPointF& TargetAnchor);
	void SetPointerAnchor(int Id, int Anchor);
	void SetPointerBend(int Id, qreal Bend);
	void SetPointerReach(int Id, qreal Reach);
	void SetPointerStyle(int Id, int Style, int Tip);
	void RemovePointer(int Id);

	// Copies of notices, images and pointers with their sizes and offsets.
	swInstructionsClipboard CopyElements(const std::vector<swInstructionsElementKey>& Keys) const;
	bool CanPasteElements(const swInstructionsClipboard& Clipboard, size_t PageIndex) const;
	std::vector<swInstructionsElementKey> PasteElements(const swInstructionsClipboard& Clipboard, size_t PageIndex, const QPointF& Shift);

	// A locked element keeps its position.
	bool IsElementLocked(const swInstructionsElementKey& Key) const;
	QPointF GetLockedPosition(const swInstructionsElementKey& Key) const;
	void LockElement(const swInstructionsElementKey& Key, const QPointF& Position);
	void UnlockElement(const swInstructionsElementKey& Key, const QPointF& Offset);

	// A locked page keeps its element positions.
	bool IsPageLocked(size_t PageIndex) const;
	void LockPage(size_t PageIndex, const std::map<swInstructionsElementKey, QPointF>& Positions);
	void UnlockPage(size_t PageIndex);

	// A frozen page keeps a copy of its settings.
	bool IsPageFrozen(size_t PageIndex) const;
	void FreezePage(size_t PageIndex);
	void UnfreezePage(size_t PageIndex);
	swInstructionsPageState GetPageState(size_t PageIndex) const;

	// The page grid shows on screen only.
	bool IsGridShown() const
	{
		return mShowGrid;
	}

	bool IsSnapToGrid() const
	{
		return mSnapToGrid;
	}

	// Anchor snapping and grid snapping exclude each other.
	bool IsSnapToAnchors() const
	{
		return mSnapToAnchors;
	}

	float GetGridSize() const
	{
		return mGridSize;
	}

	// The grid size and its divisions.
	int GetGridDivisions() const
	{
		return mGridDivisions;
	}

	float GetGridSnapSize() const
	{
		return mGridSize / mGridDivisions;
	}

	QColor GetGridPrimaryColor() const
	{
		return mGridPrimaryColor;
	}

	QColor GetGridSecondaryColor() const
	{
		return mGridSecondaryColor;
	}

	// The area the grid covers.
	bool IsGridWithinMargins() const
	{
		return mGridWithinMargins;
	}

	QRectF GetGridRect() const;

	// The margins show on screen only.
	bool IsMarginsShown() const
	{
		return mShowMargins;
	}

	QColor GetMarginColor() const
	{
		return mMarginColor;
	}

	// The element boxes show on screen only.
	bool IsElementMarginsShown() const
	{
		return mShowElementMargins;
	}

	bool IsElementPaddingShown() const
	{
		return mShowElementPadding;
	}

	bool IsElementAnchorsShown() const
	{
		return mShowElementAnchors;
	}

	swInstructionsBoundaries GetElementBoundaries() const
	{
		return mElementBoundaries;
	}

	bool IsLayoutAreasShown() const
	{
		return mShowLayoutAreas;
	}

	bool IsRulersShown() const
	{
		return mShowRulers;
	}

	QColor GetElementMarginColor() const
	{
		return mElementMarginColor;
	}

	QColor GetElementPaddingColor() const
	{
		return mElementPaddingColor;
	}

	QColor GetElementAnchorColor() const
	{
		return mElementAnchorColor;
	}

	QColor GetRulerPointerColor() const
	{
		return mRulerPointerColor;
	}

	QColor GetBoundaryEdgeColor() const
	{
		return mBoundaryEdgeColor;
	}

	QColor GetBoundaryBorderColor() const
	{
		return mBoundaryBorderColor;
	}

	QColor GetLayoutAreaColor() const
	{
		return mLayoutAreaColor;
	}

	// Rulers measure in page units.
	swInstructionsPageUnits GetRulerUnits() const;


	// Lengths use the margin units.
	swInstructionsPageUnits GetPageUnits() const;

	// The document units replace page units.
	std::optional<swInstructionsPageUnits> GetDocumentUnits() const;

	void SetShowGrid(bool ShowGrid);
	void SetSnapToGrid(bool SnapToGrid);
	void SetSnapToAnchors(bool SnapToAnchors);
	void SetGridSize(float GridSize);
	void SetGridDivisions(int Divisions);
	void SetGridPrimaryColor(const QColor& Color);
	void SetGridSecondaryColor(const QColor& Color);
	void SetMarginColor(const QColor& Color);
	void UpdateColors();
	void SetGridWithinMargins(bool WithinMargins);
	void SetShowMargins(bool ShowMargins);
	void SetShowElementMargins(bool ShowElementMargins);
	void SetShowElementPadding(bool ShowElementPadding);
	void SetShowElementAnchors(bool ShowElementAnchors);
	void SetElementBoundaries(swInstructionsBoundaries ElementBoundaries);
	void SetShowLayoutAreas(bool ShowLayoutAreas);
	void SetShowRulers(bool ShowRulers);
	bool CanUndo() const;
	bool CanRedo() const;
	void Undo();
	void Redo();

//protected:
	std::vector<swInstructionsPage> mPages;
	swInstructionsPageSettings mPageSettings;
	std::map<swInstructionsStepKey, swInstructionsPageSettings> mPageSettingsOverrides;
	std::map<swInstructionsStepKey, swInstructionsProperties> mPagePropertyOverrides;
	std::map<swInstructionsElementKey, QPointF> mElementOffsets;
	std::map<swInstructionsElementKey, QSizeF> mElementSizes;

	// sizes of moved elements and notices when placed, kept on first draw
	mutable std::map<swInstructionsElementKey, QSizeF> mPlacedSizes;
	std::map<swInstructionsElementKey, QPointF> mLockedPositions;
	std::map<swInstructionsPageKey, std::map<swInstructionsElementKey, QPointF>> mLockedPages;
	std::map<swInstructionsPageKey, swInstructionsProperties> mFrozenPages;
	swInstructionsPageState mPageState;
	std::multiset<swInstructionsStepKey> mBlankPages;
	std::vector<swInstructionsNotice> mNotices;
	std::vector<swInstructionsPointer> mPointers;
	std::vector<swInstructionsImage> mImages;
	swInstructionsPageSettings mDefaultPageSettings;
	swInstructionsPageSetup mPageSetup;
	swInstructionsProperties mStepProperties;
	swInstructionsProperties mDefaultProperties;

	std::map<swModel*, swInstructionsModel> mModels;

	static swInstructionsProperties CreateDefaultProperties();

signals:
	void StepSettingsChanged(swModel* Model, swStep Step);
	void PageSetupChanged();
	void PageLayoutChanged();
	void GridChanged();
	void UnitsChanged();
	void HistoryChanged();

protected:
	void SetDefaultProperties();
	void RemoveElementReferences(const swInstructionsElementKey& Key);
	void LoadDefaultTemplate();
	void LoadCommandLines(const QStringList& CommandLines, bool ModelLines = false);
	QStringList GetCommandLines(bool Template = false) const;
	void StoreCommandLines();
	void ReloadCommandLines();
	void RestoreCommandLines(std::vector<QStringList>& From, std::vector<QStringList>& To);
	void CreatePages();
	void AddDefaultPages(swModel* Model, std::vector<const swModel*>& AddedModels);
	void AddBlankPages(swInstructionsPage& Page, const swInstructionsStepKey& Key);
	void AddDefaultBlankPage(const swInstructionsStepKey& Key);
	void LayoutPage(swInstructionsPage& Page) const;
	void FinishPage(swInstructionsPage& Page);
	swInstructionsPageSettings GetStepPageSettings(const swInstructionsStepKey& Key) const;
	bool GetPageKey(size_t PageIndex, swInstructionsStepKey& Key) const;

	QVariant GetProperty(swInstructionsPropertyType Type, swModel* Model, swStep Step) const;
	QVariant GetPageProperty(size_t PageIndex, swInstructionsPropertyType Type) const;
	void SetPageProperty(size_t PageIndex, swInstructionsPropertyType Type, const QVariant& Value);
	bool GetStepPageIndex(swModel* Model, swStep Step, size_t& PageIndex) const;
	void EmitPagePropertyChanged(swInstructionsPropertyType Type);
	void SetDefaultProperty(swInstructionsPropertyType Type, const QVariant& Value);
	void SetShowOption(bool& Option, bool Value, SW_PROFILE_KEY Key);
	Project* mProject = nullptr;
	bool mShowGrid = true;
	bool mSnapToGrid = true;
	bool mSnapToAnchors = false;
	bool mGridWithinMargins = true;
	bool mShowMargins = true;
	bool mShowElementMargins = false;
	bool mShowElementPadding = false;
	bool mShowElementAnchors = false;
	swInstructionsBoundaries mElementBoundaries = swInstructionsBoundaries::Off;
	bool mShowLayoutAreas = false;
	bool mShowRulers = true;
	float mGridSize = 72.0f;
	int mGridDivisions = 8;
	QColor mGridPrimaryColor;
	QColor mGridSecondaryColor;
	QColor mMarginColor;
	QColor mElementMarginColor;
	QColor mElementPaddingColor;
	QColor mElementAnchorColor;
	QColor mRulerPointerColor;
	QColor mBoundaryEdgeColor;
	QColor mBoundaryBorderColor;
	QColor mLayoutAreaColor;
	int mNextNoticeId = 1;
	int mNextPointerId = 1;
	int mNextImageId = 1;
	std::vector<QStringList> mUndoLines;
	std::vector<QStringList> mRedoLines;
	std::vector<int> mChangedLines;
	QElapsedTimer mChangeTimer;

	static const float mDisplayDPI;

	// Keys of the default blank pages.
	static const swInstructionsStepKey mCoverBlankKey;
	static const swInstructionsStepKey mBillOfMaterialsBlankKey;
};

// use to draw one page with its lock and freeze
class swInstructionsPageScope
{
public:
	swInstructionsPageScope(swInstructions* Instructions, size_t PageIndex)
		: mInstructions(Instructions), mPreviousState(Instructions->mPageState)
	{
		Instructions->mPageState = Instructions->GetPageState(PageIndex);
	}

	~swInstructionsPageScope()
	{
		mInstructions->mPageState = mPreviousState;
	}

	swInstructionsPageScope(const swInstructionsPageScope&) = delete;
	swInstructionsPageScope& operator=(const swInstructionsPageScope&) = delete;

protected:
	swInstructions* mInstructions;
	swInstructionsPageState mPreviousState;
};
