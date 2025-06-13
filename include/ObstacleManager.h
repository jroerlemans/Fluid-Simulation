#pragma once
#include "SolidBoundary.h"
#include <vector>
#include <memory>

class Obstacle;
class MovableRectObstacle; // Forward declare specific type

class ObstacleManager : public SolidBoundary {
public:
    explicit ObstacleManager(int gridN);
    ~ObstacleManager() override;

    // The main interface for the solver
    void applyTo(FluidGrid& grid) override;
    void update(float dt);

    // Public methods for interaction
    void addFixedRect(int x, int y, int w, int h);
    void addMovableRect(int x, int y, int w, int h);
    MovableRectObstacle* findMovableAt(int x, int y); // Method for selection
    void draw() const;
    void clear();

private:
    int m_gridN;
    std::vector<std::unique_ptr<Obstacle>> m_obstacles;
};
