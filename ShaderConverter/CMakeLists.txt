# Copyright (c) Microsoft Corporation.
# Licensed under the MIT license.
cmake_minimum_required(VERSION 3.13)

file(GLOB SRC CONFIGURE_DEPENDS ShaderConv/*.cpp ShaderBinary/*.cpp)
file(GLOB INC Inc/*.h Inc/*.hpp ShaderConv/*.h ShaderConv/*.hpp)
file(GLOB_RECURSE EXTERNAL_INC ../external/*.h ../external/*.hpp)

add_library(d3d9on12_shaderconv STATIC ${SRC} ${INC} ${EXTERNAL_INC})
target_include_directories(d3d9on12_shaderconv
    PUBLIC Inc
    PUBLIC ../external
    PRIVATE ShaderConv)
source_group("Header Files\\External" FILES ${EXTERNAL_INC})

target_link_libraries(d3d9on12_shaderconv d3d12translationlayer)

if(MSVC)
    #This is the same set of options provided to d3d12translationlayer + /wd4201
    #/wd4201 disables the warning for "nonstandard extension used : nameless struct/union"
    target_compile_options(d3d9on12_shaderconv PRIVATE /W4 /WX /wd4238 /wd4324 /wd4201)
endif()