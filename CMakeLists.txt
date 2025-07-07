
cmake_minimum_required(VERSION 3.31)

project(axc
        VERSION 0.1
        DESCRIPTION "AXC C Compiler"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(cmake/PreventInSourceBuilds.cmake)
include(cmake/CPM.cmake)

# Dependency management
include(Dependencies.cmake)

add_subdirectory(cmd)
