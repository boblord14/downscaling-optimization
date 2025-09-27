cmake_minimum_required(VERSION 3.10)
project(downscaling-optimization)

set(CMAKE_CXX_STANDARD 17)
include_directories(include)

find_package(NLopt CONFIG REQUIRED)

add_executable(downscaling-optimization main.cpp
        DataParser.cpp
        Weapon.h
        Weapon.cpp
        bnb.h
        svd.h
        db.h
        loadCharacter.cpp
        loadCharacter.h
        character.cpp
        character.h
)
target_link_libraries(downscaling-optimization PRIVATE NLopt::nlopt)
