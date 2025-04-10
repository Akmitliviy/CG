cmake_minimum_required(VERSION 3.30)
project(Cylinder_BSpline)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)

# Find OpenGL, GLEW, and GLFW
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLUT REQUIRED)

# Add source files
add_executable(Cylinder_BSpline main.cpp)

# Link libraries
target_link_libraries(Cylinder_BSpline
        GLEW::GLEW
        glfw
        GLUT::GLUT
)
