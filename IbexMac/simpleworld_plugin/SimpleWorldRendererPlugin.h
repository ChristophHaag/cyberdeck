/*
 * SimpleWorldRendererPlugin.h
 *
 *  Created on: Sep 25, 2012
 *      Author: Hesham Wahba
 */

#ifndef SIMPLEWORLDRENDERERPLUGIN_H_
#define SIMPLEWORLDRENDERERPLUGIN_H_

#include "../RendererPlugin.h"
#include "../ibex.h"

#include "GLSLShaderProgram.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "../terrain/Terrain.h"

extern GLSLShaderProgram skyboxShaderProgram;
extern GLSLShaderProgram groundShaderProgram;
extern GLSLShaderProgram standardShaderProgram;
extern GLSLShaderProgram shadowProgram;
extern GLSLShaderProgram waterShaderProgram;

extern GLint ShadowUniformLocations[1];
extern GLint ShadowAttribLocations[3];

class SimpleWorldRendererPlugin : public RendererPlugin {
public:
    SimpleWorldRendererPlugin();
    ~SimpleWorldRendererPlugin();
    
    void loadSkybox();
    void renderSkybox(const glm::mat4 &modelView, const glm::mat4 &proj);
    void renderGround(const glm::mat4 &MVP, const glm::mat4 &V, const glm::mat4 &M, bool shadowPass, const glm::mat4 &depthMVP);
    void renderVideoDisplayFlat(const glm::mat4 &MVP, const glm::mat4 &V, const glm::mat4 &M, const glm::mat4 &depthMVP);
    void renderIbexDisplayFlat(const glm::mat4 &MVP, const glm::mat4 &V, const glm::mat4 &M, bool shadowPass, const glm::mat4 &depthMVP);
    
    void init();
    void step(const Desktop3DLocation &loc_, double timeDiff_, const double &time_);
    bool needsSwapBuffers();
    
    Window getWindowID();
private:
    void render(const glm::mat4 &proj_, const glm::mat4 &view_, const glm::mat4 &playerCamera_, const glm::mat4 &playerRotation_, bool shadowPass, const glm::mat4 &depthBiasMVP, const double &time);
    
    GLuint _skybox[6];
    GLuint _skycube;
    
    Terrain terrain;
};

#endif /* SIMPLEWORLDRENDERERPLUGIN_H_ */
