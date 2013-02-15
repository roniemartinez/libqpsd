#-------------------------------------------------
#
# Project created by QtCreator 2012-11-19T20:43:16
#
#-------------------------------------------------

QT       += core gui

CONFIG += plugin debug_and_release

CONFIG(debug, debug|release) {
    TARGET = qpsdd4
}
CONFIG(release, debug|release) {
    TARGET = qpsd4
}

TEMPLATE = lib


DESTDIR = $$[QT_INSTALL_PLUGINS]/imageformats

MOC_DIR = moc
OBJECTS_DIR = obj

SOURCES += \
    qpsdplugin.cpp

HEADERS += \
    qpsdplugin.h
symbian {
# Load predefined include paths (e.g. QT_PLUGINS_BASE_DIR) to be used in the pro-files
    load(data_caging_paths)
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xE67F0762
    TARGET.CAPABILITY = 
    TARGET.EPOCALLOWDLLDATA = 1
    pluginDeploy.sources = qpsd.dll
    pluginDeploy.path = $$QT_PLUGINS_BASE_DIR/qpsd
    DEPLOYMENT += pluginDeploy
}

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
