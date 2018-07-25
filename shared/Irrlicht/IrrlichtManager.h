//  ***************************************************************
//  IrrlichtManager - Creation date: 11/19/2009
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

//To use Irrlicht, you must define _IRR_STATIC_LIB_

//If you want to use Irrlicht's GUI stuff, you must define _IRR_COMPILE_WITH_GUI_

#ifndef IrrlichtManager_h__
#define IrrlichtManager_h__

#include "PlatformSetup.h"
#include "irrlicht.h"
#ifdef RT_IRRBULLET
#include "irrBullet/irrbullet.h"
class irrBulletWorld;
#endif
#include "util/MathUtils.h"

//helpers for conversions to and from different vector types (they all have the same data format)

#define ToIrrVector3(a) (*(irr::core::vector3df*)(&(a)))
#define ToCLVector3(a) (*(CL_Vec3f*)(&(a)))
#define ToCLVector2(a) (*(CL_Vec2f*)(&(a)))

using namespace irr;

class IrrlichtManager
{
public:
	IrrlichtManager();
	virtual ~IrrlichtManager();

	bool Init(irr::IEventReceiver *pEventReceiver);
	void Kill();

	bool IsRunning();
	void BeginScene();
	void Render();
	void EndScene();
	void ClearScene();

	irr::video::ITexture * GetTexture(const std::string &fileName);
	irr::IrrlichtDevice * GetDevice() {return m_pDevice;}
	irr::video::IVideoDriver* GetDriver() {return m_pDriver;}
	irr::scene::ISceneManager* GetScene() {return m_pScene;}

	void Render2D();
	bool GetLightingEnabled() {return m_bLightingEnabled;}
	void SetLightingEnabled(bool bNew) {m_bLightingEnabled = bNew;}

	bool GetDebugEnabled() {return m_bDebugEnabled;}
	void SetDebugEnabled(bool bNew) {m_bDebugEnabled = bNew;}
	void SetBulletPhysicsEnabled(bool bNew) {m_bBulletPhysicsEnabled = bNew;} //only applicable if RT_IRRBULLET was defined
	void OnUnloadSurfaces();
	void OnLoadSurfaces();

#ifdef RT_IRRBULLET
	irrBulletWorld * GetBulletWorld() {return m_pWorld;}
#endif

private:

	irr::IrrlichtDevice * m_pDevice;
	irr::video::IVideoDriver* m_pDriver;
	irr::scene::ISceneManager* m_pScene;

#ifdef RT_IRRBULLET
	irrBulletWorld *m_pWorld;
#endif

	bool m_bLightingEnabled;
	bool m_bDebugEnabled;
	bool m_bPhysicsEnabled;
	bool m_bBulletPhysicsEnabled;

};

IrrlichtManager * GetIrrlichtManager();
core::rect<s32> CLRectToIrrlichtRect32(CL_Rectf clR);
core::vector3df GetVectorHeadingFromNode(scene::ISceneNode *pNode);
core::vector3df RotatePositionByDirectionalVector(core::vector3df vPos, core::vector3df vNormal );
float Vector3DToAngleRadians(core::vector3df v);
std::string PrintVector3(core::vector3df v);
#endif // IrrlichtManager_h__