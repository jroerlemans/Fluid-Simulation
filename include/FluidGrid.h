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

private:
    // CORRECT ORDER: Size variables are now declared before ANYTHING that uses them.
    int m_N;
    size_t m_arrSz;

    // These vectors depend on m_arrSz, so they come after.
    std::vector<float> m_u, m_v, m_dens;

public:
    // These vectors also depend on m_arrSz, so they must also come after.
    // Making them public is fine, but their declaration order is what matters.
    std::vector<float> m_uPrev, m_vPrev, m_densPrev;
};