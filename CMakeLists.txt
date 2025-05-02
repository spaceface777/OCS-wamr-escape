# Copyright (C) 2019 Intel Corporation.  All rights reserved.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

cmake_minimum_required(VERSION 3.20.0)
project(ocs-wamr-sandbox-escape)

set(CMAKE_C_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(WAMR_BUILD_PLATFORM "linux")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(WAMR_BUILD_PLATFORM "darwin")
    add_definitions(-DBH_PLATFORM_DARWIN)
else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    set(WAMR_BUILD_TARGET "X86_64")
    set(WAMRC_TARGET "x86_64")
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm")
    set(WAMR_BUILD_TARGET "ARM")
    set(WAMRC_TARGET "armv4")
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "armv7l")
    set(WAMR_BUILD_TARGET "ARM")
    set(WAMRC_TARGET "armv7")
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
    set(WAMR_BUILD_TARGET "AARCH64")
    set(WAMRC_TARGET "aarch64v8")
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
    set(WAMR_BUILD_TARGET "AARCH64")
    set(WAMRC_TARGET "aarch64v8")
else()
    message(FATAL_ERROR "Unsupported target: ${CMAKE_SYSTEM_PROCESSOR}")
endif()

set (CMAKE_BUILD_TYPE ReleaseDebug)

set (WAMR_BUILD_AOT 1)
set (WAMR_BUILD_INTERP 0)
set (WAMR_BUILD_FAST_INTERP 0)
set (WAMR_BUILD_JIT 0)
set (WAMR_BUILD_LIBC_WASI 0)
set (WAMR_BUILD_FAST_JIT 0)
set (WAMR_BUILD_LIBC_BUILTIN 1)
set (WAMR_BUILD_GLOBAL_HEAP_POOL 1)
set (WAMR_BUILD_GLOBAL_HEAP_SIZE 32768) # 32 KB
set (WAMR_BUILD_BULK_MEMORY 0)
set (WAMR_BUILD_SIMD 1)
set (WAMR_DISABLE_HW_BOUND_CHECK 1)
set (WAMR_BH_VPRINTF wasm_vprintf)
set (WAMR_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/modules/wamr/)


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g3 -O0" CACHE STRING "" FORCE)
add_compile_options("-g3" "-O0")
link_libraries("-g3" "-O0")

add_definitions (-DBUILTIN_LIBC_BUFFERED_PRINTF=1 -Wall -Wextra -Wno-unused-command-line-argument -Wno-unused-function)

include (${WAMR_ROOT_DIR}/build-scripts/runtime_lib.cmake)
add_library(vmlib ${WAMR_RUNTIME_LIB_SOURCE})

file(GLOB_RECURSE MODULE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/modules/**/*.c")
set_source_files_properties(${MODULE_SOURCES} PROPERTIES COMPILE_FLAGS "-w")

# Source files
set(SOURCES
    src/main.c
)

set(WAMRC_FLAGS "--target=${WAMRC_TARGET}" "--xip" "--bounds-checks=0")

add_custom_command(
    OUTPUT exploit.aot exploit.o exploit.ll exploit.wasm asm.txt
    COMMAND /opt/homebrew/opt/llvm@18/bin/clang -Wall -Wextra -target wasm32 -nostdlib
            -Wl,--no-entry,--allow-undefined,--export-dynamic,-z,stack-size=16384,--max-memory=65536
            -O3 ${CMAKE_CURRENT_SOURCE_DIR}/exploit/exploit.c -o exploit.wasm

    COMMAND wamrc ${WAMRC_FLAGS} --format=llvmir-opt -o exploit.ll exploit.wasm

    COMMAND wamrc ${WAMRC_FLAGS} --format=object -o exploit.o exploit.wasm

    COMMAND wamrc ${WAMRC_FLAGS} -o exploit.aot exploit.wasm

    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/exploit* ${CMAKE_CURRENT_SOURCE_DIR}

    COMMAND objdump -d exploit.o > exploit.S

    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/exploit/exploit.c
    COMMENT "Building wasm and AOT files"
)

add_custom_target(
    build_exploit ALL
    DEPENDS exploit.aot exploit.o exploit.ll exploit.wasm asm.txt
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

add_dependencies(${PROJECT_NAME} build_exploit)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    vmlib
    m
    pthread
)
