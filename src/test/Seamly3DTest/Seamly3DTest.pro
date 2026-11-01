# SPDX-License-Identifier: GPL-3.0-or-later
include(../../../common.pri)
TEMPLATE = app
TARGET = Seamly3DTests
QT = core gui widgets testlib
CONFIG += testcase no_testcase_installs console
CONFIG -= app_bundle precompile_header
DESTDIR = bin
OBJECTS_DIR = obj
MOC_DIR = moc
SOURCES += tst_seamly3d.cpp ../../app/seamly3d/model.cpp ../../app/seamly3d/window.cpp ../../app/seamly3d/viewport.cpp
HEADERS += ../../app/seamly3d/model.h

include(../../app/seamly3d/renderer.pri)

SOURCES += ../../app/seamly3d/dressform.cpp
HEADERS += ../../app/seamly3d/dressform.h

SOURCES += ../../app/seamly3d/clothsolver.cpp
HEADERS += ../../app/seamly3d/clothsolver.h
