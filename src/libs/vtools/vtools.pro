#-------------------------------------------------
#
# Project created by QtCreator 2015-06-16T18:05:56
#
#-------------------------------------------------

# File with common stuff for whole project
message("Entering vtools.pro")
include(../../../common.pri)

QT       += widgets xml printsupport multimedia

# Name of the library
TARGET = vtools

# We want create library
TEMPLATE = lib

CONFIG += staticlib

include(vtools.pri)

# This is static library so no need in "make install"

# directory for executable file
DESTDIR = bin

# files created moc
MOC_DIR = moc

# objecs files
OBJECTS_DIR = obj

# Directory for files created rcc
#RCC_DIR = rcc

# Directory for files created uic
UI_DIR = uic

INCLUDEPATH += $$PWD/../vpatterndb

include(warnings.pri)

include (../libs.pri)
