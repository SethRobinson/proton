
#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>
#import "PlatformSetup.h"
#import "BaseApp.h"
#import "App.h"

@class MyOpenGLView;
@class Scene;

@interface MainController : NSResponder 
{
	// window mode
	IBOutlet MyOpenGLView *openGLView;

	BOOL isAnimating;
	CFAbsoluteTime renderTime;
}

- (CFAbsoluteTime) renderTime;
- (void) setRenderTime:(CFAbsoluteTime)time;

@end
