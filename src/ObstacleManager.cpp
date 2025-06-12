#include "ObstacleManager.h"
#include "RectObstacle.h" // Include concrete obstacle type
#include <GL/glut.h>

ObstacleManager::ObstacleManager(int gridN) : m_gridN(gridN) {}

// The destructor is now defined here, where Obstacle is a complete type.
// The '= default' is fine because all members have destructors.
ObstacleManager::~ObstacleManager() = default; 

void ObstacleManager::applyTo(FluidGrid& grid) {
    for (const auto& obs : m_obstacles) {
        obs->apply(grid);
    }
}

void ObstacleManager::addRect(int x, int y, int w, int h) {
    // C++11 compatible way to add to vector of unique_ptr
    m_obstacles.push_back(std::unique_ptr<Obstacle>(new RectObstacle(x, y, w, h, m_gridN)));
}

void ObstacleManager::draw() const {
    glColor3f(0.5f, 0.5f, 0.5f);
    for (const auto& obs : m_obstacles) {
        obs->draw();
    }
}

void ObstacleManager::clear() {
    m_obstacles.clear();
}