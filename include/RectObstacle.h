#pragma once
#include "Obstacle.h"

class RectObstacle : public Obstacle {
public:
    RectObstacle(int x, int y, int w, int h, int gridN);
    void apply(FluidGrid& grid) const override;
    void draw() const override;
    void updateFromFluid(FluidGrid& grid, float dt) override;

private:
    float m_x, m_y, m_w, m_h;
    float m_vx = 0.f, m_vy = 0.f;
    float m_xf = 0.f, m_yf = 0.f;
    int m_gridN; // To know grid dimensions for drawing
};