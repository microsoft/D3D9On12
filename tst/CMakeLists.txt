# Copyright (c) Microsoft Corporation.
# Licensed under the MIT license.
cmake_minimum_required(VERSION 3.14)
include(FetchContent)

#This is the SHA for the tag release-1.11.0
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest/
    GIT_TAG        e2239ee6043f73722e7aa812a459f54a28552929
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
set_target_properties(gtest PROPERTIES FOLDER "Dependencies")
set_target_properties(gtest_main PROPERTIES FOLDER "Dependencies")
set_target_properties(gmock PROPERTIES FOLDER "Dependencies")
set_target_properties(gmock_main PROPERTIES FOLDER "Dependencies")

set(TEST_BINARY ${PROJECT_NAME}_tst)

file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false *.cpp)
file(GLOB INC ./include/*.h ./include/*.hpp *.h *.hpp)
file(GLOB MOCKS ./Mocks/*.h)

set(SOURCES ${TEST_SOURCES})

add_executable(${TEST_BINARY} ${TEST_SOURCES} ${INC} ${MOCKS})

add_test(NAME ${TEST_BINARY} COMMAND ${TEST_BINARY})

target_link_libraries(${TEST_BINARY} PUBLIC ${PROJECT_NAME} gtest gmock)
target_include_directories(${TEST_BINARY}
    PRIVATE ../include
    PRIVATE ../interface
    PRIVATE ../external
    PRIVATE ./Mocks
    PRIVATE ./
    PRIVATE ./include
)

source_group(Mocks FILES ${MOCKS})