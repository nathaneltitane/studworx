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

	const QStringList Names = { "SHOW_COVER_PART_COUNT", "SHOW_BILL_OF_MATERIALS_PART_COUNT", "SHOW_PAGE_PART_COUNT", "PART_COUNT_TEXT", "PART_COUNT_FILE", "PART_COUNT_FONT", "PART_COUNT_COLOR", "PAGE_PART_COUNT_FONT", "PAGE_PART_COUNT_COLOR", "PART_COUNT_PLACEMENT", "BILL_OF_MATERIALS_PART_COUNT_PLACEMENT", "PAGE_PART_COUNT_PLACEMENT", "SHOW_SUBMODEL_PREVIEW", "SUBMODEL_PREVIEW_SIZE", "SUBMODEL_PREVIEW_PLACEMENT", "COVER_PART_COUNT", "BILL_OF_MATERIALS_PART_COUNT", "PAGE_PART_COUNT", "SUBMODEL_PREVIEW", "SUBMODEL_PREVIEW_PART_COUNT_PLACEMENT", "PARTS_LIST_QUANTITY_PLACEMENT", "SUBMODEL_PREVIEW_PART_COUNT", "PARTS_LIST_QUANTITY", "SHOW_BACK_COVER", "BACK_COVER", "IMAGE" };
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
	QVERIFY(Lines.contains("0 !SW IMAGE INSTRUCTION 2 BOTH PLACEMENT 3 SIZE 40 FILE \"logo.png\""));
}
