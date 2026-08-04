file(GLOB_RECURSE vgeometry_sources CONFIGURE_DEPENDS *.cpp)
file(GLOB_RECURSE vgeometry_headers CONFIGURE_DEPENDS *.h)
file(GLOB_RECURSE vgeometry_resources CONFIGURE_DEPENDS *.qrc)

add_library(vgeometry STATIC ${vgeometry_sources} ${vgeometry_resources})
target_include_directories(vgeometry PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(vgeometry PUBLIC Qt6::Widgets Qt6::PrintSupport)
target_link_libraries(vgeometry PUBLIC ifc libs_common_include)
target_compile_features(vgeometry PUBLIC cxx_std_14)
