# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

# for some reason i still gotta delete sdkconfig first but i'd rarely be building for prod so ill just deal with it

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(psat-station)
idf_build_set_property(
    INCLUDE_DIRECTORIES
    "${CMAKE_SOURCE_DIR}/../platform-agnostic/shared"
    APPEND
)