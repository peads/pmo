find_package(Git REQUIRED)
include(FetchContent)
include(CheckCXXCompilerFlag)

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
endif()

if (NOT DEFINED BMI2_FLAG OR DISABLE_BMI2_SUPPORT)
    add_executable(${TARGET} "${CMAKE_SOURCE_DIR}/test/main.cpp")
else()
    add_executable(${TARGET} "${CMAKE_SOURCE_DIR}/test/main.cpp" "${CMAKE_SOURCE_DIR}/src/bmi2.asm")
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