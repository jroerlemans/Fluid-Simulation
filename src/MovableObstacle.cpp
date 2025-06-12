#include "MovableRectObstacle.h"
#include "FluidGrid.h"
#include "Util.h"
#include <GL/glut.h>

MovableRectObstacle::MovableRectObstacle(int x, int y, int w, int h, int gridN)
    : m_x(x), m_y(y), m_w(w), m_h(h), m_gridN(gridN) {}

void MovableRectObstacle::apply(FluidGrid& grid) const {
    float* u = grid.u();
    float* v = grid.v();
    float* dens = grid.dens();
    int N = grid.size();

    for (int i = m_x; i < m_x + m_w; ++i) {
        for (int j = m_y; j < m_y + m_h; ++j) {
            if (i > 0 && i <= N && j > 0 && j <= N) {
                // Main change: set fluid velocity to object velocity
                u[IX(i, j, N)] = m_vx;
                v[IX(i, j, N)] = m_vy;
                dens[IX(i, j, N)] = 0.f;
            }
        }
    }
}

void MovableRectObstacle::draw() const {
    if (m_isSelected) {
        glColor3f(0.8f, 0.8f, 0.2f); // Yellow when selected
    } else {
        glColor3f(0.8f, 0.5f, 0.2f); // Orange for movable
    }

    float h = 1.0f / m_gridN;
    float x0 = (m_x - 1) * h;
    float y0 = (m_y - 1) * h;
    float x1 = (m_x - 1 + m_w) * h;
    float y1 = (m_y - 1 + m_h) * h;
    
    glBegin(GL_QUADS);
    glVertex2f(x0, y0);
    glVertex2f(x1, y0);
    glVertex2f(x1, y1);
    glVertex2f(x0, y1);
    glEnd();
}

void MovableRectObstacle::updatePosition(int newX, int newY) {
    m_x = newX;
    m_y = newY;
}

void MovableRectObstacle::setVelocity(float vx, float vy) {
    m_vx = vx;
    m_vy = vy;
}

bool MovableRectObstacle::contains(int x, int y) const {
    return (x >= m_x && x < m_x + m_w && y >= m_y && y < m_y + m_h);
}

void MovableRectObstacle::setSelected(bool selected) {
    m_isSelected = selected;
}