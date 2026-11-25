#include "sw_global.h"
#include "sw_instructions.h"
#include "sw_project.h"
#include "sw_window_main.h"
#include "sw_window_utilities.h"

// metas name properties instead of numbers
static const QLatin1String gInstructionsPropertyNames[] =
{
	QLatin1String("STEP SHOW_NUMBER"), QLatin1String("STEP SHOW_PARTS_LIST"), QLatin1String("STEP NUMBER_FONT"), QLatin1String("STEP NUMBER_COLOR"),
	QLatin1String("STEP BACKGROUND_COLOR"), QLatin1String("STEP PARTS_LIST_BACKGROUND_COLOR"), QLatin1String("STEP PARTS_LIST_FONT"),
	QLatin1String("STEP PARTS_LIST_TEXT_COLOR"), QLatin1String("STEP PARTS_LIST_BORDER_COLOR"), QLatin1String("STEP PARTS_LIST_BORDER_WIDTH"),
	QLatin1String("STEP PARTS_LIST_SHOW_COUNTS"), QLatin1String("STEP PARTS_LIST_SORT_ORDER"), QLatin1String("STEP SHOW_DIVIDERS"), QLatin1String("STEP DIVIDER_COLOR"),
	QLatin1String("STEP DIVIDER_WIDTH"), QLatin1String("BILL_OF_MATERIALS SHOW"), QLatin1String("DOCUMENT USE_FONT"), QLatin1String("DOCUMENT USE_FONT_SIZE"),
	QLatin1String("DOCUMENT FONT"), QLatin1String("COVER SHOW_FRONT"), QLatin1String("PAGE BACKGROUND_COLOR"), QLatin1String("STEP NUMBER_PLACEMENT"),
	QLatin1String("STEP NUMBER_ANCHOR"), QLatin1String("STEP PARTS_LIST_PLACEMENT"), QLatin1String("STEP PARTS_LIST_ANCHOR"), QLatin1String("BILL_OF_MATERIALS PLACEMENT"),
	QLatin1String("STEP PARTS_LIST_THUMBNAIL_SIZE"), QLatin1String("STEP PARTS_LIST_THUMBNAIL_BACKGROUND_COLOR"), QLatin1String("COVER FRONT IMAGE_PLACEMENT"),
	QLatin1String("STEP IMAGE_PLACEMENT"), QLatin1String("COVER BACKGROUND_COLOR"), QLatin1String("BILL_OF_MATERIALS BACKGROUND_COLOR"),
	QLatin1String("PAGE SHOW_COVER_BLANK_PAGE"), QLatin1String("PAGE SHOW_BILL_OF_MATERIALS_BLANK_PAGE"), QLatin1String("DOCUMENT USE_UNITS"),
	QLatin1String("DOCUMENT UNITS"), QLatin1String("PAGE SHOW_NUMBER"), QLatin1String("PAGE NUMBER_FORMAT"), QLatin1String("PAGE NUMBER_FONT"),
	QLatin1String("PAGE NUMBER_COLOR"), QLatin1String("PAGE NUMBER_PLACEMENT"), QLatin1String("PAGE SHOW_NUMBER_ALL_PAGES"), QLatin1String("STEP IMAGE_MARGIN"),
	QLatin1String("STEP IMAGE_PADDING"), QLatin1String("STEP IMAGE_BORDER_WIDTH"), QLatin1String("STEP IMAGE_BORDER_COLOR"), QLatin1String("STEP IMAGE_BACKGROUND_COLOR"),
	QLatin1String("STEP NUMBER_MARGIN"), QLatin1String("STEP NUMBER_PADDING"), QLatin1String("STEP NUMBER_BORDER_WIDTH"), QLatin1String("STEP NUMBER_BORDER_COLOR"),
	QLatin1String("STEP NUMBER_BACKGROUND_COLOR"), QLatin1String("STEP PARTS_LIST_MARGIN"), QLatin1String("STEP PARTS_LIST_PADDING"), QLatin1String("PAGE NUMBER_MARGIN"),
	QLatin1String("PAGE NUMBER_PADDING"), QLatin1String("PAGE NUMBER_BORDER_WIDTH"), QLatin1String("PAGE NUMBER_BORDER_COLOR"),
	QLatin1String("PAGE NUMBER_BACKGROUND_COLOR"), QLatin1String("COVER FRONT IMAGE_MARGIN"), QLatin1String("COVER FRONT IMAGE_PADDING"),
	QLatin1String("COVER FRONT IMAGE_BORDER_WIDTH"), QLatin1String("COVER FRONT IMAGE_BORDER_COLOR"), QLatin1String("COVER FRONT IMAGE_BACKGROUND_COLOR"),
	QLatin1String("NOTICE FONT"), QLatin1String("NOTICE TEXT_COLOR"), QLatin1String("NOTICE MARGIN"), QLatin1String("NOTICE PADDING"), QLatin1String("NOTICE BORDER_WIDTH"),
	QLatin1String("NOTICE BORDER_COLOR"), QLatin1String("NOTICE BACKGROUND_COLOR"), QLatin1String("POINTER STYLE"), QLatin1String("POINTER TIP"),
	QLatin1String("POINTER COLOR"), QLatin1String("POINTER WIDTH"), QLatin1String("POINTER TIP_SIZE"), QLatin1String("STEP PARTS_LIST_LAYOUT"),
	QLatin1String("STEP PARTS_LIST_ALIGNMENT"), QLatin1String("DOCUMENT PROJECTION"), QLatin1String("STEP FADE_PREVIOUS_STEPS"), QLatin1String("STEP FADE_COLOR"),
	QLatin1String("STEP HIGHLIGHT_NEW_PARTS"), QLatin1String("STEP HIGHLIGHT_COLOR"), QLatin1String("STEP IMAGE_CORNER_RADIUS"), QLatin1String("STEP NUMBER_CORNER_RADIUS"),
	QLatin1String("STEP PARTS_LIST_CORNER_RADIUS"), QLatin1String("PAGE NUMBER_CORNER_RADIUS"), QLatin1String("COVER FRONT IMAGE_CORNER_RADIUS"),
	QLatin1String("NOTICE CORNER_RADIUS"), QLatin1String("BILL_OF_MATERIALS TITLE_FONT"), QLatin1String("BILL_OF_MATERIALS TITLE_COLOR"),
	QLatin1String("STEP SUBMODEL_BACKGROUND_COLOR"), QLatin1String("STEP MODEL_SIZE"), QLatin1String("COVER FRONT MODEL_SIZE"),
	QLatin1String("BILL_OF_MATERIALS TITLE_TEXT"), QLatin1String("BILL_OF_MATERIALS TITLE_FILE"), QLatin1String("STEP DIVIDER_MARGIN"),
	QLatin1String("STEP DIVIDER_PADDING"), QLatin1String("STEP DIVIDER_ROUNDED"), QLatin1String("STEP DIVIDER_HORIZONTAL"), QLatin1String("STEP DIVIDER_VERTICAL"),
	QLatin1String("STEP HIGHLIGHT_WIDTH"), QLatin1String("STEP FINAL_MODEL_BORDER_WIDTH"), QLatin1String("STEP FINAL_MODEL_BORDER_COLOR"),
	QLatin1String("STEP FINAL_MODEL_MARGIN"), QLatin1String("BILL_OF_MATERIALS SHOW_PART_IDS"), QLatin1String("BILL_OF_MATERIALS SHOW_PART_COUNT"),
	QLatin1String("PAGE SHOW_PART_COUNT"), QLatin1String("PART_COUNT TEXT"), QLatin1String("PART_COUNT FILE"), QLatin1String("PART_COUNT FONT"),
	QLatin1String("PART_COUNT COLOR"), QLatin1String("PAGE PART_COUNT_FONT"), QLatin1String("PAGE PART_COUNT_COLOR"),
	QLatin1String("BILL_OF_MATERIALS PART_COUNT_PLACEMENT"), QLatin1String("PAGE PART_COUNT_PLACEMENT"), QLatin1String("STEP SHOW_SUBMODEL_PREVIEW"),
	QLatin1String("STEP SUBMODEL_PREVIEW_MODEL_SIZE"), QLatin1String("STEP SUBMODEL_PREVIEW_PLACEMENT"), QLatin1String("STEP SUBMODEL_PREVIEW_MARGIN"),
	QLatin1String("STEP SUBMODEL_PREVIEW_PADDING"), QLatin1String("STEP SUBMODEL_PREVIEW_BORDER_WIDTH"), QLatin1String("STEP SUBMODEL_PREVIEW_BORDER_COLOR"),
	QLatin1String("STEP SUBMODEL_PREVIEW_BACKGROUND_COLOR"), QLatin1String("STEP SUBMODEL_PREVIEW_CORNER_RADIUS"),
	QLatin1String("STEP SUBMODEL_PREVIEW_PART_COUNT_PLACEMENT"), QLatin1String("STEP PARTS_LIST_QUANTITY_PLACEMENT"), QLatin1String("COVER SHOW_BACK"),
	QLatin1String("BAND HEADER SHOW"), QLatin1String("BAND FOOTER SHOW"), QLatin1String("BAND LEFT SHOW"), QLatin1String("BAND RIGHT SHOW"),
	QLatin1String("BAND HEADER SIZE"), QLatin1String("BAND FOOTER SIZE"), QLatin1String("BAND LEFT SIZE"), QLatin1String("BAND RIGHT SIZE"),
	QLatin1String("BAND HEADER COLOR"), QLatin1String("BAND FOOTER COLOR"), QLatin1String("BAND LEFT COLOR"), QLatin1String("BAND RIGHT COLOR"),
	QLatin1String("BAND HEADER RESERVE"), QLatin1String("BAND FOOTER RESERVE"), QLatin1String("BAND LEFT RESERVE"), QLatin1String("BAND RIGHT RESERVE"),
	QLatin1String("WATERMARK SHOW"), QLatin1String("WATERMARK OPACITY"), QLatin1String("STEP NUMBER_ALIGNMENT"), QLatin1String("STEP NUMBER_VERTICAL_ALIGNMENT"),
	QLatin1String("BILL_OF_MATERIALS TITLE_ALIGNMENT"), QLatin1String("BILL_OF_MATERIALS TITLE_VERTICAL_ALIGNMENT"), QLatin1String("PAGE NUMBER_ALIGNMENT"),
	QLatin1String("PAGE NUMBER_VERTICAL_ALIGNMENT"), QLatin1String("PART_COUNT ALIGNMENT"), QLatin1String("PART_COUNT VERTICAL_ALIGNMENT"),
	QLatin1String("NOTICE ALIGNMENT"), QLatin1String("NOTICE VERTICAL_ALIGNMENT"), QLatin1String("COVER FRONT SHOW_IMAGE"), QLatin1String("IMAGE MARGIN"),
	QLatin1String("IMAGE PADDING"), QLatin1String("IMAGE BORDER_WIDTH"), QLatin1String("IMAGE BORDER_COLOR"), QLatin1String("IMAGE BACKGROUND_COLOR"),
	QLatin1String("IMAGE CORNER_RADIUS"), QLatin1String("POINTER BEND_RADIUS"), QLatin1String("STEP SHOW_FINAL_MODEL_PAGE"), QLatin1String("PAGE BLANK_BACKGROUND_COLOR"),
	QLatin1String("STEP PARTS_LIST_QUANTITY_MARGIN"), QLatin1String("STEP PARTS_LIST_QUANTITY_PADDING"), QLatin1String("STEP PARTS_LIST_QUANTITY_BORDER_WIDTH"),
	QLatin1String("STEP PARTS_LIST_QUANTITY_BORDER_COLOR"), QLatin1String("STEP PARTS_LIST_QUANTITY_BACKGROUND_COLOR"),
	QLatin1String("STEP PARTS_LIST_QUANTITY_CORNER_RADIUS"), QLatin1String("STEP FINAL_MODEL_PADDING"), QLatin1String("STEP FINAL_MODEL_CORNER_RADIUS"),
	QLatin1String("STEP AREA_MARGIN"), QLatin1String("STEP AREA_PADDING"), QLatin1String("STEP AREA_BORDER_WIDTH"), QLatin1String("STEP AREA_BORDER_COLOR"),
	QLatin1String("STEP AREA_CORNER_RADIUS"), QLatin1String("STEP FINAL_MODEL_BACKGROUND_COLOR"), QLatin1String("BAND HEADER MARGIN"), QLatin1String("BAND HEADER PADDING"),
	QLatin1String("BAND HEADER BORDER_WIDTH"), QLatin1String("BAND HEADER BORDER_COLOR"), QLatin1String("BAND HEADER CORNER_RADIUS"), QLatin1String("BAND FOOTER MARGIN"),
	QLatin1String("BAND FOOTER PADDING"), QLatin1String("BAND FOOTER BORDER_WIDTH"), QLatin1String("BAND FOOTER BORDER_COLOR"), QLatin1String("BAND FOOTER CORNER_RADIUS"),
	QLatin1String("BAND LEFT MARGIN"), QLatin1String("BAND LEFT PADDING"), QLatin1String("BAND LEFT BORDER_WIDTH"), QLatin1String("BAND LEFT BORDER_COLOR"),
	QLatin1String("BAND LEFT CORNER_RADIUS"), QLatin1String("BAND RIGHT MARGIN"), QLatin1String("BAND RIGHT PADDING"), QLatin1String("BAND RIGHT BORDER_WIDTH"),
	QLatin1String("BAND RIGHT BORDER_COLOR"), QLatin1String("BAND RIGHT CORNER_RADIUS"), QLatin1String("WATERMARK MARGIN"), QLatin1String("WATERMARK PADDING"),
	QLatin1String("WATERMARK BORDER_WIDTH"), QLatin1String("WATERMARK BORDER_COLOR"), QLatin1String("WATERMARK CORNER_RADIUS"), QLatin1String("WATERMARK BACKGROUND_COLOR"),
	QLatin1String("DOCUMENT LOCK_HOME_VIEW"), QLatin1String("WATERMARK FONT"), QLatin1String("WATERMARK TEXT_COLOR"), QLatin1String("WATERMARK ROTATION"),
	QLatin1String("BILL_OF_MATERIALS SHOW_TITLE"), QLatin1String("BILL_OF_MATERIALS TITLE_PLACEMENT"), QLatin1String("BILL_OF_MATERIALS FONT"),
	QLatin1String("BILL_OF_MATERIALS TEXT_COLOR"), QLatin1String("BILL_OF_MATERIALS SHOW_COUNTS"), QLatin1String("BILL_OF_MATERIALS SORT_ORDER"),
	QLatin1String("BILL_OF_MATERIALS LAYOUT"), QLatin1String("BILL_OF_MATERIALS ALIGNMENT"), QLatin1String("BILL_OF_MATERIALS THUMBNAIL_SIZE"),
	QLatin1String("BILL_OF_MATERIALS COUNT_PLACEMENT"), QLatin1String("BILL_OF_MATERIALS TITLE_MARGIN"), QLatin1String("BILL_OF_MATERIALS TITLE_PADDING"),
	QLatin1String("BILL_OF_MATERIALS TITLE_BORDER_WIDTH"), QLatin1String("BILL_OF_MATERIALS TITLE_BORDER_COLOR"), QLatin1String("BILL_OF_MATERIALS TITLE_CORNER_RADIUS"),
	QLatin1String("BILL_OF_MATERIALS TITLE_BACKGROUND_COLOR"), QLatin1String("BILL_OF_MATERIALS LIST_MARGIN"), QLatin1String("BILL_OF_MATERIALS LIST_PADDING"),
	QLatin1String("BILL_OF_MATERIALS LIST_BORDER_WIDTH"), QLatin1String("BILL_OF_MATERIALS LIST_BORDER_COLOR"), QLatin1String("BILL_OF_MATERIALS LIST_CORNER_RADIUS"),
	QLatin1String("BILL_OF_MATERIALS LIST_BACKGROUND_COLOR"), QLatin1String("BILL_OF_MATERIALS QUANTITY_MARGIN"), QLatin1String("BILL_OF_MATERIALS QUANTITY_PADDING"),
	QLatin1String("BILL_OF_MATERIALS QUANTITY_BORDER_WIDTH"), QLatin1String("BILL_OF_MATERIALS QUANTITY_BORDER_COLOR"),
	QLatin1String("BILL_OF_MATERIALS QUANTITY_CORNER_RADIUS"), QLatin1String("BILL_OF_MATERIALS QUANTITY_BACKGROUND_COLOR"),
	QLatin1String("BILL_OF_MATERIALS PART_COUNT_MARGIN"), QLatin1String("BILL_OF_MATERIALS PART_COUNT_PADDING"), QLatin1String("BILL_OF_MATERIALS PART_COUNT_BORDER_WIDTH"),
	QLatin1String("BILL_OF_MATERIALS PART_COUNT_BORDER_COLOR"), QLatin1String("BILL_OF_MATERIALS PART_COUNT_CORNER_RADIUS"),
	QLatin1String("BILL_OF_MATERIALS PART_COUNT_BACKGROUND_COLOR"), QLatin1String("STEP ASSEMBLY_BACKGROUND_COLOR"), QLatin1String("STEP DIVIDER_FINAL_MODEL"),
	QLatin1String("STEP COMPLETION_BACKGROUND_COLOR"), QLatin1String("STEP COMPLETION_MARGIN"), QLatin1String("STEP COMPLETION_PADDING"),
	QLatin1String("STEP COMPLETION_BORDER_WIDTH"), QLatin1String("STEP COMPLETION_BORDER_COLOR"), QLatin1String("STEP COMPLETION_CORNER_RADIUS"),
	QLatin1String("STEP COMPLETION_BOX_BACKGROUND_COLOR"), QLatin1String("STEP PARTS_LIST_RESERVE_AREA")
};

static_assert(std::size(gInstructionsPropertyNames) == static_cast<size_t>(swInstructionsPropertyType::CoverElements), "Missing property name");

// the cover elements follow the others, front then back
static const QLatin1String gInstructionsCoverElementNames[] =
{
	QLatin1String("TITLE"), QLatin1String("AUTHOR"), QLatin1String("PRODUCT_ID"), QLatin1String("VERSION"), QLatin1String("PUBLICATION_DATE"), QLatin1String("URL"), QLatin1String("CONTACT"),
	QLatin1String("COPYRIGHT"), QLatin1String("NOTICE"), QLatin1String("COMMENTS"), QLatin1String("MEASUREMENTS"), QLatin1String("WEIGHT"), QLatin1String("PIECES")
};

static_assert(std::size(gInstructionsCoverElementNames) == SW_COVER_ELEMENT_COUNT, "Missing cover element name");

static const QLatin1String gInstructionsCoverFieldNames[] =
{
	QLatin1String("TEXT"), QLatin1String("FONT"), QLatin1String("COLOR"), QLatin1String("PLACEMENT"), QLatin1String("ALIGNMENT"), QLatin1String("VERTICAL_ALIGNMENT"),
	QLatin1String("MARGIN"), QLatin1String("PADDING"), QLatin1String("BORDER_WIDTH"), QLatin1String("BORDER_COLOR"), QLatin1String("BACKGROUND_COLOR"), QLatin1String("CORNER_RADIUS")
};

static_assert(std::size(gInstructionsCoverFieldNames) + 1 == SW_COVER_FIELD_COUNT, "Missing cover field name");

// use to get the words naming every property
static const QStringList& swGetInstructionsPropertyNames()
{
	static const QStringList Names = []
	{
		QStringList Result;

		for (const QLatin1String& Name : gInstructionsPropertyNames)
			Result.append(Name);

		for (const QString& Cover : { QStringLiteral("COVER FRONT "), QStringLiteral("COVER BACK ") })
			for (const QLatin1String& Element : gInstructionsCoverElementNames)
			{
				Result.append(Cover + QLatin1String("SHOW_") + Element);

				for (const QLatin1String& Field : gInstructionsCoverFieldNames)
					Result.append(Cover + Element + QLatin1Char('_') + Field);
			}

		return Result;
	}();

	return Names;
}


// use to map old properties
static bool swMapOldInstructionsProperty(int& Type, const QVariant& Value)
{
	if (Type != static_cast<int>(swInstructionsPropertyType::PLIThumbnailBackgroundColor))
		return true;

	// transparent fills drew nothing
	if (!swQColorFromRGBA(Value.toUInt()).alpha())
		return false;

	Type = static_cast<int>(swInstructionsPropertyType::PLIBackgroundColor);

	return true;
}

static const QLatin1String gInstructionsElementNames[] =
{
	QLatin1String("NONE"), QLatin1String("STEP_NUMBER"), QLatin1String("PARTS_LIST"), QLatin1String("BILL_OF_MATERIALS"),
	QLatin1String("STEP_IMAGE"), QLatin1String("COVER_IMAGE"), QLatin1String("PAGE_NUMBER"), QLatin1String("NOTICE"), QLatin1String("POINTER"),
	QLatin1String("BILL_OF_MATERIALS_PART_COUNT"), QLatin1String("PAGE_PART_COUNT"), QLatin1String("SUBMODEL_PREVIEW"),
	QLatin1String("SUBMODEL_PREVIEW_PART_COUNT"), QLatin1String("PARTS_LIST_QUANTITY"), QLatin1String("IMAGE"),
	QLatin1String("COVER_FRONT_TITLE"), QLatin1String("COVER_FRONT_AUTHOR"), QLatin1String("COVER_FRONT_PRODUCT_ID"), QLatin1String("COVER_FRONT_VERSION"),
	QLatin1String("COVER_FRONT_PUBLICATION_DATE"), QLatin1String("COVER_FRONT_URL"), QLatin1String("COVER_FRONT_CONTACT"), QLatin1String("COVER_FRONT_COPYRIGHT"),
	QLatin1String("COVER_FRONT_NOTICE"), QLatin1String("COVER_FRONT_COMMENTS"), QLatin1String("COVER_FRONT_MEASUREMENTS"), QLatin1String("COVER_FRONT_WEIGHT"),
	QLatin1String("COVER_FRONT_PIECES"), QLatin1String("COVER_BACK_TITLE"), QLatin1String("COVER_BACK_AUTHOR"), QLatin1String("COVER_BACK_PRODUCT_ID"),
	QLatin1String("COVER_BACK_VERSION"), QLatin1String("COVER_BACK_PUBLICATION_DATE"), QLatin1String("COVER_BACK_URL"), QLatin1String("COVER_BACK_CONTACT"),
	QLatin1String("COVER_BACK_COPYRIGHT"), QLatin1String("COVER_BACK_NOTICE"), QLatin1String("COVER_BACK_COMMENTS"), QLatin1String("COVER_BACK_MEASUREMENTS"),
	QLatin1String("COVER_BACK_WEIGHT"), QLatin1String("COVER_BACK_PIECES")
};

static_assert(std::size(gInstructionsElementNames) == static_cast<size_t>(swInstructionsPlacementElement::CoverBackPieces) + 1, "Missing element name");

static const QLatin1String gInstructionsPageTypeNames[] =
{
	QLatin1String("STEPS"), QLatin1String("COVER"), QLatin1String("BILL_OF_MATERIALS"), QLatin1String("BLANK"), QLatin1String("BACK_COVER")
};

static const QLatin1String gInstructionsImagePagesNames[] =
{
	QLatin1String("NONE"), QLatin1String("FRONT"), QLatin1String("BACK"), QLatin1String("BOTH"),
	QLatin1String("HEADER"), QLatin1String("FOOTER"), QLatin1String("LEFT"), QLatin1String("RIGHT"), QLatin1String("WATERMARK")
};

static const QLatin1String gInstructionsTargetNames[] =
{
	QLatin1String("POINT"), QLatin1String("ELEMENT"), QLatin1String("PART")
};

static const QLatin1String gInstructionsPointerStyleNames[] =
{
	QLatin1String("LINE"), QLatin1String("WEDGE"), QLatin1String("L_SHAPE"), QLatin1String("Z_SHAPE"), QLatin1String("U_SHAPE")
};

static const QLatin1String gInstructionsPointerTipNames[] =
{
	QLatin1String("ARROW"), QLatin1String("DOT"), QLatin1String("HOLLOW_DOT")
};

// default pointers write the default
static const QLatin1String gInstructionsDefaultName("DEFAULT");
static const QLatin1String gInstructionsAutoName("AUTO");

template<size_t Count>
static int swFindInstructionsName(const QLatin1String (&Names)[Count], const QString& Name)
{
	for (size_t Index = 0; Index < Count; Index++)
		if (Names[Index] == Name)
			return static_cast<int>(Index);

	return -1;
}

// use to keep free text on one line
static QString swEscapeInstructionsText(const QString& Text)
{
	QString Escaped;
	Escaped.reserve(Text.size());

	for (const QChar Character : Text)
	{
		if (Character == QLatin1Char('\\'))
			Escaped += QLatin1String("\\\\");
		else if (Character == QLatin1Char('"'))
			Escaped += QLatin1String("\\\"");
		else if (Character == QLatin1Char('\n'))
			Escaped += QLatin1String("\\n");
		else if (Character != QLatin1Char('\r'))
			Escaped += Character;
	}

	return Escaped;
}

static QString swUnescapeInstructionsText(const QString& Text)
{
	QString Unescaped;
	Unescaped.reserve(Text.size());

	for (int Position = 0; Position < Text.size(); Position++)
	{
		const QChar Character = Text.at(Position);

		if (Character != QLatin1Char('\\') || Position + 1 == Text.size())
		{
			Unescaped += Character;
			continue;
		}

		const QChar Escaped = Text.at(++Position);
		Unescaped += Escaped == QLatin1Char('n') ? QLatin1Char('\n') : Escaped;
	}

	return Unescaped;
}

static QString swWriteInstructionsModel(const swModel* Model)
{
	return Model ? QLatin1Char('"') + Model->GetFileName() + QLatin1Char('"') : QStringLiteral("-");
}

static QString swWriteInstructionsStepKey(const swInstructionsStepKey& Key)
{
	return swWriteInstructionsModel(Key.first) + QLatin1Char(' ') + QString::number(Key.second);
}

static QString swWriteInstructionsElementKey(const swInstructionsElementKey& Key)
{
	return gInstructionsElementNames[static_cast<int>(std::get<0>(Key))] + QLatin1Char(' ') + swWriteInstructionsModel(std::get<1>(Key)) + QLatin1Char(' ') + QString::number(std::get<2>(Key));
}

static QString swWriteInstructionsPageKey(const swInstructionsPageKey& Key)
{
	return gInstructionsPageTypeNames[static_cast<int>(std::get<0>(Key))] + QLatin1Char(' ') + swWriteInstructionsModel(std::get<1>(Key)) + QLatin1Char(' ') + QString::number(std::get<2>(Key)) + QLatin1Char(' ') + QString::number(std::get<3>(Key));
}

static QString swWriteInstructionsPoint(const QPointF& Point)
{
	return swFormatValue(Point.x(), 2) + QLatin1Char(' ') + swFormatValue(Point.y(), 2);
}

// the default type sets the format
static QString swWriteInstructionsValue(const QVariant& Value, const QVariant& DefaultValue)
{
	switch (DefaultValue.userType())
	{
	case QMetaType::Bool:
		return Value.toBool() ? QStringLiteral("TRUE") : QStringLiteral("FALSE");

	case QMetaType::UInt:
	{
		const quint32 Color = Value.toUInt();
		return QString::asprintf("#%02X%02X%02X%02X", SW_RGBA_RED(Color), SW_RGBA_GREEN(Color), SW_RGBA_BLUE(Color), SW_RGBA_ALPHA(Color));
	}

	case QMetaType::Int:
		return QString::number(Value.toInt());

	default:
		return swEscapeInstructionsText(Value.toString());
	}
}

// use to read the meta command fields
class swInstructionsCommandReader
{
public:
	swInstructionsCommandReader(const QString& Line, const Project* Project)
		: mLine(Line), mProject(Project)
	{
	}

	bool IsValid() const
	{
		return mValid;
	}

	QString ReadToken()
	{
		SkipSpace();
		mQuoted = mPosition < mLine.size() && mLine.at(mPosition) == QLatin1Char('"');

		if (mQuoted)
		{
			const int End = mLine.indexOf(QLatin1Char('"'), mPosition + 1);

			if (End == -1)
			{
				mValid = false;
				mPosition = mLine.size();
				return QString();
			}

			const QString Token = mLine.mid(mPosition + 1, End - mPosition - 1);
			mPosition = End + 1;
			return Token;
		}

		const int Begin = mPosition;

		while (mPosition < mLine.size() && !mLine.at(mPosition).isSpace())
			mPosition++;

		if (Begin == mPosition)
			mValid = false;

		return mLine.mid(Begin, mPosition - Begin);
	}

	QString ReadRest()
	{
		SkipSpace();
		const QString Rest = mLine.mid(mPosition);
		mPosition = mLine.size();
		return Rest;
	}

	void ReadKeyword(const char* Keyword)
	{
		if (ReadToken() != QLatin1String(Keyword))
			mValid = false;
	}

	int ReadInt()
	{
		bool Ok = false;
		const int Value = ReadToken().toInt(&Ok);
		mValid &= Ok;
		return Value;
	}

	qreal ReadNumber()
	{
		bool Ok = false;
		const qreal Value = ReadToken().toDouble(&Ok);
		mValid &= Ok;
		return Value;
	}

	QPointF ReadPoint()
	{
		const qreal X = ReadNumber();
		return QPointF(X, ReadNumber());
	}

	template<size_t Count>
	int ReadName(const QLatin1String (&Names)[Count])
	{
		const int Index = swFindInstructionsName(Names, ReadToken());

		if (Index == -1)
			mValid = false;

		return qMax(Index, 0);
	}

	// property names have two or three words
	int ReadPropertyName()
	{
		const QStringList& Names = swGetInstructionsPropertyNames();
		QString Name = ReadToken();

		for (int Words = 1; Words <= 3 && mValid; Words++)
		{
			const int Index = Names.indexOf(Name);

			if (Index != -1)
				return Index;

			Name += QLatin1Char(' ') + ReadToken();
		}

		mValid = false;
		return 0;
	}

	// write defaults by name, others by number
	int ReadChoice(const QLatin1String& Name, int Value)
	{
		const QString Token = ReadToken();

		if (Token == Name)
			return Value;

		bool Ok = false;
		const int Choice = Token.toInt(&Ok);
		mValid &= Ok;
		return Choice;
	}

	swModel* ReadModel()
	{
		const QString Token = ReadToken();

		if (!mQuoted && Token == QLatin1String("-"))
			return nullptr;

		swModel* Model = mQuoted && mProject ? mProject->GetModel(Token) : nullptr;

		if (!Model)
			mValid = false;

		return Model;
	}

	swInstructionsStepKey ReadStepKey()
	{
		swModel* Model = ReadModel();
		return { Model, static_cast<swStep>(ReadInt()) };
	}

	swInstructionsElementKey ReadElementKey()
	{
		const swInstructionsPlacementElement Element = static_cast<swInstructionsPlacementElement>(ReadName(gInstructionsElementNames));
		swModel* Model = ReadModel();
		return { Element, Model, static_cast<swStep>(ReadInt()) };
	}

	swInstructionsPageKey ReadPageKey()
	{
		const swInstructionsPageType Type = static_cast<swInstructionsPageType>(ReadName(gInstructionsPageTypeNames));
		swModel* Model = ReadModel();
		const swStep Step = static_cast<swStep>(ReadInt());
		return { Type, Model, Step, ReadInt() };
	}

	QVariant ReadValue(const QVariant& DefaultValue)
	{
		switch (DefaultValue.userType())
		{
		case QMetaType::Bool:
		{
			const QString Token = ReadToken();

			if (Token != QLatin1String("TRUE") && Token != QLatin1String("FALSE"))
				mValid = false;

			return Token == QLatin1String("TRUE");
		}

		case QMetaType::UInt:
		{
			const QString Token = ReadToken();
			bool Ok = false;
			const quint32 Color = Token.mid(1).toUInt(&Ok, 16);

			if (!Ok || Token.size() != 9 || !Token.startsWith(QLatin1Char('#')))
			{
				mValid = false;
				return DefaultValue;
			}

			return SW_RGBA((Color >> 24) & 0xff, (Color >> 16) & 0xff, (Color >> 8) & 0xff, Color & 0xff);
		}

		case QMetaType::Int:
			return ReadInt();

		default:
			return swUnescapeInstructionsText(ReadRest());
		}
	}

protected:
	void SkipSpace()
	{
		while (mPosition < mLine.size() && mLine.at(mPosition).isSpace())
			mPosition++;
	}

	QString mLine;
	const Project* mProject;
	int mPosition = 0;
	bool mQuoted = false;
	bool mValid = true;
};

void swRenameInstructionsModels(QStringList& CommandLines, const std::map<QString, QString>& Names)
{
	// quoted names are models
	for (QString& CommandLine : CommandLines)
	{
		QString Renamed;
		int Position = 0;

		while (Position < CommandLine.size())
		{
			const QChar Character = CommandLine.at(Position);

			if (Character == QLatin1Char('\\'))
			{
				Renamed += CommandLine.mid(Position, 2);
				Position += 2;
				continue;
			}

			const int End = Character == QLatin1Char('"') ? CommandLine.indexOf(QLatin1Char('"'), Position + 1) : -1;

			if (End == -1)
			{
				Renamed += Character;
				Position++;
				continue;
			}

			const QString Name = CommandLine.mid(Position + 1, End - Position - 1);
			const std::map<QString, QString>::const_iterator NameIt = Names.find(Name);

			Renamed += QLatin1Char('"') + (NameIt != Names.end() ? NameIt->second : Name) + QLatin1Char('"');
			Position = End + 1;
		}

		CommandLine = Renamed;
	}
}

// older files used the document font with its size

static void swSetInstructionsProperty(swInstructionsProperties& Properties, int Type, const QVariant& Value)
{
	Properties[Type].Value = Value;

	if (Type == static_cast<int>(swInstructionsPropertyType::UseDocumentFont) && Value.toBool())
		Properties[static_cast<int>(swInstructionsPropertyType::UseDocumentFontSize)].Value = true;
}

// placements anchors and alignments

static bool swIsInstructionsLayoutProperty(int Type)
{
	const QString& Name = swGetInstructionsPropertyNames().at(Type);

	return Name.contains(QLatin1String("PLACEMENT")) || Name.contains(QLatin1String("ANCHOR")) || Name.contains(QLatin1String("ALIGNMENT"));
}

// older files kept the earlier cover cells

static void swSetOldCoverPlacements(swInstructionsProperties& Properties, const std::vector<bool>& ReadTypes, const swInstructionsProperties& BuiltInProperties)
{
	// shortcut: covers only the cover cell change of 2026-11-25, extend when other layout defaults change for unsaved files

	for (const swInstructionsPublicationElement& Publication : swGetPublicationElements())
	{
		const int Type = static_cast<int>(swGetPublicationType(Publication, swInstructionsPublicationField::Placement));

		if (ReadTypes[Type] || Properties[Type].Value != BuiltInProperties[Type].Value)
			continue;

		const bool Heading = Publication.Kind == swInstructionsCoverElement::Title || Publication.Kind == swInstructionsCoverElement::Author;
		int Placement = Heading ? SW_PLACEMENT_GRID_SIZE + 2 : SW_PLACEMENT_COUNT - SW_PLACEMENT_GRID_SIZE + 2;

		// pieces sat above the bottom row

		if (Publication.Kind == swInstructionsCoverElement::Pieces)
			Placement = SW_PLACEMENT_COUNT - SW_PLACEMENT_GRID_SIZE - 4;

		Properties[Type].Value = Placement;
	}
}

// the size choice follows a used document font

static bool swIsInstructionsPropertyWritten(const swInstructionsProperties& Properties, int Type, const QVariant& DefaultValue)
{
	const QVariant& Value = Properties[Type].Value;

	if (!Value.isValid())
		return false;

	// layout never follows changed defaults

	if (swIsInstructionsLayoutProperty(Type))
		return true;

	if (Type == static_cast<int>(swInstructionsPropertyType::UseDocumentFontSize) && Properties[static_cast<int>(swInstructionsPropertyType::UseDocumentFont)].Value.toBool())
		return true;

	// files without it copy the steps
	for (const std::pair<swInstructionsPropertyType, swInstructionsPropertyType>& Source : swGetBillOfMaterialsSources())
		if (static_cast<int>(Source.first) == Type && Value != Properties[static_cast<int>(Source.second)].Value)
			return true;

	return Value != DefaultValue;
}

// unwritten bill of materials values follow the steps

static void swCopyBillOfMaterialsProperties(swInstructionsProperties& Properties, const std::vector<bool>& ReadTypes)
{
	for (const std::pair<swInstructionsPropertyType, swInstructionsPropertyType>& Source : swGetBillOfMaterialsSources())
		if (!ReadTypes[static_cast<int>(Source.first)])
			Properties[static_cast<int>(Source.first)].Value = Properties[static_cast<int>(Source.second)].Value;
}

void swInstructions::LoadCommandLines(const QStringList& CommandLines, bool ModelLines)
{
	std::vector<bool> ReadTypes(static_cast<size_t>(swInstructionsPropertyType::Count), false);
	std::map<swInstructionsPageKey, std::vector<bool>> FrozenReadTypes;

	// skip commands with missing models
	for (const QString& CommandLine : CommandLines)
	{
		swInstructionsCommandReader Reader(CommandLine, mProject);
		const QString Command = Reader.ReadToken();

		if (Command == QLatin1String("PAGE_SETUP"))
		{
			swInstructionsPageSetup PageSetup;
			PageSetup.Width = Reader.ReadNumber();
			PageSetup.Height = Reader.ReadNumber();
			PageSetup.MarginLeft = Reader.ReadNumber();
			PageSetup.MarginRight = Reader.ReadNumber();
			PageSetup.MarginTop = Reader.ReadNumber();
			PageSetup.MarginBottom = Reader.ReadNumber();
			Reader.ReadKeyword("UNITS");
			const int PageUnits = static_cast<int>(Reader.ReadNumber());
			const int MarginUnits = static_cast<int>(Reader.ReadNumber());

			if (!Reader.IsValid() || PageSetup.Width < 1.0f || PageSetup.Height < 1.0f || PageSetup.MarginLeft + PageSetup.MarginRight >= PageSetup.Width || PageSetup.MarginTop + PageSetup.MarginBottom >= PageSetup.Height)
				continue;

			// the model keeps its own paper
			mPageSetup = PageSetup;
			swSetProfileFloat(SW_PROFILE_INSTRUCTIONS_PAGE_WIDTH, PageSetup.Width);
			swSetProfileFloat(SW_PROFILE_INSTRUCTIONS_PAGE_HEIGHT, PageSetup.Height);
			swSetProfileFloat(SW_PROFILE_INSTRUCTIONS_MARGIN_LEFT, PageSetup.MarginLeft);
			swSetProfileFloat(SW_PROFILE_INSTRUCTIONS_MARGIN_RIGHT, PageSetup.MarginRight);
			swSetProfileFloat(SW_PROFILE_INSTRUCTIONS_MARGIN_TOP, PageSetup.MarginTop);
			swSetProfileFloat(SW_PROFILE_INSTRUCTIONS_MARGIN_BOTTOM, PageSetup.MarginBottom);
			swSetProfileInteger(SW_PROFILE_INSTRUCTIONS_PAGE_UNITS, PageUnits);
			swSetProfileInteger(SW_PROFILE_INSTRUCTIONS_MARGIN_UNITS, MarginUnits);

			// units follow unless a property sets them
			mStepProperties[static_cast<int>(swInstructionsPropertyType::DocumentUnits)].Value = PageUnits;
			mDefaultProperties[static_cast<int>(swInstructionsPropertyType::DocumentUnits)].Value = PageUnits;
		}
		else if (Command == QLatin1String("PROPERTY") || Command == QLatin1String("PAGE_PROPERTY"))
		{
			const bool PageProperty = Command == QLatin1String("PAGE_PROPERTY");
			swInstructionsStepKey Key;

			if (PageProperty)
			{
				Reader.ReadKeyword("STEP");
				Key = Reader.ReadStepKey();
			}

			int Type = Reader.ReadPropertyName();

			if (!Reader.IsValid() || (PageProperty && !IsPageProperty(static_cast<swInstructionsPropertyType>(Type))))
				continue;

			const QVariant Value = Reader.ReadValue(mDefaultProperties[Type].Value);

			if (!Reader.IsValid() || !swMapOldInstructionsProperty(Type, Value))
				continue;

			if (!PageProperty)
			{
				swSetInstructionsProperty(mStepProperties, Type, Value);
				ReadTypes[Type] = true;
			}
			else
			{
				swInstructionsProperty& Property = mPagePropertyOverrides[Key][Type];
				Property.Mode = swInstructionsPropertyMode::Default;
				Property.Value = Value;
			}
		}
		else if (Command == QLatin1String("STEPS_PER_PAGE") || Command == QLatin1String("PAGE_STEPS_PER_PAGE"))
		{
			const bool PageSettings = Command == QLatin1String("PAGE_STEPS_PER_PAGE");
			swInstructionsStepKey Key;

			if (PageSettings)
			{
				Reader.ReadKeyword("STEP");
				Key = Reader.ReadStepKey();
			}

			swInstructionsPageSettings Settings = mPageSettings;
			Settings.Rows = Reader.ReadInt();
			Settings.Columns = Reader.ReadInt();

			if (!Reader.IsValid() || Settings.Rows < 1 || Settings.Columns < 1)
				continue;

			// flow and final model follow as words

			const QStringList Options = Reader.ReadRest().split(QLatin1Char(' '), Qt::SkipEmptyParts);

			Settings.Direction = Options.contains(QLatin1String("VERTICAL")) ? swInstructionsDirection::Vertical : swInstructionsDirection::Horizontal;
			Settings.FinalModel = Options.contains(QLatin1String("FINAL_MODEL"));

			// the old main page color word now follows the page

			if (PageSettings)
				mPageSettingsOverrides[Key] = Settings;
			else
				mPageSettings = Settings;
		}
		else if (Command == QLatin1String("BLANK_PAGE"))
		{
			Reader.ReadKeyword("STEP");
			const swInstructionsStepKey Key = Reader.ReadStepKey();

			if (Reader.IsValid() && Key.first)
				mBlankPages.insert(Key);
		}
		else if (Command == QLatin1String("SIZE"))
		{
			const swInstructionsElementKey Key = Reader.ReadElementKey();
			const QPointF Size = Reader.ReadPoint();

			if (Reader.IsValid() && Size.x() > 0.0 && Size.y() > 0.0)
				mElementSizes[Key] = QSizeF(Size.x(), Size.y());
		}
		else if (Command == QLatin1String("OFFSET") || Command == QLatin1String("LOCK"))
		{
			const swInstructionsElementKey Key = Reader.ReadElementKey();
			const QPointF Position = Reader.ReadPoint();

			if (!Reader.IsValid())
				continue;

			if (Command == QLatin1String("OFFSET"))
				mElementOffsets[Key] = Position;
			else
				mLockedPositions[Key] = Position;
		}
		else if (Command == QLatin1String("LOCK_PAGE") || Command == QLatin1String("LOCK_PAGE_ELEMENT"))
		{
			Reader.ReadKeyword("PAGE");
			const swInstructionsPageKey PageKey = Reader.ReadPageKey();

			if (!Reader.IsValid())
				continue;

			std::map<swInstructionsElementKey, QPointF>& Positions = mLockedPages[PageKey];

			if (Command == QLatin1String("LOCK_PAGE"))
				continue;

			const swInstructionsElementKey Key = Reader.ReadElementKey();
			const QPointF Position = Reader.ReadPoint();

			if (Reader.IsValid())
				Positions[Key] = Position;
		}
		else if (Command == QLatin1String("FREEZE_PAGE") || Command == QLatin1String("FREEZE_PAGE_PROPERTY"))
		{
			Reader.ReadKeyword("PAGE");
			const swInstructionsPageKey PageKey = Reader.ReadPageKey();

			if (!Reader.IsValid())
				continue;

			// unwritten values are the defaults
			std::map<swInstructionsPageKey, swInstructionsProperties>::iterator FrozenIt = mFrozenPages.find(PageKey);

			if (FrozenIt == mFrozenPages.end())
				FrozenIt = mFrozenPages.emplace(PageKey, mDefaultProperties).first;

			std::vector<bool>& FrozenTypes = FrozenReadTypes[PageKey];
			FrozenTypes.resize(static_cast<size_t>(swInstructionsPropertyType::Count), false);

			if (Command == QLatin1String("FREEZE_PAGE"))
				continue;

			int Type = Reader.ReadPropertyName();

			if (!Reader.IsValid())
				continue;

			const QVariant Value = Reader.ReadValue(mDefaultProperties[Type].Value);

			if (Reader.IsValid() && swMapOldInstructionsProperty(Type, Value))
			{
				swSetInstructionsProperty(FrozenIt->second, Type, Value);
				FrozenTypes[Type] = true;
			}
		}
		else if (Command == QLatin1String("NOTICE"))
		{
			swInstructionsNotice Notice;

			Notice.Id = Reader.ReadInt();

			// a band or watermark replaces the page
			const QString Where = Reader.ReadToken();

			if (Where == QLatin1String("SHOW"))
			{
				Notice.Pages = static_cast<swInstructionsImagePages>(Reader.ReadName(gInstructionsImagePagesNames));

				// covers and bands only
				if (Notice.Pages == swInstructionsImagePages::None || Notice.Pages == swInstructionsImagePages::Both)
					continue;
			}
			else if (Where == QLatin1String("PAGE"))
				Notice.PageKey = Reader.ReadPageKey();
			else
				continue;

			Reader.ReadKeyword("POSITION");
			Notice.Position = Reader.ReadPoint();
			// the text file comes before the text

			QString Keyword = Reader.ReadToken();

			if (Keyword == QLatin1String("FILE"))
			{
				Notice.File = Reader.ReadToken();
				Keyword = Reader.ReadToken();
			}

			// alignment words come before the text
			if (Keyword == QLatin1String("ALIGN"))
			{
				Notice.Alignment = qBound(0, Reader.ReadInt(), 2);
				Notice.VerticalAlignment = qBound(0, Reader.ReadInt(), 2);
				Keyword = Reader.ReadToken();
			}

			if (Keyword != QLatin1String("TEXT"))
				continue;

			Notice.Text = swUnescapeInstructionsText(Reader.ReadRest());

			if (!Reader.IsValid() || Notice.Id < 1)
				continue;

			// a model notice replaces the template notice of its id
			std::vector<swInstructionsNotice>::iterator NoticeIt = std::find_if(mNotices.begin(), mNotices.end(), [&Notice](const swInstructionsNotice& Other)
			{
				return Other.Id == Notice.Id;
			});

			if (NoticeIt != mNotices.end())
				*NoticeIt = Notice;
			else
				mNotices.push_back(Notice);

			mNextNoticeId = qMax(mNextNoticeId, Notice.Id + 1);
		}
		else if (Command == QLatin1String("POINTER"))
		{
			swInstructionsPointer Pointer;

			Pointer.Id = Reader.ReadInt();
			Reader.ReadKeyword("PAGE");
			Pointer.PageKey = Reader.ReadPageKey();
			Reader.ReadKeyword("SOURCE");
			Pointer.Source = Reader.ReadElementKey();
			Reader.ReadKeyword("ANCHOR");
			Pointer.Anchor = Reader.ReadChoice(gInstructionsAutoName, SW_POINTER_ANCHOR_AUTO);
			Reader.ReadKeyword("STYLE");
			const QString Style = Reader.ReadToken();
			Reader.ReadKeyword("TIP");
			const QString Tip = Reader.ReadToken();
			Reader.ReadKeyword("TARGET");
			Pointer.TargetType = static_cast<swInstructionsPointerTarget>(Reader.ReadName(gInstructionsTargetNames));
			Pointer.TargetElement = Reader.ReadElementKey();
			Reader.ReadKeyword("PART");
			Pointer.TargetPart = Reader.ReadInt();
			Reader.ReadKeyword("POSITION");
			Pointer.TipPosition = Reader.ReadPoint();

			// older files have no tip spot or bend

			swInstructionsCommandReader RestReader(Reader.ReadRest(), mProject);
			bool RestValid = true;
			QString Keyword = RestReader.ReadToken();

			while (RestValid && !Keyword.isEmpty())
			{
				if (Keyword == QLatin1String("AT"))
					Pointer.TargetAnchor = RestReader.ReadPoint();
				else if (Keyword == QLatin1String("BEND"))
					Pointer.Bend = RestReader.ReadNumber();
				else if (Keyword == QLatin1String("REACH"))
					Pointer.Reach = qMax(0.0, RestReader.ReadNumber());
				else
					RestValid = false;

				RestValid = RestValid && RestReader.IsValid();
				Keyword = RestReader.ReadToken();
			}

			const bool AnchorValid = Pointer.TargetAnchor == QPointF(-1.0, -1.0) || (Pointer.TargetAnchor.x() >= 0.0 && Pointer.TargetAnchor.x() <= 1.0 && Pointer.TargetAnchor.y() >= 0.0 && Pointer.TargetAnchor.y() <= 1.0);

			if (!RestValid || !AnchorValid || Pointer.Bend < 0.0 || Pointer.Bend > 1.0)
				continue;

			Pointer.Style = Style == gInstructionsDefaultName ? -1 : swFindInstructionsName(gInstructionsPointerStyleNames, Style);
			Pointer.Tip = Tip == gInstructionsDefaultName ? -1 : swFindInstructionsName(gInstructionsPointerTipNames, Tip);

			if (!Reader.IsValid() || Pointer.Id < 1 || FindPointer(Pointer.Id) || Pointer.Anchor < SW_POINTER_ANCHOR_AUTO || Pointer.Anchor >= SW_POINTER_ANCHOR_COUNT)
				continue;

			mPointers.push_back(Pointer);
			mNextPointerId = qMax(mNextPointerId, Pointer.Id + 1);
		}
		else if (Command == QLatin1String("IMAGE"))
		{
			swInstructionsImage Image;

			Image.Id = Reader.ReadInt();
			const int Pages = Reader.ReadName(gInstructionsImagePagesNames);
			Reader.ReadKeyword("PLACEMENT");
			Image.Placement = Reader.ReadInt();
			Reader.ReadKeyword("SIZE");
			Image.Size = Reader.ReadNumber();
			Reader.ReadKeyword("FILE");
			Image.File = Reader.ReadToken();

			if (!Reader.IsValid() || Image.Id < 1 || Image.Placement < 0 || Image.Placement >= SW_PLACEMENT_COUNT || Image.Size <= 0.0 || Image.Size > 500.0)
				continue;

			Image.Pages = static_cast<swInstructionsImagePages>(Pages);

			// a model image replaces the template image of its id
			std::vector<swInstructionsImage>::iterator ImageIt = std::find_if(mImages.begin(), mImages.end(), [&Image](const swInstructionsImage& Other)
			{
				return Other.Id == Image.Id;
			});

			if (ImageIt != mImages.end())
				*ImageIt = Image;
			else
				mImages.push_back(Image);

			mNextImageId = qMax(mNextImageId, Image.Id + 1);
		}
	}

	// files without placements predate the cover cells

	if (ModelLines && !CommandLines.isEmpty())
	{
		const swInstructionsProperties BuiltInProperties = CreateDefaultProperties();
		swSetOldCoverPlacements(mStepProperties, ReadTypes, BuiltInProperties);

		for (const std::pair<const swInstructionsPageKey, std::vector<bool>>& Frozen : FrozenReadTypes)
			swSetOldCoverPlacements(mFrozenPages[Frozen.first], Frozen.second, BuiltInProperties);
	}

	// older files share the step settings
	swCopyBillOfMaterialsProperties(mStepProperties, ReadTypes);

	for (const std::pair<const swInstructionsPageKey, std::vector<bool>>& Frozen : FrozenReadTypes)
		swCopyBillOfMaterialsProperties(mFrozenPages[Frozen.first], Frozen.second);
}

// use to write an image line
static QString swWriteInstructionsImage(const swInstructionsImage& Image, const QString& File)
{
	return QStringLiteral("IMAGE %1 ").arg(Image.Id) + gInstructionsImagePagesNames[static_cast<int>(Image.Pages)] + QStringLiteral(" PLACEMENT %1 SIZE %2 FILE \"").arg(Image.Placement).arg(Image.Size) + QString(File).remove(QLatin1Char('"')) + QLatin1Char('"');
}

// use to write a notice line
static QString swWriteInstructionsNotice(const swInstructionsNotice& Notice)
{
	return QStringLiteral("NOTICE %1 ").arg(Notice.Id) + (Notice.Pages == swInstructionsImagePages::None ? QLatin1String("PAGE ") + swWriteInstructionsPageKey(Notice.PageKey) : QLatin1String("SHOW ") + gInstructionsImagePagesNames[static_cast<int>(Notice.Pages)]) + QLatin1String(" POSITION ") + swWriteInstructionsPoint(Notice.Position) + (Notice.File.isEmpty() ? QString() : QLatin1String(" FILE \"") + QString(Notice.File).remove(QLatin1Char('"')) + QLatin1Char('"')) + (Notice.Alignment < 0 ? QString() : QStringLiteral(" ALIGN %1 %2").arg(Notice.Alignment).arg(qMax(0, Notice.VerticalAlignment))) + QLatin1String(" TEXT ") + swEscapeInstructionsText(Notice.Text);
}

// use to write the steps per page words
static QString swWriteInstructionsPageSettings(const swInstructionsPageSettings& Settings)
{
	QString Line = QStringLiteral(" %1 %2").arg(Settings.Rows).arg(Settings.Columns);

	if (Settings.Direction == swInstructionsDirection::Vertical)
		Line += QLatin1String(" VERTICAL");

	if (Settings.FinalModel)
		Line += QLatin1String(" FINAL_MODEL");

	return Line;
}

// use to read the document settings of a template, or of any model file
QStringList swInstructions::ReadTemplateLines(const QString& FileName)
{
	QStringList CommandLines;
	QFile File(FileName);

	if (FileName.isEmpty() || !File.open(QIODevice::ReadOnly | QIODevice::Text))
		return CommandLines;

	static const QRegularExpression MetaExpression(QStringLiteral("^0\\s+!SW\\s+(PROPERTY|STEPS_PER_PAGE|IMAGE|NOTICE)\\s+INSTRUCTION(\\s.*)?$"));
	static const QRegularExpression FileExpression(QStringLiteral("^0\\s+FILE\\s"));
	QTextStream Stream(&File);
	int FileCount = 0;

	// the first model of a multi part file
	while (!Stream.atEnd())
	{
		const QString Line = Stream.readLine().trimmed();

		if (FileExpression.match(Line).hasMatch() && ++FileCount > 1)
			break;

		const QRegularExpressionMatch Match = MetaExpression.match(Line);

		if (Match.hasMatch())
			CommandLines.append(Match.captured(1) + Match.captured(2));
	}

	return CommandLines;
}

bool swInstructions::SaveTemplate(const QString& FileName) const
{
	QFile File(FileName);

	if (!File.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
		return false;

	// the built-in defaults stay unwritten
	const swInstructionsProperties BuiltInProperties = CreateDefaultProperties();
	QStringList CommandLines;

	for (int Type = 0; Type < static_cast<int>(swInstructionsPropertyType::Count); Type++)
	{
		const QVariant& DefaultValue = BuiltInProperties[Type].Value;
		const QVariant& Value = mStepProperties[Type].Value;

		if (swIsInstructionsPropertyWritten(mStepProperties, Type, DefaultValue))
			CommandLines.append(QLatin1String("PROPERTY ") + swGetInstructionsPropertyNames().at(Type) + QLatin1Char(' ') + swWriteInstructionsValue(Value, DefaultValue));
	}

	if (!(mPageSettings == swInstructionsPageSettings()))
		CommandLines.append(QLatin1String("STEPS_PER_PAGE") + swWriteInstructionsPageSettings(mPageSettings));

	// images keep a full path to work from any model
	for (const swInstructionsImage& Image : mImages)
		if (Image.Pages != swInstructionsImagePages::None)
			CommandLines.append(swWriteInstructionsImage(Image, GetFilePath(Image.File)));

	// shortcut: step, parts list and blank page notices belong to one model, left out
	for (swInstructionsNotice Notice : mNotices)
	{
		const swInstructionsPageType PageType = std::get<0>(Notice.PageKey);

		if (Notice.Pages == swInstructionsImagePages::None)
		{
			if (PageType != swInstructionsPageType::Cover && PageType != swInstructionsPageType::BackCover)
				continue;

			Notice.Pages = PageType == swInstructionsPageType::Cover ? swInstructionsImagePages::Front : swInstructionsImagePages::Back;
		}

		if (!Notice.File.isEmpty())
			Notice.File = GetFilePath(Notice.File);

		CommandLines.append(swWriteInstructionsNotice(Notice));
	}

	QTextStream Stream(&File);
	Stream << "0 Instructions Template\n0 Name: " << QFileInfo(FileName).fileName() << "\n";

	for (const QString& CommandLine : CommandLines)
	{
		const int CommandEnd = CommandLine.indexOf(QLatin1Char(' '));
		Stream << "0 !SW " << CommandLine.left(CommandEnd) << " INSTRUCTION" << CommandLine.mid(CommandEnd) << "\n";
	}

	Stream.flush();

	return File.error() == QFileDevice::NoError;
}

bool swInstructions::ApplyTemplate(const QString& FileName)
{
	const QStringList CommandLines = ReadTemplateLines(FileName);

	if (CommandLines.isEmpty())
		return false;

	LoadCommandLines(CommandLines);
	CreatePages();

	// one undo step for the whole template
	emit UnitsChanged();
	emit PageLayoutChanged();
	emit StepSettingsChanged(nullptr, 0);

	return true;
}

QStringList swInstructions::GetCommandLines() const
{
	QStringList CommandLines;

	// paper and units go with the model
	CommandLines.append(QStringLiteral("PAGE_SETUP %1 %2 %3 %4 %5 %6 UNITS %7 %8").arg(mPageSetup.Width).arg(mPageSetup.Height).arg(mPageSetup.MarginLeft).arg(mPageSetup.MarginRight).arg(mPageSetup.MarginTop).arg(mPageSetup.MarginBottom).arg(swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_PAGE_UNITS)).arg(swGetProfileInteger(SW_PROFILE_INSTRUCTIONS_MARGIN_UNITS)));

	// write changed properties only
	for (int Type = 0; Type < static_cast<int>(swInstructionsPropertyType::Count); Type++)
	{
		const QVariant& DefaultValue = mDefaultProperties[Type].Value;
		const QVariant& Value = mStepProperties[Type].Value;

		if (swIsInstructionsPropertyWritten(mStepProperties, Type, DefaultValue))
			CommandLines.append(QLatin1String("PROPERTY ") + swGetInstructionsPropertyNames().at(Type) + QLatin1Char(' ') + swWriteInstructionsValue(Value, DefaultValue));
	}

	for (const std::pair<const swInstructionsStepKey, swInstructionsProperties>& PageProperties : mPagePropertyOverrides)
	{
		for (int Type = 0; Type < static_cast<int>(swInstructionsPropertyType::Count); Type++)
		{
			const swInstructionsProperty& Property = PageProperties.second[Type];

			if (Property.Mode != swInstructionsPropertyMode::NotSet)
				CommandLines.append(QLatin1String("PAGE_PROPERTY STEP ") + swWriteInstructionsStepKey(PageProperties.first) + QLatin1Char(' ') + swGetInstructionsPropertyNames().at(Type) + QLatin1Char(' ') + swWriteInstructionsValue(Property.Value, mDefaultProperties[Type].Value));
		}
	}

	if (!(mPageSettings == mDefaultPageSettings))
		CommandLines.append(QLatin1String("STEPS_PER_PAGE") + swWriteInstructionsPageSettings(mPageSettings));

	for (const std::pair<const swInstructionsStepKey, swInstructionsPageSettings>& PageSettings : mPageSettingsOverrides)
		CommandLines.append(QLatin1String("PAGE_STEPS_PER_PAGE STEP ") + swWriteInstructionsStepKey(PageSettings.first) + swWriteInstructionsPageSettings(PageSettings.second));

	for (const swInstructionsStepKey& BlankPage : mBlankPages)
		CommandLines.append(QLatin1String("BLANK_PAGE STEP ") + swWriteInstructionsStepKey(BlankPage));

	for (const std::pair<const swInstructionsElementKey, QPointF>& Offset : mElementOffsets)
		CommandLines.append(QLatin1String("OFFSET ") + swWriteInstructionsElementKey(Offset.first) + QLatin1Char(' ') + swWriteInstructionsPoint(Offset.second));

	for (const std::pair<const swInstructionsElementKey, QSizeF>& Size : mElementSizes)
		CommandLines.append(QLatin1String("SIZE ") + swWriteInstructionsElementKey(Size.first) + QLatin1Char(' ') + swWriteInstructionsPoint(QPointF(Size.second.width(), Size.second.height())));

	for (const std::pair<const swInstructionsElementKey, QPointF>& Position : mLockedPositions)
		CommandLines.append(QLatin1String("LOCK ") + swWriteInstructionsElementKey(Position.first) + QLatin1Char(' ') + swWriteInstructionsPoint(Position.second));

	for (const auto& [PageKey, Positions] : mLockedPages)
	{
		const QString PageKeyText = swWriteInstructionsPageKey(PageKey);
		CommandLines.append(QLatin1String("LOCK_PAGE PAGE ") + PageKeyText);

		for (const auto& [Key, Position] : Positions)
			CommandLines.append(QLatin1String("LOCK_PAGE_ELEMENT PAGE ") + PageKeyText + QLatin1Char(' ') + swWriteInstructionsElementKey(Key) + QLatin1Char(' ') + swWriteInstructionsPoint(Position));
	}

	// frozen pages write changed values only
	for (const auto& [PageKey, Properties] : mFrozenPages)
	{
		const QString PageKeyText = swWriteInstructionsPageKey(PageKey);
		CommandLines.append(QLatin1String("FREEZE_PAGE PAGE ") + PageKeyText);

		for (int Type = 0; Type < static_cast<int>(swInstructionsPropertyType::Count); Type++)
		{
			const QVariant& DefaultValue = mDefaultProperties[Type].Value;
			const QVariant& Value = Properties[Type].Value;

			if (swIsInstructionsPropertyWritten(Properties, Type, DefaultValue))
				CommandLines.append(QLatin1String("FREEZE_PAGE_PROPERTY PAGE ") + PageKeyText + QLatin1Char(' ') + swGetInstructionsPropertyNames().at(Type) + QLatin1Char(' ') + swWriteInstructionsValue(Value, DefaultValue));
		}
	}

	for (const swInstructionsNotice& Notice : mNotices)
		CommandLines.append(swWriteInstructionsNotice(Notice));

	for (const swInstructionsPointer& Pointer : mPointers)
	{
		const QString Anchor = Pointer.Anchor == SW_POINTER_ANCHOR_AUTO ? QString(gInstructionsAutoName) : QString::number(Pointer.Anchor);
		const QString Style = Pointer.Style < 0 ? QString(gInstructionsDefaultName) : QString(gInstructionsPointerStyleNames[Pointer.Style]);
		const QString Tip = Pointer.Tip < 0 ? QString(gInstructionsDefaultName) : QString(gInstructionsPointerTipNames[Pointer.Tip]);

		CommandLines.append(QStringLiteral("POINTER %1 PAGE ").arg(Pointer.Id) + swWriteInstructionsPageKey(Pointer.PageKey) + QLatin1String(" SOURCE ") + swWriteInstructionsElementKey(Pointer.Source) +
		                    QLatin1String(" ANCHOR ") + Anchor + QLatin1String(" STYLE ") + Style + QLatin1String(" TIP ") + Tip +
		                    QLatin1String(" TARGET ") + gInstructionsTargetNames[static_cast<int>(Pointer.TargetType)] + QLatin1Char(' ') + swWriteInstructionsElementKey(Pointer.TargetElement) +
		                    QStringLiteral(" PART %1 POSITION ").arg(Pointer.TargetPart) + swWriteInstructionsPoint(Pointer.TipPosition) +
		                    (Pointer.TargetType == swInstructionsPointerTarget::Element && Pointer.TargetAnchor.x() >= 0.0 ? QLatin1String(" AT ") + swWriteInstructionsPoint(Pointer.TargetAnchor) : QString()) +
		                    (Pointer.Bend != 0.5 ? QLatin1String(" BEND ") + swFormatValue(Pointer.Bend, 3) : QString()) +
		                    (Pointer.Reach >= 0.0 ? QLatin1String(" REACH ") + swFormatValue(Pointer.Reach, 2) : QString()));
	}

	for (const swInstructionsImage& Image : mImages)
		if (!Image.Template)
			CommandLines.append(swWriteInstructionsImage(Image, Image.File));

	return CommandLines;
}

void swInstructions::StoreCommandLines()
{
	swModel* MainModel = mProject ? mProject->GetMainModel() : nullptr;

	if (!MainModel)
		return;

	const QStringList CommandLines = GetCommandLines();
	const QStringList StoredLines = MainModel->GetInstructionLines();

	if (CommandLines == StoredLines)
		return;

	// quick changes to the same lines share one undo step

	std::vector<int> ChangedLines;

	if (CommandLines.size() == StoredLines.size())
	{
		for (int LineIndex = 0; LineIndex < CommandLines.size(); LineIndex++)
			if (CommandLines[LineIndex] != StoredLines[LineIndex])
				ChangedLines.push_back(LineIndex);
	}

	const bool Merge = !mUndoLines.empty() && !ChangedLines.empty() && ChangedLines == mChangedLines && mChangeTimer.isValid() && mChangeTimer.elapsed() < 1000;

	if (!Merge)
	{
		mUndoLines.push_back(StoredLines);

		if (mUndoLines.size() > 100)
			mUndoLines.erase(mUndoLines.begin());
	}

	mChangedLines = ChangedLines;
	mChangeTimer.start();
	mRedoLines.clear();

	MainModel->SetInstructionLines(CommandLines);
	mProject->MarkAsModified();

	if (gMainWindow)
		gMainWindow->UpdateTitle();

	emit HistoryChanged();
}

bool swInstructions::CanUndo() const
{
	return !mUndoLines.empty();
}

bool swInstructions::CanRedo() const
{
	return !mRedoLines.empty();
}

void swInstructions::Undo()
{
	RestoreCommandLines(mUndoLines, mRedoLines);
}

void swInstructions::Redo()
{
	RestoreCommandLines(mRedoLines, mUndoLines);
}

void swInstructions::RestoreCommandLines(std::vector<QStringList>& From, std::vector<QStringList>& To)
{
	swModel* MainModel = mProject ? mProject->GetMainModel() : nullptr;

	if (!MainModel || From.empty())
		return;

	// keep the current lines for the way back

	To.push_back(MainModel->GetInstructionLines());
	MainModel->SetInstructionLines(From.back());
	From.pop_back();
	mChangedLines.clear();

	ReloadCommandLines();

	emit StepSettingsChanged(nullptr, 0);

	mProject->MarkAsModified();

	if (gMainWindow)
		gMainWindow->UpdateTitle();

	emit HistoryChanged();
}
