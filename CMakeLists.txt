cmake_minimum_required(VERSION 3.29)
set(TARGET pmo)

if (NOT DEFINED GENERATE_TESTS)
    message(AUTHOR_WARNING "This is here to enable testing. The headers should be used directly by your project.")
else ()
    if (DEFINED INCLUDE_SYSWHISPERS AND INCLUDE_SYSWHISPERS)
        project(${TARGET} CXX C ASM_NASM)
    else()
        project(${TARGET} CXX)
    endif()
    set(CMAKE_CXX_STANDARD 23)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_SCRIPT_INCLUDES_PATH "${CMAKE_SOURCE_DIR}/cmake")
    include("${CMAKE_SCRIPT_INCLUDES_PATH}/populatelibs.cmake")
    set(CMAKE_C_STANDARD_LIBRARIES "" CACHE STRING "Standard libraries for C" FORCE)
    set(CMAKE_CXX_STANDARD_LIBRARIES "" CACHE STRING "Standard libraries for C++" FORCE)
    set(LIB "")
    populateLibs(LIBS)
    add_compile_definitions(WIN32_LEAN_AND_MEAN)
    add_link_options("$<$<NOT:$<CXX_COMPILER_ID:GNU>>:/NODEFAULTLIB;/MACHINE:X64;/subsystem:console>")
    add_compile_options(
        "$<$<AND:$<NOT:$<CONFIG:Debug>>,$<CXX_COMPILER_ID:GNU>,$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>>:-m64;-O3;-march=native;-flto=auto;-fomit-frame-pointer;-ffunction-sections;-fdata-sections>"
        "$<$<AND:$<NOT:$<CONFIG:Debug>>,$<CXX_COMPILER_ID:Clang>,$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>>:/clang:-m64;/clang:-O3;/clang:-march=native;/WX-;/Gw;/Gy;/clang:-flto=auto;/clang:-fomit-frame-pointer>"
        "$<$<AND:$<NOT:$<CONFIG:Debug>>,$<CXX_COMPILER_ID:MSVC>,$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>>:/O2;/Ob2;/Ot;/Oy;/Oi;/WX-;/GL;/Gy;/Gm-;/MP;-GS-;/Gw>"
    )
    add_link_options(
            "$<$<NOT:$<OR:$<CONFIG:Debug>,$<CXX_COMPILER_ID:GNU>>>:/OPT:REF;/OPT:ICF=3;/LTCG;/DEBUG:NONE;/EMITPOGOPHASEINFO>"
            "$<$<AND:$<NOT:$<CONFIG:Debug>>,$<CXX_COMPILER_ID:GNU>>:-Wl,--gc-sections>"
    )
    set(ARE_TESTS_GENERATED ${GENERATE_TESTS}) # cmake throws annoying warning if iT's NoT uSeD
    include(${CMAKE_SOURCE_DIR}/test/tests.cmake)
endif ()
