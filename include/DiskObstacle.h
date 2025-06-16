#pragma once
#include "MovableObstacle.h"

class DiskObstacle : public MovableObstacle {
public:
    DiskObstacle(int x, int y, int radius, int gridN);
    ~DiskObstacle() override = default;

    // --- Obstacle Interface ---
    void apply(FluidGrid& grid) const override;
    void draw() const override;

    // --- MovableObstacle Interface ---
    bool contains(int x, int y) const override;

    // --- Disk-specific methods ---
    float getRadius() const;
    Vec2 getCenter() const;

private:
    int m_radius;
    int m_gridN;
};