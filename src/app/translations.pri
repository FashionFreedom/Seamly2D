TRANSLATIONS_PATH = $${PWD}/../../share/translations

# Add to this variable all translation files that you want install with program.
# For generation *.qm file first you need create *.ts.
# See section TRANSLATIONS in file ../../share/translations/translations.pro.
# See section TRANSLATIONS in file ../../share/translations/measurements.pro.

# An example of using LOCALES="de_DE nl_NL"
isEmpty(LOCALES){
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
} else {
    LANGUAGES = $${LOCALES}
}

CONFIG += lrelease embed_translations

for(lang, LANGUAGES) {
    EXTRA_TRANSLATIONS += $${TRANSLATIONS_PATH}/seamly2d_$${lang}.ts
    EXTRA_TRANSLATIONS += $${TRANSLATIONS_PATH}/measurements_$${lang}.ts
}
