#include "FluidGrid.h"
#include <algorithm>

FluidGrid::FluidGrid(int N):m_N(N),m_arrSz(size_t(N+2)*(N+2)),
    m_u(m_arrSz),m_v(m_arrSz),m_dens(m_arrSz),m_vort(m_arrSz),m_temp(m_arrSz),
    m_uPrev(m_arrSz),m_vPrev(m_arrSz),m_densPrev(m_arrSz),m_tempPrev(m_arrSz){}

void FluidGrid::reset(){
    std::fill(m_u.begin(), m_u.end(), 0.f);
    std::fill(m_v.begin(), m_v.end(), 0.f);
    std::fill(m_dens.begin(), m_dens.end(), 0.f);
    std::fill(m_vort.begin(), m_vort.end(), 0.f);
    std::fill(m_temp.begin(), m_temp.end(), 0.f); // New
    std::fill(m_uPrev.begin(), m_uPrev.end(), 0.f);
    std::fill(m_vPrev.begin(), m_vPrev.end(), 0.f);
    std::fill(m_densPrev.begin(), m_densPrev.end(), 0.f);
    std::fill(m_tempPrev.begin(), m_tempPrev.end(), 0.f); // New
}