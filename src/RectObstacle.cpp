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
    float* dens = grid.dens();
    int N = grid.size();

    for (int i = m_x; i < m_x + m_w; ++i) {
        for (int j = m_y; j < m_y + m_h; ++j) {
            if (i > 0 && i <= N && j > 0 && j <= N) {
                u[IX(i, j, N)] = 0.f;
                v[IX(i, j, N)] = 0.f;
                dens[IX(i, j, N)] = 0.f;
            }
        }
    }
}

void RectObstacle::updateFromFluid(FluidGrid& grid, float dt) {
    float* u = grid.u();
    float* v = grid.v();
    int N = grid.size();

    float sumU = 0.f;
    float sumV = 0.f;
    int count = 0;

    for (int i = m_x; i < m_x + m_w; ++i) {
        for (int j = m_y; j < m_y + m_h; ++j) {
            if (i > 0 && i <= N && j > 0 && j <= N) {
                int idx = IX(i, j, N);
                sumU += u[idx];
                sumV += v[idx];
                ++count;
            }
        }
    }

    if (count == 0) return;

    float avgU = sumU / count;
    float avgV = sumV / count;

    // Drag influence (adjust coefficient as needed)
    float drag = 1.0f;
    m_vx += (avgU - m_vx) * drag * dt;
    m_vy += (avgV - m_vy) * drag * dt;

    // Update position based on velocity
    m_x += m_vx * dt;
    m_y += m_vy * dt;

    //m_x = std::max(1.0f, std::min(static_cast<float>(N) - m_w, m_xf));
    //m_y = std::max(1.0f, std::min(static_cast<float>(N) - m_h, m_y));
    //m_x = std::max(1, std::min(N - m_w, static_cast<int>(m_xf)));
    //m_y = std::max(1, std::min(N - m_h, static_cast<int>(m_yf)));
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
