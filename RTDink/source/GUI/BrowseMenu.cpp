#include "PlatformPrecomp.h"
#include "BrowseMenu.h"
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
#include "DMODMenu.h"
#include "Network/NetHTTP.h"
#include "Entity/HTTPComponent.h"


enum DMODSortEnum
{
	DMOD_SORT_RATING,
	DMOD_SORT_DATE,
	DMOD_SORT_ALPHABETICAL,
	//add above here
	DMOD_SORT_COUNT
};

int g_dmodSorting = DMOD_SORT_RATING;
int g_dmods_per_screen = 5;
int g_dmod_cur_page = 0;

struct DMODEntry
{
	
	bool operator < (const DMODEntry& str) const
	{
		return (m_rating < str.m_rating);
	}
	string m_name;
	string m_url;
	string m_author;
	float m_size;
	float m_rating;
	string m_description;
	string m_date;
	string m_version;
	string m_thumb;
};

vector<DMODEntry> g_dmodData;

void BrowseMenuAddScrollContent(Entity *pParent, TextScanner *t);
Entity * ShowScoreMessage(Entity *pMenu, string msg);

int SmartModulo(int a, int b)
{
	return (((a % b) + b) % b);
}

void BrowseMenuOnSelect(VariantList *pVList) //0=vec2 point of click, 1=entity sent from
{
	Entity *pEntClicked = pVList->m_variant[1].GetEntity();
	LogMsg("Clicked %s entity at %s", pEntClicked->GetName().c_str(),pVList->m_variant[1].Print().c_str());
	Entity *pMenu = GetEntityRoot()->GetEntityByName("BrowseMenu");

	//LogMsg("Clicked %s entity at %s", pEntClicked->GetName().c_str(),pVList->m_variant[0].Print().c_str());

	if (pEntClicked->GetName() == "Back")
	{
		SlideScreen(pEntClicked->GetParent(), false);
		GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 500, "OnDelete", NULL);
		DMODMenuCreate(pEntClicked->GetParent()->GetParent(), true);
		return;
	}
	
	int totalDMODPages = g_dmodData.size() / g_dmods_per_screen;


	if (pEntClicked->GetName() == "Next")
	{
		g_dmod_cur_page++;
		g_dmod_cur_page = SmartModulo(g_dmod_cur_page, totalDMODPages+1);
		BrowseMenuAddScrollContent(pMenu, NULL);
		return;
	}


	if (pEntClicked->GetName() == "Prev")
	{
		g_dmod_cur_page--;
		g_dmod_cur_page = SmartModulo(g_dmod_cur_page, totalDMODPages+1);
		BrowseMenuAddScrollContent(pMenu, NULL);
		return;
	}

	if (pEntClicked->GetName() == "Label")
	{
		g_dmodSorting = ( (g_dmodSorting + 1) % DMOD_SORT_COUNT);
		g_dmod_cur_page = 0;
		BrowseMenuAddScrollContent(pMenu, NULL);
		return;
	}


	//they must have clicked on a DMOD if they got this far
	if (pEntClicked->GetName() == "install")
	{
		//save position of the scroll bar first
		EntityComponent *pScrollerComp = pMenu->GetEntityByName("scroll")->GetComponentByName("Scroll");
		//GetApp()->GetVar("DMODProgress2d")->Set(pScrollerComp->GetVar("progress2d")->GetVector2());

		string dmodurl = pEntClicked->GetParent()->GetVar("dmodurl")->GetString();
		string dmodName = pEntClicked->GetParent()->GetVar("dmodtitle")->GetString();

		SendFakeInputMessageToEntity(GetEntityRoot(), MESSAGE_TYPE_GUI_CLICK_END, pVList->m_variant[0].GetVector2()); //otherwise the menu may never get the touch release message
	
		DisableAllButtonsEntity(pMenu);
		SlideScreen(pMenu, false);
		GetMessageManager()->CallEntityFunction(pMenu, 500, "OnDelete", NULL);
		DMODInstallMenuCreate(pEntClicked->GetParent()->GetParent()->GetParent()->GetParent()->GetParent(), dmodurl, GetDMODRootPath() , "", true, dmodName);

	}

	//GetEntityRoot()->PrintTreeAsText(); //useful for debugging
}


string VersionToString(float v)
{
	char tmp[32];
	sprintf(tmp, "%.2f", v);
	if (tmp[3] == '0')
	{
		//cut off this, not needed
		tmp[3] = 0;
	}

	return string(tmp);

}
void AddEntryBar(Entity *pParent, float &x, float &y, DMODEntry &s, int index)
{
	Entity *pBG = CreateOverlayEntity(pParent, s.m_name, ReplaceWithLargeInFileName("interface/iphone/browse_dmod_bar.rttex"), x, y);

	y += pBG->GetVar("size2d")->GetVector2().y;

	pBG->GetVar("dmodurl")->Set(s.m_url); //save for later
	pBG->GetVar("dmodtitle")->Set(s.m_name); //save for later
	//title

	string displayName = s.m_name;
	int maxNameChars = 30;
	int maxDescripChars = 57;


	switch (g_dmodSorting)
	{

	case DMOD_SORT_DATE:
			break;

	default:
		maxNameChars = 37;
		break;
	}


	if (displayName.length() > maxNameChars)
	{
		TruncateString(displayName, maxNameChars);
		displayName += "...";
	}

	string displayDescription = s.m_description;
	
	if (displayDescription.length() > maxDescripChars)
	{
		TruncateString(displayDescription, maxDescripChars);
		displayDescription += "...";
	}

	char stTemp[512];
	 

	switch (g_dmodSorting)
	{

	case DMOD_SORT_DATE:
		sprintf(stTemp, "#%d %s `6%s`` R:`6%.1f``", index+1, s.m_date.c_str(), displayName.c_str(), s.m_rating);
		break;

	default:
		sprintf(stTemp, "#%d `6%s`` R:`6%.1f``", index+1, displayName.c_str(), s.m_rating);
		break;
	}
	
	
	Entity *pTitle = CreateTextLabelEntity(pBG, "title", iPhoneMapX2X( 16) ,iPhoneMapY2X( 13), stTemp);
	Entity *pDescription = CreateTextBoxEntity(pBG, "descrip", iPhoneMap2X(16, 34), iPhoneMap2X(425, 54), "`6"+ displayDescription);

	//Entity *pIcon = CreateButtonHotspot(pBG, "icon_hotspot", GetDMODBarIconOffset(), GetDMODBarIconSize(), Button2DComponent::BUTTON_STYLE_CLICK_ON_TOUCH_IGNORE_DRAGGING);
	//SetTouchPaddingEntity(pIcon, CL_Rectf(0,iPhoneMapY2X(5),0,iPhoneMapY2X(5)));
	//pIcon->GetFunction("OnButtonSelected")->sig_function.connect(&BrowseMenuOnSelect);

	//processing the icon image might be slow, lets do it a bit later, sequencing the timing by using the y, which should be going up
	
	//the delete icon part
	{
		CL_Vec2f iconPos = iPhoneMap2X(379,10);
		//CL_Vec2f iconSize = iPhoneMap2X(27, 27);
		Entity *pIcon = CreateOverlayButtonEntity(pBG, "install", ReplaceWithLargeInFileName("interface/iphone/browse_install.rttex"), iconPos.x, iconPos.y);
		SetButtonStyleEntity(pIcon,  Button2DComponent::BUTTON_STYLE_CLICK_ON_TOUCH_IGNORE_DRAGGING);
		SetTouchPaddingEntity(pIcon, CL_Rectf(0,0,0,0));
		pIcon->GetFunction("OnButtonSelected")->sig_function.connect(&BrowseMenuOnSelect);
	}

	//add animation effect
	//ZoomToPositionFromThisOffsetEntity(pBG, CL_Vec2f(GetScreenSizeXf(), 0), 500, INTERPOLATE_EASE_TO, 10);
}

void UpdateBrowseControlButtons(Entity *pParent)
{
	Entity *pEnt = NULL;

	pParent->RemoveEntityByName("Prev");
	pParent->RemoveEntityByName("Next");
	pParent->RemoveEntityByName("Label");

	int totalDMODPages = g_dmodData.size() / g_dmods_per_screen;

	//if (g_dmod_cur_page > 0)
	{
		pEnt = CreateTextButtonEntity(pParent, "Prev", iPhoneMapX(25 + 50 * 1), iPhoneMapY(BACK_BUTTON_Y), "`6<<``Prev", false);
		pEnt->GetFunction("OnButtonSelected")->sig_function.connect(&BrowseMenuOnSelect);
		SetButtonRepeatDelayMS(pEnt, 50);
	}

	//if (g_dmod_cur_page < totalDMODPages)
	{
		pEnt = CreateTextButtonEntity(pParent, "Next", iPhoneMapX(25 + 50 * 2), iPhoneMapY(BACK_BUTTON_Y), "Next`6>>``", false);
		pEnt->GetFunction("OnButtonSelected")->sig_function.connect(&BrowseMenuOnSelect);
		SetButtonRepeatDelayMS(pEnt, 50);
	}

	
	string sorting;
	
	switch (g_dmodSorting)
	{

	case DMOD_SORT_DATE:
		sorting = "newest";
		break;
	case DMOD_SORT_ALPHABETICAL:
		sorting = "alphabetical";
		break;
	case DMOD_SORT_RATING:
		sorting = "ratings";
		break;

	default:
		sorting = "error";
		break;
	}
	string label = "Page `6" + toString(g_dmod_cur_page+1) + "``/`6" + toString(totalDMODPages+1) + "`` - Sorting by `6<``"+ sorting+"`6>``";

	pEnt = CreateTextButtonEntity(pParent, "Label", iPhoneMapX(25 + 50 * 4), iPhoneMapY(BACK_BUTTON_Y), label, false);
	pEnt->GetFunction("OnButtonSelected")->sig_function.connect(&BrowseMenuOnSelect);

}

bool DmodEntrySortByName(const DMODEntry& lhs, const DMODEntry& rhs)
{
	return lhs.m_name < rhs.m_name;
}

bool DmodEntrySortByDate(const DMODEntry& lhs, const DMODEntry& rhs)
{
	return lhs.m_date < rhs.m_date;
}

void BrowseMenuAddScrollContent(Entity *pParent, TextScanner *t)
{
	pParent = pParent->GetEntityByName("scroll_child");

	pParent->RemoveAllEntities();
	float x = iPhoneMapX(5);
	float y = 0;

	//Entity *pEnt;


	int dmodsAdded = 0;

	if (g_dmodData.empty())
	{
		string msg = t->GetMultipleLineStrings("msg", "|");
		vector<string> p = StringTokenize(msg, "|");

		if (p.size() == 2 && p[1].length() > 1)
		{

			StringReplace("<cr>", "\n", p[1]);
			//add a message we just downloaded
			CL_Vec2f vTextBoxPos(x + iPhoneMapX(5), y);
			CL_Vec2f vTextBounds(iPhoneMapX(434), iPhoneMapY(200));
			Entity *pEnt = CreateTextBoxEntity(pParent, "", vTextBoxPos, vTextBounds, p[1]);
			y += pEnt->GetVar("size2d")->GetVector2().y;
			y += iPhoneMapY(5);

		}

		string line;

		//populate our internal DB

		g_dmodData.clear();

		for (int i = 0; i < t->GetLineCount(); i++)
		{

			//LogMsg(line.c_str());
			vector<string> p = StringTokenize(t->GetLine(i), "|");
			if (p.size() < 8) continue; //don't care

			DMODEntry s;
			s.m_name = p[0];
			if (s.m_name == "Title") continue;
			s.m_url = p[1];
			s.m_author = p[2];
			s.m_size = atof(p[3].c_str());
			s.m_rating = atof(p[4].c_str());
			s.m_description = p[5];
			s.m_version = p[6];
			s.m_date = p[7].c_str();
			StringReplace("V", "", s.m_version);
			StringReplace("v", "", s.m_version);

			s.m_thumb = atof(p[8].c_str());


			g_dmodData.push_back(s);
			dmodsAdded++;


		}
	}

	switch (g_dmodSorting)
	{

	case DMOD_SORT_ALPHABETICAL:
		sort(g_dmodData.begin(), g_dmodData.end(), DmodEntrySortByName);
		break;

	case DMOD_SORT_DATE:

		sort(g_dmodData.rbegin(), g_dmodData.rend(), DmodEntrySortByDate);
		break;

	default:
		sort(g_dmodData.rbegin(), g_dmodData.rend());
		break;
	}
	//actually add them to the list

	for (int i = g_dmods_per_screen*(g_dmod_cur_page); i < (g_dmods_per_screen*g_dmod_cur_page)+g_dmods_per_screen && i < g_dmodData.size(); i++)
	{
		AddEntryBar(pParent, x, y, g_dmodData[i], i);
		//y += iPhoneMapY(1);
	}
  
    VariantList vList(pParent->GetParent());
	ResizeScrollBounds(&vList);
	DisableHorizontalScrolling(pParent->GetParent());

	UpdateBrowseControlButtons(pParent->GetParent()->GetParent());
}



void OnDownloadError(VariantList *pVList)
{
	NetHTTP::eError e = (NetHTTP::eError)pVList->m_variant[1].GetUINT32();

	string msg = "`4Unable to connect.  Try later. ("+toString(e)+")";
	if (e == NetHTTP::ERROR_COMMUNICATION_TIMEOUT)
	{
		msg = "`4Connection timed out. Try Later.";
	}

	ShowScoreMessage(pVList->m_variant[0].GetComponent()->GetParent(), msg);
}


Entity * ShowScoreMessage(Entity *pMenu, string msg)
{
	Entity *pInfo = pMenu->GetEntityByName("Info");
	if (pInfo)
	{
		pInfo->GetComponentByName("TextRender")->GetVar("text")->Set(msg);
		pInfo->RemoveComponentByName("Typer"); // a thing that types stuff
	} else
	{
		pInfo = CreateTextLabelEntity(pMenu, "Info", iPhoneMapX(130), iPhoneMapY(220), msg);
	}

	return pInfo;
}


void OnDownloadHTTPFinish(VariantList *pVList)
{
	Entity *pMenu = pVList->m_variant[0].GetComponent()->GetParent();

	TextScanner t((char*)pVList->m_variant[1].GetString().c_str());
	string line;

	//	ShowScoreMessage(pMenu, t.GetParmString("msg",1));
	ShowScoreMessage(pMenu, "");
	//GetApp()->GetVar("score_msg")->Set(t.GetParmString("msg",1));

	//GetHighScoreManager()->SetupOnlineScores(t);
	//GetApp()->GetVar("cur_score")->Set(uint32(0));  //reset score drawing
	//ScoresAddStuffToScroll(NULL);
	BrowseMenuAddScrollContent(pMenu, &t);

	//	AddHighScores(pMenu, -1);
}


void DownloadDMODList(Entity *pMenu)
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

	uint32 port = 80;
	//GetApp()->GetServerInfo(url, port);

	string host = "www.dinknetwork.com";
	string url = "api";


	v.m_variant[0].Set(host);
	v.m_variant[1].Set(port);
	v.m_variant[2].Set(url);
	v.m_variant[3].Set(uint32(NetHTTP::END_OF_DATA_SIGNAL_HTTP)); //need this for it to detect a disconnect instead of the weird RTsoft symbol
	pComp->GetFunction("Init")->sig_function(&v);
	pComp->GetFunction("OnError")->sig_function.connect(&OnDownloadError);
	pComp->GetFunction("OnFinish")->sig_function.connect(&OnDownloadHTTPFinish);

	
	Entity *pEnt = ShowScoreMessage(pMenu, "`6Downloading dmod data");
	EntityComponent *pTyper = pEnt->AddComponent(new TyperComponent);
	pTyper->GetVar("text")->Set("..........");
	pTyper->GetVar("speedMS")->Set(uint32(50));
	//KillScores();

}

void BrowseOnPostIntroTransition(VariantList *pVList)
{

	Entity *pBG = pVList->Get(0).GetEntity();
	
	//fresh download or use cached data?

	if (g_dmodData.empty())
	{
		DownloadDMODList(pBG);
	}
	else
	{
		//use what we got
		BrowseMenuAddScrollContent(pBG, NULL);
	}
	
	//CreateQuickTipFirstTimeOnly(pBG, "interface/iphone/quicktip_dmod.rttex", false);
}

Entity * BrowseMenuCreate( Entity *pParentEnt )
{

	SyncPersistentData();

	//GetMessageManager()->SendGame(MESSAGE_TYPE_PLAY_MUSIC, "audio/title.mp3", 200);
	GetBaseApp()->ClearError();

	Entity *pBG = NULL;

	if (IsLargeScreen())
	{
		pBG =  CreateOverlayEntity(pParentEnt, "BrowseMenu", ReplaceWithDeviceNameInFileName("interface/iphone/bkgd_stone.rttex"), 0,0);

	} else
	{
		//pBG = CreateOverlayEntity(pParentEnt, "BrowseMenu", "interface/iphone/dmod_bg.rttex", 0,0);
		pBG = CreateOverlayEntity(pParentEnt, "BrowseMenu", ReplaceWithDeviceNameInFileName("interface/iphone/bkgd_stone.rttex"), 0,0);
	}

	pBG->SetName("BrowseMenu");
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

	/*
	Entity *pLabel = CreateTextLabelEntity(pBG, "scanning", GetScreenSizeXf()/2, GetScreenSizeYf()/2, "Updating add-on browse list...");
	SetAlignmentEntity(pLabel, ALIGNMENT_CENTER);
	FadeOutAndKillEntity(pLabel, true, 300, 501);

	*/

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


	//	ZoomFromPositionEntity(pBG, CL_Vec2f(0, -GetScreenSizeYf()), 500);
	//the continue button
	Entity *pEnt;

	//pEnt = CreateOverlayRectEntity(pBG, CL_Rectf(0, GetScreenSizeYf()-offsetFromBottom, GetScreenSizeXf(), 320), MAKE_RGBA(0,0,0,100));

	eFont fontID = FONT_SMALL;

	pEnt = CreateTextButtonEntity(pBG, "Back", iPhoneMapX(25), iPhoneMapY(BACK_BUTTON_Y), "Back", false);
	pEnt->GetFunction("OnButtonSelected")->sig_function.connect(&BrowseMenuOnSelect);
	SetupTextEntity(pEnt, fontID);
	AddHotKeyToButton(pEnt, VIRTUAL_KEY_BACK);


	SlideScreen(pBG, true, 500);
	pBG->GetFunction("OnPostIntroTransition")->sig_function.connect(&BrowseOnPostIntroTransition);
	VariantList vList(pBG, string(""));
    GetMessageManager()->CallEntityFunction(pBG, 500, "OnPostIntroTransition", &vList); 

	return pBG;
}