#-------------------------------------------------
#
# Project created by QtCreator 2016-08-04T19:29:55
#
#-------------------------------------------------

QT       += testlib widgets printsupport core5compat

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
    share/all_measurements/all_measurements_v0.3.0.smis \
    share/all_measurements/all_measurements_v0.4.0.smms \
    share/all_measurements/all_measurements_v0.3.3.smis \
    share/all_measurements/all_measurements_v0.4.2.smms \
    share/broken/broken1.smis \
    share/broken/broken2.smis \
    share/broken/broken3.smis \
    share/broken/broken4.smis \
    share/keiko_skirt/keiko.smis \
    share/text/text.smis \
    share/text/text.smms \
    tst_seamlyme/GOST_man_ru_v0.3.0.smms \
    tst_seamlyme/GOST_man_ru_v0.4.2.smms \
    tst_seamlyme/empty.smis

SEAMLY2D_TEST_FILES += \
    share/text/text.sm2d \
    share/glimited/glimited_no_m.sm2d \
    share/glimited/glimited_vit.sm2d \
    share/glimited/glimited.smis \
    share/glimited/glimited_vst.sm2d \
    share/glimited/glimited.smms \
    share/issue_256/issue_256.sm2d \
    share/issue_256/issue_256_wrong_path.sm2d \
    share/issue_256/issue_256_correct.smis \
    share/issue_256/issue_256_wrong.smis \
    share/issue_256/issue_256_correct.smms \
    share/issue_256/issue_256_wrong.smms \
    tst_seamly2d/empty.sm2d \
    tst_seamly2d/issue_372.sm2d \
    tst_seamly2d/wrong_obj_type.sm2d \
    tst_seamly2d/wrong_formula.sm2d

COLLECTION_FILES += \
    share/suit/jacket1_52-176.sm2d \
    share/suit/jacket2_40-146.sm2d \
    share/suit/jacket3_40-146.sm2d \
    share/suit/jacket4_40-146.sm2d \
    share/suit/jacket5_30-110.sm2d \
    share/suit/jacket6_30-110.sm2d \
    share/suit/pants1_52-176.sm2d \
    share/suit/pants2_40-146.sm2d \
    share/bra/bra.sm2d \
    share/bra/bra.smis \
    share/suit/pants7.sm2d \
    share/patron_blusa/patron_blusa.sm2d \
    share/patron_blusa/blusa.smis \
    share/susan/pajama_top_wrap2.sm2d \
    share/susan/mens_shirt_block.sm2d \
    share/susan/test_puzzle.sm2d \
    share/susan/susan.smis \
    share/keiko_skirt/keiko_skirt.sm2d \
    share/keiko_skirt/keiko.smis \
    share/maleshirt/male_shirt.sm2d \
    share/maleshirt/male_shirt.smis \
    share/trousers/trousers.sm2d \
    share/trousers/trousers.smis \
    share/steampunk_trousers/steampunk_trousers.sm2d \
    share/steampunk_trousers/steampunk_trousers.smis \
    share/imk_zhaketa/razmernye_priznaki_dlya_zhenskogo_zhaketa.smis \
    share/imk_zhaketa/imk_zhaketa_poluprilegayuschego_silueta.sm2d \
    share/modell/lara_jil.smis \
    share/modell/modell_2.sm2d \
    share/tshirt_test.sm2d \
    share/test_dart.sm2d \
    share/pantaloon/medidas_eli2015.smis \
    share/pantaloon/pantalon_base_Eli.sm2d \
    share/basic_block_women-2016.sm2d \
    share/gent_jacket_with_tummy.sm2d \
    share/moulage/moulage_05_armhole_neckline.sm2d \
    share/moulage/07_armhole_adjustment_010.sm2d \
    share/moulage/my_calculated_measurements.smis \
    tst_seamlyme/gost_man_ru.smms

# Compilation will fail without this files after we added them to this section.
OTHER_FILES += \
    $$SEAMLYME_TEST_FILES \
    $$SEAMLY2D_TEST_FILES \
    $$COLLECTION_FILES

for(DIR, SEAMLYME_TEST_FILES) {
     #add these absolute paths to a variable which
     #ends up as 'mkcommands = path1 path2 path3 ...'
     seamlyme_path += $${PWD}/$$DIR
}

copyToDestdir($$seamlyme_path, $$shell_path($${OUT_PWD}/$$DESTDIR/tst_seamlyme))

for(DIR, SEAMLY2D_TEST_FILES) {
     #add these absolute paths to a variable which
     #ends up as 'mkcommands = path1 path2 path3 ...'
     seamly2d_path += $${PWD}/$$DIR
}

copyToDestdir($$seamly2d_path, $$shell_path($${OUT_PWD}/$$DESTDIR/tst_seamly2d))

for(DIR, COLLECTION_FILES) {
     #add these absolute paths to a variable which
     #ends up as 'mkcommands = path1 path2 path3 ...'
     collection_path += $${PWD}/$$DIR
}

copyToDestdir($$collection_path, $$shell_path($${OUT_PWD}/$$DESTDIR/tst_seamly2d_collection))
