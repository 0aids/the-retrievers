cmake_minimum_required(VERSION 3.5)
include($ENV{IDF_PATH}/tools/cmake/version.cmake)

set(TEST_COMPONENTS "buzzers" "servo")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(psat-station-test)
idf_build_set_property(
    INCLUDE_DIRECTORIES
    "${CMAKE_SOURCE_DIR}/../platform-agnostic/shared"
    APPEND
)