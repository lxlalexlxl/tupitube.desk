QT += widgets svgwidgets opengl core gui svg xml network multimedia
TEMPLATE = lib
TARGET = tupiplayer

INSTALLS += target
target.path = /lib/

CONFIG += dll warn_on

HEADERS += tupcamerawidget.h \
           tupcamerabar.h \
           tupscreen.h \
           tupcamerastatus.h \
           tupanimationspace.h \
           tupinfodialog.h

SOURCES += tupcamerawidget.cpp \
           tupcamerabar.cpp \
           tupscreen.cpp \
           tupcamerastatus.cpp \
           tupanimationspace.cpp \
           tupinfodialog.cpp

EXPORT_DIR = ../export
include($$EXPORT_DIR/export.pri)

FRAMEWORK_DIR = "../../framework"
include($$FRAMEWORK_DIR/framework.pri)

unix {
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
    QT += winextras

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
