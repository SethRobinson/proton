
#ifdef RT_TAPJOY_ENABLED

#import "TapjoyManager.h"
#import <UIKit/UIKit.h>
#import "MyAppDelegate.h"
#import "EAGLView.h"

@implementation TapjoyManager

- (void) InitTapjoy: (UIApplication *)application viewController: (MyViewController*) viewController
{
	LogMsg("Initting tapjoy");

	//Register tapjoy callbacks
#ifdef _DEBUG
    [TJCLog setLogThreshold:LOG_DEBUG];
#endif
    m_viewController = viewController;
    
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(tjcConnectSuccess:) name:TJC_CONNECT_SUCCESS object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(tjcConnectFail:) name:TJC_CONNECT_FAILED object:nil];

	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(getFeaturedApp:) name:TJC_FEATURED_APP_RESPONSE_NOTIFICATION object:nil];

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
    
    
 
 
	//virtual items hosted by tapjoy not yet not supported
	/*
	// Watch for virtual good notification indicating that items are ready to go.
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(virtualGoodsUpdated:) name:TJC_PURCHASED_ITEMS_RESPONSE_NOTIFICATION object:nil];
	*/

	//TODO: ?
	// If you are not using Tapjoy Managed currency, you would set your own user ID here.
	//[TapjoyConnect setUserID:@"A_UNIQUE_USER_ID"];
   
    
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
            
		[TapjoyConnect requestTapjoyConnect:appID secretKey:secretKey];
	
		// This will set the display count to infinity effectively always showing the featured app.
		[TapjoyConnect setFeaturedAppDisplayCount:TJC_FEATURED_COUNT_INF];

		return true; //we handled it
        }
	break;

	case OSMessage::MESSAGE_TAPJOY_GET_AD: 

        LogMsg("Getting tapjoy ad");
        //TODO: Set banner size?
        
        [TapjoyConnect getDisplayAdWithDelegate:self];
		return true; //we handled it
	break;
	
	case OSMessage::MESSAGE_TAPJOY_GET_FEATURED_APP:
		LogMsg("Getting tapjoy feature");
				
		[TapjoyConnect getFeaturedApp];
		return true; //we handled it
	break;


	case OSMessage::MESSAGE_TAPJOY_SHOW_FEATURED_APP:
		LogMsg("show tapjoy feature");
		[TapjoyConnect showFeaturedAppFullScreenAd]; 
		return true; //we handled it
	break;
	
	case OSMessage::MESSAGE_TAPJOY_GET_TAP_POINTS:
		LogMsg("Getting tapjoy points");
		[TapjoyConnect getTapPoints];
		return true; //we handled it
	break;

	case OSMessage::MESSAGE_TAPJOY_SPEND_TAP_POINTS:
		LogMsg("Spending tappoints: " + pMsg->m_parm1);
		[TapjoyConnect spendTapPoints:pMsg->m_parm1];
		return true; //we handled it
	break;	
	
	case OSMessage::MESSAGE_TAPJOY_AWARD_TAP_POINTS:
		// This method call will award 10 virtual currencies to the users total
		[TapjoyConnect awardTapPoints:pMsg->m_parm1];
		return true; //we handled it
	break;

	case OSMessage::MESSAGE_TAPJOY_SHOW_OFFERS:
		// This method returns the offers view that you can add it in your required view. Initialize its bounds depending on the main window bounds.
		[TapjoyConnect showOffers];

		// This method takes a view controller and sets its bounds according to viewController's and depend on the orientation of the view controller sent as an argument
		//[TapjoyConnect showOffersWithViewController:vController];

		// OR

		// This method adds the offers view without the library's Navigation Bar, so you can add this view into your view which already has a navigation bar
		//[TapjoyConnect showOffersWithViewController:vController withInternalNavBar:NO];			break;
		return true; //we handled it
	
		break;
		
	case OSMessage::MESSAGE_TAPJOY_SHOW_AD:
        {
		
		int showAd = (int)pMsg->m_x;
		LogMsg("Showing tapjoy ad, parm is: %d" + showAd);
		
        if (m_adView)
        {
            if (showAd)
            {
                m_adView.hidden = NO;
            } else
            {
                m_adView.hidden = YES;
            }
            
        } else
        {
           LogMsg("Can't set status of ad, its view doesn't exist yet");
                
        }
		//[TapjoyConnect getDisplayAdWithDelegate:self];
		
		//TapjoyConnect.getTapjoyConnectInstance().enableBannerAdAutoRefresh(app.tapjoy_ad_show != 0);

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


- (void)getFeaturedApp:(NSNotification*)notifyObj
{
	LogMsg("Got tapjoy featured app");
    // Displays a full screen ad, showing the current featured app. 
    //[TapjoyConnect showFeaturedAppFullScreenAd]; 
    // OR 
    // This is used when you want to add the full screen ad to an existing view controller. 
    //[TapjoyConnect showFeaturedAppFullScreenAdWithViewController:self];
	GetMessageManager()->SendGUIEx(MESSAGE_TYPE_TAPJOY_FEATURED_APP_READY, 1,0,0);

}

// This method is called after Ad data has been successfully received from the server. 
- (void)didReceiveAd:(TJCAdView*)adView
{
		LogMsg("Got ad");
		GetMessageManager()->SendGUIEx(MESSAGE_TYPE_TAPJOY_AD_READY, (int)1,0,0);
    
    if (!m_adView)
    {
        //setup our view controller, we'll keep the same one for the rest of the app
        m_adView = adView;
        [m_viewController.view addSubview:adView];
   
        
//        transform = CGAffineTransformMakeRotation((float)(M_PI / 2));

        //and move it
        
        //adView.transform = m_viewController.view.transform;
       // transform = CGAffineTransformRotate(transform, (float)(-M_PI / 2));
     
      //  transform = CGAffineTransformTranslate(transform, - (GetScreenSizeXf()-GetScreenSizeY()),0);
        
        //CGFloat x = adView.bounds.size.width / 2.0f;
        //CGFloat y = adView.bounds.size.height / 2.0f;
       // CGPoint center = CGPointMake(x,y);
        
      //  m_adView.transform = transform;

        
        m_adView.hidden = YES;
        
    }
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
	return TJC_AD_BANNERSIZE_640X100;
}

// This method must return a boolean indicating whether the Ad will automatically refresh itself.
- (BOOL)shouldRefreshAd
{
	return NO;
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
	NSLog(@"Tapjoy connect Succeeded");
}


-(void)tjcConnectFail:(NSNotification*)notifyObj
{
	NSLog(@"Tapjoy connect Failed");	
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

/*


- (void)virtualGoodsUpdated:(NSNotification*)notifyObj
{
	// Example on how to access the downloaded virtual goods.
	StoreItem * strItem;
	
	NSMutableArray* vgStoreItemArray = [TapjoyConnect getPurchasedVGStoreItems];
	
	for (int i = 0; i < [vgStoreItemArray count]; ++i) 
	{
		NSLog(@"VG Item %d Start================================================================================================", i);
		strItem = [vgStoreItemArray objectAtIndex:i];
		
		NSLog(@"Item Name:					%@", [strItem title]);
		NSLog(@"Item Type:					%@", [strItem storeItemType]);
		NSLog(@"Item Description:			%@", [strItem description]);
		NSLog(@"Item Price:					%d", [strItem price]);
		NSLog(@"Item Currency Name:			%@", [strItem currencyName]);
		NSLog(@"Item Data File Location:	%@", [strItem datafileLocation]);
		
		// Print out contents of data file.
		if(![[strItem datafileLocation] isEqualToString:@""])
		{
			NSError *error;
			NSArray *contentsArray = [[NSFileManager defaultManager] contentsOfDirectoryAtPath:[strItem datafileLocation] error:&error];
			for (int j = 0; j < [contentsArray count]; ++j) 
			{
				NSLog(@"     %d Data File Contents: %@", j, [contentsArray objectAtIndex:j]);
			}
		}
		
		NSMutableDictionary *dict = [strItem attributeValues];
		
		for (id key in dict)
		{
			id value = [dict objectForKey:key];
			NSLog(@"     Attribute:%@ Value:%@", key, value);
		}
		NSLog(@"VG Item %d End==================================================================================================", i);
	}
	
	[vgStoreItemArray release];
}

*/

@end

#endif