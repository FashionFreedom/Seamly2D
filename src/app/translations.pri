TRANSLATIONS_PATH = $${PWD}/../../share/translations

# Add to this variable all translation files that you want install with program.
# For generation *.qm file first you need create *.ts.
# See section TRANSLATIONS in file ../../share/translations/translations.pro.
# See section TRANSLATIONS in file ../../share/translations/measurements.pro.

PMSYSTEMS += \
    p0 p1 p2 p3 p4 p5 p6 p7 p8 p9 p10 p11 p12 p13 p14 p15 p16 p17 p18 p19 p20 p21 p22 p23 p24 p25 p26 p27 p28 p29 p30 \
    p31 p32 p33 p34 p35 p36 p37 p38 p39 p40 p41 p42 p43 p44 p45 p46 p47 p48 p49 p50 p51 p52 p53 p54 p998

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
}

for(lang, LANGUAGES) {
    for(sys, PMSYSTEMS) {
        EXTRA_TRANSLATIONS += $${TRANSLATIONS_PATH}/measurements_$${sys}_$${lang}.ts
    }
}