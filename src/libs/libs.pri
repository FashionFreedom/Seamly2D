#VTools static library
INCLUDEPATH += $${PWD}/vtools

#Tools static library
INCLUDEPATH += $${PWD}/tools

#VWidgets static library
INCLUDEPATH += $${PWD}/vwidgets

#VFormat static library
INCLUDEPATH += $${PWD}/vformat

#VPatternDB static library
INCLUDEPATH += $${PWD}/vpatterndb

#Fervor static library
INCLUDEPATH += $${PWD}/fervor

#VMisc static library
INCLUDEPATH += $${PWD}/vmisc

#VGeometry static library
INCLUDEPATH += $${PWD}/vgeometry

#IFC static library
INCLUDEPATH += $${PWD}/ifc

#VLayout static library
INCLUDEPATH += $${PWD}/vlayout

#QMuParser library
INCLUDEPATH += $${PWD}/qmuparser

#VPropertyExplorer static library
INCLUDEPATH += $${PWD}/vpropertyexplorer

#VTest static library
INCLUDEPATH += $${PWD}/vtest

#xerces static library, vendored for msvc, mingw and mac locally
win32:!win32-g++:INCLUDEPATH += $${PWD}/xerces-c/msvc/include
win32-g++:INCLUDEPATH += $${PWD}/xerces-c/mingw/include
macx:INCLUDEPATH += $${PWD}/xerces-c/macx/include
#xerces headers and library on linux via package manager, no extra include path required
