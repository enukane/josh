cmake_minimum_required(VERSION 2.8)

PROJECT(josh)

# Find jaonsson

include(FindPkgConfig)
pkg_check_modules(PC_JANSSON jansson)

find_path(
	JANSSON_INCLUDE_DIRS
	NAMES jansson.h
	PATHS /usr/include /usr/local/include
)

find_library(
	JANSSON_LIBRARIES
	NAMES jansson
	PATHS /usr/lib /usr/local/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
	JANSSON DEFAULT_MSG JANSSON_LIBRARIES JANSSON_INCLUDE_DIRS
)
mark_as_advanced(JANSSON_LIBRARIES JANSSON_INCLUDE_DIRS)

if(NOT JANSSON_FOUND)
	message(FATAL_ERROR "Jansson library required, but not found\n")
endif(NOT JANSSON_FOUND)

# compile
set(JOSH josh)
ADD_DEFINITIONS(-O2 -Wall -g3 -Iinclude -Llib)
ADD_EXECUTABLE(${JOSH} josh.c
)
TARGET_LINK_LIBRARIES(${JOSH} ${JANSSON_LIBRARIES})
