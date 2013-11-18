/*
 * SimpleWorldRendererPlugin.cpp
 *
 *  Created on: Sep 25, 2012
 *      Author: Hesham Wahba
 */

//#include <bullet/btBulletDynamicsCommon.h>

#include "../filesystem/Filesystem.h"

#include <iostream>
#include <math.h>
#include <stdio.h>

#include "../distortions.h"
#include "../opengl_helpers.h"
#include "../iphone_orientation_plugin/iphone_orientation_listener.h"

#ifdef _WIN32
#include "../ibex_win_utils.h"
#else
#ifdef __APPLE__
#include "ibex_mac_utils.h"
#else
#define HAVE_LIBJPEG 1
#include <jpeglib.h>
#include "../glm/glm.h"
#endif
#endif

#include "SimpleWorldRendererPlugin.h"

#include <oculus/Rift.h>

void renderSphericalDisplay(double r, double numHorizontalLines, double numVerticalLines, double width, double height) {
    glPushMatrix();
    glRotated(90, 0, 1, 0);
    
    glPushMatrix();
    for(double i = 0; i <= numHorizontalLines; i++) {
        const double latitude0 = M_PI * (-0.5 + (i-1.0)/numHorizontalLines) *(width/360.0)-(width*M_2_PI/360/2);
        const double depth0  = sin(latitude0);
        const double depthXY0 =  cos(latitude0);
        
        const double latitude1 = M_PI * (-0.5 + i / numHorizontalLines) *(width/360.0)-(width*M_2_PI/360/2);
        const double depth1 = sin(latitude1);
        const double depthXY1 = cos(latitude1);
        
        glBegin(GL_QUAD_STRIP);
        for(double j = 0; j <= numVerticalLines; j++) {
            const double longitude = 2 * M_PI * (j-1.0)/numVerticalLines *(height/360.0) -4*(height*M_2_PI/360);
            const double x = cos(longitude);
            const double y = sin(longitude);
            
            glTexCoord2d(i/(numHorizontalLines+1.0), j/numVerticalLines);
            glNormal3f(x * depthXY0, y * depthXY0, depth0);
            glVertex3f(x * depthXY0, y * depthXY0, depth0);
            
            glTexCoord2d((i+1.0)/(numHorizontalLines+1.0), j/numVerticalLines);
            glNormal3f(x * depthXY1, y * depthXY1, depth1);
            glVertex3f(x * depthXY1, y * depthXY1, depth1);
        }
        glEnd();
    }
    glPopMatrix();
    
    glPopMatrix();
}
void renderSphericalMouse(double r, double numHorizontalLines, double numVerticalLines, double width, double height, double x, double y, double sizeX, double sizeY) {
    glPushMatrix();
    glRotated(90, 0, 1, 0);
    
    glPushMatrix();
    for(double i = 0; i <= numHorizontalLines; i++) {
        const double latitude0 = M_PI * (-0.5 + (i-1.0)/numHorizontalLines) *(width/360.0)-(width*M_2_PI/360/2);
        const double depth0  = sin(latitude0);
        const double depthXY0 =  cos(latitude0);
        
        const double latitude1 = M_PI * (-0.5 + i / numHorizontalLines) *(width/360.0)-(width*M_2_PI/360/2);
        const double depth1 = sin(latitude1);
        const double depthXY1 = cos(latitude1);
        
        glBegin(GL_QUAD_STRIP);
        for(double j = 0; j <= numVerticalLines; j++) {
            const double longitude = 2 * M_PI * (j-1.0)/numVerticalLines *(height/360.0) -4*(height*M_2_PI/360);
            const double x = cos(longitude);
            const double y = sin(longitude);
            
            glTexCoord2d(i/(numHorizontalLines+1.0), j/numVerticalLines);
            glNormal3f(x * depthXY0, y * depthXY0, depth0);
            glVertex3f(x * depthXY0, y * depthXY0, depth0);
            
            glTexCoord2d((i+1.0)/(numHorizontalLines+1.0), j/numVerticalLines);
            glNormal3f(x * depthXY1, y * depthXY1, depth1);
            glVertex3f(x * depthXY1, y * depthXY1, depth1);
        }
        glEnd();
    }
    glPopMatrix();
    
    glPopMatrix();
}

void renderCylindricalDisplay(double r, double numHorizontalLines, double numVerticalLines, double width, double height) {
    glPushMatrix();
//    glRotated(90, 1, 0, 0);
    
    glPushMatrix();
    for(double i = 0; i <= numHorizontalLines; i++) {
        const double latitude0 = 2*M_PI * (-0.5 + (i-1.0)/numHorizontalLines) *(width/360.0);
        const double depth0  = sin(latitude0);
        const double depthXY0 =  cos(latitude0);
        
        const double latitude1 = 2*M_PI * (-0.5 + i / numHorizontalLines) *(width/360.0);
        const double depth1 = sin(latitude1);
        const double depthXY1 = cos(latitude1);
        
        glBegin(GL_QUAD_STRIP);
        for(double j = 0; j <= numVerticalLines; j++) {
            const double longitude = 2 * M_PI * (j-1.0)/numVerticalLines *(height/360.0);
            const double x = cos(longitude);
            const double y = sin(longitude);
            
            glTexCoord2d(1-i/(numHorizontalLines+1.0), j/numVerticalLines);
            glNormal3f(x * depthXY0, depthXY0, y * depthXY0);
            glVertex3f(depth0*r, (j/numVerticalLines)*2*r-r, depthXY0*r);
            
            glTexCoord2d(1-(i+1.0)/(numHorizontalLines+1.0), j/numVerticalLines);
            glNormal3f(x * depthXY1, y * depthXY1, depthXY1);
            glVertex3f(depth1*r, (j/numVerticalLines)*2*r-r,depthXY1*r);
        }
        glEnd();
    }
    glPopMatrix();
    
    glPopMatrix();
}

SimpleWorldRendererPlugin::SimpleWorldRendererPlugin() {
}
SimpleWorldRendererPlugin::~SimpleWorldRendererPlugin() {
}

// ---------------------------------------------------------------------------
// Function: loadSkybox
// Design:   Belongs to OpenGL component
// Purpose:
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
void SimpleWorldRendererPlugin::loadSkybox()
{
#ifdef _WIN32
    _skybox[0] = loadTexture("\\resources\\humus-skybox\\negz.jpg");
    _skybox[1] = loadTexture("\\resources\\humus-skybox\\posx.jpg");
    _skybox[2] = loadTexture("\\resources\\humus-skybox\\posz.jpg");
    _skybox[3] = loadTexture("\\resources\\humus-skybox\\negx.jpg");
    _skybox[4] = loadTexture("\\resources\\humus-skybox\\posy.jpg");
    _skybox[5] = loadTexture("\\resources\\humus-skybox\\negy.jpg");
#else
#ifdef __APPLE__
  _skybox[0] = loadTexture("/resources/humus-skybox/negz.jpg");
  _skybox[1] = loadTexture("/resources/humus-skybox/posx.jpg");
  _skybox[2] = loadTexture("/resources/humus-skybox/posz.jpg");
  _skybox[3] = loadTexture("/resources/humus-skybox/negx.jpg");
  _skybox[4] = loadTexture("/resources/humus-skybox/posy.jpg");
  _skybox[5] = loadTexture("/resources/humus-skybox/negy.jpg");
#else
  float sizeX = 2048;
  float sizeY = 2048;
  _skybox[0] = glmLoadTexture("./resources/humus-skybox/negz.jpg", GL_TRUE, GL_FALSE,
                              GL_TRUE, GL_FALSE, &sizeX, &sizeY);
  _skybox[1] = glmLoadTexture("./resources/humus-skybox/posx.jpg", GL_TRUE, GL_FALSE,
                              GL_TRUE, GL_FALSE, &sizeX, &sizeY);
  _skybox[2] = glmLoadTexture("./resources/humus-skybox/posz.jpg", GL_TRUE, GL_FALSE,
                              GL_TRUE, GL_FALSE, &sizeX, &sizeY);
  _skybox[3] = glmLoadTexture("./resources/humus-skybox/negx.jpg", GL_TRUE, GL_FALSE,
                              GL_TRUE, GL_FALSE, &sizeX, &sizeY);
  _skybox[4] = glmLoadTexture("./resources/humus-skybox/posy.jpg", GL_TRUE, GL_FALSE,
                              GL_TRUE, GL_FALSE, &sizeX, &sizeY);
  _skybox[5] = glmLoadTexture("./resources/humus-skybox/negy.jpg", GL_TRUE, GL_FALSE,
                              GL_TRUE, GL_FALSE, &sizeX, &sizeY);
#endif
#endif
  for(int i = 0; i < 6; ++i) {
    glBindTexture(GL_TEXTURE_2D, _skybox[i]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }
  glBindTexture(GL_TEXTURE_2D, 0);
  std::cout << _skybox[5] << std::endl;
}


// ---------------------------------------------------------------------------
// Function: renderSkybox
// Design:   Belongs to OpenGL component
// Purpose:
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
void SimpleWorldRendererPlugin::renderSkybox()
{
  static const double skyboxScale = 1000;
  // Store the current matrix
  glPushMatrix();
  glScaled(skyboxScale, skyboxScale, skyboxScale);

  // Enable/Disable features
  glPushAttrib(GL_ENABLE_BIT);
  glEnable(GL_TEXTURE_2D);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  glDisable(GL_BLEND);
    
  // Just in case we set all vertices to white.
  glColor4f(1, 1, 1, 1);

  // Render the front quad
//    checkForErrors();
//    std::cerr << "bind texture... " << _skybox[1] << std::endl;
  glBindTexture(GL_TEXTURE_2D, _skybox[0]);
//    checkForErrors();
//        std::cerr << "bind texture... done" << std::endl;
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

//  glBindTexture(GL_TEXTURE_2D, 0);
}

void SimpleWorldRendererPlugin::init() {
  loadSkybox();
}

void SimpleWorldRendererPlugin::step(const Desktop3DLocation &loc, double timeDiff_) {
    const bool renderCachedShader = false;
    if (USE_FBO && renderCachedShader) {
	    glClear(GL_DEPTH_BUFFER_BIT);

        glBindFramebuffer(GL_FRAMEBUFFER, fbos[0]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (!checkForErrors()) {
			std::cerr << "GL ISSUE -- SimpleWorldRendererPlugin::step" << std::endl;
			//exit(EXIT_FAILURE);
		}
    }
    
    if(renderCachedShader) {
        glViewport(0,0, textureWidth/2.0, textureHeight);
    }

    static double *orientation;
#ifdef _WIN32
		static const GLuint groundTexture = loadTexture("\\resources\\humus-skybox\\negy.jpg");
//        orientation = getRiftOrientation();
#else
#ifdef __APPLE__
  static const GLuint groundTexture = loadTexture("/resources/humus-skybox/negy.jpg");
#else
  static float sizeX = 64;
  static float sizeY = 64;
  static const GLuint groundTexture = glmLoadTexture("./resources/humus-skybox/negy.jpg", GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE, &sizeX, &sizeY);
#endif
  //        gluInvertMatrix(get_orientation(), orientation);
#endif
    
    
  for (int i2 = 0; i2 < 2; ++i2) {
      if(!renderCachedShader) {
          glViewport(i2*textureWidth/2.0,0, textureWidth/2.0, textureHeight);
      }
//      checkForErrors();
    if (USE_FBO) {
        if(!renderCachedShader) {
            glBindFramebuffer(GL_FRAMEBUFFER, fbos[i2]);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glPushMatrix();
        }
		//glBindFramebuffer(GL_FRAMEBUFFER, fbos[0]);
      //glPushMatrix();
    } else {
      if (i2 > 0)
        break;
    }

      if(renderCachedShader) {
        //glEnable (GL_SCISSOR_TEST);
        if(i2 == 0) {
            //glViewport(0,0, textureWidth/2.0, textureHeight);
        } else {
            glViewport(textureWidth/2.0,0, textureWidth/2.0, textureHeight);
        }
      }

	//glScissor((i2 == 0)? 0 : textureWidth/2.0, 0, textureWidth/2.0, textureHeight);

	// Setup the frustum for the left eye
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glTranslated((i2 == 0) ? IOD: -IOD, 0, 0);
	gluPerspective(90.0f, width/2.0/height, 0.01f, 1000.0f);
      
      if(i2 == 0) orientation = getRiftOrientation();
      
	glMultMatrixd(orientation);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glDisable(GL_SCISSOR_TEST);

    glEnable(GL_TEXTURE_2D);
    //glBindTexture(GL_TEXTURE_2D, 0);

    //double orientation[16];
    //gluInvertMatrix(get_orientation(), orientation);

    //glPushMatrix();
    {
		//glMultMatrixd(orientation);
		glRotated(loc.getXRotation(), 1, 0, 0);
		glRotated(loc.getYRotation(), 0, 1, 0);
	//  glTranslated(0, -1.5, 0);
		glTranslated(0, -0.5, 0);

		renderSkybox();
		glColor4f(1,1,1,1);

          glPushMatrix();
          {
              glTranslated(loc.getXPosition(),
                           loc.getYPosition(),
                           loc.getZPosition());
              
            if(showGround) {
              static const int gridSize = 25;
              static const int textureRepeat = 2*gridSize;

              glBindTexture(GL_TEXTURE_2D, groundTexture);
              glBegin(GL_TRIANGLE_STRIP);
                glTexCoord2d(0, 0);
                glVertex3f(-gridSize, 0, -gridSize);

                glTexCoord2d(textureRepeat, 0);
                glVertex3f(gridSize, 0, -gridSize);

                glTexCoord2d(0, textureRepeat);
                glVertex3f(-gridSize, 0, gridSize);

                glTexCoord2d(textureRepeat, textureRepeat);
                glVertex3f(gridSize, 0, gridSize);
              glEnd();
              //glBindTexture(GL_TEXTURE_2D, 0);
            }

			if (renderToTexture) {
					double ySize = ((double)height / (double)width) / 2.0;
					glTranslated(0, 0.5, 0);
					const double monitorOriginZ = -0.5;
				glBindTexture(GL_TEXTURE_2D, desktopTexture);
				glColor4f(1,1,1,1);
                
                switch(displayShape) {
                    case SphericalDisplay:
                        renderSphericalDisplay(0.5, 100, 100,180,135);//25, 25, 180, 90);
                        break;
                    case CylindricalDisplay:
                        renderCylindricalDisplay(0.5, 100,100,180,180);//25, 25, 180, 90);
                        break;
                    case FlatDisplay:
                    default:
					glBegin(GL_TRIANGLE_STRIP);
					glTexCoord2d(0, 0);
					glVertex3f(-0.5, -ySize, monitorOriginZ);

                    glTexCoord2d(1, 0);
                    glVertex3f(0.5, -ySize, monitorOriginZ);

                    glTexCoord2d(0, 1);
                    glVertex3f(-0.5, ySize, monitorOriginZ);

                    glTexCoord2d(1, 1);
                    glVertex3f(0.5, ySize, monitorOriginZ);
                  glEnd();
                }
                
                ySize = videoHeight/videoWidth/2.0;
                glBindTexture(GL_TEXTURE_2D, videoTexture[i2]);
				glColor4f(1,1,1,1);
                glBegin(GL_TRIANGLE_STRIP);
                glTexCoord2d(0, 1);
                glVertex3f(1.5-0.5, -ySize, monitorOriginZ);
                
                glTexCoord2d(1, 1);
                glVertex3f(1.5+0.5, -ySize, monitorOriginZ);
                
                glTexCoord2d(0, 0);
                glVertex3f(1.5-0.5, ySize, monitorOriginZ);
                
                glTexCoord2d(1, 0);
                glVertex3f(1.5+0.5, ySize, monitorOriginZ);
                glEnd();
                
                
				glEnable(GL_BLEND);

				if(mouseBlendAlternate) {
					glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
				} else {
					glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				}
				

                ySize = ((double)height / (double)width) / 2.0;
				glBindTexture( GL_TEXTURE_2D, cursor );
				glBegin(GL_TRIANGLE_STRIP);
				{
//					const double cursorSize = 32.0; // was 20.0, not sure which is best on each OS
					double x0, x1, y0, y1, z;
					x0 = -0.5+(cursorPosX/physicalWidth);
					x1 = -0.5+((cursorPosX+cursorSize)/physicalWidth);
					y0 = -ySize+ySize*2*(cursorPosY+0.)/physicalHeight;
					y1 = -ySize+ySize*2*(cursorPosY-cursorSize)/physicalHeight;
                    
		#ifdef WIN32
					z = monitorOriginZ+0.00001;
		#else
					z = monitorOriginZ+0.0000001;
		#endif

					glTexCoord2d(0, 0);
					glVertex3f(  x0,  y0, z);
                    
                    glTexCoord2d(1, 0);
                    glVertex3f( x1, y0, z);
                    
                    glTexCoord2d(0, -1);
                    glVertex3f(  x0, y1 ,z);
                    
                    glTexCoord2d(1, -1);
                    glVertex3f(  x1,  y1, z);
                    
                    glTexCoord2d(1, -1);
                    glVertex3f( x1, y1, z);
                    
                    glTexCoord2d(0, -1);
                    glVertex3f(  x0, y1,z);
                }
                glEnd();
//                glDisable(GL_BLEND);
                
				//glBindTexture(GL_TEXTURE_2D, 0);

				if(mouseBlendAlternate) { // restore mode
					glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				}
			} else {
				renderDesktopToTexture();
			}
		}
		glPopMatrix();
        
        if(showDialog) {
            window.render();
        }
    }
    //glPopMatrix();

    //if (USE_FBO) {
    //  glPopMatrix();
    //}
      
      if(USE_FBO && !renderCachedShader) {
          glPopMatrix();
      }

  }

    glViewport(0,0, windowWidth, windowHeight);
  if (USE_FBO) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
      if(!renderCachedShader) {
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glColor4f(1, 1, 1, 1);
      }

    if (ortho) {
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glOrtho(0, 1, 0, 1, -1, 1);
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
    }
      
    if(barrelDistort) {
        if(renderCachedShader) {
            if(lensParametersChanged) {
                render_distortion_lenses();
            }

            render_both_frames(textures[0]);
        } else {
            render_distorted_frame(true, textures[0]);
            render_distorted_frame(false, textures[1]);
        }
    } else {
        if(!SBS) {
            glViewport(0,0, windowWidth, windowHeight);
            glBindTexture(GL_TEXTURE_2D, textures[0]);
            //glPushMatrix();
            glColor4f(1, 1, 1, 1);
            glBegin(GL_TRIANGLE_STRIP);
            glTexCoord2d(0, bottom);
            glVertex3f(0, 0, 0);
            
            glTexCoord2d(0.5, bottom);
            glVertex3f(1, 0, 0);
            
            glTexCoord2d(0, top);
            glVertex3f(0, 1, 0);
            
            glTexCoord2d(0.5, top);
            glVertex3f(1, 1, 0);
            glEnd();
            
        } else {
            for (int i = 0; i < 2; ++i) {
          if (ortho) {
          double originX = (i == 0) ? 0 : 0.5;
          glBindTexture(GL_TEXTURE_2D, textures[i]);
          //glPushMatrix();
          glColor4f(1, 1, 1, 1);
          glBegin(GL_TRIANGLE_STRIP);
            glTexCoord2d(0, bottom);
            glVertex3f(originX, 0, 0);

            glTexCoord2d(1, bottom);
            glVertex3f(originX + 0.5, 0, 0);

            glTexCoord2d(0, top);
            glVertex3f(originX, 1, 0);

            glTexCoord2d(1, top);
            glVertex3f(originX + 0.5, 1, 0);
          glEnd();
          //glPopMatrix();
        } else {
          glBindTexture(GL_TEXTURE_2D, textures[i]);
          glPushMatrix();
          glTranslated((i < 1) ? -0.98 : 0, -0.5, -2.4);
          glColor4f(1, 1, 1, 1);
          glBegin(GL_TRIANGLE_STRIP);
            glTexCoord2d(0, bottom);
            glVertex3f(0, 0, 0);

            glTexCoord2d(1, bottom);
            glVertex3f(1,0,0);

            glTexCoord2d(0, top);
            glVertex3f(0,1,0);

            glTexCoord2d(1, top);
            glVertex3f(1,1,0);
          glEnd();
          glPopMatrix();
        }
      }
        }
    }
    //if (ortho) {
    //  glMatrixMode(GL_PROJECTION);
    //  glLoadIdentity();
    //  gluPerspective(90.0f, 1, 0.01f, 1000.0f);

    //  glMatrixMode(GL_MODELVIEW);
    //}
  }
}

Window SimpleWorldRendererPlugin::getWindowID() {
  return ::window;
}

bool SimpleWorldRendererPlugin::needsSwapBuffers() {
  return doubleBuffered;
}
