rmdir build-generator -Recurse -Force && rmdir build-bootstrapper -Recurse -Force
mkdir build-generator
cd build-generator && ^
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ^
  -DCMAKE_CXX_FLAGS="/clang:-m64 /clang:-O3 /clang:-march=native /clang:-flto /EHsc" ^
  -DCMAKE_EXE_LINKER_FLAGS="/LTCG /DEBUG:NONE /EMITPOGOPHASEINFO" ^
  -DCMAKE_SHARED_LINKER_FLAGS="/LTCG /DEBUG:NONE /EMITPOGOPHASEINFO" ^
  -DCMAKE_C_COMPILER=clang-cl ^
  -DCMAKE_CXX_COMPILER=clang-cl ^
  -DGENERATE_BOOTSTRAP=ON .. && cmake --build . --clean-first && cd ..
mkdir build-bootstrapper && .\build-generator\bootstrapper.exe
cd build-bootstrapper && ^
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ^
  -DCMAKE_CXX_FLAGS="/clang:-m64 /clang:-O3 /clang:-march=native /clang:-flto /EHsc" ^
  -DCMAKE_EXE_LINKER_FLAGS="/LTCG /DEBUG:NONE /EMITPOGOPHASEINFO" ^
  -DCMAKE_SHARED_LINKER_FLAGS="/LTCG /DEBUG:NONE /EMITPOGOPHASEINFO" ^
  -DCMAKE_C_COMPILER=clang-cl ^
  -DCMAKE_CXX_COMPILER=clang-cl ^
  -DGENERATE_TEST=ON .. && cmake --build . --clean-first
