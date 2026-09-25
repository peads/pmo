function(populateLibs LIBS)
    set(LIBS
        "$<$<AND:$<NOT:$<CXX_COMPILER_ID:GNU>>,$<CONFIG:Debug>>:libvcruntimed.lib;libcmtd.lib;ucrtd.lib;kernel32.lib;msvcprtd.lib>"
        "$<$<NOT:$<OR:$<CONFIG:Debug>,$<CXX_COMPILER_ID:GNU>>>:libvcruntime.lib;libcmt.lib;ucrt.lib;kernel32.lib;msvcprt.lib>"
        PARENT_SCOPE)
    add_compile_definitions("$<$<NOT:$<CONFIG:Debug>>:OPTIMIZATIONS_ON>")
endfunction()
