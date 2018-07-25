//
//  MyViewController.m
//  Created by Seth Robinson on 3/6/09.
//  For license info, check the license.txt file that should have come with this.
//


#import "MyViewController.h"
#import "EAGLView.h"
#import "iOSUtils.h"

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
