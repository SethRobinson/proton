//  ***************************************************************
//  OverlayRenderComponent - Creation date: 04/11/2009
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef OverlayRenderComponent_h__
#define OverlayRenderComponent_h__

#include "Component.h"
#include "Entity.h"
#include "Renderer/SurfaceAnim.h"

/**
 * A component that renders a \c SurfaceAnim.
 * 
 * The name of the component is initially set to "OverlayRender".
 * 
 * The following named variants are used from the parent \c Entity:
 * - <b>"pos2d" (Vector2):</b> specifies the position. You can move this component by changing this.
 * - <b>"size2d" (Vector2):</b> stores the size of the drawn surface. This is used internally,
 *   don't set this. Use "scale2d" instead.
 * - <b>"scale2d" (Vector2):</b> specifies the scale for the drawn surface. Default is (1.0, 1.0).
 * - <b>"rotation" (float):</b> specifies the rotation of the drawn surface. The value is in degrees.
 * - <b>"rotationCenter" (Vector2):</b> specifies the point around which the surface is rotated.
 *   The value is a relative measure of the size of the surface. (0, 0) means the top-left corner
 *   and (1, 1) means the bottom-right corner of the surface. Default is (0.5, 0.5) which is the
 *   center of the surface.
 * - <b>"color" (uint32), "colorMod" (uint32), "alpha" (float):</b> these values are passed to
 *   \c ColorCombine() function and the result specifies a color tint for the drawn surface.
 *   See \c SurfaceAnim::Blit() for specifics how the tinting works. The defaults are pure opaque
 *   white for "color" and "colorMod" and 1.0 for "alpha". The defaults are such that the color
 *   tinting has no effect on the resulting image.
 * - <b>"visible" (uint32):</b> sets whether the surface is visible or not. 0 means not visible,
 *   all other values mean visible. The default value is visible.
 * 
 * The following named variants are used inside the component itself:
 * - <b>"frameX" (uint32), "frameY" (uint32):</b> sets the drawn frame for the \c SurfaceAnim.
 * - <b>"flipX" (uint32), "flipY" (uint32):</b> sets if the drawn frame should be flipped horizontally
 *   or vertically. 0 means no flipping, all other values enable the flipping. The defaults are 0.
 * - <b>"fileName" (string):</b> specifies the name of the image to be loaded into the \c SurfaceAnim.
 * - <b>"frameSize2d" (Vector2):</b> a read only variant that contains the frame size of the
 *   rendered \c SurfaceAnim. If there is no current \c SurfaceAnim then this value is (0.0, 0.0).
 */
class OverlayRenderComponent: public EntityComponent
{
public:
	OverlayRenderComponent();
	virtual ~OverlayRenderComponent();

	virtual void OnAdd(Entity *pEnt);
	virtual void OnRemove();
	
	//normally you would never access this component directly through class functions but for this specific thing I needed it as it bypassed
	//the texture caching system completely

	void SetSurface(SurfaceAnim *pSurf, bool bDeleteSurface);
	SurfaceAnim * GetSurfaceAnim() { return m_pTex; } //normally you wouldn't use this, but useful to check if something didn't properly load

private:

	void UpdateFrameSizeVar();
	void UpdateSizeVar();
	
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
	string *m_pFileName;
	uint32 *m_pFrameX, *m_pFrameY;
	float *m_pRotation; //in degrees
	CL_Vec2f *m_pRotationCenter;
	bool m_bDeleteSurface;
	uint32 *m_pVisible;
	uint32 *m_pFlipX, *m_pFlipY;

};

#endif // OverlayRenderComponent_h__
