#-------------------------------------------------
#
# Project created by QtCreator 2015-06-15T15:24:19
#
#-------------------------------------------------

# File with common stuff for whole project
message("Entering vwidgets.pro")
include(../../../common.pri)

QT += gui widgets xml printsupport

# Name of the library
TARGET = vwidgets

# We want create a library
TEMPLATE = lib

CONFIG += staticlib

include(vwidgets.pri)

# This is static library so no need in "make install"

# directory for executable file
DESTDIR = bin

# files created moc
MOC_DIR = moc

# objecs files
OBJECTS_DIR = obj

include(warnings.pri)

include (../libs.pri)

HEADERS +=

SOURCES +=
