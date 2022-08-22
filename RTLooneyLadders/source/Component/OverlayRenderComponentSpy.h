
#ifndef OverlayRenderComponentSpy_h__
#define OverlayRenderComponentSpy_h__

//Well, I wanted to use OverlayRender but I don't know how to elegantly add in the building scale and position to the final
//render.. er...  so I justcopied OverlayRenderComponent.cpp and added them in this guys render.  Should figure out a 
//smarter way... could do a render filter to adjust the position, but scale.. uh.. hrm.

#include "Entity/Component.h"
#include "Entity/Entity.h"
#include "Renderer/SurfaceAnim.h"
#include "BuildingComponent.h"

class OverlayRenderComponentSpy: public EntityComponent
{
public:
	OverlayRenderComponentSpy();
	virtual ~OverlayRenderComponentSpy();

	virtual void OnAdd(Entity *pEnt);
	virtual void OnRemove();
	
	//normally you would never access this component directly through class functions but for this specific thing I needed it as it bypassed
	//the texture caching system completely

	void SetSurface(SurfaceAnim *pSurf, bool bDeleteSurface);

private:

	void OnRender(VariantList *pVList);
	void OnFileNameChanged(Variant *pDataObject);
	void OnScaleChanged(Variant *pDataObject);
	void SetupAnim(VariantList *pVList);

	CL_Vec2f *m_pPos2d;
	CL_Vec2f *m_pSize2d;
	CL_Vec2f *m_pScale2d;
	uint32 *m_pColor;
	uint32 *m_pColorMod;
	float *m_pAlpha;
	SurfaceAnim *m_pTex;
	uint32 *m_pAlignment;
	string *m_pFileName;
	uint32 *m_pFrameX, *m_pFrameY;
	float *m_pRotation; //in degrees
	bool m_bDeleteSurface;

	uint32 *m_pFlipX, *m_pFlipY;
};

#endif // OverlayRenderComponentSpy_h__