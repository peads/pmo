# Windows Examples
## Build
1. `mkdir build && cd build`
1. For clang-cl, or msvc respectively:
   - `cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="/EHsc" -DGENERATE_TEST=ON -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl .. && cmake --build . --clean-first`
     - It would be worthwhile to consider adding `/clang:-O3 /clang:-march=native` among other desired optimization flags to the configuration step
   - `cmake -Ax64 -DCMAKE_CXX_FLAGS="/EHsc" -DCMAKE_BUILD_TYPE=Release -DGENERATE_TEST=ON .. && cmake --build . --clean-first -- /p:Configuration=Release /p:Platform=x64`
       - It would be worthwhile to consider adding `/MP /GS-` among other desired optimization flags to the configuration step
