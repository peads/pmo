cmake_minimum_required(VERSION 3.29)
project(bootstrapper-generator CXX C ASM_NASM)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD_LIBRARIES "" CACHE STRING "Standard libraries for C" FORCE)
set(CMAKE_CXX_STANDARD_LIBRARIES "" CACHE STRING "Standard libraries for C++" FORCE)

add_executable(bootstrapper-generator
        "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
        "${SW3_SRC_DIR}/syscalls-asm.x64.asm"
        "${SW3_SRC_DIR}/syscalls.c"
)
set(LIBS "")
populateLibs(LIBS)
target_include_directories(bootstrapper-generator PRIVATE "${PMO_INCLUDE}" "${SW3_SRC_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../include")
target_link_libraries(bootstrapper-generator PRIVATE "${LIBS}")
target_link_options(bootstrapper-generator PRIVATE "/NODEFAULTLIB" "/MACHINE:X64" "/subsystem:console")
set_target_properties(bootstrapper-generator PROPERTIES MSVC_RUNTIME_CHECKS "")
if (DEFINED DLL_NAME)
    add_custom_command(TARGET bootstrapper-generator
            POST_BUILD
            COMMAND "$<TARGET_FILE:bootstrapper-generator>" "${DLL_NAME}" "${OUT_PATH}"
    )
endif()
