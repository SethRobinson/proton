#include "psp2Touch.h"
#include "BaseApp.h"
#include "util/MathUtils.h"

#include <psp2/touch.h>
#include <psp2/kernel/clib.h>

#define lerp(value, from_max, to_max) ((((value * 10) * (to_max * 10)) / (from_max * 10)) / 10)

SceTouchData last_touch;

void initialize_touch()
{
    sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_START);   
}

void deinitialize_touch()
{
    sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_STOP);
}

void poll_touch()
{
    SceTouchData touch;
    sceTouchPeek(SCE_TOUCH_PORT_FRONT, &touch, 1);

    if(touch.reportNum > 0)
    {
        for(int i = 0; i < touch.reportNum; i++)
        {
            float x = lerp(touch.report[i].x, 1919, 960);
		    float y = lerp(touch.report[i].y, 1087, 544);

            ConvertCoordinatesIfRequired(x, y);

            bool down = false;
            for (int j = 0; j < last_touch.reportNum; j++)
                if (touch.report[i].id == last_touch.report[j].id)
                    down = true;

            if (!down)
                GetMessageManager()->SendGUIEx(MESSAGE_TYPE_GUI_CLICK_START, x, y, i);
            else
                for (int j = 0; j < last_touch.reportNum; j++)
                    if (last_touch.report[j].x != touch.report[i].x || last_touch.report[j].y != touch.report[i].y)
                        GetMessageManager()->SendGUIEx(MESSAGE_TYPE_GUI_CLICK_MOVE, x, y, i);
        }
    }

    if (last_touch.reportNum > 0)
    {
        for(int i = 0; i < last_touch.reportNum; i++)
        {
            bool up = true;
            for (int j = 0; j < touch.reportNum; j++)
                if (touch.report[j].id == last_touch.report[i].id)
                    up = false;

            if (up)
            {
                float x = lerp(last_touch.report[i].x, 1919, 960);
		        float y = lerp(last_touch.report[i].y, 1087, 544);

                ConvertCoordinatesIfRequired(x, y);
                GetMessageManager()->SendGUIEx(MESSAGE_TYPE_GUI_CLICK_END, x, y, i);
            }
        }
    }

    last_touch = touch;
}