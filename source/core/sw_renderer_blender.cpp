#include "sw_global.h"
#include "sw_renderer_blender.h"

#define SW_BLENDER_RESOURCE_PATH ":/resources/blender"
#define SW_BLENDER_DATA_FOLDER "Blender"
#define SW_BLENDER_ADDON_FOLDER "addons"
#define SW_BLENDER_CONFIGURATION_FOLDER "configuration"
#define SW_BLENDER_TEST_MARKER "###TEST_BLENDER###"
#define SW_BLENDER_RENDER_MODULE "io_scene_render_ldraw"
#define SW_BLENDER_IMPORT_MODULE "io_scene_import_ldraw"
#define SW_BLENDER_SETTINGS_FILE "render_preferences.ini"
#define SW_BLENDER_IMPORT_SETTINGS_FILE SW_BLENDER_IMPORT_MODULE "/" SW_BLENDER_CONFIGURATION_FOLDER "/import_preferences.ini"
#define SW_BLENDER_SETTINGS_GROUP "ImportLDraw/"

static const char* const swBlenderAddonModules[] = { SW_BLENDER_IMPORT_MODULE, SW_BLENDER_RENDER_MODULE };

QString swGetBlenderDataPath()
{
	return QDir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)).filePath(QLatin1String(SW_BLENDER_DATA_FOLDER));
}

QString swGetBlenderAddonPath()
{
	return QDir(swGetBlenderDataPath()).filePath(QLatin1String(SW_BLENDER_ADDON_FOLDER));
}

QString swGetBlenderConfigurationPath()
{
	return QDir(swGetBlenderAddonPath()).filePath(QLatin1String(SW_BLENDER_RENDER_MODULE "/" SW_BLENDER_CONFIGURATION_FOLDER));
}

// use to find the settings file, seeded with the import defaults
static QString swGetBlenderSettingsFile()
{
	const QString FileName = QDir(swGetBlenderConfigurationPath()).filePath(QLatin1String(SW_BLENDER_SETTINGS_FILE));

	if (!QFileInfo::exists(FileName) && QDir().mkpath(swGetBlenderConfigurationPath()) && QFile::copy(QLatin1String(SW_BLENDER_RESOURCE_PATH "/" SW_BLENDER_IMPORT_SETTINGS_FILE), FileName))
		QFile(FileName).setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ReadGroup | QFile::ReadOther);

	return FileName;
}

QString swGetBlenderSetting(const QString& Key)
{
	return QSettings(swGetBlenderSettingsFile(), QSettings::IniFormat).value(QLatin1String(SW_BLENDER_SETTINGS_GROUP) + Key).toString();
}

void swSetBlenderSetting(const QString& Key, const QString& Value)
{
	QSettings(swGetBlenderSettingsFile(), QSettings::IniFormat).setValue(QLatin1String(SW_BLENDER_SETTINGS_GROUP) + Key, Value);
}

bool swSetupBlenderAddon(QString& Error)
{
	const QDir ResourceDir(QLatin1String(SW_BLENDER_RESOURCE_PATH));
	const QDir AddonDir(swGetBlenderAddonPath());
	const QString ConfigurationFolder = QLatin1String("/" SW_BLENDER_CONFIGURATION_FOLDER "/");

	QDirIterator Iterator(ResourceDir.path(), QDir::Files, QDirIterator::Subdirectories);

	while (Iterator.hasNext())
	{
		const QString ResourceFile = Iterator.next();
		const QString RelativeName = ResourceDir.relativeFilePath(ResourceFile);
		const QString TargetFile = AddonDir.filePath(RelativeName);
		QFile Target(TargetFile);

		// configuration files hold the settings studworx and the add-ons write
		if (Target.exists() && ResourceFile.contains(ConfigurationFolder))
			continue;

		QFile Resource(ResourceFile);

		if (!Resource.open(QIODevice::ReadOnly))
		{
			Error = QCoreApplication::translate("swBlenderRenderer", "Cannot read '%1'.").arg(ResourceFile);
			return false;
		}

		const QByteArray Data = Resource.readAll();

		if (Target.open(QIODevice::ReadOnly) && Target.readAll() == Data)
			continue;

		Target.close();

		QSaveFile Output(TargetFile);

		if (!QDir().mkpath(QFileInfo(TargetFile).absolutePath()) || !Output.open(QIODevice::WriteOnly) || Output.write(Data) < 0 || !Output.commit())
		{
			Error = QCoreApplication::translate("swBlenderRenderer", "Cannot write '%1':\n%2").arg(QDir::toNativeSeparators(TargetFile), Output.errorString());
			return false;
		}
	}

	return true;
}

QString swGetBlenderVersion(const QString& Path, QString& Error)
{
	const QString Expression = QLatin1String("import bpy, sys; print('" SW_BLENDER_TEST_MARKER "' + bpy.app.version_string); sys.stdout.flush(); sys.exit()");

	QProcess Process;
	Process.start(Path, { QLatin1String("--factory-startup"), QLatin1String("--background"), QLatin1String("--python-expr"), Expression });

	if (!Process.waitForStarted(5000) || !Process.waitForFinished(60000))
	{
		Error = Process.errorString();
		Process.kill();
		Process.waitForFinished(1000);

		return QString();
	}

	const QString Output = QString::fromLocal8Bit(Process.readAllStandardOutput());
	const int MarkerIndex = Output.indexOf(QLatin1String(SW_BLENDER_TEST_MARKER));

	if (MarkerIndex < 0)
	{
		Error = QString::fromLocal8Bit(Process.readAllStandardError()).trimmed();
		return QString();
	}

	const int VersionIndex = MarkerIndex + int(strlen(SW_BLENDER_TEST_MARKER));

	return Output.mid(VersionIndex, Output.indexOf('\n', VersionIndex) - VersionIndex).trimmed();
}

QString swGetBlenderPythonString(const QString& Text)
{
	QString Quoted = Text;

	Quoted.replace(QLatin1Char('\\'), QLatin1String("\\\\"));
	Quoted.replace(QLatin1Char('\''), QLatin1String("\\'"));
	Quoted.replace(QLatin1Char('\n'), QLatin1String("\\n"));
	Quoted.replace(QLatin1Char('\r'), QLatin1String("\\r"));

	return QLatin1Char('\'') + Quoted + QLatin1Char('\'');
}

QStringList swGetBlenderArguments(const QString& Expression)
{
	// addon_utils loads the add-ons for this session only, so the blender preferences stay untouched
	QString Setup = QString("import sys, addon_utils; sys.path.insert(0, %1); ").arg(swGetBlenderPythonString(QDir::toNativeSeparators(swGetBlenderAddonPath())));

	for (const char* Module : swBlenderAddonModules)
		Setup += QString("addon_utils.enable('%1', default_set=False); ").arg(QLatin1String(Module));

	return { QLatin1String("--python-expr"), Setup + Expression };
}
