#include "sw_global.h"
#include "sw_tests.h"
#include "sw_commands.h"

static QString ReadSourceFile(const QString& Path)
{
	QFile File(QStringLiteral(SW_SOURCE_ROOT "/") + Path);

	if (!File.open(QIODevice::ReadOnly | QIODevice::Text))
		return QString();

	return QString::fromUtf8(File.readAll());
}

// menu name as the page writes it

static QString GetCommandName(const char* MenuName)
{
	QString Name = QString::fromUtf8(MenuName);

	Name.remove(QLatin1Char('&'));

	if (Name.endsWith(QLatin1String("...")))
		Name.chop(3);

	return Name.trimmed();
}

// The site page must name every command, so a new or renamed command cannot ship undocumented.
// Its command index and help entries name them, so SITE.md can explain instead of list.
void swTests::DocumentationNamesEveryCommand()
{
	const QString Site = ReadSourceFile(QStringLiteral("index.html"));
	QVERIFY(!Site.isEmpty());

	QStringList Missing;
	const QRegularExpression Number(QStringLiteral("\\d"));

	for (int CommandIndex = 0; CommandIndex < SW_COMMAND_COUNT; CommandIndex++)
	{
		const QString Name = GetCommandName(gCommands[CommandIndex].MenuName);

		// shortcut: numbered families are described as ranges, a test per family would need a list of them

		if (Name.isEmpty() || Name.contains(Number))
			continue;

		if (!Site.contains(Name))
			Missing.append(Name);
	}

	Missing.removeDuplicates();
	QVERIFY2(Missing.isEmpty(), qPrintable(QStringLiteral("index.html does not name, regenerate the site: ") + Missing.join(QStringLiteral(", "))));
}

// Every command must explain itself in "What's This?".
void swTests::DocumentationHelpsEveryCommand()
{
	QStringList Missing;

	for (int CommandIndex = 0; CommandIndex < SW_COMMAND_COUNT; CommandIndex++)
	{
		const swCommand& Command = gCommands[CommandIndex];

		// separators and hidden entries have no menu name

		if (!Command.MenuName[0])
			continue;

		if (!Command.WhatsThis[0])
			Missing.append(GetCommandName(Command.MenuName));
	}

	Missing.removeDuplicates();
	QVERIFY2(Missing.isEmpty(), qPrintable(QStringLiteral("No WhatsThis for: ") + Missing.join(QStringLiteral(", "))));
}

// Every image is named section-kind-subject, linked from the site page, and every link has its image.
void swTests::DocumentationLinksEveryImage()
{
	const QString Site = ReadSourceFile(QStringLiteral("SITE.md"));
	QVERIFY(!Site.isEmpty());

	const QDir Images(QStringLiteral(SW_SOURCE_ROOT "/resources/images"));
	QVERIFY(Images.exists());

	const QRegularExpression Pattern(QStringLiteral("^[a-z]+-(window|menu|widget|feature)-[a-z0-9-]+\\.(png|gif)$"));
	QStringList Unnamed, Unlinked, Missing;

	for (const QString& FileName : Images.entryList(QStringList() << QStringLiteral("*.png") << QStringLiteral("*.gif"), QDir::Files))
	{
		if (!Pattern.match(FileName).hasMatch())
			Unnamed.append(FileName);

		if (!Site.contains(QStringLiteral("resources/images/") + FileName))
			Unlinked.append(FileName);
	}

	// links in the page and the site template

	const QStringList Pages = { QStringLiteral("SITE.md"), QStringLiteral("README.md"), QStringLiteral("resources/site/template.html") };
	const QRegularExpression Link(QStringLiteral("resources/images/([A-Za-z0-9_.-]+\\.(png|gif))"));

	for (const QString& Page : Pages)
	{
		QRegularExpressionMatchIterator Match = Link.globalMatch(ReadSourceFile(Page));

		while (Match.hasNext())
		{
			const QString FileName = Match.next().captured(1);

			if (!Images.exists(FileName))
				Missing.append(Page + QStringLiteral(": ") + FileName);
		}
	}

	QVERIFY2(Unnamed.isEmpty(), qPrintable(QStringLiteral("Not named section-kind-subject: ") + Unnamed.join(QStringLiteral(", "))));
	QVERIFY2(Unlinked.isEmpty(), qPrintable(QStringLiteral("Not linked in SITE.md: ") + Unlinked.join(QStringLiteral(", "))));
	QVERIFY2(Missing.isEmpty(), qPrintable(QStringLiteral("Linked but missing: ") + Missing.join(QStringLiteral(", "))));
}

// Style and theme icons keep platform colors, so every interface icon is painted.
void swTests::InterfaceUsesPaintedIcons()
{
	const QRegularExpression Pattern(QStringLiteral("(->|\\.)standardIcon\\(|QIcon::fromTheme\\("));
	QStringList Found;
	QDirIterator Files(QStringLiteral(SW_SOURCE_ROOT "/source"), QStringList() << QStringLiteral("*.cpp") << QStringLiteral("*.h"), QDir::Files, QDirIterator::Subdirectories);

	while (Files.hasNext())
	{
		const QString FileName = Files.next();

		if (!FileName.contains(QLatin1String("/testing/")) && Pattern.match(ReadSourceFile(FileName.mid(QStringLiteral(SW_SOURCE_ROOT "/").size()))).hasMatch())
			Found.append(QFileInfo(FileName).fileName());
	}

	QVERIFY2(Found.isEmpty(), qPrintable(QStringLiteral("Unpainted icons in: ") + Found.join(QStringLiteral(", "))));
}

// Feature bullets explain in a sentence or two, so lists of names and paths cannot grow back, and the
// generated page holds no unfilled placeholder.
void swTests::DocumentationKeepsFeaturesShort()
{
	const QString Text = ReadSourceFile(QStringLiteral("SITE.md"));
	QVERIFY(!Text.isEmpty());

	const int LengthMaximum = 450;
	QStringList Long;

	for (const QString& Line : Text.split(QLatin1Char('\n')))
		if (Line.startsWith(QLatin1String("- ")) && Line.size() > LengthMaximum)
			Long.append(Line.left(60));

	QVERIFY2(Long.isEmpty(), qPrintable(QStringLiteral("SITE.md bullets over 450 characters: ") + Long.join(QStringLiteral(" | "))));

	const QString Site = ReadSourceFile(QStringLiteral("index.html"));
	QVERIFY(!Site.isEmpty());

	const QRegularExpressionMatch Placeholder = QRegularExpression(QStringLiteral("@[A-Z_]+@|<!-- include ")).match(Site);
	QVERIFY2(!Placeholder.hasMatch(), qPrintable(QStringLiteral("index.html holds an unfilled placeholder, regenerate the site: ") + Placeholder.captured(0)));
}

// --menus feeds the command index of the site: it must name every command once as a menu path, a
// tab and its shortcut, and hold no recent file or other path of the machine it ran on.
void swTests::MenusNameEveryCommand()
{
	const QProcessEnvironment Environment = QProcessEnvironment::systemEnvironment();
	const QString Binary = Environment.value("SW_TEST_BINARY");

	if (Binary.isEmpty())
		QSKIP("SW_TEST_BINARY is not set");

	QTemporaryDir Folder;
	QVERIFY(Folder.isValid());
	QVERIFY(QDir(Folder.path()).mkpath("library/parts"));

	QFile Model(Folder.filePath("model.ldr"));
	QVERIFY(Model.open(QIODevice::WriteOnly));
	QVERIFY(Model.write("0 Menus\r\n0 Name: model.ldr\r\n") > 0);
	Model.close();

	QProcessEnvironment MenusEnvironment = Environment;
	MenusEnvironment.insert("XDG_CONFIG_HOME", Folder.filePath("config"));
	MenusEnvironment.insert("XDG_DATA_HOME", Folder.filePath("data"));
	MenusEnvironment.insert("XDG_CACHE_HOME", Folder.filePath("cache"));
	MenusEnvironment.insert("QT_QPA_PLATFORM", "offscreen");

	QProcess Menus;
	Menus.setProcessEnvironment(MenusEnvironment);
	Menus.start(Binary, { "--library", Folder.filePath("library"), "--menus", Model.fileName() });
	QVERIFY2(Menus.waitForFinished(180000), "the menus were not printed");
	QVERIFY2(Menus.exitCode() == 0, Menus.readAllStandardError().constData());

	const QStringList Lines = QString::fromUtf8(Menus.readAllStandardOutput()).split(QLatin1Char('\n'), Qt::SkipEmptyParts);
	QVERIFY(!Lines.isEmpty());

	QStringList Malformed, Paths;
	const QRegularExpression Path(QStringLiteral("/home/|/tmp/|/Users/|[A-Za-z]:\\\\|\\.(ldr|mpd|dat)\\b"));

	for (const QString& Line : Lines)
	{
		if (!Line.contains(QLatin1String(" > ")) || Line.count(QLatin1Char('\t')) != 1)
			Malformed.append(Line);

		if (Path.match(Line).hasMatch())
			Paths.append(Line);
	}

	QVERIFY2(Malformed.isEmpty(), qPrintable(QStringLiteral("Not a menu path, a tab and a shortcut: ") + Malformed.join(QStringLiteral(" | "))));
	QVERIFY2(Paths.isEmpty(), qPrintable(QStringLiteral("Machine paths in the menus: ") + Paths.join(QStringLiteral(" | "))));

	const QString Text = Lines.join(QLatin1Char('\n'));
	const QRegularExpression Number(QStringLiteral("\\d"));
	QStringList Missing;

	for (int CommandIndex = 0; CommandIndex < SW_COMMAND_COUNT; CommandIndex++)
	{
		const QString Name = GetCommandName(gCommands[CommandIndex].MenuName);

		// numbered families are described as ranges, as in DocumentationNamesEveryCommand

		if (!Name.isEmpty() && !Name.contains(Number) && !Text.contains(Name))
			Missing.append(Name);
	}

	Missing.removeDuplicates();
	QVERIFY2(Missing.isEmpty(), qPrintable(QStringLiteral("--menus does not name: ") + Missing.join(QStringLiteral(", "))));
}
