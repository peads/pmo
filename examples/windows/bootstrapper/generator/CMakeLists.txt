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

add_executable(bootstrapper-generator "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
target_compile_definitions(bootstrapper-generator PRIVATE ASM_EXTENSION="${ASM_EXTENSION}")

target_include_directories(bootstrapper-generator PRIVATE "${PMO_INCLUDE}" "${CMAKE_CURRENT_SOURCE_DIR}/../include")
set_target_properties(bootstrapper-generator PROPERTIES MSVC_RUNTIME_CHECKS "")
if (DEFINED DLL_NAME)
    add_custom_command(TARGET bootstrapper-generator
            POST_BUILD
            COMMAND "$<TARGET_FILE:bootstrapper-generator>" "${DLL_NAME}" "${OUT_PATH}" "$<$<BOOL:${IS_ARM64}>:ARM64>"
    )
endif()

if(DEFINED CROSS_COMPILE_ARM64 AND CROSS_COMPILE_ARM64)
    add_custom_command(TARGET bootstrapper-generator
            POST_BUILD
            COMMAND "${CMAKE_CXX_COMPILER}" "/EHsc" "/std:c++latest" "/c" "/GR-" "--target=arm64-pc-windows-msvc"
            "-I${PMO_INCLUDE}" "-I${CMAKE_CURRENT_SOURCE_DIR}/../include" "${CMAKE_CURRENT_SOURCE_DIR}/../bootstrapper/main.cpp"
            "${OUT_PATH}/${DLL_NAME_STEM}${ASM_EXTENSION}"
            COMMAND "${RESOLVED_LINKER_PATH}" "/machine:arm64" "main.obj" "dwmapi.obj" "/DEF:${OUT_PATH}/dwmapi.def" "/DLL"
            "/NODEFAULTLIB:libcpmt.lib" "/NODEFAULTLIB:libucrt.lib" ${paths} COMMAND_EXPAND_LISTS
    )
endif()
