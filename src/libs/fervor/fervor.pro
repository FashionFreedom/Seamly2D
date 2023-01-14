#-------------------------------------------------
#
# Project created by QtCreator 2016-07-14T11:55:03
#
#-------------------------------------------------

# File with common stuff for whole project
message("Entering fervor.pro")

include(../../../common.pri)

QT       += network widgets xml printsupport

# Name of library
TARGET = fervor

# We want to create a library
TEMPLATE = lib

CONFIG += \
    staticlib \# Making static library
    c++11 # We use C++11 standard

# Use out-of-source builds (shadow builds)
CONFIG -= debug_and_release debug_and_release_target

# Since Qt 5.4.0 the source code location is recorded only in debug builds.
# We need this information also in release builds. For this need define QT_MESSAGELOGCONTEXT.
DEFINES += QT_MESSAGELOGCONTEXT

include(fervor.pri)

# This is static library so no need in "make install"

# directory for executable file
DESTDIR = bin

# files created moc
MOC_DIR = moc

# objecs files
OBJECTS_DIR = obj

include(warnings.pri)

include (../libs.pri)
