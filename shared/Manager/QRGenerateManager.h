//  ***************************************************************
//  QRGenerateManager - Creation date: 07/19/2018
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2018 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef QRGenerateManager_h__
#define QRGenerateManager_h__

#include "util/QR-Code-generator/cpp/BitBuffer.hpp"
#include "util/QR-Code-generator/cpp/QrCode.hpp"

//To use this class, you need to add the following cpp files:  BitBUffer.cpp, QrCode.cpp, QrSegment.cpp
class QRGenerateManager
{
public:
	QRGenerateManager();
	virtual ~QRGenerateManager();

	vector<string> MakeQRWithText(string msg, string optionalHTMLFileName = "");

	vector<string> MakeQRWithData(string msg, string optionalHTMLFileName);
private:
	vector<string> PrintAndWriteHTMLIfNeeded(const qrcodegen::QrCode qr, string optionalHTMLFileName = "");
};

#endif // QRGenerateManager_h__