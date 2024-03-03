QT += script gui svg core-private

QT += quick
qml.files = $$files(*.qml) qmldir
qml.prefix = src
RESOURCES += qml
#CONFIG += qml_debug
CONFIG += qmltypes
QML_IMPORT_NAME = Nulloy
QML_IMPORT_MAJOR_VERSION = 1

INCLUDEPATH += $$SRC_DIR $$SRC_DIR/interfaces

HEADERS += $$SRC_DIR/*.h
SOURCES += $$files($$SRC_DIR/*.cpp)
SOURCES -= $$SRC_DIR/main.cpp

FORMS += $$SRC_DIR/*.ui

CONFIG += app_bundle

include($$SRC_DIR/func.pri)
include($$PROJECT_DIR/3rdParty/qxt-696423b-patched/qxtglobalshortcut.pri)
include($$PROJECT_DIR/3rdParty/qtsingleapplication-8fd81b2/src/qtsingleapplication.pri)
include($$PROJECT_DIR/3rdParty/qtiocompressor-2.3.1/src/qtiocompressor.pri)
include($$SRC_DIR/i18n/i18n.pri)
include($$SRC_DIR/platform/trash.pri)
win32:include($$SRC_DIR/platform/w7TaskBar.pri)
win32:include($$SRC_DIR/platform/winIcons.pri)
mac:include($$SRC_DIR/platform/macDock.pri)
unix:!mac:include($$SRC_DIR/platform/xcb.pri)

# zlib
!mac {
    CONFIG += link_pkgconfig
    PKGCONFIG += zlib
} else {
    LIBS += -L/usr/lib -lz
}

unix:!mac:PKGCONFIG += x11

# qmake -config no-skins
!no-skins {
    include($$SRC_DIR/skins/skins.pri)
    QT += uitools
    INCLUDEPATH += $$SRC_DIR/widgetCollection
    LIBS += -L$$SRC_DIR/widgetCollection -lwidget_collection
    PRE_TARGETDEPS += $$SRC_DIR/widgetCollection/libwidget_collection.a
    RESOURCES += $$SRC_DIR/native-skin-embedded.qrc
} else {
    DEFINES += _N_NO_SKINS_

    HEADERS -= $$SRC_DIR/skinFileSystem.h   $$SRC_DIR/skinLoader.h
    SOURCES -= $$SRC_DIR/skinFileSystem.cpp $$SRC_DIR/skinLoader.cpp

    HEADERS += $$files($$SRC_DIR/widgetCollection/*.h)
    SOURCES += $$files($$SRC_DIR/widgetCollection/*.cpp)
    HEADERS -= $$SRC_DIR/widgetCollection/widgetCollection.h
    SOURCES -= $$SRC_DIR/widgetCollection/widgetCollection.cpp

    DEPENDPATH +=  $$SRC_DIR/widgetCollection
    INCLUDEPATH += $$SRC_DIR/widgetCollection

    RESOURCES += $$SRC_DIR/no-skins.qrc
    FORMS += $$SRC_DIR/skins/native/form.ui
}

no-update-check:DEFINES += _N_NO_UPDATE_CHECK_

include(version.pri)
DEFINES += _N_VERSION_=\""\\\"$${N_VERSION}\\\""\"
build_pass:CONFIG(static, static|shared) {
    DEFINES += _N_STATIC_BUILD_
} else {
    DEFINES += _N_SHARED_BUILD_
}

system(cp $$SRC_DIR/icons/icon.svg $$TMP_DIR/)
system(cp $$SRC_DIR/icons/icons.qrc $$TMP_DIR/)
RESOURCES += $$TMP_DIR/icons.qrc
CONVERT_CMD = convert
COMPOSITE_CMD = composite
win32:!unix_mingw {
    CONVERT_CMD = magick convert
    COMPOSITE_CMD = magick composite
}

ico.depends = $$SRC_DIR/icons/icon.svg
ico.target = $$TMP_DIR/icon.ico
ico.commands = $$CONVERT_CMD $$ico.depends -define icon:auto-resize $$ico.target
PRE_TARGETDEPS += $$ico.target
QMAKE_EXTRA_TARGETS += ico
system($$ico.commands)

win32 {
    file_ico.depends = $$SRC_DIR/icons/file.svg $$SRC_DIR/icons/icon.svg
    file_ico.target = $$TMP_DIR/file.ico
    file_ico.commands = $$COMPOSITE_CMD \\( $$SRC_DIR/icons/icon.svg -resize 50% -gravity center \\) $$SRC_DIR/icons/file.svg -define icon:auto-resize $$file_ico.target
    PRE_TARGETDEPS += $$file_ico.target
    QMAKE_EXTRA_TARGETS += file_ico
    system($$file_ico.commands)
}

system(cp $$SRC_DIR/icons/icon.rc $$TMP_DIR/)
win32:RC_FILE = $$TMP_DIR/icon.rc

mac {
    system($$QMAKE_MKDIR $$TMP_DIR/icon.iconset)
    SIZES = 16 32 128 256
    for(size, SIZES) {
        system($$CONVERT_CMD $$SRC_DIR/icons/icon.svg -density 72 -resize $${size}x$${size} -units PixelsPerInch $$TMP_DIR/icon.iconset/icon_$${size}x$${size}.png)
    }
    system(iconutil -c icns -o $$TMP_DIR/icon.icns $$TMP_DIR/icon.iconset)
    ICON = $$TMP_DIR/icon.icns
}

unix:!mac {
    # qmake "PREFIX=/usr"
    target.path = $$PREFIX/bin
    INSTALLS += target

    system(cp $$SRC_DIR/icons/icon.svg $$TMP_DIR/$${APP_NAME}.svg)
    icon.files = $$TMP_DIR/$${APP_NAME}.svg
    icon.path = $$PREFIX/share/icons/hicolor/scalable/apps
    INSTALLS += icon

    desktop.files = ../$${APP_NAME}.desktop
    desktop.path = $$PREFIX/share/applications
    INSTALLS += desktop
}

mac {
    xcode_override.name = "CONFIGURATION_BUILD_DIR"
    xcode_override.value = $$TMP_DIR
    QMAKE_MAC_XCODE_SETTINGS += xcode_override
}
