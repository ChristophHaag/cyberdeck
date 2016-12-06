/*
 * x11.h
 *
 *  Created on: Jul 27, 2013
 *      Author: Hesham Wahba
 */

#pragma once

#ifndef OSVR_H_
#define OSVR_H_
#include <osvr/ClientKit/ClientKit.h>
#include <osvr/RenderKit/RenderManager.h>

#include <glm/common.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <osvr/ClientKit/InterfaceStateC.h>
inline glm::mat4 toGlm(const osvr::renderkit::OSVR_ProjectionMatrix &projection) {
    double proj[16];
    OSVR_Projection_to_OpenGL(proj, projection);
    return glm::make_mat4(proj);
}

static const char* OSVR_APPLICATION_IDENTIFIER = "com.cyberdeck";
// It is OK for the identifier to be identical on multiple clients connected to the same OSVR server.


extern osvr::clientkit::ClientContext* _osvrContext;
extern osvr::renderkit::RenderManager* _osvrRender;

extern osvr::renderkit::RenderManager::RenderParams _renderParams;
extern std::vector<osvr::renderkit::RenderInfo> _renderInfo;
extern std::vector<osvr::renderkit::RenderInfo> _presentInfo;

extern osvr::renderkit::RenderBuffer _colorBuffer;
extern std::vector<osvr::renderkit::RenderBuffer> _colorBuffers;
extern std::vector<osvr::renderkit::OSVR_ViewportDescription> _textureViewports;
//auto        _sensorZeroRotation = 0;//glm::inverse(toGlm(_osvrRender->GetRenderInfo()[0].pose.rotation));
//auto        _sensorZeroTranslation = 0;//-0.5f * (toGlm(_osvrRender->GetRenderInfo()[0].pose.translation));
#if 0

#include "OVR.h"

extern OVR::Ptr<OVR::DeviceManager> pManager;
extern OVR::Ptr<OVR::HMDDevice> pHMD;
extern OVR::Ptr<OVR::SensorDevice> pSensor;
extern OVR::SensorFusion FusionResult;
extern OVR::HMDInfo Info;
extern bool InfoLoaded;
extern bool riftConnected;

extern int riftX;
extern int riftY;
extern int riftResolutionX;
extern int riftResolutionY;

void initRift();
void cleanUpRift();
#endif

#endif /* OSVR_H_ */
