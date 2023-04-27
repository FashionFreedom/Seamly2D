win32{
    # Because "copy" doesn't support files that contain plus sign (+) in name we will use xcopy instead.
    unset(QMAKE_COPY)
    QMAKE_COPY = xcopy /y

    unset(QMAKE_COPY_FILE)
    QMAKE_COPY_FILE = xcopy /y

    unset(QMAKE_INSTALL_FILE)
    QMAKE_INSTALL_FILE = xcopy /y

    unset(QMAKE_INSTALL_PROGRAM)
    QMAKE_INSTALL_PROGRAM = xcopy /y
}

unix{
    macx{
        VCOPY = $$QMAKE_COPY
    } else {
        VCOPY = $$QMAKE_COPY -u
    }
}

win32{
    VCOPY = $$QMAKE_COPY /D
}

unix{
    LIBS += -L/usr/lib/x86_64-linux-gnu/ -lxerces-c
}

win32{
    INCLUDEPATH += ../../../extern/xerces-c/include
    LIBS += -L$$OUT_PWD/../../../extern/xerces-c/lib/ -lxerces-c_3D
    PRE_TARGETDEPS += -L$$OUT_PWD/../../../extern/xerces-c/lib/ -lxerces-c_3D
}

# See question on StackOwerflow "QSslSocket error when SSL is NOT used" (http://stackoverflow.com/a/31277055/3045403)
# Copy of answer:
# We occasionally had customers getting very similar warning messages but the software was also crashing.
# We determined it was because, although we weren't using SSL either, the program found a copy of OpenSSL on the
# customer's computer and tried interfacing with it. The version it found was too old though (from Qt 5.2 onwards v1.0.0
# or later is required).
#
# Our solution was to distribute the OpenSSL DLLs along with our application (~1.65 MB). The alternative is to compile
# Qt from scratch without OpenSSL support.
win32 {
    INSTALL_OPENSSL += \
                       ../../../dist/win/libcrypto-1_1-x64.dll \
                       ../../../dist/win/libssl-1_1-x64.dll
}

CONFIG(debug, debug|release){
    # Debug mode, intentionally left empty
} else {
    # Release mode
    message("Release mode: V_NO_ASSERT V_NO_DEBUG defined")
    DEFINES += V_NO_ASSERT V_NO_DEBUG
}

CONFIG += c++14

# Only do debug or release builds also on windows
CONFIG -= debug_and_release debug_and_release_target

# Since Qt 5.4.0 the source code location is recorded only in debug builds.
# We need this information also in release builds. For this need define QT_MESSAGELOGCONTEXT.
DEFINES += QT_MESSAGELOGCONTEXT

# Copies the given files to the destination directory
defineTest(copyToDestdir) {
    files = $$1
    DDIR = $$2
    mkpath($$DDIR)

    for(FILE, files) {
        unix{
            QMAKE_POST_LINK += ln -s -f $$quote($$FILE) $$quote($$DDIR/$$basename(FILE)) & $$escape_expand(\\n\\t)
        } else {
            !exists($$DDIR/$$basename(FILE)) {
                # Replace slashes in paths with backslashes for Windows
                win32{
                    FILE ~= s,/,\\,g
                    DDIR ~= s,/,\\,g
                }
                QMAKE_POST_LINK += $$VCOPY $$quote($$FILE) $$quote($$DDIR) $$escape_expand(\\n\\t)
            }

            QMAKE_CLEAN += $$DDIR/$$basename(FILE)
        }
    }

    export(QMAKE_POST_LINK)
    export(QMAKE_CLEAN)
}

# Alwayse copies the given files to the destination directory
defineTest(forceCopyToDestdir) {
    files = $$1
    DDIR = $$2
    mkpath($$DDIR)

    for(FILE, files) {
        unix{
            QMAKE_POST_LINK += ln -s -f $$quote($$FILE) $$quote($$DDIR/$$basename(FILE)) & $$escape_expand(\\n\\t)
        } else {
            # Replace slashes in paths with backslashes for Windows
            win32{
                FILE ~= s,/,\\,g
                DDIR ~= s,/,\\,g
            }
            QMAKE_POST_LINK += $$VCOPY $$quote($$FILE) $$quote($$DDIR) $$escape_expand(\\n\\t)
            QMAKE_CLEAN += $$DDIR/$$basename(FILE)
        }
    }

    export(QMAKE_POST_LINK)
    export(QMAKE_CLEAN)
}

CONFIG(debug, debug|release){
    # Debug mode, intentionally left empty
} else {
    CONFIG += precompile_header # Turn on creation precompiled headers (PCH).
    PRECOMPILED_HEADER = stable.h # Header file with all all static headers: libraries, static local headers.
    *msvc*{
        PRECOMPILED_SOURCE = stable.cpp # MSVC need also cpp file.
    }
}

defineReplace(FindBuildRevision){
CONFIG(debug, debug|release){
    # Debug mode
    return(\\\"unknown\\\")
}else{
    # Release mode
    #build revision number for using in version
    #get the short form of the latest commit's changeset hash, i.e. a 12-character hexadecimal string
    DVCS_HESH=$$system("git rev-parse --short=12 HEAD") #get SHA1 commit hash
    message("common.pri: Latest commit hash:" $${DVCS_HESH})

    isEmpty(DVCS_HESH){
       DVCS_HESH = \\\"unknown\\\" # if we can't find build revision left unknown.
    } else {
       DVCS_HESH=\\\"Git:$${DVCS_HESH}\\\"
    }

    return($${DVCS_HESH})
}
}

# Default prefix. Use for creation install path.
DEFAULT_PREFIX = /usr

# In debug mode we use all usefull for us compilers keys for checking errors.
CONFIG(debug, debug|release){
    # Debug mode
    message("Normal mode: compiler warnings enabled")
    CONFIG += warn_on
} else {
    message("Release mode: no compiler warnings")
    CONFIG += warn_off
}
