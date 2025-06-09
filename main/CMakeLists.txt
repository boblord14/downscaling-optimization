cmake_minimum_required(VERSION 3.27)
project(downscaling-optimization)

set(CMAKE_CXX_STANDARD 17)

add_executable(downscaling-optimization main.cpp
        DataParser.cpp
        Weapon.h
        Weapon.cpp
)
