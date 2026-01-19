# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)
set(EXTRA_COMPONENT_DIRS
    "${CMAKE_SOURCE_DIR}/../platform-agnostic"
)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(psat-station)
idf_build_set_property(
    INCLUDE_DIRECTORIES
    "${CMAKE_SOURCE_DIR}/../platform-agnostic/shared"
    APPEND
)
# Who the fuck set -Werror istg
idf_build_set_property(C_COMPILE_OPTIONS -Wno-error APPEND)

get_filename_component(C_COMPILER_NAME ${CMAKE_C_COMPILER} NAME)
if ("${C_COMPILER_NAME}" STREQUAL "clang")
    message("-- !!!compiler is ${C_COMPILER_NAME}, disabling -Werror (-Wno-error)!!!")
    idf_build_set_property(C_COMPILE_OPTIONS -Wno-implicit-function-declaration APPEND)
endif()
