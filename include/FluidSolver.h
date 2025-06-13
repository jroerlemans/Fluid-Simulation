#pragma once
#include "FluidGrid.h"
#include "BoundarySolver.h"

class FluidSolver {
public:
    FluidSolver(FluidGrid& grid,float dt,float diff,float visc,float vort);

    void step();                                 // one simulation tick
    void addDensity(int i,int j,float amount);
    void addVelocity(int i,int j,float u,float v);

    // run-time parameters (modifiable from UI)
    float force  = 5.f;
    float source = 100.f;

private:
    void diffuse (int b,float* x,float* x0,float diff);
    void project (float* u,float* v,float* p,float* div);
    void advect  (int b,float* d,float* d0,float* u,float* v);

    void confine (float* u, float* v, float* w);

    FluidGrid* g;  
    float dt, diff, visc, vort;
};
