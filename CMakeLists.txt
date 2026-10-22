cmake_minimum_required(VERSION 3.10)
project(OpenGL-Pong)

set(CMAKE_CXX_STANDARD 17)

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(Pong src/main.cpp)

target_link_libraries(Pong glfw OpenGL::GL)