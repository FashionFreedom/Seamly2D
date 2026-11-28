message("Entering app.pro")

TEMPLATE = subdirs
SUBDIRS = \
    seamlyme \
    seamly2d

# seamly2d deploys seamlyme after link: into the macOS app bundle, and into the shared Windows Qt runtime.
# seamlyme must exist before seamly2d links.
macx|win32{
    seamly2d.depends = seamlyme
}
