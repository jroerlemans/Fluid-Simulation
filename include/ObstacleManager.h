#pragma once
#include "SolidBoundary.h"
#include <vector>
#include <memory>

// Forward declare to avoid circular dependencies
class Obstacle; 

class ObstacleManager : public SolidBoundary {
public:
    explicit ObstacleManager(int gridN);
    ~ObstacleManager() override; // <-- CHANGED: Removed '= default'

    // The main interface for the solver
    void applyTo(FluidGrid& grid) override;

    // Public methods for interaction
    void addRect(int x, int y, int w, int h);
    void draw() const;
    void clear();

private:
    int m_gridN;
    std::vector<std::unique_ptr<Obstacle>> m_obstacles;
};