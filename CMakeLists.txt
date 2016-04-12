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

# install
install(TARGETS ${JOSH} DESTINATION bin)

# tests
include(CTest)

## simple call
add_test(NAME simple-call
	COMMAND josh
)

## generate object
add_test(NAME simple-object
	COMMAND josh foo=0 bar=1 baz=2
)
set_tests_properties(simple-object
	PROPERTIES
	PASS_REGULAR_EXPRESSION "\\{\"foo\":0,\"bar\":1,\"baz\":2\\}"
)

## generate object with pretty print
add_test(NAME simple-object-pretty
	COMMAND josh -p foo=0 bar=1 baz=2
)

## generate array
add_test(NAME simple-array
	COMMAND josh -a 0 1 2 3 4 5
)
set_tests_properties(simple-array
	PROPERTIES
	PASS_REGULAR_EXPRESSION "\\[0,1,2,3,4,5\\]"
)

## generate array with pretty print
add_test(NAME simple-array-pretty
	COMMAND josh -p -a 0 1 2 3 4 5
)

## boolean is parsed
add_test(NAME boolean-as-is
	COMMAND josh -a true false
)
set_tests_properties(boolean-as-is
	PROPERTIES
	PASS_REGULAR_EXPRESSION "\\[true,false\\]"
)

## booloff is working
add_test(NAME boolean-off
	COMMAND josh -B -a true false
)
set_tests_properties(boolean-off
	PROPERTIES
	PASS_REGULAR_EXPRESSION "\\[\"true\",\"false\"\\]"
)

## nested working
add_test(NAME nested-object-and-array
	COMMAND josh foo={\"bar\":\"baz\"} array=[0,1,2,3]
)
set_tests_properties(nested-object-and-array
	PROPERTIES
	PASS_REGULAR_EXPRESSION "\\{\"foo\":\\{\"bar\":\"baz\"\\},\"array\":\\[0,1,2,3\\]\\}"
)
