find_package(Git REQUIRED)
include(FetchContent)

FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.8.1 # or a later release
)

FetchContent_MakeAvailable(Catch2)
include(CTest)
include(Catch)

add_executable(${TARGET} "${CMAKE_SOURCE_DIR}/test/src/windows/main.cpp")

target_include_directories(${TARGET} PUBLIC
        "${CMAKE_SOURCE_DIR}/include"
        "${CMAKE_SOURCE_DIR}/test/include"
        "$<$<PLATFORM_ID:Windows>:${CMAKE_SOURCE_DIR}/include/windows>"
        "$<$<PLATFORM_ID:Windows>:${CMAKE_SOURCE_DIR}/test/include/windows>"
        "$<$<PLATFORM_ID:Windows>:${CMAKE_SOURCE_DIR}/examples/windows/helpers/debug>"
)

target_link_libraries(${TARGET} PRIVATE Catch2::Catch2WithMain)
catch_discover_tests(${TARGET})

if(TEST_OBR)
    target_compile_definitions(${TARGET} PRIVATE "TEST_OBR=${TEST_OBR}")
endif()
set_target_properties(${TARGET} PROPERTIES
        MSVC_RUNTIME_CHECKS ""
)
