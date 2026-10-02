# Example 4
## Build (standard)
1. `mkdir build && cd build`
1. For clang-cl, or msvc respectively:
   - `cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DGENERATE_TEST=ON -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl .. && cmake --build . --clean-first`
     - note: using clang automatically switches generator to produce gas/clang output
   - `cmake -DGENERATE_TEST=ON .. && cmake --build . --clean-first -- /p:Configuration=Release /p:Platform=x64`
     - note: using msvc automatically selects nasm for generator output
## Build (cross-compile, host: x64, target: arm64)
- note: only gcc/clang is supported for this
1. `mkdir build && cd build`
1. `cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl .. && cmake --build . --clean-first`
   - note: the lack of GENERATE_TEST definition
1. `/build/path/generator/generator.exe dwmapi.dll "/build/path/bootstrapper/build" arm64`
1. `cd /build/path/bootstrapper/build`
1. `clang-cl /EHsc /std:c++latest /c /Example4/src/path/bootstrapper/main.cpp .\dwmapi.asm --target=arm64-pc-windows-msvc -I/Example4/src/path/include -I/pmo/src/path/include`
1. `lld-link /machine:arm64 .\main.obj .\dwmapi.obj /DEF:dwmapi.def /DLL /NODEFAULTLIB:libcpmt.lib /NODEFAULTLIB:libucrt.lib '/path/to/Windows Kits/10/Lib/<SDK_VERSION>/lib/arm64/libcmt.lib' '/path/to/Windows Kits/10/Lib/<SDK_VERSION>/lib/arm64/libcpmt.lib' '/path/to/Windows Kits/10/Lib/<SDK_VERSION>/lib/arm64/libvcruntime.lib' '/path/to/Windows Kits/10/Lib/<SDK_VERSION>/ucrt/arm64/ucrt.lib'`
