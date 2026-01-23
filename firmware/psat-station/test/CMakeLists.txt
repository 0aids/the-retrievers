cmake_minimum_required(VERSION 3.5)
include($ENV{IDF_PATH}/tools/cmake/version.cmake)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

idf_build_set_property(C_COMPILE_OPTIONS -Wno-error APPEND)

set(EXTRA_COMPONENT_DIRS
"${CMAKE_SOURCE_DIR}/../../platform-agnostic" "../components"
)
set(TEST_COMPONENTS "buzzers" "servo")

project(psat-station-test)
