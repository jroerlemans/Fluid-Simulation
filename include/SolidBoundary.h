#pragma once
class FluidGrid;
struct SolidBoundary{ virtual ~SolidBoundary()=default; virtual void applyTo(FluidGrid&)=0; };
