cmake_minimum_required(VERSION 3.30)
project(RectangleR)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)

# Find OpenGL, GLEW, and GLFW
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLUT REQUIRED)

# Add source files
add_executable(RectangleR main.cpp)

# Link libraries
target_link_libraries(RectangleR
        GLEW::GLEW
        glfw
        GLUT::GLUT
)
