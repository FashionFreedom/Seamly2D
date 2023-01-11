TEMPLATE = aux

macx{
    seamly2ddmg.target = Seamly2D.dmg
    seamly2ddmg.commands = hdiutil create -fs HFS+ -srcfolder $${OUT_PWD}/../src/app/seamly2d/bin/Seamly2D.app -volname "Seamly2D" $$seamly2ddmg.target

    seamlymedmg.target = SeamlyME.dmg
    seamlymedmg.commands = hdiutil create -fs HFS+ -srcfolder $${OUT_PWD}/../src/app/seamlyme/bin/seamlyme.app -volname "SeamlyME" $$seamlymedmg.target

    TARGET = Seamly2D-macos.zip
    first.commands = zip Seamly2D-macos.zip Seamly2D.dmg SeamlyME.dmg
    first.depends = seamly2ddmg seamlymedmg

    QMAKE_EXTRA_TARGETS += seamly2ddmg seamlymedmg first
    QMAKE_CLEAN += Seamly2D.dmg SeamlyME.dmg Seamly2D-macos.zip
}
