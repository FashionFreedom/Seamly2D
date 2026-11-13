# SPDX-License-Identifier: GPL-3.0-or-later

# Exercise the real application, including MainWindow's update connections.
# Keep the test executable's objects separate from the production executable.
include(../../app/seamly2d/seamly2d.pro)

APP_SOURCE_DIR = $$clean_path($$PWD/../../app/seamly2d)
QT += testlib
CONFIG += testcase no_testcase_installs
CONFIG -= app_bundle
TARGET = PropertyBrowserTest
SOURCES -= $$APP_SOURCE_DIR/main.cpp
SOURCES += $$PWD/tst_propertybrowser.cpp
INCLUDEPATH += $$APP_SOURCE_DIR
DEFINES += SRCDIR=\\\"$$PWD/\\\"

# The application .pri files already use absolute source and form paths.
# Resolve its remaining resource paths relative to the application as well.
for(resource, RESOURCES) {
    TEST_RESOURCES += $$absolute_path($$resource, $$APP_SOURCE_DIR)
}
RESOURCES = $$TEST_RESOURCES

# A test executable is neither installed nor packaged as the application.
INSTALLS =
QMAKE_BUNDLE_DATA =
QMAKE_POST_LINK =
RC_FILE =
ICON =

DISTFILES += share/line.sm2d README.md
