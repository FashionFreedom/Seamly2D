# This pro file is used to run lupdate from top level pro file.
#
# Original idea "How to generate a single translation file for a large Qt project?"
# https://stackoverflow.com/questions/1829474/how-to-generate-a-single-translation-file-for-a-large-qt-project

TEMPLATE = app
DEPENDPATH +=  \
            ../../src/app/seamly2d \
            ../../src/app/seamlyme \
            ../../src/libs/qmuparser \
            ../../src/libs/vpropertyexplorer \
            ../../src/libs/ifc \
            ../../src/libs/vobj \
            ../../src/libs/vlayout \
            ../../src/libs/vgeometry \
            ../../src/libs/vpatterndb \
            ../../src/libs/vmisc \
            ../../src/libs/tools \
            ../../src/libs/vtools \
            ../../src/libs/vformat \
            ../../src/libs/vwidgets \
            ../../src/libs/fervor

include(../../src/app/seamly2d/seamly2d.pri)
include(../../src/app/seamlyme/seamlyme.pri)
include(../../src/libs/qmuparser/qmuparser.pri)
include(../../src/libs/vpropertyexplorer/vpropertyexplorer.pri)
include(../../src/libs/ifc/ifc.pri)
include(../../src/libs/vobj/vobj.pri)
include(../../src/libs/vlayout/vlayout.pri)
include(../../src/libs/vgeometry/vgeometry.pri)
include(../../src/libs/vpatterndb/vpatterndb.pri)
include(../../src/libs/vmisc/vmisc.pri)
include(../../src/libs/tools/tools.pri)
include(../../src/libs/vtools/vtools.pri)
include(../../src/libs/vformat/vformat.pri)
include(../../src/libs/vwidgets/vwidgets.pri)
include(../../src/libs/fervor/fervor.pri)

# Add here path to new translation file with name "seamly2d_*_*.ts" if you want to add new language.
# Same paths in variable INSTALL_TRANSLATIONS (translations.pri).

LANGUAGES += \
    ru_RU \
    uk_UA \
    de_DE \
    cs_CZ \
    he_IL \
    fr_FR \
    it_IT \
    nl_NL \
    id_ID \
    es_ES \
    fi_FI \
    en_US \
    en_CA \
    en_IN \
    en_GB \
    ro_RO \
    zh_CN \
    pt_BR \
    el_GR

for(lang, LANGUAGES) {
    TRANSLATIONS += seamly2d_$${lang}.ts
}
