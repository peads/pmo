# Example 4
## Build
1. `mkdir build-generator && cd build-generator`
1. For clang-cl, or msvc respectively:
   - `cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="/clang:-m64 /clang:-O3 /clang:-march=native /clang:-flto /EHsc" -DCMAKE_EXE_LINKER_FLAGS="/LTCG /DEBUG:NONE /EMITPOGOPHASEINFO" -DCMAKE_SHARED_LINKER_FLAGS="/LTCG /DEBUG:NONE /EMITPOGOPHASEINFO" -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl -DGENERATE_BOOTSTRAP=ON .. && cmake --build . --clean-first`
   - `cmake -Ax64 -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="/O2 /Ob2 /Ot /Oy /Oi /GL /Gy /Gm- /MP /EHsc" -DCMAKE_EXE_LINKER_FLAGS="/LTCG /DEBUG:NONE /EMITPOGOPHASEINFO" -DCMAKE_SHARED_LINKER_FLAGS="/LTCG /DEBUG:NONE /EMITPOGOPHASEINFO" .. && cmake --build-generator . --clean-first -- /p:Configuration=Release /p:Platform=x64`
1. Run the generator `mkdir build-bootstrapper && ./build-generator/bootstrapper.exe`
1. `cd build-bootstrapper`
1. For clang-cl, or msvc respectively:
   - `cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="/clang:-m64 /clang:-O3 /clang:-march=native /clang:-flto /EHsc" -DCMAKE_EXE_LINKER_FLAGS="/LTCG /DEBUG:NONE /EMITPOGOPHASEINFO" -DCMAKE_SHARED_LINKER_FLAGS="/LTCG /DEBUG:NONE /EMITPOGOPHASEINFO" -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl  -DGENERATE_TEST=ON .. && cmake --build . --clean-first`
   - `cmake -Ax64 -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="/O2 /Ob2 /Ot /Oy /Oi /GL /Gy /Gm- /MP /EHsc" -DCMAKE_EXE_LINKER_FLAGS="/LTCG /DEBUG:NONE /EMITPOGOPHASEINFO" -DCMAKE_SHARED_LINKER_FLAGS="/LTCG /DEBUG:NONE /EMITPOGOPHASEINFO" .. && cmake --build-bootstrapper . --clean-first -- /p:Configuration=Release /p:Platform=x64`
#### Or just use the script `./build.bat`