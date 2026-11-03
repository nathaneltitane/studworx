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
// at twice their shown size. SW_TEST_BINARY is the studworx program.
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
	Model.write("0 Test\r\n1 4 0 0 0 1 0 0 0 1 0 0 0 1 3001.dat\r\n");
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
	Settings.write("[HTML]\nDisplay=1\nImageWidth=320\nImageHeight=180\n");
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
	QVERIFY(Page.contains("<A ID=\"step-1\"></A>"));
	QVERIFY(Page.contains("WIDTH=320 HEIGHT=180"));
	QCOMPARE(QImage(Output.filePath("brick/brick-page-01.png")).size(), QSize(640, 360));
}
