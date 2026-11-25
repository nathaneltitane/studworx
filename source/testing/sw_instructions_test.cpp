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

	const QStringList Names = { "BILL_OF_MATERIALS SHOW_PART_COUNT", "PAGE SHOW_PART_COUNT", "PART_COUNT TEXT", "PART_COUNT FILE", "PART_COUNT FONT", "PART_COUNT COLOR", "PAGE PART_COUNT_FONT", "PAGE PART_COUNT_COLOR", "BILL_OF_MATERIALS PART_COUNT_PLACEMENT", "PAGE PART_COUNT_PLACEMENT", "STEP SHOW_SUBMODEL_PREVIEW", "STEP SUBMODEL_PREVIEW_MODEL_SIZE", "STEP SUBMODEL_PREVIEW_PLACEMENT", "BILL_OF_MATERIALS_PART_COUNT", "STEP SUBMODEL_PREVIEW_PART_COUNT_PLACEMENT", "STEP PARTS_LIST_QUANTITY_PLACEMENT", "SUBMODEL_PREVIEW_PART_COUNT", "PARTS_LIST_QUANTITY", "COVER SHOW_BACK", "DOCUMENT USE_FONT", "DOCUMENT USE_FONT_SIZE", "DOCUMENT FONT" };
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

	const QStringList Toggles = { "ShowBillOfMaterialsPartCount", "ShowPagePartCount", "ShowSubmodelPreview" };

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
		"0 !SW PROPERTY INSTRUCTION COVER FRONT SHOW_PIECES TRUE\r\n"
		"0 !SW PROPERTY INSTRUCTION PART_COUNT TEXT %1 PARTS\r\n"
		"0 !SW PROPERTY INSTRUCTION STEP SHOW_SUBMODEL_PREVIEW TRUE\r\n"
		"0 !SW PROPERTY INSTRUCTION STEP SUBMODEL_PREVIEW_PLACEMENT 6\r\n"
		"0 !SW PROPERTY INSTRUCTION COVER SHOW_BACK FALSE\r\n"
		"0 !SW PROPERTY INSTRUCTION DOCUMENT USE_FONT TRUE\r\n"
		"0 !SW PROPERTY INSTRUCTION BAND HEADER SHOW TRUE\r\n"
		"0 !SW PROPERTY INSTRUCTION PAGE BLANK_BACKGROUND_COLOR #FF000080\r\n"
		"0 !SW PROPERTY INSTRUCTION STEP PARTS_LIST_QUANTITY_PADDING 3\r\n"
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

	QVERIFY(Lines.contains("0 !SW PROPERTY INSTRUCTION COVER FRONT SHOW_PIECES TRUE"));
	QVERIFY(Lines.contains("0 !SW PROPERTY INSTRUCTION PART_COUNT TEXT %1 PARTS"));
	QVERIFY(Lines.contains("0 !SW PROPERTY INSTRUCTION STEP SHOW_SUBMODEL_PREVIEW TRUE"));
	QVERIFY(Lines.contains("0 !SW PROPERTY INSTRUCTION STEP SUBMODEL_PREVIEW_PLACEMENT 6"));
	QVERIFY(Lines.contains("0 !SW PROPERTY INSTRUCTION COVER SHOW_BACK FALSE"));
	QVERIFY(Lines.contains("0 !SW PROPERTY INSTRUCTION DOCUMENT USE_FONT TRUE"));
	QVERIFY(Lines.contains("0 !SW PROPERTY INSTRUCTION PAGE BLANK_BACKGROUND_COLOR #FF000080"));
	QVERIFY(Lines.contains("0 !SW PROPERTY INSTRUCTION STEP PARTS_LIST_QUANTITY_PADDING 3"));
	QVERIFY(Lines.contains("0 !SW IMAGE INSTRUCTION 2 BOTH PLACEMENT 3 SIZE 40 FILE \"logo.png\""));
	QVERIFY(Lines.contains("0 !SW PROPERTY INSTRUCTION BAND HEADER SHOW TRUE"));
	QVERIFY(Lines.contains("0 !SW NOTICE INSTRUCTION 1 SHOW FOOTER POSITION 10 5 TEXT PAGE %page"));
}

// Resaves an older model without bill of materials values and a newer one with its own, with the StudWorx binary.
// The older one must take the step values, the newer one must keep its own, and the old final model color must follow the page.
void swTests::InstructionsBillOfMaterialsCopiesSteps()
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

	QFile Model(Folder.filePath("model.ldr"));
	QVERIFY(Model.open(QIODevice::WriteOnly));
	Model.write("0 main\r\n"
		"0 !SW PROPERTY INSTRUCTION STEP PARTS_LIST_QUANTITY_PADDING 3\r\n"
		"0 !SW PROPERTY INSTRUCTION STEP PARTS_LIST_SORT_ORDER 1\r\n"
		"0 !SW PROPERTY INSTRUCTION BILL_OF_MATERIALS SORT_ORDER 0\r\n"
		"0 !SW STEPS_PER_PAGE INSTRUCTION 1 2 FINAL_MODEL FINAL_MODEL_COLOR\r\n"
		"1 16 0 0 0 1 0 0 0 1 0 0 0 1 3001.dat\r\n");
	Model.close();

	QProcessEnvironment SaveEnvironment = Environment;
	SaveEnvironment.insert("XDG_CONFIG_HOME", Folder.filePath("config"));
	SaveEnvironment.insert("XDG_DATA_HOME", Folder.filePath("data"));
	SaveEnvironment.insert("XDG_CACHE_HOME", Folder.filePath("cache"));
	SaveEnvironment.insert("QT_QPA_PLATFORM", "offscreen");

	QProcess Save;
	Save.setProcessEnvironment(SaveEnvironment);
	Save.setProcessChannelMode(QProcess::MergedChannels);
	Save.start(Binary, { "--library", Folder.filePath("library"), Model.fileName(), "--resave", Folder.filePath("saved.ldr") });
	QVERIFY(Save.waitForFinished(120000));

	QFile Saved(Folder.filePath("saved.ldr"));
	QVERIFY2(Saved.open(QIODevice::ReadOnly), Save.readAll().constData());

	const QStringList Lines = QString::fromUtf8(Saved.readAll()).split("\r\n").filter("INSTRUCTION");

	QVERIFY(Lines.contains("0 !SW PROPERTY INSTRUCTION STEP PARTS_LIST_QUANTITY_PADDING 3"));
	QVERIFY(Lines.contains("0 !SW PROPERTY INSTRUCTION BILL_OF_MATERIALS QUANTITY_PADDING 3"));
	QVERIFY(Lines.contains("0 !SW PROPERTY INSTRUCTION STEP PARTS_LIST_SORT_ORDER 1"));
	QVERIFY(Lines.contains("0 !SW PROPERTY INSTRUCTION BILL_OF_MATERIALS SORT_ORDER 0"));

	// the old final model color follows the page
	QVERIFY(Lines.contains("0 !SW STEPS_PER_PAGE INSTRUCTION 1 2 FINAL_MODEL"));
	QVERIFY(Lines.filter("FINAL_MODEL_BACKGROUND_COLOR").isEmpty());
}

// Saves a model written before the cover cells, and one without settings. The old one keeps its cells, both write every placement.
void swTests::InstructionsLayoutIsAlwaysWritten()
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

	QProcessEnvironment SaveEnvironment = Environment;
	SaveEnvironment.insert("XDG_CONFIG_HOME", Folder.filePath("config"));
	SaveEnvironment.insert("XDG_DATA_HOME", Folder.filePath("data"));
	SaveEnvironment.insert("XDG_CACHE_HOME", Folder.filePath("cache"));
	SaveEnvironment.insert("QT_QPA_PLATFORM", "offscreen");

	// saved instruction lines of a model

	const auto Resave = [&](const QString& Name, const QByteArray& Lines)
	{
		QFile Model(Folder.filePath(Name + ".ldr"));
		Model.open(QIODevice::WriteOnly);
		Model.write("0 main\r\n" + Lines + "1 16 0 0 0 1 0 0 0 1 0 0 0 1 3001.dat\r\n");
		Model.close();

		QProcess Save;
		Save.setProcessEnvironment(SaveEnvironment);
		Save.setProcessChannelMode(QProcess::MergedChannels);
		Save.start(Binary, { "--library", Folder.filePath("library"), Model.fileName(), "--resave", Folder.filePath(Name + "saved.ldr") });
		Save.waitForFinished(120000);

		QFile Saved(Folder.filePath(Name + "saved.ldr"));

		return Saved.open(QIODevice::ReadOnly) ? QString::fromUtf8(Saved.readAll()).split("\r\n").filter("INSTRUCTION") : QStringList();
	};

	const QStringList Old = Resave("old", "0 !SW PROPERTY INSTRUCTION COVER FRONT SHOW_PRODUCT_ID TRUE\r\n0 !SW PROPERTY INSTRUCTION COVER FRONT AUTHOR_PLACEMENT 18\r\n0 !SW OFFSET INSTRUCTION COVER_FRONT_PRODUCT_ID \"old.ldr\" 0 -20 -40\r\n");
	const QStringList Fresh = Resave("fresh", QByteArray());

	QVERIFY(!Old.isEmpty() && !Fresh.isEmpty());

	// old files keep the earlier cells

	QVERIFY(Old.contains("0 !SW PROPERTY INSTRUCTION COVER FRONT PRODUCT_ID_PLACEMENT 22"));
	QVERIFY(Old.contains("0 !SW PROPERTY INSTRUCTION COVER FRONT COMMENTS_PLACEMENT 22"));
	QVERIFY(Old.contains("0 !SW PROPERTY INSTRUCTION COVER FRONT PIECES_PLACEMENT 16"));
	QVERIFY(Old.contains("0 !SW PROPERTY INSTRUCTION COVER FRONT TITLE_PLACEMENT 7"));
	QVERIFY(Old.contains("0 !SW PROPERTY INSTRUCTION COVER FRONT AUTHOR_PLACEMENT 18"));
	QVERIFY(Old.contains("0 !SW OFFSET INSTRUCTION COVER_FRONT_PRODUCT_ID - 0 -20 -40"));

	// new models use the current cells

	QVERIFY(Fresh.contains("0 !SW PROPERTY INSTRUCTION COVER FRONT TITLE_PLACEMENT 2"));
	QVERIFY(Fresh.contains("0 !SW PROPERTY INSTRUCTION COVER FRONT PRODUCT_ID_PLACEMENT 20"));

	// placements never depend on defaults

	for (const QStringList& Lines : { Old, Fresh })
	{
		QVERIFY(!Lines.filter("PROPERTY INSTRUCTION STEP PARTS_LIST_PLACEMENT ").isEmpty());
		QVERIFY(!Lines.filter("PROPERTY INSTRUCTION STEP NUMBER_ANCHOR ").isEmpty());
		QVERIFY(!Lines.filter("PROPERTY INSTRUCTION STEP NUMBER_ALIGNMENT ").isEmpty());
	}
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
		"0 !SW PROPERTY INSTRUCTION POINTER BEND_RADIUS 6",
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

// Saves a model with every element of both covers set, with the StudWorx binary. The texts and toggles must come back, and the template reader must take any property line.
void swTests::InstructionsPublicationUrlRoundTrip()
{
	const QProcessEnvironment Environment = QProcessEnvironment::systemEnvironment();
	const QString Binary = Environment.value("SW_TEST_BINARY");

	if (Binary.isEmpty())
		QSKIP("SW_TEST_BINARY is not set");

	const QString FileSource = ReadInstructionsSource(QStringLiteral("source/transput/sw_instructions_file.cpp"));
	QVERIFY(FileSource.contains(QStringLiteral("([A-Z_]+)\\\\s+INSTRUCTION")));

	QTemporaryDir Folder;
	QVERIFY(Folder.isValid());
	QVERIFY(QDir(Folder.path()).mkpath("library/parts"));

	QFile Part(Folder.filePath("library/parts/3001.dat"));
	QVERIFY(Part.open(QIODevice::WriteOnly));
	Part.write("0 Brick  2 x  4\r\n0 Name: 3001.dat\r\n4 16 -40 0 -20 40 0 -20 40 0 20 -40 0 20\r\n");
	Part.close();

	// every element of both covers shows, or hides when shown by default, and writes its text
	QStringList Properties;

	for (const QString& Cover : { "FRONT", "BACK" })
		for (const QString& Name : { "TITLE", "AUTHOR", "PRODUCT_ID", "VERSION", "PUBLICATION_DATE", "URL", "CONTACT", "COPYRIGHT", "NOTICE", "COMMENTS", "MEASUREMENTS", "WEIGHT", "PIECES" })
		{
			const bool Default = Cover == QLatin1String("FRONT") && (Name == QLatin1String("TITLE") || Name == QLatin1String("AUTHOR"));

			Properties << QString("0 !SW PROPERTY INSTRUCTION COVER %1 SHOW_%2 %3").arg(Cover, Name, Default ? "FALSE" : "TRUE") << QString("0 !SW PROPERTY INSTRUCTION COVER %1 %2_TEXT %1 // %2").arg(Cover, Name);
		}

	QFile Model(Folder.filePath("model.mpd"));
	QVERIFY(Model.open(QIODevice::WriteOnly));
	Model.write("0 FILE main.ldr\r\n0 main\r\n");
	Model.write((Properties.join("\r\n") + "\r\n").toUtf8());
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

	for (const QString& Property : Properties)
		QVERIFY2(Lines.contains(Property), qPrintable(Property + "\nsaved:\n" + Lines.join("\n")));
}

// Saves a model with every publication line and the license line, with the StudWorx binary. Each line must come back.
void swTests::ModelPublicationRoundTrip()
{
	const QProcessEnvironment Environment = QProcessEnvironment::systemEnvironment();
	const QString Binary = Environment.value("SW_TEST_BINARY");

	if (Binary.isEmpty())
		QSKIP("SW_TEST_BINARY is not set");

	QTemporaryDir Folder;
	QVERIFY(Folder.isValid());

	const QStringList Lines =
	{
		"0 !LICENSE Sample License",
		"0",
		"0 !SW PRODUCT_ID MODEL SW-0001",
		"0 !SW VERSION MODEL 1.2",
		"0 !SW PUBLICATION_DATE MODEL Spring 2026",
		"0 !SW URL MODEL HTTPS://EXAMPLE.COM/MODELS/TEST",
		"0 !SW CONTACT MODEL INFO@EXAMPLE.COM",
		"0 !SW NOTICE MODEL Sample Notice"
	};

	QFile Model(Folder.filePath("model.ldr"));
	QVERIFY(Model.open(QIODevice::WriteOnly));
	Model.write("0 Test\r\n0 Name: model.ldr\r\n0 Author: Sample Author\r\n");
	Model.write((Lines.join("\r\n") + "\r\n").toUtf8());
	Model.write("1 4 0 0 0 1 0 0 0 1 0 0 0 1 3001.dat\r\n");
	Model.close();

	QProcessEnvironment SaveEnvironment = Environment;
	SaveEnvironment.insert("XDG_CONFIG_HOME", Folder.filePath("config"));
	SaveEnvironment.insert("XDG_DATA_HOME", Folder.filePath("data"));
	SaveEnvironment.insert("XDG_CACHE_HOME", Folder.filePath("cache"));
	SaveEnvironment.insert("QT_QPA_PLATFORM", "offscreen");

	QProcess Save;
	Save.setProcessEnvironment(SaveEnvironment);
	Save.setProcessChannelMode(QProcess::MergedChannels);
	Save.start(Binary, { Model.fileName(), "--flatten", Folder.filePath("saved.mpd") });
	QVERIFY(Save.waitForFinished(120000));

	QFile Saved(Folder.filePath("saved.mpd"));
	QVERIFY(Saved.open(QIODevice::ReadOnly));

	const QStringList SavedLines = QString::fromUtf8(Saved.readAll()).split("\r\n");

	for (const QString& Line : Lines)
		QVERIFY2(SavedLines.count(Line) == 1, qPrintable(Line + "\nsaved:\n" + SavedLines.join("\n")));

	// the lone 0 sits right after the header
	QCOMPARE(SavedLines.indexOf(QStringLiteral("0")), SavedLines.indexOf(Lines.first()) + 1);
}

// Applies a template with a front cover, a back cover and a footer notice and two cover text drags, set as the default template, to a fresh model with the StudWorx binary. All must come back as saved lines, a drag saved from another model must fit this one, and the covers must draw the notices. Save Template maps cover page notices to these lines in sw_instructions_file.cpp, the dialog is not reachable here.
void swTests::InstructionsTemplateKeepsNotices()
{
	const QProcessEnvironment Environment = QProcessEnvironment::systemEnvironment();
	const QString Binary = Environment.value("SW_TEST_BINARY");

	if (Binary.isEmpty())
		QSKIP("SW_TEST_BINARY is not set");

	const QString FileSource = ReadInstructionsSource(QStringLiteral("source/transput/sw_instructions_file.cpp"));
	QVERIFY(FileSource.contains(QStringLiteral("PageType == swInstructionsPageType::Cover ? swInstructionsImagePages::Front : swInstructionsImagePages::Back")));
	QVERIFY(FileSource.contains(QStringLiteral("([A-Z_]+)\\\\s+INSTRUCTION")));

	QTemporaryDir Folder;
	QVERIFY(Folder.isValid());
	QVERIFY(QDir(Folder.path()).mkpath("config/StudWorx"));
	QVERIFY(QDir(Folder.path()).mkpath("plain/StudWorx"));

	const QStringList Notices =
	{
		"0 !SW NOTICE INSTRUCTION 1 SHOW FRONT POSITION 100 50 TEXT FRONT NOTICE",
		"0 !SW NOTICE INSTRUCTION 2 SHOW BACK POSITION 100 50 TEXT BACK NOTICE",
		"0 !SW NOTICE INSTRUCTION 3 SHOW FOOTER POSITION 10 5 TEXT FOOTER NOTICE"
	};

	QFile Template(Folder.filePath("template.ldr"));
	QVERIFY(Template.open(QIODevice::WriteOnly));
	Template.write("0 Instructions Template\n0 Name: template.ldr\n");
	Template.write((Notices.join("\n") + "\n").toUtf8());

	// drags saved for any model or for another one

	Template.write("0 !SW OFFSET INSTRUCTION COVER_FRONT_COMMENTS - 0 40 -30\n0 !SW OFFSET INSTRUCTION COVER_FRONT_PRODUCT_ID \"other.ldr\" 0 -20.5 10\n");
	Template.close();

	QFile Settings(Folder.filePath("config/StudWorx/StudWorx.conf"));
	QVERIFY(Settings.open(QIODevice::WriteOnly));
	Settings.write(QString("[Defaults]\nTemplate=%1\n").arg(Template.fileName()).toUtf8());
	Settings.close();

	QFile Model(Folder.filePath("brick.ldr"));
	QVERIFY(Model.open(QIODevice::WriteOnly));
	Model.write("0 Test\r\n0 !SW PAGE_SETUP INSTRUCTION 480 240 16 16 16 16 UNITS 0 0\r\n0 !SW PROPERTY INSTRUCTION COVER SHOW_BACK TRUE\r\n1 4 0 0 0 1 0 0 0 1 0 0 0 1 3001.dat\r\n");
	Model.close();

	QStringList ConfigFolders = { "config", "plain" };
	QImage Pages[2][2];

	for (int Run = 0; Run < 2; Run++)
	{
		QProcessEnvironment RunEnvironment = Environment;
		RunEnvironment.insert("XDG_CONFIG_HOME", Folder.filePath(ConfigFolders[Run]));
		RunEnvironment.insert("XDG_DATA_HOME", Folder.filePath("data"));
		RunEnvironment.insert("XDG_CACHE_HOME", Folder.filePath("cache"));

		if (Environment.value("SW_TEST_NATIVE_DISPLAY") != QLatin1String("1"))
			RunEnvironment.insert("QT_QPA_PLATFORM", "offscreen");

		QProcess Export;
		Export.setProcessEnvironment(RunEnvironment);
		Export.setProcessChannelMode(QProcess::MergedChannels);
		Export.start(Binary, { Model.fileName(), "--resave", Folder.filePath(QString("saved%1.ldr").arg(Run)), "--export-pages", Folder.filePath(QString("pages%1").arg(Run)) });
		QVERIFY2(Export.waitForFinished(120000), "the model was not saved");
		QVERIFY2(Export.exitCode() == 0, Export.readAll().constData());

		const QStringList Names = QDir(Folder.filePath(QString("pages%1").arg(Run))).entryList({ "*.png" }, QDir::Files, QDir::Name);
		QVERIFY(Names.size() > 1);
		Pages[Run][0] = QImage(Folder.filePath(QString("pages%1/").arg(Run) + Names.first()));
		Pages[Run][1] = QImage(Folder.filePath(QString("pages%1/").arg(Run) + Names.last()));
	}

	QFile Saved(Folder.filePath("saved0.ldr"));
	QVERIFY(Saved.open(QIODevice::ReadOnly));
	const QStringList Lines = QString::fromUtf8(Saved.readAll()).split(QRegularExpression("\r?\n"));

	for (const QString& Notice : Notices)
		QVERIFY2(Lines.contains(Notice), qPrintable(Notice + "\nsaved:\n" + Lines.join("\n")));

	for (const QString& Offset : { QStringLiteral("0 !SW OFFSET INSTRUCTION COVER_FRONT_COMMENTS - 0 40 -30"), QStringLiteral("0 !SW OFFSET INSTRUCTION COVER_FRONT_PRODUCT_ID - 0 -20.5 10") })
		QVERIFY2(Lines.contains(Offset), qPrintable(Offset + "\nsaved:\n" + Lines.join("\n")));

	QVERIFY2(Pages[0][0] != Pages[1][0], "the front cover notice was not drawn");
	QVERIFY2(Pages[0][1] != Pages[1][1], "the back cover notice was not drawn");
}

// The band and watermark names must be known to the file reader and listed in the meta documentation.
void swTests::InstructionsBandsAreNamed()
{
	const QString FileSource = ReadInstructionsSource(QStringLiteral("source/transput/sw_instructions_file.cpp"));
	const QString Documentation = ReadInstructionsSource(QStringLiteral("resources/documentation/commands-meta.md"));
	QVERIFY(!FileSource.isEmpty());
	QVERIFY(!Documentation.isEmpty());

	const QStringList Names = { "DOCUMENT LOCK_HOME_VIEW", "BAND HEADER SHOW", "BAND FOOTER SHOW", "BAND LEFT SHOW", "BAND RIGHT SHOW", "BAND HEADER SIZE", "BAND FOOTER SIZE", "BAND LEFT SIZE", "BAND RIGHT SIZE", "BAND HEADER COLOR", "BAND FOOTER COLOR", "BAND LEFT COLOR", "BAND RIGHT COLOR", "BAND HEADER RESERVE", "BAND FOOTER RESERVE", "BAND LEFT RESERVE", "BAND RIGHT RESERVE", "WATERMARK SHOW", "WATERMARK OPACITY", "WATERMARK FONT", "WATERMARK TEXT_COLOR", "WATERMARK ROTATION", "HEADER", "FOOTER", "WATERMARK", "STEP FINAL_MODEL_PADDING", "STEP FINAL_MODEL_CORNER_RADIUS", "STEP FINAL_MODEL_BACKGROUND_COLOR", "STEP AREA_MARGIN", "STEP AREA_PADDING", "STEP AREA_BORDER_WIDTH", "STEP AREA_BORDER_COLOR", "STEP AREA_CORNER_RADIUS", "BAND HEADER MARGIN", "BAND HEADER PADDING", "BAND HEADER BORDER_WIDTH", "BAND HEADER BORDER_COLOR", "BAND HEADER CORNER_RADIUS", "BAND FOOTER MARGIN", "BAND FOOTER PADDING", "BAND FOOTER BORDER_WIDTH", "BAND FOOTER BORDER_COLOR", "BAND FOOTER CORNER_RADIUS", "BAND LEFT MARGIN", "BAND LEFT PADDING", "BAND LEFT BORDER_WIDTH", "BAND LEFT BORDER_COLOR", "BAND LEFT CORNER_RADIUS", "BAND RIGHT MARGIN", "BAND RIGHT PADDING", "BAND RIGHT BORDER_WIDTH", "BAND RIGHT BORDER_COLOR", "BAND RIGHT CORNER_RADIUS", "WATERMARK MARGIN", "WATERMARK PADDING", "WATERMARK BORDER_WIDTH", "WATERMARK BORDER_COLOR", "WATERMARK CORNER_RADIUS", "WATERMARK BACKGROUND_COLOR", "BILL_OF_MATERIALS SHOW_TITLE", "BILL_OF_MATERIALS TITLE_PLACEMENT", "BILL_OF_MATERIALS FONT", "BILL_OF_MATERIALS TEXT_COLOR", "BILL_OF_MATERIALS SHOW_COUNTS", "BILL_OF_MATERIALS SORT_ORDER", "BILL_OF_MATERIALS LAYOUT", "BILL_OF_MATERIALS ALIGNMENT", "BILL_OF_MATERIALS THUMBNAIL_SIZE", "BILL_OF_MATERIALS COUNT_PLACEMENT", "BILL_OF_MATERIALS TITLE_MARGIN", "BILL_OF_MATERIALS TITLE_PADDING", "BILL_OF_MATERIALS TITLE_BORDER_WIDTH", "BILL_OF_MATERIALS TITLE_BORDER_COLOR", "BILL_OF_MATERIALS TITLE_CORNER_RADIUS", "BILL_OF_MATERIALS TITLE_BACKGROUND_COLOR", "BILL_OF_MATERIALS LIST_MARGIN", "BILL_OF_MATERIALS LIST_PADDING", "BILL_OF_MATERIALS LIST_BORDER_WIDTH", "BILL_OF_MATERIALS LIST_BORDER_COLOR", "BILL_OF_MATERIALS LIST_CORNER_RADIUS", "BILL_OF_MATERIALS LIST_BACKGROUND_COLOR", "BILL_OF_MATERIALS QUANTITY_MARGIN", "BILL_OF_MATERIALS QUANTITY_PADDING", "BILL_OF_MATERIALS QUANTITY_BORDER_WIDTH", "BILL_OF_MATERIALS QUANTITY_BORDER_COLOR", "BILL_OF_MATERIALS QUANTITY_CORNER_RADIUS", "BILL_OF_MATERIALS QUANTITY_BACKGROUND_COLOR", "BILL_OF_MATERIALS PART_COUNT_MARGIN", "BILL_OF_MATERIALS PART_COUNT_PADDING", "BILL_OF_MATERIALS PART_COUNT_BORDER_WIDTH", "BILL_OF_MATERIALS PART_COUNT_BORDER_COLOR", "BILL_OF_MATERIALS PART_COUNT_CORNER_RADIUS", "BILL_OF_MATERIALS PART_COUNT_BACKGROUND_COLOR" };
	QStringList Missing;

	for (const QString& Name : Names)
		if (!FileSource.contains(QLatin1Char('"') + Name + QLatin1Char('"')) || !Documentation.contains(Name))
			Missing.append(Name);

	QVERIFY2(Missing.isEmpty(), qPrintable(Missing.join(QLatin1String(", "))));
}
