#-------------------------------------------------
#
# Project created by QtCreator 2016-08-04T16:33:17
#
#-------------------------------------------------

QT       += testlib widgets xml printsupport

QT       -= gui

TARGET = TranslationsTest

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
    tst_measurementregexp.cpp \
    tst_qmuparsererrormsg.cpp \
    tst_tstranslation.cpp \
    tst_buitinregexp.cpp \
    tst_abstractregexp.cpp

*msvc*:SOURCES += stable.cpp

HEADERS += \
    stable.h \
    tst_measurementregexp.h \
    tst_qmuparsererrormsg.h \
    tst_tstranslation.h \
    tst_buitinregexp.h \
    tst_abstractregexp.h

DEFINES += TS_DIR=\\\"$${PWD}/../../../share/translations\\\"

include(warnings.pri)

#VPatternDB static library (depend on vgeometry, vmisc, VLayout)
unix|win32: LIBS += -L$$OUT_PWD/../../libs/vpatterndb/$${DESTDIR} -lvpatterndb

INCLUDEPATH += $$PWD/../../libs/vpatterndb
DEPENDPATH += $$PWD/../../libs/vpatterndb

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vpatterndb/$${DESTDIR}/vpatterndb.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vpatterndb/$${DESTDIR}/libvpatterndb.a

#VTest static library (depend on VGeometry)
unix|win32: LIBS += -L$$OUT_PWD/../../libs/vtest/$${DESTDIR} -lvtest

INCLUDEPATH += $$PWD/../../libs/vtest
DEPENDPATH += $$PWD/../../libs/vtest

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vtest/$${DESTDIR}/vtest.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vtest/$${DESTDIR}/libvtest.a

#VMisc static library
unix|win32: LIBS += -L$$OUT_PWD/../../libs/vmisc/$${DESTDIR} -lvmisc

INCLUDEPATH += $$PWD/../../libs/vmisc
DEPENDPATH += $$PWD/../../libs/vmisc

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vmisc/$${DESTDIR}/vmisc.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vmisc/$${DESTDIR}/libvmisc.a

## VGeometry static library (depend on ifc)
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

# QMuParser library
unix|win32: LIBS += -L$${OUT_PWD}/../../libs/qmuparser/$${DESTDIR} -lqmuparser

INCLUDEPATH += $${PWD}/../../libs/qmuparser
DEPENDPATH += $${PWD}/../../libs/qmuparser

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/qmuparser/$${DESTDIR}/qmuparser.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/qmuparser/$${DESTDIR}/libqmuparser.a
