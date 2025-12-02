cmake_minimum_required(VERSION 3.23)
# The project() command is where compiler detection happens.
# The toolchain file must be specified on the command line so it runs BEFORE this.
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
project(psat_lora LANGUAGES C CXX ASM)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# --- Source Files & Directories ---
set(ai_thinker_source_dir "external/Ai-Thinker-LoRaWAN-Ra-08")
set(main_grd_st "${CMAKE_CURRENT_SOURCE_DIR}/src/main_grd_st.c")
set(main_psat_st "${CMAKE_CURRENT_SOURCE_DIR}/src/main_psat_st.c")
set(linker_script "${CMAKE_CURRENT_SOURCE_DIR}/cfg/gcc.ld")


# --- Ai-Thinker SDK Library ---
# Correctly glob for all source files recursively
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
# Manually add the printf implementation file
list(APPEND ai_thinker_c_src "${ai_thinker_source_dir}/platform/system/printf-stdarg.c")
# Create a library from all the SDK source files
add_library(ai_thinker_lib STATIC ${ai_thinker_c_src})
# Add the necessary include directories for the SDK library
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
# --- Compile Options & Definitions for the SDK library ---
# We use PUBLIC here so that any target linking this library will inherit these settings.
target_compile_options(ai_thinker_lib PUBLIC
    -Wall
    -Os
    -ggdb
    -ffunction-sections
    -fdata-sections
    -fno-common
    -mfpu=fpv4-sp-d16
    -mfloat-abi=softfp
    -mcpu=cortex-m4
    -mthumb  # <-- This fixes the ARM mode errors
    -fno-builtin-printf
    -fno-builtin-sprintf
    -fno-builtin-snprintf
)
target_compile_definitions(ai_thinker_lib PUBLIC
    CONFIG_DEBUG_UART=UART0  # <-- This fixes the undeclared identifier error
    USE_MODEM_LORA
    REGION_CN470
    d_runPsatConnectionTests
)


# --- Extra library stuff ---
# other library stuff and files
file(GLOB_RECURSE psat_c_src 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/psat_station/*"
)

file(GLOB_RECURSE ground_c_src 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ground_station/*"
)

file(GLOB_RECURSE other_lib_c_src
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*"
)

list(REMOVE_ITEM other_lib_c_src
    ${psat_c_src}
    ${ground_c_src}
)

add_library(
    other_lib STATIC ${other_lib_c_src}
)

target_compile_options(other_lib PUBLIC
    -Wall
    -Os
    -ggdb
    -ffunction-sections
    -fdata-sections
    -fno-common
    -mfpu=fpv4-sp-d16
    -mfloat-abi=softfp
    -mcpu=cortex-m4
    -mthumb  # <-- This fixes the ARM mode errors
    -fno-builtin-printf
    -fno-builtin-sprintf
    -fno-builtin-snprintf
)
target_link_libraries(other_lib PRIVATE
    ai_thinker_lib
)

# --- Executable Targets ---
add_executable(ground_station
    ${ground_c_src}
    "${ai_thinker_source_dir}/platform/system/startup_cm4.S"
)
add_executable(psat_station
    ${psat_c_src}
    "${ai_thinker_source_dir}/platform/system/startup_cm4.S"
)
# add_executable(psat_station
#     ${main_psat_st}
#     "${ai_thinker_source_dir}/platform/system/startup_cm4.S"
# )
# --- Target-Specific Options ---
foreach(TARGET IN ITEMS ground_station psat_station)
    # Link the SDK library and the pre-compiled crypto library
    target_link_libraries(${TARGET} PRIVATE
        ai_thinker_lib
        other_lib
        # "${ai_thinker_source_dir}/drivers/crypto/lib/libcrypto.a"
    )
    # The compile options and definitions are now inherited from ai_thinker_lib,
    # so we can remove them from this loop.
    # Add linker options for this target
    target_link_options(${TARGET} PRIVATE
        --specs=nano.specs
        -static
        -mcpu=cortex-m4
        -mthumb
        "-T${linker_script}"
        -Wl,--gc-sections
        -Wl,--wrap=printf
        -Wl,--wrap=sprintf
        -Wl,--wrap=snprintf
        -Wl,-Map=${CMAKE_BINARY_DIR}/${TARGET}.map
    )
    # Add custom command to generate the .bin file
    add_custom_command(TARGET ${TARGET} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${TARGET}> ${CMAKE_BINARY_DIR}/${TARGET}.bin
        COMMENT "Generating ${TARGET}.bin"
    )
endforeach()

