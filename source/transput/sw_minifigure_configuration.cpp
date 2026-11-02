#include "sw_global.h"
#include "sw_minifigure_configuration.h"
#include "sw_file.h"
#include "sw_file_zip.h"
#include "sw_http.h"

// the minifigure part list is published by ldraw.org as a zipped MLCad.ini
static const QString swMinifigureConfigurationName = QStringLiteral("minifigure.ini");
static const QString swMinifigureConfigurationResource = QStringLiteral(":/resources/data/minifigure.ini");
static const QString swMinifigureConfigurationSite = QStringLiteral("https://www.ldraw.org/");
static const QString swMinifigureConfigurationPage = QStringLiteral("mlcad-ini-download.html");
static const QString swMinifigureConfigurationArchiveEntry = QStringLiteral("MLCad.ini");
static const QRegularExpression swMinifigureConfigurationArchivePattern(QStringLiteral("uploads/files/mlcad/MLCad_ini_[0-9-]+\\.zip"));

// every minifigure part list starts its parts with this section
static const QByteArray swMinifigureConfigurationSection = QByteArrayLiteral("[HATS]");

// sections only MLCad reads
static const QList<QByteArray> swMinifigureConfigurationSkippedSections = { QByteArrayLiteral("[SCAN_ORDER]"), QByteArrayLiteral("[LSYNTH]") };

// the comments kept from the head of the file start here
static const QByteArray swMinifigureConfigurationKeptComments = QByteArrayLiteral("; SUPPORT");

// one part entry is a quoted name, a quoted file name and numbers
static const QRegularExpression swMinifigureConfigurationEntryPattern(QStringLiteral("^\"(.*)\"\\s*\"([^\"]*)\"\\s+(.*)$"));

QString swGetMinifigureConfigurationName()
{
	return swMinifigureConfigurationName;
}

QString swGetMinifigureConfigurationResource()
{
	return swMinifigureConfigurationResource;
}

// use to rewrite MLCad.ini in the layout of the bundled minifigure.ini
static QByteArray swConvertMinifigureConfiguration(const QByteArray& Source)
{
	const QList<QByteArray> SourceLines = Source.split('\n');
	QList<QByteArray> Lines;
	bool InSection = false;
	bool SkipSection = false;
	bool KeepComments = false;

	for (const QByteArray& SourceLine : SourceLines)
	{
		const QByteArray Line = QString::fromLatin1(SourceLine).trimmed().toLatin1();

		if (Line.startsWith('['))
		{
			InSection = true;
			SkipSection = swMinifigureConfigurationSkippedSections.contains(Line);

			if (SkipSection)
				continue;

			if (!Lines.isEmpty() && !Lines.last().isEmpty())
				Lines.append(QByteArray());

			Lines.append(Line);
			continue;
		}

		// the head keeps the name and version lines and the support notes
		if (!InSection)
		{
			if (Lines.size() < 2 && Line.startsWith(';'))
				Lines.append(Line);
			else if (Lines.size() == 2)
				Lines.append(QByteArray());

			if (Line.startsWith(swMinifigureConfigurationKeptComments))
				KeepComments = true;

			if (KeepComments && !(Line.isEmpty() && Lines.last().isEmpty()))
				Lines.append(Line);

			continue;
		}

		if (SkipSection || Line.isEmpty() || Line.startsWith(';'))
			continue;

		const QRegularExpressionMatch Match = swMinifigureConfigurationEntryPattern.match(QString::fromLatin1(Line));

		if (!Match.hasMatch())
		{
			Lines.append(Line);
			continue;
		}

		// hidden entries keep a blank file name
		const QString FileName = Match.captured(2).isEmpty() ? QStringLiteral(" ") : Match.captured(2);
		const QString Numbers = Match.captured(3).simplified();

		Lines.append(QStringLiteral("\"%1\" \"%2\" %3").arg(Match.captured(1), FileName, Numbers).toLatin1());
	}

	while (!Lines.isEmpty() && Lines.last().isEmpty())
		Lines.removeLast();

	return Lines.join('\n') + '\n';
}

// use to find the newest archive linked from the download page
static QString swFindMinifigureConfigurationArchive(const QByteArray& Page)
{
	QStringList Archives;
	QRegularExpressionMatchIterator Matches = swMinifigureConfigurationArchivePattern.globalMatch(QString::fromLatin1(Page));

	while (Matches.hasNext())
		Archives.append(Matches.next().captured(0));

	if (Archives.isEmpty())
		return QString();

	// archive names carry the year and month so the last one is the newest
	Archives.sort();

	return swMinifigureConfigurationSite + Archives.last();
}

// use to take MLCad.ini out of the downloaded archive
static QByteArray swExtractMinifigureConfiguration(const QByteArray& Archive)
{
	std::unique_ptr<swMemoryFile> ArchiveFile(new swMemoryFile());
	ArchiveFile->WriteBuffer(Archive.constData(), Archive.size());
	ArchiveFile->Seek(0, SEEK_SET);

	swZipFile ZipFile;
	swMemoryFile ConfigurationFile;

	if (!ZipFile.OpenRead(std::move(ArchiveFile)) || !ZipFile.ExtractFile(swMinifigureConfigurationArchiveEntry.toLatin1().constData(), ConfigurationFile))
		return QByteArray();

	return QByteArray(reinterpret_cast<const char*>(ConfigurationFile.mBuffer), static_cast<int>(ConfigurationFile.GetLength()));
}

// use to read the copy bundled with the application
static QByteArray swReadBundledMinifigureConfiguration()
{
	QFile File(swMinifigureConfigurationResource);

	return File.open(QIODevice::ReadOnly) ? File.readAll() : QByteArray();
}

swResult<void> swUpdateMinifigureConfiguration(QWidget* Parent, const QString& FileName)
{
	if (FileName.isEmpty())
		return swUnexpected(QObject::tr("No file is set for the minifigure builder settings."));

	const QString Title = QObject::tr("Update Minifigure Builder Settings");
	const QString Label = QObject::tr("Downloading %1 from ldraw.org...").arg(swMinifigureConfigurationArchiveEntry);
	const std::optional<QByteArray> Page = swDownloadWithProgress(Parent, swMinifigureConfigurationSite + swMinifigureConfigurationPage, Title, Label);
	const QString ArchiveUrl = Page ? swFindMinifigureConfigurationArchive(*Page) : QString();
	QByteArray Data;

	if (!ArchiveUrl.isEmpty())
	{
		const std::optional<QByteArray> Archive = swDownloadWithProgress(Parent, ArchiveUrl, Title, Label);

		if (Archive)
			Data = swExtractMinifigureConfiguration(*Archive);
	}

	// the bundled copy stands in when offline
	if (Data.contains(swMinifigureConfigurationSection))
		Data = swConvertMinifigureConfiguration(Data);
	else
		Data = swReadBundledMinifigureConfiguration();

	if (!Data.contains(swMinifigureConfigurationSection))
		return swUnexpected(QObject::tr("Could not download %1 from ldraw.org.").arg(swMinifigureConfigurationArchiveEntry));

	QSaveFile File(FileName);

	if (!File.open(QIODevice::WriteOnly) || File.write(Data) < 0 || !File.commit())
		return swUnexpected(QObject::tr("Could not write the minifigure builder settings file '%1'.").arg(QDir::toNativeSeparators(FileName)));

	return swResult<void>();
}
