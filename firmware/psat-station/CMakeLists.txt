# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)
set(EXTRA_COMPONENT_DIRS
    "${CMAKE_SOURCE_DIR}/../platform-agnostic"
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

option(TEST "Set the psat station testing flag" 0)
if(TEST)
    add_compile_definitions(
      "PSAT_STATION_TESTING_d"
    )
endif()
unset(TEST CACHE) 

project(psat-station)
idf_build_set_property(
    INCLUDE_DIRECTORIES
    "${CMAKE_SOURCE_DIR}/../platform-agnostic/shared"
    APPEND
)

idf_build_set_property(C_COMPILE_OPTIONS -Wno-error APPEND)
get_filename_component(C_COMPILER_NAME ${CMAKE_C_COMPILER} NAME)

