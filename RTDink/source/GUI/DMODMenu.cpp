#include "PlatformPrecomp.h"
#include "DMODMenu.h"
#include "MainMenu.h"
#include "DMODInstallMenu.h"
#include "GameMenu.h"
#include "Entity/EntityUtils.h"
#include "../dink/dink.h"
#include "PopUpMenu.h"
#include "util/TextScanner.h"
#include "EnterURLMenu.h"
#include "Renderer/SoftSurface.h"
#include "QuickTipMenu.h"
#include "BrowseMenu.h"
#include "ReadTextMenu.h"



void DMODMenuAddScrollContent(Entity *pParent);

void DMODMenuOnRemoveDMOD(VariantList *pVList)
{
	Entity *pMenu = pVList->Get(0).GetEntity();

	string dmodDirToDelete = pMenu->GetVar("dmodDirToDelete")->GetString();

	if (!dmodDirToDelete.empty())
	{
		RemoveDirectoryRecursively(dmodDirToDelete);
		DMODMenuAddScrollContent(pMenu);
	}
	LogMsg("Removing DMOD");
}

void DMODMenuOnSessionNew(VariantList *pVList)
{
	Entity *pMenu = pVList->Get(0).GetEntity();
	
	DisableAllButtonsEntity(pMenu);
	SlideScreen(pMenu, false);
	GetMessageManager()->CallEntityFunction(pMenu, 500, "OnDelete", NULL);
	GameCreate(pMenu->GetParent(), 0, "");
}

void DMODMenuOnSessionContinue(VariantList *pVList)
{
	Entity *pMenu = pVList->Get(0).GetEntity();

	DisableAllButtonsEntity(pMenu);
	SlideScreen(pMenu, false);
	GetMessageManager()->CallEntityFunction(pMenu, 500, "OnDelete", NULL);
	GameCreate(pMenu->GetParent(), 0, g_dglo.m_savePath+"continue_state.dat");
}

void DMODMenuOnSelect(VariantList *pVList) //0=vec2 point of click, 1=entity sent from
{
	Entity *pEntClicked = pVList->m_variant[1].GetEntity();
	LogMsg("Clicked %s entity at %s", pEntClicked->GetName().c_str(),pVList->m_variant[1].Print().c_str());
	Entity *pMenu = GetEntityRoot()->GetEntityByName("DMODMenu");

	//LogMsg("Clicked %s entity at %s", pEntClicked->GetName().c_str(),pVList->m_variant[0].Print().c_str());
	
	if (pEntClicked->GetName() == "Back")
	{
		DisableAllButtonsEntity(pMenu);
		SlideScreen(pEntClicked->GetParent(), false);
		GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 500, "OnDelete", NULL);
		MainMenuCreate(pEntClicked->GetParent()->GetParent());
		return;
	}
		
	if (pEntClicked->GetName() == "getmore")
	{
		DisableAllButtonsEntity(pMenu);
		SlideScreen(pEntClicked->GetParent(), false);
		GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 500, "OnDelete", NULL);
		EnterURLMenuCreate(pEntClicked->GetParent()->GetParent());
		return;
	}

	if (pEntClicked->GetName() == "browse")
	{
		DisableAllButtonsEntity(pMenu);
		SlideScreen(pEntClicked->GetParent(), false);
		GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 500, "OnDelete", NULL);
		BrowseMenuCreate(pEntClicked->GetParent()->GetParent());
		return;
	}

	//they must have clicked on a DMOD if they got this far
	if (pEntClicked->GetName() == "icon_hotspot")
	{
		//save position of the scroll bar first
	
		string dmoddir = pEntClicked->GetParent()->GetVar("dmodgamedir")->GetString();

		SendFakeInputMessageToEntity(GetEntityRoot(), MESSAGE_TYPE_GUI_CLICK_END, pVList->m_variant[0].GetVector2()); //otherwise the menu may never get the touch release message

		//first check to see if it's a valid dmod

		if (!FileExists(dmoddir+"/dmod.diz"))
		{
			//don't look valid..
			PopUpCreate(pMenu, "This add-on appears to be missing or damaged.  Delete and re-install.", "", "cancel", "Continue", "", "", true);
			return;
		}
		
		InitDinkPaths(GetBaseAppPath(), "dink", dmoddir);

		pMenu = GetEntityRoot()->GetEntityByName("DMODMenu");

		//next, see if there is a save-on-quit save state existing for this dmod
		if (FileExists(g_dglo.m_savePath+"/continue_state.dat"))
		{
			PopUpCreate(pMenu, "Continue your last playing session?", "", "cancel", "Abort", "SessionContinue", "Continue", true,"SessionNew", "No" );
			return;
		}

		DisableAllButtonsEntity(pMenu);
		SlideScreen(pMenu, false);
		GetMessageManager()->CallEntityFunction(pMenu, 500, "OnDelete", NULL);
		GameCreate(pMenu->GetParent(), 0, "");
	}

	if (pEntClicked->GetName() == "delete_hotspot")
	{
	
		EntityComponent *pScrollerComp = pMenu->GetEntityByName("scroll")->GetComponentByName("Scroll");
		GetApp()->GetVar("DMODProgress2d")->Set(pScrollerComp->GetVar("progress2d")->GetVector2());

		string dmoddir = pEntClicked->GetParent()->GetVar("dmodgamedir")->GetString();
		string dmodtitle = pEntClicked->GetParent()->GetVar("dmodtitle")->GetString();
		SendFakeInputMessageToEntity(GetEntityRoot(), MESSAGE_TYPE_GUI_CLICK_END, pVList->m_variant[0].GetVector2()); //otherwise the menu may never get the touch release message
	
		if (!dmoddir.empty())
		{
			Entity *pMenu = GetEntityRoot()->GetEntityByName("DMODMenu");
			pMenu->GetVar("dmodDirToDelete")->Set(dmoddir);
			PopUpCreate(pMenu, "Remove "+dmodtitle+" and its saved games?", "", "cancel", "Cancel", "RemoveDMOD", "Remove", true);
		}
	}

	if (pEntClicked->GetName() == "view_info")
	{
		DisableAllButtonsEntity(pMenu);
		SlideScreen(pMenu, false);
		GetMessageManager()->CallEntityFunction(pMenu, 500, "OnDelete", NULL);
	
		GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 500, "OnDelete", NULL);
		string textfile = pEntClicked->GetParent()->GetVar("textfile")->GetString();
		ReadTextMenuCreate(pMenu->GetParent(), textfile, "file");
	}

/*
	if (pEntClicked->GetName() == "rtsoft")
	{
		PopUpCreate(pEntClicked->GetParent()->GetParent()->GetParent(), "Leave the game and visit `wrtsoft.com``?", "http://www.rtsoft.com/iphone",
			"cancel", "`wCancel", "url", "`wLaunch", true);
		return;
	}
*/	

	//GetEntityRoot()->PrintTreeAsText(); //useful for debugging
}

CL_Vec2f GetDMODBarIconOffset()
{
	if (IsLargeScreen())
	{
		return CL_Vec2f(44,20); //fix a slight offset problem
	}

	return iPhoneMap2X(23,11);
}

CL_Vec2f GetDMODBarIconSize()
{
	return iPhoneMap2X(99, 74);
}

void DMODSetupExtra(VariantList *pVList)
{
	Entity *pBG = pVList->Get(0).GetEntity();

	//locate the file
	string dmodDir = pBG->GetVar("dmodgamedir")->GetString()+"/";
	string iconFile = dmodDir+"preview.bmp";
	
	if (!FileExists(iconFile))
	{
		//try again
		iconFile = dmodDir+"graphics/title-01.bmp";
	
		if (!FileExists(iconFile))
		{
			iconFile = dmodDir+"graphics/title/title-01.bmp";
		}
	}

	if (!FileExists(iconFile))
	{
		//give up, nothing to show
		return;
	}
	
	SoftSurface s8bit;
	if (!s8bit.LoadFile(iconFile, SoftSurface::COLOR_KEY_NONE, false))
	{
		return;
	}

	//if it was 8bit, this will convert it to 32

	SoftSurface s;
	s.Init(s8bit.GetWidth(),s8bit.GetHeight(), SoftSurface::SURFACE_RGBA);
	s.Blit(0,0, &s8bit);
	s.FlipY();

	SurfaceAnim *pSurf;

	pSurf = new SurfaceAnim;
	
	pSurf->SetTextureType(Surface::TYPE_DEFAULT); //insure no mipmaps are created
	pSurf->InitBlankSurface(s.GetWidth(),s.GetHeight());
	pSurf->UpdateSurfaceRect(rtRect(0,0, s.GetWidth(), s.GetHeight()), s.GetPixelData());
	
	//add the icon
	Entity *pEnt = CreateOverlayEntity(pBG, "icon", "", GetDMODBarIconOffset().x,GetDMODBarIconOffset().y);
	OverlayRenderComponent *pOverlay = (OverlayRenderComponent*) pEnt->GetComponentByName("OverlayRender");
	pOverlay->SetSurface(pSurf, true);

	EntitySetScaleBySize(pEnt,GetDMODBarIconSize());
}


void AddDMODBar(Entity *pParent, float &x, float &y, string title, string description, string iconFileName, float dmodSize, string dmodgamedir, int count)
{
	Entity *pBG = CreateOverlayEntity(pParent, dmodgamedir, ReplaceWithLargeInFileName("interface/iphone/dmod_bar.rttex"), iPhoneMapX(x), iPhoneMapY(y));
	
	if (IsLargeScreen())
	{
		y += pBG->GetVar("size2d")->GetVector2().y*.4f;
	} else
	{
		y += pBG->GetVar("size2d")->GetVector2().y;
	}
	
	pBG->GetVar("dmodgamedir")->Set(dmodgamedir); //save for later
	pBG->GetVar("dmodtitle")->Set(title); //save for later
	//title
	Entity *pTitle = CreateTextLabelEntity(pBG, "title", iPhoneMapX2X( 130) ,iPhoneMapY2X( 10), title);
	Entity *pDescription = CreateTextBoxEntity(pBG, "descrip", iPhoneMap2X(129, 33), iPhoneMap2X(277, 53), description, 0.7f);

	Entity *pIcon = CreateButtonHotspot(pBG, "icon_hotspot", GetDMODBarIconOffset(), GetDMODBarIconSize(), Button2DComponent::BUTTON_STYLE_CLICK_ON_TOUCH_IGNORE_DRAGGING);
	SetTouchPaddingEntity(pIcon, CL_Rectf(0,iPhoneMapY2X(5),0,iPhoneMapY2X(5)));
	pIcon->GetFunction("OnButtonSelected")->sig_function.connect(&DMODMenuOnSelect);

	//processing the icon image might be slow, lets do it a bit later, sequencing the timing by using the y, which should be going up
	pBG->GetFunction("SetupExtra")->sig_function.connect(&DMODSetupExtra);
    VariantList vList(pBG);
	GetMessageManager()->CallEntityFunction(pBG, 300+count*50, "SetupExtra", &vList); 

	//the delete icon part
bool bCanDelete = true;

if (!GetDMODStaticRootPath().empty())
{
	if (GetPathFromString(dmodgamedir) == GetDMODStaticRootPath())
	{
		//we better not let people delete this..
		bCanDelete = false;
	}
}

if (bCanDelete)
{
		CL_Vec2f iconPos = iPhoneMap2X(408,10);
		//Entity *pIcon = CreateButtonHotspot(pBG, "delete_hotspot", iconPos,  Button2DComponent::BUTTON_STYLE_CLICK_ON_TOUCH_IGNORE_DRAGGING);
		Entity *pIcon = CreateOverlayButtonEntity(pBG, "delete_hotspot", ReplaceWithLargeInFileName("interface/iphone/dmod_delete_button.rttex"), iconPos.x, iconPos.y);
		SetTouchPaddingEntity(pIcon, CL_Rectf(0,0,0,0));
		pIcon->GetFunction("OnButtonSelected")->sig_function.connect(&DMODMenuOnSelect);
}	
		//add an "info" icon?

		string dmodDir = dmodgamedir+"/";
		string infoFile = dmodDir+"readme.txt";

		if (!FileExists(infoFile))
		{
			infoFile = dmodDir+"read me please.txt";
		}
		
		if (!FileExists(infoFile))
		{
			infoFile = dmodDir+"credits.txt";
		}

		if (!FileExists(infoFile))
		{
			infoFile = dmodDir+"dmod.diz";
		}

		if (FileExists(infoFile))
		{
			pBG->GetVar("textfile")->Set(infoFile); //save for later

			
			CL_Vec2f iconPos = iPhoneMap2X(405,56);
			Entity *pIcon = CreateOverlayButtonEntity(pBG, "view_info", ReplaceWithLargeInFileName("interface/iphone/dmod_info_button.rttex"), iconPos.x, iconPos.y);
			SetButtonStyleEntity(pIcon,  Button2DComponent::BUTTON_STYLE_CLICK_ON_TOUCH_IGNORE_DRAGGING);
			SetTouchPaddingEntity(pIcon, CL_Rectf(0,0,0,0));
			pIcon->GetFunction("OnButtonSelected")->sig_function.connect(&DMODMenuOnSelect);
		}


	//add animation effect
	//ZoomToPositionFromThisOffsetEntity(pBG, CL_Vec2f(GetScreenSizeXf(), 0), 500, INTERPOLATE_EASE_TO, 10);
}

void GetParsedDMODInfo(string dmodPath, string &nameOut, float &versionOut, string &copyright, string &dmodwebsite, string &description)
{
	
	TextScanner t(dmodPath+"/dmod.diz", false);

	if (!t.IsLoaded())
	{
		//error, unable to load DMOD diz
		nameOut = GetFileNameFromString(dmodPath);
		copyright = "No dmod.diz found.  Corrupted?";
		versionOut = 0;
		return;
	}

	nameOut = t.GetLine(0);
	
	int maxNameChars = 39;

	if (nameOut.length() > maxNameChars)
	{
		TruncateString(nameOut, maxNameChars);
		description += "...";
	}
	description.clear();

	for (int i=1; i < t.m_lines.size(); i++)
	{
		description += t.GetLine(i);
		
		if (i == 1)
		{
			description = TrimLeft(description);
		}
		if (description.length() > 400)
		{
			break;		
		}
	
		if (i < t.m_lines.size()-1 && description.size()>0)
		{
			description += " ";
		}

	}

	int maxChars = 180;

	if (description.length() > maxChars)
	{
		TruncateString(description, maxChars);
		description += "...";
	}
}


bool EndsWith(std::string const &fullString, std::string const &ending)
{
	if (fullString.length() >= ending.length()) 
	{
		return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
	}
	else
	{
		return false;
	}
}

struct DMODDisplayEntry
{
	DMODDisplayEntry()
	{
		version = 0.0f;
	}
	string m_files;

	string dmodName;
	string dmodCopyright;
	string dmodwebsite;
	string description;
	float version;




};

bool DmodSortByName(const DMODDisplayEntry& lhs, const DMODDisplayEntry& rhs) { return lhs.dmodName < rhs.dmodName; }

void DMODMenuAddScrollContent(Entity *pParent)
{
	pParent = pParent->GetEntityByName("scroll_child");
	
	pParent->RemoveAllEntities();
	float x = 5;
	float y = 0;

	//Entity *pEnt;
	
	vector<string> temp = GetDirectoriesAtPath(GetDMODRootPath());
	vector<DMODDisplayEntry> entries;

	//actually, you know what?  Let's add the path to each one
	for (int i=0; i < temp.size(); i++)
	{

		temp[i] = GetDMODRootPath()+temp[i];
		if (FileExists(temp[i]+"/dink.dat"))
		{
			//looks valid
			DMODDisplayEntry entry;
			entry.m_files = temp[i];
			entries.push_back(entry);
		}
	}

	if (!GetDMODStaticRootPath().empty())
	{
		vector<string> staticFiles = GetDirectoriesAtPath(GetDMODStaticRootPath());

		for (int i=0; i < staticFiles.size(); i++)
		{
			//merge in if not a duplicate
			for (int n=0; n < entries.size(); n++)
			{
				if (GetFileNameFromString(entries[n].m_files) == staticFiles[i])
				{
					//duplicate
					continue;
				}
			}
			DMODDisplayEntry entry;
			entry.m_files = GetDMODStaticRootPath() + staticFiles[i];
			entries.push_back(entry);
		}
	}


	
	int dmodsAdded = 0;

	for (unsigned int i=0; i < entries.size(); i++)
	{
#ifdef WIN32
		if (
			EndsWith(entries[i].m_files, "/audio") || EndsWith(entries[i].m_files,"/dink" )|| EndsWith(entries[i].m_files,"/game") || EndsWith(entries[i].m_files,"/interface") 
			|| EndsWith(entries[i].m_files, "/develop")
			)continue;
#else
		if (IsInString(entries[i].m_files,"/Snapshot") || IsInString(entries[i].m_files, "/Snapshots")) continue;
#endif
		GetParsedDMODInfo(entries[i].m_files, entries[i].dmodName, entries[i].version, entries[i].dmodCopyright, entries[i].dmodwebsite, entries[i].description );
		
		StringReplace("\"", "", entries[i].dmodName);
		dmodsAdded++;
	}

	//sort them by DMOD name rather than filename
	sort(entries.begin(), entries.end(), DmodSortByName);


	for (int i = 0; i < entries.size(); i++)
	{
  	   if (entries[i].dmodName.empty())continue;
		
	   AddDMODBar(pParent, x, y, entries[i].dmodName, entries[i].description, "", 20.3f, entries[i].m_files, i);
	}


	if (dmodsAdded == 0)
	{
		CL_Vec2f vTextBoxPos(iPhoneMapX(20),iPhoneMapY(y));
		CL_Vec2f vTextBounds(iPhoneMapX(434), iPhoneMapY(200));
		string msg = "`6No add-on stories are currently installed.  Click ``Browse`6 below to get some.";
		
		if (!GetApp()->CanDownloadDMODS())
		{
			//a better message would be:
		msg = "`6No add-on stories are currently available.";
		}
		CreateTextBoxEntity(pParent, "", vTextBoxPos, vTextBounds, msg);
	}

    VariantList vList(pParent->GetParent());
	ResizeScrollBounds(&vList);
	DisableHorizontalScrolling(pParent->GetParent());

}

void OnDMODMenuDelete(Entity *pMenu)
{
	if (IsBaseAppInitted())
	{
		EntityComponent *pScrollerComp = pMenu->GetEntityByName("scroll")->GetComponentByName("Scroll");
		if (pScrollerComp)
		{
			GetApp()->GetVar("DMODProgress2d")->Set(pScrollerComp->GetVar("progress2d")->GetVector2());
		}
	}
}

void OnPostIntroTransition(VariantList *pVList)
{
	Entity *pBG = pVList->Get(0).GetEntity();
	DMODMenuAddScrollContent(pBG);

if (GetEmulatedPlatformID() == PLATFORM_ID_WEBOS)
{
	CreateQuickTipFirstTimeOnly(pBG, "interface/generic/quicktip_dmod.rttex", true);
} else
if (GetEmulatedPlatformID() == PLATFORM_ID_BBX)
{
	CreateQuickTipFirstTimeOnly(pBG, "interface/generic/quicktip_dmod.rttex", true);

} else
if (GetEmulatedPlatformID() == PLATFORM_ID_ANDROID)
{
	CreateQuickTipFirstTimeOnly(pBG, "interface/android/quicktip_dmod.rttex", true);
} else if (GetEmulatedPlatformID() == PLATFORM_ID_WINDOWS)
{
	CreateQuickTipFirstTimeOnly(pBG, "interface/win/quicktip_dmod.rttex", true);
} else if (GetEmulatedPlatformID() == PLATFORM_ID_OSX)
{
	CreateQuickTipFirstTimeOnly(pBG, "interface/osx/quicktip_dmod.rttex", true);
} else
{
	CreateQuickTipFirstTimeOnly(pBG, "interface/iphone/quicktip_dmod.rttex", true);
}

	//get notified when this is deleted so we can save the default settings
	pBG->sig_onRemoved.connect(&OnDMODMenuDelete);
}

Entity * DMODMenuCreate( Entity *pParentEnt, bool bFadeIn /*= false*/ )
{
	//GetMessageManager()->SendGame(MESSAGE_TYPE_PLAY_MUSIC, "audio/title.mp3", 200);
	GetBaseApp()->ClearError();

	Entity *pBG = NULL;
	
	if (IsLargeScreen())
	{
		pBG =  CreateOverlayEntity(pParentEnt, "DMODMenu", ReplaceWithDeviceNameInFileName("interface/iphone/bkgd_stone.rttex"), 0,0);
	} else
	{
		pBG = CreateOverlayEntity(pParentEnt, "DMODMenu", ReplaceWithDeviceNameInFileName("interface/iphone/bkgd_stone.rttex"), 0,0);
	}
	
	pBG->SetName("DMODMenu");
	AddFocusIfNeeded(pBG, true, 500);
	pBG->AddComponent(new FocusRenderComponent);

	CL_Vec2f vTextAreaPos = iPhoneMap(2,10);
	float offsetFromBottom = iPhoneMapY(42);
	float offsetFromRight = iPhoneMapY(0);

	CL_Vec2f vTextAreaBounds = (GetScreenSize()- CL_Vec2f(offsetFromRight,offsetFromBottom))-vTextAreaPos;
	Entity *pScroll = pBG->AddEntity(new Entity("scroll"));
	pScroll->GetVar("pos2d")->Set(vTextAreaPos);
	pScroll->GetVar("size2d")->Set(vTextAreaBounds);
	pScroll->AddComponent(new TouchHandlerComponent);

	Entity *pLabel = CreateTextLabelEntity(pBG, "scanning", GetScreenSizeXf()/2, GetScreenSizeYf()/2, "Preparing quest list...");
	SetAlignmentEntity(pLabel, ALIGNMENT_CENTER);
	FadeOutAndKillEntity(pLabel, true, 300, 501);

	EntityComponent *pFilter = pScroll->AddComponent(new FilterInputComponent);

	EntityComponent *pScrollComp = pScroll->AddComponent(new ScrollComponent);
	EntityComponent *pScrollBarComp = pScroll->AddComponent(new ScrollBarRenderComponent); 	//also let's add a visual way to see the scroller position
	//pScroll->GetVar("color")->Set(MAKE_RGBA(61,155, 193, 255)); 
	Entity *pScrollChild = pScroll->AddEntity(new Entity("scroll_child"));
	pScrollComp->GetVar("fingerTracking")->Set(uint32(1));

	//EntityComponent *pClip = pScroll->AddComponent(new RenderClipComponent);
	//pClip->GetVar("clipMode")->Set(uint32(RenderClipComponent::CLIP_MODE_BOTTOM));
	
	Entity *pOverlay = CreateOverlayEntity(pBG, "", ReplaceWithDeviceNameInFileName("interface/iphone/bg_stone_overlay.rttex"), 0, GetScreenSizeYf()); 
	SetAlignmentEntity(pOverlay, ALIGNMENT_DOWN_LEFT);

	//the continue button
	Entity *pEnt;

	//pEnt = CreateOverlayRectEntity(pBG, CL_Rectf(0, GetScreenSizeYf()-offsetFromBottom, GetScreenSizeXf(), 320), MAKE_RGBA(0,0,0,100));

	eFont fontID = FONT_SMALL;
	
	pEnt = CreateTextButtonEntity(pBG, "Back", iPhoneMapX(25), iPhoneMapY(BACK_BUTTON_Y), "Back", false);
	pEnt->GetFunction("OnButtonSelected")->sig_function.connect(&DMODMenuOnSelect);
	SetupTextEntity(pEnt, fontID);
	AddHotKeyToButton(pEnt, VIRTUAL_KEY_BACK);

	if (GetApp()->CanDownloadDMODS())
	{
		
		pEnt = CreateTextButtonEntity(pBG, "getmore", iPhoneMapX(330), iPhoneMapY(BACK_BUTTON_Y), "Install by URL", false);
		pEnt->GetFunction("OnButtonSelected")->sig_function.connect(&DMODMenuOnSelect);
		SetupTextEntity(pEnt, fontID);


		pEnt = CreateTextButtonEntity(pBG, "browse", iPhoneMapX(170), iPhoneMapY(BACK_BUTTON_Y), "Browse", false);
		pEnt->GetFunction("OnButtonSelected")->sig_function.connect(&DMODMenuOnSelect);
		SetupTextEntity(pEnt, fontID);

	}
	pBG->GetFunction("RemoveDMOD")->sig_function.connect(&DMODMenuOnRemoveDMOD);
	pBG->GetFunction("SessionNew")->sig_function.connect(&DMODMenuOnSessionNew);
	pBG->GetFunction("SessionContinue")->sig_function.connect(&DMODMenuOnSessionContinue);
	
	
	if (bFadeIn)
	{
	//	FadeInEntity(pBG, true, 500);
		SlideScreen(pBG, true, 500);
	} else
	{
		SlideScreen(pBG, true, 500);
	}
    
    VariantList vList(GetApp()->GetVar("DMODProgress2d")->GetVector2());
	GetMessageManager()->CallComponentFunction(pScrollComp, 501, "SetProgress", &vList); 

	pBG->GetFunction("OnPostIntroTransition")->sig_function.connect(&OnPostIntroTransition);
    VariantList vTemp(pBG, string(""));
	GetMessageManager()->CallEntityFunction(pBG, 500, "OnPostIntroTransition", &vTemp); 

	return pBG;
}
