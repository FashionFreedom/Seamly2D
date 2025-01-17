# ADD TO EACH PATH $$PWD VARIABLE!!!!!!
# This need for correct working file translations.pro

SOURCES  += \
    $$PWD/dialogs/me_shortcuts_dialog.cpp \
    $$PWD/dialogs/me_welcome_dialog.cpp \
    $$PWD/dialogs/new_measurements_dialog.cpp \
    $$PWD/main.cpp \
    $$PWD/tmainwindow.cpp \
    $$PWD/mapplication.cpp \
    $$PWD/dialogs/dialogaboutseamlyme.cpp \
    $$PWD/dialogs/dialogmdatabase.cpp \
    $$PWD/vlitepattern.cpp \
    $$PWD/dialogs/dialogseamlymepreferences.cpp \
    $$PWD/dialogs/configpages/seamlymepreferencesconfigurationpage.cpp \
    $$PWD/dialogs/configpages/seamlymepreferencespathpage.cpp

*msvc*:SOURCES += $$PWD/stable.cpp

HEADERS  += \
    $$PWD/dialogs/me_shortcuts_dialog.h \
    $$PWD/dialogs/me_welcome_dialog.h \
    $$PWD/dialogs/new_measurements_dialog.h \
    $$PWD/tmainwindow.h \
    $$PWD/stable.h \
    $$PWD/mapplication.h \
    $$PWD/dialogs/dialogaboutseamlyme.h \
    $$PWD/dialogs/dialogmdatabase.h \
    $$PWD/version.h \
    $$PWD/vlitepattern.h \
    $$PWD/dialogs/dialogseamlymepreferences.h \
    $$PWD/dialogs/configpages/seamlymepreferencesconfigurationpage.h \
    $$PWD/dialogs/configpages/seamlymepreferencespathpage.h

FORMS    += \
    $$PWD/dialogs/me_shortcuts_dialog.ui \
    $$PWD/dialogs/me_welcome_dialog.ui \
    $$PWD/dialogs/new_measurements_dialog.ui \
    $$PWD/tmainwindow.ui \
    $$PWD/dialogs/dialogaboutseamlyme.ui \
    $$PWD/dialogs/dialogmdatabase.ui \
    $$PWD/dialogs/dialogseamlymepreferences.ui \
    $$PWD/dialogs/configpages/seamlymepreferencesconfigurationpage.ui \
    $$PWD/dialogs/configpages/seamlymepreferencespathpage.ui
