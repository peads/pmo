cmake_minimum_required(VERSION 3.29)
project(bootstrapper CXX C)
set(CMAKE_CXX_STANDARD 23)
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    enable_language(ASM)
else()
    enable_language(ASM_NASM)
endif()

if (NOT EXISTS "${OUT_PATH}/bootstrap.txt")
    message(WARNING "Please build and run the generator half first.")
else ()
    file(STRINGS "${OUT_PATH}/bootstrap.txt" FILE_STEM)
    list(GET FILE_STEM 0 FILE_STEM)

    add_library(${FILE_STEM} SHARED
            "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
            "${OUT_PATH}/${FILE_STEM}${ASM_EXTENSION}"
            "${OUT_PATH}/${FILE_STEM}.def"
    )
    add_dependencies(${FILE_STEM} bootstrapper-generator)
    target_include_directories(${FILE_STEM} PRIVATE "${PMO_INCLUDE}" "${CMAKE_CURRENT_SOURCE_DIR}/../include")
#    if(MSVC)
#        set_target_properties(${FILE_STEM} PROPERTIES LINK_FLAGS "/DEF:\"${OUT_PATH}/${FILE_STEM}.def\"")
#    endif()
    set_target_properties(${FILE_STEM} PROPERTIES OUTPUT_NAME "${FILE_STEM}" PREFIX "")
    target_compile_definitions(${FILE_STEM} PRIVATE OUT_PATH="${OUT_PATH}")
    set_target_properties(${FILE_STEM} PROPERTIES MSVC_RUNTIME_CHECKS "")
    if (DEFINED GENERATE_TEST)
        add_compile_definitions(DLL_NAME="${CMAKE_CURRENT_BINARY_DIR}/${DLL_NAME}")
        add_subdirectory(piggy)
        add_custom_command(
                TARGET ${FILE_STEM}
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                    "$<TARGET_FILE:${FILE_STEM}>"
                    "${CMAKE_CURRENT_BINARY_DIR}/piggy"
                COMMENT "Copying build output to piggy"
        )
    endif()
endif()
