**Ibex** 3D Virtual Reality Desktop/Compositing Manager for X11/Linux, Windows and Mac OS

# About
Ibex is a simple 3D Virtual Reality (VR) world that will allow you to have a number of virtual monitors that contain your current actual desktop or the ability to watch movies.  The idea is for you to have a pretty virtual world to work in regardless of where you are
so that you can be flying on a plane, for example, but still be flanked by multiple large high resolution monitors and still be effective rather than be limited by a tiny laptop screen.  In addition, through the eventual use of plugins you can change your environment to suit your needs so you can work in a giant castle or out in a serene field.

Networking and multi-user support, physics engine and plugin support are all coming.  Movie playing has gotten intial support on the Mac as well.

It currently supports the Rift on Windows and Mac OS and when the SDK is updated Linux as well.

# Home Page and Downloads

Home page and binaries [http://hwahba.com/ibex](http://hwahba.com/ibex)

### Launch
Default simple renderer: ./ibex  
Fancy Irrlicht renderer with Quake 3 level: ./ibex -i  
Ogre3D renderer with simple terrain and water: ./ibex -o  
Disable Side-by-Side rendering: -m (for mono)  

### Controls
Toggle Control Desktop/Move Around World: CTRL+SHIFT+Y, Shift+F1 or CTRL+SHIFT+G (Mac), CTRL+SHIFT+G (Win)  
Look: Move Mouse  
Move Forward: W  
Move Backwards: S  
Walk Sideways Left (Strafe Left): A/Q  
Walk Sideways Right (Strafe Right): D/E   
Jump: SPACE (Irrlicht renderer only)  
Show FPS and Info Dialog: / (Mac Only)
Choose movies: Show info dialog with ‘/‘ then press 1 or 2 for regular or stereo movies, then navigate using arrow keys and enter to select a movie
Adjust IPD for Rift stereo viewing: -/+ keys

Toggle Barrel Lens Distort: B  
Toggle Ground Layer: G  

If using the iPhone controller app (source included) you need to connect to the
correct IP for your computer in the iPhone app.  When you hit "Broadcast" the
orientation of the phone becomes the base orientation and you will start looking
around from there.

## Requirements
### Mac
XCode 4.5, OSX 10.8 (Mountain Lion) - Build and run from XCode, check the project dependencies
to find out where the bullet3d physics library and sixense SDK need to go to build properly.  OculusSDK needs to be placed at the same level as your checked-out git repo for ibex and it must be version 0.2.1 or later.  You'll need to rebuild the libovr.a library with C++ RTTI and Exceptions enabled or else it won't link properly with ibex.

    brew install ffmpeg x264 xvid yasm faac
    brew install ffmpeg --use-clang
    
bullet physics how to build:

    cd ~/Downloads/bullet-2.81-rev2613
    mkdir bullet-build
    cd bullet-build
    cmake .. -G "Unix Makefiles" -DINSTALL_LIBS=ON
    
#-DBUILD_SHARED_LIBS=ON                                                                                                                   
 make -j4
 sudo make install

### Windows
Visual Studio 2012 - Build and run from Visual Studio, Rift SDK must be at same level as ibex folder and named: OculusSDK.

You also need to have ffmpeg installed where VS can find it.  I recommend the builds from ([http://ffmpeg.zeranoe.com/builds](http://ffmpeg.zeranoe.com/builds/ "http://ffmpeg.zeranoe.com/builds/")).  You should download the 32-bit shared and dev builds and extract them at the same level as ibex.

Your directory structure should look like:

    * Parent Directory:
    |
    |->ibex
       |...
       |->IbexMac
       |->IbexWin
    |->ffmpeg
       |
       |->lib
    |->ffmpeg-shared
       |
       |->bin
    |->OculusSDK

### Linux
Setting up my Ubuntu 12.10 install:

    sudo apt-get install mesa-utils eclipse-cdt freeglut3-dev glew-utils libglew1.6-dev libxrender-dev libxrender1-dbg libxrender1 libxdamage-dev libxdamage1-dbg libxcomposite1-dbg libxcomposite-dev libxxf86vm-dev libxxf86vm1-dbg libxcb1-dev libxrandr-dev libxft-dev libx11-xcb-dev libxi-dev cmake  
    sudo apt-get install libtinyxml-dev  
    sudo apt-get install nvidia-current-updates-dev  
    sudo apt-get install libirrlicht-dev  

* Blender
* Blender2Ogre to convert models to Ogre meshes: https://code.google.com/p/blender2ogre/
 
To run:  

    cd ~/workspace  
    mkdir build  
    cd build  
    cmake ../ibex  
    make install  
    ./ibex  

----

Author: Hesham Wahba (http://hwahba.com)  
Copyright Hesham Wahba 2012,2013