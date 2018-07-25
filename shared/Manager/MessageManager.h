//  ***************************************************************
//  MessageManager - Creation date: 03/18/2009
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef MessageManager_h__
#define MessageManager_h__

#include "util/Variant.h"

class Entity;
class EntityComponent;

enum eMessageClass
{
    MESSAGE_CLASS_GAME, //generic global game-specific messages
    MESSAGE_CLASS_GUI, //these come from GUI events like clicks
    MESSAGE_CLASS_ENTITY, //delivered directly to specific entities
};

enum eTimingSystem
{
    TIMER_SYSTEM,
    TIMER_GAME //message deliveries are paused when the game is paused/slowed down
};


enum eOSSTreamEvent{
    RT_kCFStreamEventNone = 0,
    RT_kCFStreamEventOpenCompleted = 1,
    RT_kCFStreamEventHasBytesAvailable = 2,
    RT_kCFStreamEventCanAcceptBytes = 4,
    RT_kCFStreamEventErrorOccurred = 8,
    RT_kCFStreamEventEndEncountered = 16
};

//if you change the order of these, you have to also
//update SharedActivity.java for the android side
enum eVirtualKeys
{
    VIRTUAL_KEY_NONE = 0,
    VIRTUAL_KEY_BACK = 500000, //escape on desktop computers, back key on android
    VIRTUAL_KEY_PROPERTIES,
    VIRTUAL_KEY_HOME,
    VIRTUAL_KEY_SEARCH,
    VIRTUAL_KEY_DIR_UP,
    VIRTUAL_KEY_DIR_DOWN,
    VIRTUAL_KEY_DIR_LEFT,
    VIRTUAL_KEY_DIR_RIGHT,
    VIRTUAL_KEY_DIR_CENTER,
    VIRTUAL_KEY_VOLUME_UP,
    VIRTUAL_KEY_VOLUME_DOWN,
    VIRTUAL_KEY_SHIFT,
    VIRTUAL_KEY_ALT, //doesn't work yet
    VIRTUAL_KEY_CONTROL,
    
    //useful for games when used by ArcadeInputComponent
    VIRTUAL_KEY_GAME_FIRE,
    VIRTUAL_KEY_GAME_TALK,
    VIRTUAL_KEY_GAME_MAGIC,
    VIRTUAL_KEY_GAME_INVENTORY,
    
    VIRTUAL_KEY_COMMAND, //on OSX.  equivelent would be windows key on Win, but I don't think we can really ever use that key..
    VIRTUAL_KEY_F1,
    VIRTUAL_KEY_F2,
    VIRTUAL_KEY_F3,
    VIRTUAL_KEY_F4,
    VIRTUAL_KEY_F5,
    VIRTUAL_KEY_F6,
    VIRTUAL_KEY_F7,
    VIRTUAL_KEY_F8,
    VIRTUAL_KEY_F9,
    VIRTUAL_KEY_F10,
    VIRTUAL_KEY_F11,
    VIRTUAL_KEY_F12,
    VIRTUAL_KEY_F13,
    VIRTUAL_KEY_F14,
    VIRTUAL_KEY_F15,
    VIRTUAL_KEY_F16,
    VIRTUAL_KEY_TRACKBALL_DOWN = 500035,
    
    //Controller buttons - laid out like an xbox controller
    
    //Note:  For xperia play, you must use VIRTUAL_KEY_DIR_CENTER instead of VIRTUAL_DPAD_BUTTON_DOWN, they
    //use the same keycode when hit..., so I can't have it send VIRTUAL_DPAD_BUTTON_DOWN unless I want
    //to detect HW and that seems like a hassle.
    
    VIRTUAL_DPAD_BUTTON_LEFT,
    VIRTUAL_DPAD_BUTTON_UP,
    VIRTUAL_DPAD_BUTTON_RIGHT, //generally considered a select button
    VIRTUAL_DPAD_BUTTON_DOWN, //generally considered a back button - map to VIRTUAL_KEY_DIR_CENTER instead for xperia play
    VIRTUAL_DPAD_SELECT,
    VIRTUAL_DPAD_START,
    VIRTUAL_DPAD_LBUTTON,
    VIRTUAL_DPAD_RBUTTON,
    VIRTUAL_DPAD_LTRIGGER,
    VIRTUAL_DPAD_RTRIGGER,
    VIRTUAL_DPAD_HAT_UP, //a hat is like the DPAD thingie on a 360 controller
    VIRTUAL_DPAD_HAT_RIGHT,
    VIRTUAL_DPAD_HAT_DOWN,
    VIRTUAL_DPAD_HAT_LEFT,
    
    VIRTUAL_KEY_ADB_LEFT_JOY_HORIZONTAL,
    VIRTUAL_KEY_ADB_LEFT_JOY_VERTICAL,
    VIRTUAL_KEY_ADB_RIGHT_JOY_HORIZONTAL,
    VIRTUAL_KEY_ADB_RIGHT_JOY_VERTICAL,
    VIRTUAL_KEY_ADB_LEFT_SHOULDER,
    VIRTUAL_KEY_ADB_RIGHT_SHOULDER,
    
    VIRTUAL_KEY_GAME_JUMP, //useful for games when used by ArcadeInputComponent
    
    VIRTUAL_KEY_CUSTOM_START = 510000, //if you add your own at the app specific level, do it after this
};

enum eVirtualKeyInfo
{
    VIRTUAL_KEY_RELEASE,
    VIRTUAL_KEY_PRESS
};

enum eVirtualKeyModifiers
{
    VIRTUAL_KEY_MODIFIER_CONTROL=1,
    VIRTUAL_KEY_MODIFIER_ALT=2,
    VIRTUAL_KEY_MODIFIER_SHIFT=4
};

enum eMessageType
{
    //Do NOT change the order of these!  Only add new stuff at the end.
    
    MESSAGE_TYPE_GUI_CLICK_START,
    MESSAGE_TYPE_GUI_CLICK_END,
    MESSAGE_TYPE_GUI_CLICK_MOVE, //only send when button/finger is held down
    MESSAGE_TYPE_GUI_CLICK_MOVE_RAW, //win only, the raw mouse move messages
    MESSAGE_TYPE_GUI_ACCELEROMETER,
    MESSAGE_TYPE_GUI_TRACKBALL,
    MESSAGE_TYPE_GUI_CHAR, //the input box uses it on windows since we don't have a virtual keyboard
    MESSAGE_TYPE_GUI_COPY,
    MESSAGE_TYPE_GUI_PASTE,
    MESSAGE_TYPE_GUI_TOGGLE_FULLSCREEN,
    
    MESSAGE_TYPE_SET_ENTITY_VARIANT,
    MESSAGE_TYPE_CALL_ENTITY_FUNCTION,
    MESSAGE_TYPE_CALL_COMPONENT_FUNCTION_BY_NAME,
    MESSAGE_TYPE_PLAY_SOUND,
    MESSAGE_TYPE_VIBRATE,
    MESSAGE_TYPE_REMOVE_COMPONENT,
    MESSAGE_TYPE_ADD_COMPONENT,
    MESSAGE_TYPE_OS_CONNECTION_CHECKED, //sent by macOS, will send an eOSSTreamEvent as parm1
    MESSAGE_TYPE_PLAY_MUSIC,
    MESSAGE_TYPE_UNKNOWN,
    MESSAGE_TYPE_PRELOAD_SOUND,
    MESSAGE_TYPE_GUI_CHAR_RAW,
    MESSAGE_TYPE_SET_SOUND_ENABLED,
    
    MESSAGE_TYPE_TAPJOY_AD_READY,
    MESSAGE_TYPE_TAPJOY_FEATURED_APP_READY,
    MESSAGE_TYPE_TAPJOY_MOVIE_AD_READY,
    
    MESSAGE_TYPE_IAP_RESULT,
    MESSAGE_TYPE_IAP_ITEM_STATE,
    
    MESSAGE_TYPE_TAPJOY_TAP_POINTS_RETURN,
    MESSAGE_TYPE_TAPJOY_TAP_POINTS_RETURN_ERROR,
    MESSAGE_TYPE_TAPJOY_SPEND_TAP_POINTS_RETURN,
    MESSAGE_TYPE_TAPJOY_SPEND_TAP_POINTS_RETURN_ERROR,
    MESSAGE_TYPE_TAPJOY_AWARD_TAP_POINTS_RETURN,
    MESSAGE_TYPE_TAPJOY_AWARD_TAP_POINTS_RETURN_ERROR,
    MESSAGE_TYPE_TAPJOY_EARNED_TAP_POINTS,
    MESSAGE_TYPE_GUI_JOYPAD_BUTTONS, //For Jake's android gamepad input
    MESSAGE_TYPE_GUI_JOYPAD, //For Jake's android gamepad input
    MESSAGE_TYPE_GUI_JOYPAD_CONNECT, // For Jakes android gamepad input
    
    MESSAGE_TYPE_CALL_ENTITY_FUNCTION_RECURSIVELY, //used to schedule fake clicks, helps me with debugging
    MESSAGE_TYPE_HW_TOUCH_KEYBOARD_WILL_SHOW, //ios only, when not using external keyboard
    MESSAGE_TYPE_HW_TOUCH_KEYBOARD_WILL_HIDE, //ios only, when not using external keyboard
    MESSAGE_TYPE_HW_KEYBOARD_INPUT_ENDING, //proton is done with input and requesting that the keyboard hide
    MESSAGE_TYPE_HW_KEYBOARD_INPUT_STARTING, //proton is asking for the keyboard to open
    
    MESSAGE_TYPE_IAP_PURCHASED_LIST_STATE, //used by android when it IAPManager syncs to get a list of item id's purchased
    
    MESSAGE_TYPE_CALL_STATIC_FUNCTION, //used by the message manager to call a static function with a VariantList*
    
    MESSAGE_TYPE_APP_VERSION, // version being passed through to game.
    
    MESSAGE_TYPE_GUI_MOUSEWHEEL, //mouse wheel delta movement, desktops only.  Only works on Win right now
    
    MESSAGE_TYPE_TAPJOY_OFFERWALL_CLOSED, //user has closed the tapjoy offerwall.  Useful to know when to unmute the game, if you've muted it
    
    MESSAGE_TYPE_MOVE_WINDOW_LAG_TRIGGERED, //user tried to move his window but ended up taking too long, special case for Seth's win32 stuff
    
    MESSAGE_TYPE_PLAY_MUSIC_FORCE_STREAMING,
    MESSAGE_TYPE_TAPJOY_NO_CONTENT_AVAILABLE, //none there
    MESSAGE_TYPE_TAPJOY_NO_CONTENT_TO_PRESENT, //still loading so can't show?  Unsure

	MESSAGE_TYPE_IAP_ITEM_INFO_RESULT, // IAP info result
    MESSAGE_TYPE_HTML5_GOT_UPLOAD,

    MESSAGE_USER = 1000, //users can add their own messages starting here
    
};

typedef void(*PtrFuncVarList)(VariantList *);

class Message: public boost::signals::trackable
{
    
public:
    
    Message(eMessageClass messageClass, eTimingSystem timer, eMessageType type) : m_class(messageClass), m_timerMethod(timer), m_type(type)
    {
        m_deliveryTime = 0; //delivery right away
        m_pTargetEntity = NULL;
        m_pComponent = NULL;
        m_pStaticFunction = NULL;
    }
    
    void SetParm1(float parm1) {m_parm1 = parm1;}
    float GetParm1() {return m_parm1;}
    void SetParm2(float parm2) {m_parm2 = parm2;}
    float GetParm2() {return m_parm2;}
    void SetParm3(int parm3) {m_parm3 = parm3;}
    int GetParm3() {return m_parm3;}
    void SetParm4(int parm4) {m_parm4 = parm4;}
    uint32 GetParm4() {return m_parm4;}
    eTimingSystem GetTimingMethod() {return m_timerMethod;}
    unsigned int GetDeliveryTime() {return m_deliveryTime;}
    eMessageType GetType() {return m_type;}
    eMessageClass GetClass() {return m_class;}
    void SetDeliveryTime(int deliveryTimeMS);
    Variant & Get(){return m_variant;}
    VariantList & GetVariantList(){return m_variantList;}
    void Set(const Variant &v) {m_variant = v;}
    void Set(const VariantList *v) {if (v) m_variantList = *v;}
    void SetTargetEntity(Entity *pEnt);
    Entity * GetTargetEntity() {return m_pTargetEntity;}
    
    void SetFunctionPointer(PtrFuncVarList pFunctionWithVList) { m_pStaticFunction = pFunctionWithVList;}
    PtrFuncVarList GetFunctionPointer() {return m_pStaticFunction;}
    
    void SetTargetComponent(EntityComponent *pComp);
    void SetComponentToAdd(EntityComponent *pComp);
    void ClearComponent() {m_pComponent = NULL;}
    EntityComponent * GetTargetComponent() {return m_pComponent;}
    
    void OnEntityDestroyed(Entity *pEnt);
    void OnComponentDestroyed(VariantList *pVList);
    void SetVarName(string const &varName) {m_varName = varName;}
    string const & GetVarName(){return m_varName;}
    void SetStringParm(string const &msg) {m_stringParm = msg;}
    string const & GetStringParm(){return m_stringParm;}
    
private:
    
    eMessageType m_type;
    eTimingSystem m_timerMethod;
    eMessageClass m_class;
    float m_parm1;
    float m_parm2;
    int m_parm3;
    uint32 m_parm4;
    unsigned int m_deliveryTime;
    Variant m_variant;
    VariantList m_variantList;
    Entity *m_pTargetEntity;
    EntityComponent *m_pComponent;
    string m_varName;
    string m_stringParm;
    PtrFuncVarList m_pStaticFunction;
    
};

eTimingSystem GetTiming();

class MessageManager
{
public:
    MessageManager();
    virtual ~MessageManager();
    
    void SendGUI( eMessageType type, float parm1, float parm2 = 0, int deliverTimeMS = 0, eTimingSystem timing = TIMER_SYSTEM);
    void SendGUI( eMessageType type, const Variant &v, int deliverTimeMS = 0);
    void SendGUI( eMessageType type, const VariantList &vList, int deliverTimeMS = 0);
    void SendGUIEx( eMessageType type, float parm1, float parm2, int finger, int deliverTimeMS = 0, eTimingSystem timing = TIMER_SYSTEM);
    void SendGUIEx2( eMessageType type, float parm1, float parm2, int finger, uint32 modifiers, int deliverTimeMS = 0, eTimingSystem timing = TIMER_SYSTEM);
    void SendGUIStringEx( eMessageType type, float parm1, float parm2, int finger, string s, int deliverTimeMS = 0, eTimingSystem timing = TIMER_SYSTEM);
    
    void SendGame( eMessageType type, const string msg, int deliverTimeMS = 0, eTimingSystem timing = TIMER_GAME);
    void SendGame( eMessageType type, const Variant &v, int deliverTimeMS = 0, eTimingSystem timing = TIMER_GAME);
    void SetEntityVariable( Entity *pEnt, int timeMS, const string &varName, const Variant &v, eTimingSystem timing = GetTiming());
    void SetComponentVariable( EntityComponent *pComp, int timeMS, const string &varName, const Variant &v, eTimingSystem timing = GetTiming() );
    void RemoveComponentByName(Entity *pEnt, int timeMS, const string &compName, eTimingSystem timing = GetTiming());
    void CallEntityFunction( Entity *pEnt, int timeMS, const string &funcName, const VariantList *v = NULL, eTimingSystem timing = GetTiming());
    void CallEntityFunctionRecursively( Entity *pEnt, int timeMS, const string &funcName,const VariantList *v = NULL, eTimingSystem timing = GetTiming());
    
    void CallComponentFunction( EntityComponent *pComp, int timeMS, const string &funcName, const VariantList *v = NULL, eTimingSystem timing = GetTiming()); //assumes you know the components address
    void CallComponentFunction( Entity *pEnt, const string &compName, int timeMS,const string &funcName, const VariantList *v = NULL, eTimingSystem timing = GetTiming() ); //useful for calling components that don't exist yet
    void AddComponent( Entity *pEnt, int timeMS, EntityComponent *pComp, eTimingSystem timing = GetTiming());
    
    void CallStaticFunction( PtrFuncVarList pFunctionWithVList, int timeMS, const VariantList *v = NULL, eTimingSystem timing = GetTiming() );
    
    void DeleteMessagesByFunctionCallName( const string &name, eTimingSystem timing = GetTiming());
    void DeleteMessagesToComponent( EntityComponent *pComponent); //kills messages on both timers
    void DeleteMessagesToEntity( Entity *pComponent); //kills messages on both timers
    void DeleteMessagesByType( eMessageType type, eTimingSystem timing = GetTiming());
    
    void DumpMessages();
    void Update(); //run every tick
    void DeleteAllMessages();
    
private:
    
    void Send(Message *m);
    void AddMessageToList(list <Message*> &messageList, Message *m);
    void DumpMessagesInList(list<Message*> m);
    void Deliver(Message *m);
    //a separate queue for each timing system
    list <Message*> m_gameMessages;
    list <Message*> m_systemMessages;
    
};

#endif // MessageManager_h__
