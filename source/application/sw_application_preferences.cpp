#include "sw_global.h"
#include "sw_application.h"
#include "sw_dialog_preferences.h"

void swPreferences::LoadDefaults()
{
	mFixedAxes = swGetProfileInteger(SW_PROFILE_FIXED_AXES);
	mMouseSensitivity = swGetProfileInteger(SW_PROFILE_MOUSE_SENSITIVITY);
	mShadingMode = swGetProfileShadingMode();
	mLightingStyle = swGetProfileLightingStyle();
	mHighlightRenderMode = swGetProfileHighlightRenderMode();
	mSelectionOutlineWidth = qBound(SW_SELECTION_OUTLINE_WIDTH_MINIMUM, swGetProfileInteger(SW_PROFILE_SELECTION_OUTLINE_WIDTH), SW_SELECTION_OUTLINE_WIDTH_MAXIMUM);
	mBackgroundSolidColor = swGetProfileUnsignedInteger(SW_PROFILE_BACKGROUND_COLOR);
	mDrawAxes = swGetProfileInteger(SW_PROFILE_DRAW_AXES);
	mFollowStepViews = swGetProfileInteger(SW_PROFILE_FOLLOW_STEP_VIEWS);
	mProjectionScope = static_cast<swProjectionScope>(qBound(0, swGetProfileInteger(SW_PROFILE_PROJECTION_SCOPE), static_cast<int>(swProjectionScope::AllViews)));
	mCutawaySize = swGetProfileInteger(SW_PROFILE_CUTAWAY_SIZE);
	mTurntableIdleDuration = swGetProfileInteger(SW_PROFILE_TURNTABLE_IDLE_DURATION);
	mTurntableTurnTime = swGetProfileInteger(SW_PROFILE_TURNTABLE_TURN_TIME);
	mTurntableAxis = static_cast<swTurntableAxis>(qBound(0, swGetProfileInteger(SW_PROFILE_TURNTABLE_AXIS), static_cast<int>(swTurntableAxis::Z)));
	mAxisIconLocation = swGetProfileAxisIconLocation();
	mAxisIconSize = swSnapSize(swGetProfileInteger(SW_PROFILE_AXIS_ICON_SIZE), SW_AXIS_ICON_SIZES);
	mAxisIconLabels = swGetProfileInteger(SW_PROFILE_AXIS_ICON_LABELS);
	mAxisIconNegative = swGetProfileInteger(SW_PROFILE_AXIS_ICON_NEGATIVE);
	mAxisIconCubeOutline = swGetProfileInteger(SW_PROFILE_AXIS_ICON_CUBE_OUTLINE);
	mAxisIconCubeOutlineColor = swGetProfileUnsignedInteger(SW_PROFILE_AXIS_ICON_CUBE_OUTLINE_COLOR);
	mTextColor = swGetProfileUnsignedInteger(SW_PROFILE_TEXT_COLOR);
	mTextHaloColor = swGetProfileUnsignedInteger(SW_PROFILE_TEXT_HALO_COLOR);
	mMarqueeBorderColor = swGetProfileUnsignedInteger(SW_PROFILE_MARQUEE_BORDER_COLOR);
	mMarqueeFillColor = swGetProfileUnsignedInteger(SW_PROFILE_MARQUEE_FILL_COLOR);
	mOverlayColor = swGetProfileUnsignedInteger(SW_PROFILE_OVERLAY_COLOR);
	mCutawayShadeColor = swGetProfileUnsignedInteger(SW_PROFILE_CUTAWAY_SHADE_COLOR);
	mCutawayEdges = swGetProfileInteger(SW_PROFILE_CUTAWAY_EDGES);
	mActiveViewColor = swGetProfileUnsignedInteger(SW_PROFILE_ACTIVE_VIEW_COLOR);
	mInactiveViewColor = swGetProfileUnsignedInteger(SW_PROFILE_INACTIVE_VIEW_COLOR);
	mDrawEdgeLines = swGetProfileInteger(SW_PROFILE_DRAW_EDGE_LINES);
	mDrawConditionalLines = swGetProfileInteger(SW_PROFILE_DRAW_CONDITIONAL_LINES);
	mLineWidth = swGetProfileFloat(SW_PROFILE_LINE_WIDTH);
	LoadRenderTarget(swRenderTarget::Preview);
	LoadRenderTarget(swRenderTarget::Parts);
	LoadRenderTarget(swRenderTarget::MinifigureBuilder);
	mAllowLOD = swGetProfileInteger(SW_PROFILE_ALLOW_LOD);
	mMeshLODDistance = swGetProfileFloat(SW_PROFILE_LOD_DISTANCE);
	mFadeSteps = swGetProfileInteger(SW_PROFILE_FADE_STEPS);
	mFadeStepsColor = swClampFadeColorAlpha(swGetProfileUnsignedInteger(SW_PROFILE_FADE_STEPS_COLOR));
	mFadeInline = swGetProfileInteger(SW_PROFILE_FADE_INLINE);
	mFadeInlineColor = swGetProfileUnsignedInteger(SW_PROFILE_FADE_INLINE_COLOR);
	mFadeJoint = swGetProfileInteger(SW_PROFILE_FADE_JOINT);
	mFadeJointColor = swGetProfileUnsignedInteger(SW_PROFILE_FADE_JOINT_COLOR);
	mFadeJointMoving = swGetProfileInteger(SW_PROFILE_FADE_JOINT_MOVING);
	mHoverOutline = swGetProfileInteger(SW_PROFILE_HOVER_OUTLINE);
	mHoverOutlineColor = swGetProfileUnsignedInteger(SW_PROFILE_HOVER_OUTLINE_COLOR);
	mConnectionPoints = swGetProfileInteger(SW_PROFILE_CONNECTION_POINTS);
	mConnectionSnap = swGetProfileInteger(SW_PROFILE_CONNECTION_SNAP);
	mConnectionSnapJoint = swGetProfileInteger(SW_PROFILE_CONNECTION_SNAP_JOINT);
	mHighlightNewParts = swGetProfileInteger(SW_PROFILE_HIGHLIGHT_NEW_PARTS);
	mHighlightNewPartsColor = swClampStepColorAlpha(swGetProfileUnsignedInteger(SW_PROFILE_HIGHLIGHT_NEW_PARTS_COLOR));
	mDrawGridStuds = swGetProfileInteger(SW_PROFILE_GRID_STUDS);
	mGridStudColor = swGetProfileUnsignedInteger(SW_PROFILE_GRID_STUD_COLOR);
	mDrawGridLines = swGetProfileInteger(SW_PROFILE_GRID_LINES);
	mGridLineSpacing = swGetProfileInteger(SW_PROFILE_GRID_LINE_SPACING);
	mGridLineColor = swGetProfileUnsignedInteger(SW_PROFILE_GRID_LINE_COLOR);
	mDrawGridNegativeAxes = swGetProfileInteger(SW_PROFILE_GRID_NEGATIVE_AXES);
	mGridEnabled = swGetProfileInteger(SW_PROFILE_GRID_ENABLED);
	mDrawGridHeightXZ = swGetProfileInteger(SW_PROFILE_GRID_HEIGHT_XZ);
	mDrawGridHeightYZ = swGetProfileInteger(SW_PROFILE_GRID_HEIGHT_YZ);
	mGridHeightBehindModel = swGetProfileInteger(SW_PROFILE_GRID_HEIGHT_BEHIND_MODEL);
	mDrawGridAxisX = swGetProfileInteger(SW_PROFILE_GRID_AXIS_X);
	mDrawGridAxisY = swGetProfileInteger(SW_PROFILE_GRID_AXIS_Y);
	mDrawGridAxisZ = swGetProfileInteger(SW_PROFILE_GRID_AXIS_Z);
	mGridMargin = swGetProfileInteger(SW_PROFILE_GRID_MARGIN);
	mGridHeightMargin = swGetProfileInteger(SW_PROFILE_GRID_HEIGHT_MARGIN);
	mViewCubeEnabled = swGetProfileInteger(SW_PROFILE_VIEW_CUBE_ENABLED);
	mViewCubeLocation = swGetProfileViewCubeLocation();
	mViewCubeSize = swSnapSize(swGetProfileInteger(SW_PROFILE_VIEW_CUBE_SIZE), SW_VIEW_CUBE_SIZES);
	mViewCubeColor = swGetProfileUnsignedInteger(SW_PROFILE_VIEW_CUBE_COLOR);
	mViewCubeTextColor = swGetProfileUnsignedInteger(SW_PROFILE_VIEW_CUBE_TEXT_COLOR);
	mViewCubeHighlightColor = swGetProfileUnsignedInteger(SW_PROFILE_VIEW_CUBE_HIGHLIGHT_COLOR);
	mViewCubeLabelAbbreviations = swGetProfileInteger(SW_PROFILE_VIEW_CUBE_LABEL_ABBREVIATIONS);
	mAxisXColor = swGetProfileUnsignedInteger(SW_PROFILE_AXIS_X_COLOR);
	mAxisYColor = swGetProfileUnsignedInteger(SW_PROFILE_AXIS_Y_COLOR);
	mAxisZColor = swGetProfileUnsignedInteger(SW_PROFILE_AXIS_Z_COLOR);
	mAxisXNegativeColor = swGetProfileUnsignedInteger(SW_PROFILE_AXIS_X_NEGATIVE_COLOR);
	mAxisYNegativeColor = swGetProfileUnsignedInteger(SW_PROFILE_AXIS_Y_NEGATIVE_COLOR);
	mAxisZNegativeColor = swGetProfileUnsignedInteger(SW_PROFILE_AXIS_Z_NEGATIVE_COLOR);
	mAutoLoadMostRecent = swGetProfileInteger(SW_PROFILE_AUTO_LOAD_MOST_RECENT);
	mRestoreTabLayout = swGetProfileInteger(SW_PROFILE_RESTORE_TAB_LAYOUT);
	mFirstPartAtOrigin = swGetProfileInteger(SW_PROFILE_FIRST_PART_AT_ORIGIN);
	mJointRotation = swGetProfileInteger(SW_PROFILE_JOINT_ROTATION);
	mJointModeStart = std::clamp(swGetProfileInteger(SW_PROFILE_JOINT_MODE_START), 0, 2);
	mJointSelectInView = swGetProfileInteger(SW_PROFILE_JOINT_SELECT_IN_VIEW);
	mContextMenu = swGetProfileStringList(SW_PROFILE_CONTEXT_MENU);
	mShadowLibraryEnabled = swGetProfileInteger(SW_PROFILE_SHADOW_LIBRARY_ENABLED);
	mPartsSnapToTiles = swGetProfileInteger(SW_PROFILE_PARTS_SNAP_TO_TILES);
	mTerminalFollowModel = swGetProfileInteger(SW_PROFILE_TERMINAL_FOLLOW_MODEL);
	mInsertColorMode = static_cast<swInsertColorMode>(qBound(0, swGetProfileInteger(SW_PROFILE_INSERT_COLOR_MODE), static_cast<int>(swInsertColorMode::LastInsertedColor)));
	mFlexSpreadMode = static_cast<swFlexSpreadMode>(qBound(0, swGetProfileInteger(SW_PROFILE_FLEX_SPREAD_MODE), static_cast<int>(swFlexSpreadMode::DistributedWeight)));
	mFlexClampNewControlPoints = swGetProfileInteger(SW_PROFILE_FLEX_CLAMP_NEW_CONTROL_POINTS);
	mFlexMiddleControlPoint = swGetProfileInteger(SW_PROFILE_FLEX_MIDDLE_CONTROL_POINT);
	mWatchFileChanges = swGetProfileInteger(SW_PROFILE_WATCH_FILE_CHANGES);
	mLoadPartsBeforeShowing = swGetProfileInteger(SW_PROFILE_LOAD_PARTS_BEFORE_SHOWING);
	mWarnPartsLoadFailure = swGetProfileInteger(SW_PROFILE_WARN_PARTS_LOAD_FAILURE);
	mSaveAfterReplacingParts = swGetProfileInteger(SW_PROFILE_SAVE_AFTER_REPLACING_PARTS);
	mMigrateMetaCommands = static_cast<swMigrateMetaCommandsMode>(qBound(0, swGetProfileInteger(SW_PROFILE_MIGRATE_META_COMMANDS), static_cast<int>(swMigrateMetaCommandsMode::Keep)));
	mMarkColladaTransparency = swGetProfileInteger(SW_PROFILE_MARK_COLLADA_TRANSPARENCY);
	mKMZVisibleSurfaces = swGetProfileInteger(SW_PROFILE_KMZ_VISIBLE_SURFACES);
	mColorTheme = swGetProfileColorTheme();
	mPreviewViewCubeEnabled = swGetProfileInteger(SW_PROFILE_PREVIEW_VIEW_CUBE_ENABLED);
	mPreviewViewCubeSize = swSnapSize(swGetProfileInteger(SW_PROFILE_PREVIEW_VIEW_CUBE_SIZE), SW_VIEW_CUBE_SIZES);
	mPreviewViewCubeLocation = swGetProfilePreviewViewCubeLocation();
	mDrawPreviewAxis = swGetProfileInteger(SW_PROFILE_PREVIEW_DRAW_AXES);
	mPreviewAxisIconSize = swSnapSize(swGetProfileInteger(SW_PROFILE_PREVIEW_AXIS_ICON_SIZE), SW_AXIS_ICON_SIZES);
	mPreviewKeepSelection = swGetProfileInteger(SW_PROFILE_PREVIEW_KEEP_SELECTION);
	mStudCylinderColorEnabled = swGetProfileInteger(SW_PROFILE_STUD_CYLINDER_COLOR_ENABLED);
	mStudCylinderColor = swGetProfileUnsignedInteger(SW_PROFILE_STUD_CYLINDER_COLOR);
	mPartEdgeColorEnabled = swGetProfileInteger(SW_PROFILE_PART_EDGE_COLOR_ENABLED);
	mPartEdgeColor = swGetProfileUnsignedInteger(SW_PROFILE_PART_EDGE_COLOR);
	mBlackEdgeColorEnabled = swGetProfileInteger(SW_PROFILE_BLACK_EDGE_COLOR_ENABLED);
	mBlackEdgeColor = swGetProfileUnsignedInteger(SW_PROFILE_BLACK_EDGE_COLOR);
	mDarkEdgeColorEnabled = swGetProfileInteger(SW_PROFILE_DARK_EDGE_COLOR_ENABLED);
	mDarkEdgeColor = swGetProfileUnsignedInteger(SW_PROFILE_DARK_EDGE_COLOR);
	mPartEdgeContrast = swGetProfileFloat(SW_PROFILE_PART_EDGE_CONTRAST);
	mPartColorValueLDIndex = swGetProfileFloat(SW_PROFILE_PART_COLOR_VALUE_LD_INDEX);
	mAutomateEdgeColor = swGetProfileInteger(SW_PROFILE_AUTOMATE_EDGE_COLOR);
	mObjectSelectedColor = swGetProfileUnsignedInteger(SW_PROFILE_OBJECT_SELECTED_COLOR);
	mObjectFocusedColor = swGetProfileUnsignedInteger(SW_PROFILE_OBJECT_FOCUSED_COLOR);
	mObjectDeactivatedColor = swGreyscaleColor(swGetProfileUnsignedInteger(SW_PROFILE_OBJECT_DEACTIVATED_COLOR));
	mCameraColor = swGetProfileUnsignedInteger(SW_PROFILE_CAMERA_COLOR);
	mLightColor = swGetProfileUnsignedInteger(SW_PROFILE_LIGHT_COLOR);
	mControlPointColor = swGetProfileUnsignedInteger(SW_PROFILE_CONTROL_POINT_COLOR);
	mControlPointFocusedColor = swGetProfileUnsignedInteger(SW_PROFILE_CONTROL_POINT_FOCUSED_COLOR);
	mControlPointSeatedColor = swGetProfileUnsignedInteger(SW_PROFILE_CONTROL_POINT_SEATED_COLOR);
	mPathColor = swGetProfileUnsignedInteger(SW_PROFILE_PATH_COLOR);
	mPathOutOfReachColor = swGetProfileUnsignedInteger(SW_PROFILE_PATH_OUT_OF_REACH_COLOR);
	mSpreadHandleColor = swGetProfileUnsignedInteger(SW_PROFILE_SPREAD_HANDLE_COLOR);
	mExplodeLineColor = swGetProfileUnsignedInteger(SW_PROFILE_EXPLODE_LINE_COLOR);
	mConnectionPointColor = swGetProfileUnsignedInteger(SW_PROFILE_CONNECTION_POINT_COLOR);
}

void swPreferences::SaveDefaults()
{
	swSetProfileInteger(SW_PROFILE_FIXED_AXES, mFixedAxes);
	swSetProfileInteger(SW_PROFILE_MOUSE_SENSITIVITY, mMouseSensitivity);
	swSetProfileShadingMode(mShadingMode);
	swSetProfileLightingStyle(mLightingStyle);
	swSetProfileHighlightRenderMode(mHighlightRenderMode);
	swSetProfileInteger(SW_PROFILE_SELECTION_OUTLINE_WIDTH, mSelectionOutlineWidth);
	swSetProfileInteger(SW_PROFILE_DRAW_AXES, mDrawAxes);
	swSetProfileInteger(SW_PROFILE_FOLLOW_STEP_VIEWS, mFollowStepViews);
	swSetProfileInteger(SW_PROFILE_PROJECTION_SCOPE, static_cast<int>(mProjectionScope));
	swSetProfileInteger(SW_PROFILE_CUTAWAY_SIZE, mCutawaySize);
	swSetProfileInteger(SW_PROFILE_TURNTABLE_IDLE_DURATION, mTurntableIdleDuration);
	swSetProfileInteger(SW_PROFILE_TURNTABLE_TURN_TIME, mTurntableTurnTime);
	swSetProfileInteger(SW_PROFILE_TURNTABLE_AXIS, static_cast<int>(mTurntableAxis));
	swSetProfileAxisIconLocation(mAxisIconLocation);
	swSetProfileInteger(SW_PROFILE_AXIS_ICON_SIZE, mAxisIconSize);
	swSetProfileInteger(SW_PROFILE_AXIS_ICON_LABELS, mAxisIconLabels);
	swSetProfileInteger(SW_PROFILE_AXIS_ICON_NEGATIVE, mAxisIconNegative);
	swSetProfileInteger(SW_PROFILE_AXIS_ICON_CUBE_OUTLINE, mAxisIconCubeOutline);
	swSetProfileUnsignedInteger(SW_PROFILE_AXIS_ICON_CUBE_OUTLINE_COLOR, mAxisIconCubeOutlineColor);
	swSetProfileUnsignedInteger(SW_PROFILE_TEXT_COLOR, mTextColor);
	swSetProfileUnsignedInteger(SW_PROFILE_TEXT_HALO_COLOR, mTextHaloColor);
	swSetProfileUnsignedInteger(SW_PROFILE_BACKGROUND_COLOR, mBackgroundSolidColor);
	swSetProfileUnsignedInteger(SW_PROFILE_MARQUEE_BORDER_COLOR, mMarqueeBorderColor);
	swSetProfileUnsignedInteger(SW_PROFILE_MARQUEE_FILL_COLOR, mMarqueeFillColor);
	swSetProfileUnsignedInteger(SW_PROFILE_OVERLAY_COLOR, mOverlayColor);
	swSetProfileUnsignedInteger(SW_PROFILE_CUTAWAY_SHADE_COLOR, mCutawayShadeColor);
	swSetProfileInteger(SW_PROFILE_CUTAWAY_EDGES, mCutawayEdges);
	swSetProfileUnsignedInteger(SW_PROFILE_ACTIVE_VIEW_COLOR, mActiveViewColor);
	swSetProfileUnsignedInteger(SW_PROFILE_INACTIVE_VIEW_COLOR, mInactiveViewColor);
	swSetProfileInteger(SW_PROFILE_DRAW_EDGE_LINES, mDrawEdgeLines);
	swSetProfileInteger(SW_PROFILE_DRAW_CONDITIONAL_LINES, mDrawConditionalLines);
	swSetProfileFloat(SW_PROFILE_LINE_WIDTH, mLineWidth);
	SaveRenderTarget(swRenderTarget::Preview);
	SaveRenderTarget(swRenderTarget::Parts);
	SaveRenderTarget(swRenderTarget::MinifigureBuilder);
	swSetProfileInteger(SW_PROFILE_ALLOW_LOD, mAllowLOD);
	swSetProfileFloat(SW_PROFILE_LOD_DISTANCE, mMeshLODDistance);
	swSetProfileInteger(SW_PROFILE_FADE_STEPS, mFadeSteps);
	swSetProfileUnsignedInteger(SW_PROFILE_FADE_STEPS_COLOR, mFadeStepsColor);
	swSetProfileInteger(SW_PROFILE_FADE_INLINE, mFadeInline);
	swSetProfileUnsignedInteger(SW_PROFILE_FADE_INLINE_COLOR, mFadeInlineColor);
	swSetProfileInteger(SW_PROFILE_FADE_JOINT, mFadeJoint);
	swSetProfileUnsignedInteger(SW_PROFILE_FADE_JOINT_COLOR, mFadeJointColor);
	swSetProfileInteger(SW_PROFILE_FADE_JOINT_MOVING, mFadeJointMoving);
	swSetProfileInteger(SW_PROFILE_HOVER_OUTLINE, mHoverOutline);
	swSetProfileUnsignedInteger(SW_PROFILE_HOVER_OUTLINE_COLOR, mHoverOutlineColor);
	swSetProfileInteger(SW_PROFILE_CONNECTION_POINTS, mConnectionPoints);
	swSetProfileInteger(SW_PROFILE_CONNECTION_SNAP, mConnectionSnap);
	swSetProfileInteger(SW_PROFILE_CONNECTION_SNAP_JOINT, mConnectionSnapJoint);
	swSetProfileInteger(SW_PROFILE_HIGHLIGHT_NEW_PARTS, mHighlightNewParts);
	swSetProfileUnsignedInteger(SW_PROFILE_HIGHLIGHT_NEW_PARTS_COLOR, mHighlightNewPartsColor);
	swSetProfileInteger(SW_PROFILE_GRID_STUDS, mDrawGridStuds);
	swSetProfileUnsignedInteger(SW_PROFILE_GRID_STUD_COLOR, mGridStudColor);
	swSetProfileInteger(SW_PROFILE_GRID_LINES, mDrawGridLines);
	swSetProfileInteger(SW_PROFILE_GRID_LINE_SPACING, mGridLineSpacing);
	swSetProfileUnsignedInteger(SW_PROFILE_GRID_LINE_COLOR, mGridLineColor);
	swSetProfileInteger(SW_PROFILE_GRID_NEGATIVE_AXES, mDrawGridNegativeAxes);
	swSetProfileInteger(SW_PROFILE_GRID_ENABLED, mGridEnabled);
	swSetProfileInteger(SW_PROFILE_GRID_HEIGHT_XZ, mDrawGridHeightXZ);
	swSetProfileInteger(SW_PROFILE_GRID_HEIGHT_YZ, mDrawGridHeightYZ);
	swSetProfileInteger(SW_PROFILE_GRID_HEIGHT_BEHIND_MODEL, mGridHeightBehindModel);
	swSetProfileInteger(SW_PROFILE_GRID_AXIS_X, mDrawGridAxisX);
	swSetProfileInteger(SW_PROFILE_GRID_AXIS_Y, mDrawGridAxisY);
	swSetProfileInteger(SW_PROFILE_GRID_AXIS_Z, mDrawGridAxisZ);
	swSetProfileInteger(SW_PROFILE_GRID_MARGIN, mGridMargin);
	swSetProfileInteger(SW_PROFILE_GRID_HEIGHT_MARGIN, mGridHeightMargin);
	swSetProfileInteger(SW_PROFILE_VIEW_CUBE_ENABLED, mViewCubeEnabled);
	swSetProfileViewCubeLocation(mViewCubeLocation);
	swSetProfileInteger(SW_PROFILE_VIEW_CUBE_SIZE, mViewCubeSize);
	swSetProfileUnsignedInteger(SW_PROFILE_VIEW_CUBE_COLOR, mViewCubeColor);
	swSetProfileUnsignedInteger(SW_PROFILE_VIEW_CUBE_TEXT_COLOR, mViewCubeTextColor);
	swSetProfileUnsignedInteger(SW_PROFILE_VIEW_CUBE_HIGHLIGHT_COLOR, mViewCubeHighlightColor);
	swSetProfileInteger(SW_PROFILE_VIEW_CUBE_LABEL_ABBREVIATIONS, mViewCubeLabelAbbreviations);
	swSetProfileUnsignedInteger(SW_PROFILE_AXIS_X_COLOR, mAxisXColor);
	swSetProfileUnsignedInteger(SW_PROFILE_AXIS_Y_COLOR, mAxisYColor);
	swSetProfileUnsignedInteger(SW_PROFILE_AXIS_Z_COLOR, mAxisZColor);
	swSetProfileUnsignedInteger(SW_PROFILE_AXIS_X_NEGATIVE_COLOR, mAxisXNegativeColor);
	swSetProfileUnsignedInteger(SW_PROFILE_AXIS_Y_NEGATIVE_COLOR, mAxisYNegativeColor);
	swSetProfileUnsignedInteger(SW_PROFILE_AXIS_Z_NEGATIVE_COLOR, mAxisZNegativeColor);
	swSetProfileInteger(SW_PROFILE_AUTO_LOAD_MOST_RECENT, mAutoLoadMostRecent);
	swSetProfileInteger(SW_PROFILE_RESTORE_TAB_LAYOUT, mRestoreTabLayout);
	swSetProfileInteger(SW_PROFILE_FIRST_PART_AT_ORIGIN, mFirstPartAtOrigin);
	swSetProfileInteger(SW_PROFILE_JOINT_ROTATION, mJointRotation);
	swSetProfileInteger(SW_PROFILE_JOINT_MODE_START, mJointModeStart);
	swSetProfileInteger(SW_PROFILE_JOINT_SELECT_IN_VIEW, mJointSelectInView);
	swSetProfileStringList(SW_PROFILE_CONTEXT_MENU, mContextMenu);
	swSetProfileInteger(SW_PROFILE_SHADOW_LIBRARY_ENABLED, mShadowLibraryEnabled);
	swSetProfileInteger(SW_PROFILE_PARTS_SNAP_TO_TILES, mPartsSnapToTiles);
	swSetProfileInteger(SW_PROFILE_TERMINAL_FOLLOW_MODEL, mTerminalFollowModel);
	swSetProfileInteger(SW_PROFILE_INSERT_COLOR_MODE, static_cast<int>(mInsertColorMode));
	swSetProfileInteger(SW_PROFILE_FLEX_SPREAD_MODE, static_cast<int>(mFlexSpreadMode));
	swSetProfileInteger(SW_PROFILE_FLEX_CLAMP_NEW_CONTROL_POINTS, mFlexClampNewControlPoints);
	swSetProfileInteger(SW_PROFILE_FLEX_MIDDLE_CONTROL_POINT, mFlexMiddleControlPoint);
	swSetProfileInteger(SW_PROFILE_WATCH_FILE_CHANGES, mWatchFileChanges);
	swSetProfileInteger(SW_PROFILE_LOAD_PARTS_BEFORE_SHOWING, mLoadPartsBeforeShowing);
	swSetProfileInteger(SW_PROFILE_WARN_PARTS_LOAD_FAILURE, mWarnPartsLoadFailure);
	swSetProfileInteger(SW_PROFILE_SAVE_AFTER_REPLACING_PARTS, mSaveAfterReplacingParts);
	swSetProfileInteger(SW_PROFILE_MIGRATE_META_COMMANDS, static_cast<int>(mMigrateMetaCommands));
	swSetProfileInteger(SW_PROFILE_MARK_COLLADA_TRANSPARENCY, mMarkColladaTransparency);
	swSetProfileInteger(SW_PROFILE_KMZ_VISIBLE_SURFACES, mKMZVisibleSurfaces);
	swSetProfileColorTheme(mColorTheme);
	swSetProfileInteger(SW_PROFILE_PREVIEW_VIEW_CUBE_ENABLED, mPreviewViewCubeEnabled);
	swSetProfileInteger(SW_PROFILE_PREVIEW_VIEW_CUBE_SIZE, mPreviewViewCubeSize);
	swSetProfilePreviewViewCubeLocation(mPreviewViewCubeLocation);
	swSetProfileInteger(SW_PROFILE_PREVIEW_DRAW_AXES, mDrawPreviewAxis);
	swSetProfileInteger(SW_PROFILE_PREVIEW_AXIS_ICON_SIZE, mPreviewAxisIconSize);
	swSetProfileInteger(SW_PROFILE_PREVIEW_KEEP_SELECTION, mPreviewKeepSelection);
	swSetProfileInteger(SW_PROFILE_STUD_CYLINDER_COLOR_ENABLED, mStudCylinderColorEnabled);
	swSetProfileUnsignedInteger(SW_PROFILE_STUD_CYLINDER_COLOR, mStudCylinderColor);
	swSetProfileInteger(SW_PROFILE_PART_EDGE_COLOR_ENABLED, mPartEdgeColorEnabled);
	swSetProfileUnsignedInteger(SW_PROFILE_PART_EDGE_COLOR, mPartEdgeColor);
	swSetProfileInteger(SW_PROFILE_BLACK_EDGE_COLOR_ENABLED, mBlackEdgeColorEnabled);
	swSetProfileUnsignedInteger(SW_PROFILE_BLACK_EDGE_COLOR, mBlackEdgeColor);
	swSetProfileInteger(SW_PROFILE_DARK_EDGE_COLOR_ENABLED, mDarkEdgeColorEnabled);
	swSetProfileUnsignedInteger(SW_PROFILE_DARK_EDGE_COLOR, mDarkEdgeColor);
	swSetProfileFloat(SW_PROFILE_PART_EDGE_CONTRAST, mPartEdgeContrast);
	swSetProfileFloat(SW_PROFILE_PART_COLOR_VALUE_LD_INDEX, mPartColorValueLDIndex);
	swSetProfileInteger(SW_PROFILE_AUTOMATE_EDGE_COLOR, mAutomateEdgeColor);
	swSetProfileUnsignedInteger(SW_PROFILE_OBJECT_SELECTED_COLOR, mObjectSelectedColor);
	swSetProfileUnsignedInteger(SW_PROFILE_OBJECT_FOCUSED_COLOR, mObjectFocusedColor);
	swSetProfileUnsignedInteger(SW_PROFILE_OBJECT_DEACTIVATED_COLOR, mObjectDeactivatedColor);
	swSetProfileUnsignedInteger(SW_PROFILE_CAMERA_COLOR, mCameraColor);
	swSetProfileUnsignedInteger(SW_PROFILE_LIGHT_COLOR, mLightColor);
	swSetProfileUnsignedInteger(SW_PROFILE_CONTROL_POINT_COLOR, mControlPointColor);
	swSetProfileUnsignedInteger(SW_PROFILE_CONTROL_POINT_FOCUSED_COLOR, mControlPointFocusedColor);
	swSetProfileUnsignedInteger(SW_PROFILE_CONTROL_POINT_SEATED_COLOR, mControlPointSeatedColor);
	swSetProfileUnsignedInteger(SW_PROFILE_PATH_COLOR, mPathColor);
	swSetProfileUnsignedInteger(SW_PROFILE_PATH_OUT_OF_REACH_COLOR, mPathOutOfReachColor);
	swSetProfileUnsignedInteger(SW_PROFILE_SPREAD_HANDLE_COLOR, mSpreadHandleColor);
	swSetProfileUnsignedInteger(SW_PROFILE_EXPLODE_LINE_COLOR, mExplodeLineColor);
	swSetProfileUnsignedInteger(SW_PROFILE_CONNECTION_POINT_COLOR, mConnectionPointColor);
}

struct swRenderTargetProfileKeys
{
	SW_PROFILE_KEY SyncRendering;
	SW_PROFILE_KEY ShadingMode;
	SW_PROFILE_KEY DrawEdgeLines;
	SW_PROFILE_KEY DrawConditionalLines;
	SW_PROFILE_KEY LineWidth;
};

static swRenderTargetProfileKeys swGetRenderTargetProfileKeys(swRenderTarget Target)
{
	if (Target == swRenderTarget::MinifigureBuilder)
		return { SW_PROFILE_MINIFIGURE_BUILDER_SYNCHRONIZED_RENDERING, SW_PROFILE_MINIFIGURE_BUILDER_SHADING_MODE, SW_PROFILE_MINIFIGURE_BUILDER_DRAW_EDGE_LINES, SW_PROFILE_MINIFIGURE_BUILDER_DRAW_CONDITIONAL_LINES, SW_PROFILE_MINIFIGURE_BUILDER_LINE_WIDTH };

	if (Target == swRenderTarget::Parts)
		return { SW_PROFILE_PARTS_SYNCHRONIZED_RENDERING, SW_PROFILE_PARTS_SHADING_MODE, SW_PROFILE_PARTS_DRAW_EDGE_LINES, SW_PROFILE_PARTS_DRAW_CONDITIONAL_LINES, SW_PROFILE_PARTS_LINE_WIDTH };

	return { SW_PROFILE_PREVIEW_SYNCHRONIZED_RENDERING, SW_PROFILE_PREVIEW_SHADING_MODE, SW_PROFILE_PREVIEW_DRAW_EDGE_LINES, SW_PROFILE_PREVIEW_DRAW_CONDITIONAL_LINES, SW_PROFILE_PREVIEW_LINE_WIDTH };
}

void swPreferences::LoadRenderTarget(swRenderTarget Target)
{
	const swRenderTargetProfileKeys Keys = swGetRenderTargetProfileKeys(Target);
	swTargetRenderSettings& TargetSettings = GetTargetRenderSettings(Target);

	TargetSettings.SyncRendering = swGetProfileInteger(Keys.SyncRendering);
	TargetSettings.Settings.ShadingMode = swGetProfileShadingMode(Keys.ShadingMode);
	TargetSettings.Settings.DrawEdgeLines = swGetProfileInteger(Keys.DrawEdgeLines);
	TargetSettings.Settings.DrawConditionalLines = swGetProfileInteger(Keys.DrawConditionalLines);
	TargetSettings.Settings.LineWidth = swGetProfileFloat(Keys.LineWidth);
}

void swPreferences::SaveRenderTarget(swRenderTarget Target) const
{
	const swRenderTargetProfileKeys Keys = swGetRenderTargetProfileKeys(Target);
	const swTargetRenderSettings& TargetSettings = GetTargetRenderSettings(Target);

	swSetProfileInteger(Keys.SyncRendering, TargetSettings.SyncRendering);
	swSetProfileShadingMode(TargetSettings.Settings.ShadingMode, Keys.ShadingMode);
	swSetProfileInteger(Keys.DrawEdgeLines, TargetSettings.Settings.DrawEdgeLines);
	swSetProfileInteger(Keys.DrawConditionalLines, TargetSettings.Settings.DrawConditionalLines);
	swSetProfileFloat(Keys.LineWidth, TargetSettings.Settings.LineWidth);
}

void swPreferences::SetInterfaceColors(swColorTheme ColorTheme)
{
	const bool SystemDark = ColorTheme == swColorTheme::System && gApplication && gApplication->IsSystemPaletteDark();

	if (ColorTheme == swColorTheme::Dark || SystemDark)
	{
		// the dark colors are the profile defaults
		mTextColor = swGetProfileDefaultUnsignedInteger(SW_PROFILE_TEXT_COLOR);
		mTextHaloColor = swGetProfileDefaultUnsignedInteger(SW_PROFILE_TEXT_HALO_COLOR);
		mBackgroundSolidColor = swGetProfileDefaultUnsignedInteger(SW_PROFILE_BACKGROUND_COLOR);
		mOverlayColor = swGetProfileUnsignedInteger(SW_PROFILE_OVERLAY_COLOR);
		mActiveViewColor = swGetProfileDefaultUnsignedInteger(SW_PROFILE_ACTIVE_VIEW_COLOR);
		mGridStudColor = swGetProfileDefaultUnsignedInteger(SW_PROFILE_GRID_STUD_COLOR);
		mGridLineColor = swGetProfileDefaultUnsignedInteger(SW_PROFILE_GRID_LINE_COLOR);
		mViewCubeColor = swGetProfileDefaultUnsignedInteger(SW_PROFILE_VIEW_CUBE_COLOR);
		mViewCubeTextColor = swGetProfileDefaultUnsignedInteger(SW_PROFILE_VIEW_CUBE_TEXT_COLOR);
		mAxisIconCubeOutlineColor = swGetProfileDefaultUnsignedInteger(SW_PROFILE_AXIS_ICON_CUBE_OUTLINE_COLOR);
		mViewCubeHighlightColor = swGetProfileDefaultUnsignedInteger(SW_PROFILE_VIEW_CUBE_HIGHLIGHT_COLOR);
	}
	else
	{
		mTextColor = SW_RGBA(24, 33, 43, 255);
		mTextHaloColor = SW_RGBA(248, 250, 252, 204);
		mBackgroundSolidColor = SW_RGB(255, 255, 255);
		mOverlayColor = SW_RGBA(0, 0, 0, 255);
		mActiveViewColor = SW_RGBA(255, 0, 0, 255);
		mGridStudColor = SW_RGBA(64, 64, 64, 192);
		mGridLineColor = SW_RGBA(0, 0, 0, 255);
		mViewCubeColor = SW_RGBA(248, 250, 252, 255);
		mViewCubeTextColor = SW_RGBA(24, 33, 43, 255);
		mAxisIconCubeOutlineColor = SW_RGBA(24, 33, 43, 128);
		mViewCubeHighlightColor = SW_RGBA(41, 128, 185, 255);
	}
}
