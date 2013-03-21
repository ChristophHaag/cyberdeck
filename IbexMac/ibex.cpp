//============================================================================
// Name        : ibex.cpp
// Author      : Hesham Wahba
// Version     : 0.2
// Copyright   : Copyright Hesham Wahba 2012
// Description : IBEX Virtual Reality 3D desktop
//============================================================================

// --- Standard library ------------------------------------------------------
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>
#include <cstdio>
#include <cmath>
#include <string>
#include <iostream>
#include <iomanip>
#include <set>
#include <map>
#include <vector>

// --- OpenGL ----------------------------------------------------------------
#define GLX_GLXEXT_PROTOTYPES
#ifdef __APPLE__

#include "GL/glew.h"
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>
#include <GLUT/glut.h>

#else

#include <GL/glew.h>
#include <GL/glxew.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>
#include <GL/glxext.h>
#include <GL/glut.h>
#include <GL/glu.h>

#endif

// --- X11 -------------------------------------------------------------------
//#include "opengl_setup_x11.h"

#include "opengl_helpers.h"
#include "iphone_orientation_plugin/iphone_orientation_listener.h"
#include "distortions.h"

#define HAVE_LIBJPEG 1
//#include <jpeglib.h>
//#include "glm/glm.h"

#include "RendererPlugin.h"

//#define ENABLE_OGRE3D 1
//#define ENABLE_IRRLICHT 1

#ifdef ENABLE_OGRE3D
#include "ogre3d_plugin/TutorialApplication.h"
#endif

#ifdef ENABLE_IRRLICHT
#include "irrlicht_plugin/irrlicht_plugin.h"
#endif

#include "SimpleWorldRendererPlugin.h"

#include "ibex.h"

#include "sixense_controller.h"

RendererPlugin *renderer;

GLfloat top, bottom;

// TODO: get rid of global variables
bool controlDesktop  = 1;

// external variables
bool resetPosition          = 0;
bool showGround             = 0;
bool barrelDistort          = 1;
bool ortho                  = 1;
bool renderToTexture        = 1;
bool USE_FBO                = 1;
#ifdef ENABLE_OGRE3D
bool OGRE3D                 = 1;
#else
bool OGRE3D                 = 0;
#endif
bool IRRLICHT               = 0;
bool SBS                    = 1;

GLuint fbos[2];
GLuint textures[2];

GLuint depthBuffer;

GLuint desktopFBO;
GLuint desktopTexture(0);
GLuint cursor(0);

GLfloat cursorPosX(0);
GLfloat cursorPosY(0);

GLuint cursorTexture(0);

static int xi_opcode;
static int xfixes_event_base;

double walkForward = 0;
double strafeRight = 0;

int dpy = 0;
int display = 0;
unsigned long window = 0;
unsigned long context = 0;

GLfloat physicalWidth = 1440.0;
GLfloat physicalHeight = 900.0;
//GLfloat physicalWidth = 2560.0;
//GLfloat physicalHeight = 1600.0;

GLfloat width = 1440.0;
GLfloat height = 900.0;




/* From: http://stackoverflow.com/questions/5002254/
                adapt-existing-code-for-opengl-stereoscopic-rendering
 * Stereo SBS projection info

glMatrixMode(GL_PROJECTION);
glLoadIdentity();
stereo_offset = eye * near * parallax_factor / convergence_distance;
glFrustum(stereo_offset + left, stereo_offset + right, bottom, top, near, far);

glMatrixMode(GL_MODELVIEW);
glLoadIdentity();
glTranslatef(eye * parallax_factor * convergence_distance, 0, 0);

// now use gluLookAt here as this were a normal 2D rendering

 * parallax_factor should be no larger than the ratio of
 * half_eye_distance / screen_width, so the larger the screen gets the smaller
 * the parallax_factor is.
 * A good value for parallax_factor for computer display use is 0.05,
 * for large screens (think cinema) it's something like 0.01
 */

// ---------------------------------------------------------------------------
// Function: prep_framebuffers
// Design:   Belongs to OpenGL component
// Purpose:
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
void prep_framebuffers()
{
  if (!GL_ARB_framebuffer_object ||
      !glewGetExtension("GL_ARB_framebuffer_object")) {
    std::cerr << "NO FBO SUPPORT" << std::endl;
    exit(EXIT_FAILURE);
  } else {
    std::cout << "GL_ARB_framebuffer_object SUPPORT" << std::endl;
  }

//    if (!checkForErrors()) {
//        std::cerr << "Stage 0w - Problem generating desktop FBO" << std::endl;
//        exit(EXIT_FAILURE);
//    }
    checkForErrors();
    
  glGenFramebuffers(1, &desktopFBO);
    if (!checkForErrors()) {
        std::cerr << "Stage 0z - Problem generating desktop FBO" << std::endl;
        exit(EXIT_FAILURE);
    }
  glBindFramebuffer(GL_FRAMEBUFFER, desktopFBO);
    if (!checkForErrors() ) {
        std::cerr << "Stage 0y - Problem generating desktop FBO" << std::endl;
        exit(EXIT_FAILURE);
    }
  if(desktopTexture == 0) {
    glGenTextures(1, &desktopTexture);
  }
  glBindTexture(GL_TEXTURE_2D, desktopTexture);
    if (!checkForErrors()) {
        std::cerr << "Stage 0a - Problem generating desktop FBO" << std::endl;
        exit(EXIT_FAILURE);
    }
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    if (!checkForErrors()) {
        std::cerr << "Stage 0b - Problem generating desktop FBO" << std::endl;
        exit(EXIT_FAILURE);
    }
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    if (!checkForErrors()) {
        std::cerr << "Stage 0c - Problem generating desktop FBO" << std::endl;
        exit(EXIT_FAILURE);
    }
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
               GL_RGBA, GL_UNSIGNED_BYTE, 0);
    if (!checkForErrors()) {
        std::cerr << "Stage 0d - Problem generating desktop FBO" << std::endl;
        exit(EXIT_FAILURE);
    }
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                        GL_TEXTURE_2D, desktopTexture, 0);
  if (!checkForErrors() ||
      glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cerr << "Stage 0e - Problem generating desktop FBO" << std::endl;
    exit(EXIT_FAILURE);
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindTexture(GL_TEXTURE_2D, 0);

  glGenFramebuffers(2, fbos);
  glGenRenderbuffers(1, &depthBuffer);
  glGenTextures(2, textures);

  for (int i = 0; i < 2; ++i) {
    glBindFramebuffer(GL_FRAMEBUFFER, fbos[i]);
    glBindTexture(GL_TEXTURE_2D, textures[i]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                            GL_TEXTURE_2D, textures[i], 0);

    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    if (i == 0) {
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24,
                            width, height);
    }
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER, depthBuffer);
    if (!checkForErrors()) {
      std::cerr << "Stage 1 - Problem generating FBO " << i << std::endl;
      exit(EXIT_FAILURE);
    }

    std::cout << "Generating FBO #" << i << std::endl;
    std::cout << "FBO: " << width << "x" << height << std::endl;

    if (!checkForErrors() ||
        glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      std::cerr << "Stage 2 - Problem generating FBO " << i << std::endl;
      exit(EXIT_FAILURE);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
}

// ---------------------------------------------------------------------------
// Function: renderDesktopToTexture
// Design:   Interface between OpenGL and Input Hardware Control component
// Purpose:
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
void renderDesktopToTexture()
{
    // on mac already taken care of
}



// ---------------------------------------------------------------------------
// Function: initedOpenGL
// Design:   Belongs (mostly) to OpenGL component
// Purpose:  Whether OpenGL has been inited for rendering to texture
// Updated:  Sep 10, 2012
// TODO:     Split into separate functions
// ---------------------------------------------------------------------------
static bool initedOpenGL = false;
bool didInitOpenGL() {
//  std::cerr << "didInitOpenGL: " << initedOpenGL << std::endl;
  return initedOpenGL;
}

// ---------------------------------------------------------------------------
// Function: renderGL
// Design:   Belongs (mostly) to OpenGL component
// Purpose:  Main rendering of the 3D desktop
// Updated:  Sep 10, 2012
// TODO:     Split into separate functions
// ---------------------------------------------------------------------------
void renderGL(Desktop3DLocation& loc, double timeDiff_)
{
  if (!initedOpenGL) {
    initedOpenGL = true;

    if(renderer->getWindowID()) window = renderer->getWindowID();
//    if (OGRE3D || IRRLICHT) {
      glewExperimental = true;
      GLenum err = glewInit();
      std::cerr << "Inited GLEW: " << err << std::endl;
      if (GLEW_OK != err) {
        // GLEW failed!
        exit(1);
      }
//    }
      
      if(!OGRE3D) {
          bool success = init_distortion_shader();
          if (!success) {
              std::cerr << "Failed to init distortion shader!" << std::endl;
              exit(1);
          }

          if (USE_FBO) prep_framebuffers();
      }
  }

  renderer->step(loc, timeDiff_);
}

void resizeGL(unsigned int width, unsigned int height)
{
    /* prevent divide-by-zero */
    if (height == 0)
        height = 1;
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //    gluPerspective(45.0f, 1, 0.1f, 100.0f);//(GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
    gluPerspective(110.0f, 0.81818181, 0.01f, 1000.0f);
    //    gluPerspective(120.0f, 0.75, 0.1f, 100.0f);//(GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
}

// ---------------------------------------------------------------------------
// Function: initGL
// Design:   Belongs to OpenGL component
// Purpose:
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
void initGL()
{
  glShadeModel(GL_SMOOTH);
  glClearColor(0.0f, 0.1f, 0.0f, 1.0f);
  glClearDepth(1.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

  // We use resizeGL once to set up our initial perspective
  resizeGL(physicalWidth,physicalHeight);//width, height);


    top = 1.0f;
    bottom = 0.0f;

//  top = -top;
//  bottom = -bottom;

  glFlush();
}


double relativeMouseX = 0;
double relativeMouseY = 0;

static bool jump = false;

Ibex::Ibex(int argc, char ** argv) {
    int c;
    while (argc > 0 && (c = getopt(argc, argv, "oihm")) != -1)
        switch (c) {
            case 'o':
                OGRE3D = true;
                break;
            case 'i':
                IRRLICHT = true;
                break;
            case 'm':
                SBS = false;
                break;
            case 'h':
            case '?':
            default:
                if (isprint(optopt))
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
                return;// 1;
        }
    
    //  prep_root();
    
    std::cerr << "Virtual width: " << width << " height: " << height << std::endl;
    
    if(!OGRE3D)
        initGL();
    
    setup_iphone_listener();
    
    if(OGRE3D) {
#ifdef ENABLE_OGRE3D
//        unsigned long root = 0;
        unsigned long screen = 0;
        unsigned int visinfo = 0;
        unsigned int dpy2 = 0;
//        createWindow(dpy, root);
        renderer = new Ogre3DRendererPlugin(&dpy2, screen, window, &visinfo, (unsigned long)context);
        renderer->init();
        renderer->processEvents();
        return;
        
#endif
    } else if(IRRLICHT) {
#ifdef ENABLE_IRRLICHT
        renderer = new IrrlichtRendererPlugin();
        renderer->init();
#endif
        dpy = display;
        //    irrlicht_run_loop();
    } else {
        //    createWindow(dpy, root);
        
        renderer = new SimpleWorldRendererPlugin();
        renderer->init();
    }
    
    std::cerr << "Physical Width x Height: " << physicalWidth << "x" << physicalHeight << std::endl;
    
    glutInit(&argc, argv);
    
    //  prep_overlay();
    if(renderer->getWindowID()) {
        window = renderer->getWindowID();
    }
    
    std::cerr << "dpy: " << dpy << ", display: " << display << ", " << window << std::endl;
}

void processRawMotion(double relativeMouseXDelta, double relativeMouseYDelta, Desktop3DLocation& loc)
{
  double xRotation, yRotation;


        yRotation = loc.getYRotation();
        yRotation += relativeMouseYDelta /
                     (double)width * 180.0;
        loc.setYRotation(yRotation);
        relativeMouseY = relativeMouseYDelta;

        xRotation = loc.getXRotation();
        xRotation += relativeMouseXDelta /
                     (double)width * 180.0;
        loc.setXRotation(xRotation);
        relativeMouseX = relativeMouseXDelta;
}

void Ibex::render(double timeDiff) {
    if (controlDesktop) {
        walkForward = strafeRight = 0;
    }
    double rx = relativeMouseX;
    double ry = relativeMouseY;
    relativeMouseX = 0;
    relativeMouseY = 0;
    
    if(!OGRE3D) {
        processRawMotion(ry, rx, desktop3DLocation);
        desktop3DLocation.walk(walkForward+sixenseWalkForward, strafeRight+sixenseStrafeRight, timeDiff);
    }
    
    if(resetPosition) {
        resetPosition = 0;
        desktop3DLocation.resetState();
    }
    
    renderer->setDesktopTexture(desktopTexture);
    if(OGRE3D) {
        renderer->move(walkForward+sixenseWalkForward, strafeRight+sixenseStrafeRight, jump, ry, rx);
    }
    renderer->processEvents();
    
    renderGL(desktop3DLocation, timeDiff);
//    ts_start = ts_current;
}
