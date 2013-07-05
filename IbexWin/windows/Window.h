//
//  Window.h
//  IbexMac
//
//  Created by Hesham Wahba on 5/7/13.
//  Copyright (c) 2013 Hesham Wahba. All rights reserved.
//

#ifndef __IbexMac__Window__
#define __IbexMac__Window__

#include <iostream>
#include <vector>
#include <string>
#include <set>

namespace Ibex {
    
enum VisibleWindow {
    InfoWindow,FileChooser,CameraChooser
};

class Window {
public:
    Window();
    
    void render();
    bool getIsStereoVideo() { return isStereoVideo; }
    bool getSelectedVideo() { return selectedVideo; }
    bool getSelectedCamera() { return selectedCamera; }
    bool setSelectedVideo(bool selectedVideo_) { selectedVideo = selectedVideo_; return selectedVideo; }
    bool setSelectedCamera(bool selectedCamera_) { selectedCamera = selectedCamera_; return selectedCamera; }
    std::string getSelectedVideoPath() { return videoPath; }
    int getSelectedCameraID() { return selectedCameraID; }

#ifdef __APPLE__
    int processKey(unsigned short keyCode, int down);
#else
#ifdef _WIN32
	int processKey(unsigned char key, int down);
	int processSpecialKey(unsigned char key, int down);
#endif
#endif
    
private:
    void renderFileChooser();
    void renderCameraChooser();
    void renderInfoWindow();
    
private:
    bool selectedVideo;
    bool isStereoVideo;
public:
    std::string videoPath;
    
private:
    VisibleWindow visibleWindow;
    std::string currentPath;
    bool directoryChanged;
    int selectedFile;
    int selectedCamera;
    int selectedCameraID;
    std::vector<std::string> directoryList;
    std::vector<int> cameras;
    std::set<std::string> fileTypes;
    
};
    
}

#endif /* defined(__IbexMac__Window__) */
