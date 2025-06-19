cmake_minimum_required(VERSION 3.14)
project(FluidToyProject CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

add_library(fluid STATIC
    # Core simulation files
    src/BoundarySolver.cpp    include/BoundarySolver.h
    src/FluidGrid.cpp         include/FluidGrid.h
    src/FluidSolver.cpp       include/FluidSolver.h
    src/Vec2.cpp              include/Vec2.h
    include/Util.h

    # Obstacle and boundary management
    src/ObstacleManager.cpp   include/ObstacleManager.h
    src/RectObstacle.cpp      include/RectObstacle.h
    src/MovableObstacle.cpp   include/MovableObstacle.h
    src/MovableRectObstacle.cpp include/MovableRectObstacle.h
    src/DiskObstacle.cpp      include/DiskObstacle.h
    include/Obstacle.h
    
    # Empty placeholder files
    src/SolidBoundary.cpp     include/SolidBoundary.h
    src/Source.cpp            include/Source.h
)

target_include_directories(fluid PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

add_executable(FluidToy src/main.cpp)

# Link using modern imported targets.
# Add OpenGL::GLU to link the OpenGL Utility Library.
target_link_libraries(FluidToy PRIVATE
    fluid
    GLUT::GLUT
    OpenGL::GL
    OpenGL::GLU   # <-- THE FIX IS HERE
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)