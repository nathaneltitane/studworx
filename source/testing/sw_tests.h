#pragma once

#include <QtTest/QtTest>

class swTests : public QObject
{
	Q_OBJECT

private slots:
	void PropertyKeysAffectEquality();
	void LastKeyFrameTracksTimeEdits();
	void HistoryStoresOnlyChangedObjects();
	void HistoryCancelsPendingEdit();
	void HistoryReplaysAndMergesEdits();
	void HistoryDropsRevertedMerge();
	void CommandMetaReadsBothFormats();
	void CommandMetaWritesBothFormats();
	void CommandMetaRoundTripsModel();
	void CommandMetaKeepsCutaway();
	void ShadowParsesLines();
	void ShadowParsesIncludesAndClears();
	void ShadowReadsSubFileReferences();
	void ShadowScalesAndMirrorsSubFileShapes();
	void ShadowParsesJointKinds();
	void ShadowPlacesSubFileJoint();
	void PoseBlendsRotations();
	void PoseSavesAndLoads();
	void ConnectivityFitsShapesAlongTheirAxis();
	void ConnectivityConnectsSeatedShapes();
	void ConnectivityMarksNearbyOpenEnds();
	void POVRayRendersExport();
	void TranslucentImagesKeepColor_data();
	void TranslucentImagesKeepColor();
	void KMZKeepsVisibleSurfaces_data();
	void KMZKeepsVisibleSurfaces();
	void BrickLinkUsesBundledMapping();
	void ExportsResolveMovedParts();
	void BrickLinkUsesExceptions();
	void HTMLExportUsesModelFolder();
	void FlattenReplacesNestedSubModels();
	void AxisAlignedRotations_data();
	void AxisAlignedRotations();
	void AxisAlignedRotationTolerance();
	void GridExtentGrowsWithMargins();
	void ProfileBatchSavesOnce();
	void ProfileWritesEveryKey();
	void ContextMenuDefaultResolves();
	void OpenLXFMLImportsModel();
	void DocumentationNamesEveryCommand();
	void DocumentationHelpsEveryCommand();
	void DocumentationLinksEveryImage();
	void InterfaceUsesPaintedIcons();
};
