#pragma once
#include "SolidBoundary.h"
#include <vector>
#include <memory>

class Obstacle;
class MovableObstacle; // Use the new base class
class MovableRectObstacle;
class DiskObstacle;
class FluidGrid;
struct Vec2;

class ObstacleManager : public SolidBoundary {
public:
    explicit ObstacleManager(int gridN);
    ~ObstacleManager() override;

    void applyTo(FluidGrid& grid) override;
    void update(float dt);
    void handleCollisions();
    void updateObstacles(FluidGrid& grid, float dt);

    void addFixedRect(int x, int y, int w, int h);
    void addMovableRect(int x, int y, int w, int h);
    void addDisk(int x, int y, int r, int w, int h); // New method

    MovableObstacle* findMovableAt(int x, int y); // Returns the new base class
    void draw() const;
    void clear();

private:
    // Collision checking helpers
    bool checkCollision(MovableRectObstacle* a, MovableRectObstacle* b, Vec2& mtv);
    bool checkCollision(DiskObstacle* a, DiskObstacle* b, Vec2& mtv);
    bool checkCollision(MovableRectObstacle* rect, DiskObstacle* disk, Vec2& mtv);

    // Collision resolution helper
    void resolveCollision(MovableObstacle* a, MovableObstacle* b, const Vec2& mtv);

    int m_gridN;
    std::vector<std::unique_ptr<Obstacle>> m_obstacles;
};