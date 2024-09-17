#-------------------------------------------------
#
# Project created by QtCreator 2013-06-18T12:36:43
#
#-------------------------------------------------

# Compilation main binary file
message("Entering seamly2D.pro")

# File with common stuff for whole project
include(../../../common.pri)

# Here we don't see "network" library, but, i think, "printsupport" depend on this library, so we still need this
# library in installer.
QT       += core gui widgets xml svg printsupport xmlpatterns multimedia

# We want create executable file
TEMPLATE = app

# Name of binary file
macx{
    TARGET = Seamly2D
} else {
    TARGET = seamly2d
}

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
include(seamly2d.pri)

# Resource files. This files will be included in binary.
RESOURCES += \
    share/resources/cursor.qrc \ # Tools cursor icons.
    share/resources/toolicon.qrc
    share/resources/sounds.qrc

# Compilation will fail without this files after we added them to this section.
OTHER_FILES += \
    share/resources/icon/64x64/icon64x64.ico # Seamly2D's logo.

include(warnings.pri)

# precompiled headers clash with the BUILD_REVISION define, thus disable here
CONFIG -= precompile_header

DVCS_HESH=$$FindBuildRevision()
message("seamly2d.pro: Build revision:" $${DVCS_HESH})
DEFINES += "BUILD_REVISION=$${DVCS_HESH}" # Make available build revision number in sources.

# Some extra information about Qt. Can be useful.
message(seamly2d.pro: Qt version: $$[QT_VERSION])
message(seamly2d.pro: Qt is installed in $$[QT_INSTALL_PREFIX])
message(seamly2d.pro: Qt resources can be found in the following locations:)
message(seamly2d.pro: Documentation: $$[QT_INSTALL_DOCS])
message(seamly2d.pro: Header files: $$[QT_INSTALL_HEADERS])
message(seamly2d.pro: Libraries: $$[QT_INSTALL_LIBS])
message(seamly2d.pro: Binary files (executables): $$[QT_INSTALL_BINS])
message(seamly2d.pro: Plugins: $$[QT_INSTALL_PLUGINS])
message(seamly2d.pro: Data files: $$[QT_INSTALL_DATA])
message(seamly2d.pro: Translation files: $$[QT_INSTALL_TRANSLATIONS])
message(seamly2d.pro: Settings: $$[QT_INSTALL_SETTINGS])
message(seamly2d.pro: Examples: $$[QT_INSTALL_EXAMPLES])

# Path to resource file.
win32:RC_FILE = share/resources/seamly2d.rc

# INSTALL_MULTISIZE_MEASUREMENTS and INSTALL_STANDARD_TEMPLATES and INSTALL_LABEL_TEMPLATES inside samples.pri
include(../samples.pri)

win32 {
    INSTALL_PDFTOPS += ../../../dist/win/pdftops.exe
}

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

        seamlyme.path = $$PREFIX/bin
        seamlyme.files += $${OUT_PWD}/../seamlyme/$${DESTDIR}/seamlyme

        # .desktop file
        desktop.path = $$PREFIX/share/applications/
        desktop.files += ../../../dist/$${TARGET}.desktop \
        desktop.files += ../../../dist/seamlyme.desktop

        # logo
        pixmaps.path = $$PREFIX/share/pixmaps/
        pixmaps.files += \
            ../../../dist/$${TARGET}.png \
            ../../../dist/seamlyme.png \
            ../../../dist/application-x-seamly2d-2d_file.png \
            ../../../dist/application-x-seamlyme-individual_size_file.png \
            ../../../dist/application-x-seamlyme-multi_size_file.png \

        # Path to multisize measurement after installation
        multisize.path = $$PREFIX/share/$${TARGET}/tables/multisize/
        multisize.files = $$INSTALL_MULTISIZE_MEASUREMENTS

        # Path to templates after installation
        templates.path = $$PREFIX/share/$${TARGET}/tables/templates/
        templates.files = $$INSTALL_STANDARD_TEMPLATES

        # Path to label templates after installation
        label.path = $$PREFIX/share/$${TARGET}/labels/
        label.files = $$INSTALL_LABEL_TEMPLATES

        INSTALLS += \
            target \
            seamlyme \
            desktop \
            pixmaps \
            multisize \
            templates \
            label
    }
    macx{
        message(seamly2d.pro: mac deployment target: $$QMAKE_MACOSX_DEPLOYMENT_TARGET)

        # Path to resources in app bundle
        FRAMEWORKS_DIR = "Contents/Frameworks"
        MACOS_DIR = "Contents/MacOS"
        RESOURCES_DIR = "Contents/Resources"
        # On macx we will use app bundle. Bundle doesn't need bin directory inside.
        # See issue #166: Creating OSX Homebrew (Mac OS X package manager) formula.
        target.path = $$MACOS_DIR

        #languages added inside translations.pri

        seamlyme.path = $$MACOS_DIR
        seamlyme.files += $${OUT_PWD}/../seamlyme/$${DESTDIR}/seamlyme.app/$${MACOS_DIR}/seamlyme

        # Utility pdftops need for saving a layout image to PS and EPS formats.
        xpdf.path = $$MACOS_DIR
        xpdf.files += $${PWD}/../../../dist/macx/bin64/pdftops

        # logo on macx.
        ICON = ../../../dist/seamly2d.icns

        QMAKE_INFO_PLIST = $${PWD}/../../../dist/macx/seamly2d/info.plist

        # Copy to bundle multisize measurements files
        multisize.path = $${RESOURCES_DIR}/tables/multisize
        multisize.files = $$INSTALL_MULTISIZE_MEASUREMENTS

        # Copy to bundle templates files
        templates.path = $${RESOURCES_DIR}/tables/templates
        templates.files = $$INSTALL_STANDARD_TEMPLATES

        # Path to label templates after installation
        label.path = $${RESOURCES_DIR}/labels
        label.files = $$INSTALL_LABEL_TEMPLATES

        icns_resources.path = $${RESOURCES_DIR}
        icns_resources.files += $$PWD/../../../dist/macx/individual_size_file.icns
        icns_resources.files += $$PWD/../../../dist/macx/multi_size_file.icns
        icns_resources.files += $$PWD/../../../dist/macx/2d_file.icns

        QMAKE_BUNDLE_DATA += \
            templates \
            multisize \
            label \
            seamlyme \
            xpdf \
            icns_resources
    }
}

win32 {
    for(DIR, INSTALL_PDFTOPS) {
        #add these absolute paths to a variable which
        #ends up as 'mkcommands = path1 path2 path3 ...'
        pdftops_path += $${PWD}/$$DIR
    }
    copyToDestdir($$pdftops_path, $$shell_path($${OUT_PWD}/$$DESTDIR))

    for(DIR, INSTALL_OPENSSL) {
        #add these absolute paths to a variable which
        #ends up as 'mkcommands = path1 path2 path3 ...'
        openssl_path += $${PWD}/$$DIR
    }
    copyToDestdir($$openssl_path, $$shell_path($${OUT_PWD}/$$DESTDIR))
}

# When the GNU linker sees a library, it discards all symbols that it doesn't need.
# Dependent library go first.

#Tools static library (depend on VWidgets, VMisc, VPatternDB)
unix|win32: LIBS += -L$$OUT_PWD/../../libs/tools/$${DESTDIR}/ -ltools

INCLUDEPATH += $$PWD/../../libs/tools
INCLUDEPATH += $$OUT_PWD/../../libs/tools/$${UI_DIR} # For UI files
DEPENDPATH += $$PWD/../../libs/tools

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/tools/$${DESTDIR}/tools.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/tools/$${DESTDIR}/libtools.a

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

# VGeometry static library (depend on ifc)
unix|win32: LIBS += -L$$OUT_PWD/../../libs/vgeometry/$${DESTDIR}/ -lvgeometry

INCLUDEPATH += $$PWD/../../libs/vgeometry
DEPENDPATH += $$PWD/../../libs/vgeometry

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vgeometry/$${DESTDIR}/vgeometry.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vgeometry/$${DESTDIR}/libvgeometry.a

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

# VObj static library
unix|win32: LIBS += -L$$OUT_PWD/../../libs/vobj/$${DESTDIR}/ -lvobj

INCLUDEPATH += $$PWD/../../libs/vobj
DEPENDPATH += $$PWD/../../libs/vobj

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vobj/$${DESTDIR}/vobj.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vobj/$${DESTDIR}/libvobj.a

# VDxf static library
unix|win32: LIBS += -L$$OUT_PWD/../../libs/vdxf/$${DESTDIR}/ -lvdxf

INCLUDEPATH += $$PWD/../../libs/vdxf
DEPENDPATH += $$PWD/../../libs/vdxf

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vdxf/$${DESTDIR}/vdxf.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../libs/vdxf/$${DESTDIR}/libvdxf.a

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

macx{
    APPLE_SIGN_IDENTITY_UNQUOTED = $(APPLE_SIGN_IDENTITY)
    APPLE_SIGN_IDENTITY = $$shell_quote($(APPLE_SIGN_IDENTITY))

    !macSign {
        # run macdeployqt to include all qt libraries in packet
        QMAKE_POST_LINK += $$[QT_INSTALL_BINS]/macdeployqt $${OUT_PWD}/$${DESTDIR}/$${TARGET}.app
    } else {
        # we need to manually sign with codesign --deep as pdftops otherwise will not get signed by macdeployqt
        # we need --force as seamlyme is already signed, but we need to resign it
        QMAKE_POST_LINK += $$[QT_INSTALL_BINS]/macdeployqt $${OUT_PWD}/$${DESTDIR}/$${TARGET}.app &&
        QMAKE_POST_LINK += codesign --deep --timestamp --options runtime --force -s $${APPLE_SIGN_IDENTITY} $${OUT_PWD}/$${DESTDIR}/$${TARGET}.app &&
        QMAKE_POST_LINK += codesign --verify $${OUT_PWD}/$${DESTDIR}/$${TARGET}.app
    }
}

win32{
    # run windeployqt to include all qt libraries and vc_redist in $${DESTDIR}
    QMAKE_POST_LINK += windeployqt $$shell_path($$DESTDIR/$${TARGET}.exe)
}
