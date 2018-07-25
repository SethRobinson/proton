
#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>
#import "PlatformSetup.h"
#import "BaseApp.h"

@class MyOpenGLView;
@class Scene;

@interface MainController : NSResponder 
{

	BOOL isInFullScreenMode;
	
	// full-screen mode
#if MAC_OS_X_VERSION_MIN_REQUIRED > MAC_OS_X_VERSION_10_5
	NSWindow *fullScreenWindow;
	MyOpenGLView *fullScreenView;
#else
	CGLContextObj fullScreenContextObj;
#endif
	
	// window mdoe
	IBOutlet MyOpenGLView *openGLView;
	
	//Scene *scene;
	BOOL isAnimating;
	CFAbsoluteTime renderTime;
}

- (IBAction) goFullScreen:(id)sender;
- (void) goWindow;

//- (Scene*) scene;

- (CFAbsoluteTime) renderTime;
- (void) setRenderTime:(CFAbsoluteTime)time;
- (void) OnResizeFinished;

@end
