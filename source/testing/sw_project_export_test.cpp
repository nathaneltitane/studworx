#include "sw_tests.h"
#include <zlib.h>

void swTests::KMZKeepsVisibleSurfaces_data()
{
	QTest::addColumn<bool>("AllParts");
	QTest::addColumn<QString>("ArchiveName");
	QTest::addColumn<QString>("ModelName");
	QTest::newRow("visible-surfaces") << false << "model.kmz" << "model.dae";
	QTest::newRow("all-parts") << true << "spaced model.kmz" << "spaced_model.dae";
}

// Exports a red box around a hidden blue box to KMZ with the StudWorx binary. The archive must hold
// the kml index and the COLLADA model named after the archive with spaces as underscores, both with
// fixed dates, visible surfaces only must drop the blue box
// and merge the red faces into one mesh, and --kmz-geometry all must keep both. SW_TEST_BINARY is the
// studworx program, and SW_TEST_NATIVE_DISPLAY=1 renders on the running display instead of offscreen.
void swTests::KMZKeepsVisibleSurfaces()
{
	const QProcessEnvironment Environment = QProcessEnvironment::systemEnvironment();
	const QString Binary = Environment.value("SW_TEST_BINARY");

	if (Binary.isEmpty())
		QSKIP("SW_TEST_BINARY is not set");

	QFETCH(bool, AllParts);
	QFETCH(QString, ArchiveName);
	QFETCH(QString, ModelName);

	QTemporaryDir Folder;
	QVERIFY(Folder.isValid());

	const auto WriteBox = [](QFile& File, int ColorIndex, int HalfSize)
	{
		const int Faces[6][4] = { { 0, 1, 3, 2 }, { 4, 6, 7, 5 }, { 0, 4, 5, 1 }, { 2, 3, 7, 6 }, { 0, 2, 6, 4 }, { 1, 5, 7, 3 } };

		for (const auto& Face : Faces)
		{
			QString Line = QString("4 %1").arg(ColorIndex);

			for (const int Corner : Face)
				Line += QString(" %1 %2 %3").arg(Corner & 4 ? HalfSize : -HalfSize).arg(Corner & 2 ? HalfSize : -HalfSize).arg(Corner & 1 ? HalfSize : -HalfSize);

			File.write(Line.toLatin1() + "\r\n");
		}
	};

	QFile Model(Folder.filePath("model.ldr"));
	QVERIFY(Model.open(QIODevice::WriteOnly));
	Model.write("0 Test\r\n");
	WriteBox(Model, 4, 20);
	WriteBox(Model, 1, 5);
	Model.close();

	QProcessEnvironment ExportEnvironment = Environment;
	ExportEnvironment.insert("XDG_CONFIG_HOME", Folder.filePath("config"));
	ExportEnvironment.insert("XDG_DATA_HOME", Folder.filePath("data"));
	ExportEnvironment.insert("XDG_CACHE_HOME", Folder.filePath("cache"));

	// a landscape display at a small size keeps the export quick
	QVERIFY(QDir(Folder.path()).mkpath("config/StudWorx"));
	QFile Settings(Folder.filePath("config/StudWorx/StudWorx.conf"));
	QVERIFY(Settings.open(QIODevice::WriteOnly));
	Settings.write("[HTML]\nDisplay=1\nImageWidth=320\nImageHeight=180\n");
	Settings.close();

	if (Environment.value("SW_TEST_NATIVE_DISPLAY") != QLatin1String("1"))
		ExportEnvironment.insert("QT_QPA_PLATFORM", "offscreen");

	const QString FileName = Folder.filePath(ArchiveName);
	const QStringList Arguments = { Model.fileName(), "--export-kmz", FileName, "--kmz-geometry", AllParts ? "all" : "visible" };

	QProcess Export;
	Export.setProcessEnvironment(ExportEnvironment);
	Export.setProcessChannelMode(QProcess::MergedChannels);
	Export.start(Binary, Arguments);
	QVERIFY2(Export.waitForFinished(120000), "the kmz was not saved");
	QVERIFY2(Export.exitCode() == 0, Export.readAll().constData());

	QFile Archive(FileName);
	QVERIFY(Archive.open(QIODevice::ReadOnly));
	const QByteArray Data = Archive.readAll();

	// local file headers in archive order
	QStringList Names;
	QMap<QString, QByteArray> Files;
	int Offset = 0;

	const auto ReadU16 = [&Data](int Position) { return qFromLittleEndian<quint16>(Data.constData() + Position); };
	const auto ReadU32 = [&Data](int Position) { return qFromLittleEndian<quint32>(Data.constData() + Position); };

	while (Offset + 30 <= Data.size() && ReadU32(Offset) == 0x04034b50)
	{
		QCOMPARE(ReadU16(Offset + 8), quint16(Z_DEFLATED));
		QCOMPARE(ReadU32(Offset + 10), quint32(((2000 - 1980) << 25) | (1 << 21) | (1 << 16)));

		const quint32 CompressedSize = ReadU32(Offset + 18);
		const quint32 Size = ReadU32(Offset + 22);
		const int NameLength = ReadU16(Offset + 26);
		const int DataOffset = Offset + 30 + NameLength + ReadU16(Offset + 28);
		const QString Name = QString::fromLatin1(Data.mid(Offset + 30, NameLength));

		QByteArray Contents(static_cast<int>(Size), Qt::Uninitialized);
		z_stream Stream = {};
		QVERIFY(inflateInit2(&Stream, -MAX_WBITS) == Z_OK);
		Stream.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(Data.constData() + DataOffset));
		Stream.avail_in = CompressedSize;
		Stream.next_out = reinterpret_cast<Bytef*>(Contents.data());
		Stream.avail_out = Size;
		const int Result = inflate(&Stream, Z_FINISH);
		inflateEnd(&Stream);
		QCOMPARE(Result, Z_STREAM_END);
		QCOMPARE(static_cast<quint32>(crc32(0, reinterpret_cast<const Bytef*>(Contents.constData()), Size)), ReadU32(Offset + 14));

		Names << Name;
		Files[Name] = Contents;
		Offset = DataOffset + static_cast<int>(CompressedSize);
	}

	QCOMPARE(Names, QStringList({ "doc.kml", ModelName }));
	QCOMPARE(Files["doc.kml"], QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<kml><Placemark><Model><Link><href>") + ModelName.toUtf8() + QByteArray("</href></Link></Model></Placemark></kml>\n"));

	const QString COLLADA = QString::fromUtf8(Files[ModelName]);
	QVERIFY(COLLADA.contains("<created>2000-01-01T00:00:00</created>"));
	QVERIFY(COLLADA.contains("<modified>2000-01-01T00:00:00</modified>"));
	QVERIFY(COLLADA.contains("target=\"#Color_Red-material\""));

	if (AllParts)
	{
		QCOMPARE(COLLADA.count("<triangles count=\"12\""), 2);
		QVERIFY(COLLADA.contains("target=\"#Color_Blue-material\""));
	}
	else
	{
		QCOMPARE(COLLADA.count("<geometry "), 1);
		QVERIFY(COLLADA.contains("<triangles count=\"12\" material=\"surface\">"));
		QVERIFY(!COLLADA.contains("target=\"#Color_Blue-material\""));
	}
}

// Exports two parts to BrickLink with a fresh configuration and no parts library. With no mapping
// file set the bundled mapping must translate the parts and colors to their current BrickLink numbers.
void swTests::BrickLinkUsesBundledMapping()
{
	const QProcessEnvironment Environment = QProcessEnvironment::systemEnvironment();
	const QString Binary = Environment.value("SW_TEST_BINARY");

	if (Binary.isEmpty())
		QSKIP("SW_TEST_BINARY is not set");

	QTemporaryDir Folder;
	QVERIFY(Folder.isValid());

	QFile Model(Folder.filePath("model.ldr"));
	QVERIFY(Model.open(QIODevice::WriteOnly));
	Model.write("0 Test\r\n1 4 0 0 0 1 0 0 0 1 0 0 0 1 2431pzu.dat\r\n1 72 40 0 0 1 0 0 0 1 0 0 0 1 92709c01.dat\r\n");
	Model.close();

	QProcessEnvironment ExportEnvironment = Environment;
	ExportEnvironment.insert("XDG_CONFIG_HOME", Folder.filePath("config"));
	ExportEnvironment.insert("XDG_DATA_HOME", Folder.filePath("data"));
	ExportEnvironment.insert("XDG_CACHE_HOME", Folder.filePath("cache"));
	ExportEnvironment.insert("QT_QPA_PLATFORM", "offscreen");

	const QString FileName = Folder.filePath("model.xml");

	QProcess Export;
	Export.setProcessEnvironment(ExportEnvironment);
	Export.setProcessChannelMode(QProcess::MergedChannels);
	Export.start(Binary, { Model.fileName(), "--export-bricklink", FileName });
	QVERIFY2(Export.waitForFinished(120000), "the wanted list was not saved");
	QVERIFY2(Export.exitCode() == 0, Export.readAll().constData());

	QFile WantedList(FileName);
	QVERIFY(WantedList.open(QIODevice::ReadOnly));
	const QByteArray Data = WantedList.readAll();

	QVERIFY2(Data.contains("<ITEMID>2431pb0754</ITEMID>\n    <COLOR>5</COLOR>"), Data.constData());
	QVERIFY2(Data.contains("<ITEMID>92710c01</ITEMID>\n    <COLOR>85</COLOR>"), Data.constData());
}

// A part whose description reads ~Moved to lists as the part it moved to, merged with that part, in
// the BrickLink and CSV exports. A folder library holds the alias and its target.
void swTests::ExportsResolveMovedParts()
{
	const QProcessEnvironment Environment = QProcessEnvironment::systemEnvironment();
	const QString Binary = Environment.value("SW_TEST_BINARY");

	if (Binary.isEmpty())
		QSKIP("SW_TEST_BINARY is not set");

	QTemporaryDir Folder;
	QVERIFY(Folder.isValid());
	QVERIFY(QDir(Folder.path()).mkpath("library/parts"));

	const auto WriteFile = [&Folder](const QString& Name, const QByteArray& Text)
	{
		QFile File(Folder.filePath(Name));

		if (!File.open(QIODevice::WriteOnly))
			return false;

		return File.write(Text) == Text.size();
	};

	QVERIFY(WriteFile("library/parts/3001.dat", "0 Brick  2 x  4\r\n0 Name: 3001.dat\r\n4 16 -40 0 -20 40 0 -20 40 0 20 -40 0 20\r\n"));
	QVERIFY(WriteFile("library/parts/3001old.dat", "0 ~Moved to 3001\r\n0 Name: 3001old.dat\r\n1 16 0 0 0 1 0 0 0 1 0 0 0 1 3001.dat\r\n"));
	QVERIFY(WriteFile("model.ldr", "0 Test\r\n1 4 0 0 0 1 0 0 0 1 0 0 0 1 3001.dat\r\n1 4 0 -24 0 1 0 0 0 1 0 0 0 1 3001old.dat\r\n"));

	QProcessEnvironment ExportEnvironment = Environment;
	ExportEnvironment.insert("XDG_CONFIG_HOME", Folder.filePath("config"));
	ExportEnvironment.insert("XDG_DATA_HOME", Folder.filePath("data"));
	ExportEnvironment.insert("XDG_CACHE_HOME", Folder.filePath("cache"));
	ExportEnvironment.insert("QT_QPA_PLATFORM", "offscreen");

	const QString WantedListName = Folder.filePath("model.xml");
	const QString CSVName = Folder.filePath("model.csv");

	QProcess Export;
	Export.setProcessEnvironment(ExportEnvironment);
	Export.setProcessChannelMode(QProcess::MergedChannels);
	Export.start(Binary, { "--library", Folder.filePath("library"), Folder.filePath("model.ldr"), "--export-bricklink", WantedListName, "--export-csv", CSVName });
	QVERIFY2(Export.waitForFinished(120000), "the exports were not saved");
	QVERIFY2(Export.exitCode() == 0, Export.readAll().constData());

	QFile WantedList(WantedListName);
	QVERIFY(WantedList.open(QIODevice::ReadOnly));
	const QByteArray WantedListData = WantedList.readAll();

	QVERIFY2(WantedListData.contains("<ITEMID>3001</ITEMID>\n    <COLOR>5</COLOR>\n    <MINQTY>2</MINQTY>"), WantedListData.constData());
	QVERIFY2(!WantedListData.contains("3001old"), WantedListData.constData());

	QFile CSV(CSVName);
	QVERIFY(CSV.open(QIODevice::ReadOnly));
	const QByteArray CSVData = CSV.readAll();

	QVERIFY2(CSVData.contains("\"Brick  2 x  4\",\"Red\",2,3001.dat,4"), CSVData.constData());
	QVERIFY2(!CSVData.contains("3001old"), CSVData.constData());
}

// Part exceptions replace the mapping and come before moved part numbers, read from comma, quoted
// and tab separated lines of the file given with --bricklink-exceptions.
void swTests::BrickLinkUsesExceptions()
{
	const QProcessEnvironment Environment = QProcessEnvironment::systemEnvironment();
	const QString Binary = Environment.value("SW_TEST_BINARY");

	if (Binary.isEmpty())
		QSKIP("SW_TEST_BINARY is not set");

	QTemporaryDir Folder;
	QVERIFY(Folder.isValid());
	QVERIFY(QDir(Folder.path()).mkpath("library/parts"));

	const auto WriteFile = [&Folder](const QString& Name, const QByteArray& Text)
	{
		QFile File(Folder.filePath(Name));

		if (!File.open(QIODevice::WriteOnly))
			return false;

		return File.write(Text) == Text.size();
	};

	QVERIFY(WriteFile("library/parts/3001.dat", "0 Brick  2 x  4\r\n0 Name: 3001.dat\r\n4 16 -40 0 -20 40 0 -20 40 0 20 -40 0 20\r\n"));
	QVERIFY(WriteFile("library/parts/3001old.dat", "0 ~Moved to 3001\r\n0 Name: 3001old.dat\r\n1 16 0 0 0 1 0 0 0 1 0 0 0 1 3001.dat\r\n"));
	QVERIFY(WriteFile("library/parts/3002.dat", "0 Brick  2 x  3\r\n0 Name: 3002.dat\r\n4 16 -30 0 -20 30 0 -20 30 0 20 -30 0 20\r\n"));
	QVERIFY(WriteFile("model.ldr", "0 Test\r\n1 4 0 0 0 1 0 0 0 1 0 0 0 1 3001.dat\r\n1 4 0 -24 0 1 0 0 0 1 0 0 0 1 3001old.dat\r\n1 4 0 -48 0 1 0 0 0 1 0 0 0 1 3002.dat\r\n"));
	QVERIFY(WriteFile("exceptions.csv", "# kept by hand\n3001OLD.dat\t3001x\n\"3002\", \"3002y\"\n"));

	QProcessEnvironment ExportEnvironment = Environment;
	ExportEnvironment.insert("XDG_CONFIG_HOME", Folder.filePath("config"));
	ExportEnvironment.insert("XDG_DATA_HOME", Folder.filePath("data"));
	ExportEnvironment.insert("XDG_CACHE_HOME", Folder.filePath("cache"));
	ExportEnvironment.insert("QT_QPA_PLATFORM", "offscreen");

	const QString FileName = Folder.filePath("model.xml");

	QProcess Export;
	Export.setProcessEnvironment(ExportEnvironment);
	Export.setProcessChannelMode(QProcess::MergedChannels);
	Export.start(Binary, { "--library", Folder.filePath("library"), Folder.filePath("model.ldr"), "--export-bricklink", FileName, "--bricklink-exceptions", Folder.filePath("exceptions.csv") });
	QVERIFY2(Export.waitForFinished(120000), "the wanted list was not saved");
	QVERIFY2(Export.exitCode() == 0, Export.readAll().constData());

	QFile WantedList(FileName);
	QVERIFY(WantedList.open(QIODevice::ReadOnly));
	const QByteArray Data = WantedList.readAll();

	QVERIFY2(Data.contains("<ITEMID>3001</ITEMID>\n    <COLOR>5</COLOR>\n    <MINQTY>1</MINQTY>"), Data.constData());
	QVERIFY2(Data.contains("<ITEMID>3001x</ITEMID>\n    <COLOR>5</COLOR>\n    <MINQTY>1</MINQTY>"), Data.constData());
	QVERIFY2(Data.contains("<ITEMID>3002y</ITEMID>"), Data.constData());
}


// Exports a one brick model to HTML with the StudWorx binary into a folder that already holds a file.
// The pages must land in a folder named after the model with index.html as the styled entry page,
// and the chosen folder must keep only its own file and that folder. A display shape must save the pages
// at twice their shown size, use the style sheet of the profile in place of the built-in one, and round
// the page cards like the step model box. SW_TEST_BINARY is the studworx program.
void swTests::HTMLExportUsesModelFolder()
{
	const QProcessEnvironment Environment = QProcessEnvironment::systemEnvironment();
	const QString Binary = Environment.value("SW_TEST_BINARY");

	if (Binary.isEmpty())
		QSKIP("SW_TEST_BINARY is not set");

	QTemporaryDir Folder;
	QVERIFY(Folder.isValid());

	QFile Model(Folder.filePath("brick.ldr"));
	QVERIFY(Model.open(QIODevice::WriteOnly));
	Model.write("0 Test\r\n0 !SW PROPERTY INSTRUCTION STEP IMAGE_CORNER_RADIUS 24\r\n1 4 0 0 0 1 0 0 0 1 0 0 0 1 3001.dat\r\n");
	Model.close();

	QDir Output(Folder.filePath("output"));
	QVERIFY(Output.mkpath(QLatin1String(".")));
	QFile Kept(Output.filePath("keep.txt"));
	QVERIFY(Kept.open(QIODevice::WriteOnly));
	Kept.close();

	QProcessEnvironment ExportEnvironment = Environment;
	ExportEnvironment.insert("XDG_CONFIG_HOME", Folder.filePath("config"));
	ExportEnvironment.insert("XDG_DATA_HOME", Folder.filePath("data"));
	ExportEnvironment.insert("XDG_CACHE_HOME", Folder.filePath("cache"));

	// a landscape display at a small size keeps the export quick
	QVERIFY(QDir(Folder.path()).mkpath("config/StudWorx"));
	QFile Settings(Folder.filePath("config/StudWorx/StudWorx.conf"));
	QVERIFY(Settings.open(QIODevice::WriteOnly));
	Settings.write("[HTML]\nDisplay=1\nImageWidth=320\nImageHeight=180\nStyleSheet=\"body {\\n\\tcolor: #400000;\\n}\\n\"\n");
	Settings.close();

	if (Environment.value("SW_TEST_NATIVE_DISPLAY") != QLatin1String("1"))
		ExportEnvironment.insert("QT_QPA_PLATFORM", "offscreen");

	QProcess Export;
	Export.setProcessEnvironment(ExportEnvironment);
	Export.setProcessChannelMode(QProcess::MergedChannels);
	Export.start(Binary, { Model.fileName(), "--export-html", Output.path() });
	QVERIFY2(Export.waitForFinished(120000), "the html was not saved");
	QVERIFY2(Export.exitCode() == 0, Export.readAll().constData());

	QCOMPARE(Output.entryList(QDir::AllEntries | QDir::NoDotAndDotDot, QDir::Name), QStringList({ "brick", "keep.txt" }));

	QFile Index(Output.filePath("brick/index.html"));
	QVERIFY(Index.open(QIODevice::ReadOnly));
	const QByteArray Page = Index.readAll();
	QVERIFY(Page.contains("<STYLE>"));
	QVERIFY2(Page.contains("\t\t\tbody {\n\t\t\t\tcolor: #400000;\n\t\t\t}\n"), Page.constData());
	QVERIFY2(!Page.contains("--radius"), Page.constData());
	QVERIFY2(Page.contains("section img {\n\t\t\t\tborder-radius: ") && !Page.contains("border-radius: 0.000%"), Page.constData());
	QVERIFY(Page.contains("<A ID=\"step-1\"></A>"));
	QVERIFY(Page.contains("WIDTH=320 HEIGHT=180"));
	QCOMPARE(QImage(Output.filePath("brick/brick-page-01.png")).size(), QSize(640, 360));

	// the built-in style sheet names the feature of each block
	QVERIFY(Settings.open(QIODevice::WriteOnly));
	Settings.write("[HTML]\nDisplay=1\nImageWidth=320\nImageHeight=180\n");
	Settings.close();

	QDir BuiltInOutput(Folder.filePath("built-in"));
	QVERIFY(BuiltInOutput.mkpath(QLatin1String(".")));
	Export.start(Binary, { Model.fileName(), "--export-html", BuiltInOutput.path() });
	QVERIFY2(Export.waitForFinished(120000), "the html was not saved");
	QVERIFY2(Export.exitCode() == 0, Export.readAll().constData());

	QFile BuiltInIndex(BuiltInOutput.filePath("brick/index.html"));
	QVERIFY(BuiltInIndex.open(QIODevice::ReadOnly));
	const QByteArray BuiltInPage = BuiltInIndex.readAll();
	QVERIFY2(BuiltInPage.contains("\t\t\t/* page navigation, on the right by default */\n\n\t\t\tnav {"), BuiltInPage.constData());
	QVERIFY2(BuiltInPage.contains("<NAV CLASS="), BuiltInPage.constData());
}

void swTests::InstructionsExportPDF()
{
	const QProcessEnvironment Environment = QProcessEnvironment::systemEnvironment();
	const QString Binary = Environment.value("SW_TEST_BINARY");

	if (Binary.isEmpty())
		QSKIP("SW_TEST_BINARY is not set");

	QTemporaryDir Folder;
	QVERIFY(Folder.isValid());

	QFile Model(Folder.filePath("brick.ldr"));
	QVERIFY(Model.open(QIODevice::WriteOnly));
	Model.write("0 Test\r\n1 4 0 0 0 1 0 0 0 1 0 0 0 1 3001.dat\r\n");
	Model.close();

	QProcessEnvironment ExportEnvironment = Environment;
	ExportEnvironment.insert("XDG_CONFIG_HOME", Folder.filePath("config"));
	ExportEnvironment.insert("XDG_DATA_HOME", Folder.filePath("data"));
	ExportEnvironment.insert("XDG_CACHE_HOME", Folder.filePath("cache"));

	if (Environment.value("SW_TEST_NATIVE_DISPLAY") != QLatin1String("1"))
		ExportEnvironment.insert("QT_QPA_PLATFORM", "offscreen");

	// without a name it lands beside the model

	QProcess Export;
	Export.setProcessEnvironment(ExportEnvironment);
	Export.setProcessChannelMode(QProcess::MergedChannels);
	Export.start(Binary, { Model.fileName(), "--export-pdf" });
	QVERIFY2(Export.waitForFinished(120000), "the pdf was not saved");
	QVERIFY2(Export.exitCode() == 0, Export.readAll().constData());

	QFile Document(Folder.filePath("brick.pdf"));
	QVERIFY(Document.open(QIODevice::ReadOnly));
	const QByteArray Content = Document.readAll();
	QVERIFY(Content.startsWith("%PDF"));

	// one sheet per instruction page

	const QRegularExpressionMatch Match = QRegularExpression("/Count\\s+(\\d+)").match(QString::fromLatin1(Content.constData(), Content.size()));
	QVERIFY2(Match.hasMatch() && Match.captured(1).toInt() > 1, Content.left(400).constData());
}

void swTests::InstructionsExportPages()
{
	const QProcessEnvironment Environment = QProcessEnvironment::systemEnvironment();
	const QString Binary = Environment.value("SW_TEST_BINARY");

	if (Binary.isEmpty())
		QSKIP("SW_TEST_BINARY is not set");

	QTemporaryDir Folder;
	QVERIFY(Folder.isValid());

	QFile Model(Folder.filePath("brick.ldr"));
	QVERIFY(Model.open(QIODevice::WriteOnly));
	Model.write("0 Test\r\n0 !SW PAGE_SETUP INSTRUCTION 480 240 16 16 16 16 UNITS 0 0\r\n1 4 0 0 0 1 0 0 0 1 0 0 0 1 3001.dat\r\n");
	Model.close();

	QProcessEnvironment ExportEnvironment = Environment;
	ExportEnvironment.insert("XDG_CONFIG_HOME", Folder.filePath("config"));
	ExportEnvironment.insert("XDG_DATA_HOME", Folder.filePath("data"));
	ExportEnvironment.insert("XDG_CACHE_HOME", Folder.filePath("cache"));

	if (Environment.value("SW_TEST_NATIVE_DISPLAY") != QLatin1String("1"))
		ExportEnvironment.insert("QT_QPA_PLATFORM", "offscreen");

	// a missing folder is made

	QProcess Export;
	Export.setProcessEnvironment(ExportEnvironment);
	Export.setProcessChannelMode(QProcess::MergedChannels);
	Export.start(Binary, { Model.fileName(), "--export-pages", Folder.filePath("pages") });
	QVERIFY2(Export.waitForFinished(120000), "the pages were not saved");
	QVERIFY2(Export.exitCode() == 0, Export.readAll().constData());

	// one image per page at the page setup size

	const QStringList Pages = QDir(Folder.filePath("pages")).entryList({ "brick-page-*.png" }, QDir::Files, QDir::Name);
	QVERIFY2(Pages.size() > 1, qPrintable(Pages.join(' ')));
	QCOMPARE(Pages.first(), QString("brick-page-01.png"));
	QCOMPARE(QImage(Folder.filePath("pages/" + Pages.first())).size(), QSize(480, 240));
}

// Exports the back cover of two models that differ only in the product ID. The Publication URL text uses %id, so the pages differ only when the code is filled.
void swTests::InstructionsFillsPublicationCodes()
{
	const QProcessEnvironment Environment = QProcessEnvironment::systemEnvironment();
	const QString Binary = Environment.value("SW_TEST_BINARY");

	if (Binary.isEmpty())
		QSKIP("SW_TEST_BINARY is not set");

	QTemporaryDir Folder;
	QVERIFY(Folder.isValid());

	QProcessEnvironment ExportEnvironment = Environment;
	ExportEnvironment.insert("XDG_CONFIG_HOME", Folder.filePath("config"));
	ExportEnvironment.insert("XDG_DATA_HOME", Folder.filePath("data"));
	ExportEnvironment.insert("XDG_CACHE_HOME", Folder.filePath("cache"));

	if (Environment.value("SW_TEST_NATIVE_DISPLAY") != QLatin1String("1"))
		ExportEnvironment.insert("QT_QPA_PLATFORM", "offscreen");

	QImage Backs[2];

	for (int Index = 0; Index < 2; Index++)
	{
		const QString Name = QString("model%1").arg(Index);
		QFile Model(Folder.filePath(Name + ".ldr"));
		QVERIFY(Model.open(QIODevice::WriteOnly));
		Model.write("0 Test\r\n0 !SW PAGE_SETUP INSTRUCTION 480 240 16 16 16 16 UNITS 0 0\r\n0 !SW PROPERTY INSTRUCTION COVER SHOW_BACK TRUE\r\n0 !SW PROPERTY INSTRUCTION COVER BACK SHOW_URL TRUE\r\n");
		Model.write("0 !SW PROPERTY INSTRUCTION COVER BACK URL_TEXT WEB // %id\r\n");
		Model.write(QString("0 !SW PRODUCT_ID MODEL SW-000%1\r\n").arg(Index + 1).toUtf8());
		Model.write("1 4 0 0 0 1 0 0 0 1 0 0 0 1 3001.dat\r\n");
		Model.close();

		QProcess Export;
		Export.setProcessEnvironment(ExportEnvironment);
		Export.setProcessChannelMode(QProcess::MergedChannels);
		Export.start(Binary, { Model.fileName(), "--export-pages", Folder.filePath(Name + "pages") });
		QVERIFY2(Export.waitForFinished(120000), "the pages were not saved");
		QVERIFY2(Export.exitCode() == 0, Export.readAll().constData());

		const QStringList Pages = QDir(Folder.filePath(Name + "pages")).entryList({ "*.png" }, QDir::Files, QDir::Name);
		QVERIFY(!Pages.isEmpty());
		Backs[Index] = QImage(Folder.filePath(Name + "pages/" + Pages.last()));
		QVERIFY(!Backs[Index].isNull());
	}

	QVERIFY2(Backs[0] != Backs[1], "the %id code was not filled");
}

// Exports three models that differ only in a publication text. One on the front cover must change the first page only, one on the back cover the last page only.
void swTests::InstructionsPublicationPagesFollowChoice()
{
	const QProcessEnvironment Environment = QProcessEnvironment::systemEnvironment();
	const QString Binary = Environment.value("SW_TEST_BINARY");

	if (Binary.isEmpty())
		QSKIP("SW_TEST_BINARY is not set");

	QTemporaryDir Folder;
	QVERIFY(Folder.isValid());

	QProcessEnvironment ExportEnvironment = Environment;
	ExportEnvironment.insert("XDG_CONFIG_HOME", Folder.filePath("config"));
	ExportEnvironment.insert("XDG_DATA_HOME", Folder.filePath("data"));
	ExportEnvironment.insert("XDG_CACHE_HOME", Folder.filePath("cache"));

	if (Environment.value("SW_TEST_NATIVE_DISPLAY") != QLatin1String("1"))
		ExportEnvironment.insert("QT_QPA_PLATFORM", "offscreen");

	// none, front product id, back version, front measurements, back weight, back comments
	const QStringList Extras =
	{
		QString(),
		"0 !SW PROPERTY INSTRUCTION COVER FRONT SHOW_PRODUCT_ID TRUE\r\n0 !SW PROPERTY INSTRUCTION COVER FRONT PRODUCT_ID_TEXT SAMPLE\r\n",
		"0 !SW PROPERTY INSTRUCTION COVER BACK SHOW_VERSION TRUE\r\n0 !SW PROPERTY INSTRUCTION COVER BACK VERSION_TEXT SAMPLE\r\n",
		"0 !SW PROPERTY INSTRUCTION COVER FRONT SHOW_MEASUREMENTS TRUE\r\n",
		"0 !SW PROPERTY INSTRUCTION COVER BACK SHOW_WEIGHT TRUE\r\n",
		"0 !SW PROPERTY INSTRUCTION COVER BACK SHOW_COMMENTS TRUE\r\n0 !SW COMMENT MODEL SAMPLE\r\n"
	};

	QImage Firsts[6];
	QImage Lasts[6];

	for (int Index = 0; Index < Extras.size(); Index++)
	{
		const QString Name = QString("model%1").arg(Index);
		QFile Model(Folder.filePath(Name + ".ldr"));
		QVERIFY(Model.open(QIODevice::WriteOnly));
		Model.write("0 Test\r\n0 !SW PAGE_SETUP INSTRUCTION 480 240 16 16 16 16 UNITS 0 0\r\n0 !SW PROPERTY INSTRUCTION COVER SHOW_BACK TRUE\r\n");
		Model.write(Extras[Index].toUtf8());
		Model.write("1 4 0 0 0 1 0 0 0 1 0 0 0 1 3001.dat\r\n");
		Model.close();

		QProcess Export;
		Export.setProcessEnvironment(ExportEnvironment);
		Export.setProcessChannelMode(QProcess::MergedChannels);
		Export.start(Binary, { Model.fileName(), "--export-pages", Folder.filePath(Name + "pages") });
		QVERIFY2(Export.waitForFinished(120000), "the pages were not saved");
		QVERIFY2(Export.exitCode() == 0, Export.readAll().constData());

		const QStringList Pages = QDir(Folder.filePath(Name + "pages")).entryList({ "*.png" }, QDir::Files, QDir::Name);
		QVERIFY(Pages.size() > 1);
		Firsts[Index] = QImage(Folder.filePath(Name + "pages/" + Pages.first()));
		Lasts[Index] = QImage(Folder.filePath(Name + "pages/" + Pages.last()));
		QVERIFY(!Firsts[Index].isNull() && !Lasts[Index].isNull());
	}

	QVERIFY2(Firsts[1] != Firsts[0] && Lasts[1] == Lasts[0], "the front text is not only on the cover");
	QVERIFY2(Firsts[2] == Firsts[0] && Lasts[2] != Lasts[0], "the back text is not only on the back cover");
	QVERIFY2(Firsts[3] != Firsts[0] && Lasts[3] == Lasts[0], "the measurements are not only on the cover");

	for (int Index = 4; Index < 6; Index++)
		QVERIFY2(Firsts[Index] == Firsts[0] && Lasts[Index] != Lasts[0], qPrintable(QString("model %1 text is not only on the back cover").arg(Index)));
}

// Exports models whose cover texts differ only in a model field or code. %comments, %size and %weight must fill in.
void swTests::InstructionsFillsSummaryCodes()
{
	const QProcessEnvironment Environment = QProcessEnvironment::systemEnvironment();
	const QString Binary = Environment.value("SW_TEST_BINARY");

	if (Binary.isEmpty())
		QSKIP("SW_TEST_BINARY is not set");

	QTemporaryDir Folder;
	QVERIFY(Folder.isValid());
	QVERIFY(QDir(Folder.path()).mkpath("library/parts"));

	QFile Brick(Folder.filePath("library/parts/3001.dat"));
	QVERIFY(Brick.open(QIODevice::WriteOnly));
	Brick.write("0 Brick  2 x  4\r\n0 Name: 3001.dat\r\n4 16 -40 0 -20 40 0 -20 40 0 20 -40 0 20\r\n");
	Brick.close();

	QProcessEnvironment ExportEnvironment = Environment;
	ExportEnvironment.insert("XDG_CONFIG_HOME", Folder.filePath("config"));
	ExportEnvironment.insert("XDG_DATA_HOME", Folder.filePath("data"));
	ExportEnvironment.insert("XDG_CACHE_HOME", Folder.filePath("cache"));

	if (Environment.value("SW_TEST_NATIVE_DISPLAY") != QLatin1String("1"))
		ExportEnvironment.insert("QT_QPA_PLATFORM", "offscreen");

	// the cover or back cover of a model with extra lines and parts
	const auto Export = [&](const QString& Name, const QString& Lines, const QString& Parts, bool Last) -> QImage
	{
		QFile Model(Folder.filePath(Name + ".ldr"));
		Model.open(QIODevice::WriteOnly);
		Model.write("0 Test\r\n0 !SW PAGE_SETUP INSTRUCTION 480 240 16 16 16 16 UNITS 0 0\r\n0 !SW PROPERTY INSTRUCTION COVER SHOW_BACK TRUE\r\n");
		Model.write((Lines + Parts).toUtf8());
		Model.close();

		QProcess Process;
		Process.setProcessEnvironment(ExportEnvironment);
		Process.setProcessChannelMode(QProcess::MergedChannels);
		Process.start(Binary, { "--library", Folder.filePath("library"), Model.fileName(), "--export-pages", Folder.filePath(Name + "pages") });
		Process.waitForFinished(120000);

		const QStringList Pages = QDir(Folder.filePath(Name + "pages")).entryList({ "*.png" }, QDir::Files, QDir::Name);

		return Pages.isEmpty() ? QImage() : QImage(Folder.filePath(Name + "pages/" + (Last ? Pages.last() : Pages.first())));
	};

	const QString Part = "1 4 0 0 0 1 0 0 0 1 0 0 0 1 3001.dat\r\n";
	const QString MovedPart = "1 4 200 0 0 1 0 0 0 1 0 0 0 1 3001.dat\r\n";

	// each code on the back cover with the model field that changes it
	const struct
	{
		QString Code;
		QString Different;
		QString Parts[2];
	} Cases[] =
	{
		{ "%comments", "0 !SW COMMENT MODEL SAMPLE\r\n", { Part, Part } },
		{ "%size", QString(), { Part, Part + MovedPart } },
		{ "%weight", QString(), { Part, Part + Part } }
	};

	for (const auto& Case : Cases)
	{
		const QString Lines = "0 !SW PROPERTY INSTRUCTION COVER BACK SHOW_URL TRUE\r\n0 !SW PROPERTY INSTRUCTION COVER BACK URL_TEXT " + Case.Code + "\r\n";
		const QImage First = Export("first", Lines, Case.Parts[0], true);
		const QImage Second = Export("second", Lines + Case.Different, Case.Parts[1], true);

		QVERIFY2(!First.isNull() && !Second.isNull(), qPrintable(Case.Code + " did not export"));
		QVERIFY2(First != Second, qPrintable(Case.Code + " was not filled"));
	}
}

// Exports models with the same element on both covers. Each cover must draw its own text.
void swTests::InstructionsCoverElementsPerCover()
{
	const QProcessEnvironment Environment = QProcessEnvironment::systemEnvironment();
	const QString Binary = Environment.value("SW_TEST_BINARY");

	if (Binary.isEmpty())
		QSKIP("SW_TEST_BINARY is not set");

	QTemporaryDir Folder;
	QVERIFY(Folder.isValid());

	QProcessEnvironment ExportEnvironment = Environment;
	ExportEnvironment.insert("XDG_CONFIG_HOME", Folder.filePath("config"));
	ExportEnvironment.insert("XDG_DATA_HOME", Folder.filePath("data"));
	ExportEnvironment.insert("XDG_CACHE_HOME", Folder.filePath("cache"));

	if (Environment.value("SW_TEST_NATIVE_DISPLAY") != QLatin1String("1"))
		ExportEnvironment.insert("QT_QPA_PLATFORM", "offscreen");

	// first and last page of a model with a title on each cover
	const auto Export = [&](const QString& Name, const QString& Front, const QString& Back, QImage Pages[2])
	{
		QFile Model(Folder.filePath(Name + ".ldr"));
		Model.open(QIODevice::WriteOnly);
		Model.write("0 Test\r\n0 !SW PAGE_SETUP INSTRUCTION 480 240 16 16 16 16 UNITS 0 0\r\n0 !SW PROPERTY INSTRUCTION COVER SHOW_BACK TRUE\r\n0 !SW PROPERTY INSTRUCTION COVER BACK SHOW_TITLE TRUE\r\n");
		Model.write(("0 !SW PROPERTY INSTRUCTION COVER FRONT TITLE_TEXT " + Front + "\r\n0 !SW PROPERTY INSTRUCTION COVER BACK TITLE_TEXT " + Back + "\r\n1 4 0 0 0 1 0 0 0 1 0 0 0 1 3001.dat\r\n").toUtf8());
		Model.close();

		QProcess Process;
		Process.setProcessEnvironment(ExportEnvironment);
		Process.setProcessChannelMode(QProcess::MergedChannels);
		Process.start(Binary, { Model.fileName(), "--export-pages", Folder.filePath(Name + "pages") });
		Process.waitForFinished(120000);

		const QStringList Files = QDir(Folder.filePath(Name + "pages")).entryList({ "*.png" }, QDir::Files, QDir::Name);

		if (!Files.isEmpty())
		{
			Pages[0] = QImage(Folder.filePath(Name + "pages/" + Files.first()));
			Pages[1] = QImage(Folder.filePath(Name + "pages/" + Files.last()));
		}
	};

	QImage Base[2], OtherBack[2], OtherFront[2];
	Export("base", "ONE", "TWO", Base);
	Export("back", "ONE", "THREE", OtherBack);
	Export("front", "FOUR", "TWO", OtherFront);

	QVERIFY(!Base[0].isNull() && !Base[1].isNull() && !OtherBack[1].isNull() && !OtherFront[0].isNull());
	QVERIFY2(Base[0] == OtherBack[0], "the back title changed the front cover");
	QVERIFY2(Base[1] != OtherBack[1], "the back title was not drawn");
	QVERIFY2(Base[0] != OtherFront[0], "the front title was not drawn");
	QVERIFY2(Base[1] == OtherFront[1], "the front title changed the back cover");
}

// Exports a cover with a placed product ID, then again with the comments shown in the same cell. The product ID must not move.
void swTests::InstructionsCoverPlacedElementStays()
{
	const QProcessEnvironment Environment = QProcessEnvironment::systemEnvironment();
	const QString Binary = Environment.value("SW_TEST_BINARY");

	if (Binary.isEmpty())
		QSKIP("SW_TEST_BINARY is not set");

	QTemporaryDir Folder;
	QVERIFY(Folder.isValid());

	QProcessEnvironment ExportEnvironment = Environment;
	ExportEnvironment.insert("XDG_CONFIG_HOME", Folder.filePath("config"));
	ExportEnvironment.insert("XDG_DATA_HOME", Folder.filePath("data"));
	ExportEnvironment.insert("XDG_CACHE_HOME", Folder.filePath("cache"));

	if (Environment.value("SW_TEST_NATIVE_DISPLAY") != QLatin1String("1"))
		ExportEnvironment.insert("QT_QPA_PLATFORM", "offscreen");

	// front cover with the placed product id
	const auto Export = [&](const QString& Name, const QString& Extra)
	{
		QFile Model(Folder.filePath(Name + ".ldr"));
		Model.open(QIODevice::WriteOnly);
		Model.write("0 Test\r\n0 !SW PAGE_SETUP INSTRUCTION 480 240 16 16 16 16 UNITS 0 0\r\n0 !SW PROPERTY INSTRUCTION COVER FRONT SHOW_TITLE FALSE\r\n0 !SW PROPERTY INSTRUCTION COVER FRONT SHOW_IMAGE FALSE\r\n");
		Model.write(("0 !SW PROPERTY INSTRUCTION COVER FRONT SHOW_PRODUCT_ID TRUE\r\n0 !SW PROPERTY INSTRUCTION COVER FRONT PRODUCT_ID_TEXT AAAA\r\n0 !SW OFFSET INSTRUCTION COVER_FRONT_PRODUCT_ID \"" + Name + ".ldr\" 0 0 -40\r\n" + Extra + "1 4 0 0 0 1 0 0 0 1 0 0 0 1 3001.dat\r\n").toUtf8());
		Model.close();

		QProcess Process;
		Process.setProcessEnvironment(ExportEnvironment);
		Process.setProcessChannelMode(QProcess::MergedChannels);
		Process.start(Binary, { Model.fileName(), "--export-pages", Folder.filePath(Name + "pages") });
		Process.waitForFinished(120000);

		const QStringList Files = QDir(Folder.filePath(Name + "pages")).entryList({ "*.png" }, QDir::Files, QDir::Name);

		return Files.isEmpty() ? QImage() : QImage(Folder.filePath(Name + "pages/" + Files.first()));
	};

	const QImage Alone = Export("alone", QString());
	const QImage Shared = Export("shared", "0 !SW PROPERTY INSTRUCTION COVER FRONT SHOW_COMMENTS TRUE\r\n0 !SW PROPERTY INSTRUCTION COVER FRONT COMMENTS_TEXT BBBB\r\n");

	QVERIFY(!Alone.isNull() && !Shared.isNull() && Alone.size() == Shared.size());
	QVERIFY2(Alone != Shared, "the comments were not drawn");

	// every product id pixel is still there
	for (int Y = 0; Y < Alone.height(); Y++)
		for (int X = 0; X < Alone.width(); X++)
			if (Alone.pixel(X, Y) != Alone.pixel(0, 0) && Shared.pixel(X, Y) != Alone.pixel(X, Y))
				QFAIL("the placed product id moved when the comments were shown");
}
