#include "DiskObstacle.h"
#include "FluidGrid.h"
#include "Util.h"
#include <GL/glut.h>
#include <cmath>
#include <algorithm>

#define PI 3.1415926535f

DiskObstacle::DiskObstacle(int x, int y, int radius, int gridN)
    : MovableObstacle(static_cast<float>(x), static_cast<float>(y)), m_radius(radius), m_gridN(gridN)
{
    // Mass is proportional to area
    m_mass = PI * static_cast<float>(m_radius * m_radius);
    m_inverseMass = (m_mass > 0) ? 1.0f / m_mass : 0.f;
    m_xf = static_cast<float>(m_x);  // ✅ align float and int positions
    m_yf = static_cast<float>(m_y);
}

void DiskObstacle::apply(FluidGrid& grid) const {
    float* u = grid.u();
    float* v = grid.v();
    float* dens = grid.dens();
    int N = grid.size();

    Vec2 center = getCenter();
    int i_min = std::max(1, static_cast<int>(center.x - m_radius));
    int i_max = std::min(N, static_cast<int>(center.x + m_radius));
    int j_min = std::max(1, static_cast<int>(center.y - m_radius));
    int j_max = std::min(N, static_cast<int>(center.y + m_radius));

    for (int i = i_min; i <= i_max; ++i) {
        for (int j = j_min; j <= j_max; ++j) {
            Vec2 cell_pos(static_cast<float>(i), static_cast<float>(j));
            if ((cell_pos - center).lenSq() < m_radius * m_radius) {
                u[IX(i, j, N)] = m_vx;
                v[IX(i, j, N)] = m_vy;
                dens[IX(i, j, N)] = 0.f;
            }
        }
    }
}

void DiskObstacle::updateFromFluid(FluidGrid& grid, float dt) {
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
    float drag = 0.1f;
    m_vx += (avgU - m_vx) * drag * dt;
    m_vy += (avgV - m_vy) * drag * dt;

    // Update position based on velocity
    m_xf += m_vx * dt;
    m_yf += m_vy * dt;

    m_x = std::max(1, std::min(N - m_w, static_cast<int>(m_xf)));
    m_y = std::max(1, std::min(N - m_h, static_cast<int>(m_yf)));
}

void DiskObstacle::draw() const {
    if (m_isSelected) {
        glColor3f(0.2f, 0.8f, 0.8f); // Cyan when selected
    } else {
        glColor3f(0.2f, 0.5f, 0.8f); // Blue for movable
    }

    float h = 1.0f / m_gridN;
    Vec2 center = getCenter();
    float cx = h * center.x;
    float cy = h * center.y;
    float r = h * m_radius;

    int segments = 24;
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy); // Center of circle
    for(int i = 0; i <= segments; i++) {
        float angle = 2.0f * PI * static_cast<float>(i) / static_cast<float>(segments);
        float dx = r * std::cos(angle);
        float dy = r * std::sin(angle);
        glVertex2f(cx + dx, cy + dy);
    }
    glEnd();
}

bool DiskObstacle::contains(int x, int y) const {
    Vec2 pos(static_cast<float>(x), static_cast<float>(y));
    return (pos - getCenter()).lenSq() <= m_radius * m_radius;
}

float DiskObstacle::getRadius() const {
    return static_cast<float>(m_radius);
}

Vec2 DiskObstacle::getCenter() const {
    // The disk's position (m_x, m_y) is its center.
    return {m_x, m_y};
}