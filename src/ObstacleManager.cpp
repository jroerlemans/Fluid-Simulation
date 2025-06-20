#include "ObstacleManager.h"
#include "RectObstacle.h"
#include "MovableRectObstacle.h"
#include "DiskObstacle.h"
#include "Vec2.h"
#include <GL/glut.h>
#include <algorithm>
#include <limits>

#define PI 3.1415926535f

ObstacleManager::ObstacleManager(int gridN) : m_gridN(gridN) {}
ObstacleManager::~ObstacleManager() = default; 

void ObstacleManager::applyTo(FluidGrid& grid) {
    for (const auto& obs : m_obstacles) {
        obs->apply(grid);
    }
}

void ObstacleManager::updateObstacles(FluidGrid& grid, float dt) {
    for (const auto& obs : m_obstacles) {
        obs->updateFromFluid(grid, dt);
    }
}

void ObstacleManager::update(float dt) {
    for (auto& obs : m_obstacles) {
        obs->update(dt);
    }
}

void ObstacleManager::handleCollisions() {
    std::vector<MovableObstacle*> movables;
    for (auto& obs : m_obstacles) {
        if (auto m = dynamic_cast<MovableObstacle*>(obs.get())) {
            movables.push_back(m);
        }
    }

    const int iterations = 5; // Use several iterations to better resolve multiple collisions
    for (int k=0; k < iterations; ++k) {
        for (size_t i = 0; i < movables.size(); ++i) {
            for (size_t j = i + 1; j < movables.size(); ++j) {
                MovableObstacle* a = movables[i];
                MovableObstacle* b = movables[j];

                auto rect_a = dynamic_cast<MovableRectObstacle*>(a);
                auto rect_b = dynamic_cast<MovableRectObstacle*>(b);
                auto disk_a = dynamic_cast<DiskObstacle*>(a);
                auto disk_b = dynamic_cast<DiskObstacle*>(b);

                Vec2 mtv;
                bool collided = false;

                if (rect_a && rect_b) {
                    collided = checkCollision(rect_a, rect_b, mtv);
                } else if (disk_a && disk_b) {
                    collided = checkCollision(disk_a, disk_b, mtv);
                } else if (rect_a && disk_b) {
                    collided = checkCollision(rect_a, disk_b, mtv);
                } else if (rect_b && disk_a) {
                    collided = checkCollision(rect_b, disk_a, mtv);
                    mtv = -mtv; 
                }

                if (collided) {
                    resolveCollision(a, b, mtv);
                }
            }
        }
    }
}

void ObstacleManager::resolveCollision(MovableObstacle* a, MovableObstacle* b, const Vec2& mtv) {
    float invMassA = a->getInverseMass();
    float invMassB = b->getInverseMass();
    float totalInvMass = invMassA + invMassB;

    if (totalInvMass == 0) return;

    // --- 1. Positional Correction (The part you already have) ---
    // This pushes the objects apart so they no longer overlap.
    const float percent = 0.8f; // Correction percentage to avoid sinking
    const float slop = 0.01f;   // A small buffer to prevent immediate re-collision
    Vec2 correction = mtv * (std::max(mtv.len() - slop, 0.0f) / totalInvMass) * percent;
    a->updatePosition(a->getPosition() + correction * invMassA);
    b->updatePosition(b->getPosition() - correction * invMassB);

    // --- 2. Impulse Resolution (The new part) ---
    // This changes their velocities so they bounce off each other.
    Vec2 normal = mtv.normalized();
    Vec2 velA = Vec2(0,0); a->getVelocity(velA.x, velA.y);
    Vec2 velB = Vec2(0,0); b->getVelocity(velB.x, velB.y);
    
    Vec2 relativeVel = velB - velA;
    float velAlongNormal = relativeVel.dot(normal);

    // Do not resolve if velocities are separating
    if (velAlongNormal > 0) {
        return;
    }

    float restitution = 0.3f; // Bounciness. 0 = no bounce, 1 = perfect bounce.

    // Calculate impulse scalar
    float j = -(1 + restitution) * velAlongNormal;
    j /= totalInvMass;

    // Apply impulse
    Vec2 impulse = normal * j;
    a->setVelocity(velA.x - impulse.x * invMassA, velA.y - impulse.y * invMassA);
    b->setVelocity(velB.x + impulse.x * invMassB, velB.y + impulse.y * invMassB);
}


// Rect-Rect Collision (SAT)
bool ObstacleManager::checkCollision(MovableRectObstacle* a, MovableRectObstacle* b, Vec2& mtv) {
    std::vector<Vec2> axes;
    std::vector<Vec2> verticesA, verticesB;
    a->getVertices(verticesA);
    b->getVertices(verticesB);

    for (int i = 0; i < 4; ++i) {
        axes.push_back((verticesA[(i + 1) % 4] - verticesA[i]).perpendicular().normalized());
        axes.push_back((verticesB[(i + 1) % 4] - verticesB[i]).perpendicular().normalized());
    }

    float min_overlap = std::numeric_limits<float>::max();

    for (const auto& axis : axes) {
        float minA = std::numeric_limits<float>::max(), maxA = -std::numeric_limits<float>::max();
        for (const auto& v : verticesA) { float p = v.dot(axis); minA = std::min(minA, p); maxA = std::max(maxA, p); }

        float minB = std::numeric_limits<float>::max(), maxB = -std::numeric_limits<float>::max();
        for (const auto& v : verticesB) { float p = v.dot(axis); minB = std::min(minB, p); maxB = std::max(maxB, p); }
        
        if (maxA < minB || maxB < minA) return false;

        float overlap = std::min(maxA, maxB) - std::max(minA, minB);
        if (overlap < min_overlap) {
            min_overlap = overlap;
            mtv = axis; // Store just the axis, magnitude comes later
        }
    }

    // Ensure mtv points from a to b
    Vec2 dir = b->getCenter() - a->getCenter();
    if (dir.dot(mtv) < 0) mtv = -mtv;

    // Re-apply magnitude
    mtv = mtv.normalized() * min_overlap;
    
    return true;
}

// Disk-Disk Collision
bool ObstacleManager::checkCollision(DiskObstacle* a, DiskObstacle* b, Vec2& mtv) {
    Vec2 distVec = b->getCenter() - a->getCenter();
    float distSq = distVec.lenSq();
    float totalRadius = a->getRadius() + b->getRadius();

    if (distSq >= totalRadius * totalRadius) return false;

    float dist = std::sqrt(distSq);
    float overlap = totalRadius - dist;

    // Check for zero distance case (perfect overlap)
    if (dist > 0) {
        mtv = (distVec / dist) * overlap;
    } else {
        // Objects are at the exact same position, push them apart along an arbitrary axis (e.g., x-axis)
        mtv = Vec2(overlap, 0);
    }
    return true;
}

// Rect-Disk Collision
bool ObstacleManager::checkCollision(MovableRectObstacle* rect, DiskObstacle* disk, Vec2& mtv) {
    std::vector<Vec2> vertices;
    rect->getVertices(vertices);
    Vec2 diskCenter = disk->getCenter();
    float diskRadius = disk->getRadius();

    Vec2 closestPoint = diskCenter;
    float minDistSq = std::numeric_limits<float>::max();

    // Find closest point on rectangle edges to disk center
    for(size_t i = 0; i < vertices.size(); ++i) {
        Vec2 p1 = vertices[i];
        Vec2 p2 = vertices[(i + 1) % vertices.size()];
        Vec2 edge = p2 - p1;
        Vec2 pointVec = diskCenter - p1;
        
        float t = edge.dot(pointVec) / edge.lenSq();
        t = std::max(0.f, std::min(1.f, t));

        Vec2 currentClosest = p1 + edge * t;
        float distSq = (diskCenter - currentClosest).lenSq();
        if(distSq < minDistSq) {
            minDistSq = distSq;
            closestPoint = currentClosest;
        }
    }
    
    // Check if the disk center is inside the rectangle (Voronoi region of the rect itself)
    // If it is, the closest point is the disk center.
    if (rect->contains(static_cast<int>(diskCenter.x), static_cast<int>(diskCenter.y))) {
        closestPoint = diskCenter;
    }

    Vec2 delta = diskCenter - closestPoint;
    float distSq = delta.lenSq();

    if (distSq >= diskRadius * diskRadius) return false;

    float dist = std::sqrt(distSq);
    float overlap = diskRadius - dist;
    
    if (dist > 0) {
        mtv = (delta / dist) * overlap;
    } else {
        // Special case: disk center is exactly on the closest point. Need to find a robust normal.
        // The normal from the rectangle's center to the disk's center is a good fallback.
        mtv = (disk->getCenter() - rect->getCenter()).normalized() * overlap;
    }

    return true;
}


void ObstacleManager::addFixedRect(int x, int y, int w, int h) {
    m_obstacles.push_back(std::make_unique<RectObstacle>(x, y, w, h, m_gridN));
}

void ObstacleManager::addMovableRect(int x, int y, int w, int h) {
    m_obstacles.push_back(std::make_unique<MovableRectObstacle>(x, y, w, h, m_gridN));
}

void ObstacleManager::addDisk(int x, int y, int r, int w, int h) {
    m_obstacles.push_back(std::make_unique<DiskObstacle>(x, y, r, w, h, m_gridN));
}

MovableObstacle* ObstacleManager::findMovableAt(int x, int y) {
    for (auto it = m_obstacles.rbegin(); it != m_obstacles.rend(); ++it) {
        if (auto movable = dynamic_cast<MovableObstacle*>((*it).get())) {
            if (movable->contains(x, y)) {
                return movable;
            }
        }
    }
    return nullptr;
}

void ObstacleManager::draw() const {
    for (const auto& obs : m_obstacles) { obs->draw(); }
}

void ObstacleManager::clear() {
    m_obstacles.clear();
}