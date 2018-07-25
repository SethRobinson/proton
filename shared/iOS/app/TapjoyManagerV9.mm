
#ifdef RT_TAPJOY_ENABLED

#import "TapjoyManagerV9.h"
#import <UIKit/UIKit.h>
#import "MyAppDelegate.h"
#import "EAGLView.h"
#import <Tapjoy/TJPlacement.h>

@interface TapjoyManager () <TJPlacementDelegate,TJCVideoAdDelegate>
@property (nonatomic, strong) TJDirectPlayPlacementDelegate *dpDelegate;
@property (strong, nonatomic) TJPlacement *m_addPlacement;

@end

@implementation TapjoyManager

- (void) InitTapjoy: (UIApplication *)application viewController: (MyViewController*) viewController
{
	LogMsg("Initting tapjoy");

	//Register tapjoy callbacks
    m_viewController = viewController;
    
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(tjcConnectSuccess:) name:TJC_CONNECT_SUCCESS object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(tjcConnectFail:) name:TJC_CONNECT_FAILED object:nil];

	//[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(getFeaturedApp:) name:TJC_FEATURED_APP_RESPONSE_NOTIFICATION object:nil];

    //apparently the above doesn't exist in SDK 10
    
    /*
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(getUpdatedPoints:) name:TJC_TAP_POINTS_RESPONSE_NOTIFICATION object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(getSpendPoints:) name:TJC_SPEND_TAP_POINTS_RESPONSE_NOTIFICATION object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(getAwardPoints:) name:TJC_AWARD_TAP_POINTS_RESPONSE_NOTIFICATION object:nil];

      
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(getUpdatedPointsError:) name:TJC_TAP_POINTS_RESPONSE_NOTIFICATION_ERROR object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(getSpendPointsError:) name:TJC_SPEND_TAP_POINTS_RESPONSE_NOTIFICATION_ERROR object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(getAwardPointsError:) name:TJC_AWARD_TAP_POINTS_RESPONSE_NOTIFICATION_ERROR object:nil];

    
    
    // Add an observer for when a user has successfully earned currency.
	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(showEarnedCurrencyAlert:)
												 name:TJC_TAPPOINTS_EARNED_NOTIFICATION
											   object:nil];

	//Listen for when they close the offerwall
	[[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(offerwallClosed:)
                                                 name:TJC_VIEW_CLOSED_NOTIFICATION
                                               object:nil];

    
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(showFullscreenAd:) name:TJC_FULL_SCREEN_AD_RESPONSE_NOTIFICATION object:nil];

     */

	//virtual items hosted by tapjoy not yet not supported
	/*
	// Watch for virtual good notification indicating that items are ready to go.
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(virtualGoodsUpdated:) name:TJC_PURCHASED_ITEMS_RESPONSE_NOTIFICATION object:nil];
	*/

	//TODO: ?
	// If you are not using Tapjoy Managed currency, you would set your own user ID here.
	//[TapjoyConnect setUserID:@"A_UNIQUE_USER_ID"];
   
    _dpDelegate = [[TJDirectPlayPlacementDelegate alloc] init];
    _dpDelegate.tjManager = self;

    
}


- (Boolean) onOSMessage:(OSMessage *)pMsg
{
	switch (pMsg->m_type)
	{
	
	case OSMessage::MESSAGE_TAPJOY_INIT_MAIN:
        {
	#ifdef _DEBUG
        LogMsg("Logging on to tapjoy: %s, %s", pMsg->m_string.c_str(), pMsg->m_string2.c_str());
    #endif
        NSString* appID = [NSString stringWithUTF8String:pMsg->m_string.c_str()];
        NSString* secretKey = [NSString stringWithUTF8String:pMsg->m_string2.c_str()];
            
           //[Tapjoy setDebugEnabled: true];
            
         #ifdef _DEBUG
             [Tapjoy setDebugEnabled: true];
        #endif
            [Tapjoy connect:appID];
            
         
		return true; //we handled it
        }
	break;

	case OSMessage::MESSAGE_TAPJOY_SET_USERID:
	{
	  NSString* userID = [NSString stringWithUTF8String:pMsg->m_string.c_str()];
      [Tapjoy setUserID: userID];

        LogMsg("Setting up TJ");
        
        _m_addPlacement = [TJPlacement placementWithName:@"WatchAd" delegate:_dpDelegate];
        
        [_m_addPlacement requestContent];

        m_offerWallPlacement = [TJPlacement placementWithName:@"Earn Gems" delegate:self];
        [m_offerWallPlacement requestContent];

        
	  return true; //we handled it
	}
	break;
	
	case OSMessage::MESSAGE_TAPJOY_GET_AD: 

        LogMsg("Getting tapjoy ad");
        //TODO: Set banner size?
        
       // [Tapjoy getDisplayAdWithDelegate:self];
		return true; //we handled it
	break;
	
	case OSMessage::MESSAGE_TAPJOY_GET_FEATURED_APP:
        {
		//[TapjoyConnect getFeaturedApp];

            //note: ignoring currency ID, placement name must be setup in the Tapjoy V11 website portal for this work
            
            if (_m_addPlacement)
            {
                if(_m_addPlacement.isContentAvailable)
                {
                if (_m_addPlacement.isContentReady)
                {
                    LogMsg("Showing ad");
                [_m_addPlacement showContentWithViewController:m_viewController];
                } else
                {
                    LogMsg("No content to present");
                    GetMessageManager()->SendGUIEx( MESSAGE_TYPE_TAPJOY_NO_CONTENT_TO_PRESENT, 1,0,0);
                    UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"No video ads to display"
                                                                    message:@"Please try again later, there are no video ads to show you right now."
                                                                   delegate:nil
                                                          cancelButtonTitle:@"OK"
                                                          otherButtonTitles:nil];
                    [alert show];
                    [alert release];

                }
                } else
                {
                    
                    LogMsg("No content available");
                    GetMessageManager()->SendGUIEx( MESSAGE_TYPE_TAPJOY_NO_CONTENT_AVAILABLE, 1,0,0);

                    UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"No video ads to display"
                                                                   message:@"Please try again later, there are no video ads to show you right now."
                                                                  delegate:nil
                                                         cancelButtonTitle:@"OK"
                                                         otherButtonTitles:nil];
                    [alert show];
                    [alert release];
                }
                
            } else
            {
                LogMsg("Not ready to show another ad");
            
            }
            return true; //we handled it
        }
	break;

	case OSMessage::MESSAGE_TAPJOY_SHOW_FEATURED_APP:
		LogMsg("show tapjoy feature");
		//[TapjoyConnect showFeaturedAppFullScreenAd];
	//	[Tapjoy showFullScreenAd];
            return true; //we handled it
	break;
	
	case OSMessage::MESSAGE_TAPJOY_GET_TAP_POINTS:
		LogMsg("Getting tapjoy points");
	//	[Tapjoy getTapPoints];
		return true; //we handled it
	break;

	case OSMessage::MESSAGE_TAPJOY_SPEND_TAP_POINTS:
		//LogMsg("Spending tappoints: %d" , pMsg->m_parm1);
		[Tapjoy spendCurrency:pMsg->m_parm1];
		return true; //we handled it
	break;	
	
	case OSMessage::MESSAGE_TAPJOY_AWARD_TAP_POINTS:
		// This method call will award 10 virtual currencies to the users total
		[Tapjoy awardCurrency:pMsg->m_parm1];
		return true; //we handled it
	break;

	case OSMessage::MESSAGE_TAPJOY_SHOW_OFFERS:
		{
            
            if(m_offerWallPlacement.isContentReady)
            {
                [m_offerWallPlacement showContentWithViewController: m_viewController];
            }
            else
            {
                //handle situation where there is no content to show, or it has not yet downloaded.
                 LogMsg("No offer wall content to present");
                GetMessageManager()->SendGUIEx( MESSAGE_TYPE_TAPJOY_NO_CONTENT_TO_PRESENT, 1,0,0);
                UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"No offers to display"
                                                                message:@"Please try again later, there are no Tapjoy offers to show you right now."
                                                               delegate:nil
                                                      cancelButtonTitle:@"OK"
                                                      otherButtonTitles:nil];
                [alert show];
                [alert release];

                
                }
       }
            return true; //we handled it
	
		break;
		
	case OSMessage::MESSAGE_TAPJOY_SHOW_AD:
        {
		
		int showAd = (int)pMsg->m_x;
	
		return true; //we handled it
        }
	break;

	case OSMessage::MESSAGE_REQUEST_AD_SIZE:
		m_adBannerWidth = (int)pMsg->m_x;
		m_adBannerWidth = (int)pMsg->m_y;
	
		//app.adBannerWidth = 480;
		//app.adBannerHeight = 72;
		//app.tapBannerSize = app.adBannerWidth+"x"+app.adBannerHeight;		
		LogMsg("Setting tapjoy banner size to %d x %d.. TODO: Not really handled yet!", m_adBannerWidth, m_adBannerWidth);
		return true; //we handled it
	break;
	
	default:
	
	return false; //not handled
	}
	
	return false; //not handled
}


//************* callback handlers for tapjoy

- (void)contentDidDisappear:(TJPlacement *)placement
{
    
    NSLog(@"contentDidDisappear Getting data");
    [m_offerWallPlacement requestContent];
  
}

- (void) requestDidFail:(TJPlacement *)placement error:(NSError *)error
{
    NSLog(@"Tapjoy request failed: %@", [error localizedDescription]);
}

- (void)getFeaturedApp:(NSNotification*)notifyObj
{
	LogMsg("Got tapjoy featured app");
	GetMessageManager()->SendGUIEx(MESSAGE_TYPE_TAPJOY_FEATURED_APP_READY, 1,0,0);

}

// This method is called after Ad data has been successfully received from the server. 
- (void)didReceiveAd:(TJCAdView*)adView
{
		LogMsg("Got ad");
		GetMessageManager()->SendGUIEx(MESSAGE_TYPE_TAPJOY_AD_READY, (int)1,0,0);
    
   
}

// This method is called if an error has occurred while requesting Ad data.
- (void)didFailWithMessage:(NSString*)msg
{
	LogMsg("Error getting tapjoy ad");
	GetMessageManager()->SendGUIEx(MESSAGE_TYPE_TAPJOY_AD_READY, 0,0,0);
	
}

// This method must return one of TJC_AD_BANNERSIZE_320X50, TJC_AD_BANNERSIZE_640X100, or TJC_AD_BANNERSIZE_768X90. 
- (NSString*)adContentSize
{
	return 0;
//    return TJC_AD_BANNERSIZE_640X100; Uh, this is no longer part of the SDK..
}

- (void)showFullscreenAd:(NSNotification*)notifyObj
{
    LogMsg("Got fullscreen ad, showing (outdated version)");
}

// This method must return a boolean indicating whether the Ad will automatically refresh itself.

- (BOOL)shouldRefreshAd
{
	return NO;
}

- (void)showFullscreenAdError:(NSNotification*)notifyObj
{
    NSLog(@"There is no Ad available!");
}

- (void)getUpdatedPoints:(NSNotification*)notifyObj
{
	NSNumber *tapPoints = notifyObj.object;
	NSString *tapPointsStr = [NSString stringWithFormat:@"Tap Points: %d", [tapPoints intValue]];
	// Print out the updated points value.
	NSLog(@"%@", tapPointsStr);
	GetMessageManager()->SendGUIStringEx(MESSAGE_TYPE_TAPJOY_TAP_POINTS_RETURN, [tapPoints intValue],0,0, "");

}

- (void)getSpendPoints:(NSNotification*)notifyObj
{
	NSNumber *tapPoints = notifyObj.object;
	NSString *tapPointsStr = [NSString stringWithFormat:@"Tap Points: %d", [tapPoints intValue]];
	// Print out the updated points value.
	NSLog(@"%@", tapPointsStr);
	GetMessageManager()->SendGUIStringEx(MESSAGE_TYPE_TAPJOY_SPEND_TAP_POINTS_RETURN, [tapPoints intValue],0,0, "");
}

- (void)getAwardPoints:(NSNotification*)notifyObj
{
	NSNumber *tapPoints = notifyObj.object;
	NSString *tapPointsStr = [NSString stringWithFormat:@"Tap Points: %d", [tapPoints intValue]];
	// Print out the updated points value.
	NSLog(@"%@", tapPointsStr);
	GetMessageManager()->SendGUIStringEx(MESSAGE_TYPE_TAPJOY_AWARD_TAP_POINTS_RETURN, [tapPoints intValue],0,0, "");
}

- (void)getAwardPointsError:(NSNotification*)notifyObj
{
	LogMsg("getAwardPointsError");
	GetMessageManager()->SendGUIStringEx(MESSAGE_TYPE_TAPJOY_AWARD_TAP_POINTS_RETURN_ERROR, 0,0,0, "");
}

- (void)getUpdatedPointsError:(NSNotification*)notifyObj
{
	LogMsg("GetUpdatedPointsError");
	GetMessageManager()->SendGUIStringEx(MESSAGE_TYPE_TAPJOY_TAP_POINTS_RETURN_ERROR, 0,0,0, "");
}

- (void)getSpendPointsError:(NSNotification*)notifyObj
{
	LogMsg("GetUpdatedPointsError");
	GetMessageManager()->SendGUIStringEx(MESSAGE_TYPE_TAPJOY_SPEND_TAP_POINTS_RETURN_ERROR, 0,0,0, "");
}

-(void)tjcConnectSuccess:(NSNotification*)notifyObj
{
	NSLog(@"Tapjoy connect Succeeded.  Getting data");
  
}

-(void)offerwallClosed:(NSNotification*)notifyObj
{
	NSLog(@"Offerwall closed");
    GetMessageManager()->SendGUIStringEx(MESSAGE_TYPE_TAPJOY_OFFERWALL_CLOSED, 0,0,0, "");

}
             
             
- (void)showEarnedCurrencyAlert:(NSNotification*)notifyObj
{
    NSNumber *tapPointsEarned = notifyObj.object;
   int earnedNum = [tapPointsEarned intValue];
   NSLog(@"Currency earned: %d", earnedNum);
    
    
    //Well, in my games Proton handles the alert.  Maybe some people want Tapjoy to do it though?
    
   // Pops up a UIAlert notifying the user that they have successfully earned some currency.
   // This is the default alert, so you may place a custom alert here if you choose to do so.
    //   [TapjoyConnect showDefaultEarnedCurrencyAlert];

    // This is a good place to remove this notification since it is undesirable to have a pop-up alert during gameplay.
    //[NSNotificationCenter defaultCenter] removeObserver:self name:TJC_TAPPOINTS_EARNED_NOTIFICATION object:nil];
    
    GetMessageManager()->SendGUIStringEx(MESSAGE_TYPE_TAPJOY_EARNED_TAP_POINTS, earnedNum,0,0, "");
}



- (void)requestDidSucceed:(TJPlacement*)placement
{
    NSLog(@"Tapjoy request did succeed, contentIsAvailable:%d", placement.isContentAvailable);
}

- (void)contentIsReady:(TJPlacement*)placement
{
    NSLog(@"Tapjoy placement content is ready to display");
}



-(void)tjcConnectFail:(NSNotification*)notifyObj
{
	NSLog(@"Tapjoy connect Failed");	
}


- (void)contentDidAppear:(TJPlacement*)placement
{
    NSLog(@"Content did appear for %@ placement", [placement placementName]);
}



@end


@interface TJDirectPlayPlacementDelegate ()

@end

@implementation TJDirectPlayPlacementDelegate
-(id)init
{
    self = [super init];
    
    if (self)
    {}
    
    return self;
}

- (void)requestDidSucceed:(TJPlacement*)placement
{
    NSLog(@"Tapjoy request did succeed, contentIsAvailable:%d", placement.isContentAvailable);
}

- (void)contentIsReady:(TJPlacement*)placement
{
    NSLog(@"Tapjoy placement content is ready to display");
}

- (void)requestDidFail:(TJPlacement*)placement error:(NSError *)error
{
    NSLog(@"Tapjoy request failed with error: %@", [error localizedDescription]);
}

- (void)contentDidAppear:(TJPlacement*)placement
{
    NSLog(@"Content did appear for %@ placement", [placement placementName]);
}

- (void)contentDidDisappear:(TJPlacement*)placement
{
   [_tjManager.m_addPlacement requestContent];
    NSLog(@"Content did disappear for %@ placement", [placement placementName]);
}
@end

#endif