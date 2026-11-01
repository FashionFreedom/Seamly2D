# Support subdirectories. Just better project code tree.
include(dialogs/dialogs.pri)
include(xml/xml.pri)
include(core/core.pri)

# ADD TO EACH PATH $$PWD VARIABLE!!!!!!
# This need for correct working file translations.pro

# Some source files
SOURCES += \
        $$PWD/main.cpp \
        $$PWD/mainwindow.cpp \
        $$PWD/mainwindowsnogui.cpp

*msvc*:SOURCES += $$PWD/stable.cpp

# Some header files
HEADERS  += \
        $$PWD/mainwindow.h \
        $$PWD/options.h \
        $$PWD/stable.h \
        $$PWD/version.h \
        $$PWD/mainwindowsnogui.h
# Main forms
FORMS    += \
        $$PWD/mainwindow.ui

# Shared, dependency-free assembly exchange model.
SOURCES += $$PWD/seamly3d_export.cpp $$PWD/../seamly3d/model.cpp
HEADERS += $$PWD/../seamly3d/model.h

SOURCES += $$PWD/../seamly3d/dressform.cpp
HEADERS += $$PWD/../seamly3d/dressform.h

SOURCES += $$PWD/../seamly3d/clothsolver.cpp
HEADERS += $$PWD/../seamly3d/clothsolver.h
