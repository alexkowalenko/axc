
cmake_minimum_required(VERSION 3.31)

project(axc
        VERSION 0.1
        DESCRIPTION "AXC C Compiler"
        LANGUAGES CXX)

if(${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD")
    set(CMAKE_CXX_COMPILER "/usr/local/llvm20/bin/clang++" CACHE STRING "" FORCE)
endif()
if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    set(CMAKE_CXX_COMPILER "/opt/homebrew/opt/llvm/bin/clang++" CACHE STRING "" FORCE)
endif()
message(STATUS "C++ compiler = ${CMAKE_CXX_COMPILER}")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(cmake/PreventInSourceBuilds.cmake)
include(cmake/CPM.cmake)

# Dependency management
include(Dependencies.cmake)

# install
set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})

add_subdirectory(src)
# Add the machine code generation directory
add_subdirectory(src/machine/x86_64)

add_subdirectory(cmd)

# Add the test directory
enable_testing()
add_subdirectory(test)

message(STATUS "System is ${CMAKE_SYSTEM_NAME}")
