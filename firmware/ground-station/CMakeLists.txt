cmake_minimum_required(VERSION 3.23)
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
project(psat_lora LANGUAGES C CXX ASM)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# --- Source Files & Directories ---
set(ai_thinker_source_dir "external/Ai-Thinker-LoRaWAN-Ra-08")
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
    -mthumb  
    -fno-builtin-printf
    -fno-builtin-sprintf
    -fno-builtin-snprintf
    -Wno-format-security
    -Wno-format-truncation
)
target_compile_definitions(ai_thinker_lib PUBLIC
    CONFIG_DEBUG_UART=UART0 # Sets uart0 to be debug. 
    USE_MODEM_LORA
    REGION_CN470
)

# Set this to your main executable
set(main_c 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.c"
    # DO NOT REMOVE THIS!!!!
    "${CMAKE_CURRENT_SOURCE_DIR}/src/tremo_it.c"
)
add_executable(main 
    ${main_c}
    "${ai_thinker_source_dir}/platform/system/startup_cm4.S"
)


# Any other c files should automatically get added here.
file(GLOB_RECURSE src_lib_c "src/*")
list(FILTER src_lib_c EXCLUDE REGEX ".*/(main|tremo_it)\\.c$")

# Also add any custom libraries here.
add_library(src_lib STATIC ${src_lib_c})
# For us we need this.
target_include_directories(src_lib PUBLIC 
    "../shared/"
)


target_link_libraries(main PRIVATE
    ai_thinker_lib
    # Crypto is broken
    # "${ai_thinker_source_dir}/drivers/crypto/lib/libcrypto.a"
    # Add anything else you want to link
    # Down here.
)

# Add linker options for this target
target_link_options(main PRIVATE
    -static
    -mcpu=cortex-m4
    -mthumb
    "-T${linker_script}"
    -Wl,--gc-sections
    -Wl,--wrap=printf
    -Wl,--wrap=sprintf
    -Wl,--wrap=snprintf
    -Wl,-Map=${CMAKE_BINARY_DIR}/main.map
)
