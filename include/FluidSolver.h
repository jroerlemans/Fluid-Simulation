#pragma once
#include "FluidGrid.h"
#include "BoundarySolver.h"
#include "SolidBoundary.h" // Include the interface
#include <vector>
#include <memory>

class FluidSolver {
public:
    FluidSolver(FluidGrid& grid,float dt,float diff,float visc);

    void step();
    void addDensity(int i,int j,float amount);
    void addVelocity(int i,int j,float u,float v);
    
    void addBoundary(SolidBoundary* b); // Method to add a boundary

    float force  = 5.f;
    float source = 100.f;

private:
    void diffuse (int b,float* x,float* x0,float diff);
    void project (float* u,float* v,float* p,float* div);
    void advect  (int b,float* d,float* d0,float* u,float* v);

    FluidGrid* g;  
    float dt, diff, visc;
    std::vector<SolidBoundary*> m_boundaries; // Store boundaries
};