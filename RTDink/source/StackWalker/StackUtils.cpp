#include "StackUtils.h"

#include <tchar.h>
#include <stdio.h>


// For more info about "PreventSetUnhandledExceptionFilter" see:
// "SetUnhandledExceptionFilter" and VC8
// http://blog.kalmbachnet.de/?postid=75
// and
// Unhandled exceptions in VC8 and aboveEfor x86 and x64
// http://blog.kalmbach-software.de/2008/04/02/unhandled-exceptions-in-vc8-and-above-for-x86-and-x64/
// Even better: http://blog.kalmbach-software.de/2013/05/23/improvedpreventsetunhandledexceptionfilter/


class StackWalkerToConsole : public StackWalker
{
protected:
	virtual void OnOutput(LPCSTR szText)
	{
		LogMsg(szText);
	}
};


#if defined _M_X64 || defined _M_IX86
static BOOL PreventSetUnhandledExceptionFilter()
{
	HMODULE hKernel32 = LoadLibrary(_T("kernel32.dll"));
	if (hKernel32 == NULL) return FALSE;
	void *pOrgEntry = GetProcAddress(hKernel32, "SetUnhandledExceptionFilter");
	if (pOrgEntry == NULL) return FALSE;

#ifdef _M_IX86
	// Code for x86:
	// 33 C0                xor         eax,eax  
	// C2 04 00             ret         4 
	unsigned char szExecute[] = { 0x33, 0xC0, 0xC2, 0x04, 0x00 };
#elif _M_X64
	// 33 C0                xor         eax,eax 
	// C3                   ret  
	unsigned char szExecute[] = { 0x33, 0xC0, 0xC3 };
#else
#error "The following code only works for x86 and x64!"
#endif

	DWORD dwOldProtect = 0;
	BOOL bProt = VirtualProtect(pOrgEntry, sizeof(szExecute),
		PAGE_EXECUTE_READWRITE, &dwOldProtect);

	SIZE_T bytesWritten = 0;
	BOOL bRet = WriteProcessMemory(GetCurrentProcess(),
		pOrgEntry, szExecute, sizeof(szExecute), &bytesWritten);

	if ((bProt != FALSE) && (dwOldProtect != PAGE_EXECUTE_READWRITE))
	{
		DWORD dwBuf;
		VirtualProtect(pOrgEntry, sizeof(szExecute), dwOldProtect, &dwBuf);
	}
	return bRet;
}
#else
#pragma message("This code works only for x86 and x64!")
#endif

static BOOL s_bUnhandledExeptionFilterSet = FALSE;
static LONG __stdcall CrashHandlerExceptionFilter(EXCEPTION_POINTERS* pExPtrs)
{
#ifdef _M_IX86
	if (pExPtrs->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW)
	{
		static char MyStack[1024 * 128];  // be sure that we have enought space...
										  // it assumes that DS and SS are the same!!! (this is the case for Win32)
										  // change the stack only if the selectors are the same (this is the case for Win32)
										  //__asm push offset MyStack[1024*128];
										  //__asm pop esp;
		__asm mov eax, offset MyStack[1024 * 128];
		__asm mov esp, eax;
	}
#endif

	LogMsg("DINK CRASHED! STACK TRACE BELOW");
	LogMsg("---------------------------------------------------------");

	StackWalkerToConsole sw;  // output to console
	sw.ShowCallstack(GetCurrentThread(), pExPtrs->ContextRecord);
	
	LogMsg("---------------------------------------------------------");

	TCHAR lString[500];
	
	
	_stprintf_s(lString,
		_T("Dink crashed! Info on where it crashed was written to %slog.txt. If you're a tester, give Seth that file so he can fix it!"),
	GetBaseAppPath().c_str()
	); 


	FatalAppExit(-1, lString);
	return EXCEPTION_CONTINUE_SEARCH;
}

void InitUnhandledExceptionFilter()
{
	/*
	TCHAR szModName[_MAX_PATH];
	if (GetModuleFileName(NULL, szModName, sizeof(szModName) / sizeof(TCHAR)) != 0)
	{
		_tcscpy_s(s_szExceptionLogFileName, szModName);
		_tcscat_s(s_szExceptionLogFileName, _T(".exp.log"));
	}
	*/
	if (s_bUnhandledExeptionFilterSet == FALSE)
	{
		// set global exception handler (for handling all unhandled exceptions)
		SetUnhandledExceptionFilter(CrashHandlerExceptionFilter);
#if defined _M_X64 || defined _M_IX86
		PreventSetUnhandledExceptionFilter();
#endif
		s_bUnhandledExeptionFilterSet = TRUE;
	}
}