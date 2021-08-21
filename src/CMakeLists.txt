# Copyright (c) Microsoft Corporation.
# Licensed under the MIT license.
cmake_minimum_required(VERSION 3.14)
include(FetchContent)

file(GLOB SRC CONFIGURE_DEPENDS *.cpp)
file(GLOB INC ../include/*.h ../include/*.hpp ../interface/*.h *.h *.hpp)
file(GLOB INL ../include/*.inl *.inl)

file(GLOB_RECURSE EXTERNAL_INC ../external/*.h ../external/*.hpp)

add_library(d3d9on12 SHARED ${SRC} ${INC} ${INL} ${EXTERNAL_INC} d3d9on12.rc d3d9on12.def)

FetchContent_Declare(
    d3d12translationlayer
    GIT_REPOSITORY https://github.com/microsoft/D3D12TranslationLayer.git
    GIT_TAG        d27b59d5e9b28fe001c39dafa7ab573fbcaf416c
)
FetchContent_MakeAvailable(d3d12translationlayer)

target_link_libraries(d3d9on12 d3d12translationlayer_wdk)
target_link_libraries(d3d9on12 d3d9on12_shaderconv)

if(TARGET dxbcsigner_static)
    target_link_libraries(d3d9on12 dxbcsigner_static)
else()
    message(STATUS "Adding DxbcSigner NuGet package...")
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../packages.config ${CMAKE_CURRENT_BINARY_DIR}/packages.config COPYONLY)
    add_library(dxbcsigner INTERFACE IMPORTED GLOBAL)
    set_target_properties(dxbcsigner PROPERTIES INTERFACE_LINK_LIBRARIES "${CMAKE_BINARY_DIR}/packages/Microsoft.Direct3D.DxbcSigner.1.0.0/build/native/Microsoft.Direct3D.DxbcSigner.targets")
    target_link_libraries(d3d9on12 dxbcsigner)
endif()
target_include_directories(d3d9on12
    PRIVATE ../external
    PRIVATE ../include
    PUBLIC ../interface
    PRIVATE ./)

source_group(Inlines FILES ${INL})
source_group("Header Files\\External" FILES ${EXTERNAL_INC})
