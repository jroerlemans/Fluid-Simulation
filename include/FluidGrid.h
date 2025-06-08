#pragma once
#include <vector>
#include <cstddef>
#include "Util.h"

class FluidGrid {
public:
    explicit FluidGrid(int N);

    int    size() const           { return m_N; }
    float* u()                    { return m_u.data(); }
    float* v()                    { return m_v.data(); }
    float* dens()                 { return m_dens.data(); }
    void   reset();

    // Previous-state buffers are public for fast access from FluidSolver
    std::vector<float> m_uPrev, m_vPrev, m_densPrev;

private:
   
    int m_N;
    size_t m_arrSz;
    std::vector<float> m_u, m_v, m_dens;
};