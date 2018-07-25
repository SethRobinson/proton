//
//  InAppPurchaseManager.h
//  RTAdTest
//
//  Created by Seth Robinson on 3/15/12.
//  Copyright (c) 2012 Robinson Technologies. All rights reserved.
//

//Based on the tutorial from http://troybrant.net/blog/2010/01/in-app-purchases-a-full-walkthrough/

#import <Foundation/Foundation.h>
#import <StoreKit/StoreKit.h>

#define kInAppPurchaseManagerTransactionFailedNotification @"kInAppPurchaseManagerTransactionFailedNotification"
#define kInAppPurchaseManagerTransactionSucceededNotification @"kInAppPurchaseManagerTransactionSucceededNotification"
#define kInAppPurchaseManagerProductsFetchedNotification @"kInAppPurchaseManagerProductsFetchedNotification"

@interface InAppPurchaseManager : NSObject <SKProductsRequestDelegate, SKPaymentTransactionObserver>
{
    SKProduct *proUpgradeProduct;
    SKProductsRequest *productsRequest;
} 

- (void) BuyItemByID:(string)itemID;
- (void) InitIAP; //call once after initting
- (void) GetPurchasedList;  // Gets a list of previously purchased items
- (void) GetProductData: (string)itemID; //Not finished, don't use
- (void) GetProductDataForTracking: (string)itemID; //Not finished, don't use
@end
