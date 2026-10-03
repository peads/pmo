cmake_minimum_required(VERSION 3.29)
project(bootstrapper-generator CXX C)
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    enable_language(ASM)
else()
    enable_language(ASM_NASM)
endif()
set(CMAKE_CXX_STANDARD 23)

if (CMAKE_SYSTEM_PROCESSOR MATCHES "(arm64|aarch64|ARM64)")
    add_compile_definitions(IS_ARM64=1)
else()
    add_compile_definitions(IS_ARM64=0)
endif()

if (NOT (DEFINED INCLUDE_SYSWHISPERS AND INCLUDE_SYSWHISPERS))
    add_executable(bootstrapper-generator "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
else()
    set(CMAKE_C_STANDARD_LIBRARIES "" CACHE STRING "Standard libraries for C" FORCE)
    set(CMAKE_CXX_STANDARD_LIBRARIES "" CACHE STRING "Standard libraries for C++" FORCE)
    add_executable(bootstrapper-generator
            "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
            "${SW3_SRC_DIR}/syscalls-asm.x64.asm"
            "${SW3_SRC_DIR}/syscalls.c"
    )
endif()
target_compile_definitions(bootstrapper-generator PRIVATE ASM_EXTENSION="${ASM_EXTENSION}")

target_include_directories(bootstrapper-generator PRIVATE "${PMO_INCLUDE}" "${SW3_SRC_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../include")
target_link_libraries(bootstrapper-generator PRIVATE "${LIBS}")
target_link_options(bootstrapper-generator PRIVATE "$<$<NOT:$<CXX_COMPILER_ID:GNU>>:/subsystem:console>")
set_target_properties(bootstrapper-generator PROPERTIES MSVC_RUNTIME_CHECKS "")
if (DEFINED DLL_NAME)
    add_custom_command(TARGET bootstrapper-generator
            POST_BUILD
            COMMAND "$<TARGET_FILE:bootstrapper-generator>" "${DLL_NAME}" "${OUT_PATH}" "$<$<BOOL:${IS_ARM64}>:ARM64>"
    )
endif()
