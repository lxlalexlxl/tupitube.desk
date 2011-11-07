# File generated by kdevelop's qmake manager. 
# ------------------------------------------- 
# Subdir relative project main directory: ./src/components/paintarea
# Target is a library:  paintarea

HEADERS += ktpaintarea.h \
           ktconfigurationarea.h \
           ktviewdocument.h \
           ktdocumentruler.h \
           ktpaintareastatus.h \
           ktimagedevice.h \
           ktpaintareacommand.h \
           ktlibrarydialog.h \
           ktcolorwidget.h \
           ktbrushstatus.h \
           kttoolstatus.h \
           ktcanvas.h

SOURCES += ktpaintarea.cpp \
           ktconfigurationarea.cpp \
           ktviewdocument.cpp \
           ktdocumentruler.cpp \
           ktpaintareastatus.cpp \
           ktimagedevice.cpp \
           ktpaintareacommand.cpp \
           ktlibrarydialog.cpp \
           ktcolorwidget.cpp \
           ktbrushstatus.cpp \
           kttoolstatus.cpp \
           ktcanvas.cpp

CONFIG += static warn_on
TEMPLATE = lib
TARGET = paintarea

SELECTION_DIR = ../../plugins/tools/selecttool
INCLUDEPATH += $$SELECTION_DIR

POLYLINE_DIR = ../../plugins/tools/polylinetool
INCLUDEPATH += $$POLYLINE_DIR

FRAMEWORK_DIR = "../../framework"
include($$FRAMEWORK_DIR/framework.pri)
include(../components_config.pri)
