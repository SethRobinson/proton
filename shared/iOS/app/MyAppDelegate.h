//
//  MyAppDelegate.h
//  Created by Seth Robinson on 3/6/09.
//  For license info, check the license.txt file that should have come with this.
//

#import <UIKit/UIKit.h>
#import "CFNetwork/CFSocketStream.h"

#ifdef RT_TAPJOY_ENABLED
@class TapjoyManager;
#endif

@class InAppPurchaseManager;
@class EAGLView;

@interface MyAppDelegate : NSObject <UIApplicationDelegate, UITextFieldDelegate, UIAccelerometerDelegate>
{
	IBOutlet UIWindow *window;
	IBOutlet EAGLView *glView;
	IBOutlet UIViewController *viewController;
	UITextField*			_textField;
	int m_inputboxMaxLen;

	CFHostRef m_HostRef;
	CFReadStreamRef m_ReadRef;
	CFWriteStreamRef m_WriteRef;
	CFStreamClientContext context;
	UIKeyboardType m_requestedKeyboardType; //what we'll use on the iOS side
    int m_keyboardType; //what proton actually requested, so we know what items to filter
    InAppPurchaseManager *m_IOSIAPManager;

#ifdef RT_TAPJOY_ENABLED
    TapjoyManager *m_tapjoyManager;
#endif
}

@property (nonatomic, retain) UIWindow *window;
@property (nonatomic, retain) EAGLView *glView;
@property (nonatomic, retain) UIViewController *viewController;

- (void) KillTextBox;
- (void) onOSMessage:(OSMessage *)pMsg;
- (void) ActivateNetworkConnection;
- (void)accelerometer:(UIAccelerometer*)accelerometer didAccelerate:(UIAcceleration*)acceleration;

@end

void static MyWriteStreamCallback(CFWriteStreamRef streamRef,
								  CFStreamEventType eventType,
								  void *clientCallBackInfo);

NSMutableDictionary* stringToDict (NSString* str);
