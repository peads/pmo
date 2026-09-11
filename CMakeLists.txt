cmake_minimum_required(VERSION 3.29)
set(TARGET pmo)
project(${TARGET} CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT DEFINED GENERATE_TESTS)
    message(STATUS "This is only here to enable testing. There's no need to compile the library to use its headers in
        your project.")
else()
    add_compile_definitions(
        "$<$<CXX_COMPILER_ID:Clang>:CMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS} /clang:-m64 /clang:-O3 /clang:-march=native /EHsc- /WX- /Gy /clang:-flto /clang:-fomit-frame-pointer>"
        CMAKE_EXE_LINKER_FLAGS="${CMAKE_EXE_LINKER_FLAGS} /LTCG /DEBUG:NONE /EMITPOGOPHASEINFO"
        CMAKE_SHARED_LINKER_FLAGS="${CMAKE_SHARED_LINKER_FLAGS} /LTCG /DEBUG:NONE /EMITPOGOPHASEINFO"
        WIN32_LEAN_AND_MEAN
    )
    add_compile_definitions($<$<CONFIG:Release>:OPTIMIZATIONS_ON>)
    set(ARE_TESTS_GENERATED ${GENERATE_TESTS}) # cmake throws annoying warning if iT's NoT uSeD
    include(${CMAKE_SOURCE_DIR}/test/tests.cmake)
endif()
