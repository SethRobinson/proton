//
//  TapjoyManager.h
//
//  Created by Seth Robinson on 8/24/12.
//  Copyright (c) 2012 Robinson Technologies. All rights reserved.
//

//This is stupidly named, it's actually for TJ V11

#ifdef RT_TAPJOY_ENABLED

#import <UIKit/UIKit.h>
#import <Tapjoy/Tapjoy.h>
//#import <Foundation/Foundation.h>
#import <Tapjoy/TJPlacement.h>
#import "MyViewController.h"



@interface TapjoyManager : NSObject <TJPlacementDelegate,TJCVideoAdDelegate>
{
    MyViewController *m_viewController;
   // TJCAdView* m_adView;
    UIWindow *window;
    TJPlacement *m_offerWallPlacement;
	UINavigationController *navCtrl_;

	int m_adBannerWidth;
	int m_adBannerHeight;
     } 

- (void) InitTapjoy: (UIApplication *)application viewController: (MyViewController*) viewController;
- (Boolean) onOSMessage:(OSMessage *)pMsg;


- (void)getFeaturedApp:(NSNotification*)notifyObj;
//- (void)didReceiveAd:(TJCAdView*)adView;
- (void)didFailWithMessage:(NSString*)msg;
- (void)didFailWithMessage:(NSString*)msg;
- (NSString*)adContentSize;
- (BOOL)shouldRefreshAd;
- (void)getUpdatedPoints:(NSNotification*)notifyObj;
- (void)getSpendPoints:(NSNotification*)notifyObj;
- (void)getAwardPoints:(NSNotification*)notifyObj;

@end

@interface TJDirectPlayPlacementDelegate : NSObject<TJPlacementDelegate>
@property (nonatomic, strong) TapjoyManager *tjManager;
@end

#endif
