message("Entering app.pro")

TEMPLATE = subdirs
SUBDIRS = \
    seamlyme \
    seamly2d \
    seamly3d

macx{# For making app bundle seamlyme must exist before seamly2d.app will be created
    seamly2d.depends = seamlyme seamly3d
}
