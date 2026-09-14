cmake_minimum_required(VERSION 3.29)
set(TARGET Example3)
project(${TARGET} CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(${TARGET} SHARED "${CMAKE_CURRENT_SOURCE_DIR}/qswprime.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/qswprime.hpp")
target_include_directories(${TARGET} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../../include")
target_compile_definitions(${TARGET} PRIVATE "BUILD_SHARED_LIB=ON")

# build with something like:
#
# `mkdir build && cd build \
#   cmake -G Ninja -DCMAKE_CXX_FLAGS="/clang:-m64 /clang:-O3 /clang:-march=native /WX- /EHsc- /Gy /clang:-flto" \
#   -DCMAKE_SHARED_LINKER_FLAGS="/LTCG /DEBUG:NONE /EMITPOGOPHASEINFO"" -DCMAKE_C_COMPILER=clang-cl -DCMAKE_BUILD_TYPE=Release \
#   -DCMAKE_CXX_COMPILER=clang-cl && cmake --build .`
#
# or if you're a heathen:
#
# `mkdir build && cd build \
#   && cmake -G Ninja -DCMAKE_CXX_FLAGS="/O2 /Ob2 /Ot /Oy /Oi /WX- /EHsc- /GL /Gy /Gm-" \
#   -DCMAKE_SHARED_LINKER_FLAGS="/LTCG /DEBUG:NONE /EMITPOGOPHASEINFO" .. \
#   && cmake --build . --config Release`
#
# or if you're truly lost to us:
#
# `cmake -Ax64 -DCMAKE_CXX_FLAGS="/O2 /Ob2 /Ot /Oy /Oi /WX- /EHsc- /GL /Gy /Gm-" \
#   -DCMAKE_SHARED_LINKER_FLAGS="/LTCG /DEBUG:NONE /EMITPOGOPHASEINFO" .. \
#   && cmake --build . --config Release`