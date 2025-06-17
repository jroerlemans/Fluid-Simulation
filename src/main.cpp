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

void print(const char* str) {
    std::cout << str << std::endl;
}

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
static FluidSolver solver(grid, nullptr);

// --- Window sizes ---
static int simulation_size = 512; // Size of the simulation (in pixels)
static int ui_size = 200; // Size of the UI panel (in pixels)
static bool showVel = false;
static int  winX = simulation_size + ui_size;
static int  winY = simulation_size;
static int  mouseDown[3] = {0,0,0};
static int  omx, omy, mx, my;

// --- Interaction globals ---
static MovableObstacle* selected_obstacle = nullptr;
static bool is_dragging_object = false;
static bool is_dragging_slider = false;
static int current_source_type = 0;

// --- UI Globals for Slider ---
const float slider_x = 50.f;
const float slider_y = 30.f;
const float slider_w = 200.f;
const float dt_min = 0.002f;
const float dt_max = 0.4f;

// --- UI Globals for slider --- 
const int slider_width = 100;
const int handle_size = 15;
const int padding = 50;
const int spacing = 50;
static int slider_idx = -1;

typedef struct {
    float dt; 
    float diff;
    float visc;
    float vort;
    float N;
} FluidParameters;

FluidParameters params = {0.1f, 0.f, 0.f, 5.f, 64};

typedef enum {
    INT,
    FLOAT
} Type;

typedef struct {
    float min_value;
    float max_value;
    float* value;
    const char* label;
    Type type;
} Slider;

Slider sliders[] = {
    {0.01f, 0.4f, &params.dt, "Delta time", FLOAT},
    {0.f, .01f, &params.diff, "Diffusion", FLOAT},
    {0.f, .01f, &params.visc, "Viscosity", FLOAT},
    {0.f, 20.f, &params.vort, "Vorticity", FLOAT},
    {5, 1024, &params.N, "N", FLOAT},
};

// --- drawing helpers ---
static void drawVelocity(){
    int N = grid.size(); float h = 1.0f/N; glColor3f(1,1,1); glLineWidth(1.0f); glBegin(GL_LINES);
    for(int i=1;i<=N;++i){ float x=(i-0.5f)*h; for(int j=1;j<=N;++j){ float y=(j-0.5f)*h; float u=grid.u()[IX(i,j,N)]; float v=grid.v()[IX(i,j,N)]; glVertex2f(x,y); glVertex2f(x+u, y+v); } }
    glEnd();
}
static void drawDensity(){
    int N = grid.size(); float h = 1.0f/N; glBegin(GL_QUADS);
    for(int i=0; i<N; i++){ float x = i*h; for(int j=0; j<N; j++){ float y = j*h;
            float d00 = grid.dens()[IX(i,j,N)],     t00 = grid.temp()[IX(i,j,N)];
            float d10 = grid.dens()[IX(i+1,j,N)],   t10 = grid.temp()[IX(i+1,j,N)];
            float d11 = grid.dens()[IX(i+1,j+1,N)], t11 = grid.temp()[IX(i+1,j+1,N)];
            float d01 = grid.dens()[IX(i,j+1,N)],   t01 = grid.temp()[IX(i,j+1,N)];
            
            glColor3f(std::min(1.f, d00 + t00*0.5f), std::min(1.f, d00), std::max(0.f, d00 - t00*0.5f)); glVertex2f(x,y);
            glColor3f(std::min(1.f, d10 + t10*0.5f), std::min(1.f, d10), std::max(0.f, d10 - t10*0.5f)); glVertex2f(x+h,y);
            glColor3f(std::min(1.f, d11 + t11*0.5f), std::min(1.f, d11), std::max(0.f, d11 - t11*0.5f)); glVertex2f(x+h,y+h);
            glColor3f(std::min(1.f, d01 + t01*0.5f), std::min(1.f, d01), std::max(0.f, d01 - t01*0.5f)); glVertex2f(x,y+h);
    }} glEnd();
}

static void drawUI() {
    

    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity(); gluOrtho2D(0, winX, 0, winY);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();

    glColor3f(0.4f, 0.4f, 0.4f); glBegin(GL_LINES); glVertex2f(slider_x, slider_y); glVertex2f(slider_x + slider_w, slider_y); glEnd();
    
    float handle_x = slider_x + ((dt - dt_min) / (dt_max - dt_min)) * slider_w;
    glColor3f(0.9f, 0.9f, 0.9f); glBegin(GL_QUADS);
    glVertex2f(handle_x - 4, slider_y - 8); glVertex2f(handle_x + 4, slider_y - 8); glVertex2f(handle_x + 4, slider_y + 8); glVertex2f(handle_x - 4, slider_y + 8); glEnd();

    char dt_text[64]; sprintf(dt_text, " Time (dt): %.3f", dt);
    glColor3f(1.0f, 1.0f, 1.0f); glRasterPos2f(slider_x, slider_y + 15);
    for (const char* c = dt_text; *c != '\0'; ++c) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c); }
    
    glMatrixMode(GL_PROJECTION); glPopMatrix(); glMatrixMode(GL_MODELVIEW); glPopMatrix();
}

static void getFromUI(){
    std::fill(grid.m_uPrev.begin(), grid.m_uPrev.end(), 0.f); 
    std::fill(grid.m_vPrev.begin(), grid.m_vPrev.end(), 0.f); 
    std::fill(grid.m_densPrev.begin(), grid.m_densPrev.end(), 0.f); 
    std::fill(grid.m_tempPrev.begin(), grid.m_tempPrev.end(), 0.f);
    
    if(!mouseDown[0] && !mouseDown[2]) return;
    if(is_dragging_object || is_dragging_slider) return;
    int i = int(( mx/float(simulation_size))*N+1), j = int((my/float(simulation_size))*N+1);
    if(i<1||i>N||j<1||j>N) return;

    if(mouseDown[0]){ 
        // FIX: Scale the added velocity by dt to make it consistent
        float force_x = cmd_force * (mx - omx) * dt;
        float force_y = cmd_force * (my - omy) * dt;
        solver.addVelocity(i, j, force_x, force_y); 
    }
    if(mouseDown[2]){
        // FIX: Scale the added sources by dt
        if (current_source_type == 1) { 
            solver.addTemperature(i, j, cmd_source * 2.0f * dt);
        } else {
            solver.addDensity(i, j, cmd_source * dt);
        }
    }
    omx = mx; omy = my;
}

static void display_ui() {
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity(); gluOrtho2D(0, ui_size, 0, winY);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();

    int n_sliders = sizeof(sliders) / sizeof(sliders[0]);

    for (int i = 0; i < n_sliders; i++) {
        Slider* slider = &sliders[i];

        int x = padding;
        int y = padding + spacing * i;

        glColor3f(0.4f, 0.4f, 0.4f); glBegin(GL_LINES); glVertex2f(x, y); glVertex2f(x + slider_width, y); glEnd();

        int handle_position = (int) (slider_width * (*slider->value - slider->min_value) / (slider->max_value - slider->min_value));

        glPointSize(handle_size);
        glBegin(GL_POINTS);
        glColor3f(1, 1, 1);
        glVertex2f(x + handle_position, y);
        glEnd();
        
        char label[64];
        switch (slider->type) {
            case INT:
                sprintf(label, "%s: %3d", slider->label, *slider->value);
                break;
            case FLOAT:
                sprintf(label, "%s: %.3f", slider->label, *slider->value);
                break;
        }

        glColor3f(1.0f, 1.0f, 1.0f); glRasterPos2f(x, y + 15);
        for (const char* c = label; *c != '\0'; ++c) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c); }
    }
    
    glMatrixMode(GL_PROJECTION); glPopMatrix(); glMatrixMode(GL_MODELVIEW); glPopMatrix();
}

static void display_simulation() {
    glMatrixMode(GL_PROJECTION); glLoadIdentity(); gluOrtho2D(0, 1, 0, 1);
    if(showVel) drawVelocity(); else drawDensity(); 
    if(obstacleManager) obstacleManager->draw(); 
}

static void display(){ 
    glClear(GL_COLOR_BUFFER_BIT);

    glViewport(0, 0, simulation_size, simulation_size);
    display_simulation();

    glViewport(simulation_size, 0, ui_size, winY);
    display_ui();

    glutSwapBuffers(); 
}

static void idle(){ 
    solver.dt = params.dt;
    solver.diff = params.diff;
    solver.visc = params.visc;
    solver.vort = params.vort;

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
        case 'b': case 'B':
            solver.buoyancy_on = !solver.buoyancy_on;
            printf("Buoyancy %s\n", solver.buoyancy_on ? "ON" : "OFF");
            break;
        case 'v': case 'V': showVel=!showVel; break;
        case 'q': case 'Q': std::exit(0); break;
        case '1': 
            if (obstacleManager) obstacleManager->addFixedRect(i-2,j-2,5,5); 
            break;
        case '2':
            if (obstacleManager) obstacleManager->addMovableRect(i-4,j-4,8,16); 
            break;
        case '3':
            if (obstacleManager) obstacleManager->addDisk(i, j, 6);
            break;
    }
}


// if the point is inside a slider handle, this returns the index of slider
// else it returns -1
static int slider_index(int x, int y) {
    int n_sliders = sizeof(sliders) / sizeof(sliders[0]);

    for (int i = 0; i < n_sliders; i++) {
        Slider* slider = &sliders[i];
        
        int handle_position = (int) (slider_width * (*slider->value - slider->min_value) / (slider->max_value - slider->min_value));
        int handle_x = padding + handle_position;
        int handle_y = padding + i*spacing;

        if (2*std::abs(handle_x - x) < handle_size && 2*std::abs(handle_y - y) < handle_size) {
            return i;
        }
    }
        
    return -1;
}

static void mouse_simulation(int button, int state, int x, int y) {
    omx = mx = x; omy = my = y;

    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        if (x >= slider_x - 5 && x <= slider_x + slider_w + 5 && (winY - y) >= slider_y - 10 && (winY - y) <= slider_y + 10) {
            is_dragging_slider = true;
        } else {
            int i = int((mx / float(simulation_size)) * N + 1);
            int j = int((my / float(simulation_size)) * N + 1);

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

    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
        if (glutGetModifiers() & GLUT_ACTIVE_SHIFT) {
            current_source_type = 1;
        } else {
            current_source_type = 0;
        }
    }

    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN && selected_obstacle) {
         selected_obstacle->setAngularVelocity(90.f);
    } else if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP && selected_obstacle) {
         selected_obstacle->setAngularVelocity(0.f);
    }

    mouseDown[button] = (state == GLUT_DOWN);
}

static void mouse_ui(int button, int state, int x, int y) {
    if (state == GLUT_DOWN) {
        slider_idx = slider_index(x, y);
    }
}

static void mouse(int button, int state, int x, int y) {
    if (x < simulation_size) {
        // We invert the y-coordinate so that it matches the opengl coordinate system
        mouse_simulation(button, state, x, winY - y);
    } else {
        mouse_ui(button, state, x - simulation_size, winY - y);
    }
}

static void motion_simulation(int x, int y) {
    if (is_dragging_slider) {
        float new_dt = dt_min + ((x - slider_x) / slider_w) * (dt_max - dt_min);
        dt = std::max(dt_min, std::min(dt_max, new_dt));
    } else if (is_dragging_object && selected_obstacle) {
        float new_i = (x / float(simulation_size)) * N;
        float new_j = (y / float(simulation_size)) * N;
        
        selected_obstacle->updatePosition(Vec2(new_i, new_j));
        
        const float drag_vel_scale = 1.0f; 
        float vx = (x - mx) * (N / float(simulation_size)) * vel_scale;
        float vy = (y - my) * (N / float(simulation_size)) * vel_scale;
        selected_obstacle->setVelocity(vx, vy);
    }

    mx = x; 
    my = y;
}

static void motion_ui(int x, int y) {
    // Convert to x-coordinate to slider coordinate
    int slider_position = x - padding;

    if (slider_idx != -1) {
        slider_position = std::max(0, std::min(slider_position, slider_width));
        Slider* slider = &sliders[slider_idx];
        
        switch (slider->type) {
            case INT:
                *slider->value = (int) (slider->min_value + slider_position / (float) slider_width * (slider->max_value - slider->min_value));

                break;
            case FLOAT:
                *slider->value = slider->min_value + slider_position / (float) slider_width * (slider->max_value - slider->min_value);

                if (slider->label == "N") {
                    N = (int) params.N;
                    obstacleManager.reset(new ObstacleManager(N));
                    grid = FluidGrid(N); 
                    solver = FluidSolver(grid, obstacleManager.get());
                    solver.force = cmd_force;
                    solver.source = cmd_source;
                    solver.addBoundary(obstacleManager.get());
                }
                break;
        }
    }
}

static void motion(int x, int y) {
    if (x < simulation_size) {
        // We invert the y-coordinate so that it matches the opengl coordinate system
        motion_simulation(x, winY - y);
    } else {
        motion_ui(x - simulation_size, winY - y);
    }
}

int main(int argc,char** argv){
    if(argc!=1&&argc!=8){ std::fprintf(stderr, "usage: %s [N dt diff visc vort force source]\n",argv[0]); return 1; }
    if(argc==8){ N=atoi(argv[1]); dt=atof(argv[2]); diff=atof(argv[3]); visc=atof(argv[4]); vort=atof(argv[5]); cmd_force=atof(argv[6]); cmd_source=atof(argv[7]); }
    if(N<9||N>1024){ std::fprintf(stderr,"Error: Grid size N must be between 8 and 1024.\n"); return 1; }
    printf("Using: N=%d dt=%g diff=%g visc=%g vort=%g force=%g source=%g\n",N,dt,diff,visc,vort,cmd_force,cmd_source);

    obstacleManager.reset(new ObstacleManager(N));
    grid = FluidGrid(N); 
    solver = FluidSolver(grid, obstacleManager.get());
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
              "  Right-click : add density\n"
              "  Shift + Right-click : add hot temperature\n"
              "  1           : add a fixed solid block\n"
              "  2           : add a movable solid rectangle\n"
              "  3           : add a movable solid disk\n"
              "  b           : toggle buoyancy on/off\n"
              "  v           : toggle velocity / density display\n"
              "  c           : clear simulation and obstacles\n"
              "  q           : quit\n");
    glutMainLoop();
    return 0;
}
