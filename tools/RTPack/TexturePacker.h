//  ***************************************************************
//  TexturePacker - Creation date: 03/25/2009
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef TexturePacker_h__
#define TexturePacker_h__
#include "main.h"

class TexturePacker
{
public:
	TexturePacker();
	virtual ~TexturePacker();

	bool ProcessTexture(std::string fName);
	void SetCompressionType(pvrtexlib::PixelType pix) {m_pixType = pix;}

protected:
	
	bool m_bUsesTransparency;
	pvrtexlib::PixelType m_pixType;

private:
};


#endif // TexturePacker_h__