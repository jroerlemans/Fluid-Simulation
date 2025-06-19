#pragma once
class FluidGrid;

class Obstacle {
public:
    virtual ~Obstacle() = default;
    virtual void apply(FluidGrid& grid) const = 0;
    virtual void draw() const = 0;
    virtual void update(float dt) { };
    virtual void updateFromFluid(FluidGrid& grid, float dt) = 0;
    
};
