DEPENDPATH += .
INCLUDEPATH += /usr/include/QtMobility /usr/include/QtMultimediaKit

macx {
    CONFIG -= app_bundle
    CONFIG += warn_on static console
}

# Input
SOURCES += main.cpp
LIBS += -lQtMultimediaKit
