cmake_minimum_required(VERSION 3.10)
project(downscaling-optimization)

set(CMAKE_CXX_STANDARD 17)
include_directories(include)
include_directories(lib)
link_directories(lib)

# NLopt and nlohmann are required to be installed via vcpkg
find_package(NLopt CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

#requires the following installed in main/include
#cppflow https://github.com/serizba/cppflow
#tensorflow https://www.tensorflow.org/install/lang_c
#eigen https://libeigen.gitlab.io/
#a note on tensorflow- the tensorflow lib needs to be on path, cppflow throws a fit otherwise

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

#additionally requires the tensorflow.lib file installed to main/lib from here:
#https://www.tensorflow.org/install/lang_c
target_link_libraries(downscaling-optimization
        PRIVATE
        tensorflow
        NLopt::nlopt
        nlohmann_json::nlohmann_json
)