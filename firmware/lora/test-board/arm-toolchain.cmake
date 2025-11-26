# arm-toolchain.cmake

# Target system
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Get the toolchain root from the environment (or you can hardcode it)
# This assumes you run `export ARM_TOOLCHAIN_ROOT=${pkgs.gcc-arm-embedded}` before cmake
file(TO_CMAKE_PATH "$ENV{ARM_TOOLCHAIN_ROOT}" ARM_TOOLCHAIN_ROOT)
if(NOT DEFINED ARM_TOOLCHAIN_ROOT)
message(FATAL_ERROR "ARM_TOOLCHAIN_ROOT environment variable is not set.")
endif()

# Set the sysroot
set(CMAKE_SYSROOT ${ARM_TOOLCHAIN_ROOT}/arm-none-eabi)

# Compilers
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)

# Prevent CMake from searching for programs/libraries on the host system
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

