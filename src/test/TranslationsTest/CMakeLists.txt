file(GLOB_RECURSE translationstest_sources CONFIGURE_DEPENDS *.cpp *.c)
file(GLOB_RECURSE translationstest_headers CONFIGURE_DEPENDS *.h)
file(GLOB_RECURSE translationstest_forms CONFIGURE_DEPENDS *.ui)
file(GLOB_RECURSE translationstest_resources CONFIGURE_DEPENDS *.qrc)

# Remove generated sources (moc/uic generated files) from recursive globs
cmake_glob_remove_generated_sources(translationstest_sources)

add_executable(TranslationsTest ${translationstest_sources} ${translationstest_resources})
target_include_directories(TranslationsTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(TranslationsTest PRIVATE SRCDIR="${CMAKE_CURRENT_SOURCE_DIR}" TS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../../../share/translations")
target_link_libraries(TranslationsTest PRIVATE Qt6::Test Qt6::Widgets Qt6::Xml Qt6::PrintSupport vpatterndb vtest vmisc vgeometry ifc qmuparser libs_common_include)
target_compile_features(TranslationsTest PRIVATE cxx_std_14)
