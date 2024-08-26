cmake_minimum_required(VERSION 3.25)
project(Proyecto01CPDScreensaver)

set(CMAKE_CXX_STANDARD 17)
set(SDL_INCLUDE_DIR C:/SDL/include)
set(SDL2_LIB_DIR C:/SDL/lib/x64)

include_directories(${SDL_INCLUDE_DIR})
link_directories(${SDL2_LIB_DIR})

add_executable(Proyecto01CPDScreensaver mainSecuencial.cpp)

target_link_libraries(${PROJECT_NAME} SDL2main SDL2)