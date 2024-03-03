unix:SKIN_DEST_DIR = $$PROJECT_DIR/skins
mac:SKIN_DEST_DIR = $$PROJECT_DIR/$${APP_NAME}.app/Contents/MacOS/skins
win32:SKIN_DEST_DIR = $$PROJECT_DIR/Skins
system($$QMAKE_MKDIR $$fixSlashes($$SKIN_DEST_DIR))

unix:SKINS =  metro silver slim
win32:SKINS = Metro Silver Slim
for(skin, SKINS) {
    _depends = $$SRC_DIR/skins/$$lower($${skin})
    _target = $$SKIN_DEST_DIR/$${skin}.nzs
    _commands = zip -j -X -x\*design.svg $$_target $$_depends/*
    win32:!unix_mingw:_commands = 7z a -tzip -w$$_depends -x\!design.svg $$_target $$_depends/*
    eval($${skin}.depends = $$_depends/*)
    eval($${skin}.target = $$_target)
    eval($${skin}.commands = $$_commands)

    QMAKE_EXTRA_TARGETS += $${skin}
    PRE_TARGETDEPS += $$_target
    system($$_commands)

    unix:!mac:system($$QMAKE_LN_SHLIB -f $$_depends $$SKIN_DEST_DIR/$$skin)
}

# install
unix:!mac {
    skins.files = ../skins/*
    skins.path = $$PREFIX/share/$$APP_NAME/skins
    INSTALLS += skins
}
