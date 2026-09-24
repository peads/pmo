cmake_minimum_required(VERSION 3.29)
set(TARGET Example2)
project(${TARGET} CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${TARGET} "${CMAKE_CURRENT_SOURCE_DIR}/PrintExports.cpp")
target_include_directories(${TARGET} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../../include")
