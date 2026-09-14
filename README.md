# Use
1. Download the headers
   - Get one of the archives from the Releases section and decompress it somehwere.
   - Clone the repo at--preferably at the most current release--tag.
1. Point your compiler at the directory in which you stored the headers.
   - e.g., `target_include_directories(${TARGET} PRIVATE /path/to/pmo/include)` for CMake.
1. ????
1. PROFIT
1. Check examples and test directories to get started.
# Build and run tests
1. clone the repo like you do.
1. `mkdir build && cd build`
1. For clang-cl, or msvc respectively:
    - `cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl .. && cmake --build . --clean-first`
    - `cmake -DCMAKE_BUILD_TYPE=Release .. && cmake --build . --clean-first -- /p:Configuration=Release /p:Platform=x64`
1. (Optional) If you happen to have Oblivion Remastered, use `-DTEST_OBR=ON` to enable an optional test to cover the Windows-specific external memory modification paths using the asm from the mod I made (https://github.com/peads/QuickSleepWait) that inspired the creation of this library.  
