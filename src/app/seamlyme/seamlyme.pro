#-------------------------------------------------
#
# Project created by QtCreator 2015-07-07T14:35:12
#
#-------------------------------------------------
message("Entering seamlyme.pro")

# File with common stuff for whole project
include(../../../common.pri)

QT       += core gui widgets network xml printsupport svg core5compat

# Name of binary file
TARGET = seamlyme

# We want create executable file
TEMPLATE = app

# Directory for executable file
DESTDIR = bin

# Directory for files created moc
MOC_DIR = moc

# Directory for objects files
OBJECTS_DIR = obj

# Directory for files created rcc
RCC_DIR = rcc

# Directory for files created uic
UI_DIR = uic

# Support subdirectories. Just better project code tree.
include(seamlyme.pri)

RESOURCES += \
    share/resources/seamlymeicon.qrc \
    share/resources/diagrams.qrc

# INSTALL_MULTISIZE_MEASUREMENTS and INSTALL_STANDARD_TEMPLATES and INSTALL_LABEL_TEMPLATES inside samples.pri
include(../samples.pri)

include(../translations.pri)

# Set "make install" command for Unix-like systems.
unix{
    # Prefix for binary file.
    isEmpty(PREFIX){
        PREFIX = $$DEFAULT_PREFIX
    }

    unix:!macx{
        QMAKE_RPATHDIR += $$[QT_INSTALL_LIBS]
        DATADIR =$$PREFIX/share
        DEFINES += DATADIR=\\\"$$DATADIR\\\" PKGDATADIR=\\\"$$PKGDATADIR\\\"

        # Path to bin file after installation
        target.path = $$PREFIX/bin

        INSTALLS += \
            target
    }
    macx{
        message(seamlyme.pro: mac deployment target: $$QMAKE_MACOSX_DEPLOYMENT_TARGET)

        # Path to resources in app bundle
        FRAMEWORKS_DIR = "Contents/Frameworks"
        MACOS_DIR = "Contents/MacOS"
        RESOURCES_DIR = "Contents/Resources"
        # On macx we will use app bundle. Bundle doesn't need bin directory inside.
        # See issue #166: Creating OSX Homebrew (Mac OS X package manager) formula.
        target.path = $$MACOS_DIR

        #languages added inside translations.pri

        # logo on macx.
        ICON = $${PWD}/../../../dist/seamlyme.icns

        QMAKE_INFO_PLIST = $${PWD}/../../../dist/macx/seamlyme/info.plist

        # Copy to bundle multisize measurements files
        multisize.path = $${RESOURCES_DIR}/tables/multisize
        multisize.files = $$INSTALL_MULTISIZE_MEASUREMENTS

        # Copy to bundle templates files
        templates.path = $${RESOURCES_DIR}/tables/templates
        templates.files = $$INSTALL_STANDARD_TEMPLATES

        format.path = $${RESOURCES_DIR}
        format.files += $$PWD/../../../dist/macx/individual_size_file.icns
        format.files += $$PWD/../../../dist/macx/multi_size_file.icns

        QMAKE_BUNDLE_DATA += \
            templates \
            multisize \
            format
    }
}

win32 {
    for(DIR, INSTALL_OPENSSL) {
        #add these absolute paths to a variable which
        #ends up as 'mkcommands = path1 path2 path3 ...'
        openssl_path += $${PWD}/$$DIR
    }
    copyToDestdir($$openssl_path, $$shell_path($${OUT_PWD}/$$DESTDIR))
}

# Compilation will fail without this files after we added them to this section.
OTHER_FILES += \
    share/resources/seamlymeicon/64x64/logo.ico # SeamlyMe's logo.

include(warnings.pri)

# precompiled headers clash with the BUILD_REVISION define, thus disable here
CONFIG -= precompile_header

DVCS_HESH=$$FindBuildRevision()
message("seamlyme.pro: Build revision:" $${DVCS_HESH})
DEFINES += "BUILD_REVISION=$${DVCS_HESH}" # Make available build revision number in sources.

# Path to resource file.
win32:RC_FILE = share/resources/seamlyme.rc

# When the GNU linker sees a library, it discards all symbols that it doesn't need.
# Dependent library go first.

#VTools static library (depend on VWidgets, VMisc, VPatternDB)
unix|win32: LIBS += -L$$OUT_PWD/../../libs/vtools/$${DESTDIR}/ -lvtools

INCLUDEPATH += $$PWD/../../libs/vtools
INCLUDEPATH += $$OUT_PWD/../../libs/vtools/$${UI_DIR} # For UI files
DEPENDPATH += $$PWD/../../libs/vtools

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vtools/$${DESTDIR}/vtools.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vtools/$${DESTDIR}/libvtools.a

#VWidgets static library
unix|win32: LIBS += -L$$OUT_PWD/../../libs/vwidgets/$${DESTDIR}/ -lvwidgets

INCLUDEPATH += $$PWD/../../libs/vwidgets
DEPENDPATH += $$PWD/../../libs/vwidgets

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vwidgets/$${DESTDIR}/vwidgets.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vwidgets/$${DESTDIR}/libvwidgets.a

# VFormat static library (depend on VPatternDB, IFC)
unix|win32: LIBS += -L$$OUT_PWD/../../libs/vformat/$${DESTDIR}/ -lvformat

INCLUDEPATH += $$PWD/../../libs/vformat
DEPENDPATH += $$PWD/../../libs/vformat

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vformat/$${DESTDIR}/vformat.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vformat/$${DESTDIR}/libvformat.a

#VPatternDB static library (depend on vgeometry, vmisc, VLayout)
unix|win32: LIBS += -L$$OUT_PWD/../../libs/vpatterndb/$${DESTDIR} -lvpatterndb

INCLUDEPATH += $$PWD/../../libs/vpatterndb
DEPENDPATH += $$PWD/../../libs/vpatterndb

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vpatterndb/$${DESTDIR}/vpatterndb.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vpatterndb/$${DESTDIR}/libvpatterndb.a

# Fervor static library (depend on VMisc, IFC)
unix|win32: LIBS += -L$$OUT_PWD/../../libs/fervor/$${DESTDIR}/ -lfervor

INCLUDEPATH += $$PWD/../../libs/fervor
DEPENDPATH += $$PWD/../../libs/fervor

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/fervor/$${DESTDIR}/fervor.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/fervor/$${DESTDIR}/libfervor.a

# IFC static library (depend on QMuParser, VMisc)
unix|win32: LIBS += -L$$OUT_PWD/../../libs/ifc/$${DESTDIR}/ -lifc

INCLUDEPATH += $$PWD/../../libs/ifc
DEPENDPATH += $$PWD/../../libs/ifc

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/ifc/$${DESTDIR}/ifc.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/ifc/$${DESTDIR}/libifc.a

#VMisc static library
unix|win32: LIBS += -L$$OUT_PWD/../../libs/vmisc/$${DESTDIR}/ -lvmisc

INCLUDEPATH += $$PWD/../../libs/vmisc
DEPENDPATH += $$PWD/../../libs/vmisc

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vmisc/$${DESTDIR}/vmisc.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vmisc/$${DESTDIR}/libvmisc.a

# VGeometry static library (depend on ifc)
unix|win32: LIBS += -L$$OUT_PWD/../../libs/vgeometry/$${DESTDIR}/ -lvgeometry

INCLUDEPATH += $$PWD/../../libs/vgeometry
DEPENDPATH += $$PWD/../../libs/vgeometry

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vgeometry/$${DESTDIR}/vgeometry.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vgeometry/$${DESTDIR}/libvgeometry.a

# VLayout static library
unix|win32: LIBS += -L$$OUT_PWD/../../libs/vlayout/$${DESTDIR}/ -lvlayout

INCLUDEPATH += $$PWD/../../libs/vlayout
DEPENDPATH += $$PWD/../../libs/vlayout

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vlayout/$${DESTDIR}/vlayout.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vlayout/$${DESTDIR}/libvlayout.a

# QMuParser library
unix|win32: LIBS += -L$${OUT_PWD}/../../libs/qmuparser/$${DESTDIR} -lqmuparser

INCLUDEPATH += $${PWD}/../../libs/qmuparser
DEPENDPATH += $${PWD}/../../libs/qmuparser

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/qmuparser/$${DESTDIR}/qmuparser.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/qmuparser/$${DESTDIR}/libqmuparser.a

# VPropertyExplorer library
unix|win32: LIBS += -L$${OUT_PWD}/../../libs/vpropertyexplorer/$${DESTDIR} -lvpropertyexplorer

INCLUDEPATH += $${PWD}/../../libs/vpropertyexplorer
DEPENDPATH += $${PWD}/../../libs/vpropertyexplorer

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vpropertyexplorer/$${DESTDIR}/vpropertyexplorer.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vpropertyexplorer/$${DESTDIR}/libvpropertyexplorer.a

# xerces library
macx: LIBS += -L$${PWD}/../../libs/xerces-c/macx/lib -lxerces-c
else:unix: LIBS += -lxerces-c
win32:!win32-g++: LIBS += -L$${PWD}/../../libs/xerces-c/msvc/lib -lxerces-c_3
win32-g++: LIBS += -L$${PWD}/../../libs/xerces-c/mingw/lib -lxerces-c

macx{
    APPLE_SIGN_IDENTITY = $$shell_quote($(APPLE_SIGN_IDENTITY))

    QMAKE_POST_LINK += $$[QT_INSTALL_BINS]/macdeployqt $${OUT_PWD}/$${DESTDIR}/$${TARGET}.app

    macSign {
        QMAKE_POST_LINK += && codesign --deep --timestamp --options runtime -s $${APPLE_SIGN_IDENTITY} $${OUT_PWD}/$${DESTDIR}/$${TARGET}.app
        QMAKE_POST_LINK += && codesign --verify $${OUT_PWD}/$${DESTDIR}/$${TARGET}.app
    }
}

win32{
    # run windeployqt to include all qt libraries and vc_redist in $${DESTDIR}
    QMAKE_POST_LINK += windeployqt $$shell_path($$DESTDIR/$${TARGET}.exe)
}
