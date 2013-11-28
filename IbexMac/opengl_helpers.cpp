#include "opengl_helpers.h"

bool gluInvertMatrix(const double m[16], double invOut[16])
{
  double inv[16], det;
  int i;

  inv[0] = m[5]  * m[10] * m[15] -
           m[5]  * m[11] * m[14] -
           m[9]  * m[6]  * m[15] +
           m[9]  * m[7]  * m[14] +
           m[13] * m[6]  * m[11] -
           m[13] * m[7]  * m[10];

  inv[4] = -m[4]  * m[10] * m[15] +
            m[4]  * m[11] * m[14] +
            m[8]  * m[6]  * m[15] -
            m[8]  * m[7]  * m[14] -
            m[12] * m[6]  * m[11] +
            m[12] * m[7]  * m[10];

  inv[8] = m[4]  * m[9] * m[15] -
           m[4]  * m[11] * m[13] -
           m[8]  * m[5] * m[15] +
           m[8]  * m[7] * m[13] +
           m[12] * m[5] * m[11] -
           m[12] * m[7] * m[9];

  inv[12] = -m[4]  * m[9] * m[14] +
             m[4]  * m[10] * m[13] +
             m[8]  * m[5] * m[14] -
             m[8]  * m[6] * m[13] -
             m[12] * m[5] * m[10] +
             m[12] * m[6] * m[9];

  inv[1] = -m[1]  * m[10] * m[15] +
            m[1]  * m[11] * m[14] +
            m[9]  * m[2] * m[15] -
            m[9]  * m[3] * m[14] -
            m[13] * m[2] * m[11] +
            m[13] * m[3] * m[10];

  inv[5] = m[0]  * m[10] * m[15] -
           m[0]  * m[11] * m[14] -
           m[8]  * m[2] * m[15] +
           m[8]  * m[3] * m[14] +
           m[12] * m[2] * m[11] -
           m[12] * m[3] * m[10];

  inv[9] = -m[0]  * m[9] * m[15] +
            m[0]  * m[11] * m[13] +
            m[8]  * m[1] * m[15] -
            m[8]  * m[3] * m[13] -
            m[12] * m[1] * m[11] +
            m[12] * m[3] * m[9];

  inv[13] = m[0]  * m[9] * m[14] -
            m[0]  * m[10] * m[13] -
            m[8]  * m[1] * m[14] +
            m[8]  * m[2] * m[13] +
            m[12] * m[1] * m[10] -
            m[12] * m[2] * m[9];

  inv[2] = m[1]  * m[6] * m[15] -
           m[1]  * m[7] * m[14] -
           m[5]  * m[2] * m[15] +
           m[5]  * m[3] * m[14] +
           m[13] * m[2] * m[7] -
           m[13] * m[3] * m[6];

  inv[6] = -m[0]  * m[6] * m[15] +
            m[0]  * m[7] * m[14] +
            m[4]  * m[2] * m[15] -
            m[4]  * m[3] * m[14] -
            m[12] * m[2] * m[7] +
            m[12] * m[3] * m[6];

  inv[10] = m[0]  * m[5] * m[15] -
            m[0]  * m[7] * m[13] -
            m[4]  * m[1] * m[15] +
            m[4]  * m[3] * m[13] +
            m[12] * m[1] * m[7] -
            m[12] * m[3] * m[5];

  inv[14] = -m[0]  * m[5] * m[14] +
             m[0]  * m[6] * m[13] +
             m[4]  * m[1] * m[14] -
             m[4]  * m[2] * m[13] -
             m[12] * m[1] * m[6] +
             m[12] * m[2] * m[5];

  inv[3] = -m[1] * m[6] * m[11] +
            m[1] * m[7] * m[10] +
            m[5] * m[2] * m[11] -
            m[5] * m[3] * m[10] -
            m[9] * m[2] * m[7] +
            m[9] * m[3] * m[6];

  inv[7] = m[0] * m[6] * m[11] -
           m[0] * m[7] * m[10] -
           m[4] * m[2] * m[11] +
           m[4] * m[3] * m[10] +
           m[8] * m[2] * m[7] -
           m[8] * m[3] * m[6];

  inv[11] = -m[0] * m[5] * m[11] +
             m[0] * m[7] * m[9] +
             m[4] * m[1] * m[11] -
             m[4] * m[3] * m[9] -
             m[8] * m[1] * m[7] +
             m[8] * m[3] * m[5];

  inv[15] = m[0] * m[5] * m[10] -
            m[0] * m[6] * m[9] -
            m[4] * m[1] * m[10] +
            m[4] * m[2] * m[9] +
            m[8] * m[1] * m[6] -
            m[8] * m[2] * m[5];

  det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

  if (det == 0)
      return false;

  det = 1.0 / det;

  for (i = 0; i < 16; i++)
      invOut[i] = inv[i] * det;

  return true;
}

double distort2ShaderScaleFactor(double ax, double ay)
{
  const double x = 1.0;
  const double y = 1.0;

  const double len = 1.0;
  const double len_2 = len;

  double xx = len/(1.0 - ax*len_2);
  double yy = len/(1.0 - ay*len_2);

  double px = (x/(1-ax*xx*xx)+1.0)*0.5;
  double py = (y/(1-ay*yy*yy)+1.0)*0.5;

  return px/py;
}

static GLboolean depthTestEnabled =0;
static GLboolean stencilTestEnabled =0;
static int matrixMode = 0;
void saveState()
{
//  //    driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);
//  //======================================================================
//  //rendering the raw opengl code in a custom scene node
//  // save matrices
//  glGetIntegerv(GL_MATRIX_MODE, &matrixMode);
//
//  glMatrixMode(GL_MODELVIEW);
//  glPushMatrix();
//  glLoadIdentity();
//  glMatrixMode(GL_PROJECTION);
//  glPushMatrix();
//  glLoadIdentity();
//  glMatrixMode(GL_TEXTURE);
//  glPushMatrix();
//  glLoadIdentity(); //Texture addressing should start out as direct.
//  depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
//  glDisable(GL_DEPTH_TEST);
//  stencilTestEnabled = glIsEnabled(GL_STENCIL_TEST);
//  glDisable(GL_STENCIL_TEST);
//  // save attribs
//  glPushAttrib(GL_ALL_ATTRIB_BITS);
//  // call native rendering function
//  //////////////////
//  glEnable(GL_TEXTURE_2D);
//  glDisable(GL_LIGHTING);
//  glDisable(GL_CULL_FACE);
//  glEnable(GL_DEPTH_TEST);
}
void restoreState()
{
//  ////////////////
//  // restore original state
//  glPopAttrib();
//  if (stencilTestEnabled) {
//    glEnable(GL_STENCIL_TEST);
//  }
//  if (depthTestEnabled) {
//    glEnable(GL_DEPTH_TEST);
//  }
//  // restore matrices
//  glMatrixMode(GL_TEXTURE);
//  glPopMatrix();
//  glMatrixMode(GL_PROJECTION);
//  glPopMatrix();
//  glMatrixMode(GL_MODELVIEW);
//  glPopMatrix();
//
//  glMatrixMode(matrixMode);
}
