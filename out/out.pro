TEMPLATE = aux

macx{
    TARGET = Seamly2D-macos.zip
    first.commands = \
        hdiutil create -fs HFS+ -srcfolder ../src/app/seamly2d/bin/Seamly2D.app -volname "Seamly2D" Seamly2D.dmg && \
        hdiutil create -fs HFS+ -srcfolder ../src/app/seamlyme/bin/seamlyme.app -volname "SeamlyME" SeamlyME.dmg && \
        zip Seamly2D-macos.zip Seamly2D.dmg SeamlyME.dmg

    QMAKE_EXTRA_TARGETS += first
    QMAKE_CLEAN += Seamly2D.dmg SeamlyME.dmg Seamly2D-macos.zip
}