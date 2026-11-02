#include "sw_global.h"
#include "sw_bricklink.h"
#include "sw_file.h"
#include "sw_library.h"
#include "sw_application.h"
#include "sw_profile.h"
#include "sw_string.h"
#include "sw_data_part.h"
#include "sw_colors.h"
#include "sw_http.h"

// rebrickable pages and their pacing
static const QString swRebrickablePartsUrl = QStringLiteral("https://rebrickable.com/api/v3/lego/parts/?page_size=1000&inc_part_details=1&key=%1");
static const QString swRebrickableColorsUrl = QStringLiteral("https://rebrickable.com/api/v3/lego/colors/?page_size=1000&key=%1");
static constexpr int swRebrickablePageSize = 1000;
static constexpr int swRebrickablePageDelay = 1100;
static constexpr int swRebrickableRetryDelay = 3000;
static constexpr int swRebrickableRetryCount = 3;

// part header lines read for the bricklink keyword
static constexpr int swPartHeaderLineCount = 40;

// ldraw colors resolved by their parent
static const QStringList swBrickLinkInheritedColors = { QStringLiteral("16"), QStringLiteral("24") };

static const QString swBrickLinkMappingComment = QStringLiteral("LDraw to BrickLink mappings. CC-0 licensed");
static const QString swBrickLinkMappingKeyComment = QStringLiteral("Comment");
static const QString swBrickLinkMappingKeyParts = QStringLiteral("Parts");
static const QString swBrickLinkMappingKeyColors = QStringLiteral("Colors");

// wanted lists are numbers or the address of their bricklink page
static const QString swBrickLinkUploadUrl = QStringLiteral("https://www.bricklink.com/v2/wanted/upload.page");
static const QString swRebrickableApiKeyUrl = QStringLiteral("https://rebrickable.com/api/");
static const QRegularExpression swBrickLinkWantedListNumber(QStringLiteral("^\\d*$"));
static const QRegularExpression swBrickLinkWantedListUrl(QStringLiteral("^https?://"), QRegularExpression::CaseInsensitiveOption);
static const QRegularExpression swBrickLinkWantedListUrlId(QStringLiteral("[?&]wantedMoreID=(\\d+)"), QRegularExpression::CaseInsensitiveOption);

swBrickLinkOptions swGetBrickLinkOptions()
{
	swBrickLinkOptions Options;

	Options.Condition = static_cast<swBrickLinkCondition>(qBound(static_cast<int>(swBrickLinkCondition::Any), swGetProfileInteger(SW_PROFILE_BRICKLINK_CONDITION), static_cast<int>(swBrickLinkCondition::Used)));
	Options.WantedListId = swGetProfileString(SW_PROFILE_BRICKLINK_WANTED_LIST);
	Options.Notify = swGetProfileInteger(SW_PROFILE_BRICKLINK_NOTIFY);

	return Options;
}

std::optional<QString> swParseBrickLinkWantedListId(const QString& Text)
{
	const QString WantedList = Text.trimmed();

	if (swBrickLinkWantedListNumber.match(WantedList).hasMatch())
		return WantedList;

	if (!swBrickLinkWantedListUrl.match(WantedList).hasMatch())
		return std::nullopt;

	// an address without a list number is the main wanted list
	const QRegularExpressionMatch Match = swBrickLinkWantedListUrlId.match(WantedList);

	return Match.hasMatch() ? Match.captured(1) : QString();
}

QString swGetBrickLinkUploadUrl()
{
	return swBrickLinkUploadUrl;
}

QString swGetRebrickableApiKeyUrl()
{
	return swRebrickableApiKeyUrl;
}

QString swGetRebrickableKeyRejectedMessage()
{
	return QObject::tr("Rebrickable rejected the API key, so it was cleared. Enter a new one when asked, or under Preferences > General > Online Services.");
}

// use to drop a stored key that was turned down
void swForgetRebrickableApiKey(const QString& ApiKey)
{
	if (swGetProfileString(SW_PROFILE_REBRICKABLE_API_KEY) == ApiKey)
		swSetProfileString(SW_PROFILE_REBRICKABLE_API_KEY, QString());
}

void swSetBrickLinkOptions(const swBrickLinkOptions& Options)
{
	swSetProfileInteger(SW_PROFILE_BRICKLINK_CONDITION, static_cast<int>(Options.Condition));
	swSetProfileString(SW_PROFILE_BRICKLINK_WANTED_LIST, Options.WantedListId);
	swSetProfileInteger(SW_PROFILE_BRICKLINK_NOTIFY, Options.Notify);
}

QString swGetBrickLinkMappingPath()
{
	return swGetProfileString(SW_PROFILE_BRICKLINK_MAPPING);
}

bool swBrickLinkMappingExists()
{
	const QString MappingPath = swGetBrickLinkMappingPath();

	return !MappingPath.isEmpty() && QFileInfo(MappingPath).isFile();
}

// use to read a mapping file
static swResult<QJsonObject> swLoadBrickLinkMapping(const QString& FileName)
{
	QFile File(FileName);

	if (!File.open(QIODevice::ReadOnly))
		return swUnexpected(QObject::tr("Could not open the BrickLink mapping file '%1'.").arg(QDir::toNativeSeparators(FileName)));

	QJsonParseError ParseError;
	const QJsonDocument Document = QJsonDocument::fromJson(File.readAll(), &ParseError);

	if (ParseError.error != QJsonParseError::NoError || !Document.isObject())
		return swUnexpected(QObject::tr("'%1' is not a BrickLink mapping file: %2.").arg(QDir::toNativeSeparators(FileName), ParseError.errorString()));

	return Document.object();
}

static QString swGetBrickLinkPart(const QJsonObject& PartMapping, const QString& LDrawPart)
{
	const QJsonObject::const_iterator PartIt = PartMapping.find(LDrawPart);

	return PartIt != PartMapping.end() ? PartIt.value().toString() : LDrawPart;
}

// use to find the bricklink color, zero when unmapped
static int swGetBrickLinkColor(const QJsonObject& ColorMapping, int ColorIndex)
{
	const int ColorCode = gColorList[ColorIndex].Code;
	const QJsonObject::const_iterator ColorIt = ColorMapping.find(QString::number(ColorCode));

	if (ColorIt == ColorMapping.end())
		return 0;

	const int Color = ColorIt.value().toString().toInt();

	return Color > 0 ? Color : 0;
}

swResult<int> swExportBrickLink(const QString& SaveFileName, const swPartsList& PartsList, const swBrickLinkOptions& Options)
{
	const QString MappingPath = swGetBrickLinkMappingPath();

	if (MappingPath.isEmpty())
		return swUnexpected(QObject::tr("No BrickLink mapping file is set. Choose or update one under Preferences > General > Reference Paths."));

	const swResult<QJsonObject> Mapping = swLoadBrickLinkMapping(MappingPath);

	if (!Mapping)
		return swUnexpected(QString(Mapping.error()));

	const QJsonObject PartMapping = Mapping.value()[swBrickLinkMappingKeyParts].toObject();
	const QJsonObject ColorMapping = Mapping.value()[swBrickLinkMappingKeyColors].toObject();

	// lots keyed by bricklink part and color
	std::map<std::pair<QString, int>, int> Inventory;

	for (const auto& PartIt : PartsList)
	{
		const swPartData* PartData = PartIt.first;
		QString PartName = QString::fromLatin1(PartData->mFileName).toLower();
		const int Extension = PartName.lastIndexOf(QLatin1Char('.'));

		if (Extension > 0)
			PartName.truncate(Extension);

		const QString BrickLinkPart = swGetBrickLinkPart(PartMapping, PartName);

		for (const auto& ColorIt : PartIt.second)
			Inventory[{ BrickLinkPart, swGetBrickLinkColor(ColorMapping, ColorIt.first) }] += ColorIt.second;
	}

	swDiskFile BrickLinkFile(SaveFileName);

	if (!BrickLinkFile.Open(QIODevice::WriteOnly))
		return swUnexpected(QObject::tr("Could not open file '%1' for writing.").arg(SaveFileName));

	const auto WriteElement = [&BrickLinkFile](const char* Name, const QString& Value)
	{
		BrickLinkFile.WriteLine(QString::fromLatin1("    <%1>%2</%1>\n").arg(QLatin1String(Name), Value.toHtmlEscaped()).toUtf8().constData());
	};

	const QString Condition = Options.Condition == swBrickLinkCondition::New ? QStringLiteral("N") : Options.Condition == swBrickLinkCondition::Used ? QStringLiteral("U") : QString();
	const QString WantedListId = swParseBrickLinkWantedListId(Options.WantedListId).value_or(QString());

	BrickLinkFile.WriteLine("<INVENTORY>\n");

	for (const auto& [Lot, Count] : Inventory)
	{
		BrickLinkFile.WriteLine("  <ITEM>\n");
		WriteElement("ITEMTYPE", QStringLiteral("P"));
		WriteElement("ITEMID", Lot.first);

		if (Lot.second)
			WriteElement("COLOR", QString::number(Lot.second));

		WriteElement("MINQTY", QString::number(Count));

		if (!Condition.isEmpty())
			WriteElement("CONDITION", Condition);

		if (Options.Notify)
			WriteElement("NOTIFY", QStringLiteral("Y"));

		if (!WantedListId.isEmpty())
			WriteElement("WANTEDLISTID", WantedListId);

		BrickLinkFile.WriteLine("  </ITEM>\n");
	}

	BrickLinkFile.WriteLine("</INVENTORY>\n");

	return static_cast<int>(Inventory.size());
}

// use to read the bricklink numbers set in the part keywords
static bool swReadLibraryBrickLinkParts(QProgressDialog& ProgressDialog, std::map<QString, QString>& Parts)
{
	swPartsLibrary* Library = swGetPartsLibrary();
	const std::vector<swPartData*> LibraryParts = Library->GetParts();
	const QRegularExpression KeywordExpression(QStringLiteral("(?:^|,)\\s*bricklink\\s+([^,\\s]+)"), QRegularExpression::CaseInsensitiveOption);
	char Line[1024];

	ProgressDialog.setLabelText(QObject::tr("Reading the BrickLink numbers of the library parts..."));
	ProgressDialog.setRange(0, static_cast<int>(LibraryParts.size()));

	for (size_t PartIndex = 0; PartIndex < LibraryParts.size(); PartIndex++)
	{
		if (PartIndex % 100 == 0)
		{
			ProgressDialog.setValue(static_cast<int>(PartIndex));
			QCoreApplication::processEvents();

			if (ProgressDialog.wasCanceled())
				return false;
		}

		const swPartData* PartData = LibraryParts[PartIndex];

		if (!PartData->IsLibraryPart() || PartData->IsSubPart())
			continue;

		swResult<std::unique_ptr<swFile>> File = Library->OpenPartDataFile(Library->GetPartId(PartData).c_str());

		if (!File)
			continue;

		QString PartName = QString::fromLatin1(PartData->mFileName).toLower();
		const int Extension = PartName.lastIndexOf(QLatin1Char('.'));

		if (Extension > 0)
			PartName.truncate(Extension);

		for (int LineIndex = 0; LineIndex < swPartHeaderLineCount && File.value()->ReadLine(Line, sizeof(Line)); LineIndex++)
		{
			const QString Text = QString::fromUtf8(Line).trimmed();

			if (Text.isEmpty())
				continue;

			// the header ends at the first non comment line
			if (!Text.startsWith(QLatin1Char('0')))
				break;

			if (!Text.startsWith(QLatin1String("0 !KEYWORDS")))
				continue;

			const QRegularExpressionMatch Match = KeywordExpression.match(Text.mid(11));

			if (Match.hasMatch())
			{
				Parts[PartName] = Match.captured(1);
				break;
			}
		}
	}

	return true;
}

// use to wait in the progress dialog, false when canceled
static bool swWaitWithProgress(QProgressDialog& ProgressDialog, int Milliseconds)
{
	QEventLoop EventLoop;

	QTimer::singleShot(Milliseconds, &EventLoop, &QEventLoop::quit);
	QObject::connect(&ProgressDialog, &QProgressDialog::canceled, &EventLoop, &QEventLoop::quit);
	EventLoop.exec();

	return !ProgressDialog.wasCanceled();
}

// use to download one rebrickable page
static swResult<QJsonObject> swDownloadRebrickablePage(QProgressDialog& ProgressDialog, swHttpManager& HttpManager, const QString& Url)
{
	for (int Attempt = 0; Attempt <= swRebrickableRetryCount; Attempt++)
	{
		if (Attempt && !swWaitWithProgress(ProgressDialog, swRebrickableRetryDelay))
			return swUnexpected(QString());

		QEventLoop EventLoop;
		swHttpReply* Reply = HttpManager.DownloadFile(Url);

		QObject::connect(&HttpManager, &swHttpManager::DownloadFinished, &EventLoop, &QEventLoop::quit);
		QObject::connect(&ProgressDialog, &QProgressDialog::canceled, &EventLoop, [Reply] { Reply->abort(); });
		EventLoop.exec();

		const bool Failed = Reply->error();
		const QByteArray Data = Failed ? QByteArray() : Reply->readAll();
		int Status = 0;

#ifndef Q_OS_WIN
		Status = Reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
#endif

		Reply->deleteLater();

		if (ProgressDialog.wasCanceled())
			return swUnexpected(QString());

		// a rejected key does not improve on retry
		if (Status == 401 || Status == 403)
		{
			swForgetRebrickableApiKey(QUrlQuery(QUrl(Url)).queryItemValue(QStringLiteral("key")));

			return swUnexpected(swGetRebrickableKeyRejectedMessage());
		}

		if (Failed)
			continue;

		const QJsonDocument Document = QJsonDocument::fromJson(Data);

		if (Document.isObject())
			return Document.object();
	}

	return swUnexpected(QObject::tr("Could not download the part numbers from Rebrickable."));
}

// use to walk the rebrickable pages of a list
static swResult<void> swReadRebrickableList(QProgressDialog& ProgressDialog, swHttpManager& HttpManager, const QString& Url, const QString& Label, const std::function<void(const QJsonObject&)>& ReadResult)
{
	QString PageUrl = Url;
	int PageIndex = 0;

	ProgressDialog.setLabelText(Label);
	ProgressDialog.setRange(0, 0);

	while (!PageUrl.isEmpty())
	{
		if (PageIndex && !swWaitWithProgress(ProgressDialog, swRebrickablePageDelay))
			return swUnexpected(QString());

		const swResult<QJsonObject> Page = swDownloadRebrickablePage(ProgressDialog, HttpManager, PageUrl);

		if (!Page)
			return swUnexpected(QString(Page.error()));

		const int PageCount = (Page.value()[QLatin1String("count")].toInt() + swRebrickablePageSize - 1) / swRebrickablePageSize;

		ProgressDialog.setRange(0, qMax(PageCount, 1));
		ProgressDialog.setValue(++PageIndex);

		for (const QJsonValue Result : Page.value()[QLatin1String("results")].toArray())
			ReadResult(Result.toObject());

		PageUrl = Page.value()[QLatin1String("next")].toString();
	}

	return swResult<void>();
}

swResult<int> swUpdateBrickLinkMapping(QWidget* Parent, const QString& FileName, const QString& ApiKey)
{
	QJsonObject Mapping;

	// keep the entries already in the file
	if (QFileInfo(FileName).isFile())
	{
		swResult<QJsonObject> Existing = swLoadBrickLinkMapping(FileName);

		if (!Existing)
			return swUnexpected(QString(Existing.error()));

		Mapping = Existing.value();
	}

	std::map<QString, QString> Parts;
	std::map<QString, QString> Colors;

	const QJsonObject ExistingParts = Mapping[swBrickLinkMappingKeyParts].toObject();
	const QJsonObject ExistingColors = Mapping[swBrickLinkMappingKeyColors].toObject();

	for (QJsonObject::const_iterator PartIt = ExistingParts.begin(); PartIt != ExistingParts.end(); ++PartIt)
		Parts[PartIt.key()] = PartIt.value().toString();

	for (QJsonObject::const_iterator ColorIt = ExistingColors.begin(); ColorIt != ExistingColors.end(); ++ColorIt)
		Colors[ColorIt.key()] = ColorIt.value().toString();

	QProgressDialog ProgressDialog(QString(), QObject::tr("Cancel"), 0, 0, Parent);
	ProgressDialog.setWindowTitle(QObject::tr("Update BrickLink Mapping"));
	ProgressDialog.setWindowModality(Qt::WindowModal);
	ProgressDialog.setMinimumDuration(0);
	ProgressDialog.setAutoClose(false);
	ProgressDialog.setAutoReset(false);
	ProgressDialog.setMinimumWidth(ProgressDialog.fontMetrics().averageCharWidth() * 60);
	ProgressDialog.show();

	swHttpManager HttpManager;

	// rebrickable names its parts after ldraw unless it lists the ldraw numbers
	std::map<QString, QString> RebrickableNamed;
	std::map<QString, QString> RebrickableListed;

	const swResult<void> PartsResult = swReadRebrickableList(ProgressDialog, HttpManager, swRebrickablePartsUrl.arg(ApiKey), QObject::tr("Downloading the BrickLink part numbers from Rebrickable..."), [&RebrickableNamed, &RebrickableListed](const QJsonObject& Part)
	{
		const QJsonObject ExternalIds = Part[QLatin1String("external_ids")].toObject();
		const QJsonArray BrickLinkIds = ExternalIds[QLatin1String("BrickLink")].toArray();

		if (BrickLinkIds.isEmpty())
			return;

		const QString BrickLinkPart = BrickLinkIds.first().toString();
		const QJsonArray LDrawIds = ExternalIds[QLatin1String("LDraw")].toArray();

		if (LDrawIds.isEmpty())
			RebrickableNamed[Part[QLatin1String("part_num")].toString().toLower()] = BrickLinkPart;

		for (const QJsonValue LDrawId : LDrawIds)
			RebrickableListed[LDrawId.toString().toLower()] = BrickLinkPart;
	});

	if (!PartsResult)
		return swUnexpected(QString(PartsResult.error()));

	if (!swWaitWithProgress(ProgressDialog, swRebrickablePageDelay))
		return swUnexpected(QString());

	const swResult<void> ColorsResult = swReadRebrickableList(ProgressDialog, HttpManager, swRebrickableColorsUrl.arg(ApiKey), QObject::tr("Downloading the BrickLink colors from Rebrickable..."), [&Colors](const QJsonObject& Color)
	{
		const QJsonObject ExternalIds = Color[QLatin1String("external_ids")].toObject();
		const QJsonArray BrickLinkIds = ExternalIds[QLatin1String("BrickLink")].toObject()[QLatin1String("ext_ids")].toArray();
		const QJsonArray LDrawIds = ExternalIds[QLatin1String("LDraw")].toObject()[QLatin1String("ext_ids")].toArray();

		if (BrickLinkIds.isEmpty() || BrickLinkIds.first().toInt() <= 0)
			return;

		for (const QJsonValue LDrawId : LDrawIds)
		{
			const QString LDrawColor = QString::number(LDrawId.toInt());

			if (!swBrickLinkInheritedColors.contains(LDrawColor))
				Colors[LDrawColor] = QString::number(BrickLinkIds.first().toInt());
		}
	});

	if (!ColorsResult)
		return swUnexpected(QString(ColorsResult.error()));

	std::map<QString, QString> LibraryParts;

	if (!swReadLibraryBrickLinkParts(ProgressDialog, LibraryParts))
		return swUnexpected(QString());

	ProgressDialog.hide();

	// later sources win, the part keywords are set on the part itself
	for (const std::map<QString, QString>* Source : { &RebrickableNamed, &RebrickableListed, &LibraryParts })
		for (const auto& [LDrawPart, BrickLinkPart] : *Source)
			Parts[LDrawPart] = BrickLinkPart;

	// the colors resolved by their parent are never written
	for (const QString& Color : swBrickLinkInheritedColors)
		Colors.erase(Color);

	QJsonObject PartMapping;
	QJsonObject ColorMapping;

	// identical numbers need no entry
	for (const auto& [LDrawPart, BrickLinkPart] : Parts)
		if (!BrickLinkPart.isEmpty() && LDrawPart.compare(BrickLinkPart, Qt::CaseInsensitive))
			PartMapping.insert(LDrawPart, BrickLinkPart);

	for (const auto& [LDrawColor, BrickLinkColor] : Colors)
		ColorMapping.insert(LDrawColor, BrickLinkColor);

	if (!Mapping.contains(swBrickLinkMappingKeyComment))
		Mapping.insert(swBrickLinkMappingKeyComment, swBrickLinkMappingComment);

	Mapping.insert(swBrickLinkMappingKeyParts, PartMapping);
	Mapping.insert(swBrickLinkMappingKeyColors, ColorMapping);

	QSaveFile File(FileName);

	if (!File.open(QIODevice::WriteOnly) || File.write(QJsonDocument(Mapping).toJson(QJsonDocument::Indented)) < 0 || !File.commit())
		return swUnexpected(QObject::tr("Could not write the BrickLink mapping file '%1'.").arg(QDir::toNativeSeparators(FileName)));

	return static_cast<int>(PartMapping.size());
}
