//
//  EAGLView.m
//
//  Created by Seth Robinson on 3/6/09.
//  For license info, check the license.txt file that should have come with this.
//

#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/EAGLDrawable.h>
#import "EAGLView.h"
#import "MyAppDelegate.h"
#import "util/RenderUtils.h"

CGRect iOS7StyleScreenBounds();

#define USE_DEPTH_BUFFER 1

// A class extension to declare private methods
@interface EAGLView ()

@property (nonatomic, retain) EAGLContext *context;
@property (nonatomic, assign) NSTimer *animationTimer;

- (BOOL) createFramebuffer;
- (void) destroyFramebuffer;

@end


@implementation EAGLView

@synthesize context;
@synthesize animationTimer;
@synthesize animationInterval;
@synthesize animationIntervalSave;


// You must implement this
+ (Class)layerClass {
	return [CAEAGLLayer class];
}

//The GL view is stored in the nib file. When it's unarchived it's sent -initWithCoder:
- (id)initWithCoder:(NSCoder*)coder
{

	if ((self = [super initWithCoder:coder]))
	{
		
		// Get the layer
		CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
		
		eaglLayer.opaque = YES;
		eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
		   [NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
		
		context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
		
		if (!context || ![EAGLContext setCurrentContext:context])
		{
			[self release];
			return nil;
		}
		
		animationIntervalSave = 1.0/60.0;
		animationInterval = animationIntervalSave;
	}
	
    CGFloat pixelScale = [[UIScreen mainScreen] scale];
    UIScreen *pScreen = [UIScreen mainScreen];
    //CGRect fullScreenRect = pScreen.bounds;
    //Fix for iOS 8
    CGRect fullScreenRect = iOS7StyleScreenBounds();
   
    bool bUseSizeGuess = false;
    SetProtonPixelScaleFactor(pixelScale);
    
	if (GetPrimaryGLX() == 0)
    {
        bUseSizeGuess = true;
        SetPrimaryScreenSize(fullScreenRect.size.width*GetProtonPixelScaleFactor(), fullScreenRect.size.height*GetProtonPixelScaleFactor());
        SetupScreenInfo(GetPrimaryGLX(), GetPrimaryGLY(), 0);
    }
    
	if (!GetBaseApp()->Init())
	{
		
		NSLog(@"Couldn't init app");
		[self release];
		return nil;
	}
  
    if (bUseSizeGuess)
    {
        //put it back, otherwise we may not fix it later
        SetPrimaryScreenSize(0,0);
    }
    
    GetBaseApp()->OnScreenSizeChange();
    
	return self;
}

- (void)onKill
{
	GetBaseApp()->Kill();
}

- (void)drawView 
{
		
	if (!viewRenderbuffer) return;
		
	[EAGLContext setCurrentContext:context];

	glBindFramebufferOES(GL_FRAMEBUFFER_OES, viewFramebuffer);
	glViewport(0, 0, GetPrimaryGLX(), GetPrimaryGLY());
	
	if (GetScreenSizeX() != 0)
    {
        GetBaseApp()->Update();
        GetBaseApp()->Draw();
    } else
    {
        //not quite ready
    }

	while (!GetBaseApp()->GetOSMessages()->empty())
	{
		OSMessage m = GetBaseApp()->GetOSMessages()->front();
		GetBaseApp()->GetOSMessages()->pop_front();
		
		//LogMsg("Got OS message %d, %s", m.m_type, m.m_string.c_str());
		MyAppDelegate *appDelegate = (MyAppDelegate *)[[UIApplication sharedApplication] delegate];
		
		[appDelegate onOSMessage: &m];
	}
	
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, viewRenderbuffer);
	// if(main_throttled_update()) 
	[context presentRenderbuffer:GL_RENDERBUFFER_OES];
}


- (void)layoutSubviews
{
	[EAGLContext setCurrentContext:context];
	[self destroyFramebuffer];
	[self createFramebuffer];
	[self drawView];
}



- (BOOL)createFramebuffer
{
    
    CGFloat pixelScale = [[UIScreen mainScreen] scale];
    UIScreen *pScreen = [UIScreen mainScreen];
//    CGRect fullScreenRect = pScreen.bounds;
    CGRect fullScreenRect = iOS7StyleScreenBounds();
    
    
    LogMsg("Scale: %.2f", pixelScale);
    SetProtonPixelScaleFactor(pixelScale);

    [self setContentScaleFactor: GetProtonPixelScaleFactor()];

    
    glGenFramebuffersOES(1, &viewFramebuffer);
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, viewFramebuffer);

	glGenRenderbuffersOES(1, &viewRenderbuffer);
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, viewRenderbuffer);

    if (![context renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:(CAEAGLLayer*)self.layer]) return NO;
	glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, viewRenderbuffer);
	
 	glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &backingWidth);
	glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &backingHeight);
		
    
    
	if (USE_DEPTH_BUFFER) 
	{
		glGenRenderbuffersOES(1, &depthRenderbuffer);
		glBindRenderbufferOES(GL_RENDERBUFFER_OES, depthRenderbuffer);
		glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT16_OES, backingWidth, backingHeight);
		glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, depthRenderbuffer);
	}

	if(glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES) {
		NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES));
		return NO;
	}

    if (GetPrimaryGLX() == 0)
    {
        SetPrimaryScreenSize(backingWidth, backingHeight);
        SetupScreenInfo(GetPrimaryGLX(), GetPrimaryGLY(), 0);
    }

    
	return YES;
}


- (void)destroyFramebuffer
{

	
	glDeleteFramebuffersOES(1, &viewFramebuffer);
	viewFramebuffer = 0;
	glDeleteRenderbuffersOES(1, &viewRenderbuffer);
	viewRenderbuffer = 0;
	
	if(depthRenderbuffer)
	{
		glDeleteRenderbuffersOES(1, &depthRenderbuffer);
		depthRenderbuffer = 0;
	}
}


- (void)startAnimation 
{
	self.animationTimer = [NSTimer scheduledTimerWithTimeInterval:animationInterval target:self selector:@selector(drawView) userInfo:nil repeats:YES];
}


- (void)stopAnimation 
{
	self.animationTimer = nil;
}


- (void)setAnimationTimer:(NSTimer *)newTimer
{
	[animationTimer invalidate];
	animationTimer = newTimer;
}


- (void)setAnimationInterval:(NSTimeInterval)interval
{
	
	animationInterval = interval;
	
	if (animationTimer)
	{
		[self stopAnimation];
		[self startAnimation];
	}
}

- (void)setAnimationIntervalSave:(NSTimeInterval)interval 
{
	
	animationIntervalSave = interval;

	[self setAnimationInterval:animationIntervalSave];
}

- (void)dealloc 
{

	[self stopAnimation];
	
	if ([EAGLContext currentContext] == context) 
	{
		[EAGLContext setCurrentContext:nil];
	}
	
	[context release];	
	[super dealloc];
}

const int MAX_TOUCHES= 12;

class TouchTrack
{
public:

	TouchTrack()
	{
		m_touchPointer = NULL;
	}

	void *m_touchPointer;
};

TouchTrack g_touchTracker[MAX_TOUCHES];

int GetFingerTrackIDByTouch(void* touch)
{
		for (int i=0; i < MAX_TOUCHES; i++)
		{
			if (g_touchTracker[i].m_touchPointer == touch)
			{
				return i;
			}
		}

	//LogMsg("Can't locate fingerID by touch %d", touch);
	return -1;
}

int AddNewTouch(void* touch)
{
		for (int i=0; i < MAX_TOUCHES; i++)
		{
			if (!g_touchTracker[i].m_touchPointer)
			{
				//hey, an empty slot, yay
				g_touchTracker[i].m_touchPointer = touch;
				return i;
			}
		}

	LogMsg("Can't add new fingerID");
	return -1;
}

int GetTouchesActive()
{
int count = 0;

	for (int i=0; i < MAX_TOUCHES; i++)
		{
			if (g_touchTracker[i].m_touchPointer)
			{
				count++;
			}
		}
return count;
}

// Handles the start of a touch
- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
  	// Enumerate through all the touch objects.

	for (UITouch *touch in touches)
	{
		//found a touch.  Is it already on our list?
		int fingerID = GetFingerTrackIDByTouch(touch);

		if (fingerID == -1)
		{
			//add it to our list
			fingerID = AddNewTouch(touch);
		} else
		{
			//already on the list.  Don't send this
			//LogMsg("Ignoring touch %d", fingerID);
			continue;
		}

		CGPoint pt =[touch locationInView:self];
		ConvertCoordinatesIfRequired(pt.x, pt.y);
		GetMessageManager()->SendGUIEx(MESSAGE_TYPE_GUI_CLICK_START,pt.x, pt.y,fingerID);			
	}	

	  
	#ifdef _DEBUG
	//LogMsg("%d touches active", GetTouchesActive());
	#endif
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
  	// Enumerate through all the touch objects.
	for (UITouch *touch in touches)
	{
		//found a touch.  Is it already on our list?
		int fingerID = GetFingerTrackIDByTouch(touch);
		if (fingerID != -1)
		{
			g_touchTracker[fingerID].m_touchPointer = NULL; //clear it
		} else
		{
			//wasn't on our list
			continue;
		}
		
		CGPoint pt =[touch locationInView:self];
		ConvertCoordinatesIfRequired(pt.x, pt.y);
		GetMessageManager()->SendGUIEx(MESSAGE_TYPE_GUI_CLICK_END,pt.x, pt.y, fingerID);
	}	
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
  	// Enumerate through all the touch objects.
	for (UITouch *touch in touches)
	{
		//found a touch.  Is it already on our list?
		int fingerID = GetFingerTrackIDByTouch(touch);
		if (fingerID != -1)
		{
			g_touchTracker[fingerID].m_touchPointer = NULL; //clear it
		} else
		{
			//wasn't on our list
			continue;
		}
		
		CGPoint pt =[touch locationInView:self];
		ConvertCoordinatesIfRequired(pt.x, pt.y);
		GetMessageManager()->SendGUIEx(MESSAGE_TYPE_GUI_CLICK_END,pt.x, pt.y, fingerID);
	}	
}


- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
   // Enumerate through all the touch objects.
	for (UITouch *touch in touches)
	{
	
		//found a touch.  Is it already on our list?
		int fingerID = GetFingerTrackIDByTouch(touch);
		if (fingerID != -1)
		{
			//found it
		} else
		{
			//wasn't on our list?!
			continue;
		}
	
		CGPoint pt =[touch locationInView:self];
		ConvertCoordinatesIfRequired(pt.x, pt.y);
		GetMessageManager()->SendGUIEx(MESSAGE_TYPE_GUI_CLICK_MOVE,pt.x, pt.y, fingerID);
	}	
}


@end
