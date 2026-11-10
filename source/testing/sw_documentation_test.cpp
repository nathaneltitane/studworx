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
void swTests::DocumentationNamesEveryCommand()
{
	const QString Site = ReadSourceFile(QStringLiteral("SITE.md"));
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
	QVERIFY2(Missing.isEmpty(), qPrintable(QStringLiteral("SITE.md does not name: ") + Missing.join(QStringLiteral(", "))));
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
