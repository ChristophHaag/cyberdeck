//
//  VLCVideoPlayer.h
//  IbexMac
//
//  Created by Hesham Wahba on 4/30/13.
//  Copyright (c) 2013 Hesham Wahba. All rights reserved.
//

#ifndef __VLCVideoPlayer__
#define __VLCVideoPlayer__

#include <list>
#include <vector>
#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

#if defined(__APPLE__) || defined(WIN32)
typedef unsigned int Display;
typedef unsigned int GLXDrawable;
#endif

#if !defined(__APPLE__) && !defined(WIN32)
#include "../opengl_helpers.h"
#endif

struct CvCapture;

namespace Ibex {
    

class VLCVideoPlayer {
public:
    VLCVideoPlayer();
    ~VLCVideoPlayer();
    
    int playVideo(const char *fileName, bool isStereo, Display *dpy, GLXDrawable root, const void *data);

    int openCamera(bool isStereo, int cameraId);
    static std::vector<int> listCameras();
    void stopCapturing();
    void stopPlaying();
    
public:
    unsigned int *videoTexture;
    unsigned int width,height;
    
    void createVideoTextures(bool isStereo, int width, int height);
private:
    void initOpenCV(bool isStereo, int cameraId);
    int initVideo(const char *fileName_, bool isStereo);
    
    bool done;
    bool isStereo;
    bool videoDone;
    bool audioDone;
    void *data;

    
private:
    bool openCVInited;
    bool captureVideo;
    CvCapture *cvCapture;
    
private:
    std::thread syncThread;
    std::thread audioThread;

 private:
    int playVLCVideo(const char *fileName, Display *dpy, GLXDrawable root);
};
    
}

#endif /* defined(__VLCVideoPlayer__) */
