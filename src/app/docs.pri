HELP_COLLECTION_FILES += \
    $${PWD}/share/docs/seamly.qhc \
    $${PWD}/share/docs/seamly2d.qch \
    $${PWD}/share/docs/seamlyme.qch

win32
{
    copyToDestdir($$HELP_COLLECTION_FILES, $$shell_path($${OUT_PWD}/$${DESTDIR}/docs))
}

DISTFILES += \
    $$PWD/share/docs/html/seamly2d.html \
    $$PWD/share/docs/html/seamlyme.html \
    $$PWD/share/docs/readme.txt \
    $$PWD/share/docs/seamly.qhc \
    $$PWD/share/docs/seamly.qhcp \
    $$PWD/share/docs/seamly2d.qch \
    $$PWD/share/docs/seamly2d.qhp \
    $$PWD/share/docs/seamlyme.qch \
    $$PWD/share/docs/seamlyme.qhp
