#import "MyOpenGLView.h"

@implementation MyOpenGLView

// pixel format definition
+ (NSOpenGLPixelFormat*) basicPixelFormat
{
    NSOpenGLPixelFormatAttribute attributes [] = {
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFADepthSize, (NSOpenGLPixelFormatAttribute)16,
        NSOpenGLPFAOpenGLProfile, (NSOpenGLPixelFormatAttribute)NSOpenGLProfileVersionLegacy,
        (NSOpenGLPixelFormatAttribute)0
    };
    NSOpenGLPixelFormat *pf = [[[NSOpenGLPixelFormat alloc] initWithAttributes:attributes] autorelease];
    if (!pf)
    {
        // Fallback: minimal pixel format
        NSOpenGLPixelFormatAttribute fallback [] = {
            NSOpenGLPFADoubleBuffer,
            (NSOpenGLPixelFormatAttribute)0
        };
        pf = [[[NSOpenGLPixelFormat alloc] initWithAttributes:fallback] autorelease];
    }
    return pf;
}

// per-window timer function, basic time based animation preformed here
- (void)animationTimer:(NSTimer *)timer
{
    // Use display instead of calling drawRect directly -
    // this properly triggers the display cycle and ensures
    // the OpenGL context is current before drawing
    [self display];
}

- (void) drawRect:(NSRect)rect
{
    // Ensure our OpenGL context is current
    [[self openGLContext] makeCurrentContext];

    // prepareOpenGL is called lazily by the system on first display.
    // If it hasn't fired yet and we have valid bounds, call it now.
    if (!GetBaseApp()->IsInitted())
    {
        NSRect bounds = [self bounds];
        if (bounds.size.width > 0 && bounds.size.height > 0)
        {
            [self prepareOpenGL];
        }
        else
        {
            // No valid bounds yet - clear to black and wait
            glClearColor(0, 0, 0, 1);
            glClear(GL_COLOR_BUFFER_BIT);
            [[self openGLContext] flushBuffer];
            return;
        }
    }

    if (GetBaseApp()->IsInitted())
    {
        // Drain the OS message queue - this is how SetVideoMode, quit, etc. reach us.
        // On other platforms SDL2Main.cpp or LinuxMain.cpp does this; on macOS we do it here.
        while (!GetBaseApp()->GetOSMessages()->empty())
        {
            OSMessage m = GetBaseApp()->GetOSMessages()->front();
            GetBaseApp()->GetOSMessages()->pop_front();
            [self onOSMessage:&m];
        }

        GetBaseApp()->Update();

        if (!m_bQuitASAP)
        {
            GetBaseApp()->Draw();
        }
    }

    if ([self inLiveResize])
        glFlush();
    else
        [[self openGLContext] flushBuffer];
}

// ---------------------------------

// set initial OpenGL state (current context is set)
// called after context is created
- (void) prepareOpenGL
{
    [super prepareOpenGL];

    GLint swapInt = 1;
    [[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];

    NSRect bounds = [self bounds];
    InitDeviceScreenInfoEx(bounds.size.width, bounds.size.height, ORIENTATION_LANDSCAPE_LEFT);
}
// ---------------------------------

- (void) reshape
{
    [super reshape];
    [[self openGLContext] makeCurrentContext];
    NSRect bounds = [self bounds];
    int w = (int)bounds.size.width;
    int h = (int)bounds.size.height;
    if (w > 0 && h > 0)
    {
        InitDeviceScreenInfoEx(w, h, ORIENTATION_LANDSCAPE_LEFT);
        // Force viewport to logical size — fullscreen transitions can
        // reset the viewport to Retina backing pixels
        glViewport(0, 0, w, h);
        [[self openGLContext] update];
    }
}

- (void) update // window resizes, moves and display changes (resize, depth and display config change)
{
     [super update];
    if (![self inLiveResize])
    {// if not doing live resize
    }
    
}

// ---------------------------------

-(id) initWithFrame: (NSRect) frameRect
{
    NSOpenGLPixelFormat * pf = [MyOpenGLView basicPixelFormat];

    self = [super initWithFrame: frameRect pixelFormat: pf];
    // Disable Retina backing — the engine assumes viewport == logical points.
    // On macOS 10.15+ this defaults to YES, giving a 2x backing surface that
    // breaks glScissor and other pixel-coordinate GL calls.
    [self setWantsBestResolutionOpenGLSurface:NO];
    return self;
}

// ---------------------------------

- (BOOL)acceptsFirstResponder
{
  return YES;
}

// ---------------------------------

- (BOOL)becomeFirstResponder
{
  return  YES;
}

// ---------------------------------

- (BOOL)resignFirstResponder
{
  return YES;
}

// ---------------------------------

- (void) awakeFromNib
{
    time = CFAbsoluteTimeGetCurrent();

    // Set working directory to bundle Resources so relative paths work
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
    char path[PATH_MAX];
    if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path, PATH_MAX))
    {
        LogMsg("Error getting bundle path");
    }
    CFRelease(resourcesURL);
    chdir(path);

    // Start animation timer - prepareOpenGL will be called by the system
    // on the first real draw when the window has valid bounds
    timer = [NSTimer timerWithTimeInterval:(1.0f/60.0f) target:self selector:@selector(animationTimer:) userInfo:nil repeats:YES];
    [[NSRunLoop currentRunLoop] addTimer:timer forMode:NSDefaultRunLoopMode];
    [[NSRunLoop currentRunLoop] addTimer:timer forMode:NSEventTrackingRunLoopMode];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(reshape)
                                                 name:NSViewGlobalFrameDidChangeNotification
                                               object:self];
}


// For left mouse button
- (void)mouseDown:(NSEvent *)theEvent {
    [self handleMouseDown:theEvent withButtonNumber:0];
}

- (void)mouseUp:(NSEvent *)theEvent {
    [self handleMouseUp:theEvent withButtonNumber:0];
}

// For right mouse button
- (void)rightMouseDown:(NSEvent *)theEvent
{
    [self handleMouseDown:theEvent withButtonNumber:1];
}

- (void)rightMouseUp:(NSEvent *)theEvent
{
    [self handleMouseUp:theEvent withButtonNumber:1];
}

// For other mouse buttons (e.g., middle button)
- (void)otherMouseDown:(NSEvent *)theEvent
{
    [self handleMouseDown:theEvent withButtonNumber:[theEvent buttonNumber]];
}

- (void)otherMouseUp:(NSEvent *)theEvent
{
    [self handleMouseUp:theEvent withButtonNumber:[theEvent buttonNumber]];
}


- (void)handleMouseDown:(NSEvent *)theEvent withButtonNumber:(NSInteger)buttonNumber
{
    // Delegate to the controller object for handling mouse events
    //int shiftDown = ([theEvent modifierFlags] & NSShiftKeyMask);
    //int controlDown = ([theEvent modifierFlags] & NSControlKeyMask);

    NSPoint pt = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    pt.y = GetPrimaryGLY()-pt.y; //flip it to upper left hand coords

    ConvertCoordinatesIfRequired(pt.x, pt.y);
    GetMessageManager()->SendGUIEx(MESSAGE_TYPE_GUI_CLICK_START,pt.x, pt.y, buttonNumber );

    //LogMsg("Got mouse down: %.2f, %0.2f", pt.x, pt.y);
    // [controller mouseDown:theEvent];
}

- (void)handleMouseUp:(NSEvent *)theEvent withButtonNumber:(NSInteger)buttonNumber
{
    // Delegate to the controller object for handling mouse events
    NSPoint pt = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    pt.y = GetPrimaryGLY()-pt.y; //flip it to upper left hand coords

    ConvertCoordinatesIfRequired(pt.x, pt.y);
    GetMessageManager()->SendGUIEx(MESSAGE_TYPE_GUI_CLICK_END,pt.x, pt.y, buttonNumber);
    // [controller mouseDown:theEvent];
}


- (void)scrollWheel:(NSEvent *)theEvent
{
    float deltaY = [theEvent deltaY];
    GetMessageManager()->SendGUIEx2(MESSAGE_TYPE_GUI_MOUSEWHEEL, (float)deltaY*128, 0, 0, 0); //why the 128?  Because windows does 128 a chunk
}

// For left mouse button drag
- (void)mouseDragged:(NSEvent *)theEvent {
    [self handleMouseDragged:theEvent withButtonNumber:[theEvent buttonNumber]];
}

// For right mouse button drag
- (void)rightMouseDragged:(NSEvent *)theEvent {
    [self handleMouseDragged:theEvent withButtonNumber:[theEvent buttonNumber]];
}

// For other mouse buttons drag
- (void)otherMouseDragged:(NSEvent *)theEvent {
    [self handleMouseDragged:theEvent withButtonNumber:[theEvent buttonNumber]];
}

// Handling mouse dragged events
- (void)handleMouseDragged:(NSEvent *)theEvent withButtonNumber:(NSInteger)buttonNumber {
    NSPoint pt = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    pt.y = GetPrimaryGLY()-pt.y; //flip it to upper left hand coords

    ConvertCoordinatesIfRequired(pt.x, pt.y);
    GetMessageManager()->SendGUIEx(MESSAGE_TYPE_GUI_CLICK_MOVE, pt.x, pt.y, buttonNumber);
}

- (void)mouseMovedMessage:(NSEvent *)theEvent
{
    // Delegate to the controller object for handling mouse events
    NSPoint pt = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    pt.y = GetPrimaryGLY()-pt.y; //flip it to upper left hand coords

    ConvertCoordinatesIfRequired(pt.x, pt.y);
    GetMessageManager()->SendGUI(MESSAGE_TYPE_GUI_CLICK_MOVE_RAW,pt.x, pt.y);
}

- (void)keyDown:(NSEvent *)theEvent
{
    unichar c = [[theEvent charactersIgnoringModifiers] characterAtIndex:0];

    GetMessageManager()->SendGUI(MESSAGE_TYPE_GUI_CHAR,  (float)ConvertOSXKeycodeToProtonVirtualKey(c), 0.0f);  //lParam holds a lot of random data about the press, look it up if
    
    if (theEvent.isARepeat == NO)
    {
        //if it's not a key repeat, also send here for the more arcade-like key up/down messages
        c =  [[[NSString stringWithCharacters:&c length:1] uppercaseString] characterAtIndex: 0]; //make it uppercase, same as Win

       // LogMsg("Sending key %d as DOWN: converted: %d", c,  ConvertOSXKeycodeToProtonVirtualKey(c));
        
        GetMessageManager()->SendGUI(MESSAGE_TYPE_GUI_CHAR_RAW,   (float)ConvertOSXKeycodeToProtonVirtualKey(c), 1.0f);
    }
}

- (void)keyUp:(NSEvent *)theEvent
{
    unichar c = [[theEvent charactersIgnoringModifiers] characterAtIndex:0];
    
    c =  [[[NSString stringWithCharacters:&c length:1] uppercaseString] characterAtIndex: 0]; //make it uppercase, same
    //LogMsg("Sending key %d as UP: converted: %d", c,  ConvertOSXKeycodeToProtonVirtualKey(c));
       GetMessageManager()->SendGUI(MESSAGE_TYPE_GUI_CHAR_RAW,   (float)ConvertOSXKeycodeToProtonVirtualKey(c), 0.0f);
}


- (void)onOSMessage:(OSMessage *)pMsg
{
    
    switch (pMsg->m_type)
    {
            
        case OSMessage::MESSAGE_OPEN_TEXT_BOX:
            break;
            
        case OSMessage::MESSAGE_CHECK_CONNECTION:
            //pretend we did it
            GetMessageManager()->SendGUI(MESSAGE_TYPE_OS_CONNECTION_CHECKED, (float)RT_kCFStreamEventOpenCompleted, 0.0f);
            break;
            
        case OSMessage::MESSAGE_CLOSE_TEXT_BOX:
            SetIsUsingNativeUI(false);
            break;
        case OSMessage::MESSAGE_SET_FPS_LIMIT:
            //glView.animationIntervalSave = 1.0/pMsg->m_x;
            break;
        case OSMessage::MESSAGE_SET_ACCELEROMETER_UPDATE_HZ:
            break;
        case OSMessage::MESSAGE_FINISH_APP:
        case OSMessage::MESSAGE_SUSPEND_TO_HOME_SCREEN:
        
            //TODO
            m_bQuitASAP = YES;
            [NSApp terminate:nil];
            break;
        case OSMessage::MESSAGE_SET_VIDEO_MODE:
        {
            NSWindow *window = [self window];
            if (!window) window = [NSApp mainWindow];
            if (window)
            {
                NSSize frameSize;
                frameSize.width = pMsg->m_x;
                frameSize.height = pMsg->m_y;
                [window setContentSize:frameSize];
                [window center];
            }
        }
            break;
            
        default:
            LogMsg("Error, unknown message type: %d", pMsg->m_type);
    }
}


@end
