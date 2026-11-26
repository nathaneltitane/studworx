#include <QCoreApplication>

#include "sw_global.h"
#include "sw_instructions.h"
#include "sw_project.h"
#include "sw_model.h"
#include "sw_object_part.h"
#include "sw_object_camera.h"
#include "sw_data_part.h"
#include "sw_profile.h"
#include "sw_window_main.h"
#include "sw_window_utilities.h"
#include "sw_bricklink.h"

const float swInstructions::mDisplayDPI = 72.0f;
const swInstructionsStepKey swInstructions::mCoverBlankKey = { nullptr, 0 };
const swInstructionsStepKey swInstructions::mBillOfMaterialsBlankKey = { nullptr, 1 };

float swGetPageUnitScale(swInstructionsPageUnits Units)
{
	switch (Units)
	{
	case swInstructionsPageUnits::Pixels:
		return 1.0f;

	case swInstructionsPageUnits::Millimeters:
		return 72.0f / 25.4f;

	case swInstructionsPageUnits::Centimeters:
		return 72.0f / 2.54f;

	case swInstructionsPageUnits::Inches:
		return 72.0f;
	}

	return 1.0f;
}

QString swGetPageUnitSuffix(swInstructionsPageUnits Units)
{
	switch (Units)
	{
	case swInstructionsPageUnits::Pixels:
		return QCoreApplication::translate("swInstructions", "px");

	case swInstructionsPageUnits::Millimeters:
		return QCoreApplication::translate("swInstructions", "mm");

	case swInstructionsPageUnits::Centimeters:
		return QCoreApplication::translate("swInstructions", "cm");

	case swInstructionsPageUnits::Inches:
		return QCoreApplication::translate("swInstructions", "in");
	}

	return QString();
}

const std::vector<swInstructionsPublicationElement>& swGetPublicationElements()
{
	static const std::vector<swInstructionsPublicationElement> Elements = []
	{
		std::vector<swInstructionsPublicationElement> Result;
		const int FirstElement = static_cast<int>(swInstructionsPlacementElement::CoverFrontTitle);
		const int FirstBox = static_cast<int>(swInstructionsBoxElement::CoverFrontTitle);

		for (int Index = 0; Index < 2 * SW_COVER_ELEMENT_COUNT; Index++)
			Result.push_back({ static_cast<swInstructionsPlacementElement>(FirstElement + Index), static_cast<swInstructionsBoxElement>(FirstBox + Index), static_cast<swInstructionsPropertyType>(static_cast<int>(swInstructionsPropertyType::CoverElements) + Index * SW_COVER_FIELD_COUNT), static_cast<swInstructionsCoverElement>(Index % SW_COVER_ELEMENT_COUNT), Index >= SW_COVER_ELEMENT_COUNT });

		return Result;
	}();

	return Elements;
}

QString swGetPublicationKindName(const swInstructionsPublicationElement& Publication)
{
	static const char* const Names[] =
	{
		QT_TRANSLATE_NOOP("swInstructions", "Title"), QT_TRANSLATE_NOOP("swInstructions", "Author"), QT_TRANSLATE_NOOP("swInstructions", "Product ID"), QT_TRANSLATE_NOOP("swInstructions", "Version"),
		QT_TRANSLATE_NOOP("swInstructions", "Publication Date"), QT_TRANSLATE_NOOP("swInstructions", "Publication URL"), QT_TRANSLATE_NOOP("swInstructions", "Contact"), QT_TRANSLATE_NOOP("swInstructions", "Copyright"),
		QT_TRANSLATE_NOOP("swInstructions", "Publication Notice"), QT_TRANSLATE_NOOP("swInstructions", "Comments"), QT_TRANSLATE_NOOP("swInstructions", "Measurements"), QT_TRANSLATE_NOOP("swInstructions", "Weight"),
		QT_TRANSLATE_NOOP("swInstructions", "Pieces")
	};

	return QCoreApplication::translate("swInstructions", Names[static_cast<int>(Publication.Kind)]);
}

QString swGetPublicationName(const swInstructionsPublicationElement& Publication)
{
	return (Publication.Back ? QCoreApplication::translate("swInstructions", "Back Cover") : QCoreApplication::translate("swInstructions", "Front Cover")) + QLatin1Char(' ') + swGetPublicationKindName(Publication);
}

const swInstructionsPublicationElement* swFindPublicationElement(swInstructionsPlacementElement Element)
{
	for (const swInstructionsPublicationElement& Publication : swGetPublicationElements())
		if (Publication.Element == Element)
			return &Publication;

	return nullptr;
}

swInstructionsBoxTypes swGetBoxTypes(swInstructionsBoxElement Element)
{
	// cover elements share one layout
	for (const swInstructionsPublicationElement& Publication : swGetPublicationElements())
		if (Publication.Box == Element)
			return { swGetPublicationType(Publication, swInstructionsPublicationField::Margin), swGetPublicationType(Publication, swInstructionsPublicationField::Padding), swGetPublicationType(Publication, swInstructionsPublicationField::BorderWidth), swGetPublicationType(Publication, swInstructionsPublicationField::CornerRadius), swGetPublicationType(Publication, swInstructionsPublicationField::BorderColor), swGetPublicationType(Publication, swInstructionsPublicationField::BackgroundColor) };

	switch (Element)
	{
	case swInstructionsBoxElement::StepImage:
		return { swInstructionsPropertyType::StepImageMargin, swInstructionsPropertyType::StepImagePadding, swInstructionsPropertyType::StepImageBorderWidth, swInstructionsPropertyType::StepImageCornerRadius, swInstructionsPropertyType::StepImageBorderColor, swInstructionsPropertyType::StepImageBackgroundColor };

	case swInstructionsBoxElement::StepNumber:
		return { swInstructionsPropertyType::StepNumberMargin, swInstructionsPropertyType::StepNumberPadding, swInstructionsPropertyType::StepNumberBorderWidth, swInstructionsPropertyType::StepNumberCornerRadius, swInstructionsPropertyType::StepNumberBorderColor, swInstructionsPropertyType::StepNumberBackgroundColor };

	case swInstructionsBoxElement::PartsList:
		return { swInstructionsPropertyType::PLIMargin, swInstructionsPropertyType::PLIPadding, swInstructionsPropertyType::PLIBorderWidth, swInstructionsPropertyType::PLICornerRadius, swInstructionsPropertyType::PLIBorderColor, swInstructionsPropertyType::PLIBackgroundColor };

	case swInstructionsBoxElement::PageNumber:
		return { swInstructionsPropertyType::PageNumberMargin, swInstructionsPropertyType::PageNumberPadding, swInstructionsPropertyType::PageNumberBorderWidth, swInstructionsPropertyType::PageNumberCornerRadius, swInstructionsPropertyType::PageNumberBorderColor, swInstructionsPropertyType::PageNumberBackgroundColor };

	case swInstructionsBoxElement::CoverImage:
		return { swInstructionsPropertyType::CoverImageMargin, swInstructionsPropertyType::CoverImagePadding, swInstructionsPropertyType::CoverImageBorderWidth, swInstructionsPropertyType::CoverImageCornerRadius, swInstructionsPropertyType::CoverImageBorderColor, swInstructionsPropertyType::CoverImageBackgroundColor };

	case swInstructionsBoxElement::SubmodelPreview:
		return { swInstructionsPropertyType::SubmodelPreviewMargin, swInstructionsPropertyType::SubmodelPreviewPadding, swInstructionsPropertyType::SubmodelPreviewBorderWidth, swInstructionsPropertyType::SubmodelPreviewCornerRadius, swInstructionsPropertyType::SubmodelPreviewBorderColor, swInstructionsPropertyType::SubmodelPreviewBackgroundColor };

	case swInstructionsBoxElement::Image:
		return { swInstructionsPropertyType::ImageMargin, swInstructionsPropertyType::ImagePadding, swInstructionsPropertyType::ImageBorderWidth, swInstructionsPropertyType::ImageCornerRadius, swInstructionsPropertyType::ImageBorderColor, swInstructionsPropertyType::ImageBackgroundColor };

	case swInstructionsBoxElement::PartsListQuantity:
		return { swInstructionsPropertyType::PartsListQuantityMargin, swInstructionsPropertyType::PartsListQuantityPadding, swInstructionsPropertyType::PartsListQuantityBorderWidth, swInstructionsPropertyType::PartsListQuantityCornerRadius, swInstructionsPropertyType::PartsListQuantityBorderColor, swInstructionsPropertyType::PartsListQuantityBackgroundColor };

	case swInstructionsBoxElement::FinalModel:
		return { swInstructionsPropertyType::FinalModelMargin, swInstructionsPropertyType::FinalModelPadding, swInstructionsPropertyType::FinalModelBorderWidth, swInstructionsPropertyType::FinalModelCornerRadius, swInstructionsPropertyType::FinalModelBorderColor, swInstructionsPropertyType::FinalModelBackgroundColor };

	case swInstructionsBoxElement::StepArea:
		return { swInstructionsPropertyType::StepMargin, swInstructionsPropertyType::StepPadding, swInstructionsPropertyType::StepBorderWidth, swInstructionsPropertyType::StepCornerRadius, swInstructionsPropertyType::StepBorderColor, swInstructionsPropertyType::StepBackgroundColor };

	case swInstructionsBoxElement::HeaderBand:
		return { swInstructionsPropertyType::HeaderBandMargin, swInstructionsPropertyType::HeaderBandPadding, swInstructionsPropertyType::HeaderBandBorderWidth, swInstructionsPropertyType::HeaderBandCornerRadius, swInstructionsPropertyType::HeaderBandBorderColor, swInstructionsPropertyType::HeaderBandColor };

	case swInstructionsBoxElement::FooterBand:
		return { swInstructionsPropertyType::FooterBandMargin, swInstructionsPropertyType::FooterBandPadding, swInstructionsPropertyType::FooterBandBorderWidth, swInstructionsPropertyType::FooterBandCornerRadius, swInstructionsPropertyType::FooterBandBorderColor, swInstructionsPropertyType::FooterBandColor };

	case swInstructionsBoxElement::LeftBand:
		return { swInstructionsPropertyType::LeftBandMargin, swInstructionsPropertyType::LeftBandPadding, swInstructionsPropertyType::LeftBandBorderWidth, swInstructionsPropertyType::LeftBandCornerRadius, swInstructionsPropertyType::LeftBandBorderColor, swInstructionsPropertyType::LeftBandColor };

	case swInstructionsBoxElement::RightBand:
		return { swInstructionsPropertyType::RightBandMargin, swInstructionsPropertyType::RightBandPadding, swInstructionsPropertyType::RightBandBorderWidth, swInstructionsPropertyType::RightBandCornerRadius, swInstructionsPropertyType::RightBandBorderColor, swInstructionsPropertyType::RightBandColor };

	case swInstructionsBoxElement::Watermark:
		return { swInstructionsPropertyType::WatermarkMargin, swInstructionsPropertyType::WatermarkPadding, swInstructionsPropertyType::WatermarkBorderWidth, swInstructionsPropertyType::WatermarkCornerRadius, swInstructionsPropertyType::WatermarkBorderColor, swInstructionsPropertyType::WatermarkBackgroundColor };

	case swInstructionsBoxElement::BillOfMaterialsTitle:
		return { swInstructionsPropertyType::BillOfMaterialsTitleMargin, swInstructionsPropertyType::BillOfMaterialsTitlePadding, swInstructionsPropertyType::BillOfMaterialsTitleBorderWidth, swInstructionsPropertyType::BillOfMaterialsTitleCornerRadius, swInstructionsPropertyType::BillOfMaterialsTitleBorderColor, swInstructionsPropertyType::BillOfMaterialsTitleBackgroundColor };

	case swInstructionsBoxElement::BillOfMaterialsList:
		return { swInstructionsPropertyType::BillOfMaterialsListMargin, swInstructionsPropertyType::BillOfMaterialsListPadding, swInstructionsPropertyType::BillOfMaterialsListBorderWidth, swInstructionsPropertyType::BillOfMaterialsListCornerRadius, swInstructionsPropertyType::BillOfMaterialsListBorderColor, swInstructionsPropertyType::BillOfMaterialsListBackgroundColor };

	case swInstructionsBoxElement::BillOfMaterialsQuantity:
		return { swInstructionsPropertyType::BillOfMaterialsQuantityMargin, swInstructionsPropertyType::BillOfMaterialsQuantityPadding, swInstructionsPropertyType::BillOfMaterialsQuantityBorderWidth, swInstructionsPropertyType::BillOfMaterialsQuantityCornerRadius, swInstructionsPropertyType::BillOfMaterialsQuantityBorderColor, swInstructionsPropertyType::BillOfMaterialsQuantityBackgroundColor };

	case swInstructionsBoxElement::BillOfMaterialsPartCount:
		return { swInstructionsPropertyType::BillOfMaterialsPartCountMargin, swInstructionsPropertyType::BillOfMaterialsPartCountPadding, swInstructionsPropertyType::BillOfMaterialsPartCountBorderWidth, swInstructionsPropertyType::BillOfMaterialsPartCountCornerRadius, swInstructionsPropertyType::BillOfMaterialsPartCountBorderColor, swInstructionsPropertyType::BillOfMaterialsPartCountBackgroundColor };

	case swInstructionsBoxElement::Notice:
	case swInstructionsBoxElement::Completion:
		return { swInstructionsPropertyType::CompletionMargin, swInstructionsPropertyType::CompletionPadding, swInstructionsPropertyType::CompletionBorderWidth, swInstructionsPropertyType::CompletionCornerRadius, swInstructionsPropertyType::CompletionBorderColor, swInstructionsPropertyType::CompletionBoxBackgroundColor };

	default:
		break;
	}

	return { swInstructionsPropertyType::NoticeMargin, swInstructionsPropertyType::NoticePadding, swInstructionsPropertyType::NoticeBorderWidth, swInstructionsPropertyType::NoticeCornerRadius, swInstructionsPropertyType::NoticeBorderColor, swInstructionsPropertyType::NoticeBackgroundColor };
}

// the bill of materials starts from the steps
const std::vector<std::pair<swInstructionsPropertyType, swInstructionsPropertyType>>& swGetBillOfMaterialsSources()
{
	static const std::vector<std::pair<swInstructionsPropertyType, swInstructionsPropertyType>> Sources = []()
	{
		std::vector<std::pair<swInstructionsPropertyType, swInstructionsPropertyType>> Pairs = {
			{ swInstructionsPropertyType::ShowBillOfMaterialsTitle, swInstructionsPropertyType::ShowStepNumber },
			{ swInstructionsPropertyType::BillOfMaterialsTitlePlacement, swInstructionsPropertyType::StepNumberPlacement },
			{ swInstructionsPropertyType::BillOfMaterialsFont, swInstructionsPropertyType::PLIFont },
			{ swInstructionsPropertyType::BillOfMaterialsTextColor, swInstructionsPropertyType::PLITextColor },
			{ swInstructionsPropertyType::BillOfMaterialsShowCounts, swInstructionsPropertyType::PLIShowCounts },
			{ swInstructionsPropertyType::BillOfMaterialsSortOrder, swInstructionsPropertyType::PLISortOrder },
			{ swInstructionsPropertyType::BillOfMaterialsLayout, swInstructionsPropertyType::PLILayout },
			{ swInstructionsPropertyType::BillOfMaterialsAlignment, swInstructionsPropertyType::PLIAlignment },
			{ swInstructionsPropertyType::BillOfMaterialsThumbnailSize, swInstructionsPropertyType::PLIThumbnailSize },
			{ swInstructionsPropertyType::BillOfMaterialsCountPlacement, swInstructionsPropertyType::PLICountPlacement }
		};

		const std::pair<swInstructionsBoxElement, swInstructionsBoxElement> Boxes[] = {
			{ swInstructionsBoxElement::BillOfMaterialsTitle, swInstructionsBoxElement::StepNumber },
			{ swInstructionsBoxElement::BillOfMaterialsList, swInstructionsBoxElement::PartsList },
			{ swInstructionsBoxElement::BillOfMaterialsQuantity, swInstructionsBoxElement::PartsListQuantity }
		};

		for (const std::pair<swInstructionsBoxElement, swInstructionsBoxElement>& Box : Boxes)
		{
			const swInstructionsBoxTypes To = swGetBoxTypes(Box.first);
			const swInstructionsBoxTypes From = swGetBoxTypes(Box.second);

			Pairs.insert(Pairs.end(), { { To.Margin, From.Margin }, { To.Padding, From.Padding }, { To.BorderWidth, From.BorderWidth }, { To.CornerRadius, From.CornerRadius }, { To.BorderColor, From.BorderColor }, { To.BackgroundColor, From.BackgroundColor } });
		}

		return Pairs;
	}();

	return Sources;
}

swInstructions::swInstructions(Project* Project)
	: mProject(Project)
{
	// page sizes are in points
	mPageSetup.Width = swGetProfileFloat(SW_PROFILE_INSTRUCTIONS_PAGE_WIDTH);
	mPageSetup.Height = swGetProfileFloat(SW_PROFILE_INSTRUCTIONS_PAGE_HEIGHT);
	mPageSetup.MarginLeft = swGetProfileFloat(SW_PROFILE_INSTRUCTIONS_MARGIN_LEFT);
	mPageSetup.MarginRight = swGetProfileFloat(SW_PROFILE_INSTRUCTIONS_MARGIN_RIGHT);
	mPageSetup.MarginTop = swGetProfileFloat(SW_PROFILE_INSTRUCTIONS_MARGIN_TOP);
	mPageSetup.MarginBottom = swGetProfileFloat(SW_PROFILE_INSTRUCTIONS_MARGIN_BOTTOM);

	if (mPageSetup.Width < 1.0f || mPageSetup.Height < 1.0f || mPageSetup.MarginLeft + mPageSetup.MarginRight >= mPageSetup.Width || mPageSetup.MarginTop + mPageSetup.MarginBottom >= mPageSetup.Height)
	{
		mPageSetup.Width = 11.0f * mDisplayDPI;
		mPageSetup.Height = 8.5f * mDisplayDPI;
		mPageSetup.MarginLeft = 0.5 * mDisplayDPI;
		mPageSetup.MarginRight = 0.5 * mDisplayDPI;
		mPageSetup.MarginTop = 0.5 * mDisplayDPI;
		mPageSetup.MarginBottom = 0.5 * mDisplayDPI;
	}

	mPageSettings = swInstructionsPageSettings();

	mShowGrid = swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_SHOW_GRID) != 0;
	mSnapToGrid = swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_SNAP_TO_GRID) != 0;
	mSnapToAnchors = !mSnapToGrid && swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_SNAP_TO_ANCHORS) != 0;
	mGridSize = qBound(SW_GRID_SIZE_MINIMUM, swGetProfileFloat(SW_PROFILE_INSTRUCTIONS_GRID_SIZE), SW_GRID_SIZE_MAXIMUM);
	mGridDivisions = qBound(1, swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_GRID_DIVISIONS), SW_GRID_DIVISIONS_MAXIMUM);
	mGridWithinMargins = swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_GRID_WITHIN_MARGINS) != 0;
	mShowMargins = swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_SHOW_MARGINS) != 0;
	mGridPrimaryColor = swQColorFromRGBA(swGetProfileUnsignedInteger(SW_PROFILE_INSTRUCTIONS_GRID_PRIMARY_COLOR));
	mGridSecondaryColor = swQColorFromRGBA(swGetProfileUnsignedInteger(SW_PROFILE_INSTRUCTIONS_GRID_SECONDARY_COLOR));
	mMarginColor = swQColorFromRGBA(swGetProfileUnsignedInteger(SW_PROFILE_INSTRUCTIONS_MARGIN_COLOR));
	mShowElementMargins = swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_SHOW_ELEMENT_MARGINS) != 0;
	mShowElementPadding = swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_SHOW_ELEMENT_PADDING) != 0;
	mShowElementAnchors = swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_SHOW_ELEMENT_ANCHORS) != 0;
	mElementBoundaries = static_cast<swInstructionsBoundaries>(qBound(0, swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_SHOW_ELEMENT_BOUNDARIES), static_cast<int>(swInstructionsBoundaries::EveryEdge)));
	mShowLayoutAreas = swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_SHOW_LAYOUT_AREAS) != 0;
	mShowRulers = swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_SHOW_RULERS) != 0;
	mElementMarginColor = swQColorFromRGBA(swGetProfileUnsignedInteger(SW_PROFILE_INSTRUCTIONS_ELEMENT_MARGIN_COLOR));
	mElementPaddingColor = swQColorFromRGBA(swGetProfileUnsignedInteger(SW_PROFILE_INSTRUCTIONS_ELEMENT_PADDING_COLOR));
	mElementAnchorColor = swQColorFromRGBA(swGetProfileUnsignedInteger(SW_PROFILE_INSTRUCTIONS_ELEMENT_ANCHOR_COLOR));
	mRulerPointerColor = swQColorFromRGBA(swGetProfileUnsignedInteger(SW_PROFILE_INSTRUCTIONS_RULER_POINTER_COLOR));
	mBoundaryEdgeColor = swQColorFromRGBA(swGetProfileUnsignedInteger(SW_PROFILE_INSTRUCTIONS_BOUNDARY_EDGE_COLOR));
	mBoundaryBorderColor = swQColorFromRGBA(swGetProfileUnsignedInteger(SW_PROFILE_INSTRUCTIONS_BOUNDARY_BORDER_COLOR));
	mLayoutAreaColor = swQColorFromRGBA(swGetProfileUnsignedInteger(SW_PROFILE_INSTRUCTIONS_LAYOUT_AREA_COLOR));

	LoadDefaultTemplate();
	LoadCommandLines(mProject && mProject->GetMainModel() ? mProject->GetMainModel()->GetInstructionLines() : QStringList(), true);

	CreatePages();

	// keep main model commands as written
	swModel* MainModel = mProject ? mProject->GetMainModel() : nullptr;

	if (MainModel)
		MainModel->SetInstructionLines(GetCommandLines());

	connect(this, &swInstructions::PageLayoutChanged, this, &swInstructions::StoreCommandLines);
	connect(this, &swInstructions::StepSettingsChanged, this, &swInstructions::StoreCommandLines);
	connect(this, &swInstructions::UnitsChanged, this, &swInstructions::StoreCommandLines);
	connect(this, &swInstructions::PageSetupChanged, this, &swInstructions::StoreCommandLines);
}

void swInstructions::SetProject(Project* Project)
{
	mProject = Project;

	// a new project starts a new history

	mUndoLines.clear();
	mRedoLines.clear();
	mChangedLines.clear();

	ReloadCommandLines();

	emit HistoryChanged();
}

void swInstructions::ReloadCommandLines()
{
	// read the commands of the project

	mPages.clear();
	mPageSettingsOverrides.clear();
	mPagePropertyOverrides.clear();
	mElementOffsets.clear();
	mElementSizes.clear();
	mPlacedSizes.clear();
	mLockedPositions.clear();
	mLockedPages.clear();
	mFrozenPages.clear();
	mBlankPages.clear();
	mNotices.clear();
	mPointers.clear();
	mModels.clear();
	mNextNoticeId = 1;
	mNextPointerId = 1;

	LoadDefaultTemplate();
	LoadCommandLines(mProject && mProject->GetMainModel() ? mProject->GetMainModel()->GetInstructionLines() : QStringList(), true);

	CreatePages();

	swModel* MainModel = mProject ? mProject->GetMainModel() : nullptr;

	if (MainModel)
		MainModel->SetInstructionLines(GetCommandLines());

	emit UnitsChanged();
	emit PageSetupChanged();
	emit PageLayoutChanged();
}

swInstructionsProperties swInstructions::CreateDefaultProperties()
{
	swInstructionsProperties Properties;

	Properties[static_cast<int>(swInstructionsPropertyType::ShowStepNumber)].Value = true;
	Properties[static_cast<int>(swInstructionsPropertyType::ShowStepPLI)].Value = true;
	Properties[static_cast<int>(swInstructionsPropertyType::StepNumberFont)].Value = QFont("Arial", 72).toString();
	Properties[static_cast<int>(swInstructionsPropertyType::StepNumberColor)].Value = SW_RGBA(0, 0, 0, 255);
	Properties[static_cast<int>(swInstructionsPropertyType::StepBackgroundColor)].Value = SW_RGBA(255, 255, 255, 0);
	Properties[static_cast<int>(swInstructionsPropertyType::PLIBackgroundColor)].Value = SW_RGBA(255, 255, 255, 0);
	Properties[static_cast<int>(swInstructionsPropertyType::PLIFont)].Value = QFont("Arial", 16).toString();
	Properties[static_cast<int>(swInstructionsPropertyType::PLITextColor)].Value = SW_RGBA(0, 0, 0, 255);
	Properties[static_cast<int>(swInstructionsPropertyType::PLIBorderColor)].Value = SW_RGBA(0, 0, 0, 255);
	Properties[static_cast<int>(swInstructionsPropertyType::PLIBorderWidth)].Value = 0;
	Properties[static_cast<int>(swInstructionsPropertyType::PLIShowCounts)].Value = true;
	Properties[static_cast<int>(swInstructionsPropertyType::PLISortOrder)].Value = static_cast<int>(swPartsListSort::Color);
	Properties[static_cast<int>(swInstructionsPropertyType::ShowStepDividers)].Value = true;
	Properties[static_cast<int>(swInstructionsPropertyType::StepDividerColor)].Value = SW_RGBA(0, 0, 0, 255);
	Properties[static_cast<int>(swInstructionsPropertyType::StepDividerWidth)].Value = 2;
	Properties[static_cast<int>(swInstructionsPropertyType::ShowBillOfMaterials)].Value = true;
	Properties[static_cast<int>(swInstructionsPropertyType::UseDocumentFont)].Value = false;
	Properties[static_cast<int>(swInstructionsPropertyType::UseDocumentFontSize)].Value = false;
	Properties[static_cast<int>(swInstructionsPropertyType::DocumentFont)].Value = QFont("Arial", 16).toString();
	Properties[static_cast<int>(swInstructionsPropertyType::ShowCoverPage)].Value = true;
	Properties[static_cast<int>(swInstructionsPropertyType::ShowCoverImage)].Value = true;
	Properties[static_cast<int>(swInstructionsPropertyType::PageBackgroundColor)].Value = SW_RGBA(255, 255, 255, 255);
	Properties[static_cast<int>(swInstructionsPropertyType::SubmodelBackgroundColor)].Value = SW_RGBA(255, 255, 255, 255);
	Properties[static_cast<int>(swInstructionsPropertyType::BlankPageBackgroundColor)].Value = SW_RGBA(255, 255, 255, 255);

	// clear follows the page or submodel color

	Properties[static_cast<int>(swInstructionsPropertyType::AssemblyBackgroundColor)].Value = SW_RGBA(0, 0, 0, 0);
	Properties[static_cast<int>(swInstructionsPropertyType::CompletionBackgroundColor)].Value = SW_RGBA(0, 0, 0, 0);
	Properties[static_cast<int>(swInstructionsPropertyType::StepImageSize)].Value = 0;
	Properties[static_cast<int>(swInstructionsPropertyType::CoverImageSize)].Value = 0;
	Properties[static_cast<int>(swInstructionsPropertyType::BillOfMaterialsTitleText)].Value = QString();
	Properties[static_cast<int>(swInstructionsPropertyType::BillOfMaterialsTitleFile)].Value = QString();

	// dividers reach the page margins

	Properties[static_cast<int>(swInstructionsPropertyType::StepDividerMargin)].Value = 0;
	Properties[static_cast<int>(swInstructionsPropertyType::StepDividerPadding)].Value = 0;
	Properties[static_cast<int>(swInstructionsPropertyType::StepDividerRounded)].Value = false;
	Properties[static_cast<int>(swInstructionsPropertyType::StepDividerHorizontal)].Value = true;
	Properties[static_cast<int>(swInstructionsPropertyType::StepDividerVertical)].Value = true;
	Properties[static_cast<int>(swInstructionsPropertyType::StepDividerFinalModel)].Value = true;

	// highlight keeps the view outline width

	Properties[static_cast<int>(swInstructionsPropertyType::HighlightWidth)].Value = 4;

	// final model area has no frame

	Properties[static_cast<int>(swInstructionsPropertyType::FinalModelBorderWidth)].Value = 0;
	Properties[static_cast<int>(swInstructionsPropertyType::FinalModelBorderColor)].Value = SW_RGBA(0, 0, 0, 255);
	Properties[static_cast<int>(swInstructionsPropertyType::FinalModelMargin)].Value = 0;

	// materials page names each part

	Properties[static_cast<int>(swInstructionsPropertyType::BillOfMaterialsShowPartIds)].Value = true;
	Properties[static_cast<int>(swInstructionsPropertyType::StepNumberPlacement)].Value = SW_PLACEMENT_GRID_SIZE + 1;
	Properties[static_cast<int>(swInstructionsPropertyType::StepNumberAnchor)].Value = static_cast<int>(swInstructionsPlacementAnchor::Area);
	Properties[static_cast<int>(swInstructionsPropertyType::PLIPlacement)].Value = 3 * SW_PLACEMENT_GRID_SIZE - 1;
	Properties[static_cast<int>(swInstructionsPropertyType::PLIAnchor)].Value = static_cast<int>(swInstructionsPlacementAnchor::StepNumber);
	Properties[static_cast<int>(swInstructionsPropertyType::PLIReserveArea)].Value = true;
	Properties[static_cast<int>(swInstructionsPropertyType::BillOfMaterialsPlacement)].Value = SW_PLACEMENT_GRID_SIZE + 1;
	Properties[static_cast<int>(swInstructionsPropertyType::PLIThumbnailSize)].Value = 300;
	Properties[static_cast<int>(swInstructionsPropertyType::PLIThumbnailBackgroundColor)].Value = SW_RGBA(255, 255, 255, 0);
	Properties[static_cast<int>(swInstructionsPropertyType::CoverImagePlacement)].Value = SW_PLACEMENT_COUNT / 2;
	Properties[static_cast<int>(swInstructionsPropertyType::StepImagePlacement)].Value = SW_PLACEMENT_COUNT / 2;
	Properties[static_cast<int>(swInstructionsPropertyType::CoverBackgroundColor)].Value = SW_RGBA(255, 255, 255, 255);
	Properties[static_cast<int>(swInstructionsPropertyType::BillOfMaterialsBackgroundColor)].Value = SW_RGBA(255, 255, 255, 255);
	Properties[static_cast<int>(swInstructionsPropertyType::ShowCoverBlankPage)].Value = true;
	Properties[static_cast<int>(swInstructionsPropertyType::ShowBillOfMaterialsBlankPage)].Value = true;
	Properties[static_cast<int>(swInstructionsPropertyType::UseDocumentUnits)].Value = true;
	Properties[static_cast<int>(swInstructionsPropertyType::DocumentUnits)].Value = swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_PAGE_UNITS);
	Properties[static_cast<int>(swInstructionsPropertyType::ShowPageNumber)].Value = true;
	Properties[static_cast<int>(swInstructionsPropertyType::PageNumberFormat)].Value = static_cast<int>(swInstructionsPageNumberFormat::Number);
	Properties[static_cast<int>(swInstructionsPropertyType::PageNumberFont)].Value = QFont("Arial", 16).toString();
	Properties[static_cast<int>(swInstructionsPropertyType::PageNumberColor)].Value = SW_RGBA(0, 0, 0, 255);
	Properties[static_cast<int>(swInstructionsPropertyType::PageNumberPlacement)].Value = SW_PLACEMENT_COUNT - SW_PLACEMENT_GRID_SIZE - 2;
	Properties[static_cast<int>(swInstructionsPropertyType::ShowPageNumberAllPages)].Value = false;

	// part counts are off and read like the titles
	Properties[static_cast<int>(swInstructionsPropertyType::ShowBillOfMaterialsPartCount)].Value = false;
	Properties[static_cast<int>(swInstructionsPropertyType::ShowPagePartCount)].Value = false;
	Properties[static_cast<int>(swInstructionsPropertyType::PartCountText)].Value = QString();
	Properties[static_cast<int>(swInstructionsPropertyType::PartCountFile)].Value = QString();
	Properties[static_cast<int>(swInstructionsPropertyType::PartCountFont)].Value = QFont("Arial", 24).toString();
	Properties[static_cast<int>(swInstructionsPropertyType::PartCountColor)].Value = SW_RGBA(0, 0, 0, 255);
	Properties[static_cast<int>(swInstructionsPropertyType::PagePartCountFont)].Value = QFont("Arial", 16).toString();
	Properties[static_cast<int>(swInstructionsPropertyType::PagePartCountColor)].Value = SW_RGBA(0, 0, 0, 255);
	Properties[static_cast<int>(swInstructionsPropertyType::BillOfMaterialsPartCountPlacement)].Value = SW_PLACEMENT_GRID_SIZE + 3;
	Properties[static_cast<int>(swInstructionsPropertyType::PagePartCountPlacement)].Value = SW_PLACEMENT_COUNT - SW_PLACEMENT_GRID_SIZE - 4;
	Properties[static_cast<int>(swInstructionsPropertyType::ShowSubmodelPreview)].Value = false;
	Properties[static_cast<int>(swInstructionsPropertyType::SubmodelPreviewSize)].Value = 0;
	Properties[static_cast<int>(swInstructionsPropertyType::SubmodelPreviewPlacement)].Value = SW_PLACEMENT_GRID_SIZE * 2 + 1;
	Properties[static_cast<int>(swInstructionsPropertyType::SubmodelPreviewPartCountPlacement)].Value = SW_PLACEMENT_COUNT - 3;

	// bands are off and clear
	for (int Band = 0; Band < 4; Band++)
	{
		Properties[static_cast<int>(swInstructionsPropertyType::ShowHeaderBand) + Band].Value = false;
		Properties[static_cast<int>(swInstructionsPropertyType::HeaderBandSize) + Band].Value = 72;
		Properties[static_cast<int>(swInstructionsPropertyType::HeaderBandColor) + Band].Value = SW_RGBA(0, 0, 0, 0);
		Properties[static_cast<int>(swInstructionsPropertyType::HeaderBandReserve) + Band].Value = true;
	}

	Properties[static_cast<int>(swInstructionsPropertyType::ShowWatermark)].Value = true;
	Properties[static_cast<int>(swInstructionsPropertyType::WatermarkOpacity)].Value = 25;
	Properties[static_cast<int>(swInstructionsPropertyType::WatermarkFont)].Value = QFont("Arial", 16).toString();
	Properties[static_cast<int>(swInstructionsPropertyType::WatermarkTextColor)].Value = SW_RGBA(0, 0, 0, 64);
	Properties[static_cast<int>(swInstructionsPropertyType::WatermarkRotation)].Value = 0;

	// texts start left and middle
	for (int Type = static_cast<int>(swInstructionsPropertyType::StepNumberAlignment); Type <= static_cast<int>(swInstructionsPropertyType::NoticeAlignment); Type += 2)
	{
		Properties[Type].Value = 0;
		Properties[Type + 1].Value = 1;
	}

	// cover texts start centered
	for (const swInstructionsPublicationElement& Publication : swGetPublicationElements())
	{
		Properties[static_cast<int>(swGetPublicationType(Publication, swInstructionsPublicationField::Alignment))].Value = 1;
		Properties[static_cast<int>(swGetPublicationType(Publication, swInstructionsPublicationField::VerticalAlignment))].Value = 1;
	}

	// boxes keep the former spacing
	for (int Element = 0; Element < static_cast<int>(swInstructionsBoxElement::Count); Element++)
	{
		const swInstructionsBoxTypes BoxTypes = swGetBoxTypes(static_cast<swInstructionsBoxElement>(Element));

		const bool PageBox = Element >= static_cast<int>(swInstructionsBoxElement::FinalModel) && Element < static_cast<int>(swInstructionsBoxElement::CoverFrontTitle);
		const swInstructionsBoxElement BoxElement = static_cast<swInstructionsBoxElement>(Element);

		Properties[static_cast<int>(BoxTypes.Margin)].Value = PageBox ? 0 : 6;
		Properties[static_cast<int>(BoxTypes.Padding)].Value = PageBox ? 0 : 6;
		Properties[static_cast<int>(BoxTypes.CornerRadius)].Value = 0;

		if (BoxElement == swInstructionsBoxElement::PartsList)
			continue;

		Properties[static_cast<int>(BoxTypes.BorderWidth)].Value = 0;
		Properties[static_cast<int>(BoxTypes.BorderColor)].Value = SW_RGBA(0, 0, 0, 255);

		// colors set above stay
		if (BoxElement != swInstructionsBoxElement::StepArea && (BoxElement < swInstructionsBoxElement::HeaderBand || BoxElement > swInstructionsBoxElement::RightBand))
			Properties[static_cast<int>(BoxTypes.BackgroundColor)].Value = SW_RGBA(255, 255, 255, 0);
	}

	// images keep their place unboxed
	Properties[static_cast<int>(swInstructionsPropertyType::ImageMargin)].Value = 0;
	Properties[static_cast<int>(swInstructionsPropertyType::ImagePadding)].Value = 0;

	// quantities keep their bare text
	Properties[static_cast<int>(swInstructionsPropertyType::PartsListQuantityMargin)].Value = 0;
	Properties[static_cast<int>(swInstructionsPropertyType::PartsListQuantityPadding)].Value = 0;

	// submodel previews sit on light grey
	Properties[static_cast<int>(swInstructionsPropertyType::SubmodelPreviewBackgroundColor)].Value = SW_RGBA(221, 221, 221, 255);

	// frame notices
	Properties[static_cast<int>(swInstructionsPropertyType::NoticeFont)].Value = QFont("Arial", 16).toString();
	Properties[static_cast<int>(swInstructionsPropertyType::BillOfMaterialsTitleFont)].Value = QFont("Arial", 24).toString();
	Properties[static_cast<int>(swInstructionsPropertyType::BillOfMaterialsTitleColor)].Value = SW_RGBA(0, 0, 0, 255);
	Properties[static_cast<int>(swInstructionsPropertyType::NoticeTextColor)].Value = SW_RGBA(0, 0, 0, 255);
	Properties[static_cast<int>(swInstructionsPropertyType::NoticeBorderWidth)].Value = 2;
	Properties[static_cast<int>(swInstructionsPropertyType::NoticeBackgroundColor)].Value = SW_RGBA(255, 255, 255, 0);
	Properties[static_cast<int>(swInstructionsPropertyType::PointerStyle)].Value = static_cast<int>(swInstructionsPointerStyle::Line);
	Properties[static_cast<int>(swInstructionsPropertyType::PointerTip)].Value = static_cast<int>(swInstructionsPointerTip::Arrow);
	Properties[static_cast<int>(swInstructionsPropertyType::PointerColor)].Value = SW_RGBA(0, 0, 0, 255);

	// pointers follow their element border
	Properties[static_cast<int>(swInstructionsPropertyType::PointerWidth)].Value = 0;
	Properties[static_cast<int>(swInstructionsPropertyType::PointerTipSize)].Value = 0;
	Properties[static_cast<int>(swInstructionsPropertyType::PointerBendRadius)].Value = 0;
	Properties[static_cast<int>(swInstructionsPropertyType::ShowFinalModelPage)].Value = true;

	// parts stand in rows on their bottom
	Properties[static_cast<int>(swInstructionsPropertyType::PLILayout)].Value = static_cast<int>(swPartsListLayout::Rows);
	Properties[static_cast<int>(swInstructionsPropertyType::PLIAlignment)].Value = static_cast<int>(swPartsListAlignment::End);
	Properties[static_cast<int>(swInstructionsPropertyType::PLICountPlacement)].Value = SW_PLACEMENT_COUNT - SW_PLACEMENT_GRID_SIZE + 1;

	// step models render in perspective
	Properties[static_cast<int>(swInstructionsPropertyType::ModelProjection)].Value = static_cast<int>(swCameraProjection::Perspective);

	// models follow the model view

	Properties[static_cast<int>(swInstructionsPropertyType::LockHomeView)].Value = false;

	// outline new parts in yellow
	Properties[static_cast<int>(swInstructionsPropertyType::FadePreviousSteps)].Value = true;
	Properties[static_cast<int>(swInstructionsPropertyType::FadeColor)].Value = SW_RGBA(128, 128, 128, 217);
	Properties[static_cast<int>(swInstructionsPropertyType::HighlightNewParts)].Value = true;
	Properties[static_cast<int>(swInstructionsPropertyType::HighlightColor)].Value = SW_RGBA(255, 242, 0, 230);

	// the back cover starts blank
	Properties[static_cast<int>(swInstructionsPropertyType::ShowBackCover)].Value = true;

	// cover texts start hidden except the front title and author
	for (const swInstructionsPublicationElement& Publication : swGetPublicationElements())
	{
		const bool Heading = Publication.Kind == swInstructionsCoverElement::Title || Publication.Kind == swInstructionsCoverElement::Author;

		Properties[static_cast<int>(Publication.Show)].Value = Heading && !Publication.Back;
		Properties[static_cast<int>(swGetPublicationType(Publication, swInstructionsPublicationField::Text))].Value = QString();
		Properties[static_cast<int>(swGetPublicationType(Publication, swInstructionsPublicationField::Font))].Value = QFont("Arial", 24).toString();
		Properties[static_cast<int>(swGetPublicationType(Publication, swInstructionsPublicationField::Color))].Value = SW_RGBA(0, 0, 0, 255);

		// each text starts on its own cell around the image
		static constexpr int Placements[] = { 2, 7, 20, 0, 4, 21, 23, 22, 24, 18, 10, 14, 16 };
		static_assert(std::size(Placements) == SW_COVER_ELEMENT_COUNT, "Missing cover placement");

		Properties[static_cast<int>(swGetPublicationType(Publication, swInstructionsPublicationField::Placement))].Value = Placements[static_cast<int>(Publication.Kind)];
	}

	// the bill of materials copies the steps
	for (const std::pair<swInstructionsPropertyType, swInstructionsPropertyType>& Source : swGetBillOfMaterialsSources())
		Properties[static_cast<int>(Source.first)].Value = Properties[static_cast<int>(Source.second)].Value;

	static_assert(static_cast<int>(swInstructionsPropertyType::Count) == 592, "Missing default property");

	return Properties;
}

void swInstructions::SetDefaultProperties()
{
	mStepProperties = CreateDefaultProperties();
}

// use to start from the default template, the model settings win
void swInstructions::LoadDefaultTemplate()
{
	SetDefaultProperties();
	mDefaultProperties = mStepProperties;
	mPageSettings = swInstructionsPageSettings();
	mImages.clear();
	mNextImageId = 1;

	LoadCommandLines(ReadTemplateLines(swGetProfileString(SW_PROFILE_DEFAULT_TEMPLATE)));

	for (swInstructionsImage& Image : mImages)
		Image.Template = true;

	mDefaultProperties = mStepProperties;
	mDefaultPageSettings = mPageSettings;
}

void swInstructions::ResetProperties()
{
	// restore defaults and moved elements
	mStepProperties = mDefaultProperties;
	mPagePropertyOverrides.clear();
	mElementOffsets.clear();
	mElementSizes.clear();
	mPlacedSizes.clear();
	mLockedPositions.clear();
	CreatePages();

	emit PageLayoutChanged();
}

void swInstructions::SetDefaultPageSettings(const swInstructionsPageSettings& PageSettings)
{
	mPageSettings = PageSettings;
	mPageSettingsOverrides.clear();

	CreatePages();

	emit PageLayoutChanged();
}

bool swInstructions::GetPageKey(size_t PageIndex, swInstructionsStepKey& Key) const
{
	// find pages by their first step
	if (PageIndex >= mPages.size())
	{
		swModel* Model = mProject ? mProject->GetMainModel() : nullptr;

		if (!Model)
			return false;

		Key = { Model, Model->GetLastStep() + 1 };
		return true;
	}

	const swInstructionsPage& Page = mPages[PageIndex];

	switch (Page.Type)
	{
	case swInstructionsPageType::Steps:
		Key = { Page.Steps.front().Model, Page.Steps.front().Step };
		return true;

	case swInstructionsPageType::Blank:
		Key = Page.BlankKey;
		return true;

	case swInstructionsPageType::BillOfMaterials:
		Key = { Page.Model, Page.Model->GetLastStep() + 1 };
		return true;

	case swInstructionsPageType::Cover:
	case swInstructionsPageType::BackCover:
		break;
	}

	return false;
}

swInstructionsPageSettings swInstructions::GetStepPageSettings(const swInstructionsStepKey& Key) const
{
	std::map<swInstructionsStepKey, swInstructionsPageSettings>::const_iterator SettingsIt = mPageSettingsOverrides.find(Key);
	return SettingsIt != mPageSettingsOverrides.end() ? SettingsIt->second : mPageSettings;
}

swInstructionsPageSettings swInstructions::GetPageSettings(size_t PageIndex) const
{
	if (PageIndex < mPages.size() && mPages[PageIndex].Type == swInstructionsPageType::Steps)
		return mPages[PageIndex].Settings;

	return GetStepPageSettings({ nullptr, 0 });
}

void swInstructions::SetPageSettings(size_t PageIndex, const swInstructionsPageSettings& PageSettings)
{
	// pages hold their own step count
	if (PageIndex >= mPages.size() || mPages[PageIndex].Type != swInstructionsPageType::Steps)
		return;

	if (mPages[PageIndex].Settings == PageSettings)
		return;

	swInstructionsStepKey Key;
	GetPageKey(PageIndex, Key);

	mPageSettingsOverrides[Key] = PageSettings;

	CreatePages();

	emit PageLayoutChanged();
}

void swInstructions::SetPageDividerPreset(size_t PageIndex, int Rows, int Columns)
{
	// split pages show dividers
	swInstructionsProperty& Dividers = mStepProperties[static_cast<int>(swInstructionsPropertyType::ShowStepDividers)];
	const bool ShowDividers = Rows * Columns > 1 && !Dividers.Value.toBool();

	if (ShowDividers)
		Dividers.Value = true;

	const swInstructionsPageSettings PageSettings = GetPageSettings(PageIndex);

	if (PageSettings.Rows == Rows && PageSettings.Columns == Columns)
	{
		if (ShowDividers)
			emit PageLayoutChanged();

		return;
	}

	swInstructionsPageSettings NewSettings = PageSettings;
	NewSettings.Rows = Rows;
	NewSettings.Columns = Columns;

	SetPageSettings(PageIndex, NewSettings);
}

bool swInstructions::CanInsertBlankPage(size_t PageIndex) const
{
	// default blank pages have no step
	swInstructionsStepKey Key;
	return GetPageKey(PageIndex, Key) && Key.first;
}

void swInstructions::InsertBlankPage(size_t PageIndex)
{
	// insert the blank page before this index
	swInstructionsStepKey Key;

	if (!GetPageKey(PageIndex, Key) || !Key.first)
		return;

	mBlankPages.insert(Key);

	CreatePages();

	emit PageLayoutChanged();
}

void swInstructions::RemoveBlankPage(size_t PageIndex)
{
	// locked pages stay
	if (PageIndex >= mPages.size() || mPages[PageIndex].Type != swInstructionsPageType::Blank || IsPageLocked(PageIndex))
		return;

	// switch off default blank pages
	const swInstructionsStepKey& BlankKey = mPages[PageIndex].BlankKey;

	if (BlankKey == mCoverBlankKey)
	{
		SetDefaultBool(swInstructionsPropertyType::ShowCoverBlankPage, false);
		return;
	}

	if (BlankKey == mBillOfMaterialsBlankKey)
	{
		SetDefaultBool(swInstructionsPropertyType::ShowBillOfMaterialsBlankPage, false);
		return;
	}

	std::multiset<swInstructionsStepKey>::iterator BlankIt = mBlankPages.find(mPages[PageIndex].BlankKey);

	if (BlankIt != mBlankPages.end())
		mBlankPages.erase(BlankIt);

	CreatePages();

	emit PageLayoutChanged();
}

void swInstructions::Update()
{
	CreatePages();

	emit PageLayoutChanged();
}

void swInstructions::SetPageSetup(const swInstructionsPageSetup& PageSetup)
{
	mPageSetup = PageSetup;

	swSetProfileFloat(SW_PROFILE_INSTRUCTIONS_PAGE_WIDTH, mPageSetup.Width);
	swSetProfileFloat(SW_PROFILE_INSTRUCTIONS_PAGE_HEIGHT, mPageSetup.Height);
	swSetProfileFloat(SW_PROFILE_INSTRUCTIONS_MARGIN_LEFT, mPageSetup.MarginLeft);
	swSetProfileFloat(SW_PROFILE_INSTRUCTIONS_MARGIN_RIGHT, mPageSetup.MarginRight);
	swSetProfileFloat(SW_PROFILE_INSTRUCTIONS_MARGIN_TOP, mPageSetup.MarginTop);
	swSetProfileFloat(SW_PROFILE_INSTRUCTIONS_MARGIN_BOTTOM, mPageSetup.MarginBottom);

	CreatePages();

	emit PageSetupChanged();
}

bool swInstructions::GetBoolProperty(swInstructionsPropertyType Type, swModel* Model, swStep Step) const
{
	QVariant Value = GetProperty(Type, Model, Step);
	return Value.toBool();
}

int swInstructions::GetIntProperty(swInstructionsPropertyType Type, swModel* Model, swStep Step) const
{
	QVariant Value = GetProperty(Type, Model, Step);
	return Value.toInt();
}

QColor swInstructions::GetColorProperty(swInstructionsPropertyType Type, swModel* Model, swStep Step) const
{
	QVariant Value = GetProperty(Type, Model, Step);
	return swQColorFromRGBA(Value.toUInt());
}

QFont swInstructions::GetFontProperty(swInstructionsPropertyType Type, swModel* Model, swStep Step, bool ApplyDocumentFont) const
{
	QFont Font;
	Font.fromString(GetProperty(Type, Model, Step).toString());

	if (!ApplyDocumentFont || Type == swInstructionsPropertyType::DocumentFont)
		return Font;

	// the document font replaces the typeface, its size the size

	const bool UseFont = GetBoolProperty(swInstructionsPropertyType::UseDocumentFont, Model, Step);
	const bool UseSize = GetBoolProperty(swInstructionsPropertyType::UseDocumentFontSize, Model, Step);

	if (!UseFont && !UseSize)
		return Font;

	QFont DocumentFont;
	DocumentFont.fromString(GetProperty(swInstructionsPropertyType::DocumentFont, Model, Step).toString());

	QFont Result = UseFont ? DocumentFont : Font;
	Result.setPointSizeF(UseSize ? DocumentFont.pointSizeF() : Font.pointSizeF());

	return Result;
}

bool swInstructions::IsPageProperty(swInstructionsPropertyType Type)
{
	return Type == swInstructionsPropertyType::PLIThumbnailSize || Type == swInstructionsPropertyType::PageBackgroundColor || Type == swInstructionsPropertyType::StepImageSize || Type == swInstructionsPropertyType::ShowWatermark || (Type >= swInstructionsPropertyType::ShowHeaderBand && Type <= swInstructionsPropertyType::RightBandReserve) || (Type >= swInstructionsPropertyType::FinalModelBackgroundColor && Type <= swInstructionsPropertyType::WatermarkBackgroundColor);
}

bool swInstructions::GetStepPageIndex(swModel* Model, swStep Step, size_t& PageIndex) const
{
	// step 0 is the whole model
	for (size_t Index = 0; Index < mPages.size(); Index++)
	{
		const swInstructionsPage& Page = mPages[Index];

		if (Step == 0 && Page.Type == swInstructionsPageType::BillOfMaterials && Page.Model == Model)
		{
			PageIndex = Index;
			return true;
		}

		for (const swInstructionsStep& PageStep : Page.Steps)
		{
			if (PageStep.Model == Model && PageStep.Step == Step)
			{
				PageIndex = Index;
				return true;
			}
		}
	}

	return false;
}

bool swInstructions::HasPageProperty(size_t PageIndex, swInstructionsPropertyType Type) const
{
	swInstructionsStepKey Key;

	if (!GetPageKey(PageIndex, Key) || PageIndex >= mPages.size())
		return false;

	std::map<swInstructionsStepKey, swInstructionsProperties>::const_iterator PropertiesIt = mPagePropertyOverrides.find(Key);

	return PropertiesIt != mPagePropertyOverrides.end() && PropertiesIt->second[static_cast<int>(Type)].Mode != swInstructionsPropertyMode::NotSet;
}

QVariant swInstructions::GetPageProperty(size_t PageIndex, swInstructionsPropertyType Type) const
{
	const swInstructionsPageState PageState = GetPageState(PageIndex);

	if (PageState.FrozenProperties)
		return (*PageState.FrozenProperties)[static_cast<int>(Type)].Value;

	swInstructionsStepKey Key;

	if (HasPageProperty(PageIndex, Type) && GetPageKey(PageIndex, Key))
		return mPagePropertyOverrides.at(Key)[static_cast<int>(Type)].Value;

	// blank pages have their own background
	if (Type == swInstructionsPropertyType::PageBackgroundColor && PageIndex < mPages.size() && mPages[PageIndex].Type == swInstructionsPageType::Blank)
		return mStepProperties[static_cast<int>(swInstructionsPropertyType::BlankPageBackgroundColor)].Value;

	// a completion page alone has its own background

	if (Type == swInstructionsPropertyType::PageBackgroundColor && swQColorFromRGBA(mStepProperties[static_cast<int>(swInstructionsPropertyType::CompletionBackgroundColor)].Value.toUInt()).alpha() != 0 && IsCompletionPage(PageIndex) && std::all_of(mPages[PageIndex].Steps.begin(), mPages[PageIndex].Steps.end(), [](const swInstructionsStep& Step) { return Step.FinalModel; }))
		return mStepProperties[static_cast<int>(swInstructionsPropertyType::CompletionBackgroundColor)].Value;

	// assembly pages have their own background

	if (Type == swInstructionsPropertyType::PageBackgroundColor && swQColorFromRGBA(mStepProperties[static_cast<int>(swInstructionsPropertyType::AssemblyBackgroundColor)].Value.toUInt()).alpha() != 0)
	{
		const swModel* TitleModel = nullptr;

		if (GetPageTitleModel(PageIndex, TitleModel))
			return mStepProperties[static_cast<int>(swInstructionsPropertyType::AssemblyBackgroundColor)].Value;
	}

	// submodel pages have their own background
	if (Type == swInstructionsPropertyType::PageBackgroundColor && mProject && GetPageKey(PageIndex, Key) && Key.first && Key.first != mProject->GetMainModel())
		return mStepProperties[static_cast<int>(swInstructionsPropertyType::SubmodelBackgroundColor)].Value;

	return mStepProperties[static_cast<int>(Type)].Value;
}

int swInstructions::GetPageIntProperty(size_t PageIndex, swInstructionsPropertyType Type) const
{
	return GetPageProperty(PageIndex, Type).toInt();
}

bool swInstructions::GetPageBoolProperty(size_t PageIndex, swInstructionsPropertyType Type) const
{
	return GetPageProperty(PageIndex, Type).toBool();
}

QColor swInstructions::GetPageColorProperty(size_t PageIndex, swInstructionsPropertyType Type) const
{
	return swQColorFromRGBA(GetPageProperty(PageIndex, Type).toUInt());
}

QColor swInstructions::GetPageBackgroundColor(size_t PageIndex) const
{
	if (PageIndex < mPages.size())
	{
		// cover and materials follow the freeze too
		const swInstructionsPageState PageState = GetPageState(PageIndex);
		auto GetColor = [this, &PageState](swInstructionsPropertyType Type)
		{
			return PageState.FrozenProperties ? swQColorFromRGBA((*PageState.FrozenProperties)[static_cast<int>(Type)].Value.toUInt()) : GetColorProperty(Type, nullptr, 0);
		};

		switch (mPages[PageIndex].Type)
		{
		case swInstructionsPageType::Cover:
		case swInstructionsPageType::BackCover:
			return GetColor(swInstructionsPropertyType::CoverBackgroundColor);

		case swInstructionsPageType::BillOfMaterials:
			return GetColor(swInstructionsPropertyType::BillOfMaterialsBackgroundColor);

		case swInstructionsPageType::Steps:
		case swInstructionsPageType::Blank:
			break;
		}
	}

	return GetPageColorProperty(PageIndex, swInstructionsPropertyType::PageBackgroundColor);
}

void swInstructions::EmitPagePropertyChanged(swInstructionsPropertyType Type)
{
	// draw the background, bands and watermark with the page
	if (Type == swInstructionsPropertyType::PageBackgroundColor || Type == swInstructionsPropertyType::ShowWatermark || (Type >= swInstructionsPropertyType::ShowHeaderBand && Type <= swInstructionsPropertyType::RightBandReserve) || Type >= swInstructionsPropertyType::FinalModelBackgroundColor)
		emit PageLayoutChanged();
	else
		emit StepSettingsChanged(nullptr, 0);
}

void swInstructions::SetPageScope(size_t PageIndex, swInstructionsPropertyType Type, bool PageOnly)
{
	swInstructionsStepKey Key;

	if (!IsPageProperty(Type) || PageIndex >= mPages.size() || !GetPageKey(PageIndex, Key) || HasPageProperty(PageIndex, Type) == PageOnly)
		return;

	// pages fall back to the all pages value
	const QVariant Value = GetPageProperty(PageIndex, Type);
	swInstructionsProperty& Property = mPagePropertyOverrides[Key][static_cast<int>(Type)];

	if (PageOnly)
	{
		Property.Mode = swInstructionsPropertyMode::Default;
		Property.Value = Value;
	}
	else
		Property = swInstructionsProperty();

	EmitPagePropertyChanged(Type);
}

// use to give every page the value shown on one page
void swInstructions::ApplyPageToAllPages(size_t PageIndex, swInstructionsPropertyType Type)
{
	swInstructionsStepKey Key;

	if (!IsPageProperty(Type) || !GetPageKey(PageIndex, Key))
		return;

	// the color the page shows, submodel pages included

	const QVariant Value = GetPageProperty(PageIndex, Type);

	for (auto& [PageKey, Properties] : mPagePropertyOverrides)
		Properties[static_cast<int>(Type)] = swInstructionsProperty();

	// every page includes submodel and blank pages
	if (Type == swInstructionsPropertyType::PageBackgroundColor)
	{
		SetDefaultProperty(swInstructionsPropertyType::SubmodelBackgroundColor, Value);
		SetDefaultProperty(swInstructionsPropertyType::BlankPageBackgroundColor, Value);
		SetDefaultProperty(swInstructionsPropertyType::AssemblyBackgroundColor, Value);
		SetDefaultProperty(swInstructionsPropertyType::CompletionBackgroundColor, Value);
	}

	SetDefaultProperty(Type, Value);
	EmitPagePropertyChanged(Type);
}

void swInstructions::SetPageProperty(size_t PageIndex, swInstructionsPropertyType Type, const QVariant& Value)
{
	swInstructionsStepKey Key;

	if (!HasPageProperty(PageIndex, Type) || !GetPageKey(PageIndex, Key))
		return;

	swInstructionsProperty& Property = mPagePropertyOverrides[Key][static_cast<int>(Type)];

	if (Property.Value == Value)
		return;

	Property.Value = Value;

	EmitPagePropertyChanged(Type);
}

void swInstructions::SetPageInt(size_t PageIndex, swInstructionsPropertyType Type, int Value)
{
	SetPageProperty(PageIndex, Type, Value);
}

void swInstructions::SetPageColor(size_t PageIndex, swInstructionsPropertyType Type, const QColor& Color)
{
	SetPageProperty(PageIndex, Type, swRGBAFromQColor(Color));
}

QPointF swInstructions::GetElementOffset(const swInstructionsElementKey& Key) const
{
	const std::map<swInstructionsElementKey, QPointF>::const_iterator Offset = mElementOffsets.find(Key);

	if (Offset == mElementOffsets.end())
		return QPointF();

	return Offset->second;
}

QSizeF swInstructions::GetPlacedSize(const swInstructionsElementKey& Key, const QSizeF& Size) const
{
	// the first draw keeps older files in place

	const std::map<swInstructionsElementKey, QSizeF>::const_iterator PlacedSize = mPlacedSizes.find(Key);

	if (PlacedSize != mPlacedSizes.end())
		return PlacedSize->second;

	if (Size.isValid())
		mPlacedSizes[Key] = Size;

	return Size;
}

bool swInstructions::HasElementOffset(const swInstructionsElementKey& Key) const
{
	return mElementOffsets.find(Key) != mElementOffsets.end();
}

void swInstructions::SetElementOffsets(const std::map<swInstructionsElementKey, QPointF>& Offsets, const std::map<swInstructionsElementKey, QSizeF>& Sizes)
{
	if (Offsets.empty())
		return;

	// drop the offset when back on placement
	for (const std::pair<const swInstructionsElementKey, QPointF>& Offset : Offsets)
	{
		if (Offset.second.isNull())
			mElementOffsets.erase(Offset.first);
		else
			mElementOffsets[Offset.first] = Offset.second;
	}

	// moved elements remember their size

	for (const std::pair<const swInstructionsElementKey, QSizeF>& Size : Sizes)
	{
		if (Size.second.isValid())
			mPlacedSizes[Size.first] = Size.second;
	}

	emit PageLayoutChanged();
}

QSizeF swInstructions::GetElementSize(const swInstructionsElementKey& Key) const
{
	const std::map<swInstructionsElementKey, QSizeF>::const_iterator Size = mElementSizes.find(Key);

	return Size == mElementSizes.end() ? QSizeF() : Size->second;
}

void swInstructions::SetElementSize(const swInstructionsElementKey& Key, const QSizeF& Size)
{
	// an empty size fits the content again
	if (Size.isEmpty())
		mElementSizes.erase(Key);
	else
		mElementSizes[Key] = Size;

	emit PageLayoutChanged();
}

void swInstructions::ResetElementOffset(const swInstructionsElementKey& Key)
{
	if (mElementOffsets.erase(Key))
		emit PageLayoutChanged();
}

void swInstructions::SetElementPlacement(swInstructionsPropertyType Type, int Placement, const swInstructionsElementKey& Key)
{
	// a placed element drops its moved position
	const bool OffsetRemoved = mElementOffsets.erase(Key) > 0;

	if (mStepProperties[static_cast<int>(Type)].Value == QVariant(Placement))
	{
		if (OffsetRemoved)
			emit PageLayoutChanged();

		return;
	}

	SetDefaultProperty(Type, Placement);
}

swInstructionsBoxStyle swInstructions::GetBoxStyle(swInstructionsBoxElement Element, swModel* Model, swStep Step) const
{
	const swInstructionsBoxTypes BoxTypes = swGetBoxTypes(Element);
	swInstructionsBoxStyle BoxStyle;

	BoxStyle.Margin = GetIntProperty(BoxTypes.Margin, Model, Step);
	BoxStyle.Padding = GetIntProperty(BoxTypes.Padding, Model, Step);
	BoxStyle.BorderWidth = GetIntProperty(BoxTypes.BorderWidth, Model, Step);
	BoxStyle.CornerRadius = GetIntProperty(BoxTypes.CornerRadius, Model, Step);
	BoxStyle.BorderColor = GetColorProperty(BoxTypes.BorderColor, Model, Step);
	BoxStyle.BackgroundColor = GetColorProperty(BoxTypes.BackgroundColor, Model, Step);

	// a clear final model takes the page background

	if (Element == swInstructionsBoxElement::FinalModel && BoxStyle.BackgroundColor.alpha() == 0)
		BoxStyle.BackgroundColor = GetColorProperty(swInstructionsPropertyType::PageBackgroundColor, nullptr, 0);

	// a clear completion takes its page color

	if (Element == swInstructionsBoxElement::Completion && BoxStyle.BackgroundColor.alpha() == 0)
	{
		const QColor CompletionColor = GetColorProperty(swInstructionsPropertyType::CompletionBackgroundColor, nullptr, 0);
		BoxStyle.BackgroundColor = CompletionColor.alpha() != 0 ? CompletionColor : GetColorProperty(swInstructionsPropertyType::PageBackgroundColor, nullptr, 0);
	}

	return BoxStyle;
}

swInstructionsBoxStyle swInstructions::GetPageBoxStyle(size_t PageIndex, swInstructionsBoxElement Element) const
{
	const swInstructionsBoxTypes BoxTypes = swGetBoxTypes(Element);
	swInstructionsBoxStyle BoxStyle;

	BoxStyle.Margin = GetPageIntProperty(PageIndex, BoxTypes.Margin);
	BoxStyle.Padding = GetPageIntProperty(PageIndex, BoxTypes.Padding);
	BoxStyle.BorderWidth = GetPageIntProperty(PageIndex, BoxTypes.BorderWidth);
	BoxStyle.CornerRadius = GetPageIntProperty(PageIndex, BoxTypes.CornerRadius);
	BoxStyle.BorderColor = GetPageColorProperty(PageIndex, BoxTypes.BorderColor);
	BoxStyle.BackgroundColor = GetPageColorProperty(PageIndex, BoxTypes.BackgroundColor);

	return BoxStyle;
}

swInstructionsPageKey swInstructions::GetPageElementsKey(size_t PageIndex) const
{
	if (PageIndex >= mPages.size())
		return { swInstructionsPageType::Blank, nullptr, 0, -1 };

	// tell pages apart by rank
	auto GetBaseKey = [this](size_t Index)
	{
		const swInstructionsPage& Page = mPages[Index];
		swInstructionsStepKey StepKey = { Page.Model, 0 };

		GetPageKey(Index, StepKey);

		return std::make_tuple(Page.Type, StepKey.first, StepKey.second);
	};

	const std::tuple<swInstructionsPageType, swModel*, swStep> BaseKey = GetBaseKey(PageIndex);
	int Rank = 0;

	for (size_t Index = 0; Index < PageIndex; Index++)
		if (GetBaseKey(Index) == BaseKey)
			Rank++;

	return { std::get<0>(BaseKey), std::get<1>(BaseKey), std::get<2>(BaseKey), Rank };
}

bool swInstructions::IsElementLocked(const swInstructionsElementKey& Key) const
{
	if (mPageState.LockedPositions && mPageState.LockedPositions->find(Key) != mPageState.LockedPositions->end())
		return true;

	return mLockedPositions.find(Key) != mLockedPositions.end();
}

QPointF swInstructions::GetLockedPosition(const swInstructionsElementKey& Key) const
{
	// the page lock comes first
	if (mPageState.LockedPositions)
	{
		const std::map<swInstructionsElementKey, QPointF>::const_iterator PagePosition = mPageState.LockedPositions->find(Key);

		if (PagePosition != mPageState.LockedPositions->end())
			return PagePosition->second;
	}

	const std::map<swInstructionsElementKey, QPointF>::const_iterator Position = mLockedPositions.find(Key);

	if (Position == mLockedPositions.end())
		return QPointF();

	return Position->second;
}

void swInstructions::LockElement(const swInstructionsElementKey& Key, const QPointF& Position)
{
	mLockedPositions[Key] = Position;

	emit PageLayoutChanged();
}

void swInstructions::UnlockElement(const swInstructionsElementKey& Key, const QPointF& Offset)
{
	if (!mLockedPositions.erase(Key))
		return;

	// keep the unlocked element in place
	if (Offset.isNull())
		mElementOffsets.erase(Key);
	else
		mElementOffsets[Key] = Offset;

	emit PageLayoutChanged();
}

bool swInstructions::IsPageLocked(size_t PageIndex) const
{
	return PageIndex < mPages.size() && mLockedPages.find(GetPageElementsKey(PageIndex)) != mLockedPages.end();
}

void swInstructions::LockPage(size_t PageIndex, const std::map<swInstructionsElementKey, QPointF>& Positions)
{
	if (PageIndex >= mPages.size() || IsPageLocked(PageIndex))
		return;

	mLockedPages[GetPageElementsKey(PageIndex)] = Positions;

	emit PageLayoutChanged();
}

// unlocked elements follow their placement again
void swInstructions::UnlockPage(size_t PageIndex)
{
	if (PageIndex >= mPages.size() || !mLockedPages.erase(GetPageElementsKey(PageIndex)))
		return;

	emit PageLayoutChanged();
}

bool swInstructions::IsPageFrozen(size_t PageIndex) const
{
	return PageIndex < mPages.size() && mFrozenPages.find(GetPageElementsKey(PageIndex)) != mFrozenPages.end();
}

void swInstructions::FreezePage(size_t PageIndex)
{
	if (PageIndex >= mPages.size() || IsPageFrozen(PageIndex))
		return;

	// copy what the page shows now
	swInstructionsProperties Properties = mStepProperties;

	for (int Type = 0; Type < static_cast<int>(swInstructionsPropertyType::Count); Type++)
		if (IsPageProperty(static_cast<swInstructionsPropertyType>(Type)))
			Properties[Type].Value = GetPageProperty(PageIndex, static_cast<swInstructionsPropertyType>(Type));

	mFrozenPages[GetPageElementsKey(PageIndex)] = Properties;

	emit PageLayoutChanged();
}

// unfrozen pages drop their copy
void swInstructions::UnfreezePage(size_t PageIndex)
{
	if (PageIndex >= mPages.size() || !mFrozenPages.erase(GetPageElementsKey(PageIndex)))
		return;

	emit PageLayoutChanged();
}

swInstructionsPageState swInstructions::GetPageState(size_t PageIndex) const
{
	swInstructionsPageState PageState;

	if (PageIndex >= mPages.size() || (mLockedPages.empty() && mFrozenPages.empty()))
		return PageState;

	const swInstructionsPageKey PageKey = GetPageElementsKey(PageIndex);
	const auto LockedIt = mLockedPages.find(PageKey);
	const auto FrozenIt = mFrozenPages.find(PageKey);

	if (LockedIt != mLockedPages.end())
		PageState.LockedPositions = &LockedIt->second;

	if (FrozenIt != mFrozenPages.end())
		PageState.FrozenProperties = &FrozenIt->second;

	return PageState;
}

// use to apply the preferences page defaults
void swInstructions::LoadDefaultPageSetup()
{
	const std::pair<SW_PROFILE_KEY, SW_PROFILE_KEY> FloatKeys[] =
	{
		{ SW_PROFILE_INSTRUCTIONS_PAGE_WIDTH, SW_PROFILE_INSTRUCTIONS_DEFAULT_PAGE_WIDTH },
		{ SW_PROFILE_INSTRUCTIONS_PAGE_HEIGHT, SW_PROFILE_INSTRUCTIONS_DEFAULT_PAGE_HEIGHT },
		{ SW_PROFILE_INSTRUCTIONS_MARGIN_LEFT, SW_PROFILE_INSTRUCTIONS_DEFAULT_MARGIN_LEFT },
		{ SW_PROFILE_INSTRUCTIONS_MARGIN_RIGHT, SW_PROFILE_INSTRUCTIONS_DEFAULT_MARGIN_RIGHT },
		{ SW_PROFILE_INSTRUCTIONS_MARGIN_TOP, SW_PROFILE_INSTRUCTIONS_DEFAULT_MARGIN_TOP },
		{ SW_PROFILE_INSTRUCTIONS_MARGIN_BOTTOM, SW_PROFILE_INSTRUCTIONS_DEFAULT_MARGIN_BOTTOM },
		{ SW_PROFILE_INSTRUCTIONS_GRID_SIZE, SW_PROFILE_INSTRUCTIONS_DEFAULT_GRID_SIZE }
	};

	const std::pair<SW_PROFILE_KEY, SW_PROFILE_KEY> IntegerKeys[] =
	{
		{ SW_PROFILE_INSTRUCTIONS_PAGE_UNITS, SW_PROFILE_INSTRUCTIONS_DEFAULT_PAGE_UNITS },
		{ SW_PROFILE_INSTRUCTIONS_MARGIN_UNITS, SW_PROFILE_INSTRUCTIONS_DEFAULT_MARGIN_UNITS },
		{ SW_PROFILE_INSTRUCTIONS_SHOW_GRID, SW_PROFILE_INSTRUCTIONS_DEFAULT_SHOW_GRID },
		{ SW_PROFILE_INSTRUCTIONS_SNAP_TO_GRID, SW_PROFILE_INSTRUCTIONS_DEFAULT_SNAP_TO_GRID },
		{ SW_PROFILE_INSTRUCTIONS_GRID_WITHIN_MARGINS, SW_PROFILE_INSTRUCTIONS_DEFAULT_GRID_WITHIN_MARGINS },
		{ SW_PROFILE_INSTRUCTIONS_GRID_DIVISIONS, SW_PROFILE_INSTRUCTIONS_DEFAULT_GRID_DIVISIONS }
	};

	for (const std::pair<SW_PROFILE_KEY, SW_PROFILE_KEY>& Keys : FloatKeys)
		swSetProfileFloat(Keys.first, swGetProfileFloat(Keys.second));

	for (const std::pair<SW_PROFILE_KEY, SW_PROFILE_KEY>& Keys : IntegerKeys)
		swSetProfileInteger(Keys.first, swGetProfileInteger(Keys.second));
}

// use to restore the preferences page defaults
void swInstructions::ResetPageSetup()
{
	LoadDefaultPageSetup();

	SetShowGrid(swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_SHOW_GRID) != 0);
	SetSnapToGrid(swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_SNAP_TO_GRID) != 0);
	SetGridSize(swGetProfileFloat(SW_PROFILE_INSTRUCTIONS_GRID_SIZE));
	SetGridDivisions(swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_GRID_DIVISIONS));
	SetGridWithinMargins(swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_GRID_WITHIN_MARGINS) != 0);

	// follow the paper units
	SetDefaultInt(swInstructionsPropertyType::DocumentUnits, swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_PAGE_UNITS));
	SetDefaultBool(swInstructionsPropertyType::UseDocumentUnits, true);

	SetPageSetup({ swGetProfileFloat(SW_PROFILE_INSTRUCTIONS_PAGE_WIDTH), swGetProfileFloat(SW_PROFILE_INSTRUCTIONS_PAGE_HEIGHT), swGetProfileFloat(SW_PROFILE_INSTRUCTIONS_MARGIN_LEFT), swGetProfileFloat(SW_PROFILE_INSTRUCTIONS_MARGIN_RIGHT), swGetProfileFloat(SW_PROFILE_INSTRUCTIONS_MARGIN_TOP), swGetProfileFloat(SW_PROFILE_INSTRUCTIONS_MARGIN_BOTTOM) });
}

void swInstructions::SetShowGrid(bool ShowGrid)
{
	if (mShowGrid == ShowGrid)
		return;

	mShowGrid = ShowGrid;
	swSetProfileInteger(SW_PROFILE_INSTRUCTIONS_SHOW_GRID, mShowGrid);

	emit GridChanged();
}

void swInstructions::SetSnapToGrid(bool SnapToGrid)
{
	if (mSnapToGrid == SnapToGrid)
		return;

	mSnapToGrid = SnapToGrid;
	swSetProfileInteger(SW_PROFILE_INSTRUCTIONS_SNAP_TO_GRID, mSnapToGrid);

	// grid and anchor snapping exclude each other
	if (mSnapToGrid && mSnapToAnchors)
	{
		mSnapToAnchors = false;
		swSetProfileInteger(SW_PROFILE_INSTRUCTIONS_SNAP_TO_ANCHORS, mSnapToAnchors);
	}

	emit GridChanged();
}

void swInstructions::SetSnapToAnchors(bool SnapToAnchors)
{
	if (mSnapToAnchors == SnapToAnchors)
		return;

	mSnapToAnchors = SnapToAnchors;
	swSetProfileInteger(SW_PROFILE_INSTRUCTIONS_SNAP_TO_ANCHORS, mSnapToAnchors);

	// grid and anchor snapping exclude each other
	if (mSnapToAnchors && mSnapToGrid)
	{
		mSnapToGrid = false;
		swSetProfileInteger(SW_PROFILE_INSTRUCTIONS_SNAP_TO_GRID, mSnapToGrid);
	}

	emit GridChanged();
}

void swInstructions::SetGridSize(float GridSize)
{
	GridSize = qBound(SW_GRID_SIZE_MINIMUM, GridSize, SW_GRID_SIZE_MAXIMUM);

	if (qFuzzyCompare(mGridSize, GridSize))
		return;

	mGridSize = GridSize;
	swSetProfileFloat(SW_PROFILE_INSTRUCTIONS_GRID_SIZE, mGridSize);

	emit GridChanged();
}

void swInstructions::SetGridDivisions(int Divisions)
{
	Divisions = qBound(1, Divisions, SW_GRID_DIVISIONS_MAXIMUM);

	if (mGridDivisions == Divisions)
		return;

	mGridDivisions = Divisions;
	swSetProfileInteger(SW_PROFILE_INSTRUCTIONS_GRID_DIVISIONS, mGridDivisions);

	emit GridChanged();
}

void swInstructions::SetGridPrimaryColor(const QColor& Color)
{
	if (mGridPrimaryColor == Color)
		return;

	mGridPrimaryColor = Color;
	swSetProfileUnsignedInteger(SW_PROFILE_INSTRUCTIONS_GRID_PRIMARY_COLOR, swRGBAFromQColor(Color));

	emit GridChanged();
}

void swInstructions::SetGridSecondaryColor(const QColor& Color)
{
	if (mGridSecondaryColor == Color)
		return;

	mGridSecondaryColor = Color;
	swSetProfileUnsignedInteger(SW_PROFILE_INSTRUCTIONS_GRID_SECONDARY_COLOR, swRGBAFromQColor(Color));

	emit GridChanged();
}

void swInstructions::SetMarginColor(const QColor& Color)
{
	if (mMarginColor == Color)
		return;

	mMarginColor = Color;
	swSetProfileUnsignedInteger(SW_PROFILE_INSTRUCTIONS_MARGIN_COLOR, swRGBAFromQColor(Color));

	emit GridChanged();
}

// use to follow the preferences grid colors
void swInstructions::UpdateColors()
{
	SetGridPrimaryColor(swQColorFromRGBA(swGetProfileUnsignedInteger(SW_PROFILE_INSTRUCTIONS_GRID_PRIMARY_COLOR)));
	SetGridSecondaryColor(swQColorFromRGBA(swGetProfileUnsignedInteger(SW_PROFILE_INSTRUCTIONS_GRID_SECONDARY_COLOR)));
	SetMarginColor(swQColorFromRGBA(swGetProfileUnsignedInteger(SW_PROFILE_INSTRUCTIONS_MARGIN_COLOR)));

	// element colors come from the colors preferences
	const QColor ElementMarginColor = swQColorFromRGBA(swGetProfileUnsignedInteger(SW_PROFILE_INSTRUCTIONS_ELEMENT_MARGIN_COLOR));
	const QColor ElementPaddingColor = swQColorFromRGBA(swGetProfileUnsignedInteger(SW_PROFILE_INSTRUCTIONS_ELEMENT_PADDING_COLOR));
	const QColor ElementAnchorColor = swQColorFromRGBA(swGetProfileUnsignedInteger(SW_PROFILE_INSTRUCTIONS_ELEMENT_ANCHOR_COLOR));
	const QColor RulerPointerColor = swQColorFromRGBA(swGetProfileUnsignedInteger(SW_PROFILE_INSTRUCTIONS_RULER_POINTER_COLOR));
	const QColor BoundaryEdgeColor = swQColorFromRGBA(swGetProfileUnsignedInteger(SW_PROFILE_INSTRUCTIONS_BOUNDARY_EDGE_COLOR));
	const QColor BoundaryBorderColor = swQColorFromRGBA(swGetProfileUnsignedInteger(SW_PROFILE_INSTRUCTIONS_BOUNDARY_BORDER_COLOR));
	const QColor LayoutAreaColor = swQColorFromRGBA(swGetProfileUnsignedInteger(SW_PROFILE_INSTRUCTIONS_LAYOUT_AREA_COLOR));

	if (mElementMarginColor == ElementMarginColor && mElementPaddingColor == ElementPaddingColor && mElementAnchorColor == ElementAnchorColor && mRulerPointerColor == RulerPointerColor && mBoundaryEdgeColor == BoundaryEdgeColor && mBoundaryBorderColor == BoundaryBorderColor && mLayoutAreaColor == LayoutAreaColor)
		return;

	mElementMarginColor = ElementMarginColor;
	mElementPaddingColor = ElementPaddingColor;
	mElementAnchorColor = ElementAnchorColor;
	mRulerPointerColor = RulerPointerColor;
	mBoundaryEdgeColor = BoundaryEdgeColor;
	mBoundaryBorderColor = BoundaryBorderColor;
	mLayoutAreaColor = LayoutAreaColor;

	emit GridChanged();
}

void swInstructions::SetGridWithinMargins(bool WithinMargins)
{
	if (mGridWithinMargins == WithinMargins)
		return;

	mGridWithinMargins = WithinMargins;
	swSetProfileInteger(SW_PROFILE_INSTRUCTIONS_GRID_WITHIN_MARGINS, mGridWithinMargins);

	emit GridChanged();
}

void swInstructions::SetShowMargins(bool ShowMargins)
{
	if (mShowMargins == ShowMargins)
		return;

	mShowMargins = ShowMargins;
	swSetProfileInteger(SW_PROFILE_INSTRUCTIONS_SHOW_MARGINS, mShowMargins);

	emit GridChanged();
}

void swInstructions::SetShowElementMargins(bool ShowElementMargins)
{
	SetShowOption(mShowElementMargins, ShowElementMargins, SW_PROFILE_INSTRUCTIONS_SHOW_ELEMENT_MARGINS);
}

void swInstructions::SetShowElementPadding(bool ShowElementPadding)
{
	SetShowOption(mShowElementPadding, ShowElementPadding, SW_PROFILE_INSTRUCTIONS_SHOW_ELEMENT_PADDING);
}

void swInstructions::SetShowElementAnchors(bool ShowElementAnchors)
{
	SetShowOption(mShowElementAnchors, ShowElementAnchors, SW_PROFILE_INSTRUCTIONS_SHOW_ELEMENT_ANCHORS);
}

void swInstructions::SetElementBoundaries(swInstructionsBoundaries ElementBoundaries)
{
	if (mElementBoundaries == ElementBoundaries)
		return;

	mElementBoundaries = ElementBoundaries;
	swSetProfileInteger(SW_PROFILE_INSTRUCTIONS_SHOW_ELEMENT_BOUNDARIES, static_cast<int>(ElementBoundaries));

	emit GridChanged();
}

void swInstructions::SetShowLayoutAreas(bool ShowLayoutAreas)
{
	SetShowOption(mShowLayoutAreas, ShowLayoutAreas, SW_PROFILE_INSTRUCTIONS_SHOW_LAYOUT_AREAS);
}

void swInstructions::SetShowRulers(bool ShowRulers)
{
	SetShowOption(mShowRulers, ShowRulers, SW_PROFILE_INSTRUCTIONS_SHOW_RULERS);
}

// use to store a screen option
void swInstructions::SetShowOption(bool& Option, bool Value, SW_PROFILE_KEY Key)
{
	if (Option == Value)
		return;

	Option = Value;
	swSetProfileInteger(Key, Option);

	emit GridChanged();
}

QRectF swInstructions::GetGridRect() const
{
	if (mGridWithinMargins)
		return QRectF(mPageSetup.MarginLeft, mPageSetup.MarginTop, mPageSetup.Width - mPageSetup.MarginLeft - mPageSetup.MarginRight, mPageSetup.Height - mPageSetup.MarginTop - mPageSetup.MarginBottom);

	return QRectF(0.0, 0.0, mPageSetup.Width, mPageSetup.Height);
}

swInstructionsPageUnits swInstructions::GetPageUnits() const
{
	const int Units = swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_MARGIN_UNITS);

	if (Units < 0 || Units > static_cast<int>(swInstructionsPageUnits::Inches))
		return swInstructionsPageUnits::Inches;

	return static_cast<swInstructionsPageUnits>(Units);
}

swInstructionsPageUnits swInstructions::GetRulerUnits() const
{
	const std::optional<swInstructionsPageUnits> DocumentUnits = GetDocumentUnits();

	if (DocumentUnits)
		return *DocumentUnits;

	return GetPageUnits();
}

std::optional<swInstructionsPageUnits> swInstructions::GetDocumentUnits() const
{
	if (!GetBoolProperty(swInstructionsPropertyType::UseDocumentUnits, nullptr, 0))
		return std::nullopt;

	const int Units = GetIntProperty(swInstructionsPropertyType::DocumentUnits, nullptr, 0);

	if (Units < 0 || Units > static_cast<int>(swInstructionsPageUnits::Inches))
		return swInstructionsPageUnits::Millimeters;

	return static_cast<swInstructionsPageUnits>(Units);
}

QVariant swInstructions::GetProperty(swInstructionsPropertyType Type, swModel* Model, swStep Step) const
{
	// a frozen page reads its copy
	if (mPageState.FrozenProperties)
		return (*mPageState.FrozenProperties)[static_cast<int>(Type)].Value;

	size_t PageIndex;

	// a page value overrides its steps
	if (!mPagePropertyOverrides.empty() && IsPageProperty(Type) && GetStepPageIndex(Model, Step, PageIndex) && HasPageProperty(PageIndex, Type))
		return GetPageProperty(PageIndex, Type);

	QVariant Value = mStepProperties[static_cast<int>(Type)].Value;

	std::map<swModel*, swInstructionsModel>::const_iterator ModelIt = mModels.find(Model);

	if (ModelIt == mModels.end())
		return Value;

	const swInstructionsModel& InstructionModel = ModelIt->second;

	for (swStep StepIndex = 0; StepIndex <= Step; StepIndex++)
	{
		const swInstructionsProperties& Properties = InstructionModel.StepProperties[StepIndex];
		const swInstructionsProperty& Property = Properties[static_cast<int>(Type)];

		if (Property.Mode == swInstructionsPropertyMode::NotSet || (Property.Mode == swInstructionsPropertyMode::StepOnly && StepIndex != Step))
			continue;

		Value = Property.Value;
	}

	return Value;
}

void swInstructions::SetDefaultBool(swInstructionsPropertyType Type, bool Enabled)
{
	SetDefaultProperty(Type, Enabled);
}

void swInstructions::SetDefaultInt(swInstructionsPropertyType Type, int Value)
{
	SetDefaultProperty(Type, Value);
}

void swInstructions::SetDefaultColor(swInstructionsPropertyType Type, const QColor& Color)
{
	SetDefaultProperty(Type, swRGBAFromQColor(Color));
}

void swInstructions::SetDefaultFont(swInstructionsPropertyType Type, const QFont& Font)
{
	SetDefaultProperty(Type, Font.toString());
}

void swInstructions::SetDefaultText(swInstructionsPropertyType Type, const QString& Text)
{
	SetDefaultProperty(Type, Text);
}

QString swInstructions::GetTextProperty(swInstructionsPropertyType Type) const
{
	return mStepProperties[static_cast<int>(Type)].Value.toString();
}

// use to find a linked file, relative to the model file
QString swInstructions::GetFilePath(const QString& FileName) const
{
	if (FileName.isEmpty() || !QFileInfo(FileName).isRelative() || !mProject || mProject->GetFileName().isEmpty())
		return FileName;

	return QFileInfo(mProject->GetFileName()).dir().filePath(FileName);
}

// use to read a text file, relative to the model file
QString swInstructions::GetFileText(const QString& FileName) const
{
	if (FileName.isEmpty())
		return QString();

	QFile File(GetFilePath(FileName));

	if (!File.open(QIODevice::ReadOnly | QIODevice::Text))
		return QString();

	return QString::fromUtf8(File.readAll()).trimmed();
}

// the file wins, then the typed text, then the automatic text
QString swInstructions::GetSourcedText(swInstructionsPropertyType TextType, swInstructionsPropertyType FileType, const QString& Automatic) const
{
	const QString FileText = GetFileText(GetTextProperty(FileType));

	return FillModelCodes(FileText.isEmpty() ? GetTypedText(TextType, Automatic) : FileText);
}

// the typed text wins over the automatic text
QString swInstructions::GetTypedText(swInstructionsPropertyType TextType, const QString& Automatic) const
{
	const QString Text = GetTextProperty(TextType);

	return Text.isEmpty() ? Automatic : Text;
}

// use to get the automatic title
QString swInstructions::GetModelTitle(const swModel* Model) const
{
	const swModelProperties& Properties = Model->GetProperties();
	QString Title = Properties.mDescription.trimmed();

	if (Title.isEmpty())
		Title = Properties.mModelName.trimmed();

	if (Title.isEmpty())
		Title = QFileInfo(Properties.mFileName).completeBaseName();

	return Title;
}

static int swGetPartsListTotal(const swPartsList& PartsList)
{
	int Total = 0;

	for (const auto& PartEntry : PartsList)
		for (const auto& ColorEntry : PartEntry.second)
			Total += ColorEntry.second;

	return Total;
}

// use to show the model size and weight like the summary
static QString swGetModelSizeText(const swModel* Model)
{
	const swBoundingBox Bounds = Model->GetAllPartsBoundingBox();

	return swGetMeasurementsText(Bounds.Max - Bounds.Min, QLocale());
}

static QString swGetModelWeightText(const swModel* Model)
{
	swPartsList PartsList;
	Model->GetPartsList(gDefaultColor, true, false, PartsList);

	int Missing;
	const double Weight = swGetPartsListWeight(PartsList, Missing);

	return swGetWeightText(Weight, swGetPartsListTotal(PartsList), Missing, QLocale());
}

// use to get the model field a publication text shows
QString swInstructions::GetModelFieldText(const swModel* Model, swInstructionsPlacementElement Element) const
{
	const swModelProperties& Properties = Model->GetProperties();

	const swInstructionsPublicationElement* Publication = swFindPublicationElement(Element);

	switch (Publication ? Publication->Kind : swInstructionsCoverElement::Url)
	{
	case swInstructionsCoverElement::Title:
		return GetModelTitle(Model);

	case swInstructionsCoverElement::Author:
		return Properties.mAuthor.trimmed();

	case swInstructionsCoverElement::ProductId:
		return Properties.mProductId;

	case swInstructionsCoverElement::Version:
		return Properties.mVersion;

	case swInstructionsCoverElement::PublicationDate:
		return Properties.mPublicationDate;

	case swInstructionsCoverElement::Contact:
		return Properties.mContact;

	case swInstructionsCoverElement::Copyright:
		return Properties.mCopyright;

	case swInstructionsCoverElement::PublicationNotice:
		return Properties.mNotice;

	case swInstructionsCoverElement::Comments:
		return Properties.mComments.trimmed();

	case swInstructionsCoverElement::Measurements:
		return swGetModelSizeText(Model);

	case swInstructionsCoverElement::Weight:
		return swGetModelWeightText(Model);

	case swInstructionsCoverElement::Pieces:
		return GetPartCountText(GetModelPartCount(Model));

	case swInstructionsCoverElement::Url:
		break;
	}

	return Properties.mUrl;
}

// use to fill the publication codes of a text
QString swInstructions::FillModelCodes(QString Text) const
{
	const swModel* Model = GetMainModel();

	if (Model && Text.contains(QLatin1Char('%')))
	{
		const swModelProperties& Properties = Model->GetProperties();

		Text.replace(QLatin1String("%title"), GetModelTitle(Model)).replace(QLatin1String("%author"), Properties.mAuthor.trimmed()).replace(QLatin1String("%id"), Properties.mProductId).replace(QLatin1String("%version"), Properties.mVersion)
			.replace(QLatin1String("%date"), Properties.mPublicationDate).replace(QLatin1String("%url"), Properties.mUrl).replace(QLatin1String("%contact"), Properties.mContact)
			.replace(QLatin1String("%copyright"), Properties.mCopyright).replace(QLatin1String("%notice"), Properties.mNotice).replace(QLatin1String("%comments"), Properties.mComments.trimmed());

		// size and weight are worked out when asked for
		if (Text.contains(QLatin1String("%size")))
			Text.replace(QLatin1String("%size"), swGetModelSizeText(Model));

		if (Text.contains(QLatin1String("%weight")))
			Text.replace(QLatin1String("%weight"), swGetModelWeightText(Model));
	}

	return Text;
}

swModel* swInstructions::GetMainModel() const
{
	return mProject ? mProject->GetMainModel() : nullptr;
}

int swInstructions::GetModelPartCount(const swModel* Model)
{
	swPartsList PartsList;
	Model->GetPartsList(gDefaultColor, true, false, PartsList);

	return swGetPartsListTotal(PartsList);
}

int swInstructions::GetPageStepPartCount(const swInstructionsPage& Page)
{
	swPartsList PartsList;

	for (const swInstructionsStep& Step : Page.Steps)
		if (!Step.FinalModel)
			Step.Model->GetPartsListForStep(Step.Step, gDefaultColor, PartsList, false);

	return swGetPartsListTotal(PartsList);
}

QString swInstructions::GetPartCountText(int Count) const
{
	const QString Text = GetSourcedText(swInstructionsPropertyType::PartCountText, swInstructionsPropertyType::PartCountFile, QCoreApplication::translate("swInstructionsDialog", "%1 PIECES"));

	return Text.arg(swFormatIntegerLocalized(static_cast<qlonglong>(Count), QLocale()));
}

void swInstructions::SetDefaultProperty(swInstructionsPropertyType Type, const QVariant& Value)
{
	swInstructionsProperty& Property = mStepProperties[static_cast<int>(Type)];

	if (Property.Value == Value)
		return;

	Property.Value = Value;

	// the bill of materials page redraws
	if (Type >= swInstructionsPropertyType::ShowBillOfMaterialsTitle && Type <= swInstructionsPropertyType::BillOfMaterialsPartCountBackgroundColor)
	{
		emit PageLayoutChanged();
		return;
	}

	// cover texts redraw the covers
	if (Type >= swInstructionsPropertyType::CoverElements)
	{
		emit PageLayoutChanged();
		return;
	}

	// dividers and lists change the layout
	switch (Type)
	{
	case swInstructionsPropertyType::ShowBillOfMaterials:
	case swInstructionsPropertyType::ShowCoverPage:
	case swInstructionsPropertyType::ShowBackCover:
	case swInstructionsPropertyType::ShowFinalModelPage:
	case swInstructionsPropertyType::ShowCoverBlankPage:
	case swInstructionsPropertyType::ShowBillOfMaterialsBlankPage:
		CreatePages();
		emit PageLayoutChanged();
		break;

	case swInstructionsPropertyType::UseDocumentUnits:
	case swInstructionsPropertyType::DocumentUnits:
		emit UnitsChanged();
		break;

	case swInstructionsPropertyType::UseDocumentFont:
	case swInstructionsPropertyType::UseDocumentFontSize:
	case swInstructionsPropertyType::DocumentFont:
	case swInstructionsPropertyType::ShowCoverImage:
	case swInstructionsPropertyType::StepNumberAlignment:
	case swInstructionsPropertyType::StepNumberVerticalAlignment:
	case swInstructionsPropertyType::BillOfMaterialsTitleAlignment:
	case swInstructionsPropertyType::BillOfMaterialsTitleVerticalAlignment:
	case swInstructionsPropertyType::PageNumberAlignment:
	case swInstructionsPropertyType::PageNumberVerticalAlignment:
	case swInstructionsPropertyType::PartCountAlignment:
	case swInstructionsPropertyType::PartCountVerticalAlignment:
	case swInstructionsPropertyType::NoticeAlignment:
	case swInstructionsPropertyType::NoticeVerticalAlignment:
	case swInstructionsPropertyType::PageBackgroundColor:
	case swInstructionsPropertyType::SubmodelBackgroundColor:
	case swInstructionsPropertyType::AssemblyBackgroundColor:
	case swInstructionsPropertyType::CompletionBackgroundColor:
	case swInstructionsPropertyType::CompletionMargin:
	case swInstructionsPropertyType::CompletionPadding:
	case swInstructionsPropertyType::CompletionBorderWidth:
	case swInstructionsPropertyType::CompletionBorderColor:
	case swInstructionsPropertyType::CompletionCornerRadius:
	case swInstructionsPropertyType::CompletionBoxBackgroundColor:
	case swInstructionsPropertyType::BlankPageBackgroundColor:
	case swInstructionsPropertyType::CoverBackgroundColor:
	case swInstructionsPropertyType::BillOfMaterialsBackgroundColor:
	case swInstructionsPropertyType::ShowStepDividers:
	case swInstructionsPropertyType::StepDividerColor:
	case swInstructionsPropertyType::StepDividerWidth:
	case swInstructionsPropertyType::StepDividerMargin:
	case swInstructionsPropertyType::StepDividerPadding:
	case swInstructionsPropertyType::StepDividerRounded:
	case swInstructionsPropertyType::StepDividerHorizontal:
	case swInstructionsPropertyType::StepDividerVertical:
	case swInstructionsPropertyType::StepDividerFinalModel:
	case swInstructionsPropertyType::FinalModelBorderWidth:
	case swInstructionsPropertyType::FinalModelBorderColor:
	case swInstructionsPropertyType::FinalModelMargin:
	case swInstructionsPropertyType::StepNumberPlacement:
	case swInstructionsPropertyType::StepNumberAnchor:
	case swInstructionsPropertyType::PLIPlacement:
	case swInstructionsPropertyType::PLIAnchor:
	case swInstructionsPropertyType::PLIReserveArea:
	case swInstructionsPropertyType::BillOfMaterialsPlacement:
	case swInstructionsPropertyType::CoverImagePlacement:
	case swInstructionsPropertyType::StepImagePlacement:
	case swInstructionsPropertyType::StepBackgroundColor:
	case swInstructionsPropertyType::ShowPageNumber:
	case swInstructionsPropertyType::PageNumberFormat:
	case swInstructionsPropertyType::PageNumberFont:
	case swInstructionsPropertyType::PageNumberColor:
	case swInstructionsPropertyType::PageNumberPlacement:
	case swInstructionsPropertyType::ShowPageNumberAllPages:
	case swInstructionsPropertyType::PLIBackgroundColor:
	case swInstructionsPropertyType::PLIBorderColor:
	case swInstructionsPropertyType::PLIBorderWidth:
	case swInstructionsPropertyType::StepImageMargin:
	case swInstructionsPropertyType::StepImagePadding:
	case swInstructionsPropertyType::StepImageBorderWidth:
	case swInstructionsPropertyType::StepImageBorderColor:
	case swInstructionsPropertyType::StepImageBackgroundColor:
	case swInstructionsPropertyType::StepNumberMargin:
	case swInstructionsPropertyType::StepNumberPadding:
	case swInstructionsPropertyType::StepNumberBorderWidth:
	case swInstructionsPropertyType::StepNumberBorderColor:
	case swInstructionsPropertyType::StepNumberBackgroundColor:
	case swInstructionsPropertyType::PLIMargin:
	case swInstructionsPropertyType::PLIPadding:
	case swInstructionsPropertyType::PageNumberMargin:
	case swInstructionsPropertyType::PageNumberPadding:
	case swInstructionsPropertyType::PageNumberBorderWidth:
	case swInstructionsPropertyType::PageNumberBorderColor:
	case swInstructionsPropertyType::PageNumberBackgroundColor:
	case swInstructionsPropertyType::CoverImageMargin:
	case swInstructionsPropertyType::CoverImagePadding:
	case swInstructionsPropertyType::CoverImageBorderWidth:
	case swInstructionsPropertyType::CoverImageBorderColor:
	case swInstructionsPropertyType::CoverImageBackgroundColor:
	case swInstructionsPropertyType::NoticeFont:
	case swInstructionsPropertyType::NoticeTextColor:
	case swInstructionsPropertyType::NoticeMargin:
	case swInstructionsPropertyType::NoticePadding:
	case swInstructionsPropertyType::NoticeBorderWidth:
	case swInstructionsPropertyType::NoticeBorderColor:
	case swInstructionsPropertyType::NoticeBackgroundColor:
	case swInstructionsPropertyType::PointerStyle:
	case swInstructionsPropertyType::PointerTip:
	case swInstructionsPropertyType::PointerColor:
	case swInstructionsPropertyType::PointerWidth:
	case swInstructionsPropertyType::PointerTipSize:
	case swInstructionsPropertyType::PointerBendRadius:
	case swInstructionsPropertyType::StepImageCornerRadius:
	case swInstructionsPropertyType::StepNumberCornerRadius:
	case swInstructionsPropertyType::PLICornerRadius:
	case swInstructionsPropertyType::PageNumberCornerRadius:
	case swInstructionsPropertyType::CoverImageCornerRadius:
	case swInstructionsPropertyType::NoticeCornerRadius:
	case swInstructionsPropertyType::BillOfMaterialsTitleFont:
	case swInstructionsPropertyType::BillOfMaterialsTitleColor:
	case swInstructionsPropertyType::BillOfMaterialsTitleText:
	case swInstructionsPropertyType::BillOfMaterialsTitleFile:
	case swInstructionsPropertyType::ShowBillOfMaterialsPartCount:
	case swInstructionsPropertyType::ShowPagePartCount:
	case swInstructionsPropertyType::PartCountText:
	case swInstructionsPropertyType::PartCountFile:
	case swInstructionsPropertyType::PartCountFont:
	case swInstructionsPropertyType::PartCountColor:
	case swInstructionsPropertyType::PagePartCountFont:
	case swInstructionsPropertyType::PagePartCountColor:
	case swInstructionsPropertyType::BillOfMaterialsPartCountPlacement:
	case swInstructionsPropertyType::PagePartCountPlacement:
	case swInstructionsPropertyType::ShowSubmodelPreview:
	case swInstructionsPropertyType::SubmodelPreviewSize:
	case swInstructionsPropertyType::SubmodelPreviewPlacement:
	case swInstructionsPropertyType::SubmodelPreviewMargin:
	case swInstructionsPropertyType::SubmodelPreviewPadding:
	case swInstructionsPropertyType::SubmodelPreviewBorderWidth:
	case swInstructionsPropertyType::SubmodelPreviewBorderColor:
	case swInstructionsPropertyType::SubmodelPreviewBackgroundColor:
	case swInstructionsPropertyType::SubmodelPreviewCornerRadius:
	case swInstructionsPropertyType::SubmodelPreviewPartCountPlacement:
	case swInstructionsPropertyType::ShowHeaderBand:
	case swInstructionsPropertyType::ShowFooterBand:
	case swInstructionsPropertyType::ShowLeftBand:
	case swInstructionsPropertyType::ShowRightBand:
	case swInstructionsPropertyType::HeaderBandSize:
	case swInstructionsPropertyType::FooterBandSize:
	case swInstructionsPropertyType::LeftBandSize:
	case swInstructionsPropertyType::RightBandSize:
	case swInstructionsPropertyType::HeaderBandColor:
	case swInstructionsPropertyType::FooterBandColor:
	case swInstructionsPropertyType::LeftBandColor:
	case swInstructionsPropertyType::RightBandColor:
	case swInstructionsPropertyType::HeaderBandReserve:
	case swInstructionsPropertyType::FooterBandReserve:
	case swInstructionsPropertyType::LeftBandReserve:
	case swInstructionsPropertyType::RightBandReserve:
	case swInstructionsPropertyType::ShowWatermark:
	case swInstructionsPropertyType::WatermarkOpacity:
	case swInstructionsPropertyType::WatermarkFont:
	case swInstructionsPropertyType::WatermarkTextColor:
	case swInstructionsPropertyType::WatermarkRotation:
	case swInstructionsPropertyType::ImageMargin:
	case swInstructionsPropertyType::ImagePadding:
	case swInstructionsPropertyType::ImageBorderWidth:
	case swInstructionsPropertyType::ImageBorderColor:
	case swInstructionsPropertyType::ImageBackgroundColor:
	case swInstructionsPropertyType::ImageCornerRadius:
	case swInstructionsPropertyType::FinalModelPadding:
	case swInstructionsPropertyType::FinalModelCornerRadius:
	case swInstructionsPropertyType::StepMargin:
	case swInstructionsPropertyType::StepPadding:
	case swInstructionsPropertyType::StepBorderWidth:
	case swInstructionsPropertyType::StepBorderColor:
	case swInstructionsPropertyType::StepCornerRadius:
	case swInstructionsPropertyType::FinalModelBackgroundColor:
	case swInstructionsPropertyType::HeaderBandMargin:
	case swInstructionsPropertyType::HeaderBandPadding:
	case swInstructionsPropertyType::HeaderBandBorderWidth:
	case swInstructionsPropertyType::HeaderBandBorderColor:
	case swInstructionsPropertyType::HeaderBandCornerRadius:
	case swInstructionsPropertyType::FooterBandMargin:
	case swInstructionsPropertyType::FooterBandPadding:
	case swInstructionsPropertyType::FooterBandBorderWidth:
	case swInstructionsPropertyType::FooterBandBorderColor:
	case swInstructionsPropertyType::FooterBandCornerRadius:
	case swInstructionsPropertyType::LeftBandMargin:
	case swInstructionsPropertyType::LeftBandPadding:
	case swInstructionsPropertyType::LeftBandBorderWidth:
	case swInstructionsPropertyType::LeftBandBorderColor:
	case swInstructionsPropertyType::LeftBandCornerRadius:
	case swInstructionsPropertyType::RightBandMargin:
	case swInstructionsPropertyType::RightBandPadding:
	case swInstructionsPropertyType::RightBandBorderWidth:
	case swInstructionsPropertyType::RightBandBorderColor:
	case swInstructionsPropertyType::RightBandCornerRadius:
	case swInstructionsPropertyType::WatermarkMargin:
	case swInstructionsPropertyType::WatermarkPadding:
	case swInstructionsPropertyType::WatermarkBorderWidth:
	case swInstructionsPropertyType::WatermarkBorderColor:
	case swInstructionsPropertyType::WatermarkCornerRadius:
	case swInstructionsPropertyType::WatermarkBackgroundColor:
		emit PageLayoutChanged();
		break;

	default:
		emit StepSettingsChanged(nullptr, 0);
		break;
	}
}

void swInstructions::CreatePages()
{
	mPages.clear();

	if (mProject)
	{
		std::vector<const swModel*> AddedModels;

		swModel* Model = mProject->GetMainModel();

		if (Model)
		{
			if (GetBoolProperty(swInstructionsPropertyType::ShowCoverPage, nullptr, 0))
			{
				swInstructionsPage Page;
				Page.Type = swInstructionsPageType::Cover;
				Page.Model = Model;
				mPages.emplace_back(std::move(Page));

				if (GetBoolProperty(swInstructionsPropertyType::ShowCoverBlankPage, nullptr, 0))
					AddDefaultBlankPage(mCoverBlankKey);
			}

			AddDefaultPages(Model, AddedModels);

			// the finished main model gets its own page

			const bool FinalOnPage = std::any_of(mPages.begin(), mPages.end(), [Model](const swInstructionsPage& StepsPage)
			{
				return std::any_of(StepsPage.Steps.begin(), StepsPage.Steps.end(), [Model](const swInstructionsStep& Step) { return Step.FinalModel && Step.Model == Model; });
			});

			if (GetBoolProperty(swInstructionsPropertyType::ShowFinalModelPage, nullptr, 0) && Model->GetLastStep() > 0 && !FinalOnPage)
			{
				swInstructionsStep FinalStep;
				FinalStep.Model = Model;
				FinalStep.Step = Model->GetLastStep();
				FinalStep.FinalModel = true;

				swInstructionsPage Page;
				Page.Settings = GetStepPageSettings({ nullptr, 0 });
				Page.Steps.push_back(std::move(FinalStep));
				FinishPage(Page);
			}

			if (GetBoolProperty(swInstructionsPropertyType::ShowBillOfMaterials, nullptr, 0))
			{
				if (GetBoolProperty(swInstructionsPropertyType::ShowBillOfMaterialsBlankPage, nullptr, 0))
					AddDefaultBlankPage(mBillOfMaterialsBlankKey);

				swInstructionsPage Page;
				Page.Type = swInstructionsPageType::BillOfMaterials;
				Page.Model = Model;
				mPages.emplace_back(std::move(Page));
			}

			if (GetBoolProperty(swInstructionsPropertyType::ShowBackCover, nullptr, 0))
			{
				swInstructionsPage Page;
				Page.Type = swInstructionsPageType::BackCover;
				Page.Model = Model;
				mPages.emplace_back(std::move(Page));
			}
		}
	}
}

// use to find steps placing one submodel and nothing else
static bool swIsOnlySubModelStep(const swModel* Model, swStep Step)
{
	int Count = 0;

	for (const std::unique_ptr<swPart>& Part : Model->GetParts())
	{
		if (Part->IsHidden() || Part->GetStepShow() != Step)
			continue;

		if (!Part->mPartData->IsModel() || ++Count > 1)
			return false;
	}

	return Count == 1;
}

void swInstructions::AddDefaultPages(swModel* Model, std::vector<const swModel*>& AddedModels)
{
	if (std::find(AddedModels.begin(), AddedModels.end(), Model) != AddedModels.end())
		return;

	AddedModels.push_back(Model);

	const swStep LastStep = Model->GetLastStep();
	swInstructionsPage Page;

	// add entries for new steps
	std::vector<swInstructionsProperties>& StepProperties = mModels[Model].StepProperties;

	if (StepProperties.size() < LastStep + 1)
		StepProperties.resize(LastStep + 1);

	for (swStep Step = 1; Step <= LastStep; Step++)
	{
		std::set<swModel*> StepSubModels;

		for (const std::unique_ptr<swPart>& Part : Model->GetParts())
		{
			if (!Part->IsHidden() && Part->GetStepShow() == Step && Part->mPartData->IsModel())
			{
				swModel* SubModel = Part->mPartData->GetModel();

				if (std::find(AddedModels.begin(), AddedModels.end(), SubModel) == AddedModels.end())
					StepSubModels.insert(SubModel);
			}
		}

		if (!StepSubModels.empty())
		{
			FinishPage(Page);

			for (swModel* SubModel : StepSubModels)
				AddDefaultPages(SubModel, AddedModels);
		}

		AddBlankPages(Page, { Model, Step });

		// a step adding only a just shown finished submodel repeats it

		if (StepSubModels.size() == 1 && !mPages.empty() && swIsOnlySubModelStep(Model, Step) && std::any_of(mPages.back().Steps.begin(), mPages.back().Steps.end(), [&StepSubModels](const swInstructionsStep& PageStep) { return PageStep.FinalModel && PageStep.Model == *StepSubModels.begin(); }))
			continue;

		if (Page.Steps.empty())
			Page.Settings = GetStepPageSettings({ Model, Step });

		swInstructionsStep InstructionsStep;

		InstructionsStep.Model = Model;
		InstructionsStep.Step = Step;

		Page.Steps.push_back(std::move(InstructionsStep));

		if (static_cast<int>(Page.Steps.size()) == Page.Settings.Rows * Page.Settings.Columns)
			FinishPage(Page);
	}

	// the finished model fills the room left after the last step

	// a full last page already ends on it

	if (LastStep > 0 && !Page.Steps.empty() && Page.Settings.FinalModel)
	{
		swInstructionsStep FinalStep;

		FinalStep.Model = Model;
		FinalStep.Step = LastStep;
		FinalStep.FinalModel = true;

		Page.Steps.push_back(std::move(FinalStep));
	}

	AddBlankPages(Page, { Model, LastStep + 1 });

	FinishPage(Page);
}

// use to find the page of the finished main model
bool swInstructions::IsCompletionPage(size_t PageIndex) const
{
	if (PageIndex >= mPages.size() || mPages[PageIndex].Type != swInstructionsPageType::Steps)
		return false;

	const swModel* MainModel = GetMainModel();
	const std::vector<swInstructionsStep>& Steps = mPages[PageIndex].Steps;

	return MainModel && std::any_of(Steps.begin(), Steps.end(), [MainModel](const swInstructionsStep& Step) { return Step.FinalModel && Step.Model == MainModel; });
}

// use to tell assembly pages from the rest
bool swInstructions::GetPageTitleModel(size_t PageIndex, const swModel*& TitleModel) const
{
	TitleModel = nullptr;

	if (PageIndex >= mPages.size() || mPages[PageIndex].Type != swInstructionsPageType::Steps)
		return false;

	// pages joining two or more models are assemblies

	const std::vector<swInstructionsStep>& Steps = mPages[PageIndex].Steps;
	const swModel* Model = Steps.empty() ? nullptr : Steps.front().Model;

	if (!Model)
		return false;

	swStep LastStep = 0;

	for (const swInstructionsStep& Step : Steps)
		LastStep = qMax(LastStep, Step.Step);

	std::set<const swModel*> Shown;
	const swModel* Focus = nullptr;

	for (const std::unique_ptr<swPart>& Part : Model->GetParts())
	{
		if (Part->IsHidden() || Part->GetStepShow() > LastStep)
			continue;

		const swModel* PartModel = Part->mPartData->IsModel() ? Part->mPartData->GetModel() : Model;
		Shown.insert(PartModel);

		// the first submodel added here names the page
		const bool Added = std::any_of(Steps.begin(), Steps.end(), [&Part](const swInstructionsStep& Step) { return Step.Step == Part->GetStepShow(); });

		if (Added && PartModel != Model && !Focus)
			Focus = PartModel;
	}

	// a page of only the finished model is an assembly
	if (std::all_of(Steps.begin(), Steps.end(), [](const swInstructionsStep& Step) { return Step.FinalModel; }))
		Focus = Model;

	if (Shown.size() >= 2 || Focus == Model)
	{
		TitleModel = Focus ? Focus : Model;
		return true;
	}

	TitleModel = Shown.empty() ? Model : *Shown.begin();
	return false;
}

void swInstructions::FinishPage(swInstructionsPage& Page)
{
	if (Page.Steps.empty())
		return;

	LayoutPage(Page);

	mPages.emplace_back(std::move(Page));
	Page = swInstructionsPage();
}

void swInstructions::LayoutPage(swInstructionsPage& Page) const
{
	const swInstructionsPageSettings& PageSettings = Page.Settings;
	const bool Horizontal = PageSettings.Direction == swInstructionsDirection::Horizontal;
	const int Count = static_cast<int>(Page.Steps.size());

	// partial pages drop their empty lines

	const int Rows = Horizontal ? qMin(PageSettings.Rows, (Count + PageSettings.Columns - 1) / PageSettings.Columns) : PageSettings.Rows;
	const int Columns = Horizontal ? PageSettings.Columns : qMin(PageSettings.Columns, (Count + PageSettings.Rows - 1) / PageSettings.Rows);
	const double Width = 1.0 / static_cast<double>(Columns);
	const double Height = 1.0 / static_cast<double>(Rows);

	for (int Index = 0; Index < Count; Index++)
	{
		const int Row = Horizontal ? Index / Columns : Index % Rows;
		const int Column = Horizontal ? Index % Columns : Index / Rows;
		QRectF& Rect = Page.Steps[Index].Rect;

		Rect = QRectF(Column * Width, Row * Height, Width, Height);

		// the last step stretches into the empty cells

		if (Index == Count - 1)
		{
			if (Horizontal)
				Rect.setRight(1.0);
			else
				Rect.setBottom(1.0);
		}
	}
}

void swInstructions::AddBlankPages(swInstructionsPage& Page, const swInstructionsStepKey& Key)
{
	const size_t BlankCount = mBlankPages.count(Key);

	if (!BlankCount)
		return;

	// a blank page ends the page before it
	FinishPage(Page);

	for (size_t BlankIndex = 0; BlankIndex < BlankCount; BlankIndex++)
	{
		swInstructionsPage BlankPage;
		BlankPage.Type = swInstructionsPageType::Blank;
		BlankPage.BlankKey = Key;
		mPages.emplace_back(std::move(BlankPage));
	}
}

void swInstructions::AddDefaultBlankPage(const swInstructionsStepKey& Key)
{
	swInstructionsPage BlankPage;
	BlankPage.Type = swInstructionsPageType::Blank;
	BlankPage.BlankKey = Key;
	mPages.emplace_back(std::move(BlankPage));
}
