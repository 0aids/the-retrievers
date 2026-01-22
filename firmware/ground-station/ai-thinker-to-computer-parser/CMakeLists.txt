cmake_minimum_required(VERSION 3.23)
project(ai_thinker_to_computer_parser)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_FLAGS 
    "-fmacro-prefix-map=${CMAKE_SOURCE_DIR}=."
)

file(GLOB src_files_c "./src/*.c")
add_library(src_lib ${src_files_c})
target_include_directories(src_lib PUBLIC "./inc/")

file(GLOB exe_files_c "./exes/*.c")
foreach(exe_file_c IN LISTS exe_files_c)
	get_filename_component(exe_name ${exe_file_c} NAME_WE)
	add_executable(${exe_name}
		${exe_file_c})
	target_link_libraries(${exe_name} PRIVATE src_lib)
endforeach()
