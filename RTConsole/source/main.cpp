#include "main.h"
#include "App.h"
#include "util/MiscUtils.h"

MainHarness g_mainHarness;

App g_App;

App * GetApp() {return &g_App;}

void WaitForKey();

//dummy stuff now used but needed for so things will link when including game library stuff
 
int GetPrimaryGLX() {return 0;}
int GetPrimaryGLY() {return 0;}

void AppendStringToFile(string filename, string text);
string GetDateAndTimeAsString();

#ifdef WINAPI

void LogMsg ( const char* traceStr, ... )
{
	va_list argsVA;
	const int logSize = 4096;
	char buffer[logSize];
	memset ( (void*)buffer, 0, logSize );

	va_start ( argsVA, traceStr );
	vsnprintf_s( buffer, logSize, logSize, traceStr, argsVA );
	va_end( argsVA );
	OutputDebugString(buffer);
	OutputDebugString("\n");
	printf(buffer);
	printf("\n");
	AppendStringToFile( GetBaseAppPath()+"log.txt", GetDateAndTimeAsString()+": "+string(buffer)+"\r\n");
}

void LogError ( const char* traceStr, ... )
{
	va_list argsVA;
	const int logSize = 4096;
	char buffer[logSize];
	memset ( (void*)buffer, 0, logSize );

	va_start ( argsVA, traceStr );
	vsnprintf_s( buffer, logSize, logSize, traceStr, argsVA );
	va_end( argsVA );
	LogMsg("ERROR: %s\n", buffer);
	//WaitForKey();
}


#else


void LogError ( const char* traceStr, ... )
{
	va_list argsVA;
	const int logSize = 4096;
	char buffer[logSize];
	memset ( (void*)buffer, 0, logSize );

	va_start ( argsVA, traceStr );
	vsnprintf( buffer, logSize, traceStr, argsVA );
	va_end( argsVA );

	printf ((char*)buffer);
	printf ("\r\n");
	fflush(stdout);

	AppendStringToFile( GetBaseAppPath()+"log.txt", GetDateAndTimeAsString()+": "+string(buffer)+"\r\n");
}

#endif


void WaitForKey()
{
	cout << "\nPlease press [RETURN] to continue: " << flush;
	cin.clear();
	cin.ignore(cin.rdbuf()->in_avail() , '\n');

	cin.get();
	cout << endl;
}


bool MainHarness::ParmExists( string parm )
{
	for (unsigned int i=0; i < m_parms.size(); i++)
	{
		if (ToLowerCaseString(parm) == ToLowerCaseString(m_parms[i]))
		{
			return true;
		}
	}

	return false;
}

bool MainHarness::ParmExistsWithData( string parm, string *parmData )
{
	for (unsigned int i=0; i < m_parms.size(); i++)
	{
		if (ToLowerCaseString(parm) == ToLowerCaseString(m_parms[i]))
		{
			if (i >= m_parms.size())
			{
				//no more parms to check, so no data for you
				return false;
			}
			*parmData = m_parms[i+1];
			return true;
		}
	}

	return false;
}

std::string MainHarness::GetLastParm()
{
	return m_parms[m_parms.size()-1];
}

uint32 MainHarness::GetTick()
{
	return GetSystemTimeTick();
}

#ifdef RTLINUX

/*
void term(int signum)
{
	LogError("Sigterm %d receieved.  But why?!  Ignoring.", signum);
}

*/
#endif

#ifdef WINAPI

#include <direct.h>

string GetExePath()
{
	
	// Get path to executable:
	TCHAR szDllName[_MAX_PATH];
	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	TCHAR szFilename[256];
	TCHAR szExt[256];
	GetModuleFileName(0, szDllName, _MAX_PATH);
	_splitpath(szDllName, szDrive, szDir, szFilename, szExt);

	return string(szDrive) + string(szDir); 
}
#endif

int main(int argc, char* argv[])
{
	

#ifdef WINAPI
	srand( (unsigned)GetTickCount() );
	_chdir(GetExePath().c_str());

#endif

#ifdef RTLINUX
	srand( (unsigned)time(NULL) );
	//signal(SIGTERM, term);

#endif

	if (argc > 1)
	{
		for (int i=1; i < argc; i++)
		{
#ifdef _DEBUG
			printf(argv[i]);
			printf(" ");
#endif

			g_mainHarness.m_parms.push_back(argv[i]);
		}
	}
#ifdef _DEBUG

	printf("\n\n");
#endif
	
	if (g_mainHarness.ParmExists("-debug"))
	{
		LogMsg("Debug mode!");
	}

	if (!g_App.Init())
	{
		LogError("Couldn't init app");
		return -1;
	}

	while (GetApp()->Update())
	{
#ifdef WINAPI
		if (GetAsyncKeyState(VK_ESCAPE)) break;
#endif

#ifdef WINAPI
		Sleep(1);
#else
usleep(1000*1);
#endif
	}

#ifdef _DEBUG
		WaitForKey();
#endif
	return 0;
}


bool IsLargeScreen()
{
	return true; 
}

bool IsTabletSize()
{
	return false;
}


#ifdef RTLINUX

/*
void term(int signum)
{
	LogError("Sigterm %d receieved.  But why?!  Ignoring.", signum);
}

*/

#endif