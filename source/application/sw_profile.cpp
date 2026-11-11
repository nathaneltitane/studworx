#include "sw_global.h"
#include "sw_profile.h"
#include "sw_context.h"
#include "sw_model.h"
#include "sw_project.h"
#include "sw_view_cube.h"
#include "sw_dialog_render.h"
#include "sw_library.h"
#include "sw_widget_selection_part.h"
#include "sw_colors.h"

swProfileEntry::swProfileEntry(const char* Section, const char* Key, int DefaultValue)
{
	mSection = Section;
	mKey = Key;
	mDefault = DefaultValue;
}

swProfileEntry::swProfileEntry(const char* Section, const char* Key, uint DefaultValue)
{
	mSection = Section;
	mKey = Key;
	mDefault = DefaultValue;
}

swProfileEntry::swProfileEntry(const char* Section, const char* Key, float DefaultValue)
{
	mSection = Section;
	mKey = Key;
	mDefault = DefaultValue;
}

swProfileEntry::swProfileEntry(const char* Section, const char* Key, const char* DefaultValue)
{
	mSection = Section;
	mKey = Key;
	mDefault = QString(DefaultValue);
}

swProfileEntry::swProfileEntry(const char* Section, const char* Key, const QStringList& StringList)
{
	mSection = Section;
	mKey = Key;
	mDefault = StringList;
}

swProfileEntry::swProfileEntry(const char* Section, const char* Key)
{
	mSection = Section;
	mKey = Key;
	mDefault = QByteArray();
}

static swProfileEntry gProfileEntries[SW_PROFILE_KEY_COUNT] =
{
	swProfileEntry("Settings", "FixedAxes", false),                                                        // SW_PROFILE_FIXED_AXES
	swProfileEntry("Settings", "LineWidth", 1.0f),                                                         // SW_PROFILE_LINE_WIDTH
	swProfileEntry("Settings", "AllowLOD", true),                                                          // SW_PROFILE_ALLOW_LOD
	swProfileEntry("Settings", "LODDistance", 750.0f),                                                     // SW_PROFILE_LOD_DISTANCE
	swProfileEntry("Settings", "FadeSteps", false),                                                        // SW_PROFILE_FADE_STEPS
	swProfileEntry("Settings", "FadeStepsColor", SW_RGBA(0, 0, 0, 217)),                                   // SW_PROFILE_FADE_STEPS_COLOR #000000d9
	swProfileEntry("Settings", "FadeInline", true),                                                       // SW_PROFILE_FADE_INLINE
	swProfileEntry("Settings", "FadeInlineColor", SW_RGBA(255, 255, 255, 77)),                            // SW_PROFILE_FADE_INLINE_COLOR #ffffff4d
	swProfileEntry("Settings", "FadeJoint", true),                                                        // SW_PROFILE_FADE_JOINT
	swProfileEntry("Settings", "FadeJointColor", SW_RGBA(255, 255, 255, 77)),                             // SW_PROFILE_FADE_JOINT_COLOR #ffffff4d
	swProfileEntry("Settings", "FadeJointMoving", false),                                                 // SW_PROFILE_FADE_JOINT_MOVING
	swProfileEntry("Settings", "HoverOutline", true),                                                     // SW_PROFILE_HOVER_OUTLINE
	swProfileEntry("Settings", "HoverOutlineColor", SW_RGBA(150, 230, 240, 255)),                         // SW_PROFILE_HOVER_OUTLINE_COLOR #96e6f0
	swProfileEntry("Settings", "HighlightNewParts", 0),                                                   // SW_PROFILE_HIGHLIGHT_NEW_PARTS
	swProfileEntry("Settings", "HighlightNewPartsColor", SW_RGBA(255, 85, 0, 217)),                       // SW_PROFILE_HIGHLIGHT_NEW_PARTS_COLOR #ff5500d9
	swProfileEntry("Settings", "ShadingMode", static_cast<int>(swShadingMode::DefaultLights)),             // SW_PROFILE_SHADING_MODE
	swProfileEntry("Settings", "LightingStyle", static_cast<int>(swLightingStyle::Standard)),               // SW_PROFILE_LIGHTING_STYLE
	swProfileEntry("Settings", "HighlightRenderMode", static_cast<int>(swHighlightRenderMode::Outline)),      // SW_PROFILE_HIGHLIGHT_RENDER_MODE
	swProfileEntry("Settings", "SelectionOutlineWidth", 4),                                                // SW_PROFILE_SELECTION_OUTLINE_WIDTH
	swProfileEntry("Settings", "BackgroundColor", SW_RGBA(27, 30, 32, 255)),                               // SW_PROFILE_BACKGROUND_COLOR #1b1e20ff
	swProfileEntry("Settings", "DrawAxes", 0),                                                             // SW_PROFILE_DRAW_AXES
	swProfileEntry("Settings", "FollowStepViews", 1),                                                      // SW_PROFILE_FOLLOW_STEP_VIEWS
	swProfileEntry("Settings", "ProjectionScope", static_cast<int>(swProjectionScope::AllViews)),          // SW_PROFILE_PROJECTION_SCOPE
	swProfileEntry("Settings", "CutawaySize", 4),                                                          // SW_PROFILE_CUTAWAY_SIZE
	swProfileEntry("Settings", "TurntableIdleDuration", 300),                                                  // SW_PROFILE_TURNTABLE_IDLE_DURATION
	swProfileEntry("Settings", "TurntableTurnTime", 30),                                                   // SW_PROFILE_TURNTABLE_TURN_TIME
	swProfileEntry("Settings", "TurntableAxis", static_cast<int>(swTurntableAxis::Z)),                     // SW_PROFILE_TURNTABLE_AXIS
	swProfileEntry("Settings", "DrawAxesLocation", static_cast<int>(swAxisIconLocation::BottomLeft)),      // SW_PROFILE_DRAW_AXES_LOCATION
	swProfileEntry("Settings", "AxisIconSize", 96),                                                       // SW_PROFILE_AXIS_ICON_SIZE
	swProfileEntry("Settings", "AxisIconLabels", 1),                                                      // SW_PROFILE_AXIS_ICON_LABELS
	swProfileEntry("Settings", "AxisIconNegative", 0),                                                    // SW_PROFILE_AXIS_ICON_NEGATIVE
	swProfileEntry("Settings", "AxisIconCubeOutline", 0),                                                 // SW_PROFILE_AXIS_ICON_CUBE_OUTLINE
	swProfileEntry("Settings", "AxisIconCubeOutlineAlpha", 50),                                     // SW_PROFILE_AXIS_ICON_CUBE_OUTLINE_ALPHA
	swProfileEntry("Settings", "TextColor", SW_RGBA(255, 255, 255, 255)),                                  // SW_PROFILE_TEXT_COLOR #ffffffff
	swProfileEntry("Settings", "TextHaloColor", SW_RGBA(27, 30, 32, 255)),                                // SW_PROFILE_TEXT_HALO_COLOR #1b1e20ff
	swProfileEntry("Settings", "MarqueeBorderColor", SW_RGBA(85, 170, 128, 255)),                          // SW_PROFILE_MARQUEE_BORDER_COLOR #55aa80ff
	swProfileEntry("Settings", "MarqueeFillColor", SW_RGBA(42, 46, 50, 0)),                                // SW_PROFILE_MARQUEE_FILL_COLOR #2a2e3200
	swProfileEntry("Settings", "OverlayColor", SW_RGBA(42, 46, 50, 255)),                                  // SW_PROFILE_OVERLAY_COLOR #2a2e32ff
	swProfileEntry("Settings", "CutawayShadeColor", SW_RGBA(255, 255, 255, 25)),                           // SW_PROFILE_CUTAWAY_SHADE_COLOR #ffffff19
	swProfileEntry("Settings", "CutawayShadeAlpha", 50),                                             // SW_PROFILE_CUTAWAY_SHADE_ALPHA
	swProfileEntry("Settings", "CutawayEdges", 1),                                                         // SW_PROFILE_CUTAWAY_EDGES
	swProfileEntry("Settings", "ActiveViewColor", SW_RGBA(85, 85, 127, 255)),                              // SW_PROFILE_ACTIVE_VIEW_COLOR #55557fff
	swProfileEntry("Settings", "InactiveViewColor", SW_RGBA(42, 46, 50, 255)),                             // SW_PROFILE_INACTIVE_VIEW_COLOR #2a2e32ff
	swProfileEntry("Settings", "DrawEdgeLines", 1),                                                        // SW_PROFILE_DRAW_EDGE_LINES
	swProfileEntry("Settings", "DrawConditionalLines", 1),                                                 // SW_PROFILE_DRAW_CONDITIONAL_LINES
	swProfileEntry("Settings", "PreviewSyncRendering", 1),                                                   // SW_PROFILE_PREVIEW_SYNCHRONIZED_RENDERING
	swProfileEntry("Settings", "PreviewShadingMode", static_cast<int>(swShadingMode::DefaultLights)),        // SW_PROFILE_PREVIEW_SHADING_MODE
	swProfileEntry("Settings", "PreviewDrawEdgeLines", 1),                                                   // SW_PROFILE_PREVIEW_DRAW_EDGE_LINES
	swProfileEntry("Settings", "PreviewDrawConditionalLines", 1),                                            // SW_PROFILE_PREVIEW_DRAW_CONDITIONAL_LINES
	swProfileEntry("Settings", "PreviewLineWidth", 1.0f),                                                    // SW_PROFILE_PREVIEW_LINE_WIDTH
	swProfileEntry("Settings", "PartsSyncRendering", 1),                                                    // SW_PROFILE_PARTS_SYNCHRONIZED_RENDERING
	swProfileEntry("Settings", "PartsShadingMode", static_cast<int>(swShadingMode::DefaultLights)),         // SW_PROFILE_PARTS_SHADING_MODE
	swProfileEntry("Settings", "PartsDrawEdgeLines", 1),                                                    // SW_PROFILE_PARTS_DRAW_EDGE_LINES
	swProfileEntry("Settings", "PartsDrawConditionalLines", 1),                                             // SW_PROFILE_PARTS_DRAW_CONDITIONAL_LINES
	swProfileEntry("Settings", "PartsLineWidth", 1.0f),                                                     // SW_PROFILE_PARTS_LINE_WIDTH
	swProfileEntry("Settings", "GridStuds", 1),                                                            // SW_PROFILE_GRID_STUDS
	swProfileEntry("Settings", "GridStudColor", SW_RGBA(42, 46, 50, 255)),                                 // SW_PROFILE_GRID_STUD_COLOR #2a2e32ff
	swProfileEntry("Settings", "GridLines", 1),                                                            // SW_PROFILE_GRID_LINES
	swProfileEntry("Settings", "GridLineSpacing", 5),                                                      // SW_PROFILE_GRID_LINE_SPACING
	swProfileEntry("Settings", "GridLineColor", SW_RGBA(42, 46, 50, 255)),                                 // SW_PROFILE_GRID_LINE_COLOR #2a2e32ff
	swProfileEntry("Settings", "GridNegativeAxes", 1),                                                     // SW_PROFILE_GRID_NEGATIVE_AXES
	swProfileEntry("Settings", "GridEnabled", true),                                                       // SW_PROFILE_GRID_ENABLED
	swProfileEntry("Settings", "GridHeightXZ", 0),                                                         // SW_PROFILE_GRID_HEIGHT_XZ
	swProfileEntry("Settings", "GridHeightYZ", 0),                                                         // SW_PROFILE_GRID_HEIGHT_YZ
	swProfileEntry("Settings", "GridHeightBehindModel", 0),                                                // SW_PROFILE_GRID_HEIGHT_BEHIND_MODEL
	swProfileEntry("Settings", "GridAxisX", 1),                                                            // SW_PROFILE_GRID_AXIS_X
	swProfileEntry("Settings", "GridAxisY", 1),                                                            // SW_PROFILE_GRID_AXIS_Y
	swProfileEntry("Settings", "GridAxisZ", 1),                                                            // SW_PROFILE_GRID_AXIS_Z
	swProfileEntry("Settings", "AASamples", 1),                                                            // SW_PROFILE_ANTIALIASING_SAMPLES
	swProfileEntry("Settings", "ViewCubeEnabled", 1),                                                      // SW_PROFILE_VIEW_CUBE_ENABLED
	swProfileEntry("Settings", "ViewCubeLocation", (int)swViewCubeLocation::TopRight),                   // SW_PROFILE_VIEW_CUBE_LOCATION
	swProfileEntry("Settings", "ViewCubeSize", 96),                                                       // SW_PROFILE_VIEW_CUBE_SIZE
	swProfileEntry("Settings", "ViewCubeColor", SW_RGBA(42, 46, 50, 255)),                                 // SW_PROFILE_VIEW_CUBE_COLOR #2a2e32ff
	swProfileEntry("Settings", "ViewCubeTextColor", SW_RGBA(255, 255, 255, 255)),                          // SW_PROFILE_VIEW_CUBE_TEXT_COLOR #ffffffff
	swProfileEntry("Settings", "ViewCubeHighlightColor", SW_RGBA(255, 85, 127, 255)),                      // SW_PROFILE_VIEW_CUBE_HIGHLIGHT_COLOR #ff557fff
	swProfileEntry("Settings", "ViewCubeLabelAbbreviations", 0),                                           // SW_PROFILE_VIEW_CUBE_LABEL_ABBREVIATIONS
	swProfileEntry("Settings", "AxisXColor", SW_RGBA(255, 85, 127, 255)),                                  // SW_PROFILE_AXIS_X_COLOR #ff557fff
	swProfileEntry("Settings", "AxisYColor", SW_RGBA(100, 200, 20, 255)),                                  // SW_PROFILE_AXIS_Y_COLOR #64c814ff
	swProfileEntry("Settings", "AxisZColor", SW_RGBA(0, 170, 255, 255)),                                   // SW_PROFILE_AXIS_Z_COLOR #00aaffff
	swProfileEntry("Settings", "AxisXNegativeColor", SW_RGBA(150, 62, 72, 255)),                          // SW_PROFILE_AXIS_X_NEGATIVE_COLOR #963e48ff
	swProfileEntry("Settings", "AxisYNegativeColor", SW_RGBA(96, 150, 50, 255)),                          // SW_PROFILE_AXIS_Y_NEGATIVE_COLOR #609632ff
	swProfileEntry("Settings", "AxisZNegativeColor", SW_RGBA(62, 74, 168, 255)),                          // SW_PROFILE_AXIS_Z_NEGATIVE_COLOR #3e4aa8ff
	swProfileEntry("Settings", "ObjectSelectedColor", static_cast<uint>(SW_RGBA(255, 255, 127, 255))),     // SW_PROFILE_OBJECT_SELECTED_COLOR #ffff7fff
	swProfileEntry("Settings", "ObjectFocusedColor", static_cast<uint>(SW_RGBA(102, 76, 229, 255))),       // SW_PROFILE_OBJECT_FOCUSED_COLOR #664ce5ff
	swProfileEntry("Settings", "CameraColor", static_cast<uint>(SW_RGBA(128, 204, 128, 255))),             // SW_PROFILE_CAMERA_COLOR #80cc80ff
	swProfileEntry("Settings", "LightColor", static_cast<uint>(SW_RGBA(255, 170, 0, 255))),                // SW_PROFILE_LIGHT_COLOR #ffaa00ff
	swProfileEntry("Settings", "ControlPointColor", static_cast<uint>(SW_RGBA(181, 0, 255, 128))),         // SW_PROFILE_CONTROL_POINT_COLOR #b500ff80
	swProfileEntry("Settings", "ControlPointFocusedColor", static_cast<uint>(SW_RGBA(0, 170, 255, 128))),  // SW_PROFILE_CONTROL_POINT_FOCUSED_COLOR #00aaff80
	swProfileEntry("Settings", "ControlPointSeatedColor", static_cast<uint>(SW_RGBA(255, 178, 0, 192))),   // SW_PROFILE_CONTROL_POINT_SEATED_COLOR #ffb200c0
	swProfileEntry("Settings", "PathColor", static_cast<uint>(SW_RGBA(102, 76, 229, 255))),               // SW_PROFILE_PATH_COLOR #664ce5ff
	swProfileEntry("Settings", "PathOutOfReachColor", static_cast<uint>(SW_RGBA(229, 76, 76, 255))),       // SW_PROFILE_PATH_OUT_OF_REACH_COLOR #e54c4cff
	swProfileEntry("Settings", "SpreadHandleColor", static_cast<uint>(SW_RGBA(0, 85, 255, 255))),          // SW_PROFILE_SPREAD_HANDLE_COLOR #0055ffff
	swProfileEntry("Settings", "ExplodeLineColor", static_cast<uint>(SW_RGBA(255, 85, 0, 255))),         // SW_PROFILE_EXPLODE_LINE_COLOR #ff5500ff

	swProfileEntry("Settings", "Language", ""),                                                // SW_PROFILE_LANGUAGE
	swProfileEntry("Settings", "ColorTheme", static_cast<int>(swColorTheme::Dark)),            // SW_PROFILE_COLOR_THEME
	swProfileEntry("Settings", "ProjectsPath", ""),                                            // SW_PROFILE_PROJECTS_PATH
	swProfileEntry("Settings", "PartsLibrary", ""),                                           // SW_PROFILE_PARTS_LIBRARY
	swProfileEntry("Settings", "UnofficialLibrary", ""),                                      // SW_PROFILE_UNOFFICIAL_LIBRARY
	swProfileEntry("Settings", "ShadowLibrary", ""),                                          // SW_PROFILE_SHADOW_LIBRARY
	swProfileEntry("Settings", "ShadowLibraryEnabled", true),                                 // SW_PROFILE_SHADOW_LIBRARY_ENABLED
	swProfileEntry("Settings", "PartPalettes"),                                               // SW_PROFILE_PART_PALETTES
	swProfileEntry("Settings", "MinifigureSettings", ""),                                         // SW_PROFILE_MINIFIGURE_BUILDER_SETTINGS
	swProfileEntry("Settings", "ColorConfig", ""),                                             // SW_PROFILE_COLOR_CONFIGURATION
	swProfileEntry("Settings", "ColorConfigAlternate", ""),                                    // SW_PROFILE_COLOR_CONFIGURATION_ALTERNATE
	swProfileEntry("Settings", "ColorSource", -1),                                             // SW_PROFILE_COLOR_SOURCE
	swProfileEntry("Settings", "Shortcuts"),                                                   // SW_PROFILE_KEYBOARD_SHORTCUTS
	swProfileEntry("Settings", "MouseShortcuts", QStringList()),                               // SW_PROFILE_MOUSE_SHORTCUTS
	swProfileEntry("Settings", "Categories"),                                                  // SW_PROFILE_CATEGORIES
	swProfileEntry("Settings", "RecentFile1", ""),                                             // SW_PROFILE_RECENT_FILE1
	swProfileEntry("Settings", "RecentFile2", ""),                                             // SW_PROFILE_RECENT_FILE2
	swProfileEntry("Settings", "RecentFile3", ""),                                             // SW_PROFILE_RECENT_FILE3
	swProfileEntry("Settings", "RecentFile4", ""),                                             // SW_PROFILE_RECENT_FILE4
	swProfileEntry("Settings", "AutoLoadMostRecent", false),                                   // SW_PROFILE_AUTO_LOAD_MOST_RECENT
	swProfileEntry("Settings", "RestoreTabLayout", true),                                      // SW_PROFILE_RESTORE_TAB_LAYOUT
	swProfileEntry("Settings", "AngleSnapCustom", 7.5f),                                       // SW_PROFILE_ANGLE_SNAP_CUSTOM
	swProfileEntry("Settings", "OrbitSnap", false),                                            // SW_PROFILE_ORBIT_SNAP
	swProfileEntry("Settings", "StatusBarSegments", 31),                                       // SW_PROFILE_STATUS_BAR_SEGMENTS
	swProfileEntry("Settings", "FirstPartAtOrigin", false),                                   // SW_PROFILE_FIRST_PART_AT_ORIGIN
	swProfileEntry("Settings", "JointRotation", false),                                       // SW_PROFILE_JOINT_ROTATION
	swProfileEntry("Settings", "ContextMenu", QStringList()),                                 // SW_PROFILE_CONTEXT_MENU
	swProfileEntry("Settings", "TerminalFollowModel", true),                                   // SW_PROFILE_TERMINAL_FOLLOW_MODEL
	swProfileEntry("Settings", "InsertColorMode", 0),                                          // SW_PROFILE_INSERT_COLOR_MODE
	swProfileEntry("Settings", "FlexSpreadMode", 1),                                           // SW_PROFILE_FLEX_SPREAD_MODE
	swProfileEntry("Settings", "FlexClampNewControlPoints", false),                            // SW_PROFILE_FLEX_CLAMP_NEW_CONTROL_POINTS
	swProfileEntry("Settings", "FlexMiddleControlPoint", false),                               // SW_PROFILE_FLEX_MIDDLE_CONTROL_POINT
	swProfileEntry("Settings", "WatchFileChanges", true),                                      // SW_PROFILE_WATCH_FILE_CHANGES
	swProfileEntry("Settings", "LoadPartsBeforeShowing", false),                              // SW_PROFILE_LOAD_PARTS_BEFORE_SHOWING
	swProfileEntry("Settings", "WarnPartsLoadFailure", true),                                 // SW_PROFILE_WARN_PARTS_LOAD_FAILURE
	swProfileEntry("Settings", "MigrateMetaCommands", 0),                                      // SW_PROFILE_MIGRATE_META_COMMANDS
	swProfileEntry("Settings", "MarkColladaTransparency", true),                               // SW_PROFILE_MARK_COLLADA_TRANSPARENCY
	swProfileEntry("Settings", "KMZVisibleSurfaces", true),                                    // SW_PROFILE_KMZ_VISIBLE_SURFACES
	swProfileEntry("Settings", "AutosaveInterval", 10),                                        // SW_PROFILE_AUTO_SAVE_INTERVAL
	swProfileEntry("Settings", "MouseSensitivity", 11),                                        // SW_PROFILE_MOUSE_SENSITIVITY
	swProfileEntry("Settings", "ImageWidth", 1024),                                            // SW_PROFILE_IMAGE_WIDTH
	swProfileEntry("Settings", "ImageHeight", 1024),                                           // SW_PROFILE_IMAGE_HEIGHT
	swProfileEntry("Settings", "ImageExtension", ".png"),                                      // SW_PROFILE_IMAGE_EXTENSION
	swProfileEntry("Settings", "ImageFullTurn", true),                                         // SW_PROFILE_IMAGE_FULL_TURN
	swProfileEntry("Settings", "ImageAngleStart", -45),                                        // SW_PROFILE_IMAGE_ANGLE_START
	swProfileEntry("Settings", "ImageAngleEnd", 45),                                           // SW_PROFILE_IMAGE_ANGLE_END
	swProfileEntry("Settings", "ImageFrames", 36),                                             // SW_PROFILE_IMAGE_FRAMES
	swProfileEntry("Settings", "ImageFrameRate", 12.0f),                                       // SW_PROFILE_IMAGE_FRAME_RATE
	swProfileEntry("Settings", "ImageFit", 1),                                                 // SW_PROFILE_IMAGE_FIT
	swProfileEntry("Settings", "ImageCenter", 2),                                              // SW_PROFILE_IMAGE_CENTER
	swProfileEntry("Settings", "ImageFitScope", 0),                                            // SW_PROFILE_IMAGE_FIT_SCOPE
	swProfileEntry("Settings", "ImageMargin", 10),                                             // SW_PROFILE_IMAGE_MARGIN
	swProfileEntry("Settings", "ImageRatio", 1.0f),                                            // SW_PROFILE_IMAGE_RATIO
	swProfileEntry("Settings", "ImageSteps", 0),                                               // SW_PROFILE_IMAGE_STEPS
	swProfileEntry("Settings", "ImageTransparent", false),                                     // SW_PROFILE_IMAGE_TRANSPARENT
	swProfileEntry("Settings", "ImageAxis", 2),                                                // SW_PROFILE_IMAGE_AXIS
	swProfileEntry("Settings", "ImageMotion", 0),                                              // SW_PROFILE_IMAGE_MOTION
	swProfileEntry("Settings", "ImageTurns", 1),                                               // SW_PROFILE_IMAGE_TURNS
	swProfileEntry("Settings", "ImageNextStep", 0),                                            // SW_PROFILE_IMAGE_NEXT_STEP
	swProfileEntry("Settings", "ImageNextStepTurns", 1),                                       // SW_PROFILE_IMAGE_NEXT_STEP_TURNS
	swProfileEntry("Settings", "ImageNextStepSeconds", 2.0f),                                  // SW_PROFILE_IMAGE_NEXT_STEP_SECONDS
	swProfileEntry("Settings", "ImageHoldLast", 1.0f),                                         // SW_PROFILE_IMAGE_HOLD_LAST
	swProfileEntry("Settings", "PartsListIcons", 64),                                         // SW_PROFILE_PARTS_LIST_ICONS
	swProfileEntry("Settings", "PartsListNames", 1),                                          // SW_PROFILE_PARTS_LIST_NAMES
	swProfileEntry("Settings", "PartsListFixedColor", static_cast<uint>(SW_COLOR_NO_COLOR)),                                    // SW_PROFILE_PARTS_LIST_COLOR
	swProfileEntry("Settings", "PartsListColorLocked", false),                               // SW_PROFILE_PARTS_LIST_COLOR_LOCKED
	swProfileEntry("Settings", "PartsListDecorated", 1),                                      // SW_PROFILE_PARTS_LIST_DECORATED
	swProfileEntry("Settings", "PartsListAliases", 1),                                        // SW_PROFILE_PARTS_LIST_ALIASES
	swProfileEntry("Settings", "PartsListListMode", 0),                                       // SW_PROFILE_PARTS_LIST_LIST_MODE
	swProfileEntry("Settings", "PartsPopupIcons", 64),                                            // SW_PROFILE_PARTS_POPUP_ICONS
	swProfileEntry("Settings", "PartsPopupNames", 1),                                             // SW_PROFILE_PARTS_POPUP_NAMES
	swProfileEntry("Settings", "PartsPopupDecorated", 1),                                         // SW_PROFILE_PARTS_POPUP_DECORATED
	swProfileEntry("Settings", "PartsPopupAliases", 1),                                           // SW_PROFILE_PARTS_POPUP_ALIASES
	swProfileEntry("Settings", "PartsPopupListMode", 0),                                          // SW_PROFILE_PARTS_POPUP_LIST_MODE
	swProfileEntry("Settings", "PartsListPartFilter", static_cast<int>(swPartFilterType::Default)),                                    // SW_PROFILE_PARTS_LIST_PART_FILTER
	swProfileEntry("Settings", "PartsListCaseSensitiveFilter", 0),                            // SW_PROFILE_PARTS_LIST_CASE_SENSITIVE_FILTER
	swProfileEntry("Settings", "PartsListFileNameFilter", 1),                                 // SW_PROFILE_PARTS_LIST_FILE_NAME_FILTER
	swProfileEntry("Settings", "PartsListPartDescriptionFilter", 1),                         // SW_PROFILE_PARTS_LIST_PART_DESCRIPTION_FILTER
	swProfileEntry("Settings", "StudStyle", static_cast<int>(swStudStyle::Plain)),                                                // SW_PROFILE_STUD_STYLE

	swProfileEntry("Defaults", "Author", "Author"),                                            // SW_PROFILE_DEFAULT_AUTHOR_NAME
	swProfileEntry("Defaults", "AuthorEmail", ""),                                             // SW_PROFILE_DEFAULT_AUTHOR_EMAIL
	swProfileEntry("Defaults", "AuthorComment", ""),                                           // SW_PROFILE_DEFAULT_AUTHOR_COMMENT
	swProfileEntry("Defaults", "AuthorSplitter", " | "),                                       // SW_PROFILE_DEFAULT_AUTHOR_SPLITTER
	swProfileEntry("Defaults", "AmbientColor", SW_RGBA(75, 75, 75, 255)),                           // SW_PROFILE_DEFAULT_AMBIENT_COLOR #4b4b4bff

	swProfileEntry("HTML", "Options", SW_HTML_SINGLE_PAGE),                                     // SW_PROFILE_HTML_OPTIONS
	swProfileEntry("HTML", "ImageOptions", 0),                                                  // SW_PROFILE_HTML_IMAGE_OPTIONS
	swProfileEntry("HTML", "ImageWidth", 640),                                                 // SW_PROFILE_HTML_IMAGE_WIDTH
	swProfileEntry("HTML", "ImageHeight", 480),                                                // SW_PROFILE_HTML_IMAGE_HEIGHT
	swProfileEntry("HTML", "ArrowPosition", 0),                                                // SW_PROFILE_HTML_ARROW_POSITION
	swProfileEntry("HTML", "BackgroundColor", SW_RGBA(0, 0, 0, 255)),                          // SW_PROFILE_HTML_BACKGROUND_COLOR #000000ff
	swProfileEntry("HTML", "OverrideColors", QStringList()),                                  // SW_PROFILE_HTML_OVERRIDE_COLORS
	swProfileEntry("HTML", "Display", 0),                                                     // SW_PROFILE_HTML_DISPLAY
	swProfileEntry("HTML", "StyleSheet", ""),                                                 // SW_PROFILE_HTML_STYLE_SHEET

	swProfileEntry("POVRay", "Path", "/usr/bin/povray"),                                       // SW_PROFILE_POVRAY_PATH
	swProfileEntry("POVRay", "LGEOPath", ""),                                                  // SW_PROFILE_POVRAY_LGEO_PATH

	swProfileEntry("Render", "Width", 1280),                                                   // SW_PROFILE_RENDER_WIDTH
	swProfileEntry("Render", "Height", 720),                                                   // SW_PROFILE_RENDER_HEIGHT
	swProfileEntry("Render", "Quality", (int)swPOVRayRenderQuality::High),                     // SW_PROFILE_RENDER_QUALITY
	swProfileEntry("Render", "LightRig", (int)swPOVRayLightRig::Standard),                     // SW_PROFILE_RENDER_LIGHT_RIG
	swProfileEntry("Render", "FloorStyle", (int)swPOVRayFloorStyle::Plain),                    // SW_PROFILE_RENDER_FLOOR_STYLE
	swProfileEntry("Render", "EnvironmentImage", ""),                                          // SW_PROFILE_RENDER_ENVIRONMENT_IMAGE
	swProfileEntry("Render", "Transparent", 0),                                                // SW_PROFILE_RENDER_TRANSPARENT
	swProfileEntry("Render", "Presets", QStringList({ "Draft|640|360|0|0|0|0|", "HD|1280|720|1|0|0|0|", "Full HD|1920|1080|2|0|0|0|", "Square|1080|1080|2|1|1|0|" })), // SW_PROFILE_RENDER_PRESETS
	swProfileEntry("Render", "Area", 0),                                                       // SW_PROFILE_RENDER_AREA
	swProfileEntry("Render", "Region", "0.25 0.25 0.5 0.5"),                                   // SW_PROFILE_RENDER_REGION
	swProfileEntry("Render", "Steps", 0),                                                      // SW_PROFILE_RENDER_STEPS
	swProfileEntry("Render", "Cameras", ""),                                                   // SW_PROFILE_RENDER_CAMERAS
	swProfileEntry("Render", "Parts", 0),                                                      // SW_PROFILE_RENDER_PARTS

	swProfileEntry("Blender", "Path", ""),                                                     // SW_PROFILE_BLENDER_PATH
	swProfileEntry("Blender", "Version", ""),                                                  // SW_PROFILE_BLENDER_VERSION
	swProfileEntry("Blender", "CropImage", 0),                                                 // SW_PROFILE_BLENDER_CROP_IMAGE

	swProfileEntry("Settings", "PreviewViewCubeEnabled", 0),                                      // SW_PROFILE_PREVIEW_VIEW_CUBE_ENABLED
	swProfileEntry("Settings", "PreviewViewCubeSize", 64),                                        // SW_PROFILE_PREVIEW_VIEW_CUBE_SIZE
	swProfileEntry("Settings", "PreviewViewCubeLocation", (int)swViewCubeLocation::TopRight),   // SW_PROFILE_PREVIEW_VIEW_CUBE_LOCATION
	swProfileEntry("Settings", "DrawPreviewAxis", 0),                                             // SW_PROFILE_PREVIEW_DRAW_AXES
	swProfileEntry("Settings", "PreviewAxisIconSize", 64),                                         // SW_PROFILE_PREVIEW_AXIS_ICON_SIZE

	swProfileEntry("Settings", "StudCylinderColorEnabled", true),                              // SW_PROFILE_STUD_CYLINDER_COLOR_ENABLED
	swProfileEntry("Settings", "StudCylinderColor", SW_RGBA(27, 42, 52, 255)),                 // SW_PROFILE_STUD_CYLINDER_COLOR #1b2a34ff
	swProfileEntry("Settings", "PartEdgeColorEnabled", true),                                 // SW_PROFILE_PART_EDGE_COLOR_ENABLED
	swProfileEntry("Settings", "PartEdgeColor", SW_RGBA(0, 0, 0, 255)),                       // SW_PROFILE_PART_EDGE_COLOR #000000ff
	swProfileEntry("Settings", "BlackEdgeColorEnabled", true),                                 // SW_PROFILE_BLACK_EDGE_COLOR_ENABLED
	swProfileEntry("Settings", "BlackEdgeColor", SW_RGBA(255, 255, 255, 255)),                 // SW_PROFILE_BLACK_EDGE_COLOR #ffffffff
	swProfileEntry("Settings", "DarkEdgeColorEnabled", true),                                  // SW_PROFILE_DARK_EDGE_COLOR_ENABLED
	swProfileEntry("Settings", "DarkEdgeColor", SW_RGBA(27, 42, 52, 255)),                     // SW_PROFILE_DARK_EDGE_COLOR #1b2a34ff
	swProfileEntry("Settings", "PartEdgeContrast", 0.5f),                                     // SW_PROFILE_PART_EDGE_CONTRAST
	swProfileEntry("Settings", "PartColorValueLDIndex", 0.5f),                                // SW_PROFILE_PART_COLOR_VALUE_LD_INDEX
	swProfileEntry("Settings", "AutomateEdgeColor", 0),                                        // SW_PROFILE_AUTOMATE_EDGE_COLOR

	swProfileEntry("Instructions", "PageWidth", 1008.0f),                                      // SW_PROFILE_INSTRUCTIONS_PAGE_WIDTH
	swProfileEntry("Instructions", "PageHeight", 612.0f),                                      // SW_PROFILE_INSTRUCTIONS_PAGE_HEIGHT
	swProfileEntry("Instructions", "MarginLeft", 36.0f),                                       // SW_PROFILE_INSTRUCTIONS_MARGIN_LEFT
	swProfileEntry("Instructions", "MarginRight", 36.0f),                                      // SW_PROFILE_INSTRUCTIONS_MARGIN_RIGHT
	swProfileEntry("Instructions", "MarginTop", 36.0f),                                        // SW_PROFILE_INSTRUCTIONS_MARGIN_TOP
	swProfileEntry("Instructions", "MarginBottom", 36.0f),                                     // SW_PROFILE_INSTRUCTIONS_MARGIN_BOTTOM
	swProfileEntry("Instructions", "PageUnits", 3),                                            // SW_PROFILE_INSTRUCTIONS_PAGE_UNITS
	swProfileEntry("Instructions", "MarginUnits", 3),                                          // SW_PROFILE_INSTRUCTIONS_MARGIN_UNITS
	swProfileEntry("Instructions", "ShowGrid", 1),                                             // SW_PROFILE_INSTRUCTIONS_SHOW_GRID
	swProfileEntry("Instructions", "SnapToGrid", 1),                                           // SW_PROFILE_INSTRUCTIONS_SNAP_TO_GRID
	swProfileEntry("Instructions", "SnapToAnchors", 0),                                        // SW_PROFILE_INSTRUCTIONS_SNAP_TO_ANCHORS
	swProfileEntry("Instructions", "GridSize", 72.0f),                                         // SW_PROFILE_INSTRUCTIONS_GRID_SIZE
	swProfileEntry("Instructions", "GridWithinMargins", 1),                                    // SW_PROFILE_INSTRUCTIONS_GRID_WITHIN_MARGINS
	swProfileEntry("Instructions", "ShowMargins", 1),                                          // SW_PROFILE_INSTRUCTIONS_SHOW_MARGINS
	swProfileEntry("Instructions", "EditorHighlight", 1),                                      // SW_PROFILE_INSTRUCTIONS_EDITOR_HIGHLIGHT
	swProfileEntry("Instructions", "EditorNumbers", 1),                                        // SW_PROFILE_INSTRUCTIONS_EDITOR_NUMBERS
	swProfileEntry("Instructions", "EditorWrap", 0),                                           // SW_PROFILE_INSTRUCTIONS_EDITOR_WRAP
	swProfileEntry("Instructions", "EditorMarkers", 1),                                        // SW_PROFILE_INSTRUCTIONS_EDITOR_MARKERS
	swProfileEntry("Instructions", "NewModelDefaults", 1),                                     // SW_PROFILE_INSTRUCTIONS_NEW_MODEL_DEFAULTS
	swProfileEntry("Instructions", "GridDivisions", 8),                                        // SW_PROFILE_INSTRUCTIONS_GRID_DIVISIONS
	swProfileEntry("Instructions", "GridPrimaryColor", SW_RGBA(61, 174, 233, 110)),            // SW_PROFILE_INSTRUCTIONS_GRID_PRIMARY_COLOR #3daee96e
	swProfileEntry("Instructions", "GridSecondaryColor", SW_RGBA(61, 174, 233, 40)),           // SW_PROFILE_INSTRUCTIONS_GRID_SECONDARY_COLOR #3daee928
	swProfileEntry("Instructions", "MarginColor", SW_RGBA(61, 174, 233, 160)),                 // SW_PROFILE_INSTRUCTIONS_MARGIN_COLOR #3daee9a0
	swProfileEntry("Instructions", "ShowElementMargins", 0),                                   // SW_PROFILE_INSTRUCTIONS_SHOW_ELEMENT_MARGINS
	swProfileEntry("Instructions", "ShowElementPadding", 0),                                   // SW_PROFILE_INSTRUCTIONS_SHOW_ELEMENT_PADDING
	swProfileEntry("Instructions", "ShowElementAnchors", 0),                                   // SW_PROFILE_INSTRUCTIONS_SHOW_ELEMENT_ANCHORS
	swProfileEntry("Instructions", "ShowRulers", 1),                                           // SW_PROFILE_INSTRUCTIONS_SHOW_RULERS
	swProfileEntry("Instructions", "ElementMarginColor", SW_RGBA(246, 178, 107, 255)),         // SW_PROFILE_INSTRUCTIONS_ELEMENT_MARGIN_COLOR #f6b26bff
	swProfileEntry("Instructions", "ElementMarginAlpha", 66),                                  // SW_PROFILE_INSTRUCTIONS_ELEMENT_MARGIN_ALPHA
	swProfileEntry("Instructions", "ElementPaddingColor", SW_RGBA(147, 196, 125, 255)),        // SW_PROFILE_INSTRUCTIONS_ELEMENT_PADDING_COLOR #93c47dff
	swProfileEntry("Instructions", "ElementPaddingAlpha", 55),                                 // SW_PROFILE_INSTRUCTIONS_ELEMENT_PADDING_ALPHA
	swProfileEntry("Instructions", "ElementAnchorColor", SW_RGBA(214, 51, 160, 255)),          // SW_PROFILE_INSTRUCTIONS_ELEMENT_ANCHOR_COLOR #d633a0ff
	swProfileEntry("Instructions", "ElementAnchorAlpha", 100),                                 // SW_PROFILE_INSTRUCTIONS_ELEMENT_ANCHOR_ALPHA
	swProfileEntry("Instructions", "RulerPointerColor", SW_RGBA(230, 40, 40, 255)),            // SW_PROFILE_INSTRUCTIONS_RULER_POINTER_COLOR #e62828ff
	swProfileEntry("Instructions", "RulerPointerAlpha", 100),                                  // SW_PROFILE_INSTRUCTIONS_RULER_POINTER_ALPHA
	swProfileEntry("InstructionsDefaults", "PageWidth", 1008.0f),                              // SW_PROFILE_INSTRUCTIONS_DEFAULT_PAGE_WIDTH
	swProfileEntry("InstructionsDefaults", "PageHeight", 612.0f),                              // SW_PROFILE_INSTRUCTIONS_DEFAULT_PAGE_HEIGHT
	swProfileEntry("InstructionsDefaults", "MarginLeft", 36.0f),                               // SW_PROFILE_INSTRUCTIONS_DEFAULT_MARGIN_LEFT
	swProfileEntry("InstructionsDefaults", "MarginRight", 36.0f),                              // SW_PROFILE_INSTRUCTIONS_DEFAULT_MARGIN_RIGHT
	swProfileEntry("InstructionsDefaults", "MarginTop", 36.0f),                                // SW_PROFILE_INSTRUCTIONS_DEFAULT_MARGIN_TOP
	swProfileEntry("InstructionsDefaults", "MarginBottom", 36.0f),                             // SW_PROFILE_INSTRUCTIONS_DEFAULT_MARGIN_BOTTOM
	swProfileEntry("InstructionsDefaults", "PageUnits", 3),                                    // SW_PROFILE_INSTRUCTIONS_DEFAULT_PAGE_UNITS
	swProfileEntry("InstructionsDefaults", "MarginUnits", 3),                                  // SW_PROFILE_INSTRUCTIONS_DEFAULT_MARGIN_UNITS
	swProfileEntry("InstructionsDefaults", "ShowGrid", 1),                                     // SW_PROFILE_INSTRUCTIONS_DEFAULT_SHOW_GRID
	swProfileEntry("InstructionsDefaults", "SnapToGrid", 1),                                   // SW_PROFILE_INSTRUCTIONS_DEFAULT_SNAP_TO_GRID
	swProfileEntry("InstructionsDefaults", "GridSize", 72.0f),                                 // SW_PROFILE_INSTRUCTIONS_DEFAULT_GRID_SIZE
	swProfileEntry("InstructionsDefaults", "GridWithinMargins", 1),                            // SW_PROFILE_INSTRUCTIONS_DEFAULT_GRID_WITHIN_MARGINS
	swProfileEntry("InstructionsDefaults", "GridDivisions", 8),                                // SW_PROFILE_INSTRUCTIONS_DEFAULT_GRID_DIVISIONS
	swProfileEntry("Settings", "PreviewKeepSelection", 0),                                     // SW_PROFILE_PREVIEW_KEEP_SELECTION
	swProfileEntry("Settings", "MinifigureSyncRendering", 1),                                     // SW_PROFILE_MINIFIGURE_BUILDER_SYNCHRONIZED_RENDERING
	swProfileEntry("Settings", "MinifigureShadingMode", static_cast<int>(swShadingMode::DefaultLights)), // SW_PROFILE_MINIFIGURE_BUILDER_SHADING_MODE
	swProfileEntry("Settings", "MinifigureDrawEdgeLines", 1),                                     // SW_PROFILE_MINIFIGURE_BUILDER_DRAW_EDGE_LINES
	swProfileEntry("Settings", "MinifigureDrawConditionalLines", 1),                              // SW_PROFILE_MINIFIGURE_BUILDER_DRAW_CONDITIONAL_LINES
	swProfileEntry("Settings", "MinifigureLineWidth", 1.0f),                                      // SW_PROFILE_MINIFIGURE_BUILDER_LINE_WIDTH
	swProfileEntry("Settings", "PartsSnapToTiles", 1),                                        // SW_PROFILE_PARTS_SNAP_TO_TILES
	swProfileEntry("Settings", "RebrickableApiKey", ""),                                       // SW_PROFILE_REBRICKABLE_API_KEY
	swProfileEntry("Settings", "BrickLinkMapping", ""),                                        // SW_PROFILE_BRICKLINK_MAPPING
	swProfileEntry("Settings", "BrickLinkCondition", 0),                                       // SW_PROFILE_BRICKLINK_CONDITION
	swProfileEntry("Settings", "BrickLinkWantedList", ""),                                     // SW_PROFILE_BRICKLINK_WANTED_LIST
	swProfileEntry("Settings", "BrickLinkNotify", false),                                      // SW_PROFILE_BRICKLINK_NOTIFY
	swProfileEntry("Settings", "BrickLinkExceptions", "")                                      // SW_PROFILE_BRICKLINK_EXCEPTIONS
};

// settings shared by the open batch
static QSettings* gProfileBatchSettings = nullptr;

swProfileBatch::swProfileBatch()
{
	if (gProfileBatchSettings)
		return;

	mSettings = std::make_unique<QSettings>();
	gProfileBatchSettings = mSettings.get();
}

swProfileBatch::~swProfileBatch()
{
	if (mSettings)
		gProfileBatchSettings = nullptr;
}

// use to read a key through the open batch or its own settings
static QVariant swGetProfileValue(const swProfileEntry& Entry, const QVariant& DefaultValue = QVariant())
{
	const QString Name = QString("%1/%2").arg(Entry.mSection, Entry.mKey);

	if (gProfileBatchSettings)
		return gProfileBatchSettings->value(Name, DefaultValue);

	return QSettings().value(Name, DefaultValue);
}

// use to write a key through the open batch or its own settings
static void swSetProfileValue(const swProfileEntry& Entry, const QVariant& Value)
{
	const QString Name = QString("%1/%2").arg(Entry.mSection, Entry.mKey);

	if (gProfileBatchSettings)
	{
		gProfileBatchSettings->setValue(Name, Value);
		return;
	}

	QSettings Settings;

	Settings.setValue(Name, Value);
}

void swProfileInitialize()
{
	gProfileEntries[SW_PROFILE_PROJECTS_PATH].mDefault = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
}

// every setting gets a line in the settings file
void swProfileWriteMissing()
{
	QSettings Settings;

	for (const swProfileEntry& Entry : gProfileEntries)
	{
		const QString Name = QString("%1/%2").arg(Entry.mSection, Entry.mKey);

		if (!Settings.contains(Name))
			Settings.setValue(Name, std::visit([](const auto& Value) { return QVariant::fromValue(Value); }, Entry.mDefault));
	}
}

void swRemoveProfileKey(SW_PROFILE_KEY Key)
{
	swProfileEntry& Entry = gProfileEntries[Key];
	const QString Name = QString("%1/%2").arg(Entry.mSection, Entry.mKey);

	if (gProfileBatchSettings)
	{
		gProfileBatchSettings->remove(Name);
		return;
	}

	QSettings Settings;

	Settings.remove(Name);
}

// use to get the default value of a key
int swGetProfileDefaultInteger(SW_PROFILE_KEY Key)
{
	const int* DefaultInt = std::get_if<int>(&gProfileEntries[Key].mDefault);

	return DefaultInt ? *DefaultInt : 0;
}

float swGetProfileDefaultFloat(SW_PROFILE_KEY Key)
{
	const float* DefaultFloat = std::get_if<float>(&gProfileEntries[Key].mDefault);

	return DefaultFloat ? *DefaultFloat : 0.0f;
}

uint swGetProfileDefaultUnsignedInteger(SW_PROFILE_KEY Key)
{
	const uint* DefaultUInt = std::get_if<uint>(&gProfileEntries[Key].mDefault);

	return DefaultUInt ? *DefaultUInt : 0;
}

int swGetProfileInteger(SW_PROFILE_KEY Key)
{
	swProfileEntry& Entry = gProfileEntries[Key];
	int* DefaultInt = std::get_if<int>(&Entry.mDefault);
	int DefaultValue = DefaultInt ? *DefaultInt : 0;

	const QVariant Stored = swGetProfileValue(Entry, DefaultValue);

	if (Stored.userType() == QMetaType::Float || Stored.userType() == QMetaType::Double)
		return DefaultValue;
	if (Stored.userType() == QMetaType::ULongLong && Stored.toULongLong() > static_cast<qulonglong>(std::numeric_limits<int>::max()))
		return DefaultValue;

	bool Valid = false;
	const qlonglong Value = Stored.toLongLong(&Valid);

	if (Valid && Value >= std::numeric_limits<int>::min() && Value <= std::numeric_limits<int>::max())
		return static_cast<int>(Value);

	return DefaultValue;
}

uint swGetProfileUnsignedInteger(SW_PROFILE_KEY Key)
{
	swProfileEntry& Entry = gProfileEntries[Key];
	uint* DefaultUInt = std::get_if<uint>(&Entry.mDefault);
	uint DefaultValue = DefaultUInt ? *DefaultUInt : 0;

	const QVariant Stored = swGetProfileValue(Entry, DefaultValue);

	if (Stored.userType() == QMetaType::Float || Stored.userType() == QMetaType::Double)
		return DefaultValue;

	if (Stored.userType() == QMetaType::Int || Stored.userType() == QMetaType::LongLong)
	{
		const qlonglong Value = Stored.toLongLong();

		if (Value >= std::numeric_limits<qint32>::min() && Value <= static_cast<qlonglong>(std::numeric_limits<uint>::max()))
			return static_cast<uint>(Value);
		return DefaultValue;
	}

	bool Valid = false;
	const qulonglong Value = Stored.toULongLong(&Valid);

	if (Valid && Value <= std::numeric_limits<uint>::max())
		return static_cast<uint>(Value);

	return DefaultValue;
}

float swGetProfileFloat(SW_PROFILE_KEY Key)
{
	swProfileEntry& Entry = gProfileEntries[Key];
	float* DefaultFloat = std::get_if<float>(&Entry.mDefault);
	float DefaultValue = DefaultFloat ? *DefaultFloat : 0.0f;

	bool Valid = false;
	const float Value = swGetProfileValue(Entry, DefaultValue).toFloat(&Valid);

	return Valid && std::isfinite(Value) ? Value : DefaultValue;
}

QString swGetProfileString(SW_PROFILE_KEY Key)
{
	swProfileEntry& Entry = gProfileEntries[Key];
	QString* DefaultString = std::get_if<QString>(&Entry.mDefault);
	QString DefaultValue = DefaultString ? *DefaultString : QString();

	return swGetProfileValue(Entry, DefaultValue).toString();
}

QStringList swGetProfileStringList(SW_PROFILE_KEY Key)
{
	swProfileEntry& Entry = gProfileEntries[Key];
	QStringList* DefaultStringList = std::get_if<QStringList>(&Entry.mDefault);
	QStringList DefaultValue = DefaultStringList ? *DefaultStringList : QStringList();

	return swGetProfileValue(Entry, DefaultValue).toStringList();
}

QByteArray swGetProfileBuffer(SW_PROFILE_KEY Key)
{
	swProfileEntry& Entry = gProfileEntries[Key];
	return swGetProfileValue(Entry).toByteArray();
}

void swSetProfileInteger(SW_PROFILE_KEY Key, int Value)
{
	swSetProfileValue(gProfileEntries[Key], Value);
}

void swSetProfileUnsignedInteger(SW_PROFILE_KEY Key, uint Value)
{
	swSetProfileValue(gProfileEntries[Key], Value);
}

void swSetProfileFloat(SW_PROFILE_KEY Key, float Value)
{
	if (!std::isfinite(Value))
		return;

	swSetProfileValue(gProfileEntries[Key], Value);
}

void swSetProfileString(SW_PROFILE_KEY Key, const QString& Value)
{
	swSetProfileValue(gProfileEntries[Key], Value);
}

void swSetProfileStringList(SW_PROFILE_KEY Key, const QStringList& Value)
{
	swSetProfileValue(gProfileEntries[Key], Value);
}

void swSetProfileBuffer(SW_PROFILE_KEY Key, const QByteArray& Buffer)
{
	swSetProfileValue(gProfileEntries[Key], Buffer);
}

template<typename Enum>
struct swProfileEnumerationValue
{
	Enum Value;
	const char* Token;
	int LegacyValue;
};

// keep tokens stable across enum changes
constexpr swProfileEnumerationValue<swShadingMode> ShadingModeValues[] =
{
	{ swShadingMode::Wireframe, "Wireframe", 0 },
	{ swShadingMode::Flat, "Flat", 1 },
	{ swShadingMode::DefaultLights, "DefaultLights", 2 },
	{ swShadingMode::Full, "Full", 3 }
};

constexpr swProfileEnumerationValue<swLightingStyle> LightingStyleValues[] =
{
	{ swLightingStyle::Standard, "Standard", 0 },
	{ swLightingStyle::Studio, "Studio", 1 },
	{ swLightingStyle::Matte, "Matte", 2 },
	{ swLightingStyle::Toon, "Toon", 3 }
};

constexpr swProfileEnumerationValue<swHighlightRenderMode> HighlightRenderModeValues[] =
{
	{ swHighlightRenderMode::Outline, "Outline", 0 },
	{ swHighlightRenderMode::Tint, "Tint", 1 }
};

constexpr swProfileEnumerationValue<swAxisIconLocation> AxisIconLocationValues[] =
{
	{ swAxisIconLocation::TopLeft, "TopLeft", 0 },
	{ swAxisIconLocation::TopRight, "TopRight", 1 },
	{ swAxisIconLocation::BottomLeft, "BottomLeft", 2 },
	{ swAxisIconLocation::BottomRight, "BottomRight", 3 }
};

constexpr swProfileEnumerationValue<swViewCubeLocation> ViewCubeLocationValues[] =
{
	{ swViewCubeLocation::TopLeft, "TopLeft", 0 },
	{ swViewCubeLocation::TopRight, "TopRight", 1 },
	{ swViewCubeLocation::BottomLeft, "BottomLeft", 2 },
	{ swViewCubeLocation::BottomRight, "BottomRight", 3 }
};

constexpr swProfileEnumerationValue<swColorTheme> ColorThemeValues[] =
{
	{ swColorTheme::Dark, "Dark", 0 },
	{ swColorTheme::System, "System", 1 }
};

constexpr swProfileEnumerationValue<swStudStyle> StudStyleValues[] =
{
	{ swStudStyle::Plain, "Plain", 0 },
	{ swStudStyle::ThinLinesLogo, "ThinLinesLogo", 1 },
	{ swStudStyle::OutlineLogo, "OutlineLogo", 2 },
	{ swStudStyle::SharpTopLogo, "SharpTopLogo", 3 },
	{ swStudStyle::RoundedTopLogo, "RoundedTopLogo", 4 },
	{ swStudStyle::FlattenedLogo, "FlattenedLogo", 5 },
	{ swStudStyle::HighContrast, "HighContrast", 6 },
	{ swStudStyle::HighContrastLogo, "HighContrastLogo", 7 }
};

constexpr swProfileEnumerationValue<swPartFilterType> PartFilterTypeValues[] =
{
	{ swPartFilterType::Default, "Default", 0 },
	{ swPartFilterType::RegularExpression, "RegularExpression", 1 }
};

constexpr swProfileEnumerationValue<swPOVRayRenderQuality> RenderQualityValues[] =
{
	{ swPOVRayRenderQuality::Low, "Low", 0 },
	{ swPOVRayRenderQuality::Medium, "Medium", 1 },
	{ swPOVRayRenderQuality::High, "High", 2 }
};

template<typename Enum, size_t N>
static Enum swReadProfileEnumeration(SW_PROFILE_KEY Key, const swProfileEnumerationValue<Enum> (&Values)[N])
{
	const swProfileEntry& Entry = gProfileEntries[Key];
	const Enum Default = static_cast<Enum>(std::get<int>(Entry.mDefault));
	const QString Token = swGetProfileValue(Entry).toString();

	for (const swProfileEnumerationValue<Enum>& Value : Values)
	{
		if (Token == QLatin1String(Value.Token))
			return Value.Value;
	}

	bool Valid = false;
	const int LegacyValue = Token.toInt(&Valid);

	if (Valid)
	{
		for (const swProfileEnumerationValue<Enum>& Value : Values)
		{
			if (LegacyValue == Value.LegacyValue)
				return Value.Value;
		}
	}

	return Default;
}

template<typename Enum, size_t N>
static void swWriteProfileEnumeration(SW_PROFILE_KEY Key, Enum Value, const swProfileEnumerationValue<Enum> (&Values)[N])
{
	for (const swProfileEnumerationValue<Enum>& Entry : Values)
	{
		if (Value == Entry.Value)
		{
			swSetProfileString(Key, QString::fromLatin1(Entry.Token));
			return;
		}
	}
}

swShadingMode swGetProfileShadingMode(SW_PROFILE_KEY Key)
{
	return swReadProfileEnumeration(Key, ShadingModeValues);
}

void swSetProfileShadingMode(swShadingMode Value, SW_PROFILE_KEY Key)
{
	swWriteProfileEnumeration(Key, Value, ShadingModeValues);
}

swLightingStyle swGetProfileLightingStyle()
{
	return swReadProfileEnumeration(SW_PROFILE_LIGHTING_STYLE, LightingStyleValues);
}

void swSetProfileLightingStyle(swLightingStyle Value)
{
	swWriteProfileEnumeration(SW_PROFILE_LIGHTING_STYLE, Value, LightingStyleValues);
}

swHighlightRenderMode swGetProfileHighlightRenderMode()
{
	return swReadProfileEnumeration(SW_PROFILE_HIGHLIGHT_RENDER_MODE, HighlightRenderModeValues);
}

void swSetProfileHighlightRenderMode(swHighlightRenderMode Value)
{
	swWriteProfileEnumeration(SW_PROFILE_HIGHLIGHT_RENDER_MODE, Value, HighlightRenderModeValues);
}

swAxisIconLocation swGetProfileAxisIconLocation()
{
	return swReadProfileEnumeration(SW_PROFILE_DRAW_AXES_LOCATION, AxisIconLocationValues);
}

void swSetProfileAxisIconLocation(swAxisIconLocation Value)
{
	swWriteProfileEnumeration(SW_PROFILE_DRAW_AXES_LOCATION, Value, AxisIconLocationValues);
}

swViewCubeLocation swGetProfileViewCubeLocation()
{
	return swReadProfileEnumeration(SW_PROFILE_VIEW_CUBE_LOCATION, ViewCubeLocationValues);
}

void swSetProfileViewCubeLocation(swViewCubeLocation Value)
{
	swWriteProfileEnumeration(SW_PROFILE_VIEW_CUBE_LOCATION, Value, ViewCubeLocationValues);
}

swColorTheme swGetProfileColorTheme()
{
	return swReadProfileEnumeration(SW_PROFILE_COLOR_THEME, ColorThemeValues);
}

void swSetProfileColorTheme(swColorTheme Value)
{
	swWriteProfileEnumeration(SW_PROFILE_COLOR_THEME, Value, ColorThemeValues);
}

swStudStyle swGetProfileStudStyle()
{
	return swReadProfileEnumeration(SW_PROFILE_STUD_STYLE, StudStyleValues);
}

void swSetProfileStudStyle(swStudStyle Value)
{
	swWriteProfileEnumeration(SW_PROFILE_STUD_STYLE, Value, StudStyleValues);
}

swPartFilterType swGetProfilePartFilterType()
{
	return swReadProfileEnumeration(SW_PROFILE_PARTS_LIST_PART_FILTER, PartFilterTypeValues);
}

void swSetProfilePartFilterType(swPartFilterType Value)
{
	swWriteProfileEnumeration(SW_PROFILE_PARTS_LIST_PART_FILTER, Value, PartFilterTypeValues);
}

swPOVRayRenderQuality swGetProfileRenderQuality()
{
	return swReadProfileEnumeration(SW_PROFILE_RENDER_QUALITY, RenderQualityValues);
}

void swSetProfileRenderQuality(swPOVRayRenderQuality Value)
{
	swWriteProfileEnumeration(SW_PROFILE_RENDER_QUALITY, Value, RenderQualityValues);
}

swViewCubeLocation swGetProfilePreviewViewCubeLocation()
{
	return swReadProfileEnumeration(SW_PROFILE_PREVIEW_VIEW_CUBE_LOCATION, ViewCubeLocationValues);
}

void swSetProfilePreviewViewCubeLocation(swViewCubeLocation Value)
{
	swWriteProfileEnumeration(SW_PROFILE_PREVIEW_VIEW_CUBE_LOCATION, Value, ViewCubeLocationValues);
}
