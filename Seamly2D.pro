include(common.pri)

message("Entering Seamly2D.pro")
#Check if Qt version >= 5.15.2
#!minQtVersion(5, 15, 2) {
#    message("Seamly2D.pro: Cannot build Seamly2D with Qt version $${QT_VERSION}.")
#    error("Use at least Qt 5.15.2.")
#}

#These checks need because we can quote all paths that contain space.

LIST = $$split(PWD,' ')
count(LIST, 1, >): error("The build will fail. Path '$${PWD}' contains space!!!")

LIST = $$split(OUT_PWD,' ')
count(LIST, 1, >): error("The build will fail. Path '$${OUT_PWD}' contains space!!!")

unix {
    *g++* {
        GCC_VERSION = $$system("g++ -dumpversion")
        contains(GCC_VERSION, ^15.*$) {
            message( "Seamly2D.pro: g++ version 15.x found" )
            CONFIG += g++7
        } else {
            contains(GCC_VERSION, ^14.*$) {
                message( "Seamly2D.pro: g++ version 14.x found" )
                CONFIG += g++6
            } else {
                contains(GCC_VERSION, ^13.*$) {
                    message( "Seamly2D.pro: g++ version 13.x found" )
                    CONFIG += g++5
                } else {
                    contains(GCC_VERSION, ^12.*$) {
                        message( "Seamly2D.pro: g++ version 12.x found" )
                        CONFIG += g++4
                    } else {
                        message( "Seamly2D.pro: GCC version $${GCC_VERSION} found")
                    }
                }
            }
        }
    }
}

TEMPLATE = subdirs
SUBDIRS = src
