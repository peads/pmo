cmake_minimum_required(VERSION 3.29)
set(TARGET Example3)
project(${TARGET} CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#find_package(Git REQUIRED)
#include(FetchContent)

#FetchContent_Declare(
#        zydis
#        GIT_REPOSITORY https://github.com/zyantific/zydis.git
#        GIT_TAG v4.1.1
#        GIT_SHALLOW TRUE
#        GIT_PROGRESS OFF
##        SOURCE_DIR ZYDIS_SRC_DIR
##        BINARY_DIR ZYDIS_BIN_DIR
#)
#FetchContent_MakeAvailable(zydis)
#target_compile_definitions(Zydis PRIVATE
#        ZYDIS_STATIC_BUILD=OFF
#        ZYDIS_BUILD_SHARED_LIB=ON
#        ZYDIS_BUILD_TOOLS=OFF
#        ZYDIS_BUILD_EXAMPLES=OFF
#)
add_library(${TARGET} SHARED "${CMAKE_CURRENT_SOURCE_DIR}/qswprime.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/qswprime.hpp")
target_include_directories(${TARGET} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../include" ZYDIS_SRC_DIR)
target_compile_definitions(${TARGET} PRIVATE "$<$<CONFIG:Debug>:IS_DEBUG=1>")
