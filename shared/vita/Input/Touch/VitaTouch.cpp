#include "PlatformPrecomp.h"
#include "BaseApp.h"
#include "VitaTouch.h"

#define lerp(value, from_max, to_max) ((((value * 10) * (to_max * 10)) / (from_max * 10)) / 10)

VitaTouch::VitaTouch()
{
    sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_START);
}

VitaTouch::~VitaTouch()
{
    sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_STOP);
}

void VitaTouch::Update()
{
    memcpy(&m_TouchPrevious, &m_Touch, sizeof(m_TouchPrevious));
    if(sceTouchPeek(SCE_TOUCH_PORT_FRONT, &m_Touch, 1) < 0)
        LogMsg("Couldnt't retrieve touch data..");

    if(m_Touch.reportNum > 0)
    {
        for(int i = 0; i < m_Touch.reportNum; i++)
        {
            float x = lerp(m_Touch.report[i].x, 1919, 960);
		    float y = lerp(m_Touch.report[i].y, 1087, 544);

            ConvertCoordinatesIfRequired(x, y);

            bool FingerDown = false;
            for(int j = 0; j < m_TouchPrevious.reportNum; j++)
                if (m_Touch.report[i].id == m_TouchPrevious.report[j].id)
                    FingerDown = true;

            if(!FingerDown)
                GetMessageManager()->SendGUIEx(MESSAGE_TYPE_GUI_CLICK_START, x, y, i);
            else
                for(int j = 0; j < m_TouchPrevious.reportNum; j++)
                    if(m_TouchPrevious.report[j].x != m_Touch.report[i].x || m_TouchPrevious.report[j].y != m_Touch.report[i].y)
                        GetMessageManager()->SendGUIEx(MESSAGE_TYPE_GUI_CLICK_MOVE, x, y, i);
        }
    }

    if(m_TouchPrevious.reportNum > 0)
    {
        for(int i = 0; i < m_TouchPrevious.reportNum; i++)
        {
            bool FingerUp = true;
            for(int j = 0; j < m_Touch.reportNum; j++)
                if (m_Touch.report[j].id == m_TouchPrevious.report[i].id)
                    FingerUp = false; //we are still holding!

            if(FingerUp == true)
            {
                float x = lerp(m_TouchPrevious.report[i].x, 1919, 960);
		        float y = lerp(m_TouchPrevious.report[i].y, 1087, 544);

                ConvertCoordinatesIfRequired(x, y);
                GetMessageManager()->SendGUIEx(MESSAGE_TYPE_GUI_CLICK_END, x, y, i);
            }
        }
    }
}
