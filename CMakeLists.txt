
cmake_minimum_required(VERSION 4.0)

project(axc
        VERSION 0.1
        DESCRIPTION "AXC C Compiler"
        LANGUAGES CXX)

if(${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD")
    set(CMAKE_CXX_COMPILER "/usr/local/llvm20/bin/clang++" CACHE STRING "" FORCE)
endif()
if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    set(CMAKE_CXX_COMPILER "/opt/homebrew/opt/llvm/bin/clang++" CACHE STRING "" FORCE)
    # Set flags for C++ standard library and unwind library, for LLVM installed via Homebrew
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L/opt/homebrew/opt/llvm/lib/c++ -L/opt/homebrew/opt/llvm/lib/unwind -lunwind")
endif()
message(STATUS "C++ compiler = ${CMAKE_CXX_COMPILER}")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(cmake/PreventInSourceBuilds.cmake)
include(cmake/CPM.cmake)

add_library(project_options INTERFACE)

# sanitizer options if supported by compiler
set(ENABLE_SANITIZER_ADDRESS ON)
set(ENABLE_SANITIZER_UNDEFINED_BEHAVIOR ON)
include(cmake/Sanitizers.cmake)
enable_sanitizers(project_options)

# Dependency management
include(Dependencies.cmake)

# install
set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})

add_subdirectory(src)
# Add the machine code generation directory
add_subdirectory(src/machine/x86_64)
add_subdirectory(src/machine/arm64)

add_subdirectory(cmd)

# Add the test directory
enable_testing()
add_subdirectory(test)

message(STATUS "System is ${CMAKE_SYSTEM_NAME}")
