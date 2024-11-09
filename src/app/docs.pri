HELP_COLLECTION_FILES += \
    $${PWD}/share/docs/seamly.qhc \
    $${PWD}/share/docs/en.qch \
    $${PWD}/share/docs/fr.qch

win32
{
    copyToDestdir($$HELP_COLLECTION_FILES, $$shell_path($${OUT_PWD}/$${DESTDIR}/docs))
}

DISTFILES += \
    $$PWD/share/docs/en.qch \
    $$PWD/share/docs/en.qhp \
    $$PWD/share/docs/en/seamly.html \
    $$PWD/share/docs/fr.qch \
    $$PWD/share/docs/fr.qhp \
    $$PWD/share/docs/fr/seamly.html \
    $$PWD/share/docs/readme.txt \
    $$PWD/share/docs/seamly.qhc \
    $$PWD/share/docs/seamly.qhcp
