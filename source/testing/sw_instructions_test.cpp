#include "sw_global.h"
#include "sw_tests.h"

static QString ReadInstructionsSource(const QString& Path)
{
	QFile File(QStringLiteral(SW_SOURCE_ROOT "/") + Path);

	if (!File.open(QIODevice::ReadOnly | QIODevice::Text))
		return QString();

	return QString::fromUtf8(File.readAll());
}

// The part count and submodel preview names must be known to the file reader and listed in the meta documentation.
void swTests::InstructionsPartCountsAreNamed()
{
	const QString FileSource = ReadInstructionsSource(QStringLiteral("source/transput/sw_instructions_file.cpp"));
	const QString Documentation = ReadInstructionsSource(QStringLiteral("resources/documentation/commands-meta.md"));
	QVERIFY(!FileSource.isEmpty());
	QVERIFY(!Documentation.isEmpty());

	const QStringList Names = { "SHOW_COVER_PART_COUNT", "SHOW_BILL_OF_MATERIALS_PART_COUNT", "SHOW_PAGE_PART_COUNT", "PART_COUNT_TEXT", "PART_COUNT_FILE", "PART_COUNT_FONT", "PART_COUNT_COLOR", "PAGE_PART_COUNT_FONT", "PAGE_PART_COUNT_COLOR", "PART_COUNT_PLACEMENT", "BILL_OF_MATERIALS_PART_COUNT_PLACEMENT", "PAGE_PART_COUNT_PLACEMENT", "SHOW_SUBMODEL_PREVIEW", "SUBMODEL_PREVIEW_MODEL_SIZE", "SUBMODEL_PREVIEW_PLACEMENT", "COVER_PART_COUNT", "BILL_OF_MATERIALS_PART_COUNT", "PAGE_PART_COUNT", "SUBMODEL_PREVIEW", "SUBMODEL_PREVIEW_PART_COUNT_PLACEMENT", "PARTS_LIST_QUANTITY_PLACEMENT", "SUBMODEL_PREVIEW_PART_COUNT", "PARTS_LIST_QUANTITY", "SHOW_BACK_COVER", "BACK_COVER", "IMAGE", "USE_DOCUMENT_FONT", "USE_DOCUMENT_FONT_SIZE", "DOCUMENT_FONT" };
	QStringList Missing;

	for (const QString& Name : Names)
		if (!FileSource.contains(QLatin1Char('"') + Name + QLatin1Char('"')) || !Documentation.contains(Name))
			Missing.append(Name);

	QVERIFY2(Missing.isEmpty(), qPrintable(Missing.join(QLatin1String(", "))));
}

// Every part count and the submodel preview start off, the count text is the translatable default and the preview sits on light grey.
void swTests::InstructionsPartCountsDefaultOff()
{
	const QString Source = ReadInstructionsSource(QStringLiteral("source/transput/sw_instructions.cpp"));
	QVERIFY(!Source.isEmpty());

	const QStringList Toggles = { "ShowCoverPartCount", "ShowBillOfMaterialsPartCount", "ShowPagePartCount", "ShowSubmodelPreview" };

	for (const QString& Toggle : Toggles)
		QVERIFY2(Source.contains(QStringLiteral("swInstructionsPropertyType::%1)].Value = false;").arg(Toggle)), qPrintable(Toggle));

	QVERIFY(Source.contains(QStringLiteral("translate(\"swInstructionsDialog\", \"%1 PIECES\")")));
	QVERIFY(Source.contains(QStringLiteral("SubmodelPreviewBackgroundColor)].Value = SW_RGBA(221, 221, 221, 255)")));
}

// Saves a model with part count properties, set and default, with the StudWorx binary. The set ones must come back unchanged.
// SW_TEST_BINARY is the studworx program. The model holds 1 brick and a 2 brick submodel used twice, 5 parts for the cover count.
void swTests::InstructionsPartCountsRoundTrip()
{
	const QProcessEnvironment Environment = QProcessEnvironment::systemEnvironment();
	const QString Binary = Environment.value("SW_TEST_BINARY");

	if (Binary.isEmpty())
		QSKIP("SW_TEST_BINARY is not set");

	QTemporaryDir Folder;
	QVERIFY(Folder.isValid());
	QVERIFY(QDir(Folder.path()).mkpath("library/parts"));

	QFile Part(Folder.filePath("library/parts/3001.dat"));
	QVERIFY(Part.open(QIODevice::WriteOnly));
	Part.write("0 Brick  2 x  4\r\n0 Name: 3001.dat\r\n4 16 -40 0 -20 40 0 -20 40 0 20 -40 0 20\r\n");
	Part.close();

	QFile Model(Folder.filePath("model.mpd"));
	QVERIFY(Model.open(QIODevice::WriteOnly));
	Model.write("0 FILE main.ldr\r\n0 main\r\n"
		"0 !SW PROPERTY INSTRUCTION SHOW_COVER_PART_COUNT TRUE\r\n"
		"0 !SW PROPERTY INSTRUCTION PART_COUNT_TEXT %1 PARTS\r\n"
		"0 !SW PROPERTY INSTRUCTION SHOW_SUBMODEL_PREVIEW TRUE\r\n"
		"0 !SW PROPERTY INSTRUCTION SUBMODEL_PREVIEW_PLACEMENT 6\r\n"
		"0 !SW PROPERTY INSTRUCTION SHOW_BACK_COVER FALSE\r\n"
		"0 !SW PROPERTY INSTRUCTION USE_DOCUMENT_FONT TRUE\r\n"
		"0 !SW PROPERTY INSTRUCTION SHOW_HEADER_BAND TRUE\r\n"
		"0 !SW PROPERTY INSTRUCTION BLANK_PAGE_BACKGROUND_COLOR #FF000080\r\n"
		"0 !SW PROPERTY INSTRUCTION PARTS_LIST_QUANTITY_PADDING 3\r\n"
		"0 !SW NOTICE INSTRUCTION 1 SHOW FOOTER POSITION 10 5 TEXT PAGE %page\r\n"
		"0 !SW IMAGE INSTRUCTION 2 BOTH PLACEMENT 3 SIZE 40 FILE \"logo.png\"\r\n"
		"1 16 0 0 0 1 0 0 0 1 0 0 0 1 3001.dat\r\n"
		"1 4 0 0 0 1 0 0 0 1 0 0 0 1 sub.ldr\r\n"
		"1 4 0 0 100 1 0 0 0 1 0 0 0 1 sub.ldr\r\n"
		"0 NOFILE\r\n0 FILE sub.ldr\r\n"
		"1 1 0 0 0 1 0 0 0 1 0 0 0 1 3001.dat\r\n"
		"1 1 0 0 50 1 0 0 0 1 0 0 0 1 3001.dat\r\n"
		"0 NOFILE\r\n");
	Model.close();

	QProcessEnvironment SaveEnvironment = Environment;
	SaveEnvironment.insert("XDG_CONFIG_HOME", Folder.filePath("config"));
	SaveEnvironment.insert("XDG_DATA_HOME", Folder.filePath("data"));
	SaveEnvironment.insert("XDG_CACHE_HOME", Folder.filePath("cache"));
	SaveEnvironment.insert("QT_QPA_PLATFORM", "offscreen");

	QProcess Save;
	Save.setProcessEnvironment(SaveEnvironment);
	Save.setProcessChannelMode(QProcess::MergedChannels);
	Save.start(Binary, { "--library", Folder.filePath("library"), Model.fileName(), "--flatten", Folder.filePath("saved.mpd") });
	QVERIFY(Save.waitForFinished(120000));

	QFile Saved(Folder.filePath("saved.mpd"));
	QVERIFY(Saved.open(QIODevice::ReadOnly));

	const QStringList Lines = QString::fromUtf8(Saved.readAll()).split("\r\n").filter("INSTRUCTION");

	QVERIFY(Lines.contains("0 !SW PROPERTY INSTRUCTION SHOW_COVER_PART_COUNT TRUE"));
	QVERIFY(Lines.contains("0 !SW PROPERTY INSTRUCTION PART_COUNT_TEXT %1 PARTS"));
	QVERIFY(Lines.contains("0 !SW PROPERTY INSTRUCTION SHOW_SUBMODEL_PREVIEW TRUE"));
	QVERIFY(Lines.contains("0 !SW PROPERTY INSTRUCTION SUBMODEL_PREVIEW_PLACEMENT 6"));
	QVERIFY(Lines.contains("0 !SW PROPERTY INSTRUCTION SHOW_BACK_COVER FALSE"));
	QVERIFY(Lines.contains("0 !SW PROPERTY INSTRUCTION USE_DOCUMENT_FONT TRUE"));
	QVERIFY(Lines.contains("0 !SW PROPERTY INSTRUCTION BLANK_PAGE_BACKGROUND_COLOR #FF000080"));
	QVERIFY(Lines.contains("0 !SW PROPERTY INSTRUCTION PARTS_LIST_QUANTITY_PADDING 3"));
	QVERIFY(Lines.contains("0 !SW IMAGE INSTRUCTION 2 BOTH PLACEMENT 3 SIZE 40 FILE \"logo.png\""));
	QVERIFY(Lines.contains("0 !SW PROPERTY INSTRUCTION SHOW_HEADER_BAND TRUE"));
	QVERIFY(Lines.contains("0 !SW NOTICE INSTRUCTION 1 SHOW FOOTER POSITION 10 5 TEXT PAGE %page"));
}

// Saves pointers with the newer styles, tips, tip anchor, bend, reach and bend radius. They must come back unchanged.
void swTests::InstructionsPointersRoundTrip()
{
	const QProcessEnvironment Environment = QProcessEnvironment::systemEnvironment();
	const QString Binary = Environment.value("SW_TEST_BINARY");

	if (Binary.isEmpty())
		QSKIP("SW_TEST_BINARY is not set");

	QTemporaryDir Folder;
	QVERIFY(Folder.isValid());
	QVERIFY(QDir(Folder.path()).mkpath("library/parts"));

	QFile Part(Folder.filePath("library/parts/3001.dat"));
	QVERIFY(Part.open(QIODevice::WriteOnly));
	Part.write("0 Brick  2 x  4\r\n0 Name: 3001.dat\r\n4 16 -40 0 -20 40 0 -20 40 0 20 -40 0 20\r\n");
	Part.close();

	const QStringList Pointers =
	{
		"0 !SW PROPERTY INSTRUCTION POINTER_BEND_RADIUS 6",
		"0 !SW POINTER INSTRUCTION 1 PAGE STEPS \"main.ldr\" 1 0 SOURCE STEP_NUMBER \"main.ldr\" 1 ANCHOR 3 STYLE L_SHAPE TIP HOLLOW_DOT TARGET ELEMENT PARTS_LIST \"main.ldr\" 1 PART -1 POSITION 100 50 AT 0.25 0.75",
		"0 !SW POINTER INSTRUCTION 2 PAGE STEPS \"main.ldr\" 1 0 SOURCE STEP_NUMBER \"main.ldr\" 1 ANCHOR 5 STYLE Z_SHAPE TIP DOT TARGET POINT NONE - 0 PART -1 POSITION 120 80 BEND 0.3",
		"0 !SW POINTER INSTRUCTION 3 PAGE STEPS \"main.ldr\" 1 0 SOURCE STEP_NUMBER \"main.ldr\" 1 ANCHOR 1 STYLE U_SHAPE TIP ARROW TARGET POINT NONE - 0 PART -1 POSITION 140 20 REACH 42.5"
	};

	QFile Model(Folder.filePath("model.mpd"));
	QVERIFY(Model.open(QIODevice::WriteOnly));
	Model.write("0 FILE main.ldr\r\n0 main\r\n");
	Model.write((Pointers.join("\r\n") + "\r\n").toUtf8());
	Model.write("1 16 0 0 0 1 0 0 0 1 0 0 0 1 3001.dat\r\n0 NOFILE\r\n");
	Model.close();

	QProcessEnvironment SaveEnvironment = Environment;
	SaveEnvironment.insert("XDG_CONFIG_HOME", Folder.filePath("config"));
	SaveEnvironment.insert("XDG_DATA_HOME", Folder.filePath("data"));
	SaveEnvironment.insert("XDG_CACHE_HOME", Folder.filePath("cache"));
	SaveEnvironment.insert("QT_QPA_PLATFORM", "offscreen");

	QProcess Save;
	Save.setProcessEnvironment(SaveEnvironment);
	Save.setProcessChannelMode(QProcess::MergedChannels);
	Save.start(Binary, { "--library", Folder.filePath("library"), Model.fileName(), "--flatten", Folder.filePath("saved.mpd") });
	QVERIFY(Save.waitForFinished(120000));

	QFile Saved(Folder.filePath("saved.mpd"));
	QVERIFY(Saved.open(QIODevice::ReadOnly));

	const QStringList Lines = QString::fromUtf8(Saved.readAll()).split("\r\n").filter("INSTRUCTION");

	for (const QString& Pointer : Pointers)
		QVERIFY2(Lines.contains(Pointer), qPrintable(Pointer + "\nsaved:\n" + Lines.join("\n")));
}

// The band and watermark names must be known to the file reader and listed in the meta documentation.
void swTests::InstructionsBandsAreNamed()
{
	const QString FileSource = ReadInstructionsSource(QStringLiteral("source/transput/sw_instructions_file.cpp"));
	const QString Documentation = ReadInstructionsSource(QStringLiteral("resources/documentation/commands-meta.md"));
	QVERIFY(!FileSource.isEmpty());
	QVERIFY(!Documentation.isEmpty());

	const QStringList Names = { "LOCK_HOME_VIEW", "SHOW_HEADER_BAND", "SHOW_FOOTER_BAND", "SHOW_LEFT_BAND", "SHOW_RIGHT_BAND", "HEADER_BAND_SIZE", "FOOTER_BAND_SIZE", "LEFT_BAND_SIZE", "RIGHT_BAND_SIZE", "HEADER_BAND_COLOR", "FOOTER_BAND_COLOR", "LEFT_BAND_COLOR", "RIGHT_BAND_COLOR", "HEADER_BAND_RESERVE", "FOOTER_BAND_RESERVE", "LEFT_BAND_RESERVE", "RIGHT_BAND_RESERVE", "SHOW_WATERMARK", "WATERMARK_OPACITY", "HEADER", "FOOTER", "WATERMARK", "FINAL_MODEL_PADDING", "FINAL_MODEL_CORNER_RADIUS", "FINAL_MODEL_BACKGROUND_COLOR", "STEP_MARGIN", "STEP_PADDING", "STEP_BORDER_WIDTH", "STEP_BORDER_COLOR", "STEP_CORNER_RADIUS", "HEADER_BAND_MARGIN", "HEADER_BAND_PADDING", "HEADER_BAND_BORDER_WIDTH", "HEADER_BAND_BORDER_COLOR", "HEADER_BAND_CORNER_RADIUS", "FOOTER_BAND_MARGIN", "FOOTER_BAND_PADDING", "FOOTER_BAND_BORDER_WIDTH", "FOOTER_BAND_BORDER_COLOR", "FOOTER_BAND_CORNER_RADIUS", "LEFT_BAND_MARGIN", "LEFT_BAND_PADDING", "LEFT_BAND_BORDER_WIDTH", "LEFT_BAND_BORDER_COLOR", "LEFT_BAND_CORNER_RADIUS", "RIGHT_BAND_MARGIN", "RIGHT_BAND_PADDING", "RIGHT_BAND_BORDER_WIDTH", "RIGHT_BAND_BORDER_COLOR", "RIGHT_BAND_CORNER_RADIUS", "WATERMARK_MARGIN", "WATERMARK_PADDING", "WATERMARK_BORDER_WIDTH", "WATERMARK_BORDER_COLOR", "WATERMARK_CORNER_RADIUS", "WATERMARK_BACKGROUND_COLOR" };
	QStringList Missing;

	for (const QString& Name : Names)
		if (!FileSource.contains(QLatin1Char('"') + Name + QLatin1Char('"')) || !Documentation.contains(Name))
			Missing.append(Name);

	QVERIFY2(Missing.isEmpty(), qPrintable(Missing.join(QLatin1String(", "))));
}
