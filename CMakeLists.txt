cmake_minimum_required(VERSION 3.21)
project(Lab3b C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "ON")

add_executable(Lab3b main.c Menu.c Table.c)
target_compile_options(Lab3b BEFORE PRIVATE -Wall)
