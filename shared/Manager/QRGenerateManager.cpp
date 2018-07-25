#include "PlatformPrecomp.h"
#include "QRGenerateManager.h"

using qrcodegen::QrCode;
using qrcodegen::QrSegment;

// Prints the given QR Code to the console.
vector<string> printQr(const QrCode &qr)
{
	vector<string> lines;
	int border = 4;
	char blockAscii[32];
	sprintf(blockAscii, "%c%c", 219, 219);

	for (int y = -border; y < qr.getSize() + border; y++) 
	{
		string line;
		for (int x = -border; x < qr.getSize() + border; x++)
		{
			line += (qr.getModule(x, y) ? blockAscii : "  ");
		}
		LogMsg(line.c_str());
		lines.push_back(line);
	}

	return lines;
}

QRGenerateManager::QRGenerateManager()
{

	//tests

	/*
	MakeQRWithText("QR codes are cool, dude.", "qr.html");

	
	//Note: The ZBar lib can't always correctly read binary data when decoding, so probably better to base64 your stuff
	string data;
	data.resize(10);
	data[0] = 88;
	data[1] = 89;

	MakeQRWithData(data, "qr.html");
	*/
}

vector<string> QRGenerateManager::PrintAndWriteHTMLIfNeeded(const QrCode qr, string optionalHTMLFileName)
{
	
	if (!optionalHTMLFileName.empty())
	{

		string html = qr.toSvgString(4).c_str();

		//write html version
		string htmlFileName = optionalHTMLFileName;
		RemoveFile(htmlFileName);
		
		//oh, let's add one thing so it renders betterL

		StringReplace("stroke=\"none\"", "stroke=\"none\" shape-rendering=\"crispEdges\"", html);
		
		
		AppendStringToFile(htmlFileName, html);

		//show html version in log
		// StringReplace("%", "%%", html);
		// LogMsg(html.c_str());
		
	}

	return printQr(qr);
}

vector<string> QRGenerateManager::MakeQRWithText(string msg, string optionalHTMLFileName)
{
	const QrCode::Ecc errCorLvl = QrCode::Ecc::LOW;  // Error correction level

	try {
		const QrCode qr = QrCode::encodeText(msg.c_str(), errCorLvl);
		return PrintAndWriteHTMLIfNeeded(qr, optionalHTMLFileName);
	} catch (exception& error)
	{
		LogMsg("Error creating QR: %s", error.what());
	}

	return vector<string>();
}

vector<string> QRGenerateManager::MakeQRWithData(string msg, string optionalHTMLFileName)
{
	const std::vector<uint8_t> vecData(msg.begin(), msg.end());
	const QrCode::Ecc errCorLvl = QrCode::Ecc::LOW;  // Error correction level
	const QrCode qr = QrCode::encodeBinary(vecData, errCorLvl);
	return PrintAndWriteHTMLIfNeeded(qr, optionalHTMLFileName);

}

QRGenerateManager::~QRGenerateManager()
{
}