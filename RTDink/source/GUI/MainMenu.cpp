#include "PlatformPrecomp.h"
#include "MainMenu.h"
#include "Entity/EntityUtils.h"
#include "DebugMenu.h"
#include "GameMenu.h"
#include "LoadMenu.h"
#include "../dink/dink.h"
#include "DMODMenu.h"
#include "PauseMenu.h"
#include "DMODInstallMenu.h"
#include "PopUpMenu.h"
#include "OptionsMenu.h"
#include "AboutMenu.h"
#include "FileSystem/StreamingInstance.h"
#include "Entity/CustomInputComponent.h"
#include "Entity/HTTPComponent.h"

#ifdef PLATFORM_HTML5
#include "html5/SharedJSLIB.h";
#endif

bool g_bMainMenuFirstTime = true;
bool g_bDidVersionCheck = false;

Entity * VersionShowScoreMessage(Entity *pMenu, string msg);
void GetParsedDMODInfo(string dmodPath, string &nameOut, float &versionOut, string &copyright, string &dmodwebsite, string &description);

void ReloadMainMenu(VariantList *pVList)
{
	MainMenuCreate(pVList->Get(0).GetEntity());
}

void OnVersionDownloadError(VariantList *pVList)
{
	NetHTTP::eError e = (NetHTTP::eError)pVList->m_variant[1].GetUINT32();

	string msg = "`4Unable to check for updates. (" + toString(e) + ")";
	if (e == NetHTTP::ERROR_COMMUNICATION_TIMEOUT)
	{
		msg = "`4Unable to check for updates. (timed out)";
	}

	Entity *pMenu = pVList->m_variant[0].GetComponent()->GetParent();

	VersionShowScoreMessage(pMenu, msg);

	//kill current menu
	GetMessageManager()->CallEntityFunction(pMenu, 1000, "OnDelete", NULL);
	pMenu->SetName("MainMenuDelete");

	//reload the main menu in a bit
	
	VariantList vList(pMenu->GetParent());
	GetMessageManager()->CallStaticFunction(ReloadMainMenu, 1000, &vList, TIMER_SYSTEM);
}

Entity * VersionShowScoreMessage(Entity *pMenu, string msg)
{
	Entity *pInfo = pMenu->GetEntityByName("Info");
	if (pInfo)
	{
		pInfo->GetComponentByName("TextRender")->GetVar("text")->Set(msg);
		pInfo->RemoveComponentByName("Typer"); // a thing that types stuff
	}
	else
	{
		pInfo = CreateTextLabelEntity(pMenu, "Info", iPhoneMapX(170), iPhoneMapY(220), msg);
	}

	return pInfo;
}


void OnVersionDownloadHTTPFinish(VariantList *pVList)
{
	Entity *pMenu = pVList->m_variant[0].GetComponent()->GetParent();

	TextScanner t((char*)pVList->m_variant[1].GetString().c_str());
	string line;

	//LogMsg(t.GetAll().c_str());

	//	ShowScoreMessage(pMenu, t.GetParmString("msg",1));
	//GetApp()->GetVar("score_msg")->Set(t.GetParmString("msg",1));

	string key = PlatformIDAsString(GetEmulatedPlatformID());
#ifdef RT_IS_BETA
	key += "_beta";
#endif

	vector<string> lines;

	
	for (int i = 0; i < t.GetLineCount(); i++)
	{
		lines = StringTokenize(t.GetLine(i), "|");

		if (lines.size() > 2 && lines[0] == key)
		{
			VersionShowScoreMessage(pMenu, "");

			float version = StringToFloat(lines[1]);
			if (version > GetApp()->GetVersion() + 0.001f) //the extra for an epsilon to flight floating point weirdness, trust me
			{
				PopUpCreate(pMenu, "New version detected! Download it now?", lines[2],
					"cancel_update", "`wCancel", "url_update", "`wDownload", true);
				return;
			}
		}
	}
	VersionShowScoreMessage(pMenu, "No new updates found.");


	//kill current menu
	GetMessageManager()->CallEntityFunction(pMenu, 1000, "OnDelete", NULL);
	pMenu->SetName("MainMenuDelete");

	//reload the main menu in a bit

	VariantList vList(pMenu->GetParent());
	GetMessageManager()->CallStaticFunction(ReloadMainMenu, 1000, &vList, TIMER_SYSTEM);
}



void MainMenuOnSelect(VariantList *pVList) //0=vec2 point of click, 1=entity sent from
{
	Entity *pEntClicked = pVList->m_variant[1].GetEntity();

	LogMsg("Clicked %s entity at %s", pEntClicked->GetName().c_str(),pVList->m_variant[1].Print().c_str());

#ifdef PLATFORM_HTML5
	if (GetTouchesReceived() > 0)
	{
		//using a touch screen, go into that mode
		GetApp()->SetUsingTouchScreen(true);
	}

#endif

	//fix it, was set the other way for ios' safari to get tricked into playing sound
	//SetDefaultButtonStyle(Button2DComponent::BUTTON_STYLE_CLICK_ON_TOUCH_RELEASE);


	if (pEntClicked->GetName() == "New")
	{
		DisableAllButtonsEntity(pEntClicked->GetParent());
		InitDinkPaths(GetBaseAppPath(), "dink", "");

		//slide it off the screen and then kill the whole menu tree
		SlideScreen(pEntClicked->GetParent(), false);
		GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 500, "OnDelete", NULL);
	
		GameCreate(pEntClicked->GetParent()->GetParent(), 0, "");
		GetApp()->GetVar("showViewHint")->Set(uint32(0)); //so this won't be shown here

	}
	
	if (pEntClicked->GetName() == "Load")
	{
		DisableAllButtonsEntity(pEntClicked->GetParent());
		InitDinkPaths(GetBaseAppPath(), "dink", "");
		//slide it off the screen and then kill the whole menu tree
		SlideScreen(pEntClicked->GetParent(), false);
		GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 500, "OnDelete", NULL);
		LoadMenuCreate(pEntClicked->GetParent()->GetParent());
	}

	if (pEntClicked->GetName() == "Continue")
	{
		DisableAllButtonsEntity(pEntClicked->GetParent());
		InitDinkPaths(GetBaseAppPath(), "dink", "");
		//slide it off the screen and then kill the whole menu tree
		SlideScreen(pEntClicked->GetParent(), false);
		GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 500, "OnDelete", NULL);
		GameCreate(pEntClicked->GetParent()->GetParent(), 0, GetSavePath()+"dink/"+string("continue_state.dat"));
	}

	if (pEntClicked->GetName() == "Debug")
	{
		//overlay the debug menu over this one
		pEntClicked->GetParent()->RemoveComponentByName("FocusInput");
		DebugMenuCreate(pEntClicked->GetParent());
	}

	if (pEntClicked->GetName() == "Add-ons")
	{
		DisableAllButtonsEntity(pEntClicked->GetParent());
		//slide it off the screen and then kill the whole menu tree
		SlideScreen(pEntClicked->GetParent(), false);
		GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 500, "OnDelete", NULL);
	//	DMODInstallMenuCreate(pEntClicked->GetParent()->GetParent(), "files.dinknetwork.com/three_amulets-v1_1.dmod", "");
		DMODMenuCreate(pEntClicked->GetParent()->GetParent());
	}

	
	if (pEntClicked->GetName() == "Options")
	{
		//DisableAllButtonsEntity(pEntClicked->GetParent());
		pEntClicked->GetParent()->RemoveComponentByName("FocusInput");
		pEntClicked->GetParent()->RemoveComponentByName("FocusUpdate");
	
		//SlideScreen(pEntClicked->GetParent(), false);
		//GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 500, "OnDelete", NULL);
		OptionsMenuCreate(pEntClicked->GetParent());
	}

	if (pEntClicked->GetName() == "About")
	{
		DisableAllButtonsEntity(pEntClicked->GetParent());
		//RemoveFocusIfNeeded(pEntClicked->GetParent());	//slide it off the screen and then kill the whole menu tree
		SlideScreen(pEntClicked->GetParent(), false);
		GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 500, "OnDelete", NULL);
		AboutMenuCreate(pEntClicked->GetParent()->GetParent());
	}

	if (pEntClicked->GetName() == "rtsoftlogo")
	{
		PopUpCreate(pEntClicked->GetParent(), "Would you like to visit `wrtsoft.com``?", "http://www.rtsoft.com/iphone",
			"cancel", "`wCancel", "url", "`wLaunch", true);
		return;
	}

	//GetEntityRoot()->PrintTreeAsText(); //useful for debugging
}


string GetNextDMODToInstall(bool &bIsCommandLineInstall, const bool bDeleteCommandLineParms)
{

	bIsCommandLineInstall = false;
	//if (!GetApp()->CanDownloadDMODS()) return ""; //ignore it

	if (IsDesktop() || GetEmulatedPlatformID() == PLATFORM_ID_HTML5)
	{
		vector<string> parms = GetApp()->GetCommandLineParms();

		for (int i = 0; i < parms.size(); i++)
		{
			StringReplace("\"", "", parms[i]);
			if (IsInString(ToLowerCaseString(parms[i]), ".dmod"))
			{
				bIsCommandLineInstall = true;

				//dmod sent via commandline, install it
				if (bDeleteCommandLineParms)
					GetApp()->GetReferenceToCommandLineParms()[i].clear(); //don't want to install the same dmod twice
				return parms[i];
			}
		}
	}


	vector<string> files = GetFilesAtPath(GetSavePath());

	//LogMsg("listing files");

	for (unsigned int i=0; i < files.size(); i++)
	{
		//LogMsg(files[i].c_str());
		if (GetFileExtension(files[i]) == "dmod")
		{
			 return files[i];
		}
	}

	return "";
}

void MainOnStartLoading(VariantList *pVList)
{
	Entity *pBG = pVList->m_variant[0].GetEntity();

	string fileName = pVList->m_variant[1].GetString();

	DisableAllButtonsEntity(pBG);
	SlideScreen(pBG, false);
	GetMessageManager()->CallEntityFunction(pBG, 500, "OnDelete", NULL);
	pBG->SetName("MainMenuDelete");

	bool bIsCommandLineInstall = true;

	string fName = GetNextDMODToInstall(bIsCommandLineInstall, true);
	
	if (IsInString(fName, "http:") || IsInString(fName, "https:")
		||
		(GetEmulatedPlatformID() == PLATFORM_ID_HTML5 && IsInString(fName, ".dmod"))
		)
	{
		//we should download and install this
		StringReplace("-game ", "", fName);
		StringReplace("dmod=", "", fName);
		DMODInstallMenuCreate(pBG->GetParent(), fName, GetDMODRootPath(), "", true, fName);
		return;
	}

	if (!fName.empty())
	{
		DMODInstallMenuCreate(pBG->GetParent(), "", GetDMODRootPath(), GetSavePath()+fName, false, "", !bIsCommandLineInstall);
	}  else
	{
		GameCreate(pBG->GetParent(), 0, fileName, "Continuing last game...");
	}
}


void MainMenuDMODMenuOnSessionNew(VariantList *pVList)
{
	Entity *pMenu = pVList->Get(0).GetEntity();

	DisableAllButtonsEntity(pMenu);
	SlideScreen(pMenu, false);
	GetMessageManager()->CallEntityFunction(pMenu, 500, "OnDelete", NULL);
	pMenu->SetName("MainMenuDelete");

	GameCreate(pMenu->GetParent(), 0, "");
}

void MainMenuDMODMenuOnSessionContinue(VariantList *pVList)
{
	Entity *pMenu = pVList->Get(0).GetEntity();

	DisableAllButtonsEntity(pMenu);
	SlideScreen(pMenu, false);
	GetMessageManager()->CallEntityFunction(pMenu, 500, "OnDelete", NULL);
	pMenu->SetName("MainMenuDelete");

	GameCreate(pMenu->GetParent(), 0, g_dglo.m_savePath + "continue_state.dat");
}

void MainMenuDMODCancel(VariantList *pVList)
{
	Entity *pMenu = pVList->Get(0).GetEntity();

	DisableAllButtonsEntity(pMenu);
	SlideScreen(pMenu, false);
	GetMessageManager()->CallEntityFunction(pMenu, 500, "OnDelete", NULL);
	pMenu->SetName("MainMenuDelete");
	//reload the main menu
	MainMenuCreate(pMenu->GetParent());
}

void MainOnStartLoadingDMOD(VariantList *pVList)
{
	Entity *pBG = pVList->m_variant[0].GetEntity();

	string fileName = pVList->m_variant[1].GetString();


	//first check to see if it's a valid dmod
	string dmoddir = pBG->GetVar("start_full_dmod_dir")->GetString();
	
	if (!FileExists(dmoddir + "/dmod.diz"))
	{
		//don't look valid..
		PopUpCreate(pBG, "The DMOD at "+dmoddir+" appears to be missing or damaged. Ignoring it.", "", "CancelDMODLoad", "Continue", "", "", true);
		return;
	}

	string dmodName, dmodCopyright, dmodwebsite, description;
	float version = 0;
	GetParsedDMODInfo(dmoddir, dmodName, version, dmodCopyright, dmodwebsite, description);
	
	InitDinkPaths(GetBaseAppPath(), "dink", dmoddir);
	
	//next, see if there is a save-on-quit save state existing for this dmod
	if (FileExists(g_dglo.m_savePath + "/continue_state.dat"))
	{
		PopUpCreate(pBG, "Continue your last session in "+ dmodName+"?", "",  "CancelDMODLoad", "Cancel", "SessionContinue", "Continue", true, "SessionNew", "New Game");
		return;
	}
	
	DisableAllButtonsEntity(pBG);
	SlideScreen(pBG, false);
	GetMessageManager()->CallEntityFunction(pBG, 500, "OnDelete", NULL);
	pBG->SetName("MainMenuDelete");

	GameCreate(pBG->GetParent(), 0, "", "Loading "+dmodName);
	
}


void ImportSaveFileIfApplicable(string fName)
{
	if (GetFileExtension(fName) != "dat") return;

	if (fName == "save.dat") return; //it's not this one
	if (fName == "state.dat") return; //it's not this one
	if (fName == "continue_state.dat") return; //it's not this one
	if (fName == "autosave.dat") return; //it's not this one
	if (fName == "autosavedb.dat") return; //it's not this one
	if (fName == "quicksave.dat") return; //it's not this one
	if (fName == "quickload.dat") return; //it's not this one
	
	
	//check for any dmod name specified after a _

	size_t index = fName.find_last_of('_');
	size_t periodPos = fName.find_last_of('.');

	if (periodPos == string::npos || GetFileExtension(fName) != "dat") return;

	string dmodName = "";

	if (index != string::npos)
	{
		//yeah, it has one
		dmodName = fName.substr(index+1, periodPos-(index+1));
	}

	//ok, ready to copy it.  But figure out where, and what the filename should be
	string destPath = GetSavePath()+"dink/";
	string destFile = fName;

	//modify if needed for a dmod

	if (!dmodName.empty())
	{
		destPath = GetDMODRootPath()+dmodName+"/";
		destFile = fName.substr(0, index);
		destFile += fName.substr(periodPos, fName.size()-periodPos);
	}

	LogMsg("Importing %s to %s", (GetSavePath()+fName).c_str(), (destPath+destFile).c_str());
	GetFileManager()->Copy(GetSavePath()+fName, destPath+destFile, false);

	RemoveFile(GetSavePath()+fName, false);
}

void CheckForImportedSavedGames()
{
	if (!GetEmulatedPlatformID() == PLATFORM_ID_IOS) return;

	vector<string> files = GetFilesAtPath(GetSavePath());

	for (uint32 i=0; i < files.size(); i++)
	{
		ImportSaveFileIfApplicable(files[i]);
	}
}



void MainMenuContinueLast(VariantList *pVList)
{
	Entity *pBG = pVList->Get(0).GetEntity();

	pBG->GetFunction("OnStartLoading")->sig_function.connect(&MainOnStartLoading);
	VariantList vList(pBG, GetSavePath()+string("state.dat"));
    GetMessageManager()->CallEntityFunction(pBG, 1000, "OnStartLoading", &vList); 
	g_bMainMenuFirstTime = false;

	LogMsg("Continuing");
}

void MainMenuContinueLastNewStyle(VariantList *pVList)
{
	Entity *pBG = pVList->Get(0).GetEntity();

	//ClearCommandLineParms();
	pBG->GetFunction("OnStartLoading")->sig_function.connect(&MainOnStartLoading);
    VariantList vList(pBG, ReadLastPathSaved()+string("continue_state.dat"));
	GetMessageManager()->CallEntityFunction(pBG, 1000, "OnStartLoading", &vList); 
	g_bMainMenuFirstTime = false;
	//WriteLastPathSaved("");
	LogMsg("Continuing");
}

void MainMenuCancelLast(VariantList *pVList)
{
	Entity *pMenu = pVList->Get(0).GetEntity();

	WriteLastPathSaved("");
	RemoveFile(GetSavePath()+"state.dat", false);

	//kill current menu
	GetMessageManager()->CallEntityFunction(pMenu, 0, "OnDelete", NULL);
	pMenu->SetName("MainMenuDelete");

	//reload the main menu
	MainMenuCreate(pMenu->GetParent());
}

void CheckForNewVersion(Entity *pMenu)
{

	pMenu->RemoveComponentByName("HTTP"); //just in case it already exists

										  //get the internet stuff going
	EntityComponent *pComp = pMenu->AddComponent(new HTTPComponent);

	VariantList vPostData;

	vPostData.m_variant[0].Set("version");
	vPostData.m_variant[1].Set(toString(GetApp()->GetVersion()));
	pComp->GetFunction("AddPostData")->sig_function(&vPostData);

	vPostData.m_variant[0].Set("build");
	vPostData.m_variant[1].Set(toString(GetApp()->GetBuild()));
	pComp->GetFunction("AddPostData")->sig_function(&vPostData);

	vPostData.m_variant[0].Set("platform");
	vPostData.m_variant[1].Set(toString(GetEmulatedPlatformID()));
	pComp->GetFunction("AddPostData")->sig_function(&vPostData);

	VariantList v;

	string url;
	uint32 port;
	GetApp()->GetServerInfo(url, port);

	v.m_variant[0].Set(url);
	v.m_variant[1].Set(port);
	v.m_variant[2].Set("dink/versions.php");
	pComp->GetFunction("OnError")->sig_function.connect(&OnVersionDownloadError);
	pComp->GetFunction("OnFinish")->sig_function.connect(&OnVersionDownloadHTTPFinish);
	
	//pComp->GetFunction("Init")->sig_function(&v);
	GetMessageManager()->CallComponentFunction(pComp,100, "Init", &v); //call it in a bit


	Entity *pEnt = VersionShowScoreMessage(pMenu, "`wChecking for updates..");
	EntityComponent *pTyper = pEnt->AddComponent(new TyperComponent);
	pTyper->GetVar("text")->Set("...................");
	pTyper->GetVar("speedMS")->Set(uint32(200));


}


void OnSyncUpdate(VariantList *pVList)
{

	if (IsStillLoadingPersistentData()) return;

	if (g_bDidVersionCheck) return;

	g_bDidVersionCheck = true;
	GetBaseApp()->GetEntityRoot()->PrintTreeAsText();
	//Entity *pMenu = pVList->m_variant[0].GetComponent()->GetParent();
	Entity *pMenu = GetBaseApp()->GetEntityRoot()->GetEntityByName("MainMenu");

	
	//kill current menu
	GetMessageManager()->CallEntityFunction(pMenu, 200, "OnDelete", NULL);
	pMenu->SetName("MainMenuDelete");

	//reload the main menu in a bit

	VariantList vList(pMenu->GetParent());
	GetMessageManager()->CallStaticFunction(ReloadMainMenu, 200, &vList, TIMER_SYSTEM);
}

void WaitForSync(Entity *pMenu)
{
	Entity *pEnt = VersionShowScoreMessage(pMenu, "`wFinding saved data...");
	EntityComponent *pTyper = pEnt->AddComponent(new TyperComponent);
	pTyper->GetVar("text")->Set("...................");
	pTyper->GetVar("speedMS")->Set(uint32(200));
	pEnt->GetFunction("OnUpdate")->sig_function.connect(&OnSyncUpdate);

}

Entity * MainMenuCreate( Entity *pParentEnt, bool bFadeIn )
{
	CheckForImportedSavedGames();

	GetApp()->GetVar("showViewHint")->Set(uint32(1)); //show that one tip
	
	Entity *pBG = CreateOverlayEntity(pParentEnt, "MainMenu", ReplaceWithDeviceNameInFileName("interface/iphone/bkgd_stone.rttex"), 0,0);
	
	OverlayRenderComponent *pOverlay = (OverlayRenderComponent*) pBG->GetComponentByName("OverlayRender");
	if (!pOverlay->GetSurfaceAnim())
	{

		LogMsg("Can't find media.  If running from Visual Studio, make sure you set the active dir to ../bin first!");
#ifdef WINAPI
		MessageBox(NULL, "Can't find media.  If running from Visual Studio, make sure you set the active dir to ../bin and have run media/update_media.bat first!", "Woah nelly!" , 0);
#endif
	}
	//Entity *pBG = pParentEnt->AddEntity(new Entity);
	GetBaseApp()->ClearError();
	AddFocusIfNeeded(pBG, true);
	

	pBG->GetFunction("ContinueLast")->sig_function.connect(&MainMenuContinueLast);
	pBG->GetFunction("ContinueLastNewStyle")->sig_function.connect(&MainMenuContinueLastNewStyle);
	pBG->GetFunction("CancelLast")->sig_function.connect(&MainMenuCancelLast);

	Entity *pButtonEntity;
	float x = 50;
	float yStart = 200;
	float y = yStart;
	float ySpacer = 55;
	eFont fontID = FONT_LARGE;
	float fontScale = 1;
	float fireAnimY = 240;
	CL_Vec2f vRtsoftLogoPt = CL_Vec2f(402, 149);

	CL_Vec2f vNewButPt = CL_Vec2f(26, 199);
	CL_Vec2f vLoadButPt = CL_Vec2f(135, 199);
	CL_Vec2f vAddonButPt = CL_Vec2f(26, 245);
	CL_Vec2f vContinueButPt = CL_Vec2f(246, 198);

	CL_Vec2f vAboutButPt = CL_Vec2f(251, 245);

	CL_Vec2f vOptionsButPt = CL_Vec2f(360, 246);

	if (IsIPADSize)
	{
		vNewButPt = CL_Vec2f(0, 468);
		vLoadButPt = CL_Vec2f(256, 468);
		fireAnimY = GetScreenSizeYf()-256;
		vRtsoftLogoPt = CL_Vec2f(830, 370);
	
		vAddonButPt = CL_Vec2f(13, 565);
		vContinueButPt = CL_Vec2f(525, 468);
		vAboutButPt = CL_Vec2f(525, 565);
		vOptionsButPt = CL_Vec2f(748, 565);
	} else if (IsIphone4Size)
	{
		vNewButPt = CL_Vec2f(66, 364);
		vLoadButPt = CL_Vec2f(277, 362);
		fireAnimY = GetScreenSizeYf()-160;
		vRtsoftLogoPt = CL_Vec2f(775, 262);

		vAddonButPt = CL_Vec2f(67, 453);
		vContinueButPt = CL_Vec2f(498, 364 );
		vAboutButPt = CL_Vec2f(512, 454);
		vOptionsButPt = CL_Vec2f(716, 455);
	}
	

#ifdef RT_IS_BETA
	Entity *pText = CreateTextLabelEntity(pBG, "text", GetScreenSizeXf()/2, GetScreenSizeYf()-20, 
		"`wBeta Version("+GetApp()->GetVersionString()+")");
	SetAlignmentEntity(pText, ALIGNMENT_CENTER);

#endif


	if (!g_bDidVersionCheck && IsDesktop())
	{
		g_bDidVersionCheck = true;
		CheckForNewVersion(pBG);
		return pBG;
	}


#ifdef PLATFORM_HTML5
	if (!g_bDidVersionCheck && IsStillLoadingPersistentData() )
	{
		WaitForSync(pBG);
		return pBG;
	}

#endif

	Entity *pEntDinkLogo = CreateOverlayEntity(pBG, "dinklogo", ReplaceWithDeviceNameInFileName("interface/iphone/logo_dink.rttex"), 0, 0);

	pButtonEntity = CreateOverlayEntity(pBG, "flameAnim", ReplaceWithDeviceNameInFileName("interface/iphone/bkgd_anim_fire.rttex"), 0, fireAnimY);
	SetupAnimEntity(pButtonEntity, 1, 4, 0, 0);
	AnimateEntity(pButtonEntity, 0, 3, 125, InterpolateComponent::ON_FINISH_REPEAT, 0);

	//for android, so the back key (or escape on windows/OSX) will quit out of the game
	EntityComponent *pComp = pBG->AddComponent(new CustomInputComponent);
	//tell the component which key has to be hit for it to be activated
	pComp->GetFunction("OnActivated")->sig_function.connect(1, boost::bind(&App::OnExitApp, GetApp(), _1));
	pComp->GetVar("keycode")->Set(uint32(VIRTUAL_KEY_BACK));

	pButtonEntity = CreateOverlayButtonEntity(pBG, "rtsoftlogo", ReplaceWithDeviceNameInFileName("interface/iphone/logo_rtsoft.rttex"), vRtsoftLogoPt.x, vRtsoftLogoPt.y);
	SetTouchPaddingEntity(pButtonEntity, CL_Rectf(0, 0, 0, -10)); //no padding, it overlaps other buttons..
	pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&MainMenuOnSelect);
	FadeInEntity(pButtonEntity, false, 300, 1000);
	

	static bool bOneTimeDMODLoaded = false;

	if (!bOneTimeDMODLoaded)
	{
		bOneTimeDMODLoaded = true;
		string dmodfilename;
		string dmodDir = GetDMODRootPath(&dmodfilename);

		if (dmodfilename != "")
		{
			//jump start a DMOD load
			pBG->GetFunction("OnStartLoadingDMOD")->sig_function.connect(&MainOnStartLoadingDMOD);
			pBG->GetVar("start_full_dmod_dir")->Set(dmodDir + dmodfilename);
			pBG->GetVar("start_dmod_dir")->Set(dmodfilename);

			VariantList vList(pBG, string(""));
			GetMessageManager()->CallEntityFunction(pBG, 1000, "OnStartLoadingDMOD", &vList);

			pBG->GetFunction("SessionNew")->sig_function.connect(&MainMenuDMODMenuOnSessionNew);
			pBG->GetFunction("SessionContinue")->sig_function.connect(&MainMenuDMODMenuOnSessionContinue);
			pBG->GetFunction("CancelDMODLoad")->sig_function.connect(&MainMenuDMODCancel);

			return pBG;
		}
	}

	bool bIsCommandLineInstall = false;

	
	if ( ! GetNextDMODToInstall(bIsCommandLineInstall, false).empty())
	{
		pBG->GetFunction("OnStartLoading")->sig_function.connect(&MainOnStartLoading);
        VariantList vList(pBG, string(""));
		GetMessageManager()->CallEntityFunction(pBG, 1000, "OnStartLoading", &vList); 
	
	} else
	{

		if (FileExists(GetSavePath()+"state.dat"))
		{
			PopUpCreate(pBG, "Continue your last session?", "", "CancelLast", "Cancel", "ContinueLast", "Continue", true);
			return pBG;
		};
		
		if (!ReadLastPathSaved().empty())
		{

			string lastDMOD = ReadLastPathSaved();

			PopUpCreate(pBG, "Continue your last session?", "", "CancelLast", "Cancel", "ContinueLastNewStyle", "Continue", true);
			return pBG;
		}
		
			if (g_bMainMenuFirstTime)
			{
#ifdef RT_MOGA_ENABLED
				ShowTextMessage("Moga enabled test version - not for distribution");
#endif
				PlayMenuMusic();
			}

			ZoomToPositionFromThisOffsetEntity(pEntDinkLogo, CL_Vec2f(0, -300), 2000, INTERPOLATE_EASE_TO, 5);
			pButtonEntity = CreateOverlayButtonEntity(pBG, "New", ReplaceWithDeviceNameInFileName("interface/iphone/main_but_new.rttex"), vNewButPt.x, vNewButPt.y); 
			pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&MainMenuOnSelect);
			SetTouchPaddingEntity(pButtonEntity, CL_Rectf(0,0,0,0));
			FadeInEntity(pButtonEntity, false, 500, 100);

			//SetupTextEntity(pButtonEntity, fontID, fontScale);
			pButtonEntity = CreateOverlayButtonEntity(pBG, "Add-ons", ReplaceWithDeviceNameInFileName("interface/iphone/main_but_addon10.rttex"), vAddonButPt.x, vAddonButPt.y); 
			pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&MainMenuOnSelect);
			//SetupTextEntity(pButtonEntity, fontID, fontScale);
			SetTouchPaddingEntity(pButtonEntity, CL_Rectf(0,0,0,0));
			FadeInEntity(pButtonEntity, false, 500, 1000);
		
			pButtonEntity = CreateOverlayButtonEntity(pBG, "Load", ReplaceWithDeviceNameInFileName("interface/iphone/main_but_load.rttex"), vLoadButPt.x, vLoadButPt.y); 
			pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&MainMenuOnSelect);
			//SetupTextEntity(pButtonEntity, fontID, fontScale);
			SetTouchPaddingEntity(pButtonEntity, CL_Rectf(0,0,0,0));
			FadeInEntity(pButtonEntity, false, 500, 400);

			pButtonEntity = CreateOverlayButtonEntity(pBG, "Continue", ReplaceWithDeviceNameInFileName("interface/iphone/main_but_continue.rttex"), vContinueButPt.x, vContinueButPt.y); 
			pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&MainMenuOnSelect);
			SetTouchPaddingEntity(pButtonEntity, CL_Rectf(0,0,0,0));
			FadeInEntity(pButtonEntity, false, 500, 700);

			if (!FileExists(GetSavePath()+"dink/"+string("continue_state.dat")))
			{
				pButtonEntity->GetVar("color")->Set(MAKE_RGBA(100,100,100,255));
				DisableAllButtonsEntity(pButtonEntity, false);
			}

			pButtonEntity = CreateOverlayButtonEntity(pBG, "About", ReplaceWithDeviceNameInFileName("interface/iphone/main_but_about.rttex"), vAboutButPt.x, vAboutButPt.y); 
			pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&MainMenuOnSelect);
			SetTouchPaddingEntity(pButtonEntity, CL_Rectf(0,0,0,0));
			FadeInEntity(pButtonEntity, false, 500, 1300);

			pButtonEntity = CreateOverlayButtonEntity(pBG, "Options", ReplaceWithDeviceNameInFileName("interface/iphone/main_but_options.rttex"), vOptionsButPt.x, vOptionsButPt.y); 
			pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&MainMenuOnSelect);
			SetTouchPaddingEntity(pButtonEntity, CL_Rectf(0,0,0,0));
			FadeInEntity(pButtonEntity, false, 500, 1500);
			DestroyUnusedTextures();
	

	//pButtonEntity = CreateTextButtonEntity(pBG, "Debug", x, y, "Debug and MP3 Music"); y += ySpacer;
	//pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&MainMenuOnSelect);
	//pButtonEntity->GetVar("alignment")->Set(uint32(ALIGNMENT_CENTER));
	}

	if (bFadeIn)
	{
		FadeInEntity(pBG, true, 500, 0);
	} else
	{
	
		if (g_bMainMenuFirstTime)
		{
			g_bMainMenuFirstTime = false;
		} else
		{
			SlideScreen(pBG, true);
		}
	}

	return pBG;
}


Entity *  AddTitle( Entity *pEnt, string title)
{
	Entity *pTitle;

	float titleHeight = 25;
	float scale = 1;
	float x = 480/2;

	pTitle = CreateTextLabelEntity(pEnt, "Title", x, titleHeight, title); 
	pTitle->GetVar("scale2d")->Set(CL_Vec2f(scale, scale));
	pTitle->GetComponentByName("TextRender")->GetVar("font")->Set(uint32(FONT_LARGE));
	EntityRetinaRemapIfNeeded(pTitle, true, false, true);
	pTitle->GetVar("alignment")->Set(uint32(ALIGNMENT_CENTER));

	return pTitle;
}