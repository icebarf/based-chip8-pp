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

# the executable target
add_executable(chip8)

target_sources(chip8
                PRIVATE src/cmake_testing.cpp
                PRIVATE src/libchip8_impl.cpp
                PRIVATE "${imgui_SOURCE_DIR}/imgui_demo.cpp"
                PRIVATE "${imgui_SOURCE_DIR}/imgui_draw.cpp"
                PRIVATE "${imgui_SOURCE_DIR}/imgui_tables.cpp"
                PRIVATE "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
                PRIVATE "${imgui_SOURCE_DIR}/imgui.cpp"
                PRIVATE "${imgui_SOURCE_DIR}/backends/imgui_impl_sdl.cpp"
                PRIVATE "${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer.cpp"
)

target_compile_features(chip8 INTERFACE std_cxx_20)

target_include_directories(chip8 PRIVATE src/core
                                 PRIVATE "${SDL2_INCLUDE_DIRS}"
                                 PRIVATE "${imgui_SOURCE_DIR}"
                                 PRIVATE "${imgui_SOURCE_DIR}/backends"
)

target_link_libraries(chip8 PUBLIC "${SDL2_LIBRARIES}")