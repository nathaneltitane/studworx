#pragma once

#include <KFileMetaData/ExtractorPlugin>

// use to list ldraw model details in kde file managers and search
class swMetadataExtractor : public KFileMetaData::ExtractorPlugin
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "org.kde.kf5.kfilemetadata.ExtractorPlugin" FILE "sw_metadata.json")
	Q_INTERFACES(KFileMetaData::ExtractorPlugin)

public:
	explicit swMetadataExtractor(QObject* Parent = nullptr);

	QStringList mimetypes() const override;
	void extract(KFileMetaData::ExtractionResult* Result) override;
};
