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

# Use out-of-source builds (shadow builds)
CONFIG -= debug_and_release debug_and_release_target

CONFIG += staticlib

# Since Q5.4 available support C++14
greaterThan(QT_MAJOR_VERSION, 4):greaterThan(QT_MINOR_VERSION, 3) {
    CONFIG += c++14
} else {
    # We use C++11 standard
    CONFIG += c++11
}

# Since Qt 5.4.0 the source code location is recorded only in debug builds.
# We need this information also in release builds. For this need define QT_MESSAGELOGCONTEXT.
DEFINES += QT_MESSAGELOGCONTEXT

# directory for executable file
DESTDIR = bin

# files created moc
MOC_DIR = moc

# objecs files
OBJECTS_DIR = obj

include(qmuparser.pri)

include(warnings.pri)

include (../libs.pri)
