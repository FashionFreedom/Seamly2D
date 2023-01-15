#-------------------------------------------------
#
# Project created by QtCreator 2015-05-07T13:42:04
#
#-------------------------------------------------

# File with common stuff for whole project
message("Entering vgeometry.pro")
include(../../../common.pri)

QT += widgets printsupport

# Name of library
TARGET = vgeometry

# We want create a library
TEMPLATE = lib

CONFIG += staticlib

include(vgeometry.pri)

# This is static library so no need in "make install"

# directory for executable file
DESTDIR = bin

# files created moc
MOC_DIR = moc

# objecs files
OBJECTS_DIR = obj

include(warnings.pri)

include (../libs.pri)
