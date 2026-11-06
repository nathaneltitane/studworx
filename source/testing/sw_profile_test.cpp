#include "sw_global.h"
#include "sw_tests.h"
#include "sw_commands.h"
#include "sw_profile.h"

// Writes inside a batch must stay readable at once and reach the settings file only when the batch
// ends, so Preferences saves the file once instead of once per key.
void swTests::ProfileBatchSavesOnce()
{
	QTemporaryDir Directory;
	QVERIFY(Directory.isValid());

	const QString OrganizationName = QCoreApplication::organizationName();
	const QString ApplicationName = QCoreApplication::applicationName();
	const QSettings::Format Format = QSettings::defaultFormat();

	QCoreApplication::setOrganizationName(QLatin1String("StudWorxTests"));
	QCoreApplication::setApplicationName(QLatin1String("ProfileBatch"));
	// windows and macos keep native settings in the registry and plists, which ignore the path
	QSettings::setDefaultFormat(QSettings::IniFormat);
	QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, Directory.path());

	const QString FileName = QSettings().fileName();

	QVERIFY(FileName.startsWith(Directory.path()));
	QVERIFY(!QFileInfo::exists(FileName));

	{
		swProfileBatch Batch;

		// a nested batch shares the open one
		{
			swProfileBatch NestedBatch;

			swSetProfileInteger(SW_PROFILE_ANTIALIASING_SAMPLES, 4);
		}

		swSetProfileString(SW_PROFILE_LANGUAGE, QLatin1String("fr"));
		swSetProfileFloat(SW_PROFILE_INSTRUCTIONS_DEFAULT_GRID_SIZE, 2.5f);

		QCOMPARE(swGetProfileInteger(SW_PROFILE_ANTIALIASING_SAMPLES), 4);
		QCOMPARE(swGetProfileString(SW_PROFILE_LANGUAGE), QLatin1String("fr"));
		QCOMPARE(swGetProfileFloat(SW_PROFILE_INSTRUCTIONS_DEFAULT_GRID_SIZE), 2.5f);
		QVERIFY(!QFileInfo::exists(FileName));

		swRemoveProfileKey(SW_PROFILE_LANGUAGE);

		QCOMPARE(swGetProfileString(SW_PROFILE_LANGUAGE), QString());
	}

	QVERIFY(QFileInfo::exists(FileName));

	QSettings Settings(FileName, QSettings::IniFormat);

	QCOMPARE(Settings.value(QLatin1String("Settings/AASamples")).toInt(), 4);
	QVERIFY(!Settings.contains(QLatin1String("Settings/Language")));

	// writes without a batch save on their own
	swSetProfileInteger(SW_PROFILE_ANTIALIASING_SAMPLES, 2);

	Settings.sync();

	QCOMPARE(Settings.value(QLatin1String("Settings/AASamples")).toInt(), 2);

	QCoreApplication::setOrganizationName(OrganizationName);
	QCoreApplication::setApplicationName(ApplicationName);
	QSettings::setDefaultFormat(Format);
}

// A first start must give every setting a line in the settings file, without replacing one already set.
void swTests::ProfileWritesEveryKey()
{
	QTemporaryDir Directory;
	QVERIFY(Directory.isValid());

	const QString OrganizationName = QCoreApplication::organizationName();
	const QString ApplicationName = QCoreApplication::applicationName();
	const QSettings::Format Format = QSettings::defaultFormat();

	QCoreApplication::setOrganizationName(QLatin1String("StudWorxTests"));
	QCoreApplication::setApplicationName(QLatin1String("ProfileWriteMissing"));
	QSettings::setDefaultFormat(QSettings::IniFormat);
	QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, Directory.path());

	swSetProfileInteger(SW_PROFILE_ANTIALIASING_SAMPLES, 8);
	swProfileWriteMissing();

	QSettings Settings;

	QCOMPARE(Settings.allKeys().size(), static_cast<int>(SW_PROFILE_KEY_COUNT));
	QCOMPARE(Settings.value(QLatin1String("Settings/AASamples")).toInt(), 8);

	QCoreApplication::setOrganizationName(OrganizationName);
	QCoreApplication::setApplicationName(ApplicationName);
	QSettings::setDefaultFormat(Format);
}

void swTests::ContextMenuDefaultResolves()
{
	for (const QString& Line : swGetDefaultContextMenu())
	{
		const QString Entry = Line.trimmed();

		if (Entry == "-" || Entry.startsWith('>'))
			continue;

		if (Entry.startsWith('@'))
			QVERIFY2(swGetContextMenuBuiltins().contains(Entry.mid(1)), qPrintable(Entry));
		else
			QVERIFY2(swFindCommand(Entry) >= 0, qPrintable(Entry));
	}

	QCOMPARE(swFindCommand("Missing.Command"), -1);
}
