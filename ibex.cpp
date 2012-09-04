//============================================================================
// Name        : ibix.cpp
// Author      : Hesham Wahba
// Version     :
// Copyright   : Copyright Hesham Wahba 2012
// Description : Hello World in C, Ansi-style
//============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <string>
#include <iostream>
#include <set>
#include <map>

#include <time.h>


#define GLX_GLXEXT_PROTOTYPES
#include <GL/glew.h>
#include <GL/glxew.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>
#include <GL/glxext.h>
#include <GL/glut.h>

#include "opengl_setup_x11.h"

#define HAVE_LIBJPEG 1
#include <jpeglib.h>
#include "glm/glm.h"

Display *dpy;
Screen *scrn;
Window root;
Window overlay;

GLMmodel *pmodel = NULL;

double yRotation = 0.0;
double xRotation = 0.0;
double zRotation = 0.0;
double xPosition = 0.0;
double yPosition = 0.0;
double zPosition = 0.0;

static bool controlDesktop = true;
static bool USE_FBO = 1;

static int xi_opcode;

double walkForward = 0;
double strafeRight = 0;

const double WALK_SPEED = 1.0;

///// FROM COMPIZ
static int errors = 0;

static int
errorHandler (Display     *dpy,
	      XErrorEvent *e)
{

#ifdef DEBUG
    char str[128];
#endif

    errors++;

#ifdef DEBUG
    XGetErrorDatabaseText (dpy, "XlibMessage", "XError", "", str, 128);
    fprintf (stderr, "%s", str);

    XGetErrorText (dpy, e->error_code, str, 128);
    fprintf (stderr, ": %s\n  ", str);

    XGetErrorDatabaseText (dpy, "XlibMessage", "MajorCode", "%d", str, 128);
    fprintf (stderr, str, e->request_code);

    sprintf (str, "%d", e->request_code);
    XGetErrorDatabaseText (dpy, "XRequest", str, "", str, 128);
    if (strcmp (str, ""))
	fprintf (stderr, " (%s)", str);
    fprintf (stderr, "\n  ");

    XGetErrorDatabaseText (dpy, "XlibMessage", "MinorCode", "%d", str, 128);
    fprintf (stderr, str, e->minor_code);
    fprintf (stderr, "\n  ");

    XGetErrorDatabaseText (dpy, "XlibMessage", "ResourceID", "%d", str, 128);
    fprintf (stderr, str, e->resourceid);
    fprintf (stderr, "\n");

    /* abort (); */
#endif

    return 0;
}

int
checkForError (Display *dpy)
{
    int e;

    XSync (dpy, false);

    e = errors;
    errors = 0;

    return e;
}

////////////////////

// OpenGL Errors
static inline bool CheckForErrors()
{
    static bool checkForErrors = true;

    //
    if( !checkForErrors )
    {
        return false;
    }

    //
    const char * errorString = NULL;
    bool retVal = false;

    switch( glGetError() )
    {
		case GL_NO_ERROR:
			retVal = true;
			break;

		case GL_INVALID_ENUM:
			errorString = "GL_INVALID_ENUM";
			break;

		case GL_INVALID_VALUE:
			errorString = "GL_INVALID_VALUE";
			break;

		case GL_INVALID_OPERATION:
			errorString = "GL_INVALID_OPERATION";
			break;

		case GL_INVALID_FRAMEBUFFER_OPERATION:
			errorString = "GL_INVALID_FRAMEBUFFER_OPERATION";
			break;

            // OpenGLES Specific Errors
#ifdef ATHENA_OPENGLES
		case GL_STACK_OVERFLOW:
			errorString = "GL_STACK_OVERFLOW";
			break;

		case GL_STACK_UNDERFLOW:
			errorString = "GL_STACK_UNDERFLOW";
			break;
#endif

		case GL_OUT_OF_MEMORY:
			errorString = "GL_OUT_OF_MEMORY";
			break;

		default:
			errorString = "UNKNOWN";
			break;
    }

    //
    if( !retVal )
    {
        std::cerr << "OpenGL ERROR: " << errorString << std::endl;
        //        assert( retVal );
    }

    //
    return retVal;
}


void allow_input_passthrough (Window w)
{
    XserverRegion region = XFixesCreateRegion (dpy, NULL, 0);

    XFixesSetWindowShapeRegion (dpy, w, ShapeBounding, 0, 0, 0);
    XFixesSetWindowShapeRegion (dpy, w, ShapeInput, 0, 0, region);

    XFixesDestroyRegion (dpy, region);
}

void prep_root (void)
{
    dpy = XOpenDisplay(NULL);

	bool hasNamePixmap = false;
	int event_base, error_base;
	if ( XCompositeQueryExtension( dpy, &event_base, &error_base ) )
	{
		int major = 0, minor = 2; // The highest version we support
		XCompositeQueryVersion( dpy, &major, &minor );
		hasNamePixmap = ( major > 0 || minor >= 2 );
		std::cerr << "1. has composite extension! hasNamePixmap: " << hasNamePixmap << "\n" << std::endl;
	}

	/* XInput Extension available? */
	int event, error;
	if (!XQueryExtension(dpy, "XInputExtension", &xi_opcode, &event, &error)) {
	   fprintf(stderr, "X Input extension not available.\n");
	   exit(-1);
	}

	/* Which version of XI2? We support 2.0 */
	int major = 2, minor = 0;
	if (XIQueryVersion(dpy, &major, &minor) == BadRequest) {
	  fprintf(stderr, "XI2 not available. Server supports %d.%d\n", major, minor);
	  exit(-1);
	}

    root = DefaultRootWindow(dpy);
    scrn = DefaultScreenOfDisplay(dpy);
    screen = XDefaultScreen(dpy);
    for ( int i = 0; i < ScreenCount( dpy ); ++i ) {
		XCompositeRedirectSubwindows( dpy, RootWindow( dpy, i ), CompositeRedirectAutomatic); // Manual);//Automatic ); // CompositeRedirectManual);
		XSelectInput(dpy, RootWindow(dpy, i), SubstructureNotifyMask /*| PointerMotionMask */ | KeyPressMask);
//
//		XSelectInput (dpy, root,
//				      SubstructureNotifyMask|
//				      ExposureMask|
//				      StructureNotifyMask|
//				      PropertyChangeMask);
//			XShapeSelectInput (dpy, root, ShapeNotifyMask);


	    XIEventMask evmask;
	    unsigned char mask[2] = { 0, 0 };

	    XISetMask(mask, XI_HierarchyChanged | XI_Motion | XI_RawMotion);
	    evmask.deviceid = XIAllDevices;
	    evmask.mask_len = sizeof(mask);
	    evmask.mask = mask;

	    XISelectEvents(dpy, RootWindow(dpy, i), &evmask, 1);
	}

    XIEventMask evmask;
    unsigned char mask[2] = { 0, 0 };

    XISetMask(mask, XI_HierarchyChanged | XI_Motion | XI_RawMotion);
    evmask.deviceid = XIAllDevices;
    evmask.mask_len = sizeof(mask);
    evmask.mask = mask;

    XISelectEvents(dpy, DefaultRootWindow(dpy), &evmask, 1);
}

void prep_overlay (void)
{
    overlay = XCompositeGetOverlayWindow (dpy, root);
    allow_input_passthrough (overlay);
}

Window stage_win;
void prep_stage (void)
{
    XReparentWindow (dpy, window, overlay, 0, 0);
    XSelectInput (dpy, window, ExposureMask | PointerMotionMask | KeyPressMask);
    allow_input_passthrough (window);
}

Window input;
void prep_input (void)
{
    XWindowAttributes attr;

    XGetWindowAttributes (dpy, root, &attr);
    input = XCreateWindow (dpy, root,
                           0, 0,  /* x, y */
                           attr.width, attr.height,
                           0, 0, /* border width, depth */
                           InputOnly, DefaultVisual (dpy, 0), 0, NULL);

    XSelectInput (dpy, input,
                  StructureNotifyMask | FocusChangeMask | PointerMotionMask
                  | KeyPressMask | KeyReleaseMask | ButtonPressMask
                  | ButtonReleaseMask | PropertyChangeMask);
    XMapWindow (dpy, input);
//    XSetInputFocus (dpy, input, RevertToPointerRoot, CurrentTime);

//    attach_event_source ();
}


//Pixmap pixmap = 0;
static GLfloat top(0), bottom(1);
static GLXFBConfig fbconfig;
static GLuint texture = 0;
//double b = 0;
typedef struct {
	Window window;
	Pixmap pixmap;
	GLXPixmap glxpixmap;
	GLuint texture;
} WindowInfo;

std::map<Window, WindowInfo> redirectedWindows;

void unbindRedirectedWindow(Window window) {
	if(redirectedWindows.find(window) != redirectedWindows.end()) {
		WindowInfo windowInfo = redirectedWindows[window];

		glXReleaseTexImageEXT (display, windowInfo.glxpixmap, GLX_FRONT_LEFT_EXT);
		glXDestroyGLXPixmap(display, windowInfo.glxpixmap);
		glDeleteTextures(1, &windowInfo.texture);
		XFreePixmap(display, windowInfo.pixmap);
		redirectedWindows.erase(window);
	}
}
void bindRedirectedWindowToTexture(Display *display, Window window, int screen, XWindowAttributes &attrib) {
	const double w = attrib.width/(double)width;
	const double h = attrib.height/(double)height;
	const double right = w;
	const double t = -h;

	static const int pixmapAttribs[] = { GLX_TEXTURE_TARGET_EXT, GLX_TEXTURE_2D_EXT,
					  GLX_TEXTURE_FORMAT_EXT, GLX_TEXTURE_FORMAT_RGBA_EXT,
					  None };

	WindowInfo windowInfo;
	if(redirectedWindows.find(window) != redirectedWindows.end()) {
		windowInfo = redirectedWindows[window];
	} else {
		Pixmap pixmap = XCompositeNameWindowPixmap(display, window);
		if(!pixmap) return;

		GLXPixmap glxpixmap = glXCreatePixmap (display, fbconfig, pixmap, pixmapAttribs);

		GLuint tempTexture;
		glGenTextures(1, &tempTexture);
		windowInfo.pixmap = pixmap;
		windowInfo.glxpixmap = glxpixmap;
		windowInfo.texture = tempTexture;
		windowInfo.window = window;

		redirectedWindows[window] = windowInfo;

		glBindTexture(GL_TEXTURE_2D, windowInfo.texture);
		glXBindTexImageEXT(display, windowInfo.glxpixmap, GLX_FRONT_LEFT_EXT, NULL);
	}
	glBindTexture(GL_TEXTURE_2D, windowInfo.texture);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // draw using pixmap as texture
	double originX = attrib.x/(double)width-0.5;
	double originY = -attrib.y/(double)height+0.5;
	glColor4f(1, 1, 1, 1);
	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2d(0, bottom);
		glVertex3f(originX,originY,0);

		glTexCoord2d(1, bottom);
		glVertex3f(originX+right,originY,0);

		glTexCoord2d(0, top);
		glVertex3f(originX,originY+t,0);

		glTexCoord2d(1, top);
		glVertex3f(originX+right,originY+t,0);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, 0);
	CheckForErrors();
}

/*From: http://stackoverflow.com/questions/5002254/adapt-existing-code-for-opengl-stereoscopic-rendering
 * Stereo SBS projection info

glMatrixMode(GL_PROJECTION);
glLoadIdentity();
stereo_offset = eye * near * parallax_factor / convergence_distance;
glFrustum(stereo_offset + left, stereo_offset + right, bottom, top, near, far);

glMatrixMode(GL_MODELVIEW);
glLoadIdentity();
glTranslatef(eye * parallax_factor * convergence_distance, 0, 0);

// now use gluLookAt here as this were a normal 2D rendering

 *  parallax_factor should be no larger than the ratio of half_eye_distance / screen_width,
 *  so the larger the screen gets the smaller the parallax_factor is.
 *  A good value for parallax_factor for computer display use is 0.05,
 *  for large screens (think cinema) it's something like 0.01
 */

GLuint fbos[2];
GLuint textures[2];
GLuint depthBuffer;
void prep_framebuffers() {
	if(!GL_ARB_framebuffer_object || !glewGetExtension("GL_ARB_framebuffer_object")) {
		std::cerr << "NO FBO SUPPORT" << std::endl;
		exit(0);
	} else {
		std::cerr << "GL_ARB_framebuffer_object SUPPORT" << std::endl;
	}

	glGenFramebuffers(2, fbos);
	glGenRenderbuffers(1, &depthBuffer);
	glGenTextures(2, textures);

	for(int i = 0; i < 2; ++i) {
		glBindFramebuffer(GL_FRAMEBUFFER, fbos[i]);

		glBindTexture(GL_TEXTURE_2D, textures[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
		                GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
								 GL_TEXTURE_2D, textures[i],
								 0);

	    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	    if(i == 0) {
	    	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
	    }
	    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
	    if(!CheckForErrors()) {
	    	std::cerr << "Stage 1 - Problem generating FBO " << i << std::endl;
	    	exit(0);
	    }

		std::cerr << "Generating FBO #" << i << std::endl;
		std::cerr << "FBO: " << width << "x" << height << std::endl;

		if(!CheckForErrors() || glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			std::cerr << "Stage 2 - Problem generating FBO " << i << std::endl;
			exit(0);
		}
		glBindFramebuffer(GL_FRAMEBUFFER,0);
	}
}

GLuint _skybox[6];
void loadSkybox() {
	float sizeX = 2048;
	float sizeY = 2048;
	_skybox[0] = glmLoadTexture("humus-skybox/negz.jpg", GL_TRUE, GL_FALSE, GL_TRUE, GL_FALSE, &sizeX, &sizeY);
	_skybox[1] = glmLoadTexture("humus-skybox/posx.jpg", GL_TRUE, GL_FALSE, GL_TRUE, GL_FALSE, &sizeX, &sizeY);
	_skybox[2] = glmLoadTexture("humus-skybox/posz.jpg", GL_TRUE, GL_FALSE, GL_TRUE, GL_FALSE, &sizeX, &sizeY);
	_skybox[3] = glmLoadTexture("humus-skybox/negx.jpg", GL_TRUE, GL_FALSE, GL_TRUE, GL_FALSE, &sizeX, &sizeY);
	_skybox[4] = glmLoadTexture("humus-skybox/posy.jpg", GL_TRUE, GL_FALSE, GL_TRUE, GL_FALSE, &sizeX, &sizeY);
	_skybox[5] = glmLoadTexture("humus-skybox/negy.jpg", GL_TRUE, GL_FALSE, GL_TRUE, GL_FALSE, &sizeX, &sizeY);

	std::cerr << _skybox[5] << std::endl;
}
void renderSkybox() {
    // Store the current matrix
    glPushMatrix();
    glScaled(100, 100, 100);

    // Enable/Disable features
    glPushAttrib(GL_ENABLE_BIT);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_BLEND);

    // Just in case we set all vertices to white.
    glColor4f(1,1,1,1);

    // Render the front quad
    glBindTexture(GL_TEXTURE_2D, _skybox[0]);
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(  0.5f, -0.5f, -0.5f );
        glTexCoord2f(1, 0); glVertex3f( -0.5f, -0.5f, -0.5f );
        glTexCoord2f(1, 1); glVertex3f( -0.5f,  0.5f, -0.5f );
        glTexCoord2f(0, 1); glVertex3f(  0.5f,  0.5f, -0.5f );
    glEnd();

    // Render the left quad
    glBindTexture(GL_TEXTURE_2D, _skybox[1]);
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(  0.5f, -0.5f,  0.5f );
        glTexCoord2f(1, 0); glVertex3f(  0.5f, -0.5f, -0.5f );
        glTexCoord2f(1, 1); glVertex3f(  0.5f,  0.5f, -0.5f );
        glTexCoord2f(0, 1); glVertex3f(  0.5f,  0.5f,  0.5f );
    glEnd();

    // Render the back quad
    glBindTexture(GL_TEXTURE_2D, _skybox[2]);
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f( -0.5f, -0.5f,  0.5f );
        glTexCoord2f(1, 0); glVertex3f(  0.5f, -0.5f,  0.5f );
        glTexCoord2f(1, 1); glVertex3f(  0.5f,  0.5f,  0.5f );
        glTexCoord2f(0, 1); glVertex3f( -0.5f,  0.5f,  0.5f );

    glEnd();

    // Render the right quad
    glBindTexture(GL_TEXTURE_2D, _skybox[3]);
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f( -0.5f, -0.5f, -0.5f );
        glTexCoord2f(1, 0); glVertex3f( -0.5f, -0.5f,  0.5f );
        glTexCoord2f(1, 1); glVertex3f( -0.5f,  0.5f,  0.5f );
        glTexCoord2f(0, 1); glVertex3f( -0.5f,  0.5f, -0.5f );
    glEnd();

    // Render the top quad
    glBindTexture(GL_TEXTURE_2D, _skybox[4]);
    glBegin(GL_QUADS);
        glTexCoord2f(0, 1); glVertex3f( -0.5f,  0.5f, -0.5f );
        glTexCoord2f(0, 0); glVertex3f( -0.5f,  0.5f,  0.5f );
        glTexCoord2f(1, 0); glVertex3f(  0.5f,  0.5f,  0.5f );
        glTexCoord2f(1, 1); glVertex3f(  0.5f,  0.5f, -0.5f );
    glEnd();

    // Render the bottom quad
    glBindTexture(GL_TEXTURE_2D, _skybox[5]);
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f( -0.5f, -0.5f, -0.5f );
        glTexCoord2f(0, 1); glVertex3f( -0.5f, -0.5f,  0.5f );
        glTexCoord2f(1, 1); glVertex3f(  0.5f, -0.5f,  0.5f );
        glTexCoord2f(1, 0); glVertex3f(  0.5f, -0.5f, -0.5f );
    glEnd();

    // Restore enable bits and matrix
    glPopAttrib();
    glPopMatrix();
}

void renderDesktop() {
	long unsigned int wId;
	static std::set<Window> s;

	static Window parent, root2;
	static Window *children;
	static unsigned int countChildren;
	static XWindowAttributes attr;

	Bool mousePositionGrabbed(false);
	Window window_returned;
	int root_x, root_y;
	int win_x, win_y;
	unsigned int mask_return;
	XSync(dpy, false);
	XGrabServer(dpy);
	mousePositionGrabbed = XQueryPointer(display, XDefaultRootWindow(dpy), &window_returned, &window_returned, &root_x, &root_y, &win_x, &win_y, &mask_return);
	XQueryTree(dpy, XDefaultRootWindow(dpy), &parent, &root2, &children, &countChildren);
	glPushMatrix();
	glTranslatef(0, 0, -1.21);
	for(unsigned int i = 0; i < countChildren; ++i) {
		wId = children[i];
		if(wId == window || wId == overlay) continue;

		if(s.find(wId) == s.end()) {
			s.insert(wId);
			XCompositeRedirectSubwindows( dpy, wId, CompositeRedirectAutomatic); // Manual);
			// XSelectInput(dpy, wId, SubstructureNotifyMask | PointerMotionMask);
		}
		XGetWindowAttributes( dpy, wId, &attr );

		#if defined(__cplusplus) || defined(c_plusplus)
			bool isInputOnly = attr.c_class == InputOnly;
		#else
			bool isInputOnly = attr.class == InputOnly;
		#endif
		if(attr.map_state == IsViewable && !isInputOnly) { // && attr.override_redirect == 0
			glTranslatef(0, 0, i*0.001);
			bindRedirectedWindowToTexture(dpy, wId, screen, attr);
		}
	}
	glPopMatrix();

	XUngrabServer(dpy);

	if(mousePositionGrabbed == True) {
		glBindTexture(GL_TEXTURE_2D, 0);
		glColor4f(1, 0, 0, 1);
		glBegin(GL_TRIANGLES);
		glVertex3f(((double)root_x-5)/width-0.5,0.5-((double)root_y+15.0)/height,-1.21+0.02);
		glVertex3f(((double)root_x+5)/width-0.5,0.5-((double)root_y+15.0)/height,-1.21+0.02);
		glVertex3f((double)root_x/width-0.5,0.5-(double)root_y/height,-1.21+0.02);
		glEnd();
	}
}

void renderGL(void) {
	static int frame=0,time,timebase=0, count=0;
	frame++;
	count++;
	count %= 360;
	time=glutGet(GLUT_ELAPSED_TIME);
	if (time - timebase > 1000) {
		fprintf(stderr,"FPS:%4.2f\n", frame*1000.0/(time-timebase));
		timebase = time;
		frame = 0;
	}

	static XWindowAttributes attr;

	static int value;
	static int nfbconfigs;
	static GLXFBConfig *fbconfigs = glXGetFBConfigs (display, screen, &nfbconfigs);
	static int i = 0;

	XGetWindowAttributes( dpy, root, &attr );
	static bool init = false;
	if(!init) {
		init = true;
		int attrib[] = {
		        GLX_RENDER_TYPE, GLX_RGBA_BIT,
		        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
		        GLX_RED_SIZE, 1,
		        GLX_GREEN_SIZE, 1,
		        GLX_BLUE_SIZE, 1,
		        GLX_ALPHA_SIZE, 1,
		        GLX_DOUBLEBUFFER, True,
		        GLX_DEPTH_SIZE, 1,
		        None };
//		    GLXFBConfig *fbconfigs, fbconfig;
		int numfbconfigs, render_event_base, render_error_base;
		XVisualInfo *visinfo;
		XRenderPictFormat *pictFormat;

		/* Make sure we have the RENDER extension */
		if(!XRenderQueryExtension(dpy, &render_event_base, &render_error_base)) {
			fprintf(stderr, "No RENDER extension found\n");
			exit(EXIT_FAILURE);
		}


		/* Get the list of FBConfigs that match our criteria */
		int scrnum = 0;
		fbconfigs = glXChooseFBConfig(dpy, scrnum, attrib, &numfbconfigs);
		if (!fbconfigs) {
			/* None matched */
			exit(EXIT_FAILURE);
		}

		/* Find an FBConfig with a visual that has a RENDER picture format that
		 * has alpha */
		for (i = 0; i < numfbconfigs; i++) {
			visinfo = glXGetVisualFromFBConfig(dpy, fbconfigs[i]);
			if (!visinfo) continue;
			pictFormat = XRenderFindVisualFormat(dpy, visinfo->visual);
			if (!pictFormat) continue;

			if(pictFormat->direct.alphaMask > 0) {
				fbconfig = fbconfigs[i];
				break;
			}

			XFree(visinfo);
		}

		if (i == numfbconfigs) {
			/* None of the FBConfigs have alpha.  Use a normal (opaque)
			 * FBConfig instead */
			fbconfig = fbconfigs[0];
			visinfo = glXGetVisualFromFBConfig(dpy, fbconfig);
			pictFormat = XRenderFindVisualFormat(dpy, visinfo->visual);
		}

			glXGetFBConfigAttrib (dpy, fbconfigs[i],
								  GLX_Y_INVERTED_EXT,
								  &value);
			if (value == TRUE)
			{
				top = 0.0f;
				bottom = 1.0f;
			}
			else
			{
				top = 1.0f;
				bottom = 0.0f;
			}

		XFree(fbconfigs);

		glEnable(GL_TEXTURE_2D);

		top = -top;
		bottom = -bottom;

		fbconfig = fbconfigs[i];
	}

	for(int i2 = 0; i2 < 2; ++i2) {
		if(USE_FBO) {
			glBindFramebuffer(GL_FRAMEBUFFER, fbos[i2]);
			if(!CheckForErrors()) {
				std::cerr << "GL ISSUE" << std::endl;
				exit(0);
			}
			glPushMatrix();
		} else {
			if(i2 > 0) break;
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		glPushMatrix();
		{
			glRotated(xRotation, 1, 0, 0);
			glRotated(yRotation, 0, 1, 0);

			glPushMatrix();
			{
				renderSkybox();
				glTranslated(xPosition, yPosition, zPosition);
				glTranslated((i2 == 0) ? -0.01 : 0.01, 0, 0);

				glPushMatrix();
				{

					/* Lighting Variables */
					GLfloat light_ambient[] = {
						0.0, 0.0, 0.0, 1.0
					};

					GLfloat light_diffuse[] = {
						1.0, 1.0, 1.0, 1.0
					};

					GLfloat light_specular[] = {
						1.0, 1.0, 1.0, 1.0
					};

					GLfloat light_position[] = {
						1.0, 1.0, 1.0, 0.0
					};

					GLfloat mat_ambient[] = {
						0.7, 0.7, 0.7, 1.0
					};

					GLfloat mat_diffuse[] = {
						0.8, 0.8, 0.8, 1.0
					};

					GLfloat mat_specular[] = {
						1.0, 1.0, 1.0, 1.0
					};

					GLfloat high_shininess[] = {
						100.0
					};

					glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
					glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
					glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
					glLightfv(GL_LIGHT0, GL_POSITION, light_position);
					glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
					glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
					glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
					glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);
					glEnable(GL_LIGHTING);
					glEnable(GL_LIGHT0);
					glDepthFunc(GL_LESS);
					glEnable(GL_DEPTH_TEST);
					glEnable(GL_NORMALIZE);

					glEnable(GL_LIGHTING);
					glPushMatrix();
					{
						glTranslated(0, -0.1, -2.21);
						glRotated(90, 0, 1, 0);
						glScaled(1, 0.72, 0.6);
						glPushMatrix();
						{
//							glmDraw(pmodel, GLM_SMOOTH | GLM_COLOR);
						}
						glPopMatrix();
						glDisable(GL_LIGHTING);
					}
					glPopMatrix();

					renderDesktop();
				}
				glPopMatrix();
			}
			glPopMatrix();
		}
		glPopMatrix();

		if(USE_FBO) {
			glPopMatrix();
		}
	}

	const bool ortho = true;
	if(USE_FBO) {
		if(ortho) {
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glOrtho(0, 1, 0, 1, -1, 1);
			glMatrixMode(GL_MODELVIEW);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glColor4f(1, 1, 1, 1);
		for(int i = 0; i < 2; ++i) {
			if(ortho) {
				double originX = (i == 0) ? 0 : 0.5;
				glBindTexture(GL_TEXTURE_2D, textures[i]);
				glPushMatrix();
//				glTranslated((i < 1) ? -0.98 : 0, -0.5, -2.4);
				glColor4f(1, 1, 1, 1);
				glBegin(GL_TRIANGLE_STRIP);
					glTexCoord2d(0, bottom);
					glVertex3f(originX,0,0);

					glTexCoord2d(1, bottom);
					glVertex3f(originX+0.5,0,0);

					glTexCoord2d(0, top);
					glVertex3f(originX,1,0);

					glTexCoord2d(1, top);
					glVertex3f(originX+0.5,1,0);
				glEnd();
				glPopMatrix();
			} else {
//				glPushMatrix();
//				glTranslated((i == 0) ? -0.01 : 0.01, 0, 0);
//
//				glBindTexture(GL_TEXTURE_2D, 0);
//				glPushMatrix();
//				glTranslated((i < 1) ? -0.5 : 0.5, 0.7, -2.2);
//				glRotated(count*2%360, 0, 1, 0);
//				glutWireTeapot(0.1);
//				glPopMatrix();

				glBindTexture(GL_TEXTURE_2D, textures[i]);
				glPushMatrix();
				glTranslated((i < 1) ? -0.98 : 0, -0.5, -2.4);
				glColor4f(1, 1, 1, 1);
				glBegin(GL_TRIANGLE_STRIP);
					glTexCoord2d(0, bottom);
					glVertex3f(0,0,0);

					glTexCoord2d(1, bottom);
					glVertex3f(1,0,0);

					glTexCoord2d(0, top);
					glVertex3f(0,1,0);

					glTexCoord2d(1, top);
					glVertex3f(1,1,0);
				glEnd();
				glPopMatrix();

//				glPopMatrix();
			}
		}
		if(ortho) {
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
//			gluPerspective(45.0f, 1, 0.1f, 100.0f);//(GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
		    gluPerspective(120.0f, 0.75, 0.1f, 100.0f);//(GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
			glMatrixMode(GL_MODELVIEW);
		}
	}

	if (doubleBuffered)
	{
		glXSwapBuffers(display, window);
	}
}

void initGL()
{
    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.1f, 0.0f, 0.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    // we use resizeGL once to set up our initial perspective
    resizeGL(width, height);

	if(GLXEW_EXT_texture_from_pixmap) {
		std::cerr << "SUPPORT GLXEW_EXT_texture_from_pixmap" << std::endl;
	} else {
		std::cerr << "Don't have support for GLXEW_EXT_texture_from_pixmap" << std::endl;
		exit(0);
	}

    glFlush();
}

void prep_input2 (void)
{
//    XSelectInput (dpy, root, PointerMotionMask);

//
//    XSelectInput (dpy, root,
//                      StructureNotifyMask | FocusChangeMask | PointerMotionMask
//                      | KeyPressMask | KeyReleaseMask //| ButtonPressMask
//                      | ButtonReleaseMask
//                      | PropertyChangeMask);
//	XMapWindow (dpy, root);
//	XSetInputFocus (dpy, root, RevertToPointerRoot, CurrentTime);
	XSetInputFocus (dpy, window, RevertToPointerRoot, CurrentTime);
}

void loadModel() {
	if (!pmodel) {		/* load up the model */
		pmodel = glmReadOBJ("lcd_monitor.obj");
		if (!pmodel) {
		    printf("\nUsage: objviewV2 <-s> <obj filename>\n");
		    exit(0);
		}
		glmUnitize(pmodel);
		glmVertexNormals(pmodel, 90.0, GL_TRUE);
	}
}
void setup_hotkey(Display *display_) {
	unsigned int    modifiers       = ControlMask | ShiftMask;
	int             keycode         = XKeysymToKeycode(display_,XK_Y);
	Window          grab_window     =  root;
	Bool            owner_events    = False;
	int             pointer_mode    = GrabModeAsync;
	int             keyboard_mode   = GrabModeAsync;

	XGrabKey(display_, keycode, modifiers, grab_window, owner_events, pointer_mode,
			 keyboard_mode);

	XSelectInput(display_, root, KeyPressMask );
}

void disallow_input_passthrough(Window w) {
	XRectangle r;
	r.x = r.y = 0;
	r.width = width;
	r.height = height;
	XserverRegion region = XFixesCreateRegion (dpy, &r, 1);


//	XFixesSetWindowShapeRegion (dpy, w, ShapeBounding, 0, 0, region);
	XFixesSetWindowShapeRegion (dpy, w, ShapeInput, 0, 0, region);
//	XFixesSetWindowShapeRegion (dpy, w, None, 0, 0, region);

	XFixesDestroyRegion(dpy, region);
}

void toggleControl() {
	controlDesktop = !controlDesktop;
	if(controlDesktop) {
		allow_input_passthrough (overlay);
		allow_input_passthrough (window);
	} else {
		disallow_input_passthrough (overlay);
		disallow_input_passthrough (window);


		XIEventMask eventmask;
//		unsigned char mask[2] = { 0, 0 }; /* the actual mask */

		eventmask.deviceid = XIAllDevices;
//		eventmask.mask_len = sizeof(mask); /* always in bytes */
//		eventmask.mask = mask;
		eventmask.mask_len = XIMaskLen(XI_RawMotion);
		eventmask.mask = (unsigned char *)calloc(eventmask.mask_len, sizeof(char));
		/* now set the mask */
//		XISetMask(eventmask.mask, XI_ButtonPress);
		XISetMask(eventmask.mask, XI_Motion);
		XISetMask(eventmask.mask, XI_RawMotion);
		XISetMask(eventmask.mask, XI_KeyPress);
		XISetMask(eventmask.mask, XI_KeyRelease);

		/* select on the window */
		XISelectEvents(dpy, root, &eventmask, 1);
		XISelectEvents(dpy, window, &eventmask, 1);
		XISelectEvents(dpy, overlay, &eventmask, 1);

		XIGrabDevice(dpy, XIAllDevices, root, CurrentTime,
				           0 /*cursor*/, GrabModeAsync, 0 /*pairedMode*/,
				           False, &eventmask);
		XIGrabDevice(dpy, XIAllDevices, overlay, CurrentTime,
		           0 /*cursor*/, GrabModeAsync, 0 /*pairedMode*/,
		           False, &eventmask);
		XIGrabDevice(dpy, XIAllDevices, window, CurrentTime,
				           0 /*cursor*/, GrabModeAsync, 0 /*pairedMode*/,
				           False, &eventmask);
	}
}

void walk(double forward, double right, double seconds) {
	xPosition -= sin(yRotation/90.0*M_PI_2) * WALK_SPEED * seconds * forward;
	zPosition += cos(yRotation/90.0*M_PI_2) * WALK_SPEED * seconds * forward;

	xPosition -= cos(-yRotation/90.0*M_PI_2) * WALK_SPEED * seconds * right;
	zPosition += sin(-yRotation/90.0*M_PI_2) * WALK_SPEED * seconds * right;
}

void processKey(XKeyEvent ke) {
	static KeyCode toggleKey = XKeysymToKeycode(dpy,XK_Y);

	if((ke.state & ControlMask) && (ke.state & ShiftMask) && ke.keycode == toggleKey) {
		toggleControl();
		return;
	}
}

void print_rawmotion(XIRawEvent *event)
{
    int i;
    double *raw_valuator = event->raw_values,
           *valuator = event->valuators.values;

    for (i = 0; i < event->valuators.mask_len * 8; i++) {
        if (XIMaskIsSet(event->valuators.mask, i)) {
            printf("Acceleration on valuator %d: %f\n",
                   i, *valuator - *raw_valuator);
            valuator++;
            raw_valuator++;
        }
    }
}

void process_rawmotion(XIRawEvent *event) {
	int i;
	    double *raw_valuator = event->raw_values,
	           *valuator = event->valuators.values;

	    for (i = 0; i < event->valuators.mask_len * 8; i++) {
	        if (XIMaskIsSet(event->valuators.mask, i)) {
//	            printf("Acceleration on valuator %d: %f\n",
//	                   i, *valuator - *raw_valuator);
	        	switch(i) {
	        	case 0:
	        		yRotation += (double)(*valuator - *raw_valuator)/(double)width * 180.0;;
	        		break;
	        	case 1:
	        		xRotation += (double)(*valuator - *raw_valuator)/(double)width * 180.0;
	        		break;
	        	}
	            valuator++;
	            raw_valuator++;
	        }
	    }
}

static void print_deviceevent(XIDeviceEvent* event)
{
    double *val;
    int i;

    printf("    device: %d (%d)\n", event->deviceid, event->sourceid);
    printf("    detail: %d\n", event->detail);
    if (event->flags & XIKeyRepeat)
       printf("    event is a key repeat.\n");

    printf("    root: %.2f/%.2f\n", event->root_x, event->root_y);
    printf("    event: %.2f/%.2f\n", event->event_x, event->event_y);

    printf("    buttons:");
    for (i = 0; i < event->buttons.mask_len * 8; i++)
        if (XIMaskIsSet(event->buttons.mask, i))
            printf(" %d", i);
    printf("\n");

    printf("    modifiers: locked %#x latched %#x base %#x\n",
            event->mods.locked, event->mods.latched,
            event->mods.base);
    printf("    group: locked %#x latched %#x base %#x\n",
            event->group.locked, event->group.latched,
            event->group.base);
    printf("    valuators:");

    val = event->valuators.values;
    for (i = 0; i < event->valuators.mask_len * 8; i++)
        if (XIMaskIsSet(event->valuators.mask, i))
            printf(" %.2f", *val++);
    printf("\n");

    printf("    windows: root 0x%lx event 0x%lx child 0x%ld\n",
            event->root, event->event, event->child);
}
void processKey(XIDeviceEvent *event, bool pressed) {
	static KeyCode W = XKeysymToKeycode(dpy,XK_W);
	static KeyCode S = XKeysymToKeycode(dpy,XK_S);
	static KeyCode A = XKeysymToKeycode(dpy,XK_A);
	static KeyCode D = XKeysymToKeycode(dpy,XK_D);
	static KeyCode Q = XKeysymToKeycode(dpy,XK_Q);
	static KeyCode E = XKeysymToKeycode(dpy,XK_E);

	static KeyCode R = XKeysymToKeycode(dpy,XK_R);

	if(!controlDesktop) {
		if(event->detail == W) {
			walkForward = 1;
//			walk(1, 0);
		} else if(event->detail == S) {
			walkForward = -1;
//			walk(-1, 0);
		} else if(event->detail == A) {
			strafeRight = -1;
//			walk(0, -1);
		} else if(event->detail == D) {
			strafeRight = 1;
//			walk(0, 1);
		} else if(event->detail == Q) {
			strafeRight = -1;
//			walk(0, -1);
		} else if(event->detail == E) {
			strafeRight = 1;
//			walk(0, 1);
		} else if(event->detail == R) {
			std::cerr << "RESET POSITION!" << std::endl;
			xRotation = 0;
			yRotation = 0;
			zRotation = 0;
			xPosition = 0;
			yPosition = 0;
			zPosition = 0;
		}

		if(!pressed) {
			walkForward = strafeRight = 0;
		}
	}
}

int main(int argc, char ** argv)
{
	prep_root();
	XSetErrorHandler(errorHandler);

    XEvent event;
    Bool done = False;


    XWindowAttributes attr;
    XGetWindowAttributes( dpy, root, &attr );
    width = attr.width;//WIDTH;
    height = attr.height;//HEIGHT;

    createWindow();

    prep_overlay();
    prep_stage();
//    prep_input();
//    prep_input2();

    glutInit(&argc, argv);

    if(texture == 0) {
    	std::cerr << "gen texture" << std::endl;
    	glGenTextures (1, &texture);
//    	texture = LoadTextureRAW("texture.raw", false);
    	CheckForErrors();
    	std::cerr << "gen texture done" << std::endl;
    }

    if(USE_FBO) prep_framebuffers();
    std::cerr << "A" << std::endl;


    XSync(dpy, false);
    std::cerr << "*********** HERE" << std::endl;

	static GLXContext ctx = glXGetCurrentContext();
	static GLXDrawable d = glXGetCurrentDrawable();
	glXMakeContextCurrent(dpy, d, d, ctx);

	std::cerr << "dpy: " << dpy << ", display: " << display << std::endl;

	loadSkybox();


	XFixesHideCursor (dpy, overlay);
	setup_hotkey(dpy);

//	XUndefineCursor(dpy, root);

	loadModel();

	struct timespec ts_start;
	clock_gettime(CLOCK_MONOTONIC, &ts_start);

    XEvent peekEvent;
    int pointerX, pointerY;
    unsigned int pointerMods;
    /* wait for events and eat up cpu. ;-) */
    while (!done)
    {
//    	display = dpy;
        /* handle the events in the queue */
        while (XPending(dpy) > 0)
        {
//        	std::cerr << XPending(dpy) << std::endl;
            XNextEvent(dpy, &event);
//            std::cerr << "Event: " << event.type << std::endl;

            XGenericEventCookie *cookie = &event.xcookie;
            if(event.xcookie.extension==xi_opcode && event.xcookie.type == GenericEvent) {
            	XGetEventData(dpy, cookie);

//            	std::cerr << "XI_OPCODE!" << std::endl;
            	XIDeviceEvent *xi_event = (XIDeviceEvent*)event.xcookie.data;
//				printf("XI EVENT type %d\n", xi_event->evtype);
				switch (xi_event->evtype)
				{
				case XI_KeyPress:
//					std::cerr << "Key PRESS!!!!!!!!!!!!!!!!!!" << std::endl;
//					print_deviceevent(xi_event);
					if(!controlDesktop) {
						processKey(xi_event, true);
					}
					break;
				case XI_KeyRelease:
					std::cerr << "RELEASE" << std::endl;
					if(!controlDesktop) {
						processKey(xi_event, false);
					}
					break;
				case XI_RawMotion:
//					std::cerr << "XI_RawMotion!" << std::endl;
//					print_rawmotion((XIRawEvent*)event.xcookie.data);
					if(!controlDesktop) {
						process_rawmotion((XIRawEvent*)event.xcookie.data);
					}
					break;
//				case XI_Motion:
//					std::cerr << "XI_Motion!" << std::endl;
//					std::cerr << xi_event->event_x << std::endl;
//					break;
				}

				XFreeEventData(dpy, cookie);
				continue;
            }
            switch (event.type)
            {
            case MapNotify:
//            	std::cerr << "MapNotify" << std::endl;
            	break;
            case UnmapNotify:
//				std::cerr << "UnmapNotify" << std::endl;
            	unbindRedirectedWindow(event.xclient.window);
				break;
            case ButtonPress:
            	case ButtonRelease:
            	    pointerX = event.xbutton.x_root;
            	    pointerY = event.xbutton.y_root;
            	    pointerMods = event.xbutton.state;
            	    break;
            	case KeyPress:
            	case KeyRelease:
            		std::cerr << "KEY PreSS" << std::endl;
            	    pointerX = event.xkey.x_root;
            	    pointerY = event.xkey.y_root;
            	    pointerMods = event.xbutton.state;
            	    if(event.type == KeyRelease) {
            	    	processKey(event.xkey);
            	    }
            	    break;
            	case MotionNotify:
//            		std::cerr << "MOTION" << std::endl;
            	    while (XPending (dpy))
            	    {
            		XPeekEvent (dpy, &peekEvent);

            		if (peekEvent.type != MotionNotify)
            		    break;

            		XNextEvent (dpy, &event);
            	    }

            	    pointerX = event.xmotion.x_root;
            	    pointerY = event.xmotion.y_root;
            	    pointerMods = event.xbutton.state;

//            		std::cerr << "MOUSE MOVED" << ", " << pointerX << ", " << pointerY << ", event.xmotion.x: " << event.xmotion.x << ", event.xmotion.y: " << event.xmotion.y << std::endl;
            	    break;
            	case EnterNotify:
            	case LeaveNotify:
            	    pointerX = event.xcrossing.x_root;
            	    pointerY = event.xcrossing.y_root;
            	    pointerMods = event.xbutton.state;
            	    break;
            	default:
            		break;
            }
        }

        struct timespec ts_current;
        clock_gettime(CLOCK_MONOTONIC, &ts_current);

        if(controlDesktop) {
        	walkForward = strafeRight = 0;
        }
        walk(walkForward, strafeRight, (double)(ts_current.tv_sec-ts_start.tv_sec)+((double)(ts_current.tv_nsec-ts_start.tv_nsec)/(double)1.0e9));

        renderGL();

        ts_start = ts_current;
    }

    destroyWindow();

    return 0;
}
