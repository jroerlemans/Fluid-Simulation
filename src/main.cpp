#include <GL/glut.h>
#include <cstdlib>
#include <cstdio>
#include <algorithm>
#include <memory>
#include <string>
#include <iostream>
#include "FluidGrid.h"
#include "FluidSolver.h"
#include "ObstacleManager.h"
#include "MovableObstacle.h" // Use the new base class
#include "Vec2.h"

// --- runtime-tunable params ---
static float dt     = 0.1f; 
static int   N      = 64;
static float diff   = 0.f;
static float visc   = 0.f;
static float vort   = 5.f;

// parsed in main
static float cmd_force  = 5.f;
static float cmd_source = 100.f;

// --- globals ---
static FluidGrid   grid(N);
static std::unique_ptr<ObstacleManager> obstacleManager;
static FluidSolver solver(grid, nullptr, dt, diff, visc, vort);

static bool showVel = false;
static int  winX = 512, winY = 512;
static int  mouseDown[3] = {0,0,0};
static int  omx, omy, mx, my;

// --- Interaction globals ---
static MovableObstacle* selected_obstacle = nullptr; // Changed to base class pointer
static bool is_dragging_object = false;
static bool is_dragging_slider = false;

// --- UI Globals for Slider ---
const float slider_x = 50.f;
const float slider_y = 30.f;
const float slider_w = 200.f;
const float dt_min = 0.01f;
const float dt_max = 0.4f;

// --- drawing helpers ---
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

static void drawUI() {
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity(); gluOrtho2D(0, winX, 0, winY);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();

    glColor3f(0.4f, 0.4f, 0.4f); glBegin(GL_LINES); glVertex2f(slider_x, slider_y); glVertex2f(slider_x + slider_w, slider_y); glEnd();
    
    float handle_x = slider_x + ((dt - dt_min) / (dt_max - dt_min)) * slider_w;
    glColor3f(0.9f, 0.9f, 0.9f); glBegin(GL_QUADS);
    glVertex2f(handle_x - 4, slider_y - 8); glVertex2f(handle_x + 4, slider_y - 8); glVertex2f(handle_x + 4, slider_y + 8); glVertex2f(handle_x - 4, slider_y + 8); glEnd();

    char dt_text[64]; sprintf(dt_text, "Delta Time (dt): %.3f", dt);
    glColor3f(1.0f, 1.0f, 1.0f); glRasterPos2f(slider_x, slider_y + 15);
    for (const char* c = dt_text; *c != '\0'; ++c) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c); }
    
    glMatrixMode(GL_PROJECTION); glPopMatrix(); glMatrixMode(GL_MODELVIEW); glPopMatrix();
}

static void getFromUI(){
    std::fill(grid.m_uPrev.begin(), grid.m_uPrev.end(), 0.f); std::fill(grid.m_vPrev.begin(), grid.m_vPrev.end(), 0.f); std::fill(grid.m_densPrev.begin(), grid.m_densPrev.end(), 0.f);
    if(!mouseDown[0] && !mouseDown[2]) return;
    if(is_dragging_object || is_dragging_slider) return;
    int i = int(( mx/float(winX))*N+1), j = int(((winY-my)/float(winY))*N+1);
    if(i<1||i>N||j<1||j>N) return;
    if(mouseDown[0]){ solver.addVelocity(i, j, cmd_force*(mx-omx), cmd_force*(omy-my)); }
    if(mouseDown[2]){ solver.addDensity(i, j, cmd_source); }
    omx = mx; omy = my;
}

static void display(){ 
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION); glLoadIdentity(); gluOrtho2D(0, 1, 0, 1);
    if(showVel) drawVelocity(); else drawDensity(); 
    if(obstacleManager) obstacleManager->draw(); 
    drawUI();
    glutSwapBuffers(); 
}

static void idle(){ 
    solver.dt = dt;
    getFromUI(); 

    if (obstacleManager) {
        obstacleManager->update(dt);
        obstacleManager->handleCollisions();
    }
    
    solver.step();
    glutPostRedisplay(); 
}

static void key(unsigned char c, int x, int y){
    mx = x; my = y;
    int i = int((mx / float(winX)) * N + 1);
    int j = int(((winY - my) / float(winY)) * N + 1);
    switch(c){
        case 'c': case 'C':
            grid.reset(); if (obstacleManager) obstacleManager->clear();
            selected_obstacle = nullptr; is_dragging_object = false;
            break;
        case 'v': case 'V': showVel=!showVel; break;
        case 'q': case 'Q': std::exit(0); break;
        case '1': 
            if (obstacleManager) obstacleManager->addFixedRect(i-2,j-2,5,5); 
            break;
        case '2':
            if (obstacleManager) obstacleManager->addMovableRect(i-4,j-4,8,16); 
            break;
        case '3': // New binding for disks
            if (obstacleManager) obstacleManager->addDisk(i, j, 6);
            break;
    }
}

static void mouse(int button, int state, int x, int y) {
    omx = mx = x; omy = my = y;
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        if (x >= slider_x - 5 && x <= slider_x + slider_w + 5 && (winY - y) >= slider_y - 10 && (winY - y) <= slider_y + 10) {
            is_dragging_slider = true;
        } else {
            int i = int((mx / float(winX)) * N + 1);
            int j = int(((winY - my) / float(winY)) * N + 1);
            selected_obstacle = obstacleManager->findMovableAt(i, j); // This now returns MovableObstacle*
            if (selected_obstacle) {
                is_dragging_object = true;
                selected_obstacle->setSelected(true);
            }
        }
    } else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
        if (selected_obstacle) {
            selected_obstacle->setVelocity(0, 0);
            selected_obstacle->setSelected(false);
            selected_obstacle = nullptr;
        }
        is_dragging_object = false;
        is_dragging_slider = false;
    } 

    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN && selected_obstacle) {
         selected_obstacle->setAngularVelocity(90.f);
    } else if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP && selected_obstacle) {
         selected_obstacle->setAngularVelocity(0.f);
    }

    mouseDown[button] = (state == GLUT_DOWN);
}

static void motion(int x, int y) {
    if (is_dragging_slider) {
        float new_dt = dt_min + ((x - slider_x) / slider_w) * (dt_max - dt_min);
        dt = std::max(dt_min, std::min(dt_max, new_dt));
    }
    else if (is_dragging_object && selected_obstacle) {
        float new_i = (x / float(winX)) * N;
        float new_j = ((winY - y) / float(winY)) * N;
        
        selected_obstacle->updatePosition(Vec2(new_i, new_j));
        
        float vel_scale = dt > 0 ? 1.0f / dt : 0.f;
        float vx = (x - mx) * (N / float(winX)) * vel_scale;
        float vy = (my - y) * (N / float(winY)) * vel_scale;
        selected_obstacle->setVelocity(vx, vy);
    }
    mx = x; 
    my = y;
}

int main(int argc,char** argv){
    if(argc!=1&&argc!=8){ std::fprintf(stderr, "usage: %s [N dt diff visc vort force source]\n",argv[0]); return 1; }
    if(argc==8){ N=atoi(argv[1]); dt=atof(argv[2]); diff=atof(argv[3]); visc=atof(argv[4]); vort=atof(argv[5]); cmd_force=atof(argv[6]); cmd_source=atof(argv[7]); }
    if(N<9||N>1024){ std::fprintf(stderr,"Error: Grid size N must be between 8 and 1024.\n"); return 1; }
    printf("Using: N=%d dt=%g diff=%g visc=%g vort=%g force=%g source=%g\n",N,dt,diff,visc,vort,cmd_force,cmd_source);

    obstacleManager.reset(new ObstacleManager(N));
    grid = FluidGrid(N); 
    solver = FluidSolver(grid, obstacleManager.get(), dt, diff, visc, vort);
    solver.force = cmd_force;
    solver.source = cmd_source;
    solver.addBoundary(obstacleManager.get());

    glutInit(&argc,argv); glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE); glutInitWindowSize(winX,winY); glutCreateWindow("FluidToy – Stable Fluids Demo");
    glutDisplayFunc(display); glutIdleFunc(idle); glutKeyboardFunc(key); glutMouseFunc(mouse); glutMotionFunc(motion);
    
    puts("\nControls:\n"
              "  Left-drag   : add velocity (if not on object)\n"
              "  Left-click-drag object: move object\n"
              "  Left-drag bottom slider: change delta time\n"
              "  Right-click on selected object: rotate object\n"
              "  Middle-click (or Right-click on empty space): add density\n"
              "  1           : add a fixed solid block\n"
              "  2           : add a movable solid rectangle\n"
              "  3           : add a movable solid disk\n"
              "  v           : toggle velocity / density display\n"
              "  c           : clear simulation and obstacles\n"
              "  q           : quit\n");
    glutMainLoop();
    return 0;
}