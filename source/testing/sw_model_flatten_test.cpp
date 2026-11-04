#include "sw_tests.h"

// Flattens a model holding a submodel inside a moved submodel with --flatten. The main model must
// keep only part lines, each at its world position and rotation, with color 16 parts taking the
// color of the submodel above them.
void swTests::FlattenReplacesNestedSubModels()
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
	QVERIFY(WriteFile("model.mpd",
		"0 FILE main.ldr\r\n"
		"1 14 0 0 0 1 0 0 0 1 0 0 0 1 3001.dat\r\n"
		"1 4 100 0 0 1 0 0 0 1 0 0 0 1 outer.ldr\r\n"
		"0 NOFILE\r\n"
		"0 FILE outer.ldr\r\n"
		"1 16 0 -24 0 0 0 1 0 1 0 -1 0 0 inner.ldr\r\n"
		"0 NOFILE\r\n"
		"0 FILE inner.ldr\r\n"
		"1 16 20 0 0 1 0 0 0 1 0 0 0 1 3001.dat\r\n"
		"1 1 0 0 0 1 0 0 0 1 0 0 0 1 3001.dat\r\n"
		"0 NOFILE\r\n"));

	QProcessEnvironment FlattenEnvironment = Environment;
	FlattenEnvironment.insert("XDG_CONFIG_HOME", Folder.filePath("config"));
	FlattenEnvironment.insert("XDG_DATA_HOME", Folder.filePath("data"));
	FlattenEnvironment.insert("XDG_CACHE_HOME", Folder.filePath("cache"));
	FlattenEnvironment.insert("QT_QPA_PLATFORM", "offscreen");

	const QString FlatName = Folder.filePath("flat.mpd");

	QProcess Flatten;
	Flatten.setProcessEnvironment(FlattenEnvironment);
	Flatten.setProcessChannelMode(QProcess::MergedChannels);
	Flatten.start(Binary, { "--library", Folder.filePath("library"), Folder.filePath("model.mpd"), "--flatten", FlatName });
	QVERIFY2(Flatten.waitForFinished(120000), "the flattened model was not saved");
	QVERIFY2(Flatten.exitCode() == 0, Flatten.readAll().constData());

	QFile Flat(FlatName);
	QVERIFY(Flat.open(QIODevice::ReadOnly));
	const QString FlatData = QString::fromUtf8(Flat.readAll());

	// the main model is the first file, up to its NOFILE
	const QString MainData = FlatData.left(FlatData.indexOf("0 NOFILE"));
	QList<QStringList> PartLines;

	for (const QString& Line : MainData.split('\n'))
	{
		const QStringList Fields = Line.simplified().split(' ');

		if (Fields.size() == 15 && Fields[0] == "1")
			PartLines.append(Fields);
	}

	const QList<QList<float>> Expected =
	{
		{ 14, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1 },
		{ 4, 100, -24, -20, 0, 0, 1, 0, 1, 0, -1, 0, 0 },
		{ 1, 100, -24, 0, 0, 0, 1, 0, 1, 0, -1, 0, 0 }
	};

	QVERIFY2(PartLines.size() == Expected.size(), qPrintable(MainData));

	for (int LineIndex = 0; LineIndex < Expected.size(); LineIndex++)
	{
		QVERIFY2(PartLines[LineIndex][14].toLower() == "3001.dat", qPrintable(MainData));

		for (int FieldIndex = 0; FieldIndex < Expected[LineIndex].size(); FieldIndex++)
			QVERIFY2(qAbs(PartLines[LineIndex][FieldIndex + 1].toFloat() - Expected[LineIndex][FieldIndex]) < 0.01f, qPrintable(QString("line %1 field %2:\n%3").arg(LineIndex).arg(FieldIndex + 1).arg(MainData)));
	}
}
