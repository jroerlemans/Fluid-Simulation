#include <GL/glut.h>
#include <cstdlib>
#include <cstdio>
#include <algorithm> // Added for std::fill
#include "FluidGrid.h"
#include "FluidSolver.h"

// ---------------- runtime-tunable params (parse like Stam’s demo) ----------
static int   N      = 64;
static float dt     = 0.1f;
static float diff   = 0.f;
static float visc   = 0.f;
static float vort   = 5.f;

// parsed in main
static float cmd_force  = 5.f;
static float cmd_source = 100.f;

// --- globals ---------------------------------------------------------------
static FluidGrid   grid(N);
static FluidSolver solver(grid, dt, diff, visc, vort);

static bool showVel = false;
static int  winX = 512, winY = 512;
static int  mouseDown[3] = {0,0,0};
static int  omx, omy, mx, my;

// -------------------- helper drawing --------------------------------------
static void drawVelocity(){
    int N = grid.size();
    float h = 1.0f/N;
    glColor3f(1,1,1);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    for(int i=1;i<=N;++i){
        float x = (i-0.5f)*h;
        for(int j=1;j<=N;++j){
            float y = (j-0.5f)*h;
            float u = grid.u()[IX(i,j,N)];
            float v = grid.v()[IX(i,j,N)];
            glVertex2f(x,y);
            glVertex2f(x+u, y+v);
        }
    }
    glEnd();
}
static void drawDensity(){
    int N = grid.size();
    float h = 1.0f/N;
    glBegin(GL_QUADS);
    for(int i=0; i<N; i++){ // Corrected loop bounds
        float x = i*h;      // Corrected vertex coordinate calculation
        for(int j=0; j<N; j++){ // Corrected loop bounds
            float y = j*h;  // Corrected vertex coordinate calculation
            // Density values are at the vertices of the quad
            float d00 = grid.dens()[IX(i,  j,   N)];
            float d10 = grid.dens()[IX(i+1,j,   N)];
            float d11 = grid.dens()[IX(i+1,j+1, N)];
            float d01 = grid.dens()[IX(i,  j+1, N)];
            glColor3f(d00,d00,d00); glVertex2f(x,     y);
            glColor3f(d10,d10,d10); glVertex2f(x+h,   y);
            glColor3f(d11,d11,d11); glVertex2f(x+h, y+h);
            glColor3f(d01,d01,d01); glVertex2f(x,   y+h);
        }
    }
    glEnd();
}

// ---------------- interaction helpers (mirror demo.c) ---------------------
static void getFromUI(){
    int N = grid.size();
    // clear prev buffers
    std::fill(grid.m_uPrev.begin(),   grid.m_uPrev.end(),   0.f);
    std::fill(grid.m_vPrev.begin(),   grid.m_vPrev.end(),   0.f);
    std::fill(grid.m_densPrev.begin(),grid.m_densPrev.end(),0.f);

    if(!mouseDown[0] && !mouseDown[2]) return;

    int i = int(( mx / float(winX))*N + 1);
    int j = int(((winY-my)/float(winY))*N + 1);
    if(i<1||i>N||j<1||j>N) return;

    if(mouseDown[0]){        // left drag – add velocity
        grid.m_uPrev[IX(i,j,N)] = solver.force*(mx-omx);
        grid.m_vPrev[IX(i,j,N)] = solver.force*(omy-my);
    }
    if(mouseDown[2]){        // right click – add density
        grid.m_densPrev[IX(i,j,N)] = solver.source;
    }
    omx = mx; omy = my;
}

// ---------------- GLUT callbacks ------------------------------------------
static void display(){
    glClear(GL_COLOR_BUFFER_BIT);
    if(showVel) drawVelocity();
    else        drawDensity();
    glutSwapBuffers();
}
static void idle(){
    getFromUI();
    solver.step();                 // main simulation tick
    glutPostRedisplay();
}
static void key(unsigned char c,int,int){
    switch(c){
        case 'c': case 'C': grid.reset(); break;
        case 'v': case 'V': showVel=!showVel; break;
        case 'q': case 'Q': std::exit(0);    break;
    }
}
static void mouse(int button,int state,int x,int y){
    omx = mx = x; omy = my = y;
    mouseDown[button] = (state==GLUT_DOWN);
}
static void motion(int x,int y){ mx=x; my=y; }

// ---------------- main -----------------------------------------------------
int main(int argc,char** argv){
    // ----- parse cmd line like original demo.c -----------------------------
    if(argc!=1 && argc!=7){
        std::fprintf(stderr,
            "usage: %s [N dt diff visc vort force source]\n",argv[0]);
        std::fprintf(stderr,
            "  - example: %s 128 0.1 0 0 10 5 100\n", argv[0]);
        return 1;
    }
    if(argc==8){
        N      = std::atoi (argv[1]);
        dt     = std::atof (argv[2]);
        diff   = std::atof (argv[3]);
        visc   = std::atof (argv[4]);
        vort   = std::atof (argv[5]);
        cmd_force  = std::atof(argv[6]);
        cmd_source = std::atof(argv[7]);
    }

    // --- ADDED: Input validation to prevent bad_alloc crash ---
    if (N < 8 || N > 1024) {
        std::fprintf(stderr, "Error: Grid size N must be between 8 and 1024.\n");
        return 1;
    }

    std::printf("Using: N=%d dt=%g diff=%g visc=%g vort=%g force=%g source=%g\n",
                N,dt,diff,visc,vort,cmd_force,cmd_source);

    grid   = FluidGrid(N);
    solver = FluidSolver(grid,dt,diff,visc,vort);
    solver.force  = cmd_force;
    solver.source = cmd_source;

    // ----- GLUT ------------------------------------------------------------
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE);
    glutInitWindowSize(winX,winY);
    glutCreateWindow("FluidToy – Stable Fluids Demo");

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0,1,0,1);

    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutKeyboardFunc(key);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);

    std::puts("\nControls:\n"
              "  Left-drag   : add velocity\n"
              "  Right-click : add density\n"
              "  v           : toggle velocity / density display\n"
              "  c           : clear\n"
              "  q           : quit\n");

    glutMainLoop();
    return 0;
}
