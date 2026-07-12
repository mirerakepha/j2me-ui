cmake_minimum_required(VERSION 3.16)
project(j2me_ui CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)

add_compile_options(-Os -fno-exceptions -fno-rtti -fno-unwind-tables -ffunction-sections -fdata-sections)
add_link_options(-Wl,--gc-sections -s)

add_executable(j2me_ui
    src/main.cpp
    src/retro_core.cpp
)

target_include_directories(j2me_ui PRIVATE include ${SDL2_INCLUDE_DIRS})
target_link_libraries(j2me_ui PRIVATE dl ${SDL2_LIBRARIES})

