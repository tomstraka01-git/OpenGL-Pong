cmake_minimum_required(VERSION 3.10)
project(OpenGLPong)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)

# Check if we are cross-compiling for Windows
if(WIN32)
    # Point CMake to your local GLFW headers and pre-compiled Windows library
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
    add_library(glfw3 STATIC IMPORTED)
    set_target_properties(glfw3 PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/lib/windows-mingw/libglfw3.a
    )
    # Windows needs gdi32 and opengl32 linked for GLFW
    set(GLFW_LIBRARIES glfw3 gdi32 opengl32)
else()
    # For native Linux build
    find_package(glfw3 REQUIRED)
    set(GLFW_LIBRARIES glfw OpenGL::GL)
endif()

add_executable(OpenGLPong src/main.cpp src/glad.c)

target_include_directories(OpenGLPong PRIVATE include)
target_link_libraries(OpenGLPong PRIVATE ${GLFW_LIBRARIES})

# Automatically copy shaders and font to build folders
file(COPY shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY Roboto-VariableFont_wdth,wght.ttf DESTINATION ${CMAKE_CURRENT_BINARY_DIR})