TEMPLATE = aux

macx{
    APPLE_SIGN_IDENTITY = $$shell_quote($(APPLE_SIGN_IDENTITY))
    APPLE_NOTARIZE_KEY = $(APPLE_NOTARIZE_KEY)
    APPLE_NOTARIZE_KEY_ID = $(APPLE_NOTARIZE_KEY_ID)
    APPLE_NOTARIZE_ISSUER_ID = $(APPLE_NOTARIZE_ISSUER_ID)

    seamly2ddmg.target = Seamly2D.dmg
    seamly2ddmg.commands = hdiutil create -fs HFS+ -srcfolder $${OUT_PWD}/../src/app/seamly2d/bin/Seamly2D.app -volname "Seamly2D" $$seamly2ddmg.target

    macSign {
        seamly2ddmg.commands += && codesign --options runtime --timestamp -s $${APPLE_SIGN_IDENTITY} $$seamly2ddmg.target
        seamly2ddmg.commands += && xcrun notarytool submit --key $${APPLE_NOTARIZE_KEY} --key-id $${APPLE_NOTARIZE_KEY_ID} --issuer $${APPLE_NOTARIZE_ISSUER_ID} --wait $$seamly2ddmg.target
        seamly2ddmg.commands += && xcrun stapler staple -v $$seamly2ddmg.target
    }

    seamlymedmg.target = SeamlyME.dmg
    seamlymedmg.commands = hdiutil create -fs HFS+ -srcfolder $${OUT_PWD}/../src/app/seamlyme/bin/seamlyme.app -volname "SeamlyME" $$seamlymedmg.target

    macSign {
        seamlymedmg.commands += && codesign --options runtime --timestamp -s $${APPLE_SIGN_IDENTITY} $$seamlymedmg.target
        seamlymedmg.commands += && xcrun notarytool submit --key $${APPLE_NOTARIZE_KEY} --key-id $${APPLE_NOTARIZE_KEY_ID} --issuer $${APPLE_NOTARIZE_ISSUER_ID} --wait $$seamlymedmg.target
        seamlymedmg.commands += && xcrun stapler staple -v $$seamlymedmg.target
    }

    TARGET = Seamly2D-macos.zip
    first.commands = zip Seamly2D-macos.zip Seamly2D.dmg SeamlyME.dmg
    first.depends = seamly2ddmg seamlymedmg

    QMAKE_EXTRA_TARGETS += seamly2ddmg seamlymedmg first
    QMAKE_CLEAN += Seamly2D.dmg SeamlyME.dmg Seamly2D-macos.zip
}
