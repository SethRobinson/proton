#include "PlatformPrecomp.h"
#include "DMODInstallMenu.h"
#include "Entity/EntityUtils.h"
#include "DMODMenu.h"
#include "../dink/dink.h"
#include "GameMenu.h"
#include "Entity/UnpackArchiveComponent.h"
#include "Network/NetUtils.h"
#include "MainMenu.h"
#include "BrowseMenu.h"
#include "Network/NetHTTP.h"
#include "Entity/HTTPComponent.h"

#ifdef WINAPI
extern 	bool g_bAppCanRunInBackground;
#endif

void DMODInstallMenuOnSelect(VariantList *pVList) //0=vec2 point of click, 1=entity sent from
{
	Entity *pEntClicked = pVList->m_variant[1].GetEntity();
	Entity *pMenu = pEntClicked->GetParent();

	LogMsg("Clicked %s entity", pEntClicked->GetName().c_str());

	if (pEntClicked->GetName() == "Back")
	{
		//slide it off the screen and then kill the whole menu tree
		SlideScreen(pEntClicked->GetParent(), false);
		GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 500, "OnDelete", NULL);
		if (pMenu->GetVar("exitto")->GetString() == "main")
		{
			MainMenuCreate(pMenu->GetParent());
		} else if (pMenu->GetVar("exitto")->GetString() == "browse")
		{
			BrowseMenuCreate(pEntClicked->GetParent()->GetParent());

		} else  if (pMenu->GetVar("exitto")->GetString() == "play")
		{
				
			InitDinkPaths(GetBaseAppPath(), "dink", pMenu->GetVar("dmoddir")->GetString());

			GameCreate(pMenu->GetParent(), 0, "");			
		} else
		{
			DMODMenuCreate(pEntClicked->GetParent()->GetParent());
		}
		
	}

	if (pEntClicked->GetName() == "Abort")
	{
		//slide it off the screen and then kill the whole menu tree
		SlideScreen(pEntClicked->GetParent(), false);
		GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 500, "OnDelete", NULL);
		BrowseMenuCreate(pEntClicked->GetParent()->GetParent());
	}
	
	//GetEntityRoot()->PrintTreeAsText(); //useful for Loading
}

void DMODInstallUpdateStatus(Entity *pMenu, string msg)
{
	if (!pMenu)
	{
		pMenu = GetEntityRoot()->GetEntityByName("DMODInstall");
	}

	Entity *pStatus = pMenu->GetEntityByName("status");
	if (pStatus)
	{
		pStatus->GetComponentByName("TextRender")->GetVar("text")->Set(msg);
	}

}

void DMODInstallShowMsg(Entity *pMenu, string myMsg, bool bSuccess = false)
{
#ifdef WINAPI
	g_bAppCanRunInBackground = false;
#endif

	Entity *pMsg = pMenu->GetEntityByName("status");

		Entity *pLabel = pMenu->GetEntityByName("title_label");
		if (pLabel)
		{
			pLabel->RemoveComponentByName("Typer"); // a thing that types stuff

			if (!bSuccess)
			{
				pLabel->GetComponentByName("TextRender")->GetVar("text")->Set("Error!");
			} else
			{
				pLabel->GetComponentByName("TextRender")->GetVar("text")->Set("New quest added successfully.");
			}
		}
	
	if (pMsg)
	{
		pMsg->GetComponentByName("TextRender")->GetVar("text")->Set(myMsg);
		if (!pMsg->RemoveComponentByName("Typer"))
		{
			LogMsg("Failed to remove typer;");
		}; // a thing that types stuff


	}

	Entity *pSkip = pMenu->GetEntityByName("Back");

	if (bSuccess)
	{
		if (pSkip)
		{
			pSkip->GetComponentByName("TextRender")->GetVar("text")->Set("`wPlay it now");
		}

		//also add a button to keep browsing DMODs
		float yStart = iPhoneMapY(230);
		yStart = GetPos2DEntity(pSkip).y;
		CL_Vec2f vPos(iPhoneMapX(300), yStart);

		SetPos2DEntity(pSkip, vPos);

		Entity *pEnt = CreateTextButtonEntity(pMenu, "Abort", iPhoneMapX(100), yStart, "Back", true);
		SetAlignmentEntity(pEnt, ALIGNMENT_CENTER);
		pEnt->GetFunction("OnButtonSelected")->sig_function.connect(&DMODInstallMenuOnSelect);
	}
	else
	{
		if (pSkip)
		{
			pSkip->GetComponentByName("TextRender")->GetVar("text")->Set("`wContinue");
		}

	}

}

void DMODSetTitleLabel(Entity *pMenu, string myMsg)
{

	Entity *pLabel = pMenu->GetEntityByName("title_label");
	if (pLabel)
	{
		pLabel->RemoveComponentByName("Typer"); // a thing that types stuff
		pLabel->GetComponentByName("TextRender")->GetVar("text")->Set(myMsg);
		
		//just kidding, add typer back
		EntityComponent *pTyper = pLabel->AddComponent(new TyperComponent);
		pTyper->GetVar("text")->Set(".......");
		pTyper->GetVar("speedMS")->Set(uint32(500));
		
	}
	

}

void DMODInstallOnError(VariantList *pVList)
{
	NetHTTP::eError e = (NetHTTP::eError)pVList->m_variant[1].GetUINT32();

	string msg = "`4Unable to connect to the\nnetwork.``\nPlease try again later.";
	
	switch (e)
	{

	case NetHTTP::ERROR_COMMUNICATION_TIMEOUT:
		msg = "`4Connection timed out. Try Later?";
		break;

	case NetHTTP::ERROR_CANT_RESOLVE_URL:
		msg = "`4Can't find website.  Bad url?";
		break;

	case NetHTTP::ERROR_WRITING_FILE:
		msg = "`4Error writing file.  Out of space?";
		break;

	case NetHTTP::ERROR_404_FILE_NOT_FOUND:
		msg = "`4Server gave a 404: File not found. Bad url?";
		break;
	}


	DMODInstallShowMsg(pVList->m_variant[0].GetComponent()->GetParent(), msg);
}


void DMODUnpackOnError(VariantList *pVList)
{
	int error = pVList->m_variant[1].GetUINT32();

	string msg = "`4Error "+toString(error)+" unpacking.  Out of space or malformed .dmod file?";

	DMODInstallShowMsg(pVList->m_variant[0].GetComponent()->GetParent(), msg);
}

void DMODInstallSetProgressBar(float progress)
{
	Entity *pBar = GetEntityRoot()->GetEntityByName("bar");

	if (pBar)
	{
		pBar->GetComponentByName("ProgressBar")->GetVar("progress")->Set(progress);
	}
}


void OnDMODUnpackStatusUpdate(VariantList *pVList)
{
	int curBytes = pVList->Get(1).GetUINT32();
	int totalBytes = pVList->Get(2).GetUINT32();

	int barSize = 1024*1024*5; //5 megs of unpacking will fill up one bar
	float progress = float( (curBytes%barSize)) /float(barSize);
	
	//LogMsg("prog: %.2f", progress);
	int installSize = curBytes / 1024;
	string sizeType = "K";
	if (installSize > 10000)
	{
		installSize /= 1024;
		sizeType = "MB";
	}
	DMODInstallUpdateStatus(NULL, "Writing "+toString(installSize)+ " "+sizeType);
	DMODInstallSetProgressBar(progress);
}

void OnDMODUnpackFinish(VariantList *pVList)
{
	Entity *pMenu = pVList->m_variant[0].GetComponent()->GetParent();

	DMODInstallSetProgressBar(1);
	DMODInstallShowMsg(pMenu, pMenu->GetVar("originalFileName")->GetString()+" installed.", true);

	RemoveFile(GetDMODRootPath()+"temp.dmod");
	RemoveFile("temp.dmod");

	if (pMenu->GetVar("autoplay")->GetUINT32() == 1)
	{
		pMenu->GetVar("exitto")->Set("play");
		pMenu->GetVar("dmoddir")->Set(GetDMODRootPath()+ pVList->m_variant[0].GetComponent()->GetVar("firstDirCreated")->GetString());

	}
	
	SyncPersistentData();
}

void OnDMODInstallHTTPFinish(VariantList *pVList)
{
	Entity *pMenu = pVList->m_variant[0].GetComponent()->GetParent();

#ifdef _DEBUG
	LogMsg("Download finished...");
#endif

	DMODSetTitleLabel(pMenu, string("Installing ")+pMenu->GetVar("originalFileName")->GetString()+"...");
	EntityComponent *pUnpack = pMenu->AddComponent(new UnpackArchiveComponent);
	pUnpack->GetVar("sourceFileName")->Set(pMenu->GetVar("tempFileName")->GetString());
	bool bDeleteOnFinish = pMenu->GetVar("deleteOnFinish")->GetUINT32();
	pUnpack->GetVar("limitToSingleSubdir")->Set(uint32(1));

	pUnpack->GetVar("deleteSourceOnFinish")->Set(uint32(bDeleteOnFinish));
	
	pUnpack->GetVar("destDirectory")->Set(pMenu->GetVar("installDirectory")->GetString());
	DMODInstallSetProgressBar(0);

	pUnpack->GetFunction("OnError")->sig_function.connect(&DMODUnpackOnError);
	pUnpack->GetFunction("OnFinish")->sig_function.connect(&OnDMODUnpackFinish);
	pUnpack->GetFunction("OnStatusUpdate")->sig_function.connect(&OnDMODUnpackStatusUpdate);
	
}

void OnDMODInstallStatusUpdate(VariantList *pVList)
{
	int curBytes = pVList->Get(1).GetUINT32();
	int totalBytes = pVList->Get(2).GetUINT32();

	if (totalBytes == 0)
	{
		DMODInstallUpdateStatus(NULL, "Network active, getting file data...");
	} else
	{
		DMODInstallUpdateStatus(NULL, ""+toString(curBytes/1024)+"K/"+toString(totalBytes/1024)+"K");
	}

	//also update the progress bar thingie
	if (totalBytes == 0) totalBytes = 1; //avoid /1 error
	DMODInstallSetProgressBar(float(curBytes)/float(totalBytes));
}

void InitNetStuff(VariantList *pVList)
{
	Entity *pMenu = pVList->m_variant[0].GetEntity();
	//get the internet stuff going
	EntityComponent *pComp = pMenu->AddComponent(new HTTPComponent);

	string url = pMenu->GetVar("dmodURL")->GetString();
	string tempFileName = pMenu->GetVar("tempFileName")->GetString();

	string domain;
	string request;
	int port = 80;

	BreakDownURLIntoPieces(url, domain, request, port);
	VariantList v;
	v.m_variant[0].Set(tempFileName);
	pComp->GetFunction("SetFileOutput")->sig_function(&v);

	v.Reset();
	
	v.m_variant[0].Set(domain);
	v.m_variant[1].Set(uint32(port));
	v.m_variant[2].Set(request);
	pComp->GetFunction("Init")->sig_function(&v);

	pComp->GetFunction("OnError")->sig_function.connect(&DMODInstallOnError);
	pComp->GetFunction("OnFinish")->sig_function.connect(&OnDMODInstallHTTPFinish);
	pComp->GetFunction("OnStatusUpdate")->sig_function.connect(&OnDMODInstallStatusUpdate);
}


Entity * DMODInstallMenuCreate(Entity *pParentEnt, string dmodURL, string installDirectory, string sourceFileName, bool bFromBrowseMenu, string dmodName, bool bDeleteOnFinish)
{

	Entity *pBG = CreateOverlayEntity(pParentEnt, "DMODInstall", ReplaceWithDeviceNameInFileName("interface/iphone/bkgd_stone.rttex"), 0,0);
	AddFocusIfNeeded(pBG, true);

	Entity *pButtonEntity;
	float x = GetScreenSizeXf()/2;
	float yStart = iPhoneMapY(230);
	float y = yStart;
	float ySpacer = iPhoneMapY(50);
	Entity *pProgressBar = pBG->AddEntity(new Entity("bar"));
	Entity *pTitleLabel = CreateTextLabelEntity(pBG, "title_label", iPhoneMapX(100), iPhoneMapY(80), "Please wait");

	//save these for later
	pBG->GetVar("dmodURL")->Set(dmodURL);
	pBG->GetVar("dmodName")->Set(dmodName);
	pBG->GetVar("installDirectory")->Set(installDirectory);
	pBG->GetVar("tempFileName")->Set(GetDMODRootPath()+"temp.dmod");
	pBG->GetVar("originalFileName")->Set(GetFileNameFromString(dmodURL));
	pBG->GetVar("fromBrowseMenu")->Set(uint32(bFromBrowseMenu));
	pBG->GetVar("deleteOnFinish")->Set(uint32(bDeleteOnFinish));
	if (IsLargeScreen())
	{
		//SetupTextEntity(pTitleLabel, FONT_LARGE);
	}

	//SetAlignmentEntity(pTitleLabel, ALIGNMENT_CENTER);

	EntityComponent *pTyper = pTitleLabel->AddComponent(new TyperComponent);
	pTyper->GetVar("text")->Set(".......");
	pTyper->GetVar("speedMS")->Set(uint32(500));

	EntityComponent *pBar = pProgressBar->AddComponent(new ProgressBarComponent);
	pProgressBar->GetVar("pos2d")->Set(CL_Vec2f(iPhoneMapX(80),iPhoneMapY(120)));
	pProgressBar->GetVar("size2d")->Set(CL_Vec2f(iPhoneMapX(310),iPhoneMapY(15)));
	pProgressBar->GetVar("color")->Set(MAKE_RGBA(200,200,0,60));
	pBar->GetVar("interpolationTimeMS")->Set(uint32(1)); //update faster
	pBar->GetVar("borderColor")->Set(MAKE_RGBA(200,200,0,180));

	pButtonEntity = CreateTextButtonEntity(pBG, "Back", x, y, "Cancel"); y += ySpacer;
	pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&DMODInstallMenuOnSelect);
	SetAlignmentEntity(pButtonEntity, ALIGNMENT_CENTER);
	AddHotKeyToButton(pButtonEntity, VIRTUAL_KEY_BACK);

	Entity *pStatus = CreateTextLabelEntity(pBG, "status", x, iPhoneMapY(180), "Initializing...");
	SetAlignmentEntity(pStatus, ALIGNMENT_CENTER);

#ifdef WINAPI
	g_bAppCanRunInBackground = true;
#endif


	if (bFromBrowseMenu)
	{
		Entity *pStatus = CreateTextLabelEntity(pBG, "title", x, iPhoneMapY(30), "-= Installing "+dmodName+" =-");
		
		SetAlignmentEntity(pStatus, ALIGNMENT_CENTER);
		pBG->GetVar("exitto")->Set("browse");
	
	}
	if (!bDeleteOnFinish)
	{
		pBG->GetVar("autoplay")->Set(uint32(1));
	}


	if (!sourceFileName.empty())
	{
		//don't download, we already have the file
		pBG->GetVar("tempFileName")->Set(sourceFileName);
		pBG->GetVar("originalFileName")->Set(GetFileNameFromString(sourceFileName));
		pBG->GetVar("limitToSingleSubdir")->Set(uint32(1));

		EntityComponent *pCrapComp = pBG->AddComponent(new EntityComponent("CRAP")); //I don't need this, but the function want a component and gets the parent for the menu, so fine
		pBG->GetVar("exitto")->Set("main");

		//start the install in 500 ms, so we don't lag out the screen transition
		pBG->GetFunction("StartInstall")->sig_function.connect(&OnDMODInstallHTTPFinish);
		VariantList vList(pCrapComp);
        GetMessageManager()->CallEntityFunction(pBG, 500, "StartInstall", &vList);
		pStatus->GetVar("text")->Set("New .dmod file detected");
	} else
	{
		pBG->GetVar("autoplay")->Set(uint32(1));

		pBG->GetFunction("InitNetStuff")->sig_function.connect(&InitNetStuff);
        VariantList vList(pBG);
		GetMessageManager()->CallEntityFunction(pBG, 500, "InitNetStuff", &vList);
	}


	SlideScreen(pBG, true, 500);
	return pBG;
}

