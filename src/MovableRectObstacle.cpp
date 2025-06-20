#include "MovableRectObstacle.h"
#include "FluidGrid.h"
#include "Util.h"
#include <GL/glut.h>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <limits>

#define PI 3.1415926535f

MovableRectObstacle::MovableRectObstacle(int x, int y, int w, int h, int gridN)
    : MovableObstacle(static_cast<float>(x), static_cast<float>(y)), m_w(w), m_h(h), m_gridN(gridN) 
{
    m_mass = static_cast<float>(m_w * m_h);
    m_inverseMass = (m_mass > 0) ? 1.0f / m_mass : 0.0f;
    m_xf = static_cast<float>(m_x);  // ✅ align float and int positions
    m_yf = static_cast<float>(m_y);
}

bool MovableRectObstacle::contains(int x, int y) const {
    float rads = PI * m_angle / 180.0f;
    
    Vec2 center = getCenter();
    
    float tx = static_cast<float>(x) - center.x;
    float ty = static_cast<float>(y) - center.y;

    float sin_angle = std::sin(-rads);
    float cos_angle = std::cos(-rads);

    float rotated_x = tx * cos_angle - ty * sin_angle;
    float rotated_y = tx * sin_angle + ty * cos_angle;

    return std::abs(rotated_x) <= m_w / 2.f && std::abs(rotated_y) <= m_h / 2.f;
}

void MovableRectObstacle::apply(FluidGrid& grid) const {
    float* u = grid.u();
    float* v = grid.v();
    float* dens = grid.dens();
    int N = grid.size();

    float max_dim = std::sqrt(static_cast<float>(m_w*m_w + m_h*m_h)) / 2.f + 2.f;
    Vec2 center = getCenter();
    int i_min = std::max(1, static_cast<int>(center.x - max_dim));
    int i_max = std::min(N, static_cast<int>(center.x + max_dim));
    int j_min = std::max(1, static_cast<int>(center.y - max_dim));
    int j_max = std::min(N, static_cast<int>(center.y + max_dim));

    for (int i = i_min; i <= i_max; ++i) {
        for (int j = j_min; j <= j_max; ++j) {
            if(contains(i, j)) {
                u[IX(i, j, N)] = m_vx;
                v[IX(i, j, N)] = m_vy;
                dens[IX(i, j, N)] = 0.f;
            }
        }
    }
}

void MovableRectObstacle::updateFromFluid(FluidGrid& grid, float dt) {
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
    m_x += m_vx * dt;
    m_y += m_vy * dt;

    //m_x = std::max(1.0f, std::min(static_cast<float>(N) - m_w, m_xf));
    //m_y = std::max(1.0f, std::min(static_cast<float>(N) - m_h, m_y));
}

void MovableRectObstacle::draw() const {
    if (m_isSelected) {
        glColor3f(0.8f, 0.8f, 0.2f); // Yellow when selected
    } else {
        glColor3f(0.8f, 0.5f, 0.2f); // Orange for movable
    }

    float h = 1.0f / m_gridN;
    
    Vec2 center = getCenter();
    float cx = h * center.x;
    float cy = h * center.y;

    float hw = h * (m_w / 2.f);
    float hh = h * (m_h / 2.f);
    
    glPushMatrix();
    glTranslatef(cx, cy, 0.f);
    glRotatef(m_angle, 0.f, 0.f, 1.f);
    
    glBegin(GL_QUADS);
    glVertex2f(-hw, -hh);
    glVertex2f( hw, -hh);
    glVertex2f( hw,  hh);
    glVertex2f(-hw,  hh);
    glEnd();

    glPopMatrix();
}

void MovableRectObstacle::updatePosition(int newX, int newY) {
    m_x = static_cast<float>(newX);
    m_y = static_cast<float>(newY);
}

Vec2 MovableRectObstacle::getCenter() const {
    return getPosition() + Vec2(m_w/2.f, m_h/2.f);
}

void MovableRectObstacle::getVertices(std::vector<Vec2>& vertices) const {
    vertices.resize(4);
    Vec2 center = getCenter();
    
    vertices[0] = Vec2(-m_w/2.f, -m_h/2.f);
    vertices[1] = Vec2( m_w/2.f, -m_h/2.f);
    vertices[2] = Vec2( m_w/2.f,  m_h/2.f);
    vertices[3] = Vec2(-m_w/2.f,  m_h/2.f);

    float rads = m_angle * PI / 180.0f;
    float s = std::sin(rads);
    float c = std::cos(rads);

    for (int i = 0; i < 4; ++i) {
        float x_rot = vertices[i].x * c - vertices[i].y * s;
        float y_rot = vertices[i].x * s + vertices[i].y * c;
        vertices[i].x = x_rot + center.x;
        vertices[i].y = y_rot + center.y;
    }
}