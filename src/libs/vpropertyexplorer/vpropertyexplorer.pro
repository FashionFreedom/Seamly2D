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

include(vpropertyexplorer.pri)

include(warnings.pri)

include (../libs.pri)
