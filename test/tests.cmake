find_package(Git REQUIRED)
include(FetchContent)
include(CheckCXXCompilerFlag)

FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.8.1 # or a later release
)

FetchContent_MakeAvailable(Catch2)
include(CTest)
include(ParseAndAddCatchTests)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    check_cxx_compiler_flag("/arch:AVX2" COMPILER_SUPPORTS_BMI2)
    if(COMPILER_SUPPORTS_BMI2)
        set(BMI2_FLAG "/arch:AVX2")
    endif()
else()
    check_cxx_compiler_flag("-mbmi2" COMPILER_SUPPORTS_BMI2)
    if(COMPILER_SUPPORTS_BMI2)
        set(BMI2_FLAG "/clang:-mbmi2")
    endif()
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /clang:--coverage /clang:-fprofile-arcs /clang:-ftest-coverage /clang:-fprofile-instr-generate /clang:-fcoverage-mapping /MTd /clang:-gcodeview")
endif()

add_executable(${TARGET} "${CMAKE_SOURCE_DIR}/test/main.cpp")
if (DEFINED BMI2_FLAG)
#        add_executable(${TARGET} "${CMAKE_SOURCE_DIR}/test/main.cpp" "${CMAKE_SOURCE_DIR}/src/bmi2.asm")
    target_compile_options(${TARGET} PUBLIC "$<$<COMPILE_LANGUAGE:CXX>:${BMI2_FLAG}>")
endif()

if(NOT IS_PCH_GENERATED)
    target_include_directories(${TARGET} PUBLIC
            "${CMAKE_SOURCE_DIR}/include"
            "$<$<PLATFORM_ID:Windows>:${CMAKE_SOURCE_DIR}/include/windows>"
    )
else()
    target_precompile_headers(${TARGET} PUBLIC
            "${CMAKE_SOURCE_DIR}/include/bmi2.hpp"
            "${CMAKE_SOURCE_DIR}/include/types/ImportInfo.hpp"
            "${CMAKE_SOURCE_DIR}/include/types/PointerUnion.hpp"
            "${CMAKE_SOURCE_DIR}/include/types/PseudoContainer.hpp"
            "${CMAKE_SOURCE_DIR}/include/MemoryOps.hpp"
            "$<$<PLATFORM_ID:Windows>:${CMAKE_SOURCE_DIR}/include/windows/ImageDirectoryEntryToData.hpp>"
            "$<$<PLATFORM_ID:Windows>:${CMAKE_SOURCE_DIR}/include/windows/MemoryOps.hpp>"
    )
endif()

target_link_libraries(${TARGET} PRIVATE Catch2::Catch2WithMain)
ParseAndAddCatchTests(${TARGET})