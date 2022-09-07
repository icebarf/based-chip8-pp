cmake_minimum_required(VERSION 3.10)

project(Based-chip8 VERSION 0.0.1)

set(CMAKE_EXPORT_COMPILE_COMMANDS true)
set(SRC src/main.cpp)

add_executable(chip8 "${SRC}")

target_include_directories(chip8 PUBLIC src/core)