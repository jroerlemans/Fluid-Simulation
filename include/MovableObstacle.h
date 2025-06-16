#pragma once
#include "Obstacle.h"
#include "Vec2.h"

// An abstract base class for any obstacle that can move and be selected.
class MovableObstacle : public Obstacle {
public:
    // Constructor initializes common physics and state properties.
    MovableObstacle(float x, float y, float angle = 0.f);
    ~MovableObstacle() override = default;

    // --- Obstacle Interface ---
    // update is implemented here as it's common to all movable objects.
    void update(float dt) override;

    // --- Movable-specific Interface (implemented here) ---
    void setVelocity(float vx, float vy);
    void getVelocity(float& vx, float& vy) const; // New getter
    void setAngularVelocity(float degreesPerSecond);
    void setSelected(bool selected);
    void updatePosition(const Vec2& newPos);

    // --- Pure virtual methods to be implemented by derived classes ---
    virtual bool contains(int x, int y) const = 0;

    // --- Getters for the physics system ---
    Vec2 getPosition() const;
    float getInverseMass() const;

protected:
    // Common properties accessible by derived classes (Rect, Disk, etc.)
    float m_x, m_y;
    float m_vx = 0.f, m_vy = 0.f;
    float m_angle, m_angularVelocity;
    
    float m_mass;
    float m_inverseMass;
    
    bool m_isSelected = false;
};