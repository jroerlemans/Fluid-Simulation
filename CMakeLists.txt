cmake_minimum_required(VERSION 3.10)
project(FluidToyProject CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Find External Packages ---
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

# --- Define Our Simulation Library ---
add_library(fluid STATIC
    src/BoundarySolver.cpp
    src/FluidGrid.cpp
    src/FluidSolver.cpp
    src/SolidBoundary.cpp
    src/Source.cpp
    src/Vec2.cpp
    src/ObstacleManager.cpp   # <-- ADD THIS
    src/RectObstacle.cpp      # <-- ADD THIS
    # Obstacle.h is header-only, no .cpp needed
)

# Tell the 'fluid' library where to find its own header files.
target_include_directories(fluid PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include   
)

# --- Define Our Main Executable ---
add_executable(FluidToy src/main.cpp)

# Tell the FluidToy executable where to find the graphics headers.
target_include_directories(FluidToy PRIVATE
    ${OPENGL_INCLUDE_DIR}
    ${GLUT_INCLUDE_DIR}
)

# --- Link Everything Together ---
target_link_libraries(FluidToy PRIVATE
    fluid
    ${GLUT_LIBRARIES}
    ${OPENGL_LIBRARIES}
)