#-------------------------------------------------
#
# Project created by QtCreator 2016-08-04T19:29:55
#
#-------------------------------------------------

QT       += testlib widgets printsupport

QT       -= gui

TARGET = CollectionTest

# File with common stuff for whole project
include(../../../common.pri)

# CONFIG += testcase adds a  'make check' which is great. But by default it also
# adds a 'make install' that installs the test cases, which we do not want.
# Can configure it not to do that with 'no_testcase_installs'
CONFIG += testcase no_testcase_installs

# disable app bundle to have stable cross-platform relative paths to seamly2d qm files
CONFIG -= app_bundle

# directory for executable file
DESTDIR = bin

# Directory for files created moc
MOC_DIR = moc

# objecs files
OBJECTS_DIR = obj

DEFINES += SRCDIR=\\\"$$PWD/\\\"

SOURCES += \
    qttestmainlambda.cpp \
    tst_seamlymecommandline.cpp \
    tst_seamly2dcommandline.cpp

*msvc*:SOURCES += stable.cpp

HEADERS += \
    stable.h \
    tst_seamlymecommandline.h \
    tst_seamly2dcommandline.h

include(warnings.pri)

#VTools static library (depend on VWidgets, VMisc, VPatternDB)
unix|win32: LIBS += -L$$OUT_PWD/../../libs/vtools/$${DESTDIR}/ -lvtools

INCLUDEPATH += $$PWD/../../libs/vtools
DEPENDPATH += $$PWD/../../libs/vtools

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vtools/$${DESTDIR}/vtools.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vtools/$${DESTDIR}/libvtools.a

#VWidgets static library
unix|win32: LIBS += -L$$OUT_PWD/../../libs/vwidgets/$${DESTDIR}/ -lvwidgets

INCLUDEPATH += $$PWD/../../libs/vwidgets
DEPENDPATH += $$PWD/../../libs/vwidgets

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vwidgets/$${DESTDIR}/vwidgets.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vwidgets/$${DESTDIR}/libvwidgets.a

# VFormat static library (depend on VPatternDB, IFC)
unix|win32: LIBS += -L$$OUT_PWD/../../libs/vformat/$${DESTDIR}/ -lvformat

INCLUDEPATH += $$PWD/../../libs/vformat
DEPENDPATH += $$PWD/../../libs/vformat

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vformat/$${DESTDIR}/vformat.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vformat/$${DESTDIR}/libvformat.a

#VPatternDB static library (depend on vgeometry, vmisc, VLayout)
unix|win32: LIBS += -L$$OUT_PWD/../../libs/vpatterndb/$${DESTDIR} -lvpatterndb

INCLUDEPATH += $$PWD/../../libs/vpatterndb
DEPENDPATH += $$PWD/../../libs/vpatterndb

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vpatterndb/$${DESTDIR}/vpatterndb.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vpatterndb/$${DESTDIR}/libvpatterndb.a

#VTest static library
unix|win32: LIBS += -L$$OUT_PWD/../../libs/vtest/$${DESTDIR} -lvtest

INCLUDEPATH += $$PWD/../../libs/vtest
DEPENDPATH += $$PWD/../../libs/vtest

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vtest/$${DESTDIR}/vtest.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vtest/$${DESTDIR}/libvtest.a

# VGeometry static library (depend on ifc)
unix|win32: LIBS += -L$$OUT_PWD/../../libs/vgeometry/$${DESTDIR} -lvgeometry

INCLUDEPATH += $$PWD/../../libs/vgeometry
DEPENDPATH += $$PWD/../../libs/vgeometry

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vgeometry/$${DESTDIR}/vgeometry.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vgeometry/$${DESTDIR}/libvgeometry.a

# IFC static library (depend on QMuParser)
unix|win32: LIBS += -L$$OUT_PWD/../../libs/ifc/$${DESTDIR}/ -lifc

INCLUDEPATH += $$PWD/../../libs/ifc
DEPENDPATH += $$PWD/../../libs/ifc

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/ifc/$${DESTDIR}/ifc.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/ifc/$${DESTDIR}/libifc.a

#VMisc static library
unix|win32: LIBS += -L$$OUT_PWD/../../libs/vmisc/$${DESTDIR} -lvmisc

INCLUDEPATH += $$PWD/../../libs/vmisc
DEPENDPATH += $$PWD/../../libs/vmisc

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vmisc/$${DESTDIR}/vmisc.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vmisc/$${DESTDIR}/libvmisc.a

# VLayout static library
unix|win32: LIBS += -L$$OUT_PWD/../../libs/vlayout/$${DESTDIR} -lvlayout

INCLUDEPATH += $$PWD/../../libs/vlayout
DEPENDPATH += $$PWD/../../libs/vlayout

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vlayout/$${DESTDIR}/vlayout.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vlayout/$${DESTDIR}/libvlayout.a

# QMuParser library
unix|win32: LIBS += -L$${OUT_PWD}/../../libs/qmuparser/$${DESTDIR} -lqmuparser

INCLUDEPATH += $${PWD}/../../libs/qmuparser
DEPENDPATH += $${PWD}/../../libs/qmuparser

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/qmuparser/$${DESTDIR}/qmuparser.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/qmuparser/$${DESTDIR}/libqmuparser.a

# VPropertyExplorer library
unix|win32: LIBS += -L$${OUT_PWD}/../../libs/vpropertyexplorer/$${DESTDIR} -lvpropertyexplorer

INCLUDEPATH += $${PWD}/../../libs/vpropertyexplorer
DEPENDPATH += $${PWD}/../../libs/vpropertyexplorer

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vpropertyexplorer/$${DESTDIR}/vpropertyexplorer.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vpropertyexplorer/$${DESTDIR}/libvpropertyexplorer.a

SEAMLYME_TEST_FILES += \
    tst_seamlyme/keiko.vit \
    tst_seamlyme/empty.vit \
    tst_seamlyme/all_measurements_v0.3.0.vit \
    tst_seamlyme/all_measurements_v0.4.0.vst \
    tst_seamlyme/GOST_man_ru_v0.3.0.vst \
    tst_seamlyme/all_measurements_v0.3.3.vit \
    tst_seamlyme/all_measurements_v0.4.2.vst \
    tst_seamlyme/GOST_man_ru_v0.4.2.vst \
    tst_seamlyme/broken1.vit \
    tst_seamlyme/broken2.vit \
    tst_seamlyme/broken3.vit \
    tst_seamlyme/broken4.vit \
    tst_seamlyme/text.vit \
    tst_seamlyme/text.vst

seamly2d_TEST_FILES += \
    tst_seamly2d/empty.val \
    tst_seamly2d/issue_372.val \
    tst_seamly2d/wrong_obj_type.val \
    tst_seamly2d/text.val \
    tst_seamly2d/glimited_no_m.val \
    tst_seamly2d/glimited_vit.val \
    tst_seamly2d/glimited.vit \
    tst_seamly2d/glimited_vst.val \
    tst_seamly2d/glimited.vst \
    tst_seamly2d/issue_256.val \
    tst_seamly2d/issue_256_wrong_path.val \
    tst_seamly2d/issue_256_correct.vit \
    tst_seamly2d/issue_256_wrong.vit \
    tst_seamly2d/issue_256_correct.vst \
    tst_seamly2d/issue_256_wrong.vst \
    tst_seamly2d/wrong_formula.val

COLLECTION_FILES += \
    $${PWD}/../../app/share/tables/multisize/GOST_man_ru.vst \
    $${PWD}/../../app/share/collection/bra.val \
    $${PWD}/../../app/share/collection/bra.vit \
    $${PWD}/../../app/share/collection/jacket1_52-176.val \
    $${PWD}/../../app/share/collection/jacket2_40-146.val \
    $${PWD}/../../app/share/collection/jacket3_40-146.val \
    $${PWD}/../../app/share/collection/jacket4_40-146.val \
    $${PWD}/../../app/share/collection/jacket5_30-110.val \
    $${PWD}/../../app/share/collection/jacket6_30-110.val \
    $${PWD}/../../app/share/collection/pants1_52-176.val \
    $${PWD}/../../app/share/collection/pants2_40-146.val \
    $${PWD}/../../app/share/collection/pants7.val \
    $${PWD}/../../app/share/collection/TShirt_test.val \
    $${PWD}/../../app/share/collection/TestDart.val \
    $${PWD}/../../app/share/collection/patron_blusa.val \
    $${PWD}/../../app/share/collection/blusa.vit \
    $${PWD}/../../app/share/collection/PajamaTopWrap2.val \
    $${PWD}/../../app/share/collection/Susan.vit \
    $${PWD}/../../app/share/collection/Moulage_0.5_armhole_neckline.val \
    $${PWD}/../../app/share/collection/0.7_Armhole_adjustment_0.10.val \
    $${PWD}/../../app/share/collection/my_calculated_measurements_for_val.vit \
    $${PWD}/../../app/share/collection/Keiko_skirt.val \
    $${PWD}/../../app/share/collection/keiko.vit \
    $${PWD}/../../app/share/collection/medidas_eli2015.vit \
    $${PWD}/../../app/share/collection/pantalon_base_Eli.val \
    $${PWD}/../../app/share/collection/Razmernye_priznaki_dlya_zhenskogo_zhaketa.vit \
    $${PWD}/../../app/share/collection/IMK_Zhaketa_poluprilegayuschego_silueta.val \
    $${PWD}/../../app/share/collection/Lara_Jil.vit \
    $${PWD}/../../app/share/collection/modell_2.val \
    $${PWD}/../../app/share/collection/MaleShirt/MaleShirt.val \
    $${PWD}/../../app/share/collection/MaleShirt/MaleShirt.vit \
    $${PWD}/../../app/share/collection/Trousers/Trousers.val \
    $${PWD}/../../app/share/collection/Trousers/trousers.vit \
    $${PWD}/../../app/share/collection/Steampunk_trousers.val \
    $${PWD}/../../app/share/collection/Steampunk_trousers.vit \
    $${PWD}/../../app/share/collection/Basic_block_women-2016.val \
    $${PWD}/../../app/share/collection/Gent_Jacket_with_tummy.val

# Compilation will fail without this files after we added them to this section.
OTHER_FILES += \
    $$SEAMLYME_TEST_FILES \
    $$seamly2d_TEST_FILES \
    $$COLLECTION_FILES

for(DIR, SEAMLYME_TEST_FILES) {
     #add these absolute paths to a variable which
     #ends up as 'mkcommands = path1 path2 path3 ...'
     seamlyme_path += $${PWD}/$$DIR
}

copyToDestdir($$seamlyme_path, $$shell_path($${OUT_PWD}/$$DESTDIR/tst_seamlyme))

for(DIR, seamly2d_TEST_FILES) {
     #add these absolute paths to a variable which
     #ends up as 'mkcommands = path1 path2 path3 ...'
     seamly2d_path += $${PWD}/$$DIR
}

copyToDestdir($$seamly2d_path, $$shell_path($${OUT_PWD}/$$DESTDIR/tst_seamly2d))

for(DIR, COLLECTION_FILES) {
     #add these absolute paths to a variable which
     #ends up as 'mkcommands = path1 path2 path3 ...'
     collection_path += $$DIR
}

copyToDestdir($$collection_path, $$shell_path($${OUT_PWD}/$$DESTDIR/tst_seamly2d_collection))
