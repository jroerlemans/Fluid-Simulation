#pragma once
#include "Obstacle.h"

class RectObstacle : public Obstacle {
public:
    RectObstacle(int x, int y, int w, int h, int gridN);
    void apply(FluidGrid& grid) const override;
    void draw() const override;

private:
    int m_x, m_y, m_w, m_h;
    int m_gridN; // To know grid dimensions for drawing
};