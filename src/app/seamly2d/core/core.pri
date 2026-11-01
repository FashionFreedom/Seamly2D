# ADD TO EACH PATH $$PWD VARIABLE!!!!!!
# This need for correct working file translations.pro

HEADERS += \
    $$PWD/mcp_bridge.h \
    $$PWD/mcp_measurements.h \
    $$PWD/application_2d.h \
    $$PWD/vformulaproperty.h \
    $$PWD/vformulapropertyeditor.h \
    $$PWD/vtooloptionspropertybrowser.h \
    $$PWD/vcmdexport.h

SOURCES += \
    $$PWD/mcp_bridge.cpp \
    $$PWD/mcp_measurements.cpp \
    $$PWD/mcp_toolbar.cpp \
    $$PWD/application_2d.cpp \
    $$PWD/vformulaproperty.cpp \
    $$PWD/vformulapropertyeditor.cpp \
    $$PWD/vtooloptionspropertybrowser.cpp \
    $$PWD/vcmdexport.cpp

macx{
    HEADERS += $$PWD/macos_dock.h
    OBJECTIVE_SOURCES += $$PWD/macos_dock.mm
    LIBS += -framework AppKit
}
