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

CONFIG += \
    staticlib \# Making static library
    c++11 # We use C++11 standard

# Use out-of-source builds (shadow builds)
CONFIG -= debug_and_release debug_and_release_target

# Since Qt 5.4.0 the source code location is recorded only in debug builds.
# We need this information also in release builds. For this need define QT_MESSAGELOGCONTEXT.
DEFINES += QT_MESSAGELOGCONTEXT

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
