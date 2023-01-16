#-------------------------------------------------
#
# Project created by QtCreator 2014-08-26T14:18:08
#
#-------------------------------------------------

# File with common stuff for whole project
message("Entering vpropertyexplorer.pro")
include(../../../common.pri)

# Library use widgets
QT       += core widgets

# We don't need gui library.
QT       -= gui

# Name of library
TARGET = vpropertyexplorer

# We want create library
TEMPLATE = lib

CONFIG += staticlib

# directory for executable file
DESTDIR = bin

# files created moc
MOC_DIR = moc

# objecs files
OBJECTS_DIR = obj

include(vpropertyexplorer.pri)

include(warnings.pri)

include (../libs.pri)
