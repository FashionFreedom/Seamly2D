# ADD TO EACH PATH $$PWD VARIABLE!!!!!!
# This need for correct working file translations.pro

HEADERS += \
    $$PWD/image_dialog.h \
    $$PWD/stable.h \
    $$PWD/image_item.h

SOURCES += \
    $$PWD/image_dialog.cpp \
    $$PWD/image_item.cpp

FORMS += \
    $$PWD/image_dialog.ui

*msvc*:SOURCES += $$PWD/stable.cpp
