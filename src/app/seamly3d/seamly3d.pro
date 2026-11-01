# SPDX-License-Identifier: GPL-3.0-or-later
include(../../../common.pri)
TEMPLATE = app
TARGET = seamly3d
QT = core gui widgets
CONFIG -= precompile_header
DESTDIR = bin
OBJECTS_DIR = obj
MOC_DIR = moc
SOURCES += main.cpp model.cpp viewport.cpp window.cpp
HEADERS += model.h window.h
unix:!macx {
    isEmpty(PREFIX): PREFIX = $$DEFAULT_PREFIX
    target.path = $$PREFIX/bin
    INSTALLS += target
}

include(renderer.pri)

# Deploy QML imports as well as linked frameworks for the standalone app.
macx {
    qt_configuration.path = Contents/Resources
    qt_configuration.files = $$PWD/../../../dist/macx/seamly2d/qt.conf
    QMAKE_BUNDLE_DATA += qt_configuration
    PRE_TARGETDEPS += $$PWD/../../../dist/macx/seamly2d/qt.conf
    QMAKE_POST_LINK += DYLD_FRAMEWORK_PATH=$$shell_quote($$[QT_INSTALL_LIBS]) $$[QT_INSTALL_BINS]/macdeployqt $${OUT_PWD}/$${DESTDIR}/$${TARGET}.app -libpath=$$[QT_INSTALL_LIBS] -qmldir=$$PWD && touch $$shell_quote($${OUT_PWD}/$${DESTDIR}/$${TARGET}.app)
}
win32-msvc {
    QMAKE_POST_LINK += windeployqt --qmldir $$shell_path($$PWD) $$shell_path($$DESTDIR/$${TARGET}.exe)
}
win32-arm64-msvc {
    qtPrepareTool(WINDEPLOYQT, windeployqt)
    QMAKE_POST_LINK += $$WINDEPLOYQT --qtpaths $$shell_path($$[QT_INSTALL_BINS]/qtpaths.bat) --qmldir $$shell_path($$PWD) $$shell_path($$DESTDIR/$${TARGET}.exe)
}

SOURCES += dressform.cpp
HEADERS += dressform.h

SOURCES += clothsolver.cpp
HEADERS += clothsolver.h
