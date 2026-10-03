find_package(Git REQUIRED)
include(FetchContent)

set(PMO_SOURCES
        "${CMAKE_SOURCE_DIR}/test/src/windows/main.cpp"
        "${CMAKE_SOURCE_DIR}/examples/windows/helpers/debug/debug.cpp"
)
set(PMO_INCLUDES
        "${CMAKE_SOURCE_DIR}/include"
        "${CMAKE_SOURCE_DIR}/test/include"
        "${CMAKE_SOURCE_DIR}/examples"
)

FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.8.1 # or a later release
)

if (DEFINED INCLUDE_SYSWHISPERS AND INCLUDE_SYSWHISPERS)
    set(SW3_SRC_DIR "${CMAKE_CURRENT_BINARY_DIR}/_deps/syswhispers3-src")
    set(SW3_BIN_DIR "${CMAKE_CURRENT_BINARY_DIR}/_deps/syswhispers3-build")
    set(SW3_SUB_DIR "${CMAKE_CURRENT_BINARY_DIR}/_deps/syswhispers3-subbuild")

    FetchContent_Populate(
            syswhispers3
            GIT_REPOSITORY https://github.com/klezVirus/SysWhispers3.git
            SOURCE_DIR ${SW3_SRC_DIR}
            BINARY_DIR ${SW3_BIN_DIR}
            SUBBUILD_DIR ${SW3_SUB_DIR}
    )
    configure_file(
            "${CMAKE_SOURCE_DIR}/examples/windows/bootstrapper/syswhispers3.cmake"
            "${SW3_SRC_DIR}/CMakeLists.txt"
            COPYONLY
    )
    set(PMO_SOURCES "${PMO_SOURCES}" "${SW3_SRC_DIR}/syscalls-asm.x64.asm" "${SW3_SRC_DIR}/syscalls.c")
    set(PMO_INCLUDES "${PMO_INCLUDES}" "${SW3_SRC_DIR}")
endif()

FetchContent_MakeAvailable(Catch2)
set(Example4_SOURCE_DIR "${CMAKE_SOURCE_DIR}/examples/windows/bootstrapper")
include(CTest)
include(Catch)

add_executable(${TARGET} ${PMO_SOURCES})

target_compile_options(${TARGET} PRIVATE
        "$<$<AND:$<NOT:$<CONFIG:Debug>>,$<CXX_COMPILER_ID:GNU>,$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>>:-O3;-flto=auto;-fuse-linker-plugin;-fomit-frame-pointer;-ffunction-sections;-fdata-sections;-mconsole;-Wl,--gc-sections>"
        "$<$<AND:$<NOT:$<CONFIG:Debug>>,$<CXX_COMPILER_ID:Clang>,$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>>:/EHsc;/clang:-O3;/Gw;/Gy;/clang:-flto=auto;/clang:-fomit-frame-pointer>"
        "$<$<AND:$<NOT:$<CONFIG:Debug>>,$<CXX_COMPILER_ID:MSVC>,$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>>:-nologo;/EHsc;/O2;/Ob3;/Ot;/Oy;/Oi;/GL;/Gy;/Gm-;/MP;-GS-;/Gw>"
)
target_link_options(${TARGET} PRIVATE
        "$<$<NOT:$<CXX_COMPILER_ID:GNU>>:/subsystem:console>"
        "$<$<NOT:$<OR:$<CONFIG:Debug>,$<CXX_COMPILER_ID:GNU>>>:/NODEFAULTLIB;/OPT:REF;/OPT:ICF=3;/LTCG;/DEBUG:NONE;/EMITPOGOPHASEINFO>"
)
target_link_libraries(Catch2 PRIVATE "$<$<NOT:$<CXX_COMPILER_ID:GNU>>:${LIBS};bufferoverflowU.lib>")
target_link_libraries(${TARGET} PRIVATE Catch2::Catch2WithMain)

string(REPLACE "/RTC1" "" MY_CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_DEBUG "${MY_CMAKE_CXX_FLAGS_DEBUG}" CACHE STRING "CXX Debug Flags" FORCE)
target_compile_options(${TARGET} PRIVATE "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Debug>>:/RTC1>")

target_include_directories(${TARGET} PUBLIC
        "${CMAKE_SOURCE_DIR}/include"
        "${CMAKE_SOURCE_DIR}/test/include"
        "${CMAKE_SOURCE_DIR}/examples"
)
target_link_options(Catch2 PRIVATE
        "$<$<NOT:$<CXX_COMPILER_ID:GNU>>:/NODEFAULTLIB;/subsystem:console;/OPT:REF;/OPT:ICF=3;/LTCG;/DEBUG:NONE;/EMITPOGOPHASEINFO>"
)
target_compile_options(Catch2 PRIVATE
        "$<$<AND:$<CXX_COMPILER_ID:GNU>,$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>>:-O3;-flto=auto;-fuse-linker-plugin;-fomit-frame-pointer;-ffunction-sections;-fdata-sections;-Wl,--gc-sections>"
        "$<$<AND:$<CXX_COMPILER_ID:Clang>,$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>>:/EHsc;/clang:-O3;/Gw;/Gy;/clang:-flto=auto;/clang:-fomit-frame-pointer>"
        "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>>:-nologo;/EHsc;/O2;/Ob3;/Ot;/Oy;/Oi;/GL;/Gy;/Gm-;/MP;-GS-;/Gw>"
)

catch_discover_tests(${TARGET} DISCOVERY_MODE PRE_TEST)

if (DEFINED INCLUDE_SYSWHISPERS AND INCLUDE_SYSWHISPERS)
    add_subdirectory(${SW3_SRC_DIR} ${SW3_BIN_DIR})
endif()

if(TEST_OBR)
    target_compile_definitions(${TARGET} PRIVATE "TEST_OBR=${TEST_OBR}")
endif()
set_target_properties(${TARGET} PROPERTIES MSVC_RUNTIME_CHECKS "")
