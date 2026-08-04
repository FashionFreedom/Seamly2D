file(GLOB_RECURSE vformat_sources CONFIGURE_DEPENDS *.cpp)
file(GLOB_RECURSE vformat_headers CONFIGURE_DEPENDS *.h)
file(GLOB_RECURSE vformat_resources CONFIGURE_DEPENDS *.qrc)

add_library(vformat STATIC ${vformat_sources} ${vformat_resources})
target_include_directories(vformat PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(vformat PUBLIC Qt6::Xml Qt6::PrintSupport Qt6::Svg)
target_link_libraries(vformat PUBLIC vpatterndb ifc libs_common_include)
target_compile_features(vformat PUBLIC cxx_std_14)
