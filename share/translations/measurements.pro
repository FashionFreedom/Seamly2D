# Don't use this pro file for building project!!!
# I made this hack for creation one TS file for whole project.
# Because we have several libraries that also have strings for translation, have several TS files for each
# subproject annoying.
#
# Original idea "How to generate a single translation file for a large Qt project?"
# https://stackoverflow.com/questions/1829474/how-to-generate-a-single-translation-file-for-a-large-qt-project

TEMPLATE = app
DEPENDPATH +=  \
            ../../src/libs/vpatterndb

include(../../src/libs/vpatterndb/trmeasurements.pri)

# Add here path to new translation file with name "measurements_*_*.ts" if you want to add new language.
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
    ro_RO \
    zh_CN \
    pt_BR \
    el_GR

for(lang, LANGUAGES) {
    TRANSLATIONS += measurements_$${lang}.ts
}
