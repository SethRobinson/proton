#import "PlatformSetup.h"
#import "BaseApp.h"
#import <Cocoa/Cocoa.h>

@interface MyOpenGLView : NSOpenGLView
{
    CFAbsoluteTime msgTime; // message posting time for expiration
    
    NSTimer* timer;
 
    CFAbsoluteTime time;
    bool m_bQuitASAP;
}

+ (NSOpenGLPixelFormat*) basicPixelFormat;


- (void) updateObjectRotationForTimeDelta:(CFAbsoluteTime)deltaTime;
- (void)animationTimer:(NSTimer *)timer;


-(IBAction) info: (id) sender;

- (void)keyDown:(NSEvent *)theEvent;

- (void) drawRect:(NSRect)rect;

- (void) prepareOpenGL;
- (void) update;        // moved or resized

- (BOOL) acceptsFirstResponder;
- (BOOL) becomeFirstResponder;
- (BOOL) resignFirstResponder;

- (id) initWithFrame: (NSRect) frameRect;
- (void) awakeFromNib;

@end
