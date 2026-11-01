# SPDX-License-Identifier: GPL-3.0-or-later
# Shared by the companion and its renderer tests; the 2D exporter stays CPU-only.
lessThan(QT_MAJOR_VERSION, 6): error("Seamly3D requires Qt 6 with Qt Quick 3D")
QT += quickwidgets quick quick3d qml
SOURCES += $$PWD/scenegeometry.cpp
HEADERS += $$PWD/scenegeometry.h $$PWD/viewport.h $$PWD/atelier.h
RESOURCES += $$PWD/seamly3d.qrc
RCC_DIR = rcc
