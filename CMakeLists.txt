# Specify the minimum version for CMake
cmake_minimum_required(VERSION 3.0)

option(MINGW "MINGW Target" 0)

if (MINGW)
        set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/mingw.cmake)
endif()

# Project's name
project(agde)

# Set the output folder where your program will be created
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(SOURCES main.c parse.c csv.c)

# The following folder will be included
include_directories("${PROJECT_SOURCE_DIR}")
add_executable(agde ${SOURCES})
target_link_libraries(agde m)
