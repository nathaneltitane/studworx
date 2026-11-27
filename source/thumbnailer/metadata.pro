# kde metadata plugin listing ldraw model details in file managers and search
TEMPLATE = lib
TARGET = kfilemetadata_studworxextractor

# kde names metadata plugins without the lib prefix
CONFIG += plugin no_plugin_name_prefix c++17
CONFIG -= create_prl

QT += KFileMetaData

isEmpty(PLUGIN_DIR):PLUGIN_DIR = $$[QT_INSTALL_PLUGINS]/kf5/kfilemetadata

HEADERS += sw_metadata.h
SOURCES += sw_metadata.cpp
DISTFILES += sw_metadata.json

target.path = $$PLUGIN_DIR
INSTALLS += target
