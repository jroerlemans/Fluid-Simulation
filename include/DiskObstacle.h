#pragma once
#include "MovableObstacle.h"

class DiskObstacle : public MovableObstacle {
public:
    DiskObstacle(int x, int y, int radius, int gridN);
    ~DiskObstacle() override = default;

    // --- Obstacle Interface ---
    void apply(FluidGrid& grid) const override;
    void draw() const override;
    void updateFromFluid(FluidGrid& grid, float dt) override;

    // --- MovableObstacle Interface ---
    bool contains(int x, int y) const override;

    // --- Disk-specific methods ---
    float getRadius() const;
    Vec2 getCenter() const;

private:
    int m_radius;
    int m_x, m_y, m_w, m_h;
    float m_vx = 0.f, m_vy = 0.f;
    float m_xf = 0.f, m_yf = 0.f;
    int m_gridN;
};