# Keep path to all files with individual measurements we support right now
INSTALL_INDIVIDUAL_MEASUREMENTS += \
    $${PWD}/share/samples/measurements/individual/male_shirt.smis \
    $${PWD}/share/samples/measurements/individual/trousers.smis

# Keep path to all files with multisize measurements we support right now
INSTALL_MULTISIZE_MEASUREMENTS += \
    $${PWD}/share/samples/measurements/multisize/GOST_man_ru.smms

# Keep path to all template files we have right now
INSTALL_STANDARD_TEMPLATES += \
    $${PWD}/share/samples/measurements/templates/all_measurements_template.smis \
    $${PWD}/share/samples/measurements/templates/aldrich_women_template.smis

# Keep path to all patterns we provide right now
INSTALL_PATTERNS += \
    $${PWD}/share/samples/patterns/jacket1_52-176.sm2d \
    $${PWD}/share/samples/patterns/jacket2_40-146.sm2d \
    $${PWD}/share/samples/patterns/jacket3_40-146.sm2d \
    $${PWD}/share/samples/patterns/jacket4_40-146.sm2d \
    $${PWD}/share/samples/patterns/jacket5_30-110.sm2d \
    $${PWD}/share/samples/patterns/jacket6_30-110.sm2d \
    $${PWD}/share/samples/patterns/male_shirt.sm2d \
    $${PWD}/share/samples/patterns/trousers.sm2d

# Keep path to all label templates we provide right now
INSTALL_LABEL_TEMPLATES += \
    $${PWD}/share/labels/default_pattern_label.xml \
    $${PWD}/share/labels/default_piece_label.xml

win32
{
    copyToDestdir($$INSTALL_INDIVIDUAL_MEASUREMENTS, $$shell_path($${OUT_PWD}/$${DESTDIR}/samples/measurements/individual))
    copyToDestdir($$INSTALL_MULTISIZE_MEASUREMENTS, $$shell_path($${OUT_PWD}/$${DESTDIR}/samples/measurements/multisize))
    copyToDestdir($$INSTALL_STANDARD_TEMPLATES, $$shell_path($${OUT_PWD}/$${DESTDIR}/samples/measurements/templates))
    copyToDestdir($$INSTALL_PATTERNS, $$shell_path($${OUT_PWD}/$${DESTDIR}/samples/patterns))
    copyToDestdir($$INSTALL_LABEL_TEMPLATES, $$shell_path($${OUT_PWD}/$${DESTDIR}/labels))
}
