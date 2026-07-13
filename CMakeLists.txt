cmake_minimum_required(VERSION 3.16)
project(j2me_ui CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)

set(CMAKE_CXX_FLAGS_RELEASE "-Os -fno-exceptions -fno-rtti -fno-unwind-tables -ffunction-sections -fdata-sections")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-Wl,--gc-sections -s")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")


add_executable(j2me_ui
    src/main.cpp
    src/retro_core.cpp
)

target_include_directories(j2me_ui PRIVATE include ${SDL2_INCLUDE_DIRS})
target_link_libraries(j2me_ui PRIVATE dl ${SDL2_LIBRARIES})

