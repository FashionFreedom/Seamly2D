#-------------------------------------------------
#
# Project created by QtCreator 2017-06-01T10:07:06
#
#-------------------------------------------------

# File with common stuff for whole project
message("Entering vtest.pro")
include(../../../common.pri)

QT += testlib widgets printsupport

# Name of library
TARGET = vtest

# We want to create a library
TEMPLATE = lib

CONFIG += staticlib

include(vtest.pri)

# This is static library so no need in "make install"

# directory for executable file
DESTDIR = bin

# files created moc
MOC_DIR = moc

# objecs files
OBJECTS_DIR = obj

include(warnings.pri)

include (../libs.pri)
