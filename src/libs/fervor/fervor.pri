# ADD TO EACH PATH $$PWD VARIABLE!!!!!!
# This need for corect working file translations.pro

SOURCES += \
    $$PWD/fvavailableupdate.cpp \
    $$PWD/fvignoredversions.cpp \
    $$PWD/fvupdater.cpp \
    $$PWD/fvversioncomparator.cpp \
    $$PWD/updater_dilaog.cpp

*msvc*:SOURCES += $$PWD/stable.cpp

HEADERS += \
    $$PWD/fvavailableupdate.h \
    $$PWD/fvignoredversions.h \
    $$PWD/fvupdater.h \
    $$PWD/fvversioncomparator.h \
    $$PWD/stable.h \
    $$PWD/updater_dialog.h

FORMS += \
    $$PWD/updater_dialog.ui
