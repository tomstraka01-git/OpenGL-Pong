cmake_minimum_required(VERSION 3.10)
project(OpenGLPong)

set(CMAKE_CXX_STANDARD 17)

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(OpenGLPong src/main.cpp src/glad.c)

target_include_directories(OpenGLPong PRIVATE include)


target_link_libraries(OpenGLPong PRIVATE glfw OpenGL::GL)

file(COPY shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})