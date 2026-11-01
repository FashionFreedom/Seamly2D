message("Entering app.pro")

TEMPLATE = subdirs
SUBDIRS = \
    seamlyme \
    seamly2d

unix: SUBDIRS += seamly2d_mcp

macx{# For making app bundle seamlyme must exist before seamly2d.app will be created
    seamly2d.depends = seamlyme seamly2d_mcp
}
