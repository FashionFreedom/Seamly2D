message("Entering Seamly2D.pro")

#Check if spaces are in directory names
LIST = $$split(PWD,' ')
count(LIST, 1, >): error("The build will fail. Path '$${PWD}' contains space!!!")

LIST = $$split(OUT_PWD,' ')
count(LIST, 1, >): error("The build will fail. Path '$${OUT_PWD}' contains space!!!")

TEMPLATE = subdirs
SUBDIRS = \
    src \
    out

out.depends = src

qtPrepareTool(LUPDATE, lupdate)
lupdate.commands = $$LUPDATE -noobsolete -locations none $$shell_path($${PWD}/share/translations/translations.pro)
lupdate.commands += && $$LUPDATE -noobsolete $$shell_path($${PWD}/share/translations/measurements.pro)

QMAKE_EXTRA_TARGETS += lupdate
