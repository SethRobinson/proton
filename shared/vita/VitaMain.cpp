#include "BaseApp.h"
#include "VitaUtils.h"
#include <psp2/ctrl.h>

#include "Input/Touch/VitaTouch.h"

int g_winVideoScreenX = 960;
int g_winVideoScreenY = 544;

int GetPrimaryGLX() 
{
	return g_winVideoScreenX;
}

int GetPrimaryGLY() 
{
	return g_winVideoScreenY;
}

VitaTouch g_VitaTouch;

int main()
{
	vglInit(0x800000);
    
	SetEmulatedPlatformID(PLATFORM_ID_PSVITA);
	SetForcedOrientation(ORIENTATION_PORTRAIT);
	GetBaseApp()->OnPreInitVideo();
	SetupScreenInfo(GetPrimaryGLX(), GetPrimaryGLY(), ORIENTATION_PORTRAIT);

	glClearColor(0,0,0,1);
	glClear(GL_COLOR_BUFFER_BIT);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	if(!GetBaseApp()->Init())
	{
		sceKernelExitProcess(0);
	}
    
	SceCtrlData ctrlData;
	sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);

	do
	{
		sceCtrlPeekBufferPositive(0, &ctrlData, 1);

		GetBaseApp()->Update();
		GetBaseApp()->Draw();

		while (!GetBaseApp()->GetOSMessages()->empty())
		{
			OSMessage m = GetBaseApp()->GetOSMessages()->front();
			GetBaseApp()->GetOSMessages()->pop_front();

			switch (m.m_type)
			{
				case OSMessage::MESSAGE_CHECK_CONNECTION:
					GetMessageManager()->SendGUI(MESSAGE_TYPE_OS_CONNECTION_CHECKED, (float)RT_kCFStreamEventOpenCompleted, 0.0f);	
					break;
			}
		}

		g_VitaTouch.Update();
		vglSwapBuffers(GL_FALSE);

	} while(ctrlData.buttons != SCE_CTRL_PSBUTTON); //kill switch.
	
	GetBaseApp()->Kill();
	vglEnd();

	sceKernelExitProcess(0);
}