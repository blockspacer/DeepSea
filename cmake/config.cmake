if (MSVC)
	set(commonFlags "/W3 /WX /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_WARNINGS")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${commonFlags}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${commonFlags}")
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
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${commonFlags} ${otherCFlags}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${commonFlags} -std=c++11 ${otherCXXFlags}")
endif()

enable_testing()

function(set_folder target folderName)
	if (DEEPSEA_ROOT_FOLDER AND folderName)
		set_property(TARGET ${target} PROPERTY FOLDER ${DEEPSEA_ROOT_FOLDER}/${folderName})
	elseif (NOT DEEPSEA_ROOT_FOLDER AND folderName)
		set_property(TARGET ${target} PROPERTY FOLDER ${folderName})
	else()
		set_property(TARGET ${target} PROPERTY FOLDER ${DEEPSEA_ROOT_FOLDER})
	endif()
endfunction()

function(setup_filters)
	set(options)
	set(oneValueArgs SRC_DIR INCLUDE_DIR)
	set(multiValueArgs FILES)
	cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
	
	foreach (fileName ${ARGS_FILES})
		# Get the directory name. Make sure there's a trailing /.
		get_filename_component(directoryName ${fileName} DIRECTORY)
		set(directoryName ${directoryName}/)
		
		set(filterName)
		string(REGEX MATCH ${ARGS_SRC_DIR}/.* matchSrc ${directoryName})
		string(REGEX MATCH ${ARGS_INCLUDE_DIR}/.* matchInclude ${directoryName})
		
		if (matchSrc)
			string(REPLACE ${ARGS_SRC_DIR}/ "" filterName ${directoryName})
			set(filterName src/${filterName})
		elseif (matchInclude)
			string(REPLACE ${ARGS_INCLUDE_DIR}/ "" filterName ${directoryName})
			set(filterName include/${filterName})
		endif()
		
		if (filterName)
			string(REPLACE "/" "\\" filterName ${filterName})
			source_group(${filterName} FILES ${fileName})
		endif()
	endforeach()
endfunction()
