#include "BaseApp.h"

#include <vitaGL.h> // GLES1
#include <psp2/kernel/threadmgr.h> // sceKernelDelayThread
#include <psp2/kernel/processmgr.h> // sceKernelExitProcess

#include "psp2Touch.h"

int g_winVideoScreenX = 960;
int g_winVideoScreenY = 544;

#define MEMORY_VITAGL_THRESHOLD_MB 32

int GetPrimaryGLX() 
{
	return g_winVideoScreenX;
}

int GetPrimaryGLY() 
{
	return g_winVideoScreenY;
}

int _newlib_heap_size_user = 200 * 1024 * 1024; // extend the newlib heap.

void check_system_messages()
{
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
}

int main() {

    // Initializing graphics device
	vglInitExtended(0, GetPrimaryGLX(), GetPrimaryGLY(), MEMORY_VITAGL_THRESHOLD_MB * 1024 * 1024, SCE_GXM_MULTISAMPLE_4X);

	// Enabling V-Sync
	vglWaitVblankStart(GL_TRUE);

    // Inform which resolution we're going to use, and what rotation.
    SetupScreenInfo(GetPrimaryGLX(), GetPrimaryGLY(), ORIENTATION_PORTRAIT);

    // This is required for RTDink networking.
    SetEmulatedPlatformID(PLATFORM_ID_PSVITA);

    glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    if (!GetBaseApp()->Init()) sceKernelExitProcess(0);    // Check the logs for the reason of failure.

    // Initialize touch input
    initialize_touch();

    while (true) {

        // Run logic and draw the graphics.
        GetBaseApp()->Update();
		GetBaseApp()->Draw();

        // Performing buffer swap
		vglSwapBuffers(GL_FALSE);

        // Poll for new touch input(s)
        poll_touch();

        // Poll for new system messages
        check_system_messages();

        // Sleep for 1ms, give time for the program to rest.
        sceKernelDelayThread(1000);
    }

    // Terminating graphics device	
	vglEnd();

    // Terminate touch input
    deinitialize_touch();

    sceKernelExitProcess(0);
}