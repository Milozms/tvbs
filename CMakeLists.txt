cmake_minimum_required(VERSION 3.7)
project(untitled)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES tvbs_decode.cpp)
add_executable(tvbs ${SOURCE_FILES})
TARGET_LINK_LIBRARIES(tvbs pthread)