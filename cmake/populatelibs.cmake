function(populateLibs LIBS)

    set(LIBS
            "$<$<CONFIG:Debug>:libvcruntimed.lib;libcmtd.lib;ucrtd.lib;kernel32.lib;msvcprtd.lib>"
            "$<$<NOT:$<CONFIG:Debug>>:libvcruntime.lib;libcmt.lib;ucrt.lib;kernel32.lib;msvcprt.lib>"
            PARENT_SCOPE)
    add_compile_definitions("$<$<NOT:$<CONFIG:Debug>>:OPTIMIZATIONS_ON>")
endfunction()
