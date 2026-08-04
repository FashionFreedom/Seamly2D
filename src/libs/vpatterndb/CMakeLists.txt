file(GLOB_RECURSE vpatterndb_sources CONFIGURE_DEPENDS *.cpp)
file(GLOB_RECURSE vpatterndb_headers CONFIGURE_DEPENDS *.h)
file(GLOB_RECURSE vpatterndb_resources CONFIGURE_DEPENDS *.qrc)

add_library(vpatterndb STATIC ${vpatterndb_sources} ${vpatterndb_resources})
target_include_directories(vpatterndb PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(vpatterndb PUBLIC Qt6::Widgets Qt6::PrintSupport)
target_link_libraries(vpatterndb PUBLIC vgeometry vmisc vlayout libs_common_include)
target_compile_features(vpatterndb PUBLIC cxx_std_14)
