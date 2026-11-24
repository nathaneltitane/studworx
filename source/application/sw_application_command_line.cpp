#include "sw_global.h"
#include "sw_application.h"
#include "sw_library.h"
#include "sw_project.h"
#include "sw_dialog_preferences.h"
#include "sw_object_camera.h"
#include "sw_window_utilities.h"

swCommandLineOptions swApplication::ParseCommandLineOptions()
{
	swPreferences Preferences;
	Preferences.LoadDefaults();

	swCommandLineOptions Options;

	Options.FadeSteps = Preferences.mFadeSteps;
	Options.ImageHighlight = Preferences.mHighlightNewParts;
	Options.ImageWidth = swGetProfileInteger(SW_PROFILE_IMAGE_WIDTH);
	Options.ImageHeight = swGetProfileInteger(SW_PROFILE_IMAGE_HEIGHT);
	Options.ShadingMode = Preferences.mShadingMode;
	Options.LightingStyle = Preferences.mLightingStyle;
	Options.LineWidth = Preferences.mLineWidth;
	Options.AASamples = swGetProfileInteger(SW_PROFILE_ANTIALIASING_SAMPLES);
	Options.StudStyle = swGetProfileStudStyle();
	Options.ImageStart = 0;
	Options.ImageEnd = 0;
	Options.CameraPosition[0] = swVector3(0.0f, 0.0f, 0.0f);
	Options.CameraPosition[1] = swVector3(0.0f, 0.0f, 0.0f);
	Options.CameraPosition[2] = swVector3(0.0f, 0.0f, 0.0f);
	Options.CameraLatLon = swVector2(0.0f, 0.0f);
	Options.FoV = 0.0f;
	Options.ZPlanes = swVector2(0.0f, 0.0f);
	Options.Viewpoint = swViewpoint::Count;
	Options.FadeStepsColor = Preferences.mFadeStepsColor;
	Options.HighlightColor = Preferences.mHighlightNewPartsColor;
	Options.StudCylinderColorEnabled = Preferences.mStudCylinderColorEnabled;
	Options.StudCylinderColor = Preferences.mStudCylinderColor;
	Options.PartEdgeColorEnabled = Preferences.mPartEdgeColorEnabled;
	Options.PartEdgeColor = Preferences.mPartEdgeColor;
	Options.BlackEdgeColorEnabled = Preferences.mBlackEdgeColorEnabled;
	Options.BlackEdgeColor = Preferences.mBlackEdgeColor;
	Options.DarkEdgeColorEnabled = Preferences.mDarkEdgeColorEnabled;
	Options.DarkEdgeColor = Preferences.mDarkEdgeColor;
	Options.PartEdgeContrast = Preferences.mPartEdgeContrast;
	Options.PartColorValueLDIndex = Preferences.mPartColorValueLDIndex;
	Options.AutomateEdgeColor = Preferences.mAutomateEdgeColor;

	QStringList Arguments = arguments();

	if (Arguments.isEmpty())
		return Options;

	Arguments.removeFirst();

	while (!Arguments.isEmpty())
	{
		QString Option = Arguments.takeFirst();

		if (Option.isEmpty())
			continue;

		auto ParseString = [&Option, &Arguments, &Options](QString& Value, bool Required)
		{
			if (!Arguments.isEmpty() && Arguments.front()[0] != '-')
			{
				QString Parameter = Arguments.takeFirst();
				Value = Parameter;
			}
			else if (Required)
			{
				Options.StdErr += tr("Not enough parameters for the '%1' option.\n").arg(Option);
				Options.ParseOK = false;
				return false;
			}

			return true;
		};

		auto ParseInteger = [&Option, &Arguments, &Options](int& Value, int Min, int Max)
		{
			if (!Arguments.isEmpty() && Arguments.front()[0] != '-')
			{
				bool Ok = false;
				QString Parameter = Arguments.takeFirst();
				int NewValue = Parameter.toInt(&Ok);

				if (Ok && NewValue >= Min && NewValue <= Max)
				{
					Value = NewValue;
					return true;
				}
				else
					Options.StdErr += tr("Invalid parameter value specified for the '%1' option: '%2'.\n").arg(Option, Parameter);
			}
			else
				Options.StdErr += tr("Not enough parameters for the '%1' option.\n").arg(Option);

			Options.ParseOK = false;
			return false;
		};

		auto ParseUnsigned = [&Option, &Arguments, &Options](uint& Value, uint Min, uint Max)
		{
			if (!Arguments.isEmpty() && Arguments.front()[0] != '-')
			{
				bool Ok = false;
				QString Parameter = Arguments.takeFirst();
				uint NewValue = Parameter.toUInt(&Ok);

				if (Ok && NewValue >= Min && NewValue <= Max)
				{
					Value = NewValue;
					return true;
				}
				else
					Options.StdErr += tr("Invalid parameter value specified for the '%1' option: '%2'.\n").arg(Option, Parameter);
			}
			else
				Options.StdErr += tr("Not enough parameters for the '%1' option.\n").arg(Option);

			Options.ParseOK = false;
			return false;
		};

		auto ParseFloat = [&Option, &Arguments, &Options](float& Value, float Min, float Max)
		{
			if (!Arguments.isEmpty() && Arguments.front()[0] != '-')
			{
				bool Ok = false;
				QString Parameter = Arguments.takeFirst();
				float NewValue = Parameter.toFloat(&Ok);

				if (Ok && NewValue >= Min && NewValue <= Max)
				{
					Value = NewValue;
					return true;
				}
				else
					Options.StdErr += tr("Invalid parameter value specified for the '%1' option: '%2'.\n").arg(Option, Parameter);
			}
			else
				Options.StdErr += tr("Not enough parameters for the '%1' option.\n").arg(Option);

			Options.ParseOK = false;
			return false;
		};

		auto ParseFloatArray = [&Option, &Arguments, &Options](int Count, float* ValueArray, bool NegativesValid)
		{
			if (Arguments.size() < Count)
			{
				Options.StdErr += tr("Not enough parameters for the '%1' option.\n").arg(Option);
				Arguments.clear();
				Options.ParseOK = false;
				return false;
			}

			for (int ParseIndex = 0; ParseIndex < Count; ParseIndex++)
			{
				if (NegativesValid || Arguments.front()[0] != '-')
				{
					bool Ok = false;
					QString Parameter = Arguments.takeFirst();
					float NewValue = Parameter.toFloat(&Ok);

					if (Ok)
					{
						*(ValueArray++) = NewValue;
						continue;
					}

					Options.StdErr += tr("Invalid parameter value specified for the '%1' option: '%2'.\n").arg(Option, Parameter);
				}
				else
					Options.StdErr += tr("Not enough parameters for the '%1' option.\n").arg(Option);

				Options.ParseOK = false;
				return false;
			}

			return true;
		};

		auto ParseColor = [&Option, &Arguments, &Options](quint32& Color)
		{
			if (!Arguments.isEmpty() && Arguments.front()[0] != '-')
			{
				QString Parameter = Arguments.takeFirst();
				QColor ParsedColor = QColor(Parameter);

				if (ParsedColor.isValid())
				{
					Color = SW_RGBA(ParsedColor.red(), ParsedColor.green(), ParsedColor.blue(), ParsedColor.alpha());
					return true;
				}
				else
					Options.StdErr += tr("Invalid parameter value specified for the '%1' option: '%2'.\n").arg(Option, Parameter);
			}
			else
				Options.StdErr += tr("Not enough parameters for the '%1' option.\n").arg(Option);

			Options.ParseOK = false;
			return false;
		};

		if (Option[0] != '-')
		{
			if (QFileInfo(Option).isReadable())
				Options.ProjectName = Option;
			else
				Options.StdErr += tr("The file '%1' is not readable.\n").arg(Option);

			continue;
		}

		if (Option == QLatin1String("-l") || Option == QLatin1String("--library"))
		{
			QString LibPath;

			if (ParseString(LibPath, true))
				Options.LibraryPaths += QPair<QString, bool>(LibPath, false);
		}
		else if (Option == QLatin1String("--unofficial-library"))
			ParseString(Options.UnofficialLibraryPath, true);
		else if (Option == QLatin1String("--shadow-library"))
			ParseString(Options.ShadowLibraryPath, true);
		else if (Option == QLatin1String("-i") || Option == QLatin1String("--image"))
		{
			Options.SaveImage = true;
			ParseString(Options.ImageName, false);
		}
		else if (Option == QLatin1String("-w") || Option == QLatin1String("--width"))
			ParseInteger(Options.ImageWidth, 1, SW_MAXIMUM_IMAGE_DIMENSION);
		else if (Option == QLatin1String("--height"))
			ParseInteger(Options.ImageHeight, 1, SW_MAXIMUM_IMAGE_DIMENSION);
		else if (Option == QLatin1String("-f") || Option == QLatin1String("--from"))
			ParseUnsigned(Options.ImageStart, 1, SW_STEP_MAXIMUM);
		else if (Option == QLatin1String("-t") || Option == QLatin1String("--to"))
			ParseUnsigned(Options.ImageEnd, 1, SW_STEP_MAXIMUM);
		else if (Option == QLatin1String("-s") || Option == QLatin1String("--submodel"))
			ParseString(Options.ModelName, true);
		else if (Option == QLatin1String("-c") || Option == QLatin1String("--camera"))
			ParseString(Options.CameraName, true);
		else if (Option == QLatin1String("--viewpoint"))
		{
			QString ViewpointName;

			if (ParseString(ViewpointName, true))
			{
				Options.Viewpoint = swCamera::GetViewpoint(ViewpointName);

				if (Options.Viewpoint == swViewpoint::Count)
				{
					Options.StdErr += tr("Invalid parameter value specified for the '%1' option: '%2'.\n").arg(Option, ViewpointName);
					Options.ParseOK = false;
				}
			}
		}
		else if (Option == QLatin1String("--camera-angles"))
		{
			if ((Options.SetCameraAngles = ParseFloatArray(2, Options.CameraLatLon.GetFloats(), true)) && (fabsf(Options.CameraLatLon[0]) > 360.0f || fabsf(Options.CameraLatLon[1]) > 360.0f))
			{
				Options.StdErr += tr("Invalid parameter values specified for the '%1' option: limits are +/- 360.\n").arg(Option);
				Options.ParseOK = false;
			}
		}
		else if (Option == QLatin1String("--camera-position") || Option == QLatin1String("--camera-position-ldraw"))
		{
			if ((Options.SetCameraPosition = ParseFloatArray(9, Options.CameraPosition[0].GetFloats(), true)))
			{
				if (Option == QLatin1String("--camera-position-ldraw"))
				{
					Options.CameraPosition[0] = swVector3LDrawToStudWorx(Options.CameraPosition[0]);
					Options.CameraPosition[1] = swVector3LDrawToStudWorx(Options.CameraPosition[1]);
					Options.CameraPosition[2] = swVector3LDrawToStudWorx(Options.CameraPosition[2]);
				}

				swVector3 Front = Options.CameraPosition[1] - Options.CameraPosition[0];

				if (Front.LengthSquared() < 0.1f)
				{
					Options.StdErr += tr("The camera position is too close to the camera target of the '%1' option.\n").arg(Option);
					Options.ParseOK = false;
				}
				else if (Options.CameraPosition[2].LengthSquared() < 0.1f)
				{
					Options.StdErr += tr("Invalid up vector length specified for the '%1' option.\n").arg(Option);
					Options.ParseOK = false;
				}
				else if (fabsf(swDot(swNormalize(Front), swNormalize(Options.CameraPosition[2]))) > 0.99f)
				{
					Options.StdErr += tr("The up vector is parallel to the front vector specified for the '%1' option.\n").arg(Option);
					Options.ParseOK = false;
				}
			}
		}
		else if (Option == QLatin1String("--projection"))
		{
			QString ProjectionString;

			if (ParseString(ProjectionString, true))
			{
				const QStringList ProjectionNames = { QLatin1String("perspective"), QLatin1String("orthographic") };
				const int Projection = ProjectionNames.indexOf(ProjectionString.toLower());

				if (Projection != -1)
				{
					Options.SetProjection = true;
					Options.Orthographic = Projection == 1;
				}
				else
				{
					Options.StdErr += tr("Invalid parameter value specified for the '%1' option: '%2'.\n").arg(Option, ProjectionString);
					Options.ParseOK = false;
				}
			}
		}
		else if (Option == QLatin1String("--transparent"))
			Options.TransparentImage = true;
		else if (Option == QLatin1String("--field-of-view"))
			Options.SetFoV = ParseFloat(Options.FoV, 1.0f, 180.0f);
		else if (Option == QLatin1String("--clip-planes"))
		{
			if ((Options.SetZPlanes = ParseFloatArray(2, Options.ZPlanes.GetFloats(), false)) && (Options.ZPlanes[0] < 1.0 || Options.ZPlanes[0] >= Options.ZPlanes[1]))
			{
				Options.StdErr += tr("Invalid parameter values specified for the '%1' option: requirements are: 1 <= <near> < <far>.\n").arg(Option);
				Options.ParseOK = false;
			}
		}
		else if (Option == QLatin1String("--no-stud-cylinder-color"))
			Options.StudCylinderColorEnabled = false;
		else if (Option == QLatin1String("--stud-cylinder-color"))
			ParseColor(Options.StudCylinderColor);
		else if (Option == QLatin1String("--no-edge-color"))
			Options.PartEdgeColorEnabled = false;
		else if (Option == QLatin1String("--edge-color"))
			ParseColor(Options.PartEdgeColor);
		else if (Option == QLatin1String("--no-edge-color-black"))
			Options.BlackEdgeColorEnabled = false;
		else if (Option == QLatin1String("--edge-color-black"))
			ParseColor(Options.BlackEdgeColor);
		else if (Option == QLatin1String("--no-edge-color-dark"))
			Options.DarkEdgeColorEnabled = false;
		else if (Option == QLatin1String("--edge-color-dark"))
			ParseColor(Options.DarkEdgeColor);
		else if (Option == QLatin1String("--edge-color-automatic"))
			Options.AutomateEdgeColor = true;
		else if (Option == QLatin1String("--edge-contrast"))
			ParseFloat(Options.PartEdgeContrast, 0.0f, 1.0f);
		else if (Option == QLatin1String("--edge-light-dark"))
			ParseFloat(Options.PartColorValueLDIndex, 0.0f, 1.0f);
		else if (Option == QLatin1String("--fade-steps"))
			Options.FadeSteps = true;
		else if (Option == QLatin1String("--no-fade-steps"))
			Options.FadeSteps = false;
		else if (Option == QLatin1String("--fade-color"))
		{
			if (ParseColor(Options.FadeStepsColor))
			{
				Options.SetFadeStepsColor = true;
				Options.FadeSteps = true;
			}
		}
		else if (Option == QLatin1String("--highlight"))
			Options.ImageHighlight = true;
		else if (Option == QLatin1String("--no-highlight"))
			Options.ImageHighlight = false;
		else if (Option == QLatin1String("--highlight-color"))
		{
			if (ParseColor(Options.HighlightColor))
			{
				Options.SetHighlightColor = true;
				Options.ImageHighlight = true;
			}
		}
		else if (Option == QLatin1String("--shading"))
		{
			QString ShadingString;

			if (ParseString(ShadingString, true))
			{
				if (ShadingString == QLatin1String("wireframe"))
					Options.ShadingMode = swShadingMode::Wireframe;
				else if (ShadingString == QLatin1String("flat"))
					Options.ShadingMode = swShadingMode::Flat;
				else if (ShadingString == QLatin1String("default"))
					Options.ShadingMode = swShadingMode::DefaultLights;
				else if (ShadingString == QLatin1String("full"))
					Options.ShadingMode = swShadingMode::Full;
				else
				{
					Options.StdErr += tr("Invalid parameter value specified for the '%1' option: '%2'.\n").arg(Option, ShadingString);
					Options.ParseOK = false;
				}
			}
		}
		else if (Option == QLatin1String("--lighting"))
		{
			QString LightingString;

			if (ParseString(LightingString, true))
			{
				if (LightingString == QLatin1String("standard"))
					Options.LightingStyle = swLightingStyle::Standard;
				else if (LightingString == QLatin1String("studio"))
					Options.LightingStyle = swLightingStyle::Studio;
				else if (LightingString == QLatin1String("matte"))
					Options.LightingStyle = swLightingStyle::Matte;
				else if (LightingString == QLatin1String("toon"))
					Options.LightingStyle = swLightingStyle::Toon;
				else
				{
					Options.StdErr += tr("Invalid parameter value specified for the '%1' option: '%2'.\n").arg(Option, LightingString);
					Options.ParseOK = false;
				}
			}
		}
		else if (Option == QLatin1String("--line-width"))
			ParseFloat(Options.LineWidth, 0.0f, 10.0f);
		else if (Option == QLatin1String("--antialiasing-samples"))
		{
			if (ParseInteger(Options.AASamples, 1, 8) && Options.AASamples != 1 && Options.AASamples != 2 && Options.AASamples != 4 && Options.AASamples != 8)
			{
				Options.StdErr += tr("Invalid parameter value specified for the '%1' option: '%2'.\n").arg(Option, QString::number(Options.AASamples));
				Options.ParseOK = false;
			}
		}
		else if (Option == QLatin1String("--stud-style"))
		{
			int StudStyle;

			if (ParseInteger(StudStyle, 0, static_cast<int>(swStudStyle::Count) - 1))
				Options.StudStyle = static_cast<swStudStyle>(StudStyle);
		}
		else if (Option == QLatin1String("--flatten"))
		{
			Options.SaveFlatten = true;
			ParseString(Options.SaveFlattenName, true);
		}
		else if (Option == QLatin1String("--undo"))
		{
			Options.SaveUndo = true;
			ParseString(Options.SaveUndoName, true);
		}
		else if (Option == QLatin1String("--resave"))
		{
			Options.SaveResave = true;
			ParseString(Options.SaveResaveName, true);
		}
		else if (Option == QLatin1String("--export-wavefront"))
		{
			Options.SaveWavefront = true;
			ParseString(Options.SaveWavefrontName, false);
		}
		else if (Option == QLatin1String("--export-3ds"))
		{
			Options.Save3DS = true;
			ParseString(Options.Save3DSName, false);
		}
		else if (Option == QLatin1String("--export-collada"))
		{
			Options.SaveCOLLADA = true;
			ParseString(Options.SaveCOLLADAName, false);
		}
		else if (Option == QLatin1String("--export-kmz"))
		{
			Options.SaveKMZ = true;
			ParseString(Options.SaveKMZName, false);
		}
		else if (Option == QLatin1String("--kmz-geometry"))
		{
			QString GeometryString;

			if (ParseString(GeometryString, true))
			{
				const QStringList GeometryNames = { QLatin1String("all"), QLatin1String("visible") };
				const int Geometry = GeometryNames.indexOf(GeometryString.toLower());

				if (Geometry != -1)
				{
					Options.SetKMZGeometry = true;
					Options.KMZVisibleSurfaces = Geometry == 1;
				}
				else
				{
					Options.StdErr += tr("Invalid parameter value specified for the '%1' option: '%2'.\n").arg(Option, GeometryString);
					Options.ParseOK = false;
				}
			}
		}
		else if (Option == QLatin1String("--export-csv"))
		{
			Options.SaveCSV = true;
			ParseString(Options.SaveCSVName, false);
		}
		else if (Option == QLatin1String("--export-bricklink"))
		{
			Options.SaveBrickLink = true;
			ParseString(Options.SaveBrickLinkName, false);
		}
		else if (Option == QLatin1String("--bricklink-condition"))
		{
			QString ConditionString;

			if (ParseString(ConditionString, true))
			{
				const QStringList ConditionNames = { QLatin1String("any"), QLatin1String("new"), QLatin1String("used") };
				const int Condition = ConditionNames.indexOf(ConditionString.toLower());

				if (Condition != -1)
				{
					Options.SetBrickLinkCondition = true;
					Options.BrickLinkCondition = Condition;
				}
				else
				{
					Options.StdErr += tr("Invalid parameter value specified for the '%1' option: '%2'.\n").arg(Option, ConditionString);
					Options.ParseOK = false;
				}
			}
		}
		else if (Option == QLatin1String("--bricklink-wanted-list"))
		{
			if (ParseString(Options.BrickLinkWantedList, true))
			{
				if (const std::optional<QString> WantedListId = swParseBrickLinkWantedListId(Options.BrickLinkWantedList))
				{
					Options.BrickLinkWantedList = *WantedListId;
					Options.SetBrickLinkWantedList = true;
				}
				else
				{
					Options.StdErr += tr("Invalid parameter value specified for the '%1' option: '%2'.\n").arg(Option, Options.BrickLinkWantedList);
					Options.ParseOK = false;
				}
			}
		}
		else if (Option == QLatin1String("--bricklink-notify"))
			Options.BrickLinkNotify = true;
		else if (Option == QLatin1String("--bricklink-exceptions"))
			Options.SetBrickLinkExceptions = ParseString(Options.BrickLinkExceptions, true);
		else if (Option == QLatin1String("--export-html"))
		{
			Options.SaveHTML = true;
			ParseString(Options.SaveHTMLName, false);
		}
		else if (Option == QLatin1String("--export-pdf"))
		{
			Options.SavePDF = true;
			ParseString(Options.SavePDFName, false);
		}
		else if (Option == QLatin1String("--export-povray"))
		{
			Options.SavePOVRay = true;
			ParseString(Options.SavePOVRayName, false);
		}
		else if (Option == QLatin1String("--render-povray"))
		{
			Options.RenderPOVRay = true;
			ParseString(Options.RenderPOVRayName, false);
		}
		else if (Option == QLatin1String("--import-settings"))
			ParseString(Options.SettingsImportName, true);
		else if (Option == QLatin1String("--export-settings"))
			ParseString(Options.SettingsExportName, true);
		else if (Option == QLatin1String("--import-keyboard-shortcuts"))
			ParseString(Options.KeyboardShortcutsImportName, true);
		else if (Option == QLatin1String("--export-keyboard-shortcuts"))
			ParseString(Options.KeyboardShortcutsExportName, true);
		else if (Option == QLatin1String("--import-mouse-shortcuts"))
			ParseString(Options.MouseShortcutsImportName, true);
		else if (Option == QLatin1String("--export-mouse-shortcuts"))
			ParseString(Options.MouseShortcutsExportName, true);
		else if (Option == QLatin1String("--import-categories"))
			ParseString(Options.CategoriesImportName, true);
		else if (Option == QLatin1String("--export-categories"))
			ParseString(Options.CategoriesExportName, true);
		else if (Option == QLatin1String("--menus"))
			Options.PrintMenus = true;
		else if (Option == QLatin1String("-v") || Option == QLatin1String("--version"))
		{
			Options.StdOut += tr("StudWorx Version %1\n").arg(SW_VERSION_TEXT);
			Options.StdOut += tr("Compiled %1\n").arg(__DATE__);
			Options.Exit = true;
		}
		else if (Option == QLatin1String("-h") || Option == QLatin1String("--help"))
		{
			Options.StdOut += tr("Usage: studworx [options] [file]\n");
			Options.StdOut += tr("\nLibrary:\n");
			Options.StdOut += tr("  -l, --library <path>: Set the Parts Library location to path.\n");
			Options.StdOut += tr("  --unofficial-library <path>: Set the Unofficial Parts Library archive to path.\n");
			Options.StdOut += tr("  --shadow-library <path>: Set the LDCad Shadow Library location to path.\n");
			Options.StdOut += tr("\nPictures:\n");
			Options.StdOut += tr("  -i, --image [outfile.ext]: Save a picture in the format specified by ext and exit.\n");
			Options.StdOut += tr("  -w, --width <width>: Set the picture width.\n");
			Options.StdOut += tr("  --height <height>: Set the picture height.\n");
			Options.StdOut += tr("  -f, --from <step>: Set the first step to save pictures.\n");
			Options.StdOut += tr("  -t, --to <step>: Set the last step to save pictures.\n");
			Options.StdOut += tr("  -s, --submodel <submodel>: Set the active submodel.\n");
			Options.StdOut += tr("  --transparent: Render images with a transparent background (PNG and GIF only).\n");
			Options.StdOut += tr("\nCamera:\n");
			Options.StdOut += tr("  -c, --camera <camera name>: Set the active camera.\n");
			Options.StdOut += tr("  --viewpoint <viewpoint>: Set the view to one of the preset viewpoints (front, back, left, right, top, bottom, home).\n");
			Options.StdOut += tr("  --camera-angles <latitude> <longitude>: Set the camera angles in degrees around the model.\n");
			Options.StdOut += tr("  --camera-position <x> <y> <z> <tx> <ty> <tz> <ux> <uy> <uz>: Set the camera position, target and up vector.\n");
			Options.StdOut += tr("  --camera-position-ldraw <x> <y> <z> <tx> <ty> <tz> <ux> <uy> <uz>: Set the camera position, target and up vector using LDraw coordinates.\n");
			Options.StdOut += tr("  --projection <perspective|orthographic>: Render images with a perspective or an orthographic projection.\n");
			Options.StdOut += tr("  --field-of-view <degrees>: Set the vertical field of view used to render images (< 180).\n");
			Options.StdOut += tr("  --clip-planes <near> <far>: Set the near and far clipping planes used to render images (1 <= <near> < <far>).\n");
			Options.StdOut += tr("\nRendering:\n");
			Options.StdOut += tr("  --shading <mode>: Select shading mode for rendering (default, flat, wireframe, full).\n");
			Options.StdOut += tr("  --lighting <style>: Select the lighting style of the default shading (studio, standard, matte, toon).\n");
			Options.StdOut += tr("  --line-width <width>: Set the width of the edge lines.\n");
			Options.StdOut += tr("  --antialiasing-samples <samples>: Anti-aliasing sample size (1, 2, 4, or 8).\n");
			Options.StdOut += tr("  --fade-steps: Render parts from prior steps faded.\n");
			Options.StdOut += tr("  --no-fade-steps: Do not render parts from prior steps faded.\n");
			Options.StdOut += tr("  --fade-color <#AARRGGBB>: Rendering color for prior step parts.\n");
			Options.StdOut += tr("  --highlight: Highlight parts in the steps they appear.\n");
			Options.StdOut += tr("  --no-highlight: Do not highlight parts in the steps they appear.\n");
			Options.StdOut += tr("  --highlight-color <#AARRGGBB>: Rendering color for highlighted parts.\n");
			Options.StdOut += tr("\nStuds and Edges:\n");
			Options.StdOut += tr("  --stud-style <id>: Set the stud style (0=Plain, 1=Thin-Line Logo, 2=Outline Logo, 3=Sharp Top Logo, 4=Rounded Top Logo, 5=Flattened Logo, 6=High Contrast, 7=High Contrast Logo).\n");
			Options.StdOut += tr("  --stud-cylinder-color <#AARRGGBB>: High contrast stud cylinder color.\n");
			Options.StdOut += tr("  --no-stud-cylinder-color: Disable high contrast stud cylinder color.\n");
			Options.StdOut += tr("  --edge-color <#AARRGGBB>: High contrast edge color.\n");
			Options.StdOut += tr("  --no-edge-color: Disable high contrast edge color.\n");
			Options.StdOut += tr("  --edge-color-black <#AARRGGBB>: High contrast edge color for black parts.\n");
			Options.StdOut += tr("  --no-edge-color-black: Disable high contrast edge color for black parts.\n");
			Options.StdOut += tr("  --edge-color-dark <#AARRGGBB>: High contrast edge color for dark color parts.\n");
			Options.StdOut += tr("  --no-edge-color-dark: Disable high contrast edge color for dark color parts.\n");
			Options.StdOut += tr("  --edge-color-automatic: Enable automatically adjusted edge colors.\n");
			Options.StdOut += tr("  --edge-contrast <contrast>: Edge color contrast of automatic edge colors, between 0.0 and 1.0.\n");
			Options.StdOut += tr("  --edge-light-dark <value>: Light/Dark color value of automatic edge colors, between 0.0 and 1.0.\n");
			Options.StdOut += tr("\nExport:\n");
			Options.StdOut += tr("  --flatten <outfile.ldr>: Replace every submodel at all depths with its parts and save the model, before any other export.\n");
			Options.StdOut += tr("  --undo <outfile.ldr>: Undo every command line edit and save the model, to check that undo restores it.\n");
			Options.StdOut += tr("  --resave <outfile.ldr>: Load the instruction settings and save the model, to check how older files load.\n");
			Options.StdOut += tr("  --export-wavefront [outfile.obj]: Export the model to Wavefront OBJ format.\n");
			Options.StdOut += tr("  --export-3ds [outfile.3ds]: Export the model to 3D Studio 3DS format.\n");
			Options.StdOut += tr("  --export-collada [outfile.dae]: Export the model to COLLADA DAE format.\n");
			Options.StdOut += tr("  --export-kmz [outfile.kmz]: Export the model to KMZ format.\n");
			Options.StdOut += tr("  --kmz-geometry <all|visible>: Keep every part, or flatten to the visible surfaces with one mesh per color, in KMZ exports.\n");
			Options.StdOut += tr("  --export-csv [outfile.csv]: Export the list of parts used in CSV format.\n");
			Options.StdOut += tr("  --export-bricklink [outfile.xml]: Export the list of parts used as a BrickLink XML wanted list, translated through the BrickLink mapping file set in the preferences.\n");
			Options.StdOut += tr("  --bricklink-condition <any|new|used>: Condition wanted in BrickLink exports.\n");
			Options.StdOut += tr("  --bricklink-wanted-list <id|url>: BrickLink wanted list the exported parts are added to, by number or by the address of its page.\n");
			Options.StdOut += tr("  --bricklink-notify: Ask BrickLink to notify new listings of the exported parts.\n");
			Options.StdOut += tr("  --bricklink-exceptions <file>: Comma or tab separated file of parts and the BrickLink numbers that replace their mapping in BrickLink exports.\n");
			Options.StdOut += tr("  --export-html [folder]: Create HTML instructions in a folder named after the model inside the folder.\n");
			Options.StdOut += tr("  --export-pdf [outfile.pdf]: Print the instruction pages to a PDF file on the page setup paper.\n");
			Options.StdOut += tr("  --export-povray [outfile.pov]: Export the model to POV-Ray format.\n");
			Options.StdOut += tr("  --render-povray [outfile.png]: Render the model with POV-Ray using the POV-Ray window settings, the picture width and height, and the transparent background.\n");
			Options.StdOut += tr("\nSettings:\n");
			Options.StdOut += tr("  --import-settings <infile.ini>: Import the application settings and exit.\n");
			Options.StdOut += tr("  --export-settings <outfile.ini>: Export the application settings and exit.\n");
			Options.StdOut += tr("  --import-keyboard-shortcuts <infile.txt>: Import the keyboard shortcuts and exit.\n");
			Options.StdOut += tr("  --export-keyboard-shortcuts <outfile.txt>: Export the keyboard shortcuts and exit.\n");
			Options.StdOut += tr("  --import-mouse-shortcuts <infile.txt>: Import the mouse shortcuts and exit.\n");
			Options.StdOut += tr("  --export-mouse-shortcuts <outfile.txt>: Export the mouse shortcuts and exit.\n");
			Options.StdOut += tr("  --import-categories <infile.txt>: Import the part categories and exit.\n");
			Options.StdOut += tr("  --export-categories <outfile.txt>: Export the part categories and exit.\n");
			Options.StdOut += tr("\nInformation:\n");
			Options.StdOut += tr("  -v, --version: Output version information and exit.\n");
			Options.StdOut += tr("  -h, --help: Display this help message and exit.\n");
			Options.StdOut += QLatin1String("\n");
			Options.Exit = true;
		}
		else
		{
			Options.StdErr += tr("Unknown option: '%1'.\n").arg(Option);
			Options.ParseOK = false;
		}
	}

	if (Options.AutomateEdgeColor && swIsHighContrast(Options.StudStyle))
	{
		Options.StdErr += tr("High contrast stud and edge color settings are ignored when --edge-color-automatic is set.\n");
	}

	if (!Options.AutomateEdgeColor && !Preferences.mAutomateEdgeColor)
	{
		QString const Message = tr("Automate edge color is required for the '%1' option but is not enabled.\n");

		if (Options.PartEdgeContrast != Preferences.mPartEdgeContrast)
			Options.StdErr += Message.arg(QString("--edge-contrast %1").arg(Options.PartEdgeContrast));

		if (Options.PartColorValueLDIndex != Preferences.mPartColorValueLDIndex)
			Options.StdErr += Message.arg(QString("--edge-light-dark %1").arg(Options.PartColorValueLDIndex));
	}

	if (!swIsHighContrast(Options.StudStyle) && !swIsHighContrast(swGetProfileStudStyle()))
	{
		QString const Message = tr("High contrast stud style is required for the '%1' option but is not enabled.\n");

		if (Options.StudCylinderColorEnabled != Preferences.mStudCylinderColorEnabled)
			Options.StdErr += Message.arg(QLatin1String("--no-stud-cylinder-color"));

		if (Options.StudCylinderColor != Preferences.mStudCylinderColor)
			Options.StdErr += Message.arg(QString("--stud-cylinder-color %1").arg(Options.StudCylinderColor));

		if (Options.PartEdgeColorEnabled != Preferences.mPartEdgeColorEnabled)
			Options.StdErr += Message.arg(QLatin1String("--no-edge-color"));

		if (Options.PartEdgeColor != Preferences.mPartEdgeColor)
			Options.StdErr += Message.arg(QString("--edge-color %1").arg(Options.PartEdgeColor));

		if (Options.BlackEdgeColorEnabled != Preferences.mBlackEdgeColorEnabled)
			Options.StdErr += Message.arg(QLatin1String("--no-edge-color-black"));

		if (Options.BlackEdgeColor != Preferences.mBlackEdgeColor)
			Options.StdErr += Message.arg(QString("--edge-color-black %1").arg(Options.BlackEdgeColor));

		if (Options.DarkEdgeColorEnabled != Preferences.mDarkEdgeColorEnabled)
			Options.StdErr += Message.arg(QLatin1String("--no-edge-color-dark"));

		if (Options.DarkEdgeColor != Preferences.mDarkEdgeColor)
			Options.StdErr += Message.arg(QString("--edge-color-dark %1").arg(Options.DarkEdgeColor));
	}

	if (!Options.CameraName.isEmpty())
	{
		if (Options.Viewpoint != swViewpoint::Count)
			Options.StdErr += tr("--viewpoint is ignored when --camera is set.\n");

		if (Options.SetProjection)
			Options.StdErr += tr("--projection is ignored when --camera is set.\n");

		if (Options.SetCameraAngles)
			Options.StdErr += tr("--camera-angles is ignored when --camera is set.\n");

		if (Options.SetCameraPosition)
			Options.StdErr += tr("--camera-position is ignored when --camera is set.\n");
	}
	else if (Options.Viewpoint != swViewpoint::Count)
	{
		if (Options.SetCameraAngles)
			Options.StdErr += tr("--camera-angles is ignored when --viewpoint is set.\n");

		if (Options.SetCameraPosition)
			Options.StdErr += tr("--camera-position is ignored when --viewpoint is set.\n");
	}
	else if (Options.SetCameraAngles)
	{
		if (Options.SetCameraPosition)
			Options.StdErr += tr("--camera-position is ignored when --camera-angles is set.\n");
	}

	const bool SaveAndExit = Options.HasSaveAndExit();

	if (SaveAndExit && Options.ProjectName.isEmpty())
	{
		Options.StdErr += tr("No file name specified.\n");
		Options.ParseOK = false;
	}

	return Options;
}
