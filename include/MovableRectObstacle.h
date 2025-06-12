#pragma once
#include "Obstacle.h"

class MovableRectObstacle : public Obstacle {
public:
    MovableRectObstacle(int x, int y, int w, int h, int gridN);
    ~MovableRectObstacle() override = default;

    // Obstacle interface
    void apply(FluidGrid& grid) const override;
    void draw() const override;

    // Movable-specific methods
    void updatePosition(int newX, int newY);
    void setVelocity(float vx, float vy);
    bool contains(int x, int y) const;
    void setSelected(bool selected);

private:
    int m_x, m_y, m_w, m_h;
    int m_gridN;
    float m_vx = 0.f, m_vy = 0.f; // Object's velocity in grid cells/s
    bool m_isSelected = false;
};