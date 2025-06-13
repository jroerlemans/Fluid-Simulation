#include "ObstacleManager.h"
#include "RectObstacle.h"
#include "MovableRectObstacle.h" // Include new obstacle type
#include <GL/glut.h>
#include <algorithm>

ObstacleManager::ObstacleManager(int gridN) : m_gridN(gridN) {}

ObstacleManager::~ObstacleManager() = default; 

void ObstacleManager::applyTo(FluidGrid& grid) {
    for (const auto& obs : m_obstacles) {
        obs->apply(grid);
    }
}

void ObstacleManager::update(float dt) {
    for (const auto& obs : m_obstacles) {
        obs->update(dt);
    }
}

void ObstacleManager::addFixedRect(int x, int y, int w, int h) {
    m_obstacles.push_back(std::unique_ptr<Obstacle>(new RectObstacle(x, y, w, h, m_gridN)));
}

void ObstacleManager::addMovableRect(int x, int y, int w, int h) {
    m_obstacles.push_back(std::unique_ptr<Obstacle>(new MovableRectObstacle(x, y, w, h, m_gridN)));
}

MovableRectObstacle* ObstacleManager::findMovableAt(int x, int y) {
    // Iterate backwards so we pick the top-most object
    for (auto it = m_obstacles.rbegin(); it != m_obstacles.rend(); ++it) {
        // Try to cast the base pointer to a movable pointer
        MovableRectObstacle* movable = dynamic_cast<MovableRectObstacle*>((*it).get());
        if (movable && movable->contains(x, y)) {
            return movable;
        }
    }
    return nullptr;
}

void ObstacleManager::draw() const {
    // Fixed objects are grey
    glColor3f(0.5f, 0.5f, 0.5f);
    for (const auto& obs : m_obstacles) {
        if (dynamic_cast<RectObstacle*>(obs.get())) {
            obs->draw();
        }
    }
    // Movable objects handle their own colors
    for (const auto& obs : m_obstacles) {
        if (dynamic_cast<MovableRectObstacle*>(obs.get())) {
            obs->draw();
        }
    }
}

void ObstacleManager::clear() {
    m_obstacles.clear();
}
