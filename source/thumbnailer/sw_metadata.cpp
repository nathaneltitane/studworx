#include "sw_metadata.h"
#include <KFileMetaData/ExtractionResult>
#include <KFileMetaData/Properties>
#include <QFile>
#include <QHash>
#include <QSet>
#include <QStringList>

// header lines that are not the model title
static const char* const gMetadataKeywords[] = { "FILE", "NOFILE", "Name:", "Author:", "!", "STEP", "BFC", "//", "ROTSTEP", "CLEAR", "PAUSE", "SAVE", "WRITE", "PRINT" };

// one model of the file and the files it places
struct swMetadataModel
{
	QStringList References;
	int StepCount = 0;
	bool HasPartsAfterStep = false;
};

// use to count the parts of a model through its submodels
static int swMetadataCountParts(const QString& Name, const QHash<QString, swMetadataModel>& Models, QHash<QString, int>& Counts, QSet<QString>& Visiting)
{
	if (Counts.contains(Name))
		return Counts.value(Name);

	// a model placing itself counts nothing
	if (Visiting.contains(Name))
		return 0;

	Visiting.insert(Name);

	int Count = 0;

	for (const QString& Reference : Models.value(Name).References)
		Count += Models.contains(Reference) ? swMetadataCountParts(Reference, Models, Counts, Visiting) : 1;

	Visiting.remove(Name);
	Counts.insert(Name, Count);

	return Count;
}

swMetadataExtractor::swMetadataExtractor(QObject* Parent)
	: KFileMetaData::ExtractorPlugin(Parent)
{
}

QStringList swMetadataExtractor::mimetypes() const
{
	return { QStringLiteral("application/x-ldraw"), QStringLiteral("application/x-multi-part-ldraw") };
}

void swMetadataExtractor::extract(KFileMetaData::ExtractionResult* Result)
{
	// ldraw files hold no searchable text, only details
	if (!(Result->inputFlags() & KFileMetaData::ExtractionResult::ExtractMetaData))
		return;

	QFile File(Result->inputUrl());

	if (!File.open(QIODevice::ReadOnly))
		return;

	QHash<QString, swMetadataModel> Models;
	QString MainName;
	QString ModelName;
	QString Title;
	QString Author;
	bool FirstLine = true;

	while (!File.atEnd())
	{
		QByteArray Data = File.readLine().trimmed();

		if (FirstLine && Data.startsWith("\xEF\xBB\xBF"))
			Data.remove(0, 3);

		FirstLine = false;

		if (Data.isEmpty())
			continue;

		const char LineType = Data.at(0);

		// skip data files sharing the dat extension, ldraw lines start with a line type from 0 to 5
		if (LineType < '0' || LineType > '5' || (Data.size() > 1 && Data.at(1) != ' ' && Data.at(1) != '\t'))
			return;

		const QString Line = QString::fromUtf8(Data);
		const QStringList Words = Line.split(QLatin1Char(' '), Qt::SkipEmptyParts);

		if (LineType == '0')
		{
			const QString Text = Line.mid(1).trimmed();

			if (Words.size() > 2 && Words.at(1) == QLatin1String("FILE"))
			{
				ModelName = Text.mid(4).trimmed().toLower();

				if (MainName.isEmpty())
					MainName = ModelName;

				Models[ModelName];
				continue;
			}

			// only the main model describes the file
			if (ModelName != MainName)
				continue;

			if (Text.startsWith(QLatin1String("Author:")))
			{
				if (Author.isEmpty())
					Author = Text.mid(7).section(QLatin1String(" // "), 0, 0).trimmed();
			}
			else if (Words.size() > 1 && Words.at(1) == QLatin1String("STEP"))
			{
				Models[ModelName].StepCount++;
				Models[ModelName].HasPartsAfterStep = false;
			}
			else if (Title.isEmpty() && !Text.isEmpty())
			{
				bool Keyword = false;

				for (const char* Name : gMetadataKeywords)
					Keyword = Keyword || Text.startsWith(QLatin1String(Name));

				if (!Keyword)
					Title = Text;
			}
		}
		else if (LineType == '1' && Words.size() >= 15)
		{
			// the file name may hold spaces
			const QString Reference = Words.mid(14).join(QLatin1Char(' ')).toLower();

			Models[ModelName].References.append(Reference);

			if (ModelName == MainName)
				Models[ModelName].HasPartsAfterStep = true;
		}
	}

	const swMetadataModel& Main = Models.value(MainName);

	QHash<QString, int> Counts;
	QSet<QString> Visiting;
	const int PartCount = swMetadataCountParts(MainName, Models, Counts, Visiting);
	const int StepCount = Main.StepCount + (Main.HasPartsAfterStep ? 1 : 0);
	const int ModelCount = qMax(static_cast<int>(Models.size()), 1);

	if (!Title.isEmpty())
		Result->add(KFileMetaData::Property::Title, Title);

	if (!Author.isEmpty())
		Result->add(KFileMetaData::Property::Author, Author);

	// kde has no part or step details, so they share the description
	const QString Description = QStringLiteral("%1 %2, %3 %4, %5 %6").arg(PartCount).arg(PartCount == 1 ? QStringLiteral("part") : QStringLiteral("parts")).arg(StepCount).arg(StepCount == 1 ? QStringLiteral("step") : QStringLiteral("steps")).arg(ModelCount).arg(ModelCount == 1 ? QStringLiteral("model") : QStringLiteral("models"));

	Result->add(KFileMetaData::Property::Description, Description);
}
