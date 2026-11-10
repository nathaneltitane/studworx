#include "sw_global.h"
#include "sw_migration.h"

static void swCopyFolder(const QString& Source, const QString& Destination)
{
	QDir().mkpath(Destination);

	for (const QFileInfo& Entry : QDir(Source).entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden))
	{
		const QString Target = QDir(Destination).filePath(Entry.fileName());

		if (Entry.isDir())
			swCopyFolder(Entry.absoluteFilePath(), Target);
		else
			QFile::copy(Entry.absoluteFilePath(), Target);
	}
}

// use to take over settings and data stored under an earlier name
static bool swTakeOverSettings(const QString& EarlierDomain, const QString& EarlierOrganization, const QString& EarlierApplication, bool MoveData)
{
	QSettings Settings;

	const QString OrganizationDomain = QCoreApplication::organizationDomain();
	const QString OrganizationName = QCoreApplication::organizationName();
	const QString ApplicationName = QCoreApplication::applicationName();
	const QString DataPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);

	QCoreApplication::setOrganizationDomain(EarlierDomain);
	QCoreApplication::setOrganizationName(EarlierOrganization);
	QCoreApplication::setApplicationName(EarlierApplication);

	QSettings OldSettings;
	const QStringList OldKeys = OldSettings.allKeys();
	const QString OldDataPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);

	for (const QString& Key : OldKeys)
		Settings.setValue(Key, OldSettings.value(Key));

	QCoreApplication::setOrganizationDomain(OrganizationDomain);
	QCoreApplication::setOrganizationName(OrganizationName);
	QCoreApplication::setApplicationName(ApplicationName);

	// move large libraries instead of copying them
	if (QDir(OldDataPath).exists() && !QDir(DataPath).exists())
	{
		QDir().mkpath(QFileInfo(DataPath).absolutePath());

		if (!MoveData || !QDir().rename(OldDataPath, DataPath))
			swCopyFolder(OldDataPath, DataPath);
	}

	Settings.sync();

	return !OldKeys.isEmpty();
}

static void swMigrateEarlierSettings()
{
	// take over earlier application settings once
	QSettings Settings;

	if (!Settings.allKeys().isEmpty())
		return;

	// the domain names the settings on macos
	const QString EarlierDomain = QLatin1String("nathaneltitane.github.io");

	if (swTakeOverSettings(EarlierDomain, QLatin1String("StudWorx"), QLatin1String("StudWorx"), false))
		return;

	if (!swTakeOverSettings(EarlierDomain, QLatin1String("StudWorks"), QLatin1String("StudWorks"), true))
		swTakeOverSettings(EarlierDomain, QLatin1String("LeoCAD Software"), QLatin1String("LeoCAD"), false);
}

// short minifig names not yet spelled out
static const QRegularExpression swMinifigureShortName(QStringLiteral("Minifig(?!ure)"));

QString swRenameStoredName(QString Name)
{
	return Name.replace(QLatin1String("Edit.Tool.Pose"), QLatin1String("Edit.Tool.Joints")).replace(QLatin1String("View.Toolbars.Poses"), QLatin1String("View.Toolbars.Joints")).replace(QLatin1String("Pieces"), QLatin1String("Parts")).replace(QLatin1String("Piece"), QLatin1String("Part")).replace(QLatin1String("ViewSphere"), QLatin1String("ViewCube")).replace(swMinifigureShortName, QLatin1String("Minifigure"));
}

static void swMigrateRenamedKeys()
{
	// rename piece, view sphere and short minifig keys
	QSettings Settings;

	for (const QString& Key : Settings.allKeys())
	{
		if (!Key.contains(QLatin1String("Piece")) && !Key.contains(QLatin1String("ViewSphere")) && !Key.contains(swMinifigureShortName))
			continue;

		Settings.setValue(swRenameStoredName(Key), Settings.value(Key));
		Settings.remove(Key);
	}

	Settings.remove(QLatin1String("Settings/mPartColorValueLDIndex"));
	Settings.sync();
}

static void swMigrateRenamedShortcuts()
{
	// keep keys of renamed commands
	QSettings Settings;
	const QString Key = QLatin1String("Settings/Shortcuts");
	const QString OldShortcuts = QString::fromUtf8(Settings.value(Key).toByteArray());
	QStringList Lines = OldShortcuts.split(QLatin1Char('\n'));

	for (QString& Line : Lines)
	{
		const qsizetype Equals = Line.indexOf(QLatin1Char('='));

		if (Equals != -1)
			Line = swRenameStoredName(Line.left(Equals)) + Line.mid(Equals);
	}

	const QString Shortcuts = Lines.join(QLatin1Char('\n'));

	if (Shortcuts == OldShortcuts)
		return;

	Settings.setValue(Key, Shortcuts.toUtf8());
	Settings.sync();
}

static QByteArray swSerializeName(const QString& Name)
{
	QByteArray Data;
	QDataStream Stream(&Data, QIODevice::WriteOnly);
	Stream << Name;

	return Data;
}

static void swMigrateRenamedToolbars()
{
	// keep toolbar places under new names
	const QString RenamedToolbars[][2] =
	{
		{ QLatin1String("PiecesToolbar"), QLatin1String("PartsToolbar") },
		{ QLatin1String("MinifigToolbar"), QLatin1String("MinifigureToolbar") },
		{ QLatin1String("PosesToolbar"), QLatin1String("JointsToolbar") }
	};

	QSettings Settings;
	const QByteArray OldState = Settings.value("MainWindow/State").toByteArray();
	QByteArray State = OldState;

	for (const auto& [OldName, NewName] : RenamedToolbars)
		State.replace(swSerializeName(OldName), swSerializeName(NewName));

	if (State == OldState)
		return;

	Settings.setValue("MainWindow/State", State);
	Settings.sync();
}

static void swMigrateRenamedCategories()
{
	// spell out the short minifig category name
	QSettings Settings;
	const QString Key = QLatin1String("Settings/Categories");
	const QString OldCategories = QString::fromUtf8(Settings.value(Key).toByteArray());
	QString Categories = OldCategories;

	Categories.replace(QRegularExpression(QStringLiteral("^Minifig="), QRegularExpression::MultilineOption), QLatin1String("Minifigure="));

	if (Categories == OldCategories)
		return;

	Settings.setValue(Key, Categories.toUtf8());
	Settings.sync();
}

void swMigrateSettings()
{
	// register stored number lists
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	qRegisterMetaTypeStreamOperators<QList<int>>("QList<int>");
#endif

	swMigrateEarlierSettings();
	swMigrateRenamedKeys();
	swMigrateRenamedShortcuts();
	swMigrateRenamedToolbars();
	swMigrateRenamedCategories();
}
