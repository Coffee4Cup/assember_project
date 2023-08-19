cmake_minimum_required(VERSION 3.25)
project(asmbler_2 C)

set(CMAKE_C_STANDARD 90)

add_executable(asmbler_2 main.c assmbler.c assmbler.h lookup_table.c lookup_table.h main.c macro.h macro.c)
