#-------------------------------------------------
#
# Project created by QtCreator 2015-06-15T14:07:14
#
#-------------------------------------------------

# File with common stuff for whole
message("Entering vmisc.pro")
include(../../../common.pri)

QT += widgets printsupport testlib gui

# Name of library
TARGET = vmisc

# We want to create a library
TEMPLATE = lib

CONFIG += staticlib

include(vmisc.pri)

# Resource files. This files will be included in binary.
RESOURCES += \
    share/resources/sounds.qrc \
    share/resources/theme.qrc \ # Windows theme icons.
    share/resources/icon.qrc \ # All other icons except cursors and Windows theme.
    share/resources/flags.qrc

# This is static library so no need in "make install"

# directory for executable file
DESTDIR = bin

# files created moc
MOC_DIR = moc

# objecs files
OBJECTS_DIR = obj

include(warnings.pri)

include (../libs.pri)
