#include "PlatformPrecomp.h"
#include "App.h"
#include "util/MiscUtils.h"
#include "util/ResourceUtils.h"
#include "util/Variant.h"

#include "FileSystem/FileManager.h"
#include "util/TextScanner.h"


FileManager g_fileManager;

FileManager * GetFileManager()
{
	return &g_fileManager;
}

App::App()
{
}

App::~App()
{
}

bool App::Init()
{
	return true;
}


bool App::Update()
{
	
	LogMsg("-- Proton RTConsole test --");
	LogMsg("");

	Variant v("-- C++/Template/Variant Test");
	LogMsg(v.Print().c_str());
	LogMsg("Success");

	TextScanner s;

	s.LoadFile("interface/test.txt");
	s.DumpToLog();

	return false; //false signals we're done
}

void App::Kill()
{
}
