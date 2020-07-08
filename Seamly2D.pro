include(common.pri)

message("Entering Seamly2D.pro")
#Check if Qt version >= 5.12.0
#!minQtVersion(5, 12, 0) {
#    message("Seamly2D.pro: Cannot build Seamly2D with Qt version $${QT_VERSION}.")
#    error("Use at least Qt 5.12.0.")
#}

#These checks need because we can quote all paths that contain space.

LIST = $$split(PWD,' ')
count(LIST, 1, >): error("The build will fail. Path '$${PWD}' contains space!!!")

LIST = $$split(OUT_PWD,' ')
count(LIST, 1, >): error("The build will fail. Path '$${OUT_PWD}' contains space!!!")

unix {
    *g++* {
        GCC_VERSION = $$system("g++ -dumpversion")
        contains(GCC_VERSION, ^7.*$) {
            message( "Seamly2D.pro: g++ version 7.x found" )
            CONFIG += g++7
        } else {
            contains(GCC_VERSION, ^6.*$) {
                message( "Seamly2D.pro: g++ version 6.x found" )
                CONFIG += g++6
            } else {
                contains(GCC_VERSION, ^5.*$) {
                    message( "Seamly2D.pro: g++ version 5.x found" )
                    CONFIG += g++5
                } else {
                    contains(GCC_VERSION, ^4.*$) {
                        message( "Seamly2D.pro: g++ version 4.x found" )
                        CONFIG += g++4
                    } else {
                        message( "Seamly2D.pro: Unknown GCC configuration" )
                    }
                }
            }
        }
    }
}

TEMPLATE = subdirs
SUBDIRS = src
