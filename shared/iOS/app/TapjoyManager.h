//
//  TapjoyManager.h
//
//  Created by Seth Robinson on 8/24/12.
//  Copyright (c) 2012 Robinson Technologies. All rights reserved.
//

#ifdef RT_TAPJOY_ENABLED

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "TapjoyConnect.h"
#import "MyViewController.h"

@interface TapjoyManager : NSObject <TJCAdDelegate>
{
    MyViewController *m_viewController;
    TJCAdView* m_adView;
    UIWindow *window;
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

#endif
