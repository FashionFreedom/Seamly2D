TEMPLATE = subdirs
SUBDIRS = \
    tape \
    seamly2d

macx{# For making app bundle tape must exist before seamly2d.app will be created
    seamly2d.depends = tape
}
