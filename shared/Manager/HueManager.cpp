#include "PlatformPrecomp.h"
#include "HueManager.h"
#include "hueplusplus/WinHttpHandler.h"
#include "hueplusplus/BridgeConfig.h"
#include "Entity/EntityUtils.h"
#include <format>
#include "App.h"

HueManager::HueManager()
{
	
}

HueManager::~HueManager()
{
	SAFE_DELETE(m_pBridge);
}
 
void HueManager::Update()
{
}

bool HueManager::Init(string bridgeIP, string userName)
{
	GetApp()->Update();
	ShowTextMessageSimple("Connecting to Hue bridge...", 50);
	GetApp()->Draw();
	ForceVideoUpdate();
	
	/*
	
	LogMsg("Searching for Hue bridges...");

	//let's not, it's slow and making a "user" requires that the user go press the physical button on the bridge, easier to just
	//make one manually and skip all that

	auto handler = std::make_shared<hueplusplus::WinHttpHandler>();
	hueplusplus::BridgeFinder finder(handler);
	std::vector<hueplusplus::BridgeFinder::BridgeIdentification> m_bridges = finder.findBridges();
	if (m_bridges.empty())
	{
		string msg = "No bridges found, ignoring hue stuff";
		LogMsg(msg.c_str());
		ShowTextMessage(msg);
		return true;
	}
	else
	{
		std::string msg = std::format("Found {} hue bridges. Using {}", m_bridges.size(), m_bridges[0].ip);
		LogMsg(msg.c_str());
		ShowTextMessage(msg, 1500, 1000);
		
		hueplusplus::Bridge bridge = finder.getBridge(m_bridges[0]);
		std::vector<hueplusplus::Light> lights = bridge.lights().getAll();
		for (auto& light : lights)
		{
			LogMsg("Found light: %s", light.getName().c_str());
		}

	}

	*/

	handler = std::make_shared<hueplusplus::WinHttpHandler>();

	try
	{
		m_pBridge = new hueplusplus::Bridge(bridgeIP, 80, userName, handler);
	}
	catch (...)
	{
		// Catch any other types of exceptions here
		LogMsg("Caught huelight exception");
		return false;
	}

	try
	{
		lights = m_pBridge->lights().getAll();
	}
	catch (...)
	{
		// Catch any other types of exceptions here
		LogMsg("Caught huelight exception");
		return false;
	}

	for (auto& light : lights)
	{
		LogMsg("Found light: %s", light.getName().c_str());
	}
	m_lightsInitted = true;

	return true;
}

vector<int> HueManager::GetLightIDsByName(string name, bool bAllowPartialMatch)
{
	vector<int> idVec;

	//case insensitive
	for (auto& light : lights)
	{

		//two ways to search, if bAllowPartialMatch is true, we'll allow a partial match (also case insensitive)
		if (bAllowPartialMatch)
		{
			if (light.getName().find(name) != std::string::npos)
			{
				idVec.push_back(light.getId());
			}
		}
		else

		if (_stricmp(light.getName().c_str(), name.c_str()) == 0)
		{
			idVec.push_back(light.getId());
		}
	}

	return idVec;
}

void HueManager::SetLightRGB(string lightName, string rgb, bool bAllowPartialMatch)
{
	
	auto lightIDs = GetLightIDsByName(lightName, bAllowPartialMatch);
	LogMsg("Setting light %s (lights found with this name: %d) to %s", lightName.c_str(), lightIDs.size(), rgb.c_str());
	
	if (!lightIDs.empty())
	{
		//the string rgb will contain something like "255,23,0", let's break it up into three ints
		std::vector<std::string> rgbParts = StringTokenize(rgb, ",");

		if (rgbParts.size() != 3)
		{
			LogMsg("Error: rgb string must be in the format of 255,255,255");
			return;
		}

		//go through the ids and set the rgb of each
		for (auto& id : lightIDs)
		{
			hueplusplus::Light light = m_pBridge->lights().get(id);
			hueplusplus::RGB hueRGB(StringToInt(rgbParts[0]), StringToInt(rgbParts[1]), StringToInt(rgbParts[2]));
			light.setColorRGB(hueRGB, 4);
		
		}

	}
}

void SetHueLightRGB(VariantList *pVList)
{
	string lightName = pVList->Get(0).GetString();
	string rgb = pVList->Get(1).GetString();
	bool bAllowPartialMatch = pVList->Get(2).GetINT32() == 1;
	GetApp()->m_hueManager.SetLightRGB(lightName,rgb, bAllowPartialMatch);
}
