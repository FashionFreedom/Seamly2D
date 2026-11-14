# ADD TO EACH PATH $$PWD VARIABLE!!!!!!
# This need for correct working file translations.pro

HEADERS += \
    $$PWD/application_2d.h \
    $$PWD/command_line.h \
    $$PWD/vformulaproperty.h \
    $$PWD/vformulapropertyeditor.h \
    $$PWD/vtooloptionspropertybrowser.h

SOURCES += \
    $$PWD/application_2d.cpp \
    $$PWD/command_line.cpp \
    $$PWD/vformulaproperty.cpp \
    $$PWD/vformulapropertyeditor.cpp \
    $$PWD/vtooloptionspropertybrowser.cpp

macx{
    HEADERS += $$PWD/macos_dock.h
    OBJECTIVE_SOURCES += $$PWD/macos_dock.mm
    LIBS += -framework AppKit
}
