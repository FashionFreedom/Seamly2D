#-------------------------------------------------
#
# Project created by QtCreator 2015-06-11T20:46:46
#
#-------------------------------------------------

# File with common stuff for whole
message("Entering vpatterndb.pro")
include(../../../common.pri)

QT += widgets printsupport

# Name of the library
TARGET = vpatterndb

# We want create a library
TEMPLATE = lib

CONFIG += staticlib

include(trmeasurements.pri)
include(vpatterndb.pri)

# This is static library so no need in "make install"

# directory for executable file
DESTDIR = bin

# files created moc
MOC_DIR = moc

# objecs files
OBJECTS_DIR = obj

include(warnings.pri)

include (../libs.pri)
