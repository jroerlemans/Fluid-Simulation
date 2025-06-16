cmake_minimum_required(VERSION 3.14)
project(FluidToyProject CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

add_library(fluid STATIC
    # Core simulation files
    src/BoundarySolver.cpp
    src/FluidGrid.cpp
    src/FluidSolver.cpp
    src/Vec2.cpp

    # Obstacle and boundary management
    src/ObstacleManager.cpp
    src/RectObstacle.cpp
    src/MovableObstacle.cpp       # <-- ADDED
    src/MovableRectObstacle.cpp
    src/DiskObstacle.cpp          # <-- ADDED
    
    # Empty placeholder files
    src/SolidBoundary.cpp
    src/Source.cpp
)

target_include_directories(fluid PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(FluidToy src/main.cpp)

target_link_libraries(FluidToy PRIVATE
    fluid
    ${GLUT_LIBRARIES}
    ${OPENGL_LIBRARIES}
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)