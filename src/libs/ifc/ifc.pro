#-------------------------------------------------
#
# Project created by QtCreator 2014-12-10T17:53:37
#
#-------------------------------------------------

# File with common stuff for whole project
message("Entering ifc.pro")

include(../../../common.pri)

# Library work with xml.
QT       += xml xmlpatterns printsupport

# We don't need gui library.
QT       -= gui

# Name of library
TARGET = ifc # Internal Format Converter

# We want create library
TEMPLATE = lib

CONFIG += staticlib

include(ifc.pri)

# This is static library so no need in "make install"

# directory for executable file
DESTDIR = bin

# files created moc
MOC_DIR = moc

# objecs files
OBJECTS_DIR = obj

# Directory for files created rcc
RCC_DIR = rcc

# Resource files. This files will be included in binary.
RESOURCES += \
    schema.qrc  # Schemas for validation xml files.

include(warnings.pri)

include (../libs.pri)
