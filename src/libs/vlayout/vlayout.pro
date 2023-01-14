#-------------------------------------------------
#
# Project created by QtCreator 2015-01-02T15:39:01
#
#-------------------------------------------------

# File with common stuff for whole
message("Entering vlayout.pro")
include(../../../common.pri)

QT += core gui widgets printsupport xml

# Name of library
TARGET = vlayout

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

include(vlayout.pri)

# This is static library so no need in "make install"

# directory for executable file
DESTDIR = bin

# files created moc
MOC_DIR = moc

# objecs files
OBJECTS_DIR = obj

# Directory for files created rcc
RCC_DIR = rcc

include(warnings.pri)

CONFIG(release, debug|release){
    !*msvc* {
        QMAKE_CXXFLAGS -= -O2 # Disable default optimization level
        QMAKE_CXXFLAGS += -O3 # For vlayout library enable speed optimizations
    }
}

RESOURCES += \
    share/icons.qrc

include (../libs.pri)
