#pragma once
#include "MovableObstacle.h" // Inherit from the new base class
#include <vector>

class MovableRectObstacle : public MovableObstacle {
public:
    MovableRectObstacle(int x, int y, int w, int h, int gridN);
    ~MovableRectObstacle() override = default;

    // --- Obstacle Interface ---
    void apply(FluidGrid& grid) const override;
    void draw() const override;

    // --- MovableObstacle Interface ---
    void updatePosition(int newX, int newY); // For legacy mouse dragging
    bool contains(int x, int y) const override;
    
    // --- Rect-specific methods for collision detection ---
    void getVertices(std::vector<Vec2>& vertices) const;
    Vec2 getCenter() const;
    int getWidth() const { return m_w; }
    int getHeight() const { return m_h; }

private:
    int m_w, m_h;
    int m_gridN;
};