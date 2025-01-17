# ADD TO EACH PATH $$PWD VARIABLE!!!!!!
# This need for corect working file translations.pro

SOURCES += \
    $$PWD/measurements.cpp \
    $$PWD/vlabeltemplate.cpp

*msvc*:SOURCES += $$PWD/stable.cpp

HEADERS += \
    $$PWD/measurements.h \
    $$PWD/stable.h \
    $$PWD/vlabeltemplate.h
