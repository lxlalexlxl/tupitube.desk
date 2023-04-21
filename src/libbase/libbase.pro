QT += widgets svgwidgets opengl core gui svg xml network core5compat
TEMPLATE = lib
TARGET = tupibase

macx {
    CONFIG += plugin warn_on
} else {
    CONFIG += dll warn_on
}

unix { 
    !include(../../tupiglobal.pri) {
        error("Please configure first")
    }
}

win32 {
    # LIBS += -llibEGL -llibGLESv2 -lgdi32 -luser32
    LIBS += -lgdi32 -luser32
    include(../../win.pri)
}

INSTALLS += target 
target.path = /lib

contains("DEFINES", "ADD_HEADERS") {
    INSTALLS += headers 
    headers.commands = cp *.h $(INSTALL_ROOT)/include/tupibase
    headers.path = /include/tupibase
}

HEADERS += tupexportinterface.h \
           tupexportpluginobject.h \
           tupfilterinterface.h \
           tupmodulewidgetbase.h \
           tuppluginmanager.h \
           tuptoolinterface.h \
           tuptoolplugin.h  \
           tuppaintareabase.h \
           # tuppaintarearotator.h \
           tupgraphicsscene.h \
           tupguideline.h \
           tupanimationrenderer.h \
           # tupwebhunter.h \
           tuprotationdial.h \
           tupsecurity.h \
           tupwatermark.h

SOURCES += tupexportpluginobject.cpp \
           tupmodulewidgetbase.cpp \
           tuppluginmanager.cpp \
           tuptoolplugin.cpp  \
           tuppaintareabase.cpp \
           # tuppaintarearotator.cpp \
           tupgraphicsscene.cpp \
           tupguideline.cpp \
           tupanimationrenderer.cpp \
           # tupwebhunter.cpp \
           tuprotationdial.cpp \
           tupsecurity.cpp \
           tupwatermark.cpp

FRAMEWORK_DIR = "../framework"
include($$FRAMEWORK_DIR/framework.pri)
LIBTUPI_DIR = ../libtupi
include($$LIBTUPI_DIR/libtupi.pri)
STORE_DIR = ../store
include($$STORE_DIR/store.pri)
