#include "main.h"
#include "TexturePacker.h"
#include "FontPacker.h"
#include "FileSystem/FileManager.h"

using namespace std;

App g_App;

App * GetApp() {return &g_App;}
void WaitForKey();

//dummy stuff now used but needed for so things will link when including game library stuff

int GetPrimaryGLX() {return 0;}
int GetPrimaryGLY() {return 0;}

#ifdef _WIN32
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
}
#endif

void LogError ( const char* traceStr, ... )
{
	va_list argsVA;
	const int logSize = 4096;
	char buffer[logSize];
	memset ( (void*)buffer, 0, logSize );

	va_start ( argsVA, traceStr );
#ifdef _WIN32
	vsnprintf_s( buffer, logSize, logSize, traceStr, argsVA );
#else
	vsnprintf( buffer, logSize, traceStr, argsVA );
#endif
	va_end( argsVA );

	LogMsg("ERROR: %s\n", buffer);
	WaitForKey();
}

void WaitForKey()
{
	cout << "\nPlease press [RETURN] to continue: " << flush;
	cin.clear();
	cin.ignore(cin.rdbuf()->in_avail() , '\n');

	cin.get();
	cout << endl;
}



bool App::ParmExists( string parm )
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

bool App::ParmExistsWithData( string parm, string *parmData )
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

std::string App::GetLastParm()
{
	return m_parms[m_parms.size()-1];
}

string App::GetPixelTypeText()
{
	return m_pixelTypeText;
}

void App::SetPixelTypeText( string s )
{
 m_pixelTypeText = s;
}

void App::SetPixelType( pvrtexlib::PixelType ptype )
{
	m_pixelType = ptype;

	switch (ptype)
	{
	case pvrtexlib::OGL_PVRTC2:
		SetPixelTypeText("PVRTC2"); break;

	case pvrtexlib::OGL_PVRTC4:
		SetPixelTypeText("PVRTC4"); break;

	case pvrtexlib::OGL_RGBA_4444:
		SetPixelTypeText("RGBA 16 bit"); break;
	case pvrtexlib::OGL_RGBA_8888:
		SetPixelTypeText("RGBA 32 bit"); break;
	case pvrtexlib::OGL_RGB_565:
		SetPixelTypeText("RGB 16 bit (565)"); break;
	case pvrtexlib::OGL_RGB_888:
		SetPixelTypeText("RGB 24 bit"); break;

	default:

		SetPixelTypeText("Unknown");
	}
}

void ShowHelp()
{
	LogMsg("Help and examples\n");
	LogMsg("RTPack <any file> (Compresses it as an rtpack without changing the name)");
	LogMsg("RTPack -make_font <filename.txt> (Create a .rtfont)");
	LogMsg("RTPack -add_hash_to_text_file <any file> hashlist_to_add_it_to.txt");
	
	LogMsg("");
	LogMsg("More options/flags for making textures:\n");
	LogMsg("RTPack -4444 <image file> (Makes raw rgba 16 bit 4444 or 565 if no alpha .rttex)");
	LogMsg("RTPack -8888 <image file> (Creates raw rgba 32 bit .rttex, or 24 bit if no alpha");
	LogMsg("RTPack -8888 -ultra_compress 90 <image file> (Writes .rttex with good compression when there isn't alpha)");
	LogMsg("RTPack -pvrtc4 <image file> (Makes pvrtc .rttex - for PowerVR chipsets)");
	LogMsg("RTPack -pvrtc2 <image file> (Makes low quality pvrtc .rttex - for PowerVR chipsets)");
	LogMsg("More extra flags you can use with texture generation:");
	LogMsg("-mipmaps (Causes mipmaps to be generated)");
	LogMsg("-stretch (Stretches instead of pads to reach power of 2)");
	LogMsg("-force_square (forces textures to be square in addition to being power of 2)");
	LogMsg("-4444_if_not_square_or_too_big (1024 width or height and non square will use -4444)");
	LogMsg("-8888_if_not_square_or_too_big (1024 width or height and non square will use -8888)");
	LogMsg("-flipv (vertical flip, applies to textures only)");
	LogMsg("-force_alpha (force including the alpha channel, even if its not needed");
	LogMsg("-ultra_compress <0 to 100> (100 is best quality.  only applied to things that DON'T use alpha)");
	LogMsg("-nopowerof2 (stops rtpack from adjusting images to be power of 2)");
	LogMsg("-o <format> Writes final output as a normal image, useful for testing.  Formats can be: png, jpg, or pvr");

}


int main(int argc, char* argv[])
{
	LogMsg("\nRTPack V1.4 by Seth A. Robinson.  /h for help\n");
	
#ifdef _DEBUG
	//printf("Got %d parms:", argc-1);
#endif

	if (argc > 1)
	{
		for (int i=1; i < argc; i++)
		{
#ifdef _DEBUG
			printf(argv[i]);
			printf(" ");
#endif
			g_App.m_parms.push_back(argv[i]);
		}
	}
#ifdef _DEBUG
	
	printf("\n\n");
#endif
	string outputFormat;
	if (GetApp()->ParmExistsWithData("-o", &outputFormat))
	{
		if (outputFormat == "pvr")
		{
			GetApp()->SetOutput(App::PVR);
		}

		if (outputFormat == "png")
		{
			GetApp()->SetOutput(App::PNG);
		}
		if (outputFormat == "jpg")
		{
			GetApp()->SetOutput(App::JPG);
		}
	}

	string qualityLevel;
	if (GetApp()->ParmExistsWithData("-ultra_compress", &qualityLevel))
	{
		int quality = atoi(qualityLevel.c_str());
		if (quality < 1 || quality > 100)
		{
			LogMsg("ERROR:  -ultra_compress has invalid quality level set. Should be 1 to 100. Example: -ultracompress 70");
			WaitForKey();
			return -1;
		}
		GetApp()->SetUltraCompressQuality(quality);
	}
	if (GetApp()->ParmExists("-mipmaps"))
	{
		GetApp()->SetMaxMipLevel(20);
	}
	
	if (GetApp()->ParmExists("-flipv"))
	{
		GetApp()->SetFlipV(true);
	}

	if (GetApp()->ParmExists("-stretch"))
	{
		GetApp()->SetStretchImage(true);
	}

	if (GetApp()->ParmExists("-force_square"))
	{
		GetApp()->SetForceSquare(true);
	}

	if (GetApp()->ParmExists("-8888_if_not_square_or_too_big"))
	{
		GetApp()->SetPixelTypeIfNotSquareOrTooBig(pvrtexlib::OGL_RGBA_8888);
	}

	if (GetApp()->ParmExists("-4444_if_not_square_or_too_big"))
	{
		GetApp()->SetPixelTypeIfNotSquareOrTooBig(pvrtexlib::OGL_RGBA_4444);
	}

	if (GetApp()->ParmExists("/h") || GetApp()->ParmExists("-h") || argc == 1)
	{
		ShowHelp();
	} else

	if (argc == 2)
	{
		//just compress whatever the hell this is
		CompressFile(GetApp()->m_parms[0]);
		return 0;
	}
	
	if (GetApp()->ParmExists("-pvrtc4"))
	{
		GetApp()->SetPixelType(pvrtexlib::OGL_PVRTC4);

	} 
	if (GetApp()->ParmExists("-forcealpha"))
	{
		GetApp()->SetForceAlpha(true);
	} 
	if (GetApp()->ParmExists("-force_alpha"))
	{
		GetApp()->SetForceAlpha(true);
	} 
	if (GetApp()->ParmExists("-nopowerof2"))
	{
		GetApp()->SetNoPowerOfTwo(true);
	} 

	if (GetApp()->ParmExists("-pvrtc2"))
	{
		GetApp()->SetPixelType(pvrtexlib::OGL_PVRTC2);
	
	} 
	if (GetApp()->ParmExists("-pvrt4444") || GetApp()->ParmExists("-4444"))
		{
			GetApp()->SetPixelType(pvrtexlib::OGL_RGBA_4444);
		}
		if (GetApp()->ParmExists("-pvrt8888") || GetApp()->ParmExists("-8888"))
		{
			GetApp()->SetPixelType(pvrtexlib::OGL_RGBA_8888);
		}

		if (GetApp()->GetPixelType() != 0)
		{
			if (GetApp()->m_parms.size() < 2)
			{
				LogError("Aren't you missing the filename?");
			} else
			{
				TexturePacker packer;
				packer.SetCompressionType(GetApp()->GetPixelType());
				packer.ProcessTexture(GetApp()->GetLastParm());
			}
		}

			if (GetApp()->ParmExists("-make_font"))
			{
				if (GetApp()->m_parms.size() < 2)
				{
					LogError("Aren't you missing the filename of the .txt?");
				} else
				{
					FontPacker packer;
					packer.PackFont(GetApp()->GetLastParm());
				}
			}

			if (GetApp()->ParmExists("-add_hash_to_text_file"))
			{
				if (GetApp()->m_parms.size() < 3)
				{
					LogError("Aren't you missing some parms to use -add_hash_to_text_file?");
				} else
				{
					string fileToHash = GetApp()->m_parms[1];
					string fileToAddTo = GetApp()->m_parms[2];
					
					if (StringFromStartMatches(fileToHash, GetBaseAppPath()))
					{
						//remove this part of the path, it's probably C:\PROTON\CRAP\YOURMOM\ or something
						fileToHash = fileToHash.substr(GetBaseAppPath().size(), fileToHash.size()-GetBaseAppPath().size());
					}
					StringReplace("\\", "/", fileToHash); //more portable
					string msg = fileToHash+"|"+toString(GetHashOfFile(fileToHash, false));
					AppendStringToFile(fileToAddTo, msg+"\r\n");
					LogMsg("Added '%s' to %s", msg.c_str(), fileToAddTo.c_str());
				}
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


ePlatformID GetEmulatedPlatformID()
{
	return PLATFORM_ID_WINDOWS;
}

FileManager * GetFileManager()
{
	return NULL;
}

int GetScreenSizeX() {assert(!"Not used"); return 0;}
int GetScreenSizeY() {assert(!"Not used"); return 0;}

bool IsTabletSize() {return false;}