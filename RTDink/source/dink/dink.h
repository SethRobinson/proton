#ifndef dink_h__
#define dink_h__

#include "../App.h"

#define DSBSTATUS_PLAYING           0x00000001
#define DSBSTATUS_BUFFERLOST        0x00000002
#define DSBSTATUS_LOOPING           0x00000004
#define DSBSTATUS_LOCHARDWARE       0x00000008
#define DSBSTATUS_LOCSOFTWARE       0x00000010
#define DSBSTATUS_TERMINATED        0x00000020
#include "misc_util.h"
#include "../video_gl.h"
#include "util/TextScanner.h"

extern bool g_dinkMouseRightClick;
bool InitDinkEngine();
bool LoadGameChunk(int gameIDToLoad, float &progressOut); //0 for new game
void updateFrame();
void finiObjects();
void ClearCommandLineParms();

struct SpriteStruct;

//finished loading is used with g_dglo.m_curLoadState to fake the loader out
#define FINISHED_LOADING 100

void ClearBitmapCopy();
void ApplyAspectRatioGLMatrix();
void RecomputeAspectRatio();

void CheckForHotkeys();
enum eDinkInput
{
	DINK_INPUT_UP,
	DINK_INPUT_RIGHT,
	DINK_INPUT_DOWN,
	DINK_INPUT_LEFT,
	DINK_INPUT_BUTTON1, //punch
	DINK_INPUT_BUTTON2, //talk
	DINK_INPUT_BUTTON3,
	DINK_INPUT_BUTTON4,
	DINK_INPUT_BUTTON5, //escape
	DINK_INPUT_BUTTON6, //map
	DINK_INPUT_BUTTON7,

	DINK_INPUT_COUNT
};

enum eDinkGameMode
{
	DINK_GAME_MODE_NONE,
	DINK_GAME_MODE_MOUSE,
	DINK_GAME_MODE_NORMAL,
	DINK_GAME_MODE_INVENTORY,
};

enum eDinkSubGameMode
{
	DINK_SUB_GAME_MODE_NONE,
	DINK_SUB_GAME_MODE_NORMAL,
	DINK_SUB_GAME_MODE_SHOWING_BMP,
	DINK_SUB_GAME_MODE_DIALOG
};

struct BackgroundSprite
{
	rtRect32 dstRect;
	rtRect32 srcRect;
	int32 pic;
};

class BackgroundSpriteManager
{
public:

	void Clear();
	void Add(int spriteID);
	void Render(LPDIRECTDRAWSURFACE lpdest);
	int GetCount() {return m_sprites.size();}
private:

	deque<BackgroundSprite> m_sprites;
};

class DinkGlobals
{
public:

	enum eView
	{
		VIEW_ZOOMED,
		VIEW_FULL_WITHOUT_EDGES,
		VIEW_FULL, //all 640X480 of it, used in the title screen parts
		VIEW_ONE_TO_ONE,
		VIEW_NONE,
		VIEW_COUNT
	};
	
	DinkGlobals()
	{
		memset(m_dirInput, 0, sizeof(DINK_INPUT_COUNT*sizeof(bool)));
		memset(m_dirInputFinished, 0, sizeof(DINK_INPUT_COUNT*sizeof(bool)));
		m_curLoadState = 0;
		m_bFullKeyboardActive = false;
		m_bLastFullKeyboardActive = false;
		m_bSpeedUpMode = false;
		m_bUsingDinkPak = false;
		m_bForceControlsRebuild = false;
		m_aspectRatioModX = 1.0f; //not the aspect ratio, the change to the aspect ratio so it's the correct aspect ratio.  I know, it makes no sense
		m_aspectRatioModY = 1.0f;

	}
	
	void SetView(eView view);
	void ToggleView();
	void SetViewOverride(eView view); //don't allow anyone to change it
	void UnSetViewOverride();
	eView GetActiveView() {if (m_viewOverride != VIEW_NONE) return m_viewOverride; return m_curView;}
	bool m_dirInput[DINK_INPUT_COUNT];
	bool m_dirInputFinished[DINK_INPUT_COUNT]; //allows us to handle button up/down events on the same frame without losing
	//data
	string m_lastMusicPath;
	string m_gamePath;  //where the dink dir is located
	string m_gamePathWithDir; //same as above but with /dink tagged on
	string m_gameDir; //"dink/"
	string m_dmodGamePathWithDir;
	string m_dmodGameDir;
	string m_savePath; //where the save games are
	float m_fontSize;
	rtRectf m_nativeGameArea;
	rtRect32 m_gameArea;
	rtRect32 m_orthoRenderRect; //control which part of Dink's 640X480 get drawn to the screen and at what scale
	unsigned int m_transitionTimer;
	float m_transitionProgress;
	CL_Vec2f m_transitionOffset;
	CL_Vec2f m_transitionOffsetNative; //same thing in actual screen pixels
	eView m_curView;
	eView m_viewOverride;
	bool m_bWaitingForSkippableConversation;
	//handle the loading process
	int m_curLoadState;
	TextScanner m_iniScanner;
	eDinkGameMode m_lastGameMode;
	eDinkSubGameMode m_lastSubGameMode;
	eView m_lastActiveView;
	bool m_lastIsUpdatingDinkStatusBar;
	bool m_bFullKeyboardActive;
	bool m_bLastFullKeyboardActive;
	bool m_bSpeedUpMode;
	bool m_bForceControlsRebuild;

	BackgroundSpriteManager m_bgSpriteMan;
	bool m_bUsingDinkPak;
	float m_aspectRatioModX;
	float m_aspectRatioModY;
	CL_Vec3f m_centeringOffset;
	CL_Mat4f m_dink_matrix;
	CL_Mat4f m_dink_matrix_inverted;
};
//#define KYLES_CRAZY_VERSION

#ifdef KYLES_CRAZY_VERSION
const int C_MAX_SEQUENCES = 9999; //Max # of sprite animations
const int C_MAX_SPRITES = 19999;
const int C_MAX_SPRITES_AT_ONCE = 500;
const int C_MAX_SCRIPT_CALLBACKS = 300;
const int max_vars = 5000;

#define C_MAX_SPRITE_FRAMES 99
#define C_SPRITE_MAX_FILENAME_SIZE 64
const int C_MAX_SCRIPTS = 400;

#else

//Watch yourself, changing any of these will break quicksaves unless you check the quicksave version and upgrading old saves
const int32 C_MAX_SEQUENCES = 1300; //Max # of sprite animations
const int32 C_MAX_SPRITES = 6000;
const int32 C_MAX_SPRITES_AT_ONCE = 300; //don't change, there are still hardcoded numbers elsewhere and some weird stuff with screen lock and "flub" stuff whatever that is
const int32 C_MAX_SCRIPT_CALLBACKS = 100;
const int32 max_vars = 250;

#define C_MAX_SPRITE_FRAMES 100
#define C_SPRITE_MAX_FILENAME_SIZE 64
const int32 C_MAX_SCRIPTS = 200;


#endif


struct sequence
{ 
	int16 frame[C_MAX_SPRITE_FRAMES];
	int16 originalFrame[C_MAX_SPRITE_FRAMES];
	int32 delay[C_MAX_SPRITE_FRAMES];
	unsigned char special[C_MAX_SPRITE_FRAMES];

	byte active;
	short m_xoffset, m_yoffset;
	rtRect32 m_hardbox;
	eTransparencyType m_transType;
	byte m_bLeftAlign;
	int32 m_speed;
	char m_fileName[C_SPRITE_MAX_FILENAME_SIZE];
	short x,y;
	short s;
	byte last;
	byte m_spaceAllowed;
	byte m_bFrameSetUsed;
	byte m_bIsAnim;
	byte m_bDidFileScan;
	
};

const int32 C_DINK_VERSION = 110;
const int32 num_soundbanks = 20;
const int32 max_idata = 1000; 
const int32 max_sounds = 100;
const int32 text_timer = 77;
const int32 text_min = 2700;


const int32 max_game = 20;
const int32 C_TILE_SCREEN_COUNT = 41+1;

const int32 g_gameAreaRightBarStartX = 620;
const int32 g_gameAreaLeftOffset = 20;
const int32 C_DINK_ORIGINAL_GAME_AREA_Y = 400; //redink1's fix for the 'no sprites on pixel line above status bar' bug


eDinkGameMode GetDinkGameMode();
eDinkSubGameMode GetDinkSubGameMode();

struct font_color
{
	int32 red;
	int32 green;
	int32 blue;
};

struct ShowingBitmapInfo
{
	bool active;
	bool showdot;
	int32 reserved;
	int32 script;
	int32 stime;
	int32 picframe;
};

struct wait_for_button
{
	int32 script;
	int32 button;
	bool active;
};


struct talk_struct
{
	char line[21][101];
	int32 line_return[21];
	char buffer[3000];
	int32 cur;
	int32 last;
	bool active;
	int32 cur_view;
	int32 cur_view_end;
	int32 page;
	int32 script;
	int32 offset;
	int32 newy;
	int32 color;
	int32 curf;
	int32 timer;
};

struct mydata
{
	unsigned char type[100];
	unsigned short seq[100];
	unsigned char frame[100];
	int32 last_time;
};

struct varman
{
	int32 var;
	char name[20];
	int32 scope;
	bool active;
};


struct item_struct
{
	bool active;
	char name[10];
	int32 seq;
	int32 frame;
};

struct player_info_tile
{
	char file[50];
};

struct global_function
{
	char file[10];
	char func[20];
};

struct player_info
{
	int32 version;
	char gameinfo[196];
	int32 minutes;
	int32 x,y,die, size, defense, dir, pframe, pseq, seq, frame, strength, base_walk, base_idle, base_hit,que;

	item_struct g_MagicData[9]; //1 index based, man I was dumb(er) back then
	item_struct g_itemData[17];

	int32 curitem, unused;
	int32 counter;
	bool idle;
	mydata spmap[769];
	int32 button[10];
	varman var[max_vars];
	bool push_active;
	int32 push_dir;
	uint32 push_timer;
	int32 last_talk;
	int32 mouse;
	bool item_magic;
	int32 last_map;
	int32 crap;
	int32 buff[95];
	uint32 dbuff[20];
	int32 lbuff[10];

	//redink1... use wasted space for storing file location of map.dat, dink.dat, palette, and tiles
	char mapdat[50];
	char dinkdat[50];
	char palette[50];
	player_info_tile tile[42];
	global_function func[100];
	uint32 m_gameTime;
	char  cbuff[746];
};

struct attackinfo_struct
{
	int32 time;
	bool active;
	int32 script;
	bool hitme;
	int32 last_power;
	int32 wait;
	int32 pull_wait;
};

struct player_short_info
{
	int32 version;
	char gameinfo[196];
	int32 minutes;
};

struct refinfo
{
	char name[10];
	int32 location;
	int32 current;
	int32 level;
	int32 end;
	int32 sprite; //if more than 0, it was spawned and is owned by a sprite, if 1000 doesn't die
	bool skipnext;
	int32 onlevel;
	int32 proc_return;
	int32 arg1;
	int32 arg2;
	int32 arg3;
	int32 arg4;
	int32 arg5;
	int32 arg6;
	int32 arg7;
	int32 arg8;
	int32 arg9;
};

struct call_back
{
	int32 owner;
	bool active;
	int32 type;
	char name[20];
	int32 offset;
	int32 min, max;
	int32 lifespan;
	uint32 timer;
};

struct SpriteStruct
{
	int32 x,moveman;
	int32 y; 
	int32 mx,my;
	int32 lpx[C_MAX_SPRITE_FRAMES],lpy[C_MAX_SPRITE_FRAMES];
	int32 speed;
	int32 brain;
	int32 seq_orig,dir;
	int32 seq;
	int32 frame;
	uint32 delay;
	int32 pseq;
	int32 pframe;

	bool active;
	int32 attrib;
	uint32 wait;
	int32 timer;
	int32 skip;
	int32 skiptimer;
	int32 size;
	int32 que;
	int32 base_walk;
	int32 base_idle;
	int32 base_attack;

	int32 base_hit;
	int32 last_sound;
	int32 hard;
	rtRect32 alt;
	int32 althard;
	int32 sp_index;
	bool nocontrol;
	int32 idle;
	int32 strength;
	int32 damage;
	int32 defense;
	int32 hitpoints;
	int32 exp;
	int32 gold;
	int32 base_die;
	int32 kill;
	int32 kill_timer;
	int32 script_num;
	char text[200];
	int32 owner;
	int32 script;
	int32 sound;
	int32 callback;
	int32 freeze;
	bool move_active;
	int32 move_script;
	int32 move_dir;
	int32 move_num;
	bool move_nohard;
	int32 follow;
	int32 nohit;
	bool notouch;
	uint32 notouch_timer;
	bool flying;
	int32 touch_damage;
	int32 brain_parm;
	int32 brain_parm2;
	bool noclip;
	bool reverse;
	bool disabled;
	int32 target;
	int32 attack_wait;
	int32 move_wait;
	int32 distance;
	int32 last_hit;
	bool live;
	int32 range;
	int32 attack_hit_sound;
	int32 attack_hit_sound_speed;
	int32 action;
	int32 nodraw;
	int32 frame_delay;
	int32 picfreeze;
	//redink1
	int32 bloodseq;
	int32 bloodnum;
	int32 m_containsSpriteMapData;  //1 if yes, 0 if no (the save/load function adds data here if needed)
};

extern std::map<std::string, int32>* g_customSpriteMap[C_MAX_SPRITES_AT_ONCE];

struct seth_joy
{
	bool joybit[17]; //is button held down?
	bool letgo[17]; //copy of old above
	bool button[17]; //has button been pressed recently?
	bool key[256];
	bool kletgo[256];
	bool realkey[256];
	bool right,left,up,down;
	bool rightd,leftd,upd,downd;
	bool rightold,leftold,upold,downold;

};


//sub struct for hardness map

struct mega_y
{
	byte y[401];
};

//struct for hardness map

struct hit_map
{
	mega_y x[601];
};

//sub struct for tile hardness

#define C_DINK_TILE_SIZE_IN_PIXELS 50
struct block_y
{
	byte y[C_DINK_TILE_SIZE_IN_PIXELS+1]; //the +1 is because I did everything 1 index based.  Yeah, I was an idiot.
};

struct ts_block
{
	block_y x[C_DINK_TILE_SIZE_IN_PIXELS+1];
	bool used;
	int32 hold;
};

//struct for hardness info, INDEX controls which hardness each block has.  800 max
//types available.
struct hardness
{
	ts_block tile[800];
	int32 index[8000];
};

struct map_info
{
	char name[20];
	int32 loc[769];
	int32 music[769];
	int32 indoor[769];
	int32 v[40];
	char s[80];
	char buffer[2000];

};

struct tile
{
	int32 num, property, althard, more2;
	byte  more3,more4;
	int32 buff[15];
};

struct sprite_placement
{
	int32 x,y,seq,frame, type,size;
	bool active;
	int32 rotation, special,brain;
	char script[13];
	char hit[13];
	char die[13];
	char talk[13];
	int32 speed, base_walk,base_idle,base_attack,base_hit,timer,que;
	int32 hard;
	rtRect32 alt;
	int32 prop;
	int32 warp_map;
	int32 warp_x;
	int32 warp_y;
	int32 parm_seq;
	int32 base_die, gold, hitpoints, strength, defense,exp, sound, vision, nohit, touch_damage;
	int32 buff[5];
};

struct small_map
{
	char name[20];
	tile t[97];
	int32 v[40];
	char s[80];
	sprite_placement sprite[101];

	char script[13];
	char random[13];
	char load[13];
	char buffer[1000];
};


struct pic_info
{
	int32 m_filler; //used to store a pointer here, was moved out of the struct as it's saved/loaded in a raw way that depended on 32 bit pointers
	rtRect32                box,hardbox;
	int16 yoffset;
	int16 xoffset;
	int16 m_parentSeq;
	byte m_bCustomSettingsApplied;
	int16 m_parentFrame;
};

extern LPDIRECTDRAWSURFACE     g_pSpriteSurface[C_MAX_SPRITES];

struct seth_sound
{
	string m_fileName;
};

struct soundstruct
{
	bool repeat;
	int32 owner;
	int32 survive;
	int32 vol;
	int32 freq; //what speed it was played at
};

class SoundBankDummy
{
public:
	SoundBankDummy()
	{
		m_audioID = AUDIO_HANDLE_BLANK;
	}

	void Stop() {GetAudioManager()->Stop(m_audioID); m_audioID = 0; m_soundIDThatPlayedUs = 0;};
	void SetPan(float f) {GetAudioManager()->SetPan(m_audioID,f/1700);};
	void SetVolume(float f){GetAudioManager()->SetVol(m_audioID, (1800+f) / 1800);};
	bool IsInUse() {if (m_audioID == 0) return false; return GetAudioManager()->IsPlaying(m_audioID);}
	void Reset()
	{
		if (m_audioID)
		{
			GetAudioManager()->Stop(m_audioID);
			m_audioID = AUDIO_HANDLE_BLANK;
		}
	}
	AudioHandle m_audioID;
	int32 m_soundIDThatPlayedUs;
};

#define C_SHOWN_BITMAP_SIZE 128

struct DinkGlobalsStatic
{
	sequence g_seq[C_MAX_SEQUENCES];
	pic_info g_picInfo[C_MAX_SPRITES];       // Sprite data

	ShowingBitmapInfo g_bShowingBitmap;
	wait_for_button g_wait_for_button;
	attackinfo_struct g_bowStatus;
	small_map g_smallMap;
	player_info g_playerInfo;
	
	talk_struct g_talkInfo;
	hit_map g_hitmap;

	int32 but_timer;
	int32 dinkspeed;


	uint32 time_start;
	uint32 g_DinkUpdateTimerMS;
	uint32 mDinkBasePush;
	int32 walk_off_screen;
	int32 keep_mouse;
	int32 last_sprite_created;
	int32 g_curPicIndex; //we actually need this set before finiObjects is called
	int32 cur_map,cur_tile;
	uint32 g_dinkTick,lastTickCount; 
	char current_map[50];
	char current_dat[50];
	char dversion_string[6];
	char save_game_info[200];
	font_color font_colors[16];
	int32 g_stopEntireGame;
	int32 g_pushingEnabled;
	bool no_running_main;
	int32 process_count;
	int32 show_dot;
	int32 plane_process;
	int32 base_timing;
	int32 weapon_script;
	int32 magic_script;
	int32 g_gameMode;
	bool midi_active;
	bool mLoopMidi;
	int32 flub_mode;
	int32 process_warp;
	bool process_upcycle;
	bool process_downcycle;
	uint32 cycle_clock;
	int32 cycle_script;
	bool bFadedDown;
	bool smooth_follow;
	bool mSwapped;
	int32 item_timer;
	int32 item_pic;
	int32 mbase_count;
	int32 mbase_timing;
	call_back g_scriptCallback[C_MAX_SCRIPT_CALLBACKS];
	int32 g_guiLife, g_guiExp, g_guiStrength, g_guiDefense, g_guiGold, g_guiMagic, g_guiMagicLevel, g_guiLifeMax, g_guiRaise, g_guiLastMagicDraw;
	int32 g_returnint;
	bool bKeepReturnInt;
	int32 screenlock;

	bool m_bRenderBackgroundOnLoad;
	
	
	char g_lastBitmapShown[C_SHOWN_BITMAP_SIZE];
	int32 last_fill_screen_palette_color;

	char copy_bmp_to_screen[C_SHOWN_BITMAP_SIZE];
	int32 status_might_not_require_update;

	char m_bufferForExpansion[4835];
};



enum eDinkGameState
{
	//helps us to know when to automatically save the game
	DINK_GAME_STATE_NOT_PLAYING,
	DINK_GAME_STATE_PLAYING
};
extern DinkGlobals g_dglo;
extern DinkGlobalsStatic g_dglos; //static data, made to write/read from disk
extern string g_lastSaveSlotFileSaved;

bool load_game_small(int num, char * line, int *mytime);
void InitDinkPaths(string gamePath, string gameDir, string dmodGameDir);
void DinkUnloadGraphicsCache();
void ProcessGraphicGarbageCollection();
string GetDMODRootPath(string *pDMODNameOutOrNull = NULL); //where dmods are stored
bool DinkIsWaitingForSkippableDialog();
bool DinkSkipDialogLine(); //returns true if a line was actually skipped
void DinkSetCursorPosition(CL_Vec2f vPos);
CL_Vec2f NativeToDinkCoords(CL_Vec2f vPos);
CL_Vec2f DinkToNativeCoords(CL_Vec2f vPos);
bool DinkIsMouseActive();
bool IsDrawingDinkStatusBar();
bool DinkSetInventoryPosition(CL_Vec2f vPos); //returns true if an item was actually set
bool DinkCanRunScriptNow();
bool DinkLoadPlayerScript(const string fileName);
void DinkUnloadUnusedGraphicsByUsageTime(unsigned int timeMS);

bool LoadState(string const &path, bool bLoadPathsOnly);
bool SaveState(string const &path, bool bSyncSaves = true);
bool GetDMODDirFromState(string const &path, string &dmodDirOut);
eDinkGameState GetDinkGameState();
void SetDinkGameState(eDinkGameState state);
void DinkModStrength(int mod);
void DinkModDefense(int mod);
void DinkModMagic(int mod);
void DinkModLifeMax(int mod);
void DinkFillLife();
void DinkModGold(int mod);
Surface* DinkGetMagicIconImage();
Surface* DinkGetWeaponIconImage();
float DinkGetMagicChargePercent();
float DinkGetHealthPercent();
bool DinkIsDoingScreenTransition();
string DinkGetSavePath();
void DinkAddBow();
bool DinkGetSpeedUpMode();
void DinkSetSpeedUpMode(bool bSpeedup);
void SaveStateWithExtra();
void LoadStateWithExtra(string forcedFileName="");
void SaveAutoSave();
void DinkOnForeground(); 
string GetDMODStaticRootPath();
void DinkReInitSurfacesAfterVideoChange();
void WriteLastPathSaved(string dmodDir);
string ReadLastPathSaved();
//is the DMOD dir if applicable

#endif // dink_h__