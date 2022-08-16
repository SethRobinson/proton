//
//  MyViewController.m
//  Created by Seth Robinson on 3/6/09.
//  For license info, check the license.txt file that should have come with this.
//


#import "MyViewController.h"
#import "EAGLView.h"
#import "iOSUtils.h"
#import <GameController/GameController.h>

CGRect iOS7StyleScreenBounds();

@implementation MyViewController

@synthesize glView;
@synthesize window;

// Implement viewDidLoad to do additional setup after loading the view.
- (void)viewDidLoad 
{
	[glView startAnimation];
	
	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(didRotate:)
												 name:UIDeviceOrientationDidChangeNotification
											   object:nil];
	
	self.view.backgroundColor = [UIColor blackColor];
    [super viewDidLoad];
	
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)myinterfaceOrientation
{
    if (!GetBaseApp()->GetManualRotationMode())
    {
        //Let our view controller handle our rotations for us
        
        if (!CanRotateTo(myinterfaceOrientation))
        {
			LogMsg("Auto, but refusing orientation %d", myinterfaceOrientation);
			return false;
        }

        return true;
    }
    // Return YES for supported orientations
	//No matter what, we tell it to do portrait mode
	/*
	
	//LogMsg("Setting orientation to interfaceOrientation %d", myinterfaceOrientation);
	[[UIApplication sharedApplication] setStatusBarOrientation: myinterfaceOrientation animated:NO];
	SetupScreenInfoIPhone(myinterfaceOrientation); 
	*/
	return false;
}

- (BOOL)shouldAutorotate
{
    if (!GetBaseApp()->GetManualRotationMode())
    {
        //Let our view controller handle our rotations for us
        return true;
    }
   	return false;
}

//in 2020 Apple added a way to get scancodes from low level keyboard events.  Naturally they aren't the same as the Windows ones
//Proton uses, so let's convert them to normal ascii codes and proton virtual keys

- (long) HIDKeyToProtonKey: (long) keyCode type: (long) type
{
    if (keyCode >= UIKeyboardHIDUsageKeyboardA && keyCode <= UIKeyboardHIDUsageKeyboardZ)
    {
        return ((long)'a')+(keyCode-UIKeyboardHIDUsageKeyboardA); // convert A-Z
    }
    
    if (keyCode >= UIKeyboardHIDUsageKeyboard1 && keyCode <= UIKeyboardHIDUsageKeyboard9)
    {
        return ((long)'1')+(keyCode-UIKeyboardHIDUsageKeyboard1); // cobvert 1-9
    }
    
    if (keyCode >= UIKeyboardHIDUsageKeyboardF1 && keyCode <= UIKeyboardHIDUsageKeyboardF12)
    {
        return  VIRTUAL_KEY_F1  +(keyCode-UIKeyboardHIDUsageKeyboardF1); // covert F1-F16
    }

    //get into the nitty gritty where tricks don't work
    switch(keyCode)
    {
        case UIKeyboardHIDUsageKeyboard0: return '0';
            
        case UIKeyboardHIDUsageKeyboardLeftControl:
        case UIKeyboardHIDUsageKeyboardRightControl: return VIRTUAL_KEY_CONTROL;
            
        case UIKeyboardHIDUsageKeyboardLeftShift:
        case UIKeyboardHIDUsageKeyboardRightShift: return VIRTUAL_KEY_SHIFT;

        case UIKeyboardHIDUsageKeyboardLeftAlt:
        case UIKeyboardHIDUsageKeyboardRightAlt: return VIRTUAL_KEY_ALT;

        case UIKeyboardHIDUsageKeyboardTab: return 9;
        case UIKeyboardHIDUsageKeyboardDeleteOrBackspace: return 8;
        case UIKeyboardHIDUsageKeyboardEscape: return VIRTUAL_KEY_BACK;
        case UIKeyboardHIDUsageKeyboardLeftArrow: return VIRTUAL_KEY_DIR_LEFT;
        case UIKeyboardHIDUsageKeyboardRightArrow: return VIRTUAL_KEY_DIR_RIGHT;
        case UIKeyboardHIDUsageKeyboardUpArrow: return VIRTUAL_KEY_DIR_UP;
        case UIKeyboardHIDUsageKeyboardDownArrow: return VIRTUAL_KEY_DIR_DOWN;
        case UIKeyboardHIDUsageKeyboardSpacebar:  return ' ';
        case UIKeyboardHIDUsageKeyboardGraveAccentAndTilde: return '`';
        case UIKeyboardHIDUsageKeyboardBackslash: return '\\';
        case UIKeyboardHIDUsageKeyboardReturnOrEnter: return 13;
        case UIKeyboardHIDUsageKeyboardComma: return ',';
        case UIKeyboardHIDUsageKeyboardPeriod: return '.';
        case UIKeyboardHIDUsageKeyboardSlash: return '/';
        case UIKeyboardHIDUsageKeyboardOpenBracket: return '[';
        case UIKeyboardHIDUsageKeyboardCloseBracket: return ']';
        default:;
    }
    
    return keyCode;
}

- (void) pressesBegan:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event
{
    
    for(UIPress *press in presses)
    {
        int protonChar = (int)[self HIDKeyToProtonKey: press.key.keyCode type: press.type];
        //LogMsg("Pressed %d, converted to %d (type: %d)", press.key.keyCode, protonChar, press.type);
        
        if (protonChar > 0)
        {
            //generate events
            GetMessageManager()->SendGUI(MESSAGE_TYPE_GUI_CHAR_RAW, (float)protonChar, 1.0f);
            
            //Um, I think we need to lower case this, then check if Shift is down for it to work right
            GetMessageManager()->SendGUI(MESSAGE_TYPE_GUI_CHAR, (float)protonChar, 1.0f);
        }
        
    }
}

- (void) pressesEnded:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event
{
    for(UIPress *press in presses)
    {
        int protonChar = (int)[self HIDKeyToProtonKey: press.key.keyCode type: press.type];
     
        if (protonChar > 0)
        {
            GetMessageManager()->SendGUI(MESSAGE_TYPE_GUI_CHAR_RAW, (float)protonChar, 0.0f);
        }
    }
}


- (NSUInteger)supportedInterfaceOrientations
{
    if (!GetBaseApp()->GetManualRotationMode())
    {
        if (GetLockedLandscape())
        {
			return UIInterfaceOrientationMaskLandscape;
        }
        
        return UIInterfaceOrientationMaskAll;
    }
    
    return 0;
}

-(void)didRotate:(NSNotification *)theNotification
{
	UIDeviceOrientation interfaceOrientation = [[UIDevice currentDevice] orientation];
	//NSLog(@"Got didRotate: %d", interfaceOrientation);
	
    CGFloat pixelScale = [[UIScreen mainScreen] scale];
    //LogMsg("Learned Scale: %.2f", pixelScale);
    SetProtonPixelScaleFactor(pixelScale);
    
    if (!UIDeviceOrientationIsValidInterfaceOrientation(interfaceOrientation))
	{
		//it's probably like a face up/down message.  Ignore it
		return;
	}

    //when in manual rotation mode, this gets hit before a valid SetProtonPixelScaleFactor is set, so we need to set it here
    //to be safe
     
	UIScreen *pScreen = [UIScreen mainScreen];
//    CGRect fullScreenRect = pScreen.bounds;
    CGRect fullScreenRect = iOS7StyleScreenBounds();

    LogMsg("Rotated to orientation %d (%.2f, %.2f)", interfaceOrientation, fullScreenRect.size.width* GetProtonPixelScaleFactor(), fullScreenRect.size.height* GetProtonPixelScaleFactor());
    
    if (GetBaseApp()->GetManualRotationMode())
    {
        SetPrimaryScreenSize(fullScreenRect.size.width* GetProtonPixelScaleFactor(), fullScreenRect.size.height* GetProtonPixelScaleFactor());
        
        LogMsg("Forcing status bar orientation to interfaceOrientation %d", interfaceOrientation);
        [[UIApplication sharedApplication] setStatusBarOrientation: (UIInterfaceOrientation) interfaceOrientation animated:NO];
        
        if (GetLockedLandscape())
        {
            if (interfaceOrientation == UIInterfaceOrientationPortrait)
            {
                interfaceOrientation = (UIDeviceOrientation) UIInterfaceOrientationLandscapeLeft;
            }
            if (interfaceOrientation == UIInterfaceOrientationPortraitUpsideDown)
            {
                interfaceOrientation = (UIDeviceOrientation) UIInterfaceOrientationLandscapeRight;
            }
        }
     
        
        if (interfaceOrientation == UIInterfaceOrientationLandscapeRight ||
            interfaceOrientation == UIInterfaceOrientationLandscapeLeft)
        {
            SetupScreenInfo( GetPrimaryGLY(), GetPrimaryGLX(), interfaceOrientation);
        } else
        {
            SetupScreenInfo( GetPrimaryGLX(), GetPrimaryGLY(), interfaceOrientation);
        }
        
        return;
	}
    
	if (!CanRotateTo(interfaceOrientation))
	{
		LogMsg("Refusing orientation %d",interfaceOrientation);
		return;
	}
    
    if (!GetBaseApp()->GetManualRotationMode())
    {
    
       if (interfaceOrientation == UIInterfaceOrientationLandscapeRight ||
           interfaceOrientation == UIInterfaceOrientationLandscapeLeft)
       {
           //must be reversed
           SetPrimaryScreenSize(fullScreenRect.size.height * GetProtonPixelScaleFactor(), fullScreenRect.size.width* GetProtonPixelScaleFactor());
            
       } else
       {
           SetPrimaryScreenSize(fullScreenRect.size.width* GetProtonPixelScaleFactor(), fullScreenRect.size.height* GetProtonPixelScaleFactor());
       }
    }
   
    
    
    SetupScreenInfo( GetPrimaryGLX(), GetPrimaryGLY(), interfaceOrientation);
	
}

- (BOOL) prefersStatusBarHidden
{
    return YES;
}
 
- (void)didRotateFromInterfaceOrientation:(UIInterfaceOrientation)fromInterfaceOrientation
{
	LogMsg("DidRotateFromInterface: Got %d, App orientation is %d", fromInterfaceOrientation, GetOrientation());
	return;
}

- (void)didReceiveMemoryWarning 
{
    [super didReceiveMemoryWarning]; // Releases the view if it doesn't have a superview
    GetBaseApp()->OnMemoryWarning();
    
    // Release anything that's not essential, such as cached data
}

- (void)dealloc {
  [super dealloc];
}


@end
