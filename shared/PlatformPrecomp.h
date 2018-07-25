#ifndef _PLATFORMPRECOMP_H
#define _PLATFORMPRECOMP_H

#if defined __cplusplus 

	#include "PlatformSetup.h"
	#ifndef _CONSOLE
	#include "BaseApp.h"
	#else
	bool IsTabletSize();
	#endif

#endif

#endif
