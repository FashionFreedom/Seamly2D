message("Entering test.pro")
TEMPLATE = subdirs
SUBDIRS = \
    ParserTest \
    Seamly2DTest \
    TranslationsTest \
    CollectionTest

unix: SUBDIRS += McpTest
