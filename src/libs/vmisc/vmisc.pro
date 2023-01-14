#-------------------------------------------------
#
# Project created by QtCreator 2015-06-15T14:07:14
#
#-------------------------------------------------

# File with common stuff for whole
message("Entering vmisc.pro")
include(../../../common.pri)

QT += widgets printsupport testlib gui multimedia

# Name of library
TARGET = vmisc

# We want to create a library
TEMPLATE = lib

CONFIG += staticlib # Making static library

# Since Q5.4 available support C++14
greaterThan(QT_MAJOR_VERSION, 4):greaterThan(QT_MINOR_VERSION, 3) {
    CONFIG += c++14
} else {
    # We use C++11 standard
    CONFIG += c++11
}

# Use out-of-source builds (shadow builds)
CONFIG -= debug_and_release debug_and_release_target

# Since Qt 5.4.0 the source code location is recorded only in debug builds.
# We need this information also in release builds. For this need define QT_MESSAGELOGCONTEXT.
DEFINES += QT_MESSAGELOGCONTEXT

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
