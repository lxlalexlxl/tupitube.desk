QT += widgets svgwidgets opengl core gui svg xml network core5compat
QT += multimedia multimediawidgets printsupport
TEMPLATE = lib
CONFIG += dll warn_on
TARGET = tupiworkspace

INSTALLS += target
target.path = /lib/

SHELL_DIR = ../../shell/
INCLUDEPATH += $$SHELL_DIR

PLUGIN_DIR = ../../plugins/export/genericexportplugin
INCLUDEPATH += $$PLUGIN_DIR

COMMON_DIR = ../../plugins/tools/common
INCLUDEPATH += $$COMMON_DIR

POLYLINE_DIR = ../../plugins/tools/polyline
INCLUDEPATH += $$POLYLINE_DIR

FRAMEWORK_DIR = "../../framework"
include($$FRAMEWORK_DIR/framework.pri)

LIBCOLOR_DIR = "../../libcolor"
include($$LIBCOLOR_DIR/libcolor.pri)

MYPAINT_DIR = "../../mypaint"
include($$MYPAINT_DIR/mypaint.pri)

PAPAGAYO_DIR = "../papagayo"
include($$PAPAGAYO_DIR/papagayo.pri)

unix {
    INCLUDEPATH += /usr/include/qt5/QtMultimedia /usr/include/qt5/QtMultimediaWidgets

    STORE_DIR = ../../store/
    INCLUDEPATH += $$STORE_DIR
    LIBS += -L$$STORE_DIR -ltupistore

    LIBBASE_DIR = ../../libbase/
    INCLUDEPATH += $$LIBBASE_DIR
    LIBS += -L$$LIBBASE_DIR -ltupibase

    LIBTUPI_DIR = ../../libtupi/
    INCLUDEPATH += $$LIBTUPI_DIR
    LIBS += -L$$LIBTUPI_DIR -ltupi

    !include(../../../tupiglobal.pri) {
             error("Run ./configure first!")
    }
}

win32 {
    include(../../../win.pri)

    STORE_DIR = ../../store/
    INCLUDEPATH += $$STORE_DIR
    LIBS += -L$$STORE_DIR/release/ -ltupistore

    LIBBASE_DIR = ../../libbase/
    INCLUDEPATH += $$LIBBASE_DIR
    LIBS += -L$$LIBBASE_DIR/release/ -ltupibase

    LIBTUPI_DIR = ../../libtupi/
    INCLUDEPATH += $$LIBTUPI_DIR
    LIBS += -L$$LIBTUPI_DIR/release/ -ltupi 
}

HEADERS += tuppaintarea.h \
           tupconfigurationarea.h \
           tupimagedevice.h \
           tuppaintareacommand.h \
           tuplibrarydialog.h \
           tupcanvas.h \
           tupcanvasview.h \
           tupimagedialog.h \
           tupstoryboarddialog.h \
           tuppendialog.h \
           tuponiondialog.h \
           # tupinfowidget.h \
           tupruler.h \
           tupcamerainterface.h \
           tupreflexinterface.h \
           tupbasiccamerainterface.h \
           tupcameradialog.h \
           tupcamerawindow.h \
           tupvideosurface.h \
           tupreflexrenderarea.h \
           tupmodesitem.h \
           tupmodessettingsdialog.h \
           tupmodeslist.h \
           tupprojectimporterdialog.h \
           tupprojectsizedialog.h \
           tupdocumentview.h

SOURCES += tuppaintarea.cpp \
           tupconfigurationarea.cpp \
           tupimagedevice.cpp \
           tuppaintareacommand.cpp \
           tuplibrarydialog.cpp \
           tupcanvas.cpp \
           tupcanvasview.cpp \
           tupimagedialog.cpp \
           tupstoryboarddialog.cpp \
           tuppendialog.cpp \
           tuponiondialog.cpp \
           # tupinfowidget.cpp \
           tupruler.cpp \
           tupcamerainterface.cpp \
           tupreflexinterface.cpp \
           tupbasiccamerainterface.cpp \
           tupcameradialog.cpp \
           tupcamerawindow.cpp \
           tupvideosurface.cpp \
           tupreflexrenderarea.cpp \
           tupmodesitem.cpp \
           tupmodessettingsdialog.cpp \
           tupmodeslist.cpp \
           tupprojectimporterdialog.cpp \
           tupprojectsizedialog.cpp \
           tupdocumentview.cpp
