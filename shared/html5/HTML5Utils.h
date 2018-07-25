//  ***************************************************************
//  HTML5Utils - Creation date: 06/6/2015
//  -------------------------------------------------------------
//  Robinson Technologies - Check license.txt for license info.
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef HTML5Utils_h__
#define HTML5Utils_h__
#include "../PlatformEnums.h"
const char * GetAppName();
void HTMLDownloadFileFromFileSystem(std::string sourceFile, std::string fileToWrite);
void HTMLUploadFileToFileSystem();

#endif // HTML5Utils_h__
