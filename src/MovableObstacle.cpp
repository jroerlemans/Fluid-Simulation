#include "MovableObstacle.h"

MovableObstacle::MovableObstacle(float x, float y, float angle)
    : m_x(x), m_y(y), m_angle(angle), m_angularVelocity(0.f), m_mass(1.f), m_inverseMass(1.f) {}

void MovableObstacle::update(float dt) {
    m_x += m_vx * dt;
    m_y += m_vy * dt;
    m_angle += m_angularVelocity * dt;
}

void MovableObstacle::setVelocity(float vx, float vy) {
    m_vx = vx;
    m_vy = vy;
}

// New method implementation
void MovableObstacle::getVelocity(float& vx, float& vy) const {
    vx = m_vx;
    vy = m_vy;
}

void MovableObstacle::setAngularVelocity(float degreesPerSecond) {
    m_angularVelocity = degreesPerSecond;
}

void MovableObstacle::setSelected(bool selected) {
    m_isSelected = selected;
}

void MovableObstacle::updatePosition(const Vec2& newPos) {
    m_x = newPos.x;
    m_y = newPos.y;
}

Vec2 MovableObstacle::getPosition() const {
    return {m_x, m_y};
}

float MovableObstacle::getInverseMass() const {
    return m_inverseMass;
}