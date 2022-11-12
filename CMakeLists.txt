
cmake_minimum_required(VERSION 3.24)
project(chip8)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(chip8 main.cpp Chip8/Chip8.cpp Chip8/Display.cpp)

find_package(SDL2 REQUIRED)

target_include_directories(chip8 PRIVATE ${SDL2_INCLUDE_DIRS})

target_link_libraries(chip8 PRIVATE ${SDL2_LIBRARIES})
