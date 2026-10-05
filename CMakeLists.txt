cmake_minimum_required(VERSION 3.29)

if (NOT DEFINED GENERATE_TESTS)
    message(AUTHOR_WARNING "This is here to enable testing. The headers should be used directly by your project.")
else ()
    set(TARGET pmo)
    project(${TARGET} CXX)

    set(CMAKE_CXX_STANDARD 23)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)

    add_compile_definitions("$<$<NOT:$<CONFIG:Debug>>:OPTIMIZATIONS_ON>")
    add_compile_definitions(WIN32_LEAN_AND_MEAN)
    set(ARE_TESTS_GENERATED ${GENERATE_TESTS}) # cmake throws annoying warning if iT's NoT uSeD
    include(${CMAKE_SOURCE_DIR}/test/tests.cmake)
endif ()
