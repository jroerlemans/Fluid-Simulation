#pragma once
class FluidSolver;
struct Source { virtual ~Source()=default; virtual void apply(FluidSolver&)=0; };
