cmake_minimum_required(VERSION 3.29)
project(piggy C)
set(CMAKE_C_STANDARD 23)

add_library(piggy SHARED piggy.c)
