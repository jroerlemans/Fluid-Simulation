#include "FluidSolver.h"
#include "Util.h" 
#include <cstring>
#include <cmath>
#include <algorithm> 

FluidSolver::FluidSolver(FluidGrid& grid, ObstacleManager* manager, float dt_,float diff_,float visc_)
    :g(&grid),m_obstacleManager(manager),dt(dt_),diff(diff_),visc(visc_){} 

void FluidSolver::addBoundary(SolidBoundary* b) {
    m_boundaries.push_back(b);
}

// ===== public helpers =====================================================
void FluidSolver::addDensity(int i,int j,float amount){
    g->dens()[IX(i,j,g->size())]+=amount;
}
void FluidSolver::addVelocity(int i,int j,float uu,float vv){
    int N=g->size();
    g->u()[IX(i,j,N)]+=uu;
    g->v()[IX(i,j,N)]+=vv;
}

// ===== source/force application ===========================================
static void addSource(int N,float* x,const float* s,float dt){
    int size=(N+2)*(N+2);
    for(int i=0;i<size;++i) x[i]+=dt*s[i];
}

// ===== Gauss-Seidel linear solver =========================================
static void linSolve(int N,int b,float* x,float* x0,float a,float c){
    for(int k=0;k<20;++k){
        for(int i=1;i<=N;++i)
            for(int j=1;j<=N;++j)
                x[IX(i,j,N)]=(x0[IX(i,j,N)]+
                a*(x[IX(i-1,j,N)]+x[IX(i+1,j,N)]+x[IX(i,j-1,N)]+x[IX(i,j+1,N)]))/c;
        BoundarySolver::setBounds(N,b,x);
    }
}
// ===== private steps ======================================================
void FluidSolver::diffuse(int b,float* x,float* x0,float diffc){
    int N=g->size(); float a=dt*diffc*N*N;
    linSolve(N,b,x,x0,a,1+4*a);
}
void FluidSolver::advect(int b,float* d,float* d0,float* u,float* v){
    int N=g->size(); float dt0=dt*N;
    for(int i=1;i<=N;++i)for(int j=1;j<=N;++j){
        float x=i-dt0*u[IX(i,j,N)], y=j-dt0*v[IX(i,j,N)];
        if(x<0.5f) x=0.5f; if(x>N+0.5f) x=N+0.5f; int i0=int(x); int i1=i0+1;
        if(y<0.5f) y=0.5f; if(y>N+0.5f) y=N+0.5f; int j0=int(y); int j1=j0+1;
        float s1=x-i0, s0=1-s1, t1=y-j0, t0=1-t1;
        d[IX(i,j,N)] =
            s0*(t0*d0[IX(i0,j0,N)]+t1*d0[IX(i0,j1,N)]) +
            s1*(t0*d0[IX(i1,j0,N)]+t1*d0[IX(i1,j1,N)]);
    }
    BoundarySolver::setBounds(N,b,d);
}
void FluidSolver::project(float* u,float* v,float* p,float* div){
    int N=g->size();
    for(int i=1;i<=N;++i)for(int j=1;j<=N;++j){
        div[IX(i,j,N)]=-0.5f*(u[IX(i+1,j,N)]-u[IX(i-1,j,N)]
                            + v[IX(i,j+1,N)]-v[IX(i,j-1,N)])/N;
        p[IX(i,j,N)]=0;
    }
    BoundarySolver::setBounds(N,0,div); BoundarySolver::setBounds(N,0,p);
    linSolve(N,0,p,div,1,4);
    for(int i=1;i<=N;++i)for(int j=1;j<=N;++j){
        u[IX(i,j,N)]-=0.5f*N*(p[IX(i+1,j,N)]-p[IX(i-1,j,N)]);
        v[IX(i,j,N)]-=0.5f*N*(p[IX(i,j+1,N)]-p[IX(i,j-1,N)]);
    }
    BoundarySolver::setBounds(N,1,u); BoundarySolver::setBounds(N,2,v);
}

// ===== main solver tick ====================================================
void FluidSolver::step(){
    int N=g->size();
    auto *u=g->u(), *v=g->v(),
         *u0=g->m_uPrev.data(), *v0=g->m_vPrev.data(),
         *dens=g->dens(), *dens0=g->m_densPrev.data();

    addSource(N, u, u0, dt);
    addSource(N, v, v0, dt);
    addSource(N, dens, dens0, dt);

    std::swap(u0, u); diffuse (1,u,u0,visc);
    std::swap(v0, v); diffuse (2,v,v0,visc);
    project (u,v,u0,v0);

    std::swap(u0, u); std::swap(v0, v);
    advect  (1,u,u0,u0,v0);
    advect  (2,v,v0,u0,v0);
    project (u,v,u0,v0);

    std::swap(dens0, dens); diffuse (0,dens,dens0,diff);
    std::swap(dens0, dens); advect  (0,dens,dens0,u,v);

    // Apply all solid boundary conditions at the end of the step
    for (auto& b : m_boundaries) {
        b->applyTo(*g);
    }

    m_obstacleManager->update(dt);
}
