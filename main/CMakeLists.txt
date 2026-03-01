cmake_minimum_required(VERSION 3.20)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
project(downscaling-optimization)

set(CMAKE_CXX_STANDARD 17)
include_directories(include)
include_directories(lib)
link_directories(lib)

# NLopt and nlohmann are required to be installed via vcpkg
find_package(NLopt CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(Eigen3 CONFIG REQUIRED)
find_package(onnxruntime CONFIG REQUIRED)

#requires onnx's files in main/include

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

set_property(TARGET downscaling-optimization
        PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

#additionally requires the tensorflow.lib file installed to main/lib from here:
#https://www.tensorflow.org/install/lang_c
target_link_libraries(downscaling-optimization
        PRIVATE
        onnxruntime::onnxruntime
        NLopt::nlopt
        nlohmann_json::nlohmann_json
        Eigen3::Eigen
)