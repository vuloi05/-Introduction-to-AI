cmake_minimum_required(VERSION 4.0)
project(Gomoku LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif ()

add_library(ryehl_lib INTERFACE)
target_include_directories(ryehl_lib
        INTERFACE
        ${PROJECT_SOURCE_DIR}/include
)

add_library(argparse INTERFACE)
target_include_directories(argparse
        INTERFACE
        ${PROJECT_SOURCE_DIR}/include
)

add_executable(Gomoku
        src/main.cpp
        src/board.cpp
        src/board.hpp
        src/cli_game.cpp
        src/cli_game.hpp
        src/types.hpp
        src/inplace_vector.hpp
        src/tptable.cpp
        src/tptable.hpp
        src/memory.cpp
        src/memory.hpp
        src/misc.hpp
        src/zobrist.cpp
        src/zobrist.hpp
        src/thread.cpp
        src/thread.hpp
        src/engine.cpp
        src/engine.hpp
)

target_include_directories(Gomoku PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(Gomoku PRIVATE ryehl_lib argparse)

# Compiler options
target_compile_options(Gomoku PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        $<$<CONFIG:Release>:-O3>
        $<$<CONFIG:Release>:-flto>
)

# x86-specific SIMD flags
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|i[3-6]86")
    target_compile_options(Gomoku PRIVATE
            -mavx2
            -mbmi
            -mbmi2
            -mpopcnt
    )
endif()

target_link_options(Gomoku PRIVATE
        $<$<CONFIG:Release>:-flto>
        $<$<CONFIG:Release>:-flto-partition=one>
)
