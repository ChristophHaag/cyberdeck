//
//  AppDelegate.h
//  IbexMac
//
//  Created by Hesham Wahba on 12/25/12.
//  Copyright (c) 2012 Hesham Wahba. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class MyOpenGLView;
@class MyOgreView;
@class ServerController;

extern "C" {
    typedef int CGSConnectionID;
    CGError CGSSetConnectionProperty(CGSConnectionID cid, CGSConnectionID targetCID, CFStringRef key, CFTypeRef value);
    int _CGSDefaultConnection();
}

@interface AppDelegate : NSObject <NSApplicationDelegate,NSWindowDelegate> {
#ifdef ENABLE_OGRE3D
    MyOgreView *fullScreenView;
#else
    MyOpenGLView *fullScreenView;
#endif
    
    ServerController *serverController;
}

@property (assign) IBOutlet NSWindow *window;
//@property (retain,nonatomic) IBOutlet NSWindow *window;
@property (retain,nonatomic) NSWindow *retainedWindow;
@property (retain,nonatomic) NSOpenGLPixelFormat* pixelFormat;

@end
