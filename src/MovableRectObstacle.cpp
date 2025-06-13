#include "MovableRectObstacle.h"
#include "FluidGrid.h"
#include "Util.h"
#include <GL/glut.h>
#include <iostream>
#include <cmath>

#define PI 3.1415

MovableRectObstacle::MovableRectObstacle(int x, int y, int w, int h, int gridN)
    : m_x(x), m_y(y), m_w(w), m_h(h), m_gridN(gridN) {m_angle = 0.f; m_angularVelocity = 180.f;}


bool MovableRectObstacle::pointInsideRect(int x, int y) const {
    float h = 1.f/ m_gridN;
    float rads = PI*m_angle / 180;
    
    // Translate to m_x, m_y
    float tx = x - m_x - m_w/2.f;
    float ty = y - m_y - m_h/2.f;

    float sin_angle = std::sin(rads);
    float cos_angle = std::cos(rads);

    float lx = std::abs(tx * cos_angle + ty * sin_angle);
    float ly = std::abs(-tx * sin_angle + ty * cos_angle);

    return lx <= m_w/2.f && ly <= m_h/2.f;
}

void MovableRectObstacle::update(float dt) {
    m_angle += m_angularVelocity * dt; 
}


void MovableRectObstacle::apply(FluidGrid& grid) const {
    float* u = grid.u();
    float* v = grid.v();
    float* dens = grid.dens();
    int N = grid.size();

    // TODO: do this with bounding box
    for (int i = 1; i <= N; ++i) {
        for (int j = 1; j <= N; ++j) {
            if(!pointInsideRect(i, j)) continue;

            float h = 1.f / N;
            float cwx = (m_x + m_w / 2.f)*h;
            float cwy = (m_y + m_h / 2.f)*h;
            float cewx = (i + .5f) *h;
            float cewy = (j + .5f) *h;

            float rx = cewx - cwx;
            float ry = cewy - cwy;

            float rads_second = PI*m_angularVelocity/180;
            float rot_vx = -rads_second*ry;
            float rot_vy =  rads_second*rx;

            rot_vx = 0;
            rot_vy = 0;
                        
            u[IX(i, j, N)] = m_vx + rot_vx;
            v[IX(i, j, N)] = m_vy + rot_vy;
            dens[IX(i, j, N)] = 0.f;
        }
    }
}

void MovableRectObstacle::draw() const {
    if (m_isSelected) {
        glColor3f(0.8f, 0.8f, 0.2f); // Yellow when selected
    } else {
        glColor3f(0.8f, 0.5f, 0.2f); // Orange for movable
    }

    // Convert radians to degrees
    float h = 1.0f / m_gridN;

    float cx = h*(m_x + m_x + m_w) / 2;
    float cy = h*(m_y + m_y + m_h) / 2;

    float x0 = h*(-m_w / 2.f);
    float y0 = h*(-m_h / 2.f);
    float x1 = h*(m_w / 2.f);
    float y1 = h*(m_h / 2.f);

    glPushMatrix();
    glTranslatef(cx, cy, 0.f);
    glRotatef(m_angle, 0.f, 0.f, 1.f);
    
    glBegin(GL_QUADS);
    glVertex2f(x0, y0);
    glVertex2f(x1, y0);
    glVertex2f(x1, y1);
    glVertex2f(x0, y1);
    glEnd();

    glPopMatrix();
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
