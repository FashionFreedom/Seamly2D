# ADD TO EACH PATH $$PWD VARIABLE!!!!!!
# This need for corect working file translations.pro

SOURCES  += \
    $$PWD/main.cpp \
    $$PWD/tmainwindow.cpp \
    $$PWD/mapplication.cpp \
    $$PWD/dialogs/dialogaboutseamlyme.cpp \
    $$PWD/dialogs/dialognewmeasurements.cpp \
    $$PWD/dialogs/dialogmdatabase.cpp \
    $$PWD/vlitepattern.cpp \
    $$PWD/dialogs/dialogseamlymepreferences.cpp \
    $$PWD/dialogs/configpages/seamlymepreferencesconfigurationpage.cpp \
    $$PWD/dialogs/configpages/seamlymepreferencespathpage.cpp

*msvc*:SOURCES += $$PWD/stable.cpp

HEADERS  += \
    $$PWD/tmainwindow.h \
    $$PWD/stable.h \
    $$PWD/mapplication.h \
    $$PWD/dialogs/dialogaboutseamlyme.h \
    $$PWD/dialogs/dialognewmeasurements.h \
    $$PWD/dialogs/dialogmdatabase.h \
    $$PWD/version.h \
    $$PWD/vlitepattern.h \
    $$PWD/dialogs/dialogseamlymepreferences.h \
    $$PWD/dialogs/configpages/seamlymepreferencesconfigurationpage.h \
    $$PWD/dialogs/configpages/seamlymepreferencespathpage.h

FORMS    += \
    $$PWD/tmainwindow.ui \
    $$PWD/dialogs/dialogaboutseamlyme.ui \
    $$PWD/dialogs/dialognewmeasurements.ui \
    $$PWD/dialogs/dialogmdatabase.ui \
    $$PWD/dialogs/dialogseamlymepreferences.ui \
    $$PWD/dialogs/configpages/seamlymepreferencesconfigurationpage.ui \
    $$PWD/dialogs/configpages/seamlymepreferencespathpage.ui
