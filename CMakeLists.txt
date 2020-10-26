cmake_minimum_required(VERSION 3.17)
project(mem_allo C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(mem_allo
        mymem.c
        mymem.h)
