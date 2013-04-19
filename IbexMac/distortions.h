/*
 * distortions.h
 *
 *  Created on: Sep 12, 2012
 *      Author: Hesham Wahba
 */

#ifndef DISTORTIONS_H_
#define DISTORTIONS_H_

#define GLX_GLXEXT_PROTOTYPES

#ifdef __APPLE__

#include "GL/glew.h"
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>
#include <GLUT/glut.h>

#else
#ifdef _WIN32

#include "GL/glew.h"
#include <GL/gl.h>
#include <GL/glu.h>
//#include <GL/glext.h>
#include <GL/glut.h>

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
#endif

extern char RiftMonitorName[33];
extern GLfloat EyeDistance;
extern GLfloat DistortionK[4];

int init_distortion_shader();
void render_distorted_frame(const bool left, const GLuint textureId);

int init_distortion_shader_cache();
void render_distortion_lenses();

#endif /* DISTORTIONS_H_ */
