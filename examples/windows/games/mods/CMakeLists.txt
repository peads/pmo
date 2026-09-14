cmake_minimum_required(VERSION 3.29)
set(TARGET Example3)
project(${TARGET} CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(${TARGET} SHARED "${CMAKE_CURRENT_SOURCE_DIR}/qswprime.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/qswprime.hpp")
target_include_directories(${TARGET} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../../include")
target_compile_definitions(${TARGET} PRIVATE "BUILD_SHARED_LIB=ON")
