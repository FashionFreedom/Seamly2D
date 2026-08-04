file(GLOB_RECURSE vlayout_sources CONFIGURE_DEPENDS *.cpp)
file(GLOB_RECURSE vlayout_headers CONFIGURE_DEPENDS *.h)
file(GLOB_RECURSE vlayout_resources CONFIGURE_DEPENDS *.qrc)

add_library(vlayout STATIC ${vlayout_sources} ${vlayout_resources})
target_include_directories(vlayout PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(vlayout PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets Qt6::PrintSupport Qt6::Xml)
target_link_libraries(vlayout PUBLIC libs_common_include)
target_compile_features(vlayout PUBLIC cxx_std_14)
