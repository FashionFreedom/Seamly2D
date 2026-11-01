# SPDX-License-Identifier: GPL-3.0-or-later
include(../../../common.pri)
TEMPLATE = app
TARGET = McpTest
QT = core network testlib
CONFIG += console testcase no_testcase_installs
CONFIG -= app_bundle precompile_header
DESTDIR = bin
OBJECTS_DIR = obj
MOC_DIR = moc
SOURCES += tst_mcp.cpp ../../app/seamly2d_mcp/mcp_server.cpp
HEADERS += ../../app/seamly2d_mcp/mcp_server.h
RESOURCES += ../../app/seamly2d_mcp/resources/mcp.qrc
RCC_DIR = rcc
