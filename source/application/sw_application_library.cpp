#include "sw_global.h"
#include "sw_application.h"
#include "sw_library.h"
#include "sw_project.h"
#include "sw_window_main.h"
#include "sw_dialog_preferences.h"
#include "sw_widget_preview.h"
#include "sw_http.h"

bool swApplication::LoadPartsLibrary(const QList<QPair<QString, bool>>& LibraryPaths, bool OnlyUsePaths, swStudStyle StudStyle, bool StudCylinderColorEnabled)
{
	if (mLibrary == nullptr)
		mLibrary = new swPartsLibrary(StudStyle, StudCylinderColorEnabled);

	const bool ShowProgress = gMainWindow != nullptr;

	if (!OnlyUsePaths)
	{
		const QString PathEnvironment = GetEnvironmentLibraryPath();

		if (!PathEnvironment.isEmpty())
			return mLibrary->Load(PathEnvironment, ShowProgress, false);

		QString CustomPath = swGetProfileString(SW_PROFILE_PARTS_LIBRARY);

		if (!CustomPath.isEmpty())
			return mLibrary->Load(CustomPath, ShowProgress, false);
	}

	for (const QPair<QString, bool>& LibraryPathEntry : LibraryPaths)
		if (mLibrary->Load(LibraryPathEntry.first, ShowProgress, LibraryPathEntry.second))
			return true;

	return false;
}

// use to name the library archive
QString swApplication::GetDownloadLibraryPath()
{
	return QDir::cleanPath(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/complete.zip");
}

// use to read the library set in the environment
QString swApplication::GetEnvironmentLibraryPath()
{
	const QString LibraryPath = qEnvironmentVariable("LDRAW_DIRECTORY");

	return LibraryPath.isEmpty() ? qEnvironmentVariable("LDRAWDIR") : LibraryPath;
}

// use to name the folder of the library in use
QString swApplication::GetLibraryDirectory()
{
	const QString LibraryPath = gApplication && gApplication->mLibrary ? gApplication->mLibrary->GetLibraryPath() : swGetProfileString(SW_PROFILE_PARTS_LIBRARY);

	if (LibraryPath.isEmpty())
		return QString();

	const QFileInfo LibraryInfo(LibraryPath);

	return LibraryInfo.isFile() ? LibraryInfo.absolutePath() : LibraryInfo.absoluteFilePath();
}

// use to download the library archive
bool swApplication::DownloadPartsLibrary(const QString& FileName)
{
	const std::optional<QByteArray> Data = swDownloadWithProgress(gMainWindow, QLatin1String(SW_LIBRARY_DOWNLOAD_URL), tr("Download Parts Library"), tr("Downloading the LDraw parts library..."));

	if (!Data)
		return false;

	if (Data->isEmpty())
	{
		QMessageBox::warning(gMainWindow, tr("Download Parts Library"), tr("Could not download the parts library from %1.").arg(QLatin1String(SW_LIBRARY_DOWNLOAD_URL)));
		return false;
	}

	QDir().mkpath(QFileInfo(FileName).absolutePath());

	QSaveFile File(FileName);

	if (!File.open(QIODevice::WriteOnly) || File.write(*Data) != Data->size() || !File.commit())
	{
		QMessageBox::warning(gMainWindow, tr("Download Parts Library"), tr("Could not save the parts library to '%1'.").arg(FileName));
		return false;
	}

	return true;
}

// use to get a missing library
bool swApplication::SetupPartsLibrary()
{
	const QString DownloadPath = GetDownloadLibraryPath();
	const QString Question = tr("StudWorx could not find an LDraw parts library.\n\nDownload the official library (about 140 MB) from library.ldraw.org into '%1'?").arg(QDir::toNativeSeparators(DownloadPath));

	if (QMessageBox::question(gMainWindow, tr("Load Parts Library"), Question, QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes && DownloadPartsLibrary(DownloadPath))
	{
		if (mLibrary->Load(DownloadPath, true, true))
			return true;

		QMessageBox::warning(gMainWindow, tr("Load Parts Library"), tr("Could not open the parts library '%1'.").arg(DownloadPath));
	}

	for (;;)
	{
		const QString FileName = QFileDialog::getOpenFileName(gMainWindow, tr("Open Parts Library Archive"), QDir::homePath(), tr("Parts Library Archives (*.zip);;All Files (*)"));

		if (FileName.isEmpty())
			break;

		if (mLibrary->Load(FileName, true, false))
		{
			swSetProfileString(SW_PROFILE_PARTS_LIBRARY, FileName);
			return true;
		}

		QMessageBox::warning(gMainWindow, tr("Load Parts Library"), tr("Could not open the parts library '%1'.").arg(FileName));
	}

	QMessageBox::warning(gMainWindow, tr("Load Parts Library"), tr("No parts library was downloaded or set.\n\nDownload one from %1 and set it under Preferences \u203a Parts Library the next time StudWorx is started.").arg(QLatin1String(SW_LIBRARY_DOWNLOAD_URL)));

	return false;
}

// use to reload the library
bool swApplication::ReloadPartsLibrary(const QString& LibraryPath, bool OfficialLibrary)
{
	if (!mLibrary->Reload(LibraryPath, OfficialLibrary))
		return false;

	std::vector<Project*> Projects = mLibrary->GetExternalProjects();
	Projects.insert(Projects.begin(), mProject);

	std::vector<std::pair<swPartDataReference, swPartDataReference>> Replacements;

	for (Project* CurrentProject : Projects)
	{
		std::vector<std::pair<swPartDataReference, swPartDataReference>> ProjectReplacements = mLibrary->RebindProjectPartData(CurrentProject);

		if (ProjectReplacements.empty())
			continue;

		std::vector<swModel*> UpdatedModels;

		for (const std::unique_ptr<swModel>& Model : CurrentProject->GetModels())
			Model->UpdatePartData(UpdatedModels);

		std::move(ProjectReplacements.begin(), ProjectReplacements.end(), std::back_inserter(Replacements));
	}

	if (!gMainWindow)
		return true;

	for (const std::pair<swPartDataReference, swPartDataReference>& Replacement : Replacements)
	{
		if (gMainWindow->GetInsertPartData() == Replacement.first)
			gMainWindow->SetInsertPartData(Replacement.second.Get());

		gMainWindow->GetPreviewWidget()->RebindPartData(Replacement.first.Get(), Replacement.second.Get());
	}

	// vanished insert part
	if (gMainWindow->GetInsertPartData() && mLibrary->IsRetiredPartData(gMainWindow->GetInsertPartData().Get()))
		gMainWindow->SetInsertPartData(nullptr);

	gMainWindow->UpdateCategories();
	gMainWindow->UpdateInUseCategory();
	gMainWindow->UpdateMinifigureBuilder();
	swView::UpdateProjectViews(mProject);

	return true;
}
