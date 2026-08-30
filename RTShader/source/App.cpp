/*
 *  App.cpp - the RTShader example
 *  For license info, check the license.txt file that should have come with this.
 *
 *  A minimal tour of Proton's shader pipeline, in two steps:
 *
 *    1. Draw a normal 2D scene into an offscreen texture instead of the
 *       screen (Surface::InitRenderTarget / BeginRenderTarget / EndRenderTarget).
 *
 *    2. Blit that texture to the screen through a custom GLSL fragment
 *       shader (the RTShader class + SetActiveShader), giving a full-screen
 *       post-processing effect in a few lines of shader code.
 *
 *  Click / tap (or press space) to cycle through the effects.
 *
 *  While an RTShader is active, EVERY engine draw call renders through it:
 *  Surface blits, fonts, DrawFilledRect, the RenderBatcher... so the same
 *  technique also works on individual sprites, not just full screens.
 *
 *  The engine binds these for you in any custom shader that declares them:
 *    attributes:  a_pos (vec4), a_uv (vec2), a_color (vec4)
 *    uniforms:    uProj, uMV (mat4), uColor (vec4), uTex (sampler2D, unit 0)
 *  Anything else is yours; set it with SetUniform1f/SetUniform4f.
 *
 *  Write GLSL ES 1.00 style shaders and they run everywhere Proton does:
 *  Windows/Mac/Linux (desktop GL), iOS/Android (GLES2) and WebGL.
 */

#include "PlatformPrecomp.h"
#include "App.h"
#include "Entity/CustomInputComponent.h" //used for the back button (android) / escape (desktop)
#include "Entity/FocusInputComponent.h" //needed to let the input component see input messages
#include "Manager/MessageManager.h"

#ifndef RT_SHADER_PIPELINE_AVAILABLE
#error RTShader is a shader pipeline example; the project must compile shared/Renderer/ShaderPipeline.cpp and define RT_SHADER_PIPELINE_AVAILABLE (see any of the included platform projects)
#endif

MessageManager g_messageManager;
MessageManager * GetMessageManager() {return &g_messageManager;}

FileManager g_fileManager;
FileManager * GetFileManager() {return &g_fileManager;}

#include "Audio/AudioManager.h"
AudioManager g_audioManager; //to disable sound, this is a dummy
AudioManager * GetAudioManager(){return &g_audioManager;}

#ifdef PLATFORM_OSX
// Required by MainController.mm and BaseApp.cpp - defined in SDL2Main.cpp for SDL builds
bool g_bIsFullScreen = false;
#endif

App *g_pApp = NULL;

BaseApp * GetBaseApp()
{
	if (!g_pApp)
	{
		g_pApp = new App;
	}
	return g_pApp;
}

App * GetApp()
{
	assert(g_pApp && "GetBaseApp must be called used first");
	return g_pApp;
}

//---------------------------------------------------------------------------
// The effects.  One shared vertex shader (a plain 2D transform), and one
// tiny fragment shader per effect.  That's really all there is to it.
//---------------------------------------------------------------------------

//the standard 2D vertex shader: transform the position, pass the UV along.
//Every effect below uses this same one.
static const char *g_vertexShader =
	"attribute vec4 a_pos;\n"
	"attribute vec2 a_uv;\n"
	"uniform mat4 uProj;\n"
	"uniform mat4 uMV;\n"
	"varying vec2 v_uv;\n"
	"void main()\n"
	"{\n"
	"	gl_Position = uProj * uMV * vec4(a_pos.xyz, 1.0);\n"
	"	v_uv = a_uv;\n"
	"}\n";

//effect 1: wavy.  Offsets each row's UV by a moving sine wave.
static const char *g_fragWave =
	"uniform sampler2D uTex;\n"
	"uniform vec4 uColor;\n"
	"uniform float uTime;\n"
	"varying vec2 v_uv;\n"
	"void main()\n"
	"{\n"
	"	vec2 uv = v_uv;\n"
	"	uv.x += sin(uv.y * 20.0 + uTime * 3.0) * 0.02;\n"
	"	gl_FragColor = texture2D(uTex, uv) * uColor;\n"
	"}\n";

//effect 2: grayscale.  Classic luminance weights.
static const char *g_fragGrayscale =
	"uniform sampler2D uTex;\n"
	"uniform vec4 uColor;\n"
	"varying vec2 v_uv;\n"
	"void main()\n"
	"{\n"
	"	vec4 c = texture2D(uTex, v_uv);\n"
	"	float gray = dot(c.rgb, vec3(0.299, 0.587, 0.114));\n"
	"	gl_FragColor = vec4(gray, gray, gray, c.a) * uColor;\n"
	"}\n";

//effect 3: color cycle.  Rotates the color channels around over time.
static const char *g_fragColorCycle =
	"uniform sampler2D uTex;\n"
	"uniform vec4 uColor;\n"
	"uniform float uTime;\n"
	"varying vec2 v_uv;\n"
	"void main()\n"
	"{\n"
	"	vec4 c = texture2D(uTex, v_uv);\n"
	"	float t = uTime * 0.7;\n"
	"	vec3 cycled = c.rgb * (0.5 + 0.5 * cos(t + vec3(0.0, 2.1, 4.2)))\n"
	"	            + c.gbr * (0.5 - 0.5 * cos(t + vec3(0.0, 2.1, 4.2)));\n"
	"	gl_FragColor = vec4(cycled, c.a) * uColor;\n"
	"}\n";

//effect 4: CRT-ish scanlines plus a vignette tint (shows SetUniform4f).
static const char *g_fragScanlines =
	"uniform sampler2D uTex;\n"
	"uniform vec4 uColor;\n"
	"uniform vec4 uEdgeColor;\n"
	"varying vec2 v_uv;\n"
	"void main()\n"
	"{\n"
	"	vec4 c = texture2D(uTex, v_uv);\n"
	"	float scan = 0.85 + 0.15 * sin(v_uv.y * 800.0);\n"
	"	float edge = distance(v_uv, vec2(0.5, 0.5));\n"
	"	vec3 outColor = mix(c.rgb * scan, uEdgeColor.rgb, edge * edge * uEdgeColor.a);\n"
	"	gl_FragColor = vec4(outColor, c.a) * uColor;\n"
	"}\n";

static const char *g_effectNames[EFFECT_COUNT] = { "No shader", "Wavy", "Grayscale", "Color cycle", "Scanlines" };

//---------------------------------------------------------------------------

App::App()
{
	m_bDidPostInit = false;
	m_curEffect = 1; //start on Wavy so it's obvious a shader is running
}

App::~App()
{
}

bool App::Init()
{
	if (m_bInitted)
	{
		return true;
	}

	if (!BaseApp::Init()) return false;

	LogMsg("The Save path is %s", GetSavePath().c_str());

	if (!GetFont(FONT_SMALL)->Load("interface/font_trajan.rtfont")) return false;

	//optional: let the regression harness (or a curious user) pick the
	//starting effect with "-effect <0..4>" on the command line
	vector<string> parms = GetCommandLineParms();
	for (unsigned int i = 0; i < parms.size(); i++)
	{
		if (ToLowerCaseString(parms[i]) == "-effect" && i + 1 < parms.size())
		{
			m_curEffect = atoi(parms[i + 1].c_str()) % EFFECT_COUNT;
		}
	}

	GetBaseApp()->SetFPSVisible(true);

	//run uncapped (vsync off, platform timers freed) so the FPS display shows
	//real render throughput.  Android stays display-synced (GLSurfaceView),
	//and phone/tablet displays still limit what you can SEE, but everywhere
	//the number is what the hardware can actually do.
	SetFPSLimit(0);
	return true;
}

void App::Kill()
{
	BaseApp::Kill();
}

void App::OnExitApp(VariantList *pVarList)
{
	LogMsg("Exiting the app");
	OSMessage o;
	o.m_type = OSMessage::MESSAGE_FINISH_APP;
	GetBaseApp()->AddOSMessage(o);
}

void App::NextEffect()
{
	m_curEffect = (m_curEffect + 1) % EFFECT_COUNT;
	LogMsg("Switched to effect %d: %s", m_curEffect, g_effectNames[m_curEffect]);
}

//tap/click anywhere (or press space) to cycle effects
static void AppOnInput(VariantList *pVList)
{
	eMessageType msgType = eMessageType(int(pVList->Get(0).GetFloat()));

	switch (msgType)
	{
	case MESSAGE_TYPE_GUI_CLICK_START:
		GetApp()->NextEffect();
		break;

	case MESSAGE_TYPE_GUI_CHAR:
		if ((char)pVList->Get(2).GetUINT32() == ' ') GetApp()->NextEffect();
		break;

	default:
		break;
	}
}

void App::Update()
{
	BaseApp::Update();

	if (!m_bDidPostInit)
	{
		//stuff I want loaded during the first "Update"
		m_bDidPostInit = true;

		//for android, so the back key (or escape on windows) will quit out of the app
		Entity *pEnt = GetEntityRoot()->AddEntity(new Entity);
		EntityComponent *pComp = pEnt->AddComponent(new CustomInputComponent);
		pComp->GetVar("keycode")->Set(uint32(VIRTUAL_KEY_BACK));
		pComp->GetFunction("OnActivated")->sig_function.connect(1, boost::bind(&App::OnExitApp, this, _1));
		pEnt->AddComponent(new FocusInputComponent); //nothing will happen unless we give it input focus

		GetBaseApp()->m_sig_input.connect(&AppOnInput);
	}
}

//compile the effect shaders the first time we need them (a GL context must
//exist, so we can't do it in Init).  Loading a shader is one call: give
//RTShader::Load your vertex and fragment source and it compiles+links.
bool App::InitEffectsIfNeeded()
{
	if (m_effect[1].IsLoaded()) return true;

	//m_effect[0] deliberately stays unloaded: it means "draw normally"
	if (!m_effect[1].Load(g_vertexShader, g_fragWave)) return false;
	if (!m_effect[2].Load(g_vertexShader, g_fragGrayscale)) return false;
	if (!m_effect[3].Load(g_vertexShader, g_fragColorCycle)) return false;
	if (!m_effect[4].Load(g_vertexShader, g_fragScanlines)) return false;

	//uniforms can be set any time; they're applied whenever the shader is active
	m_effect[4].SetUniform4f("uEdgeColor", 0.1f, 0.05f, 0.3f, 1.5f); //vignette tint (a is strength)
	return true;
}

//step 1: draw an ordinary animated 2D scene, but into a texture instead of
//the screen.  Everything between BeginRenderTarget and EndRenderTarget lands
//in m_sceneSurf.  Coordinates work exactly like normal screen drawing.
void App::DrawSceneIntoTexture()
{
	//animate with the game tick so the demo is deterministic (the regression
	//harness's -autoscreenshot mode locks the timestep; wall-clock animation
	//would make every capture different)
	float animTime = float(GetGameTick()) / 1000.0f;

	m_sceneSurf.BeginRenderTarget();

	glClearColor(0.08f, 0.13f, 0.30f, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	float w = float(m_sceneSurf.GetWidth());
	float h = float(m_sceneSurf.GetHeight());

	//a row of pulsing bars along the bottom
	for (int i = 0; i < 12; i++)
	{
		float barH = h * (0.12f + 0.10f * sinf(animTime * 2.0f + float(i) * 0.7f));
		DrawFilledRect(w * float(i) / 12.0f + 4.0f, h - barH, w / 12.0f - 8.0f, barH,
			MAKE_RGBA(60 + i * 16, 220 - i * 12, 255, 255));
	}

	//the proton logo bouncing around, plus a spinning copy
	float logoX = (w - m_logoSurf.GetWidth()) * (0.5f + 0.5f * sinf(animTime * 1.3f));
	float logoY = (h * 0.5f - m_logoSurf.GetHeight()) * (0.5f + 0.5f * sinf(animTime * 1.7f));
	m_logoSurf.Blit(logoX, logoY);
	m_logoSurf.BlitScaled(w * 0.5f, h * 0.55f, CL_Vec2f(0.6f, 0.6f), ALIGNMENT_CENTER,
		MAKE_RGBA(255, 255, 255, 255), animTime * 90.0f);

	GetFont(FONT_SMALL)->DrawScaled(20.0f, 20.0f, "This scene was rendered to a texture!", 1.0f);

	m_sceneSurf.EndRenderTarget();

	glClearColor(0, 0, 0, 1); //back to the engine default
}

void App::Draw()
{
	PrepareForGL();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	CLEAR_GL_ERRORS() //emscripten quirk, see RTBareBones

	if (!m_logoSurf.IsLoaded())
	{
		m_logoSurf.LoadFile("interface/test.rttex");
	}

	//on desktop builds the shader pipeline is the default, but someone may
	//have launched with -fixedpipeline; render targets and custom shaders
	//don't exist there, so say so instead of drawing garbage
	if (!g_bShaderPipelineActive)
	{
		GetFont(FONT_SMALL)->DrawScaled(20.0f, GetScreenSizeYf() * 0.5f,
			"This demo needs the shader pipeline. Re-run without -fixedpipeline!", 1.0f);
		BaseApp::Draw();
		return;
	}

	//(re)create the offscreen render target at screen size (handles the
	//first frame and any later resolution change)
	if (!m_sceneSurf.IsRenderTarget() || m_sceneSurf.GetWidth() != GetScreenSizeX()
		|| m_sceneSurf.GetHeight() != GetScreenSizeY())
	{
		if (!m_sceneSurf.InitRenderTarget(GetScreenSizeX(), GetScreenSizeY()))
		{
			GetFont(FONT_SMALL)->Draw(20, 20, "InitRenderTarget failed?!");
			BaseApp::Draw();
			return;
		}
	}

	if (!InitEffectsIfNeeded())
	{
		GetFont(FONT_SMALL)->Draw(20, 20, "Shader compile failed, check the log");
		BaseApp::Draw();
		return;
	}

	//step 1: the scene goes into a texture...
	DrawSceneIntoTexture();

	//step 2: ...and the texture goes to the screen, through the current
	//effect.  While a shader is active every engine draw uses it, so the
	//Blit below (and anything else we drew before deactivating) is filtered.
	RTShader *pEffect = NULL;
	if (m_curEffect != 0 && m_effect[m_curEffect].IsLoaded()) pEffect = &m_effect[m_curEffect];

	if (pEffect)
	{
		SetActiveShader(pEffect);
		//feed the shaders their time uniform, tick-driven for determinism
		pEffect->SetUniform1f("uTime", float(GetGameTick()) / 1000.0f);
	}

	m_sceneSurf.Blit(0, 0);

	SetActiveShader(NULL); //back to normal rendering for the UI text below

	char msg[128];
	sprintf(msg, "Effect %d of %d: %s  (click, tap or space to change)", m_curEffect + 1,
		EFFECT_COUNT, g_effectNames[m_curEffect]);
	GetFont(FONT_SMALL)->Draw(20.0f, GetScreenSizeYf() - 40.0f, msg);

	BaseApp::Draw();
}

void App::OnScreenSizeChange()
{
	BaseApp::OnScreenSizeChange();
}

void App::OnEnterBackground()
{
	LogMsg("Entered background");
	BaseApp::OnEnterBackground();
}

void App::OnEnterForeground()
{
	LogMsg("Entered foreground");
	BaseApp::OnEnterForeground();
}

const char * GetAppName() {return "RTShader";}

//the stuff below is for android/webos builds.  Your app needs to be named like this.

const char * GetBundlePrefix()
{
	const char * bundlePrefix = "com.rtsoft.";
	return bundlePrefix;
}

const char * GetBundleName()
{
	const char * bundleName = "RTShader";
	return bundleName;
}

bool App::OnPreInitVideo()
{
	//only called for desktop systems
#if defined (_DEBUG) && defined(WINAPI)
	SetupScreenInfo(1024, 768, ORIENTATION_DONT_CARE);
#endif
	return true; //no error
}
