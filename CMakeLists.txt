cmake_minimum_required(VERSION 3.29)
set(TARGET pmo)
project(${TARGET} CXX C ASM_NASM)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT DEFINED GENERATE_TESTS)
    message(AUTHOR_WARNING "This is here to enable testing. The headers should be used directly by your project.")
else ()
    set(CMAKE_SCRIPT_INCLUDES_PATH "${CMAKE_SOURCE_DIR}/cmake")
    include("${CMAKE_SCRIPT_INCLUDES_PATH}/populatelibs.cmake")
    set(CMAKE_C_STANDARD_LIBRARIES "" CACHE STRING "Standard libraries for C" FORCE)
    set(CMAKE_CXX_STANDARD_LIBRARIES "" CACHE STRING "Standard libraries for C++" FORCE)
    string(TOUPPER "${CMAKE_BUILD_TYPE}" uppercase_CMAKE_BUILD_TYPE)
#    set(LIBS "libvcruntimed.lib" "libcmtd.lib" "ucrtd.lib" "kernel32.lib" "msvcprtd.lib")
    set(LIB "")
    populateLibs(LIBS)
    add_link_options(/NODEFAULTLIB /MACHINE:X64 /subsystem:console)
    add_compile_definitions(WIN32_LEAN_AND_MEAN)
    add_compile_options(
            "$<$<AND:$<NOT:$<CONFIG:Debug>>,$<CXX_COMPILER_ID:Clang>,$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>>:-Zp8;/clang:-m64;/clang:-O3;/clang:-march=native;/WX-;/Gy;/clang:-flto;/clang:-fomit-frame-pointer>"
            "$<$<AND:$<NOT:$<CONFIG:Debug>>,$<CXX_COMPILER_ID:MSVC>,$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>>:-Zp8;/O2;/Ox;/Ob2;/Ot;/Oy;/Oi;/WX-;/GL;/Gy;/Gm-;/MP>"
            "$<$<AND:$<NOT:$<CONFIG:Debug>>,$<COMPILE_LANGUAGE:C>>:-GS>"
    )
    add_link_options("$<$<NOT:$<CONFIG:Debug>>:/OPT:REF;/OPT:ICF=3;/LTCG;/DEBUG:NONE;/EMITPOGOPHASEINFO>")
    set(ARE_TESTS_GENERATED ${GENERATE_TESTS}) # cmake throws annoying warning if iT's NoT uSeD
    include(${CMAKE_SOURCE_DIR}/test/tests.cmake)
endif ()
