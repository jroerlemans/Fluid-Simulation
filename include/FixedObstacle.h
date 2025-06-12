#pragma once
#include "SolidBoundary.h"
#include <vector>
#include <memory>

class FixedObstacle : public SolidBoundary {
public:
    explicit FixedObstacle(int N);

    // Add a rectangular solid region to the obstacle map
    void addRect(int x, int y, int w, int h);
    
    // Implementation of the virtual function from SolidBoundary
    void applyTo(FluidGrid& grid) override;

    // Helper to draw the obstacle for visualization
    void draw() const;

private:
    int m_N;
    std::vector<bool> m_isSolid;
};