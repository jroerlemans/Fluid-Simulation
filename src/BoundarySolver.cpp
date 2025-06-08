#include "BoundarySolver.h"
#include "Util.h"

void BoundarySolver::setBounds(int N,int b,float* x){
    for(int i=1;i<=N;++i){
        x[IX(0 ,i,N)] = b==1? -x[IX(1 ,i,N)] : x[IX(1 ,i,N)];
        x[IX(N+1,i,N)] = b==1? -x[IX(N,i,N)]  : x[IX(N,i,N)];
        x[IX(i,0 ,N)] = b==2? -x[IX(i,1 ,N)] : x[IX(i,1 ,N)];
        x[IX(i,N+1,N)] = b==2? -x[IX(i,N ,N)] : x[IX(i,N ,N)];
    }
    x[IX(0 ,0 ,N)]       = .5f*(x[IX(1 ,0 ,N)]+x[IX(0 ,1 ,N)]);
    x[IX(0 ,N+1,N)]     = .5f*(x[IX(1 ,N+1,N)]+x[IX(0 ,N ,N)]);
    x[IX(N+1,0 ,N)]     = .5f*(x[IX(N ,0 ,N)]+x[IX(N+1,1 ,N)]);
    x[IX(N+1,N+1,N)]   = .5f*(x[IX(N ,N+1,N)]+x[IX(N+1,N ,N)]);
}
