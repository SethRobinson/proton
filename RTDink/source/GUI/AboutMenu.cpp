#include "PlatformPrecomp.h"
#include "AboutMenu.h"
#include "MainMenu.h"
#include "Entity/EntityUtils.h"
#include "PopUpMenu.h"

void AboutMenuAddScrollContent(Entity *pParent);

void AboutMenuOnSelect(VariantList *pVList) //0=vec2 point of click, 1=entity sent from
{
	Entity *pEntClicked = pVList->m_variant[1].GetEntity();
	LogMsg("Clicked %s entity at %s", pEntClicked->GetName().c_str(),pVList->m_variant[1].Print().c_str());
	Entity *pMenu = GetEntityRoot()->GetEntityByName("AboutMenu");

	//LogMsg("Clicked %s entity at %s", pEntClicked->GetName().c_str(),pVList->m_variant[0].Print().c_str());

	if (pEntClicked->GetName() == "Back")
	{
		DisableAllButtonsEntity(pEntClicked->GetParent());
		SlideScreen(pEntClicked->GetParent(), false);
		GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 500, "OnDelete", NULL);
		MainMenuCreate(pEntClicked->GetParent()->GetParent());
		return;
	}

	if (pEntClicked->GetName() == "mindwall_ad")
	{
		string url = "http://www.codedojo.com/?p=138";

		if (GetEmulatedPlatformID() == PLATFORM_ID_ANDROID)
		{
			url = "market://details?id=com.rtsoft.rtmindwall";
		} else if (GetEmulatedPlatformID() == PLATFORM_ID_IOS)
		{
			url = "http://www.rtsoft.com/mindwall/purchase.php";
		}


		PopUpCreate(pEntClicked->GetParent()->GetParent()->GetParent(), "Would you like to check out `wMind Wall?``", url,
			"cancel", "`wCancel", "url", "`wLaunch", true);
		return;
	}


	if (pEntClicked->GetName() == "dink_ad")
	{

		string url = "http://www.rtsoft.com/pages/dink.php";

		switch  (GetEmulatedPlatformID())
		{

		case PLATFORM_ID_WEBOS:
			//	url = "http://www.rtsoft.com/mindwall/purchase_webos.php";
			break;
		case PLATFORM_ID_IOS:
			break;

		case PLATFORM_ID_OSX:
			url = "http://itunes.apple.com/us/app/dink-smallwood-hd/id391690243?mt=8";
			break;

		case PLATFORM_ID_ANDROID:
			url = "market://details?id=com.rtsoft.rtdink";
			break;
		}

		PopUpCreate(pEntClicked->GetParent()->GetParent()->GetParent(), "Leave the game and check out Dink Smallwood HD?", url,
			"cancel", "`wCancel", "url", "`wLaunch", true);
		return;
	}

	if (pEntClicked->GetName() == "fling_ad")
	{
		string url = "http://tenonedesign.com/fling";
		PopUpCreate(pEntClicked->GetParent()->GetParent()->GetParent(), "Would you like to visit Ten One Design's webpage and learn more about the `wFling``?", url,
			"cancel", "`wCancel", "url", "`wLaunch", true);
		return;
	}

	if (pEntClicked->GetName() == "ds_ad")
	{

		string url = "http://www.rtsoft.com/pages/dscroll.php";

		if (GetEmulatedPlatformID() == PLATFORM_ID_ANDROID)
		{
			url = "market://details?id=com.rtsoft.rtdscroll";
		} else if (GetEmulatedPlatformID() == PLATFORM_ID_IOS)

		{
			url = "http://www.rtsoft.com/dscroll_iphone/purchase.php";
		}

		PopUpCreate(pEntClicked->GetParent()->GetParent()->GetParent(), "Would you like to check out `wDungeon Scroll``?", url,
			"cancel", "`wCancel", "url", "`wLaunch", true);
		return;
	}
	if (pEntClicked->GetName() == "dinknetwork")
	{
		PopUpCreate(pEntClicked->GetParent()->GetParent()->GetParent(), "Would you like to check out `wThe Dink Network``?", "http://www.dinknetwork.com?platform="+toString(GetEmulatedPlatformID()),
			"cancel", "`wCancel", "url", "`wLaunch", true);
		return;
	}
	if (pEntClicked->GetName() == "email")
	{
		PopUpCreate(pEntClicked->GetParent()->GetParent()->GetParent(), "Would you like to email `wsupport@rtsoft.com now``?", "mailto:support@rtsoft.com",
			"cancel", "`wCancel", "url", "`wEmail", true);
		return;
	}
	if (pEntClicked->GetName() == "twitter")
	{
		PopUpCreate(pEntClicked->GetParent()->GetParent()->GetParent(), "View the `wRTsoft twitter page``?", "http://twitter.com/rtsoft",
			"cancel", "`wCancel", "url", "`wLaunch", true);
		return;
	}

	if (pEntClicked->GetName() == "list")
	{
		PopUpCreate(pEntClicked->GetParent()->GetParent()->GetParent(), "Would you like to enter your email to subscribe to the `wRTsoft Newsletter``?", "http://www.rtsoft.com/lists/?p=subscribe",
			"cancel", "`wCancel", "url", "`wOh yeah!", true);
		return;
	}

	if (pEntClicked->GetName() == "forums")
	{
		PopUpCreate(pEntClicked->GetParent()->GetParent()->GetParent(), "Would you like to visit the `wRTsoft Dink Forums``?", "http://www.rtsoft.com/forums/forumdisplay.php?6-Dink-Smallwood",
			"cancel", "`wCancel", "url", "`wLaunch", true);
		return;
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

void AddBlurb(Entity* pParent, float x, float y, string fileName, string msg)
{
	
	Entity * pLogo = CreateOverlayButtonEntity(pParent, fileName, string("interface/")+fileName+".rttex", x, y);
	EntityRetinaRemapIfNeeded(pLogo, false, false, true);
	pLogo->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&AboutMenuOnSelect);


	CL_Vec2f imageSize = pLogo->GetVar("size2d")->GetVector2();

	float imagePaddingX = iPhoneMapX(20);

	CL_Vec2f vTextBoxPos(iPhoneMapX(x)+imageSize.x+imagePaddingX,y);
	CL_Vec2f vTextBounds(iPhoneMapX(400)-imageSize.x+imagePaddingX , iPhoneMapY(200));


	Entity *pEnt = CreateTextBoxEntity(pParent, "", vTextBoxPos, vTextBounds, msg);


}

void AboutMenuAddScrollContent(Entity *pParent)
{
	pParent = pParent->GetEntityByName("scroll_child");

	pParent->RemoveAllEntities();
	float x = 5;
	float y = 0;
	float ySpacer = iPhoneMapY(27);

	
	Entity *pTitle = CreateTextLabelEntity(pParent, "Title", x, 0, "About & Help"); 
	SetupTextEntity(pTitle, FONT_LARGE);
	y += iPhoneMapY(30);

	float blurbSpacingY = iPhoneMapX(27);

	/*
	if (IsIPAD())
	{
		AddBlurb(pParent, x, y, "fling_ad", "`6This game is compatible with the `wFling analog joystick``.  Tap the Fling logo to visit Ten One Design to learn more.");
		y += iPhoneMapY(blurbSpacingY);
		//y += ySpacer;
	}
	*/
	CL_Vec2f vTextBoxPos(iPhoneMapX(x),y);
	CL_Vec2f vTextBounds(iPhoneMapX(434), iPhoneMapY(200));
	

	string msg =
		GetAppName()+string("`$ ")+GetApp()->GetVersionString()+" Build "+toString(GetApp()->GetBuild())+"``\nCopyright (c) 1997-2018 Robinson Technologies\n"\
		"\nDink Script Version: `$1.10``\n\n"\
		;

	


if (IsDesktop())
{



	msg += \
		"Keyboard controls:\n\n"\
		"Esc ``-`8 Bring up the game menu/pause``\n"\
		"Arrow keys ``-`8 Movement/menu selections``\n"\
		"Ctrl ``- `8Attack\n``"\
		"Shift ``- `8Magic\n``"\
		"Enter ``- `8Inventory screen/Inventory select\n``"\
		"M ``- `8Show map\n\n``"\
		"TAB ``- `8Speed up game (hold it down)\n``"\
		"F4 ``- `8Quick state save\n``"\
		"F8 ``- `8Quick state load\n``"\
		"F1 ``- `8Dink HD Menu\n``"\
		"Alt-Enter ``- `8Toggle fullscreen\n``"\
		"Drag window corners ``- `8Changes screensize. Hold Shift to allow any aspect ratio\n``"\
	"\n`6Stuck? Try visiting `wdinknetwork.com`` or use google to find a walkthrough.\n"\
	"\n`wPushing:`` If you make Dink walk against an object for one second, he will begin to push it.  Useful when you see rocks blocking cave entrances.\n"\
	"\n`wTrees:`` Some trees can be burned down with magic to reveal secrets.\n"\
	"\n`wSave Machines:`` Use Save Machines frequently and don't only rely on quick saves as they may sometimes put you in a difficult spot.\n"\
	"\n`wAuto Save:`` Your game is automatically saved every five minutes to save slot 10 as long as you have more than 30% health.\n"\

	"\n`wQuick save/load:`` In addition to the full auto state save whenever you exit the game, and the normal save system, you can use `wQuick Save/Load`` from the pause menu.  It's very useful to beat a tough boss.  Each add-on you install will also remember its own unique save states as well.\n"\
	"\n`wTo use an existing DMOD directory:`` Start the game with a command line parm of `w-dmodpath <dmod dir path>`` to use it instead of the default dmod directory.  `w-game <dmod path>`` is also supported to jump right into a dmod.\n"\
"";
	
} else if (GetEmulatedPlatformID() == PLATFORM_ID_ANDROID)
{

	//android
	msg += \
		"`6Here are some tips and info to get started.\n"\
		"\n`wControl mode - Virtual Joypad:`` This is the default control method, simple to use.\n"\
		"\n`wControl mode - Drag Anywhere:`` This control scheme allows you to draw an angle with your finger and dink will walk in that angle. It doesn't matter where you draw it on the screen. As long as you don't let your finger up, he'll keep moving.\n"\
		"\n`wiCade:`` Pair the iCade with your device, then start the game.  Choose `$Use iCade Controller Mode`` from the options menu.\n"\
		"\n`wHW Keyboard:`` If your device has a hardware keyboard, you can use WASZ to move, I for inventory, shift for punch, enter/menu for magic, space to talk.\n"\

		"\n`wPushing:`` If you make Dink walk against an object for one second, he will begin to push it.  Useful when you see rocks blocking cave entrances.\n"\
		"\n`wTrees:`` Some trees can be burned down with magic to reveal secrets.\n"\
		"\n`wQuick save/load:`` In addition to the full auto state save whenever you exit the game, and the normal save system, you can use `wQuick Save/Load`` from the pause menu.  It's very useful to beat a tough boss.\n"\
		"\n`wSave Machines:`` Use Save Machines frequently and don't only rely on quick saves as they may sometimes put you in a difficult spot.\n"\
		"\n`wAuto Save:`` Your game is automatically saved every five minutes to save slot 10 as long as you have more than 30% health.\n"\
		"\n`wInstalling DMODS by Browse:`` Click Browse from the Add-On menu to see a selection of recommended DMODs to install.\n"\
		"\n`wInstalling DMODS from URL:`` Click Install from URL from the Add-On menu.\n"\
		"\n`wInstalling DMODS from SD card:`` Place a .dmod file on your SD card in `w/Android/data/com.rtsoft.dink/files`` and it will be installed the next time you play.\n"\
		"\n`wIf the game is slow:`` Try disabling the screen scroll effect and improved shadows in the options menu.\n"\
		;
} else  if (  GetEmulatedPlatformID() == PLATFORM_ID_WEBOS || GetEmulatedPlatformID() == PLATFORM_ID_BBX)
{

	//android
	msg += \
		"`6Here are some tips and info to get started.\n"\
		"\n`wControl mode - Virtual Joypad:`` This is the default control method, simple to use.\n"\
		"\n`wControl mode - Drag Anywhere:`` This control scheme allows you to draw an angle with your finger and dink will walk in that angle. It doesn't matter where you draw it on the screen. As long as you don't let your finger up, he'll keep moving.\n"\
	
		"\n`wPushing:`` If you make Dink walk against an object for one second, he will begin to push it.  Useful when you see rocks blocking cave entrances.\n"\
		"\n`wTrees:`` Some trees can be burned down with magic to reveal secrets.\n"\
		"\n`wQuick save/load:`` In addition to the full auto state save whenever you exit the game, and the normal save system, you can use `wQuick Save/Load`` from the pause menu.  It's very useful to beat a tough boss.\n"\
		"\n`wSave Machines:`` Use Save Machines frequently and don't only rely on quick saves as they may sometimes put you in a difficult spot.\n"\
		"\n`wAuto Save:`` Your game is automatically saved every five minutes to save slot 10 as long as you have more than 30% health.\n"\
		"\n`wInstalling DMODS by Browse:`` Click Browse from the Add-On menu to see a selection of recommended DMODs to install.\n"\
		"\n`wInstalling DMODS from URL:`` Click Install from URL from the Add-On menu.\n"\
		;
} else
{

	if (IsIPADSize)
	{

//iphone
	msg += \
		"`6Here are some tips and info to get started.\n"\
		"\n`wControls - Virtual Joypad:`` This is the default control method, simple to use.\n"\
		"\n`wControls - Drag Anywhere:`` This control scheme allows you to draw an angle with your finger and dink will walk in that angle. It doesn't matter where you draw it on the screen. As long as you don't let your finger up, he'll keep moving.\n"\
		"\n`wPushing:`` If you make Dink walk against an object for one second, he will begin to push it.  Useful when you see rocks blocking cave entrances.\n"\
		"\n`wTrees:`` Some trees can be burned down with magic to reveal secrets.\n"\
		"\n`wQuick save/load:`` In addition to the full auto state save whenever you exit the game, and the normal save system, you can use `wQuick Save/Load`` from the pause menu.  It's very useful to beat a tough boss.\n"\
		"\n`wSave Machines:`` Use Save Machines frequently and don't only rely on quick saves as they may sometimes put you in a difficult spot.\n"\
		"\n`wAuto Save:`` Your game is automatically saved every five minutes to save slot 10 as long as you have more than 30% health.\n"\
	;
	} else
	{
		msg += \
			"`6Here are some tips and info to get started.\n"\
			"\n`wControls - Virtual Joypad:`` This is the default control method, simple to use.\n"\
			"\n`wControls - Drag Anywhere:`` This control scheme allows you to draw an angle with your finger and dink will walk in that angle. It doesn't matter where you draw it on the screen. As long as you don't let your finger up, he'll keep moving.\n"\
			"\n`wPushing:`` If you make Dink walk against an object for one second, he will begin to push it.  Useful when you see rocks blocking cave entrances.\n"\
			"\n`wTrees:`` Some trees can be burned down with magic to reveal secrets.\n"\
			"\n`wQuick save/load:`` In addition to the full auto state save whenever you exit the game, and the normal save system, you can use `wQuick Save/Load`` from the pause menu.  It's very useful to beat a tough boss.\n"\
			"\n`wSave Machines:`` Use Save Machines frequently and don't only rely on quick saves as they may sometimes put you in a difficult spot.\n"\
			"\n`wAuto Save:`` Your game is automatically saved every five minutes to save slot 10 as long as you have more than 30% health.\n"\
			;
	}
}

if (!IsDesktop())
{


msg += \
"\n`wAdd-on warning:`` Depending on your device's speed and memory not all quest add-ons may be smoothly playable.\n"\
"\n`wOutdated instructions warning:`` Sometimes quest add-ons will refer to controls not available, like pressing a specific key.  Keep in mind most add-ons were developed for the desktop versions of the game.\n"\
"\n`wKeyboard:`` You can probably work around these issues by using the in-game keyboard option, available from the pause menu.\n";
}

	Entity *pEnt = CreateTextBoxEntity(pParent, "", vTextBoxPos, vTextBounds, msg);
	y += pEnt->GetVar("size2d")->GetVector2().y;
	y += ySpacer;

	//some special links:
	Entity *pButton;

	if (GetApp()->CanDownloadDMODS())
	{
		pButton = CreateTextButtonEntity(pParent, "dinknetwork", iPhoneMapX(x), y, "Visit the Dink Network (Dink files, forums and more)", true);
		pButton->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&AboutMenuOnSelect);
		y += pButton->GetVar("size2d")->GetVector2().y;
		y += ySpacer;
	}

// 
// 	pButton = CreateTextButtonEntity(pParent, "email", iPhoneMapX(x), y, "Have a suggestion, bug report, or need help? Tap here to email us", true);
// 	pButton->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&AboutMenuOnSelect);
// 	y += pButton->GetVar("size2d")->GetVector2().y;
// 	y += ySpacer;

// 
// 	pButton = CreateTextButtonEntity(pParent, "list", iPhoneMapX(x), y, "Subscribe to the RTsoft Newsletter", true);
// 	pButton->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&AboutMenuOnSelect);
// 	y += pButton->GetVar("size2d")->GetVector2().y;
// 	y += ySpacer;
	

	pButton = CreateTextButtonEntity(pParent, "twitter", iPhoneMapX(x), y, "Click here for the RTsoft twitter page", true);
	pButton->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&AboutMenuOnSelect);
	y += pButton->GetVar("size2d")->GetVector2().y;
	y += ySpacer;

// 
// 	pButton = CreateTextButtonEntity(pParent, "forums", iPhoneMapX(x), y, "Visit the official RTsoft forums", true);
// 	pButton->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&AboutMenuOnSelect);
// 	y += pButton->GetVar("size2d")->GetVector2().y;
// 	y += ySpacer;


	vTextBoxPos = CL_Vec2f(iPhoneMapX(x),y);
	msg = "\nIf you like Dink, please check out our other games:\n";
	pEnt = CreateTextBoxEntity(pParent, "", vTextBoxPos, vTextBounds, msg);
	y += pEnt->GetVar("size2d")->GetVector2().y;
	y += ySpacer;


	if (!IsLargeScreen())
	{
		blurbSpacingY = iPhoneMapX(95); //for whatever reason we need more on the small iphone screen
	}

	AddBlurb(pParent, x, y, "mindwall_ad", "`wMind Wall```8 is a unique 3D arcade puzzler that is instantly understood, beautifully simple to control, and diabolically difficult to master.");
	y += iPhoneMapY(blurbSpacingY);
	y += ySpacer;

	AddBlurb(pParent, x, y, "ds_ad", "`wDungeon Scroll```8 - If a word game and a dungeon crawler had a baby, this would be it. Arrange tiles to create magic words and blast rats, spiders and skeletons. But beware, a great evil looms beneath the depths.");
	y += iPhoneMapY(blurbSpacingY);
	y += ySpacer;

	if (IsDesktop())
	{
		string adText = "`wDink Smallwood HD for iOS```8 - Love Dink? Then get it on your iPhone and iPad!";
		
		if (GetEmulatedPlatformID() == PLATFORM_ID_WINDOWS || GetEmulatedPlatformID() == PLATFORM_ID_HTML5)
		{
			adText = "`wDink Smallwood HD for mobile```8 - Dink also has free native mobile versions available!";
		}
		AddBlurb(pParent, x, y, "dink_ad", adText);
	y += iPhoneMapY(blurbSpacingY);
	y += ySpacer;
	}



	//credits:

	pTitle = CreateTextLabelEntity(pParent, "Title", x, y, "Credits"); 
	SetupTextEntity(pTitle, FONT_LARGE);
	y += pTitle->GetVar("size2d")->GetVector2().y;
	y += ySpacer;

	vTextBoxPos = CL_Vec2f(iPhoneMapX(x),y);

	msg = "`8`$Dink Smallwood HD`` was created by `wSeth A. Robinson`` (code), `wJustin Martin`` (graphics) and includes additional contributions from Dan Walma.\n\n"\
		"`8The original `$Dink Smallwood`` was created by `wSeth A. Robinson`` (engine/scripting/music), `wJustin Martin`` (graphics), `wGreg Smith`` (scripting/music), and `wShawn Teal``. It also featured music by `wJoel Bakker`` and `wMitch Brink``."\
		"\n\nSpecial thanks to `wDan Walma`` and the `wdinksmallwood.net`` community for their `$Dink`` creations and support over the years!"\
		"\n\n"\
		"This product is less buggy thanks to:\n\n`w"\
		"Shawn Teal\n"\
		"Myra Russell\n"\
		"David Stathis\n"\
		"Steve Gargolinski\n"\
		"Linus Lindberg\n"\
		"Phil Hassey\n"\
		"Kyle/Ragura\n"\
		"John H. Anthony\n"\
		"L McMillan\n"\
		"scratcher\n"\
		"Skull\n"\
		"Yeoldetoast\n"\
		"metatarasal\n"\
		"Sparrowhawk\n"\
		"ToKu\n"\
		"synbi\n"\
		"Impact\n"\
		"Andrew Perry\n"\
		"Rob, Marcus, Davor\n"\
		"Brett Profitt\n"\
		"Chris Black\n"\
		"Erik Harrison\n"\
		"James Hutt\n" \
		"Robert J. Deans\n"\
		"liquidsnakehpks\n"\
		"Tal\n"\
		"Quiztis\n"\
		"rabidwolf9\n"\
		"RangerLord\n"\
		"LeProChaUn\n"\
		"DinkDude95\n"\
		"thenewguy\n"\
	
		
		;
	
			
		if (GetEmulatedPlatformID() == PLATFORM_ID_WINDOWS || GetEmulatedPlatformID() == PLATFORM_ID_IOS)
		{

		msg += "`8\nFMOD Sound System, copyright (c) Firelight Technologies Pty, Ltd., 1994-2010.";
		}
	

	pEnt = CreateTextBoxEntity(pParent, "", vTextBoxPos, vTextBounds, msg);
	y += pEnt->GetVar("size2d")->GetVector2().y;
	y += ySpacer;


	//ads
/*
	Entity *pLogo;
	if (IsLargeScreen())
	{
		pLogo = CreateOverlayButtonEntity(pParent, "logo", "interface/logo_88_88.rttex", x, y);
	} else
	{
		pLogo = CreateOverlayButtonEntity(pParent, "logo", "interface/rtsoft_logo.rttex", x, y);
	}

	pLogo->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&AboutMenuOnSelect);

	y += iPhoneMapY(86);

	//add the dscroll ad
	pLogo = CreateOverlayButtonEntity(pParent, "dscroll", "interface/ds_ad.rttex", x, y);
	EntityRetinaRemapIfNeeded(pLogo, false, true, true);
	pLogo->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&AboutMenuOnSelect);
	float textX = iPhoneMapX(170);

	//some text to go with the ad
	Entity * pTextEnt = CreateTextLabelEntity(pParent, "text", textX-20, y, "Do you like word games and\nkilling things? Then tap the\n"\
		"icon on the left to check out\nDungeon Scroll for iPhone!");
	SetupTextEntity(pTextEnt, FONT_SMALL, 0.8f);

	y += iPhoneMapY(120);

	//add the blip ad
	pLogo = CreateOverlayButtonEntity(pParent, "blip", "interface/ba_ad.rttex", x, y);
	EntityRetinaRemapIfNeeded(pLogo, false, true, true);
	pLogo->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&AboutMenuOnSelect);

	//some text to go with the ad
	pTextEnt = CreateTextLabelEntity(pParent, "text", textX-20, y, "Three player on one iPhone\nis now possible!  Tap the icon\nto see Blip Arcade!");
	SetupTextEntity(pTextEnt, FONT_SMALL, 0.8f);

	y += iPhoneMapY(120);

*/

	VariantList vList(pParent->GetParent());
    ResizeScrollBounds(&vList);
}

Entity * AboutMenuCreate( Entity *pParentEnt)
{
	
	Entity *pBG = NULL;
	pBG =  CreateOverlayEntity(pParentEnt, "AboutMenu", ReplaceWithDeviceNameInFileName("interface/iphone/bkgd_stone.rttex"), 0,0);

	pBG->SetName("AboutMenu");
	AddFocusIfNeeded(pBG, true, 500);
	pBG->AddComponent(new FocusRenderComponent);

	//add the header

	CL_Vec2f vTextAreaPos = iPhoneMap(2,10);
	float offsetFromBottom = iPhoneMapY(42);
	float offsetFromRight = iPhoneMapY(0);

	CL_Vec2f vTextAreaBounds = (GetScreenSize()- CL_Vec2f(offsetFromRight,offsetFromBottom))-vTextAreaPos;
	Entity *pScroll = pBG->AddEntity(new Entity("scroll"));
	pScroll->GetVar("pos2d")->Set(vTextAreaPos);
	pScroll->GetVar("size2d")->Set(vTextAreaBounds);
	pScroll->AddComponent(new TouchHandlerComponent);

	EntityComponent *pFilter = pScroll->AddComponent(new FilterInputComponent);
	EntityComponent *pScrollComp = pScroll->AddComponent(new ScrollComponent);
	EntityComponent *pScrollBarComp = pScroll->AddComponent(new ScrollBarRenderComponent); 	//also let's add a visual way to see the scroller position
	//pScroll->GetVar("color")->Set(MAKE_RGBA(61,155, 193, 255)); 
	Entity *pScrollChild = pScroll->AddEntity(new Entity("scroll_child"));
	pScrollComp->GetVar("fingerTracking")->Set(uint32(1));

	/*
	//too slow/broken on Android, we'll do it another way
	EntityComponent *pClip = pScroll->AddComponent(new RenderClipComponent);
	pClip->GetVar("clipMode")->Set(uint32(RenderClipComponent::CLIP_MODE_BOTTOM));
	*/

	Entity *pOverlay = CreateOverlayEntity(pBG, "", ReplaceWithDeviceNameInFileName("interface/iphone/bg_stone_overlay.rttex"), 0, GetScreenSizeYf()); 
	SetAlignmentEntity(pOverlay, ALIGNMENT_DOWN_LEFT);

	
	AboutMenuAddScrollContent(pBG);
	//	ZoomFromPositionEntity(pBG, CL_Vec2f(0, -GetScreenSizeYf()), 500);
	//the continue button
	Entity *pEnt;

	//pEnt = CreateOverlayRectEntity(pBG, CL_Rectf(0, GetScreenSizeYf()-offsetFromBottom, GetScreenSizeXf(), 320), MAKE_RGBA(0,0,0,100));

	eFont fontID = FONT_SMALL;

	pEnt = CreateTextButtonEntity(pBG, "Back", iPhoneMapX(5), iPhoneMapY(BACK_BUTTON_Y), "Back", false);
	pEnt->GetFunction("OnButtonSelected")->sig_function.connect(&AboutMenuOnSelect);
	SetupTextEntity(pEnt, fontID);
	AddHotKeyToButton(pEnt, VIRTUAL_KEY_BACK);

	SlideScreen(pBG, true, 500);

//	pBG->GetFunction("OnPostIntroTransition")->sig_function.connect(&OnPostIntroTransition);
//	GetMessageManager()->CallEntityFunction(pBG, 1000, "OnPostIntroTransition", &VariantList(pBG, string(""))); 


	return pBG;
}