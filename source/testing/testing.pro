TEMPLATE = app
TARGET = studworx_tests

CONFIG += console testcase c++17
CONFIG -= app_bundle

QT += concurrent opengl printsupport testlib widgets

greaterThan(QT_MAJOR_VERSION, 5) {
	QT += openglwidgets
}

INCLUDEPATH += ../application ../core ../library ../rendering ../transput ../interface/dialogs ../interface/widgets ../interface/window

HEADERS += \
	../core/sw_connectivity.h \
	../library/sw_parser_shadow.h \
	sw_tests.h

SOURCES += \
	../application/sw_profile.cpp \
	../core/sw_connectivity.cpp \
	../core/sw_object_property.cpp \
	../transput/sw_command_meta.cpp \
	../library/sw_parser_shadow.cpp \
	sw_command_meta_test.cpp \
	sw_connectivity_test.cpp \
	sw_math_test.cpp \
	sw_model_flatten_test.cpp \
	sw_object_history_test.cpp \
	sw_object_property_test.cpp \
	sw_profile_test.cpp \
	sw_project_export_test.cpp \
	sw_project_import_test.cpp \
	sw_renderer_povray_test.cpp \
	sw_view_image_test.cpp \
	main.cpp

win32 {
	INCLUDEPATH += $$[QT_INSTALL_HEADERS]/QtZlib
} else {
	LIBS += -lz
}
