cmake_minimum_required(VERSION 3.29)
set(TARGET pmo)
project(${TARGET} CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT DEFINED GENERATE_TESTS)
    message(STATUS "This is only here to enable testing. There's no need to compile the library to use its headers in
        your project.")
else()
    if(GENERATE_PCH)
        set(IS_PCH_GENERATED ${GENERATE_PCH})
    endif()
    set(ARE_TESTS_GENERATED ${GENERATE_TESTS}) # cmake throws annoying warning if iT's NoT uSeD
    include(${CMAKE_SOURCE_DIR}/test/tests.cmake)
endif()
