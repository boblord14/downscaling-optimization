cmake_minimum_required(VERSION 3.20)

project(downscaling-optimization)

set(CMAKE_CXX_STANDARD 17)
include_directories(include)
include_directories(lib)

#The below 4 are required to be installed via vcpkg
find_package(NLopt CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(Eigen3 CONFIG REQUIRED)

link_directories(lib)

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

target_link_libraries(downscaling-optimization
        PRIVATE
        onnxruntime
        NLopt::nlopt
        nlohmann_json::nlohmann_json
        Eigen3::Eigen
)