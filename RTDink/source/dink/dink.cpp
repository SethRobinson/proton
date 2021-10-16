#include "PlatformPrecomp.h"
#include "dink.h"
#include "FFReader.h"
#include "../GUI/GameMenu.h"
#include <cstdlib>
#include "../GUI/PauseMenu.h"
#include "../GUI/QuickTipMenu.h"
#include "ScriptAccelerator.h"
#include "Renderer/SoftSurface.h"
#include "FileSystem/StreamingInstance.h"
#include <time.h>

const int C_DINK_MAX_ITEMS = 16;
const int C_DINK_MAX_MAGICS = 8;

void ThinkSprite(int h, bool get_frame);
void ApplyAspectRatioGLMatrix();
void ScanSeqFilesIfNeeded(int seq);
bool pre_figure_out(const char *line, int load_seq, bool bLoadSpriteOnly);

#define C_DINK_SCREEN_TRANSITION_TIME_MS 400


const float SAVE_FORMAT_VERSION = 3.1f;
const int C_DINK_FADE_TIME_MS = 300;

const float G_TRANSITION_SCALE_TRICK = 1.01f;
bool g_forceBuildBackgroundFromScratch = false;

bool g_dinkMouseRightClick = false;


float g_dinkFadeAlpha = 0;
DinkGlobals g_dglo;
DinkGlobalsStatic g_dglos; //static data, made to write/read from disk

int32 g_spriteRank[C_MAX_SPRITES_AT_ONCE];
string g_lastSaveSlotFileSaved;

int32 C_DINK_MEM_MAX_ALLOWED = (1024*1024*20);
int32 C_DINK_TEX_MEM_MAX_ALLOWED = (1024*1024*60);

//avoid texture thrashing with this
int32 C_DINK_MEM_CACHE_MAX_ALLOWED_AFTER_A_DUMP = (1024*1024*8);
int32 C_DINK_TEX_MEM_MAX_ALLOWED_AFTER_A_DUMP = (1024*1024*16);
uint32 g_DebugKeyTimer = 0;

#ifdef _WIN32
	extern bool g_bHasFocus;
	#define C_DINK_KEYBOARD_INPUT
#endif

std::map<std::string, int32>* g_customSpriteMap[C_MAX_SPRITES_AT_ONCE];

eDinkGameState g_dinkGameState = DINK_GAME_STATE_NOT_PLAYING;
int water_timer;
bool fire_forward;
int fire_flip;

bool no_cheat;
bool g_itemScreenActive;

int32 last_saved_game;
bool g_b_kill_app; //if true, will close app as soon as the message pump is
Surface g_transitionSurf;
Surface g_onePixelSurf; //used for drawing blocks of color

SoundBankDummy soundbank[num_soundbanks+1];
soundstruct soundinfo[num_soundbanks+1];
seth_sound g_soundInfo[max_sounds];

player_short_info short_play;

char *g_scriptBuffer[C_MAX_SCRIPTS]; //pointers to buffers we may need
refinfo *g_scriptInstance[C_MAX_SCRIPTS];
SpriteStruct g_sprite[C_MAX_SPRITES_AT_ONCE]; //max sprite control systems at once

ScriptAccelerator g_scriptAccelerator[C_MAX_SCRIPTS];
hardness g_hmap;

seth_joy sjoy;

map_info g_MapInfo;

int32 * pvision, * plife, * presult, * pspeed, * ptiming, *plifemax, *pexper, *pmap,
*pstrength, * pcur_weapon,* pcur_magic, *pdefense, *pgold, *pmagic, *plevel, *plast_text, *pmagic_level;
int32 *pupdate_status, *pmissile_target, *penemy_sprite, *pmagic_cost, *pmissle_source;

bool no_transition;
bool g_abort_this_flip;

bool sound_on;
bool turn_on_plane;

char returnstring[200];
char slist[10][200];
int32 g_nlist[10];
char in_default[200];
bool g_bInitiateScreenMove;
bool g_bTransitionActive;
bool g_script_debug_mode =false;
uint16 decipher_savegame;
uint32 g_soundTimer = 0;

void FreeSequence(int h);

LPDIRECTDRAWSURFACE     lpDDSBackGround;       // Offscreen surface 
LPDIRECTDRAWSURFACE     lpDDSBuffer;       // Offscreen surface 
LPDIRECTDRAWSURFACE     g_tileScreens[C_TILE_SCREEN_COUNT];       // Game pieces
LPDIRECTDRAWSURFACE     g_pSpriteSurface[C_MAX_SPRITES];

bool IsCorruptedSeq(int seq);
//redink1 added for recursive scope checking
void decipher_string(char line[200], int script);
void check_midi(void);
bool ReloadSequence(int seqID, int frame = 0, bool bScanOnly = false);
void kill_sprite_all (int sprite);
bool add_time_to_saved_game(int num);
void move(int u, int amount, char kind,  char kindy);
void draw_box(rtRect32 box, int color);
void run_through_tag_list_push(int h);
void random_blood(int mx, int my, int h);
int check_if_move_is_legal(int u);
void change_dir_to_diag( int *dir);

int hurt_thing(int h, int damage, int special);
void kill_all_scripts_for_real(void);
bool check_sprite_status(int spriteID);
bool InitSound();
bool DestroySound( void );
int get_var(int script, char* name);
void init_scripts(void);
int load_script(const char *pScript, int sprite, bool set_sprite, bool bQuietError=false);
void update_status_all(void);
int add_sprite(int x1, int y, int brain,int pseq, int pframe );
void load_info(); //redink1
void add_exp(int num, int h, bool addEvenIfNotLastSpriteHit=false);
bool locate(int script, char proc[20]);

bool SoundStopEffect( int sound );
void draw_status_all(void);
bool SoundDestroyEffect( int sound );
int SoundPlayEffect( int sound,int min,int plus ,int sound3d, bool repeat);
void SoundLoadBanks( void);
bool StopMidi(void);
bool check_seq_status(int h, int frame = 0);
bool PlayMidi(const char *sFileName);
void get_word(char line[300], int word, char *crap);

void run_script (int script);

void program_idata(void);
void BuildScreenBackground( bool bFullRebuild = true, bool buildImageFromScratch = true);
int realhard(int tile);
void kill_repeat_sounds_all( void );
int process_line (int script, char *s, bool doelse);
void SetDefaultVars(bool bFullClear);
int getpic(int h);
bool check_pic_status(int picID);
bool get_box (int spriteID, rtRect32 * box_crap, rtRect32 * box_real );
void fill_screen(int num);

#ifdef WINAPI
#define C_MAX_BACKGROUND_SPRITES_AT_ONCE 300 //too many and it will slow down

#else
#define C_MAX_BACKGROUND_SPRITES_AT_ONCE 200 //too many and it will slow down

#endif

void BackgroundSpriteManager::Clear()
{
	m_sprites.clear();
}

void BackgroundSpriteManager::Render(LPDIRECTDRAWSURFACE lpdest)
{
	//LogMsg("Drawing %d sprites", m_sprites.size());
	rtRect32 box_crap,box_real;
	DDBLTFX     ddbltfx;
	ddbltfx.dwSize = sizeof( ddbltfx);
	ddbltfx.dwFillColor = 0;

	deque<BackgroundSprite>::iterator itor = m_sprites.begin();

	while (itor != m_sprites.end())
	{

		if (!check_pic_status(itor->pic))
		{
		#ifdef _DEBUG
					LogMsg("Hmm, bad tilepic at %d..", itor->pic);
		#endif
			continue;
		}
			lpdest->Blt(&itor->dstRect, g_pSpriteSurface[itor->pic],
			&itor->srcRect  , DDBLT_KEYSRC ,&ddbltfx);
	
		itor++;
	}
	

}

void BackgroundSpriteManager::Add(int spriteID)
{

	BackgroundSprite s;
	s.pic = getpic(spriteID);
	get_box(spriteID, &s.dstRect, &s.srcRect);
	m_sprites.push_back(s);

	if (m_sprites.size() > C_MAX_BACKGROUND_SPRITES_AT_ONCE)
	{
		m_sprites.pop_front();
	}
}

int next_raise(void)
{
	int crap = *plevel;
	assert(crap >= 0 && crap < 200);
	int num = ((100 * crap) * crap);

	if (num > 99999) num = 99999;
	return(num);
}

void OffsetRect(rtRect32 *pR, int x, int y)
{
	pR->AdjustPosition(x,y);
}
void OffsetRect(rtRect *pR, int x, int y)
{
    pR->AdjustPosition(x,y);
}


void InflateRect(rtRect32 *pR, int x, int y)
{
	pR->Inflate(x,y);
}

char * rt_ltoa( int num, char *pDest, int bufSize)
{
	sprintf(pDest, "%d", num);
	return pDest;
}

void KillScriptAccelerators()
{
	for (int i=0; i < C_MAX_SCRIPTS; i++)
	{
		g_scriptAccelerator[i].Kill();
	}
}

void ResetDinkTimers()
{
	g_soundTimer = 0;
	g_DebugKeyTimer = 0;
}

void SoundLoadBanks( void) {}

void OneTimeDinkInit()
{

	for (int i = 0; i < C_MAX_SPRITES_AT_ONCE; i++)
	{
		g_customSpriteMap[i] = NULL;
	}

	//defaults for memory class 1 device
	C_DINK_MEM_MAX_ALLOWED = (1024*1024*10);
	C_DINK_TEX_MEM_MAX_ALLOWED = (1024*1024*30);

	//avoid texture thrashing with this
	C_DINK_MEM_CACHE_MAX_ALLOWED_AFTER_A_DUMP = (1024*1024*8);
	C_DINK_TEX_MEM_MAX_ALLOWED_AFTER_A_DUMP = (1024*1024*16);

	
	if (GetDeviceMemoryClass() >= C_DEVICE_MEMORY_CLASS_2)
	{
		C_DINK_MEM_MAX_ALLOWED = (1024*1024*20);
		C_DINK_TEX_MEM_MAX_ALLOWED = (1024*1024*60);

		//avoid texture thrashing with this
		C_DINK_MEM_CACHE_MAX_ALLOWED_AFTER_A_DUMP = (1024*1024*16);
		C_DINK_TEX_MEM_MAX_ALLOWED_AFTER_A_DUMP = (1024*1024*32);
	}

	if (GetDeviceMemoryClass() >= C_DEVICE_MEMORY_CLASS_3)
	{
		C_DINK_MEM_MAX_ALLOWED = (1024*1024*40);
		C_DINK_TEX_MEM_MAX_ALLOWED = (1024*1024*100);

		//avoid texture thrashing with this
		C_DINK_MEM_CACHE_MAX_ALLOWED_AFTER_A_DUMP = (1024*1024*32);
		C_DINK_TEX_MEM_MAX_ALLOWED_AFTER_A_DUMP = (1024*1024*64);
	}

	if (GetDeviceMemoryClass() >= C_DEVICE_MEMORY_CLASS_4)
	{
		C_DINK_MEM_MAX_ALLOWED = (1024*1024*80);
		C_DINK_TEX_MEM_MAX_ALLOWED = (1024*1024*200);

		//avoid texture thrashing with this
		C_DINK_MEM_CACHE_MAX_ALLOWED_AFTER_A_DUMP = (1024*1024*64);
		C_DINK_TEX_MEM_MAX_ALLOWED_AFTER_A_DUMP = (1024*1024*128);
	}


	static bool bInitted = false;
	g_dglos.g_DinkUpdateTimerMS = 0;
	if (bInitted) return;

	lpDDSBackGround = NULL;       // Offscreen surface 
	lpDDSBuffer = NULL;       // Offscreen surface 

	for (int i=0; i < C_TILE_SCREEN_COUNT; i++)
	{
		g_tileScreens[i] = NULL;
	}

	bInitted = true;
	g_dglos.g_curPicIndex = 1; //we actually need this set before finiObjects is called

	memset(g_dglos.g_picInfo, 0, sizeof(pic_info)*C_MAX_SPRITES);
	memset(g_sprite, 0, sizeof(SpriteStruct) *C_MAX_SPRITES_AT_ONCE);
	memset(g_dglos.g_seq, 0, sizeof(sequence) * C_MAX_SEQUENCES);
	memset(g_dglos.g_scriptCallback, 0, sizeof(call_back) * C_MAX_SCRIPT_CALLBACKS);

	memset(&g_dglos.g_hitmap, 0, sizeof(hit_map));
	memset(&g_hmap, 0, sizeof(hardness));

//just to help me keep track of memory better
	GetApp()->ModMemUsed(sizeof(pic_info)*C_MAX_SPRITES);
	GetApp()->ModMemUsed(sizeof(SpriteStruct) *C_MAX_SPRITES_AT_ONCE);
	GetApp()->ModMemUsed(sizeof(sequence) * C_MAX_SEQUENCES);
	GetApp()->ModMemUsed( sizeof(call_back) * C_MAX_SCRIPT_CALLBACKS);

	GetApp()->ModMemUsed( sizeof(hit_map));
	GetApp()->ModMemUsed( sizeof(hardness));
}

void finiObjects()
{
	if (last_saved_game > 0)
	{
		LogMsg("Modifying saved game.");

		if (!add_time_to_saved_game(last_saved_game))
			LogMsg("Error modifying saved game.");
	}

	for (int i=1; i < C_MAX_SPRITES_AT_ONCE; i++)
	{
		SAFE_DELETE(g_customSpriteMap[i]);
	}

	memset(g_sprite, 0, sizeof(SpriteStruct) *C_MAX_SPRITES_AT_ONCE);

	for (int i=1; i < C_MAX_SPRITES; i++)
	{
		SAFE_DELETE(g_pSpriteSurface[i]);
	}
	g_dglos.g_curPicIndex = 1;

	kill_all_scripts_for_real();

	for (int i=0; i < C_TILE_SCREEN_COUNT; i++)
	{
		SAFE_DELETE(g_tileScreens[i]);
	}

	SAFE_DELETE(lpDDSBuffer);
	SAFE_DELETE(lpDDSBackGround);

	g_transitionSurf.HardKill();

	KillVideoEngine();

	g_dglo.m_iniScanner.Kill();
	
	g_b_kill_app = true;
	LogMsg("Game shutdown run");

	GetAudioManager()->KillCachedSounds(false, true, 0, 100, true);
} 

void clear_talk(void)
{
	memset(&g_dglos.g_talkInfo, 0, sizeof(g_dglos.g_talkInfo));
	g_dglos.g_playerInfo.mouse = 0;
}

#if !defined PLATFORM_LINUX && !defined PLATFORM_HTML5
/* Case insensitive strncmp. Non-ISO, deprecated. */

int strnicmp(const char *pStr1, const char *pStr2, size_t Count)
{
	char c1, c2;
	int v;

	if (Count == 0)
		return 0;

	do {
		c1 = *pStr1++;
		c2 = *pStr2++;
		/* the casts are necessary when pStr1 is shorter & char is signed */
		v = (uint32) tolower(c1) - (uint32) tolower(c2);
	} while ((v == 0) && (c1 != '\0') && (--Count > 0));

	return v;
}
#endif

bool compare(char *orig, char *comp)
{
	int len;
	len = strlen(comp);
	if (strlen(orig) != len) return(false);
	
	if (strnicmp(orig,comp,len) == 0)
	{
		return(true);
	}

	//Msg("I'm sorry, but %s does not equal %s.",orig, comp);
	return(false);
}

bool CreateBufferFromWaveFile(char* FileName, uint32 dwBuf)
{
	g_soundInfo[dwBuf].m_fileName = FileName;
	return true;
}

bool getkey(int key)
{
	if (sjoy.realkey[key]) return(true); else return(false);
}

//add hardness from a sprite

int getpic(int h)
{
	if (g_sprite[h].pseq == 0) return(0);
	if (g_sprite[h].pseq > C_MAX_SEQUENCES)
	{

		LogMsg("Sequence %d?  But max is %d!", g_sprite[h].pseq, C_MAX_SEQUENCES);
		return(0);
	}
	return(g_dglos.g_seq[g_sprite[h].pseq].frame[g_sprite[h].pframe]);
}




string GetFileLocationString(const string fName)
{
	if (!fName[0])
	{
		return fName;
	}

	if(GetEmulatedPlatformID() == PLATFORM_ID_PSVITA)
	{

	}

	if (!g_dglo.m_dmodGameDir.empty())
	{
		//extra checks for dmod stuff

		if (FileExists(g_dglo.m_dmodGamePathWithDir+fName))
		{
			//found it
			return g_dglo.m_dmodGamePathWithDir+fName;
		}
	}

	if (GetEmulatedPlatformID() == PLATFORM_ID_IOS || GetEmulatedPlatformID() == PLATFORM_ID_OSX || GetEmulatedPlatformID() == PLATFORM_ID_HTML5)
	{
		//actually, we need to do this to grab stuff from the .pak we added
		if (FileExists(g_dglo.m_gameDir+fName))
		{
			//found it
			return g_dglo.m_gameDir+fName;
		}
	}

	//default, from dink dir
	return g_dglo.m_gamePathWithDir + fName;
}

void add_hardness (int sprite, int num)
{

	for (int  xx = g_sprite[sprite].x + g_dglos.g_picInfo[getpic(sprite)].hardbox.left; xx < g_sprite[sprite].x + g_dglos.g_picInfo[getpic(sprite)].hardbox.right; xx++)
	{
		for (int yy = g_sprite[sprite].y + g_dglos.g_picInfo[getpic(sprite)].hardbox.top; yy < g_sprite[sprite].y + g_dglos.g_picInfo[getpic(sprite)].hardbox.bottom; yy++)
		{
			if ( (xx-20 > 600) | (xx-20 < 0)| (yy > 400) | (yy < 0))
			{
			} else

				g_dglos.g_hitmap.x[xx-20].y[yy] = num;
		}   
	}
}


void setup_anim (int seq, int sequence,int delay)
{
	
	for (int o = 1; o <= g_dglos.g_seq[sequence].last; o++)
	{
		g_dglos.g_seq[seq].frame[o] = g_dglos.g_seq[sequence].s+o;
		g_dglos.g_seq[seq].originalFrame[o] = g_dglos.g_seq[sequence].s + o; //remember this for later, handy after set_frame_frame's are used but we still want the
		//original frame's offsets
		g_dglos.g_seq[seq].delay[o] = delay;
		g_dglos.g_picInfo[g_dglos.g_seq[seq].frame[o]].m_parentSeq = seq; //so we can know who the parent is if we need to reload later
		g_dglos.g_picInfo[g_dglos.g_seq[seq].frame[o]].m_parentFrame = o; //so we can know who the parent is if we need to reload later
	}

#ifdef _DEBUG
	if (seq == 855)
	{

	LogMsg("yo, yo!");
	
	}
#endif
	//g_dglos.g_seq[seq].frame[g_dglos.g_seq[sequence].last+1] = 0;
}

byte get_hard(int h,int x1, int y1)
{
	int value;

	//redink1 fix for screenlock bug
	if (g_dglos.screenlock)
	{
		if ( x1 < 0 && x1 > -5 ) x1 = 0;
		else if ( x1 > 599 && x1 < 605 ) x1 = 599;

		if ( y1 < 0 && y1 > -5 ) y1 = 0;
		else if ( y1 > 399 && x1 < 405 ) y1 = 399;
	}
	if (x1 < 0 || y1 < 0 || x1 > 599 || y1 > 399) return(0);
	value =  g_dglos.g_hitmap.x[x1].y[y1];    
	
	//if (GetApp()->GetGhostMode() && value != 100) return 0; //cheat enabled

	return(value);  
}

byte get_hard_play(int h,int x1, int y1)
{

	int value;
	x1 -= 20;

	//redink1 fix for screenlock bug
	if (g_dglos.screenlock)
	{
		if ( x1 < 0 && x1 > -5 ) x1 = 0;
		else if ( x1 > 599 && x1 < 605 ) x1 = 599;

		if ( y1 < 0 && y1 > -5 ) y1 = 0;
		else if ( y1 > 399 && x1 < 405 ) y1 = 399;
	}
	if (x1 < 0 || y1 < 0 || x1 > 599 || y1 > 399) return(0);

	value =  g_dglos.g_hitmap.x[x1].y[y1];    

	if (value > 100)
	{
		if (g_dglos.g_smallMap.sprite[value-100].prop != 0) 
		{
			g_dglos.flub_mode = value;
			value = 0;
		}
	}

	//if (GetApp()->GetGhostMode() && value != 100) return 0; //cheat enabled

	return(value);  
}


byte get_hard_map(int h,int x1, int y1)
{
	if ((x1 < 0) || (y1 < 0)) return(0);
	if ((x1 > 599) ) return(0);
	if (y1 > 399) return(0);

	int til = (x1 / 50) + ( ((y1 / 50)) * 12);
	int offx = x1 - ((x1 / 50) * 50);
	int offy = y1 - ((y1 / 50) * 50);

	//Msg("tile %d ",til);

	return( g_hmap.tile[ realhard(til )  ].x[offx].y[offy]);
}

void fill_hardxy(rtRect32 box)
{
	//Msg("filling hard of %d %d %d %d", box.top, box.left, box.right, box.bottom);

	if (box.right > 599) box.right = 600; //redink1 screenlock bug
	if (box.top < 0) box.top = 0;
	if (box.bottom > 399) box.bottom = 400; //redink1 screenlock bug
	if (box.left < 0) box.left = 0;

	for (int x1 = box.left; x1 < box.right; x1++)
	{
		for (int y1 = box.top; y1 < box.bottom; y1++)
		{
			g_dglos.g_hitmap.x[x1].y[y1] = get_hard_map(0,x1,y1);
		}
	}
}

void add_exp(int num, int h, bool addEvenIfNotLastSpriteHit)
{
	//redink1 fix - made work with all sprites when using add_exp DinkC command
	if (addEvenIfNotLastSpriteHit == false)
	{
		if (g_sprite[h].last_hit != 1) return;
	}

	if (num > 0)
	{

		check_sprite_status(h);
		//add experience
		*pexper += num;
		int crap2 = add_sprite(g_sprite[h].x,g_sprite[h].y,8,0,0);

		g_sprite[crap2].y -= g_dglos.g_picInfo[g_dglos.g_seq[g_sprite[h].pseq].frame[g_sprite[h].pframe]].yoffset;
		g_sprite[crap2].x -= g_dglos.g_picInfo[g_dglos.g_seq[g_sprite[h].pseq].frame[g_sprite[h].pframe]].xoffset;
		g_sprite[crap2].y -= g_dglos.g_picInfo[g_dglos.g_seq[g_sprite[h].pseq].frame[g_sprite[h].pframe]].box.bottom / 3;
		g_sprite[crap2].x += g_dglos.g_picInfo[g_dglos.g_seq[g_sprite[h].pseq].frame[g_sprite[h].pframe]].box.right / 5;
		g_sprite[crap2].y -= 30;
		g_sprite[crap2].speed = 1;
		g_sprite[crap2].hard = 1;
		g_sprite[crap2].brain_parm = 5000;  
		g_sprite[crap2].my = -1;
		g_sprite[crap2].kill = 1000;
		g_sprite[crap2].dir = 8;
		g_sprite[crap2].damage = num;
		if (*pexper > 99999) *pexper = 99999;
	}

}

int realhard(int tile)
{
	//  if (pam.t[tile].num > 3000) Msg("Hard is %d", pam.t[tile].num );
	if (g_dglos.g_smallMap.t[tile].althard > 0) return(g_dglos.g_smallMap.t[tile].althard); else return(g_hmap.index[g_dglos.g_smallMap.t[tile].num]);
}

void fill_whole_hard(void)
{

	for (int til=0; til < 96; til++)
	{
		int offx = (til * 50 - ((til / 12) * 600));
		int offy = (til / 12) * 50;

		for (int x = 0; x < 50; x++)
		{
			for (int y = 0; y < 50; y++)
			{
				int tileToWrite = realhard(til);
				if (tileToWrite >= 0 && tileToWrite < 800)
				{
					g_dglos.g_hitmap.x[offx + x].y[offy + y] = g_hmap.tile[tileToWrite].x[x].y[y];
				}
				else
				{
					LogMsg("Avoiding crash, it tried to write hardness data to block %d from illegal tile %d",til, tileToWrite);
					goto skip;
				}
			}
		}

	skip:;

	}
}

void DrawCollision()
{
	rtRect32 box_crap;
	DDBLTFX     ddbltfx;


	for (int x1=0; x1 < 600; x1++)
		for (int y1=0; y1 < 400; y1++)
		{
			if (g_dglos.g_hitmap.x[x1].y[y1] == 1) 
			{
				
				ddbltfx.dwFillColor = 1;
				
				ddbltfx.dwSize = sizeof(ddbltfx);
				box_crap.top = y1;
				box_crap.bottom = y1+1;
				box_crap.left = x1+g_gameAreaLeftOffset; //20 is to compensate for the border
				box_crap.right = x1+1+g_gameAreaLeftOffset;
				lpDDSBack->Blt(&box_crap ,NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);
		
			}

			if (g_dglos.g_hitmap.x[x1].y[y1] == 2) 
			{
				
				ddbltfx.dwFillColor = 128;
				
				ddbltfx.dwSize = sizeof(ddbltfx);
				box_crap.top = y1;
				box_crap.bottom = y1+1;
				box_crap.left = x1+g_gameAreaLeftOffset; //20 is to compensate for the border
				box_crap.right = x1+1+g_gameAreaLeftOffset;
				lpDDSBack->Blt(&box_crap ,NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);
				
			}


			if (g_dglos.g_hitmap.x[x1].y[y1] == 3) 
			{
				
					ddbltfx.dwFillColor = 45;
				ddbltfx.dwSize = sizeof(ddbltfx);
				box_crap.top = y1;
				box_crap.bottom = y1+1;
				box_crap.left = x1+g_gameAreaLeftOffset; //20 is to compensate for the border
				box_crap.right = x1+1+g_gameAreaLeftOffset;
				lpDDSBack->Blt(&box_crap ,NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);
			
			}

			if (g_dglos.g_hitmap.x[x1].y[y1] > 100)
			{
				if (g_dglos.g_smallMap.sprite[  (g_dglos.g_hitmap.x[x1].y[y1]) - 100].prop == 1)
				{
					
					
						ddbltfx.dwFillColor = 20;

				//draw a little pixel
					ddbltfx.dwSize = sizeof(ddbltfx);
					box_crap.top = y1;
					box_crap.bottom = y1+1;
					box_crap.left = x1+g_gameAreaLeftOffset; //20 is to compensate for the border
					box_crap.right = x1+1+g_gameAreaLeftOffset;
					lpDDSBack->Blt(&box_crap ,NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);
				} else
				{
#ifdef _DEBUG
	/*
	if (x1 == 423+20 && y1 == 120)
	{
					//if (Random(200) == 0)
					LogMsg("Drawing g_dglos.g_smallMap.sprite %d's hardness..", (g_dglos.g_hitmap.x[x1].y[y1]) - 100);
					int hh= 5;
	}
	*/
#endif
					//draw a little pixel
					
					if (x1 > 580)
					{
						ddbltfx.dwFillColor = 70;

					} else
					{

					ddbltfx.dwFillColor = 23;
					}
					ddbltfx.dwSize = sizeof(ddbltfx);
					box_crap.top = y1;
					box_crap.bottom = y1+1;
					box_crap.left = x1+g_gameAreaLeftOffset; //20 is to compensate for the border
					box_crap.right = x1+1+g_gameAreaLeftOffset;
					lpDDSBack->Blt(&box_crap ,NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);
					
				}
			}
		}
}


bool LoadTileScreenIfNeeded(int h, bool &bRequireRebuildOut)
{
	 bRequireRebuildOut = false;

	string fName = "tiles/ts";
	if (h < 10) fName += "0";
	fName += toString(h)+".bmp";

	
	if (g_dglos.g_playerInfo.tile[h].file[0] != 0)
	{
		//LogMsg("We should load %s", g_dglos.g_playerInfo.tile[h].file);
		fName = ToLowerCaseString(g_dglos.g_playerInfo.tile[h].file);
	}

	if (g_tileScreens[h])
	{

#ifdef _DEBUG
		//LogMsg("tilescreen %s already loaded, skipping", fName.c_str());
#endif

#ifdef _DEBUG
		if (g_tileScreens[h] && g_tileScreens[h]->m_pSurf->GetSurfaceType() != SoftSurface::SURFACE_PALETTE_8BIT)
		{
			//we're going to require a hicolor backbuffer.
			if (lpDDSBackGround->m_pSurf->GetSurfaceType() == SoftSurface::SURFACE_PALETTE_8BIT)
			{
				LogMsg("BIG ERROR: Detected high color tilescreen bmps.  Converting backbuffers to 32 bit on the fly.");

			}
		}
#endif

		return true; //already loaded
	}

	assert(!g_tileScreens[h]);
#ifdef _DEBUG
LogMsg("Loading tilescreen %s", fName.c_str());
#endif
	g_tileScreens[h] = LoadBitmapIntoSurface(GetFileLocationString(fName).c_str(), TRANSPARENT_NONE, IDirectDrawSurface::MODE_NORMAL);
	
	if (g_tileScreens[h] && g_tileScreens[h]->m_pSurf->GetSurfaceType() != SoftSurface::SURFACE_PALETTE_8BIT)
	{
		//we're going to require a hicolor backbuffer.
		if (lpDDSBackGround->m_pSurf->GetSurfaceType() == SoftSurface::SURFACE_PALETTE_8BIT)
		{
			LogMsg("Detected high color tilescreen bmps.  Converting backbuffers to 32 bit on the fly.");

			//switch it
			delete lpDDSBackGround;
			lpDDSBackGround = InitOffscreenSurface(C_DINK_SCREENSIZE_X, C_DINK_SCREENSIZE_Y, IDirectDrawSurface::MODE_SHADOW_GL, true);
			
			
				DDBLTFX     ddbltfx;
				ddbltfx.dwFillColor = g_dglos.last_fill_screen_palette_color;
				lpDDSBackGround->Blt(NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);
			
			bRequireRebuildOut = true;

		}

	}

	if (g_tileScreens[h]->m_pSurf->GetSurfaceType() == SoftSurface::SURFACE_PALETTE_8BIT && lpDDSBuffer->m_pSurf->GetSurfaceType() == SoftSurface::SURFACE_PALETTE_8BIT)
	//if (0)

	{
		//test to make sure the palettes are the same.. some dmods change them slightly which breaks things (Cast Awakening)

		if (!g_tileScreens[h]->m_pSurf->IsPaletteTheSame(lpDDSBuffer->m_pSurf->GetPalette(), lpDDSBuffer->m_pSurf->GetPaletteColorCount()))
		{
	
			if (lpDDSBackGround->m_pSurf->GetSurfaceType() == SoftSurface::SURFACE_PALETTE_8BIT)
			{
				//switch it
				delete lpDDSBackGround;
				lpDDSBackGround = InitOffscreenSurface(C_DINK_SCREENSIZE_X, C_DINK_SCREENSIZE_Y, IDirectDrawSurface::MODE_SHADOW_GL, true);
				bRequireRebuildOut = true;

				DDBLTFX     ddbltfx;
				ddbltfx.dwFillColor = g_dglos.last_fill_screen_palette_color;
				lpDDSBackGround->Blt(NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);

			}

			LogMsg("Detected tilescreen palette doesn't match backbuffers. Converting backbuffers to 32 bit on the fly.");
			delete lpDDSBuffer;
			lpDDSBuffer = InitOffscreenSurface(C_DINK_SCREENSIZE_X, C_DINK_SCREENSIZE_Y, IDirectDrawSurface::MODE_SHADOW_GL, true);
		}
	}

	if (g_tileScreens[h]->m_pSurf->GetSurfaceType() == SoftSurface::SURFACE_NONE)
	{
	//some kind of error
		LogMsg("Error loading tilescreen %d", h);
		delete g_tileScreens[h];
		g_tileScreens[h] = 0;
	}
	return g_tileScreens[h] != NULL;

}

//this is completely different than how the original dink worked.  The original Dink used GetTickCount() which drastically changed
//depending on when you last rebooted your computer.  The new way just uses the clock in the save game which is accurate, but the result
//is if we continue a save created with another version of dink, it's likely sprites will never come back.


void fix_dead_sprites( void )
{

	for (int i = 1; i < 100; i++)
	{
		if (g_dglos.g_playerInfo.spmap[*pmap].type[i] == 6)
		{
			if (g_dglos.g_playerInfo.spmap[*pmap].last_time > GetApp()->GetGameTick())
			{
#ifdef _DEBUG
				LogMsg("Woah, crazy time on dead map sprite (%d) detected", i);
#endif
				//g_dglos.g_playerInfo.spmap[*pmap].last_time = GetApp()->GetGameTick();
				//I don't think this is needed, because it's reset elsewhere to the current time if visited
			}
			if  ((g_dglos.g_dinkTick > (g_dglos.g_playerInfo.spmap[*pmap].last_time +  300000)) ||
				(g_dglos.g_dinkTick  +400000 < g_dglos.g_playerInfo.spmap[*pmap].last_time +  300000) )
			{
				//this sprite can come back online now
				g_dglos.g_playerInfo.spmap[*pmap].type[i] = 0;
			}
		}

		if (g_dglos.g_playerInfo.spmap[*pmap].type[i] == 7)
		{

			if (g_dglos.g_dinkTick > (g_dglos.g_playerInfo.spmap[*pmap].last_time +  180000))
			{
				//this sprite can come back online now
				g_dglos.g_playerInfo.spmap[*pmap].type[i] = 0;
			}
		}

		if (g_dglos.g_playerInfo.spmap[*pmap].type[i] == 8)
		{

			if (g_dglos.g_dinkTick > (g_dglos.g_playerInfo.spmap[*pmap].last_time +  60000))
			{
				//this sprite can come back online now
				g_dglos.g_playerInfo.spmap[*pmap].type[i] = 0;
			}
		}
	}
}

void load_map(const int num)
{
	FILE *          fp;
	int holdme,lsize;

	//LogMsg("Loading map %d...",num);
	g_dglos.m_bRenderBackgroundOnLoad = true;

	StreamingInstance *pFile = GetFileManager()->GetStreaming(GetFileLocationString(g_dglos.current_map), NULL, false);

//	fp = fopen( GetFileLocationString(g_dglos.current_map).c_str(), "rb");

	//g_dglos.last_fill_screen_palette_color = 0; //it's ok to forget the last fillscreen command

	if (!pFile)
	{
		LogMsg("Cannot find %s file!!!",g_dglos.current_map);
		return;
	}

	//redink1 set correctly so Dink appears on mini-map for warps and such
	//doesn't work, because 'num' is actually the offset in map.dat, not the map screen number
	//if (map.indoor[num] == 0)
	//   play.last_map = num;

	lsize = sizeof(struct small_map);
	holdme = (lsize * (num-1));
	//fseek( fp, holdme, SEEK_SET);
	
	
	//pFile->SeekFromStart(holdme); //only will work if the file is uncompressed in the zip

		//another way is this..
	const int bufferSize = 1024*4;

	char buffer[bufferSize];

	while (holdme > 0)
	{
		int bytesRead = pFile->Read((byte*)buffer, rt_min(holdme, bufferSize));
		holdme -= bytesRead;
	}
	
	//Msg("Trying to read %d bytes with offset of %d",lsize,holdme);
	pFile->Read((byte*)&g_dglos.g_smallMap, lsize);
	//int shit = fread( &g_dglos.g_smallMap, lsize, 1, fp);       /* current player */
	//       Msg("Read %d bytes.",shit);
	//if (shit == 0) LogMsg("ERROR:  Couldn't read map %d?!?", num);
//	fclose(fp);
	delete pFile;

	g_sprite[1].move_active = false;
	g_sprite[1].move_nohard = false;
	g_sprite[1].freeze = false;
	g_dglos.screenlock = 0;
	*pvision = 0; //reset vision
	fill_whole_hard();
	fix_dead_sprites();          
	check_midi();
}

void save_game(int num)
{
	FILE *          fp;
	char crap[256];
	//redink1 created this
	char info_temp[200];
	
	CreateDirectoryRecursively(g_dglo.m_savePath, g_dglo.m_gameDir);
	sprintf(crap, "%ssave%d.dat", g_dglo.m_savePath.c_str(), num);
	fp = fopen(crap, "wb");
	g_lastSaveSlotFileSaved = crap;

	if (!fp)
	{
		LogError("Unable to save game to %s", crap);
		return;
	}
	//lets set some vars first

	g_dglos.g_playerInfo.x = g_sprite[1].x;
	g_dglos.g_playerInfo.y = g_sprite[1].y;
	g_dglos.g_playerInfo.version = C_DINK_VERSION;
	g_dglos.g_playerInfo.pseq =  g_sprite[1].pseq;
	g_dglos.g_playerInfo.pframe =    g_sprite[1].pframe;
	g_dglos.g_playerInfo.seq    =    g_sprite[1].seq;
	g_dglos.g_playerInfo.frame  =    g_sprite[1].frame;
	g_dglos.g_playerInfo.size   =    g_sprite[1].size;
	g_dglos.g_playerInfo.dir    =    g_sprite[1].dir;
	g_dglos.g_playerInfo.strength = g_sprite[1].strength;
	g_dglos.g_playerInfo.defense  =  g_sprite[1].defense;
	g_dglos.g_playerInfo.que  =  g_sprite[1].que;
	g_dglos.g_playerInfo.minutes = ( GetBaseApp()->GetGameTick()-g_dglos.time_start) / (1000*60);
	//g_dglos.g_playerInfo.m_gameTime = GetBaseApp()->GetGameTick();
	//g_dglos.time_start = GetBaseApp()->GetGameTick();
	g_dglos.g_playerInfo.base_idle = g_sprite[1].base_idle;
	g_dglos.g_playerInfo.base_walk = g_sprite[1].base_walk;
	g_dglos.g_playerInfo.base_hit = g_sprite[1].base_hit;

	//redink1 - save game things for storing new map, palette, and tile information
	strncpy(g_dglos.g_playerInfo.mapdat, g_dglos.current_map, 50);
	strncpy(g_dglos.g_playerInfo.dinkdat, g_dglos.current_dat, 50);

	//redink1 code for custom save game names
	strcpy(info_temp,g_dglos.save_game_info);
	decipher_string(info_temp, 0);
	strncpy(g_dglos.g_playerInfo.gameinfo,info_temp,77);
	//sprintf(play.gameinfo, "Level %d",*plevel);

	last_saved_game = num;
	fwrite(&g_dglos.g_playerInfo,sizeof(g_dglos.g_playerInfo),1,fp);
	fclose(fp);

	//SyncPersistentData();

}


void kill_all_vars(void)
{
	memset(&g_dglos.g_playerInfo, 0, sizeof(g_dglos.g_playerInfo));
}

bool attach(void)
{

	for (int i = 1; i < max_vars; i++)
	{
		if (compare((char*)"&life", g_dglos.g_playerInfo.var[i].name))
		{
			plife = &g_dglos.g_playerInfo.var[i].var;

		}
		if (compare((char*)"&vision", g_dglos.g_playerInfo.var[i].name))
		{
			pvision = &g_dglos.g_playerInfo.var[i].var;

		}
		if (compare((char*)"&result", g_dglos.g_playerInfo.var[i].name))
		{
			presult = &g_dglos.g_playerInfo.var[i].var;

		}
		if (compare((char*)"&speed", g_dglos.g_playerInfo.var[i].name))
		{
			pspeed = &g_dglos.g_playerInfo.var[i].var;

		}
		if (compare((char*)"&timing", g_dglos.g_playerInfo.var[i].name))
		{
			ptiming = &g_dglos.g_playerInfo.var[i].var;
		}

		if (compare((char*)"&lifemax", g_dglos.g_playerInfo.var[i].name))
		{
			plifemax = &g_dglos.g_playerInfo.var[i].var;
		}

		if (compare((char*)"&exp", g_dglos.g_playerInfo.var[i].name))  pexper = &g_dglos.g_playerInfo.var[i].var;
		if (compare((char*)"&strength", g_dglos.g_playerInfo.var[i].name))  pstrength = &g_dglos.g_playerInfo.var[i].var;   
		if (compare((char*)"&defense", g_dglos.g_playerInfo.var[i].name))  pdefense = &g_dglos.g_playerInfo.var[i].var;   
		if (compare((char*)"&gold", g_dglos.g_playerInfo.var[i].name))  pgold = &g_dglos.g_playerInfo.var[i].var;   
		if (compare((char*)"&magic", g_dglos.g_playerInfo.var[i].name))  pmagic = &g_dglos.g_playerInfo.var[i].var;   
		if (compare((char*)"&level", g_dglos.g_playerInfo.var[i].name))  plevel = &g_dglos.g_playerInfo.var[i].var;   
		if (compare((char*)"&player_map", g_dglos.g_playerInfo.var[i].name))  pmap = &g_dglos.g_playerInfo.var[i].var;   
		if (compare((char*)"&cur_weapon", g_dglos.g_playerInfo.var[i].name))  pcur_weapon = &g_dglos.g_playerInfo.var[i].var;   
		if (compare((char*)"&cur_magic", g_dglos.g_playerInfo.var[i].name))  pcur_magic = &g_dglos.g_playerInfo.var[i].var;   
		if (compare((char*)"&last_text", g_dglos.g_playerInfo.var[i].name))  plast_text = &g_dglos.g_playerInfo.var[i].var;   
		if (compare((char*)"&magic_level", g_dglos.g_playerInfo.var[i].name))  pmagic_level = &g_dglos.g_playerInfo.var[i].var;   
		if (compare((char*)"&update_status", g_dglos.g_playerInfo.var[i].name))
		{
			pupdate_status = &g_dglos.g_playerInfo.var[i].var;
		}
		if (compare((char*)"&missile_target", g_dglos.g_playerInfo.var[i].name))  pmissile_target = &g_dglos.g_playerInfo.var[i].var;   
		if (compare((char*)"&enemy_sprite", g_dglos.g_playerInfo.var[i].name))  penemy_sprite = &g_dglos.g_playerInfo.var[i].var;   
		if (compare((char*)"&magic_cost", g_dglos.g_playerInfo.var[i].name))  pmagic_cost = &g_dglos.g_playerInfo.var[i].var;   
		if (compare((char*)"&missle_source", g_dglos.g_playerInfo.var[i].name))  pmissle_source = &g_dglos.g_playerInfo.var[i].var;   
	}

	if (!pstrength)
	{
		LogError("Corrupted save or .c file?  strength global var not set.");
		return false;
	}
	g_dglos.g_guiStrength = *pstrength;
	g_dglos.g_guiMagic = *pmagic;
	g_dglos.g_guiGold = *pgold;
	g_dglos.g_guiDefense = *pdefense;
	g_dglos.g_guiLife = *plife;
	g_dglos.g_guiLifeMax = *plifemax;
	g_dglos.g_guiExp = *pexper;
	g_dglos.g_guiMagicLevel = *pmagic_level;
	g_dglos.g_guiRaise = next_raise();

	return true; //no error

}


bool add_time_to_saved_game(int num)
{
	/*
	FILE *          fp;
	char crap[256];

	sprintf(crap, "%ssave%d.dat", g_dglo.m_gamePath.c_str(), num);

	fp = fopen(crap, "rb");
	if (!fp)
	{
		LogMsg("Couldn't load save game %d", num);
		return(false);
	}
	else
	{
		fread(&g_dglos.g_playerInfo,sizeof(g_dglos.g_playerInfo),1,fp);
		fclose(fp);
	}

	//great, now let's resave it with added time
	LogMsg("Ok, adding time.");

	g_dglos.g_playerInfo.minutes += 0;



	sprintf(crap, "save%d.dat", num);
	fp = fopen(crap, "wb");
	if (fp)
	{
		fwrite(&g_dglos.g_playerInfo,sizeof(g_dglos.g_playerInfo),1,fp);
		fclose(fp);
	}
	LogMsg("Wrote it.(%d of time)", g_dglos.g_playerInfo.minutes);
	*/

	return(true);   
}

bool load_game(int num)
{
	FILE *          fp;
	char crap[256];
	
	//lets get rid of our magic and weapon scripts
	if (g_dglos.weapon_script != 0)
	{
		if (locate(g_dglos.weapon_script, "DISARM")) 
		{
			run_script(g_dglos.weapon_script);
		} 
	}

	if (g_dglos.magic_script != 0) if (locate(g_dglos.magic_script, "DISARM")) run_script(g_dglos.magic_script);

	g_dglos.g_bowStatus.active = false;
	g_dglos.weapon_script = 0;
	g_dglos.magic_script = 0;
	g_dglos.midi_active = true;

	if (last_saved_game > 0)
	{
		LogMsg("Modifying saved game.");

		if (!add_time_to_saved_game(last_saved_game))
			LogMsg("Error modifying saved game.");
	}
	StopMidi();

	for (int i=1; i < C_MAX_SPRITES_AT_ONCE; i++)
	{
		
		kill_sprite_all(i);
	}

	SetDefaultVars(false);
	
	g_sprite[1].active = true;
	g_dglos.g_gameMode = 2;
	g_sprite[1].noclip = 0;
	g_sprite[1].brain = 1;
	g_sprite[1].nocontrol = 0;

	sprintf(crap, "%ssave%d.dat", g_dglo.m_savePath.c_str(), num);

	fp = fopen(crap, "rb");
	if (!fp)
	{
		LogMsg("Couldn't load save game %d", num);
		return(false);
	}
	else
	{
		fread(&g_dglos.g_playerInfo,sizeof(g_dglos.g_playerInfo),1,fp);
		fclose(fp);

		//redink1 - new map, if exist
		if (strlen(g_dglos.g_playerInfo.mapdat) > 0 && strlen(g_dglos.g_playerInfo.dinkdat) > 0)
		{
			strcpy(g_dglos.current_map, g_dglos.g_playerInfo.mapdat);
			strcpy(g_dglos.current_dat, g_dglos.g_playerInfo.dinkdat);
			ToLowerCase(g_dglos.current_map);
			ToLowerCase(g_dglos.current_dat);
			load_info();
		}
				
		g_sprite[1].damage = 0;
		g_sprite[1].x = g_dglos.g_playerInfo.x;
		g_sprite[1].y = g_dglos.g_playerInfo.y;
		g_dglos.walk_off_screen = 0;
		g_sprite[1].nodraw = 0;
		g_dglos.g_pushingEnabled = 1;
		g_sprite[1].pseq = g_dglos.g_playerInfo.pseq;
		g_sprite[1].pframe = g_dglos.g_playerInfo.pframe;
		g_sprite[1].size = g_dglos.g_playerInfo.size;
		g_sprite[1].seq = g_dglos.g_playerInfo.seq;
		g_sprite[1].frame = g_dglos.g_playerInfo.frame;
		g_sprite[1].dir = g_dglos.g_playerInfo.dir;
		g_sprite[1].strength = g_dglos.g_playerInfo.strength;
		g_sprite[1].defense = g_dglos.g_playerInfo.defense;
		g_sprite[1].que = g_dglos.g_playerInfo.que;

		if (g_dglos.g_playerInfo.m_gameTime != 0)
		{
			GetApp()->SetGameTick(g_dglos.g_playerInfo.m_gameTime);
		} else
		{
			//convert from old style of dink timing
			GetApp()->SetGameTick(g_dglos.g_playerInfo.minutes*1000*60);
			g_dglos.time_start = 0;
		}
		//g_dglos.time_start = GetBaseApp()->GetGameTick();
		g_dglos.g_dinkTick = GetTick(TIMER_GAME);
		g_sprite[1].base_idle = g_dglos.g_playerInfo.base_idle;
		g_sprite[1].base_walk = g_dglos.g_playerInfo.base_walk;
		g_sprite[1].base_hit = g_dglos.g_playerInfo.base_hit;

		int script = load_script("main", 0, true);
		locate(script, "main");
		run_script(script);
		//lets attach our vars to the scripts

		attach();    
		
		*pupdate_status = 1;
		LogMsg("Attached vars.");

		//redink1 fixes
		g_dglos.dinkspeed = 3;

		if (*pcur_weapon != 0) 
		{
			if (g_dglos.g_playerInfo.g_itemData[*pcur_weapon].active == false)
			{
				*pcur_weapon = 1;
				g_dglos.weapon_script = 0;
				LogMsg("Loadgame error: Player doesn't have armed weapon - changed to 1.");
			} else
			{

				g_dglos.weapon_script = load_script(g_dglos.g_playerInfo.g_itemData[*pcur_weapon].name, 1000, false);
				if (locate(g_dglos.weapon_script, "DISARM")) run_script(g_dglos.weapon_script);

				g_dglos.weapon_script = load_script(g_dglos.g_playerInfo.g_itemData[*pcur_weapon].name, 1000, false);
				if (locate(g_dglos.weapon_script, "ARM")) run_script(g_dglos.weapon_script);
			}
		}
	
		if (*pcur_magic != 0) 
		{
			if (g_dglos.g_playerInfo.g_itemData[*pcur_magic].active == false)
			{
				*pcur_magic = 0;
				g_dglos.magic_script = 0;
				LogMsg("Loadgame error: Player doesn't have armed magic - changed to 0.");
			} else
			{

				g_dglos.magic_script = load_script(g_dglos.g_playerInfo.g_MagicData[*pcur_magic].name, 1000, false);
				if (locate(g_dglos.magic_script, "DISARM")) run_script(g_dglos.magic_script);
				g_dglos.magic_script = load_script(g_dglos.g_playerInfo.g_MagicData[*pcur_magic].name, 1000, false);
				if (locate(g_dglos.magic_script, "ARM")) run_script(g_dglos.magic_script);
			}
		}
		kill_repeat_sounds_all();
		load_map(g_MapInfo.loc[*pmap]);
		LogMsg("Loaded map."); 
		BuildScreenBackground();
		LogMsg("Map drawn.");

		//stop any weird noises from happening on load
		g_dglos.g_guiStrength = *pstrength;
		g_dglos.g_guiMagic = *pmagic;
		g_dglos.g_guiGold = *pgold;
		g_dglos.g_guiDefense = *pdefense;


		//redink1 fixes
		g_dglos.g_guiExp = *pexper;
		draw_status_all();
		
		
		LogMsg("Status drawn.");
		last_saved_game = num;
		return(true);
	}
}

void kill_cur_item( void )
{

	if (*pcur_weapon != 0) 
	{
		if (g_dglos.g_playerInfo.g_itemData[*pcur_weapon].active == true)
		{
			if (g_dglos.weapon_script != 0) if (locate(g_dglos.weapon_script, "DISARM")) run_script(g_dglos.weapon_script);
			g_dglos.weapon_script = load_script(g_dglos.g_playerInfo.g_itemData[*pcur_weapon].name, 0, false);
			g_dglos.g_playerInfo.g_itemData[*pcur_weapon].active = false;
			*pcur_weapon = 0;
			if (g_dglos.weapon_script != 0) if (locate(g_dglos.weapon_script, "HOLDINGDROP")) run_script(g_dglos.weapon_script);

			if (g_dglos.weapon_script != 0) if (locate(g_dglos.weapon_script, "DROP")) run_script(g_dglos.weapon_script);
			g_dglos.weapon_script = 0;
		} else
		{
			LogMsg("Error:  Can't kill cur item, none armed.");
		}
	}
}



void kill_cur_item_script( char name[20])
{
	int select = 0;
	for (int i = 1; i < C_DINK_MAX_ITEMS + 1; i++)
	{
		if (g_dglos.g_playerInfo.g_itemData[i].active)
			if (compare(g_dglos.g_playerInfo.g_itemData[i].name, name))
			{
				select = i; 
				goto found;
			}
	}

	return;

found:

	if (*pcur_weapon == select)
	{
		//holding it right now  
		if (locate(g_dglos.weapon_script, "HOLDINGDROP")) run_script(g_dglos.weapon_script);                
		if (locate(g_dglos.weapon_script, "DISARM")) run_script(g_dglos.weapon_script);

		*pcur_weapon = 0;
		g_dglos.weapon_script = 0;
	}

	int script = load_script(g_dglos.g_playerInfo.g_itemData[select].name, 0, false);
	g_dglos.g_playerInfo.g_itemData[select].active = false;

	if (locate(script, "DROP")) run_script(script);

	draw_status_all();
}

void kill_cur_magic_script( char name[20])
{
	int select = 0;
	for (int i = 1; i < 9; i++)
	{
		if (g_dglos.g_playerInfo.g_MagicData[i].active)
			if (compare(g_dglos.g_playerInfo.g_MagicData[i].name, name))
			{
				select = i; 
				goto found;
			}
	}

	return;

found:

	if (*pcur_magic == select)

	{
		//holding it right now  
		if (locate(g_dglos.magic_script, "HOLDINGDROP")) run_script(g_dglos.magic_script);              
		if (locate(g_dglos.magic_script, "DISARM")) run_script(g_dglos.magic_script);


		*pcur_weapon = 0;
		g_dglos.magic_script = 0;
	}

	int script = load_script(g_dglos.g_playerInfo.g_MagicData[select].name, 0, false);
	g_dglos.g_playerInfo.g_MagicData[select].active = false;

	if (locate(script, "DROP")) run_script(script);

	draw_status_all();
}


void kill_cur_magic( void )
{

	if (*pcur_magic != 0) 
	{
		if (g_dglos.g_playerInfo.g_MagicData[*pcur_magic].active == true)
		{

			if (g_dglos.magic_script != 0) if (locate(g_dglos.magic_script, "DISARM")) run_script(g_dglos.magic_script);
			g_dglos.magic_script = load_script(g_dglos.g_playerInfo.g_MagicData[*pcur_magic].name, 0, false);
			g_dglos.g_playerInfo.g_MagicData[*pcur_magic].active = false;
			*pcur_magic = 0;

			if (g_dglos.magic_script != 0) if (locate(g_dglos.magic_script, "HOLDINGDROP")) run_script(g_dglos.magic_script);   
			if (g_dglos.magic_script != 0) if (locate(g_dglos.magic_script, "DROP")) run_script(g_dglos.magic_script);
			g_dglos.magic_script = 0;
		} else
		{
			LogMsg("Error:  Can't kill cur magic, none armed.");
		}
	}
}

void update_screen_time(void )
{
	//Msg("Cur time is %d", play.spmap[*pmap].last_time);
	//Msg("Map is %d..", *pmap);
	g_dglos.g_playerInfo.spmap[*pmap].last_time = g_dglos.g_dinkTick;
	//Msg("Time was saved as %d", play.spmap[*pmap].last_time);
}

bool load_game_small(int num, char * line, int *mytime)
{
	FILE *          fp;
	char crap[255];
	sprintf(crap, "%ssave%d.dat", g_dglo.m_savePath.c_str(),  num);


	fp = fopen(crap, "rb");
	if (!fp)
	{
		//LogMsg("Couldn't quickload save game %d", num);
		line[0] = 0;
		*mytime = 0;
		return(false);
	}
	

		fread(&short_play,sizeof(player_short_info),1,fp);
		fclose(fp);
		*mytime = short_play.minutes;               
		strcpy(line, short_play.gameinfo); 
		return(true);
	
}


void load_info()
{
	FILE *          fp;
	string fName = GetFileLocationString(g_dglos.current_dat);
	//redink1 changed 'crap' to 'current_dat'
	
	StreamingInstance *pFile = GetFileManager()->GetStreaming(GetFileLocationString(g_dglos.current_dat), NULL, false);

	
	//fp = fopen(fName.c_str(), "rb");
	if (!pFile)
	{
		LogMsg("Unable to load_info() on %s", fName.c_str());
		return;
	}
	else
	{
		LogMsg("World data loaded."); 
		
		pFile->Read((byte*)&g_MapInfo, sizeof(struct map_info));
		//fread(&g_MapInfo,sizeof(struct map_info),1,fp);
		delete pFile;
		//fclose(fp);
	}
}


bool load_hard(void)
{
	//FILE *          fp;
	string fName = GetFileLocationString("hard.dat");

	StreamingInstance *pFile = GetFileManager()->GetStreaming(fName, NULL, false);

	//fp = fopen(fName.c_str(), "rb");
	if (!pFile)
	{
		return false;
	}
	else
	{
		//fread(&g_hmap,sizeof(struct hardness),1,fp);
		pFile->Read((byte*)&g_hmap, sizeof(struct hardness));
		delete(pFile);
	}
	return true;
}

void blit_background(void)
{

	rtRect32 rcRect( 0,0,C_DINK_SCREENSIZE_X,C_DINK_SCREENSIZE_Y);
	lpDDSBack->BltFast( 0, 0, lpDDSBackGround,
		&rcRect, DDBLTFAST_NOCOLORKEY);
}

void draw_wait()
{
	LogMsg("waiting..");
}


bool LoadSpriteSingleFrame(string fNameBase, int seq, int oo, int picIndex, eTransparencyType transType, FFReader *pReader, rtRect32 hardbox, int xoffset, int yoffset, int notanim,
	bool *pLoadWasTruncated = NULL)
{

#ifdef _DEBUG
	if (seq == 12)
	{
		//LogMsg("LoadSpriteSingleFrame");
	}
#endif
	int work;
	string fName = fNameBase;
	if (oo < 10) fName += "0";
	fName += toString(oo) + ".bmp";

	int pMemSize = 0;
	byte *pMem = pReader->LoadFileIntoMemory(fName, &pMemSize, fNameBase + "01.bmp");
	
	if (g_dglos.g_seq[seq].m_spaceAllowed != 0)
	{
		if (pMem && oo > g_dglos.g_seq[seq].m_spaceAllowed)
		{
#ifdef _DEBUG
			LogMsg("Truncating anim %s to fit in existing seq %d?! forcing reload with new length", fName.c_str(), seq);
#endif
			SAFE_DELETE_ARRAY(pMem);
			if (pLoadWasTruncated)
			{
				*pLoadWasTruncated = true;
			}
		}
	}

	if (pReader->GetLastError() != FFReader::ERROR_NONE)
	{
		LogMsg("Low mem error");
		assert(!"Low mem");
		FreeSequence(seq);
		return false;
	}

	if (pMem)
	{
#ifdef _DEBUG
		//LogMsg("Loaded %s", (fName).c_str());
#endif	
	//assert(!g_dglos.g_picInfo[g_cur_sprite].pSurface);
		SAFE_DELETE(g_pSpriteSurface[picIndex]);
#ifdef _DEBUG
		if (seq == 192 && oo == 5)
		{
			LogMsg("Woah, ");
		}
		if (seq == 341 && oo == 1)
		{
			LogMsg("Woah, ");
		}

#endif	


		bool bUseCheckerboardFix = GetApp()->GetVar("checkerboard_fix")->GetUINT32() != 0;

		//hack so dialog box doesn't look bad:
		if (bUseCheckerboardFix)
		{
			
			/*
			if (oo >=2 && oo <= 4 && fNameBase =="main-")
			{
				//nah, because this is connected it makes this look weird, even with the V2 checkerboard processing
				bUseCheckerboardFix = false;
			}
			*/
			

		}

#ifdef _DEBUG
		if (seq == 868 && oo == 17)
		{
			//LogMsg("hey");

		}
#endif
		if (seq == 181 && oo == 11)
		{
			//hack for exp divider to be trans
			transType = TRANSPARENT_WHITE;
		}

		g_pSpriteSurface[picIndex] = LoadBitmapIntoSurface("", transType, IDirectDrawSurface::MODE_SHADOW_GL, pMem, pMemSize, bUseCheckerboardFix);
	}
	else
	{

		if (oo == 1)
		{
#ifdef _DEBUG
			LogMsg("load_sprites:  Anim %s not found.", (fName).c_str());
#endif
			//assert(0);
			return false;
		}
		return false;
	}

	SAFE_DELETE_ARRAY(pMem);

	if (picIndex > 0)
	{
		int surfSizeX, surfSizeY;
		GetSizeOfSurface(g_pSpriteSurface[picIndex], &surfSizeX, &surfSizeY);

		g_dglos.g_picInfo[picIndex].box.top = 0;
		g_dglos.g_picInfo[picIndex].box.left = 0;
		g_dglos.g_picInfo[picIndex].box.right = surfSizeX;
		g_dglos.g_picInfo[picIndex].box.bottom = surfSizeY;

		
	}


	if (oo == 1)
	{

		//special handling for the first one
		if (g_dglos.g_seq[seq].m_bIsAnim)
		{

			if (xoffset > 0)
			{
				//an offset was set for the whole anim specifically, we don't need to guess
				g_dglos.g_seq[seq].m_xoffset = xoffset;
			}
			else
			{
				g_dglos.g_seq[seq].m_xoffset = (g_dglos.g_picInfo[picIndex].box.right -
					(g_dglos.g_picInfo[picIndex].box.right / 2)) + (g_dglos.g_picInfo[picIndex].box.right / 6);
			}


			if (yoffset > 0)
			{
				//an offset was set for the whole anim specifically, we don't need to guess
				g_dglos.g_seq[seq].m_yoffset = yoffset;
			}
			else
			{
				g_dglos.g_seq[seq].m_yoffset = (g_dglos.g_picInfo[picIndex].box.bottom -
					(g_dglos.g_picInfo[picIndex].box.bottom / 4)) - (g_dglos.g_picInfo[picIndex].box.bottom / 30);
			}

		}
	}

	if (g_dglos.g_seq[seq].m_bIsAnim)
	{
		//it's an animation.  Generally we'd take the offset from frame 1

		if (!g_dglos.g_picInfo[picIndex].m_bCustomSettingsApplied)
		{
			g_dglos.g_picInfo[picIndex].yoffset = g_dglos.g_seq[seq].m_yoffset;
			g_dglos.g_picInfo[picIndex].xoffset = g_dglos.g_seq[seq].m_xoffset;
		}
		else
		{
			//presumably it's already been set with a SET_SPRITE_INFO or something
		}
	}
	else
	{
		//not an animation

		if (!g_dglos.g_picInfo[picIndex].m_bCustomSettingsApplied)
		{
			//nothing custom set, so let's set with default values

			if (yoffset > 0)
				g_dglos.g_picInfo[picIndex].yoffset = yoffset; else
			{
				g_dglos.g_picInfo[picIndex].yoffset = (g_dglos.g_picInfo[picIndex].box.bottom -
					(g_dglos.g_picInfo[picIndex].box.bottom / 4)) - (g_dglos.g_picInfo[picIndex].box.bottom / 30);	//for the rest
			}


			if (xoffset > 0)
				g_dglos.g_picInfo[picIndex].xoffset = xoffset; else
			{
				g_dglos.g_picInfo[picIndex].xoffset = (g_dglos.g_picInfo[picIndex].box.right -// 	
					(g_dglos.g_picInfo[picIndex].box.right / 2)) + (g_dglos.g_picInfo[picIndex].box.right / 6);
			}
		}
	}

	


	if (!g_dglos.g_picInfo[picIndex].m_bCustomSettingsApplied)
	{
		g_dglos.g_picInfo[picIndex].hardbox.Clear();
		
		//ok, setup main offsets, lets build the hard block

		if (hardbox.right > 0) 
		{     
			//forced setting      
			g_dglos.g_picInfo[picIndex].hardbox.left = hardbox.left;
			g_dglos.g_picInfo[picIndex].hardbox.right = hardbox.right;
		}
		else
		{
			//guess setting   
			work = g_dglos.g_picInfo[picIndex].box.right / 4;
			g_dglos.g_picInfo[picIndex].hardbox.left -= work;
			g_dglos.g_picInfo[picIndex].hardbox.right += work;
		}

		if (hardbox.bottom > 0) 
		{
			g_dglos.g_picInfo[picIndex].hardbox.top = hardbox.top;                  
			g_dglos.g_picInfo[picIndex].hardbox.bottom = hardbox.bottom;
		}
		else
		{
			work = g_dglos.g_picInfo[picIndex].box.bottom / 10;
			g_dglos.g_picInfo[picIndex].hardbox.top -= work;
			g_dglos.g_picInfo[picIndex].hardbox.bottom += work;
		}
	}
	
	return true;
}

bool load_sprites(char org[512], int seq, int speed, int xoffset, int yoffset, rtRect32 hardbox, eTransparencyType transType, bool leftalign, bool bScanOnly = false,
				  int frame = 0, bool *bLoadWasTruncated = NULL)
{

	char hold[5];

	ToLowerCase(org);

	string tempStr(org);
	StringReplace("\\", "/", tempStr);

	static FFReader reader;
#ifdef _DEBUG

	if (seq == 868 )
	{
		LogMsg("Yeah");
	}
#endif
	reader.Init(g_dglo.m_gameDir, g_dglo.m_dmodGamePathWithDir, GetPathFromString(tempStr), g_dglo.m_bUsingDinkPak);

	string fNameBase = GetFileNameFromString(tempStr);

	if (seq == 181 && frame == 11)
	{
		//hack for exp divider to be trans
		transType = TRANSPARENT_WHITE;
	}


#ifdef _DEBUG
	//LogMsg("Loading %s", org);
#endif

	if (frame != 0)
	{
		//special code for loading a single frame...

		LoadSpriteSingleFrame(fNameBase, seq, frame, g_dglos.g_seq[seq].s+frame, transType, &reader, hardbox, xoffset, yoffset, false,
			NULL);
		return true;
	}

	g_dglos.g_seq[seq].m_bDidFileScan = true;
	// redink1 added to fix bug where loading sequences over others wouldn't work quite right.
	int save_cur = g_dglos.g_curPicIndex;
	bool reload = false;
	
	if (g_dglos.g_seq[seq].last != 0)
	{
		FreeSequence(seq);
		//  Msg("Saving sprite %d", save_cur);
		g_dglos.g_curPicIndex = g_dglos.g_seq[seq].s+1;
#ifdef _DEBUG
	//LogMsg("Reloading: Temp g_curPicIndex is %d", g_curPicIndex);
#endif
		reload = true;
		//g_dglos.g_seq[seq].last = 0;
		//g_dglos.g_seq[seq].m_spaceAllowed = 0; //forget the limits
	} else
	{
		//LogMsg("Not reloading..");
		g_dglos.g_seq[seq].s = g_dglos.g_curPicIndex -1;
	}
	
	if (bScanOnly || g_dglos.g_seq[seq].frame[1] == 0)
	{
		if (reload)
		{
			g_dglos.g_curPicIndex = save_cur;
			return true;
		}
	
		for (int oo = 1; oo <= C_MAX_SPRITE_FRAMES+1; oo++) //the +1 is so we can detect when we go over our limit
		{
			if (oo < 10) strcpy(hold, "0"); else strcpy(hold,"");

#ifdef _DEBUG
			if (seq == 35 && oo == 22)
			{
			//	LogMsg("Loading seq %d frame %d", seq, oo);
			}
#endif
			// 2 hours of debugging found that this is key to make 'png' graphics work.
			if (oo <= C_MAX_SPRITE_FRAMES && (reader.DoesFileExist(fNameBase+string(hold)+toString(oo)+".bmp", fNameBase + "01.bmp") || 
											  reader.DoesFileExist(fNameBase+string(hold)+toString(oo)+".png", fNameBase + "01.png")))
			{
				if (!reload)
				{
					save_cur++;
				}
				g_dglos.g_curPicIndex++;
			} else
			{
				if (oo ==  1)
				{
					LogMsg("load_sprites:  Anim %s not found.",tempStr.c_str());
					//assert(0);
				}
				if (oo == C_MAX_SPRITE_FRAMES+1)
				{
					LogMsg("Warning: Sequence %d tries to use over %d frames which is the max we can load!", seq, oo);
				}
				g_dglos.g_seq[seq].m_spaceAllowed = (oo - 1);
				g_dglos.g_seq[seq].last = (oo - 1);
				
				setup_anim(seq,seq,speed);
				break;
			}
		}


		if (bScanOnly)
			return true;
	}



	for (int oo = 1; oo <= C_MAX_SPRITE_FRAMES; oo++)
	{
	
		//LogMsg("Loading seq %d, oo is %d", seq, oo);

#ifdef _DEBUG
		if (seq == 35 && oo == 1)
		{
			LogMsg("Gotcha");
		}

#endif

		if (LoadSpriteSingleFrame(fNameBase, seq, oo, g_dglos.g_curPicIndex, transType, &reader, hardbox, xoffset, yoffset, false, bLoadWasTruncated))
		{
			g_dglos.g_curPicIndex++;

			if (!reload)
				save_cur++;
		} else
		{
			if (oo > 1)
			{

				g_dglos.g_seq[seq].m_spaceAllowed = (oo - 1);
				g_dglos.g_seq[seq].last = (oo - 1);
				
				//make reloaded anims of different lengths work right, without breaking anims like idle that replay frames, confusing the length
				if (g_dglos.g_seq[seq].frame[g_dglos.g_seq[seq].last] == g_dglos.g_seq[seq].s+g_dglos.g_seq[seq].last && !g_dglos.g_seq[seq].m_bFrameSetUsed)
				{
					

					//looks like it was a standard anim without any extra frames added, truncate it here
					g_dglos.g_seq[seq].frame[g_dglos.g_seq[seq].last+1] = 0;
				}
				//setup_anim(seq,seq,speed);
				
			}
			break;
		}

	}
	g_dglos.g_curPicIndex = save_cur;
	return true;
}

eTransparencyType GetTransparencyOverrideForSequence(eTransparencyType defaultTrans, int seqID)
{
	
	//we don't support turning the color key on and off at blit time like Dink did, but this can help us get it right anyway.  Basically these are blits of numbers for the status
	//bar that we force transparency off on

	/*
	switch (seqID)
	{
	case 442: //level#
		return TRANSPARENT_WHITE;
	case 423: //item menu bg
		return TRANSPARENT_WHITE;
	case 437: //magic icons
		return TRANSPARENT_BLACK;
	case 438: //item icons

		return TRANSPARENT_BLACK;
	}
	*/

	switch (seqID)
	{
	case 181: //ns- numbers
		return TRANSPARENT_NONE;
	case 182: //nr- numbers
		return TRANSPARENT_NONE;
	case 183: //nb- numbers
		return TRANSPARENT_NONE;
	case 184: //nb- numbers
		return TRANSPARENT_NONE;
	case 185: //ny- numbers
		return TRANSPARENT_NONE;
	case 190: //health bar
		return TRANSPARENT_NONE;
	case 451: //health bar
		return TRANSPARENT_NONE;

	}

	return defaultTrans;
}

void ReadFromLoadSequenceString(char ev[15][100] )
{

	//           name   seq    speed       offsetx     offsety       hardx      hardy   
	int seqID = atol(ev[3]);


	int speed = 0;
	rtRect32 hardbox;
	//redink1 set hardbox to zero memory by default... fixed some weird compiler warnings in debug mode.  Might screw up default hard box?
	hardbox.Clear();

	g_dglos.g_seq[seqID].active = true;
	g_dglos.g_seq[seqID].m_bIsAnim = false;

#ifdef _DEBUG
	if (seqID == 192)
	{
//		LogMsg("Booya");

	}
#endif

	assert(strlen(ev[2]) < C_SPRITE_MAX_FILENAME_SIZE);
	strcpy(g_dglos.g_seq[seqID].m_fileName, ev[2]);

	g_dglos.g_seq[seqID].m_transType = TRANSPARENT_WHITE;


	if (compare(ev[4], "BLACK"))
	{
		g_dglos.g_seq[seqID].m_transType = TRANSPARENT_BLACK;
		g_dglos.g_seq[seqID].m_bIsAnim = true;
	} else if (compare(ev[4], "LEFTALIGN"))
	{
		g_dglos.g_seq[seqID].m_bLeftAlign = true;
		g_dglos.g_seq[seqID].m_transType = GetTransparencyOverrideForSequence(g_dglos.g_seq[seqID].m_transType, seqID);
	}else if (compare(ev[4], "NOTANIM") /*|| compare(ev[4], "NOTANIN")*/ ) //to work around a typo in MsDink's DMOD, but not to work around the notanin error in seq 424 in original dini.ini's.. yeah, complicated.  Why!?!?
	{
		

		g_dglos.g_seq[seqID].m_transType = GetTransparencyOverrideForSequence(TRANSPARENT_WHITE, seqID);
	}  else
	{
		
		g_dglos.g_seq[seqID].m_bIsAnim = true;
		#ifdef _DEBUG
				//LogMsg("Hardbox: %s", PrintRect(hardbox).c_str());
		#endif
	}

	if (g_dglos.g_seq[seqID].m_bIsAnim)
	{
		//yes, an animation! Set default values for entire animation if we've got them
		g_dglos.g_seq[seqID].m_speed = atol(ev[4]);
		g_dglos.g_seq[seqID].m_xoffset = atol(ev[5]);
		g_dglos.g_seq[seqID].m_yoffset = atol(ev[6]);
		g_dglos.g_seq[seqID].m_hardbox.left = atol(ev[7]);
		g_dglos.g_seq[seqID].m_hardbox.top = atol(ev[8]);
		g_dglos.g_seq[seqID].m_hardbox.right = atol(ev[9]);
		g_dglos.g_seq[seqID].m_hardbox.bottom = atol(ev[10]);
	}
}

bool ReloadSequence(int seqID, int frame, bool bScanOnly)
{

	//handle a possible case where we need to always load frame 1 before any other frame to get the correct offset for anims
	if (frame > 1 && !bScanOnly && g_dglos.g_seq[seqID].m_bIsAnim)
	{
			if (g_pSpriteSurface[g_dglos.g_seq[seqID].s+1] == 0) //make sure it's not already loaded
		ReloadSequence(seqID, 1, bScanOnly);
	}

	bool bLoadWasTruncated = false;

	bool bReturn = load_sprites(g_dglos.g_seq[seqID].m_fileName, seqID, g_dglos.g_seq[seqID].m_speed, g_dglos.g_seq[seqID].m_xoffset, g_dglos.g_seq[seqID].m_yoffset, g_dglos.g_seq[seqID].m_hardbox,
		g_dglos.g_seq[seqID].m_transType, g_dglos.g_seq[seqID].m_bLeftAlign, bScanOnly, frame, &bLoadWasTruncated);
					
	if (bLoadWasTruncated && g_dglos.g_seq[seqID].m_bIsAnim)
	{
		//ok, here's the deal, an INI was set that specified X frames, but now we're suddenly loading a different anim which has more.  The original Dink allowed this,
		//but Dink HD doesn't due to how it has to have the ability to re-load all graphic data at any point for quick saves.  So we're going to "forget" the amount
		//we reserved previously, and reallocate it
		
		
		//force frames to get recalculated completely
		g_dglos.g_seq[seqID].m_bDidFileScan = false;
		g_dglos.g_seq[seqID].last = 0;
		g_dglos.g_seq[seqID].s = -1;
		g_dglos.g_seq[seqID].m_spaceAllowed = 0;
		g_dglos.g_seq[seqID].frame[1] = 0;
		g_dglos.g_seq[seqID].m_bFrameSetUsed = false;

		return ReloadSequence(seqID, frame, bScanOnly);
	}
	if (g_dglos.g_seq[seqID].m_bFrameSetUsed)
	{
		//a set_frame_frame has been used here.  This means we may reference another sprite that isn't loaded yet, better check
		for (int i = 1; i < C_MAX_SPRITE_FRAMES; i++)
		{
			if (g_dglos.g_seq[seqID].frame[i] == 0)
			{
				//done I guess
				continue; //it's blank
			}

			if (!g_pSpriteSurface[g_dglos.g_seq[seqID].frame[i]])
			{
				//this needs to be loaded
#ifdef _DEBUG
//	if (seqID == 470 && i == 9)
				{
					//LogMsg("Need to load pic %d that was in seq %d frame %d.  The parent is seq %d", g_dglos.g_seq[seqID].frame[i], seqID, i, g_dglos.g_picInfo[g_dglos.g_seq[seqID].frame[i]].m_parentSeq);

				}
#endif

				check_seq_status(g_dglos.g_picInfo[g_dglos.g_seq[seqID].frame[i]].m_parentSeq, g_dglos.g_picInfo[g_dglos.g_seq[seqID].frame[i]].m_parentFrame);
			}
		}
	}
																								

	return bReturn;
}

bool figure_out(const char *line, int load_seq)
{
	char ev[15][100];
	rtRect32 hardbox;
	hardbox.Clear();
	bool bReturn = true;

	memset(&ev,0, sizeof(ev));
	int myseq = 0,myframe = 0; int special = 0;
	int special2 = 0;
	
	for (int i=1; i <= 14; i++)
	{
		separate_string(line, i,' ',ev[i]);
		//   Msg("Word %d is \"%s\"",i,ev[i]);
		if (!ev[i][0]) break;
	}

	if (    (compare(ev[1],"LOAD_SEQUENCE_NOW")) | ( compare(ev[1],"LOAD_SEQUENCE"))  ) 
	{
		//           name   seq    speed       offsetx     offsety       hardx      hardy   
		
		
		int seqID = atol(ev[3]);


		if (!g_dglos.g_seq[seqID].active)
		{
			ReadFromLoadSequenceString(ev);


			//first time, actually init it
			bReturn = load_sprites(g_dglos.g_seq[seqID].m_fileName,seqID,g_dglos.g_seq[seqID].m_speed,g_dglos.g_seq[seqID].m_xoffset,g_dglos.g_seq[seqID].m_yoffset, g_dglos.g_seq[seqID].m_hardbox
				, g_dglos.g_seq[seqID].m_transType, g_dglos.g_seq[seqID].m_bLeftAlign, true);

		} else
		{
			if (compare(ev[1], "LOAD_SEQUENCE"))
			{
				//detect if this was already set somewhere first in the ini, on original Dink, this is a bug but doesn't matter because it doesn't load it, where with Dink HD LOAD_SEQUENCE_NOW's are
				//loaded "on demand" so this can actually cause problems if we don't ignore the extra setting done
			
					//it's already been set, ignore this
					//it's possible we should still set the offsets though, unsure
			//		return bReturn;
				
			}

	
			ReadFromLoadSequenceString(ev);
			//ScanSeqFilesIfNeeded(seqID);

			FreeSequence(seqID); //force a full reload, the anim probably changed
			ReloadSequence(seqID);
		}
		
		
		return bReturn;     
	}
	
	return pre_figure_out(line, load_seq, false);

	//assert(!"Uh.. why are people calling other things besides load sequence with init?  Should we support that?");
	//return bReturn;
}

void program_idata(void)
{
	return;
}

bool pre_figure_out(const char *line, int load_seq, bool bLoadSpriteOnly)
{

	if (line[0] == 0 || line[0] == ';') return true;

	bool bReturn = true;

	char ev[15][100];

	memset(&ev,0, sizeof(ev));
	int myseq = 0,myframe = 0; int special = 0;
	int special2 = 0;
	for (int i=1; i <= 14; i++)
	{
		separate_string(line, i,' ',ev[i]);
		//   Msg("Word %d is \"%s\"",i,ev[i]);
		if (!ev[i][0]) break;
	}
	
	if (bLoadSpriteOnly)
	{

		if ( compare(ev[1],"LOAD_SEQUENCE_NOW") || compare(ev[1],"LOAD_SEQUENCE") ) 
			//  if (     (load_seq == -1) | (load_seq == atol(ev[3]))  )
		{
			int seqID = atol(ev[3]);

#ifdef _DEBUG
			if (seqID == 439)
			{

				LogMsg("Loading sand stuff prefigure out");
			}

#endif

			/*
			if (compare(ev[1], "LOAD_SEQUENCE") && g_dglos.g_seq[seqID].active == true)
			{
				//detect if this was already set somewhere first in the ini, on original Dink, this is a bug but doesn't matter because it doesn't load it, where with Dink HD LOAD_SEQUENCE_NOW's are
				//loaded "on demand" so this can actually cause problems if we don't ignore the extra setting done

				//it's already been set, ignore this
				//it's possible we should still set the offsets though, unsure
				return bReturn;

			}
			*/
			//ignore above, we need it for dmods


			ReadFromLoadSequenceString(ev);
			
			if (compare(ev[1], "LOAD_SEQUENCE_NOW"))
				//  if (     (load_seq == -1) | (load_seq == atol(ev[3]))  )
			{
				bReturn = load_sprites(g_dglos.g_seq[seqID].m_fileName, seqID, g_dglos.g_seq[seqID].m_speed, g_dglos.g_seq[seqID].m_xoffset, g_dglos.g_seq[seqID].m_yoffset, g_dglos.g_seq[seqID].m_hardbox
					, g_dglos.g_seq[seqID].m_transType, g_dglos.g_seq[seqID].m_bLeftAlign, true);
				return bReturn;
			}

			//ScanSeqFilesIfNeeded(seqID);

			return 1;
		}
	
		return true;
	}
	
	if (compare(ev[1],"playmidi"))
	{
		PlayMidi(ev[2]);
		return bReturn;
	}

	
	if (compare(ev[1],"SET_SPRITE_INFO"))
	{

		
	//           name   seq    speed       offsetx     offsety       hardx      hardy   
	//if (k[seq[myseq].frame[myframe]].frame = 0) Msg("Changing sprite that doesn't exist...");
	myseq = atol(ev[2]);
	myframe = atol(ev[3]);

	ScanSeqFilesIfNeeded(myseq);

	g_dglos.g_picInfo[g_dglos.g_seq[myseq].frame[myframe]].xoffset = atol(ev[4]);
	g_dglos.g_picInfo[g_dglos.g_seq[myseq].frame[myframe]].yoffset = atol(ev[5]);
	g_dglos.g_picInfo[g_dglos.g_seq[myseq].frame[myframe]].hardbox.left = atol(ev[6]);
	g_dglos.g_picInfo[g_dglos.g_seq[myseq].frame[myframe]].hardbox.top = atol(ev[7]);
	g_dglos.g_picInfo[g_dglos.g_seq[myseq].frame[myframe]].hardbox.right = atol(ev[8]);
	g_dglos.g_picInfo[g_dglos.g_seq[myseq].frame[myframe]].hardbox.bottom = atol(ev[9]);
	g_dglos.g_picInfo[g_dglos.g_seq[myseq].frame[myframe]].m_bCustomSettingsApplied = true;
	
	
	/*
	if (myframe == 1 && g_dglos.g_seq[myseq].m_bIsAnim)
	{
		//set these to be the default for the whole anim.. (replacing that idata crap from  the old source)
#ifdef _DEBUG
	//LogMsg("Setting seq %d frame 1 to default for anim", myseq);
#endif
		g_dglos.g_seq[myseq].m_xoffset = g_dglos.g_picInfo[g_dglos.g_seq[myseq].frame[myframe]].xoffset;
		g_dglos.g_seq[myseq].m_yoffset = g_dglos.g_picInfo[g_dglos.g_seq[myseq].frame[myframe]].yoffset;
		//g_dglos.g_seq[myseq].m_hardbox = g_dglos.g_picInfo[g_dglos.g_seq[myseq].frame[myframe]].hardbox;
	}
	*/
	
	
	}

	if (compare(ev[1],"SET_FRAME_SPECIAL"))
	{
	//           name   seq    speed       offsetx     offsety       hardx      hardy   
	//if (k[seq[myseq].frame[myframe]].frame = 0) Msg("Changing sprite that doesn't exist...");

	myseq = atol(ev[2]);
	myframe = atol(ev[3]);
	special = atol(ev[4]);

	g_dglos.g_seq[myseq].special[myframe] = special;
	//LogMsg("Set special.  %d %d %d",myseq, myframe, special);
	}

	if (compare(ev[1],"SET_FRAME_DELAY"))
	{
	//           name   seq    speed       offsetx     offsety       hardx      hardy   
	//if (k[seq[myseq].frame[myframe]].frame = 0) Msg("Changing sprite that doesn't exist...");



	myseq = atol(ev[2]);
	myframe = atol(ev[3]);
	special = atol(ev[4]);

	ScanSeqFilesIfNeeded(myseq);

	g_dglos.g_seq[myseq].delay[myframe] = special;
	//LogMsg("Set delay.  %d %d %d",myseq, myframe, special);
	}

	if (compare(ev[1],"STARTING_DINK_X"))
	{
	myseq = atol(ev[2]);
	g_dglos.g_playerInfo.x = myseq;
	}

	if (compare(ev[1],"STARTING_DINK_Y"))
	{
		myseq = atol(ev[2]);
		g_dglos.g_playerInfo.y = myseq;
	}

	if (compare(ev[1],"SET_FRAME_FRAME"))
	{

	myseq = atol(ev[2]);
	myframe = atol(ev[3]);
	special = atol(ev[4]);
	special2 = atol(ev[5]);


	ScanSeqFilesIfNeeded(myseq);
	if (myseq != special && special > 0)
		ScanSeqFilesIfNeeded(special);

#ifdef _DEBUG
	//LogMsg("Set frame.  %d %d %d",myseq, myframe, special);

	if (myseq == 16)
	{
		//LogMsg("Idle..");

	}

#endif

	g_dglos.g_seq[myseq].m_bFrameSetUsed = true;
	
	if (special == -1)
		g_dglos.g_seq[myseq].frame[myframe] = special; 
	else
	{
		g_dglos.g_seq[myseq].frame[myframe] = g_dglos.g_seq[special].frame[special2];

		//also copy over the details...

	}
	

	}

	return bReturn;     
}

int draw_num(int mseq, char nums[50], int mx, int my)
{
	int length = 0;
	int             ddrval;
	int rnum = 0;

	if (!check_seq_status(mseq)) return 0;

	for (int i=0; i < strlen(nums); i++)
	{

		if (nums[i] == '0') rnum = 10;
		else if (nums[i] == '1') rnum = 1;
		else if (nums[i] == '2') rnum = 2;
		else if (nums[i] == '3') rnum = 3;
		else if (nums[i] == '4') rnum = 4;
		else if (nums[i] == '5') rnum = 5;
		else if (nums[i] == '6') rnum = 6;
		else if (nums[i] == '7') rnum = 7;
		else if (nums[i] == '8') rnum = 8;
		else if (nums[i] == '9') rnum = 9;
		else if (nums[i] == '/') rnum = 11;
        
		if ( (rnum != 11) && (!(mseq == 442)) )
			ddrval = lpDDSBack->BltFast( mx+length, my,g_pSpriteSurface[g_dglos.g_seq[mseq].frame[rnum]], &g_dglos.g_picInfo[g_dglos.g_seq[mseq].frame[rnum]].box  , DDBLTFAST_NOCOLORKEY);
		else 
			ddrval = lpDDSBack->BltFast( mx+length, my, g_pSpriteSurface[g_dglos.g_seq[mseq].frame[rnum]], &g_dglos.g_picInfo[g_dglos.g_seq[mseq].frame[rnum]].box  , DDBLTFAST_SRCCOLORKEY);

			length += g_dglos.g_picInfo[g_dglos.g_seq[mseq].frame[rnum]].box.right;
		
	}
	return(length);
}



void draw_exp(bool bDraw)
{
	
	if (!bDraw) return;

	char buffer[30];
	char nums[30];
	char final[30];

	//Msg("Drawing exp.. which is %d and %d",fexp, *pexp);
	strcpy(final, "");
	strcpy(nums,rt_ltoa(g_dglos.g_guiExp, buffer, 10));
	if (strlen(nums) < 5)
		for (int i = 1; i < (6 - strlen(nums)); i++)
			strcat(final, "0");
	strcat(final, nums);
	strcat(final,"/");

	strcpy(nums,rt_ltoa(g_dglos.g_guiRaise, buffer, 10));
	if (strlen(nums) < 5)
		for (int i = 1; i < (6 - strlen(nums)); i++)
			strcat(final, "0");
	strcat(final, nums);
	
	check_seq_status(181);
	draw_num(181, final, 404, 459);
}

void draw_strength(bool bDraw)
{
	if (!bDraw) return;

	char final[30];
	char buffer[30];
	char nums[30];
	//Msg("Drawing exp.. which is %d and %d",fexp, *pexp);
	strcpy(final, "");

	strcpy(nums,rt_ltoa(g_dglos.g_guiStrength, buffer, 10));
	if (strlen(nums) < 3)
		for (int i = 1; i < (4 - strlen(nums)); i++)
			strcat(final, "0");
	strcat(final, nums);
	//Msg("Drawing %s..",final);
	draw_num(182, final, 81, 415);
}


void draw_defense(bool bDraw)
{

	if (!bDraw) return;

	char final[30];
	char buffer[30];
	char nums[30];
	//Msg("Drawing exp.. which is %d and %d",fexp, *pexp);
	strcpy(final, "");
	strcpy(nums,rt_ltoa(g_dglos.g_guiDefense, buffer, 10));
	if (strlen(nums) < 3)
		for (int i = 1; i < (4 - strlen(nums)); i++)
			strcat(final, "0");
	strcat(final, nums);
	draw_num(183, final, 81, 437);
}


void draw_magic(bool bDraw)
{

	if (!bDraw) return;
	char final[30];
	char buffer[30];
	char nums[30];
	//Msg("Drawing exp.. which is %d and %d",fexp, *pexp);
	strcpy(final, "");
	strcpy(nums,rt_ltoa(g_dglos.g_guiMagic, buffer, 10));
	if (strlen(nums) < 3)
		for (int i = 1; i < (4 - strlen(nums)); i++)
			strcat(final, "0");
	strcat(final, nums);
	draw_num(184, final, 81, 459);
}


void draw_level()
{
	char final[30];
	char buffer[30];

	//*plevel = 15;
	//Msg("Drawing level.. which is %d ",*plevel);
	strcpy(final, rt_ltoa(*plevel, buffer, 10));

	if (strlen(final) == 1)

		draw_num(442, final, 528, 456); else
		draw_num(442, final, 523, 456);
}


void draw_gold(bool bDraw)
{
	if (!bDraw) return;

	char final[30];
	char buffer[30];
	char nums[30];
	//Msg("Drawing exp.. which is %d and %d",fexp, *pexp);
	strcpy(final, "");
	strcpy(nums,rt_ltoa(g_dglos.g_guiGold, buffer, 10));
	if (strlen(nums) < 5)
		for (int i = 1; i < (6 - strlen(nums)); i++)
			strcat(final, "0");
	strcat(final, nums);
	draw_num(185, final, 298, 457);
}


void draw_bar(int life, int seqman)
{
	int ddrval;
	int cur = 0;
	int curx = 284;
	int cury = 412;
	int rnum = 3;
	int curx_start = curx;

	rtRect32 box;

	if (!check_seq_status(seqman)) return;

	while(1)
	{
		cur++;
		if (cur > life)
		{
			cur--;
			int rem = (cur) - (cur / 10) * 10;
			if (rem != 0)
			{

				box = g_dglos.g_picInfo[g_dglos.g_seq[seqman].frame[rnum]].box;
				//Msg("Drawing part bar . cur is %d", rem);
				box.right = (box.right * ((rem) * 10)/100);
				//woah, there is part of a bar remaining.  Lets do it.

				lpDDSBack->BltFast( curx, cury, g_pSpriteSurface[g_dglos.g_seq[seqman].frame[rnum]],
					&box , DDBLTFAST_NOCOLORKEY);
			
			}

			//are we done?
			return;
		}

		rnum = 2;
		if (cur < 11) rnum = 1;
		if (cur == *plifemax) rnum = 3;

		if  ( (cur / 10) * 10 == cur)
		{
			ddrval = lpDDSBack->BltFast( curx, cury, g_pSpriteSurface[g_dglos.g_seq[seqman].frame[rnum]],
				&g_dglos.g_picInfo[g_dglos.g_seq[seqman].frame[rnum]].box  , DDBLTFAST_NOCOLORKEY);

			//if (ddrval != DD_OK) dderror(ddrval);
			curx += g_dglos.g_picInfo[g_dglos.g_seq[seqman].frame[rnum]].box.right-1;
			if (cur == 110)
			{
				cury += g_dglos.g_picInfo[g_dglos.g_seq[seqman].frame[rnum]].box.bottom+5;
				curx = curx_start;

			}

			if (cur == 220) return;
		}
	}
}

void draw_health()
{
	//g_dglos.g_guiLifeMax = *plifemax;
	draw_bar(g_dglos.g_guiLifeMax, 190);
	//g_dglos.g_guiLife = *plife;
	draw_bar(g_dglos.g_guiLife, 451);
}

void draw_icons()
{
	int ddrval;
	int seq;
	int frame;

	if (*pcur_weapon != 0) if (g_dglos.g_playerInfo.g_itemData[*pcur_weapon].active)
	{
		//disarm old weapon
		//play.item[*pcur_weapon].seq,
		seq = g_dglos.g_playerInfo.g_itemData[*pcur_weapon].seq;
		frame = g_dglos.g_playerInfo.g_itemData[*pcur_weapon].frame;

		if (!check_seq_status(seq, frame)) return;
		
		/*
		DrawFilledRect(557, 413, 
			g_pSpriteSurface[g_dglos.g_seq[seq].frame[frame]]->m_pSurf->GetWidth()
			,g_pSpriteSurface[g_dglos.g_seq[seq].frame[frame]]->m_pSurf->GetHeight(), MAKE_RGBA(0,0,0,255));
			*/


		ddrval = lpDDSBack->BltFast( 557, 413, g_pSpriteSurface[g_dglos.g_seq[seq].frame[frame]],
			&g_dglos.g_picInfo[g_dglos.g_seq[seq].frame[frame]].box, DDBLTFAST_SRCCOLORKEY);
		
	}

	if (*pcur_magic != 0) if (g_dglos.g_playerInfo.g_MagicData[*pcur_magic].active)
	{
		//disarm old weapon
		//play.mitem[*pcur_magic].seq,
	
		seq = g_dglos.g_playerInfo.g_MagicData[*pcur_magic].seq;
		frame = g_dglos.g_playerInfo.g_MagicData[*pcur_magic].frame;

#ifdef _DEBUG
		if (seq == 437)
		{
		//	LogMsg("Hey");
		}
#endif
		if (!check_seq_status(seq, frame)) return;

		if (!g_pSpriteSurface[g_dglos.g_seq[seq].frame[frame]])
		{
			LogMsg("Bad magic item frame of %d", frame);
			return;
		}
		DrawFilledRect(153, 413, 
			g_pSpriteSurface[g_dglos.g_seq[seq].frame[frame]]->m_pSurf->GetWidth()
			,g_pSpriteSurface[g_dglos.g_seq[seq].frame[frame]]->m_pSurf->GetHeight(), MAKE_RGBA(0,0,0,255));

		ddrval = lpDDSBack->BltFast( 153, 413, g_pSpriteSurface[g_dglos.g_seq[seq].frame[frame]],
			&g_dglos.g_picInfo[g_dglos.g_seq[seq].frame[frame]].box, DDBLTFAST_SRCCOLORKEY);
	}
}


void draw_vertical(int percent, int mx, int my, int mseq, int mframe)
{
	int ddrval;  
	int cut;
	if (percent > 25) percent = 25;
	percent = (percent * 4);
	rtRect32 myrect;
	myrect =  g_dglos.g_picInfo[g_dglos.g_seq[mseq].frame[mframe]].box;
	int full = myrect.bottom;
	cut = (full * percent) / 100;

	myrect.bottom = cut;
	my += (full - cut);

	ddrval = lpDDSBack->BltFast( mx, my, g_pSpriteSurface[g_dglos.g_seq[mseq].frame[mframe]],
		&myrect, DDBLTFAST_NOCOLORKEY);
}

void draw_virt2(int percent, int mx, int my, int mseq, int mframe)
{
	int ddrval;   
	int cut;
	if (percent > 25) percent = 25;
	percent = (percent * 4);
	rtRect32 myrect;
	myrect =  g_dglos.g_picInfo[g_dglos.g_seq[mseq].frame[mframe]].box;
	int full = myrect.bottom;
	cut = (full * percent) / 100;
	myrect.bottom = cut;

	ddrval = lpDDSBack->BltFast( mx, my, g_pSpriteSurface[g_dglos.g_seq[mseq].frame[mframe]],
		&myrect, DDBLTFAST_NOCOLORKEY);
}

void draw_hor(int percent, int mx, int my, int mseq, int mframe)
{
	int ddrval;   
	int cut;
	if (percent > 25) percent = 25;
	percent = (percent * 4);
	rtRect32 myrect;
	myrect  =  g_dglos.g_picInfo[g_dglos.g_seq[mseq].frame[mframe]].box;
	int full = myrect.right;
	cut = (full * percent) / 100;
	full = cut;
	myrect.right = full;

	ddrval = lpDDSBack->BltFast( mx, my, g_pSpriteSurface[g_dglos.g_seq[mseq].frame[mframe]],
		&myrect, DDBLTFAST_NOCOLORKEY);
}

void draw_hor2(int percent, int mx, int my, int mseq, int mframe)
{
	int ddrval; 
	int cut;
	if (percent > 25) percent = 25;
	percent = (percent * 4);
	rtRect32 myrect;
	myrect = g_dglos.g_picInfo[g_dglos.g_seq[mseq].frame[mframe]].box;
	int full = myrect.right;
	cut = (full * percent) / 100;
	myrect.right = cut;
	mx += (full - cut);
	ddrval = lpDDSBack->BltFast( mx, my, g_pSpriteSurface[g_dglos.g_seq[mseq].frame[mframe]],
		&myrect, DDBLTFAST_NOCOLORKEY);
}

void draw_mlevel(int percent, bool bDraw)
{
	if (!bDraw) return;

	int mseq = 180;
	int bary = 6;
	int barx = 7;

	if (percent > 0) draw_vertical(percent, 149, 411, mseq, bary);
	percent -= 25;
	if (percent > 0) draw_hor(percent, 149, 409, mseq, barx);
	percent -= 25;
	if (percent > 0) draw_virt2(percent, 215, 411, mseq, bary);
	percent -= 25;
	if (percent > 0) draw_hor2(percent, 149, 466, mseq, barx);
}


void draw_status_all(void)
{
	g_forceBuildBackgroundFromScratch = true;
	ClearBitmapCopy();

	
	g_dglos.g_guiStrength = *pstrength;
	g_dglos.g_guiMagic = *pmagic;
	g_dglos.g_guiGold = *pgold;
	g_dglos.g_guiDefense = *pdefense;

	return;
	
}


bool SwitchToRGBAIfNeeded(LPDIRECTDRAWSURFACE *pDXSurf, SoftSurface *pSoftSurf)
{
	if ( (*pDXSurf) || (*pDXSurf)->m_pSurf) return false;

	 	if ( (*pDXSurf)->m_pSurf->GetSurfaceType() == SoftSurface::SURFACE_PALETTE_8BIT
		&& 
			(pSoftSurf->GetSurfaceType() == SoftSurface::SURFACE_RGBA
		|| pSoftSurf->GetSurfaceType() == SoftSurface::SURFACE_RGB)
				)
	{
		delete *pDXSurf;
		*pDXSurf = InitOffscreenSurface(C_DINK_SCREENSIZE_X, C_DINK_SCREENSIZE_Y, IDirectDrawSurface::MODE_SHADOW_GL, true);
		return true;
	}
	
	return false;
}

void BlitGUIOverlay()
{
	
	//if (GetDinkGameMode() == DINK_GAME_MODE_MOUSE) return;

	if (GetDinkSubGameMode() == DINK_SUB_GAME_MODE_SHOWING_BMP) return;

	
	if (g_dglos.status_might_not_require_update == 1) return;

	rtRect32 rcRect;
	rcRect.left = 0;
	rcRect.top = 0;
	rcRect.right = C_DINK_SCREENSIZE_X;
	rcRect.bottom = 80;

	if (*pupdate_status == 0)
	{
		//draw black bars around things
		DrawFilledRect(0, 400, rcRect.right, rcRect.bottom, MAKE_RGBA(0,0,0,255));

		rcRect.right = 20;
		rcRect.bottom = 400;
		DrawFilledRect(0, 0, rcRect.right, rcRect.bottom, MAKE_RGBA(0,0,0,255));
		DrawFilledRect(620, 0, rcRect.right, rcRect.bottom, MAKE_RGBA(0,0,0,255));
		
		return;
	}


	/*

	g_dglos.g_guiRaise = next_raise();
	if ( *pexper < g_dglos.g_guiRaise )
	{
		g_dglos.g_guiExp = *pexper;
	}
	else
	{
		g_dglos.g_guiExp = g_dglos.g_guiRaise - 1;
	}

	*/

	/*
	g_dglos.g_guiStrength = *pstrength;
	g_dglos.g_guiMagic = *pmagic;
	g_dglos.g_guiGold = *pgold;
	g_dglos.g_guiDefense = *pdefense;
	*/
	g_dglos.g_guiLastMagicDraw = 0;    
	
	if (g_dglo.m_curView == DinkGlobals::VIEW_ZOOMED || g_dglo.m_viewOverride == DinkGlobals::VIEW_ZOOMED) return;
	
	if (!check_seq_status(180)) return;

	/*
	if (lpDDSBack && lpDDSBack->m_pSurf && lpDDSBack->m_pSurf->GetSurfaceType() == SoftSurface::SURFACE_PALETTE_8BIT
		&& g_pSpriteSurface[g_dglos.g_seq[180].frame[3]]->m_pSurf->GetSurfaceType() == SoftSurface::SURFACE_RGBA
		|| g_pSpriteSurface[g_dglos.g_seq[180].frame[3]]->m_pSurf->GetSurfaceType() == SoftSurface::SURFACE_RGBA)
	{
		LogMsg("Freak out!");
		//delete lpDDSBuffer;
		//lpDDSBuffer = InitOffscreenSurface(C_DINK_SCREENSIZE_X, C_DINK_SCREENSIZE_Y, IDirectDrawSurface::MODE_SHADOW_GL, true);

	}

	SwitchToRGBAIfNeeded(&lpDDSBack, g_pSpriteSurface[g_dglos.g_seq[180].frame[1]]->m_pSurf);
	SwitchToRGBAIfNeeded(&lpDDSBack, g_pSpriteSurface[g_dglos.g_seq[180].frame[2]]->m_pSurf);
	SwitchToRGBAIfNeeded(&lpDDSBack, g_pSpriteSurface[g_dglos.g_seq[180].frame[3]]->m_pSurf);
	*/
		
	lpDDSBack->BltFast( 0, 400, g_pSpriteSurface[g_dglos.g_seq[180].frame[3]], &rcRect  , DDBLTFAST_NOCOLORKEY );
	
	
	
	rcRect.left = 0;
	rcRect.top = 0;
	rcRect.right = 20;
	rcRect.bottom = 400;

	lpDDSBack->BltFast( 0, 0, g_pSpriteSurface[g_dglos.g_seq[180].frame[1]], &rcRect  , DDBLTFAST_NOCOLORKEY  );
	lpDDSBack->BltFast( 620, 0,g_pSpriteSurface[g_dglos.g_seq[180].frame[2]], &rcRect  , DDBLTFAST_NOCOLORKEY  );

	
	draw_exp(true);
	draw_health();
	draw_strength(true);
	draw_defense(true);
	draw_magic(true);
	draw_gold(true);
	draw_level();
	draw_icons();

	if (*pmagic_cost > 0 && *pmagic_level > 0)
	{
		draw_mlevel( (float(*pmagic_level) / float(*pmagic_cost))*100, true );
	}

	


	
}

bool inside_box(int x1, int y1, rtRect32 box)
{
	if (x1 > box.right) return(false);
	if (x1 < box.left) return(false);
	if (y1 > box.bottom) return(false);
	if (y1 < box.top) return(false);
	return(true);
}

int add_sprite_dumb(int x1, int y, int brain,int pseq, int pframe,int size )
{

#ifdef _DEBUG
	 
	if (pseq == 180)
	{
		LogMsg("wtf!");
	}
#endif
	for (int x=1; x < C_MAX_SPRITES_AT_ONCE; x++)
	{
		if (g_sprite[x].active == false)
		{
			SAFE_DELETE(g_customSpriteMap[x]);
			memset(&g_sprite[x], 0, sizeof(g_sprite[x]));

			//Msg("Making sprite %d.",x);
			g_sprite[x].active = true;
			g_sprite[x].x = x1;
			g_sprite[x].y = y;
			g_sprite[x].my = 0;
			g_sprite[x].mx = 0;
			g_sprite[x].speed = 0;
			g_sprite[x].brain = brain;
			g_sprite[x].frame = 0;
			g_sprite[x].pseq = pseq;
			g_sprite[x].pframe = pframe;
			g_sprite[x].size = size;
			g_sprite[x].seq = 0;
			if (x > g_dglos.last_sprite_created)
				g_dglos.last_sprite_created = x;

			g_sprite[x].timer = 0;
			g_sprite[x].wait = 0;
			g_sprite[x].lpx[0] = 0;
			g_sprite[x].lpy[0] = 0;
			g_sprite[x].moveman = 0;
			g_sprite[x].seq_orig = 0;

			g_sprite[x].base_hit = -1;
			g_sprite[x].base_walk = -1;
			g_sprite[x].base_die = -1;
			g_sprite[x].base_idle = -1;
			g_sprite[x].base_attack = -1;
			g_sprite[x].last_sound = 0;
			g_sprite[x].hard = 1;

			g_sprite[x].alt.Clear();
			g_sprite[x].althard = 0;
			g_sprite[x].sp_index = 0;
			g_sprite[x].nocontrol = 0;
			g_sprite[x].idle = 0;
			g_sprite[x].strength = 0;
			g_sprite[x].damage = 0;
			g_sprite[x].defense = 0;
			

			if ( g_customSpriteMap[x] == NULL )
			{
				g_customSpriteMap[x] = new std::map<std::string, int32>;
			}
			else
			{
				g_customSpriteMap[x]->clear();
			}
			return(x);
		}
	}

	return(0);
}

bool get_box (int spriteID, rtRect32 * pDstRect, rtRect32 * pSrcRect )
{
	rtRect32 math;
	int32 sz,sy,x_offset,y_offset;

	int32 mplayx = g_gameAreaRightBarStartX;
	int32 mplayl = g_gameAreaLeftOffset;
	int32 mplayy = C_DINK_ORIGINAL_GAME_AREA_Y;

	if (g_sprite[spriteID].noclip)
	{
		mplayx = C_DINK_SCREENSIZE_X;
		mplayl = 0;
		mplayy = C_DINK_SCREENSIZE_Y;
	}

	rtRect32 krect;

	if (getpic(spriteID) < 1)
	{
#ifdef _DEBUG
		LogMsg("Yo, sprite %d has a bad pic. (Map %d) Seq %d, Frame %d",spriteID,*pmap, g_sprite[spriteID].pseq, g_sprite[spriteID].pframe);
#endif
		//redink1 added to fix frame-not-in-memory immediately
		if (!check_seq_status(g_sprite[spriteID].pseq, g_sprite[spriteID].pframe)) return false;
	}

#ifdef _DEBUG
	if (g_sprite[spriteID].pseq == 204 )
	{
		//LogMsg("Yo");
	}
#endif

#ifdef _DEBUG
	if (g_sprite[spriteID].pseq == 202)
	{
		//LogMsg("Original");
	}
#endif

	int picID = getpic(spriteID);
	if (g_sprite[spriteID].size == 0) g_sprite[spriteID].size = 100;

	int scale = g_sprite[spriteID].size;

	int txoffset;
	int tyoffset;
	
	int originalSurfPic = g_dglos.g_seq[g_sprite[spriteID].pseq].originalFrame[g_sprite[spriteID].pframe];

	
	if (originalSurfPic != g_dglos.g_seq[g_sprite[spriteID].pseq].frame[g_sprite[spriteID].pframe]
		&& originalSurfPic != 0)
	{
		//wait.. this isn't the original picture, a set_frame_frame has been used!  We want the offset from the original.
		
		/*
		//is the parent seq of the original an anim?
		if (g_dglos.g_seq[g_dglos.g_picInfo[originalSurfPic].m_parentSeq].m_bIsAnim )
		{
		
			//first, does the original pic have stuff set for it?
			txoffset = g_dglos.g_picInfo[originalSurfPic].xoffset;
			tyoffset = g_dglos.g_picInfo[originalSurfPic].yoffset;
					
			if (txoffset == 0 && tyoffset == 0)
			{
				//No?  Well, how about the whole anim in general
				txoffset = g_dglos.g_seq[g_dglos.g_picInfo[originalSurfPic].m_parentSeq].m_xoffset;
				tyoffset = g_dglos.g_seq[g_dglos.g_picInfo[originalSurfPic].m_parentSeq].m_yoffset;

			}
		}
		else
		{
			txoffset = g_dglos.g_picInfo[originalSurfPic].xoffset;
			tyoffset = g_dglos.g_picInfo[originalSurfPic].yoffset;

		}
		*/
//		picID = originalSurfPic;
	//	txoffset = g_dglos.g_picInfo[picID].xoffset;
		//tyoffset = g_dglos.g_picInfo[picID].yoffset;


		//this is.. probably not a good solution.  Some dmods want the original offset, others want to keep the offset of the frame they are replacing.  If speed == 0, it's probably not a real anim and we'll
		//be happy with the replacement pics offset.  I think.

		//this works for the dmod mayhem
		if (g_dglos.g_seq[g_dglos.g_picInfo[originalSurfPic].m_parentSeq].m_speed == 0)
		{
			txoffset = g_dglos.g_picInfo[originalSurfPic].xoffset;
			tyoffset = g_dglos.g_picInfo[originalSurfPic].yoffset;
		}
		else
		{
			//but this works with cast awakening 5, the sycth guy swings that have frame sets in them
			txoffset = g_dglos.g_picInfo[picID].xoffset;
			tyoffset = g_dglos.g_picInfo[picID].yoffset;
		}

	

	}
	else
	{
		//normal way
		txoffset = g_dglos.g_picInfo[picID].xoffset;
		tyoffset = g_dglos.g_picInfo[picID].yoffset;
	}



	*pSrcRect = g_dglos.g_picInfo[picID].box;
	krect =  g_dglos.g_picInfo[picID].box;

	if (scale != 100)
	{
		sz = ((krect.right * scale) / 100); 
		sy = ((krect.bottom * scale) / 100); 
	} else
	{
		sz = 0;
		sy = 0;
	}

	if (scale != 100)
	{
		sz = ((sz - krect.right) / 2);
		sy = ((sy - krect.bottom) / 2);

	}

	pDstRect->left = g_sprite[spriteID].x-txoffset-sz;
	math.left = g_sprite[spriteID].x-txoffset;

	pDstRect->top = g_sprite[spriteID].y - tyoffset-sy;
	math.top = g_sprite[spriteID].y-tyoffset;

	pDstRect->right = (math.left+ (krect.right -krect.left)) + sz;
	math.right = math.left+ krect.right;

	pDstRect->bottom = (math.top + (krect.bottom - krect.top)) + sy;
	math.bottom = math.top + krect.bottom;

	if ( (g_sprite[spriteID].alt.right != 0) | (g_sprite[spriteID].alt.left != 0) | (g_sprite[spriteID].alt.top != 0) | (g_sprite[spriteID].alt.right != 0))
	{
		//redink1 checks for correct box stuff
		if (g_sprite[spriteID].alt.left < 0)
			g_sprite[spriteID].alt.left = 0;
		if (g_sprite[spriteID].alt.left > g_dglos.g_picInfo[picID].box.right)
			g_sprite[spriteID].alt.left =  g_dglos.g_picInfo[picID].box.right;
		if (g_sprite[spriteID].alt.top < 0)
			g_sprite[spriteID].alt.top = 0;
		if (g_sprite[spriteID].alt.top > g_dglos.g_picInfo[picID].box.bottom)
			g_sprite[spriteID].alt.top =  g_dglos.g_picInfo[picID].box.bottom;
		if (g_sprite[spriteID].alt.right < 0)
			g_sprite[spriteID].alt.right = 0;
		if (g_sprite[spriteID].alt.right > g_dglos.g_picInfo[picID].box.right)
			g_sprite[spriteID].alt.right =  g_dglos.g_picInfo[picID].box.right;
		if (g_sprite[spriteID].alt.bottom < 0)
			g_sprite[spriteID].alt.bottom = 0;
		if (g_sprite[spriteID].alt.bottom > g_dglos.g_picInfo[picID].box.bottom)
			g_sprite[spriteID].alt.bottom =  g_dglos.g_picInfo[picID].box.bottom;
		//spr[h].alt.bottom = 10;   
		pDstRect->left = pDstRect->left +  g_sprite[spriteID].alt.left;
		pDstRect->top = pDstRect->top + g_sprite[spriteID].alt.top;
		pDstRect->right = pDstRect->right -  (g_dglos.g_picInfo[picID].box.right - g_sprite[spriteID].alt.right);
		pDstRect->bottom = pDstRect->bottom - (g_dglos.g_picInfo[picID].box.bottom - g_sprite[spriteID].alt.bottom);
		*pSrcRect = g_sprite[spriteID].alt;
		//Msg("I should be changing box size... %d %d,%d,%d",spr[h].alt.right,spr[h].alt.left,spr[h].alt.top,spr[h].alt.bottom);
	} 

	//********* Check to see if they need to be cut down and do clipping
	if (g_sprite[spriteID].size == 0) g_sprite[spriteID].size = 100;

	
	if (pDstRect->left < mplayl)
	{
		x_offset = pDstRect->left * (-1) + mplayl;
		pDstRect->left = mplayl;
		//box_real->left += (math.left * (-1)) + mplayl;

		if (g_sprite[spriteID].size != 100)
			pSrcRect->left += (((x_offset * 100) / (g_sprite[spriteID].size )) ); else

			pSrcRect->left += x_offset;
		if (pDstRect->right-1 < mplayl) goto nodraw;
	}

	if (pDstRect->top < 0)
	{
		y_offset = pDstRect->top * (-1);
		pDstRect->top = 0;

		//box_real->top += math.top * (-1) + 0;
		if (g_sprite[spriteID].size != 100)
			pSrcRect->top += (((y_offset * 100) / (g_sprite[spriteID].size ))  );

		else pSrcRect->top += y_offset;
		if (pDstRect->bottom-1 < 0) goto nodraw;
	}

	if (pDstRect->right > mplayx)
	{
		x_offset = (pDstRect->right) - mplayx;
		pDstRect->right = mplayx;
		//x_real->right -= math.right - mplayx;
		if (g_sprite[spriteID].size != 100)
			pSrcRect->right -= ((x_offset * 100) / (g_sprite[spriteID].size ));
		else pSrcRect->right -= x_offset;
		if (pDstRect->left+1 > mplayx) goto nodraw;

		//  Msg("ok, crap right is %d, real right is %d.",box_crap->right - box_crap->left,box_real->right);
	}

	if (pDstRect->bottom > mplayy)
	{
		y_offset = (pDstRect->bottom) - mplayy;
		pDstRect->bottom = mplayy;
		if (g_sprite[spriteID].size != 100)
			pSrcRect->bottom -= ((y_offset * 100) / (g_sprite[spriteID].size ));
		else pSrcRect->bottom -= y_offset;
		if (pDstRect->top+1 > mplayy) goto nodraw;
	}

	return(true);

nodraw:         

	return(false);  
}


void kill_callbacks_owned_by_script(int script)
{
	for (int i = 1; i < C_MAX_SCRIPT_CALLBACKS; i++)
	{
		if (g_dglos.g_scriptCallback[i].owner == script)
		{ 
			if (g_script_debug_mode) LogMsg("Kill_all_callbacks just killed %d for script %d", i, script);
			//killed callback
			g_dglos.g_scriptCallback[i].active = false;
		}
	}
}

void kill_script(int k)
{
	if (g_scriptInstance[k] != NULL)
	{
		kill_callbacks_owned_by_script(k);

		//now lets kill all local vars associated with this script

		for (int i = 1; i < max_vars; i++)
		{
			if (g_dglos.g_playerInfo.var[i].active) if (g_dglos.g_playerInfo.var[i].scope == k)
			{
				g_dglos.g_playerInfo.var[i].active = false;
			}
		}

		if (g_script_debug_mode) LogMsg("Killed script %s. (num %d)", g_scriptInstance[k]->name, k);

		//tell associated sprite that it no longer has a script
		assert(C_MAX_SPRITES_AT_ONCE < 1000 && "I think 1000 has a special meaning so you better not do that?  Or was that -1000.  I dunno");
/*
	//NOTE:  This would fix a bug where a script doesn't properly remove itself from an owner sprite but.. could cause behavior differences so not enabling it for now
		if (g_scriptInstance[k]->sprite > 0 && g_scriptInstance[k]->sprite < C_MAX_SPRITES_AT_ONCE)
		{
			if (g_sprite[g_scriptInstance[k]->sprite].active && g_sprite[g_scriptInstance[k]->sprite].script == k)
			{
				//um.. guess we should tell the sprite it no longer has an associated script, right?!
				g_sprite[g_scriptInstance[k]->sprite].script = 0;
			}

		}
		*/
		SAFE_FREE(g_scriptInstance[k]);
		SAFE_FREE(g_scriptBuffer[k]);
		
		g_scriptAccelerator[k].Kill();
	}
}

void kill_all_scripts(void)
{
	for (int k = 1; k < C_MAX_SCRIPTS; k++)
	{
		if (g_scriptInstance[k] != NULL) if (g_scriptInstance[k]->sprite != 1000)
			kill_script(k);
	}

	for (int k = 1; k < C_MAX_SCRIPT_CALLBACKS; k++)
	{
		if (g_dglos.g_scriptCallback[k].active)
		{
			if ( (g_scriptInstance[g_dglos.g_scriptCallback[k].owner] != NULL) && (g_scriptInstance[g_dglos.g_scriptCallback[k].owner]->sprite == 1000) )
			{
			} else
			{
				if (g_script_debug_mode) LogMsg("Killed callback %d.  (was attached to script %d.)",k, g_dglos.g_scriptCallback[k].owner);      
				g_dglos.g_scriptCallback[k].active = 0;
			}
		}
	}
}

void kill_all_scripts_for_real(void)
{
	for (int k = 1; k < C_MAX_SCRIPTS; k++)
	{
		if (g_scriptInstance[k] != NULL) 
			kill_script(k);
	}

	for (int k = 1; k < C_MAX_SCRIPT_CALLBACKS; k++)
	{
		g_dglos.g_scriptCallback[k].active = 0;
	}
}

bool ScriptEOF(int script)
{
	if (!g_scriptInstance[script]) return true;

	return (g_scriptInstance[script]->current >= g_scriptInstance[script]->end);
}

bool read_next_line(int script, char *line)
{
	if (  (g_scriptInstance[script] == NULL) || (g_scriptBuffer == NULL) )
	{

		//this happens a lot in the revolution mod, enough that I'm commenting out in release mode because it can slow
		//down the game
#ifdef _DEBUG
		LogMsg("  ERROR:  Tried to read script %d, it doesn't exist.", script);
#endif
		return(false);
	}

	if (g_scriptInstance[script]->current >= g_scriptInstance[script]->end)
	{
		//at end of buffer
		return(false);
	}

	strcpy(line, "");

	for (int k = g_scriptInstance[script]->current;  (k < g_scriptInstance[script]->end); k++)
	{
		//      Msg("..%d",k);
		strchar(line, g_scriptBuffer[script][k]);
		

		g_scriptInstance[script]->current++;

		if (  (g_scriptBuffer[script][k] == '\n') || (g_scriptBuffer[script][k] == '\r')  )
		{
			return(true);
		}

		if (g_scriptInstance[script]->current >= g_scriptInstance[script]->end) return(false);
	}

	return(false);
}


int load_script(const char *pScript, int sprite, bool set_sprite, bool bQuietError)
{

	string fName = "story/"+ToLowerCaseString(pScript);

    StringReplace("\\", "/", fName);
	int script;
	FILE *stream;  
	bool comp = false;

	bool bFound = false;

	string fileName;

	if (!g_dglo.m_dmodGameDir.empty())
	{
		//first check the dmod path
		fileName = g_dglo.m_dmodGamePathWithDir+fName+".d";

		if (FileExists(fileName))
		{
			bFound = true;
			comp = true;
		} else
		{	
			fileName = g_dglo.m_dmodGamePathWithDir+fName+".c";
			if (FileExists(fileName))
			{
				bFound = true;
			}
		}
	}

	if (!bFound)
	{
		
		if (g_dglo.m_bUsingDinkPak)
		{
			fileName = g_dglo.m_gameDir+fName+".d";
		} else
		{
			fileName = g_dglo.m_gamePathWithDir+fName+".d";
		}

	
		if (FileExists(fileName))
		{
			comp = true;
		} else
		{	
			if (g_dglo.m_bUsingDinkPak)
			{
				fileName = g_dglo.m_gameDir+fName+".c";
			} else
			{
				fileName = g_dglo.m_gamePathWithDir+fName+".c";
			}	
			
			if (!FileExists(fileName))
			{
				if (bQuietError) return 0;
				LogMsg("Script %s not found. (checked for .C and .D) (requested by %d?)",fileName.c_str(), sprite);
				return 0;
			}
		}
	}


	int k;

	for (k=1; k < C_MAX_SCRIPTS; k++)
	{
		if (g_scriptBuffer[k] == NULL)
		{
			//found one not being used
			goto found;
		}
	}

	LogMsg("Couldn't find unused buffer for script.");
	return(0);

	
found:
if (g_script_debug_mode)
	LogMsg("Loading script %s..", fileName.c_str());
	script = k;
	g_scriptAccelerator[script].Kill();
	g_scriptInstance[script] = (struct refinfo *) malloc( sizeof(struct refinfo));
	memset(g_scriptInstance[script], 0, sizeof(struct refinfo));
	
	//load compiled script
		
	FileInstance scriptFile(fileName, false);
	if (!scriptFile.IsLoaded()) 
	{
		LogMsg("Script %s not found. (checked for .C and .D) (requested by %d?)",fileName.c_str(), sprite);
		return(0);
	}

		char * pMemBuffer = new char[1024*128];
		pMemBuffer[0] = 0;

		if (!pMemBuffer)
		{
			LogMsg("Mem error");
			return false;
		}

	//	LogMsg("Checking compression...");
		if (comp)
		{
			dink_decompress(scriptFile.GetAsBytes(), pMemBuffer);
		}
		else
		{
			
			decompress_nocomp(scriptFile.GetAsBytes(), pMemBuffer);
		}

		//LogMsg("file in cbuf");

		g_scriptInstance[script]->end = (strlen(pMemBuffer) );
		//LogMsg("length of %s is %d!", fileName.c_str(), g_scriptInstance[script]->end);                

		g_scriptBuffer[script] = (char *) malloc( g_scriptInstance[script]->end+1 );

		if (g_scriptBuffer[script] == NULL)
		{
			LogMsg("Couldn't allocate rbuff %d.",script);
			SAFE_DELETE_ARRAY(pMemBuffer);
			return(0);
		}
	//	LogMsg("Copying script");

		memcpy(g_scriptBuffer[script], pMemBuffer, g_scriptInstance[script]->end);
		g_scriptBuffer[script][g_scriptInstance[script]->end] = 0; //add a 
		SAFE_DELETE_ARRAY(pMemBuffer);
	

	//LogMsg("Script loaded by sprite %d into space %d.", sprite,script);
	strcpy(g_scriptInstance[script]->name, pScript);
	g_scriptInstance[script]->sprite = sprite;

	if (set_sprite)
	{
		if (sprite != 0) if (sprite != 1000)
			g_sprite[sprite].script = script;
	}
	
	//LogMsg("Returning script");

	return (script);
}


void strip_beginning_spaces(char *pInput)
{
	char * h;
	h = pInput;
	
	if (pInput[0] != 32) 
	{
		return;
	}

	while(h[0] == 32)
	{
		h = &h[1];
	}
	strcpy(pInput, h);
}



bool locate(int script, char proc[20])
{

	if (g_scriptInstance[script] == NULL) 
	{
		return(false);
	}


	int saveme = g_scriptInstance[script]->current;
	g_scriptInstance[script]->current = 0;
	char line[200];
	char ev[3][100];
	char temp[100];

	//Msg("locate is looking for %s in %s", proc, rinfo[script]->name);

	while(read_next_line(script, line))
	{
		strip_beginning_spaces(line);
		memset(&ev, 0, sizeof(ev));

		get_word(line, 1, ev[1]);
		if (compare(ev[1], (char*)"VOID")) 
		{
			get_word(line, 2, ev[2]);

			separate_string(ev[2], 1,'(',temp);

			//      Msg("Found procedure %s.",temp);
			if (compare(temp,proc))
			{ 
				//              Msg("Located %s",proc);
				//clean up vars so it is ready to run
				if (g_scriptInstance[script]->sprite != 1000)
				{
					g_sprite[g_scriptInstance[script]->sprite].move_active = false;
					g_sprite[g_scriptInstance[script]->sprite].move_nohard = false;

				}
				g_scriptInstance[script]->skipnext = false;
				g_scriptInstance[script]->onlevel = 0;
				g_scriptInstance[script]->level = 0;

				return(true);
				//this is desired proc

			}
		}

	}

	//Msg("Locate ended on %d.", saveme);
	g_scriptInstance[script]->current = saveme;
	return(false);

}

bool locate_goto(char proc[50], int script)
{
	g_scriptInstance[script]->current = 0;
	
	int procLen = strlen(proc);
	
	if (proc[procLen - 1] == ';')
	{
		proc[procLen - 1] = ':';
	}
	else
	{
	//just add it
		proc[procLen] = ':';
		proc[procLen + 1] = 0;
	}
	
	ScriptPosition *pScriptPos = g_scriptAccelerator[script].GetPositionByName(proc);

	if (pScriptPos)
	{
		//LogMsg("Did fast lookup of %s", proc);
		//found it, nice shortcut 
		g_scriptInstance[script]->current = pScriptPos->current;
		g_scriptInstance[script]->skipnext = false;
		g_scriptInstance[script]->onlevel = 0;
		g_scriptInstance[script]->level = 0;
		return true;

	}
	char line[512];


	//  Msg("locate is looking for %s", proc);

	while(read_next_line(script, line))
	{
		strip_beginning_spaces(line);

		if (strnicmp(line, proc, procLen) == 0)
		{
			//if (debug_mode) LogMsg("Found goto : Line is %s, word is %s.", line, ev[1]);

			g_scriptInstance[script]->skipnext = false;
			g_scriptInstance[script]->onlevel = 0;
			g_scriptInstance[script]->level = 0;
			
			//add it for faster look-up nexttime
			g_scriptAccelerator[script].AddPosition(proc, g_scriptInstance[script]->current);

			return(true);
		}

	}
	LogMsg("ERROR:  Cannot goto %s in %s.", proc, g_scriptInstance[script]->name);
	return(false);

}

void decipher(char *crap, int script)
{

	if (compare(crap, (char*)"&current_sprite")) 
	{
		sprintf(crap, "%d",g_scriptInstance[script]->sprite);

		//LogMsg("cur sprite returning %s, ",crap);
		return;
	}

	if (compare(crap, (char*)"&current_script")) 
	{
		sprintf(crap, "%d",script);
		return;
	}

	//v1.08 special variables.
	if (compare(crap, (char*)"&return")) 
	{
		sprintf(crap, "%d", g_dglos.g_returnint);
		return;
	}

	if (compare(crap, (char*)"&arg1")) 
	{
		sprintf(crap, "%d", g_scriptInstance[script]->arg1);
		return;
	}

	if (compare(crap, (char*)"&arg2")) 
	{
		sprintf(crap, "%d", g_scriptInstance[script]->arg2);
		return;
	}

	if (compare(crap, (char*)"&arg3")) 
	{
		sprintf(crap, "%d", g_scriptInstance[script]->arg3);
		return;
	}

	if (compare(crap, (char*)"&arg4")) 
	{
		sprintf(crap, "%d", g_scriptInstance[script]->arg4);
		return;
	}

	if (compare(crap, (char*)"&arg5"))
	{
		sprintf(crap, "%d", g_scriptInstance[script]->arg5);
		return;
	}

	if (compare(crap, (char*)"&arg6")) 
	{
		sprintf(crap, "%d", g_scriptInstance[script]->arg6);
		return;
	}

	if (compare(crap, (char*)"&arg7")) 
	{
		sprintf(crap, "%d", g_scriptInstance[script]->arg7);
		return;
	}

	if (compare(crap, (char*)"&arg8"))
	{
		sprintf(crap, "%d", g_scriptInstance[script]->arg8);
		return;
	}

	if (compare(crap, (char*)"&arg9")) 
	{
		sprintf(crap, "%d", g_scriptInstance[script]->arg9);
		return;
	}

	for (int i = 1; i < max_vars; i ++)
	{
		if (g_dglos.g_playerInfo.var[i].active == true) if (  i == get_var(script, g_dglos.g_playerInfo.var[i].name)) //redink1 changed for recursive scoping
			if (compare(g_dglos.g_playerInfo.var[i].name, crap))
			{
				sprintf(crap, "%d",g_dglos.g_playerInfo.var[i].var);   
				//        check_for_real_vars(crap, i);
				return;
			}
	}
}

//redink1 added, grabs the var index matching 'name' with the shortest scope.  Basically iterates for each scope seperate until it finds a match.
int get_var(int script, char* name)
{
	//Can optimize here, by searching through variable array for start and end limits

	//Loop forever...
	while (1)
	{
		//We'll start going through every var, starting at one
		int var = 1;
		while (var < max_vars)
		{
			//Okay... make sure the var is active,
			//The scope should match the script,
			//Then make sure the name is the same.
			if (g_dglos.g_playerInfo.var[var].active && g_dglos.g_playerInfo.var[var].scope == script && compare(g_dglos.g_playerInfo.var[var].name, name))
				return var;

			//Otherwise, go to the next var.
			var++;
		}

		//If we just went through the global list, let's return
		if (script <= 0)
			break;

		//Bugfix... if there is no rinfo[script] entry (like if kill this task was used), we go directly to the globals.
		//Thanks Tal!
		//if (!rinfo[script])
		//  script = 0;
		//Go into the next proc from the script.  If there are no parent procs, it should be 0, which is global.
		//else
		//    script = rinfo[script]->proc_return;

		//Changed to not reference the parent procedure's variable list at all... just go on to globals.
		script = 0;
	}

	return 0;
}

//redink1 changes for replacing var in string
bool recurse_var_replace(int i, int script, char* line, char* prevar)
{
	while (i < max_vars)
	{
		//First, make sure the variable is active.
		//Then, make sure it is in scope,
		//Then, see if the variable name is in the line
		//Then, prevar is null, or if prevar isn't null, see if current variable starts with prevar
		
		if (g_dglos.g_playerInfo.var[i].active &&
			i == get_var(script, g_dglos.g_playerInfo.var[i].name) &&
			strstr(line, g_dglos.g_playerInfo.var[i].name) &&
			(prevar == NULL || prevar != NULL && strstr(g_dglos.g_playerInfo.var[i].name, prevar)))
		{
			//Look for shorter variables
			if (!recurse_var_replace(i + 1, script, line, g_dglos.g_playerInfo.var[i].name))
			{
				//we didn't find any, so we replace!
				char crap[20];
				sprintf(crap, "%d", g_dglos.g_playerInfo.var[i].var);
				replace(g_dglos.g_playerInfo.var[i].name, crap, line);
				//return true;
			}
		}
		i++;
	}
	return false;
}

bool var_compare( varman* lhs, varman* rhs )
{
    // Primarily sort by var string length descending (long strings take precedence over short strings).
    // Also, long global variables take precedence over short local variables.
    return (strlen( lhs->name ) > strlen( rhs->name )) ||
           // If strings are the same length, then sort by scope descending
           (strlen( lhs->name ) == strlen( rhs->name ) &&
             // Sort by scope descending (local scope takes precedence over global scope); not as important as string length.
             (lhs->scope > rhs->scope));
}

//redink1 changes for replacing var in string, 13 years later, not as hellish.
//Still seems inefficient to create a vector on every line of DinkC that contains
//a variable though.
void var_replace( int script, char* line )
{
    char crap[255];
    if ( strchr( line, '&' ) != nullptr ) //This may not be necessary
    {
        // Filter vars to those only in script scope and global scope
        std::vector<varman*> vars;
        for ( int i = 1; i < max_vars; i++ )
        {
            auto& var = g_dglos.g_playerInfo.var[i];
            if ( var.active == true && ( var.scope == 0 || var.scope == script ) )
            {
                vars.push_back( &var );
            }
        }

        // Sort so long variables first, then local variables first.
        std::sort( vars.begin(), vars.end(), var_compare );

        // Now replace
        for ( auto& var : vars )
        {
            sprintf( crap, "%d", var->var );
            replace( var->name, crap, line );

            if ( strchr( line, '&' ) == nullptr )
            {
                break;
            }
        }
    }
}

void decipher_string(char line[512], int script)
{
	char crap[255];
	char buffer[255];
	char crab[255];
	int mytime;

	//redink1 replaced with recursive function for finding longest variable
    //recurse_var_replace( 1, script, line, NULL );
    var_replace(script, line);

	//  	
// Old version that can make mistakes, I turned it back on to test the difference in speed...
// Hmm, Dan had recurse_scope(g_dglos.g_playerInfo.var[i].scope, script) in here but I don't see that function so
// skipping for now
//

// 
// 	for (int i = 1; i < max_vars; i ++)
// 	{
// 		
// 		if (g_dglos.g_playerInfo.var[i].active == true)
// 			if ( g_dglos.g_playerInfo.var[i].scope == 0 || g_dglos.g_playerInfo.var[i].scope == script)
// 			{
// 			sprintf(crap, "%d", g_dglos.g_playerInfo.var[i].var);
// 			replace(g_dglos.g_playerInfo.var[i].name, crap, line);
// 			
// 			}
// 	}


	if ((strchr(line, '&') != NULL) && (script != 0))
	{
		replace((char*)"&current_sprite",rt_ltoa(g_scriptInstance[script]->sprite, buffer, 10), line);
		replace((char*)"&current_script",rt_ltoa(script, buffer, 10), line);
		//v1.08 special variables.
		replace((char*)"&return",rt_ltoa(g_dglos.g_returnint, buffer, 10), line);
		replace((char*)"&arg1",rt_ltoa(g_scriptInstance[script]->arg1, buffer, 10), line);
		replace((char*)"&arg2",rt_ltoa(g_scriptInstance[script]->arg2, buffer, 10), line);
		replace((char*)"&arg3",rt_ltoa(g_scriptInstance[script]->arg3, buffer, 10), line);
		replace((char*)"&arg4",rt_ltoa(g_scriptInstance[script]->arg4, buffer, 10), line);
		replace((char*)"&arg5",rt_ltoa(g_scriptInstance[script]->arg5, buffer, 10), line);
		replace((char*)"&arg6",rt_ltoa(g_scriptInstance[script]->arg6, buffer, 10), line);
		replace((char*)"&arg7",rt_ltoa(g_scriptInstance[script]->arg7, buffer, 10), line);
		replace((char*)"&arg8",rt_ltoa(g_scriptInstance[script]->arg8, buffer, 10), line);
		replace((char*)"&arg9",rt_ltoa(g_scriptInstance[script]->arg9, buffer, 10), line);

		if (decipher_savegame != 0)
		{
			if (g_dglos.g_playerInfo.button[decipher_savegame] == 1)    replace((char*)"&buttoninfo", "Attack", line);
			else
				if (g_dglos.g_playerInfo.button[decipher_savegame] == 2)    replace((char*)"&buttoninfo", "Talk/Examine", line);
			if (g_dglos.g_playerInfo.button[decipher_savegame] == 3)    replace((char*)"&buttoninfo", "Magic", line);
			if (g_dglos.g_playerInfo.button[decipher_savegame] == 4)    replace((char*)"&buttoninfo", "Item Screen", line);
			if (g_dglos.g_playerInfo.button[decipher_savegame] == 5)    replace((char*)"&buttoninfo", "Main Menu", line);
			if (g_dglos.g_playerInfo.button[decipher_savegame] == 6)    replace((char*)"&buttoninfo", "Map", line);
			if (g_dglos.g_playerInfo.button[decipher_savegame] == 7)    replace((char*)"&buttoninfo", "Unused", line);
			if (g_dglos.g_playerInfo.button[decipher_savegame] == 8)    replace((char*)"&buttoninfo", "Unused", line);
			if (g_dglos.g_playerInfo.button[decipher_savegame] == 9)    replace((char*)"&buttoninfo", "Unused", line);
			if (g_dglos.g_playerInfo.button[decipher_savegame] == 10)   replace((char*)"&buttoninfo", "Unused", line);
		}
	}

	if (decipher_savegame != 0)
		if (compare(line, (char*)"&savegameinfo"))
		{
			if (decipher_savegame == 10)
			{
				string autoSave = DinkGetSavePath() + "autosave.dat";
			
				if (!FileExists(autoSave))
				{
					sprintf(line, "Auto Save - None yet");
				} else
				{
					mytime = 0;
					string description = "Unknown";

					VariantDB db;
					bool bFileExisted = false;

					if (db.Load(DinkGetSavePath()+"autosavedb.dat", &bFileExisted, false) && bFileExisted )
					{
						mytime = db.GetVar("minutes")->GetUINT32();
						description = db.GetVar("description")->GetString();
					}

					sprintf(line, "Auto Save - %d:%02d - %s", (mytime / 60), mytime - ((mytime / 60) * 60), description.c_str());
				}
			} else
			{
	
				sprintf(crap, "%ssave%d.dat",g_dglo.m_savePath.c_str(), decipher_savegame);   
				
				if (FileExists(crap))
				{
					load_game_small(decipher_savegame, crab, &mytime);
					//redink1 fix for savegame time bug
					sprintf(line, "Slot %d - %d:%02d - %s", decipher_savegame, (mytime / 60), mytime - ((mytime / 60) * 60) , crab);
					//sprintf(line, "In Use");  
				} else
				{
					sprintf(line, "Slot %d - Empty",decipher_savegame);
				}
			}
		}
}

bool get_parms(char proc_name[20], int32 script, char *h, int32 p[10])
{
	memset(g_nlist, 0, 10 * sizeof(int));
	char crap[256];
	strip_beginning_spaces(h);
	if (h[0] == '(')
	{
		//Msg("Found first (.");
		h = &h[1];

	} else
	{
		LogMsg("Missing ( in %s, offset %d.", g_scriptInstance[script]->name, g_scriptInstance[script]->current);
		return(false);
	}

	for (int i = 0; i < 10; i++)
	{
		strip_beginning_spaces(h);

		if (p[i] == 1)
		{
			// Msg("Checking for number..");


			if (strchr(h, ',') != NULL) 
				separate_string(h, 1,',',crap); else
				if (strchr(h, ')') != NULL) 
					separate_string(h, 1,')',crap);


			h = &h[strlen(crap)];     


			if (crap[0] == '&')
			{
				replace(" ", "", crap);
				//  Msg("Found %s, 1st is %c",crap, crap[0]);    
				decipher(crap, script);
			}

			g_nlist[i] = atol( crap);

		} else

			if (p[i] == 2)
			{
				// Msg("Checking for string..");
				separate_string(h, 2,'"',crap);
				h = &h[strlen(crap)+2];     

				//Msg("Found %s",crap);    
				strcpy(slist[i], crap);
			}

			if ( p[i+1] == 0)
			{
				//finish
				strip_beginning_spaces(h);

				if (h[0] == ')')
				{
					h = &h[1];
				} else
				{

					LogMsg("Missing ) in %s, offset %d.", g_scriptInstance[script]->name, g_scriptInstance[script]->current);
					h = &h[1];

					return(false);
				}

				strip_beginning_spaces(h);

				if (h[0] == ';')
				{
					//  Msg("Found ending ;");
					h = &h[1];

				} else
				{
					//Msg("Missing ; in %s, offset %d.", rinfo[script]->name, rinfo[script]->current);
					//  h = &h[1];
					return(true);
				}

				return(true);
			}


			//got a parm, but there is more to get, lets make sure there is a comma there
			strip_beginning_spaces(h);

			if (h[0] == ',')
			{
				//     Msg("Found expected ,");
				h = &h[1];

			} else
			{
				if (strcmp("external", proc_name) != 0)
				{
					LogMsg("Procedure %s does not take %d parms in %s, offset %d. (%s?)", proc_name, i + 1, g_scriptInstance[script]->name, g_scriptInstance[script]->current, h);
				}
				else
				{
					//fake error, external commands always generate this error because of Dan's weird user-function overloading thing
					return true;
				}
				//set it to zero to be "safe"?
				//(p[i]
				return(false);
			}
	}


	return(true);
}

int GetCallbacksActive()
{
	int count = 0;

	for (int k = 1; k < C_MAX_SCRIPT_CALLBACKS; k++)
	{
		if (g_dglos.g_scriptCallback[k].active)
		{
			count++;
		}
	}

	return count;
}

int GetScriptsActive()
{
	int count = 0;

	for (int k = 1; k < C_MAX_SCRIPTS; k++)
	{
		if (g_scriptInstance[k])
		{
			count++;
		}
	}

	return count;
}

int add_callback(char name[20], int n1, int n2, int script)
{

#ifdef _DEBUG
	//LogMsg("%d callbacks active", GetCallbacksActive());

#endif
	for (int k = 1; k < C_MAX_SCRIPT_CALLBACKS; k++)
	{
		if (g_dglos.g_scriptCallback[k].active == false)
		{
			memset(&g_dglos.g_scriptCallback[k],0, sizeof(g_dglos.g_scriptCallback[k]));

			g_dglos.g_scriptCallback[k].active = true;
			g_dglos.g_scriptCallback[k].min = n1;
			g_dglos.g_scriptCallback[k].max = n2;
			g_dglos.g_scriptCallback[k].owner = script;
			strcpy(g_dglos.g_scriptCallback[k].name, name);

			if (g_script_debug_mode) LogMsg("Callback added to %d.", k);
			return(k);
		}

	}

#ifdef _DEBUG
	LogMsg("Couldn't add callback, all out of space");
#endif

	return(0);

}

int add_sprite(int x1, int y, int brain,int pseq, int pframe )
{

	for (int x=1; x < C_MAX_SPRITES_AT_ONCE; x++)
	{
		if (g_sprite[x].active == false)
		{
			SAFE_DELETE(g_customSpriteMap[x]);

			memset(&g_sprite[x], 0, sizeof(g_sprite[x]));

			g_sprite[x].active = true;
			g_sprite[x].x = x1;
			g_sprite[x].y = y;
			g_sprite[x].my = 0;
			g_sprite[x].mx = 0;
			g_sprite[x].speed = 1;
			g_sprite[x].brain = brain;
			g_sprite[x].frame = 0;
			g_sprite[x].pseq = pseq;
			g_sprite[x].pframe = pframe;
			g_sprite[x].seq = 0;
			if (x > g_dglos.last_sprite_created)
				g_dglos.last_sprite_created = x;
			g_sprite[x].timer = 33;
			g_sprite[x].wait = 0;
			g_sprite[x].lpx[0] = 0;
			g_sprite[x].lpy[0] = 0;
			g_sprite[x].moveman = 0;
			g_sprite[x].size = 100;
			g_sprite[x].que = 0;
			g_sprite[x].strength = 0;
			g_sprite[x].damage = 0;
			g_sprite[x].defense = 0; 
			g_sprite[x].hard = 1;

			if ( g_customSpriteMap[x] == NULL )
			{
				g_customSpriteMap[x] = new std::map<std::string, int32>;
			}
			else
			{
				g_customSpriteMap[x]->clear();
			}

			return(x);
		}

	}
	LogMsg("Out of sprites, can't create!");
	return(0);
}

LPDIRECTDRAWSURFACE GetSurfaceFromSeq(int seq, int frame = 1)
{
	return g_pSpriteSurface[g_dglos.g_seq[seq].frame[frame]];
}

bool check_sprite_status(int spriteID)
{

	
	check_seq_status(g_sprite[spriteID].pseq, g_sprite[spriteID].pframe);
	check_seq_status(g_sprite[spriteID].seq, g_sprite[spriteID].frame);
	return true;
}


void check_frame_status(int h, int frame)
{

	if (g_dglos.g_seq[h].active == false) return;

	if (h > 0) 
	{
		// Msg("Smartload: Loading seq %d..", spr[h].seq);
		if (GetSurfaceFromSeq(h, frame) == 0)   
		{   
			ReloadSequence(h, frame);
		}
	}
}

void FreeSequence(int seq)
{
	assert(seq && seq < C_MAX_SEQUENCES);
		
	if (1)
	{
		for (int i=0; i < g_dglos.g_seq[seq].m_spaceAllowed; i++)
		{
			//note, instead of doing frame[1+1] we do frame[1]+1, to really delete whatever
			//was loaded, otherwise things get corrupted because of how the SET_FRAME_FRAME
			//command allows some frames to be reused.
			assert(g_dglos.g_seq[seq].s+1 == g_dglos.g_seq[seq].frame[1]);
			//LogMsg("Deleting seq %d, frame %d  (%d) (image: %d)", seq, i, g_dglos.g_seq[seq].frame[1]+i, g_dglos.g_picInfo[g_dglos.g_seq[seq].frame[1]+i] );
			SAFE_DELETE(g_pSpriteSurface[g_dglos.g_seq[seq].frame[1] + i]);
		}

	}
}

bool check_pic_status(int picID)
{
	if (picID == 0) return true;

	
#ifdef _DEBUG
	if (g_dglos.g_picInfo[picID].m_parentSeq == 0)
	{
		LogMsg("Huh, this should have a parent..");
	}

#endif

	if (g_pSpriteSurface[picID]) return true;

	return check_seq_status(g_dglos.g_picInfo[picID].m_parentSeq);
}

void ScanSeqFilesIfNeeded(int seq)
{
	if (!g_dglos.g_seq[seq].m_bDidFileScan)
	{
		//LogMsg("Need to scan it now");
		load_sprites(g_dglos.g_seq[seq].m_fileName, seq, g_dglos.g_seq[seq].m_speed, g_dglos.g_seq[seq].m_xoffset, g_dglos.g_seq[seq].m_yoffset, g_dglos.g_seq[seq].m_hardbox,
			g_dglos.g_seq[seq].m_transType, g_dglos.g_seq[seq].m_bLeftAlign, true);
	}

}

bool check_seq_status(int seq, int frame)
{
	
	if (seq == 0) return true;
	if (seq < 0 || seq >= C_MAX_SEQUENCES)
	{
		assert(!"Illegal sequence!");
	}
	if (g_dglos.g_seq[seq].active == false) 
	{
#ifdef _DEBUG
LogMsg("Seq %d missing?", seq);
#endif
		return true;
	}
	
	
#ifdef _DEBUG
	if (seq == 180)
	{
		//LogMsg("Woah!");
	}

#endif


	if (frame < 0 || frame >= C_MAX_SPRITE_FRAMES)
	{
		//invalid!
		assert(!"Illegal sprite frame. Track where it came from!");
		return true; //avoid crash
	}

	ScanSeqFilesIfNeeded(seq);

	if (frame != 0)
	{
		if (g_pSpriteSurface[g_dglos.g_seq[seq].frame[frame]]) return true;
		//load a single frame
		return ReloadSequence(seq, frame);
	}

	if (seq > 0) if (seq < C_MAX_SEQUENCES) 
	{
		// Msg("Smartload: Loading seq %d..", spr[h].seq);

		for (int i=0; i < g_dglos.g_seq[seq].last; i++)
		{
			if (g_pSpriteSurface[g_dglos.g_seq[seq].frame[i + 1]] == NULL)
			{
				if (!ReloadSequence(seq, i+1)) return false;
			}
		}
	}
	return true; //no error
}

void check_base(int base)
{
	/*
	for (int i=1; i < 10; i++)
	{
		if (g_dglos.g_seq[base+i].active == true) check_seq_status(base+i);
	}
	*/
}

void check_sprite_status_full(int spriteID)
{
	//same as above but checks for all seq's used by the (base) commands
	//is sprite in memory?
	//check_seq_status(g_sprite[spriteID].pseq);
	 check_sprite_status(spriteID);
	//if (g_sprite[spriteID].base_walk > -1) check_base(g_sprite[spriteID].base_walk);    
}


int say_text(char text[512], int h, int script)
{
	int crap2;
	//Msg("Creating new sprite with %s connect to %d.",text, h);
	if (h == 1000) crap2 = add_sprite(100,100,8,0,0);
	else crap2 = add_sprite(g_sprite[h].x,g_sprite[h].y,8,0,0);   

	if (crap2 == 0)
	{
		LogMsg("Couldn't say something, out of sprites.");
		return(0);

	}
	*plast_text = crap2;    
	strcpy(g_sprite[crap2].text, text);
	g_sprite[crap2].kill = strlen(text) * text_timer;
	if (g_sprite[crap2].kill < text_min) g_sprite[crap2].kill = text_min;
	g_sprite[crap2].damage = -1;
	g_sprite[crap2].owner = h;
	g_sprite[crap2].hard = 1;
	g_sprite[crap2].script = script;
	//set X offset for text, using strength var since it's unused
	g_sprite[crap2].strength = 75;
	//spr[h].x - spr[crap2;
	g_sprite[crap2].nohit = 1;
	check_seq_status(g_sprite[g_sprite[crap2].owner].seq, g_sprite[g_sprite[crap2].owner].frame);
	g_sprite[crap2].defense = ( ((g_dglos.g_picInfo[getpic(g_sprite[crap2].owner)].box.bottom) - g_dglos.g_picInfo[getpic(g_sprite[crap2].owner)].yoffset) + 100);

	g_sprite[crap2].x = g_sprite[g_sprite[crap2].owner].x - g_sprite[crap2].strength;
	g_sprite[crap2].y = g_sprite[g_sprite[crap2].owner].y - g_sprite[crap2].defense;

	return(crap2);
}


int say_text_xy(char text[512], int mx, int my, int script)
{
	int crap2;
	//Msg("Creating new sprite with %s connect to %d.",text, h);
	crap2 = add_sprite(mx,my,8,0,0);    

	if (crap2 == 0)
	{
		LogMsg("Couldn't say something, out of sprites.");
		return(0);

	}
	*plast_text = crap2;    
	strcpy(g_sprite[crap2].text, text);
	g_sprite[crap2].kill = strlen(text) * text_timer;
	if (g_sprite[crap2].kill < text_min) g_sprite[crap2].kill = text_min;
	g_sprite[crap2].damage = -1;
	g_sprite[crap2].nohit = 1;
	g_sprite[crap2].owner = 1000;
	g_sprite[crap2].hard = 1;
	g_sprite[crap2].script = script;
#ifdef _DEBUG
  	LogMsg("Setting say_xy script to %d, returning sprite %d", script, crap2);
#endif
	return(crap2);
}

int does_sprite_have_text(int sprite)
{
	//Msg("getting callback # with %d..", sprite);
	for (int k = 1; k <= C_MAX_SPRITES_AT_ONCE; k++)
	{
		if (   g_sprite[k].active) if (g_sprite[k].owner == sprite) if (g_sprite[k].brain == 8) 
		{
			//Msg("Found it!  returning %d.", k);

			return(k);
		}
	}
	return(0);
}

int var_exists(char name[20], int scope)
{

	for (int i = 1; i < max_vars; i++)
	{
		if (g_dglos.g_playerInfo.var[i].active)
		{
			if (compare(g_dglos.g_playerInfo.var[i].name, name))
			{ 

				if (g_dglos.g_playerInfo.var[i].scope == scope) //redink1 changed to check recursively... then changed back.  Hrm.
				{
					//Msg("Found match for %s.", name);
					return(i);
				}
			}
		}
	}

	return(0);
}

//redink1 added this to make new global functions
void make_function(char file[10], char func[20])
{
	//See if it already exists
	bool exists = false;
	int i;
	for (i = 0; strlen(g_dglos.g_playerInfo.func[i].func) > 0 && i < 100; i++)
	{
		if (compare(func, g_dglos.g_playerInfo.func[i].func))
		{
			exists = true;
			break;
		}
	}

	if (exists)
	{
		strncpy(g_dglos.g_playerInfo.func[i].file, file, 10);
	}
	else
	{
		strncpy(g_dglos.g_playerInfo.func[0].file, file, 10);
		strncpy(g_dglos.g_playerInfo.func[0].func, func, 20);
	}
}

void make_int(char name[80], int value, int scope, int script)
{
	int dupe;
	if (strlen(name) > 19)
	{

		LogMsg("ERROR:  Varname %s is too long in script %s.",name, g_scriptInstance[script]->name);
		return;
	}
	dupe = var_exists(name, scope);

	if (dupe > 0)
	{
		if (scope != 0)
		{
#ifdef _DEBUG
			//LogMsg("Local var %s already used in this procedure in script %s.",name, g_scriptInstance[script]->name);
#endif

			g_dglos.g_playerInfo.var[dupe].var = value;

		} else
		{
#ifdef _DEBUG
			LogMsg("Var %s is already a global, not changing value.",name);
#endif
		}

		return;
	}

	//make new var

	for (int i = 1; i < max_vars; i++)
	{
		if (g_dglos.g_playerInfo.var[i].active == false)
		{

			g_dglos.g_playerInfo.var[i].active = true;
			g_dglos.g_playerInfo.var[i].scope = scope;
			strcpy(g_dglos.g_playerInfo.var[i].name, name); 
			//g("var %s created, used slot %d ", name,i);
			g_dglos.g_playerInfo.var[i].var = value;
			return;
		}
	}

	LogMsg("ERROR: Out of var space, all %d used.", max_vars);
}

int var_equals(char name[20], char newname[20], char math, int script, char rest[200])
{
	int k;  
	//redink1 set newret to NULL so debug errors did not appear.
	int newret = NULL; // = NULL;

	if (name[0] != '&')
	{
		LogMsg("ERROR (var equals): Unknown var %s in %s offset %d.",name, g_scriptInstance[script]->name, g_scriptInstance[script]->current);
		return(0);
	}

#ifdef _DEBUG
	if (string(name) == "&vision")
	{
		LogMsg("Var!");
	}
#endif
	int i = get_var(script, name);
	if (i > 0)
	{
		goto next;
	}

	LogMsg("ERROR: (var equals2) Unknown var %s in %s offset %d.",name, g_scriptInstance[script]->name, g_scriptInstance[script]->current);
	return(0);

next:
	int newval = 0;

	if (strchr(rest, '(') != NULL) 

	{
		newret = process_line(script, rest, false);
		newval = g_dglos.g_returnint;
		goto next2;
	}

	if (strchr(newname, ';') != NULL) replace(";", "", newname); 

	//redink1 fixed for scope and such
	k = get_var(script, newname);
	if (k > 0)
	{
		newval = g_dglos.g_playerInfo.var[k].var;
		goto next2;
	}

	if (compare(newname, (char*)"&current_sprite")) 
	{
		newval = g_scriptInstance[script]->sprite;
		goto next2;
	}

	if (compare(newname, (char*)"&current_script")) 
	{
		newval = script;
		goto next2;

	}

	//v1.08 special variables.
	if (compare(newname, (char*)"&return")) 
	{
		newval = g_dglos.g_returnint;
		goto next2;
	}

	if (compare(newname, (char*)"&arg1")) 
	{
		newval = g_scriptInstance[script]->arg1;
		goto next2;
	}

	if (compare(newname, (char*)"&arg2")) 
	{
		newval = g_scriptInstance[script]->arg2;
		goto next2;
	}

	if (compare(newname, (char*)"&arg3")) 
	{
		newval = g_scriptInstance[script]->arg3;
		goto next2;
	}

	if (compare(newname, (char*)"&arg4")) 
	{
		newval = g_scriptInstance[script]->arg4;
		goto next2;
	}

	if (compare(newname, (char*)"&arg5")) 
	{
		newval = g_scriptInstance[script]->arg5;
		goto next2;
	}

	if (compare(newname, (char*)"&arg6")) 
	{
		newval = g_scriptInstance[script]->arg6;
		goto next2;
	}

	if (compare(newname, (char*)"&arg7")) 
	{
		newval = g_scriptInstance[script]->arg7;
		goto next2;
	}

	if (compare(newname, (char*)"&arg8")) 
	{
		newval = g_scriptInstance[script]->arg8;
		goto next2;
	}

	if (compare(newname, (char*)"&arg9")) 
	{
		newval = g_scriptInstance[script]->arg9;
		goto next2;
	}

	newval = atol(newname);

next2:

	if (math == '=')
		g_dglos.g_playerInfo.var[i].var = newval;

	if (math == '+')
		g_dglos.g_playerInfo.var[i].var += newval;

	if (math == '-')
		g_dglos.g_playerInfo.var[i].var -= newval;

	if (math == '/')
		g_dglos.g_playerInfo.var[i].var = g_dglos.g_playerInfo.var[i].var / newval;

	if (math == '*')
		g_dglos.g_playerInfo.var[i].var = g_dglos.g_playerInfo.var[i].var * newval;

	return(newret);
}

void get_word(char line[300], int word, char *crap)
{
	int cur = 0;

	bool space_mode = false;
	char save_word[100];
	save_word[0] = 0;

	for (int k = 0; k < strlen(line); k++)
	{

		if (space_mode == true)
		{
			if (line[k] != ' ')
			{
				space_mode = false;
				strcpy(save_word, "");

			}
		}

		if (space_mode == false)
		{
			if (line[k] == ' ')
			{
				cur++;        
				if (word == cur) goto done;
				space_mode = true;
				strcpy(save_word, "");

				goto dooba;
			} else
			{
				strchar(save_word, line[k]);

			}
		}

dooba:;

	}

	if (space_mode == false)
	{

		if (cur+1 != word) strcpy(save_word, "");
	} 

done:

	strcpy(crap, save_word);

	//Msg("word %d of %s is %s.", word, line, crap);
}

int var_figure(char h[512], int script)
{
	char crap[512];
	int ret = 0;
	int n1 = 0, n2 = 0;
	//Msg("Figuring out %s...", h);
	get_word(h, 2, crap);
	//Msg("Word two is %s...", crap);

	if (compare(crap, (char*)""))
	{
		//one word equation

		if (h[0] == '&')
		{
			//its a var possibly
			decipher_string(h, script);
		} 

		//Msg("truth is %s", h);
		ret =  atol(h);
		//  Msg("returning %d, happy?", ret);
		return(ret);
	}

	get_word(h, 1, crap);
	//Msg("Comparing %s...", crap);

	decipher_string(crap,script);
	n1 = atol(crap);

	get_word(h, 3, crap);
	replace(")", "", crap);
	//Msg("to  %s...", crap);
	decipher_string(crap,script);
	n2 = atol(crap);

	get_word(h, 2, crap);
	if (g_script_debug_mode)
		LogMsg("Compared %d to %d",n1, n2);

	if (compare(crap, (char*)"=="))
	{
		if (n1 == n2) ret = 1; else ret = 0;
		return(ret);
	}

	if (compare(crap, (char*)">"))
	{
		if (n1 > n2) ret = 1; else ret = 0;
		return(ret);
	}

	if (compare(crap, (char*)"<"))
	{
		if (n1 < n2) ret = 1; else ret = 0;
		return(ret);
	}
	
	if (compare(crap,(char*) "!="))
	{
		if (n1 != n2) ret = 1; else ret = 0;
		return(ret);
	}

	if (compare(crap, (char*)"<="))
	{
		if (n1 <= n2) ret = 1; else ret = 0;
		return(ret);
	}
	if (compare(crap, (char*)">="))
	{
		if (n1 >= n2) ret = 1; else ret = 0;
		return(ret);
	}

	return(ret);

}

void kill_text_owned_by(int sprite)
{
	for (int i = 1; i < C_MAX_SPRITES_AT_ONCE; i++)
	{
		if (g_sprite[i].active)
			if (g_sprite[i].brain == 8) if (g_sprite[i].owner == sprite)
			{
				g_sprite[i].active = false;
			}
	}
}

bool text_owned_by(int sprite)
{
	for (int i = 1; i < C_MAX_SPRITES_AT_ONCE; i++)
	{
		if (g_sprite[i].active)
			if (g_sprite[i].brain == 8) if (g_sprite[i].owner == sprite)
			{
				return(true);
			}
	}
	return(false);  
}


void kill_text_owned_by_safe(int sprite)
{
	for (int i = 1; i < C_MAX_SPRITES_AT_ONCE; i++)
	{
		if (g_sprite[i].active)
			if (g_sprite[i].brain == 8) if (g_sprite[i].owner == sprite)
			{
				g_sprite[i].active = false;

				if (g_sprite[i].callback != 0) run_script(g_sprite[i].callback);
			}
	}

}

void kill_scripts_owned_by(int sprite)
{
	for (int i = 1; i < C_MAX_SCRIPTS; i++)
	{
		if (g_scriptInstance[i] != NULL)
		{
			if (g_scriptInstance[i]->sprite == sprite)
			{
				kill_script(i);
			}
		}
	}

}

void kill_sprite_all (int sprite)
{
	g_sprite[sprite].active = false;

	kill_text_owned_by(sprite);
	kill_scripts_owned_by(sprite);
}


void kill_returning_stuff( int script)
{
	int i;
	for (i = 1; i < C_MAX_SCRIPT_CALLBACKS; i++)
	{
		if (g_dglos.g_scriptCallback[i].active) if (g_dglos.g_scriptCallback[i].owner == script)
		{
			//LogMsg("killed a returning callback, ha!");
			g_dglos.g_scriptCallback[i].active = false;
		}
	}

	
	for (i = 1; i <= g_dglos.last_sprite_created; i++)
	{
		if (g_sprite[i].active) if (g_sprite[i].brain == 8) if (g_sprite[i].callback == script)
		{
			LogMsg("Killed sprites callback command");
			g_sprite[i].callback = 0;
		}
	}    
}

bool talk_get(int script)
{
	char line[512], check[512], checker[512];
	int cur = 1;
	char *p;
	int retnum = 0;
	clear_talk();                
	g_dglos.g_talkInfo.newy = -5000;
	while(1)
	{

redo:
		read_next_line(script, line);
		strip_beginning_spaces(line);
		//Msg("Comparing to %s.", line);

		get_word(line, 1, checker);

		if (compare(checker, (char*)"set_y"))
		{
			get_word(line, 2, checker);
			g_dglos.g_talkInfo.newy = atol(checker);
			goto redo;
		}

		if (compare(checker, (char*)"set_title_color"))
		{
			get_word(line, 2, checker);
			g_dglos.g_talkInfo.color = atol(checker);
			goto redo;
		}

		if (compare(line, (char*)"\n")) goto redo;
		if (compare(line, (char*)"\\\\")) goto redo;


		strip_beginning_spaces(line);
		//Msg("Comparing to %s.", line);
		if (compare(line, (char*)"\n")) goto redo;
		if (compare(line, (char*)"\\\\")) goto redo;

morestuff:

		separate_string(line, 1, '(', check);
		strip_beginning_spaces(check);

		if (compare(check, (char*)"title_start"))
		{
			while(read_next_line(script, line))
			{
				strcpy(check, line);    
				strip_beginning_spaces(line);
				get_word(line, 1, checker);
				separate_string(line, 1, '(', check);
				strip_beginning_spaces(check);

				if (compare(check, (char*)"title_end"))
				{
					replace((char*)"\n\n\n\n",(char*)"\n \n", g_dglos.g_talkInfo.buffer);
					replace((char*)"\n\n",(char*)"\n", g_dglos.g_talkInfo.buffer);
					goto redo;
				}

				assert("!You better check this..");
				line[strlen(line)] = 0;
				//Msg("LINE IS: %s: Like it?",line);

				decipher_string(line, script);
				strcat(g_dglos.g_talkInfo.buffer, line);
				//talk.buffer[strlen(talk.buffer)-1] = 0;   
			}

			goto redo;
		}

		if (compare(check, (char*)"choice_end"))
		{
			if (cur-1 == 0) 
			{
				LogMsg("Error: choice() has 0 options in script %s, offset %d.",
					g_scriptInstance[script]->name, g_scriptInstance[script]->current);

				return(false);
			}
			//all done, lets jam
			//Msg("found choice_end, leaving!");
			g_dglos.g_talkInfo.last = cur-1;
			g_dglos.g_talkInfo.cur = 1;
			g_dglos.g_talkInfo.active = true;
			g_dglos.g_talkInfo.page = 1;
			g_dglos.g_talkInfo.cur_view = 1;
			g_dglos.g_talkInfo.script = script;
			//kill the punch button if it was pressed, otherwise if you punch and talk to someone it crashes the game.
			//this was also in the original dink, amazing nobody saw it?
			g_dglo.m_dirInputFinished[DINK_INPUT_BUTTON1] = false;
			g_dglo.m_dirInput[DINK_INPUT_BUTTON1] = false;
			g_dglos.g_playerInfo.mouse = 0;
			g_dglo.m_dirInputFinished[DINK_INPUT_BUTTON2] = false;
			g_dglo.m_dirInput[DINK_INPUT_BUTTON2] = false;
			sjoy.button[1] = false;
			sjoy.button[2] = false;
			return(true);

		}

		separate_string(line, 1, '\"', check);
		strip_beginning_spaces(check);

		//Msg("Check is %s.",check);

		if (strlen(check) > 2)
		{
			//found conditional statement
			if (strchr(check, '(') == NULL)

			{
				LogMsg("Error with choice() statement in script %s, offset %d. (%s?)",
					g_scriptInstance[script]->name, g_scriptInstance[script]->current, check);
				return(false);
			}

			separate_string(check, 2, '(', checker);      
			separate_string(checker, 1, ')', check);      

			//Msg("Running %s through var figure..", check);
			if (var_figure(check, script) == 0)
			{
				//LogMsg("Answer is no.");
				retnum++;
				goto redo;
				//said NO to statement
			}
			//Msg("Answer is yes.");
			separate_string(line, 1, ')', check);      

			p = &line[strlen(check)+1];

			strcpy(check, p);


			strcpy(line, check);

			//Msg("new line is %s, happy?", line);
			goto morestuff;
		}

		separate_string(line, 2, '\"', check);
		strip_beginning_spaces(check);
#ifdef _DEBUG		
		//LogMsg("Line %d is %s.",cur,check);
#endif
		if (strcmp(check, "Quit to Windows") == 0)
		{
			strcpy(check, "Quit");
		}

		retnum++;
		decipher_savegame = retnum;
		decipher_string(check, script);              
		decipher_savegame = 0;
		strcpy(g_dglos.g_talkInfo.line[cur], check);
		g_dglos.g_talkInfo.line_return[cur] = retnum;
		cur++;
	}

}

bool PlayMidi(const char *sFileName)
{
	//first check for mp3 versions..
	string fName = "sound/"+ToLowerCaseString(sFileName);
	StringReplace("\\", "/", fName);

	if (GetFileExtension(fName) == "mid")
	{
		bool bTryUsingOgg = true;

		if (!g_dglo.m_dmodGameDir.empty() && FileExists(g_dglo.m_dmodGamePathWithDir+fName))
		{
			//actually, don't replace with ogg, they probably have their own midi stuff here
			bTryUsingOgg = false;
		}
		

		string tempName = fName;
		
		/*
		if (GetEmulatedPlatformID() == PLATFORM_ID_HTML5)
		{
			//ios browsers don't support ogg?!  Fine, let's assume we have MP3 versions
			tempName = ModifyFileExtension(fName, "mp3");
		}
		else
		*/
		{
			tempName = ModifyFileExtension(fName, "ogg");
		}
		
		if (bTryUsingOgg && !g_dglo.m_dmodGameDir.empty() && FileExists(g_dglo.m_dmodGamePathWithDir+tempName))
		{

			//found it
			fName =  tempName;

		} else
		if (!g_dglo.m_dmodGameDir.empty() && FileExists(g_dglo.m_dmodGamePathWithDir+fName))
		{
			
				//found it, no changed needed
				
			
		} else
		{
			//try the base dir for oggs too, but not if it's a dmod
			if (g_dglo.m_dmodGameDir.empty() && FileExists(g_dglo.m_gamePathWithDir+tempName))
			{
				//found it
				fName =  tempName;
			}
		}

	}

	string finalPath = GetFileLocationString(fName);
	if (FileExists(finalPath))
	{
		GetAudioManager()->Play(GetFileLocationString(fName), true, true, false);
	}
	else
	{

		return false;
	}

	g_dglo.m_lastMusicPath = sFileName;

	/*
	if (!FileExists(fName) && !g_dglo.m_dmodGameDir.empty())
	{
		//asking for a mid that doesn't exist in the DMOD dir probably, let's use the original Dink one
		fName = g_dglo.m_gamePathWithDir+
	}
	*/

	
	//LogMsg("Playing music %s", sFileName);
	return true;
}


void check_midi(void)
{

	char hold[20];

	if (!g_dglos.midi_active) return;
	if (g_MapInfo.music[*pmap] != 0)
	{

		if (g_MapInfo.music[*pmap] == -1)
		{
			//kill music
			LogMsg("Stopped cd");
			StopMidi();

		}

		if (g_MapInfo.music[*pmap] > 1000) 
		{
				sprintf(hold, "%d.mid",g_MapInfo.music[*pmap]-1000);
				
				PlayMidi(hold);
		} else
		{
			//there is music associated with this screen
			sprintf(hold, "%d.mid",g_MapInfo.music[*pmap]);
			PlayMidi(hold);
		}
		
	}

}


//------------------------------------------------------------------
// 
// Function     : StopMidi
//
// Purpose      : Stops a midi file playing
//
//------------------------------------------------------------------

bool StopMidi()
{
	g_dglo.m_lastMusicPath = "";
	//LogMsg("Stopping midi");
	GetAudioManager()->StopMusic();
	// Yahoo!
	return true;
}

void get_right(char line[512], char thing[100], char *ret)
{
	char *dumb;
	int pos = strcspn(line, thing );

	if (pos == 0){ strcpy(ret, ""); return; }

	dumb = &ret[pos+1];
	strcpy(ret, dumb);
}

void int_prepare(char line[256], int script)
{
	int def = 0;
	char hold[256];
	strcpy(hold, line);
	char name[100];
	char crap[256];
	replace("="," ",line);
	strcpy(crap, line);
	separate_string(crap, 1,';',line);
	get_word(line, 2, name);

	if (name[0] != '&')
	{
		LogMsg("ERROR:  Can't create var %s, should be &%s.", name,name);
		return;
	}


	make_int(name, def,script, script);

	strcpy(line, hold);

}

int32 change_sprite(int32 h,  int32 val, int32 * change)
{
	//Msg("Searching sprite %s with val %d.  Cur is %d", h, val, *change);
	if (h >= C_MAX_SPRITES_AT_ONCE)
	{
 		LogMsg("Error: Can't use sp_ commands on a sprite after it's connected to sprite 1000, can crash. Ignoring command.");
		return 0;
	}

	if (h < 1)
	{
#ifdef _DEBUG
		LogMsg("Error with an SP command - Sprite %d is invalid.", h);
#endif
		return(-1);
	}
	if (g_sprite[h].active == false) return(-1);
	if (val != -1)
	{
		*change = val;
	}

	return(*change);
}

int32 change_sprite(int32 h,  int32 val, bool * change)
{
	//Msg("Searching sprite %s with val %d.  Cur is %d", h, val, *change);

	if (h < 1)
	{
	#ifdef _DEBUG
		LogMsg("Error with an SP command - Sprite %d is invalid.", h);
#endif
		return(-1);
	}
	if (h >= C_MAX_SPRITES_AT_ONCE || g_sprite[h].active == false) return(-1);
	if (val != -1)
	{
		*change = val;
	}

	return(*change);
}

int change_edit(int h,  int val, unsigned short * change)
{
	//Msg("Searching sprite %s with val %d.  Cur is %d", h, val, *change);

	if (h > 99) return(-1);
	if (h < 1) return(-1);
	if (val != -1)
	{
		*change = val;
	}

	return(*change);

}
int change_edit_char(int h,  int val, unsigned char * change)
{
	//Msg("Searching sprite %s with val %d.  Cur is %d", h, val, *change);
	//  Msg("h is %d..",val); 
	if (h > 99) return(-1);
	if (h < 1) return(-1);
	if (val != -1)
	{
		*change = val;
	}

	return(*change);

}


int32 change_sprite_noreturn(int32 h,  int32 val, int32 * change)
{
	//Msg("Searching sprite %s with val %d.  Cur is %d", h, val, *change);
	if (h >= C_MAX_SPRITES_AT_ONCE || g_sprite[h].active == false) return(-1);

	{
		*change = val;
	}

	return(*change);

}


void draw_sprite_game(LPDIRECTDRAWSURFACE lpdest,int h)
{
	if (::g_b_kill_app) return; //don't try, we're quitting
	if (g_sprite[h].brain == 8) return;

	if (g_sprite[h].nodraw == 1) return;
	rtRect32 dstRect,srcRect;

	int             ddrval;

	DDBLTFX     ddbltfx;
	ddbltfx.dwSize = sizeof( ddbltfx);
	ddbltfx.dwFillColor = 0;

	if (getpic(h) < 1) return;



	
	
	if (!check_pic_status(getpic(h)))
	{
		
		LogMsg("Hmm, bad sprite at %d.. you need to setup a way for a pic to load itself", getpic(h));
		return;
	}
#ifdef _DEBUG
	if (!g_pSpriteSurface[getpic(h)])
	{
		int pic = getpic(h);

		LogMsg("Debug:  Bad pic here");
	}

#endif

 
#ifdef _DEBUG
	if (g_sprite[h].pseq == 75 && g_sprite[h].pframe == 8)
	{
		//LogMsg("Drawing the rock");
	}

	if (g_sprite[h].pseq == 133)
	{
		//LogMsg("Drawing a wall");
		
	}
#endif

	if (get_box(h, &dstRect, &srcRect))
	{
		//redink1 error checking for invalid rectangle
		if (dstRect.left >= dstRect.right || dstRect.top >= dstRect.bottom)
		{
			return;
			assert(!"Bad rect");
		}
	
		//check to see if we need a 32 bit buffer for this or not
		if (lpdest->m_pSurf && lpdest->m_pSurf->GetSurfaceType() == SoftSurface::SURFACE_PALETTE_8BIT)
		{
#ifdef _DEBUG
			
			SoftSurface *pSoftTemp = g_pSpriteSurface[getpic(h)]->m_pSurf;
#endif
			if (g_pSpriteSurface[getpic(h)]->m_pSurf->GetSurfaceType() == SoftSurface::SURFACE_RGBA)
			{
				//yep, convert what we've got to 32 bit.  We don't lose what we've done so far.
				assert(lpdest == lpDDSBackGround);
				//convert it to a high color surface on the fly, without losing the data on it
				LPDIRECTDRAWSURFACE pNewSurf = InitOffscreenSurface(C_DINK_SCREENSIZE_X, C_DINK_SCREENSIZE_Y, IDirectDrawSurface::MODE_SHADOW_GL, true, lpdest->m_pSurf);
				LogMsg("Detected high color bmps that need to drawn to the static landscape, converting backbuffers to 32 bit on the fly.");
				delete lpDDSBackGround;

				lpdest = lpDDSBackGround = pNewSurf;
				
				DDBLTFX     ddbltfx;
				ddbltfx.dwFillColor = g_dglos.last_fill_screen_palette_color;
				lpDDSBackGround->Blt(NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);

				if (lpDDSBuffer->m_pSurf->GetSurfaceType() == SoftSurface::SURFACE_PALETTE_8BIT)
				{
				//this one too
					delete lpDDSBuffer;
					lpDDSBuffer = InitOffscreenSurface(C_DINK_SCREENSIZE_X, C_DINK_SCREENSIZE_Y, IDirectDrawSurface::MODE_SHADOW_GL, true);
					lpDDSBuffer->m_pGLSurf->SetUsesAlpha(true);

					//however, we need to force a rebuild of the background now
					g_forceBuildBackgroundFromScratch = true;

				}
			}
		}

	
			ddrval = lpdest->Blt(&dstRect, g_pSpriteSurface[getpic(h)],
				&srcRect  , DDBLT_KEYSRC ,&ddbltfx );

			/*
				LogMsg("MainSpriteDraw(): Could not draw sprite %d, pic %d.",h,getpic(h));
				LogMsg("Box_crap: %d %d %d %d, Box_real: %d %d %d %d",box_crap.left,box_crap.top,
					box_crap.right, box_crap.bottom,box_real.left,box_real.top,
					box_real.right, box_real.bottom);
				if (g_sprite[h].pseq != 0) check_seq_status(g_sprite[h].pseq);
				*/
	}
}

void changedir( int dir1, int k,int base)
{

	if (k < 0 || k >= C_MAX_SPRITES_AT_ONCE)
	{
		LogMsg("Illegal change dir (to %d) command on sprite %d which is invalid", base + dir1, k);
		return;
	}

	int hspeed;
	int speed_hold = g_sprite[k].speed;
	if (k > 1) if (g_sprite[k].brain != 9) if (g_sprite[k].brain != 10)
	{   

		//if (mbase_timing > 20) mbase_timing = 20;

		//   Msg(",base_timing is %d", base_timing);
		hspeed = g_sprite[k].speed * (g_dglos.base_timing / 4);
		if (hspeed > 49)
		{
#ifdef _DEBUG
			LogMsg("Speed was %d", hspeed);
#endif
			g_sprite[k].speed = 49;
		} else
			g_sprite[k].speed = hspeed;
	}
	int old_seq = g_sprite[k].seq;
	g_sprite[k].dir = dir1;

	if (dir1 == 1)
	{
		g_sprite[k].mx = (0 - g_sprite[k].speed ) + (g_sprite[k].speed / 3);
		g_sprite[k].my = g_sprite[k].speed - (g_sprite[k].speed / 3);

		if (base != -1)
		{
			g_sprite[k].seq = base + 1;
			if (g_dglos.g_seq[g_sprite[k].seq].active == false)
			{
				g_sprite[k].seq = base + 9;
			}
		}

		if (old_seq != g_sprite[k].seq)
		{
			g_sprite[k].frame = 0;
			g_sprite[k].delay = 0;
		}
	}

	if (dir1 == 2)
	{
		g_sprite[k].mx = 0;
		g_sprite[k].my = g_sprite[k].speed;
		if (base != -1)
			g_sprite[k].seq = base + 2;

		if (g_dglos.g_seq[g_sprite[k].seq].active == false) if (g_dglos.g_seq[base+3].active) g_sprite[k].seq = base +3;
		if (g_dglos.g_seq[g_sprite[k].seq].active == false) if (g_dglos.g_seq[base+1].active) g_sprite[k].seq = base +1;      

		if (old_seq != g_sprite[k].seq)
		{
			g_sprite[k].frame = 0;
			g_sprite[k].delay = 0;
		}
	}

	if (dir1 == 3)
	{
		g_sprite[k].mx = g_sprite[k].speed - (g_sprite[k].speed / 3);
		g_sprite[k].my = g_sprite[k].speed - (g_sprite[k].speed / 3);
		if (base != -1)
		{
			g_sprite[k].seq = base + 3;
			if (g_dglos.g_seq[g_sprite[k].seq].active == false)
				g_sprite[k].seq = base + 7;

		}

		if (old_seq != g_sprite[k].seq)
		{
			g_sprite[k].frame = 0;
			g_sprite[k].delay = 0;
		}
	}

	if (dir1 == 4)
	{

		//Msg("Changing %d to four..",k);
		g_sprite[k].mx = (0 - g_sprite[k].speed);
		g_sprite[k].my = 0;
		if (base != -1)
			g_sprite[k].seq = base + 4;
		if (g_dglos.g_seq[g_sprite[k].seq].active == false) if (g_dglos.g_seq[base+7].active) g_sprite[k].seq = base +7;
		if (g_dglos.g_seq[g_sprite[k].seq].active == false) if (g_dglos.g_seq[base+1].active) g_sprite[k].seq = base +1;
	}

	if (dir1 == 6)
	{
		g_sprite[k].mx = g_sprite[k].speed;
		g_sprite[k].my = 0;
		if (base != -1)
			g_sprite[k].seq = base + 6;

		if (g_dglos.g_seq[g_sprite[k].seq].active == false) if (g_dglos.g_seq[base+3].active) g_sprite[k].seq = base +3;
		if (g_dglos.g_seq[g_sprite[k].seq].active == false) if (g_dglos.g_seq[base+9].active) g_sprite[k].seq = base +9;      

	}

	if (dir1 == 7)
	{
		g_sprite[k].mx = (0 - g_sprite[k].speed) + (g_sprite[k].speed / 3);
		g_sprite[k].my = (0 - g_sprite[k].speed)+ (g_sprite[k].speed / 3);
		if (base != -1)
		{
			g_sprite[k].seq = base + 7;


			if (g_dglos.g_seq[g_sprite[k].seq].active == false)
			{

				g_sprite[k].seq = base + 3;
			}
		}

	}
	if (dir1 == 8)
	{
		g_sprite[k].mx = 0;
		g_sprite[k].my = (0 - g_sprite[k].speed);
		if (base != -1)
			g_sprite[k].seq = base + 8;

		if (g_dglos.g_seq[g_sprite[k].seq].active == false) if (g_dglos.g_seq[base+7].active) g_sprite[k].seq = base +7;
		if (g_dglos.g_seq[g_sprite[k].seq].active == false) if (g_dglos.g_seq[base+9].active) g_sprite[k].seq = base +9;      

	}


	if (dir1 == 9)
	{
		g_sprite[k].mx = g_sprite[k].speed- (g_sprite[k].speed / 3);
		g_sprite[k].my = (0 - g_sprite[k].speed)+ (g_sprite[k].speed / 3);
		if (base != -1)
		{
			g_sprite[k].seq = base + 9;
			if (g_dglos.g_seq[g_sprite[k].seq].active == false)
			{
				g_sprite[k].seq = base + 1;
			}
		}
	}

	if (old_seq != g_sprite[k].seq)
	{
		g_sprite[k].frame = 0;
		g_sprite[k].delay = 0;
	}


	if (g_dglos.g_seq[g_sprite[k].seq].active == false)
	{
		//spr[k].mx = 0;
		//spr[k].my = 0;
		g_sprite[k].seq = old_seq;
	}

	//Msg("Leaving with %d..", spr[k].dir);

	//Msg("Changedir: Tried to switch sprite %d to dir %d",k,dir1);

	g_sprite[k].speed = speed_hold;

}

void update_play_changes( void )
{

	for (int j = 1; j < 100; j++)
	{
		if (g_dglos.g_smallMap.sprite[j].active)
			if (g_dglos.g_playerInfo.spmap[*pmap].type[j] != 0)
			{
				//lets make some changes, player has extra info
				if (g_dglos.g_playerInfo.spmap[*pmap].type[j] == 1) 
				{
					g_dglos.g_smallMap.sprite[j].active = 0;
				}       

				if (g_dglos.g_playerInfo.spmap[*pmap].type[j] == 2) 
				{
					g_dglos.g_smallMap.sprite[j].type = 1;
					g_dglos.g_smallMap.sprite[j].hard = 1;
				}
				if (g_dglos.g_playerInfo.spmap[*pmap].type[j] == 3)
				{

					//      Msg("Changing sprite %d", j);
					g_dglos.g_smallMap.sprite[j].type = 0;
					g_dglos.g_smallMap.sprite[j].hard = 1;

				}

				if (g_dglos.g_playerInfo.spmap[*pmap].type[j] == 4) 
				{
					g_dglos.g_smallMap.sprite[j].type = 1;
					g_dglos.g_smallMap.sprite[j].hard = 0;
				}

				if (g_dglos.g_playerInfo.spmap[*pmap].type[j] == 5) 
				{
					g_dglos.g_smallMap.sprite[j].type = 0;
					g_dglos.g_smallMap.sprite[j].hard = 0;
				}

				if (g_dglos.g_playerInfo.spmap[*pmap].type[j] == 6) 
				{
					g_dglos.g_smallMap.sprite[j].active = 0;

				}       
				if (g_dglos.g_playerInfo.spmap[*pmap].type[j] == 7) 
				{
					g_dglos.g_smallMap.sprite[j].active = 0;

				}       
				if (g_dglos.g_playerInfo.spmap[*pmap].type[j] == 8) 
				{
					g_dglos.g_smallMap.sprite[j].active = 0;

				}       

				g_dglos.g_smallMap.sprite[j].seq = g_dglos.g_playerInfo.spmap[*pmap].seq[j];
				g_dglos.g_smallMap.sprite[j].frame = g_dglos.g_playerInfo.spmap[*pmap].frame[j];
				strcpy(g_dglos.g_smallMap.sprite[j].script, "");
			}
	}
}

void update_status_all(void)
{
#ifdef _DEBUG
	//LogMsg("Updating status... %d", g_dglos.g_dinkTick);
#endif

	bool drawexp = false;
	int next = next_raise();
	int script;
	if (next != g_dglos.g_guiRaise)
	{
		g_dglos.g_guiRaise += next / 40;
		if (g_dglos.g_guiRaise > next) g_dglos.g_guiRaise = next;
		//make noise here 
		drawexp = true;
		SoundPlayEffect( 13,15050, 0,0 ,0);
	}

	if (*pexper != g_dglos.g_guiExp)
	{
		if ( ( g_dglos.g_talkInfo.active == false && g_itemScreenActive == false && g_sprite[1].freeze == 0 ) || g_dglos.g_guiExp + 10 < g_dglos.g_guiRaise )
		{
			//update screen experience
			g_dglos.g_guiExp += 10;
			//make noise here

			if (g_dglos.g_guiExp > *pexper) g_dglos.g_guiExp = *pexper;
			drawexp = true;
			SoundPlayEffect( 13,29050, 0,0 ,0); 

			if (g_dglos.g_guiExp >= g_dglos.g_guiRaise)
			{

				*pexper -= next;
				g_dglos.g_guiExp = 0;

				script = load_script("lraise", 1, false);
				if (locate(script, "raise")) run_script(script);
			}
		}
	}

	if (drawexp)
	{
		draw_exp(false);
	}

	if ( (g_dglos.g_guiLifeMax != *plifemax) || (g_dglos.g_guiLife != *plife) )
	{
		if (g_dglos.g_guiLifeMax < *plifemax) g_dglos.g_guiLifeMax++;
		if (g_dglos.g_guiLifeMax > *plifemax) g_dglos.g_guiLifeMax--;
		if (g_dglos.g_guiLife > *plife) g_dglos.g_guiLife--;
		if (g_dglos.g_guiLife < *plife) g_dglos.g_guiLife++;
		if (g_dglos.g_guiLife > *plife) g_dglos.g_guiLife--;
		if (g_dglos.g_guiLife < *plife) g_dglos.g_guiLife++;
		//draw_bar(flifemax, 190);
		//draw_bar(flife, 451);
	}

	if ( g_dglos.g_guiStrength != *pstrength)
	{
		if (g_dglos.g_guiStrength < *pstrength) g_dglos.g_guiStrength++;
		if (g_dglos.g_guiStrength > *pstrength) g_dglos.g_guiStrength--;
		SoundPlayEffect( 22,22050, 0,0 ,0);
		draw_strength(false);
	}

	if ( g_dglos.g_guiDefense != *pdefense)
	{
		if (g_dglos.g_guiDefense < *pdefense) g_dglos.g_guiDefense++;
		if (g_dglos.g_guiDefense > *pdefense) g_dglos.g_guiDefense--;
		SoundPlayEffect( 22,22050, 0,0 ,0);
		draw_defense(false);
	}
	if ( g_dglos.g_guiMagic != *pmagic)
	{
		if (g_dglos.g_guiMagic < *pmagic) g_dglos.g_guiMagic++;
		if (g_dglos.g_guiMagic > *pmagic) g_dglos.g_guiMagic--;
		SoundPlayEffect( 22,22050, 0,0 ,0);
		draw_magic(false);
	}

	if (g_dglos.g_guiGold != *pgold)
	{
		if (g_dglos.g_guiGold < *pgold)
		{
			g_dglos.g_guiGold += 20;
			if (g_dglos.g_guiGold > *pgold) g_dglos.g_guiGold = *pgold;
		}

		if (g_dglos.g_guiGold > *pgold)
		{
			g_dglos.g_guiGold -= 20;
			if (g_dglos.g_guiGold < *pgold) g_dglos.g_guiGold = *pgold;
		}
		SoundPlayEffect( 14,22050, 0,0 ,0);
		draw_gold(false);
	}

	if (*pmagic_level < *pmagic_cost) 
	{
		if (g_itemScreenActive == false)
			*pmagic_level += *pmagic;
		if (*pmagic_level > *pmagic_cost) *pmagic_level = *pmagic_cost;
	}
	if (*pmagic_cost > 0) if (*pmagic_level > 0)                        
	{
		double mnumd = *pmagic_level;
		mnumd *= 100;
		mnumd /= *pmagic_cost;
		int mnum = static_cast<int>(mnumd);
		//int mnum = *pmagic_level / (*pmagic_cost / 100);
		if (mnum != g_dglos.g_guiLastMagicDraw)
		{
			draw_mlevel(mnum, false);
			//draw_status_all();
			g_dglos.g_guiLastMagicDraw = mnum;
		}
	}

	g_sprite[1].strength = g_dglos.g_guiStrength;
	g_sprite[1].defense = g_dglos.g_guiDefense;

	if (g_dglos.g_guiLife < 1)
	{
		script = load_script("dinfo", 1000, false);
		if (locate(script, "die")) run_script(script);
	}
}

void place_sprites_game(bool bBackgroundOnly )
{
	int sprite;

	bool bs[C_MAX_SPRITES_AT_ONCE];
	int rank[C_MAX_SPRITES_AT_ONCE];
	int highest_sprite;

	update_play_changes();

	memset(&bs,0,sizeof(bs));
	memset(&rank,0,sizeof(rank));
	int hs;

	for (int r1 = 1; r1 < 100; r1++)
	{
		highest_sprite = 20000; //more than it could ever be
		rank[r1] = 0;

		for (int h1 = 1; h1 < 100;  h1++)
		{
			if (bs[h1] == false)
			{
				if (g_dglos.g_smallMap.sprite[h1].active && (!bBackgroundOnly  ||  g_dglos.g_smallMap.sprite[h1].type == 0) )
				{
					if (g_dglos.g_smallMap.sprite[h1].que != 0) hs = g_dglos.g_smallMap.sprite[h1].que; else hs = g_dglos.g_smallMap.sprite[h1].y;
					if ( hs < highest_sprite )
					{
						highest_sprite =hs;
						rank[r1] = h1;
					}
				}

			}
		}
		if (rank[r1] != 0)  
			bs[rank[r1]] = true;
	}

	int j;
	bool bScaledBackgroundSpritesRequired = false; //if true, we do a different, more memory intensive method for the rest of the sprites
	
	for (int oo =1; rank[oo] > 0; oo++)
	{
		//Msg("Ok, rank[%d] is %d.",oo,rank[oo]);       
		j = rank[oo];

#ifdef _DEBUG
		if (g_dglos.g_smallMap.sprite[j].seq == 66)
		{
			//LogMsg("Garden");
			//bScaledBackgroundSpritesRequired = true;
			//continue;
		}

		if (g_dglos.g_smallMap.sprite[j].vision != 0)
		{
			LogMsg("Found sprite %d with vision %d",
				j, g_dglos.g_smallMap.sprite[j].vision);
		}
#endif


		if (g_dglos.g_smallMap.sprite[j].active == true) if ( ( g_dglos.g_smallMap.sprite[j].vision == 0) || (g_dglos.g_smallMap.sprite[j].vision == *pvision))
		{
			check_seq_status(g_dglos.g_smallMap.sprite[j].seq, g_dglos.g_smallMap.sprite[j].frame);

			//we have instructions to make a sprite
			if (  (g_dglos.g_smallMap.sprite[j].type == 0)  || (g_dglos.g_smallMap.sprite[j].type == 2) )

			{


				//make it part of the background (much faster)
				sprite = add_sprite_dumb(g_dglos.g_smallMap.sprite[j].x,g_dglos.g_smallMap.sprite[j].y,0,
					g_dglos.g_smallMap.sprite[j].seq,g_dglos.g_smallMap.sprite[j].frame,
					g_dglos.g_smallMap.sprite[j].size);
				

				
				//("Background sprite %d has hard of %d..", j, pam.sprite[j].hard);
				g_sprite[sprite].hard = g_dglos.g_smallMap.sprite[j].hard;
				g_sprite[sprite].sp_index = j;
				g_sprite[sprite].alt =  g_dglos.g_smallMap.sprite[j].alt;

				check_sprite_status_full(sprite);
				if (g_dglos.g_smallMap.sprite[j].type == 0)
				{
						if (g_dglos.g_smallMap.sprite[j].size != 0 && g_dglos.g_smallMap.sprite[j].size != 100)
						{
							//it requires scaling, need to do things differently as our low mem fast custom blits won't work with this
							bScaledBackgroundSpritesRequired = true;
						}
					
					if (bScaledBackgroundSpritesRequired)
					{
						g_dglo.m_bgSpriteMan.Add(sprite);

					} else
					{
						draw_sprite_game(lpDDSBackGround,sprite);

					}
				}


				if (g_sprite[sprite].hard == 0)
				{
					add_hardness(sprite,100+j);
				}
				

				g_sprite[sprite].active = false;
			}

			if (g_dglos.g_smallMap.sprite[j].type == 1)
			{
				//make it a living sprite

				sprite = add_sprite_dumb(g_dglos.g_smallMap.sprite[j].x,g_dglos.g_smallMap.sprite[j].y,0,
					g_dglos.g_smallMap.sprite[j].seq,g_dglos.g_smallMap.sprite[j].frame,
					g_dglos.g_smallMap.sprite[j].size);

				g_sprite[sprite].hard = g_dglos.g_smallMap.sprite[j].hard;

				//assign addition parms to the new sprite
				g_sprite[sprite].sp_index = j;

				g_sprite[sprite].brain = g_dglos.g_smallMap.sprite[j].brain;
				g_sprite[sprite].speed = g_dglos.g_smallMap.sprite[j].speed;
				g_sprite[sprite].base_walk = g_dglos.g_smallMap.sprite[j].base_walk;
				g_sprite[sprite].base_idle = g_dglos.g_smallMap.sprite[j].base_idle;
				g_sprite[sprite].base_attack = g_dglos.g_smallMap.sprite[j].base_attack;
				g_sprite[sprite].base_hit = g_dglos.g_smallMap.sprite[j].base_hit;
				g_sprite[sprite].hard = g_dglos.g_smallMap.sprite[j].hard;
				g_sprite[sprite].timer = g_dglos.g_smallMap.sprite[j].timer;
				g_sprite[sprite].que = g_dglos.g_smallMap.sprite[j].que;
				g_sprite[sprite].alt = g_dglos.g_smallMap.sprite[j].alt;
				g_sprite[sprite].base_die = g_dglos.g_smallMap.sprite[j].base_die;
				g_sprite[sprite].strength = g_dglos.g_smallMap.sprite[j].strength;
				g_sprite[sprite].defense = g_dglos.g_smallMap.sprite[j].defense;
				g_sprite[sprite].gold = g_dglos.g_smallMap.sprite[j].gold;
				g_sprite[sprite].exp = g_dglos.g_smallMap.sprite[j].exp;
				g_sprite[sprite].nohit = g_dglos.g_smallMap.sprite[j].nohit;                
				g_sprite[sprite].touch_damage = g_dglos.g_smallMap.sprite[j].touch_damage;              
				g_sprite[sprite].hitpoints = g_dglos.g_smallMap.sprite[j].hitpoints;
				g_sprite[sprite].sound = g_dglos.g_smallMap.sprite[j].sound;
				check_sprite_status_full(sprite);   
			
#ifdef _DEBUG
				if (j == 60)
				{

				//	LogMsg("Drawing it");
				}
#endif
				
				if (g_dglos.g_smallMap.sprite[j].prop == 0) if (g_sprite[sprite].sound != 0)
				{
					//make looping sound
#ifdef _DEBUG
					LogMsg("making sound with sprite %d..", sprite);
#endif
					SoundPlayEffect( g_sprite[sprite].sound,22050, 0,sprite, 1);
				}   

				if (g_sprite[sprite].hard == 0)
				{
					add_hardness(sprite,100+j);
				}

				//does it need a script loaded?

				if (strlen(g_dglos.g_smallMap.sprite[j].script) > 1)
				{
#ifdef _DEBUG
					//LogMsg("Sprite %d is requesting that script %s is loaded when the map is drawn, vision is %d", j, g_dglos.g_smallMap.sprite[j].script, *pvision);
#endif
					g_sprite[sprite].script = load_script(g_dglos.g_smallMap.sprite[j].script, sprite, true);
				}

			}
			//Msg("I just made sprite %d because rank[%d] told me to..",sprite,j);
		}
	}

#ifdef _DEBUG

	LogMsg("Using %d background sprites in realdraw mode.", g_dglo.m_bgSpriteMan.GetCount());
#endif
}

bool kill_last_sprite(void)
{
	int found;
	found = 0;
	bool nosetlast = false;
	for (int k=1; k < C_MAX_SPRITES_AT_ONCE; k++ )

		if (g_sprite[k].active) 
		{

			if (g_sprite[k].live)
			{
				nosetlast = true;
				goto crazy;

			}

			found = k;
crazy:;

		}
		if (found > 1) 
		{g_sprite[found].active = false;
		if (nosetlast == false)
			g_dglos.last_sprite_created = found -1;
		return(true);
		}

		//we didn't kill any sprites, only 1 remains
		return(false);
}


void SetBitmapCopy(char *pFileName)
{

	g_dglos.status_might_not_require_update = 1;
	strncpy(g_dglos.copy_bmp_to_screen, pFileName, C_SHOWN_BITMAP_SIZE);

}

void ClearBitmapCopy()
{
	g_dglos.status_might_not_require_update = 0;
	g_dglos.copy_bmp_to_screen[0] = 0;
}

bool IsBitmapCopySet()
{
	return g_dglos.copy_bmp_to_screen[0] != 0;
}



void CopyBitmapToBackBuffer (char *pName)
{
	
	int ddrval;

	string fName = pName;
	StringReplace("\\", "/", fName);

	fName = GetFileLocationString(fName);

	if (!FileExists(fName)) 
	{
		LogMsg("Error: Can't find bitmap at %s.",fName.c_str());
		return;
	}
	
	if (lpDDSBuffer && lpDDSBuffer->m_pSurf && lpDDSBuffer->m_pSurf->GetSurfaceType() == SoftSurface::SURFACE_RGBA || lpDDSBuffer->m_pSurf->GetSurfaceType() == SoftSurface::SURFACE_RGB)
	{
		LogMsg("Warning, losing high color back buffer");
		//assert(0);
	}
	SAFE_DELETE(lpDDSBuffer);
	assert(!lpDDSBuffer);
	lpDDSBuffer = LoadBitmapIntoSurface(fName.c_str(), TRANSPARENT_NONE);

	rtRect32 rcRect(0,0,C_DINK_SCREENSIZE_X, C_DINK_SCREENSIZE_Y);

	ddrval = lpDDSBack->BltFast( 0, 0, lpDDSBuffer,
		&rcRect, DDBLTFAST_NOCOLORKEY);

}

void show_bmp( char *pName, int showdot, int reserved, int script)
{
	CopyBitmapToBackBuffer(pName);

	g_dglos.g_bShowingBitmap.active = true;
	g_dglos.g_bShowingBitmap.showdot = showdot;
	g_dglos.g_bShowingBitmap.script = script;
	strncpy(g_dglos.g_lastBitmapShown, pName, C_SHOWN_BITMAP_SIZE-1);
	g_abort_this_flip = true;

}


void copy_bmp( char *pName)
{
	int ddrval;

	string fName = pName;
	StringReplace("\\", "/", fName);

	fName = GetFileLocationString(fName);

	if (!FileExists(fName)) 
	{
		LogMsg("Error: Can't find bitmap at %s.",fName.c_str());
		return;
	}
	
	if (lpDDSBuffer && lpDDSBuffer->m_pSurf && lpDDSBuffer->m_pSurf->GetSurfaceType() == SoftSurface::SURFACE_RGBA || lpDDSBuffer->m_pSurf->GetSurfaceType() == SoftSurface::SURFACE_RGB)
	{
		//LogMsg("Warning, losing high color back buffer");
		//assert(0);
	}

	SAFE_DELETE(lpDDSBuffer);
	assert(!lpDDSBuffer);
	lpDDSBuffer = LoadBitmapIntoSurface(fName.c_str(), TRANSPARENT_NONE);

	
		if (lpDDSBackGround->m_pSurf->GetSurfaceType() == SoftSurface::SURFACE_PALETTE_8BIT && (lpDDSBuffer->m_pSurf->GetSurfaceType() == SoftSurface::SURFACE_RGBA || lpDDSBuffer->m_pSurf->GetSurfaceType() == SoftSurface::SURFACE_RGB))
		{
			//LogMsg("Detected high color tilescreen bmps.  Converting backbuffers to 32 bit on the fly.");

			//switch it
			delete lpDDSBackGround;
			lpDDSBackGround = InitOffscreenSurface(C_DINK_SCREENSIZE_X, C_DINK_SCREENSIZE_Y, IDirectDrawSurface::MODE_SHADOW_GL, true);


			DDBLTFX     ddbltfx;
			ddbltfx.dwFillColor = g_dglos.last_fill_screen_palette_color;
			lpDDSBackGround->Blt(NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);

			//bRequireRebuildOut = true;

		}


	g_abort_this_flip = true;

	rtRect32 rcRect(0,0,C_DINK_SCREENSIZE_X,  C_DINK_SCREENSIZE_Y);
	
	ddrval = lpDDSBack->BltFast( 0, 0, lpDDSBuffer,
		&rcRect, DDBLTFAST_NOCOLORKEY);

	//assert(!"Clear this?");
	ddrval = lpDDSBackGround->BltFast( 0, 0, lpDDSBuffer,
		&rcRect, DDBLTFAST_NOCOLORKEY);
	
	/*
	g_dglos.g_bShowingBitmap.active = true;
	g_dglos.g_bShowingBitmap.showdot = showdot;
	g_dglos.g_bShowingBitmap.script = script;
	strncpy(g_dglos.g_lastBitmapShown, pNam, C_SHOWN_BITMAP_SIZE - 1);
	*/
	
}


bool playing( int sound)
{
	return false;
}

int get_pan(int h)
{

	if (h < 0 || h > C_MAX_SPRITES_AT_ONCE)
	{
		LogMsg("ignoring get_pan (probably initiated by sound play command) as it's connected to an invalid sprite #");
		return 0;
	}

	int pan = 0;
	int x1 = 320;

	//uncomment to allow math to be done from Dink's current location
	//x1 = spr[1].x;

	if (g_sprite[h].active)
	{
		if (g_sprite[h].x > x1) pan += (g_sprite[h].x - x1) * 6;
		if (x1 > g_sprite[h].x) pan -= (x1 - g_sprite[h].x) * 6;
	}

	if (pan > 10000) pan = 10000;
	if (pan < -10000) pan = -10000;

	return(pan);
}



int get_vol(int h)
{
	if (h < 0 || h > C_MAX_SPRITES_AT_ONCE)
	{
		LogMsg("ignoring get_vol (probably initiated by sound play command) as it's connected to an invalid sprite #");
		return 0;
	}

	int pan = 0;
	int pan2 = 0;

	if (g_sprite[h].active)
	{
		if (g_sprite[h].x > g_sprite[1].x) pan -= (g_sprite[h].x - g_sprite[1].x) * 4;
		if (g_sprite[1].x > g_sprite[h].x) pan -= (g_sprite[1].x - g_sprite[h].x) * 4;
		if (g_sprite[h].y > g_sprite[1].y) pan2 -= (g_sprite[h].y - g_sprite[1].y) * 4;
		if (g_sprite[1].y > g_sprite[h].y) pan2 -= (g_sprite[1].y - g_sprite[h].y) * 4;

		//Msg("pan %d, pan2 %d", pan, pan2);

		if (pan2 < pan) pan = pan2;
	}

	if (pan > -100) pan = 0;
	if (pan < -10000) pan = -10000;
	return(pan);
}

void kill_all_sounds()
{
	for (int i=1; i <= num_soundbanks; i++)
	{       
		soundbank[i].Stop();          
		//Msg("REPEAT Sound %d playing.. owner is %d.", i,soundinfo[i].owner);
		soundinfo[i].owner = 0;
		soundinfo[i].repeat = 0;
		soundinfo[i].freq = 0;
		soundinfo[i].survive = 0;

	}
}

void kill_repeat_sounds( void )
{
	if (!sound_on) return;

	for (int i=1; i <= num_soundbanks; i++)
	{       
		if (soundinfo[i].repeat) if (soundinfo[i].owner == 0) if (soundinfo[i].survive == 0)
		{

			soundbank[i].Stop();          
			//Msg("REPEAT Sound %d playing.. owner is %d.", i,soundinfo[i].owner);
			soundinfo[i].owner = 0;
			soundinfo[i].repeat = 0;
		}
	}
}

void kill_repeat_sounds_all( void )
{
	if (!sound_on) return;

	for (int i=1; i <= num_soundbanks; i++)
	{       
		if (soundinfo[i].repeat) if (soundinfo[i].owner == 0)
		{
			soundbank[i].Stop();          
			//Msg("REPEAT Sound %d playing.. owner is %d.", i,soundinfo[i].owner);
			soundinfo[i].owner = 0;
			soundinfo[i].repeat = 0;
		}
	}
}

void update_sound(void)
{
	
	if (!sound_on) return;

	g_soundTimer = 0;

	if (! (g_soundTimer < GetBaseApp()->GetGameTick()))
	{
		//wait before updating
		return;
	}

	g_soundTimer = GetBaseApp()->GetGameTick()+120;
	
	for (int i=1; i <= num_soundbanks; i++)
	{                          
		if (soundinfo[i].repeat) if (soundinfo[i].owner != 0)
		{

			if ( ( g_sprite[soundinfo[i].owner].sound == 0) || ( soundinfo[i].owner == 0)

				|| (g_sprite[soundinfo[i].owner].active == false) || soundbank[i].m_audioID == 0)
			{
				soundbank[i].Stop();          
				//Msg("Killed bank %d", i); 
				//Msg("REPEAT Sound %d playing.. owner is %d.", i,soundinfo[i].owner);
				soundinfo[i].owner = 0;
				soundinfo[i].repeat = 0;
			} else
			{
				soundbank[i].SetPan(get_pan(soundinfo[i].owner));
				soundbank[i].SetVolume(get_vol(soundinfo[i].owner));

			}
		}

		if (soundbank[i].IsInUse())
		{
			{
				//Msg("Sound %d playing.. owner is %d.", i,soundinfo[i].owner);
				if (soundinfo[i].owner != 0)
				{
					if (g_sprite[soundinfo[i].owner].active == false)
					{
						//Msg("Killed bank %d", i); 

						soundbank[i].Stop();          

					} else
					{
						soundbank[i].SetPan(get_pan(soundinfo[i].owner));
						soundbank[i].SetVolume(get_vol(soundinfo[i].owner));
					}

				}
			}
		}
	}
	

}

int playbank( int sound, int min,int plus, int sound3d, bool repeat, int forceBank = 0 )
{
	
	if (g_soundInfo[sound].m_fileName.empty()) return 0;

if (GetEmulatedPlatformID() == PLATFORM_ID_ANDROID)
{
	if (repeat)
		return 0; //we don't support looping sounds because we can't figure out if they are playing or not, missing an IsPlaying()
}
	//Msg("Playing bank %d..", sound);
	int i;

	if (forceBank != 0)
	{
		i = forceBank;
		goto madeit;
	}
	for (i=1; i <= num_soundbanks; i++)
	{

		if (!soundbank[i].IsInUse())
		{
			goto madeit;
		}
	}

	return(false);

madeit:    

soundbank[i].Reset();

StringReplace("\\", "/", g_soundInfo[sound].m_fileName);
string soundFilePathAndName = "sound/"+ToLowerCaseString(g_soundInfo[sound].m_fileName);
string fName;

//hack because android sucks
/*
if (  (GetEmulatedPlatformID() == PLATFORM_ID_ANDROID) || (GetEmulatedPlatformID() == PLATFORM_ID_WEBOS)
	|| (GetEmulatedPlatformID() == PLATFORM_ID_BBX))
	*/
if (1)
{
	//hack the sound value a bit

	if (sound == 10 && g_soundInfo[sound].m_fileName == "SWORD2.WAV" && min == 22050)
	{
		GetAudioManager()->Play("dink/sound/sword2_item.wav");
		return 0;
	}

	//first try with oggs
	
	string temp;
	
	if (GetEmulatedPlatformID() == PLATFORM_ID_HTML5)
	{
		temp = ModifyFileExtension(soundFilePathAndName, "mp3");

	}
	else
	{
		temp = ModifyFileExtension(soundFilePathAndName, "ogg");

	}
	
	temp = GetFileLocationString(temp);

	if (FileExists(temp))
	{
		fName = temp;
	} else
	{
	//	LogMsg("Can't find %s", GetFileLocationString(temp).c_str());
		fName = GetFileLocationString(soundFilePathAndName);
		
	}


} else
{
	fName = GetFileLocationString(soundFilePathAndName);
}



//before we load it, let's see how big it really is.
int fileSize = GetFileManager()->GetFileSize(fName, false);

if (fileSize < 1)
{
	LogMsg("Unable to locate sound %s", fName.c_str());
	return 0;
}


bool bForceStreaming = false;

if (fileSize > 1024*1024*2)
{
	//too big, let's stream it
	bForceStreaming = true;
}

soundbank[i].m_audioID = GetAudioManager()->Play(fName, repeat != NULL, false, false, bForceStreaming);
#ifdef _DEBUG
	//LogMsg("Got audioid %d when playing %s",soundbank[i].m_audioID, fName.c_str() );
#endif
if (soundbank[i].m_audioID == 0) return 0;
soundbank[i].m_soundIDThatPlayedUs = sound;

int freq = RandomRange(min, min+plus);
		
if (GetEmulatedPlatformID() != PLATFORM_ID_ANDROID)
	GetAudioManager()->SetFrequency(soundbank[i].m_audioID, freq);

if (sound3d > 0) 
{
	soundbank[i].SetPan(get_pan(sound3d));
	soundbank[i].SetVolume(get_vol(sound3d));
}

soundinfo[i].owner = sound3d;
soundinfo[i].repeat = repeat;
soundinfo[i].survive = 0;
soundinfo[i].vol = 0;
soundinfo[i].freq = freq;

return i;



} /* SoundPlayEffect */


int SoundPlayEffect( int sound, int min,int plus , int sound3d, bool repeat)
{
	return playbank(sound, min, plus, sound3d, repeat);
} /* SoundPlayEffect */


int hurt_thing(int h, int damage, int special)
{
	//lets hurt this sprite but good
	if (damage < 1) return(0);
	int num = damage - g_sprite[h].defense;

	//  Msg("num is %d.. defense was %d.of sprite %d", num, spr[h].defense, h);
	if (num < 1) num = 0;

	if (num == 0) 
	{
		if ((rand() % 2)+1 == 1) num = 1;
	}

	g_sprite[h].damage += num;
	return(num);    
	//draw blood here
}

void random_blood(int mx, int my, int h)
{
	//if ((rand() % 2) == 1) myseq = 188; else myseq = 187;
	//redink1 - customizable blood depending on what sprite we hit!!
	int myseq;
	int randy;

	if (g_sprite[h].bloodseq > 0 && g_sprite[h].bloodnum > 0)
	{
		myseq = g_sprite[h].bloodseq;
		randy = g_sprite[h].bloodnum;
	}
	else
	{
		myseq = 187;
		randy = 3;
	}

	myseq += (rand() % randy);

	int crap2 = add_sprite(mx,my,5,myseq,1);
	g_sprite[crap2].speed = 0;
	g_sprite[crap2].base_walk = -1;
	g_sprite[crap2].nohit = 1;
	g_sprite[crap2].seq = myseq;
	if (h > 0) g_sprite[crap2].que = g_sprite[h].y+1;
}

bool SoundStopEffect( int sound )
{
	/*
	if(!g_soundInfo[sound].sound )
	{
		return false;
	}

	g_soundInfo[sound].sound->Stop();

	return true;
	*/

	return true;
} /* SoundStopEffect */



bool InitSound()
{

	for (int i=1; i < num_soundbanks; i++)
	{
		soundbank[i].Reset();
	}
	return true;
} /* InitSound */

void BuildScreenBackground( bool bFullRebuild, bool bBuildImageFromScratch )
{
	rtRect32                rcRect;
	int pa, cool;   
//	*pvision = 0; //this was bad because save stats call this.  Moved to where new maps are loaded

	if (g_forceBuildBackgroundFromScratch)
	{
		bBuildImageFromScratch = true;
	
	}

	if (bFullRebuild)
	{
		bBuildImageFromScratch = true;
#ifdef _DEBUG
		LogMsg("Doing full rebuild of screen background...");
#endif
		while (kill_last_sprite()); 
		kill_repeat_sounds();
		kill_all_scripts();
		g_dglo.m_bgSpriteMan.Clear();

	}

	if (bBuildImageFromScratch)
	{

		if (lpDDSBackGround)
		{
			DDBLTFX     ddbltfx;
			ddbltfx.dwFillColor = g_dglos.last_fill_screen_palette_color;
			lpDDSBackGround->Blt(NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);
		}
	}

restart:
	
	if (g_dglos.g_gameMode > 2 || g_dglos.m_bRenderBackgroundOnLoad || bBuildImageFromScratch)
	{
		//we want to render tiles to the BG too

		int tileScreenID;

		for (int x = 0; x < 96; x++)
		{
			cool = g_dglos.g_smallMap.t[x].num / 128;
			pa = g_dglos.g_smallMap.t[x].num - (cool * 128);
			rcRect.left = (pa * 50 - (pa / 12) * 600);
			rcRect.top = (pa / 12) * 50;
			rcRect.right = rcRect.left + 50;
			rcRect.bottom = rcRect.top + 50;

			tileScreenID = cool + 1;

			bool bRequireRebuild;

			if (!LoadTileScreenIfNeeded(tileScreenID, bRequireRebuild))
			{
				continue;
			}

			if (bRequireRebuild) goto restart;
			g_tileScreens[tileScreenID]->UpdateLastUsedTime();


			lpDDSBackGround->BltFast((x * 50 - ((x / 12) * 600)) + g_gameAreaLeftOffset, (x / 12) * 50, g_tileScreens[tileScreenID],
				&rcRect, DDBLTFAST_NOCOLORKEY | DDBLTFAST_WAIT);
		}
	}

	g_forceBuildBackgroundFromScratch = false;

	if (bFullRebuild)
	{
		if (strlen(g_dglos.g_smallMap.script) > 1)
		{
			int ms = load_script(g_dglos.g_smallMap.script,0, true);

			if (ms > 0) 
			{
				locate(ms, "main");
				g_dglos.no_running_main = true;
				run_script(ms);
				g_dglos.no_running_main = false;
			}
		}
	}

	
	//lets add the sprites hardness to the real hardness, adding its own uniqueness to our collective.
	place_sprites_game(!bFullRebuild);

	if (!bFullRebuild) return; //don't mess with the game tick stuff or initscripts
	//if script for overall screen, run it

	//g_dglos.g_dinkTick = GetBaseApp()->GetGameTick();
	init_scripts();
}



void fill_back_sprites(void )
{
	int sprite;

	bool bs[C_MAX_SPRITES_AT_ONCE];
	int rank[C_MAX_SPRITES_AT_ONCE];
	int highest_sprite;

	memset(&bs,0,sizeof(bs));
	int hs;

	for (int r1 = 1; r1 < 100; r1++)
	{

		highest_sprite = 20000; //more than it could ever be
		rank[r1] = 0;

		for (int h1 = 1; h1 < 100;  h1++)
		{
			if (bs[h1] == false)
			{
				if (g_dglos.g_smallMap.sprite[h1].active) if (g_dglos.g_smallMap.sprite[h1].type != 1) if (g_dglos.g_smallMap.sprite[h1].hard == 0)
				{
					if (g_dglos.g_smallMap.sprite[h1].que != 0) hs = g_dglos.g_smallMap.sprite[h1].que; else hs = g_dglos.g_smallMap.sprite[h1].y;
					if ( hs < highest_sprite )
					{
						highest_sprite =hs;
						rank[r1] = h1;
					}
				}

			}
		}
		if (rank[r1] != 0)  
			bs[rank[r1]] = true;
	}

	int j;

	for (int oo =1; rank[oo] > 0; oo++)
	{
		//Msg("Ok, rank[%d] is %d.",oo,rank[oo]);       
		j = rank[oo];

		if (g_dglos.g_smallMap.sprite[j].active == true) if ( ( g_dglos.g_smallMap.sprite[j].vision == 0) || (g_dglos.g_smallMap.sprite[j].vision == *pvision))
		{
			{
				//make it part of the background (much faster)

				sprite = add_sprite_dumb(g_dglos.g_smallMap.sprite[j].x,g_dglos.g_smallMap.sprite[j].y,0,
					g_dglos.g_smallMap.sprite[j].seq,g_dglos.g_smallMap.sprite[j].frame,
					g_dglos.g_smallMap.sprite[j].size);
				g_sprite[sprite].hard = g_dglos.g_smallMap.sprite[j].hard;
				g_sprite[sprite].sp_index = j;
				g_sprite[sprite].alt = g_dglos.g_smallMap.sprite[j].alt;
				check_sprite_status_full(sprite);
				if (g_sprite[sprite].hard == 0)
				{
					/*if (pam.sprite[j].prop == 0)
					add_hardness(sprite, 1); else */ add_hardness(sprite,100+j);

				}
				g_sprite[sprite].active = false;
			}
		}
	}
}


void add_item(char name[10], int mseq, int mframe, bool magic)
{
	if (magic == false)
	{
		//add reg item

		for (int i = 1; i < C_DINK_MAX_ITEMS + 1; i ++)
		{
			if (g_dglos.g_playerInfo.g_itemData[i].active == false)
			{
				if (g_script_debug_mode)
					LogMsg("Weapon/item %s added to inventory.",name);
				g_dglos.g_playerInfo.g_itemData[i].seq = mseq;
				g_dglos.g_playerInfo.g_itemData[i].frame = mframe;
				strncpy(g_dglos.g_playerInfo.g_itemData[i].name, name, 10);
				g_dglos.g_playerInfo.g_itemData[i].active = true;
				//if (debug_mode)
				//	LogMsg("wep: Checking seq",name);

				check_seq_status(mseq, mframe);

			
				int crap1 = load_script(g_dglos.g_playerInfo.g_itemData[i].name, 1000, false);

			
				if (locate(crap1, "PICKUP"))
				{
					run_script(crap1);
				}

		
				return;
			}
		}

	} else
	{
		//add magic item
		for (int i = 1; i < 9; i ++)
		{
			if (g_dglos.g_playerInfo.g_MagicData[i].active == false)
			{
				if (g_script_debug_mode)
					LogMsg("Magic %s added to inventory.",name);
				g_dglos.g_playerInfo.g_MagicData[i].seq = mseq;
				g_dglos.g_playerInfo.g_MagicData[i].frame = mframe;
				strncpy(g_dglos.g_playerInfo.g_MagicData[i].name, name, 10);

				g_dglos.g_playerInfo.g_MagicData[i].active = true;
				//check_seq_status(mseq, mframe);
				int crap = load_script(g_dglos.g_playerInfo.g_MagicData[i].name, 1000, false);
				if (locate(crap, "PICKUP")) run_script(crap);

				return;
			}
		}
	}
}

void fill_screen(int num)
{

#ifdef _DEBUG
	LogMsg("Filling screen with %d", num);
#endif
	DDBLTFX     ddbltfx;
	ddbltfx.dwFillColor = num;
	lpDDSBackGround->Blt(NULL ,NULL,NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);

/*
	
	DDBLTFX     ddbltfx;
	ZeroMemory(&ddbltfx, sizeof(ddbltfx));
	ddbltfx.dwSize = sizeof( ddbltfx);

	//redink1 fix for correct fill_screen colors in truecolor mode
	if (truecolor)
	{
		lpDDPal->GetEntries(0,0,256,pe);
		ddbltfx.dwFillColor = pe[num].peBlue << wBPos | pe[num].peGreen << wGPos | pe[num].peRed << wRPos;
	}
	else
		ddbltfx.dwFillColor = num;
	crap = lpDDSTwo->Blt(NULL ,NULL,NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);
*/

}


void fill_hard_sprites(void )
{
	bool bs[C_MAX_SPRITES_AT_ONCE];
	int rank[C_MAX_SPRITES_AT_ONCE];
	int highest_sprite;
	int h;  
	memset(&bs,0,sizeof(bs));

	//Msg("filling sprite hardness...");
	int max_s = g_dglos.last_sprite_created;
	int height;

	for (int r1 = 1; r1 <= max_s; r1++)
	{
		highest_sprite = 22024; //more than it could ever be
		rank[r1] = 0;
		for (int h1 = 1; h1 < max_s+1; h1++)
		{
			if (g_sprite[h1].active)
			{ 
				if (bs[h1] == false)
				{
					//Msg( "Ok,  %d is %d", h1,(spr[h1].y + k[spr[h1].pic].yoffset) );
					if (g_sprite[h1].que != 0) height = g_sprite[h1].que; else height = g_sprite[h1].y;
					if ( height < highest_sprite )
					{
						highest_sprite = height;
						rank[r1] = h1;
					}
				}
			}
		}
		if (rank[r1] != 0)  
			bs[rank[r1]] = true;
	}

	for ( int j = 1; j <= max_s; j++)
	{
		h = rank[j];
		if (h > 0) 
			if (g_sprite[h].active)
			{
				//          Msg("proccesing sprite %d", h);
				if (g_sprite[h].sp_index != 0)
				{
					//Msg("has spindex of %d prop is %d",spr[h].sp_index,pam.sprite[spr[h].sp_index].prop);
					if (g_dglos.g_smallMap.sprite[g_sprite[h].sp_index].hard == 0)
					{
						add_hardness(h,100+g_sprite[h].sp_index);
						//Msg("added warp hardness for %d", spr[h].sp_index);
					}
				} else
				{
					if (g_sprite[h].hard == 0)
					{
						//Msg("adding a new sprite hardness %d (not from editor)", h);               
						add_hardness(h, 1);                     
					}

				}
			}
	}
}   

void LoadGame(int gameSlot)
{
	kill_all_scripts_for_real();    
	g_dglos.g_returnint = load_game(gameSlot);
	LogMsg("load completed. ");

	*pupdate_status = 1;
	draw_status_all();
}


int process_line (int script, char *pLineIn, bool doelse)
{
	char * h, *p;
	int i;
	char line[512];
	char ev[15][100];
	char temp[512];
	char first[2];
	int sprite = 0;
	ev[0][0] = 0;
	if (g_scriptInstance[script]->level < 1) g_scriptInstance[script]->level = 1;

	for (int kk =1; kk < 15; kk++) ev[kk][0] = 0;

	h = pLineIn;
	if (h[0] == 0) return(0);    
	if (  (h[0] == '/') && (h[1] == '/'))

	{
		//Msg("It was a comment!");
		goto bad;
	}

	
	for ( i=1; i <= 14; i++)
	{
		if (separate_string(h, i,' ',ev[i]) == false) goto pass;
	}

pass:
#ifdef _DEBUG
	//LogMsg("first line is %s (second is %s), third is %s", ev[1], ev[2], ev[3]);
#endif

	if (compare(ev[1], (char*)"VOID"))
	{

		if (g_scriptInstance[script]->proc_return != 0)
		{
			run_script(g_scriptInstance[script]->proc_return);
			kill_script(script);
		}

		//Msg("returning..");
		return(2);      
	}
	//replace("\n","",ev[1]);
	if (ev[1][strlen(ev[1]) -1] == ':') if (strlen(ev[2]) < 2) if (strncmp(ev[1],"say",3) != 0)
	{
		//  Msg("Found label %s..",ev[1]);   
		return(0); //its a label
	}

	if (ev[1][0] == '(')
	{
		//this procedure has been passed a conditional statement finder
		//what kind of conditional statement is it?
		p = h;              
		separate_string(h, 2,')',temp);
		//Msg("We found %s, woah!", temp);
		separate_string(h, 1,')',ev[1]);

		// Msg("Ok, turned h %s to  ev1 %s.",h,ev[1]);
		p = &p[strlen(ev[1])+1];  

		strip_beginning_spaces(p);
		//  Msg("does %s have a ( in front?", p);
		//Msg("We found %s, woah!", temp);

		//These are used for conditionals??
		if (strchr(temp, '=') != NULL) 
		{
			h = &h[1];
			strip_beginning_spaces(h);
			process_line(script, h, false);
			replace("==", "", temp);
			sprintf(line, "%d == %s", g_dglos.g_returnint, temp); 
			g_dglos.g_returnint = var_figure(line, script);                   
			strcpy(h, "\n");
			return(0);
		}

		if (strchr(temp, '>') != NULL) 
		{
			h = &h[1];
			strip_beginning_spaces(h);
			process_line(script, h, false);
			replace("==", "", temp);
			sprintf(line, "%d > %s", g_dglos.g_returnint, temp); 
			g_dglos.g_returnint = var_figure(line, script);                   
			strcpy(h, "\n");
			return(0);
		}

		if (strchr(temp, '<') != NULL) 
		{
			h = &h[1];
			strip_beginning_spaces(h);
			process_line(script, h, false);
			replace("==", "", temp);
			sprintf(line, "%d < %s", g_dglos.g_returnint, temp); 
			g_dglos.g_returnint = var_figure(line, script);                   
			strcpy(h, "\n");
			return(0);
		}
		
		if (strchr(temp, '<=') != NULL) 
		{
			h = &h[1];
			strip_beginning_spaces(h);
			process_line(script, h, false);
			replace("==", "", temp);
			sprintf(line, "%d <= %s", g_dglos.g_returnint, temp);
			g_dglos.g_returnint = var_figure(line, script);
			strcpy(h, "\n");
			return(0);
		}
		if (strchr(temp, '>=') != NULL) 
		{
			h = &h[1];
			strip_beginning_spaces(h);
			process_line(script, h, false);
			replace("==", "", temp);
			sprintf(line, "%d >= %s", g_dglos.g_returnint, temp);
			g_dglos.g_returnint = var_figure(line, script);
			strcpy(h, "\n");
			return(0);
		}
		

		if (strchr(temp, '!=') != NULL) 
		{
			h = &h[1];
			strip_beginning_spaces(h);
			process_line(script, h, false);
			replace("==", "", temp);
			sprintf(line, "%d != %s", g_dglos.g_returnint, temp); 
			g_dglos.g_returnint = var_figure(line, script);                   
			strcpy(h, "\n");
			return(0);
		}
		
		if (p[0] == ')')
		{
			//its a procedure in the if statement!!!  
			h = &h[1];
			p = &p[1];
			strcpy(line, p); 
			process_line(script, h, false);

			//8
			//LogMsg("Returned %d for the returnint", g_dglos.g_returnint);
			h = pLineIn; 
			strcpy(pLineIn, line); 

			//  Msg("Returing %s..", s);
			return(0);
		} else
		{
			h = &h[1];

			separate_string(h, 1,')',line);
			h = &h[strlen(line)+1];
			g_dglos.g_returnint = var_figure(line, script);         
			strcpy(pLineIn, h); 

			return(0);
		}

		strip_beginning_spaces(h);
		strip_beginning_spaces(ev[1]);
		pLineIn = h;       
	} else
	{
	}


	if (strchr(ev[1], '(') != NULL) 
	{
		//Msg("Has a (, lets change it");
		separate_string(h, 1,'(',ev[1]);
		//Msg("Ok, first is now %s",ev[1]);
	}

	first[0] = ev[1][0];
	first[1] = 0;
//	sprintf(first, "%c",ev[1][0]); 

	if (compare(first, "{"))    
	{
		g_scriptInstance[script]->level++;
		//Msg("Went up level, now at %d.", rinfo[script]->level);
		h = &h[1];
		if (g_scriptInstance[script]->skipnext) 
		{
			g_scriptInstance[script]->skipnext = false;
			g_scriptInstance[script]->onlevel = ( g_scriptInstance[script]->level - 1);
			//Msg("Skipping until level %d is met..", rinfo[script]->onlevel);
		}
		goto good;
	}

	if (compare(first, "}"))    
	{
		g_scriptInstance[script]->level--;
		//Msg("Went down a level, now at %d.", rinfo[script]->level);
		h = &h[1];

		if (g_scriptInstance[script]->onlevel > 0) if (g_scriptInstance[script]->level == g_scriptInstance[script]->onlevel)
		{
			strip_beginning_spaces(h);
			strcpy(pLineIn, h);
			return(4);
		}   
		goto good;
	} 

	if (g_scriptInstance[script]->level < 0)
	{
		g_scriptInstance[script]->level = 0;
	}

	if (compare(ev[1], (char*)"void"))
	{
		//     Msg("Next procedure starting, lets quit");
		strcpy(pLineIn, h);
		if (g_scriptInstance[script]->proc_return != 0)
		{
			run_script(g_scriptInstance[script]->proc_return);
			kill_script(script);
		}
		return(2);
	} 

	{ //used to be an if..


		if (g_scriptInstance[script]->onlevel > 0)
		{
			if (g_scriptInstance[script]->level > g_scriptInstance[script]->onlevel) return(0);

		}
		g_scriptInstance[script]->onlevel = 0;

		if (g_scriptInstance[script]->skipnext)
		{
			//sorry, can't do it, you were told to skip the next thing
			g_scriptInstance[script]->skipnext = false;
			strcpy(pLineIn, h);
			return(3);
		}

		//if (debug_mode) Msg("%s",s);


		if (compare(ev[1], (char*)"void"))
		{
			LogMsg("ERROR: Missing } in %s, offset %d.", g_scriptInstance[script]->name,g_scriptInstance[script]->current);
			strcpy(pLineIn, h);
			return(2);
		}

		if (compare(ev[1], (char*)"else"))
		{
			//Msg("Found else!");
			h = &h[strlen(ev[1])];

			if (!doelse)
			{
				//they shouldn't run the next thing
				g_scriptInstance[script]->skipnext = true;
				//Msg("No to else...");

			}
			strcpy(pLineIn, h);  
			return(1);

		}

		if (compare(ev[1], (char*)"unfreeze"))
		{

			h = &h[strlen(ev[1])];
			int32 p[20] = {1,0,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				//LogMsg("UnFreeze called for %d.", g_nlist[0]);
				
				if (g_nlist[0] < 0 || g_nlist[0] >= C_MAX_SPRITES_AT_ONCE)
				{
					LogMsg("Crash averted: Couldn't unfreeze sprite %d in script %d, it doesn't exist.", g_nlist[0], script);

				}
				else
				{
					if (g_sprite[g_nlist[0]].active) g_sprite[g_nlist[0]].freeze = 0; else
						LogMsg("Couldn't unfreeze sprite %d in script %d, it doesn't exist.", g_nlist[0], script);
				}

			}

			strcpy(pLineIn, h);  
			return(0);
		}

		if (compare(ev[1], (char*)"freeze"))
		{
			//Msg("Freeze called (%s)", h);     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,0,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{


				if (g_nlist[0] < 0 || g_nlist[0] >= C_MAX_SPRITES_AT_ONCE)
				{
					LogMsg("Crash averted: Couldn't unfreeze sprite %d in script %d, it doesn't exist.", g_nlist[0], script);

				}
				else
				{
					if (g_sprite[g_nlist[0]].active) g_sprite[g_nlist[0]].freeze = script; else
						LogMsg("Couldn't freeze sprite %d in script %d, it doesn't exist.", g_nlist[0], script);
				}

			}

			strcpy(pLineIn, h);  
			return(0);
		}


		//redink1 added so we can have return values and crap.
		if (compare(ev[1], (char*)"return"))
		{

			if (g_script_debug_mode) LogMsg("Found return; statement");

			h = &h[strlen(ev[1])];
			strip_beginning_spaces(h);
			process_line(script, h, false);

			if (g_scriptInstance[script]->proc_return != 0)
			{
				g_dglos.bKeepReturnInt = true;
				run_script(g_scriptInstance[script]->proc_return);
				kill_script(script);
			}

			return(2);
		}


		if (compare(ev[1], (char*)"if"))
		{

			h = &h[strlen(ev[1])];
			strip_beginning_spaces(h);
			//LogMsg("running if with string of %s", h);

			process_line(script, h, false);

			//Msg("Result is %d", returnint);    

			if (g_dglos.g_returnint != 0)
			{
				if (g_script_debug_mode) LogMsg("If returned true");


			} else
			{
				//don't do it!
				g_scriptInstance[script]->skipnext = true;
				if (g_script_debug_mode) LogMsg("If returned false, skipping next thing");
			}

			//DO STUFF HERE! 
			strcpy(pLineIn, h);  
			//g("continuing to run line %s..", h);
			return(5);

		}

		if (compare(ev[1], (char*)"sp_dir"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = change_sprite(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].dir);

				if (g_nlist[1] != -1) changedir(g_sprite[g_nlist[0]].dir, g_nlist[0], g_sprite[g_nlist[0]].base_walk);
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}


		if (compare(ev[2], "="))
		{
			h = &h[strlen(ev[1])];
			strip_beginning_spaces(h);
			h = &h[1];
			strip_beginning_spaces(h);
			var_equals(ev[1], ev[3], '=', script, h);
			strcpy(pLineIn, h);  
			return(0);
		}


		if (compare(ev[1], (char*)"set_callback_random"))
		{
			
			h = &h[strlen(ev[1])];
			int32 p[20] = {2,1,1,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				int cb = add_callback(slist[0],g_nlist[1],g_nlist[2],script); 
				//got all parms, let do it
				g_dglos.g_returnint = cb;
			}

			strcpy(pLineIn, h);  
			return(0);
		}

		// redink1 added
		if (compare(ev[1], (char*)"callback_kill"))
		{

			h = &h[strlen(ev[1])];
			int32 p[20] = {1,0,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				if ( g_nlist[0] >= 0 && g_nlist[0] <= 99)
				{
					g_dglos.g_scriptCallback[g_nlist[0]].active = false;
				}
			}
			strcpy(pLineIn, h);  
			return(0);
		}

		if (compare(ev[1], (char*)"set_dink_speed"))
		{
		
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,0,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p) && g_nlist[0] != 0)
			{
				g_dglos.dinkspeed = g_nlist[0];
			}

			strcpy(pLineIn, h);  
			return(0);
		}

		//redink1
		if (compare(ev[1], (char*)"set_dink_base_push"))
		{
			
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,0,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.mDinkBasePush = g_nlist[0];
			}
			strcpy(pLineIn, h);  
			return(0);
		}

		if (compare(ev[1], (char*)"reset_timer"))
		{
			h = &h[strlen(ev[1])];
			g_dglos.time_start = GetBaseApp()->GetGameTick();
			g_dglos.g_playerInfo.minutes = 0;
			strcpy(pLineIn, h);  
			return(0);
		}


		if (compare(ev[1], (char*)"set_keep_mouse"))
		{
			
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,0,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.keep_mouse = g_nlist[0];

			}

			strcpy(pLineIn, h);  
			return(0);
		}




		if (compare(ev[1], (char*)"add_item"))
		{

			h = &h[strlen(ev[1])];
			int32 p[20] = {2,1,1,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				add_item(slist[0], g_nlist[1], g_nlist[2], false);
			}

			strcpy(pLineIn, h);  
			return(0);
		}

		if (compare(ev[1], (char*)"add_exp"))
		{

			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				add_exp(g_nlist[0], g_nlist[1], true);
			}

			strcpy(pLineIn, h);  
			return(0);
		}


		if (compare(ev[1], (char*)"add_magic"))
		{

			h = &h[strlen(ev[1])];
			int32 p[20] = {2,1,1,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				add_item(slist[0], g_nlist[1], g_nlist[2], true);
			}

			strcpy(pLineIn, h);  
			return(0);
		}


		if (compare(ev[1], (char*)"kill_this_item"))
		{

			h = &h[strlen(ev[1])];
			int32 p[20] = {2,0,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				kill_cur_item_script(slist[0]);
			}

			strcpy(pLineIn, h);  
			return(0);
		}

		if (compare(ev[1], (char*)"kill_this_magic"))
		{

			h = &h[strlen(ev[1])];
			int32 p[20] = {2,0,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				kill_cur_magic_script(slist[0]);
			}

			strcpy(pLineIn, h);  
			return(0);
		}



		if (compare(ev[1], (char*)"show_bmp"))
		{
			//LogMsg("showing BMP");
			h = &h[strlen(ev[1])];
			int32 p[20] = {2,1,1,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_wait_for_button.active = false;
				show_bmp(slist[0], g_nlist[1], g_nlist[2], script);
			}

			strcpy(pLineIn, h);  
			return(2);
		}

		if (compare(ev[1], (char*)"wait_for_button"))
		{
			//LogMsg("waiting for button with script %d", script);
			h = &h[strlen(ev[1])];
			strcpy(pLineIn, h);  
			g_dglos.g_wait_for_button.script = script;
			g_dglos.g_wait_for_button.active = true;
			g_dglos.g_wait_for_button.button = 0;
			return(2);
		}

		if (compare(ev[1], (char*)"stop_wait_for_button"))
		{
			g_dglos.g_wait_for_button.active = false;
			return(0);
		}

		if (compare(ev[1], (char*)"copy_bmp_to_screen"))
		{
			LogMsg("copying BMP");
			h = &h[strlen(ev[1])];
			int32 p[20] = {2,0,0,0,0,0,0,0,0,0};  
			

			if (get_parms(ev[1], script, h, p))
			{
				copy_bmp(slist[0]);
				SetBitmapCopy(slist[0]);
			}

			strcpy(pLineIn, h);  
			return(0);
		}

		if (compare(ev[1], (char*)"say"))
		{

			h = &h[strlen(ev[1])];
			int32 p[20] = {2,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{

				if (g_nlist[1] == 0)
				{
					LogMsg("Say_stop error:  Sprite 0 can talk? Yeah, didn't think so.");
					return(0);
				}

				if (g_nlist[1] != 1000)
					kill_text_owned_by(g_nlist[1]);   
				decipher_string(slist[0], script);               
				g_dglos.g_returnint = say_text(slist[0], g_nlist[1], script);
				//Msg("Just said %s.", slist[0]);
			}

			strcpy(pLineIn, h);  
			return(0);
		}

		if (compare(ev[1], (char*)"draw_screen"))
		{
			if (g_dglos.g_gameMode == 1 || g_dglos.g_gameMode == 2)
			{
				g_dglos.m_bRenderBackgroundOnLoad = true;
				g_forceBuildBackgroundFromScratch = true;
			} else
			{
				g_dglos.m_bRenderBackgroundOnLoad = false;
			}
			
			if (g_scriptInstance[script]->sprite == 1000)
			{
				BuildScreenBackground();
				return(0);
			}
			BuildScreenBackground();
			return(2);
		}

		if (compare(ev[1], (char*)"free_items"))
		{
			g_dglos.g_returnint = 0;
			for (int i = 1; i < C_DINK_MAX_ITEMS + 1; i ++)
			{
				if (g_dglos.g_playerInfo.g_itemData[i].active == false)
				{
					g_dglos.g_returnint += 1;
				}
			}
			return(0);
		}


		if (compare(ev[1], (char*)"kill_cur_item"))
		{
			g_dglos.g_returnint = 0;
			kill_cur_item();
			return(2);
		}

		if (compare(ev[1], (char*)"kill_cur_magic"))
		{
			g_dglos.g_returnint = 0;
			kill_cur_magic();
			return(2);
		}

		if (compare(ev[1], (char*)"free_magic"))
		{
			g_dglos.g_returnint = 0;

			for (int i = 1; i < 9; i ++)
			{
				if (g_dglos.g_playerInfo.g_MagicData[i].active == false)
				{
					g_dglos.g_returnint += 1;
				}
			}
			return(0);
		}




		if (compare(ev[1], (char*)"draw_status"))
		{
			draw_status_all();
			return(0);
		}


		if (compare(ev[1], (char*)"arm_weapon"))
		{

			if (g_dglos.weapon_script != 0) if (locate(g_dglos.weapon_script, "DISARM")) run_script(g_dglos.weapon_script);
			g_dglos.weapon_script = load_script(g_dglos.g_playerInfo.g_itemData[*pcur_weapon].name, 1000, false);
			if (locate(g_dglos.weapon_script, "ARM")) run_script(g_dglos.weapon_script);


			return(0);
		}

		if (compare(ev[1], (char*)"arm_magic"))
		{


			if (g_dglos.magic_script != 0) if (locate(g_dglos.magic_script, "DISARM")) run_script(g_dglos.magic_script);
			g_dglos.magic_script = load_script(g_dglos.g_playerInfo.g_MagicData[*pcur_magic].name, 1000, false);
			if (locate(g_dglos.magic_script, "ARM")) run_script(g_dglos.magic_script);

			return(0);
		}


		if (compare(ev[1], (char*)"load_screen"))
		{
			//Msg("Loading map %d..",*pmap);  
			update_screen_time();
			load_map(g_MapInfo.loc[*pmap]);

			//redink1 fix for correct indicator on mini-map
			if (g_MapInfo.indoor[*pmap] == 0)
				g_dglos.g_playerInfo.last_map = *pmap;
			return(0);
		}


		if (compare(ev[1], (char*)"choice_start"))
		{

			kill_text_owned_by(1);      
			if (talk_get(script))
			{
				//      Msg("Question gathered successfully.");
				return(2);
			}

			return(0);
		}


		if (compare(ev[1], (char*)"say_stop"))
		{

			h = &h[strlen(ev[1])];
			int32 p[20] = {2,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				if (g_nlist[1] == 0)
				{
					LogMsg("Say_stop error:  Sprite 0 can talk? Yeah, didn't think so.");
					return(0);
				}

				kill_text_owned_by(g_nlist[1]);   
				kill_text_owned_by(1);  
				kill_returning_stuff(script);

				decipher_string(slist[0], script);               
				sprite = say_text(slist[0], g_nlist[1], script);
				g_dglos.g_returnint = sprite;
				g_sprite[sprite].callback = script;
				g_dglos.g_playerInfo.last_talk = script;     
				//Msg("Sprite %d marked callback true.", sprite);

				strcpy(pLineIn, h);  
				return(2);
			}

			strcpy(pLineIn, h);  
			return(0);
		}


		if (compare(ev[1], (char*)"say_stop_npc"))
		{

			h = &h[strlen(ev[1])];
			int32 p[20] = {2,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{

				if (text_owned_by(g_nlist[1])) 
				{
					g_dglos.g_returnint = 0;
					return(0);  
				}

				kill_returning_stuff(script);
				decipher_string(slist[0], script);               
				sprite = say_text(slist[0], g_nlist[1], script);
				g_dglos.g_returnint = sprite;
				g_sprite[sprite].callback = script;
				strcpy(pLineIn, h);  

				return(2);

			}

			strcpy(pLineIn, h);  
			return(0);
		}


		if (compare(ev[1], (char*)"say_stop_xy"))
		{

			h = &h[strlen(ev[1])];
			int32 p[20] = {2,1,1,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{

				//LogMsg("Say_stop_xy: Adding %s", slist[0]);
				kill_returning_stuff(script);
				decipher_string(slist[0], script);               
				sprite = say_text_xy(slist[0], g_nlist[1], g_nlist[2], script);
				g_sprite[sprite].callback = script;
				g_sprite[sprite].live = true;
				g_dglos.g_playerInfo.last_talk = script;     
				strcpy(pLineIn, h);  

				return(2);

			}

			strcpy(pLineIn, h);  
			return(0);
		}


		if (compare(ev[1], (char*)"say_xy"))
		{

			h = &h[strlen(ev[1])];
			int32 p[20] = {2,1,1,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				kill_returning_stuff(script);
				decipher_string(slist[0], script);               
				sprite = say_text_xy(slist[0], g_nlist[1], g_nlist[2], script);                             
				g_dglos.g_returnint = sprite;
				strcpy(pLineIn, h);               
				return(0);

			}

			strcpy(pLineIn, h);  
			return(0);
		}

		if (compare(ev[1], (char*)"restart_game"))
		{
			DinkRestartGame();

			/*
			while (kill_last_sprite()); 
			kill_repeat_sounds_all();
			kill_all_scripts_for_real();
			g_gameMode = 0;
			screenlock = 0;
			kill_all_vars();
			memset(&g_dglos.g_hitmap, 0, sizeof(g_dglos.g_hitmap));
			for (int u = 1; u <= 10; u++)
				g_dglos.g_playerInfo.button[u] = u;
			int script = load_script("main", 0, true);

			locate(script, "main");
			run_script(script);
			//lets attach our vars to the scripts
			attach();
			*/
			return(2);
		}       

		if (compare(ev[1], (char*)"wait"))
		{

			h = &h[strlen(ev[1])];
			int32 p[20] = {1,0,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				//           Msg("Wait called for %d.", nlist[0]);
				strcpy(pLineIn, h);  
				kill_returning_stuff(script);
				
				
#ifdef _DEBUG
				//seth's hack so glittering's crazy long intro goes faster
				//g_nlist[0] = 100;
#endif
				int cb1 = add_callback("",g_nlist[0],0,script);        

				return(2);
			}

			strcpy(pLineIn, h);  
			return(0);
		}

		if (compare(ev[1], (char*)"preload_seq"))
		{
			/*

			h = &h[strlen(ev[1])];
			int32 p[20] = {1,0,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				check_seq_status(g_nlist[0]);
			}

			strcpy(pLineIn, h);  
			*/
			return(0);
		}

		if (compare(ev[1], (char*)"script_attach"))
		{

			h = &h[strlen(ev[1])];
			int32 p[20] = {1,0,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{

				g_scriptInstance[script]->sprite = g_nlist[0];
			}
			strcpy(pLineIn, h);  
			return(0);
		}

		if (compare(ev[1], (char*)"draw_hard_sprite"))
		{

			h = &h[strlen(ev[1])];
			int32 p[20] = {1,0,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				update_play_changes();
				int l = g_nlist[0];
				rtRect32 mhard;
				mhard = g_dglos.g_picInfo[g_dglos.g_seq[g_sprite[l].pseq].frame[g_sprite[l].pframe]].hardbox;
				OffsetRect(&mhard, (g_sprite[l].x- 20), g_sprite[l].y);

				fill_hardxy(mhard);
				fill_back_sprites();
				fill_hard_sprites();


			}
			strcpy(pLineIn, h);  
			return(0);
		}


		if (compare(ev[1], (char*)"move"))
		{
			// (sprite, direction, until, nohard);
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,1,1,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				if (g_nlist[0] < 0 || g_nlist[0] >= C_MAX_SPRITES_AT_ONCE)
				{
					LogMsg("Illegal move command on sprite %d", g_nlist[0]);
				}
				else
				{
					g_sprite[g_nlist[0]].move_active = true;
					g_sprite[g_nlist[0]].move_dir = g_nlist[1];
					g_sprite[g_nlist[0]].move_num = g_nlist[2];
					g_sprite[g_nlist[0]].move_nohard = g_nlist[3];
					g_sprite[g_nlist[0]].move_script = 0;
					if (g_script_debug_mode) LogMsg("Moving: Sprite %d, dir %d, num %d", g_nlist[0], g_nlist[1], g_nlist[2]);

				}


			}

			strcpy(pLineIn, h);  
			return(0);
		}


		if (compare(ev[1], (char*)"sp_script"))
		{
			// (sprite, direction, until, nohard);
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,2,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				if (g_nlist[0] == 0)
				{
					LogMsg("Error: sp_script cannot process sprite 0??");  
					return(0);
				}
				kill_scripts_owned_by(g_nlist[0]);
				if (!slist[1][0] || load_script(slist[1], g_nlist[0], true) == 0)
				{
					g_dglos.g_returnint = 0;
					return(0);
				}
				//if (no_running_main == true) LogMsg("Not running %s until later..", g_scriptInstance[g_sprite[g_nlist[0]].script]->name);

				if (g_dglos.no_running_main == false)
					locate(g_sprite[g_nlist[0]].script, "MAIN");


				int tempreturn = g_sprite[g_nlist[0]].script;

				if (g_dglos.no_running_main == false)
					run_script(g_sprite[g_nlist[0]].script);       


				g_dglos.g_returnint = tempreturn;
			}

			strcpy(pLineIn, h);  
			return(0);
		}


		if (compare(ev[1], (char*)"spawn"))
		{
			// (sprite, direction, until, nohard);
			h = &h[strlen(ev[1])];
			int32 p[20] = {2,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				int mysc = load_script(slist[0], 1000, true);
				if (mysc == 0)
				{
					g_dglos.g_returnint = 0;
					return(0);
				}
				locate(mysc, "MAIN");
				int tempreturn = mysc;
				run_script(mysc);       
				g_dglos.g_returnint = tempreturn;
			}

			strcpy(pLineIn, h);  
			return(0);
		}

		if (compare(ev[1], (char*)"run_script_by_number"))
		{
			// (sprite, direction, until, nohard);
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,2,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				if (locate(g_nlist[0], slist[1]))
				{
					run_script(g_nlist[0]);
				}

			}

			strcpy(pLineIn, h);  
			return(0);
		}



		if (compare(ev[1], (char*)"draw_hard_map"))
		{
			// (sprite, direction, until, nohard);
			LogMsg("Drawing hard map.."); 
			update_play_changes();
			fill_whole_hard();     
			fill_hard_sprites();
			fill_back_sprites();
			strcpy(pLineIn, h);  
			return(0);
		}


		if (compare(ev[1], (char*)"draw_background"))
		{
			g_forceBuildBackgroundFromScratch = true;
			BuildScreenBackground(false, true);
			strcpy(pLineIn, h);  
			return(0);
		}

		if (compare(ev[1], (char*)"fade_down"))
		{
			g_dglos.process_downcycle = true;
			g_dglos.process_upcycle = false;

			g_dglos.cycle_clock = g_dglos.g_dinkTick;
		
			g_dglos.cycle_script = script;

			strcpy(pLineIn, h);

			return(2);
		}

		if (compare(ev[1], (char*)"fade_up"))
		{
			h = &h[strlen(ev[1])];
			g_dglos.process_upcycle = true;
			g_dglos.process_downcycle = false;

			g_dglos.cycle_clock = g_dglos.g_dinkTick;
			g_dglos.cycle_script = script; 
			g_dinkFadeAlpha = 1;
			strcpy(pLineIn, h);  
			return(2);
		}

		if (compare(ev[1], (char*)"kill_this_task"))
		{
			if (g_scriptInstance[script]->proc_return != 0)
			{
				run_script(g_scriptInstance[script]->proc_return);
			}
			kill_script(script);
			return(2);
		}

	
		if (compare(ev[1], (char*)"kill_game"))
		{
			LogMsg("Was told to kill game, so doing it like a good boy."); 
			
			g_sprite[1].freeze = 0;
			SaveState(g_dglo.m_savePath+"continue_state.dat", false);
			WriteLastPathSaved("");

			//kill our state.dat if it existed, not needed now, this can exist if an iphone goes into suspend, but then is resumed
			RemoveFile(GetSavePath()+"state.dat", false);

			DinkQuitGame();
			//SyncPersistentData();
			//uncomment below if you want this to actually work
			//PostMessage(g_hWndMain, WM_CLOSE, 0, 0);
			
			return(2);
		}

		//redink1 added
		if (compare(ev[1], (char*)"loopmidi"))
		{
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,0,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				if ( g_nlist[0] > 0 )
				{
					g_dglos.mLoopMidi = true;
				}
				else
				{
					g_dglos.mLoopMidi = false;
				}
			}
			return(0);
		}

		if (compare(ev[1], (char*)"playmidi"))
		{
			h = &h[strlen(ev[1])];
			int32 p[20] = {2,0,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{

				int regm = atol(slist[0]);      
				LogMsg("Processing playmidi command.");            
				if (regm > 1000)
				{
					
					LogMsg("playmidi - cd play command detected.");            
					PlayMidi((toString(regm-1000)+".mid").c_str());

				} 
				
				//ok, weird, but the actual dink plays both and this is needed - if the fake CD tune can't play, it will probably find it below
				//else
				{
					PlayMidi(slist[0]);
				}

			}

			strcpy(pLineIn, h);  
			return(0);
		}
		if (compare(ev[1], (char*)"stopmidi"))
		{
			// (sprite, direction, until, nohard);
			h = &h[strlen(ev[1])];
			StopMidi();
			strcpy(pLineIn, h);  
			return(0);
		}

		if (compare(ev[1], (char*)"LOGMSG"))
		{
			h = &h[strlen(ev[1])];
			int32 p[20] = {2,0,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				LogMsg(slist[0]);
			}

			strcpy(pLineIn, h);  
			return(0);
		}


		if (compare(ev[1], (char*)"kill_all_sounds"))
		{
			kill_repeat_sounds_all();
			strcpy(pLineIn, h);  
			return(0);

		}

		if (compare(ev[1], (char*)"turn_midi_off"))
		{
			g_dglos.midi_active = false;
			strcpy(pLineIn, h);  
			return(0);

		}
		if (compare(ev[1], (char*)"turn_midi_on"))
		{
			g_dglos.midi_active = true;
			strcpy(pLineIn, h);  
			return(0);
		}

		if (compare(ev[1], (char*)"Playsound"))
		{
			// (sprite, direction, until, nohard);
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,1,1,1,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				if (sound_on)  
					g_dglos.g_returnint = SoundPlayEffect(g_nlist[0], g_nlist[1], g_nlist[2], g_nlist[3],g_nlist[4]);
				else g_dglos.g_returnint = 0;                     

			} else
				g_dglos.g_returnint = 0;

			strcpy(pLineIn, h);  
			return(0);
		}


		if (compare(ev[1], (char*)"sound_set_survive"))
		{
			// (sprite, direction, until, nohard);
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				if (sound_on)  
				{
					//let's set one sound to survive
					if (g_nlist[0] > 0)
						soundinfo[g_nlist[0]].survive = g_nlist[1];
				}
			}

			strcpy(pLineIn, h);  
			return(0);
		}


		if (compare(ev[1], (char*)"sound_set_vol"))
		{
			// (sprite, direction, until, nohard);
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				if (sound_on)  
				{
					//let's set one sound to survive
					if (g_nlist[0] > 0)
					{
						soundinfo[g_nlist[0]].vol = g_nlist[1];

						soundbank[g_nlist[0]].SetVolume(g_nlist[1]);
					}
				}
			}

			strcpy(pLineIn, h);  
			return(0);
		}

		if (compare(ev[1], (char*)"sound_set_kill"))
		{
			// (sprite, direction, until, nohard);
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,0,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				if (sound_on)  
				{
					//let's set one sound to survive
					if (g_nlist[0] > 0)
					{
						soundbank[g_nlist[0]].Stop();
					}

				}
			}

			strcpy(pLineIn, h);  
			return(0);
		}

		if (compare(ev[1], (char*)"save_game"))
		{
			// (sprite, direction, until, nohard);
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,0,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				save_game(g_nlist[0]);
			}

			strcpy(pLineIn, h);  
			return(0);
		}


		if (compare(ev[1], (char*)"force_vision"))
		{
			// (sprite, direction, until, nohard);
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,0,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				*pvision = g_nlist[0];
				g_scriptInstance[script]->sprite = 1000;
				fill_whole_hard();
				BuildScreenBackground(true, true);
			}

			strcpy(pLineIn, h);  
			return(0);
		}

		if (compare(ev[1], (char*)"fill_screen"))
		{
			// (sprite, direction, until, nohard);
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,0,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.last_fill_screen_palette_color = g_nlist[0];

				fill_screen(g_nlist[0]);
				SetBitmapCopy(""); //no bitmap, but it will trigger the no status bar rendering until "something happens"
				g_dglos.m_bRenderBackgroundOnLoad = false;

			}

			strcpy(pLineIn, h);  
			return(0);
		}


		if (compare(ev[1], (char*)"load_game"))
		{
			// (sprite, direction, until, nohard);
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,0,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				int gameSlot = g_nlist[0];

				if (gameSlot == 10)
				{
					string fName = DinkGetSavePath()+"autosave.dat";

					bool bSuccess = LoadState(fName, true);

					if (!bSuccess)
					{
						RemoveFile(fName, false);
						GetAudioManager()->Play("audio/buzzer2.wav");
						ShowQuickMessage("Error loading save state.  Old version?");
					} else
					{
						GetAudioManager()->Play("audio/quick_load.wav");
						LoadState(fName, false);
						ShowQuickMessage("Game loaded.");
					}



				} else
				{
					LoadGame(gameSlot);

				}
				return(2);
			}

			strcpy(pLineIn, h);  
			return(0);
		}

		if (compare(ev[1], (char*)"game_exist"))
		{
			// (sprite, direction, until, nohard);
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,0,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
			if (g_nlist[0] == 10)
			{
				//hack to check for the autosave slot instead
				sprintf(temp, "%sautosavedb.dat",(g_dglo.m_savePath).c_str());
			} else
			{
				sprintf(temp, "%ssave%d.dat",(g_dglo.m_savePath).c_str(), g_nlist[0]);
			}
				if (FileExists(temp)) g_dglos.g_returnint = 1; else g_dglos.g_returnint = 0;
			}

			strcpy(pLineIn, h);  
			return(0);
		}

		if (compare(ev[1], (char*)"move_stop"))
		{
			// (sprite, direction, until, nohard);
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,1,1,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{

				if (g_nlist[0] < 0 || g_nlist[0] >= C_MAX_SPRITES_AT_ONCE)
				{
					LogMsg("Invalid move_stop command on sprite %d", g_nlist[0]);

				}
				else
				{
					//Msg("Move stop running %d to %d..", nlist[0], nlist[0]);  
					g_sprite[g_nlist[0]].move_active = true;
					g_sprite[g_nlist[0]].move_dir = g_nlist[1];
					g_sprite[g_nlist[0]].move_num = g_nlist[2];
					g_sprite[g_nlist[0]].move_nohard = g_nlist[3];
					g_sprite[g_nlist[0]].move_script = script;
					strcpy(pLineIn, h);
					if (g_script_debug_mode) LogMsg("Move_stop: Sprite %d, dir %d, num %d", g_nlist[0], g_nlist[1], g_nlist[2]);
					return(2);
				}

			}

			strcpy(pLineIn, h);  
			return(0);
		}

		if (compare(ev[1], (char*)"load_sound"))
		{

			h = &h[strlen(ev[1])];
			int32 p[20] = {2,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				if (sound_on)
				{
#ifdef _DEBUG
					LogMsg("getting %s",slist[0]);
#endif
					CreateBufferFromWaveFile(slist[0],g_nlist[1]);
				}
			}

			strcpy(pLineIn, h);  
			return(0);
		}

		if (compare(ev[1], (char*)"debug"))
		{

			h = &h[strlen(ev[1])];
			int32 p[20] = {2,0,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				decipher_string(slist[0], script);       
				LogMsg(slist[0]);
			}

			strcpy(pLineIn, h);  
			return(0);
		}

		if (compare(ev[1], (char*)"goto"))
		{
			//LogMsg("Goto %s", ev[2]);
			locate_goto(ev[2], script);
			return(0);
		}

		//redink1 added for global functions
		if (compare(ev[1], (char*)"make_global_function"))
		{
			h = &h[strlen(ev[1])];
			int32 p[20] = {2,2,0,0,0,0,0,0,0,0};
			if (get_parms(ev[1], script, h, p))
			{

				make_function(slist[0], slist[1]);
				//Msg(slist[0]);
			}
			strcpy(pLineIn, h);
			return(0);
		}

		if (compare(ev[1], (char*)"make_global_int"))
		{

			h = &h[strlen(ev[1])];
			int32 p[20] = {2,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{

				make_int(slist[0], g_nlist[1],0, script);
				//Msg(slist[0]);
			}

			strcpy(pLineIn, h);
			return(0);
		}


		if (compare(ev[1], (char*)"int"))
		{

			int_prepare(h, script);

			//Msg(slist[0]);

			h = &h[strlen(ev[1])];

			//Msg("Int is studying %s..", h);
			if (strchr(h, '=') != NULL)
			{
				strip_beginning_spaces(h);
				//Msg("Found =...continuing equation");
				strcpy(pLineIn, h);  
				return(4);
			}

			return(0);

		}

		if (compare(ev[1], (char*)"busy"))
		{

			h = &h[strlen(ev[1])];
			// Msg("Running busy, h is %s", h);    
			int32 p[20] = {1,0,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				if (g_nlist[0] == 0) LogMsg("ERROR:  Busy cannot get info on sprite 0 in %s.",g_scriptInstance[script]->name);
				else
				{

					g_dglos.g_returnint = does_sprite_have_text(g_nlist[0]);

					LogMsg("Busy: Return int is %d and %d.  Nlist got %d.", g_dglos.g_returnint,does_sprite_have_text(g_nlist[0]), g_nlist[0]);

				}

			}  else LogMsg("Failed getting parms for Busy()");

			strcpy(pLineIn, h);  
			return(0);
		}

		//redink1 added
		if (compare(ev[1], (char*)"sp_freeze"))
		{
			h = &h[strlen(ev[1])];
			// Msg("Running busy, h is %s", h);    
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{

				if (g_nlist[0] < 0 || g_nlist[0] > C_MAX_SPRITES_AT_ONCE)
				{
					LogMsg("sp_freeze ignored, sprite %d is no good", g_nlist[0]);
				}
				else
				{
					// Set the value
					if (g_nlist[1] == 0)
					{
						g_sprite[g_nlist[0]].freeze = 0;
					}
					else if (g_nlist[1] == 1)
					{
						g_sprite[g_nlist[0]].freeze = script;
					}

					// Return the value
					if (g_sprite[g_nlist[0]].freeze > 0)
					{
						g_dglos.g_returnint = 1;
					}
					else
					{
						g_dglos.g_returnint = 0;
					}
				}
			}

			strcpy(pLineIn, h);  
			return(0);
		}

		if (compare(ev[1], (char*)"inside_box"))
		{

			h = &h[strlen(ev[1])];
			//LogMsg("Running pigs with h", h);
			int32 p[20] = {1,1,1,1,1,1,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{

				rtRect32 myrect(g_nlist[2], g_nlist[3], g_nlist[4], g_nlist[5]);
				g_dglos.g_returnint = inside_box(g_nlist[0], g_nlist[1], myrect);

				if (g_script_debug_mode)
					LogMsg("Inbox is int is %d and %d.  Nlist got %d.", g_dglos.g_returnint, g_nlist[0], g_nlist[1]);



			}  else LogMsg("Failed getting parms for inside_box");

			strcpy(pLineIn, h);  
			return(0);
		}


		if (compare(ev[1], (char*)"random"))
		{
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = (rand() % g_nlist[0])+g_nlist[1];
			}  else LogMsg("Failed getting parms for Random()");

			strcpy(pLineIn, h);  
			return(0);
		}
		
		if (compare(ev[1], (char*)"get_version"))
		{
			h = &h[strlen(ev[1])];
			g_dglos.g_returnint = C_DINK_VERSION;
			strcpy(pLineIn, h);  
			return(0);
		}

		if (compare(ev[1], (char*)"is_base_game"))
		{
			h = &h[strlen(ev[1])];
			g_dglos.g_returnint = 1;
			if (!g_dglo.m_dmodGameDir.empty()) g_dglos.g_returnint = 0;
			strcpy(pLineIn, h);
			return(0);
		}

		if (compare(ev[1], (char*)"get_client_version"))
		{
			h = &h[strlen(ev[1])];
			g_dglos.g_returnint = GetApp()->GetVersion()*100;
			strcpy(pLineIn, h);  
			return(0);
		}

		if (compare(ev[1], (char*)"get_platform"))
		{
			//h = &h[strlen(ev[1])];
			g_dglos.g_returnint = GetEmulatedPlatformID();

			strcpy(pLineIn, h);  
			return(0);
		}
	
		if (compare(ev[1], (char*)"SHOW_POPUP"))
		{
			h = &h[strlen(ev[1])];
			int32 p[20] = {2,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				LogMsg("Showing %s in %d..",slist[0], g_nlist[1]);
				
				Entity *pBG = GetEntityRoot()->GetEntityByName("GameMenu");
				assert(pBG);
				if (!pBG) return 0;
                VariantList vList(pBG, string(slist[0]));
				GetMessageManager()->CallEntityFunction(pBG, g_nlist[1], "ShowQuickTip", &vList); 
			}

			strcpy(pLineIn, h);  
			return(0);
		}


		if (compare(ev[1], (char*)"initfont"))
		{
			LogMsg("Ignoring Initfont command");
			return(0);
		}

		if (compare(ev[1], (char*)"get_truecolor"))
		{
			//h = &h[strlen(ev[1])];
			if (g_dglo.m_dmodGameDir == "lyna/") return 0;
			
			g_dglos.g_returnint = 1;
			strcpy(pLineIn, h);  
			return(0);
		}

		if (compare(ev[1], (char*)"get_burn"))
		{
			h = &h[strlen(ev[1])];
			g_dglos.g_returnint = 0;
			strcpy(pLineIn, h);  
			return(0);
		}

		if (compare(ev[1], (char*)"set_mode"))
		{
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,0,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_gameMode = g_nlist[0];
				g_dglos.g_returnint = g_dglos.g_gameMode;
				//if (g_dglos.g_gameMode == )
			}  else LogMsg("Failed to set mode");

			strcpy(pLineIn, h);  
			return(0);
		}

		if (compare(ev[1], (char*)"kill_shadow"))
		{
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,0,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{

				for (int jj = 1; jj <= g_dglos.last_sprite_created; jj++)
				{
					if (g_sprite[jj].brain == 15) if (g_sprite[jj].brain_parm == g_nlist[0])
					{

						g_sprite[jj].active = 0;
					}


				}
			}  

			strcpy(pLineIn, h);  
			return(0);
		}

		if (compare(ev[1], (char*)"create_sprite"))
		{
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,1,1,1,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{

				g_dglos.g_returnint = add_sprite_dumb(g_nlist[0],g_nlist[1],g_nlist[2],
					g_nlist[3],g_nlist[4],
					100);

				return(0);
			}
			g_dglos.g_returnint =  0;
			return(0);
		}



		if (compare(ev[1], (char*)"sp"))
		{
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,0,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				for (int ii = 1; ii <= g_dglos.last_sprite_created; ii++)
				{

					if (g_sprite[ii].sp_index == g_nlist[0])
					{

						if (g_script_debug_mode) LogMsg("Sp returned %d.", ii);
						g_dglos.g_returnint = ii;
						return(0);
					}

				}
				if (g_dglos.last_sprite_created == 1)
				{
					LogMsg("warning - you can't call SP() from a screen-ref, no sprites have been created yet.");
				}

			}
			g_dglos.g_returnint =  0;
			return(0);
		}


		if (compare(ev[1], (char*)"is_script_attached"))
		{
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,0,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{

				if (g_nlist[0] < 0 || g_nlist[0] >= C_MAX_SPRITES_AT_ONCE)
				{
					return 0;
				}
				g_dglos.g_returnint =  g_sprite[g_nlist[0]].script;

			}
			return(0);
		}



		if (compare(ev[1], (char*)"sp_speed"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = change_sprite(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].speed);

				if (g_nlist[1] != -1) changedir(g_sprite[g_nlist[0]].dir, g_nlist[0], g_sprite[g_nlist[0]].base_walk);

				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}


		if (compare(ev[1], (char*)"sp_range"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = change_sprite(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].range);

				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}


		if (compare(ev[1], (char*)"sp_nocontrol"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = change_sprite(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].nocontrol);
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}

		if (compare(ev[1], (char*)"sp_nodraw"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = change_sprite(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].nodraw);
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}


		if (compare(ev[1], (char*)"sp_picfreeze"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = change_sprite(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].picfreeze);
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}



		if (compare(ev[1], (char*)"get_sprite_with_this_brain"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				for (int i = 1; i <= g_dglos.last_sprite_created; i++)
				{
					if (   (g_sprite[i].brain == g_nlist[0]) && (i != g_nlist[1]) ) if
						(g_sprite[i].active == 1)
					{
						//LogMsg("Ok, sprite with brain %d is %d", g_nlist[0], i);
						g_dglos.g_returnint = i;
						return(0);
					}

				}
			}
			//LogMsg("Ok, sprite with brain %d is 0", g_nlist[0], i);

			g_dglos.g_returnint =  0;
			return(0);
		}

		//redink1 added this to make Paul Pliska's life more fulfilling
		if (compare(ev[1], (char*)"get_next_sprite_with_this_brain"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,1,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				for (int i = g_nlist[2]; i <= g_dglos.last_sprite_created; i++)
				{
					if (   (g_sprite[i].brain == g_nlist[0]) && (i != g_nlist[1]) ) if
						(g_sprite[i].active == 1)
					{
						//LogMsg("Ok, sprite with brain %d is %d", g_nlist[0], i);
						g_dglos.g_returnint = i;
						return(0);
					}

				}
			}
			//LogMsg("Ok, sprite with brain %d is 0", g_nlist[0], i);

			g_dglos.g_returnint =  0;
			return(0);
		}


		if (compare(ev[1], (char*)"get_rand_sprite_with_this_brain"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{

				int cter = 0;
				for (int i = 1; i <= g_dglos.last_sprite_created; i++)
				{
					if (   (g_sprite[i].brain == g_nlist[0]) && (i != g_nlist[1]) ) if
						(g_sprite[i].active == 1)
					{
						cter++;

					}

				}

				if (cter == 0)
				{
					LogMsg("Get rand brain can't find any brains with %d.",g_nlist[0]);
					g_dglos.g_returnint =  0;
					return(0);
				}

				int mypick = (rand() % cter)+1;
				cter = 0;
				for (int ii = 1; ii <= g_dglos.last_sprite_created; ii++)
				{
					if (   (g_sprite[ii].brain == g_nlist[0]) && (ii != g_nlist[1]) ) if
						(g_sprite[ii].active == 1)
					{
						cter++;
						if (cter == mypick)
						{
							g_dglos.g_returnint = ii;
							return(0);
						}
					}

				}


			}


			g_dglos.g_returnint =  0;
			return(0);
		}



		if (compare(ev[1], (char*)"sp_sound"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = change_sprite(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].sound);

				if (g_nlist[1] > 0)
				{
					SoundPlayEffect( g_sprite[g_nlist[0]].sound,22050, 0,g_nlist[0], 1);

				}
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}

		if (compare(ev[1], (char*)"sp_attack_wait"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{

				g_dglos.g_returnint = change_sprite(g_nlist[0], g_nlist[1]+g_dglos.g_dinkTick, &g_sprite[g_nlist[0]].attack_wait);
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}


		if (compare(ev[1], (char*)"sp_active"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{

				g_dglos.g_returnint = change_sprite(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].active);


				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}

		if (compare(ev[1], (char*)"sp_disabled"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{

				g_dglos.g_returnint = change_sprite(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].disabled);


				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}

		if (compare(ev[1], (char*)"sp_size"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = change_sprite(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].size);
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}
		if (compare(ev[1], (char*)"activate_bow"))
		{     

			g_sprite[1].seq = 0;
			g_sprite[1].pseq = 100+g_sprite[1].dir;
			g_sprite[1].pframe = 1;
			g_dglos.g_bowStatus.active = true;
			g_dglos.g_bowStatus.script = script;
			g_dglos.g_bowStatus.hitme = false;
			g_dglos.g_bowStatus.time = 0;
			return(2);      
		}

		if (compare(ev[1], (char*)"get_last_bow_power"))
		{     
			g_dglos.g_returnint = g_dglos.g_bowStatus.last_power;
			return(0);      
		}

		if (compare(ev[1], (char*)"sp_que"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = change_sprite(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].que);
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}

		if (compare(ev[1], (char*)"sp_gold"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = change_sprite(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].gold);
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}

		if (compare(ev[1], (char*)"sp_base_walk"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = change_sprite_noreturn(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].base_walk);
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}


		if (compare(ev[1], (char*)"sp_target"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = change_sprite(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].target);
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}

		if (compare(ev[1], (char*)"stopcd"))
		{     

			LogMsg("Stopped cd");

			return(0);
		}


		if (compare(ev[1], (char*)"sp_base_hit"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = change_sprite_noreturn(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].base_hit);
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}

		if (compare(ev[1], (char*)"sp_base_attack"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = change_sprite_noreturn(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].base_attack);
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}


		if (compare(ev[1], (char*)"sp_base_idle"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = change_sprite_noreturn(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].base_idle);
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}


		if ( (compare(ev[1], (char*)"sp_base_die")) || (compare(ev[1], (char*)"sp_base_death"))  )
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = change_sprite_noreturn(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].base_die);
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}


		if (compare(ev[1], (char*)"disable_all_sprites"))
		{     
			for (int jj = 1; jj < g_dglos.last_sprite_created; jj++)
				if (g_sprite[jj].active) g_sprite[jj].disabled = true;
			return(0);
		}
		if (compare(ev[1], (char*)"enable_all_sprites"))
		{     
			for (int jj = 1; jj < g_dglos.last_sprite_created; jj++)
				if (g_sprite[jj].active) g_sprite[jj].disabled = false;
			return(0);
		}


		if (compare(ev[1], (char*)"sp_pseq"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = change_sprite(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].pseq);
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}


		if (compare(ev[1], (char*)"sp_pframe"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = change_sprite(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].pframe);
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}


		if (compare(ev[1], (char*)"sp_seq"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = change_sprite(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].seq);
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}

		if (compare(ev[1], (char*)"editor_type"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				//Msg("Setting editor_type..");
				g_dglos.g_returnint = change_edit_char(g_nlist[0], g_nlist[1], &g_dglos.g_playerInfo.spmap[*pmap].type[g_nlist[0]]);
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}
		if (compare(ev[1], (char*)"editor_seq"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = change_edit(g_nlist[0], g_nlist[1], &g_dglos.g_playerInfo.spmap[*pmap].seq[g_nlist[0]]);
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}

		if (compare(ev[1], (char*)"editor_frame"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = change_edit_char(g_nlist[0], g_nlist[1], &g_dglos.g_playerInfo.spmap[*pmap].frame[g_nlist[0]]);
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}



		if (compare(ev[1], (char*)"sp_editor_num"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,0,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				if (g_nlist[0] < 0 || g_nlist[0] >= C_MAX_SPRITES_AT_ONCE)
				{
					g_dglos.g_returnint = 0;
					return 0;
				}
				g_dglos.g_returnint = g_sprite[g_nlist[0]].sp_index;
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}

		if (compare(ev[1], (char*)"sp_brain"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{

				if (g_nlist[0] < 0 || g_nlist[0] >= C_MAX_SPRITES_AT_ONCE)
				{
					LogMsg("so_brain sent bad sprite %d", g_nlist[0]);
				}
				else
				{

					g_dglos.g_returnint = change_sprite(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].brain);
					if (g_nlist[1] == 13)
					{
						//a mouse brain was set...
						g_dglo.SetViewOverride(DinkGlobals::VIEW_FULL);

					}
				}
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}

		if (compare(ev[1], (char*)"sp_exp"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = change_sprite(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].exp);
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}
		if (compare(ev[1], (char*)"set_button"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{

				g_dglos.g_playerInfo.button[g_nlist[0]] = g_nlist[1];

				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}


		if (compare(ev[1], (char*)"sp_reverse"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = change_sprite(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].reverse);
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}


		if (compare(ev[1], (char*)"sp_noclip"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = change_sprite(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].noclip);
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}

		if (compare(ev[1], (char*)"sp_touch_damage"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = change_sprite_noreturn(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].touch_damage);
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}


		if (compare(ev[1], (char*)"sp_brain_parm"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = change_sprite(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].brain_parm);
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}
		if (compare(ev[1], (char*)"sp_brain_parm2"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = change_sprite(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].brain_parm2);
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}

		if (compare(ev[1], (char*)"sp_follow"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = change_sprite(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].follow);
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}

		if (compare(ev[1], (char*)"set_smooth_follow"))
		{
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,0,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				if ( g_nlist[0] == 0 )
				{
					g_dglos.smooth_follow = false;
				}
				else if ( g_nlist[0] == 1 )
				{
					g_dglos.smooth_follow = true;
				}
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}

		if (compare(ev[1], (char*)"sp_frame"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				//Allow -1, in case a script needs to get the current frame.
				if (g_nlist[1] < -1 || g_nlist[1] >= C_MAX_SPRITE_FRAMES)
				{
					LogMsg("sp_frame trying to set something to frame %d?  Illegal, forcing to 1.", g_nlist[1]);
					g_nlist[1] = 1;
				}
				g_dglos.g_returnint = change_sprite(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].frame);
				
				
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}

		if (compare(ev[1], (char*)"sp_frame_delay"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = change_sprite(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].frame_delay);
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}


		if (compare(ev[1], (char*)"hurt"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				//redink1 fix for freeze if hurt value is less than 0
				if (g_nlist[1] < 0)
					return(0);

				if (g_nlist[0] <0 || g_nlist[0] >= C_MAX_SPRITES_AT_ONCE)
				{
					LogMsg("hurt command used on sprite %d, ignoring", g_nlist[0]);
					return 0;
				}

				if (hurt_thing(g_nlist[0], g_nlist[1], 0) > 0)
					random_blood(g_sprite[g_nlist[0]].x, g_sprite[g_nlist[0]].y-40, g_nlist[0]);
				if (g_sprite[g_nlist[0]].nohit != 1)
					if (g_sprite[g_nlist[0]].script != 0)

						if (locate(g_sprite[g_nlist[0]].script, "HIT"))
						{

							if (g_scriptInstance[script]->sprite != 1000)
							{
								*penemy_sprite = g_scriptInstance[script]->sprite;
								//redink1 addition of missle_source stuff
								*pmissle_source = g_scriptInstance[script]->sprite;
							}

							kill_returning_stuff(g_sprite[g_nlist[0]].script);
							run_script(g_sprite[g_nlist[0]].script);
						}

						return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}


		if (compare(ev[1], (char*)"sp_hard"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				if (g_nlist[0] < 0 || g_nlist[0] >= C_MAX_SPRITES_AT_ONCE)
				{
					LogMsg("Bad sprite %d sent to sp_hard", g_nlist[0]);
					g_dglos.g_returnint = -1;
					return 0;
				}
				else
				{
					g_dglos.g_returnint = change_sprite(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].hard);
					if (g_sprite[g_nlist[0]].sp_index != 0) if (g_nlist[1] != -1)
					{

						g_dglos.g_smallMap.sprite[g_sprite[g_nlist[0]].sp_index].hard = g_dglos.g_returnint;
					}
				}
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}

		if (compare(ev[1], (char*)"sp_move_nohard"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = change_sprite(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].move_nohard);
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}
		if (compare(ev[1], (char*)"sp_flying"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = change_sprite(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].flying);
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}




		if (compare(ev[1], (char*)"sp_kill_wait"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,0,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				if (g_nlist[0] < 0 || g_nlist[0] >= C_MAX_SPRITES_AT_ONCE)
				{
					LogMsg("sp_kill_wait sent invalid %d sprite", g_nlist[0]);
				}
				else
				{
					g_sprite[g_nlist[0]].wait = 0;
				}
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}
		if (compare(ev[1], (char*)"sp_kill"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
			if (g_nlist[0] < C_MAX_SPRITES_AT_ONCE && g_nlist[0] >= 0) //SETH this fixes crash when killing milder
			{
				g_sprite[g_nlist[0]].kill = g_nlist[1];
				return(0);
			} else
			{
				LogMsg("Aborting crash in sp_kill");
			}
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}

		if (compare(ev[1], (char*)"screenlock"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,0,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				if ( g_nlist[0] == 0 || g_nlist[0] == 1 )
				{
					g_dglos.screenlock = g_nlist[0];
				}
			}
			//redink1 - set screenlock() to return the screenlock value
			g_dglos.g_returnint = g_dglos.screenlock; 
			return(0);
		}

		if (compare(ev[1], (char*)"stop_entire_game"))
		{
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,0,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_stopEntireGame = g_nlist[0];

			
				/*	
				rtRect32 rcRect(0,0,C_DINK_SCREENSIZE_X,C_DINK_SCREENSIZE_Y);
					int ddrval;
					ddrval = lpDDSBackGround->BltFast( 0, 0, lpDDSBack,
						&rcRect, DDBLTFAST_NOCOLORKEY);
						*/
			
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}


		if (compare(ev[1], (char*)"dink_can_walk_off_screen"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,0,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.walk_off_screen = g_nlist[0];
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}

		if (compare(ev[1], (char*)"push_active"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,0,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_pushingEnabled = g_nlist[0];
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}


		if (compare(ev[1], (char*)"sp_x"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = change_sprite(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].x);
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}


		if (compare(ev[1], (char*)"count_item"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {2,0,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = 0;
				for (int i = 1; i < C_DINK_MAX_ITEMS + 1; i++)
				{
					if (g_dglos.g_playerInfo.g_itemData[i].active)
					{
						if (compare(g_dglos.g_playerInfo.g_itemData[i].name, slist[0])) g_dglos.g_returnint++;                  
					}

				}

				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}


		if (compare(ev[1], (char*)"count_magic"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {2,0,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = 0;
				for (int i = 1; i < 9; i++)
				{
					if (g_dglos.g_playerInfo.g_MagicData[i].active)
					{
						if (compare(g_dglos.g_playerInfo.g_MagicData[i].name, slist[0])) g_dglos.g_returnint++;                 
					}

				}

				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}


		if (compare(ev[1], (char*)"sp_mx"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = change_sprite(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].mx);
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}

		if (compare(ev[1], (char*)"sp_move_x"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				change_sprite_noreturn(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].mx);
				return(0);
			}
			return(0);
		}

		if (compare(ev[1], (char*)"sp_my"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = change_sprite(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].my);
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}

		if (compare(ev[1], (char*)"sp_move_y"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				change_sprite_noreturn(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].my);
				return(0);
			}
			return(0);
		}

		if (compare(ev[1], (char*)"scripts_used"))
		{     
			h = &h[strlen(ev[1])];
			int m = 0;
			for (int i = 1; i < C_MAX_SCRIPTS; i++)
				if (g_scriptInstance[i] != NULL) m++;
			g_dglos.g_returnint = m;
#ifdef _DEBUG
LogMsg("%d scripts used", g_dglos.g_returnint);
#endif
			return(0);
		}

		if (compare(ev[1], (char*)"sp_hitpoints"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = change_sprite(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].hitpoints);

				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}

		if (compare(ev[1], (char*)"sp_attack_hit_sound"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = change_sprite(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].attack_hit_sound);

				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}

		if (compare(ev[1], (char*)"sp_attack_hit_sound_speed"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = change_sprite(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].attack_hit_sound_speed);

				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}


		if (compare(ev[1], (char*)"sp_strength"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = change_sprite(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].strength);

				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}

		if (compare(ev[1], (char*)"sp_defense"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = change_sprite(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].defense);

				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}

		if (compare(ev[1], (char*)"init"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {2,0,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				figure_out(slist[0], 0);
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}

		if (compare(ev[1], (char*)"sp_distance"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = change_sprite(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].distance);
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}

		if (compare(ev[1], (char*)"sp_nohit"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = change_sprite(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].nohit);
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}

		if (compare(ev[1], (char*)"sp_notouch"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = change_sprite(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].notouch);
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}

		if (compare(ev[1], (char*)"compare_weapon"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {2,0,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = 0;
				if (*pcur_weapon == 0)
				{
					return(0);
				}

				if (compare(g_dglos.g_playerInfo.g_itemData[*pcur_weapon].name, slist[0]))
				{
					g_dglos.g_returnint = 1;

				}
				return(0);
			}
			return(0);
		}


		if (compare(ev[1], (char*)"compare_magic"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {2,0,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = 0;
				if (*pcur_magic == 0)
				{
					return(0);
				}

				//redink1 fix so compare_magic works!
				if (compare(g_dglos.g_playerInfo.g_MagicData[*pcur_magic].name, slist[0]))
				{
					g_dglos.g_returnint = 1;

				}
				return(0);
			}
			return(0);
		}


		if (compare(ev[1], (char*)"compare_sprite_script"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,2,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = 0;

				if (g_nlist[0] < 1 || g_nlist[0] >= C_MAX_SPRITES_AT_ONCE)
				{
					LogMsg("Error: Can't compare sprite script for sprite %d!??!?!", g_nlist[0]);
					return(0);
				}
				if (g_sprite[g_nlist[0]].active)
				{

					if (g_sprite[g_nlist[0]].script == 0)
					{
						LogMsg("Compare sprite script says: Sprite %d has no script.",g_nlist[0]);
						return(0);
					}

					if (!g_scriptInstance[g_sprite[g_nlist[0]].script])
					{
						LogMsg("Compare sprite script says: Sprite %d says it has script %d, but actually it's null, so no?", g_nlist[0], g_sprite[g_nlist[0]].script);
						return(0);
					}
					if (compare(slist[1], g_scriptInstance[g_sprite[g_nlist[0]].script]->name))
					{
						g_dglos.g_returnint = 1;
						return(0);
					}

				} else
				{
					LogMsg("Can't compare sprite script, sprite not active.");
				}
				return(0);
			}

			return(0);
		}

		if (compare(ev[1], (char*)"sp_y"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = change_sprite(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].y);
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}


		if (compare(ev[1], (char*)"sp_timing"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = change_sprite(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].timer);
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}


		if (compare(ev[1], (char*)"return;"))
		{
			if (g_script_debug_mode) LogMsg("Found return; statement");

			if (g_scriptInstance[script]->proc_return != 0)
			{
				g_dglos.bKeepReturnInt = true;
				run_script(g_scriptInstance[script]->proc_return);
				kill_script(script);
			}
			return(2);
		} 


		//redink1 - sets font color
		if (compare(ev[1], (char*)"set_font_color"))
		{
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,1,1,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				if (g_nlist[0] >= 1 && g_nlist[0] <= 15 &&
					g_nlist[1] >= 0 && g_nlist[1] <= 255 &&
					g_nlist[2] >= 0 && g_nlist[2] <= 255 &&
					g_nlist[3] >= 0 && g_nlist[3] <= 255)
				{
					g_dglos.font_colors[g_nlist[0]].red   = g_nlist[1];
					g_dglos.font_colors[g_nlist[0]].green = g_nlist[2];
					g_dglos.font_colors[g_nlist[0]].blue  = g_nlist[3];
				}
			}

			strcpy(pLineIn, h);  
			return(0);
		}

		//redink1 - clears the editor information, useful for save games and such
		if (compare(ev[1], (char*)"clear_editor_info"))
		{
			h = &h[strlen(ev[1])];
			for (int i = 0; i < 769; i++)
			{
				for (int j = 0; j < 100; j++)
				{
					g_dglos.g_playerInfo.spmap[i].seq[j] = 0;
					g_dglos.g_playerInfo.spmap[i].frame[j] = 0;
					g_dglos.g_playerInfo.spmap[i].type[j] = 0;
					g_dglos.g_playerInfo.spmap[i].last_time = 0;
				}
			}
			g_dglos.g_returnint = 1;
			return(0);
		}

		//redink1 - returns the number of variables used
		if (compare(ev[1], (char*)"var_used"))
		{
			h = &h[strlen(ev[1])];
			int m = 0;
			for (int i = 1; i < max_vars; i++)
				if (g_dglos.g_playerInfo.var[i].active == true)
					m++;
			g_dglos.g_returnint = m;
			return(0);
		}

		//redink1 added this function to load a new map/dink.dat
		if (compare(ev[1], (char*)"load_map"))
		{
			h = &h[strlen(ev[1])];
			int32 p[20] = {2,2,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				strcpy(g_dglos.current_map,slist[0]);
				strcpy(g_dglos.current_dat,slist[1]);
				load_info();
			}

			strcpy(pLineIn, h);  
			return(0);
		}

		//redink1 added this function to load a pallete from any bmp
		if (compare(ev[1], (char*)"load_palette"))
		{
			h = &h[strlen(ev[1])];
			int32 p[20] = {2,0,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				assert(!"We don't support this");
			}
			strcpy(pLineIn, h);  
			return(0);
		}


		if (compare(ev[1], "load_tile"))
		{
			h = &h[strlen(ev[1])];
			int32 p[20] = {2,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{  
				string fName = ToLowerCaseString(slist[0]);
				int tileIndex = g_nlist[1];

				//force a reload of this tile screen (Dink loads all graphics "on demand")
				SAFE_DELETE(g_tileScreens[tileIndex]); //logically
				
				//remember this change
				strncpy(g_dglos.g_playerInfo.tile[tileIndex].file, fName.c_str(), 50); //this 50 is hardcoded in the player data
				
				//don't force it right now?
				//g_forceBuildBackgroundFromScratch = true;
																					   //BuildScreenBackground(true); //trigger full rebuild, this could be optimized by setting a flag and only doing it once...
			}

			strcpy(pLineIn, h);  
			return(0);
		}


		//redink1 added this function to change the save game 'info'
		if (compare(ev[1], (char*)"set_save_game_info"))
		{
			h = &h[strlen(ev[1])];
			int32 p[20] = {2,0,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				strcpy(g_dglos.save_game_info,slist[0]);
			}
			strcpy(pLineIn, h);  
			return(0);
		}

		//redink1 added this function to show the item screen
		if (compare(ev[1], (char*)"show_inventory"))
		{
			h = &h[strlen(ev[1])];
			g_itemScreenActive = true;
			if (!IsLargeScreen())
			{
				g_dglo.SetViewOverride(DinkGlobals::VIEW_ZOOMED);
			}

			strcpy(pLineIn, h);  
			return(0);
		}

		//redink1 added this function,, and took it away.
		/*if (compare(ev[1], (char*)"get_compatibility"))
		{
		returnint = 0;

		h = &h[strlen(ev[1])];
		int32 p[20] = {2,1,0,0,0,0,0,0,0,0};  
		if (get_parms(ev[1], script, h, p))
		{
		if (compare(slist[0],"get_compatibility"))
		{
		if (nlist[1] <= 1)
		{
		returnint = 1;
		}
		}
		}
		strcpy(s, h);  
		return(0);
		}*/

		//redink1 added this function
		if (compare(ev[1], (char*)"get_time_game"))
		{
			h = &h[strlen(ev[1])];


			g_dglos.g_returnint = (GetBaseApp()->GetGameTick()-g_dglos.time_start) / (1000*60);
			strcpy(pLineIn, h);  
			return(0);
		}

		//redink1 added this function
		if (compare(ev[1], (char*)"get_time_real"))
		{
			h = &h[strlen(ev[1])];

			h = &h[strlen(ev[1])];
			char mytime[5];
			time_t ct;
			struct tm *time_now;
			time(&ct);
			time_now = localtime(&ct);
			strftime(mytime,5,"%M",time_now);
			g_dglos.g_returnint = atoi(mytime);
			strftime(mytime,5,"%H",time_now);
			g_dglos.g_returnint += 60*atoi(mytime);
			strcpy(pLineIn, h);  
			return(0);
		}

		//redink1 added this function
		if (compare(ev[1], (char*)"get_date_year"))
		{
			h = &h[strlen(ev[1])];
			char mytime[5];
			time_t ct;
			struct tm *time_now;
			time(&ct);
			time_now = localtime(&ct);
			strftime(mytime,5,"%Y",time_now);
			g_dglos.g_returnint = atoi(mytime);
			strcpy(pLineIn, h);  
			return(0);
		}

		//redink1 added this function
		if (compare(ev[1], (char*)"get_date_month"))
		{
			h = &h[strlen(ev[1])];
			char mytime[5];
			time_t ct;
			struct tm *time_now;
			time(&ct);
			time_now = localtime(&ct);
			strftime(mytime,5,"%m",time_now);
			g_dglos.g_returnint = atoi(mytime);
			strcpy(pLineIn, h);  
			return(0);
		}

		//redink1 added this function
		if (compare(ev[1], (char*)"get_date_day"))
		{
			h = &h[strlen(ev[1])];
			char mytime[5];
			time_t ct;
			struct tm *time_now;
			time(&ct);
			time_now = localtime(&ct);
			strftime(mytime,5,"%d",time_now);
			g_dglos.g_returnint = atoi(mytime);
			strcpy(pLineIn, h);  
			return(0);
		}

		//redink1 added this function
		if (compare(ev[1], (char*)"math_abs"))
		{
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,0,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = abs(g_nlist[0]);
			}
			strcpy(pLineIn, h);  
			return(0);
		}

		//redink1 added this function
		/*if (compare(ev[1], (char*)"math_sin"))
		{
		h = &h[strlen(ev[1])];
		int32 p[20] = {1,0,0,0,0,0,0,0,0,0};  
		if (get_parms(ev[1], script, h, p))
		{
		returnint = sin((double)nlist[0]);
		}
		strcpy(s, h);  
		return(0);
		}

		//redink1 added this function
		if (compare(ev[1], (char*)"math_cos"))
		{
		h = &h[strlen(ev[1])];
		int32 p[20] = {1,0,0,0,0,0,0,0,0,0};  
		if (get_parms(ev[1], script, h, p))
		{
		returnint = cos((double)nlist[0]);
		}
		strcpy(s, h);  
		return(0);
		}

		//redink1 added this function
		if (compare(ev[1], (char*)"math_tan"))
		{
		h = &h[strlen(ev[1])];
		int32 p[20] = {1,0,0,0,0,0,0,0,0,0};  
		if (get_parms(ev[1], script, h, p))
		{
		returnint = tan((double)nlist[0]);
		}
		strcpy(s, h);  
		return(0);
		}*/

		//redink1 added this function
		if (compare(ev[1], (char*)"math_sqrt"))
		{
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,0,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = sqrt((double)abs(g_nlist[0]));
			}
			strcpy(pLineIn, h);  
			return(0);
		}

		//redink1 added this function
		if (compare(ev[1], (char*)"math_mod"))
		{
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = (g_nlist[0] % g_nlist[1]);
			}
			strcpy(pLineIn, h);  
			return(0);
		}

		if (compare(ev[1], (char*)"breakpoint"))
		{
			h = &h[strlen(ev[1])];
			int32 p[20] = {0,0,0,0,0,0,0,0,0,0};  
			assert(!"Breakpoint!");
			return(0);
		}
		//redink1
		if (compare(ev[1], (char*)"sp_custom"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {2,1,1,0,0,0,0,0,0,0};  
			if ( get_parms(ev[1], script, h, p) && g_sprite[g_nlist[1]].active == true )
			{
				if ( g_nlist[1] < 1 || g_sprite[g_nlist[1]].active == false )
				{
					g_dglos.g_returnint = -1;
				}
				else
				{
					// If key doesn't exist, create it.
					if ( g_customSpriteMap[g_nlist[1]]->find( slist[0] ) == g_customSpriteMap[g_nlist[1]]->end() )
					{
						g_customSpriteMap[g_nlist[1]]->insert( std::make_pair( slist[0], 0 ) );
					}

					// Set the value
					if ( g_nlist[2] != -1 )
					{
						g_customSpriteMap[g_nlist[1]]->erase( slist[0] );
						g_customSpriteMap[g_nlist[1]]->insert( std::make_pair( slist[0], g_nlist[2] ) );
					}

					g_dglos.g_returnint = g_customSpriteMap[g_nlist[1]]->find( slist[0] )->second;
				}
				return(0);
			}
			g_dglos.g_returnint = -1;
			return(0);
		}

		//redink1
		if (compare(ev[1], (char*)"sp_blood_seq"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				change_sprite(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].bloodseq);

				g_dglos.g_returnint = g_sprite[g_nlist[0]].bloodseq;

				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}

		//redink1
		if (compare(ev[1], (char*)"sp_blood_num"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				change_sprite(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].bloodnum);

				g_dglos.g_returnint = g_sprite[g_nlist[0]].bloodseq;

				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}

		//redink1 added to get index of specified item
		if (compare(ev[1], (char*)"get_item"))
		{
			h = &h[strlen(ev[1])];
			int32 p[20] = {2,0,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = 0;
				for (int i = 1; i < C_DINK_MAX_ITEMS + 1; i++)
				{
					if (g_dglos.g_playerInfo.g_itemData[i].active)
					{
						if (compare(g_dglos.g_playerInfo.g_itemData[i].name, slist[0]))
						{
							g_dglos.g_returnint = i;
							break;
						}
					}
				}
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}

		//redink1 added to get index of specified magic spell
		if (compare(ev[1], (char*)"get_magic"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {2,0,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				g_dglos.g_returnint = 0;
				for (int i = 1; i < 9; i++)
				{
					if (g_dglos.g_playerInfo.g_MagicData[i].active)
					{
						if (compare(g_dglos.g_playerInfo.g_MagicData[i].name, slist[0]))
						{
							g_dglos.g_returnint = i;
							break;
						}
					}
				}
				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}

		//redink1 clip stuff
		if (compare(ev[1], (char*)"sp_clip_left"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				change_sprite(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].alt.left);

				g_dglos.g_returnint = g_sprite[g_nlist[0]].alt.left;

				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}

		//redink1 clip stuff
		if (compare(ev[1], (char*)"sp_clip_top"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				change_sprite(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].alt.top);

				g_dglos.g_returnint = g_sprite[g_nlist[0]].alt.top;

				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}

		//redink1 clip stuff
		if (compare(ev[1], (char*)"sp_clip_right"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				change_sprite(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].alt.right);

				g_dglos.g_returnint = g_sprite[g_nlist[0]].alt.right;

				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}

		//redink1 clip stuff
		if (compare(ev[1], (char*)"sp_clip_bottom"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				change_sprite(g_nlist[0], g_nlist[1], &g_sprite[g_nlist[0]].alt.bottom);

				g_dglos.g_returnint = g_sprite[g_nlist[0]].alt.bottom;

				return(0);
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}

		//redink1 added so developers can change or see what tile is at any given position
		if (compare(ev[1], (char*)"map_tile"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				//Yeah... they can only modify valid tiles
				if (g_nlist[0] > 0 && g_nlist[0] <= 96)
				{
					//Only change the value if it is greater than 0...
					if (g_nlist[1] > 0)
					{
						g_dglos.g_smallMap.t[g_nlist[0]-1].num = g_nlist[1];
					}
					g_dglos.g_returnint = g_dglos.g_smallMap.t[g_nlist[0]-1].num;
					return(0);
				}
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}

		//redink1 added so a developer can retrieve/modify a hard tile
		if (compare(ev[1], (char*)"map_hard_tile"))
		{     
			h = &h[strlen(ev[1])];
			int32 p[20] = {1,1,0,0,0,0,0,0,0,0};  
			if (get_parms(ev[1], script, h, p))
			{
				//Yeah... they can only modify valid tiles
				if (g_nlist[0] > 0 && g_nlist[0] <= 96)
				{
					//Only change the value if it is greater than 0...
					if (g_nlist[1] > 0)
					{
						g_dglos.g_smallMap.t[g_nlist[0]-1].althard = g_nlist[1];
					}
					g_dglos.g_returnint = g_dglos.g_smallMap.t[g_nlist[0]-1].althard;
					return(0);
				}
			}
			g_dglos.g_returnint =  -1;
			return(0);
		}


		if (compare(ev[2], "+="))
		{
			h = &h[strlen(ev[1])];
			strip_beginning_spaces(h);
			h = &h[2];
			strip_beginning_spaces(h);
			var_equals(ev[1], ev[3], '+', script, h);
			strcpy(pLineIn, h);  
			return(0);
		}

		if (compare(ev[2], "*="))
		{
			h = &h[strlen(ev[1])];
			strip_beginning_spaces(h);
			h = &h[2];
			strip_beginning_spaces(h);
			var_equals(ev[1], ev[3], '*', script, h);
			strcpy(pLineIn, h);  
			return(0);
		}



		if (compare(ev[2], "-="))
		{
			h = &h[strlen(ev[1])];
			strip_beginning_spaces(h);
			h = &h[2];
			strip_beginning_spaces(h);

			var_equals(ev[1], ev[3], '-', script, h);

			strcpy(pLineIn, h);  
			return(0);
		}


		if (compare(ev[2], "/") || compare(ev[2], "/="))
		{
			h = &h[strlen(ev[1])];
			strip_beginning_spaces(h);
			h = &h[1];
			strip_beginning_spaces(h);

			var_equals(ev[1], ev[3], '/', script, h);

			strcpy(pLineIn, h);  
			return(0);
		}

		if (compare(ev[2], "*") || compare(ev[2], "*="))
		{
			h = &h[strlen(ev[1])];
			strip_beginning_spaces(h);
			h = &h[1];
			strip_beginning_spaces(h);

			var_equals(ev[1], ev[3], '*', script, h);

			strcpy(pLineIn, h);  
			return(0);
		}
		if (compare(ev[1], (char*)"external"))
		{
			h = &h[strlen(ev[1])];
			int32 p[20] = {2,2,1,1,1,1,1,1,1,1};
			for (int i = 0; i < 10; i++) slist[i][0] = 0;
			get_parms(ev[1], script, h, p);
			if (slist[0][0] && slist[1][0])
			{
				int myscript1 = load_script(slist[0],g_scriptInstance[script]->sprite, false);
				if (myscript1 == 0)
				{
					LogMsg("Error:  Couldn't find %s.c (for procedure %s)", slist[0], slist[1]);
					return(0);
				}
				g_scriptInstance[myscript1]->arg1 = g_nlist[2];
				g_scriptInstance[myscript1]->arg2 = g_nlist[3];
				g_scriptInstance[myscript1]->arg3 = g_nlist[4];
				g_scriptInstance[myscript1]->arg4 = g_nlist[5];
				g_scriptInstance[myscript1]->arg5 = g_nlist[6];
				g_scriptInstance[myscript1]->arg6 = g_nlist[7];
				g_scriptInstance[myscript1]->arg7 = g_nlist[8];
				g_scriptInstance[myscript1]->arg8 = g_nlist[9];
				if (locate( myscript1, slist[1]))
				{
					g_scriptInstance[myscript1]->proc_return = script;
					run_script(myscript1);    

					return(2);
				} else 
				{
					LogMsg("Error:  Couldn't find procedure %s in %s.", slist[1], slist[0]);
					kill_script(myscript1);
				}
			}    
			strcpy(pLineIn, h);  
			return(0);
		}


		if (strchr(h, '(') != NULL)
		{

			//lets attempt to run a procedure

			int myscript = load_script(g_scriptInstance[script]->name, g_scriptInstance[script]->sprite, false);

			h = &h[strlen(ev[1])];

			int32 p[20] = {1,1,1,1,1,1,1,1,1,1};
			get_parms(ev[1], script, h, p);

			if (locate( myscript, ev[1]))
			{
				g_scriptInstance[myscript]->arg1 = g_nlist[0];
				g_scriptInstance[myscript]->arg2 = g_nlist[1];
				g_scriptInstance[myscript]->arg3 = g_nlist[2];
				g_scriptInstance[myscript]->arg4 = g_nlist[3];
				g_scriptInstance[myscript]->arg5 = g_nlist[4];
				g_scriptInstance[myscript]->arg6 = g_nlist[5];
				g_scriptInstance[myscript]->arg7 = g_nlist[6];
				g_scriptInstance[myscript]->arg8 = g_nlist[7];
				g_scriptInstance[myscript]->arg9 = g_nlist[8];

				g_scriptInstance[myscript]->proc_return = script;
				run_script(myscript);    
				return(2);
			} else
			{
				for (int i = 0; strlen(g_dglos.g_playerInfo.func[i].func) > 0 && i < 100; i++)
				{
					if (compare(g_dglos.g_playerInfo.func[i].func, ev[1]))
					{
						myscript = load_script(g_dglos.g_playerInfo.func[i].file, g_scriptInstance[script]->sprite, false);
						g_scriptInstance[myscript]->arg1 = g_nlist[0];
						g_scriptInstance[myscript]->arg2 = g_nlist[1];
						g_scriptInstance[myscript]->arg3 = g_nlist[2];
						g_scriptInstance[myscript]->arg4 = g_nlist[3];
						g_scriptInstance[myscript]->arg5 = g_nlist[4];
						g_scriptInstance[myscript]->arg6 = g_nlist[5];
						g_scriptInstance[myscript]->arg7 = g_nlist[6];
						g_scriptInstance[myscript]->arg8 = g_nlist[7];
						g_scriptInstance[myscript]->arg9 = g_nlist[8];
						if (locate(myscript, ev[1]))
						{
							g_scriptInstance[myscript]->proc_return = script;
							run_script(myscript);    
							return(2);
						}
						break;
					}
				}
				LogMsg("ERROR:  Procedure void %s( void ); not found in script %s. (word 2 was %s) ", line,
					ev[2], g_scriptInstance[myscript]->name); 
				kill_script(myscript);          
			}

			return(0);

		}

		LogMsg("MERROR: \"%s\" unknown in %s, offset %d.",ev[1], g_scriptInstance[script]->name,g_scriptInstance[script]->current);

	}

bad:
	strcpy(pLineIn, h);
	return(0);

good:
	strcpy(pLineIn, h);
	//s = h
	//Msg("ok, continuing with running %s..",s);
	return(1);
}


void run_script (int script)
{
	int result;
	char line[512];

	line[0] = 0;

	if (g_dglos.bKeepReturnInt)
	{
		g_dglos.bKeepReturnInt = false;
	}
	else
	{
		g_dglos.g_returnint = 0;
	}
	returnstring[0] = 0;
	if (g_scriptInstance[script] != NULL)
	{
		if (g_script_debug_mode)
			LogMsg("Script %s is entered at offset %d.", g_scriptInstance[script]->name, g_scriptInstance[script]->current);
	} else
	{
		LogMsg("Error:  Tried to run a script that doesn't exist in memory.  Nice work.");
	}

	while(read_next_line(script, line))
	{
		while(1)
		{

			/*
			if (ScriptEOF(script)) 
			{
				if (g_scriptInstance[script]->proc_return != 0)
				{
					run_script(g_scriptInstance[script]->proc_return);
					kill_script(script);
				}
				return;
			}
			*/

			strip_beginning_spaces(line);
			if (compare(line, "\n")) break;

			result = process_line(script,line, false);
			if (result == 3) 
			{
redo:
				if (!read_next_line(script, line))
				{
					//we've reached the end of the script
					return;
				}
crappa:
				strip_beginning_spaces(line);
				if (compare(line, "\n")) goto redo;
				if (compare(line, "\\\\")) goto redo;
				//       Msg("processing %s knowing we are going to skip it...", line);
				result = process_line(script,line, true);
			}

			if (result == 5) goto crappa;

			if (result == 3) 
			{

				goto redo;
			}

			if (result == 2) 
			{
				if (g_script_debug_mode) LogMsg("giving script the boot");
				//quit script
				return;
			}
			if (result == 0) break;

			if (result == 4) 
			{
				//   Msg("Was sent %s, length %d", line, strlen(line));

				if (strlen(line) < 2)
				{
redo2:
					if (!read_next_line(script, line))
					{
						//script is actually empty
						break;
					}
					strip_beginning_spaces(line);
					//Msg("Comparing to %s.", line);
					if (compare(line, "\n")) goto redo2;
					if (compare(line, "\\\\")) goto redo2;
				}
				result = process_line(script,line, true);
			}


			if (result == 2) 
			{
				if (g_script_debug_mode) LogMsg("giving script the boot");
				//quit script
				return;
			}
			if (result == 0) break;
		}
	}

	if (g_scriptInstance[script] != NULL)
	{
		if (g_scriptInstance[script]->proc_return != 0)
		{
			run_script(g_scriptInstance[script]->proc_return);
			kill_script(script);
		}
	}
}


void process_callbacks(void)
{
	uint32 thist = (int)GetBaseApp()->GetGameTick();

	for (int i = 1; i < C_MAX_SCRIPTS; i++)
	{
		if (g_scriptInstance[i] != NULL)
		{
			if (g_scriptInstance[i]->sprite > 0) if (g_scriptInstance[i]->sprite != 1000) if (g_sprite[g_scriptInstance[i]->sprite].active == false) 
			{
				//kill this script, owner is dead
				if (g_script_debug_mode)
					LogMsg("Killing script Set%s, owner sprite %d is dead.",g_scriptInstance[i]->name, g_scriptInstance[i]->sprite);
				kill_script(i);
			}

		}
	}


	for (int k = 1; k < C_MAX_SCRIPT_CALLBACKS; k++)
	{
		if (g_dglos.g_scriptCallback[k].active)
		{

			if (g_dglos.g_scriptCallback[k].owner > 0) if (g_scriptInstance[g_dglos.g_scriptCallback[k].owner] == NULL)
			{
				//kill this process, it's owner sprite is 'effin dead.
				if (g_script_debug_mode) LogMsg("Killed callback %s because script %d is dead.",
					k, g_dglos.g_scriptCallback[k].owner);
				g_dglos.g_scriptCallback[k].active = false;

			} else
			{

				if (g_dglos.g_scriptCallback[k].timer == 0)
				{
					//set timer

					if (g_dglos.g_scriptCallback[k].max > 0)    
						g_dglos.g_scriptCallback[k].timer = thist + (rand() % g_dglos.g_scriptCallback[k].max)+g_dglos.g_scriptCallback[k].min;
					else g_dglos.g_scriptCallback[k].timer = thist + g_dglos.g_scriptCallback[k].min;

				} else
				{

					if (g_dglos.g_scriptCallback[k].timer < thist)
					{
						g_dglos.g_scriptCallback[k].timer = 0;
						//g_dglos.g_scriptCallback[k].active = false; //SETH Adding this fixes one prob, but breaks others?
						if (compare(g_dglos.g_scriptCallback[k].name, ""))
						{
							//callback defined no proc name, so lets assume they want to start the script where it   
							//left off
							//kill this callback
							g_dglos.g_scriptCallback[k].active = false;
							run_script(g_dglos.g_scriptCallback[k].owner);
							if (g_script_debug_mode) LogMsg("Called script %d with callback %d.", g_dglos.g_scriptCallback[k].owner, k);

						} else
						{

							if (g_script_debug_mode) LogMsg("Called proc %s with callback %d.", g_dglos.g_scriptCallback[k].name, k);

							//callback defined a proc name
							if (locate(g_dglos.g_scriptCallback[k].owner,g_dglos.g_scriptCallback[k].name))
							{
								
								//found proc, lets run it    
								run_script(g_dglos.g_scriptCallback[k].owner);

							}

						}

					}

				}
			}
		}
	}

}


void init_scripts(void)
{
	for (int k = 1; k < C_MAX_SCRIPTS; k++)
	{
		if (g_scriptInstance[k] != NULL && g_scriptInstance[k]->sprite != 0 && g_scriptInstance[k]->sprite < C_MAX_SPRITES_AT_ONCE && g_sprite[g_scriptInstance[k]->sprite].active )
		{
			if (locate(k,"main"))
			{
#ifdef _DEBUG
				//LogMsg("Screendraw: running main of script %s..", g_scriptInstance[k]->name);
#endif
				run_script(k);
			}
		}
	}

}

//redink1 added for font colors
void init_font_colors(void)
{
	//Light Magenta
	g_dglos.font_colors[1].red   = 255;
	g_dglos.font_colors[1].green = 198;
	g_dglos.font_colors[1].blue  = 255;

	//Dark Green
	g_dglos.font_colors[2].red   = 131;
	g_dglos.font_colors[2].green = 181;
	g_dglos.font_colors[2].blue  = 74;

	//Bold Cyan
	g_dglos.font_colors[3].red   = 99;
	g_dglos.font_colors[3].green = 242;
	g_dglos.font_colors[3].blue  = 247;

	//Orange
	g_dglos.font_colors[4].red   = 255;
	g_dglos.font_colors[4].green = 156;
	g_dglos.font_colors[4].blue  = 74;

	//Magenta
	g_dglos.font_colors[5].red   = 222;
	g_dglos.font_colors[5].green = 173;
	g_dglos.font_colors[5].blue  = 255;

	//Brown Orange
	g_dglos.font_colors[6].red   = 244;
	g_dglos.font_colors[6].green = 188;
	g_dglos.font_colors[6].blue  = 73;

	//Light Gray
	g_dglos.font_colors[7].red   = 173;
	g_dglos.font_colors[7].green = 173;
	g_dglos.font_colors[7].blue  = 173;

	//Dark Gray
	g_dglos.font_colors[8].red   = 85;
	g_dglos.font_colors[8].green = 85;
	g_dglos.font_colors[8].blue  = 85;

	//Sky Blue
	g_dglos.font_colors[9].red   = 148;
	g_dglos.font_colors[9].green = 198;
	g_dglos.font_colors[9].blue  = 255;

	//Bright Green
	g_dglos.font_colors[10].red   = 0;
	g_dglos.font_colors[10].green = 255;
	g_dglos.font_colors[10].blue  = 0;

	//Yellow
	g_dglos.font_colors[11].red   = 255;
	g_dglos.font_colors[11].green = 255;
	g_dglos.font_colors[11].blue  = 2;

	//Yellow
	g_dglos.font_colors[12].red   = 255;
	g_dglos.font_colors[12].green = 255;
	g_dglos.font_colors[12].blue  = 2;

	//Hot Pink
	g_dglos.font_colors[13].red   = 255;
	g_dglos.font_colors[13].green = 132;
	g_dglos.font_colors[13].blue  = 132;

	//Yellow
	g_dglos.font_colors[14].red   = 255;
	g_dglos.font_colors[14].green = 255;
	g_dglos.font_colors[14].blue  = 2;

	//White
	g_dglos.font_colors[15].red   = 255;
	g_dglos.font_colors[15].green = 255;
	g_dglos.font_colors[15].blue  = 255;
}

void text_draw(int h)
{

    char crap[512];
    char *cr;
    rtRect32 rcRect;
    int color = 15;

	int maxX = 620;
	int minX = 0;

	if (g_dglo.m_curView == DinkGlobals::VIEW_ZOOMED)
	{	
		//maxX = 600;
		minX = 20;
	}
    if (g_sprite[h].damage == -1)
    {
        //redink1 fix for : and '%deee bugs?
        strcpy(crap, g_sprite[h].text);
        //sprintf(crap, "%s", spr[h].text);
        cr = &crap[0];
        color = 14;
        while( cr[0] == '`') 
        {
            //color code at top
            if (cr[1] == '#') color = 13;
            if (cr[1] == '1') color = 1;
            if (cr[1] == '2') color = 2;
            if (cr[1] == '3') color = 3;
            if (cr[1] == '5') color = 5;
            if (cr[1] == '6') color = 6;
            if (cr[1] == '7') color = 7;
            if (cr[1] == '8') color = 8;
            if (cr[1] == '9') color = 9;
            if (cr[1] == '0') color = 10;
            if (cr[1] == '$') color = 14;
            if (cr[1] == '%') color = 15;
            //redink1 support for additional colors
            if (cr[1] == '@') color = 12;
            if (cr[1] == '!') color = 11;

            if (cr[1] == '4') color = 4;
            cr = &cr[2];
        }

        //Msg("Final is %s.",cr);
        if (g_sprite[h].owner == 1000)
        {
            rcRect = rtRect32 (g_sprite[h].x,g_sprite[h].y,g_sprite[h].x+620,g_sprite[h].y+400);
        } else
        {

           rcRect = rtRect32 (g_sprite[h].x,g_sprite[h].y,g_sprite[h].x+150,g_sprite[h].y+150);

		
            if (g_sprite[h].x+150 > maxX)
                OffsetRect(&rcRect, ((g_sprite[h].x+150)-maxX) - (((g_sprite[h].x+150)-maxX) * 2), 0);
        
		
			if (rcRect.left < minX && rcRect.left >= 0)
			{
					OffsetRect(&rcRect, minX - rcRect.left, 0);
			}
		}

    } else
    {

        sprintf(crap, "%d", g_sprite[h].damage);
        cr = &crap[0];
        if (g_sprite[h].brain_parm == 5000)  color = 14;


        if (g_sprite[h].y < minX) g_sprite[h].y = minX;
        rcRect = rtRect32 (g_sprite[h].x,g_sprite[h].y,g_sprite[h].x+50 ,g_sprite[h].y+50);
    }       

	
/*   
	if (g_dglos.bFadedDown || g_dglos.process_downcycle)
        color = 15;
		*/

	uint32 rgbColor = MAKE_RGBA(g_dglos.font_colors[color].red, g_dglos.font_colors[color].green, g_dglos.font_colors[color].blue, 255);

	uint32 bgColor = MAKE_RGBA(0,0,0,100);
	
    rtRect rTemp(rcRect);
    
   // SetTextColor(hdc,RGB(8,14,21));
    if (g_sprite[h].owner == 1200)
    {
		GetApp()->GetFont(FONT_SMALL)->DrawWrapped(rTemp, cr, false, false, rgbColor, g_dglo.m_fontSize, false, bgColor);
    } else
    {
		
		if (StripWhiteSpace(cr) == "")
		{
			//skip it, it's just blank, otherwise it will draw the bg which looks dumb
		} else
		{
			GetApp()->GetFont(FONT_SMALL)->DrawWrapped(rTemp, cr, true, false, rgbColor,  g_dglo.m_fontSize, false , bgColor);
		}
	}

    OffsetRect(&rTemp,0,1);

}

void get_last_sprite(void)
{

    for (int i = C_MAX_SPRITES_AT_ONCE - 1; i > 2; i--)
    {

        if (g_sprite[i].active)
        {
            g_dglos.last_sprite_created = i;
            //   Msg("last sprite created is %d.", i);
            return;
        }
    }
}

bool DestroySound( void )
{
/*
	uint32       idxKill;

    for( idxKill = 0; idxKill < max_sounds; idxKill++ )
    {
        SoundDestroyEffect( idxKill );
    }
  
    return true;
	*/
	return true;

} /* DestroySound */

/*
* SoundDestroyEffect
*
* Frees up resources associated with a sound effect
*/
bool SoundDestroyEffect( int sound )
{
/*
	if(g_soundInfo[sound].sound)
    {
        delete g_soundInfo[sound].sound;
		g_soundInfo[sound].sound = NULL;
    }
    return true;
*/

	return true;
} 

#ifdef C_DINK_KEYBOARD_INPUT

short GetKeyboard(int key)
{
	// returns 0 if the key has been depressed, else returns 1 and sets key to code recd.
	return (GetAsyncKeyState(key) & 0x8000);
}

BOOL keypressed( void )
{
	for (int x=0; x<256; x++)
    {
	    if (GetKeyboard(x) > 0)
        {
            return(TRUE);
        }
    }
    return(FALSE);
}

#endif

#ifdef WINAPI
extern bool g_bAppFinished;
#endif
void CheckForHotkeys()
{


#ifdef C_DINK_KEYBOARD_INPUT
#ifdef WINAPI
	if ((GetKeyboard('Q')) && (GetKeyboard(18)))
	{
		if (g_DebugKeyTimer < GetApp()->GetTick())
		{
			g_DebugKeyTimer = GetApp()->GetTick() + 500;

			LogMsg("Pressed Alt-Q, shutting down");

			g_bAppFinished = true;
		}
	}


	if ((GetKeyboard(68)) && (GetKeyboard(18)))
	{
		if (g_DebugKeyTimer < GetApp()->GetTick())
		{
			g_DebugKeyTimer = GetApp()->GetTick() + 500;

			if (g_script_debug_mode)
			{
				g_script_debug_mode = false;
				ShowQuickMessage("Script debug/extra logging off");
			}
			else
			{
				g_script_debug_mode = true;
				ShowQuickMessage("Script debug/extra logging on");
			}

		}
	}

#endif
#endif
}


void check_joystick(void)
{
    
    for (int e2=1; e2 <=10; e2++) 
    {
        sjoy.joybit[e2] = false;
    }

	sjoy.right = false;
    sjoy.left = false;
    sjoy.up = false;
    sjoy.down = false;

    sjoy.rightd = false;
    sjoy.leftd = false;
    sjoy.upd = false;
    sjoy.downd = false;

	for (int i=0; i < 7; i++)
	{
		if (g_dglo.m_dirInput[DINK_INPUT_BUTTON1+i]) sjoy.joybit[1+i] = true;
	}

     for (int x5=1; x5 <=10; x5++) sjoy.button[x5] = false; 

    for (int x=1; x <=10; x++)
    {
        if (sjoy.joybit[x])
        {
            if (sjoy.letgo[x] == true) 
            {
                sjoy.button[x] = true;
                sjoy.letgo[x] = false;
            }
        }
    }

	if (g_dglo.m_dirInput[DINK_INPUT_LEFT]) sjoy.left = true;
	if (g_dglo.m_dirInput[DINK_INPUT_RIGHT]) sjoy.right = true;
	if (g_dglo.m_dirInput[DINK_INPUT_UP]) sjoy.up = true;
	if (g_dglo.m_dirInput[DINK_INPUT_DOWN]) sjoy.down = true;
	


    for (int x2=1; x2 <=10; x2++) 
    {
        if (sjoy.joybit[x2])  sjoy.letgo[x2] = false; else sjoy.letgo[x2] = true;
    }

    if (sjoy.right) if (sjoy.rightold == true)
    {
        sjoy.rightd = true;
        sjoy.rightold = false;
    }

    if (sjoy.right) sjoy.rightold = false; else sjoy.rightold = true;


    if (sjoy.left) if (sjoy.leftold == true)
    {
        sjoy.leftd = true;
        sjoy.leftold = false;
    }

    if (sjoy.left) sjoy.leftold = false; else sjoy.leftold = true;


    if (sjoy.up) if (sjoy.upold == true)
    {
        sjoy.upd = true;
        sjoy.upold = false;
    }

    if (sjoy.up) sjoy.upold = false; else sjoy.upold = true;


    if (sjoy.down) if (sjoy.downold == true)
    {
        sjoy.downd = true;
        sjoy.downold = false;
    }

    if (sjoy.down) sjoy.downold = false; else sjoy.downold = true;


    if (g_dglos.g_wait_for_button.active)
    {

        //check for dirs

        if (sjoy.rightd) g_dglos.g_wait_for_button.button = 16;
        if (sjoy.leftd) g_dglos.g_wait_for_button.button = 14;
        if (sjoy.upd) g_dglos.g_wait_for_button.button = 18;
        if (sjoy.downd) g_dglos.g_wait_for_button.button = 12;

        sjoy.rightd = false;
        sjoy.downd = false;
        sjoy.upd = false;
        sjoy.leftd = false;

        //check buttons
        for (int ll=1; ll <= 10; ll++)
        {
            if (sjoy.button[ll])
            {
                //button was pressed
                g_dglos.g_wait_for_button.button = ll;

            }
            sjoy.button[ll] = false;

        }

        if (g_dglos.g_wait_for_button.button != 0)
        {
            *presult = g_dglos.g_wait_for_button.button;
            g_dglos.g_wait_for_button.active = false;
            run_script(g_dglos.g_wait_for_button.script);
        }

    }
	for (int i=0; i < DINK_INPUT_COUNT; i++)
	{
		if (g_dglo.m_dirInputFinished[i]) 
		{
			g_dglo.m_dirInput[i] = false;
			g_dglo.m_dirInputFinished[i] = false;

		}
	}


}


// ********* CHECK TO SEE IF THIS CORD IS ON A HARD SPOT *********
bool not_in_this_base(int seq, int base)
{
    int realbase = (seq / 10) * 10;

	if (realbase != base)
    {
        return(true); 
    } else
    {
        return(false);
    }
}

bool in_this_base(int seq, int base)
{

    int realbase = (seq / 10) * 10;
    if (realbase == base)
    {
        //  Msg("TRUE - Ok, realbase is %d, compared to the base, which is %d.", realbase, base);
        return(true); 
    }
    else
    {
        //  Msg("FALSE - Ok, realbase is %d, compared to the base, which is %d.", realbase, base);
        return(false);
    }
}


void automove (int j)
{
    char kindx,kindy;
    int speedx = 0;
    int speedy = 0;

    if (g_sprite[j].mx != 0)
    { 
        if (g_sprite[j].mx < 0)
            kindx = '-'; else kindx = '+';
        if (kindx == '-') speedx = (g_sprite[j].mx - (g_sprite[j].mx * 2)); else
            speedx = g_sprite[j].mx;
    } else kindx = '0';

    if (g_sprite[j].my != 0)
    { 
        if (g_sprite[j].my < 0)
            kindy = '-'; else kindy = '+';
        if (kindy == '-') speedy = (g_sprite[j].my - (g_sprite[j].my * 2)); else
            speedy = g_sprite[j].my;

    } else kindy = '0';

    int speed = speedx;
    if (speedy > speedx) speed = speedy;
    if (speed > 0)
        move(j,speed,kindx,kindy);
    //move(j, 1, '+','+'); 

}


int autoreverse(int j)
{
    //Msg("reversing die %d",spr[j].dir);
    int r = ((rand() % 2)+1);   
    if ( (g_sprite[j].dir == 1) || (g_sprite[j].dir == 2) ) 
    {
        if (r == 1)
            return(8);
        if (r == 2)
            return(6);

    }

    if ( (g_sprite[j].dir == 3) || (g_sprite[j].dir == 6) ) 
    {
        if (r == 1)
            return(2);
        if (r == 2)

            return(4);

    }

    if ( (g_sprite[j].dir == 9) || (g_sprite[j].dir == 8) ) 
    {
        if (r == 1)
            return(2);
        if (r == 2)

            return(6);


    }

    if ( (g_sprite[j].dir == 7) || (g_sprite[j].dir == 4) ) 
    {
        if (r == 1)
            return(8);
        if (r == 2)
            return(6);

    }

    return(0);
}


int autoreverse_diag(int j)
{
    if (g_sprite[j].dir == 0) g_sprite[j].dir = 7;
    int r = ((rand() % 2)+1);   

    if ( (g_sprite[j].dir == 1) || (g_sprite[j].dir == 3) ) 
    {

        if (r == 1)
            return(9);
        if (r == 2)
            return(7);
    }

    if ( (g_sprite[j].dir == 3) || (g_sprite[j].dir == 6) ) 
    {
        if (r == 1)
            return(7);
        if (r == 2)
            return(1);

    }

    if ( (g_sprite[j].dir == 9) || (g_sprite[j].dir == 8) ) 
    {
        if (r == 1)
            return(1);
        if (r == 2)
            return(7);
    }

    if ( (g_sprite[j].dir == 7) || (g_sprite[j].dir == 4) ) 
    {
        if (r == 1)
            return(3);
        if (r == 2)
            return(9);

    }

#ifdef _DEBUG
	//LogMsg("Auto Reverse Diag was sent a dir %d sprite, base %d walk.",g_sprite[j].dir, g_sprite[j].base_walk);
#endif
    return(0);
}

void draw_damage(int h)
{

    int crap2 = add_sprite(g_sprite[h].x,g_sprite[h].y,8,0,0);

    g_sprite[crap2].y -= g_dglos.g_picInfo[g_dglos.g_seq[g_sprite[h].pseq].frame[g_sprite[h].pframe]].yoffset;
    g_sprite[crap2].x -= g_dglos.g_picInfo[g_dglos.g_seq[g_sprite[h].pseq].frame[g_sprite[h].pframe]].xoffset;
    g_sprite[crap2].y -= g_dglos.g_picInfo[g_dglos.g_seq[g_sprite[h].pseq].frame[g_sprite[h].pframe]].box.bottom / 3;
    g_sprite[crap2].x += g_dglos.g_picInfo[g_dglos.g_seq[g_sprite[h].pseq].frame[g_sprite[h].pframe]].box.right / 5;

    g_sprite[crap2].speed = 1;
    g_sprite[crap2].hard = 1;
    g_sprite[crap2].brain_parm = h;  
    g_sprite[crap2].my = -1;
    g_sprite[crap2].kill = 1000;
    g_sprite[crap2].dir = 8;
    g_sprite[crap2].damage = g_sprite[h].damage;
}


void add_kill_sprite(int h)
{
    if ( (g_sprite[h].dir > 9) || (g_sprite[h].dir < 1) )
    {
        LogMsg("Error:  Changing sprites dir from %d (!?) to 3.", g_sprite[h].dir);
        g_sprite[h].dir = 3;
    }

    int dir = g_sprite[h].dir;
    int base = g_sprite[h].base_die;

    //Msg("Base die is %d", base);
    if (base == -1) 
    {

        if (g_dglos.g_seq[g_sprite[h].base_walk+5].active != 0)
        {
            add_exp(g_sprite[h].exp, h);

            int crap2 = add_sprite(g_sprite[h].x,g_sprite[h].y,5,g_sprite[h].base_walk +5,1);
            g_sprite[crap2].speed = 0;
            g_sprite[crap2].seq = g_sprite[h].base_walk + 5;
            //redink1 added this so corpses are the same size
            g_sprite[crap2].size = g_sprite[h].size;
            return;
        } else
        {
            dir = 0;
            base = 164;
        }
    }


    if (g_dglos.g_seq[base+dir].active == false)
    {  

        if (dir == 1) dir = 9;
        else if (dir == 3) dir = 7;         
        else if (dir == 7) dir = 3;         
        else if (dir == 9) dir = 1;         

        else if (dir == 4) dir = 6;         
        else if (dir == 6) dir = 4;         
        else if (dir == 8) dir = 2;         
        else if (dir == 2) dir = 8;         
    }

	if (g_dglos.g_seq[base+dir].active == false)
    {
        LogMsg("Can't make a death sprite for dir %d!", base+dir);
    }

    int crap2 = add_sprite(g_sprite[h].x,g_sprite[h].y,5,base +dir,1);
    g_sprite[crap2].speed = 0;
    g_sprite[crap2].base_walk = 0;
    g_sprite[crap2].seq = base + dir;

    if (base == 164) g_sprite[crap2].brain = 7;

    g_sprite[crap2].size = g_sprite[h].size;

    add_exp(g_sprite[h].exp, h);
}

void done_moving(int h)
{
    g_sprite[h].move_active = false;
    g_sprite[h].move_nohard = false;

    if (g_sprite[h].move_script > 0)
    {
        //  Msg("mover running script %d..", spr[h].move_script);
        run_script(g_sprite[h].move_script);
    }
}

int get_distance_and_dir(int h, int h1, int *dir)
{
    if ( g_dglos.smooth_follow )
    {
        unsigned int x_diff( abs( g_sprite[h].x - g_sprite[h1].x ) );
        unsigned int y_diff( abs( g_sprite[h].y - g_sprite[h1].y ) );
        if ( g_sprite[h].x < g_sprite[h1].x )
        {
            if ( g_sprite[h].y < g_sprite[h1].y )
            {
                // 6, 3, 2
                if ( y_diff * 4 < x_diff )
                {
                    *dir = 6;
                }
                else if ( x_diff * 4 < y_diff )
                {
                    *dir = 2;
                }
                else
                {
                    *dir = 3;
                }
            }
            else if ( g_sprite[h].y > g_sprite[h1].y )
            {
                // 4, 9, 8
                if ( y_diff * 4 < x_diff )
                {
                    *dir = 6;
                }
                else if ( x_diff * 4 < y_diff )
                {
                    *dir = 8;
                }
                else
                {
                    *dir = 9;
                }
            }
            else
            {
                *dir = 6;
            }
        }
        else if ( g_sprite[h].x > g_sprite[h1].x )
        {
            if ( g_sprite[h].y < g_sprite[h1].y )
            {
                // 4, 1, 2
                if ( y_diff * 4 < x_diff )
                {
                    *dir = 4;
                }
                else if ( x_diff * 4 < y_diff )
                {
                    *dir = 2;
                }
                else
                {
                    *dir = 1;
                }
            }
            else if ( g_sprite[h].y > g_sprite[h1].y )
            {
                // 4, 7, 8
                if ( y_diff * 4 < x_diff )
                {
                    *dir = 4;
                }
                else if ( x_diff * 4 < y_diff )
                {
                    *dir = 8;
                }
                else
                {
                    *dir = 7;
                }
            }
            else
            {
                *dir = 4;
            }
        }
        else
        {
            if ( g_sprite[h].y < g_sprite[h1].y )
            {
                *dir = 2;
            }
            else if ( g_sprite[h].y > g_sprite[h1].y )
            {
                *dir = 8;
            }
        }
        return max( x_diff, y_diff );
    }

    int distancex = 5000;
    int distancey = 5000;
    int dirx = *dir;
    int diry = *dir;
    if (g_sprite[h].x > g_sprite[h1].x) if ((g_sprite[h].x - g_sprite[h1].x) < distancex)
    {
        distancex = (g_sprite[h].x - g_sprite[h1].x);
        dirx = 4;
    }

    if (g_sprite[h].x <= g_sprite[h1].x) if ((g_sprite[h1].x - g_sprite[h].x) < distancex)
    {
        distancex = (g_sprite[h1].x - g_sprite[h].x);
        dirx = 6;
    }
    if (g_sprite[h].y > g_sprite[h1].y) if ((g_sprite[h].y - g_sprite[h1].y) < distancey)
    {
        distancey = (g_sprite[h].y - g_sprite[h1].y);
        diry = 8;

    }
    if (g_sprite[h].y <= g_sprite[h1].y) if ((g_sprite[h1].y - g_sprite[h].y) < distancey)
    {
        distancey = (g_sprite[h1].y - g_sprite[h].y);
        diry = 2;
    }
    if (distancex > distancey)
    {

        *dir = dirx;
        return(distancex);
    }
    else 
    {
        *dir = diry;
        return(distancey);
    }


}

void process_follow(int h)
{
    int hx, hy;

    if (g_sprite[h].follow > 299)
    {
        LogMsg("ERROR:  Sprite %d cannot 'follow' sprite %d??",h,g_sprite[h].follow);
        return;
    }

    if (g_sprite[g_sprite[h].follow].active == false)
    {
        LogMsg("Killing follow");
        g_sprite[h].follow = 0;
        return;
    }

    hx = g_sprite[g_sprite[h].follow].x;
    hy = g_sprite[g_sprite[h].follow].y;

    int dir;
    int distance = get_distance_and_dir(h, g_sprite[h].follow, &dir);

    if (distance < 40) return;

    changedir(dir,h,g_sprite[h].base_walk);
    automove(h);


}


void process_target(int h)
{
    int hx, hy;

    if (g_sprite[h].target > 299)
    {
        LogMsg("ERROR:  Sprite %d cannot 'target' sprite %d??",h,g_sprite[h].follow);
        return;
    }

    if (g_sprite[g_sprite[h].target].active == false)
    {
        LogMsg("Killing target");
        g_sprite[h].target = 0;
        return;
    }

    hx = g_sprite[g_sprite[h].target].x;
    hy = g_sprite[g_sprite[h].target].y;

    int dir;
    int distance = get_distance_and_dir(h, g_sprite[h].target, &dir);

    if (distance < g_sprite[h].distance) return;

    changedir(dir,h,g_sprite[h].base_walk);

    automove(h);


}


bool check_for_kill_script(int i)
{


    if (g_sprite[i].script > 0)
    {
        //if (  (spr[i].brain == 0) | (spr[i].brain == 5) | (spr[i].brain == 6) | (spr[i].brain == 7))

        if (locate(g_sprite[i].script, "DIE")) run_script(g_sprite[i].script);

        return(true);   
    }

    return(false);
}

bool check_for_duck_script(int i)
{
    if (g_sprite[i].script > 0)
    {
        //if (  (spr[i].brain == 0) | (spr[i].brain == 5) | (spr[i].brain == 6) | (spr[i].brain == 7))

        if (locate(g_sprite[i].script, "DUCKDIE")) run_script(g_sprite[i].script);

        return(true);   
    }

    return(false);
}

void process_move(int h)
{
    //  Msg("Proccesing sprite %d, dir %d (script is %d)", h, spr[h].dir, spr[h].move_script);
    if ((g_sprite[h].move_dir == 4) | (g_sprite[h].move_dir == 1) | (g_sprite[h].move_dir == 7) )
    {
        if (g_sprite[h].x <= g_sprite[h].move_num)
        {
            //done moving
            done_moving(h);
            return;
        }
        changedir(g_sprite[h].move_dir,h,g_sprite[h].base_walk);      
        automove(h);    
    }

    if ( (g_sprite[h].move_dir == 6) | (g_sprite[h].move_dir == 9) | (g_sprite[h].move_dir == 3))
    {
        if (g_sprite[h].x >= g_sprite[h].move_num)
        {
            //done moving
            done_moving(h);
            return;
        }
        changedir(g_sprite[h].move_dir,h,g_sprite[h].base_walk);      
        automove(h);    
    }


    if (g_sprite[h].move_dir == 2)
    {
        if (g_sprite[h].y >= g_sprite[h].move_num)
        {
            //done moving
            done_moving(h);
            return;
        }
        changedir(g_sprite[h].move_dir,h,g_sprite[h].base_walk);      
        automove(h);    
    }


    if (g_sprite[h].move_dir == 8)
    {
        if (g_sprite[h].y <= g_sprite[h].move_num)
        {
            //done moving
            done_moving(h);
            return;
        }
        changedir(g_sprite[h].move_dir,h,g_sprite[h].base_walk);      
        automove(h);    
    }
}

void duck_brain(int h)
{
    int hold;

    if (   (g_sprite[h].damage > 0) && (in_this_base(g_sprite[h].pseq, 110)  ) )
    {
        check_for_duck_script(h);

        //hit a dead duck
        int crap2 = add_sprite(g_sprite[h].x,g_sprite[h].y,7,164,1);
        g_sprite[crap2].speed = 0;
        g_sprite[crap2].base_walk = 0;
        g_sprite[crap2].seq = 164;
        draw_damage(h);
        g_sprite[h].damage = 0;
        add_exp(g_sprite[h].exp, h);

        kill_sprite_all(h);
        return;
    }

    if (   (g_sprite[h].damage > 0) && (in_this_base(g_sprite[h].pseq, g_sprite[h].base_walk)  ) )
    {
      
        draw_damage(h);
        add_exp(g_sprite[h].exp, h);
        g_sprite[h].damage = 0;

        //lets kill the duck here, ha.
        check_for_kill_script(h);
        g_sprite[h].follow = 0;
        int crap = add_sprite(g_sprite[h].x,g_sprite[h].y,5,1,1);
        g_sprite[crap].speed = 0;
        g_sprite[crap].base_walk = 0;
        g_sprite[crap].size = g_sprite[h].size;                       
        g_sprite[crap].speed =  ((rand() % 3)+1);


        g_sprite[h].base_walk = 110;
        g_sprite[h].speed = 1;
        g_sprite[h].timer = 0;
        g_sprite[h].wait = 0;
        g_sprite[h].frame = 0;

        if (g_sprite[h].dir == 0) g_sprite[h].dir = 1;
        if (g_sprite[h].dir == 4) g_sprite[h].dir = 7;
        if (g_sprite[h].dir == 6) g_sprite[h].dir = 3;

        changedir(g_sprite[h].dir,h,g_sprite[h].base_walk);
        g_sprite[crap].dir = g_sprite[h].dir;
        g_sprite[crap].base_walk = 120;
        changedir(g_sprite[crap].dir,crap,g_sprite[crap].base_walk);
        automove(h);
        return;
    }


    if (g_sprite[h].move_active)
    {
        process_move(h);
        return;
    }

    if (g_sprite[h].freeze)
    {
        return;
    }


    if (g_sprite[h].follow > 0)
    {
        process_follow(h);
        return;
    }

    if (g_sprite[h].base_walk == 110)
    {
        if ( (rand() % 100)+1 == 1)
            random_blood(g_sprite[h].x, g_sprite[h].y-18, h);
        goto walk;
    }

    if (g_sprite[h].seq == 0 ) 
    {
        if (((rand() % 12)+1) == 1 )
        {  
            hold = ((rand() % 9)+1);
            if ((hold != 2) && (hold != 8) && (hold != 5))
            {
                //Msg("random dir change started.. %d", hold);
                changedir(hold,h,g_sprite[h].base_walk);
            }
            else
            {
                int junk = g_sprite[h].size;

                if (junk >=  100)
                    junk = 18000 - (junk * 50);

                if (junk < 100)
                    junk = 16000 + (junk * 100);

                SoundPlayEffect( 1,junk, 800,h ,0);
                g_sprite[h].mx = 0;
                g_sprite[h].my = 0;
                g_sprite[h].wait = g_dglos.g_dinkTick + (rand() % 300)+200;
            }

			return;     
        } 

        if ((g_sprite[h].mx != 0) || (g_sprite[h].my != 0))
        {
            g_sprite[h].seq = g_sprite[h].seq_orig;

        }
    }


walk:
    if (g_sprite[h].y > C_DINK_ORIGINAL_GAME_AREA_Y)
    {
        changedir(9,h,g_sprite[h].base_walk);
    }         

    if (g_sprite[h].x > g_gameAreaRightBarStartX-30)
    {
        changedir(7,h,g_sprite[h].base_walk);
    }         

    if (g_sprite[h].y < 10)
    {
        changedir(1,h,g_sprite[h].base_walk);
    }         

    if (g_sprite[h].x < 30) 
    {
        changedir(3,h,g_sprite[h].base_walk);
    }         

    //   Msg("Duck dir is %d, seq is %d.", spr[h].dir, spr[h].seq); 
    automove(h);

    if (check_if_move_is_legal(h) != 0)

    {
        if (g_sprite[h].dir != 0)
            changedir(autoreverse_diag(h),h,g_sprite[h].base_walk);
    }

}

// end duck_brain

void change_dir_to_diag( int32 *dir)
{

    if (*dir == 8) *dir = 7;
    if (*dir == 4) *dir = 1;
    if (*dir == 2) *dir = 3;
    if (*dir == 6) *dir = 9;

}

void pill_brain(int h)
{
    int hold;

    if  (g_sprite[h].damage > 0)
    {
        //got hit
        
        if (g_sprite[h].hitpoints > 0)
        {
            draw_damage(h);
            if (g_sprite[h].damage > g_sprite[h].hitpoints) g_sprite[h].damage = g_sprite[h].hitpoints;
            g_sprite[h].hitpoints -= g_sprite[h].damage;

            if (g_sprite[h].hitpoints < 1)
            {
                //they killed it
                check_for_kill_script(h);

                if (g_sprite[h].brain == 9)
                {
                    if (g_sprite[h].dir == 0) g_sprite[h].dir = 3;
                    change_dir_to_diag(&g_sprite[h].dir);
                    add_kill_sprite(h);
                    g_sprite[h].active = false;
                }
                return;

            }
        }
        g_sprite[h].damage = 0;
    }

    if (g_sprite[h].move_active) 
    {
        process_move(h);
        return;
    }

    if (g_sprite[h].freeze) return;

    if (g_sprite[h].follow > 0)
    {
        process_follow(h);
    }

    if (g_sprite[h].target != 0) 
    {

        if (in_this_base(g_sprite[h].seq, g_sprite[h].base_attack))
        {
            //still attacking
            return;
        }

        int dir;
        if (g_sprite[h].distance == 0) g_sprite[h].distance = 5;
        int distance = get_distance_and_dir(h, g_sprite[h].target, &dir);

        if (distance < g_sprite[h].distance) if (g_sprite[h].attack_wait < g_dglos.g_dinkTick)
        {
            //  Msg("base attack is %d.",spr[h].base_attack);
            if (g_sprite[h].base_attack != -1)
            {
                int attackdir;
                bool old_smooth_follow = g_dglos.smooth_follow;
                g_dglos.smooth_follow = false;
                get_distance_and_dir(h, g_sprite[h].target, &attackdir);
                g_dglos.smooth_follow = old_smooth_follow;
                //Msg("attacking with %d..", spr[h].base_attack+dir);

                g_sprite[h].dir = attackdir;

                g_sprite[h].seq = g_sprite[h].base_attack+g_sprite[h].dir;
                g_sprite[h].frame = 0;

                if (g_sprite[h].script != 0)
				{
                    if (locate(g_sprite[h].script, "ATTACK")) run_script(g_sprite[h].script);
				} else
                        g_sprite[h].move_wait = g_dglos.g_dinkTick + ((rand() % 300)+10);;
                return;

            }

        }

        if (g_sprite[h].move_wait  < g_dglos.g_dinkTick)
        {
            process_target(h);
            g_sprite[h].move_wait = g_dglos.g_dinkTick + 200;

        }
        else
        {
            goto walk_normal;
        }

        return;
    }

walk_normal:

    if (g_sprite[h].base_walk != -1)
    {
        if ( g_sprite[h].seq == 0) goto recal;
    }

    if (( g_sprite[h].seq == 0) && (g_sprite[h].move_wait < g_dglos.g_dinkTick))
    {
recal:
        if (((rand() % 12)+1) == 1 )
        {  
            hold = ((rand() % 9)+1);
            if (  (hold != 4) &&   (hold != 6) &&  (hold != 2) && (hold != 8) && (hold != 5))
            {
                changedir(hold,h,g_sprite[h].base_walk);
                g_sprite[h].move_wait = g_dglos.g_dinkTick +((rand() % 2000)+200);
            }

        } else
        {
            //keep going the same way
            if (in_this_base(g_sprite[h].seq_orig, g_sprite[h].base_attack)) goto recal;
            g_sprite[h].seq = g_sprite[h].seq_orig;
            if (g_sprite[h].seq_orig == 0) goto recal;
        }
    }

    if (g_sprite[h].y > (C_DINK_ORIGINAL_GAME_AREA_Y - 15))

    {
        changedir(9,h,g_sprite[h].base_walk);
    }         

    if (g_sprite[h].x > (g_gameAreaRightBarStartX - 15))

    {
        changedir(1,h,g_sprite[h].base_walk);
    }         

    if (g_sprite[h].y < 18)
    {
        changedir(1,h,g_sprite[h].base_walk);
    }         

    if (g_sprite[h].x < 18) 
    {
        changedir(3,h,g_sprite[h].base_walk);
    }         

    automove(h);

    if (check_if_move_is_legal(h) != 0)
    {
        g_sprite[h].move_wait = g_dglos.g_dinkTick + 400;
        changedir(autoreverse_diag(h),h,g_sprite[h].base_walk);
    }

}

void find_action(int h)
{
    g_sprite[h].action = (rand() % 2)+1;

    if (g_sprite[h].action == 1)
    {
        //sit and think
        g_sprite[h].move_wait = g_dglos.g_dinkTick +((rand() % 3000)+400);
        if (g_sprite[h].base_walk != -1)
        {
            int dir = (rand() % 4)+1;  

            g_sprite[h].pframe = 1;
            if (dir == 1)  g_sprite[h].pseq = g_sprite[h].base_walk+1;
            if (dir == 2)  g_sprite[h].pseq = g_sprite[h].base_walk+3;
            if (dir == 3)  g_sprite[h].pseq = g_sprite[h].base_walk+7;
            if (dir == 4)  g_sprite[h].pseq = g_sprite[h].base_walk+9;
        }

        return;
    }

    if (g_sprite[h].action == 2)
    {
        //move
        g_sprite[h].move_wait = g_dglos.g_dinkTick +((rand() % 3000)+500);
        int dir = (rand() % 4)+1;  
        g_sprite[h].pframe = 1;
        if (dir == 1)  changedir(1,h,g_sprite[h].base_walk);
        if (dir == 2)  changedir(3,h,g_sprite[h].base_walk);
        if (dir == 3)  changedir(7,h,g_sprite[h].base_walk);
        if (dir == 4)  changedir(9,h,g_sprite[h].base_walk);
        return;
    }


    LogMsg("Internal error:  Brain 16, unknown action.");
}

void people_brain(int h)
{
    if  (g_sprite[h].damage > 0)
    {
        //got hit
 
		if (g_sprite[h].hitpoints > 0)
        {
            draw_damage(h);
            if (g_sprite[h].damage > g_sprite[h].hitpoints) g_sprite[h].damage = g_sprite[h].hitpoints;
            g_sprite[h].hitpoints -= g_sprite[h].damage;

            if (g_sprite[h].hitpoints < 1)
            {
                //they killed it
                check_for_kill_script(h);

                if (g_sprite[h].brain == 16)
                {
                    if (g_sprite[h].dir == 0) g_sprite[h].dir = 3;
                    g_sprite[h].brain = 0;
                    change_dir_to_diag(&g_sprite[h].dir);
                    add_kill_sprite(h);
                    g_sprite[h].active = false;
                }
                return;

            }
        }
        g_sprite[h].damage = 0;

    }

    if (g_sprite[h].move_active) 
    {
        process_move(h);
        return;
    }

    if (g_sprite[h].freeze) return;

    if (g_sprite[h].follow > 0)
    {
        process_follow(h);
        return;
    }

    if ((g_sprite[h].move_wait < g_dglos.g_dinkTick) && (g_sprite[h].seq == 0))
    {
        g_sprite[h].action = 0;
    }

    if (g_sprite[h].action == 0) find_action(h);

    if (g_sprite[h].action != 2) 
    {
        g_sprite[h].seq = 0;
        return;

    }
    if (g_sprite[h].seq_orig != 0)
        if (g_sprite[h].seq == 0) g_sprite[h].seq = g_sprite[h].seq_orig;


    if (g_sprite[h].y > C_DINK_ORIGINAL_GAME_AREA_Y)
    {

        if ( ((rand() % 2)+1) == 1)
            changedir(9,h,g_sprite[h].base_walk);
        else changedir(7,h,g_sprite[h].base_walk);
    }         

    if (g_sprite[h].x > g_gameAreaRightBarStartX)

    {
        if ( ((rand() % 2)+1) == 1)
            changedir(1,h,g_sprite[h].base_walk);
        else changedir(7,h,g_sprite[h].base_walk);

    }         

    if (g_sprite[h].y < 20)
    {
        if ( ((rand() % 2)+1) == 1)
            changedir(1,h,g_sprite[h].base_walk);
        else changedir(3,h,g_sprite[h].base_walk);
    }         

    if (g_sprite[h].x < 30) 
    {
        if ( ((rand() % 2)+1) == 1)
            changedir(3,h,g_sprite[h].base_walk);
        else changedir(9,h,g_sprite[h].base_walk);
    }         

    automove(h);

    if (check_if_move_is_legal(h) != 0)
    {
        if ((rand() % 3) == 2)
        {
            changedir(autoreverse_diag(h),h,g_sprite[h].base_walk);

        } else
        {
            g_sprite[h].move_wait = 0;
            g_sprite[h].pframe = 1;
            g_sprite[h].seq = 0;
        }
    }
}


void no_brain(int h)
{
    if (g_sprite[h].move_active) 
    {
        process_move(h);
        return;
    }

    if (g_sprite[h].freeze) return;

    if (g_sprite[h].follow > 0)
    {
        process_follow(h);
        return;
    }
}


void shadow_brain(int h)
{
    if (g_sprite[g_sprite[h].brain_parm].active == false)
    {
        g_sprite[h].active = false;
        return;
    }

    g_sprite[h].x = g_sprite[g_sprite[h].brain_parm].x;
    g_sprite[h].y = g_sprite[g_sprite[h].brain_parm].y;
    g_sprite[h].size = g_sprite[g_sprite[h].brain_parm].size;

    if (g_sprite[h].seq == 0) if (g_sprite[h].seq_orig != 0) g_sprite[h].seq = g_sprite[h].seq_orig;
}


void dragon_brain(int h)
{
    int hold;


    if  (g_sprite[h].damage > 0)
    {
        //got hit
        if (g_sprite[h].hitpoints > 0)
        {
            draw_damage(h);
            if (g_sprite[h].damage > g_sprite[h].hitpoints) g_sprite[h].damage = g_sprite[h].hitpoints;
            g_sprite[h].hitpoints -= g_sprite[h].damage;

            if (g_sprite[h].hitpoints < 1)
            {
                //they killed it

                check_for_kill_script(h);
                if (g_sprite[h].brain == 10)
                {
                    add_kill_sprite(h);
                    g_sprite[h].active = false;
                }

                return;

            }
        }
        g_sprite[h].damage = 0;
    }


    if (g_sprite[h].move_active) 
    {
        process_move(h);
        return;
    }


    if (g_sprite[h].freeze) return;


    if (g_sprite[h].follow > 0)
    {
        process_follow(h);
        return;
    }

    if (g_sprite[h].target != 0)
        if (g_sprite[h].attack_wait < g_dglos.g_dinkTick)
        {
            if (g_sprite[h].script != 0) 
            {

                if (locate(g_sprite[h].script, "ATTACK")) run_script(g_sprite[h].script);
            }   
        }

        if (g_sprite[h].seq == 0)
        {
recal:
            if (((rand() % 12)+1) == 1 )
            {  
                hold = ((rand() % 9)+1);
                if (  (hold != 1) &&   (hold != 3) &&  (hold != 7) && (hold != 9) && (hold != 5))
                {
                    changedir(hold,h,g_sprite[h].base_walk);

                }

            } else
            {
                //keep going the same way
                g_sprite[h].seq = g_sprite[h].seq_orig;
                if (g_sprite[h].seq_orig == 0) goto recal;
            }

        }


        if (g_sprite[h].y > C_DINK_ORIGINAL_GAME_AREA_Y)

        {
            changedir(8,h,g_sprite[h].base_walk);
        }         

        if (g_sprite[h].x > C_DINK_SCREENSIZE_X)
        {
            changedir(4,h,g_sprite[h].base_walk);
        }         

        if (g_sprite[h].y < 0)
        {
            changedir(2,h,g_sprite[h].base_walk);
        }         

        if (g_sprite[h].x < 0) 
        {
            changedir(6,h,g_sprite[h].base_walk);
        }         

        automove(h);

        if (check_if_move_is_legal(h) != 0)

        {

            int mydir = autoreverse(h);

            //  Msg("Real dir now is %d, autoresver changed to %d.",spr[h].dir, mydir);

            changedir(mydir,h,g_sprite[h].base_walk);

#ifdef _DEBUG
			LogMsg("real dir changed to %d",g_sprite[h].dir);
#endif
        }

}


void pig_brain(int h)
{
    int hold;

    if (g_sprite[h].move_active) 
    {
        process_move(h);
        return;
    }

    if (   (g_sprite[h].damage > 0) )
    {
        
        draw_damage(h);
        g_sprite[h].hitpoints -= g_sprite[h].damage;
        g_sprite[h].damage = 0;
        if (g_sprite[h].hitpoints < 1)
        {
            add_exp(g_sprite[h].exp, h);
            g_sprite[h].damage = 0;
            //lets kill the duck here, ha.
            check_for_kill_script(h);
            g_sprite[h].speed = 0;
            g_sprite[h].base_walk = -1;
            g_sprite[h].seq = 164;
            g_sprite[h].brain = 7;    
        }

        return;
    }

    if (g_sprite[h].freeze) return;

    if (g_sprite[h].seq == 0 ) 
    {

        if (((rand() % 12)+1) == 1 )
        {  
            hold = ((rand() % 9)+1);

            if (  (hold != 4) &&   (hold != 6) &&  (hold != 2) && (hold != 8) && (hold != 5))
            {
                changedir(hold,h,g_sprite[h].base_walk);

            }
            else
            {
                int junk = g_sprite[h].size;

                if (junk >=  100)
                    junk = 18000 - (junk * 50);

                if (junk < 100)
                    junk = 16000 + (junk * 100);


                hold = ((rand() % 4)+1);

                if (!playing(g_sprite[h].last_sound)) g_sprite[h].last_sound = 0;

                if (g_sprite[h].last_sound == 0)
                {


                    if (hold == 1) 
                        g_sprite[h].last_sound = SoundPlayEffect( 2,junk, 800 ,h,0);
                    if (hold == 2) 
                        g_sprite[h].last_sound = SoundPlayEffect( 3,junk, 800,h ,0);
                    if (hold == 3) 
                        g_sprite[h].last_sound = SoundPlayEffect( 4,junk, 800 ,h,0);
                    if (hold == 4) 
                        g_sprite[h].last_sound = SoundPlayEffect( 5,junk, 800,h,0 );

                }

                g_sprite[h].mx = 0;
                g_sprite[h].my = 0;
                g_sprite[h].wait = g_dglos.g_dinkTick + (rand() % 300)+200;

            }

        } 
        else
        {

            if ((g_sprite[h].mx != 0) || (g_sprite[h].my != 0))

            {
                g_sprite[h].seq = g_sprite[h].seq_orig;

            }                                                                                                                                                                                                                                                                                                                          

        }
    }


    if (g_sprite[h].y > (C_DINK_ORIGINAL_GAME_AREA_Y-g_dglos.g_picInfo[getpic(h)].box.bottom / 4))
    {
        changedir(9,h,g_sprite[h].base_walk);
    }         

    if (g_sprite[h].x > (C_DINK_SCREENSIZE_X-g_dglos.g_picInfo[getpic(h)].box.right-10))
    {
        changedir(1,h,g_sprite[h].base_walk);
    }         

    if (g_sprite[h].y < 10)
    {
        changedir(1,h,g_sprite[h].base_walk);
    }         

    if (g_sprite[h].x < 10) 
    {
        changedir(3,h,g_sprite[h].base_walk);
    }         

    automove(h);

    if (check_if_move_is_legal(h) != 0)

    {
        changedir(autoreverse_diag(h),h,g_sprite[h].base_walk);
    }

}
// end duck_brain




int check_if_move_is_legal(int u)

{
    //redink1 removed so move_nohard is active for all movements, not just active moves.
    //if (spr[u].move_active)
    if (g_sprite[u].move_nohard == 1)
        return(0);
    if (u == 1) if (in_this_base(g_sprite[u].seq, g_dglos.mDinkBasePush)) return(0);

    //if (u == 1) if (!no_cheat) if (g_script_debug_mode) return(0);
    
	
	int hardness = 0;
    if (g_sprite[u].moveman > 0)
    {
        for (int i=1; i <= g_sprite[u].moveman; i++)
        {
            hardness = get_hard(u,g_sprite[u].lpx[i]-20  , g_sprite[u].lpy[i]);
            if (hardness == 2) if (g_sprite[u].flying) 
            {
                g_sprite[u].moveman = 0;         
                // redink1 changed so flying works properly
                return(0);
            }
			
			if (u == 1)
			{
				//it's dink, should we cheat and walk through stuff?
				if (GetApp()->GetGhostMode()) return 0;
			}

			if (hardness > 0)
			{

			
				g_sprite[u].x = g_sprite[u].lpx[i - 1];
				g_sprite[u].y = g_sprite[u].lpy[i - 1];
				g_sprite[u].moveman = 0;

				if (g_dglos.g_pushingEnabled)
					if (u == 1) if (hardness != 2) if (g_dglos.g_playerInfo.push_active == false)
					{
						if ((g_sprite[u].dir == 2) | (g_sprite[u].dir == 4) | (g_sprite[u].dir == 6) | (g_sprite[u].dir == 8))
						{
							//he  (dink)  is definatly pushing on something
							g_dglos.g_playerInfo.push_active = true;
							g_dglos.g_playerInfo.push_dir = g_sprite[u].dir;
							g_dglos.g_playerInfo.push_timer = g_dglos.g_dinkTick;

						}
					}
					else
					{
						if (g_dglos.g_playerInfo.push_dir != g_sprite[1].dir) g_dglos.g_playerInfo.push_active = false;
					}
				
		
				return(hardness);
			}
        }
    }

    if (u == 1)  g_dglos.g_playerInfo.push_active = false;
    return(0);
}



void move(int u, int amount, char kind,  char kindy)
{
    int mx = 0;
    int my = 0; 
    bool clearx;
    bool cleary;
    clearx = false;
    cleary = false;

    for (int i=1; i <= amount; i++)
    {
        g_sprite[u].moveman++;
        if (mx >= g_sprite[u].mx) clearx = true;
        if (my >= g_sprite[u].my) clearx = true;

        if ((clearx) && (cleary))
        {
            mx = 0;
            my = 0;
            clearx = false;
            cleary = false;
        }

        if (kind == '+')
        {
            if (mx < g_sprite[u].mx)
                g_sprite[u].x++;
            mx++;

        }
        if (kind == '-')
        {


            if (mx < (g_sprite[u].mx - (g_sprite[u].mx * 2)))
                g_sprite[u].x--;
            mx++;
        }

        if (kindy == '+')
        {

            if (my < g_sprite[u].my)
                g_sprite[u].y++;
            my++;
        }
        if (kindy == '-')
        {

            if (my < (g_sprite[u].my - (g_sprite[u].my * 2)))
                g_sprite[u].y--;
            my++;
        }

        g_sprite[u].lpx[g_sprite[u].moveman] = g_sprite[u].x;
        g_sprite[u].lpy[g_sprite[u].moveman] = g_sprite[u].y;
    }
}

void bounce_brain(int h)
{
    if (g_sprite[h].y > (C_DINK_ORIGINAL_GAME_AREA_Y-g_dglos.g_picInfo[getpic(h)].box.bottom))
    {
        g_sprite[h].my -= (g_sprite[h].my * 2);
    }         

    if (g_sprite[h].x > (C_DINK_SCREENSIZE_X-g_dglos.g_picInfo[getpic(h)].box.right))
    {
        g_sprite[h].mx -= (g_sprite[h].mx * 2);
    }         

    if (g_sprite[h].y < 0)
    {
        g_sprite[h].my -= (g_sprite[h].my * 2);
    }         


    if (g_sprite[h].x < 0) 
    {
        g_sprite[h].mx -= (g_sprite[h].mx * 2);
    }         


    g_sprite[h].x += g_sprite[h].mx;
    g_sprite[h].y += g_sprite[h].my;
}
//end bounce brain      

void CheckTransitionSurface()
{
	if (!g_transitionSurf.IsLoaded())
	{
		g_transitionSurf.SetUsesAlpha(false);
		g_transitionSurf.InitBlankSurface(GetPrimaryGLX(), GetPrimaryGLY());
		g_transitionSurf.SetSmoothing(false);
		g_transitionSurf.SetBlendingMode(SurfaceAnim::BLENDING_PREMULTIPLIED_ALPHA);
	}
}

void UpdateFrameWithoutTransitionAndThinking()
{
	SetOrthoRenderSize(g_dglo.m_orthoRenderRect.right, g_dglo.m_orthoRenderRect.GetHeight(), -g_dglo.m_orthoRenderRect.left, -g_dglo.m_orthoRenderRect.top);

	GetBaseApp()->SetGameTickPause(true); //don't let logic actually happen in here

	rtRect32 rcRect;

	rcRect.left = 0;
	rcRect.top = 0;
	rcRect.right = C_DINK_SCREENSIZE_X;
	rcRect.bottom = C_DINK_SCREENSIZE_Y;

	//Blit from Two, which holds the base scene.

	lpDDSBack->BltFast( 0, 0, lpDDSBackGround, &rcRect, DDBLTFAST_NOCOLORKEY);
	g_dglo.m_bgSpriteMan.Render(lpDDSBack); //blit sprites that have been shoved into the bg, too slow to actually add them, so we fake it until the screen is rebuilt
	
	
	//render sprites

	int h= 0;

	for (int j = 1; j < C_MAX_SPRITES_AT_ONCE; j++)
	{
		if (g_dglos.plane_process)
			h = g_spriteRank[j]; else h = j;
		//Msg( "Ok, rank %d is %d", j,h);

		if (h > 0)
		{
			ThinkSprite(h, g_bTransitionActive || g_dglos.g_stopEntireGame == 1);
		}
	}    
	
	GetBaseApp()->SetGameTickPause(false);

	RemoveOrthoRenderSize();

}


void StartScreenScrollTransition(int direction)
{
   
	//this is where we grab a screenshot of the current pic before scrolling to the next screen
	rtRect32 rcRect;
   
    if (no_transition)
    {
         //g_bInitiateScreenMove = true;
         return;
    }

	CheckTransitionSurface();
	//remember this for later

	if (GetApp()->GetVar("disable_glread")->GetUINT32() == 1)
	{
		//no_transition = true;
		return;
	}

	//remove the aspect ratio hack
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	UpdateFrameWithoutTransitionAndThinking();
	
	if (GetApp()->GetVar("disable_glread")->GetUINT32() == 1)
	{
		g_transitionSurf.FillColor(glColorBytes(0, 0, 0, 255));
	}
	else
	{
		g_transitionSurf.CopyFromScreen();
	}

	ApplyAspectRatioGLMatrix();
	if (g_dglo.GetActiveView() != DinkGlobals::VIEW_ZOOMED)
	{
		//clear background if needed
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);
	}
	//redraw it the correct way, otherwise we can see a flash
	UpdateFrameWithoutTransitionAndThinking();
		
	g_bInitiateScreenMove = true;
	g_bTransitionActive = true;
	g_dglo.m_transitionTimer = 0;
	
	//look at your numpad.., that's the dir we're going
	
	//here we're set an offset needed for where we START FROM for the animation.  When the anim is done, this will be applied 0%
	switch(direction)
	{
		case 4:
			g_dglo.m_transitionOffsetNative = CL_Vec2f((int32)-g_dglo.m_nativeGameArea.GetWidth(), 0);
			g_dglo.m_transitionOffset = CL_Vec2f((int32)-g_dglo.m_gameArea.GetWidth(), 0);
		break;
		
		case 6:
			g_dglo.m_transitionOffsetNative = CL_Vec2f((int32)g_dglo.m_nativeGameArea.GetWidth(), 0);
			g_dglo.m_transitionOffset = CL_Vec2f((int32)g_dglo.m_gameArea.GetWidth(), 0);
			break;
		case 8:
			g_dglo.m_transitionOffsetNative = CL_Vec2f(0, -g_dglo.m_nativeGameArea.GetHeight());
			g_dglo.m_transitionOffset = CL_Vec2f(0, -g_dglo.m_gameArea.GetHeight());
			break;
		case 2:
			g_dglo.m_transitionOffsetNative = CL_Vec2f(0, (int32)g_dglo.m_nativeGameArea.GetHeight());
			g_dglo.m_transitionOffset = CL_Vec2f(0, (int32)g_dglo.m_gameArea.GetHeight());
			break;
		
		default:
			assert(!"er");
	}
}


void ProcessTransition(void)
{
	if (!g_bTransitionActive) return;

	g_dglo.m_transitionProgress = float(GetBaseApp()->GetGameTick()-g_dglo.m_transitionTimer) / C_DINK_SCREEN_TRANSITION_TIME_MS;

	if (g_dglo.m_transitionTimer == 0)
	{
		g_dglo.m_transitionProgress = 0;
	}

	float inverseProg = 1.0f - g_dglo.m_transitionProgress;
	
	if (g_dglo.m_transitionProgress >= 1)
	{
		g_bTransitionActive = false;
		g_bInitiateScreenMove = 0;
		g_dglo.m_transitionOffset = CL_Vec2f(0,0);
		g_dglo.m_transitionTimer = 0;
	}

	//float offsetX = 0.5f;

	float fTemp = inverseProg;
	//fTemp = 1.0f;
	glTranslatef((g_dglo.m_transitionOffset.x*fTemp),(g_dglo.m_transitionOffset.y*fTemp), 0);
	//glScalef(G_TRANSITION_SCALE_TRICK,G_TRANSITION_SCALE_TRICK,1);

	
}

void EndProcessTransition()
{
	if (!g_bTransitionActive) return;
	float inverseProg = 1.0f - g_dglo.m_transitionProgress;
	//glScalef(1/G_TRANSITION_SCALE_TRICK,1/G_TRANSITION_SCALE_TRICK,1);

	glTranslatef((-g_dglo.m_transitionOffset.x*inverseProg), (- ( (g_dglo.m_transitionOffset.y*inverseProg))), 0);

};

void BlitSecondTransitionScreen()
{

	if (g_bTransitionActive)
	{

		CheckTransitionSurface();
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();

		float offsetX = g_dglo.m_centeringOffset.x*(1.0f - ((float)GetFakePrimaryScreenSizeX() / (float)C_DINK_SCREENSIZE_X));
		float offsetY = g_dglo.m_centeringOffset.y*(1.0f - ((float)GetFakePrimaryScreenSizeY() / (float)C_DINK_SCREENSIZE_Y));

		glTranslatef(-offsetX, -offsetY, 0);

		rtRectf dstRect = g_dglo.m_nativeGameArea;

		static rtRectf dstOffset;
		static rtRectf srcOffset;

		if (GetApp()->GetVar("smoothing")->GetUINT32() != 0)
		{
			//fix black lines due to antialiasing
			dstOffset = rtRectf(-1, -1, 1, 1);
			srcOffset = rtRectf(1, 1, -1, -1);
		}
		else
		{
			//without normal antialiasing we don't need to do much, but this does fix tiny black artifacts during the screen transition
			dstOffset = rtRectf(-0.05, -0.05f, 0.05f, 0.05);
			srcOffset = rtRectf(0.4, 0.4, -0.4, -0.4);
		}

		dstRect.AdjustPosition((-g_dglo.m_transitionOffsetNative.x*g_dglo.m_transitionProgress),
			(-g_dglo.m_transitionOffsetNative.y*g_dglo.m_transitionProgress));
	
			rtRectf srcRect = ConvertFakeScreenRectToReal(g_dglo.m_nativeGameArea);
#ifdef _DEBUG
			//LogMsg("Final Trans Src rect: %s", PrintRect(srcRect).c_str());
			//LogMsg("Trans Dest rect: %s", PrintRect(dstRect).c_str());
			//LogMsg("Trans surf size: %s", PrintRect(g_transitionSurf.GetRectf()).c_str());
#endif
			g_transitionSurf.BlitEx(dstRect+dstOffset, srcRect+ srcOffset);
		
			
		if (!g_onePixelSurf.IsLoaded())
		{
			g_onePixelSurf.InitBlankSurface(1, 1);
			g_onePixelSurf.FillColor(glColorBytes(255, 255, 255, 255));
		}

		if (g_dglo.GetActiveView() != DinkGlobals::VIEW_ZOOMED)
		{
			static uint32 blackBarsColor = MAKE_RGBA(0, 0, 0, 255);
			//well, at this point we're done except there is garbage on the area outside the playfield if we're letterboxed so aspect ratio is right.  This could have all been
			//avoided with a render to surface but I think this will be faster

			g_onePixelSurf.BlitScaled(g_dglo.m_nativeGameArea.left, 0, CL_Vec2f(5000, 5000), ALIGNMENT_RIGHT_CENTER, blackBarsColor);
			g_onePixelSurf.BlitScaled(g_dglo.m_nativeGameArea.right, 0, CL_Vec2f(5000, 5000), ALIGNMENT_LEFT_CENTER, blackBarsColor);

			g_onePixelSurf.BlitScaled(0, g_dglo.m_nativeGameArea.top, CL_Vec2f(5000, 5000), ALIGNMENT_DOWN_CENTER, blackBarsColor);
			g_onePixelSurf.BlitScaled(0, g_dglo.m_nativeGameArea.bottom, CL_Vec2f(5000, 5000), ALIGNMENT_UPPER_CENTER, blackBarsColor);

		}
	
		glPopMatrix( );

	}

	
}

void did_player_cross_screen(bool bCheckWithoutMoving, int playerID)
{
	bool move_gonna = false;

	bool bNotScreenLocked = (g_dglos.screenlock == 0);
	if (GetApp()->GetGhostMode()) bNotScreenLocked = true;

    if (g_dglos.walk_off_screen == 1) return;
	//DO MATH TO SEE IF THEY HAVE CROSSED THE SCREEN, IF SO LOAD NEW ONE
    if ((g_sprite[playerID].x) < g_gameAreaLeftOffset) 
    {
        if ((g_MapInfo.loc[*pmap-1] > 0) && bNotScreenLocked)
        {
            //move one map to the left
            if (bCheckWithoutMoving)
            {
                move_gonna = true;
                return;
            }
            update_screen_time();
            StartScreenScrollTransition(4);
            *pmap -= 1; 
            load_map(g_MapInfo.loc[*pmap]);
            if (g_MapInfo.indoor[*pmap] == 0) g_dglos.g_playerInfo.last_map = *pmap;

            BuildScreenBackground();                                    
            g_sprite[playerID].x = 619;
            g_sprite[playerID].y = g_sprite[playerID].lpy[0];
            goto b1end;
        } else
        {
            g_sprite[playerID].x = g_gameAreaLeftOffset;
        }
    }

    if ((g_sprite[playerID].x) > 619) 
    {
        if ((g_MapInfo.loc[*pmap+1] > 0)  && bNotScreenLocked)
        {
            //move one map to the right
            if (bCheckWithoutMoving)
            {
                move_gonna = true;
                return;
            }

            update_screen_time();
            StartScreenScrollTransition(6);
            *pmap += 1; 
            load_map(g_MapInfo.loc[*pmap]);
            if (g_MapInfo.indoor[*pmap] == 0) g_dglos.g_playerInfo.last_map = *pmap;

            BuildScreenBackground();
            g_sprite[playerID].x =  g_gameAreaLeftOffset;
            g_sprite[playerID].y = g_sprite[playerID].lpy[0];
            goto b1end;
        } else
        {
            g_sprite[playerID].x = 619;
        }
    }

    if (g_sprite[playerID].y < 0)
    {
        if ((g_MapInfo.loc[*pmap-32] > 0)  && bNotScreenLocked)
        {
            //move one map up
            if (bCheckWithoutMoving)
            {
                move_gonna = true;
                return;
            }
            update_screen_time();
            StartScreenScrollTransition(8);
            *pmap -= 32;    
            load_map(g_MapInfo.loc[*pmap]);
            if (g_MapInfo.indoor[*pmap] == 0) g_dglos.g_playerInfo.last_map = *pmap;

            g_sprite[playerID].x = g_sprite[playerID].lpx[0];
            BuildScreenBackground();
            g_sprite[playerID].y =  399;

            goto b1end;
        } else
        {
            g_sprite[playerID].y = 0;
        }
    }

    if ( (g_sprite[playerID].y > 399 ) )
    {
        if ( (g_MapInfo.loc[*pmap+32] > 0)  && bNotScreenLocked)
        {
            //move one map down
            if (bCheckWithoutMoving)
            {
                move_gonna = true;
                return;
            }
            update_screen_time();
            StartScreenScrollTransition(2);
            *pmap += 32;    
            load_map(g_MapInfo.loc[*pmap]);
            if (g_MapInfo.indoor[*pmap] == 0) g_dglos.g_playerInfo.last_map = *pmap;

            BuildScreenBackground();
            g_sprite[playerID].y = 0;

            g_sprite[playerID].x = g_sprite[playerID].lpx[0];

            goto b1end;
        } else
        {
            g_sprite[playerID].y = 399;
        }
    }

b1end:;
}

bool run_through_tag_list_talk(int h)
{
    rtRect32 box;
    int amount, amounty;

    for (int i = 1; i <= g_dglos.last_sprite_created; i++)
    {

        if (g_sprite[i].active) if (i != h) if (g_sprite[i].brain != 8)
        {
            box = g_dglos.g_picInfo[getpic(i)].hardbox;
            OffsetRect(&box, g_sprite[i].x, g_sprite[i].y);
            InflateRect(&box, 10,10);

            amount = 50;        
            amounty = 35;
            if (g_sprite[h].dir == 6)
            {
                box.left -= amount;
            }

            if (g_sprite[h].dir == 4)
            {
                box.right += amount;
            }

            if (g_sprite[h].dir == 2)
            {
                box.top -= amounty;
            }

            if (g_sprite[h].dir == 8)
            {
                box.bottom += amounty;
            }

            //      draw_box(box, 33);

            if (inside_box(g_sprite[h].x, g_sprite[h].y, box))
            {   
                //Msg("Talking to sprite %d", i);
                if (g_sprite[i].script > 0)
                {
                    //Msg("trying to find TALK in script %d", spr[i].script);
                    if (locate(g_sprite[i].script, "TALK")) 
                    {
                        kill_returning_stuff(g_sprite[i].script);
                        run_script(g_sprite[i].script);
                        return(true);   
                    }
                }
            }
        }
    }

	return(false);
}

void make_missile(int x1, int y1, int dir, int speed, int seq, int frame, int strength)
{
    int crap = add_sprite(x1,y1,11,seq,frame);
    g_sprite[crap].speed = speed;
    g_sprite[crap].seq = seq;
    g_sprite[crap].timer = 0;
    g_sprite[crap].strength = strength;
    g_sprite[crap].flying = true;
    changedir(dir, crap, 430);
}

void missile_brain( int h, bool repeat)
{
    rtRect32 box;
    automove(h);

    *pmissle_source = h;
    int hard = check_if_move_is_legal(h);
    if (repeat)  if (g_sprite[h].seq == 0) g_sprite[h].seq = g_sprite[h].seq_orig;
    g_sprite[1].hitpoints = *plife; 

    if (hard > 0) if (hard != 2) 
    {
        //lets check to see if they hit a sprites hardness
        if (hard > 100)
        {
            for (int ii = 1; ii < g_dglos.last_sprite_created; ii++)
            {
                if (g_sprite[ii].sp_index == hard-100)
                {
                    if (g_sprite[ii].script > 0)
                    {
                        *pmissile_target = 1;
                        *penemy_sprite = 1;

                        if (locate(g_sprite[ii].script, "HIT"))
                        {
                            kill_returning_stuff(g_sprite[ii].script);
                            run_script(g_sprite[ii].script);
                        }
                    }

                    if (g_sprite[h].script > 0)
                    {
                        *pmissile_target = ii;
                        *penemy_sprite = 1;
                        if (locate(g_sprite[h].script, "DAMAGE")) 
                        {
                            kill_returning_stuff(g_sprite[h].script);
                            run_script(g_sprite[h].script);
                        }
                    } else
                    {
                        if (g_sprite[h].attack_hit_sound == 0)
                        {
                            SoundPlayEffect( 9,22050, 0 ,0,0);
                        } else
                        {
                            SoundPlayEffect( g_sprite[h].attack_hit_sound,g_sprite[h].attack_hit_sound_speed, 0 ,0,0);
                        }
                        g_sprite[h].active = 0;
                    }

                    //run missile end   
                    return;
                }
            }
        }
      
		//run missile end   

        if (g_sprite[h].script > 0)
        {
            *pmissile_target = 0;
            if (locate(g_sprite[h].script, "DAMAGE")) run_script(g_sprite[h].script);
        } else
        {
            if (g_sprite[h].attack_hit_sound == 0)
            {
                SoundPlayEffect( 9,22050, 0 ,0,0);
            } else
            {
                SoundPlayEffect( g_sprite[h].attack_hit_sound,g_sprite[h].attack_hit_sound_speed, 0 ,0,0);
            }

            g_sprite[h].active = 0;
            return;
        }
    }

    if (g_sprite[h].x > 1000) g_sprite[h].active = false;
    if (g_sprite[h].y > 700) g_sprite[h].active = false;
    if (g_sprite[h].y < -500) g_sprite[h].active = false;
    if (g_sprite[h].x < -500) g_sprite[h].active = false;

    //did we hit anything that can die?

    for (int j = 1; j <= g_dglos.last_sprite_created; j++)
    {
        if (g_sprite[j].active) if (h != j) if (g_sprite[j].nohit != 1) if (g_sprite[j].notouch == false)
            if (g_sprite[h].brain_parm != j) if (g_sprite[h].brain_parm2!= j) //if (spr[j].brain != 15) if
                //(spr[j].brain != 11)
            {
                box = g_dglos.g_picInfo[getpic(j)].hardbox;
                OffsetRect(&box, g_sprite[j].x, g_sprite[j].y);

                if (g_sprite[h].range != 0)
                    InflateRect(&box, g_sprite[h].range,g_sprite[h].range);

                if (g_script_debug_mode) draw_box(box, 33);

				if (inside_box(g_sprite[h].x, g_sprite[h].y, box))
                {
                    g_sprite[j].notouch = true;
                    g_sprite[j].notouch_timer = g_dglos.g_dinkTick+100;
                    g_sprite[j].target = 1;
                    *penemy_sprite = 1;
                    //change later to reflect REAL target
                    if (g_sprite[h].script > 0)
                    {

                        *pmissile_target = j;
                        if (locate(g_sprite[h].script, "DAMAGE")) run_script(g_sprite[h].script);
                    } else
                    {

                        if (g_sprite[h].attack_hit_sound == 0)
                        {
                            SoundPlayEffect( 9,22050, 0 ,0,0);
                        } else
                        {
                            SoundPlayEffect( g_sprite[h].attack_hit_sound,g_sprite[h].attack_hit_sound_speed, 0 ,0,0);
                        }
                    }

                    if ( g_sprite[j].hitpoints > 0)  if (g_sprite[h].strength != 0)
                    {
                        int hit = 0;
                        if (g_sprite[h].strength == 1) hit = g_sprite[h].strength - g_sprite[j].defense; else

                            hit = (g_sprite[h].strength / 2) + ((rand() % (g_sprite[h].strength / 2))+1)
                            - g_sprite[j].defense;

                        if (hit < 0) hit = 0;


                        g_sprite[j].damage += hit;
                        if (hit > 0)
                        {
                            random_blood(g_sprite[j].x, g_sprite[j].y-40, j);
                        }
                        g_sprite[j].last_hit = 1;
                        //Msg("Damage done is %d..", spr[j].damage);
                    }

                    if (g_sprite[j].script > 0)
                    {
                        //CHANGED did = h
                        *pmissile_target = 1;

                        if (locate(g_sprite[j].script, "HIT"))
                        {
                            kill_returning_stuff(g_sprite[j].script);
                            run_script(g_sprite[j].script);
                        }
                    }
                }
            }
    }

}


void missile_brain_expire(int h)
{
    missile_brain(h, false);
    if (g_sprite[h].seq == 0) g_sprite[h].active = 0;
}

void run_through_mouse_list(int h, bool special)
{
    rtRect32 box;

    for (int i = 1; i <= g_dglos.last_sprite_created; i++)
    {

        if (g_sprite[i].active) if (i != h) if
            ((g_sprite[i].touch_damage != 0) )
        {

            if (g_sprite[i].touch_damage != -1) if (g_sprite[h].notouch) return;
            box = g_dglos.g_picInfo[getpic(i)].hardbox;
            OffsetRect(&box, g_sprite[i].x, g_sprite[i].y);

            if (inside_box(g_sprite[h].x, g_sprite[h].y, box))
            {   

                if ((g_sprite[i].touch_damage == -1) && (g_sprite[i].script != 0))
                {
                    //LogMsg("running %d's script..",g_sprite[i].script);
                    if (locate(g_sprite[i].script, "CLICK")) run_script(g_sprite[i].script);
                } 
                else
                {
                    if (g_sprite[i].touch_damage == -1)
                    {
                        LogMsg("Sprites touch damage is set to -1 but there is no script set!");
                    } else
                    {
                        //lets hurt the guy
                    }
                }

                if (special) return;    
            }
        }

    }

    if (special)        SoundPlayEffect(19, 22050, 0, 0,0);

}


void mouse_brain(int h)
{

    if (g_sprite[h].move_active) 
    {
        process_move(h);
        return;
    }

	int diag = 0;

    if (sjoy.right) diag++;
    if (sjoy.left) diag++;
    if (sjoy.down) diag++;
    if (sjoy.up) diag++;

    //*********************************PROCESS MOVEMENT                         

    if (diag == 1)
    {

        if (sjoy.right)
        {
            move(h,g_sprite[h].speed,'+','0');
            changedir(6,h,g_sprite[h].base_walk);
        }

        if (sjoy.left) 
        {
            move(h,g_sprite[h].speed,'-','0');
            changedir(4,h,g_sprite[h].base_walk);
        }

        if (sjoy.down)
        {
            move(h,g_sprite[h].speed,'0','+');
            changedir(2,h,g_sprite[h].base_walk);
        }

        if (sjoy.up) 
        {
            move(h,g_sprite[h].speed,'0','-');
            changedir(8,h,g_sprite[h].base_walk);
        }

    }
    // ***************** DIAGONAL!!!!

    if (diag > 1)
    {

        if ( (sjoy.up) && (sjoy.left) ) 
        {
            changedir(7,h,g_sprite[h].base_walk);
            move(h,g_sprite[h].speed - (g_sprite[h].speed / 3),'-','-');
        }

        if ( (sjoy.down) && (sjoy.left))
        {
            changedir(1,h,g_sprite[h].base_walk);
            move(h,g_sprite[h].speed - (g_sprite[h].speed / 3),'-','+');
        }

        if ( (sjoy.down) && (sjoy.right))
        {
            changedir(3,h,g_sprite[h].base_walk);
            move(h,g_sprite[h].speed - (g_sprite[h].speed / 3),'+','+');
        }


        if ( (sjoy.up) && (sjoy.right))
        {
            changedir(9,h,g_sprite[h].base_walk);
            move(h,g_sprite[h].speed - (g_sprite[h].speed / 3),'+','-');
        }
    }

    if ( sjoy.button[1] == true || g_dinkMouseRightClick)
    {
        //LogMsg("running through mouse list..");
        run_through_mouse_list(h, true);
        sjoy.button[1] = false;
		g_dinkMouseRightClick = false;
    }

}

void process_bow( int h)
{
    int timetowait = 100;

    if (g_dglos.g_bowStatus.wait < g_dglos.g_dinkTick)
    {
        if (sjoy.right) g_sprite[h].dir = 6;
        if (sjoy.left) g_sprite[h].dir = 4;
        if (sjoy.up) g_sprite[h].dir = 8;
        if (sjoy.down) g_sprite[h].dir = 2;
    }

    if (sjoy.right) if (sjoy.up) 
    {
        g_sprite[h].dir = 9;
        g_dglos.g_bowStatus.wait = g_dglos.g_dinkTick + timetowait;
    }
    if (sjoy.left) if (sjoy.up) 
    {
        g_sprite[h].dir = 7;
        g_dglos.g_bowStatus.wait = g_dglos.g_dinkTick + timetowait;
    }
    if (sjoy.right) if (sjoy.down) 
    {
        g_sprite[h].dir = 3;
        g_dglos.g_bowStatus.wait = g_dglos.g_dinkTick + timetowait;

    }
    if (sjoy.left) if (sjoy.down) 
    {
        g_sprite[h].dir = 1;
        g_dglos.g_bowStatus.wait = g_dglos.g_dinkTick + timetowait;

    }
    g_sprite[h].pseq = 100+g_sprite[h].dir;

    if (g_dglos.g_bowStatus.pull_wait < g_dglos.g_dinkTick)
    {
        g_dglos.g_bowStatus.pull_wait = g_dglos.g_dinkTick + 10;
        if (g_dglos.g_bowStatus.hitme) g_dglos.g_bowStatus.time += 7;
        if (g_dglos.g_bowStatus.time > 500) g_dglos.g_bowStatus.time = 500;
        
		float progress = (float(g_dglos.g_bowStatus.time)/500); //was +1

		g_sprite[h].pframe =  (float(4)*progress)+1;
#ifdef _DEBUG
	//LogMsg("sprite %d's pframe is now %d", h, g_sprite[h].pframe);
#endif

    }

    if (sjoy.letgo[1])
    {
        g_dglos.g_bowStatus.active = false;
        g_dglos.g_bowStatus.last_power = g_dglos.g_bowStatus.time;
        run_script(g_dglos.g_bowStatus.script);
        return;
    }

}

bool DinkIsWaitingForSkippableDialog()
{
	return (g_sprite[1].active && g_sprite[1].brain == 1 && g_sprite[1].freeze && !g_dglos.g_talkInfo.active);
}

bool DinkSkipDialogLine()
{
	bool bSkipped = false;

	for (int jj = 1; jj <=  g_dglos.last_sprite_created; jj++)
	{

		//Msg("Checking %d, brain %d, script %d, my freeze is %d",jj, spr[jj].brain, spr[jj].script, spr[h].freeze);
		if (g_sprite[jj].brain == 8) if (g_sprite[jj].script == g_dglos.g_playerInfo.last_talk)
		{
			//this sprite owns its freeze

			g_sprite[jj].kill_timer = 1;
			
			//force the message to be over
			bSkipped = true;
		}
	}

	return bSkipped;
}

bool DinkCanRunScriptNow()
{
	if (GetDinkGameMode() != DINK_GAME_MODE_NORMAL) return false;
 
	if (!g_sprite[1].active) return false;
	
	if (g_sprite[1].move_active) return false;

	if (g_sprite[1].nocontrol) return false;

	if (g_sprite[1].freeze) return false;

	if (GetDinkSubGameMode() == DINK_SUB_GAME_MODE_DIALOG) return false;
	if (GetDinkSubGameMode() == DINK_SUB_GAME_MODE_SHOWING_BMP) return false;
	
	if (g_bTransitionActive) return false;
	return true;
}

bool DinkLoadPlayerScript(const string fileName)
{
	int mycrap = load_script(fileName.c_str(), 1, false, true);
	if (locate(mycrap, (char*)"MAIN")) 
	{
		run_script(mycrap);
		return true;
	}

	return false;
}

void human_brain(int h)
{
    int diag;
    int crap;
    bool bad;

    if (g_dglos.g_gameMode == 0) goto b1end;          

    if (g_sprite[h].move_active) 
    {
        process_move(h);
        return;
    }

    if (g_sprite[h].damage > 0)
    {
        draw_damage(h);

        *plife -= g_sprite[h].damage;

        g_sprite[h].damage = 0;
        if (*plife < 0) *plife = 0;

        int hurt = (rand() % 2)+1;

        if (hurt == 1) SoundPlayEffect( 15,25050, 2000 ,0,0);
        if (hurt == 2) SoundPlayEffect( 16,25050, 2000 ,0,0);

        //draw blood
    }


    if (g_dglos.g_playerInfo.push_active)
    {

        if (g_dglos.g_playerInfo.push_dir == 2) if (!sjoy.down) 
        {
            g_sprite[h].nocontrol = false;
            g_dglos.g_playerInfo.push_active = false;
        }

        if (g_dglos.g_playerInfo.push_dir == 4) if (!sjoy.left) 
        {
            g_sprite[h].nocontrol = false;
            g_dglos.g_playerInfo.push_active = false;
        }
        if (g_dglos.g_playerInfo.push_dir == 6) if (!sjoy.right) 
        {
            g_sprite[h].nocontrol = false;
            g_dglos.g_playerInfo.push_active = false;
        }

        if (g_dglos.g_playerInfo.push_dir == 8) if (!sjoy.up) 
        {
            g_sprite[h].nocontrol = false;
            g_dglos.g_playerInfo.push_active = false;
        }
    }

    if (g_sprite[h].nocontrol) return;                   

    if (g_dglos.g_talkInfo.active) goto freeze;


    if ( g_sprite[h].freeze)
    {
        //they are frozen

        if ( (sjoy.button[2] == true) || (sjoy.key[32]))
        {
            //they hit the talk button while frozen, lets hurry up the process

           DinkSkipDialogLine();
        }
        goto freeze;
    }
    //******************************  KEYS THAT CAN BE PRESSED AT ANY TIME **************

    if (g_dglos.g_bowStatus.active)
    {
        //bow is active!!
        process_bow(h);
        return;
    }

    if (g_dglos.g_playerInfo.push_active) if (g_dglos.g_playerInfo.push_timer + 600 < g_dglos.g_dinkTick)
    {
        g_sprite[h].seq = g_dglos.mDinkBasePush+g_sprite[h].dir;
        g_sprite[h].frame = 1;
        g_sprite[h].nocontrol = true;
        //play.push_active = false;
        run_through_tag_list_push(h);
        return;
    }

    if ( (sjoy.button[2] == true) )
    {
        if (!run_through_tag_list_talk(h))
        {
            //redink1 addition of 'not talking to anything' script
         
			int sc = load_script("DNOTALK", 0, false, true);
            if (sc != 0 && locate(sc,"MAIN"))
            {
                run_script(sc);
            }
            else
			
            {
                kill_text_owned_by(h);  
                int randy = (rand() % 6)+1;

                if (randy == 1) say_text((char*)"`$I don't see anything here.",h,0);
                if (randy == 2) say_text((char*)"`$Huh?",h,0);
                if (randy == 3) say_text((char*)"`$I'm fairly sure I can't talk to or use that.",h,0);
                if (randy == 4) say_text((char*)"`$What?",h,0);
                if (randy == 5) say_text((char*)"`$I'm bored.",h,0);
                if (randy == 6) say_text((char*)"`$Not much happening here.",h,0);
            }
        }   
    }

    if ( (sjoy.button[1] == true) && (g_dglos.weapon_script != 0) && (!sjoy.button[2] == true))
    {

        if (g_sprite[h].base_hit > 0)
        {
            if (locate(g_dglos.weapon_script, "USE")) run_script(g_dglos.weapon_script);
            
			goto b1end;
        }
    }

#ifdef C_DINK_KEYBOARD_INPUT
    //added AGAIN 10-19-99
    //Let's check keys for getting hit
   
	/*
	int x5;
	if (g_bHasFocus)
	{

	if (g_dglos.but_timer < g_dglos.g_dinkTick)  
    {
             for (x5=29; x5<255; x5++)
            { 
                if (x5 == 32) x5++;
                if (x5 == 54) x5++;
                if (x5 == 55) x5++;
                if (x5 == 37) x5++;
                if (x5 == 38) x5++;
                if (x5 == 39) x5++;
                if (x5 == 40) x5++;
                if (x5 == 77) x5++;

				char msg[32];
              
				if (GetKeyboard(x5))
                {
                    sprintf(msg, "key-%d",x5);
                    g_dglos.but_timer = g_dglos.g_dinkTick+200;


					if (DinkLoadPlayerScript(msg))
					{
						goto b1end;
					}
                }
            }
	    }
	}
	*/
#endif

    
    if ( sjoy.button[6] == true )
    {
        int mycrap = load_script("BUTTON6", 1, false, true); //map
        if (locate(mycrap, (char*)"MAIN")) run_script(mycrap);
        goto b1end;
    }

    if ( sjoy.button[7] == true )
    {
        int mycrap = load_script("BUTTON7", 1, false, true);
        if (locate(mycrap, (char*)"MAIN")) run_script(mycrap);
        goto b1end;
    }

    if ( sjoy.button[8] == true )
    {
        int mycrap = load_script("BUTTON8", 1, false, true);
        if (locate(mycrap, (char*)"MAIN")) run_script(mycrap);
        goto b1end;
    }

    if ( sjoy.button[9] == true )
    {
        int mycrap = load_script("BUTTON9", 1, false, true);
        if (locate(mycrap, (char*)"MAIN")) run_script(mycrap);
        goto b1end;
    }

    if ( sjoy.button[10] == true )
    {
        int mycrap = load_script("BUTTON10", 1, false, true);
        if (locate(mycrap, (char*)"MAIN")) run_script(mycrap);
        goto b1end;
    }

    if (g_dglos.magic_script != 0) if (sjoy.joybit[3]) goto shootm;
    if ( (sjoy.button[3] == true) )
    {
        if (g_dglos.magic_script == 0)
        {
            //redink1 addition of 'no magic' script
            
			int sc = load_script("DNOMAGIC", 0, false, true);
            if (sc != 0 && locate(sc,"MAIN"))
            {
                run_script(sc);
            }
            else
			
            {
                int randy = (rand() % 6)+1;
                kill_text_owned_by(h);  

                if (randy == 1) say_text((char*)"`$I don't know any magic.",h,0);
                if (randy == 2) say_text((char*)"`$I'm no wizard!",h,0);
                if (randy == 3) say_text((char*)"`$I need to learn magic before trying this.",h,0);
                if (randy == 4) say_text((char*)"`$I'm gesturing wildly to no avail!",h,0); //redink1 removed an extra space
                if (randy == 5) say_text((char*)"`$Nothing happened.",h,0);
                if (randy == 6) say_text((char*)"`$Hocus pocus!",h,0);
            }

            goto b1end;
        }

        //player pressed 1
        //lets magiced something
shootm: 
        if (*pmagic_level >= *pmagic_cost)
        {
            if (locate(g_dglos.magic_script, "USE")) run_script(g_dglos.magic_script);

            goto b1end; 
        } 
    }

    if (sjoy.button[4])
    {
        //redink1 addition of 'enter key/inventory' script
        int sc = load_script("BUTTON4", 0, false, true);
        if (sc != 0) {
            if (locate(sc,"MAIN")) {
                run_script(sc);
                return;
            }
        }

        g_itemScreenActive = true;
		if (!IsLargeScreen() || IsIphone4Size)
		{
			g_dglo.SetViewOverride(DinkGlobals::VIEW_ZOOMED);
		}
        SoundPlayEffect(18, 22050,0,0,0);
        return;
    }

#if defined(C_DINK_KEYBOARD_INPUT) && defined(_DEBUG)
    
	if ( GetKeyboard(50) )
    {
		if (g_bHasFocus)
		{

			//player pressed 2
            //lets add a duck with brain 2

            crap = add_sprite(g_sprite[h].x-20,g_sprite[h].y-50,3,26,1);
            g_sprite[crap].speed = 1;
            g_sprite[crap].base_walk = 20;
            g_sprite[crap].exp = 11;
            g_sprite[crap].hitpoints = 5;
		}
    }
#endif

    if ( (sjoy.button[5] == true) )
    {

        if (!g_dglos.g_bShowingBitmap.active) if (!g_dglos.g_bowStatus.active) if (!g_dglos.g_talkInfo.active)
        {
            int sc = load_script("ESCAPE", 1000, false);
            if (sc != 0) if (locate(sc,"MAIN")) run_script(sc);
            return;
        }
    }

    if (g_sprite[h].skip > 0)

        if (g_sprite[h].skip <= g_sprite[h].skiptimer)
        {
            g_sprite[h].skiptimer = 0;
            goto b1end;
        }

        diag = 0;
        if (sjoy.right) diag++;
        if (sjoy.left) diag++;
        if (sjoy.down) diag++;
        if (sjoy.up) diag++;

        //*********************************PROCESS MOVEMENT                         

        if (diag == 1)
        {

            if (sjoy.right)
            {
                move(h,g_sprite[h].speed,'+','0');
                changedir(6,h,g_sprite[h].base_walk);
            }

            if (sjoy.left) 
            {
                move(h,g_sprite[h].speed,'-','0');
                changedir(4,h,g_sprite[h].base_walk);
            }

            if (sjoy.down)
            {
                move(h,g_sprite[h].speed,'0','+');
                changedir(2,h,g_sprite[h].base_walk);
	           }


            if (sjoy.up) 
            {
                move(h,g_sprite[h].speed,'0','-');
                changedir(8,h,g_sprite[h].base_walk);
            }

        }
        // ***************** DIAGONAL!!!!


        if (diag > 1) if (diag < 3)
        {

            if ( (sjoy.up) && (sjoy.left) ) 
            {
                changedir(7,h,g_sprite[h].base_walk);
                move(h,g_sprite[h].speed - (g_sprite[h].speed / 3),'-','-');

            }

            if ( (sjoy.down) && (sjoy.left))
            {
                changedir(1,h,g_sprite[h].base_walk);
                move(h,g_sprite[h].speed - (g_sprite[h].speed / 3),'-','+');

            }

            if ( (sjoy.down) && (sjoy.right))
            {
                changedir(3,h,g_sprite[h].base_walk);
                move(h,g_sprite[h].speed - (g_sprite[h].speed / 3),'+','+');
            }


            if ( (sjoy.up) && (sjoy.right))
            {
                changedir(9,h,g_sprite[h].base_walk);
                move(h,g_sprite[h].speed - (g_sprite[h].speed / 3),'+','-');
            }

        }

        bad = false;
        if (sjoy.right) bad = true;    
        if (sjoy.left) bad = true;    
        if (sjoy.up) bad = true;    
        if (sjoy.down) bad = true;    

        if (bad)
        {
            if (g_sprite[h].idle)
            {
                g_sprite[h].frame = 1;
                g_sprite[h].idle = false;
            }
            goto badboy;
        }


        if (not_in_this_base(g_sprite[h].seq, g_sprite[h].base_idle)) //uncomment to allow walk anim to end before idle anim to start
        {
freeze:
            if (g_sprite[h].dir == 1) g_sprite[h].dir = 2;
            if (g_sprite[h].dir == 3) g_sprite[h].dir = 2;
            if (g_sprite[h].dir == 7) g_sprite[h].dir = 8;
            if (g_sprite[h].dir == 9) g_sprite[h].dir = 8;

            if (g_sprite[h].base_idle != 0) changedir(g_sprite[h].dir,h,g_sprite[h].base_idle);                                
            g_sprite[h].idle = true;   
        }
badboy: 

b1end:;

check_sprite_status(h);

        if ( (g_sprite[h].dir == 2)  | (g_sprite[h].dir == 4) | (g_sprite[h].dir == 6) | (g_sprite[h].dir == 8)) goto smoothend;
        crap = check_if_move_is_legal(h);
        if (crap != 0)
        {
            if (g_dglos.g_smallMap.sprite[crap-100].prop != 0) g_dglos.flub_mode = crap;

            //hit something, can we move around it?


            if( (g_sprite[h].seq == g_sprite[h].base_walk + 4) |
                (g_sprite[h].seq == g_sprite[h].base_walk + 6) )
            {
                int hardm = get_hard_play(h, g_sprite[h].x, g_sprite[h].y-1);
                if (hardm == 0)
                {  
                    g_sprite[h].y -= 1;
                }
            }

            if( (g_sprite[h].seq == g_sprite[h].base_walk + 8) |
                (g_sprite[h].seq == g_sprite[h].base_walk + 2) )
            {
                int hardm = get_hard_play(h, g_sprite[h].x-1, g_sprite[h].y);
                if (hardm == 0)
                {  
                    g_sprite[h].x -= 1;
                }
            }

            if (g_sprite[h].seq == g_sprite[h].base_walk + 9)
            {
                int hardm = get_hard_play(h, g_sprite[h].x+1, g_sprite[h].y);
                if (hardm == 0)
                {  
                    g_sprite[h].x += 1;

                } else
                {
                    int hardm = get_hard_play(h, g_sprite[h].x+1, g_sprite[h].y+1);
                    if (hardm == 0)
                    {  
                        g_sprite[h].x += 1;
                        g_sprite[h].y += 1;
                    } else
                    {
                        int hardm = get_hard_play(h, g_sprite[h].x+1, g_sprite[h].y+2);
                        if (hardm == 0)
                        {  
                            g_sprite[h].x += 1;
                            g_sprite[h].y += 2;
                        } else
                        {
                            int hardm = get_hard_play(h, g_sprite[h].x, g_sprite[h].y-1);
                            if (hardm == 0)
                            {  
                                g_sprite[h].y -= 1;

                            } else
                            {
                                int hardm = get_hard_play(h, g_sprite[h].x-1, g_sprite[h].y-1);
                                if (hardm == 0)
                                {  
                                    g_sprite[h].x -= 1;
                                    g_sprite[h].y -= 1;
                                }

                            }
                        }
                    }
                }
            }

            if (g_sprite[h].seq == g_sprite[h].base_walk + 7)
            {
                int hardm = get_hard_play(h, g_sprite[h].x-1, g_sprite[h].y);
                if (hardm == 0)
                {  
                    g_sprite[h].x -= 1;

                } else
                {
                    int hardm = get_hard_play(h, g_sprite[h].x-1, g_sprite[h].y+1);
                    if (hardm == 0)
                    {  
                        g_sprite[h].x -= 1;
                        g_sprite[h].y += 1;
                    } else
                    {
                        int hardm = get_hard_play(h, g_sprite[h].x-1, g_sprite[h].y+2);
                        if (hardm == 0)
                        {  
                            g_sprite[h].x -= 1;
                            g_sprite[h].y += 2;
                        } else
                        {

                            int hardm = get_hard_play(h, g_sprite[h].x, g_sprite[h].y-1);
                            if (hardm == 0)
                            {               
                                g_sprite[h].y -= 1;
                            } else
                            {
                                int hardm = get_hard_play(h, g_sprite[h].x+1, g_sprite[h].y-1);
                                if (hardm == 0)
                                {               
                                    g_sprite[h].x += 1;
                                    g_sprite[h].y -= 1;
                                }
                            }
                        }
                    }
                }
            }

            if (g_sprite[h].seq == g_sprite[h].base_walk + 1)
            {
                int hardm = get_hard_play(h, g_sprite[h].x-1, g_sprite[h].y);
                if (hardm == 0)
                {
                    g_sprite[h].x -= 1;
                } else
                {
                    int hardm = get_hard_play(h, g_sprite[h].x-1, g_sprite[h].y-1);
                    if (hardm == 0)
                    {  
                        g_sprite[h].x -= 1;
                        g_sprite[h].y -= 1;
                    } else
                    {
                        int hardm = get_hard_play(h, g_sprite[h].x-1, g_sprite[h].y-2);
                        if (hardm == 0)
                        {  
                            g_sprite[h].x -= 1;
                            g_sprite[h].y -= 2;
                        } else
                        {
                            int hardm = get_hard_play(h, g_sprite[h].x, g_sprite[h].y+1);
                            if (hardm == 0)
                            {  

                                g_sprite[h].y += 1;
                            } else
                            {
                                int hardm = get_hard_play(h, g_sprite[h].x+1, g_sprite[h].y+1);
                                if (hardm == 0)
                                {  
                                    g_sprite[h].x += 1;
                                    g_sprite[h].y += 1;
                                } 

                            }

                        }

                    }
                }

            }

            if (g_sprite[h].seq == g_sprite[h].base_walk + 3)
            {
                int hardm = get_hard_play(h, g_sprite[h].x+1, g_sprite[h].y);
                if (hardm == 0)
                {  
                    g_sprite[h].x += 1;

                } else
                {
                    int hardm = get_hard_play(h, g_sprite[h].x+1, g_sprite[h].y-1);
                    if (hardm == 0)
                    {  
                        g_sprite[h].x += 1;
                        g_sprite[h].y -= 1;
                    } else
                    {
                        int hardm = get_hard_play(h, g_sprite[h].x+1, g_sprite[h].y-2);
                        if (hardm == 0)
                        {  
                            g_sprite[h].x += 1;
                            g_sprite[h].y -= 2;
                        } else
                        {
                            int hardm = get_hard_play(h, g_sprite[h].x, g_sprite[h].y+1);
                            if (hardm == 0)
                            {  

                                g_sprite[h].y += 1;
                            } else
                            {
                                int hardm = get_hard_play(h, g_sprite[h].x-1, g_sprite[h].y+1);
                                if (hardm == 0)
                                {  
                                    g_sprite[h].x -= 1;
                                    g_sprite[h].y += 1;
                                }

                            }

                        }

                    }


                }

            }
        }


smoothend:;
}



int find_sprite(int block)
{

    for (int k = 1; k <= g_dglos.last_sprite_created; k++)
    {
        if (g_sprite[k].sp_index == block)
        {
            return(k);
        }
    }
    return(0);
}

int special_block(int block, int h)
{
    if (g_dglos.g_smallMap.sprite[block].prop == 1)
    {
        //they touched a warp       

        if (g_dglos.g_smallMap.sprite[block].sound == 0)
            SoundPlayEffect( 7,12000, 0 , 0,0); else
            SoundPlayEffect( g_dglos.g_smallMap.sprite[block].sound,22050, 0 , 0,0);

        if (g_dglos.g_smallMap.sprite[block].parm_seq != 0)
        {
            //we'll also play an animation here

            int sprite = find_sprite(block);
            if (sprite > 0)
            {
                g_sprite[sprite].seq = g_dglos.g_smallMap.sprite[block].parm_seq;
                g_dglos.process_warp = block;
            }
            return(1);
        }
        g_dglos.process_warp = block;   
       
        return(1); //redraw screen with fade
    }
    return(0);
}

void down_cycle()
{
    //redink1 truecolor fadedown...
    g_dglos.process_downcycle = true;

	float alpha = float(g_dglos.g_dinkTick - g_dglos.cycle_clock)/ (float)C_DINK_FADE_TIME_MS;
	ForceRange(alpha, 0, 1);
	g_dinkFadeAlpha = alpha;

//	LogMsg("Alpha is %.2f", alpha);
}

void up_cycle(void)
{
    //redink1 added this for true-color fade support
      g_dglos.process_upcycle = true;

	  float alpha = float(g_dglos.g_dinkTick - g_dglos.cycle_clock)/ (float)C_DINK_FADE_TIME_MS;
	  ForceRange(alpha, 0, 1);
	  g_dinkFadeAlpha  = 1-alpha;
	

	 // LogMsg("Alpha is %.2f", alpha);
}

void draw_box(rtRect32 box, int color)
{
   
	DrawRect(box);
	//DDBLTFX     ddbltfx;
	

    //ddbltfx.dwSize = sizeof(ddbltfx);
    //ddbltfx.dwFillColor = color;
    //ddrval = lpDDSBack->Blt(&box ,NULL, NULL, DDBLT_COLORFILL| DDBLT_WAIT, &ddbltfx);
}

//redink1 and Invertigo fix for windowed/high color mode
void flip_it(void)
{
  
    //redink1 fix for true-color transition
    //The idea is to apply the fade to the backbuffer right before the main flip/blt.
    if (g_dglos.process_downcycle || g_dglos.process_upcycle || g_dglos.bFadedDown)
    {
     
        //Make sure we're not 'stuck'... i.e. fade down when already black, or fade up when not black
        if (g_dglos.process_downcycle && g_dglos.bFadedDown || g_dglos.process_upcycle && !g_dglos.bFadedDown)
        {
            if (g_dglos.process_downcycle)
                g_dglos.process_downcycle = false;
            if (g_dglos.process_upcycle)
            {
                g_dglos.process_upcycle = false;
                if ( g_dglos.mSwapped )
                {
                       
	                    g_dglos.mSwapped = false;
                }
            }
           
			if (g_dglos.cycle_script != 0)
            {
                int junk = g_dglos.cycle_script;
                g_dglos.cycle_script = 0;
                run_script(junk);   
            }
        }
     
		if ( g_dglos.process_downcycle || g_dglos.bFadedDown || g_dglos.process_upcycle )
        {
			//just tell them the fade is complete for now

			if ( g_dglos.process_downcycle == true )
                {
                  
					if (g_dglos.g_dinkTick > g_dglos.cycle_clock+C_DINK_FADE_TIME_MS)
					{
						g_dglos.bFadedDown = true;
					}
					
                }
                else if ( g_dglos.process_upcycle == true )
                {
					if (g_dglos.g_dinkTick > g_dglos.cycle_clock+C_DINK_FADE_TIME_MS)
					{
						  g_dglos.bFadedDown = false;
					}
                  
                }
        }
    }
}

void run_through_tag_list(int h, int strength)
{
    rtRect32 box;
    int amount, amounty;

    for (int i = 1; i <= g_dglos.last_sprite_created; i++)
    {
        if (g_sprite[i].active) if (i != h) if
            (! ( (g_sprite[i].nohit == 1) && (g_sprite[i].script == 0)) )
        {
            box = g_dglos.g_picInfo[getpic(i)].hardbox;
            OffsetRect(&box, g_sprite[i].x, g_sprite[i].y);
            box.right += 5;
            box.left -= 5;
            box.top -= 5;
            box.bottom += 10;
            if (g_sprite[h].range == 0)      
                amount = 28; else amount = g_sprite[h].range;

            if (g_sprite[h].range == 0)      
                amounty = 36; else amounty = (g_sprite[h].range + (g_sprite[h].range / 6));

            int range_amount = g_sprite[h].range / 8;

            if (g_sprite[h].dir == 6)
            {
                box.top -= 10;
                box.bottom += 10;
                if (g_sprite[h].range != 0) box.top -= range_amount;
                if (g_sprite[h].range != 0) box.bottom += range_amount;
                box.left -= amount;
            }

            if (g_sprite[h].dir == 4)
            {
                box.right += amount;

                box.top -= 10;
                box.bottom += 10;
                if (g_sprite[h].range != 0) box.top -= range_amount;
                if (g_sprite[h].range != 0) box.bottom += range_amount;
            }

            if (g_sprite[h].dir == 2)
            {
                box.right += 10;
                box.left -= 10;
                box.top -= amounty;

                if (g_sprite[h].range != 0) box.right += range_amount;
                if (g_sprite[h].range != 0) box.left -= range_amount;
            }

            if (g_sprite[h].dir == 8)
            {
                box.right += 10;
                box.left -= 10;
                box.bottom += amounty;

                if (g_sprite[h].range != 0) box.right += range_amount;
                if (g_sprite[h].range != 0) box.right -= range_amount;
            }

            if (g_script_debug_mode) draw_box(box, 33);

            if (inside_box(g_sprite[h].x, g_sprite[h].y, box))
            {   
                //redink1 addition for fixing missle_source problems
                *pmissle_source = h;
                if (g_sprite[i].nohit == 1)
                {
                    if (g_sprite[i].script > 0)
                    {
                        //if (  (spr[i].brain == 0) | (spr[i].brain == 5) | (spr[i].brain == 6) | (spr[i].brain == 7))
                        *penemy_sprite = h;

                        if (  (g_sprite[i].base_attack != -1) || (g_sprite[i].touch_damage > 0))
                            g_sprite[i].target = h;   

                        if (locate(g_sprite[i].script, "HIT"))
                        {
                            kill_returning_stuff(g_sprite[i].script);
                            run_script(g_sprite[i].script);
                        }
                    }

                } else
                {
                    //hit this personb/thing
                    if (g_sprite[h].attack_hit_sound == 0)
                    {
                        SoundPlayEffect( 9,22050, 0 ,0,0);
                    } else
                    {
                        SoundPlayEffect( g_sprite[h].attack_hit_sound,g_sprite[h].attack_hit_sound_speed, 0 ,0,0);
                    }
                    if (  (g_sprite[i].base_attack != -1) || (g_sprite[i].touch_damage > 0))
                        g_sprite[i].target = h;   
                    if (g_sprite[h].strength == 0)
                    {

                    } else
                    {
                        if (  (g_sprite[i].hitpoints > 0) || (i == 1) )
                        {

                            g_sprite[i].last_hit = h; 
                            if ( hurt_thing(i, (g_sprite[h].strength / 2) + ((rand() % ((g_sprite[h].strength+1) / 2))+1), 0) > 0)
                                random_blood(g_sprite[i].x, g_sprite[i].y-40, i); //redink1
                        }

                    }
                    if (g_sprite[i].script > 0)
                    {
                        //if (  (spr[i].brain == 0) | (spr[i].brain == 5) | (spr[i].brain == 6) | (spr[i].brain == 7))
                        g_sprite[i].last_hit = h;    
                        *penemy_sprite = h;
                        if (  (g_sprite[i].base_attack != -1) || (g_sprite[i].touch_damage > 0))
                            g_sprite[i].target = h;   

                        if (locate(g_sprite[i].script, "HIT"))
                        {
                            kill_returning_stuff(g_sprite[i].script);
                            run_script(g_sprite[i].script);
                        }

                    }

                }

            }

        }

    }

}



void run_through_tag_list_push(int h)
{
    rtRect32 box;

    for (int i = 1; i <= g_dglos.last_sprite_created; i++)
    {
        if (g_sprite[i].active) if (i != h) if
            ((g_sprite[i].script != 0) )
        {

            box = g_dglos.g_picInfo[getpic(i)].hardbox;
            OffsetRect(&box, g_sprite[i].x, g_sprite[i].y);

            //InflateRect(&box, 10,10);

            box.right += 2;
            box.left -= 2;
            box.top -= 2;
            box.bottom += 2;
            //draw_box(box, 33);

            if (inside_box(g_sprite[h].x, g_sprite[h].y, box))
            {   
                if (locate(g_sprite[i].script, "PUSH")) run_script(g_sprite[i].script);
            }
        }
    }
}

void run_through_touch_damage_list(int h)
{
    rtRect32 box;
 
    for (int i = 1; i <= g_dglos.last_sprite_created; i++)
    {
        if (g_sprite[i].active) if (i != h) if
            ((g_sprite[i].touch_damage != 0) )
        {

            if (g_sprite[i].touch_damage != -1) if (g_sprite[h].notouch) return;
            box = g_dglos.g_picInfo[getpic(i)].hardbox;
            OffsetRect(&box, g_sprite[i].x, g_sprite[i].y);

            //InflateRect(&box, 10,10);

            box.right += 2;
            box.left -= 2;
            box.top -= 2;
            box.bottom += 2;
            if (g_script_debug_mode)     
                draw_box(box, 33);

            if (inside_box(g_sprite[h].x, g_sprite[h].y, box))
            {   

                if ((g_sprite[i].touch_damage == -1) && (g_sprite[i].script != 0))
                {
                    if (locate(g_sprite[i].script, "TOUCH")) run_script(g_sprite[i].script);
                } else
                {
                    if (g_sprite[i].touch_damage == -1)
                    {
                        LogMsg("Sprites touch damage is set to -1 but there is no script set!");
                    } else
                    {
                        //lets hurt the guy

                        g_sprite[h].notouch = true;
                        g_sprite[h].notouch_timer = g_dglos.g_dinkTick+400;
                        g_sprite[h].last_hit = i;
                        if (g_sprite[i].script != 0)
                            if (locate(g_sprite[i].script, "TOUCH")) run_script(g_sprite[i].script);
                        if (hurt_thing(h, g_sprite[i].touch_damage, 0) > 0)
                            random_blood(g_sprite[h].x, g_sprite[h].y-40, h);
                    }
                }

            }
        }
    }
}


void process_warp_man(void)
{
    rtRect32 box_crap;
    DDBLTFX     ddbltfx;

    int sprite = find_sprite(g_dglos.process_warp);

    if (sprite == 0 || g_sprite[sprite].seq == 0)
    {
        g_dglos.process_count++;
    
		if (g_dglos.process_count == 1) 
		{
			g_dglos.cycle_clock = g_dglos.g_dinkTick;

			//redink1 Limit palette cycles for true color mode
            down_cycle();
		}
        
		if (g_dglos.process_downcycle == false) //redink1 more limits for fade down stuff
        {
            ddbltfx.dwSize = sizeof(ddbltfx);

            ddbltfx.dwFillColor = 0;
           box_crap = rtRect32 (0,0,C_DINK_SCREENSIZE_X,C_DINK_SCREENSIZE_Y);

			//int ddrval = lpDDSBack->Blt(&box_crap ,NULL, NULL, DDBLT_COLORFILL| DDBLT_WAIT, &ddbltfx);

            flip_it();

            g_dglos.process_count = 0;
            int block = g_dglos.process_warp;
            update_screen_time();
            g_sprite[1].x = g_dglos.g_smallMap.sprite[block].warp_x;
            g_sprite[1].y = g_dglos.g_smallMap.sprite[block].warp_y;
            *pmap = g_dglos.g_smallMap.sprite[block].warp_map; 

            //redink1 change so map indicator is correct on warp.
            if (g_MapInfo.indoor[g_dglos.g_smallMap.sprite[block].warp_map] == 0)
                g_dglos.g_playerInfo.last_map = g_dglos.g_smallMap.sprite[block].warp_map;

            load_map(g_MapInfo.loc[g_dglos.g_smallMap.sprite[block].warp_map]);

            BuildScreenBackground();

			g_dglos.cycle_clock = g_dglos.g_dinkTick;
			g_dinkFadeAlpha = 1;
            g_dglos.process_upcycle = true;
            g_dglos.process_warp = 0;
        }

    } else
    {
        g_dglos.process_count = 0;      
    }

}

void one_time_brain(int h)
{

    //goes once then draws last frame to background

    if (g_sprite[h].move_active) 
    {
        process_move(h);
        return;
    }

    if (g_sprite[h].follow > 0)
    {
        process_follow(h);
    }


    if (g_sprite[h].seq == 0)
    {
        
		g_dglo.m_bgSpriteMan.Add(h);
		//draw_sprite_game(lpDDSTwo,h);
        g_sprite[h].active = false;          
        return;
    }

    changedir(g_sprite[h].dir,h,-1);
    automove(h);

}

void one_time_brain_for_real(int h)
{

    if (g_sprite[h].move_active) 
    {
        process_move(h);
    }

    if (g_sprite[h].follow > 0)
    {
        process_follow(h);
    }

    if (g_sprite[h].seq == 0)
    {
        g_sprite[h].active = false;          
        return;
    }

	if (g_sprite[h].dir > 0)
    {
        changedir(g_sprite[h].dir,h,-1);
        automove(h);
    }
}

void scale_brain(int h)
{
    if (g_sprite[h].size == g_sprite[h].brain_parm)
    {
        g_sprite[h].active = false;
        return;
    }

    int num = 5 * (g_dglos.base_timing / 4);

    if (g_sprite[h].size > g_sprite[h].brain_parm)
    {
        if (g_sprite[h].size - num < g_sprite[h].brain_parm) num = g_sprite[h].size - g_sprite[h].brain_parm;
        g_sprite[h].size -= num;
    }

    if (g_sprite[h].size < g_sprite[h].brain_parm) 
    {
        if (g_sprite[h].size + num > g_sprite[h].brain_parm) num = g_sprite[h].brain_parm - g_sprite[h].size;   
        g_sprite[h].size += num;
    }
    if (g_sprite[h].move_active) 
    {
        process_move(h);
        return;
    }

    if (g_sprite[h].dir > 0)
    {
        changedir(g_sprite[h].dir,h,-1);
        automove(h);
    }
}

void repeat_brain(int h)
{

    if (g_sprite[h].move_active) 
    {
        process_move(h);
    }

    if (g_sprite[h].seq_orig == 0) if (g_sprite[h].sp_index != 0) 
    {
        g_sprite[h].seq_orig = g_dglos.g_smallMap.sprite[g_sprite[h].sp_index].seq;
        g_sprite[h].frame = g_dglos.g_smallMap.sprite[g_sprite[h].sp_index].frame;
        g_sprite[h].wait = 0;
    }

    if (g_sprite[h].seq == 0) g_sprite[h].seq = g_sprite[h].seq_orig;

}


void text_brain(int h)
{

    if (  (g_sprite[h].damage == -1) && (g_sprite[h].owner != 1000))
    {

        if (g_sprite[g_sprite[h].owner].active == false)
        {
            //msg("Killing text brain %d, because owner %d is dead.",h, spr[h].owner);
            g_sprite[h].active = false;
            return;
        }

        //give this text the cords from it's owner sprite
        g_sprite[h].x = g_sprite[g_sprite[h].owner].x - g_sprite[h].strength;


        g_sprite[h].y = g_sprite[g_sprite[h].owner].y - g_sprite[h].defense;

        if (g_sprite[h].x < 1) g_sprite[h].x = 1;

        if (g_sprite[h].y < 1) g_sprite[h].y = 1;


    } else
    {
        //Msg("automoving %d.. ", h);

        if (g_sprite[h].move_active) 
        {
            process_move(h);
            return;
        }

        automove(h);
    }

}


void process_talk()
{

    int px = 48, py = 44;

    int sx = 184;
    int sy = 94, sy_hold, sy_ho;
    int spacing = 12;
    int curxl = 126;
    int curxr = 462;
    int curyr = 200;
    int curyl = 200;

    int y_last = 0, y_hold = 0, y_ho = 0; 
    rtRect32 rcRect;
    int i;
    int x_depth = 335;
    if (g_dglos.g_talkInfo.newy != -5000)
        sy = g_dglos.g_talkInfo.newy;

    sy_hold = sy;
    sy_ho = sy;
	int ddrval;

	int fake_page;

    if (check_seq_status(30))
	{

	
	
    ddrval = lpDDSBack->BltFast( px, py, g_pSpriteSurface[g_dglos.g_seq[30].frame[2]],
        &g_dglos.g_picInfo[g_dglos.g_seq[30].frame[2]].box  , DDBLTFAST_SRCCOLORKEY  );

	

	ddrval = lpDDSBack->BltFast(px + 169, py + 42, g_pSpriteSurface[g_dglos.g_seq[30].frame[3]],
		&g_dglos.g_picInfo[g_dglos.g_seq[30].frame[3]].box, DDBLTFAST_SRCCOLORKEY);

	ddrval = lpDDSBack->BltFast(px + 169 + 180, py + 1, g_pSpriteSurface[g_dglos.g_seq[30].frame[4]],
		&g_dglos.g_picInfo[g_dglos.g_seq[30].frame[4]].box, DDBLTFAST_SRCCOLORKEY);
	/*
    ddrval = lpDDSBack->BltFast( px+170, py+42, g_pSpriteSurface[g_dglos.g_seq[30].frame[3]],
        &g_dglos.g_picInfo[g_dglos.g_seq[30].frame[3]].box  , DDBLTFAST_SRCCOLORKEY  );
   
    ddrval = lpDDSBack->BltFast( px+170+181, py+1, g_pSpriteSurface[g_dglos.g_seq[30].frame[4]],
        &g_dglos.g_picInfo[g_dglos.g_seq[30].frame[4]].box  , DDBLTFAST_SRCCOLORKEY  );
		*/

	}

    int talk_hold = g_dglos.g_talkInfo.cur;  
//    if (sjoy.rightd) g_dglos.g_talkInfo.cur++;
//	if (sjoy.leftd) g_dglos.g_talkInfo.cur--;
 
	if (sjoy.downd) g_dglos.g_talkInfo.cur++;
    if (sjoy.upd) g_dglos.g_talkInfo.cur--;
    
	
    if (g_dglos.g_playerInfo.mouse > 20)
    {
        g_dglos.g_talkInfo.cur++;
        g_dglos.g_playerInfo.mouse = 0;
    }

    if (g_dglos.g_playerInfo.mouse < -20)
    {
        g_dglos.g_talkInfo.cur--;
        g_dglos.g_playerInfo.mouse = 0;
    }

    if (talk_hold != g_dglos.g_talkInfo.cur)
    {
        if (g_dglos.g_talkInfo.cur >= g_dglos.g_talkInfo.cur_view) if (g_dglos.g_talkInfo.cur <= g_dglos.g_talkInfo.cur_view_end) 
            SoundPlayEffect(11, 22050,0,0,0);
    }
uint32 rgbColor = MAKE_RGBA(255,255,255,255);

        if (strlen(g_dglos.g_talkInfo.buffer) > 0)
        {

           rcRect = rtRect32 (sx,94,463,400);
            if (g_dglos.g_talkInfo.newy != -5000) rcRect.bottom = g_dglos.g_talkInfo.newy+15;
			
			int color = 15;

          //  SetTextColor(hdc,RGB(8,14,21));
            //DrawText(hdc,talk.buffer,strlen(talk.buffer),&rcRect,DT_VCENTER | DT_CENTER | DT_WORDBREAK);    

            if (g_dglos.g_talkInfo.color >= 1 && g_dglos.g_talkInfo.color <= 15)
            {
	             color = g_dglos.g_talkInfo.color;
            }

			rgbColor = MAKE_RGBA(g_dglos.font_colors[color].red, g_dglos.font_colors[color].green, g_dglos.font_colors[color].blue, 255);

          //  OffsetRect(&rcRect, 1, 1);
            //DrawText(hdc,talk.buffer,strlen(talk.buffer),&rcRect,DT_VCENTER | DT_CENTER | DT_WORDBREAK);    
			
            rtRect rTemp(rcRect);
            
            GetApp()->GetFont(FONT_SMALL)->DrawWrapped(rTemp, g_dglos.g_talkInfo.buffer, true, false, rgbColor,  g_dglo.m_fontSize)  ;
          //  SetTextColor(hdc,RGB(8,14,21));
        }

        //tabulate distance needed by text, LORDII experience helped here

        for (i = g_dglos.g_talkInfo.cur_view; i < g_dglos.g_talkInfo.last; i++)
        {
            rcRect = rtRect32 (sx,y_hold,463,x_depth+100);
            rtRect rTemp(rcRect);
            
            y_hold = (int)GetApp()->GetFont(FONT_SMALL)->DrawWrapped(rTemp, g_dglos.g_talkInfo.line[i], true, false, rgbColor, g_dglo.m_fontSize, true).y;
            sy_hold += y_hold;   

            //Msg("Sy_hold = %d (%d)", sy_hold,i);

            if (sy_hold > x_depth) 
            {

                g_dglos.g_talkInfo.cur_view_end = i-1;
                //Msg("Sy is over, sp cur_view is %d ", talk.cur_view_end);
                goto death;
            }
        }

        g_dglos.g_talkInfo.cur_view_end = i;

        if (g_dglos.g_talkInfo.cur_view == 1) if (g_dglos.g_talkInfo.cur_view_end == g_dglos.g_talkInfo.last)
        {

            //Msg("Small enough to fit on one screen, lets center it!");
            sy += ( (x_depth - sy_hold) / 2) - 20;

        }
death:


        if (g_dglos.g_talkInfo.cur > g_dglos.g_talkInfo.last) 
        {
            SoundPlayEffect(11, 22050,0,0,0);

            g_dglos.g_talkInfo.cur = 1;

        }
        if (g_dglos.g_talkInfo.cur < 1) 
        {
            SoundPlayEffect(11, 22050,0,0,0);

            g_dglos.g_talkInfo.cur = g_dglos.g_talkInfo.last;
        }

         //Msg("Talkcur is %d, talk cur view is %d", talk.cur, talk.cur_view);
            //total options too large for page, lets scroll


            if (g_dglos.g_talkInfo.cur > g_dglos.g_talkInfo.cur_view_end) 
            {
                //     Msg("advancing page:  talkcur is %d, changing cur_view to same", talk.cur, talk.cur_view);
                g_dglos.g_talkInfo.cur_view = g_dglos.g_talkInfo.cur;
                g_dglos.g_talkInfo.page ++;

                // Msg("Page advanced to %d. (cur_end is %d, cur is %d)", talk.page,talk.cur_view_end, talk.cur);
                goto fin;
            }

            if (g_dglos.g_talkInfo.cur < g_dglos.g_talkInfo.cur_view) 
            {
                //  Msg("Turning back the clock from page %d..", talk.page);

                g_dglos.g_talkInfo.cur_view = 1;
                // talk.cur = 1;

                g_dglos.g_talkInfo.page--;
                LogMsg("Page backed to %d.", g_dglos.g_talkInfo.page);
                fake_page = 1;
                for (i = 1; i < g_dglos.g_talkInfo.last; i++)
                {
                    rcRect = rtRect32 (sx,sy_ho,463,x_depth);
                    rtRect rTemp = rtRect(rcRect);
                    
					y_ho += (int)GetApp()->GetFont(FONT_SMALL)->DrawWrapped(rTemp, g_dglos.g_talkInfo.line[i], true, false, rgbColor, g_dglo.m_fontSize, true).y;
                    //y_ho =  DrawText(hdc,talk.line[i],lstrlen(talk.line[i]),&rcRect,DT_CALCRECT | DT_CENTER | DT_WORDBREAK);    
                    sy_ho += y_ho;   
                    //Msg("adding y_yo %d.. (on %d)", y_ho,i);
                    if (sy_ho > x_depth) 
                    {
                        /*if (fake_page == talk.page)
                        {
                        goto fin;
                        }
                        */
                        fake_page++;      
                        sy_ho = sy+ y_ho;
                        //Msg("Does fake page (%d) match desired page (%d) %d", fake_page, talk.page, i);
                    }
                    if (fake_page == g_dglos.g_talkInfo.page)
                    {
                        g_dglos.g_talkInfo.cur_view = i;
                        g_dglos.g_talkInfo.cur_view_end = g_dglos.g_talkInfo.cur;
                        //Msg("Going to fin with end being %d, and.cur being %d.  View is %d.",
                        //         talk.cur_view_end, talk.cur, talk.cur_view);
                        goto fin;
                    }
                    //         Msg("Second: Sy is over, sp cur_view is %d", talk.cur_view_end);
                }

                g_dglos.g_talkInfo.cur_view_end = i;
            }

        //Msg("talk last is %d.  cur_view_end is %d, Cur is %d", talk.last, talk.cur_view_end, talk.cur);

        for ( i = g_dglos.g_talkInfo.cur_view; i <= g_dglos.g_talkInfo.cur_view_end; i++)
        {
            //lets figure out where to draw this line

            rcRect = rtRect32 (sx,sy,463,x_depth+100);
            rtRect rTemp=rtRect(rcRect);
            
			if (i == g_dglos.g_talkInfo.cur)
            {
                curyl = sy-4;
                curyr = sy-4;
               // SetTextColor(hdc,RGB(255,255,255));
				rgbColor = MAKE_RGBA(255,255,255,255);
            }
            else
				rgbColor = MAKE_RGBA(255,255,2,255);
            
            
			   y_last =  (int)GetApp()->GetFont(FONT_SMALL)->DrawWrapped(rTemp, g_dglos.g_talkInfo.line[i], true, false, rgbColor,  g_dglo.m_fontSize).y;
            sy += y_last;
        }

fin:
      
        if (g_dglos.g_talkInfo.timer < g_dglos.g_dinkTick)
        {   
            g_dglos.g_talkInfo.curf++;
            g_dglos.g_talkInfo.timer = g_dglos.g_dinkTick+100;
        }

        if (g_dglos.g_talkInfo.curf == 0) g_dglos.g_talkInfo.curf = 1;
        if (g_dglos.g_talkInfo.curf > 7) g_dglos.g_talkInfo.curf = 1;


		if (check_seq_status(456) && check_seq_status(457))
		{
        ddrval = lpDDSBack->BltFast( curxl, curyl, g_pSpriteSurface[g_dglos.g_seq[456].frame[g_dglos.g_talkInfo.curf]],
            &g_dglos.g_picInfo[g_dglos.g_seq[456].frame[g_dglos.g_talkInfo.curf]].box  , DDBLTFAST_SRCCOLORKEY  );
  
        ddrval = lpDDSBack->BltFast( curxr, curyr, g_pSpriteSurface[g_dglos.g_seq[457].frame[g_dglos.g_talkInfo.curf]],
            &g_dglos.g_picInfo[g_dglos.g_seq[456].frame[g_dglos.g_talkInfo.curf]].box  , DDBLTFAST_SRCCOLORKEY  );
		}
     
		if (GetBaseApp()->GetGameTickPause()) return;

    if ( sjoy.button[1] || g_dinkMouseRightClick) //(sjoy.button[2]))
    {
		g_dinkMouseRightClick = false;
        g_dglos.g_talkInfo.active = false;
        *presult = g_dglos.g_talkInfo.line_return[g_dglos.g_talkInfo.cur];
        SoundPlayEffect(17, 22050,0,0,0);

        if (g_dglos.g_talkInfo.script != 0) 
        { 
            //we need to continue a script
            run_script(g_dglos.g_talkInfo.script);
        }
    }
}

CL_Vec2f NativeToDinkCoords(CL_Vec2f vPos)
{

	CL_Vec2f r = CL_Vec2f(
		g_dglo.m_orthoRenderRect.left + vPos.x * (g_dglo.m_orthoRenderRect.GetWidth() / GetScreenSizeXf()),
		vPos.y * (g_dglo.m_orthoRenderRect.GetHeight() / GetScreenSizeYf())
	);

	r.x /= g_dglo.m_aspectRatioModX;
	r.y /= g_dglo.m_aspectRatioModY;
	r -= g_dglo.m_centeringOffset;
	return r;
}


CL_Vec2f DinkToNativeCoords(CL_Vec2f vPos)
{
	CL_Vec2f r = vPos;
	float xmod = (float(g_dglo.m_orthoRenderRect.GetWidth()) / GetScreenSizeXf());
	float ymod = (float(g_dglo.m_orthoRenderRect.GetHeight()) / GetScreenSizeYf());
	r += g_dglo.m_centeringOffset;
	r.x *= g_dglo.m_aspectRatioModX;
	r.y *= g_dglo.m_aspectRatioModY;

	r.x /= xmod;
	r.y /= ymod;
	return r;
}

bool DinkIsMouseActive()
{
	if (g_sprite[1].active) if (g_sprite[1].brain == 13)
	{
		return true;
	}
	return false;
}
void DinkSetCursorPosition(CL_Vec2f vPos)
{

	static float vLastMouseY = vPos.y;
	float difY = vPos.y - vLastMouseY;
	vLastMouseY = vPos.y;

	if (g_sprite[1].active) if (g_sprite[1].brain == 13)
	{
#ifdef _DEBUG
		//LogMsg("Setting pos %s", toString(vPos).c_str());
#endif
		g_sprite[1].x = vPos.x;
		g_sprite[1].y = vPos.y;
	}

	if (g_dglos.g_talkInfo.active != 0 && fabs(difY) < 100) //dialog select? the 100 is an ugly hack to get rid of accumulated pixels due to .. something
	{

#ifdef _DEBUG
		//LogMsg("Mouse diff: %.2f", difY);
#endif
		if (!GetApp()->GetUsingTouchScreen())
		{
			g_dglos.g_playerInfo.mouse += difY;
		}
	}

}

void UpdateCursorPosition(int dx, int dy)
{
    /*
    *  Pick up any leftover fuzz from last time.  This is important
    *  when scaling down mouse motions.  Otherwise, the user can
    *  drag to the right extremely slow for the length of the table
    *  and not get anywhere.
    */
    if (g_sprite[1].active) if (g_sprite[1].brain == 13)
    {
        g_sprite[1].x += dx;
        g_sprite[1].y += dy;
        /* Clip the cursor to our client area */


        if (g_sprite[1].x > C_DINK_SCREENSIZE_X) g_sprite[1].x = C_DINK_SCREENSIZE_X;
        if (g_sprite[1].y > C_DINK_SCREENSIZE_Y) g_sprite[1].y = C_DINK_SCREENSIZE_Y;
        if (g_sprite[1].x < 0) g_sprite[1].x = 0;
        if (g_sprite[1].y < 0) g_sprite[1].y = 0;
    }
    
	/*
	if (g_gameMode == 1) 
    {
        g_dglos.g_playerInfo.mouse += dy;
        //Msg("play mousey is now %d", play.mouse);
    }
	*/
}

void  Scrawl_OnMouseInput(void)
{
	/*
	if (g_lastMouseClick)
	{
		mouse1 = true;
		g_lastMouseClick = false;
	}

	if (g_lastMouseX != 0 || g_lastMouseY != 0)
	{
		g_lastMouseX = 0;
		g_lastMouseY = 0;
	}

	*/

	UpdateCursorPosition(0, 0);

}

void button_brain(int h )
{
    rtRect32 box;
    if (g_sprite[h].move_active) 
    {
        process_move(h);
        return;
    }

    if (g_sprite[h].script == 0) return;

    box = g_dglos.g_picInfo[getpic(h)].hardbox;
    OffsetRect(&box, g_sprite[h].x, g_sprite[h].y);

    if (g_sprite[h].brain_parm == 0)
    {
        if (inside_box(g_sprite[1].x, g_sprite[1].y, box))
        {   
            g_sprite[h].brain_parm = 1;

            if (locate(g_sprite[h].script, "BUTTONON")) 
            {
                run_script(g_sprite[h].script);
                return;
            }
        }
    }
    else
    {
        if (!inside_box(g_sprite[1].x, g_sprite[1].y, box))
        {   
            g_sprite[h].brain_parm = 0;

            if (locate(g_sprite[h].script, "BUTTONOFF")) 
            {
                run_script(g_sprite[h].script);
                return;
            }
        }
    }
}

CL_Rect GetItemRectFromIndex(int num, bool magic)
{
	 int mx = 20;
	int my = 0;

	int vert = 0;

	if (magic == false)
	{
		mx = 260;
		my = 83;

		vert = ((num-1) / 4);
		mx += (((num-1) - (vert * 4)) * (18 + 65));
		my += (vert * (20 + 55));
	} else
	{
		mx = 45;
		my = 83;

		vert = ((num-1) / 2);
		mx += (((num-1) - (vert * 2)) * (18 + 65));
		my += (vert * (20 + 55));
	}

	return CL_Rect(mx, my, mx+65, my+55);
}



void SetCurInventoryPositionIndex(int itemIndex, bool bIsMagic)
{

	g_dglos.g_playerInfo.item_magic = bIsMagic;
	g_dglos.g_playerInfo.curitem = itemIndex;
}

bool DinkSetInventoryPosition(CL_Vec2f vPos)
{

	if (g_itemScreenActive)
	{
		
		for (int i = 1; i < C_DINK_MAX_MAGICS+1; i++)
		{
			if (GetItemRectFromIndex(i, true).contains(vPos))
			{
				SetCurInventoryPositionIndex(i, true);
				return true;
			}
		}

		for (int i = 1; i < C_DINK_MAX_ITEMS+1; i++)
		{
			if (GetItemRectFromIndex(i, false).contains(vPos))
			{
				SetCurInventoryPositionIndex(i, false);
				return true;
			}
		}

	}

	return false; //invalid
}


void draw_item(int num, bool magic, int mseq, int mframe)
{
	CL_Rectf r = GetItemRectFromIndex(num, magic);

    check_seq_status(mseq);

    if (g_pSpriteSurface[g_dglos.g_seq[mseq].frame[mframe]] == NULL)
    {

        if (!magic)
        {
            LogMsg("Whups, item %d seq %d frame %d not loaded, killed it",
                num, mseq, mframe);
            g_dglos.g_playerInfo.g_itemData[num].active = false;
        } else
        {
            LogMsg("Whups, magic %d seq %d frame %d not loaded, killed it",
                num, mseq, mframe);
            g_dglos.g_playerInfo.g_MagicData[num].active = false;

        }

        return;
    }

	
    int ddrval = lpDDSBack->BltFast( r.left, r.top, g_pSpriteSurface[g_dglos.g_seq[mseq].frame[mframe]],
        &g_dglos.g_picInfo[g_dglos.g_seq[mseq].frame[mframe]].box, DDBLTFAST_SRCCOLORKEY);

}

void process_item( void )
{

    rtRect32                rcRect;
    rcRect.left = 0;
    rcRect.top = 0;
    rcRect.right = C_DINK_SCREENSIZE_X;
    rcRect.bottom = C_DINK_SCREENSIZE_Y;
    int hor, virt;  
	int ddrval;
    ddrval = lpDDSBack->BltFast( 0, 0, lpDDSBackGround, &rcRect, DDBLTFAST_NOCOLORKEY);
    
	if (!check_seq_status(423)) return;

    //lets blit the main screen over it

    ddrval = lpDDSBack->BltFast( 20, 0, g_pSpriteSurface[g_dglos.g_seq[423].frame[1]], &g_dglos.g_picInfo[g_dglos.g_seq[423].frame[1]].box, DDBLTFAST_SRCCOLORKEY);

      //draw all currently owned items; magic
	int i;   
	
	for (i = 1; i < C_DINK_MAX_MAGICS+1; i++)
    {
        if (g_dglos.g_playerInfo.g_MagicData[i].active) draw_item(i, true, g_dglos.g_playerInfo.g_MagicData[i].seq,g_dglos.g_playerInfo.g_MagicData[i].frame);
    }

    //draw all currently owned items; normal
    for ( i = 1; i < C_DINK_MAX_ITEMS+1; i++)
    {
        if (g_dglos.g_playerInfo.g_itemData[i].active) draw_item(i, false, g_dglos.g_playerInfo.g_itemData[i].seq,g_dglos.g_playerInfo.g_itemData[i].frame);

    }

    //draw selection box around armed weapon
    if (*pcur_weapon != 0) if (g_dglos.g_playerInfo.g_itemData[*pcur_weapon].active)
        draw_item(*pcur_weapon, false, 423, 4);


    //draw selection box around armed magic
    if (*pcur_magic != 0) if (g_dglos.g_playerInfo.g_itemData[*pcur_magic].active)
        draw_item(*pcur_magic, true, 423, 5);


    //draw the selector around it, alternating from 2 to 3
    if (g_dglos.g_playerInfo.curitem < 1) g_dglos.g_playerInfo.curitem = 1;


    if (g_dglos.g_dinkTick > g_dglos.item_timer)
    {
        if (g_dglos.item_pic == 2) g_dglos.item_pic = 3; else g_dglos.item_pic = 2;
        g_dglos.item_timer = g_dglos.g_dinkTick + 400;

    }
    
	draw_item(g_dglos.g_playerInfo.curitem, g_dglos.g_playerInfo.item_magic, 423, g_dglos.item_pic);

    if (!g_dglos.g_playerInfo.item_magic)
    {
        
		
		hor = (g_dglos.g_playerInfo.curitem - (((g_dglos.g_playerInfo.curitem-1) / 4) * 4));
        virt = ((g_dglos.g_playerInfo.curitem-1) / 4);

        //choosing weapon/item

        if (sjoy.button[1]|| sjoy.button[4])
        {
            if (g_dglos.g_playerInfo.g_itemData[g_dglos.g_playerInfo.curitem].active)
            {
                //arm weapon
                SoundPlayEffect(18, 42050,0,0,0);        
                if (*pcur_weapon != 0)
                {
                    //disarm old weapon
                    if (locate(g_dglos.weapon_script, "DISARM")) run_script(g_dglos.weapon_script);
                }
                //load weapons script
                *pcur_weapon = g_dglos.g_playerInfo.curitem;
                g_dglos.weapon_script = load_script(g_dglos.g_playerInfo.g_itemData[*pcur_weapon].name, 1000, false);
                if (locate(g_dglos.weapon_script, "ARM")) run_script(g_dglos.weapon_script);
                if (locate(g_dglos.weapon_script, "ARMMOVIE")) run_script(g_dglos.weapon_script);

                draw_status_all();
            } else
            {
                //can't arm nothing, play sound
            }
        } else
            if (sjoy.rightd) 
            {
                if (hor < 4) g_dglos.g_playerInfo.curitem++;
                SoundPlayEffect(11, 22050,0,0,0);   
            } else
                if (sjoy.leftd) 
                {
                    if (hor > 1)
                    {
                        g_dglos.g_playerInfo.curitem--; 
                        SoundPlayEffect(11, 22050,0,0,0);   

                    }
                    else
                    {
                        SoundPlayEffect(11, 22050,0,0,0);   

                        g_dglos.g_playerInfo.item_magic = true;
                        g_dglos.g_playerInfo.curitem = (virt * 2) + 2;
                        //switch to magic mode
                    }
                } else


                    if (sjoy.downd)
                    {
                        if (virt < 3)
                        {
                            g_dglos.g_playerInfo.curitem += 4;
                            SoundPlayEffect(11, 22050,0,0,0);   

                        }
                    } else

                        if (sjoy.upd)
                        {
                            if (virt > 0)
                            {
                                g_dglos.g_playerInfo.curitem -= 4;
                                SoundPlayEffect(11, 22050,0,0,0);   

                            }
                        }
    } else

    {
        hor = (g_dglos.g_playerInfo.curitem - (((g_dglos.g_playerInfo.curitem-1) / 2) * 2));
        virt = ((g_dglos.g_playerInfo.curitem-1) / 2);

        if (sjoy.button[1]|| sjoy.button[4])
        {
            if (g_dglos.g_playerInfo.g_MagicData[g_dglos.g_playerInfo.curitem].active)
            {
                //arm magic
                SoundPlayEffect(18, 42050,0,0,0);  
                if (*pcur_magic != 0)
                {
                    //disarm old weapon
                    if (locate(g_dglos.magic_script, "DISARM")) run_script(g_dglos.magic_script);
                }
                //load magics script
                *pcur_magic = g_dglos.g_playerInfo.curitem;
                g_dglos.magic_script = load_script(g_dglos.g_playerInfo.g_MagicData[*pcur_magic].name, 1000, false);
                if (locate(g_dglos.magic_script, "ARM")) run_script(g_dglos.magic_script);
                if (locate(g_dglos.magic_script, "ARMMOVIE")) run_script(g_dglos.magic_script);
                draw_status_all();
            } else
            {
                //can't arm nothing, play sound
            }
        }

        if (sjoy.rightd) 
        {
            if (hor < 2)
            {
                g_dglos.g_playerInfo.curitem++;
                SoundPlayEffect(11, 22050,0,0,0);   

            }
            else
            { 
                g_dglos.g_playerInfo.item_magic = false;
                g_dglos.g_playerInfo.curitem = (virt * 4) +1;
                SoundPlayEffect(11, 22050,0,0,0);   

            }
        } else
            if (sjoy.leftd) 
            {
                if (hor > 1)
                {
                    g_dglos.g_playerInfo.curitem--;
                    SoundPlayEffect(11, 22050,0,0,0);   

                }
            } else

                if (sjoy.downd)
                {
                    if (virt < 3)
                    {
                        g_dglos.g_playerInfo.curitem += 2;
                        SoundPlayEffect(11, 22050,0,0,0);   

                    }
                } else

                    if (sjoy.upd)
                    {
                        if (virt > 0) 
                        {
                            g_dglos.g_playerInfo.curitem -= 2;
                            SoundPlayEffect(11, 22050,0,0,0);   
                        }
                    }
    }
    
	if (g_dglos.g_talkInfo.active) 
	{
		assert(!"People use this?")	;
		process_talk();
	}

	//g_dglo.SetViewOverride(DinkGlobals::VIEW_ZOOMED);

    //a special process callbacks for just stuff that was created in this mode? 
    // process_callbacks_special();
    flip_it(); 

    if (sjoy.button[1] || sjoy.button[4])
    {
        SoundPlayEffect(17, 22050,0,0,0);
        g_itemScreenActive = false;
		g_dglo.SetViewOverride(DinkGlobals::VIEW_NONE);
    }
}

void process_animated_tiles( void )
{
    rtRect32 rcRect;
    int cool;
    int flip;
    int pa;

    //process water tiles

    if (water_timer < g_dglos.g_dinkTick)
    {

        water_timer = g_dglos.g_dinkTick + ((rand() % 2000));

        flip = ((rand() % 2)+1);        

		int tileScreenID;

        for (int x=0; x<96; x++)
        {
            //redink1 fix for first broken water tile
            if (g_dglos.g_smallMap.t[x].num >= 896) if (g_dglos.g_smallMap.t[x].num < (896+128))
            {

                cool = g_dglos.g_smallMap.t[x].num / 128;
                pa = g_dglos.g_smallMap.t[x].num - (cool * 128);
                rcRect.left = (pa * 50- (pa / 12) * 600);
                rcRect.top = (pa / 12) * 50;
                rcRect.right = rcRect.left + 50;
                rcRect.bottom = rcRect.top + 50;
			
				tileScreenID = cool+flip;
				bool bRequireRebuild;

				LoadTileScreenIfNeeded(tileScreenID, bRequireRebuild);
				g_tileScreens[tileScreenID]->UpdateLastUsedTime();

				lpDDSBackGround->BltFast( (x * 50 - ((x / 12) * 600))+g_gameAreaLeftOffset, (x / 12) * 50, g_tileScreens[tileScreenID],
	         &rcRect, DDBLTFAST_NOCOLORKEY| DDBLTFAST_WAIT );

            }   
        }

    }

    //end of water processing

        if (fire_forward) fire_flip++;
        if (!fire_forward) fire_flip--;

        if (fire_flip < 1)
        {
            fire_flip = 5;
            fire_forward = false;
        }

		int tileScreenID;

        for (int x=0; x<96; x++)
        {
            //redink1 fix for first broken fire tile
            if (g_dglos.g_smallMap.t[x].num >= 2304) if (g_dglos.g_smallMap.t[x].num < (2304+128))
            {

                cool = g_dglos.g_smallMap.t[x].num / 128;
                pa = g_dglos.g_smallMap.t[x].num - (cool * 128);
                rcRect.left = (pa * 50- (pa / 12) * 600);
                rcRect.top = (pa / 12) * 50;
                rcRect.right = rcRect.left + 50;
                rcRect.bottom = rcRect.top + 50;
		
				tileScreenID = cool+fire_flip;
				bool bRequireRebuild;

				LoadTileScreenIfNeeded(tileScreenID, bRequireRebuild);
				g_tileScreens[tileScreenID]->UpdateLastUsedTime();

				lpDDSBackGround->BltFast( (x * 50 - ((x / 12) * 600))+g_gameAreaLeftOffset, (x / 12) * 50, g_tileScreens[tileScreenID],
					&rcRect, DDBLTFAST_NOCOLORKEY| DDBLTFAST_WAIT );


            }   
        }
}

void ThinkShowBmp()
{
	if (  (sjoy.button[1])
		|| (sjoy.button[2])
		|| (sjoy.button[3])
		|| (sjoy.button[4])
		|| (sjoy.button[5])
		|| (sjoy.button[6])
		)
	{
		g_dglos.g_bShowingBitmap.active = false;
		if (g_dglos.g_bShowingBitmap.script != 0)
			run_script(g_dglos.g_bShowingBitmap.script);
		g_dglos.g_bShowingBitmap.stime = g_dglos.g_dinkTick+2000;
		g_dglos.but_timer = g_dglos.g_dinkTick + 200;

		int sprite = say_text_xy("", 1, 440, 0);                                
		g_sprite[sprite].noclip = 1;
	}
}
void process_show_bmp( void )
{
    rtRect32 rcRect(0,0,C_DINK_SCREENSIZE_X, C_DINK_SCREENSIZE_Y);

	lpDDSBack->BltFast( 0, 0, lpDDSBuffer, &rcRect, DDBLTFAST_NOCOLORKEY);

    if (g_dglos.g_bShowingBitmap.showdot)
    {
        //let's display a nice dot to mark where they are on the map
        int x = g_dglos.g_playerInfo.last_map - 1;
        int mseq = 165;

		if (check_seq_status(mseq))
		{
			g_dglos.g_bShowingBitmap.picframe++;
			if (g_dglos.g_bShowingBitmap.picframe > g_dglos.g_seq[mseq].last) g_dglos.g_bShowingBitmap.picframe = 1;
			int mframe = g_dglos.g_bShowingBitmap.picframe;
			lpDDSBack->BltFast( (x % 32) * 20, (x / 32) * 20, g_pSpriteSurface[g_dglos.g_seq[mseq].frame[mframe]],
				&g_dglos.g_picInfo[g_dglos.g_seq[mseq].frame[mframe]].box, DDBLTFAST_SRCCOLORKEY| DDBLTFAST_WAIT );
		}

		
    }

}

void drawscreenlock( void )
{
	DrawRect(g_dglo.m_gameArea, MAKE_RGBA( (255-80)+ 80*(SinToZeroToOneRange(SinGamePulseByMS(1000))),0,0,255));
	//TODO draw screenlock
  if (check_seq_status(423, 9))
	{
  
	lpDDSBack->BltFast(0, 0, g_pSpriteSurface[g_dglos.g_seq[423].frame[9]],
        &g_dglos.g_picInfo[g_dglos.g_seq[423].frame[9]].box  , DDBLTFAST_NOCOLORKEY  );
  }

  if (check_seq_status(423, 10))
  {

	  //draw the screenlock icon
    lpDDSBack->BltFast(620, 0, g_pSpriteSurface[g_dglos.g_seq[423].frame[10]],
        &g_dglos.g_picInfo[g_dglos.g_seq[423].frame[10]].box  , DDBLTFAST_NOCOLORKEY  );
  }
	
}    


void ThinkSprite(int h, bool get_frame)
{

	if (g_sprite[h].active)
	{
		int move_result = 0;

		if (GetBaseApp()->GetGameTickPause())
			{
				goto past;
			}

		g_sprite[h].moveman = 0; //init thing that keeps track of moving path    
		g_sprite[h].lpx[0] = g_sprite[h].x;
		g_sprite[h].lpy[0] = g_sprite[h].y; //last known legal cords

		g_sprite[h].skiptimer++;
		//inc delay, used by "skip" by all sprites
//		box_crap = g_dglos.g_picInfo[getpic(h)].box;

		if (g_sprite[h].kill > 0)
		{
#ifdef _DEBUG
			if (h == 10)
			{
			//	LogMsg("Yo!");
			}
#endif
			if (g_sprite[h].kill_timer == 0) g_sprite[h].kill_timer = g_dglos.g_dinkTick;
			if (g_sprite[h].kill_timer + g_sprite[h].kill < g_dglos.g_dinkTick)
			{

				g_sprite[h].active = false;
				if (g_script_debug_mode)
					LogMsg("Killing sprite %d.", h);

				get_last_sprite();
				if (g_sprite[h].callback > 0) 
				{
					//  Msg("Callback running script %d.", spr[h].script);
					run_script(g_sprite[h].callback);
				}
			}
		}

		if (g_sprite[h].timer > 0)
		{
			if (g_dglos.g_dinkTick > g_sprite[h].wait)
			{
				g_sprite[h].wait = g_dglos.g_dinkTick + g_sprite[h].timer;
			}else
			{
				goto animate;
			}
		}

		//brains - predefined bahavior patterns available to any sprite

		if (g_sprite[h].notouch) if (g_dglos.g_dinkTick > g_sprite[h].notouch_timer) g_sprite[h].notouch = false;
		if (get_frame == false)
		{
			if (   (g_sprite[h].brain == 1)/* || (spr[h].brain == 9) || (spr[h].brain == 3) */ )
			{

				run_through_touch_damage_list(h);
			}       

			if (g_sprite[h].brain == 1)
			{
				if (g_dglos.process_warp == 0)
					human_brain(h); 
			}

			if (g_sprite[h].brain == 2) bounce_brain(h);
			if (g_sprite[h].brain == 0) no_brain(h);
			if (g_sprite[h].brain == 3) duck_brain(h);
			if (g_sprite[h].brain == 4) pig_brain(h);
			if (g_sprite[h].brain == 5) one_time_brain(h);
			if (g_sprite[h].brain == 6) repeat_brain(h);
			if (g_sprite[h].brain == 7) one_time_brain_for_real(h);
			if (g_sprite[h].brain == 8) text_brain(h);
			if (g_sprite[h].brain == 9) pill_brain(h);
			if (g_sprite[h].brain == 10) dragon_brain(h);
			if (g_sprite[h].brain == 11) missile_brain(h, true);
			if (g_sprite[h].brain == 12) scale_brain(h);
			if (g_sprite[h].brain == 13) mouse_brain(h);
			if (g_sprite[h].brain == 14) button_brain(h);
			if (g_sprite[h].brain == 15) shadow_brain(h);
			if (g_sprite[h].brain == 16) people_brain(h);
			if (g_sprite[h].brain == 17) missile_brain_expire(h);
		} else
		{
			goto past;
		}
		if (::g_b_kill_app) return;

		
animate:

		
		if (g_sprite[h].brain != 13)
		{
			move_result = check_if_move_is_legal(h);
		}

		if (g_dglos.flub_mode != -500)
		{
#ifdef _DEBUG
			LogMsg("move result is %d", g_dglos.flub_mode);
#endif
			move_result = g_dglos.flub_mode;
			g_dglos.flub_mode = -500;
		}

		if (g_sprite[h].brain == 1) if (move_result > 100)
		{
			if (g_dglos.g_smallMap.sprite[move_result-100].prop == 1)
				special_block(move_result - 100, h);
		}

		if (g_sprite[h].reverse)
		{
			//reverse instructions
			if (g_sprite[h].seq > 0)
			{
				if (g_sprite[h].frame < 1)
				{
					// new anim
					g_sprite[h].pseq = g_sprite[h].seq;
					g_sprite[h].pframe = g_dglos.g_seq[g_sprite[h].seq].last;
					g_sprite[h].frame = g_dglos.g_seq[g_sprite[h].seq].last;
					if (g_sprite[h].frame_delay != 0) g_sprite[h].delay = (g_dglos.g_dinkTick+ g_sprite[h].frame_delay); else
						g_sprite[h].delay = (g_dglos.g_dinkTick + g_dglos.g_seq[g_sprite[h].seq].delay[g_dglos.g_seq[g_sprite[h].seq].last]);
				}   else
				{
					// not new anim

					//is it time?

					if (g_dglos.g_dinkTick > g_sprite[h].delay)
					{
						g_sprite[h].frame--;

						if (g_sprite[h].frame_delay != 0) g_sprite[h].delay = (g_dglos.g_dinkTick + g_sprite[h].frame_delay); else

							g_sprite[h].delay = (g_dglos.g_dinkTick + g_dglos.g_seq[g_sprite[h].seq].delay[g_sprite[h].frame]);

						g_sprite[h].pseq = g_sprite[h].seq;
						g_sprite[h].pframe = g_sprite[h].frame;


						if (g_dglos.g_seq[g_sprite[h].seq].frame[g_sprite[h].frame]  < 2)
						{

							g_sprite[h].pseq = g_sprite[h].seq;
							g_sprite[h].pframe = g_sprite[h].frame+1;

							g_sprite[h].frame = 0;
							g_sprite[h].seq_orig = g_sprite[h].seq;
							g_sprite[h].seq = 0;
							g_sprite[h].nocontrol = false;


							if (h == 1) if (in_this_base(g_sprite[h].seq_orig,g_dglos.mDinkBasePush))
							{
								g_dglos.g_playerInfo.push_active = false;
								if (g_dglos.g_playerInfo.push_dir == 2) if (sjoy.down) g_dglos.g_playerInfo.push_active = true;
								if (g_dglos.g_playerInfo.push_dir == 4) if (sjoy.left) g_dglos.g_playerInfo.push_active = true;
								if (g_dglos.g_playerInfo.push_dir == 6) if (sjoy.right) g_dglos.g_playerInfo.push_active = true;
								if (g_dglos.g_playerInfo.push_dir == 8) if (sjoy.up) g_dglos.g_playerInfo.push_active = true;
								goto past;
							}
						}
						if (g_sprite[h].seq > 0) if (g_dglos.g_seq[g_sprite[h].seq].special[g_sprite[h].frame] == 1)
						{
							//this sprite can damage others right now!
							//lets run through the list and tag sprites who were hit with their damage

							run_through_tag_list(h, g_sprite[h].strength);
						}
					}
				}
			}


		} else
		{

			if (g_sprite[h].seq > 0) if (g_sprite[h].picfreeze == 0)
			{
				if (g_sprite[h].frame < 1)
				{
					// new anim
					g_sprite[h].pseq = g_sprite[h].seq;
					g_sprite[h].pframe = 1;
					g_sprite[h].frame = 1;
					if (g_sprite[h].frame_delay != 0) g_sprite[h].delay = g_dglos.g_dinkTick + g_sprite[h].frame_delay; else

						g_sprite[h].delay = (g_dglos.g_dinkTick + g_dglos.g_seq[g_sprite[h].seq].delay[1]);
				}   else
				{
					// not new anim

					//is it time?

					if (g_dglos.g_dinkTick > g_sprite[h].delay)
					{
						g_sprite[h].frame++;
						if (g_sprite[h].frame_delay != 0) g_sprite[h].delay = g_dglos.g_dinkTick + g_sprite[h].frame_delay; else

							g_sprite[h].delay = (g_dglos.g_dinkTick + g_dglos.g_seq[g_sprite[h].seq].delay[g_sprite[h].frame]);

						g_sprite[h].pseq = g_sprite[h].seq;
						g_sprite[h].pframe = g_sprite[h].frame;

						if (g_dglos.g_seq[g_sprite[h].seq].frame[g_sprite[h].frame] == -1)
						{
							g_sprite[h].frame = 1;
							g_sprite[h].pseq = g_sprite[h].seq;
							g_sprite[h].pframe = g_sprite[h].frame;
							if (g_sprite[h].frame_delay != 0) g_sprite[h].delay = g_dglos.g_dinkTick + g_sprite[h].frame_delay; else

								g_sprite[h].delay = (g_dglos.g_dinkTick + g_dglos.g_seq[g_sprite[h].seq].delay[g_sprite[h].frame]);
						}

					//if (g_sprite[h].frame == g_dglos.g_seqData[g_sprite[h].seq].last+1)

						if (g_dglos.g_seq[g_sprite[h].seq].frame[g_sprite[h].frame] < 1)
						{

							g_sprite[h].pseq = g_sprite[h].seq;
							g_sprite[h].pframe = g_sprite[h].frame-1;

							g_sprite[h].frame = 0;
							g_sprite[h].seq_orig = g_sprite[h].seq;
							g_sprite[h].seq = 0;
							g_sprite[h].nocontrol = false;


							if (h == 1) if (in_this_base(g_sprite[h].seq_orig,g_dglos.mDinkBasePush))
							{

								g_dglos.g_playerInfo.push_active = false;
								if (g_dglos.g_playerInfo.push_dir == 2) if (sjoy.down) g_dglos.g_playerInfo.push_active = true;
								if (g_dglos.g_playerInfo.push_dir == 4) if (sjoy.left) g_dglos.g_playerInfo.push_active = true;
								if (g_dglos.g_playerInfo.push_dir == 6) if (sjoy.right) g_dglos.g_playerInfo.push_active = true;
								if (g_dglos.g_playerInfo.push_dir == 8) if (sjoy.up) g_dglos.g_playerInfo.push_active = true;
								goto past;
							}
						}
						if (g_sprite[h].seq > 0) if (g_dglos.g_seq[g_sprite[h].seq].special[g_sprite[h].frame] == 1)
						{
							//this sprite can damage others right now!
							//lets run through the list and tag sprites who were hit with their damage
							run_through_tag_list(h, g_sprite[h].strength);
						}
					}
				}
			}
		}


		if (g_sprite[h].active && g_sprite[h].brain == 1)
		{
			did_player_cross_screen(true, h);
		}       

past: 

		check_sprite_status(h);
		draw_sprite_game(lpDDSBack,h);


	}
}

void SetupFirstScript()
{
	//memset(&g_sprite[1], 0, sizeof(g_sprite[1]));

	
	g_sprite[1].speed = 3;

	g_sprite[1].timer = 0;
	g_sprite[1].brain = 1;
	g_sprite[1].hard = 1;
	g_sprite[1].pseq = 2;
	g_sprite[1].pframe = 1;
	g_sprite[1].seq = 2;
	g_sprite[1].dir = 2;
	g_sprite[1].damage = 0;
	g_sprite[1].strength = 10;
	g_sprite[1].defense = 0;
	g_sprite[1].skip = 0;
	g_sprite[1].alt.Clear();
	g_sprite[1].base_idle = 10;
	g_sprite[1].base_walk = -1;
	g_sprite[1].size = 100;       
	g_sprite[1].base_hit = 100;
	g_sprite[1].active = true;

	
	if (!g_dglo.m_dmodGameDir.empty())
	{

	int crap2 = add_sprite(0,450,8,0,0);

	g_sprite[crap2].hard = 1;
	g_sprite[crap2].noclip = 1;
	strcpy(g_sprite[crap2].text, g_dglos.dversion_string);

	g_sprite[crap2].damage = -1;
	g_sprite[crap2].owner = 1000;
	}

	
	int scr = load_script("START",1000, true);
	if (locate(scr, "MAIN") == false)
	{
		LogMsg("Error: Can't locate MAIN in script START!");
	}
	run_script(scr);
	
	if (!g_dglo.m_dmodGameDir.empty())
	{
		g_dglos.g_gameMode = 1;
	}

}

void DrawDinkText(int max_s, int32 *rank)
{
	int h;

	for (int j = 1; j < max_s+1; j++)
	{
		if (g_dglos.plane_process)
			h = rank[j]; else h = j;
		if (h > 0) 
			if (g_sprite[h].active)
			{
				if (g_sprite[h].brain == 8) 
				{
					//LogMsg("Drawing text %d..", h);
					text_draw(h);
				}
			}
	}

}

void updateFrame()
{

	if (!lpDDSBack || g_dglo.m_curLoadState != FINISHED_LOADING) return;
	bool bRenderDinkText = true;
	g_dinkFadeAlpha = 0;
	if (g_dglos.bFadedDown) g_dinkFadeAlpha = 1;

#ifdef _DEBUG

	int cur = 0;
	for (int i=0; i < C_MAX_SPRITES_AT_ONCE; i++)
	{

		if (g_sprite[i].active) cur = i;
	}
	assert(cur <= g_dglos.last_sprite_created);
	
#endif


	byte state[256];
	rtRect32 rcRect;
	bool bCaptureScreen = false;
	int h, j;

	bool bs[C_MAX_SPRITES_AT_ONCE];
	int highest_sprite;

	g_abort_this_flip = false;
	
	ProcessGraphicGarbageCollection();
	
	SetOrthoRenderSize(g_dglo.m_orthoRenderRect.right, g_dglo.m_orthoRenderRect.GetHeight(), -g_dglo.m_orthoRenderRect.left, -g_dglo.m_orthoRenderRect.top);
	
	if (5 > 9) //I'm sorry about this
	{
		trigger_start:
		g_bInitiateScreenMove = false;
		bCaptureScreen = true;    
	}

	check_joystick();
	
	if (g_dglos.g_gameMode == 1) 
	{
		Scrawl_OnMouseInput(); 
	}  else
	{
		if (g_dglos.keep_mouse)
		{
			if ((g_dglos.g_talkInfo.active) || (g_sprite[1].brain == 13))
				Scrawl_OnMouseInput();
		}
	}


#ifdef _WIN32
	static int LastWindowsTimer = 0;

//if (!bSpeedUp)
if (0)
{

//Sleep(50);

	while (GetTickCount() <= LastWindowsTimer)
	{
		Sleep(0);
	}

}
LastWindowsTimer = GetTickCount();
#else

	/*
	static uint32 lastTick = 0;


	if (!GetBaseApp()->GetGameTickPause())
	{
		while (GetSystemTimeTick()-lastTick < 33)
		{

		}
	}

	lastTick = GetSystemTimeTick();
	 */
#endif


	//non-windows timer


	g_dglos.lastTickCount = g_dglos.g_dinkTick;
	//g_dglos.g_dinkTick = GetBaseApp()->GetGameTick();
	g_dglos.g_dinkTick += (1000.0f / 60.0f); //FPS lock at 60 fps
	
	/*
	int fps_final = g_dglos.g_dinkTick - g_dglos.lastTickCount;

	//redink1 changed to 12-12 from 10-15... maybe work better on faster computers?
	if (fps_final < 12) fps_final = 12;
	if (fps_final > 68) fps_final = 68;  

	fps_final = 24; //force it

	g_dglos.base_timing = fps_final / 3;
	if (g_dglos.base_timing < 4) g_dglos.base_timing = 4;
	
	int junk3;

	//redink1 added these changes to set Dink's speed correctly, even on fast machines.
	

	if (g_dglos.dinkspeed <= 0)
		junk3 = 0;
	else if (g_dglos.dinkspeed == 1)
		junk3 = 12;
	else if (g_dglos.dinkspeed == 2)
		junk3 = 6;
	else if (g_dglos.dinkspeed == 3)
		junk3 = 3;
	else
		junk3 = 1;

	junk3 *= (g_dglos.base_timing / 4);

	g_sprite[1].speed = junk3;
	*/

	//assume we're locked at 60 fps
	
	g_dglos.base_timing = 7;
	float junk3 = 1;
	/*
	if (g_dglos.dinkspeed <= 0)
		junk3 = 0;
	else if (g_dglos.dinkspeed == 1)
		junk3 = 12;
	else if (g_dglos.dinkspeed == 2)
		junk3 = 6;
	else if (g_dglos.dinkspeed == 3)
		junk3 = 3;
		*/
	if (g_dglos.dinkspeed <= 0)
		junk3 = 0;
	else if (g_dglos.dinkspeed == 1)
		junk3 = 12;
	else if (g_dglos.dinkspeed == 2)
		junk3 = 6;
	else if (g_dglos.dinkspeed == 3)
		junk3 = 3;

	//g_sprite[1].speed = (g_dglos.base_timing / 4);
	//g_sprite[1].speed = 5;
	bool bSpeedUp = false;
	g_sprite[1].speed = (int)junk3*1.0f;
	if (DinkGetSpeedUpMode())
	{
		bSpeedUp = true;
	}

	if (bSpeedUp)
	{

		/*
		if (!GetApp()->GetGameTickPause())
		{
			GetApp()->SetGameTick(GetApp()->GetGameTick() + GetApp()->GetDeltaTick() * 5);
		}
		*/
	}


	if (g_dglos.g_bShowingBitmap.active)
	{
		//grab main loop and divert it to show a bmp instead
		
		process_show_bmp();
		ThinkShowBmp();
		bRenderDinkText = false;
		goto flip;
	}

	g_dglos.mbase_count++;


	
	if (g_dglos.g_dinkTick > g_dglos.g_DinkUpdateTimerMS+100)
	{
	//	g_dglos.mbase_timing = (g_dglos.mbase_count / 100);
		g_dglos.mbase_timing = 135; //hardcoded now to avoid fluctuations
		g_dglos.g_DinkUpdateTimerMS = g_dglos.g_dinkTick;
		if (g_dglos.g_bowStatus.active) g_dglos.g_bowStatus.hitme = true;
		
		
		
		if (*pupdate_status == 1) update_status_all();

		update_sound();
	
		//TODO Animated tiles
		//if (IsDesktop())
		{
			//TODO:  Maybe mobile can handle this now?
				process_animated_tiles();
		}
	}


	if (g_forceBuildBackgroundFromScratch)
	{
		BuildScreenBackground(false, true);
	}
	state[1] = 0;  

	//figure out frame rate

	if (g_itemScreenActive)
	{
		process_item();
		bRenderDinkText = false;
		
		if (!g_dglo.m_curView == DinkGlobals::VIEW_ZOOMED)
		{
			BlitGUIOverlay();
		}

		goto flip;
	}

	ProcessTransition();

	if (g_dglos.process_upcycle) 
	{
		up_cycle();
	}
	if (g_dglos.process_warp > 0) process_warp_man();
	if (g_dglos.process_downcycle)
	{
		down_cycle();
	}

	int max_s;

	if (g_dglos.plane_process)
	{
		memset(&bs,0,sizeof(bs));
		max_s = g_dglos.last_sprite_created;

		int height;

		for (int r1 = 1; r1 < max_s+1; r1++)
		{
			highest_sprite = 22024; //more than it could ever be

			g_spriteRank[r1] = 0;

			for (int h1 = 1; h1 < max_s+1; h1++)
			{
				if (g_sprite[h1].active) if (g_sprite[h1].disabled == false)
				{ 
					if (bs[h1] == false)
					{
						//Msg( "Ok,  %d is %d", h1,(spr[h1].y + k[spr[h1].pic].yoffset) );
						if (g_sprite[h1].que != 0) height = g_sprite[h1].que; else height = g_sprite[h1].y;
						if ( height < highest_sprite )
						{
							highest_sprite = height;
							g_spriteRank[r1] = h1;
						}
					}
				}
			}

			if (g_spriteRank[r1] != 0)  
				bs[g_spriteRank[r1]] = true;
		}

	} else
	{
		//not processing planes
		max_s = C_MAX_SPRITES_AT_ONCE;
	}

	rcRect.left = 0;
	rcRect.top = 0;
	rcRect.right = C_DINK_SCREENSIZE_X;
	rcRect.bottom = C_DINK_SCREENSIZE_Y;

	//Blit from Two, which holds the base scene.

	//this doesn't really make much sense to me but it works so not screwing with it
	if (g_dglo.m_curView == DinkGlobals::VIEW_ZOOMED)
	{
		lpDDSBack->BltFast(g_dglo.m_gameArea.left, g_dglo.m_gameArea.top, lpDDSBackGround, &g_dglo.m_orthoRenderRect, DDBLTFAST_NOCOLORKEY);
	}
	else
	{
		lpDDSBack->BltFast(0,0, lpDDSBackGround, &g_dglo.m_orthoRenderRect, DDBLTFAST_NOCOLORKEY);
	}

	g_dglo.m_bgSpriteMan.Render(lpDDSBack); //blit sprites that have been shoved into the bg, too slow to actually add them, so we fake it until the screen is rebuilt

	if (!g_bTransitionActive)
	{
		BlitGUIOverlay();
	}

	for ( j = 1; j < max_s+1; j++)
	{
		if (g_dglos.plane_process)
			h = g_spriteRank[j]; else h = j;
		//Msg( "Ok, rank %d is %d", j,h);

		if (h > 0)
		{
			ThinkSprite(h, g_bTransitionActive || g_dglos.g_stopEntireGame == 1);
		}
	}                               
	
	EndProcessTransition(); 

	
	if (g_dglos.g_stopEntireGame == 1)
	{
		if (g_dglos.g_talkInfo.active)
		{
			process_talk(); 
		}
		else
		{
			g_dglos.g_stopEntireGame = 0;
			//BuildScreenBackground(false);
			draw_status_all();

		}
		bRenderDinkText = false;
		goto flip;

	}

	if (g_bTransitionActive)
	{
		//don't actually think or process game stuff
		bRenderDinkText = false;
		goto flip;
	}

	if ( (sjoy.joybit[7] == true) )
	{
		//space is pressed, lets draw the hitmap, why not?
		if (!no_cheat) DrawCollision();
	}

	if (g_dglos.g_gameMode == 2)
	{
		g_dglos.g_gameMode = 3;
		load_map(g_MapInfo.loc[*pmap]);
		BuildScreenBackground();
		g_dglos.g_guiLife = *plife;

		if (g_dglos.keep_mouse == 0)
		{
			//kill the mouse here?
			g_dglo.UnSetViewOverride();
		}
	}

	if (g_sprite[1].active) if (g_sprite[1].brain == 1) did_player_cross_screen(false, 1);

	if (g_bInitiateScreenMove)
	{
		goto trigger_start;
	}

	if (g_dglos.screenlock == 1)
	{
		drawscreenlock();
	}
	
	if (!GetBaseApp()->GetGameTickPause())
	{
		if (g_dglos.g_talkInfo.active) process_talk();
		process_callbacks();
	}

flip:

	if (g_dinkFadeAlpha *255 > 1)
	{
		//g_dglo.m_gameArea.bottom++; //hack to fix a little glitchy line
		
		if (IsDrawingDinkStatusBar())
		{
			DrawFilledRect(g_dglo.m_gameArea, MAKE_RGBA(0,0,0,g_dinkFadeAlpha* 255));
		} else
		{
			DrawFilledRect(g_dglo.m_orthoRenderRect, MAKE_RGBA(0,0,0,g_dinkFadeAlpha* 255));

		}

		//g_dglo.m_gameArea.bottom--;
	}

	if (::g_b_kill_app) return; 
	if (!g_abort_this_flip)
	{
		flip_it(); 
	}

	if (bRenderDinkText)
		DrawDinkText(max_s, g_spriteRank);

	RemoveOrthoRenderSize();
	if (turn_on_plane) g_dglos.plane_process = true;

	BlitSecondTransitionScreen();
		
	if (g_bTransitionActive)	
	{
		SetOrthoRenderSize(C_DINK_SCREENSIZE_X, g_dglo.m_orthoRenderRect.GetHeight(), 0, -g_dglo.m_orthoRenderRect.top);
		BlitGUIOverlay();
		RemoveOrthoRenderSize();
	}
	
	if (bCaptureScreen)
	{
		//reset the timer because we probably just wasted a bunch loading crap
		g_dglo.m_transitionTimer = GetBaseApp()->GetGameTick();
	}

	if (!GetBaseApp()->GetGameTickPause())
	{
		 UpdateControlsGUIIfNeeded();
	}

} 


void load_batch(int linesToProcess, float &percentOut)
{

	static int iLineCount = 0;

	if (!g_dglo.m_iniScanner.IsLoaded())
	{
		//first pass, open the file
		iLineCount = 0;
		if (!g_dglo.m_iniScanner.LoadFile(GetFileLocationString("dink.ini"), false))
		{
			LogMsg("dink.ini not found.");   
			assert(0);
			return;
		}

//		g_sprite[1].x = 200;
	//	g_sprite[1].y = 300;


	//	g_sprite[1].x = 0;
	//	g_sprite[1].y = 450;
	}

        for (int i=0; i < linesToProcess; i++)
        {

                pre_figure_out(g_dglo.m_iniScanner.GetLine(iLineCount).c_str(), 0, true);
				iLineCount++;
				if (iLineCount == g_dglo.m_iniScanner.m_lines.size())
				{
				
					for (int b=0; b < g_dglo.m_iniScanner.m_lines.size(); b++)
					{
						   pre_figure_out(g_dglo.m_iniScanner.GetLine(b).c_str(), 0, false);
					}
					
					program_idata();

					percentOut = 1;
					g_dglo.m_iniScanner.Kill();
					return;
				}
			
				ProcessGraphicGarbageCollection();

		}

		//we'll need to come back do the rest
		percentOut = float(iLineCount) / float(g_dglo.m_iniScanner.m_lines.size());
}

void SetDinkGameState(eDinkGameState state)
{
	g_dinkGameState = state;
}

eDinkGameState GetDinkGameState()
{
	return g_dinkGameState;
}

void SetDefaultVars(bool bFullClear)
{
	ResetDinkTimers();

	g_dglo.m_lastActiveView = g_dglo.VIEW_NONE;
	g_dglo.UnSetViewOverride();
	g_dglo.m_lastGameMode = DINK_GAME_MODE_NONE;
	g_dglo.m_lastSubGameMode = DINK_SUB_GAME_MODE_NONE;
	g_dglo.m_bFullKeyboardActive = false;
	g_dglo.m_bForceControlsRebuild = false;
	g_dglo.m_bWaitingForSkippableConversation = false;
	g_dglos.g_DinkUpdateTimerMS =0;

	memset(g_dglo.m_dirInput, 0, sizeof(DINK_INPUT_COUNT*sizeof(bool)));
	memset(g_dglo.m_dirInputFinished, 0, sizeof(DINK_INPUT_COUNT*sizeof(bool)));
	clear_talk();
	

	no_transition = false;
	g_abort_this_flip = false;
	g_dglos.screenlock = 0;
	sound_on = true;
	g_dglos.g_pushingEnabled = 1;
	turn_on_plane = false;
	g_dglos.item_pic = 2;
	
	fire_forward = 0;
	g_dglos.g_stopEntireGame = 0;
	g_bInitiateScreenMove = false;
	g_bTransitionActive = false;

#ifdef _DEBUG
	//g_script_debug_mode = true; //script debugging mode. Alt-D toggles this also, plus there is a toggle on the debug menu
#endif
	g_dglos.flub_mode = -500;
	g_dglos.last_sprite_created = 1;
	decipher_savegame = 0;
	g_dglos.bFadedDown = false;
	g_dglos.smooth_follow = false;
	g_dglos.mSwapped = false;
	g_dglos.mLoopMidi = false;
	g_dglos.mDinkBasePush = 310;
	g_dglos.walk_off_screen = 0;
	g_dglos.keep_mouse = 0;
	g_dglos.show_dot = false;
	g_dglos.plane_process = true;
	g_dglos.weapon_script = 0;
	g_dglos.magic_script = 0;
	g_dglos.last_sprite_created;
	g_dglos.no_running_main = false;
	g_dglos.process_warp = 0;
	g_dglos.process_upcycle = false;
	g_dglos.process_downcycle = false;
	g_dglos.cycle_clock = 0;
	g_dglos.cycle_script = 0;
	g_dglos.but_timer = 0;
	g_dglo.m_bgSpriteMan.Clear();
	g_dglos.last_fill_screen_palette_color = 0;
	g_dglos.copy_bmp_to_screen[0] = 0;
	g_dglos.status_might_not_require_update = 0;

	no_cheat =  !GetApp()->GetCheatsEnabled();
	g_itemScreenActive = false;
	g_dglos.midi_active = true;
	last_saved_game = 0;
	g_b_kill_app = false; //if true, will close app as soon as the message pump is
	g_dglos.dinkspeed = 3;

	memset(&g_dglos.g_bShowingBitmap, 0, sizeof(ShowingBitmapInfo));
	memset(&g_dglos.g_bowStatus, 0, sizeof(attackinfo_struct));
	
	g_dglos.g_returnint = 0;
	g_dglos.bKeepReturnInt = false;
	g_dglos.process_count = 0;

	if (bFullClear)
	{
		g_dglo.m_curLoadState = 0;
		g_dglo.m_bSpeedUpMode = false;
		g_dglos.m_bRenderBackgroundOnLoad = false;

		memset(g_dglos.m_bufferForExpansion, 0, sizeof(g_dglos.m_bufferForExpansion));
		memset(g_dglos.g_picInfo, 0, sizeof(pic_info)*C_MAX_SPRITES);
		//memset(g_id, '\0', sizeof(idata) * max_idata);
		memset(g_dglos.g_seq, 0, sizeof(sequence) * C_MAX_SEQUENCES);
		kill_all_vars(); //it zero's out the player struct
		strcpy(g_dglos.current_map, "map.dat");
		strcpy(g_dglos.current_dat,"dink.dat");
		memset(&short_play, 0, sizeof(player_short_info));

		//redink1 code for version change
		strcpy(g_dglos.dversion_string, "v1.10");
		strcpy(g_dglos.save_game_info, "Level &level");
		g_dglos.g_curPicIndex = 1;
		//GetBaseApp()->SetGameTick(0); //can cause problems .. don't do it here
		g_dglos.time_start = GetBaseApp()->GetGameTick();
		g_dglos.g_dinkTick = GetTick(TIMER_GAME);
		g_dglos.g_playerInfo.minutes = 0;
	}
}

//in some cases, we include DMOD's with the app, but they are static and can't be changed
string GetDMODStaticRootPath()
{
	if (GetPlatformID() == PLATFORM_ID_IOS)
	{
		return GetBaseAppPath()+"dmods/";
	} 

	return ""; //unused

}

void ClearCommandLineParms()
{

	GetBaseApp()->GetCommandLineParms().clear();
}

string GetDMODRootPath(string *pDMODNameOutOrNull)
{
	if (pDMODNameOutOrNull)
	{
		*pDMODNameOutOrNull = "";
	}
	

#if defined(WIN32)
	
	string dmodpath = "dmods/";
	string refdir = "";

	vector<string> parms = GetBaseApp()->GetCommandLineParms();
	
	for (int i = 0; i < parms.size(); i++)
	{
		if (parms[i] == "--refdir" || parms[i] == "-dmodpath")
		{
			if (parms.size() > i + 1)
			{
				refdir = parms[i + 1]; i++;

				if (refdir[0] == '\"')
				{
					//special handling for quotes
				
					refdir = ""; //try again

					for (; i < parms.size(); i++)
					{
						if (!refdir.empty())
						{
							refdir += " ";
						}
						refdir += parms[i];
					}

					//pull just the part we want out
					refdir = SeparateStringSTL(refdir, 1, '\"');

				}

				StringReplace("\\", "/", refdir);
				if (refdir[refdir.size() - 1] != '/') refdir += '/'; //need a trailing slash

				//remove "
				StringReplace("\"", "", refdir);
			}
			else
			{
				LogMsg("--refdir used wrong");
			}
		}
	}
		for (int i = 0; i < parms.size(); i++)
		{
		if (parms[i] == "-game")
		{
			if (parms.size() > i + 1)
			{
				dmodpath = parms[i + 1]; i++;

				if (!refdir.empty())
				{
					if (dmodpath.find(":") != string::npos)
					{
						//the dmod dir we got already has a path. Ignore --refdir, it will just break things
					}
					else
					{
						//prepend the refdir path so it works with how dfarc does it
						dmodpath = refdir + dmodpath;
					}
				}
				StringReplace("\\", "/", dmodpath);
				if (dmodpath[dmodpath.size() - 1] != '/') dmodpath += '/'; //need a trailing slash

				int len = dmodpath.find_last_of("/", dmodpath.length() - 2);
				if (len == string::npos)
				{
					//no demod dir?  Weird but ok
					if (pDMODNameOutOrNull)
						*pDMODNameOutOrNull = dmodpath;
					dmodpath = "";
				}
				else
				{
					if (pDMODNameOutOrNull)
						*pDMODNameOutOrNull = dmodpath.substr(len + 1, dmodpath.length());
					dmodpath = dmodpath.substr(0, len + 1);
				}
			}
			else
			{
				LogMsg("-game used wrong");
			}
		}

	}
	
	return dmodpath;
#endif

	if (GetPlatformID() == PLATFORM_ID_WEBOS)
	{
		return "dmods/";
	}
	return GetAppCachePath();
}

void InitDinkPaths(string gamePath, string gameDir, string dmodGameDir)
{
	g_lastSaveSlotFileSaved = "";

	string dmodPath;
	if (!dmodGameDir.empty())
	{
		dmodPath = RemoveTrailingBackslash(GetPathFromString(RemoveTrailingBackslash(dmodGameDir)));
		dmodPath += "/"; //fixing problem with multiple slashes showing up on some OS's and am too lazy to "realyl" fix it or even correct that typo I just made
		
		dmodGameDir = RemoveTrailingBackslash (GetFileNameFromString(RemoveTrailingBackslash(dmodGameDir)));

		if (dmodPath == dmodGameDir)
		{
			//it's the old format that doesn't include the full path too, guess
			dmodPath = GetDMODRootPath();
			dmodPath = RemoveTrailingBackslash(dmodPath);
		}

	}
	StringReplace("/", "", dmodGameDir);
	StringReplace("\\", "", dmodGameDir);
	g_dglo.m_gamePath = gamePath;
	g_dglo.m_gameDir = gameDir + "/";
	g_dglo.m_gamePathWithDir = g_dglo.m_gamePath + g_dglo.m_gameDir;
	
	if (dmodGameDir.empty())
	{
		g_dglo.m_savePath =  GetSavePath()+ g_dglo.m_gameDir;
		g_dglo.m_dmodGamePathWithDir.clear();
		g_dglo.m_dmodGameDir.clear();
	} else
	{
		g_dglo.m_dmodGameDir = dmodGameDir+"/";
		g_dglo.m_savePath =  GetDMODRootPath()+g_dglo.m_dmodGameDir;
		g_dglo.m_dmodGamePathWithDir = dmodPath+g_dglo.m_dmodGameDir;
	}
}

bool InitDinkEngine()
{
	g_lastSaveSlotFileSaved = "";

	OneTimeDinkInit();
	finiObjects();
	SetDefaultVars(true);
	g_dglo.SetView(DinkGlobals::VIEW_FULL);
	g_b_kill_app = false;
	
#ifdef C_DINK_KEYBOARD_INPUT
	while (keypressed()); //clear keyboard buffer
#endif
	
	return true;
	 
}

//call this repeately until progress == 1
bool LoadGameChunk(int gameIDToLoad, float &progressOut)
{

	switch(g_dglo.m_curLoadState)
	{

	case 0:
		progressOut = 0.05f;
		
		if (!InitializeVideoSystem())
		{
			return false;
		}

		//redink1 init for color depth information

		assert(!lpDDSBackGround);
		
		//init back buffer at 8 bit, if highcolor is needed later it will auto convert
		lpDDSBackGround = InitOffscreenSurface(C_DINK_SCREENSIZE_X, C_DINK_SCREENSIZE_Y, IDirectDrawSurface::MODE_SHADOW_GL, false);
		DDBLTFX     ddbltfx;
		ddbltfx.dwFillColor = g_dglos.last_fill_screen_palette_color;
		lpDDSBackGround->Blt(NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);

		lpDDSBuffer = InitOffscreenSurface(C_DINK_SCREENSIZE_X, C_DINK_SCREENSIZE_Y, IDirectDrawSurface::MODE_SHADOW_GL);

		

		if (!lpDDSBuffer ) return false;
		
		//blit_background();

		sound_on = InitSound();

		break;

	case 1:
	progressOut = 0.1f;
	
	break;

	case 2:
	{
		//this one is going to get called a lot...
		
		progressOut = 0.2f;
		//LogMsg("loading batch...");
		float percent = 0;
		load_batch(20, percent);

		//get the progress meter to update within the range of this chunk type
		progressOut += .5 * percent;
		
		if (percent != 1)
		{
			//call this one again next time
			return true;
		}
	}

	break;
	
	case 3:
	progressOut = 0.7f;

	if (!load_hard()) return false;
	break;

	//Activate dink, but don't really turn him on
	g_sprite[1].timer = 33;

	//copy from player info
	g_sprite[1].x = g_dglos.g_playerInfo.x;
	g_sprite[1].y = g_dglos.g_playerInfo.y;

// ** SETUP **
	g_dglos.last_sprite_created = 1;
	g_dglos.g_gameMode = 0;
	case 4:
	progressOut = 0.75;
	load_info();        

	break;

	case 5:
progressOut = 0.8f;
#ifdef C_DINK_KEYBOARD_INPUT
	//clear keyboard buffer 
	for (int x=0; x<256; x++)
	{
		GetKeyboard(x);
	} 
#endif

	for (int u = 1; u <= 10; u++)
		g_dglos.g_playerInfo.button[u] = u;

	for (int x1=1; x1 <= 10; x1++) 
	{
		sjoy.letgo[x1] = true;
	}

	init_font_colors();
	//lets run our init script
	int script;
	break;
	
	case 6:
		//StopMidi();

	progressOut = 0.9f;	
	script = load_script("main", 0, true);
	locate(script, "main");
	run_script(script);
	attach();

	break;
	case 7:
	progressOut = 0.97f;
	//first time init
	SetupFirstScript();
	//to load a game..
	// script = load_script("instant_load", 1000, true);

	if (gameIDToLoad == 0)
	{
		if (g_dglo.m_dmodGameDir.empty())
		{
			//to start from scatch
			script = load_script("newgame", 1000, true);
			locate(script, "main");
			run_script(script);
		} 
	
	} else
	{
		if (!load_game(gameIDToLoad))
		{
			LogError("Couldn't load save game");
		}
	}
		
	break;
	
	case 8:
		progressOut = 1;
		g_dglo.m_curLoadState = FINISHED_LOADING-1;
		break;

	default:

		assert(!"oops");
	
	}
	
	g_dglo.m_curLoadState++;
	return true;
}

void DinkGlobals::SetView( eView view )
{
	if (m_viewOverride != VIEW_NONE)
	{
		view = m_viewOverride;
	} else
	{
		m_curView = view;
	}
	switch (view)
	{
	case VIEW_ZOOMED:
	
		g_dglo.m_aspectRatioModX = g_dglo.m_aspectRatioModY = 1.0f;
		g_dglo.m_centeringOffset = CL_Vec2f(0, 0);
		g_dglo.m_nativeGameArea = rtRectf(0,0,GetScreenSizeX(),GetScreenSizeY());
		g_dglo.m_gameArea = rtRect32 (20, 0, 620, 400);
		g_dglo.m_orthoRenderRect = rtRect32 (20, 0, 620, 400);
		break;
	
	case VIEW_FULL_WITHOUT_EDGES:
		g_dglo.m_nativeGameArea = rtRectf(0,0,480,267);
		g_dglo.m_gameArea = rtRect32 (20, 0, 620, 400);
		g_dglo.m_orthoRenderRect = rtRect32 (20, 0, 620, 480);
		break;

	case VIEW_FULL:
		{
		g_dglo.m_gameArea = rtRect32 (20, 0, 620, 400);
		
		float aspect = (float(C_DINK_SCREENSIZE_X)/GetScreenSizeXf());
		float aspectY = (float(C_DINK_SCREENSIZE_Y)/GetScreenSizeYf());

		g_dglo.m_nativeGameArea = rtRectf(float(g_dglo.m_gameArea.left)/ aspect,0,float(g_dglo.m_gameArea.right)/aspect,float(g_dglo.m_gameArea.bottom)/aspectY);
		g_dglo.m_orthoRenderRect = rtRect32 (0, 0, 640, 480);
		RecomputeAspectRatio();

#ifdef _DEBUG
		LogMsg("DinkGlobals::SetView>VIEW_FULL Rect %s AspectX: %.4f, Aspect Y: %.4f", PrintRect(g_dglo.m_nativeGameArea).c_str(), aspect, aspectY);
#endif
		}
		
		break;

	default:
		
		assert(!"Unhandled view");

	}

	
	g_dglo.m_fontSize = 1.24;

	if (IsIPADSize || IsDesktop())
	{
		//the screens are big enough here were we can keep the text smaller, comparable to the original Dink
		g_dglo.m_fontSize = 0.88f;
	}
	
	
	if (IsLargeScreen())
	{
		g_dglo.m_fontSize = 0.88f;
		g_dglo.m_fontSize *= (C_DINK_SCREENSIZE_X/GetScreenSizeXf());
	}

}

void DinkGlobals::SetViewOverride( eView view )
{
	m_viewOverride = view;
	SetView(m_curView);
}

void DinkGlobals::UnSetViewOverride()
{
	m_viewOverride = VIEW_NONE;
	SetView(m_curView);
}

void DinkGlobals::ToggleView()
{

	if (g_bTransitionActive) return; //now is a bad time..

	if (m_curView == VIEW_FULL)
	{
		SetView(VIEW_ZOOMED);
	} else if (m_curView == VIEW_ZOOMED)
	{
		SetView(VIEW_FULL);
	} else
	{
		assert(!"huh?");
	}
}


void DinkUnloadGraphicsCache()
{
	GetAudioManager()->KillCachedSounds(false, true, 0, 1, false);
	
	DinkUnloadUnusedGraphicsByUsageTime(100); //unload anything not used in the last second

#ifdef _WIN32
	/*
	if (GetKeyboard(18))
	{
		for (int i=1; i < C_MAX_SEQUENCES; i++)
		{
			FreeSequence(i);
		}
	}
	*/
#endif
}


//to help with debugging something..
bool IsCorruptedSeq(int seq)
{
	return false;

	if (g_dglos.g_seq[seq].frame[1] != 0)
	{
		if (g_pSpriteSurface[g_dglos.g_seq[seq].frame[1]])
		{
			//the first deal has something in it.  But do they all?
			
			for (int g=0; g < g_dglos.g_seq[seq].last; g++)
			{
				if (!g_pSpriteSurface[g_dglos.g_seq[seq].frame[1] + g])
				{
					LogMsg("bad..");
				}
			}


		}
	}


	//looks blank

	return false;
}

//send in 1000*60 and all images not used in the last one minute will be destroyed

void DinkUnloadUnusedGraphicsByUsageTime(unsigned int timeMS)
{
	
	const unsigned int tickMS = GetBaseApp()->GetGameTick();
	unsigned int mostRecentUsageTick = 0;

	for (int i=1; i < C_MAX_SEQUENCES; i++)
	{
			for (int g=0; g < g_dglos.g_seq[i].last; g++)
			{
				//assert(g_dglos.g_picInfo[g_dglos.g_seq[i].frame[1]+g].pSurface);
				if (g_pSpriteSurface[g_dglos.g_seq[i].frame[1] + g])
				{
					if (g_pSpriteSurface[g_dglos.g_seq[i].frame[1] + g]->m_gameTickOfLastUse+timeMS <= tickMS)
					{
						SAFE_DELETE(g_pSpriteSurface[g_dglos.g_seq[i].frame[1] + g]);
					}
				}
			}
	}

	for (int i=1; i < C_TILE_SCREEN_COUNT; i++)
	{
		if (g_tileScreens[i])
		{
			if (g_tileScreens[i]->m_gameTickOfLastUse + timeMS <= tickMS)
			{
				SAFE_DELETE(g_tileScreens[i]);
			}
		}
	}
}

void ProcessGraphicGarbageCollection()
{
	//unload images that haven't been used in a while.  If we accidentally unload the wrong one, or all of them, no harm done, the engine
	//will reload them as needed.
	
	/*
	if (GetBaseApp()->GetTexMemUsed() < 1024*1024*13) return;
	
	static uint32 garbageTimer = 0;

	if (garbageTimer < GetBaseApp()->GetGameTick())
	{
		
		garbageTimer = GetBaseApp()->GetGameTick()+5000;
	} else
	{
		return;
	}
	
	LogMsg("Garbage collection..");
	DinkUnloadUnusedGraphicsByUsageTime(1000*120);

	*/
	if (GetBaseApp()->GetMemUsed() > C_DINK_MEM_MAX_ALLOWED || GetBaseApp()->GetTexMemUsed() > C_DINK_TEX_MEM_MAX_ALLOWED)
	{
		DinkUnloadUnusedGraphicsByUsageTime(1000*5);
		GetAudioManager()->KillCachedSounds(false, true, 5000, 1,false);
		if (GetBaseApp()->GetMemUsed() > C_DINK_MEM_CACHE_MAX_ALLOWED_AFTER_A_DUMP || GetBaseApp()->GetTexMemUsed() > C_DINK_TEX_MEM_MAX_ALLOWED_AFTER_A_DUMP)
		{
			//still having problems.. let's go deeper
			DinkUnloadUnusedGraphicsByUsageTime(1000*3);
			
			if (GetBaseApp()->GetMemUsed() > C_DINK_MEM_CACHE_MAX_ALLOWED_AFTER_A_DUMP || GetBaseApp()->GetTexMemUsed() > C_DINK_TEX_MEM_MAX_ALLOWED_AFTER_A_DUMP)
			{
				//still having problems.. let's go deeper
				DinkUnloadUnusedGraphicsByUsageTime(1000*1);

				if (GetBaseApp()->GetMemUsed() > C_DINK_MEM_CACHE_MAX_ALLOWED_AFTER_A_DUMP || GetBaseApp()->GetTexMemUsed() > C_DINK_TEX_MEM_MAX_ALLOWED_AFTER_A_DUMP)
				{
					//still having problems.. let's go deeper
					DinkUnloadUnusedGraphicsByUsageTime(0);
					//GetAudioManager()->KillCachedSounds(false, false, 5000, 1, true);
				}
			}

		}
	}
}

eDinkGameMode GetDinkGameMode()
{

	if (g_itemScreenActive) return DINK_GAME_MODE_INVENTORY;
	if (g_dglos.g_gameMode == 1 || g_sprite[1].brain == 13) return DINK_GAME_MODE_MOUSE;
	if (g_dglos.g_gameMode == 2) return DINK_GAME_MODE_NORMAL; //loading a map I guess
	if (g_dglos.g_gameMode == 3) return DINK_GAME_MODE_NORMAL;
	if (g_dglos.g_gameMode == 0) return DINK_GAME_MODE_NORMAL; //uhh.. dink died?
	
	assert(!"Unknown mode");
	
	return DINK_GAME_MODE_NORMAL;
}

bool IsDrawingDinkStatusBar()
{
	return *pupdate_status != 0;
}

eDinkSubGameMode GetDinkSubGameMode()
{
	if (g_dglos.g_bShowingBitmap.active) return DINK_SUB_GAME_MODE_SHOWING_BMP;
	if (g_dglos.g_talkInfo.active) return DINK_SUB_GAME_MODE_DIALOG;

	//default
	return DINK_SUB_GAME_MODE_NONE;
}


bool SaveHeader(FILE *fp)
{
	SaveToFile(SAVE_FORMAT_VERSION, fp);
	SaveToFile(g_dglo.m_dmodGamePathWithDir, fp);
	SaveToFile(g_dglo.m_gameDir, fp);
	
	SaveToFile((uint32)GetApp()->GetGameTick(), fp);
	return true;
}

bool LoadHeader(FILE *fp)
{
	float version;
	LoadFromFile(version, fp);
	if (version < SAVE_FORMAT_VERSION) //save_state_version
	{
		LogMsg("Save state from newer version?!");
		return false;
	}

	string tempGameDir = g_dglo.m_dmodGameDir;

	LoadFromFile(g_dglo.m_dmodGameDir, fp);
	LoadFromFile(g_dglo.m_gameDir, fp);

	if (!g_dglo.m_dmodGameDir.empty())
	{
		if (!FileExists(g_dglo.m_dmodGameDir + "dmod.diz"))
		{
			LogMsg("DMOD directory invalid. Trying original directory %s instead", g_dglo.m_savePath.c_str());
			g_dglo.m_dmodGameDir = g_dglo.m_savePath;
		}
	}


	uint32 gameTime;
	LoadFromFile(gameTime, fp);
	GetApp()->SetGameTick(gameTime);

	
	//let's initialize our data based on this
	InitDinkPaths(GetBaseAppPath(),  RemoveTrailingBackslash(g_dglo.m_gameDir), RemoveTrailingBackslash(g_dglo.m_dmodGameDir));
	return true;
}

bool SaveSoundState(FILE *fp)
{
	if (!GetAudioManager()->IsPlaying(GetAudioManager()->GetLastMusicID()))
	{
		g_dglo.m_lastMusicPath = "";
	} 

	
	SaveToFile(g_dglo.m_lastMusicPath, fp);
	if (g_dglo.m_lastMusicPath.empty())
	{
		SaveToFile(uint32(0), fp);
	} else
	{
		SaveToFile(GetAudioManager()->GetPos(GetAudioManager()->GetLastMusicID()), fp);
	}

	fwrite(&soundinfo, 1, sizeof(soundstruct)*(num_soundbanks+1), fp);

	for (int i=0; i < max_sounds; i++)
	{
		SaveToFile(g_soundInfo[i].m_fileName, fp);
	}
	
	for (int i=1; i < num_soundbanks; i++)
	{
		if (soundbank[i].IsInUse())
		{
			if (soundinfo[i].repeat)
			{
				SaveToFile(i, fp);
				SaveToFile(soundinfo[i].owner, fp);
				SaveToFile(soundinfo[i].survive, fp);
				SaveToFile(soundinfo[i].repeat, fp);
				SaveToFile(soundinfo[i].vol, fp);
				SaveToFile(soundinfo[i].freq, fp);
				SaveToFile(soundbank[i].m_soundIDThatPlayedUs, fp);
			}
		}
	}

	//signal that we're done
	SaveToFile((int32)0, fp);
	return true;
}

bool LoadSoundState(FILE *fp)
{
	
	LoadFromFile(g_dglo.m_lastMusicPath, fp);
	
	uint32 pos;
	LoadFromFile(pos, fp);

	if (!g_dglo.m_lastMusicPath.empty())
	{
		PlayMidi(g_dglo.m_lastMusicPath.c_str());
		GetAudioManager()->SetPos(GetAudioManager()->GetLastMusicID(), pos);
	}
	
	fread(&soundinfo, 1,  sizeof(soundstruct)*(num_soundbanks+1), fp);
	for (int i=0; i < max_sounds; i++)
	{
		LoadFromFile(g_soundInfo[i].m_fileName, fp);
	}

	int32 bankNum;

	while(1)
	{
		LoadFromFile(bankNum, fp);
		if (bankNum == 0) break;

		int32 owner;
		LoadFromFile(owner, fp);
		int32 survive;
		LoadFromFile(survive, fp);
		int32 repeat;
		LoadFromFile(repeat, fp);
		int32 vol;
		LoadFromFile(vol, fp);
		int32 freq;
		LoadFromFile(freq, fp);

		int32 soundID;
		LoadFromFile(soundID, fp);

		playbank(soundID, freq, 0, owner, repeat, bankNum);
		soundinfo[bankNum].survive = survive;
	}
	
	return true;
}

bool SaveScriptState(FILE *fp)
{
	//first save the actual script data
	
	for (int32 i = 1; i < C_MAX_SCRIPTS; i++)
	{
		if (g_scriptInstance[i])
		{
			SaveToFile(i, fp);
			fwrite(g_scriptInstance[i], 1, sizeof(refinfo), fp);
			//now write the actual script
			fwrite(g_scriptBuffer[i], 1, g_scriptInstance[i]->end, fp);
		}
	}

	SaveToFile((int32)0, fp); //signal end
	return true;
}

bool SaveSpriteState(FILE *fp)
{
	for (int32 i=0; i < C_MAX_SPRITES_AT_ONCE; i++)
	{

		if (g_customSpriteMap[i] != 0)
		{
			g_sprite[i].m_containsSpriteMapData = 1; //signal we have additional sprite data
		}
		else
		{
			g_sprite[i].m_containsSpriteMapData = 0;
		}

		fwrite(&g_sprite[i], sizeof(SpriteStruct), 1, fp);

		if (g_customSpriteMap[i] != 0)
		{
			int32 count = g_customSpriteMap[i]->size();

			SaveToFile(count, fp); //let them know how many are coming

			std::map<std::string, int32>::iterator itor = g_customSpriteMap[i]->begin();

			while (itor != g_customSpriteMap[i]->end())
			{
				SaveToFile((*itor).first, fp);
				SaveToFile((*itor).second, fp);
				itor++;
			}

		}
	}
	
	return true; //success
}

bool LoadSpriteState(FILE *fp)
{
	for (int i=0; i < C_MAX_SPRITES_AT_ONCE; i++)
	{
		SAFE_DELETE(g_customSpriteMap[i]);

		fread(&g_sprite[i], sizeof(SpriteStruct), 1, fp);

		if (g_sprite[i].m_containsSpriteMapData != 0)
		{
			int sizeofBool = sizeof(bool);

			int32 count;
			LoadFromFile(count, fp);
			
			g_customSpriteMap[i] = new std::map<std::string, int32>;

			for (int propCount = 0; propCount < count; propCount++)
			{
				string st;
				int32 value;
				LoadFromFile(st, fp);
				LoadFromFile(value, fp);
				//insert it
				(*g_customSpriteMap[i])[st] = value;
			}
		}
	}

	return true; //success
}

bool LoadScriptState(FILE *fp)
{

	while (1)
	{
		int32 scriptID;
		LoadFromFile(scriptID, fp);
		if (scriptID == 0) break; //all done

		//load this script data
		assert(!g_scriptInstance[scriptID]);
		g_scriptInstance[scriptID] = (refinfo*) malloc(sizeof(refinfo));
		fread(g_scriptInstance[scriptID], 1, sizeof(refinfo), fp);
		assert(!g_scriptBuffer[scriptID]);
		g_scriptBuffer[scriptID] = new char[g_scriptInstance[scriptID]->end+1];
		g_scriptBuffer[scriptID][g_scriptInstance[scriptID]->end] = 0;
		fread(g_scriptBuffer[scriptID], 1,g_scriptInstance[scriptID]->end, fp);
	}

	return true;
}


bool SaveState(string const &path, bool bSyncSaves)
{
	LogMsg("Saving %s (inside %s which is off of %s)", path.c_str(), g_dglo.m_gameDir.c_str(), g_dglo.m_savePath.c_str());
	CreateDirectoryRecursively(g_dglo.m_savePath, g_dglo.m_gameDir);
	CreateDirectoryRecursively("", g_dglo.m_savePath); //added to fix issue with emscripten

	FILE *fp = fopen(path.c_str(), "wb");
	
	
	//why unload crap when you're saving?!
	//DinkUnloadUnusedGraphicsByUsageTime(0);

	if (!fp)
	{
		LogMsg("Error saving state");
		return false;
	}

	bool bOk = true;

	bOk = SaveHeader(fp);

	if (bOk)
	{
		int dinkGloSize = sizeof(DinkGlobalsStatic);
		fwrite(&dinkGloSize, 1, sizeof(int), fp);
		
		fwrite(&g_dglos, 1, dinkGloSize, fp);
	}

	if (bOk)
	{
		bOk = SaveScriptState(fp);
	}

	if (bOk)
	{
		bOk = SaveSpriteState(fp);
		bOk = SaveSoundState(fp);
	}

	fclose(fp);

	if (bSyncSaves)
		//SyncPersistentData();

	return bOk; //success
}

bool GetDMODDirFromState(string const &path, string &dmodDirOut)
{
	LogMsg("Loading %s", path.c_str());
	FILE *fp = fopen(path.c_str(), "rb");

	if (!fp)
	{
		LogMsg("Unable to load state, file not found");
		return false;
	}
	
	float version;
	LoadFromFile(version, fp);
	if (version < SAVE_FORMAT_VERSION) //save_state_version
	{
		LogMsg("Save state from newer version?!");
		return false;
	}
	string unused;

	LoadFromFile(dmodDirOut, fp);
	LoadFromFile(unused, fp);

	StringReplace("\\", "/", dmodDirOut);
	bool bDidRemoveTrailingSlash = false;

	if (dmodDirOut[dmodDirOut.length() - 1] == '/')
	{
		dmodDirOut = dmodDirOut.substr(0, dmodDirOut.length() - 1);
		bDidRemoveTrailingSlash = true;
	}
	int index = dmodDirOut.find_last_of('/');
	if (index != string::npos)
	{
		//just grab the last dmod dir part
		dmodDirOut = dmodDirOut.substr(index+1, dmodDirOut.length());
	}

	fclose(fp);

	if (bDidRemoveTrailingSlash)
	{
		//put it back
		dmodDirOut += "/";
	}
	return true;
}

bool LoadState(string const &path, bool bLoadPathsOnly)
{
	LogMsg("Loading %s", path.c_str());
	FILE *fp = fopen(path.c_str(), "rb");
	
	if (!fp)
	{
		LogMsg("Unable to load state, file not found");
		return false;
	}

	if (bLoadPathsOnly)
	{

		float version;
		LoadFromFile(version, fp);
		if (version < SAVE_FORMAT_VERSION)
		{
			LogMsg("Save state from newer version?!");
			fclose(fp);
			return false;
		}
		fclose(fp);
		return true;
	}

	StopMidi();
	DinkUnloadUnusedGraphicsByUsageTime(0);
	KillScriptAccelerators();
	kill_all_sounds();
	kill_all_scripts_for_real();
	ResetDinkTimers();
	g_dglo.m_bgSpriteMan.Clear();
	g_dglo.UnSetViewOverride();

	bool bOk = LoadHeader(fp);


	if (bOk)
	{
		//keep loading
		int dinkGloStaticSize = 0;
		fread(&dinkGloStaticSize, 1, sizeof(int32), fp);
		memset(&g_dglos, 0, sizeof(DinkGlobalsStatic));
		fread(&g_dglos, 1, dinkGloStaticSize, fp);

	
		//kill the bad pointers to the graphics
		for (int i=0; i < C_MAX_SPRITES; i++)
		{
			g_pSpriteSurface[i] = NULL;
		}

		load_hard();
		load_info();

	}

	if (bOk)
	{
		bOk = LoadScriptState(fp);
	}
	if (bOk)
	{
		bOk = LoadSpriteState(fp);
		bOk = LoadSoundState(fp);
	}
	
	fclose(fp);
	if (!attach())
	{
		return false;
	}

	if ( g_dglos.g_gameMode > 2  || g_dglos.m_bRenderBackgroundOnLoad)
	{
		if (g_dglos.m_bRenderBackgroundOnLoad && g_sprite[1].brain != 13)
		{
			g_forceBuildBackgroundFromScratch = true;
		}

		if (g_sprite[1].brain != 13)
			BuildScreenBackground(false, true);
	}
	
	if (g_dglos.g_bShowingBitmap.active)
	{
		CopyBitmapToBackBuffer(g_dglos.g_lastBitmapShown);
	}

	if (g_dglos.copy_bmp_to_screen[0] != 0)
	{
		copy_bmp(g_dglos.copy_bmp_to_screen);
	}
	
	return bOk; //success
}

void DinkModStrength(int mod)
{
	*pstrength += mod;
	GetAudioManager()->Play("dink/sound/secret.wav");
}

void DinkModDefense(int mod)
{
	*pdefense += mod;
	GetAudioManager()->Play("dink/sound/secret.wav");
}

void DinkModMagic(int mod)
{
	*pmagic+= mod;
	GetAudioManager()->Play("dink/sound/secret.wav");
}

void DinkModLifeMax(int mod)
{
	*plifemax += mod;
	*plife += mod;
	GetAudioManager()->Play("dink/sound/secret.wav");
}

void DinkFillLife()
{
	*plife = *plifemax;
}

void DinkModGold(int mod)
{
	*pgold += mod;
	GetAudioManager()->Play("dink/sound/secret.wav");
}

Surface* DinkGetMagicIconImage()
{
	if (*pcur_magic == 0) return NULL;
	item_struct *pItem = &g_dglos.g_playerInfo.g_MagicData[*pcur_magic];
	int picID = g_dglos.g_seq[pItem->seq].frame[pItem->frame];
	if (picID == 0) return NULL;
	check_pic_status(picID);
	
	return g_pSpriteSurface[picID]->GetGLSuface();
}


Surface* DinkGetWeaponIconImage()
{
	if (*pcur_weapon == 0) return NULL;
	item_struct *pItem = &g_dglos.g_playerInfo.g_itemData[*pcur_weapon];
	int picID = g_dglos.g_seq[pItem->seq].frame[pItem->frame];
	if (picID == 0) return NULL;

	check_pic_status(picID);
	return g_pSpriteSurface[picID]->GetGLSuface();
}

float DinkGetMagicChargePercent()
{
	if (*pmagic_cost > 0 && *pmagic_level > 0)
	{
		return float(*pmagic_level) / float(*pmagic_cost);


	} 

	return 0;

}

float DinkGetHealthPercent()
{
	if (*plifemax > 0)
	{
		return float(g_dglos.g_guiLife)/ float(g_dglos.g_guiLifeMax);
	} 
	return 0;
}
bool DinkIsDoingScreenTransition()
{
	return g_bTransitionActive;
}

string DinkGetSavePath()
{
	return g_dglo.m_savePath;
}

void DinkAddBow()
{
	add_item("item-b2", 438, 12, false);
}

bool DinkGetSpeedUpMode()
{
	return g_dglo.m_bSpeedUpMode;
}

void DinkSetSpeedUpMode(bool bSpeedup)
{
	g_dglo.m_bSpeedUpMode = bSpeedup;
}


void SaveStateWithExtra()
{
	if (!lpDDSBack || g_dglo.m_curLoadState != FINISHED_LOADING) return;

	LogMsg("Saving state");
	GetAudioManager()->Play("audio/quick_save.wav");
	SaveState(DinkGetSavePath()+"quicksave.dat");
	ShowQuickMessage("State saved.");
};

void SaveAutoSave()
{

	
	LogMsg("Saving autosave");
	//GetAudioManager()->Play("audio/quick_save.wav");
	SaveState(DinkGetSavePath()+"autosave.dat");
	
	
	//add extra info because I don't want to change the save format
	uint32 minutes = ( GetBaseApp()->GetGameTick()-g_dglos.time_start) / (1000*60);
	
	VariantDB db;
	db.GetVar("minutes")->Set(minutes);
	db.GetVar("description")->Set("Level "+toString(*plevel));
	
#ifdef _DEBUG
	LogMsg("Saving autosave to %s", (DinkGetSavePath()+"autosavedb.dat").c_str());
#endif
	db.Save(DinkGetSavePath()+"autosavedb.dat", false);

	ShowQuickMessage("(Auto-saved)");
};

void LoadStateWithExtra(string forcedFileName)
{
	if (!lpDDSBack || g_dglo.m_curLoadState != FINISHED_LOADING) return;

	//LogMsg("Loading state");
	string fName = DinkGetSavePath() + "quicksave.dat";
	
	if (!forcedFileName.empty())
	{
		fName = forcedFileName;
	}

	GetAudioManager()->Play("audio/quick_load.wav");
	LoadState(fName, false);
	ShowQuickMessage("State loaded.");

};

//note, you must glPopMatrix(); yourself if you use this...

void ApplyAspectRatioGLMatrix()
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	
	
	if (g_dglo.m_aspectRatioModX == 1.0f && g_dglo.m_aspectRatioModY == 1.0f)
	{
		//not actually using this
		g_dglo.m_centeringOffset = CL_Vec3f(0,0,0);
		return;
	}

	//float offsetX = ((float)GetPrimaryGLX() - (float)C_DINK_SCREENSIZE_X) / 2;

	glScalef(g_dglo.m_aspectRatioModX, g_dglo.m_aspectRatioModY, 1);

	CL_Mat4f mat;
	glGetFloatv(GL_MODELVIEW_MATRIX, &mat[0]);
	g_dglo.m_dink_matrix = mat;
	//OPTIMIZE - All this can be cached... maybe done in RecomputeAspectRatio()
	mat.inverse();
	g_dglo.m_dink_matrix_inverted = mat;

	CL_Vec3f vTotal =  mat.get_transformed_point(CL_Vec3f(C_DINK_SCREENSIZE_X, C_DINK_SCREENSIZE_Y, 0));
	CL_Vec3f vDinkSize = mat.get_transformed_point(CL_Vec3f(C_DINK_SCREENSIZE_X*g_dglo.m_aspectRatioModX, C_DINK_SCREENSIZE_Y *g_dglo.m_aspectRatioModY, 0));

	//CL_Vec3f vScreenToWorldPixelMod;

	//vScreenToWorldPixelModvTotal.x / vDinkSize

	g_dglo.m_centeringOffset = (vTotal - vDinkSize)/2.0f;

	glTranslatef(g_dglo.m_centeringOffset.x, g_dglo.m_centeringOffset.y, 0);

	//glGetFloatv(GL_PROJECTION_MATRIX, &Matrix);
}

void RecomputeAspectRatio()
{
	bool bStretchToFit = GetApp()->GetVar("check_stretch")->GetUINT32() != 0;

	if (bStretchToFit || g_dglo.GetActiveView() == DinkGlobals::VIEW_ZOOMED)
	{
		g_dglo.m_aspectRatioModX = 1.0f;
		g_dglo.m_aspectRatioModY = 1.0f;

	}
	else
	{
		float aspect_r = (float)GetPrimaryGLX() / (float)GetPrimaryGLY(); // aspect ratio
		const float correctAspectRatio = (float)C_DINK_SCREENSIZE_X / (float)C_DINK_SCREENSIZE_Y;

		/*
		if (GetFakePrimaryScreenSizeX() != 0)
		{
		//more reliable way to get the aspect ratio
		aspect_r=(float)GetFakePrimaryScreenSizeX()/(float)GetFakePrimaryScreenSizeY(); // aspect ratio
		}
		*/

		float aspectChange = correctAspectRatio / aspect_r;


		if (aspectChange < 1.0f)
		{
			//width too big, but leave Y where it is
			g_dglo.m_aspectRatioModX = aspectChange;
			g_dglo.m_aspectRatioModY = 1.0f;

		}
		else
		{
			//opposite problem, leave width where it is, but scale Y
			g_dglo.m_aspectRatioModX = 1.0f;
			g_dglo.m_aspectRatioModY = aspect_r / correctAspectRatio;
		}

	}
}

void DinkReInitSurfacesAfterVideoChange()
{
	if (GetDinkGameState() == DINK_GAME_STATE_PLAYING)
	{
		g_transitionSurf.HardKill();
		//CheckTransitionSurface();
		bool bHighColor = false;
		if (lpDDSBackGround && lpDDSBackGround->m_pSurf && lpDDSBackGround->m_pSurf->GetSurfaceType() == SoftSurface::SURFACE_RGBA)
			bHighColor = true;

		SAFE_DELETE(lpDDSBackGround);
		LogMsg("Initting Surface after video change");
		lpDDSBackGround = InitOffscreenSurface(C_DINK_SCREENSIZE_X, C_DINK_SCREENSIZE_Y, IDirectDrawSurface::MODE_SHADOW_GL, bHighColor);
		DDBLTFX     ddbltfx;
		ddbltfx.dwFillColor = g_dglos.last_fill_screen_palette_color;
		lpDDSBackGround->Blt(NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);

		g_onePixelSurf.HardKill();

		
	}

	RecomputeAspectRatio();
}

void DinkOnForeground()
{
	if (GetDinkGameState() == DINK_GAME_STATE_PLAYING)
	{

		bool bForceReinit = false;

#ifdef WINAPI
		//force reinit on windows even if using iphone or android mode
		bForceReinit = true;
#endif
#ifdef PLATFORM_HTML5
		bForceReinit = true;
#endif

		if (IsDesktop() || GetEmulatedPlatformID() == PLATFORM_ID_ANDROID || bForceReinit) //xoom needs this too after a suspend/resume from hitting the power button
		{
			LogMsg("Forcing surface unloads via DinkOnForeground");
			DinkReInitSurfacesAfterVideoChange();
		}

		//reinit any lost surfaces that we need to
		if ( g_dglos.g_gameMode > 2  || g_dglos.m_bRenderBackgroundOnLoad )
		{
			if (g_dglos.m_bRenderBackgroundOnLoad && g_sprite[1].brain != 13)
			{
				g_forceBuildBackgroundFromScratch = true;
			}

			if (g_sprite[1].brain != 13 || g_dglos.m_bRenderBackgroundOnLoad)
			{
				//brain 13 means mouse so we probably don't need this.  Could be wrong though..
				BuildScreenBackground(false);
			}
		}

		if (g_dglos.g_bShowingBitmap.active)
		{
			CopyBitmapToBackBuffer(g_dglos.g_lastBitmapShown);
		}
		if (g_dglos.copy_bmp_to_screen[0] != 0)
		{
			copy_bmp(g_dglos.copy_bmp_to_screen);
		}
	}

}

void WriteLastPathSaved(string dmodDir)
{
	GetApp()->GetShared()->GetVar("last_saved_path")->Set(dmodDir);
}

string ReadLastPathSaved()
{
	return GetApp()->GetShared()->GetVar("last_saved_path")->GetString();
}
