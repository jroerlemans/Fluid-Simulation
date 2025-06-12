#include "FixedObstacle.h"
#include "FluidGrid.h"
#include "Util.h"
#include <GL/glut.h>
#include <algorithm>

FixedObstacle::FixedObstacle(int N) 
    : m_N(N), m_isSolid(size_t(N + 2) * (N + 2), false) {}

void FixedObstacle::addRect(int x, int y, int w, int h) {
    for (int i = x; i < x + w; ++i) {
        for (int j = y; j < y + h; ++j) {
            if (i > 0 && i <= m_N && j > 0 && j <= m_N) {
                m_isSolid[IX(i, j, m_N)] = true;
            }
        }
    }
}

void FixedObstacle::applyTo(FluidGrid& grid) {
    float* u = grid.u();
    float* v = grid.v();
    float* dens = grid.dens();

    for (int i = 1; i <= m_N; ++i) {
        for (int j = 1; j <= m_N; ++j) {
            if (m_isSolid[IX(i, j, m_N)]) {
                u[IX(i, j, m_N)] = 0.f;
                v[IX(i, j, m_N)] = 0.f;
                dens[IX(i, j, m_N)] = 0.f;
            }
        }
    }
}

void FixedObstacle::draw() const {
    float h = 1.0f / m_N;
    glColor3f(0.5f, 0.5f, 0.5f); // Gray for solids
    glBegin(GL_QUADS);
    for (int i = 1; i <= m_N; ++i) {
        for (int j = 1; j <= m_N; ++j) {
            if (m_isSolid[IX(i, j, m_N)]) {
                float x0 = (i - 1) * h;
                float y0 = (j - 1) * h;
                glVertex2f(x0, y0);
                glVertex2f(x0 + h, y0);
                glVertex2f(x0 + h, y0 + h);
                glVertex2f(x0, y0 + h);
            }
        }
    }
    glEnd();
}