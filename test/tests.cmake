find_package(Git REQUIRED)
include(FetchContent)

FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.8.1 # or a later release
)
set(SW3_SRC_DIR "${CMAKE_CURRENT_BINARY_DIR}/_deps/syswhispers3-src")
set(SW3_BIN_DIR "${CMAKE_CURRENT_BINARY_DIR}/_deps/syswhispers3-build")
set(SW3_SUB_DIR "${CMAKE_CURRENT_BINARY_DIR}/_deps/syswhispers3-subbuild")

FetchContent_Populate(
        syswhispers3
        QUIET
        GIT_REPOSITORY https://github.com/klezVirus/SysWhispers3.git
        SOURCE_DIR ${SW3_SRC_DIR}
        BINARY_DIR ${SW3_BIN_DIR}
        SUBBUILD_DIR ${SW3_SUB_DIR}
)
FetchContent_MakeAvailable(Catch2)
configure_file(
        "${CMAKE_SOURCE_DIR}/examples/windows/bootstrapper/syswhispers3.cmake"
        "${SW3_SRC_DIR}/CMakeLists.txt"
        COPYONLY
)
set(Example4_SOURCE_DIR "${CMAKE_SOURCE_DIR}/examples/windows/bootstrapper")
include(CTest)
include(Catch)

add_executable(${TARGET} "${CMAKE_SOURCE_DIR}/test/src/windows/main.cpp"
        "${CMAKE_SOURCE_DIR}/examples/windows/helpers/debug/debug.cpp"
        "${SW3_SRC_DIR}/syscalls-asm.x64.asm"
        "${SW3_SRC_DIR}/syscalls.c")

#if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND MSVC AND uppercase_CMAKE_BUILD_TYPE STREQUAL "DEBUG")
##    set(MY_CXX_FLAGS "${CMAKE_CXX_FLAGS} /clang:-fcoverage-mapping /clang:-fprofile-instr-generate")
##    set(MY_C_FLAGS "${CMAKE_C_FLAGS} /clang:-fcoverage-mapping /clang:-fprofile-instr-generate")
#    set(MY_LLVM_RTS "${LLVM_ENABLE_RUNTIMES} compiler-rt")
#    set(MY_LLVM_PROJS "${LLVM_ENABLE_PROJECTS} compiler-rt")
#
#    execute_process(
#            COMMAND ${CMAKE_C_COMPILER} -print-resource-dir
#            OUTPUT_VARIABLE CLANG_RESOURCE_DIR
#            OUTPUT_STRIP_TRAILING_WHITESPACE
#    )
#    set(CLANG_RT_SUBDIR "lib/windows")
#
#    find_library(CLANG_RT_PROFILE_LIB
#            NAMES clang_rt.profile-x86_64 libclang_rt.profile-x86_64.a clang_rt.profile-x86_64.lib
#            HINTS "${CLANG_RESOURCE_DIR}"
#            PATH_SUFFIXES "${CLANG_RT_SUBDIR}"
#    )
#    target_link_libraries(${TARGET} PRIVATE ${CLANG_RT_PROFILE_LIB})
#    target_link_libraries(Catch2 PRIVATE ${CLANG_RT_PROFILE_LIB})
#endif()
#target_compile_options(${TARGET} PRIVATE "$<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:Clang>,$<COMPILE_LANGUAGE:CXX>>:/clang:-fcoverage-mapping;/clang:-fprofile-instr-generate>")
#target_compile_options(Catch2 PRIVATE "$<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:Clang>,$<COMPILE_LANGUAGE:CXX>>:/clang:-fcoverage-mapping;/clang:-fprofile-instr-generate>")

target_link_libraries(Catch2 PRIVATE "${LIBS}" "bufferoverflowU.lib")
target_link_libraries(${TARGET} PRIVATE Catch2::Catch2WithMain)

target_include_directories(${TARGET} PUBLIC
        "${CMAKE_SOURCE_DIR}/include"
        "${CMAKE_SOURCE_DIR}/test/include"
        "${CMAKE_SOURCE_DIR}/examples"
        "${SW3_SRC_DIR}"
)

catch_discover_tests(${TARGET})
add_subdirectory(${SW3_SRC_DIR} ${SW3_BIN_DIR})

if(TEST_OBR)
    target_compile_definitions(${TARGET} PRIVATE "TEST_OBR=${TEST_OBR}")
endif()
set_target_properties(${TARGET} PROPERTIES MSVC_RUNTIME_CHECKS "")
