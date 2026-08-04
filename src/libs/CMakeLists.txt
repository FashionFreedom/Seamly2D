cmake_minimum_required(VERSION 3.20)
project(Seamly2DLibs LANGUAGES CXX)

add_library(libs_common_include INTERFACE)

target_include_directories(libs_common_include INTERFACE
    ${CMAKE_SOURCE_DIR}/src/libs/vtools
    ${CMAKE_SOURCE_DIR}/src/libs/tools
    ${CMAKE_SOURCE_DIR}/src/libs/vwidgets
    ${CMAKE_SOURCE_DIR}/src/libs/vformat
    ${CMAKE_SOURCE_DIR}/src/libs/vpatterndb
    ${CMAKE_SOURCE_DIR}/src/libs/fervor
    ${CMAKE_SOURCE_DIR}/src/libs/vmisc
    ${CMAKE_SOURCE_DIR}/src/libs/vgeometry
    ${CMAKE_SOURCE_DIR}/src/libs/ifc
    ${CMAKE_SOURCE_DIR}/src/libs/vlayout
    ${CMAKE_SOURCE_DIR}/src/libs/qmuparser
    ${CMAKE_SOURCE_DIR}/src/libs/vpropertyexplorer
    ${CMAKE_SOURCE_DIR}/src/libs/vtest
    ${CMAKE_SOURCE_DIR}/src/libs/vobj
    ${CMAKE_SOURCE_DIR}/src/libs/vdxf
)

add_subdirectory(qmuparser)
add_subdirectory(vpropertyexplorer)
add_subdirectory(ifc)
add_subdirectory(vobj)
add_subdirectory(vdxf)
add_subdirectory(vlayout)
add_subdirectory(vgeometry)
add_subdirectory(vpatterndb)
add_subdirectory(vmisc)
add_subdirectory(vwidgets)
add_subdirectory(vtools)
add_subdirectory(vformat)
add_subdirectory(fervor)
add_subdirectory(vtest)
add_subdirectory(tools)
