//  ***************************************************************
//  BaseApp - Creation date:  03/06/2009
//  -------------------------------------------------------------
//  Robinson Technologies - Check license.txt for license info.
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#pragma once

#ifndef BaseApp_h__
#define BaseApp_h__

#define C_MAX_TOUCHES_AT_ONCE 12


#include "PlatformSetup.h"

using namespace std;

#include "Manager/GameTimer.h"
#include "Manager/Console.h"
#include "util/MiscUtils.h"
#include "Manager/ResourceManager.h"
#include "Renderer/RenderBatcher.h"

//these aren't really used by this class but makes it easy for me to share these with everyone by only including this header
#include "util/RenderUtils.h"
#include "Manager/MessageManager.h"
#include "FileSystem/FileManager.h"
#include "util/ResourceUtils.h"
#include "util/GLESUtils.h"
#include "util/RenderUtils.h"
#include "GUI/RTFont.h"
#include "Entity/Entity.h"
#include "Audio/AudioManager.h"

class GamepadManager;

enum eFont
{
    FONT_SMALL,
    FONT_LARGE,
    FONT_FIXED,
    FONT_BASE_COUNT
};

//structure used to pass messages from Proton to the platform glue code on android, win, iOS, etc.  Parms are used differently
//for different messages so it's sort of confusing to use, just find an example and copy it of the message type you are trying
//to use.

struct OSMessage
{
    enum eMessageType
    {
        //DO *NOT* change the order of these, only add new ones at the end!
        
        MESSAGE_NONE,
        MESSAGE_OPEN_TEXT_BOX,
        MESSAGE_CLOSE_TEXT_BOX,
        MESSAGE_CHECK_CONNECTION,
        MESSAGE_SET_FPS_LIMIT, // Limit the frames per second of the main loop. The fps limit is passed in the m_x parameter. 0 means no limitation.
        MESSAGE_SET_ACCELEROMETER_UPDATE_HZ,
        MESSAGE_FINISH_APP, //only respected by windows, OSX, and android (Other platforms don't need it, as they have their own way...)
        MESSAGE_SET_VIDEO_MODE,
        
        //for tapjoy integration
        MESSAGE_TAPJOY_GET_FEATURED_APP,
        MESSAGE_TAPJOY_GET_AD,
        MESSAGE_TAPJOY_GET_MOVIE,
        
        MESSAGE_TAPJOY_SHOW_FEATURED_APP,  //used by AdManager
        MESSAGE_TAPJOY_SHOW_AD, //used by AdManager
        MESSAGE_TAPJOY_SHOW_MOVIE_AD, //used by AdManager  13
        
        MESSAGE_IAP_PURCHASE, //used by IAPManager
        MESSAGE_IAP_GET_PURCHASED_LIST, //for managed items with google billing, used by IAPManager
                
        MESSAGE_TAPJOY_GET_TAP_POINTS,
        MESSAGE_TAPJOY_SPEND_TAP_POINTS, // 17
        MESSAGE_TAPJOY_AWARD_TAP_POINTS,
        MESSAGE_TAPJOY_SHOW_OFFERS,
        
        MESSAGE_HOOKED_SHOW_RATE_DIALOG,
        MESSAGE_ALLOW_SCREEN_DIMMING, //send 1 for yes, 0 to no (supported by iOS only right now)
        
        MESSAGE_REQUEST_AD_SIZE, //send x and y, applicable to Tapjoy only on Android right now
        
        //for chartboost, only used for Android
        MESSAGE_CHARTBOOST_CACHE_INTERSTITIAL, //23
        MESSAGE_CHARTBOOST_SHOW_INTERSTITIAL,
        MESSAGE_CHARTBOOST_CACHE_MORE_APPS,
        MESSAGE_CHARTBOOST_SHOW_MORE_APPS,
        MESSAGE_CHARTBOOST_SETUP, //send chartboost app id/app sig as strings
        MESSAGE_CHARTBOOST_NOTIFY_INSTALL,
        MESSAGE_CHARTBOOST_RESERVED1,
        MESSAGE_CHARTBOOST_RESERVED2,
        
        //FLURRY
        MESSAGE_FLURRY_SETUP,
        MESSAGE_FLURRY_ON_PAGE_VIEW,
        MESSAGE_FLURRY_LOG_EVENT,
        
        MESSAGE_SUSPEND_TO_HOME_SCREEN, //Only on Android, if you want BACK from the main menu to look like its quitting, but really only
        //suspend due to Flurry/etc needing more time to process things.  Really, Android is not supposed to
        //have a "BACK TO QUIT" button at all, but users have gotten used to it.
        //On Windows/OS, this message functions the same as MESSAGE_FINISH_APP
        
        
        //tapjoy again
        
        MESSAGE_TAPJOY_INIT_MAIN,
        MESSAGE_TAPJOY_INIT_PAID_APP_WITH_ACTIONID,
        MESSAGE_TAPJOY_SET_USERID, //used with un-managed currency, this is what it will send on  the cb to server
        MESSAGE_IAP_CONSUME_ITEM, //used with android's IAB by IAPManager
        

		// Apps flyer
		MESSAGE_IAP_ITEM_DETAILS, // Used to get the item information
		MESSAGE_APPSFLYER_LOG_PURCHASE, //used to log purchase
		MESSAGE_FLURRY_START_TIMED_EVENT,
		MESSAGE_FLURRY_STOP_TIMED_EVENT,
	
        MESSAGE_USER = 1000,
    };
    
    enum eParmKeyboardType
    {
        PARM_KEYBOARD_TYPE_ASCII,
        PARM_KEYBOARD_TYPE_NUMBERS,
        PARM_KEYBOARD_TYPE_URL,
        PARM_KEYBOARD_TYPE_ASCII_FULL,
        PARM_KEYBOARD_TYPE_EMAIL
    };
    
    eMessageType m_type;
    int m_parm1; //max length of input box
    float m_x, m_y; //location of text box, or screen size if using MESSAGE_SET_VIDEO_MODE
    float m_sizeX, m_sizeY;
    float m_fontSize; //aspect ratio if using MESSAGE_SET_VIDEO_MODE
    string m_string; //first text parm
    uint32 m_parm2; //well, I use it to describe the input box type with the input stuff
    bool m_fullscreen; //used with MESSAGE_SET_VIDEO_MODE
    string m_string2; //second text parm
    string m_string3; //third text parm
    
};

enum eInputMode
{
    INPUT_MODE_NORMAL,
    INPUT_MODE_SEPARATE_MOVE_TOUCHES //move messages will only be on sig_input_move if this is set, useful because
    //on a 4 player game it's just too slow to send them to entire trees..
};

class TouchTrackInfo
{
public:
    
    TouchTrackInfo()
    {
        m_bHandled = false;
        m_bIsDown = false;
        m_vPos = m_vLastPos = CL_Vec2f(-1,-1);
        m_pEntityThatHandledIt = NULL;
        m_bPreHandled = false;
        m_pEntityThatPreHandledIt = NULL;
    }
    
    bool WasHandled() {return m_bHandled;}
    CL_Vec2f GetPos() {return m_vPos;}
    CL_Vec2f GetLastPos() {return m_vLastPos;}  //normally you wouldn't care, but this helps if you need to emulate the way iOS gives touch data
    
    bool WasPreHandled() {return m_bPreHandled;}
    Entity *GetEntityThatPreHandledIt() {return m_pEntityThatPreHandledIt;}
    void SetWasPreHandled(bool bNew, Entity *pEntity = NULL);
    
    bool IsDown() {return m_bIsDown;}
    Entity *GetEntityThatHandledIt() {return m_pEntityThatHandledIt;} //NULL if we don't know, or the click wasn't claimed as "handled"
    void SetIsDown(bool bNew)  { if (!bNew) m_vPos = m_vLastPos = CL_Vec2f(-1,-1);  m_bIsDown = bNew;}
    void SetPos(const CL_Vec2f &vPos)
    {
        m_vLastPos = m_vPos;
        m_vPos = vPos;
        //LogMsg("pos: %.2f, last pos: %.2f", m_vPos, m_vLastPos);
    }
    void SetWasHandled(bool bNew, Entity *pEntity = NULL); //pEntity should be null if not applicable
    
private:
    
    bool m_bHandled, m_bPreHandled;
    bool m_bIsDown;
    CL_Vec2f m_vPos, m_vLastPos;
    Entity *m_pEntityThatHandledIt, *m_pEntityThatPreHandledIt;
};


class BaseApp
{
public:
    
    //global errors can be set by anybody
    enum eErrorType
    {
        ERROR_NONE,
        ERROR_MEM,
        ERROR_SPACE
    };
    
    BaseApp();
    virtual ~BaseApp();
    
    virtual bool Init();
    virtual void Kill();
    virtual bool OnPreInitVideo();
    virtual void Draw();
    virtual void Update();
    virtual void OnEnterBackground(); //OS4 got a phonecall or changed apps, should save your junk
    virtual void OnEnterForeground(); //switched back to the app
    virtual void OnScreenSizeChange();
    virtual void OnFullscreenToggleRequest(); //Alt-Enter on Win, Ctrl-F on Mac - override if you want custom functionality
    void SetConsoleVisible(bool bNew);
    bool GetConsoleVisible() {return m_bConsoleVisible;}
    bool GetFPSVisible() {return m_bFPSVisible;}
    void SetFPSVisible(bool bNew) {m_bFPSVisible = bNew;}
    unsigned int GetGameTick();
    void SetGameTick(unsigned int tick) {m_gameTimer.SetGameTick(tick);}
    unsigned int GetTick() {return m_gameTimer.GetTick();}
    eTimingSystem GetActiveTimingSystem();
    unsigned int GetTickTimingSystem(eTimingSystem timingSystem);
    float GetDelta() {return m_gameTimer.GetDelta();}
    float GetGameDelta() {return m_gameTimer.GetGameDelta();}
    int GetDeltaTick();
    int GetGameDeltaTick() {return m_gameTimer.GetDeltaGameTick();} //elapsed time in milliseconds (1000=1 second)
    float GetElapsedTime() {return (float)m_gameTimer.GetDeltaTick()/1000.0f;} //elapsed time in seconds (1 = 1 second)
    float GetGameElapsedTime() {return (float)m_gameTimer.GetDeltaGameTick()/1000.0f;} //elapsed time in seconds (1 = 1 second)
    Console * GetConsole() {return &m_console;}
    void SetGameTickPause(bool bNew) {m_gameTimer.SetGameTickPause(bNew);}
    bool GetGameTickPause() {return m_gameTimer.GetGameTickPause();}
    GameTimer * GetGameTimer() {return &m_gameTimer;}
    virtual void OnMessage(Message &m);
    RTFont * GetFont(eFont font) {return &m_fontArray[font];}
    void SetInputMode(eInputMode mode){m_inputMode = mode;}
    eInputMode GetInputMode() {return m_inputMode;}
    virtual void OnMemoryWarning();
    //FocusComponents connect to these, which will tricky down their hierarchy
    /**
     * Taps, clicks, and basic keyboard input.
     *
     * The parameter variant list contains the following:
     * - 0: the type of the event cast to a float. One of:
     *   - \link eMessageType::MESSAGE_TYPE_GUI_CLICK_START MESSAGE_TYPE_GUI_CLICK_START\endlink
     *   - \link eMessageType::MESSAGE_TYPE_GUI_CLICK_END MESSAGE_TYPE_GUI_CLICK_END\endlink
     *   - \link eMessageType::MESSAGE_TYPE_GUI_CLICK_MOVE MESSAGE_TYPE_GUI_CLICK_MOVE\endlink
     *   - \link eMessageType::MESSAGE_TYPE_GUI_CLICK_MOVE_RAW MESSAGE_TYPE_GUI_CLICK_MOVE_RAW\endlink
     *   - \link eMessageType::MESSAGE_TYPE_GUI_CHAR MESSAGE_TYPE_GUI_CHAR\endlink
     *   - \link eMessageType::MESSAGE_TYPE_GUI_PASTE MESSAGE_TYPE_GUI_PASTE\endlink
     * - 1: the position of the event as a Vector2 if it makes sense. If the first member (the type)
     *   is either \link eMessageType::MESSAGE_TYPE_GUI_CHAR MESSAGE_TYPE_GUI_CHAR\endlink or
     *   \link eMessageType::MESSAGE_TYPE_GUI_PASTE MESSAGE_TYPE_GUI_PASTE\endlink then this is always (0, 0).
     * - 2: this parameter depends on the value of the first member (the type).
     *   - For \link eMessageType::MESSAGE_TYPE_GUI_CLICK_START MESSAGE_TYPE_GUI_CLICK_START\endlink,
     *     \link eMessageType::MESSAGE_TYPE_GUI_CLICK_END MESSAGE_TYPE_GUI_CLICK_END\endlink,
     *     \link eMessageType::MESSAGE_TYPE_GUI_CLICK_MOVE MESSAGE_TYPE_GUI_CLICK_MOVE\endlink,
     *     \link eMessageType::MESSAGE_TYPE_GUI_CLICK_MOVE_RAW MESSAGE_TYPE_GUI_CLICK_MOVE_RAW\endlink
     *     this parameter defines the id of the finger used for the touch event as a uint32.
     *   - For \link eMessageType::MESSAGE_TYPE_GUI_CHAR MESSAGE_TYPE_GUI_CHAR\endlink this
     *     parameter contains the keycode of the pressed key as a uint32.
     *   - For \link eMessageType::MESSAGE_TYPE_GUI_PASTE MESSAGE_TYPE_GUI_PASTE\endlink this
     *     parameter contains the contents of the clipboard as a \c string.
     * - 3: this parameter depends on the value of the first member (the type).
     *   - For \link eMessageType::MESSAGE_TYPE_GUI_CLICK_START MESSAGE_TYPE_GUI_CLICK_START\endlink,
     *     \link eMessageType::MESSAGE_TYPE_GUI_CLICK_END MESSAGE_TYPE_GUI_CLICK_END\endlink,
     *     \link eMessageType::MESSAGE_TYPE_GUI_CLICK_MOVE MESSAGE_TYPE_GUI_CLICK_MOVE\endlink,
     *     \link eMessageType::MESSAGE_TYPE_GUI_CLICK_MOVE_RAW MESSAGE_TYPE_GUI_CLICK_MOVE_RAW\endlink
     *     \link eMessageType::MESSAGE_TYPE_GUI_CHAR MESSAGE_TYPE_GUI_CHAR\endlink this
     *     \link eMessageType::MESSAGE_TYPE_GUI_CHAR MESSAGE_TYPE_GUI_CHAR_RAW\endlink this
     *		parameter contains the state of the control/shift/alt keys as a bitfield (uint32).
     *	    Test against the eVirtualKeyModifiers to determine which keys were down at the time this msg was generated.
     *   - For \link eMessageType::MESSAGE_TYPE_GUI_PASTE MESSAGE_TYPE_GUI_PASTE\endlink this
     *     parameter contains the contents of the clipboard as a \c string.
     */
    boost::signal<void (VariantList*)> m_sig_input;
    /**
     * "Move" touch messages.
     * Used if eInputMode::INPUT_MODE_SEPARATE_MOVE_TOUCHES was set. Otherwise they are signaled
     * via \c BaseApp::m_sig_input.
     */
    boost::signal<void (VariantList*)> m_sig_input_move;
    /**
     * Messages from the platform itself.
     * To get the type of the message use code like this:
     * \code eMessageType mType = (eMessageType)(int)pVList->m_variant[0].GetFloat(); \endcode
     */
    boost::signal<void (VariantList*)> m_sig_os;
    /**
     * Update signal for the game logic.
     * Called once per frame, usually.
     */
    boost::signal<void (VariantList*)> m_sig_update;
    /**
     * Signal for doing rendering.
     * Called once per frame. You should render but not do game logic here.
     */
    boost::signal<void (VariantList*)> m_sig_render;
    /**
     * Early signal that we are about to enter background.
     * Don't mess with video here. Useful for shutting down audio on android.
     */
    boost::signal<void (VariantList*)> m_sig_pre_enterbackground;
    boost::signal<void (VariantList*)> m_sig_enterbackground; ///< Game lost focus
    boost::signal<void (VariantList*)> m_sig_enterforeground; ///< Game restored focus
    boost::signal<void (VariantList*)> m_sig_accel; ///< Accelerometer data from iphone
    boost::signal<void (VariantList*)> m_sig_trackball; ///< Used for android trackball move data
    /**
     * For arcade movement controls like left/right/up/down.
     * If MovementInputComponent is used, trackball/wasd are converted to send through this as well.
     */
    boost::signal<void (VariantList*)> m_sig_arcade_input;
    /**
     * For raw data from keyboards that give pressed/released messages.
     * Generally you would convert them into arcade messages.
     *
     * The parameter variant list contains two members:
     * - 0: the keycode of the pressed or released key as a uint32.
     * - 1: VIRTUAL_KEY_PRESS or VIRTUAL_KEY_RELEASE as a uint32 depending on whether
     *   the event was a key press or release.
     */
    boost::signal<void (VariantList*)> m_sig_raw_keyboard;
    
    /**
     - 0: the eSystemType, as float.  Should be cast to (eMessageType)(int)
     * Signal to notify about hardware messages.  Currently there are only two,
     * MESSAGE_TYPE_HW_TOUCH_KEYBOARD_WILL_SHOW and MESSAGE_TYPE_HW_TOUCH_KEYBOARD_WILL_HIDE
     */
    boost::signal<void (VariantList*)> m_sig_hardware;
    
	boost::signal<void(void)> m_sig_onScreenSizeChanged;

    /**
     * Signal to notify that it's time to release surfaces.
     * Sent for example when the app goes to background.
     */
    boost::signal<void (void)> m_sig_unloadSurfaces;
    boost::signal<void (void)> m_sig_loadSurfaces; ///< Signal to notify that it's time to reload surfaces.
    
    boost::signal<void (VariantList*)> m_sig_joypad_events; //only used for the android Moga game controller implementation
    
    boost::signal<void (VariantList*)> m_sig_native_input_state_changed; //first part is a uint32 that is 1 if input box is open, 0 if closed.  Useful for turning off WASD while inputting a name on desktops
    
    deque <OSMessage> * GetOSMessages() {return &m_OSMessages;}
    void AddOSMessage(OSMessage &m);
    void SetManualRotationMode(bool bRotation); //if true, we manually rotate our GL and coordinates for the screen.
    bool GetManualRotationMode() {return m_bManualRotation;}
    ResourceManager * GetResourceManager() {return &m_resourceManager;}
    
    void ModMemUsed(int mod) {m_memUsed += mod;}
    void ModTexUsed(int mod) {m_texMemUsed += mod;}
    
    int GetMemUsed() {return m_memUsed;}
    int GetTexMemUsed() {return m_texMemUsed;}
    
    eErrorType GetLastError() {return m_error;}
    void ClearError() {m_error = ERROR_NONE;}
    void SetLastError(eErrorType error) {m_error = error;}
    bool IsInBackground() {return m_bIsInBackground;}
    bool GetDisableSubPixelBlits() {return m_bDisableSubPixelBlits;}
    void SetDisableSubPixelBlits(bool bNew) {m_bDisableSubPixelBlits = bNew;} //fixes issue where scaling makes 2d tile based games have cracks
    void AddCommandLineParm(string parm);
    vector<string> GetCommandLineParms();
    void SetAccelerometerUpdateHz(float hz); //another way to think of hz is "how many times per second to update"
    void SetAllowScreenDimming(bool bAllowDimming); //respected by iOS only for now
    void PrintGLString(const char *name, GLenum s);
    bool IsInitted() {return m_bInitted;}
    virtual void InitializeGLDefaults();
    CL_Mat4f * GetProjectionMatrix() {return &m_projectionMatrix;}
    Entity * GetEntityRoot() {return &m_entityRoot;} //an entity created by default, add children to become your entity hierarchy
    
    //not really used by framework, but useful if your app has cheat modes and you want an easy way to enable/disable them
    void SetCheatMode(bool bCheatMode) {m_bCheatMode = bCheatMode;}
    bool GetCheatMode() {return m_bCheatMode;}
    void SetVideoMode(int width, int height, bool bFullScreen, float aspectRatio = 0) /*aspectRatio should be 0 to ignore */;
    void KillOSMessagesByType(OSMessage::eMessageType type);
    
    /**
     * Limits the running rate of the main loop. The main loop will execute (approximately)
     * at most \a fps number of times per second.
     *
     * Setting \a fps to zero removes the limitation and lets the main loop run as quickly
     * as possible. This is also the default.
     *
     * Negative values for \a fps are meaningless and are silently ignored.
     *
     * \note Using this function sets an <b>upper bound</b> for the execution rate of the main
     * loop. It can't be used for example to ensure a given rate of calls to \c Update().
     */
    void SetFPSLimit(float fps);
    
    TouchTrackInfo * GetTouch(int index);
    int GetTotalActiveTouches(); //will return the total number of fingers currently touching the screen.
    void ResetTouches(); //not really advised or needed, ignore this
    
    string GetAppVersion();
protected:
    
    bool m_bConsoleVisible;
    bool m_bFPSVisible;
    bool m_bInitted;
    GameTimer m_gameTimer;
    Console m_console;
    RTFont m_fontArray[FONT_BASE_COUNT];
    deque <OSMessage> m_OSMessages; //simple way to send things to the OS, it will poll this
    bool m_bManualRotation;
    ResourceManager m_resourceManager; 
    eInputMode m_inputMode;
    int m_memUsed;
    int m_texMemUsed;
    eErrorType m_error;
    bool m_bIsInBackground;
    vector<string> m_commandLineParms;
    
    CL_Mat4f m_projectionMatrix;
    Entity m_entityRoot;
    bool m_bCheatMode;
    vector<TouchTrackInfo> m_touchTracker;
    string m_version;
    bool m_bDisableSubPixelBlits;
};

BaseApp * GetBaseApp(); //supply this yourself.  You create it on the first call if needed.
bool IsBaseAppInitted();
MessageManager * GetMessageManager(); //supply this yourself
FileManager * GetFileManager(); //supply this yourself
AudioManager * GetAudioManager();  //supply this yourself
Entity * GetEntityRoot(); //we supply this
GamepadManager * GetGamepadManager(); //supply this yourself, if you want gamepads
ResourceManager * GetResourceManager();
unsigned int GetTick(eTimingSystem timingSystem = GetBaseApp()->GetActiveTimingSystem()); //faster to write
eTimingSystem GetTiming();
extern RenderBatcher g_globalBatcher; //can be used by anyone
bool GetDefaultSmoothing(); //default antialising on or off
void SetDefaultSmoothing(bool bNew);

#endif // BaseApp_h__
