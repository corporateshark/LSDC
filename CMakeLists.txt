cmake_minimum_required(VERSION 3.0)

project("LSDC" CXX C)
set(PROJECT_NAME "LSDC")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

macro(setup_source_groups src_files)
	foreach(FILE ${src_files})
		get_filename_component(PARENT_DIR "${FILE}" PATH)

		# skip src or include and changes /'s to \\'s
		string(REGEX REPLACE "(\\./)?(src|include)/?" "" GROUP "${PARENT_DIR}")
		string(REPLACE "/" "\\" GROUP "${GROUP}")

		# group by the folder name
		if(${FILE} MATCHES ".*/.+[.]cpp")
			set(GROUP "${GROUP}")
		elseif(${FILE} MATCHES ".*/.+[.]c")
			set(GROUP "${GROUP}")
		elseif(${FILE} MATCHES ".*/.+[.]h")
			set(GROUP "${GROUP}")
		endif()

		source_group("${GROUP}" FILES "${FILE}")
	endforeach()
endmacro()

if(MSVC)
	if(CMAKE_SIZEOF_VOID_P LESS 8)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:SSE2")
	endif()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /bigobj")
	add_definitions(-D_CONSOLE)
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
	add_compile_options(-msse4.1)
endif()

include_directories(
	libsrc
	src
)

file(GLOB_RECURSE SRC_FILES LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} libsrc/*.cpp libsrc/*.c)
file(GLOB_RECURSE HEADER_FILES LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} libsrc/*.h)

set(ALL_FILES
	${SRC_FILES}
	${HEADER_FILES}
)

setup_source_groups("${ALL_FILES}")

add_library(LSDClib ${ALL_FILES})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(${PROJECT_NAME} "src/LSDC.cpp" )

target_link_libraries(LSDC
	LSDClib
)

#add_executable("LSDCtest" "src/test.cpp" )
#target_link_libraries(LSDCtest
#	LSDClib
#)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME_RELEASE ${PROJECT_NAME})
