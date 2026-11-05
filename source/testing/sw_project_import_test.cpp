#include "sw_tests.h"

// Opens a one brick LEGO Digital Designer text model on the command line. It must load as an
// untitled project and leave the model with its part, written here by --flatten as an LDraw file.
// SW_TEST_BINARY is the studworx program.
void swTests::OpenLXFMLImportsModel()
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
	QVERIFY(WriteFile("model.lxfml",
		"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"
		"<LXFML versionMajor=\"5\" versionMinor=\"0\"><Bricks><Brick refID=\"0\" designID=\"3001\">"
		"<Part refID=\"0\" designID=\"3001\" materials=\"26\"><Bone refID=\"0\" transformation=\"1,0,0,0,1,0,0,0,1,0,0,0\"/></Part>"
		"</Brick></Bricks></LXFML>\n"));

	QProcessEnvironment ImportEnvironment = Environment;
	ImportEnvironment.insert("XDG_CONFIG_HOME", Folder.filePath("config"));
	ImportEnvironment.insert("XDG_DATA_HOME", Folder.filePath("data"));
	ImportEnvironment.insert("XDG_CACHE_HOME", Folder.filePath("cache"));
	ImportEnvironment.insert("QT_QPA_PLATFORM", "offscreen");

	const QString OutName = Folder.filePath("out.mpd");

	QProcess Import;
	Import.setProcessEnvironment(ImportEnvironment);
	Import.setProcessChannelMode(QProcess::MergedChannels);
	Import.start(Binary, { "--library", Folder.filePath("library"), Folder.filePath("model.lxfml"), "--flatten", OutName });
	QVERIFY2(Import.waitForFinished(120000), "the imported model was not saved");
	QVERIFY2(Import.exitCode() == 0, Import.readAll().constData());

	QFile File(OutName);
	QVERIFY(File.open(QIODevice::ReadOnly));
	QVERIFY2(QString::fromUtf8(File.readAll()).toLower().contains("3001.dat"), "the imported part is missing");
}
