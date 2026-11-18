#include "sw_global.h"
#include "sw_application.h"
#include "sw_library.h"
#include "sw_library_shadow.h"
#include "sw_project.h"
#include "sw_object_part.h"
#include "sw_window_main.h"
#include "sw_dialog_preferences.h"
#include "sw_widget_selection_part.h"
#include "sw_object_camera.h"
#include "sw_widget_preview.h"
#include "sw_window_utilities.h"
#include "sw_dialog_render.h"

#ifdef Q_OS_WIN
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#include <QtPlatformHeaders\QWindowsWindowFunctions>
#endif
#endif

swApplication* gApplication;

swApplication::swApplication(int& Argc, char** Argv)
	: QApplication(Argc, Argv)
{
	setApplicationDisplayName(QLatin1String("StudWorx"));

	gApplication = this;
	mDefaultStyle = style()->objectName();
	mDefaultPalette = QApplication::palette();

	swProfileInitialize();
	swProfileWriteMissing();

	mPreferences.LoadDefaults();

	installEventFilter(new swWindowGeometryFilter(this));
	installEventFilter(new swInterfaceFitFilter(this));
}

swApplication::~swApplication()
{
	gApplication = nullptr;
}

void swApplication::UpdateStyle()
{
	if (mPreferences.mColorTheme == swColorTheme::Dark)
	{
		QStyle* Style = QStyleFactory::create(QLatin1String("fusion"));

		if (!Style)
			return;

		QApplication::setStyle(new swInterfaceStyle(Style, true));

		QPalette Palette = QApplication::palette();

		Palette.setColor(QPalette::Window, QColor(49, 52, 55));
		Palette.setColor(QPalette::WindowText, QColor(240, 240, 240));
		Palette.setColor(QPalette::Base, QColor(35, 38, 41));
		Palette.setColor(QPalette::AlternateBase, QColor(44, 47, 50));
		Palette.setColor(QPalette::ToolTipBase, QColor(224, 224, 244));
		Palette.setColor(QPalette::ToolTipText, QColor(58, 58, 58));
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
		Palette.setColor(QPalette::PlaceholderText, QColor(100, 100, 100));
#endif
		Palette.setColor(QPalette::Text, QColor(224, 224, 224));
		Palette.setColor(QPalette::Button, QColor(45, 48, 51));
		Palette.setColor(QPalette::ButtonText, QColor(224, 224, 244));
		Palette.setColor(QPalette::Light, QColor(65, 65, 65));
		Palette.setColor(QPalette::Midlight, QColor(62, 62, 62));
		Palette.setColor(QPalette::Dark, QColor(35, 35, 35));
		Palette.setColor(QPalette::Mid, QColor(50, 50, 50));
		Palette.setColor(QPalette::Shadow, QColor(20, 20, 20));

//		Palette.setColor(QPalette::Highlight, QColor(46, 108, 219));
		Palette.setColor(QPalette::Highlight, QColor(41, 128, 185));
		Palette.setColor(QPalette::HighlightedText, QColor(232, 232, 232));
		Palette.setColor(QPalette::Link, QColor(41, 128, 185));

		Palette.setColor(QPalette::Disabled, QPalette::Text, QColor(128, 128, 128));
		Palette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(128, 128, 128));
		Palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(128, 128, 128));

		QApplication::setPalette(Palette);
		mCustomStyleApplied = true;

		QFile StylesheetFile(QLatin1String(":/stylesheet/stylesheet.qss"));

		if (StylesheetFile.open(QIODevice::ReadOnly))
		{
			QString Stylesheet = QString::fromLatin1(StylesheetFile.readAll());
			qApp->setStyleSheet(Stylesheet);
		}
	}
	else if (mCustomStyleApplied || !dynamic_cast<swInterfaceStyle*>(QApplication::style()))
	{
		// restore the system style and colors
		QApplication::setStyle(new swInterfaceStyle(QStyleFactory::create(mDefaultStyle)));

		if (mCustomStyleApplied)
		{
			QApplication::setPalette(mDefaultPalette);
			qApp->setStyleSheet(QString());
		}

		mCustomStyleApplied = false;
	}
}

bool swApplication::IsDarkTheme() const
{
	if (mPreferences.mColorTheme == swColorTheme::Dark)
		return true;

	return IsSystemPaletteDark();
}

void swApplication::SaveTabLayout() const
{
	if (!gMainWindow || !mProject || mProject->GetFileName().isEmpty())
		return;

	QSettings Settings;
	QByteArray TabLayout = gMainWindow->GetTabLayout();

	Settings.setValue(GetTabLayoutKey(), TabLayout);
}

QString swApplication::GetTabLayoutKey() const
{
	if (mProject)
	{
		QString FileName = mProject->GetFileName();
		if (!FileName.isEmpty())
		{
			FileName.replace('\\', '?');
			FileName.replace('/', '?');
			return QString("TabLayouts/%1").arg(FileName);
		}
	}

	return QString();
}

void swApplication::SetProject(Project* Project)
{
	SaveTabLayout();

	if (gMainWindow)
	{
		gMainWindow->RemoveAllModelTabs();

		// instructions belong to the replaced project
		gMainWindow->CloseInstructionsDialog();

		if (gMainWindow->GetPreviewWidget())
			gMainWindow->GetPreviewWidget()->ClearPreview();
	}

	delete mProject;
	mProject = Project;
	mProject->UpdateFileWatcher();

	if (gMainWindow)
		gMainWindow->ScheduleAssetUpdate();

	Project->SetActiveModel(0, true);
	swGetPartsLibrary()->RemoveTemporaryPartData();

	if (mProject && !mProject->GetFileName().isEmpty() && mPreferences.mRestoreTabLayout)
	{
		QSettings Settings;
		QByteArray TabLayout = Settings.value(GetTabLayoutKey()).toByteArray();

		if (gMainWindow)
			gMainWindow->RestoreTabLayout(TabLayout);
	}
}

void swApplication::SetClipboard(const QByteArray& Clipboard)
{
	mClipboard = Clipboard;
	gMainWindow->UpdatePaste(!mClipboard.isEmpty());
}

void swApplication::ExportClipboard(const QByteArray& Clipboard)
{
	QMimeData* MimeData = new QMimeData();

	MimeData->setData("application/vnd.studworx-clipboard", Clipboard);
	QApplication::clipboard()->setMimeData(MimeData);

	SetClipboard(Clipboard);
}

// use to transfer settings without the interface
static bool swTransferSettings(const swCommandLineOptions& Options, QTextStream& StdErr)
{
	bool TransferOK = true;

	auto ReportError = [&StdErr, &TransferOK](const QString& Message)
	{
		StdErr << Message;
		StdErr.flush();
		TransferOK = false;
	};

	// import before export
	if (!Options.SettingsImportName.isEmpty())
	{
		QSettings SettingsFile(Options.SettingsImportName, QSettings::IniFormat);

		if (!QFileInfo(Options.SettingsImportName).isReadable() || SettingsFile.status() != QSettings::NoError)
			ReportError(swApplication::tr("Could not load settings file '%1'.\n").arg(Options.SettingsImportName));
		else
		{
			QSettings Settings;
			const QStringList Keys = SettingsFile.allKeys();

			for (const QString& Key : Keys)
				Settings.setValue(Key, SettingsFile.value(Key));

			Settings.sync();
		}
	}

	if (!Options.KeyboardShortcutsImportName.isEmpty())
	{
		if (!gKeyboardShortcuts.Load(Options.KeyboardShortcutsImportName))
			ReportError(swApplication::tr("Could not load keyboard shortcuts file '%1'.\n").arg(Options.KeyboardShortcutsImportName));
		else
			swSaveDefaultKeyboardShortcuts();
	}

	if (!Options.MouseShortcutsImportName.isEmpty())
	{
		if (!gMouseShortcuts.Load(Options.MouseShortcutsImportName))
			ReportError(swApplication::tr("Could not load mouse shortcuts file '%1'.\n").arg(Options.MouseShortcutsImportName));
		else
			swSaveDefaultMouseShortcuts();
	}

	if (!Options.CategoriesImportName.isEmpty())
	{
		std::vector<swLibraryCategory> Categories;

		if (!swLoadCategories(Options.CategoriesImportName, Categories) || Categories.empty())
			ReportError(swApplication::tr("Could not load categories file '%1'.\n").arg(Options.CategoriesImportName));
		else
		{
			gCategories = Categories;
			swSaveDefaultCategories();
		}
	}

	if (!Options.SettingsExportName.isEmpty())
	{
		QSettings Settings;
		QSettings SettingsFile(Options.SettingsExportName, QSettings::IniFormat);
		const QStringList Keys = Settings.allKeys();

		SettingsFile.clear();

		for (const QString& Key : Keys)
			SettingsFile.setValue(Key, Settings.value(Key));

		SettingsFile.sync();

		if (SettingsFile.status() != QSettings::NoError)
			ReportError(swApplication::tr("Could not save settings file '%1'.\n").arg(Options.SettingsExportName));
	}

	if (!Options.KeyboardShortcutsExportName.isEmpty())
	{
		swLoadDefaultKeyboardShortcuts();

		if (!gKeyboardShortcuts.Save(Options.KeyboardShortcutsExportName))
			ReportError(swApplication::tr("Could not save keyboard shortcuts file '%1'.\n").arg(Options.KeyboardShortcutsExportName));
	}

	if (!Options.MouseShortcutsExportName.isEmpty())
	{
		swLoadDefaultMouseShortcuts();

		if (!gMouseShortcuts.Save(Options.MouseShortcutsExportName))
			ReportError(swApplication::tr("Could not save mouse shortcuts file '%1'.\n").arg(Options.MouseShortcutsExportName));
	}

	if (!Options.CategoriesExportName.isEmpty())
	{
		swLoadDefaultCategories();

		if (!swSaveCategories(Options.CategoriesExportName, gCategories))
			ReportError(swApplication::tr("Could not save categories file '%1'.\n").arg(Options.CategoriesExportName));
	}

	return TransferOK;
}

swStartupMode swApplication::Initialize(const QList<QPair<QString, bool>>& LibraryPaths)
{
	mDefaultLibraryPaths = LibraryPaths;

	swCommandLineOptions Options = ParseCommandLineOptions();
	QTextStream StdErr(stderr, QIODevice::WriteOnly);
	QTextStream StdOut(stdout, QIODevice::WriteOnly);

	if (!Options.StdErr.isEmpty())
	{
		StdErr << Options.StdErr;
		StdErr.flush();
	}

	if (!Options.StdOut.isEmpty())
	{
		StdOut << Options.StdOut;
		StdOut.flush();
	}

	if (!Options.ParseOK)
		return swStartupMode::Error;

	if (Options.Exit)
		return swStartupMode::Success;

	if (Options.HasSettingsTransfer())
		return swTransferSettings(Options, StdErr) ? swStartupMode::Success : swStartupMode::Error;

	const bool SaveAndExit = Options.HasSaveAndExit();

#ifdef Q_OS_IOS
	if (Options.SaveImage)
	{
		StdErr << tr("Command line image export is not supported on iOS.\n");
		return swStartupMode::Error;
	}
#endif

#ifndef Q_OS_IOS
	const bool CreateRenderResources = !SaveAndExit || Options.SaveImage || Options.SaveKMZ || Options.SaveHTML || Options.SavePOVRay || Options.RenderPOVRay;
	const swResult<void> RendererResult = swContext::InitializeRenderer(CreateRenderResources);

	if (!RendererResult)
	{
		StdErr << RendererResult.error() << '\n';

		if (!SaveAndExit && !Options.PrintMenus)
			QMessageBox::critical(nullptr, tr("Initialize Renderer"), RendererResult.error());

		return swStartupMode::Error;
	}

	emit RendererInitialized();
#endif

	bool OutputFailed = false;

	if (!SaveAndExit)
	{
		UpdateStyle();

		gMainWindow = new swMainWindow();

		swLoadDefaultKeyboardShortcuts();
		swLoadDefaultMouseShortcuts();
	}

	mPreferences.mShadingMode = Options.ShadingMode;
	mPreferences.mLightingStyle = Options.LightingStyle;
	mPreferences.mLineWidth = Options.LineWidth;
	mPreferences.mStudCylinderColorEnabled = Options.StudCylinderColorEnabled;
	mPreferences.mStudCylinderColor = Options.StudCylinderColor;
	mPreferences.mPartEdgeColorEnabled = Options.PartEdgeColorEnabled;
	mPreferences.mPartEdgeColor = Options.PartEdgeColor;
	mPreferences.mBlackEdgeColorEnabled = Options.BlackEdgeColorEnabled;
	mPreferences.mBlackEdgeColor = Options.BlackEdgeColor;
	mPreferences.mDarkEdgeColorEnabled = Options.DarkEdgeColorEnabled;
	mPreferences.mDarkEdgeColor = Options.DarkEdgeColor;
	mPreferences.mPartEdgeContrast = Options.PartEdgeContrast;
	mPreferences.mPartColorValueLDIndex = Options.PartColorValueLDIndex;
	mPreferences.mAutomateEdgeColor = Options.AutomateEdgeColor;

	const QString UnofficialLibraryPath = Options.UnofficialLibraryPath.isEmpty() ? swGetProfileString(SW_PROFILE_UNOFFICIAL_LIBRARY) : Options.UnofficialLibraryPath;

	if (mLibrary == nullptr)
		mLibrary = new swPartsLibrary(Options.StudStyle, Options.StudCylinderColorEnabled);

	mLibrary->SetUnofficialArchivePath(UnofficialLibraryPath);

	if (!LoadPartsLibrary(Options.LibraryPaths.isEmpty() ? LibraryPaths : Options.LibraryPaths, !Options.LibraryPaths.isEmpty(), Options.StudStyle, Options.StudCylinderColorEnabled))
	{
		// a menu dump has no one to answer the setup dialog

		if (!gMainWindow || Options.PrintMenus)
		{
			StdErr << tr("StudWorx could not find an LDraw parts library.\n\nDownload one from %1 and pass it with --library.").arg(QLatin1String(SW_LIBRARY_DOWNLOAD_URL)) << "\n";
			StdErr.flush();
			mLibrary->LoadEmptyLibrary();
		}
		else if (!SetupPartsLibrary())
			mLibrary->LoadEmptyLibrary();
	}

	if (!UnofficialLibraryPath.isEmpty() && !mLibrary->HasUnofficialArchive())
	{
		StdErr << tr("Could not open the unofficial library archive '%1'.").arg(UnofficialLibraryPath) << "\n";
		StdErr.flush();
	}

	const swColorSource ColorSource = swGetColorSource();
	const QString ColorConfigurationPath = swGetColorConfigurationPath(ColorSource);
	// an unset file uses the bundled copy so only a lost file is missing
	const bool ColorConfigurationMissing = ColorSource != swColorSource::Library && !ColorConfigurationPath.isEmpty() && !swColorConfigurationExists(ColorConfigurationPath);

	if (ColorConfigurationMissing && !gMainWindow)
	{
		StdErr << tr("The %1 file '%2' was not found, using the bundled copy.").arg(swGetColorConfigurationName(ColorSource), ColorConfigurationPath) << "\n";
		StdErr.flush();
	}

	const QString ShadowLibraryPath = Options.ShadowLibraryPath.isEmpty() ? swGetProfileString(SW_PROFILE_SHADOW_LIBRARY) : Options.ShadowLibraryPath;

	if (!ShadowLibraryPath.isEmpty() && !swShadowLibrary::SetPath(ShadowLibraryPath))
	{
		StdErr << tr("Could not find a shadow library in '%1'.").arg(ShadowLibraryPath) << "\n";
		StdErr.flush();
	}

	if (!SaveAndExit)
		gMainWindow->CreateWidgets();

	Project* NewProject = new Project();
	SetProject(NewProject);

	if (!SaveAndExit && Options.ProjectName.isEmpty() && swGetProfileInteger(SW_PROFILE_AUTO_LOAD_MOST_RECENT))
		Options.ProjectName = swGetProfileString(SW_PROFILE_RECENT_FILE1);

	bool ProjectLoaded = false;

	if (!Options.ProjectName.isEmpty())
	{
		if (gMainWindow)
			gMainWindow->OpenProject(Options.ProjectName);
		else
		{
			Project* LoadedProject = new Project();

			if (LoadedProject->Load(Options.ProjectName, false))
			{
				SetProject(LoadedProject);
				ProjectLoaded = true;
			}
			else
			{
				delete LoadedProject;
			}
		}
	}

	if (ProjectLoaded)
	{
		if (!Options.ModelName.isEmpty())
			mProject->SetActiveModel(Options.ModelName, true);

		std::unique_ptr<swView> ActiveView;

		if (Options.SaveImage || Options.SavePOVRay || Options.RenderPOVRay)
		{
			swModel* Model;

			if (!Options.ModelName.isEmpty())
			{
				Model = mProject->GetModel(Options.ModelName);

				if (!Model)
				{
					StdErr << tr("Error: model '%1' does not exist.\n").arg(Options.ModelName);
					return swStartupMode::Error;
				}
			}
			else
				Model = mProject->GetMainModel();

			ActiveView = std::unique_ptr<swView>(new swView(swViewType::View, Model));

			ActiveView->SetOffscreenContext();
			ActiveView->MakeCurrent();
		}

		if (ActiveView)
			ActiveView->SetSize(Options.ImageWidth, Options.ImageHeight);

		if (ActiveView)
		{
			if (!Options.CameraName.isEmpty())
				ActiveView->SetCamera(Options.CameraName);
			else
			{
				ActiveView->SetCameraProjection(Options.Orthographic ? swCameraProjection::Orthographic : swCameraProjection::Perspective);

				if (Options.SetFoV)
					ActiveView->GetCamera()->m_fovy = Options.FoV;

				if (Options.SetZPlanes)
				{
					swCamera* Camera = ActiveView->GetCamera();

					Camera->m_zNear = Options.ZPlanes[0];
					Camera->m_zFar = Options.ZPlanes[1];
				}

				if (Options.Viewpoint != swViewpoint::Count)
					ActiveView->SetViewpoint(Options.Viewpoint);
				else if (Options.SetCameraAngles)
					ActiveView->SetCameraAngles(Options.CameraLatLon[0], Options.CameraLatLon[1]);
				else if (Options.SetCameraPosition)
					ActiveView->SetViewpoint(Options.CameraPosition[0], Options.CameraPosition[1], Options.CameraPosition[2]);
			}
		}

		if (Options.SaveFlatten)
		{
			mProject->GetMainModel()->FlattenModel();

			const swResult<void> FlattenResult = mProject->SaveCopy(Options.SaveFlattenName);

			if (FlattenResult)
				StdOut << tr("Saved '%1'.\n").arg(Options.SaveFlattenName);
			else
			{
				OutputFailed = true;
				StdErr << FlattenResult.error() << '\n';
			}
		}

		if (Options.SaveUndo)
		{
			while (mProject->GetMainModel()->UndoAction());

			const swResult<void> UndoResult = mProject->SaveCopy(Options.SaveUndoName);

			if (UndoResult)
				StdOut << tr("Saved '%1'.\n").arg(Options.SaveUndoName);
			else
			{
				OutputFailed = true;
				StdErr << UndoResult.error() << '\n';
			}
		}

		if (Options.SaveImage)
		{
			swModel* ActiveModel = ActiveView->GetModel();

			if (Options.ImageName.isEmpty())
				Options.ImageName = mProject->GetImageFileName(true);

			const QString ImageSuffix = QFileInfo(Options.ImageName).suffix().toLower();
			swImageBackground ImageBackground = Options.TransparentImage ? swImageBackground::Transparent : swImageBackground::Opaque;

			// only png and gif files keep transparency
			if (Options.TransparentImage && ImageSuffix != QLatin1String("png") && ImageSuffix != QLatin1String("gif"))
			{
				StdErr << tr("--transparent is ignored for '%1' files: only PNG and GIF files keep transparency.\n").arg(ImageSuffix);
				ImageBackground = swImageBackground::Opaque;
			}

			ActiveView->SetBackgroundMode(ImageBackground == swImageBackground::Transparent ? swBackgroundMode::Transparent : swBackgroundMode::Opaque);

			if (Options.ImageEnd < Options.ImageStart)
				Options.ImageEnd = Options.ImageStart;
			else if (Options.ImageStart > Options.ImageEnd)
				Options.ImageStart = Options.ImageEnd;

			if ((Options.ImageStart == 0) && (Options.ImageEnd == 0))
				Options.ImageStart = Options.ImageEnd = ActiveModel->GetCurrentStep();
			else if ((Options.ImageStart == 0) && (Options.ImageEnd != 0))
				Options.ImageStart = Options.ImageEnd;
			else if ((Options.ImageStart != 0) && (Options.ImageEnd == 0))
				Options.ImageEnd = Options.ImageStart;

			if (Options.ImageStart > 255)
				Options.ImageStart = 255;

			if (Options.ImageEnd > 255)
				Options.ImageEnd = 255;

			QString Frame;

			if (Options.ImageStart != Options.ImageEnd)
			{
				QString Extension = QFileInfo(Options.ImageName).suffix();
				Frame = Options.ImageName.left(Options.ImageName.length() - Extension.length() - 1) + QLatin1String("%1.") + Extension;
			}
			else
				Frame = Options.ImageName;

			mPreferences.mFadeSteps = Options.FadeSteps;
			if (Options.SetFadeStepsColor)
				mPreferences.mFadeStepsColor = Options.FadeStepsColor;
			mPreferences.mHighlightNewParts = Options.ImageHighlight;
			if (Options.SetHighlightColor)
				mPreferences.mHighlightNewPartsColor = Options.HighlightColor;

			ActiveModel->WaitForAssets();

			// skip pictures of files without parts
			if (ActiveModel->GetPartsBoundingBoxPoints(swGeometryBoundsMode::AvailableOnly).empty())
			{
				StdErr << tr("The file '%1' has no parts to render.").arg(Options.ProjectName) << '\n';

				return swStartupMode::Error;
			}

			const bool UsesCameraPosition = Options.SetCameraPosition && Options.Viewpoint == swViewpoint::Count && !Options.SetCameraAngles;

			if (Options.CameraName.isEmpty() && !UsesCameraPosition)
			{
				const swStep FitStep = Options.ImageStart == Options.ImageEnd ? Options.ImageStart : ActiveModel->GetLastStep();

				ActiveView->ZoomExtentsAtStep(FitStep, swGeometryBoundsMode::AvailableOnly, !Options.SetZPlanes);
			}

			auto ProgressCallback = [&StdOut](const QString& FileName)
			{
				StdOut << tr("Saved '%1'.\n").arg(FileName);
			};

			const swResult<void> ImagesSaved = ActiveView->SaveStepImages(Frame, Options.ImageStart != Options.ImageEnd, Options.ImageStart, Options.ImageEnd, ImageBackground, ProgressCallback);

			if (!ImagesSaved)
			{
				StdErr << ImagesSaved.error() << '\n';

				return swStartupMode::Error;
			}
		}

		if (Options.SavePOVRay)
		{
			swModel* ActiveModel = ActiveView->GetModel();
			QString FileName = !Options.SavePOVRayName.isEmpty() ? Options.SavePOVRayName : Options.ProjectName;
			const QString Extension = QFileInfo(FileName).suffix().toLower();

			if (Extension.isEmpty())
				FileName += QLatin1String(".pov");
			else if (Extension != QLatin1String("pov"))
				FileName = FileName.left(FileName.length() - Extension.length() - 1) + QLatin1String(".pov");

			ActiveModel->WaitForAssets();

			const bool UsesCameraPosition = Options.SetCameraPosition && Options.Viewpoint == swViewpoint::Count && !Options.SetCameraAngles;

			if (Options.CameraName.isEmpty() && !UsesCameraPosition)
				ActiveView->ZoomExtentsAtStep(ActiveModel->GetCurrentStep(), swGeometryBoundsMode::AvailableOnly, !Options.SetZPlanes);

			swResult<void> ExportResult = mProject->ExportPOVRay(FileName, ActiveView->GetCamera());

			if (ExportResult)
				StdOut << tr("Saved '%1'.\n").arg(FileName);
			else
			{
				OutputFailed = true;

				if (!ExportResult.error().isEmpty())
					StdErr << ExportResult.error() << '\n';
			}
		}

		if (Options.RenderPOVRay)
		{
			swModel* ActiveModel = ActiveView->GetModel();
			QString FileName = !Options.RenderPOVRayName.isEmpty() ? Options.RenderPOVRayName : Options.ProjectName;
			const QString Extension = QFileInfo(FileName).suffix().toLower();

			if (Extension.isEmpty())
				FileName += QLatin1String(".png");
			else if (Extension != QLatin1String("png"))
				FileName = FileName.left(FileName.length() - Extension.length() - 1) + QLatin1String(".png");

			const swPOVRayProgram Program = swFindPOVRay();
			const bool UseLGEO = (ActiveModel->GetPOVRayOptions().UseLGEO || swGetProfileInteger(SW_PROFILE_RENDER_PARTS) == 1) && !swFindLGEOPath().isEmpty();
			QTemporaryDir TemporaryDir;

			if (!Program.Supported)
			{
				OutputFailed = true;
				StdErr << swGetPOVRayMissingMessage(Program) << '\n';
			}
			else if (UseLGEO && !swPOVRayHasIncludeFiles(Program))
			{
				OutputFailed = true;
				StdErr << tr("LGEO parts need the standard POV-Ray include files such as colors.inc, which POV-Ray %1 could not find.\n").arg(Program.Version);
			}
			else if (!TemporaryDir.isValid())
			{
				OutputFailed = true;
				StdErr << tr("Could not create a temporary folder for the POV-Ray scene.\n");
			}
			else
			{
				const QString SceneName = TemporaryDir.filePath(QLatin1String("scene.pov"));

				ActiveModel->WaitForAssets();

				const bool UsesCameraPosition = Options.SetCameraPosition && Options.Viewpoint == swViewpoint::Count && !Options.SetCameraAngles;

				if (Options.CameraName.isEmpty() && !UsesCameraPosition)
					ActiveView->ZoomExtentsAtStep(ActiveModel->GetCurrentStep(), swGeometryBoundsMode::AvailableOnly, !Options.SetZPlanes);

				swResult<void> ExportResult = mProject->ExportPOVRay(SceneName, ActiveView->GetCamera(), SW_STEP_MAXIMUM, Options.TransparentImage);

				if (!ExportResult)
				{
					OutputFailed = true;

					if (!ExportResult.error().isEmpty())
						StdErr << ExportResult.error() << '\n';
				}
				else
				{
					QStringList Arguments;

					Arguments.append(QString::fromLatin1("+I\"%1\"").arg(SceneName));
					Arguments.append(QString::fromLatin1("+O\"%1\"").arg(QFileInfo(FileName).absoluteFilePath()));
					Arguments.append(QString::fromLatin1("+W%1").arg(Options.ImageWidth));
					Arguments.append(QString::fromLatin1("+H%1").arg(Options.ImageHeight));

					if (Options.TransparentImage || swGetProfileInteger(SW_PROFILE_RENDER_TRANSPARENT))
						Arguments.append("+UA");

					Arguments.append(swGetPOVRayArguments(Program, swGetProfileRenderQuality(), UseLGEO));

					if (Program.Bundled)
					{
						Arguments.append("+FN");
						Arguments.append("-D");
					}

					QProcess Process;

					Process.setEnvironment(swGetPOVRayEnvironment());
					Process.setProcessChannelMode(QProcess::MergedChannels);
					Process.start(Program.Path, Arguments);

					// only the render start is timed
					if (!Process.waitForStarted() || !Process.waitForFinished(-1) || Process.exitStatus() != QProcess::NormalExit || Process.exitCode() != 0 || !QFileInfo::exists(FileName))
					{
						OutputFailed = true;
						StdErr << tr("Could not render '%1' with POV-Ray:\n").arg(FileName);

						const QStringList OutputLines = QString::fromLocal8Bit(Process.readAll()).trimmed().split('\n');

						StdErr << OutputLines.mid(std::max(0, static_cast<int>(OutputLines.size()) - 20)).join('\n') << '\n';
					}
					else
						StdOut << tr("Saved '%1'.\n").arg(FileName);
				}
			}
		}

		if (Options.SaveWavefront)
		{
			QString FileName;

			if (!Options.SaveWavefrontName.isEmpty())
				FileName = Options.SaveWavefrontName;
			else
				FileName = Options.ProjectName;

			QString Extension = QFileInfo(FileName).suffix().toLower();

			if (Extension.isEmpty())
			{
				FileName += ".obj";
			}
			else if (Extension != "obj")
			{
				FileName = FileName.left(FileName.length() - Extension.length() - 1);
				FileName += ".obj";
			}

			swResult<void> ExportResult = mProject->ExportWavefront(FileName);

			if (ExportResult)
				StdOut << tr("Saved '%1'.\n").arg(FileName);
			else
			{
				OutputFailed = true;

				if (!ExportResult.error().isEmpty())
					StdErr << ExportResult.error() << '\n';
			}
		}

		if (Options.Save3DS)
		{
			QString FileName;

			if (!Options.Save3DSName.isEmpty())
				FileName = Options.Save3DSName;
			else
				FileName = Options.ProjectName;

			QString Extension = QFileInfo(FileName).suffix().toLower();

			if (Extension.isEmpty())
			{
				FileName += ".3ds";
			}
			else if (Extension != "3ds")
			{
				FileName = FileName.left(FileName.length() - Extension.length() - 1);
				FileName += ".3ds";
			}

			swResult<void> ExportResult = mProject->Export3DStudio(FileName);

			if (ExportResult)
				StdOut << tr("Saved '%1'.\n").arg(FileName);
			else
			{
				OutputFailed = true;

				if (!ExportResult.error().isEmpty())
					StdErr << ExportResult.error() << '\n';
			}
		}

		if (Options.SaveCOLLADA)
		{
			QString FileName;

			if (!Options.SaveCOLLADAName.isEmpty())
				FileName = Options.SaveCOLLADAName;
			else
				FileName = Options.ProjectName;

			QString Extension = QFileInfo(FileName).suffix().toLower();

			if (Extension.isEmpty())
			{
				FileName += ".dae";
			}
			else if (Extension != "dae")
			{
				FileName = FileName.left(FileName.length() - Extension.length() - 1);
				FileName += ".dae";
			}

			swResult<void> ExportResult = mProject->ExportCOLLADA(FileName);

			if (ExportResult)
				StdOut << tr("Saved '%1'.\n").arg(FileName);
			else
			{
				OutputFailed = true;

				if (!ExportResult.error().isEmpty())
					StdErr << ExportResult.error() << '\n';
			}
		}

		if (Options.SaveKMZ)
		{
			QString FileName;

			if (!Options.SaveKMZName.isEmpty())
				FileName = Options.SaveKMZName;
			else
				FileName = Options.ProjectName;

			QString Extension = QFileInfo(FileName).suffix().toLower();

			if (Extension.isEmpty())
			{
				FileName += ".kmz";
			}
			else if (Extension != "kmz")
			{
				FileName = FileName.left(FileName.length() - Extension.length() - 1);
				FileName += ".kmz";
			}

			if (Options.SetKMZGeometry)
				mPreferences.mKMZVisibleSurfaces = Options.KMZVisibleSurfaces;

			swResult<void> ExportResult = mProject->ExportKMZ(FileName);

			if (ExportResult)
				StdOut << tr("Saved '%1'.\n").arg(FileName);
			else
			{
				OutputFailed = true;

				if (!ExportResult.error().isEmpty())
					StdErr << ExportResult.error() << '\n';
			}
		}

		if (Options.SaveCSV)
		{
			QString FileName;

			if (!Options.SaveCSVName.isEmpty())
				FileName = Options.SaveCSVName;
			else
				FileName = Options.ProjectName;

			QString Extension = QFileInfo(FileName).suffix().toLower();

			if (Extension.isEmpty())
			{
				FileName += ".csv";
			}
			else if (Extension != "csv")
			{
				FileName = FileName.left(FileName.length() - Extension.length() - 1);
				FileName += ".csv";
			}

			swResult<void> ExportResult = mProject->ExportCSV(FileName);

			if (ExportResult)
				StdOut << tr("Saved '%1'.\n").arg(FileName);
			else
			{
				OutputFailed = true;

				if (!ExportResult.error().isEmpty())
					StdErr << ExportResult.error() << '\n';
			}
		}

		if (Options.SaveBrickLink)
		{
			QString FileName = !Options.SaveBrickLinkName.isEmpty() ? Options.SaveBrickLinkName : Options.ProjectName;
			const QString Extension = QFileInfo(FileName).suffix().toLower();

			if (Extension.isEmpty())
				FileName += QLatin1String(".xml");
			else if (Extension != QLatin1String("xml"))
				FileName = FileName.left(FileName.length() - Extension.length() - 1) + QLatin1String(".xml");

			swBrickLinkOptions BrickLinkOptions = swGetBrickLinkOptions();

			if (Options.SetBrickLinkCondition)
				BrickLinkOptions.Condition = static_cast<swBrickLinkCondition>(Options.BrickLinkCondition);

			if (Options.SetBrickLinkWantedList)
				BrickLinkOptions.WantedListId = Options.BrickLinkWantedList;

			if (Options.BrickLinkNotify)
				BrickLinkOptions.Notify = true;

			if (Options.SetBrickLinkExceptions)
				BrickLinkOptions.ExceptionsPath = Options.BrickLinkExceptions;

			swResult<swBrickLinkExport> ExportResult = mProject->ExportBrickLink(FileName, BrickLinkOptions);

			if (ExportResult)
			{
				const swBrickLinkExport& Export = ExportResult.value();
				StdOut << (Export.Lots == 1 ? tr("Saved 1 lot to '%1'.\n").arg(Export.FileName) : tr("Saved %1 lots to '%2'.\n").arg(Export.Lots).arg(Export.FileName));
			}
			else
			{
				OutputFailed = true;

				if (!ExportResult.error().isEmpty())
					StdErr << ExportResult.error() << '\n';
			}
		}

		if (Options.SaveHTML)
		{
			swHTMLExportOptions HTMLOptions(mProject);

			if (!Options.SaveHTMLName.isEmpty())
				HTMLOptions.PathName = Options.SaveHTMLName;

			swResult<void> ExportResult = mProject->ExportHTML(HTMLOptions);

			if (ExportResult)
				StdOut << tr("Saved '%1'.\n").arg(HTMLOptions.PathName);
			else
			{
				OutputFailed = true;

				if (!ExportResult.error().isEmpty())
					StdErr << ExportResult.error() << '\n';
			}
		}
	}

	if (!SaveAndExit)
	{
		gMainWindow->SetColorIndex(swGetColorIndex(7));
		gMainWindow->GetPartSelectionWidget()->SetDefaultPart();

		// every menu as text for the site

		if (Options.PrintMenus)
		{
			StdOut << gMainWindow->GetMenuText();
			StdOut.flush();
			return swStartupMode::Success;
		}

#ifdef Q_OS_IOS
		const swResult<void> RendererResult = swContext::InitializeRenderer(true);

		if (!RendererResult)
		{
			StdErr << RendererResult.error() << '\n';
			QMessageBox::critical(gMainWindow, tr("Initialize Renderer"), RendererResult.error());
			return swStartupMode::Error;
		}

		emit RendererInitialized();
#endif

		gMainWindow->show();

		// a missing chosen color file is asked for once the window shows
		if (ColorConfigurationMissing)
		{
			QTimer::singleShot(0, gMainWindow, [ColorSource, ColorConfigurationPath]()
			{
				const QString FileName = swEnsureColorConfigurationFile(gMainWindow, ColorSource, ColorConfigurationPath);

				if (FileName.isEmpty())
					return;

				swSetColorConfigurationPath(ColorSource, FileName);
				swGetPartsLibrary()->LoadColors();
				swView::UpdateAllViews();
			});
		}

#ifdef Q_OS_WIN
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
		QWindowsWindowFunctions::setHasBorderInFullScreen(gMainWindow->windowHandle(), true);
#endif
#endif
	}

	if (OutputFailed)
		return swStartupMode::Error;

	return SaveAndExit ? swStartupMode::Success : swStartupMode::ShowWindow;
}

void swApplication::Shutdown()
{
	delete gMainWindow;
	gMainWindow = nullptr;

	delete mProject;
	mProject = nullptr;

	delete mLibrary;
	mLibrary = nullptr;

	swContext::ShutdownRenderer();
}

void swApplication::ShowPreferencesDialog()
{
	swPreferencesDialogOptions Options;
	int CurrentAASamples = swGetProfileInteger(SW_PROFILE_ANTIALIASING_SAMPLES);
	swStudStyle CurrentStudStyle = swGetPartsLibrary()->GetStudStyle();

	Options.Preferences = mPreferences;

	Options.LibraryPath = swGetProfileString(SW_PROFILE_PARTS_LIBRARY);
	Options.UnofficialLibraryPath = swGetProfileString(SW_PROFILE_UNOFFICIAL_LIBRARY);
	Options.ShadowLibraryPath = swGetProfileString(SW_PROFILE_SHADOW_LIBRARY);
	Options.MinifigureBuilderSettingsPath = swGetProfileString(SW_PROFILE_MINIFIGURE_BUILDER_SETTINGS);
	Options.BrickLinkMappingPath = swGetProfileString(SW_PROFILE_BRICKLINK_MAPPING);
	Options.ColorConfigPath = swGetProfileString(SW_PROFILE_COLOR_CONFIGURATION);
	Options.ColorConfigAlternatePath = swGetProfileString(SW_PROFILE_COLOR_CONFIGURATION_ALTERNATE);
	Options.ColorSource = swGetColorSource();
	Options.POVRayPath = swGetProfileString(SW_PROFILE_POVRAY_PATH);
	Options.BlenderPath = swGetProfileString(SW_PROFILE_BLENDER_PATH);
	Options.BlenderVersion = swGetProfileString(SW_PROFILE_BLENDER_VERSION);
	Options.LGEOPath = swGetProfileString(SW_PROFILE_POVRAY_LGEO_PATH);
	Options.RebrickableApiKey = swGetProfileString(SW_PROFILE_REBRICKABLE_API_KEY);
	Options.DefaultAuthorName = swGetProfileString(SW_PROFILE_DEFAULT_AUTHOR_NAME);
	Options.DefaultAuthorEmail = swGetProfileString(SW_PROFILE_DEFAULT_AUTHOR_EMAIL);
	Options.DefaultAuthorComment = swGetProfileString(SW_PROFILE_DEFAULT_AUTHOR_COMMENT);
	Options.DefaultAuthorSplitter = swGetProfileString(SW_PROFILE_DEFAULT_AUTHOR_SPLITTER);
	Options.Language = swGetProfileString(SW_PROFILE_LANGUAGE);
	Options.Instructions.Load();

	Options.AASamples = CurrentAASamples;
	Options.StudStyle = CurrentStudStyle;

	Options.Categories = gCategories;
	Options.CategoriesModified = false;
	Options.CategoriesDefault = false;

	Options.KeyboardShortcuts = gKeyboardShortcuts;
	Options.KeyboardShortcutsModified = false;
	Options.KeyboardShortcutsDefault = false;
	Options.MouseShortcuts = gMouseShortcuts;
	Options.MouseShortcutsModified = false;
	Options.MouseShortcutsDefault = false;

	swPreferencesDialog Dialog(gMainWindow, &Options);
	if (Dialog.exec() != QDialog::Accepted)
		return;

	// save the settings file once instead of once per key
	swProfileBatch ProfileBatch;

	bool LanguageChanged = Options.Language != swGetProfileString(SW_PROFILE_LANGUAGE);
	bool LibraryChanged = Options.LibraryPath != swGetProfileString(SW_PROFILE_PARTS_LIBRARY);
	bool UnofficialLibraryChanged = Options.UnofficialLibraryPath != swGetProfileString(SW_PROFILE_UNOFFICIAL_LIBRARY);
	bool ShadowLibraryChanged = Options.ShadowLibraryPath != swGetProfileString(SW_PROFILE_SHADOW_LIBRARY);
	bool ShadowLibraryEnabledChanged = Options.Preferences.mShadowLibraryEnabled != mPreferences.mShadowLibraryEnabled;
	bool ColorConfigChanged = Options.ColorConfigPath != swGetProfileString(SW_PROFILE_COLOR_CONFIGURATION);
	ColorConfigChanged |= Options.ColorConfigAlternatePath != swGetProfileString(SW_PROFILE_COLOR_CONFIGURATION_ALTERNATE);
	ColorConfigChanged |= Options.ColorSource != swGetColorSource();

	// an updated file keeps its path but has new colors
	ColorConfigChanged |= Options.ColorSource != swColorSource::Library;
	bool AAChanged = CurrentAASamples != Options.AASamples;
	bool StudStyleChanged = CurrentStudStyle != Options.StudStyle;
	bool StudCylinderColorEnabledChanged = Options.Preferences.mStudCylinderColorEnabled != mPreferences.mStudCylinderColorEnabled;
	bool EdgeColorSettingsChanged = Options.Preferences.mAutomateEdgeColor != mPreferences.mAutomateEdgeColor;
	EdgeColorSettingsChanged |= Options.Preferences.mPartEdgeContrast != mPreferences.mPartEdgeContrast;
	EdgeColorSettingsChanged |= Options.Preferences.mPartColorValueLDIndex != mPreferences.mPartColorValueLDIndex;
	EdgeColorSettingsChanged |= Options.Preferences.mStudCylinderColorEnabled != mPreferences.mStudCylinderColorEnabled;
	EdgeColorSettingsChanged |= Options.Preferences.mStudCylinderColor != mPreferences.mStudCylinderColor;
	EdgeColorSettingsChanged |= Options.Preferences.mPartEdgeColorEnabled != mPreferences.mPartEdgeColorEnabled;
	EdgeColorSettingsChanged |= Options.Preferences.mPartEdgeColor != mPreferences.mPartEdgeColor;
	EdgeColorSettingsChanged |= Options.Preferences.mBlackEdgeColorEnabled != mPreferences.mBlackEdgeColorEnabled;
	EdgeColorSettingsChanged |= Options.Preferences.mBlackEdgeColor != mPreferences.mBlackEdgeColor;
	EdgeColorSettingsChanged |= Options.Preferences.mDarkEdgeColorEnabled != mPreferences.mDarkEdgeColorEnabled;
	EdgeColorSettingsChanged |= Options.Preferences.mDarkEdgeColor != mPreferences.mDarkEdgeColor;
	bool WatchFileChangesChanged = Options.Preferences.mWatchFileChanges != mPreferences.mWatchFileChanges;
	const bool FlexSpreadModeChanged = Options.Preferences.mFlexSpreadMode != mPreferences.mFlexSpreadMode;
	const swRenderSettings OldPartsRendering = mPreferences.GetRenderSettings(swRenderTarget::Parts);
	const swRenderSettings NewPartsRendering = Options.Preferences.GetRenderSettings(swRenderTarget::Parts);
	const bool PartsRenderingChanged = OldPartsRendering.ShadingMode != NewPartsRendering.ShadingMode || OldPartsRendering.DrawEdgeLines != NewPartsRendering.DrawEdgeLines ||
		OldPartsRendering.DrawConditionalLines != NewPartsRendering.DrawConditionalLines || OldPartsRendering.LineWidth != NewPartsRendering.LineWidth ||
		Options.Preferences.mLightingStyle != mPreferences.mLightingStyle;


	mPreferences = Options.Preferences;

	mPreferences.SaveDefaults();
	UpdateStyle();

	if (WatchFileChangesChanged && mProject)
		mProject->UpdateFileWatcher();

	// redraw flexible part paths
	if (FlexSpreadModeChanged && mProject)
		for (const std::unique_ptr<swModel>& Model : mProject->GetModels())
			for (const std::unique_ptr<swPart>& Part : Model->GetParts())
				if (!Part->GetControlPoints().empty())
					Part->SetControlPoints(Part->GetControlPoints());
	if (PartsRenderingChanged && gMainWindow && gMainWindow->GetPartSelectionWidget())
		gMainWindow->GetPartSelectionWidget()->UpdateThumbnails();

	swSetProfileString(SW_PROFILE_DEFAULT_AUTHOR_NAME, Options.DefaultAuthorName);
	swSetProfileString(SW_PROFILE_DEFAULT_AUTHOR_EMAIL, Options.DefaultAuthorEmail);
	swSetProfileString(SW_PROFILE_DEFAULT_AUTHOR_COMMENT, Options.DefaultAuthorComment);
	swSetProfileString(SW_PROFILE_DEFAULT_AUTHOR_SPLITTER, Options.DefaultAuthorSplitter);
	swSetProfileString(SW_PROFILE_PARTS_LIBRARY, Options.LibraryPath);
	swSetProfileString(SW_PROFILE_UNOFFICIAL_LIBRARY, Options.UnofficialLibraryPath);
	swSetProfileString(SW_PROFILE_SHADOW_LIBRARY, Options.ShadowLibraryPath);
	swSetProfileString(SW_PROFILE_MINIFIGURE_BUILDER_SETTINGS, Options.MinifigureBuilderSettingsPath);
	swSetProfileString(SW_PROFILE_BRICKLINK_MAPPING, Options.BrickLinkMappingPath);
	swSetProfileString(SW_PROFILE_POVRAY_PATH, Options.POVRayPath);
	swSetProfileString(SW_PROFILE_BLENDER_PATH, Options.BlenderPath);
	swSetProfileString(SW_PROFILE_BLENDER_VERSION, Options.BlenderVersion);
	swSetProfileString(SW_PROFILE_POVRAY_LGEO_PATH, Options.LGEOPath);
	swSetProfileString(SW_PROFILE_REBRICKABLE_API_KEY, Options.RebrickableApiKey.trimmed());
	swSetProfileString(SW_PROFILE_LANGUAGE, Options.Language);
	swSetProfileInteger(SW_PROFILE_ANTIALIASING_SAMPLES, Options.AASamples);
	swSetProfileStudStyle(Options.StudStyle);
	Options.Instructions.Save();

	if (gMainWindow)
		gMainWindow->UpdateInstructionsOptions();

	// libraries reload while running
	if (LibraryChanged || UnofficialLibraryChanged)
	{
		const bool LibraryOverridden = !GetEnvironmentLibraryPath().isEmpty();

		mLibrary->SetUnofficialArchivePath(Options.UnofficialLibraryPath);

		// the environment library stays in use
		if (LibraryChanged && LibraryOverridden)
		{
			if (UnofficialLibraryChanged && !ReloadPartsLibrary(mLibrary->GetLibraryPath(), mLibrary->IsOfficialLibrary()))
				QMessageBox::warning(gMainWindow, tr("Load Parts Library"), tr("Could not reload the parts library '%1'.").arg(mLibrary->GetLibraryPath()));
		}
		// an empty path falls back to the default libraries
		else if (LibraryChanged && Options.LibraryPath.isEmpty())
		{
			bool LibraryLoaded = false;

			for (const QPair<QString, bool>& LibraryPathEntry : mDefaultLibraryPaths)
			{
				if (ReloadPartsLibrary(LibraryPathEntry.first, LibraryPathEntry.second))
				{
					LibraryLoaded = true;
					break;
				}
			}

			if (!LibraryLoaded)
				QMessageBox::warning(gMainWindow, tr("Load Parts Library"), tr("Could not find a default parts library. '%1' stays in use until StudWorx starts again and offers to download one.").arg(mLibrary->GetLibraryPath()));
		}
		else
		{
			const QString LibraryPath = LibraryChanged ? Options.LibraryPath : mLibrary->GetLibraryPath();
			const bool OfficialLibrary = LibraryChanged ? false : mLibrary->IsOfficialLibrary();

			if (!ReloadPartsLibrary(LibraryPath, OfficialLibrary))
				QMessageBox::warning(gMainWindow, tr("Load Parts Library"), tr("Could not open the parts library '%1'.").arg(LibraryPath));
		}

		if (UnofficialLibraryChanged && !Options.UnofficialLibraryPath.isEmpty() && !mLibrary->HasUnofficialArchive())
			QMessageBox::warning(gMainWindow, tr("Load Parts Library"), tr("Could not open the unofficial library archive '%1'.").arg(Options.UnofficialLibraryPath));
	}

	// shadow library reloads while running
	if (ShadowLibraryChanged)
	{
		if (!swShadowLibrary::SetPath(Options.ShadowLibraryPath) && !Options.ShadowLibraryPath.isEmpty())
			QMessageBox::warning(gMainWindow, tr("Load Part Connections"), tr("Could not find a shadow library in '%1'.").arg(Options.ShadowLibraryPath));
	}

	if ((ShadowLibraryChanged || ShadowLibraryEnabledChanged) && gMainWindow)
		gMainWindow->UpdateShadowLibraryAction();

	// name the settings that wait for a restart
	if (LanguageChanged && AAChanged)
		QMessageBox::information(gMainWindow, tr("Preferences"), tr("The new language and antialiasing settings apply the next time you start StudWorx."));
	else if (LanguageChanged)
		QMessageBox::information(gMainWindow, tr("Preferences"), tr("The new language applies the next time you start StudWorx."));
	else if (AAChanged)
		QMessageBox::information(gMainWindow, tr("Preferences"), tr("The new antialiasing setting applies the next time you start StudWorx."));

	if (Options.CategoriesModified)
	{
		if (Options.CategoriesDefault)
			swResetDefaultCategories();
		else
		{
			gCategories = Options.Categories;
			swSaveDefaultCategories();
		}

		gMainWindow->UpdateCategories();
	}

	if (Options.KeyboardShortcutsModified)
	{
		if (Options.KeyboardShortcutsDefault)
			swResetDefaultKeyboardShortcuts();
		else
		{
			gKeyboardShortcuts = Options.KeyboardShortcuts;
			swSaveDefaultKeyboardShortcuts();
		}

		gMainWindow->UpdateShortcuts();
	}

	if (Options.MouseShortcutsModified)
	{
		if (Options.MouseShortcutsDefault)
			swResetDefaultMouseShortcuts();
		else
		{
			gMouseShortcuts = Options.MouseShortcuts;
			swSaveDefaultMouseShortcuts();
		}
	}

	// both paths need the new config
	if (ColorConfigChanged)
	{
		swSetProfileString(SW_PROFILE_COLOR_CONFIGURATION, Options.ColorConfigPath);
		swSetProfileString(SW_PROFILE_COLOR_CONFIGURATION_ALTERNATE, Options.ColorConfigAlternatePath);
		swSetColorSource(Options.ColorSource);
	}

	if (StudStyleChanged || StudCylinderColorEnabledChanged)
		swGetPartsLibrary()->UpdateStudStyleAndColors(Options.StudStyle, Options.Preferences.mStudCylinderColorEnabled);
	else if (ColorConfigChanged || EdgeColorSettingsChanged)
		swGetPartsLibrary()->LoadColors();

	gMainWindow->SetShadingMode(Options.Preferences.mShadingMode);
	swView::UpdateAllViews();
}
