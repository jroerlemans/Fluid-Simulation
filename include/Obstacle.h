#pragma once
class FluidGrid;

class Obstacle {
public:
    virtual ~Obstacle() = default;
    virtual void apply(FluidGrid& grid) const = 0;
    virtual void draw() const = 0;
};