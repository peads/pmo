function(populateLibs LIBS)

    set(LIBS
            "$<$<CONFIG:Debug>:libvcruntimed.lib;libcmtd.lib;ucrtd.lib;kernel32.lib;msvcprtd.lib>"
            "$<$<NOT:$<CONFIG:Debug>>:libvcruntime.lib;libcmt.lib;ucrt.lib;kernel32.lib;msvcprt.lib>"
            PARENT_SCOPE)
    add_compile_definitions("$<$<NOT:$<CONFIG:Debug>>:OPTIMIZATIONS_ON>")
#    set(LIBS "libvcruntimed.lib" "libcmtd.lib" "ucrtd.lib" "kernel32.lib" "msvcprtd.lib" PARENT_SCOPE)
#    if(DEFINED CMAKE_BUILD_TYPE AND CMAKE_BUILD_TYPE)
#        string(TOUPPER "${CMAKE_BUILD_TYPE}" uppercase_CMAKE_BUILD_TYPE)
#        if (NOT uppercase_CMAKE_BUILD_TYPE STREQUAL "DEBUG")
##            unset(LIBS)
##            set(LIBS "libvcruntime.lib" "libcmt.lib" "ucrt.lib" "kernel32.lib" "msvcprt.lib" PARENT_SCOPE)
#            add_compile_definitions(OPTIMIZATIONS_ON)
#        endif()
#    endif()
endfunction()
