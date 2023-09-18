#import "MainController.h"
#import "MyOpenGLView.h"
#import "MyApplication.h"


@implementation MainController

- (void) awakeFromNib
{
    [openGLView awakeFromNib];
}

- (void) dealloc
{

	[super dealloc];
}


- (CFAbsoluteTime) renderTime
{
	return renderTime;
}

- (void) setRenderTime:(CFAbsoluteTime)time
{
	renderTime = time;
}


- (NSSize) computeFrameSize: (NSSize) frameSize
{
	float aspect_r=(float)GetPrimaryGLX()/(float)GetPrimaryGLY(); // aspect ratio
	aspect_r = 1.333333; //TODO this shouldn't be hardcoded, but I need to as the Zoom button screws up the aspect ratio
	NSWindow *window = [NSApp mainWindow];
	float addX = 0;
	float addY = [window frame].size.height - [[window contentView] frame].size.height;
	
	//LogMsg("Shift down is %d", int([NSApp isShiftDown]));
	if ( 
		(GetForceAspectRatioWhenResizing() && ![NSApp isShiftDown])
		||
		(!GetForceAspectRatioWhenResizing() && [NSApp isShiftDown] )
		)
	{
		float oldHeight = frameSize.height;
		frameSize.height = ( (frameSize.width-addX) /aspect_r)+addY;
		
		LogMsg("Forcing aspect ratio %.2f offsetY: %.2f:  %.2f %.2f to %.2f %.2f", aspect_r, addY, frameSize.width, oldHeight, frameSize.width, frameSize.height);
	} else
	{
		
		//let them change it to whatever they want
	}

    
    
	return frameSize;
}

- (NSSize)windowWillResize:(NSWindow*)sender
                    toSize:(NSSize)frameSize
{
	
	
	frameSize = [self computeFrameSize: frameSize];
	
    [self OnResizeFinished];
    
	return frameSize;	
}

- (void) OnResizeFinished
{
		NSRect bounds = [openGLView bounds];
		LogMsg("Finished resizing");
	//	CGLLockContext( (_CGLContextObject*)[[openGLView openGLContext] CGLContextObj]);
		InitDeviceScreenInfoEx(bounds.size.width, bounds.size.height, ORIENTATION_LANDSCAPE_LEFT);
	//	CGLUnlockContext( (_CGLContextObject*) [[openGLView openGLContext] CGLContextObj]);
	
}

- (void) windowDidResize: (NSNotification *) aNotification
{
   NSLog (@"windowDidResize: called");
	
	[openGLView reshape];
}

- (void)windowDidMiniaturize:(NSNotification *)notification
{
	//CGLLockContext( (_CGLContextObject*)[[openGLView openGLContext] CGLContextObj]);
	GetBaseApp()->OnEnterBackground();
	//CGLUnlockContext( (_CGLContextObject*) [[openGLView openGLContext] CGLContextObj]);
	
}

- (BOOL)windowShouldZoom:(NSWindow *)window toFrame:(NSRect)newFrame
{
	
	LogMsg("Window should zoom");	
	return TRUE;
}

- (NSRect)windowWillUseStandardFrame:(NSWindow *)window defaultFrame:(NSRect)newFrame
{
	bool bZoomed = false; // [window isZoomed];
	LogMsg("Window will use standard frame: %.2f %.2f is zoomed is %d", newFrame.size.width, newFrame.size.height, int(bZoomed));	
	newFrame.size = [self computeFrameSize: newFrame.size];

	//this is wrong, temporary
	//newFrame.size.width = 1024;
	//newFrame.size.height = 768;
	
	return newFrame;
}


- (void) applicationDidResignActive: (NSNotification *) aNotification
{
	//CGLLockContext( (_CGLContextObject*)[[openGLView openGLContext] CGLContextObj]);
#ifndef RT_RUNS_IN_BACKGROUND
    GetBaseApp()->OnEnterBackground();
#endif
	//CGLUnlockContext( (_CGLContextObject*) [[openGLView openGLContext] CGLContextObj]);
	
}

- (void) applicationDidBecomeActive: (NSNotification *) aNotification
{
	//CGLLockContext( (_CGLContextObject*)[[openGLView openGLContext] CGLContextObj]);
#ifndef RT_RUNS_IN_BACKGROUND
	GetBaseApp()->OnEnterForeground();
#endif
	//CGLUnlockContext( (_CGLContextObject*) [[openGLView openGLContext] CGLContextObj]);

}
- (void)windowDidDeminiaturize:(NSNotification *)notification
{
//	CGLLockContext( (_CGLContextObject*)[[openGLView openGLContext] CGLContextObj]);
	GetBaseApp()->OnEnterForeground();
	//CGLUnlockContext( (_CGLContextObject*) [[openGLView openGLContext] CGLContextObj]);
	
}


- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
	NSLog(@"Last window closed MainController");
	return YES;
}

- (void)applicationWillTerminate:(NSNotification *)notification
{
	LogMsg("App terminating");	
	
	//wait for render thread to complete
	
	//CGLLockContext( (_CGLContextObject*)[[openGLView openGLContext] CGLContextObj]);
	
	if (GetBaseApp()->IsInitted())
	{
		GetBaseApp()->OnEnterBackground();
		GetBaseApp()->Kill();
	}
//TODO
    //openGLView->m_bQuitASAP = true;
	
	//CGLUnlockContext( (_CGLContextObject*) [[openGLView openGLContext] CGLContextObj]);
	
}
@end
