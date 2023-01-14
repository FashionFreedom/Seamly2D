#-------------------------------------------------
#
# Project created by QtCreator 2015-07-14T20:08:14
#
#-------------------------------------------------

# File with common stuff for whole project
message("Entering vformat.pro")
include(../../../common.pri)

# Library work with xml.
QT       += xml xmlpatterns printsupport

# We don't need gui library.
QT       -= gui

# Name of the library
TARGET = vformat

# We want create a library
TEMPLATE = lib

CONFIG += \
    staticlib \# Making static library
    c++11 # We use C++11 standard

# Use out-of-source builds (shadow builds)
CONFIG -= debug_and_release debug_and_release_target

# Since Qt 5.4.0 the source code location is recorded only in debug builds.
# We need this information also in release builds. For this need define QT_MESSAGELOGCONTEXT.
DEFINES += QT_MESSAGELOGCONTEXT

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
