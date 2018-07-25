//  ***************************************************************
//  App - Creation date: 04/19/2012
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2012 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef App_h__
#define App_h__

#define RT_ENABLE_FILEMANAGER

class FileManager;
FileManager * GetFileManager();

class App
{
public:
	App();
	virtual ~App();

	bool Init();
	bool Update();
	void Kill();

protected:
	

private:
};

App * GetApp();

#endif // App_h__