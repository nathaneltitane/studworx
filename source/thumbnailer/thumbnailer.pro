# kde file manager plugins, thumbnails with kio and model details with kfilemetadata
TEMPLATE = subdirs

SUBDIRS += thumbnail
thumbnail.file = thumbnail.pro

qtHaveModule(KFileMetaData) {
	SUBDIRS += metadata
	metadata.file = metadata.pro
}
