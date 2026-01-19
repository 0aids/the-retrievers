cmake_minimum_required(VERSION 3.23)

project(the_retrievers_firmware_shared_api)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS 
    "-fmacro-prefix-map=${CMAKE_SOURCE_DIR}=. -fdiagnostics-show-template-tree"
)

add_compile_options(-Wall -Wextra -Og -g)

set(shared_dir "../platform-agnostic/shared/")
set(required_impls_dir "../platform-agnostic/required-impls/")

#########################
# Building test helpers #
#########################

file(GLOB test_helpers_cfiles test_helpers/*.c)
add_library(test_helpers STATIC ${test_helpers_cfiles})
target_include_directories(test_helpers PUBLIC test_helpers)
# target_link_libraries(test_helpers PRIVATE shared_api)

##################################
# Building the shared components #
##################################

# Build the implementations required.
file(GLOB impls_c impls/*.c)
add_library(impls ${impls_c})
target_include_directories(impls PUBLIC ${required_impls_dir})
target_link_libraries(impls PRIVATE test_helpers)

# Build the shared apis
file(GLOB shared_api_cfiles ${shared_dir}/*.c)
add_library(shared_api STATIC ${shared_api_cfiles})
target_include_directories(shared_api PUBLIC ${shared_dir} ${required_impls_dir})
target_link_libraries(shared_api PRIVATE impls)



##################
# Building tests #
##################

file(GLOB test_cfiles tests/*.c)
foreach(test_c IN LISTS test_cfiles)
    get_filename_component(test_name ${test_c} NAME_WE)
    add_executable(${test_name} ${test_c})
    target_link_libraries(${test_name} 
        PRIVATE shared_api
        PRIVATE test_helpers
        PRIVATE impls
    )
endforeach()
