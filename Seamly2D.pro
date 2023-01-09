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

TEMPLATE = subdirs
SUBDIRS = \
    src \
    out

out.depends = src