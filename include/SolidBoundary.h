#pragma once
class FluidGrid;
struct SolidBoundary{ 
    virtual ~SolidBoundary()=default; virtual void applyTo(FluidGrid&)=0;
    virtual void updateObstacles(FluidGrid& grid, float dt)=0;
};
