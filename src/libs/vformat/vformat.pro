#-------------------------------------------------
#
# Project created by QtCreator 2015-07-14T20:08:14
#
#-------------------------------------------------

# File with common stuff for whole project
message("Entering vformat.pro")
include(../../../common.pri)

# Library work with xml.
QT       += xml xmlpatterns printsupport svg

# We don't need gui library.
QT       -= gui

# Name of the library
TARGET = vformat

# We want create a library
TEMPLATE = lib

CONFIG += staticlib

include(vformat.pri)

# This is static library so no need in "make install"

# directory for executable file
DESTDIR = bin

# files created moc
MOC_DIR = moc

# objecs files
OBJECTS_DIR = obj

include(warnings.pri)

include (../libs.pri)
