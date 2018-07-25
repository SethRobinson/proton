//
//  InAppPurchaseManager.m
//  RTAdTest
//
//  Created by Seth Robinson on 3/15/12.
//  Copyright (c) 2012 Robinson Technologies. All rights reserved.
//

#import "InAppPurchaseManager.h"
#include "Manager/IAPManager.h"

#ifdef RT_APPSFLYER_ENABLED
#import <AppsFlyerLib/AppsFlyerTracker.h>
#endif

@implementation InAppPurchaseManager

//we'd only need this stuff to scan the store and get names and prices... Not implemented yet!
- (void)GetProductData: (string)itemID
{
    NSString *str =  [NSString stringWithCString: itemID.c_str() encoding: [NSString defaultCStringEncoding]];
    
    LogMsg("Requesting product data for %s", itemID.c_str());
    NSSet *productIdentifiers   = [NSSet setWithObject:str ];
    productsRequest             = [[SKProductsRequest alloc] initWithProductIdentifiers:productIdentifiers];
    productsRequest.delegate    = self;
    [productsRequest start];
    
    // we will release the request object in the delegate callback
}

//we'd only need this stuff to scan the store and get names and prices... Not implemented yet!
- (void)GetProductDataForTracking: (string)itemID
{
    NSString *str =  [NSString stringWithCString: itemID.c_str() encoding: [NSString defaultCStringEncoding]];
    
    LogMsg("Requesting product data for %s", itemID.c_str());
    NSSet *productIdentifiers   = [NSSet setWithObject:str ];
    productsRequest             = [[SKProductsRequest alloc] initWithProductIdentifiers:productIdentifiers];
    productsRequest.delegate    = self;
    [productsRequest start];
    
    // we will release the request object in the delegate callback
}

- (void)GetPurchasedList
{
    [[SKPaymentQueue defaultQueue] restoreCompletedTransactions];
}

- (void)request:(SKRequest *)request didFailWithError:(NSError *)error
{
    LogMsg("From iOS> We have a error in the IAP request");
}


- (void)productsRequest:(SKProductsRequest *)request didReceiveResponse:(SKProductsResponse *)response
{
    NSArray *products = response.products;
    proUpgradeProduct = [products count] == 1 ? [[products objectAtIndex:0] retain] : nil;
    if (proUpgradeProduct)
    {
        string itemInfo           = std::string([
                                                 [
                                                  NSString stringWithFormat:@"%@,%@,%@",
                                                  proUpgradeProduct.productIdentifier,
                                                  [proUpgradeProduct.priceLocale objectForKey:NSLocaleCurrencyCode],
                                                  proUpgradeProduct.price
                                                  ]
                                                 UTF8String
                                                 ]);
        
        NSLog(@"Product title: %@" ,        proUpgradeProduct.localizedTitle);
        NSLog(@"Product description: %@" ,  proUpgradeProduct.localizedDescription);
        NSLog(@"Product price: %@" ,        proUpgradeProduct.price);
        NSLog(@"Product id: %@" ,           proUpgradeProduct.productIdentifier);
        NSLog(@"Product currency: %s" ,     [[proUpgradeProduct.priceLocale objectForKey:NSLocaleCurrencyCode] UTF8String]);
        
        // Send the message to  C++
        GetMessageManager()->SendGUIStringEx(MESSAGE_TYPE_IAP_ITEM_INFO_RESULT,(float)IAPManager::RESULT_OK,0.0f,0,itemInfo.c_str());
    }
    
    for (NSString *invalidProductId in response.invalidProductIdentifiers)
    {
        NSLog(@"Invalid product id: %@" , invalidProductId);
    }
    
    [[NSNotificationCenter defaultCenter] postNotificationName:kInAppPurchaseManagerProductsFetchedNotification object:self userInfo:nil];
}


//
// call this method once on startup
//
- (void)InitIAP
{
    // restarts any purchases if they were interrupted last time the app was open
    [[SKPaymentQueue defaultQueue] addTransactionObserver:self];
}

//
// call this before making a purchase
//
- (BOOL)canMakePurchases
{
    return [SKPaymentQueue canMakePayments];
}

//
// kick off the upgrade transaction
//
- (void) BuyItemByID:(string)itemID
{
    
    if ([self canMakePurchases] == false)
    {
        
        GetMessageManager()->SendGUIStringEx(MESSAGE_TYPE_IAP_RESULT,(float)IAPManager::RESULT_SERVICE_UNAVAILABLE,0.0f,0, "Service unavailable");
        return;
    }
    NSString *str =  [NSString stringWithCString: itemID.c_str() encoding: [NSString defaultCStringEncoding]];
    
    SKPayment *payment = [SKPayment paymentWithProductIdentifier:str];
    [[SKPaymentQueue defaultQueue] addPayment:payment];
}

//
// called when the transaction was successful
//
- (void)completeTransaction:(SKPaymentTransaction *)transaction
{
    LogMsg("Transaction complete");
    
    //get the receipt..
    NSString *receiptStr = [[NSString alloc] initWithData:transaction.transactionReceipt encoding:NSUTF8StringEncoding];
    
    string receipt = string([receiptStr cStringUsingEncoding:NSUTF8StringEncoding]);
    
    [[SKPaymentQueue defaultQueue] finishTransaction:transaction];
    
    GetMessageManager()->SendGUIStringEx(MESSAGE_TYPE_IAP_RESULT,(float)IAPManager::RESULT_OK,0.0f,0, receipt);
}

//
// called when a transaction has been restored and successfully completed
//
- (void)restoreTransaction:(SKPaymentTransaction *)transaction
{
    LogMsg("Restoring transaction");
    
    string itemStr([transaction.payment.productIdentifier cStringUsingEncoding:NSUTF8StringEncoding]);
    GetMessageManager()->SendGUIStringEx(MESSAGE_TYPE_IAP_PURCHASED_LIST_STATE, (float)IAPManager::PURCHASED, 0.0f, 0, itemStr);
    
    [[SKPaymentQueue defaultQueue] finishTransaction:transaction];
}

//
// called when a transaction has failed
//
- (void)failedTransaction:(SKPaymentTransaction *)transaction
{
    if (transaction.error.code != SKErrorPaymentCancelled)
    {
        LogMsg("Transaction failed");
        // error!
        GetMessageManager()->SendGUIStringEx(MESSAGE_TYPE_IAP_RESULT,(float)IAPManager::RESULT_ERROR,0.0f,0, "Error");
    }
    else
    {
        GetMessageManager()->SendGUIStringEx(MESSAGE_TYPE_IAP_RESULT,(float)IAPManager::RESULT_USER_CANCELED,0.0f,0, "Canceled");
        LogMsg("Transaction failed, user canceled");
    }
    
    [[SKPaymentQueue defaultQueue] finishTransaction:transaction];
}

//
// called when the transaction status is updated
//
- (void)paymentQueue:(SKPaymentQueue *)queue updatedTransactions:(NSArray *)transactions
{
    for (SKPaymentTransaction *transaction in transactions)
    {
        switch (transaction.transactionState)
        {
            case SKPaymentTransactionStatePurchased:
                [self completeTransaction:transaction];
                break;
            case SKPaymentTransactionStateFailed:
                [self failedTransaction:transaction];
                break;
            case SKPaymentTransactionStateRestored:
                [self restoreTransaction:transaction];
                break;
            default:
                break;
        }
    }
}

- (void)paymentQueueRestoreCompletedTransactionsFinished:(SKPaymentQueue *)queue
{
    GetMessageManager()->SendGUI(MESSAGE_TYPE_IAP_PURCHASED_LIST_STATE, (float)IAPManager::END_OF_LIST);
}

- (void)paymentQueue:(SKPaymentQueue *)queue restoreCompletedTransactionsFailedWithError:(NSError *)error
{
    GetMessageManager()->SendGUI(MESSAGE_TYPE_IAP_PURCHASED_LIST_STATE, (float)IAPManager::END_OF_LIST);
}

@end
