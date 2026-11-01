# SPDX-License-Identifier: GPL-3.0-or-later
include(../../../common.pri)
TEMPLATE = app
TARGET = seamly2d-mcp
QT = core network
CONFIG += console
CONFIG -= app_bundle precompile_header
DESTDIR = bin
OBJECTS_DIR = obj
HEADERS += mcp_server.h
SOURCES += main.cpp mcp_server.cpp
RESOURCES += resources/mcp.qrc
RCC_DIR = rcc

unix:!macx {
    isEmpty(PREFIX): PREFIX = $$DEFAULT_PREFIX
    target.path = $$PREFIX/bin
    INSTALLS += target
}
