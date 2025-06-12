#include <GL/glut.h>
#include <cstdlib>
#include <cstdio>
#include <algorithm>
#include <memory>
#include "FluidGrid.h"
#include "FluidSolver.h"
#include "ObstacleManager.h"
#include "MovableRectObstacle.h" // Include for pointer type

// --- runtime-tunable params ---
static int   N      = 64;
static float dt     = 0.1f;
static float diff   = 0.f;
static float visc   = 0.f;
static float cmd_force  = 5.f;
static float cmd_source = 100.f;

// --- globals ---
static FluidGrid   grid(N);
static FluidSolver solver(grid, dt, diff, visc);
static std::unique_ptr<ObstacleManager> obstacleManager;
static bool showVel = false;
static int  winX = 512, winY = 512;
static int  mouseDown[3] = {0,0,0};
static int  omx, omy, mx, my;

// --- Interaction globals ---
static MovableRectObstacle* selected_obstacle = nullptr;
static bool is_dragging = false;

// --- drawing helpers (unchanged) ---
static void drawVelocity(){
    int N = grid.size(); float h = 1.0f/N; glColor3f(1,1,1); glLineWidth(1.0f); glBegin(GL_LINES);
    for(int i=1;i<=N;++i){ float x=(i-0.5f)*h; for(int j=1;j<=N;++j){ float y=(j-0.5f)*h; float u=grid.u()[IX(i,j,N)]; float v=grid.v()[IX(i,j,N)]; glVertex2f(x,y); glVertex2f(x+u, y+v); } }
    glEnd();
}
static void drawDensity(){
    int N = grid.size(); float h = 1.0f/N; glBegin(GL_QUADS);
    for(int i=0; i<N; i++){ float x = i*h; for(int j=0; j<N; j++){ float y = j*h;
            float d00 = grid.dens()[IX(i,j,N)], d10 = grid.dens()[IX(i+1,j,N)], d11 = grid.dens()[IX(i+1,j+1,N)], d01 = grid.dens()[IX(i,j+1,N)];
            glColor3f(d00,d00,d00); glVertex2f(x,y); glColor3f(d10,d10,d10); glVertex2f(x+h,y); glColor3f(d11,d11,d11); glVertex2f(x+h,y+h); glColor3f(d01,d01,d01); glVertex2f(x,y+h);
    }} glEnd();
}

// --- interaction helper (unchanged) ---
static void getFromUI(){
    std::fill(grid.m_uPrev.begin(), grid.m_uPrev.end(), 0.f); std::fill(grid.m_vPrev.begin(), grid.m_vPrev.end(), 0.f); std::fill(grid.m_densPrev.begin(), grid.m_densPrev.end(), 0.f);
    if(!mouseDown[0] && !mouseDown[2]) return;
    if(is_dragging) return; // Don't add sources while dragging
    int i = int(( mx/float(winX))*N+1), j = int(((winY-my)/float(winY))*N+1);
    if(i<1||i>N||j<1||j>N) return;
    if(mouseDown[0]){ solver.addVelocity(i, j, solver.force*(mx-omx), solver.force*(omy-my)); }
    if(mouseDown[2]){ solver.addDensity(i, j, solver.source); }
    omx = mx; omy = my;
}

// --- GLUT callbacks ---
static void display(){ glClear(GL_COLOR_BUFFER_BIT); if(showVel) drawVelocity(); else drawDensity(); if(obstacleManager) obstacleManager->draw(); glutSwapBuffers(); }
static void idle(){ getFromUI(); solver.step(); glutPostRedisplay(); }

static void key(unsigned char c, int x, int y){
    mx = x; my = y;
    int i = int((mx / float(winX)) * N + 1);
    int j = int(((winY - my) / float(winY)) * N + 1);
    switch(c){
        case 'c': case 'C':
            grid.reset(); if (obstacleManager) obstacleManager->clear();
            selected_obstacle = nullptr; is_dragging = false;
            break;
        case 'v': case 'V': showVel=!showVel; break;
        case 'q': case 'Q': std::exit(0); break;
        case '1': if (obstacleManager) obstacleManager->addFixedRect(i-2,j-2,5,5); break;
        case '2': if (obstacleManager) obstacleManager->addMovableRect(i-4,j-4,8,8); break;
    }
}

static void mouse(int button, int state, int x, int y) {
    omx = mx = x; omy = my = y;
    int i = int((mx / float(winX)) * N + 1);
    int j = int(((winY - my) / float(winY)) * N + 1);

    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        selected_obstacle = obstacleManager->findMovableAt(i, j);
        if (selected_obstacle) {
            is_dragging = true;
            selected_obstacle->setSelected(true);
        }
    } else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
        if (selected_obstacle) {
            selected_obstacle->setVelocity(0, 0);
            selected_obstacle->setSelected(false);
            selected_obstacle = nullptr;
        }
        is_dragging = false;
    }
    mouseDown[button] = (state == GLUT_DOWN);
}

static void motion(int x, int y) {
    if (is_dragging && selected_obstacle) {
        int new_i = int((x / float(winX)) * N + 1);
        int new_j = int(((winY - y) / float(winY)) * N + 1);
        selected_obstacle->updatePosition(new_i - 4, new_j - 4); // Center on mouse

        float vx = (x - mx) * (N / float(winX)) / dt;
        float vy = (my - y) * (N / float(winY)) / dt;
        selected_obstacle->setVelocity(vx, vy);
    }
    mx = x; my = y;
}

// --- main ---
int main(int argc,char** argv){
    if(argc!=1&&argc!=7){ std::fprintf(stderr, "usage: %s [N dt diff visc force source]\n",argv[0]); return 1; }
    if(argc==7){ N=atoi(argv[1]); dt=atof(argv[2]); diff=atof(argv[3]); visc=atof(argv[4]); cmd_force=atof(argv[5]); cmd_source=atof(argv[6]); }
    if(N<8||N>1024){ std::fprintf(stderr,"Error: Grid size N must be between 8 and 1024.\n"); return 1; }
    printf("Using: N=%d dt=%g diff=%g visc=%g force=%g source=%g\n",N,dt,diff,visc,cmd_force,cmd_source);

    grid = FluidGrid(N); solver = FluidSolver(grid,dt,diff,visc); solver.force=cmd_force; solver.source=cmd_source;
    obstacleManager.reset(new ObstacleManager(N));
    obstacleManager->addMovableRect(N/2-5, N/2-5, 10, 10); // Add a movable block to start
    solver.addBoundary(obstacleManager.get());

    glutInit(&argc,argv); glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE); glutInitWindowSize(winX,winY); glutCreateWindow("FluidToy – Stable Fluids Demo");
    glMatrixMode(GL_PROJECTION); glLoadIdentity(); gluOrtho2D(0,1,0,1);
    glutDisplayFunc(display); glutIdleFunc(idle); glutKeyboardFunc(key); glutMouseFunc(mouse); glutMotionFunc(motion);
    std::puts("\nControls:\n"
              "  Left-drag   : add velocity (if not on object)\n"
              "  Left-click-drag object: move object\n"
              "  Right-click : add density\n"
              "  1           : add a fixed solid block\n"
              "  2           : add a movable solid block\n"
              "  v           : toggle velocity / density display\n"
              "  c           : clear simulation and obstacles\n"
              "  q           : quit\n");
    glutMainLoop();
    return 0;
}