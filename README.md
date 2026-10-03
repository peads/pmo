# Use
1. Download the headers
   - Get one of the archives from the Releases section and decompress it somehwere.
   - Clone the repo at--preferably at the most current release--tag.
1. Point your compiler at the directory in which you stored the headers.
   - e.g., `target_include_directories(${TARGET} PRIVATE /path/to/pmo/include)` for CMake.
1. Check examples and test directories to get started.

# Build and run (Windows) tests
1. clone the repo like you do.
1. `mkdir build && cd build`
1.  - **clang-cl with ninja**
      - `cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl .. && cmake --build . --clean-first`
    - **clang-cl with vs2022**
      - `cmake -G "Visual Studio 17 2022" -T ClangCL .. && cmake --build . --clean-first -- /p:Configuration=Release /p:Platform=x64`
    - **msys2/gcc with ninja**
      - `cmake -G Ninja -DCMAKE_AR=$(which gcc-ar) -DCMAKE_RANLIB=$(which gcc-ranlib) .. && cmake --build . --clean-first`
1. (Optional) If you happen to have Oblivion Remastered, use `-DTEST_OBR=ON` to enable an optional test to cover the Windows-specific external memory modification paths using the asm from the mod I made (https://github.com/peads/QuickSleepWait) that inspired the creation of this library.
1. `/path/to/built/exe/pmo.exe -d yes --order lex`

# Build (Windows) examples
1. `cd /path/to/pmo/examples/windows && mkdir build && cd build`
1. `cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl .. && cmake --build . --clean-first`
1. run whichever example(s) you'd like.
