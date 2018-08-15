#include "PlatformPrecomp.h"
#include "App.h"
#include "util/MiscUtils.h"
#include "util/ResourceUtils.h"
#include "util/Variant.h"
#include "FileSystem/FileManager.h"
#include "util/TextScanner.h"
#include "FontPacker.h"

extern MainHarness g_mainHarness;

FileManager g_fileManager;

FileManager * GetFileManager()
{
	return &g_fileManager;
}

App::App()
{

	m_bForceAlpha = false;
	m_bNoPowerOf2 = false;
	m_output = RTTEX;
	SetPixelType(pvrtexlib::PixelType(0));
	SetPixelTypeIfNotSquareOrTooBig(pvrtexlib::PixelType(0));
	SetMaxMipLevel(1);
	SetStretchImage(false);
	SetForceSquare(false);
	SetFlipV(false);
	m_ultraCompressQuality = 0;
}

App::~App()
{
}

bool App::Init()
{
	return true;
}

string App::GetPixelTypeText()
{
	return m_pixelTypeText;
}

void App::SetPixelTypeText(string s)
{
	m_pixelTypeText = s;
}

void App::SetPixelType(pvrtexlib::PixelType ptype)
{
	m_pixelType = ptype;

	switch (ptype)
	{
#ifndef RT_NO_PVR

	case pvrtexlib::OGL_PVRTC2:
		SetPixelTypeText("PVRTC2"); break;

	case pvrtexlib::OGL_PVRTC4:
		SetPixelTypeText("PVRTC4"); break;
#endif
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
	LogMsg("RTPack -8888 <image file> (Creates raw rgba 32 bit .rttex, or 24 bit if no alpha");
	LogMsg("RTPack -8888 -ultra_compress 90 <image file> (Writes .rttex with jpg compression when there isn't alpha)");
	
	
#ifndef RT_NO_PVR
	LogMsg("RTPack -4444 <image file> (Makes raw rgba 16 bit 4444 or 565 if no alpha .rttex)");
	LogMsg("RTPack -pvrtc4 <image file> (Makes pvrtc .rttex - for PowerVR chipsets)");
	LogMsg("RTPack -pvrtc2 <image file> (Makes low quality pvrtc .rttex - for PowerVR chipsets)");
	LogMsg("-4444_if_not_square_or_too_big (1024 width or height and non square will use -4444)");
#endif
	
	LogMsg("More extra flags you can use with texture generation:");
	LogMsg("-mipmaps (Causes mipmaps to be generated)");
	LogMsg("-stretch (Stretches instead of pads to reach power of 2)");
	LogMsg("-force_square (forces textures to be square in addition to being power of 2)");
	LogMsg("-8888_if_not_square_or_too_big (1024 width or height and non square will use -8888)");
	LogMsg("-flipv (vertical flip, applies to textures only)");
	LogMsg("-force_alpha (force including the alpha channel, even if its not needed");
	LogMsg("-ultra_compress <0 to 100> (100 is best quality.  only applied to things that DON'T use alpha)");
	LogMsg("-nopowerof2 (stops rtpack from adjusting images to be power of 2)");
	LogMsg("-o <format> Writes final output as a normal image, useful for testing.  Formats can be: bmp jpg");

}

bool App::Update()
{
	string mode = "(compiled with PVRTC support)";
#ifdef RT_NO_PVR
	mode = "(compiled without PVRTC support)";
#endif
	LogMsg("\nRTPack V1.5 by Seth A. Robinson. %s /h for help\n", mode.c_str());

	string outputFormat;
	if (g_mainHarness.ParmExistsWithData("-o", &outputFormat))
	{
		if (outputFormat == "pvr")
		{
#ifdef RT_NO_PVR
			LogMsg("Can't output as PVR, this was compiled with RT_NO_PVR.");
			WaitForKey();
			return false;
#endif
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
		if (outputFormat == "bmp")
		{
			GetApp()->SetOutput(App::BMP);
		}
	}

	string qualityLevel;
	if (g_mainHarness.ParmExistsWithData("-ultra_compress", &qualityLevel))
	{
		int quality = atoi(qualityLevel.c_str());
		if (quality < 1 || quality > 100)
		{
			LogMsg("ERROR:  -ultra_compress has invalid quality level set. Should be 1 to 100. Example: -ultracompress 70");
			WaitForKey();
			return false;
		}

		GetApp()->SetUltraCompressQuality(quality);
	}
	if (g_mainHarness.ParmExists("-mipmaps"))
	{
		GetApp()->SetMaxMipLevel(20);
	}

	if (g_mainHarness.ParmExists("-flipv"))
	{
		GetApp()->SetFlipV(true);
	}

	if (g_mainHarness.ParmExists("-stretch"))
	{
		GetApp()->SetStretchImage(true);
	}

	if (g_mainHarness.ParmExists("-force_square"))
	{
		GetApp()->SetForceSquare(true);
	}

	if (g_mainHarness.ParmExists("-8888_if_not_square_or_too_big"))
	{
		GetApp()->SetPixelTypeIfNotSquareOrTooBig(pvrtexlib::OGL_RGBA_8888);
	}

	if (g_mainHarness.ParmExists("-4444_if_not_square_or_too_big"))
	{
		GetApp()->SetPixelTypeIfNotSquareOrTooBig(pvrtexlib::OGL_RGBA_4444);
	}

	int argc = g_mainHarness.GetParmCount();

	if (g_mainHarness.ParmExists("/h") || g_mainHarness.ParmExists("-h") || argc < 1)
	{
		ShowHelp();
	}
	else
		 
		if (argc == 1)
		{
			//just compress whatever the hell this is
			CompressFile(g_mainHarness.m_parms[0]);
			return 0;
		}

#ifndef RT_NO_PVR
	if (g_mainHarness.ParmExists("-pvrtc4"))
	{
		GetApp()->SetPixelType(pvrtexlib::OGL_PVRTC4);

	}
	if (g_mainHarness.ParmExists("-pvrtc2"))
	{
		GetApp()->SetPixelType(pvrtexlib::OGL_PVRTC2);

	}
	if (g_mainHarness.ParmExists("-pvrt4444") || g_mainHarness.ParmExists("-4444"))
	{
		GetApp()->SetPixelType(pvrtexlib::OGL_RGBA_4444);
	}
#endif

	if (g_mainHarness.ParmExists("-forcealpha"))
	{
		GetApp()->SetForceAlpha(true);
	}
	if (g_mainHarness.ParmExists("-force_alpha"))
	{
		GetApp()->SetForceAlpha(true);
	}
	if (g_mainHarness.ParmExists("-nopowerof2"))
	{
		GetApp()->SetNoPowerOfTwo(true);
	}

	
	if (g_mainHarness.ParmExists("-pvrt8888") || g_mainHarness.ParmExists("-8888"))
	{
		GetApp()->SetPixelType(pvrtexlib::OGL_RGBA_8888);
	}

	if (GetApp()->GetPixelType() != 0)
	{
		if (g_mainHarness.GetParmCount() < 2)
		{
			LogError("Aren't you missing the filename?");
		}
		else
		{
			TexturePacker packer;
			packer.SetCompressionType(GetApp()->GetPixelType());
			packer.ProcessTexture(g_mainHarness.GetLastParm());
		}
	}

	if (g_mainHarness.ParmExists("-make_font"))
	{
		if (g_mainHarness.m_parms.size() < 2)
		{
			LogError("Aren't you missing the filename of the .txt?");
		}
		else
		{
			FontPacker packer;
			packer.PackFont(g_mainHarness.GetLastParm());
		}
	}

	if (g_mainHarness.ParmExists("-add_hash_to_text_file"))
	{
		if (g_mainHarness.m_parms.size() < 3)
		{
			LogError("Aren't you missing some parms to use -add_hash_to_text_file?");
		}
		else
		{
			string fileToHash = g_mainHarness.m_parms[1];
			string fileToAddTo = g_mainHarness.m_parms[2];

			if (StringFromStartMatches(fileToHash, GetBaseAppPath()))
			{
				//remove this part of the path, it's probably C:\PROTON\CRAP\YOURMOM\ or something
				fileToHash = fileToHash.substr(GetBaseAppPath().size(), fileToHash.size() - GetBaseAppPath().size());
			}
			StringReplace("\\", "/", fileToHash); //more portable
			string msg = fileToHash + "|" + toString(GetHashOfFile(fileToHash, false));
			AppendStringToFile(fileToAddTo, msg + "\r\n");
			LogMsg("Added '%s' to %s", msg.c_str(), fileToAddTo.c_str());
		}
	}


	return false; //false signals we're done
}

void App::Kill()
{
}
