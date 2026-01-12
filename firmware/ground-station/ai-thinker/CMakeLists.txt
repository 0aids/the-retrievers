cmake_minimum_required(VERSION 3.23)

# =============================================================================
# Project Configuration
# =============================================================================

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
project(psat_lora LANGUAGES C CXX ASM)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS 
    "-fmacro-prefix-map=${CMAKE_SOURCE_DIR}=. -fdiagnostics-show-template-tree"
)

add_compile_options(
    # -Os
    -Wall 
    -Wextra 
    $<$<COMPILE_LANGUAGE:C>:-std=c23>
    -ffunction-sections
    -fdata-sections
    -fno-common
    -mfpu=fpv4-sp-d16
    -mfloat-abi=softfp
    -mcpu=cortex-m4
    -mthumb
)

# =============================================================================
# Global Paths
# =============================================================================

set(ai_thinker_source_dir "external/Ai-Thinker-LoRaWAN-Ra-08")
set(linker_script "${CMAKE_CURRENT_SOURCE_DIR}/cfg/gcc.ld")

# =============================================================================
# Ai-Thinker SDK Sources
# =============================================================================

file(GLOB_RECURSE ai_thinker_c_src
    "${ai_thinker_source_dir}/drivers/peripheral/src/*.c"
    "${ai_thinker_source_dir}/lora/driver/*.c"
    "${ai_thinker_source_dir}/lora/mac/*.c"
    "${ai_thinker_source_dir}/lora/mac/region/*.c"
    "${ai_thinker_source_dir}/lora/radio/sx126x/*.c"
    "${ai_thinker_source_dir}/lora/system/*.c"
    "${ai_thinker_source_dir}/lora/system/crypto/*.c"
    "${ai_thinker_source_dir}/platform/system/*.c"
)

list(APPEND ai_thinker_c_src
    "${ai_thinker_source_dir}/platform/system/printf-stdarg.c"
)

# =============================================================================
# Ai-Thinker SDK Library Target
# =============================================================================

add_library(ai_thinker_lib STATIC ${ai_thinker_c_src})

target_include_directories(ai_thinker_lib PUBLIC
    "${ai_thinker_source_dir}/drivers/peripheral/inc"
    "${ai_thinker_source_dir}/drivers/crypto/inc"
    "${ai_thinker_source_dir}/lora/driver"
    "${ai_thinker_source_dir}/lora/mac"
    "${ai_thinker_source_dir}/lora/mac/region"
    "${ai_thinker_source_dir}/lora/radio"
    "${ai_thinker_source_dir}/lora/radio/sx126x"
    "${ai_thinker_source_dir}/lora/system"
    "${ai_thinker_source_dir}/lora/system/crypto"
    "${ai_thinker_source_dir}/platform/CMSIS"
    "${ai_thinker_source_dir}/platform/system"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc"
)

target_compile_options(ai_thinker_lib PUBLIC
    -Wall
    -Os
    # -ggdb
    -fno-builtin-printf
    -fno-builtin-sprintf
    -fno-builtin-snprintf
    -Wno-format-security
    -Wno-format-truncation
)

target_compile_definitions(ai_thinker_lib PUBLIC
    CONFIG_DEBUG_UART=UART0
    USE_MODEM_LORA
    REGION_AU915
)

# =============================================================================
# Implementation
# =============================================================================
file(GLOB_RECURSE impls_files_c "impls/*.c")
message("-- impls_lib_files: ${impls_files_c}")
add_library(impls_lib STATIC ${impls_files_c})
target_link_libraries(impls_lib PRIVATE ai_thinker_lib)
target_include_directories(
    impls_lib PUBLIC
    "../../platform-agnostic/required-impls/"
)

# =============================================================================
# Application Support Library
# =============================================================================

file(GLOB_RECURSE src_lib_c "src/*")

add_library(src_lib STATIC ${src_lib_c})
target_include_directories(src_lib PUBLIC
    "./inc/"
)
target_link_libraries(src_lib PRIVATE 
    ai_thinker_lib
    impls_lib
)

# =============================================================================
# Application Sources
# =============================================================================

file(GLOB exe_dirs "exes/*")
message("-- Exes: ${exe_dirs}")
foreach(exe_dir IN LISTS exe_dirs)
    if(NOT IS_DIRECTORY ${exe_dir})
        continue()
    endif()
    file(GLOB exe_files_c "${exe_dir}/*.c")
    get_filename_component(exe_name ${exe_dir} NAME)
    message("-- Exe filename component: ${exe_name}")
    add_executable(${exe_name}
        ${exe_files_c}
        "${ai_thinker_source_dir}/platform/system/startup_cm4.S"
    )
    target_compile_options(${exe_name} PUBLIC
        -ffreestanding # Custom entrypoint so freestanding.
    )
    # Target Linking
    target_link_libraries(${exe_name} PRIVATE
        ai_thinker_lib
        impls_lib
        src_lib
    )

    # Linker Options
    target_link_options(${exe_name} PRIVATE
        -static
        -mcpu=cortex-m4
        -mthumb
        "-T${linker_script}"
        -Wl,--gc-sections
        -Wl,--wrap=printf
        -Wl,--wrap=sprintf
        -Wl,--wrap=snprintf
        -Wl,-Map=${CMAKE_BINARY_DIR}/main.map
        -ffreestanding
    )
    add_custom_command(
        TARGET ${exe_name}
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY}
        ARGS -O binary $<TARGET_FILE:${exe_name}> ${exe_name}.bin
    )
endforeach()
# set(main_c
#     "${CMAKE_CURRENT_SOURCE_DIR}/src/main.c"
# )

# add_executable(main
#     ${main_c}
# )
