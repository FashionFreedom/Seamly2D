#-------------------------------------------------
#
# Project created by QtCreator 2014-05-21T20:43:38
#
#-------------------------------------------------

# Build QMuParser tests.

# File with common stuff for whole project
include(../../../common.pri)

# We use many core functions.
QT       += core

# Consol application doesn't need gui.
QT       -= gui

# Name of binary file.
TARGET = ParserTest

# Console application
CONFIG   += console

# CONFIG += testcase adds a  'make check' which is great. But by default it also
# adds a 'make install' that installs the test cases, which we do not want.
# Can configure it not to do that with 'no_testcase_installs'
CONFIG += testcase no_testcase_installs

# directory for executable file
DESTDIR = bin

# objecs files
OBJECTS_DIR = obj

HEADERS += \
    stable.h

SOURCES += \
    main.cpp

*msvc*:SOURCES += stable.cpp

include(warnings.pri)

unix|win32: LIBS += -L$$OUT_PWD/../../libs/qmuparser/bin/ -lqmuparser

INCLUDEPATH += $$PWD/../../libs/qmuparser
DEPENDPATH += $$PWD/../../libs/qmuparser

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/qmuparser/$${DESTDIR}/qmuparser.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/qmuparser/$${DESTDIR}/libqmuparser.a
