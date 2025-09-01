cmake_minimum_required(VERSION 4.1)
project(downscaling-optimization)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)

find_package(Ceres REQUIRED)
add_executable(downscaling-optimization main.cpp
        DataParser.cpp
        Weapon.h
        Weapon.cpp
        bnb.h
        svd.h
        db.h
        loadCharacter.cpp
        loadCharacter.h
)
target_link_libraries(downscaling-optimization Ceres::ceres)
