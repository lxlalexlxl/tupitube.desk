QT += widgets svgwidgets opengl core gui svg xml network
TEMPLATE = lib
TARGET = tupitimeline

INSTALLS += target
target.path = /lib/

CONFIG += dll warn_on

HEADERS += tuptimeline.h \
           tuptimelineruler.h \
           tuptimelinetable.h \ 
           tuptimelinescenecontainer.h \
           # tuplayerindex.h \
           tuptimelineheader.h

SOURCES += tuptimeline.cpp \
           tuptimelineruler.cpp \
           tuptimelinetable.cpp \
           tuptimelinescenecontainer.cpp \
           # tuplayerindex.cpp \
           tuptimelineheader.cpp

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
