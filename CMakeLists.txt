cmake_minimum_required(VERSION 3.14)

project(Based-chip8 VERSION 0.0.1)

set(CMAKE_EXPORT_COMPILE_COMMANDS True)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# SDL2 - expect it to be installed system-wide
find_package(SDL2)

# imgui - not many cmake versions found
# better thought of including it directly at configure phase
include(FetchContent)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.88
)
FetchContent_MakeAvailable(imgui)

# all source files in imgui directory will be compiled
file(GLOB IMGUI_SRC
        "${imgui_SOURCE_DIR}/imgui*.cpp"
)

# sdl2 renderer backend, I will use
set(IMGUI_BACKEND
        "${imgui_SOURCE_DIR}/backends/imgui_impl_sdl.cpp"
        "${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer.cpp"
)

set(SRC src/cmake_testing.cpp
        src/libchip8_impl.cpp
        "${IMGUI_SRC}"
        "${IMGUI_BACKEND}"
)

add_executable(chip8 "${SRC}")

target_include_directories(chip8 PUBLIC src/core
                                 PUBLIC "${SDL2_INCLUDE_DIRS}"
                                 PUBLIC "${imgui_SOURCE_DIR}"
                                 PUBLIC "${imgui_SOURCE_DIR}/backends"
)

target_link_libraries(chip8 PUBLIC "${SDL2_LIBRARIES}")