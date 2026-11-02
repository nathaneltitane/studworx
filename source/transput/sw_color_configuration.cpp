#include "sw_global.h"
#include "sw_color_configuration.h"
#include "sw_file.h"
#include "sw_file_zip.h"
#include "sw_library.h"
#include "sw_application.h"
#include "sw_profile.h"
#include "sw_http.h"

// official color configurations and where they are published
static const QString swColorConfigurationStandardName = QStringLiteral("LDConfig.ldr");
static const QString swColorConfigurationAlternateName = QStringLiteral("LDCfgalt.ldr");
static const QString swColorConfigurationUrl = QStringLiteral("https://library.ldraw.org/library/official/%1");
static const QString swColorConfigurationArchiveFolder = QStringLiteral("ldraw/%1");
static const QString swColorConfigurationStandardResource = QStringLiteral(":/resources/data/ldraw-color-configuration.ldr");
static const QString swColorConfigurationAlternateResource = QStringLiteral(":/resources/data/ldraw-color-configuration-alternate.ldr");

// every color configuration defines colors with this meta
static const QByteArray swColorConfigurationMeta = QByteArrayLiteral("!COLOUR");

swColorSource swGetColorSource()
{
	const int Source = swGetProfileInteger(SW_PROFILE_COLOR_SOURCE);

	// an unset source keeps a configuration chosen before sources existed
	if (Source < 0)
		return swGetProfileString(SW_PROFILE_COLOR_CONFIGURATION).isEmpty() ? swColorSource::Library : swColorSource::Standard;

	return static_cast<swColorSource>(qBound(static_cast<int>(swColorSource::Library), Source, static_cast<int>(swColorSource::Alternate)));
}

void swSetColorSource(swColorSource Source)
{
	swSetProfileInteger(SW_PROFILE_COLOR_SOURCE, static_cast<int>(Source));
}

QString swGetColorConfigurationName(swColorSource Source)
{
	switch (Source)
	{
	case swColorSource::Library:
		return QCoreApplication::translate("swColorConfiguration", "Parts Library");

	case swColorSource::Standard:
		return QCoreApplication::translate("swColorConfiguration", "LDraw Color Configuration");

	case swColorSource::Alternate:
		return QCoreApplication::translate("swColorConfiguration", "LDraw Alternate Color Configuration");
	}

	return QString();
}

QString swGetColorConfigurationOfficialName(swColorSource Source)
{
	return Source == swColorSource::Alternate ? swColorConfigurationAlternateName : swColorConfigurationStandardName;
}

QString swGetColorConfigurationPath(swColorSource Source)
{
	switch (Source)
	{
	case swColorSource::Library:
		break;

	case swColorSource::Standard:
		return swGetProfileString(SW_PROFILE_COLOR_CONFIGURATION);

	case swColorSource::Alternate:
		return swGetProfileString(SW_PROFILE_COLOR_CONFIGURATION_ALTERNATE);
	}

	return QString();
}

void swSetColorConfigurationPath(swColorSource Source, const QString& FileName)
{
	if (Source == swColorSource::Standard)
		swSetProfileString(SW_PROFILE_COLOR_CONFIGURATION, FileName);
	else if (Source == swColorSource::Alternate)
		swSetProfileString(SW_PROFILE_COLOR_CONFIGURATION_ALTERNATE, FileName);
}

bool swColorConfigurationExists(const QString& FileName)
{
	return !FileName.isEmpty() && QFileInfo(FileName).isFile();
}

// use to read the official file from the parts library
static QByteArray swReadLibraryColorConfiguration(const QString& OfficialName)
{
	const QString LibraryPath = swGetPartsLibrary() ? swGetPartsLibrary()->GetLibraryPath() : swGetProfileString(SW_PROFILE_PARTS_LIBRARY);

	if (LibraryPath.isEmpty())
		return QByteArray();

	const QFileInfo LibraryInfo(LibraryPath);

	if (LibraryInfo.isFile())
	{
		std::unique_ptr<swDiskFile> File(new swDiskFile(LibraryPath));
		swZipFile ZipFile;
		swMemoryFile ColorFile;

		if (!File->Open(QIODevice::ReadOnly) || !ZipFile.OpenRead(std::move(File)) || !ZipFile.ExtractFile(swColorConfigurationArchiveFolder.arg(OfficialName).toLatin1().constData(), ColorFile))
			return QByteArray();

		return QByteArray(reinterpret_cast<const char*>(ColorFile.mBuffer), static_cast<int>(ColorFile.GetLength()));
	}

	// folder libraries may use any case
	const QDir LibraryDir(LibraryPath);
	const QStringList FileNames = LibraryDir.entryList(QStringList(OfficialName), QDir::Files);

	if (FileNames.isEmpty())
		return QByteArray();

	QFile File(LibraryDir.absoluteFilePath(FileNames.first()));

	return File.open(QIODevice::ReadOnly) ? File.readAll() : QByteArray();
}

QString swGetColorConfigurationResource(swColorSource Source)
{
	return Source == swColorSource::Alternate ? swColorConfigurationAlternateResource : swColorConfigurationStandardResource;
}

// use to read the copy bundled with the application
static QByteArray swReadBundledColorConfiguration(swColorSource Source)
{
	QFile File(swGetColorConfigurationResource(Source));

	return File.open(QIODevice::ReadOnly) ? File.readAll() : QByteArray();
}

swResult<void> swUpdateColorConfiguration(QWidget* Parent, swColorSource Source, const QString& FileName)
{
	if (Source == swColorSource::Library)
		return swResult<void>();

	if (FileName.isEmpty())
		return swUnexpected(QObject::tr("No file is set for the %1.").arg(swGetColorConfigurationName(Source)));

	const QString OfficialName = swGetColorConfigurationOfficialName(Source);
	const QString Title = QObject::tr("Update %1").arg(swGetColorConfigurationName(Source));
	const std::optional<QByteArray> Download = swDownloadWithProgress(Parent, swColorConfigurationUrl.arg(OfficialName), Title, QObject::tr("Downloading %1 from library.ldraw.org...").arg(OfficialName));

	// the parts library and then the bundled copy stand in when offline
	QByteArray Data = Download ? *Download : QByteArray();

	if (!Data.contains(swColorConfigurationMeta))
		Data = swReadLibraryColorConfiguration(OfficialName);

	if (!Data.contains(swColorConfigurationMeta))
		Data = swReadBundledColorConfiguration(Source);

	if (!Data.contains(swColorConfigurationMeta))
		return swUnexpected(QObject::tr("Could not download %1 from library.ldraw.org or find it in the parts library.").arg(OfficialName));

	QSaveFile File(FileName);

	if (!File.open(QIODevice::WriteOnly) || File.write(Data) < 0 || !File.commit())
		return swUnexpected(QObject::tr("Could not write the %1 file '%2'.").arg(swGetColorConfigurationName(Source), QDir::toNativeSeparators(FileName)));

	return swResult<void>();
}
