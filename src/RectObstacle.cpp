#include "RectObstacle.h"
#include "FluidGrid.h"
#include "Util.h"
#include <GL/glut.h>

RectObstacle::RectObstacle(int x, int y, int w, int h, int gridN)
    : m_x(x), m_y(y), m_w(w), m_h(h), m_gridN(gridN) {
        m_xf = static_cast<float>(m_x);  // ✅ align float and int positions
        m_yf = static_cast<float>(m_y);
    }

void RectObstacle::apply(FluidGrid& grid) const {
    float* u = grid.u();
    float* v = grid.v();
    // float* dens = grid.dens(); // No longer needed here
    int N = grid.size();

    for (int i = m_x; i < m_x + m_w; ++i) {
        for (int j = m_y; j < m_y + m_h; ++j) {
            if (i > 0 && i <= N && j > 0 && j <= N) {
                // Enforce a zero-velocity boundary condition.
                // By not touching density, we allow it to be pushed by pressure.
                u[IX(i, j, N)] = 0.f;
                v[IX(i, j, N)] = 0.f;
                // dens[IX(i, j, N)] = 0.f; // REMOVED: This was destroying density.
            }
        }
    }
}

void RectObstacle::updateFromFluid(FluidGrid& grid, float dt) {
    // This is a fixed obstacle, so it is not affected by the fluid.
    // This method is required by the interface but does nothing here.
}

void RectObstacle::draw() const {
    float h = 1.0f / m_gridN;
    float x0 = (m_x - 1) * h;
    float y0 = (m_y - 1) * h;
    float x1 = (m_x - 1 + m_w) * h;
    float y1 = (m_y - 1 + m_h) * h;
    
    glBegin(GL_QUADS);
    glColor3f(0.2,0.5,0.3);
    glVertex2f(x0, y0);
    glVertex2f(x1, y0);
    glVertex2f(x1, y1);
    glVertex2f(x0, y1);
    glEnd();
}