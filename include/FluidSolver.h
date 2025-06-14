#pragma once
#include "FluidGrid.h"
#include "BoundarySolver.h"
#include "SolidBoundary.h"
#include "ObstacleManager.h"
#include <vector>
#include <memory>

class FluidSolver {
public:
    FluidSolver(FluidGrid& grid, ObstacleManager* manager, float dt,float diff,float visc,float vort);

    void step();
    void addDensity(int i,int j,float amount);
    void addVelocity(int i,int j,float u,float v);
    void addBoundary(SolidBoundary* b);

    // run-time parameters
    float force  = 5.f;
    float source = 100.f;
    float dt;

private:
    void diffuse (int b,float* x,float* x0,float diff);
    void project (float* u,float* v,float* p,float* div);
    void advect  (int b,float* d,float* d0,float* u,float* v);

    void confine (float* u, float* v, float* w);

    FluidGrid* g;  
    std::vector<SolidBoundary*> m_boundaries;
    ObstacleManager* m_obstacleManager; 
    float diff, visc, vort;
};
