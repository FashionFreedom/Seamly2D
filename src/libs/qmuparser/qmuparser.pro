#-------------------------------------------------
#
# Project created by QtCreator 2014-04-25T12:01:49
#
#-------------------------------------------------

# File with common stuff for whole project
message("Entering qmuparser.pro")
include(../../../common.pri)

# We don't need gui library.
QT       -= gui

# Name of library
TARGET = qmuparser

# We want create a library
TEMPLATE = lib

CONFIG += staticlib

# directory for executable file
DESTDIR = bin

# files created moc
MOC_DIR = moc

# objecs files
OBJECTS_DIR = obj

include(qmuparser.pri)

include(warnings.pri)

include (../libs.pri)
