//
//  ScreenshotView.h
//  IbexMac
//
//  Created by Hesham Wahba on 4/25/13.
//  Copyright (c) 2013 Hesham Wahba. All rights reserved.
//

#include "opengl_helpers.h"

#import <Cocoa/Cocoa.h>

extern NSCondition *cocoaCondition;

@interface ScreenshotView : NSView

@property (retain,nonatomic) NSOpenGLPixelFormat *pixelFormat;
@property (retain,nonatomic) NSOpenGLContext *share;
@property (nonatomic) CGContextRef spriteContext;

- (void)loopScreenshot;

@end
