cmake_minimum_required(VERSION 3.23)

# =============================================================================
# Project Configuration
# =============================================================================

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
project(psat_lora LANGUAGES C CXX ASM)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# set(CMAKE_CXX_FLAGS 
#     "-fmacro-prefix-map=${CMAKE_SOURCE_DIR}=. -fdiagnostics-show-template-tree"
# )

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

set(asr_lora_6601_source_dir "external/asr_lora_6601")
set(linker_script "${CMAKE_CURRENT_SOURCE_DIR}/cfg/gcc.ld")

# =============================================================================
# asr_lora_6601 SDK Sources
# =============================================================================

file(GLOB_RECURSE asr_lora_6601_c_src
    "${asr_lora_6601_source_dir}/drivers/peripheral/src/*.c"
    "${asr_lora_6601_source_dir}/lora/driver/*.c"
    "${asr_lora_6601_source_dir}/lora/mac/*.c"
    "${asr_lora_6601_source_dir}/lora/mac/region/*.c"
    "${asr_lora_6601_source_dir}/lora/radio/sx126x/*.c"
    "${asr_lora_6601_source_dir}/lora/system/*.c"
    "${asr_lora_6601_source_dir}/lora/system/crypto/*.c"
    "${asr_lora_6601_source_dir}/platform/system/*.c"
)

list(APPEND asr_lora_6601_c_src
    "${asr_lora_6601_source_dir}/platform/system/printf-stdarg.c"
)

# =============================================================================
# asr_lora_6601 SDK Library Target
# =============================================================================

add_library(asr_lora_6601_lib STATIC ${asr_lora_6601_c_src})

target_include_directories(asr_lora_6601_lib PUBLIC
    "${asr_lora_6601_source_dir}/drivers/peripheral/inc"
    "${asr_lora_6601_source_dir}/drivers/crypto/inc"
    "${asr_lora_6601_source_dir}/lora/driver"
    "${asr_lora_6601_source_dir}/lora/mac"
    "${asr_lora_6601_source_dir}/lora/mac/region"
    "${asr_lora_6601_source_dir}/lora/radio"
    "${asr_lora_6601_source_dir}/lora/radio/sx126x"
    "${asr_lora_6601_source_dir}/lora/system"
    "${asr_lora_6601_source_dir}/lora/system/crypto"
    "${asr_lora_6601_source_dir}/platform/CMSIS"
    "${asr_lora_6601_source_dir}/platform/system"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc"
)

target_compile_options(asr_lora_6601_lib PUBLIC
    -Wall
    -Os
    # -ggdb
    -fno-builtin-printf
    -fno-builtin-sprintf
    -fno-builtin-snprintf
    -Wno-format-security
    -Wno-format-truncation
)

target_compile_definitions(asr_lora_6601_lib PUBLIC
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
target_link_libraries(impls_lib PRIVATE asr_lora_6601_lib)
target_include_directories(
    impls_lib PUBLIC
    "../../platform-agnostic/required-impls/"
)

# =============================================================================
# Shared
# =============================================================================
set(platform_agnostic_dir "../../platform-agnostic/shared/")
file(GLOB_RECURSE shared_files_c "${platform_agnostic_dir}/*.c")
message("-- shared_files_c: ${shared_files_c}")
add_library(shared_lib STATIC ${shared_files_c})
target_link_libraries(shared_lib PRIVATE impls_lib)
target_compile_options(shared_lib PUBLIC
    -fmacro-prefix-map=${platform_agnostic_dir}=.
)
target_include_directories(shared_lib PUBLIC ${platform_agnostic_dir})

# =============================================================================
# Application Support Library
# =============================================================================

file(GLOB_RECURSE src_lib_c "src/*")

add_library(src_lib STATIC ${src_lib_c})
target_include_directories(src_lib PUBLIC
    "./inc/"
)
target_link_libraries(src_lib PRIVATE 
    asr_lora_6601_lib
    impls_lib
    shared_lib
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
        "${asr_lora_6601_source_dir}/platform/system/startup_cm4.S"
    )
    target_compile_options(${exe_name} PUBLIC
        -ffreestanding # Custom entrypoint so freestanding.
    )
    # Target Linking
    target_link_libraries(${exe_name} PRIVATE
        asr_lora_6601_lib
        impls_lib
        src_lib
        shared_lib
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
        -Wl,--wrap=vprintf
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
