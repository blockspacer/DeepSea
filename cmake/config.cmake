# Copyright 2017 Aaron Barany
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

if (MSVC)
	set(commonFlags "/W3 /WX /wd4146 /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_WARNINGS /MP")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${commonFlags}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${commonFlags} /D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING")
else()
	if (DEEPSEA_SHARED AND
		(CMAKE_C_COMPILER_ID MATCHES "GNU" OR CMAKE_C_COMPILER_ID MATCHES "Clang"))
		set(otherCFlags "-fvisibility=hidden")
		set(otherCXXFlags "${otherCFlags} -fvisibility-inlines-hidden")
	else()
		set(otherCFlags)
		set(otherCXXFlags)
	endif()

	set(commonFlags "-fPIC -Wall -Werror -Wconversion -Wno-sign-conversion -fno-strict-aliasing")
	if (APPLE)
		set(commonFlags "${commonFlags} -fobjc-arc")
	endif()
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${commonFlags} ${otherCFlags}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${commonFlags} -std=c++11 ${otherCXXFlags}")
endif()

enable_testing()

if (DEEPSEA_SINGLE_SHARED)
	# NOTE: Need at least one source to prevent CMake warning.
	add_library(deepsea ${DEEPSEA_LIB} CMakeLists.txt)
endif()

function(ds_set_folder target folderName)
	if (DEEPSEA_ROOT_FOLDER AND folderName)
		set_property(TARGET ${target} PROPERTY FOLDER ${DEEPSEA_ROOT_FOLDER}/${folderName})
	elseif (NOT DEEPSEA_ROOT_FOLDER AND folderName)
		set_property(TARGET ${target} PROPERTY FOLDER ${folderName})
	else()
		set_property(TARGET ${target} PROPERTY FOLDER ${DEEPSEA_ROOT_FOLDER})
	endif()
endfunction()

function(ds_setup_filters)
	set(options)
	set(oneValueArgs SRC_DIR INCLUDE_DIR FOLDER)
	set(multiValueArgs FILES)
	cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	if (ARGS_FOLDER)
		set(prefix "${ARGS_FOLDER}/")
	else()
		set(prefix)
	endif()

	foreach (fileName ${ARGS_FILES})
		# Get the directory name. Make sure there's a trailing /.
		get_filename_component(directoryName ${fileName} DIRECTORY)
		set(directoryName ${directoryName}/)

		set(filterName)
		string(REGEX MATCH ${ARGS_SRC_DIR}/.* matchSrc ${directoryName})
		string(REGEX MATCH ${ARGS_INCLUDE_DIR}/.* matchInclude ${directoryName})

		if (matchSrc)
			string(REPLACE ${ARGS_SRC_DIR}/ "" filterName ${directoryName})
			set(filterName src/${prefix}${filterName})
		elseif (matchInclude)
			string(REPLACE ${ARGS_INCLUDE_DIR}/ "" filterName ${directoryName})
			set(filterName include/${prefix}${filterName})
		endif()

		if (filterName)
			string(REPLACE "/" "\\" filterName ${filterName})
			source_group(${filterName} FILES ${fileName})
		endif()
	endforeach()
endfunction()

macro(ds_add_module moduleName)
	set(DEEPSEA_MODULES ${DEEPSEA_MODULES} ${moduleName})
	add_subdirectory(${moduleName})
endmacro()

macro(ds_finish_modules)
	if (DEEPSEA_SINGLE_SHARED)
		if (DEEPSEA_EXTERNAL_SOURCES)
			if (MSVC)
				set_source_files_properties(${DEEPSEA_EXTERNAL_SOURCES} PROPERTIES
					COMPILE_FLAGS /w)
			else()
				set_source_files_properties(${DEEPSEA_EXTERNAL_SOURCES} PROPERTIES
					COMPILE_FLAGS -w)
			endif()
		endif()
		target_sources(deepsea PRIVATE ${DEEPSEA_SOURCES} ${DEEPSEA_EXTERNAL_SOURCES})
		target_link_libraries(deepsea ${DEEPSEA_EXTERNAL_LIBRARIES})
	endif()
endmacro()

macro(ds_add_library target)
	set(options)
	set(oneValueArgs MODULE)
	set(multiValueArgs FILES EXTERNAL_FILES DEPENDS)
	cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	if (NOT ARGS_MODULE)
		message(FATAL_ERROR "No module set for ds_add_library()")
	endif()

	get_filename_component(mainModule ${ARGS_MODULE} NAME)
	if (DEEPSEA_SINGLE_SHARED)
		add_library(${target} INTERFACE)
		target_link_libraries(${target} INTERFACE deepsea ${ARGS_DEPENDS})

		set(DEEPSEA_SOURCES ${DEEPSEA_SOURCES} ${ARGS_FILES} PARENT_SCOPE)
		set(DEEPSEA_EXTERNAL_SOURCES ${DEEPSEA_EXTERNAL_SOURCES} ${ARGS_EXTERNAL_FILES}
			PARENT_SCOPE)

		set_source_files_properties(${ARGS_FILES} PROPERTIES COMPILE_FLAGS -w)
		target_include_directories(deepsea PUBLIC
			$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
			$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>)

		ds_setup_filters(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${ARGS_MODULE}/src
			INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${ARGS_MODULE}/include/DeepSea/${mainModule}
			FILES ${ARGN} FOLDER ${mainModule})
	else()
		# NOTE: This only takes affect if set in the same scope as the target was created.
		if (ARGS_EXTERNAL_FILES)
			if (MSVC)
				set_source_files_properties(${ARGS_EXTERNAL_FILES} PROPERTIES COMPILE_FLAGS /w)
			else()
				set_source_files_properties(${ARGS_EXTERNAL_FILES} PROPERTIES COMPILE_FLAGS -w)
			endif()
		endif()

		add_library(${target} ${DEEPSEA_LIB} ${ARGS_FILES} ${ARGS_EXTERNAL_FILES})
		target_link_libraries(${target} PUBLIC ${ARGS_DEPENDS})

		ds_set_folder(${target} libs)
		ds_setup_filters(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src
			INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/DeepSea/${ARGS_MODULE}
			FILES ${ARGN})
	endif()
endmacro()

macro(ds_target_link_libraries target)
	if (DEEPSEA_SINGLE_SHARED)
		set(DEEPSEA_EXTERNAL_LIBRARIES ${DEEPSEA_EXTERNAL_LIBRARIES} ${ARGN} PARENT_SCOPE)
	else()
		target_link_libraries(${target} ${ARGN})
	endif()
endmacro()

macro(ds_target_include_directories target)
	if (DEEPSEA_SINGLE_SHARED)
		target_include_directories(deepsea ${ARGN})
	else()
		target_include_directories(${target} ${ARGN})
	endif()
endmacro()

macro(ds_target_compile_definitions target)
	if (DEEPSEA_SINGLE_SHARED)
		target_compile_definitions(deepsea ${ARGN})
	else()
		target_compile_definitions(${target} ${ARGN})
	endif()
endmacro()
