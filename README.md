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
- clone the repo like you do.
- `cd /path/to/pmo && mkdir build && cd build && cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DGENERATE_TESTS=ON .. && cmake --build . && ./pmo.exe --order lex`
  - suggested: add `-DCMAKE_CXX_COMPILER=clang-cl` if you want to use clang-cl instead of the standard msvc compiler
  - If you happen to have Oblivion Remastered, use `-DTEST_OBR=ON` to enable an optional test to cover the Windows-specific external memory modification paths using the asm from the mod I made (https://github.com/peads/QuickSleepWait) that inspired the creation of this library.  
