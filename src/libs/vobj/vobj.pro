#-------------------------------------------------
#
# Project created by QtCreator 2014-12-12T14:55:06
#
#-------------------------------------------------

# File with common stuff for whole project
message("Entering vobj.pro")
include(../../../common.pri)

# Name of library
TARGET = vobj

# We want create a library
TEMPLATE = lib

CONFIG += staticlib

include(vobj.pri)

# This is static library so no need in "make install"

# directory for executable file
DESTDIR = bin

# files created moc
MOC_DIR = moc

# objecs files
OBJECTS_DIR = obj

include(warnings.pri)

include (../libs.pri)
