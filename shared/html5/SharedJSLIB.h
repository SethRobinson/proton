#ifndef SharedJSLIB_h__
#define SharedJSLIB_h__
#include "../PlatformEnums.h"

	extern "C" 
	{
		extern void JLIB_OpenURL(const char * URLStr);
		extern char * JLIB_EnterString(const char * message, const char * defaultText);
		extern char * JLIB_GetURL();
		extern void JLIB_OnClickSomethingByID(const char * IDStr);
	}

#endif
