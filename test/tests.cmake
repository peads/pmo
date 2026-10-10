find_package(Git REQUIRED)
include(FetchContent)
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "GNU")
    execute_process(
            COMMAND "which" "gcc-ar"
            OUTPUT_VARIABLE CMAKE_AR
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
            COMMAND "which" "gcc-ranlib"
            OUTPUT_VARIABLE CMAKE_RANLIB
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endif()

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

FetchContent_MakeAvailable(Catch2)
set(Example4_SOURCE_DIR "${CMAKE_SOURCE_DIR}/examples/windows/bootstrapper")
include(CTest)
include(Catch)

add_executable(${TARGET} ${PMO_SOURCES})

#target_link_libraries(Catch2 PRIVATE "$<$<NOT:$<CXX_COMPILER_ID:GNU>>:${LIBS};bufferoverflowU.lib>")
target_link_libraries(${TARGET} PRIVATE Catch2::Catch2WithMain)
target_include_directories(${TARGET} PUBLIC
        "${CMAKE_SOURCE_DIR}/include"
        "${CMAKE_SOURCE_DIR}/test/include"
        "${CMAKE_SOURCE_DIR}/examples"
)

catch_discover_tests(${TARGET} DISCOVERY_MODE PRE_TEST EXTRA_ARGS "--order" "lex")

target_compile_definitions(${TARGET} PRIVATE "$<$<CONFIG:Debug>:IS_DEBUG=1>")
if(TEST_OBR)
    target_compile_definitions(${TARGET} PRIVATE "TEST_OBR=${TEST_OBR}")
endif()
set_target_properties(${TARGET} PROPERTIES MSVC_RUNTIME_CHECKS "")
if(DEFINED IS_DEPLOYMENT)
    target_compile_definitions(${TARGET} PRIVATE IS_DEPLOYMENT=${IS_DEPLOYMENT})
endif()
