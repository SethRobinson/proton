/*
 * main.cpp
 *
 *  Created on: Oct 26, 2011
 *      Author: User
 */
//#include <curses.h>
#include <assert.h>
#include <screen/screen.h>
#include <bps/accelerometer.h>
#include <bps/navigator.h>
#include <bps/screen.h>
#include <bps/bps.h>
#include <bps/event.h>
#include <bps/dialog.h>
#include <bps/virtualkeyboard.h>
#include <sys/keycodes.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include "bbutil.h"

#include <string>
#include "BaseApp.h"

using namespace std;
int exit_application = 0;
unsigned int fpsTimerLoopMS = 0; //set to 60 fps limit at the start

static screen_context_t screen_cxt;
//Query g_primaryGLX and g_primaryGLY of the window surface created by utility code
 EGLint g_primaryGLX, g_primaryGLY;

 int GetPrimaryGLX() {return g_primaryGLX;}
 int GetPrimaryGLY() {return g_primaryGLY;}

 bool g_leftMouseButtonDown = false;

 bool g_bUsingAccelerometer = false;

void handleScreenEvent(bps_event_t *event)
{
    screen_event_t screen_event = screen_event_get_event(event);

    int screen_val;
    screen_get_event_property_iv(screen_event, SCREEN_PROPERTY_TYPE, &screen_val);

    switch (screen_val) {
    case SCREEN_EVENT_MTOUCH_TOUCH:
    case SCREEN_EVENT_MTOUCH_MOVE:
    case SCREEN_EVENT_MTOUCH_RELEASE:
        break;
    }
}

int initialize()
{

	eglQuerySurface(egl_disp, egl_surf, EGL_WIDTH, &g_primaryGLX);
    eglQuerySurface(egl_disp, egl_surf, EGL_HEIGHT, &g_primaryGLY);

	EGLint err = eglGetError();
	if (err != 0x3000) {
		fprintf(stderr, "Unable to querry egl surface dimensions\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

void ProcessEvents()
{

	    while (true)
	    {
	        bps_event_t* event = NULL;
	        int rc = bps_get_event(&event, 0);

	        assert(BPS_SUCCESS == rc);
	        if (rc != BPS_SUCCESS)
	        {
	         LogMsg("HUH?!");
	            break;
	        }

	        if (event == NULL)
	        {
	            // No more events in the queue
	            break;
	        }

	        int domain = bps_event_get_domain(event);
	        int code = bps_event_get_code(event);

	        if (navigator_get_domain() == domain)
	        {

	            switch(code)
	            {

	            case NAVIGATOR_ORIENTATION_CHECK:

	            	LogMsg("Device rotated, but telling device to ignore it.  Change later?");
	            	navigator_orientation_check_response(event, false); //tell it we won't rotate.. for now

	            	break;

	            case NAVIGATOR_EXIT:

	            	LogMsg("Leaving BBX app");
	            	exit_application = 1;

	            	break;

	            case NAVIGATOR_WINDOW_STATE:
	            {

	            	navigator_window_state_t winState = navigator_event_get_window_state(event);

	            	switch (winState)
	            	{
	            	case NAVIGATOR_WINDOW_FULLSCREEN:
	            		  GetBaseApp()->OnEnterForeground();
	            		LogMsg("Full screen");

	            		break;
	            	case NAVIGATOR_WINDOW_THUMBNAIL:

	            		LogMsg("App thumbnailed");
	                  	GetBaseApp()->OnEnterBackground();

	            		break;

	            	case NAVIGATOR_WINDOW_INVISIBLE:
	            		LogMsg("App in background");
	                  	GetBaseApp()->OnEnterBackground();

	            		break;

	            	}
	            }

	            	break;

	            case NAVIGATOR_WINDOW_INACTIVE:
	               // m_isPaused = true;
	               // m_handler->onPause();
		               LogMsg("Window unactive");
		           	GetBaseApp()->OnEnterBackground();
		               break;

	            case NAVIGATOR_WINDOW_ACTIVE:
	               LogMsg("Window active");
	               GetBaseApp()->OnEnterForeground();
	               break;
	            }

	        } else if (screen_get_domain() == domain)
	        {
	            screen_event_t screenEvent = screen_event_get_event(event);
	            int screenEventType;
	            int screenEventPosition[2];
	            int screenTouchID = 0;

	            screen_get_event_property_iv(screenEvent, SCREEN_PROPERTY_TYPE, &screenEventType);
	            screen_get_event_property_iv(screenEvent, SCREEN_PROPERTY_SOURCE_POSITION, screenEventPosition);

	         //  LogMsg("Input type: %d, (touchid %d)", screenEventType, screenTouchID);

	           switch (screenEventType)
	           {

	           case SCREEN_EVENT_MTOUCH_TOUCH:
	            {
	            	screen_get_event_property_iv(screenEvent, SCREEN_PROPERTY_TOUCH_ID,  &screenTouchID);
	            	//LogMsg("Touched");
	        	  	if (screenTouchID > C_MAX_TOUCHES_AT_ONCE)
					{
						LogMsg("How can this be finger %d?!", screenTouchID);
						break;;
					}
					float xPos = screenEventPosition[0];
            	    float yPos = screenEventPosition[1];
            	    ConvertCoordinatesIfRequired(xPos, yPos);
        			GetMessageManager()->SendGUIEx(MESSAGE_TYPE_GUI_CLICK_START, xPos, yPos, screenTouchID);

	            }
	        	break;

	           case SCREEN_EVENT_MTOUCH_RELEASE:
	            {
	            	  screen_get_event_property_iv(screenEvent, SCREEN_PROPERTY_TOUCH_ID,  &screenTouchID);
	            	
					  if (screenTouchID > C_MAX_TOUCHES_AT_ONCE)
					  {
						  LogMsg("How can this be finger %d?!", screenTouchID);
						  break;;
					  }
					  // LogMsg("Touch release");
	            	 	float xPos = screenEventPosition[0];
	            	    float yPos = screenEventPosition[1];
	            	    ConvertCoordinatesIfRequired(xPos, yPos);
	            	    GetMessageManager()->SendGUIEx(MESSAGE_TYPE_GUI_CLICK_END, xPos, yPos, screenTouchID);
    //    m_handler->onLeftRelease(static_cast<float>(screenEventPosition[0]), static_cast<float>(screenEventPosition[1]));
	            }
	            break;

	            case SCREEN_EVENT_MTOUCH_MOVE:
	            {
	            	screen_get_event_property_iv(screenEvent, SCREEN_PROPERTY_TOUCH_ID,  &screenTouchID);
					if (screenTouchID > C_MAX_TOUCHES_AT_ONCE)
					{
						LogMsg("How can this be finger %d?!", screenTouchID);
						break;;
					}
	            	float xPos = screenEventPosition[0];
	            	    float yPos = screenEventPosition[1];
	            	    ConvertCoordinatesIfRequired(xPos, yPos);
	            	    GetMessageManager()->SendGUIEx(MESSAGE_TYPE_GUI_CLICK_MOVE, xPos, yPos, screenTouchID);
    //    m_handler->onLeftRelease(static_cast<float>(screenEventPosition[0]), static_cast<float>(screenEventPosition[1]));
	            }
	            break;

	            case SCREEN_EVENT_POINTER:
	            {
	                int pointerButton = 0;
	                screen_get_event_property_iv(screenEvent, SCREEN_PROPERTY_BUTTONS, &pointerButton);
		          //	   LogMsg("PointerType: %d - ", screenEventType);
					screenTouchID = 0;
	                if (pointerButton == SCREEN_LEFT_MOUSE_BUTTON)
	                {
	              	  	float xPos = screenEventPosition[0];
	            	    float yPos = screenEventPosition[1];
	            	    ConvertCoordinatesIfRequired(xPos, yPos);


	                	if (!g_leftMouseButtonDown)
	                	{

	                     	g_leftMouseButtonDown = true;
	          	  			//LogMsg("Mouse down at %.2f, %.2f", xPos, yPos);
	            			GetMessageManager()->SendGUIEx(MESSAGE_TYPE_GUI_CLICK_START, xPos, yPos, screenTouchID);
	                	} else
	                	{
	          	  			//LogMsg("Mouse move at %.2f, %.2f", xPos, yPos);
	          				GetMessageManager()->SendGUIEx(MESSAGE_TYPE_GUI_CLICK_MOVE, xPos, yPos, screenTouchID);
	                	}


	                 } else
	                 {
	                	  //well, it's the mouse moving or the button was released.  No way to tell which one in the
	                	  //current API?
	                  	if (g_leftMouseButtonDown)
	                  	{
	                  		//LogMsg("Releasing mouse button");
	                  		g_leftMouseButtonDown = false;
	                   	  	float xPos = screenEventPosition[0];
	                	    float yPos = screenEventPosition[1];
	                	    ConvertCoordinatesIfRequired(xPos, yPos);
	                		GetMessageManager()->SendGUIEx(MESSAGE_TYPE_GUI_CLICK_END, xPos, yPos, screenTouchID);
	                  	} else
	                  	{

	                  		//LogMsg("Uh, already released, so what is this, a mouse move? ",pointerButton );

	                  	}

	                }

	            }
	            break;

	            case SCREEN_EVENT_KEYBOARD:
	          	            {
	          	            	int flags, val;
	          	            	screen_get_event_property_iv(screenEvent, SCREEN_PROPERTY_KEY_FLAGS, &flags);

	          	            	screen_get_event_property_iv(screenEvent, SCREEN_PROPERTY_KEY_SYM, &val);
	          					if (flags & KEY_DOWN)
	          					{
	          						GetMessageManager()->SendGUI(MESSAGE_TYPE_GUI_CHAR, (float)val, (float)1);

	          						if (! (flags & KEY_REPEAT))
	          						{
	          							if (val < 128) val = toupper(val);
	          							GetMessageManager()->SendGUI(MESSAGE_TYPE_GUI_CHAR_RAW, (float)val, (float)1);
	          						}

	          					} else
	          					{
	          						//key released
	          						if (val < 128) val = toupper(val);
	          						GetMessageManager()->SendGUI(MESSAGE_TYPE_GUI_CHAR_RAW, (float)val, (float)0);
	          					}
	          	            }
	                      	break;


	            default:
	            	//LogMsg("Unhandled msg: %d", code);
	            	break;



	           }

	        } else if (dialog_get_domain() == domain)
	        {

	        	/*

	        	 /if (DIALOG_RESPONSE == code)
	            {
	             //   ASSERT(m_promptInProgress);
	              //  m_promptInProgress = false;
	                //m_handler->onPromptOk(dialog_event_get_prompt_input_field(event));
	            	LogMsg("Got dialog response %d", code);
	            }
	            */
	        } else {
	           LogMsg("Unrecognized and unrequested event! domain: %d, code: %d", domain, code);
	        }

	       // bps_event_destroy(event);  //Conflicting docs about needing this.. but it will crash, so no, I guess not
	    }

}

int main(int argc, char *argv[])
{
    int rc;

    //Create a screen context that will be used to create an EGL surface to to receive libscreen events
    screen_create_context(&screen_cxt, 0);

    //Initialize BPS library
	bps_initialize();

	//Use utility code to initialize EGL for 2D rendering with GL ES 1.1
	if (EXIT_SUCCESS != bbutil_init_egl(screen_cxt, GL_ES_1, AUTO)) {
		fprintf(stderr, "bbutil_init_egl failed\n");
		bbutil_terminate();
		screen_destroy_context(screen_cxt);
		return 0;
	}

	//Initialize application logic
	if (EXIT_SUCCESS != initialize()) {
		fprintf(stderr, "initialize failed\n");
		bbutil_terminate();
		screen_destroy_context(screen_cxt);
		return 0;
	}

	//Signal BPS library that navigator and screen events will be requested
	if (BPS_SUCCESS != screen_request_events(screen_cxt)) {
		fprintf(stderr, "screen_request_events failed\n");
		bbutil_terminate();
		screen_destroy_context(screen_cxt);
		return 0;
	}

	if (BPS_SUCCESS != navigator_request_events(0)) {
		fprintf(stderr, "navigator_request_events failed\n");
		bbutil_terminate();
		screen_destroy_context(screen_cxt);
		return 0;
	}

	 navigator_rotation_lock(true);
	 bbutil_init_egl(screen_cxt, GL_ES_1, LANDSCAPE);

	 /*
	//Signal BPS library that navigator orientation is not to be locked
	if (BPS_SUCCESS != navigator_rotation_lock(false)) {
		fprintf(stderr, "navigator_rotation_lock failed\n");
		bbutil_terminate();
		screen_destroy_context(screen_cxt);
		return 0;
	}
	*/

	//get proton going

	if (!GetBaseApp()->Init())
	{
		LogMsg("Proton failed to init");
		return 0;
	}

	if (GetFakePrimaryScreenSizeX() == 0)
	{
		//without this, RTBasicApp never sets up its screen.  But RTSimpleApp does, so we don't want this happening with that.
		//Waiting to test on a real device to really nail down rotation/portait mode, it's sort of hardcoded to landscape now.. I think..
		SetupScreenInfo(GetPrimaryGLX(), GetPrimaryGLY(), ORIENTATION_PORTRAIT);
	}

   int gameTimer = 0;

   while(exit_application == 0)
    {
    	//Request and process BPS next available event

	    ProcessEvents();

	    if (exit_application != 0)
	    	{
	    		LogMsg("Exitting main game loop");
	    		break;
	    	}


	    if (fpsTimerLoopMS != 0)
	    		{
	    			while (gameTimer > GetSystemTimeTick())
	    			{
	    				usleep(1); //give control back to the system for a bit
	    			}
	    			gameTimer = GetSystemTimeTick()+fpsTimerLoopMS;
	    		}


	    if (!GetBaseApp()->IsInBackground())
	    {
			if (g_bUsingAccelerometer)
			{
				//poll, and send messages
				double x,y,z;
				if (accelerometer_read_forces(&x, &y, &z) == BPS_SUCCESS)
				{
					CL_Vec3f v = CL_Vec3f(x,y,z);
					GetMessageManager()->SendGUI(MESSAGE_TYPE_GUI_ACCELEROMETER, Variant(v));
				}
			}

			GetBaseApp()->Update();
			GetBaseApp()->Draw();
			bbutil_swap();
	    }

    	while (!GetBaseApp()->GetOSMessages()->empty())
    		{
    			OSMessage m = GetBaseApp()->GetOSMessages()->front();
    			GetBaseApp()->GetOSMessages()->pop_front();
#ifdef _DEBUG
    			LogMsg("Got OS message %d, %s", m.m_type, m.m_string.c_str());
#endif
	 			switch (m.m_type)
    			{
    			case OSMessage::MESSAGE_CHECK_CONNECTION:
    				//pretend we did it
    				GetMessageManager()->SendGUI(MESSAGE_TYPE_OS_CONNECTION_CHECKED, (float)RT_kCFStreamEventOpenCompleted, 0.0f);
    				break;
    			case OSMessage::MESSAGE_OPEN_TEXT_BOX:

    				int height;
    				virtualkeyboard_get_height(&height); //use this later?
    				virtualkeyboard_show();
    				break;

    			case OSMessage::MESSAGE_CLOSE_TEXT_BOX:
    				virtualkeyboard_hide();
    				SetIsUsingNativeUI(false);
    				break;

    			case OSMessage::MESSAGE_SET_FPS_LIMIT:
    				fpsTimerLoopMS = (int) (1000.0f/m.m_x);
    				break;

    			case OSMessage::MESSAGE_SET_ACCELEROMETER_UPDATE_HZ:

					if (!accelerometer_is_supported())
					{
						LogMsg("Ignoring acceleremeter command, device doesn't have one");
					} else
					{
						if (m.m_x != 0)
						{
							g_bUsingAccelerometer = true;
							//turn it on at 40hz.  The API wants enums, not custom settings (?), so I guess I'll just do this.
							accelerometer_set_update_frequency(FREQ_40_HZ);
						} else
						{
							g_bUsingAccelerometer = false;
						}
					}
    				break;
    			}
    		}
    }

   LogMsg("Shutting down in main");


   if (IsBaseAppInitted())
   {
	  // GetBaseApp()->OnEnterBackground();
	   GetBaseApp()->Kill();
   }

	screen_stop_events(screen_cxt);

	//Destroy libscreen context
    screen_destroy_context(screen_cxt);

    //Shut down BPS library for this process
    bps_shutdown();

    //Use utility code to terminate EGL setup
    bbutil_terminate();

    return 0;
}

void ForceVideoUpdate()
{
	g_globalBatcher.Flush();
	bbutil_swap();
}
