QT += widgets svgwidgets opengl core gui svg xml network core5compat
CONFIG += plugin warn_on
TEMPLATE = lib
TARGET = tupiselectiontool

unix {
    !include(../../../../tupiglobal.pri){
        error("Please run configure first")
    }
}

win32 {
    include(../../../../win.pri)
}

INSTALLS += target 
target.path = /plugins/

HEADERS += selectiontool.h \
           selectionsettings.h

SOURCES += selectiontool.cpp \
           selectionsettings.cpp

FRAMEWORK_DIR = "../../../framework"
include($$FRAMEWORK_DIR/framework.pri)

LIBBASE_DIR = ../../../libbase
STORE_DIR = ../../../store
LIBTUPI_DIR = ../../../libtupi
COMMON_DIR = ../common

include($$LIBBASE_DIR/libbase.pri)	
include($$STORE_DIR/store.pri)
include($$LIBTUPI_DIR/libtupi.pri)
include($$COMMON_DIR/common.pri)
