#include "PlatformPrecomp.h"
#include "IAPManager.h"

#ifdef _DEBUG
	#define SHOW_DEBUG_IAP_MESSAGES
#endif

#if defined PLATFORM_WINDOWS || defined PLATFORM_LINUX
	//you can also define FAKE_SUCCESSFULLY_IAP_REPLY in your project for iOS/android/webos builds, for testing IAP that hasn't been setup yet
	#define FAKE_IAP_REPLY
#endif

//set a default for when faking IAP responses
#if defined(FAKE_IAP_REPLY) && !defined(FAKE_IAP_RESPONSE_SUCCESS) && !defined(FAKE_IAP_RESPONSE_FAILED) && !defined(FAKE_IAP_RESPONSE_ALREADY_PURCHASED) && !defined(FAKE_IAP_RESPONSE_TIME_OUT)
	#define FAKE_IAP_RESPONSE_SUCCESS
	//#define FAKE_IAP_RESPONSE_FAILED
	//#define FAKE_IAP_RESPONSE_ALREADY_PURCHASED
	//#define FAKE_IAP_RESPONSE_TIME_OUT
#endif

IAPManager::IAPManager()
{
    m_bTreatAllItemsAsConsumable = false;
    m_state = STATE_NONE;
    m_returnState = RETURN_STATE_NONE;
    m_failureReason = FAILURE_REASON_NONE;
    m_timer = 0;
    m_bWaitingForReply = false;
    
    //This gives us a way to schedule calls to m_sig_item_unexpected_purchase_result, used with TestDelayedIAP
    m_entity.GetFunction("OnUnexpectedPurchase")->sig_function.connect(m_sig_item_unexpected_purchase_result);
}

IAPManager::~IAPManager()
{
}

bool IAPManager::IsItemPurchased(const string &item) const
{
    return m_items.find(item) != m_items.end();
}

void IAPManager::HandlePurchaseListReply(Message &m)
{
    //we must have asked for a list of purchases the user has made on android
    switch (ItemStateCode((int)m.GetParm1()))
    {
        case END_OF_LIST:
        {

			
            RestoreLostPurchases(false);
#ifdef SHOW_DEBUG_IAP_MESSAGES
            LogMsg("(finished receiving purchase history of managed items)");
#endif
            
            VariantList vList;
            m_sig_purchased_list_updated(&vList);
            
            if (!m_itemToBuy.empty())
            {
                //now buy the item
                
#ifdef SHOW_DEBUG_IAP_MESSAGES
                LogMsg("Well, we don't already own it.  Sending buy request for %s", m_itemToBuy.c_str());
#endif
                sendPurchaseMessage();
            }
            

            if (!m_itemToConsume.empty())
            {
                sendConsumeMessage();
            }
            break;
        }
            
        case PURCHASED:
        {

#ifdef SHOW_DEBUG_IAP_MESSAGES
            LogMsg("Already purchased %s", m.GetStringParm().c_str());
#endif
            
            string orderData;
            //If there is a | in the result, assume it's the newer format that breaks down the data, name, then actual order data
            if (IsInString(m.GetStringParm(), "|"))
            {
                orderData = m.GetStringParm();
                string itemName = SeparateStringSTL(m.GetStringParm(), 0,'|');
                //Yes, we're making a pretty big assumption that "gems_bag_etc|" only exists once at the start, but that should be safe
                StringReplace(itemName+"|", "", orderData);
                m.SetStringParm(itemName);
            }
            
            bool added = m_items.insert(m.GetStringParm()).second;
#ifdef SHOW_DEBUG_IAP_MESSAGES
            if (added)
            {
                LogMsg("Added %s, now has %d purchased items", m.GetStringParm().c_str(), m_items.size());
            }
#endif
            
            if (!m_itemToBuy.empty())
            {
                //now buy the item
                
                //but wait, do we already own it?
                if (m_itemToBuy == m.GetStringParm())
                {
                    
#ifdef SHOW_DEBUG_IAP_MESSAGES
                    LogMsg("Already bought but it wasn't processed properly? Buying %s again", m_itemToBuy.c_str());
#endif
                    m_extraData = orderData; //the real order info
                    //just fake a yes to the purchase request now
                    endPurchaseProcessWithResult(RETURN_STATE_ALREADY_PURCHASED);
                    m_itemToBuy.clear();
                    m_itemDeveloperData.clear();
                    break;
                }
                
#ifdef SHOW_DEBUG_IAP_MESSAGES
                LogMsg("Well, we don't already own it.  Sending buy request for %s", m_itemToBuy.c_str());
#endif
                sendPurchaseMessage();
            } else
            {
                
                if (m_bTreatAllItemsAsConsumable)
                {
                    //that's weird, why is this here?  It must be a half processed order on android.  Send it to be processed to the app, if they are listening
#ifdef SHOW_DEBUG_IAP_MESSAGES
                    LogMsg("SendUnexpectedPurchaseSignal -  Buying %s again because it's not consumed", m.GetStringParm().c_str());
#endif
                    m_extraData = orderData; //the real order info
                    //just fake a yes to the purchase request now
                    
                    SendUnexpectedPurchaseSignal(RETURN_STATE_ALREADY_PURCHASED, m.GetStringParm(), m_extraData);
                    
                    m_itemToConsume = m.GetStringParm();
                    sendConsumeMessage();
                    m_itemToConsume.clear(); //don't want this consumed again when this list finishes
                    m_itemDeveloperData.clear();
                    break;
                    
                }
            }

            }

            break;
            
        case CANCELED:
        case REFUNDED:
            LogMsg("Got canceled or refunded (%d) when asking about previous purchases.  That's weird.", (int)m.GetParm1());
            break;
    }
}


void IAPManager::SendUnexpectedPurchaseSignal(eReturnState returnState, string itemID, string extra)
{
	#ifdef SHOW_DEBUG_IAP_MESSAGES
    LogMsg("SendUnexpectedPurchaseSignal> Sending message %d about %s", returnState,  itemID.c_str());
	#endif
    
    VariantList vList( (uint32)returnState, extra, itemID);
    m_sig_item_unexpected_purchase_result(&vList);
}

void IAPManager::HandleIAPBuyResult(Message &m)
{
    m_extraData = m.GetStringParm();
    
#ifdef SHOW_DEBUG_IAP_MESSAGES
    LogMsg("Got MESSAGE_TYPE_IAP_RESULT response: %d - Extra: %s", (int)m.GetParm1(), m_extraData.c_str());
#endif
    
    ResponseCode result = ResponseCode((int)m.GetParm1());
    switch (result)
    {
        case RESULT_OK:
        case RESULT_OK_ALREADY_PURCHASED:
            endPurchaseProcessWithResult(result == RESULT_OK_ALREADY_PURCHASED ? RETURN_STATE_ALREADY_PURCHASED : RETURN_STATE_PURCHASED);
            break;
            
        case RESULT_USER_CANCELED:
            m_failureReason = FAILURE_REASON_USER_CANCELED;
        default:
            endPurchaseProcessWithResult(RETURN_STATE_FAILED);
            break;
    }
    
    m_itemToBuy.clear();
    m_itemDeveloperData.clear();
}

void IAPManager::HandleItemInfo(class Message& m)
{
	std::string infoStream = m.GetStringParm().c_str();
	LogMsg(infoStream.c_str());
	std::istringstream ss(infoStream);
	std::string token;
	std::vector<std::string> info;

	while (std::getline(ss, token, ',')) {
		info.push_back(token);
	}
#ifdef _DEBUG
	for (int i = 0; i < 3; i++) {
		LogMsg(info.at(i).c_str());
	}
#endif;

	// info.at(0) = itemId, info.at(1) = currency, info.at(2) = price
	// set current product info
	item_info_vector = info;
}


void IAPManager::HandleItemUpdateState(Message &m)
{
    
	#ifdef SHOW_DEBUG_IAP_MESSAGES
    LogMsg("New order received/changed: %d, about %s", (int)m.GetParm1(), m.GetStringParm().c_str());
	#endif
    
    switch (ItemStateCode((int)m.GetParm1()))
    {
        case END_OF_LIST:
        {
            LogMsg("END_OF_LIST while getting an item update state?");
            break;
        }
            
        case PURCHASED:
        {
			#ifdef SHOW_DEBUG_IAP_MESSAGES
            LogMsg("Has purchased %s", m.GetStringParm().c_str());
			#endif
            if (m_state == STATE_WAITING)
            {
                endPurchaseProcessWithResult(RETURN_STATE_PURCHASED);
            } else
            {
				#ifdef SHOW_DEBUG_IAP_MESSAGES
                LogMsg("Got unplanned item purchase message, must have been delayed and delivered later.  Item: %s", m.GetStringParm().c_str());
				#endif
                
                SendUnexpectedPurchaseSignal(RETURN_STATE_PURCHASED, m.GetStringParm(), "");
            }
            break;
        }
            
        case CANCELED:
            if (m_state == STATE_WAITING)
            {
                endPurchaseProcessWithResult(RETURN_STATE_FAILED);
            } else
            {
                //huh?  This apparently gets sent instead of REFUNDED sometimes
                //handle as a real refund message that we didn't expect
                SendUnexpectedPurchaseSignal(RETURN_STATE_REFUNDED, m.GetStringParm(), "");
            }
            break;
            
        case REFUNDED:
            
            LogMsg("Got item refund message. item: %s", m.GetStringParm().c_str());
            
            //I don't think we'd normally ever get this while waiting for a purchase to go through (can't refund what we don't own yet) but
            //when buying the android.test.refunded item this does happen, so we'll handle it as a cancel if we're waiting on a reply
            if (m_state == STATE_WAITING)
            {
                endPurchaseProcessWithResult(RETURN_STATE_FAILED);
            } else
            {
                //handle as a real refund message that we didn't expect
                SendUnexpectedPurchaseSignal(RETURN_STATE_REFUNDED, m.GetStringParm(), "");
            }
            
            break;
    }
}

void IAPManager::OnMessage( Message &m )
{
    
    switch (m.GetType())
    {
        case MESSAGE_TYPE_IAP_PURCHASED_LIST_STATE:
            HandlePurchaseListReply(m);
            break;
            
        case MESSAGE_TYPE_IAP_RESULT:
        {
            HandleIAPBuyResult(m);
            break;
        }
            
        case MESSAGE_TYPE_IAP_ITEM_INFO_RESULT:
        {
            // Send Appsflyer informartion
         //   IAPManager::sendAppsflyerPurchaseTracking(info.at(0), info.at(1),info.at(2));
			
        	HandleItemInfo(m);
			break;
        }
            
        case MESSAGE_TYPE_IAP_ITEM_STATE:
        {
            //an item was purchased, or refunded.  Currently called by android only, and can be called at any time the app is initizalized, not
            //just when we are asking to buy something.
            HandleItemUpdateState(m);
            break;
        }
            
        default:
            break;
    }
}

bool IAPManager::Init()
{
    return true;
}

void IAPManager::Update()
{
#if defined (FAKE_IAP_REPLY)
    
    if (m_bWaitingForReply)
    {
        //don't support billing on this platform (probably testing in desktop), fake it after 3 seconds for testing purposes
        
	#ifdef FAKE_IAP_RESPONSE_TIME_OUT
        //we will never send a reply, as if they suddenly lost connection or whatever
        return;
	#endif
        
        if (m_timer+3000 < GetTick(TIMER_SYSTEM))
        {
            LogMsg("Doing fake reply");
            //fake a successful sale message back to us, so we'll process the order even while emulating on desktop
            m_bWaitingForReply = false;
            
            //WebOS & iOS
            //DEBUG - To test successful or faked purchases under desktop emulation
				#if defined (FAKE_IAP_RESPONSE_SUCCESS)
            GetMessageManager()->SendGUIStringEx(MESSAGE_TYPE_IAP_RESULT, (float)IAPManager::RESULT_OK, 0.0f, 0, "Faked order for testing: OK");
				#elif defined (FAKE_IAP_RESPONSE_ALREADY_PURCHASED)
            GetMessageManager()->SendGUIStringEx(MESSAGE_TYPE_IAP_RESULT, (float)IAPManager::RESULT_OK_ALREADY_PURCHASED, 0.0f, 0, "Faked order for testing: already purchased");
				#elif defined (FAKE_IAP_RESPONSE_FAILED)
            GetMessageManager()->SendGUIStringEx(MESSAGE_TYPE_IAP_RESULT, (float)IAPManager::RESULT_ERROR, 0.0f, 0, "Faked order for testing: error");
				#else
            GetMessageManager()->SendGUIStringEx(MESSAGE_TYPE_IAP_RESULT, (float)IAPManager::RESULT_USER_CANCELED, 0.0f, 0, "Faked order for testing: user canceled");
				#endif
            
        }
    }
#endif
}

void IAPManager::BuyItem(const string &itemName, const string &developerData)
{
    Reset();
    
    m_lastItemID = itemName;
    m_itemToBuy = itemName;
    m_itemDeveloperData = developerData;
    
#ifdef SHOW_DEBUG_IAP_MESSAGES
    LogMsg("Planning to buy %s", itemName.c_str());
#endif
    
    if (GetEmulatedPlatformID() == PLATFORM_ID_ANDROID)
    {
        //The Android in-app-billing way
        
        //we'll query all purchased items first, because if we try to buy something already purchased, it will give an
        //annoying error to the user and not send us any response.
        //We should probably modify this to only scan once in a while but it seems very quick, so  meh.
        
        m_items.clear();
        m_state = STATE_WAITING;
        
		#ifndef FAKE_IAP_REPLY
        OSMessage o;
        o.m_type = OSMessage::MESSAGE_IAP_GET_PURCHASED_LIST;
        GetBaseApp()->AddOSMessage(o);
		#else
        //faking it, this way is easier
        
        sendPurchaseMessage();
		#endif
    } else
    {
        //issue buy command the normal way, it's not like Android where you need to get a list of stuff first
        sendPurchaseMessage();
    }
    
}

void IAPManager::Reset()
{
    m_state = STATE_NONE;
    m_returnState = RETURN_STATE_NONE;
    m_failureReason = FAILURE_REASON_NONE;
    m_extraData.clear();
    m_timer = 0;
}

void IAPManager::sendPurchaseMessage()
{
	#ifndef FAKE_IAP_REPLY
    //this is the real thing, send message to native system
    OSMessage o;
    o.m_type = OSMessage::MESSAGE_IAP_PURCHASE;
    o.m_string = m_itemToBuy;
    o.m_string2 = m_itemDeveloperData;
    
    GetBaseApp()->AddOSMessage(o);
	#endif
    
    m_itemToBuy.clear();
    m_itemDeveloperData.clear();
    m_timer = GetTick(TIMER_SYSTEM);
    m_bWaitingForReply = true;
    m_state = STATE_WAITING;
}

void IAPManager::sendAppsflyerPurchaseTracking(string item, string currency, string price)
{
    OSMessage o;
    o.m_type    = OSMessage::MESSAGE_APPSFLYER_LOG_PURCHASE;
    o.m_string  = item;
    o.m_string2 = currency;
    o.m_string3 = price;
    GetBaseApp()->AddOSMessage(o);
}

void IAPManager::getItemDetailsForTracking(string item)
{
    OSMessage o;
    o.m_type    = OSMessage::MESSAGE_IAP_ITEM_DETAILS;
    o.m_string  = item;
    GetBaseApp()->AddOSMessage(o);
}

void IAPManager::endPurchaseProcessWithResult(eReturnState returnState)
{
    m_state = STATE_NONE;
    m_returnState = returnState;
    VariantList vList(uint32(m_returnState), m_extraData, m_lastItemID, uint32(m_failureReason));
    m_sig_item_purchase_result(&vList);
}

void IAPManager::SyncPurchases()
{
    LogMsg("Syncing purchases...");
    m_items.clear();
    
    OSMessage o;
    o.m_type = OSMessage::MESSAGE_IAP_GET_PURCHASED_LIST;
    GetBaseApp()->AddOSMessage(o);
}

void IAPManager::TestDelayedIAP( string itemID, string receipt, int delayMS, eReturnState returnState /*= RETURN_STATE_PURCHASED */ )
{
    VariantList vList( (uint32)returnState, receipt, itemID);
    GetMessageManager()->CallEntityFunction(&m_entity, delayMS, "OnUnexpectedPurchase", &vList, TIMER_SYSTEM);
}


void IAPManager::sendConsumeMessage()
{
    OSMessage o;
    o.m_type = OSMessage::MESSAGE_IAP_CONSUME_ITEM;
    o.m_string = m_itemToConsume;
    GetBaseApp()->AddOSMessage(o);
    m_itemToConsume.clear();
    //m_timer = GetTick(TIMER_SYSTEM);
    //m_bWaitingForReply = true;
    //m_state = STATE_WAITING;
}


void IAPManager::ConsumeItem( string itemID )
{
    m_itemToConsume = itemID; //reall command will happen after the sync completes
    //the android side will start the sync, and not do the consume until the sync is finished.  You hope.
    SyncPurchases();
    
}
