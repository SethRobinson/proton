/*
 *  App.h - the RTShader example
 *  For license info, check the license.txt file that should have come with this.
 *
 *  The smallest possible demonstration of Proton's shader pipeline features:
 *  render-to-texture (Surface::InitRenderTarget) and custom GLSL effects
 *  (the RTShader class + SetActiveShader).  See App.cpp for the tour.
 */

#pragma once

#include "BaseApp.h"
#include "Renderer/ShaderPipeline.h" //RTShader, SetActiveShader

const int EFFECT_COUNT = 7; //effect 0 is "no shader", the rest are in App.cpp

class App: public BaseApp
{
public:

	App();
	virtual ~App();
	virtual bool Init();
	virtual void Kill();
	virtual void Draw();
	virtual void OnScreenSizeChange();
	virtual void OnEnterBackground();
	virtual void OnEnterForeground();
	virtual bool OnPreInitVideo();
	virtual void Update();
	void OnExitApp(VariantList *pVarList);

	void NextEffect();

private:

	void DrawSceneIntoTexture();
	void DrawSpinningCube(bool bUseCustomShader);
	bool InitEffectsIfNeeded();

	bool m_bDidPostInit;
	int m_curEffect;

	SurfaceAnim m_logoSurf;  //a normal texture used inside the little scene
	Surface m_sceneSurf;     //the render target the scene is drawn into
	RTShader m_effect[EFFECT_COUNT]; //[0] stays unloaded ("no shader")
};


App * GetApp();
const char * GetAppName();
const char * GetBundlePrefix();
const char * GetBundleName();
