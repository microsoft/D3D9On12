# Copyright (c) Microsoft Corporation.
# Licensed under the MIT license.
cmake_minimum_required(VERSION 3.13)
project(d3d9on12)
option(BUILD_TESTS "This is settable from the command line" ON)
set_property(GLOBAL PROPERTY USE_FOLDERS TRUE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(ShaderConverter)
add_subdirectory(src)

if(BUILD_TESTS)
    add_subdirectory(tst)
endif()