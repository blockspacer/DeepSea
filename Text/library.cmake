find_package(Freetype REQUIRED)
find_package(HarfBuzz REQUIRED)

set(textIncludeDirs )
set(textLibraries )

file(GLOB_RECURSE externalSources ${sheenBidiPatterns})
# Don't care about warnings for external files.
if (MSVC)
	set_source_files_properties(${externalSources} PROPERTIES COMPILE_FLAGS /w)
else()
	set_source_files_properties(${externalSources} PROPERTIES COMPILE_FLAGS -w)
endif()

ds_glob_library_sources(sources Text src/*.c src/*.h include/*.h)
ds_add_library(deepsea_text Text ${sources})
ds_target_include_directories(deepsea_text PRIVATE
	${FREETYPE_INCLUDE_DIRS} ${HARFBUZZ_INCLUDE_DIRS})
ds_target_link_libraries(deepsea_text PRIVATE ${FREETYPE_LIBRARIES} ${HARFBUZZ_LIBRARIES})
