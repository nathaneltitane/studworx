#include "sw_global.h"
#include "sw_tests.h"

struct swTranslationMessage
{
	QString Context;
	QString Source;
	QString Comment;
	QString Translation;
	QString Type;
	bool Numerus = false;
};

// every message of one catalog

static QList<swTranslationMessage> ReadCatalog(const QString& Path, bool* Valid)
{
	QList<swTranslationMessage> Messages;
	QFile File(Path);

	*Valid = File.open(QIODevice::ReadOnly);

	if (!*Valid)
		return Messages;

	QXmlStreamReader Reader(&File);
	QString Context;
	swTranslationMessage Message;

	while (!Reader.atEnd())
	{
		Reader.readNext();

		if (!Reader.isStartElement())
			continue;

		const auto Name = Reader.name();

		if (Name == QLatin1String("name"))
			Context = Reader.readElementText();
		else if (Name == QLatin1String("message"))
		{
			Message = swTranslationMessage();
			Message.Context = Context;
			Message.Numerus = Reader.attributes().value(QLatin1String("numerus")) == QLatin1String("yes");
		}
		else if (Name == QLatin1String("source"))
			Message.Source = Reader.readElementText();
		else if (Name == QLatin1String("comment"))
			Message.Comment = Reader.readElementText();
		else if (Name == QLatin1String("translation"))
		{
			Message.Type = Reader.attributes().value(QLatin1String("type")).toString();
			Message.Translation = Reader.readElementText(QXmlStreamReader::IncludeChildElements);
			Messages.append(Message);
		}
	}

	*Valid = !Reader.hasError();

	return Messages;
}

// catalogs the project file lists

static QStringList GetCatalogs()
{
	QFile File(QStringLiteral(SW_SOURCE_ROOT "/studworx.pro"));

	if (!File.open(QIODevice::ReadOnly | QIODevice::Text))
		return QStringList();

	const QRegularExpressionMatch Match = QRegularExpression(QStringLiteral("TRANSLATIONS\\s*=([^\\n]*)")).match(QString::fromUtf8(File.readAll()));

	return Match.captured(1).split(QLatin1Char(' '), Qt::SkipEmptyParts);
}

// Every listed catalog must parse, and its compiled .qm must give back each translation it holds,
// so a catalog edited without running tools/studworx-language-release cannot ship stale.
void swTests::TranslationsMatchCompiledCatalogs()
{
	const QStringList Catalogs = GetCatalogs();
	QVERIFY(Catalogs.size() > 1);

	QStringList Problems;

	for (const QString& Catalog : Catalogs)
	{
		const QString Path = QStringLiteral(SW_SOURCE_ROOT "/") + Catalog;
		bool Valid = false;
		const QList<swTranslationMessage> Messages = ReadCatalog(Path, &Valid);

		if (!Valid)
		{
			Problems.append(Catalog + QStringLiteral(": does not parse"));
			continue;
		}

		QTranslator Translator;
		QString PathCompiled = Path;
		PathCompiled.replace(QLatin1String(".ts"), QLatin1String(".qm"));

		if (!Translator.load(PathCompiled))
		{
			Problems.append(Catalog + QStringLiteral(": no compiled .qm"));
			continue;
		}

		int Stale = 0;

		for (const swTranslationMessage& Message : Messages)
		{
			if (Message.Numerus || Message.Translation.isEmpty() || Message.Type == QLatin1String("obsolete") || Message.Type == QLatin1String("vanished"))
				continue;

			const QByteArray Comment = Message.Comment.toUtf8();

			if (Translator.translate(Message.Context.toUtf8().constData(), Message.Source.toUtf8().constData(), Comment.isEmpty() ? nullptr : Comment.constData()) != Message.Translation)
				Stale++;
		}

		if (Stale)
			Problems.append(Catalog + QStringLiteral(": %1 translations missing from the .qm").arg(Stale));
	}

	QVERIFY2(Problems.isEmpty(), qPrintable(Problems.join(QStringLiteral("; "))));
}

// A shown translation must keep the placeholders of its source and never carry the {1?} guesses
// that the lupdate number heuristic writes, which the program would show as they are.
void swTests::TranslationsKeepPlaceholders()
{
	const QStringList Catalogs = GetCatalogs();
	QVERIFY(Catalogs.size() > 1);

	const QRegularExpression Guess(QStringLiteral("\\{[0-9,]*\\?\\}"));
	const QRegularExpression Placeholder(QStringLiteral("%L?([1-9])"));
	QStringList Problems;

	for (const QString& Catalog : Catalogs)
	{
		bool Valid = false;
		const QList<swTranslationMessage> Messages = ReadCatalog(QStringLiteral(SW_SOURCE_ROOT "/") + Catalog, &Valid);

		QVERIFY2(Valid, qPrintable(Catalog));

		for (const swTranslationMessage& Message : Messages)
		{
			if (Message.Numerus || Message.Translation.isEmpty() || Message.Type == QLatin1String("obsolete") || Message.Type == QLatin1String("vanished"))
				continue;

			const QString Where = Catalog.section(QLatin1Char('/'), -1) + QStringLiteral(" ") + Message.Context + QStringLiteral(" \"") + Message.Source.left(40) + QStringLiteral("\"");

			if (Guess.match(Message.Translation).hasMatch() && !Guess.match(Message.Source).hasMatch())
				Problems.append(Where + QStringLiteral(" has a merge guess"));

			QRegularExpressionMatchIterator Match = Placeholder.globalMatch(Message.Source);

			while (Match.hasNext())
			{
				const QString Number = Match.next().captured(1);

				if (!QRegularExpression(QStringLiteral("%L?") + Number + QStringLiteral("(?![0-9])")).match(Message.Translation).hasMatch())
				{
					Problems.append(Where + QStringLiteral(" drops %") + Number);
					break;
				}
			}
		}
	}

	QVERIFY2(Problems.isEmpty(), qPrintable(QString::number(Problems.size()) + QStringLiteral(" problems: ") + Problems.mid(0, 20).join(QStringLiteral("; "))));
}
