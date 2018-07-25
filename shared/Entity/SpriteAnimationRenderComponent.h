//  ***************************************************************
//  SpriteAnimationRenderComponent - Creation date: 04/04/2012
//  -------------------------------------------------------------
//
//  ***************************************************************
//  Programmer(s):  Aki Koskinen
//  ***************************************************************

#ifndef SpriteAnimationRenderComponent_h__
#define SpriteAnimationRenderComponent_h__

#include "Component.h"

class SpriteSheetSurface;
class SpriteAnimation;
class SpriteAnimationSet;

/**
 * A component that renders sprite animations.
 *
 * The name of the component is initially set to "SpriteAnimationRender".
 *
 * The following named variants are used from the parent \c Entity:
 * - <b>"pos2d" (Vector2):</b> specifies the position. You can move this component by changing this.
 * - <b>"size2d" (Vector2):</b> stores the size of the drawn surface. This is used internally,
 *   don't set this. Use "scale2d" instead.
 * - <b>"scale2d" (Vector2):</b> specifies the scale for the drawn surface. Default is (1.0, 1.0).
 * - <b>"rotation" (float):</b> specifies the rotation of the drawn surface. The value is in degrees.
 * - <b>"color" (uint32), "colorMod" (uint32), "alpha" (float):</b> these values are passed to
 *   \c ColorCombine() function and the result specifies a color tint for the drawn surface.
 *   See \c SurfaceAnim::Blit() for specifics how the tinting works. The defaults are pure opaque
 *   white for "color" and "colorMod" and 1.0 for "alpha". The defaults are such that the color
 *   tinting has no effect on the resulting image.
 * - <b>"visible" (uint32):</b> sets whether the surface is visible or not. 0 means not visible,
 *   all other values mean visible. The default value is visible.
 *
 * The following named variants are used inside the component itself:
 * - <b>"flipX" (uint32), "flipY" (uint32):</b> sets if the drawn frame should be flipped horizontally
 *   or vertically. 0 means no flipping, all other values enable the flipping. The defaults are 0.
 * - <b>"fileName" (string):</b> specifies the name of the sprite animation file that is used.
 *   See \c ResourceManager::GetSpriteAnimationSet().
 * - <b>"animationName" (string):</b> specifies the name of the animation this component should draw.
 * - <b>"phase" (float):</b> specifies the phase or progress of the animation. This must be a value
 *   between 0.0 and 1.0, inclusive. When "phase" is 0.0 the first frame of the animation is drawn.
 *   When "phase" is 1.0 the last frame of the animation is drawn. The values between 0.0 and 1.0
 *   map evenly to the frame numbers of the currently selected animation. The animation can be played
 *   back by progressively changing the value of "phase" (with \c InterpolateComponent for example).
 * - <b>"frameSize2d" (Vector2):</b> a read only variant that contains the unscaled frame size of the
 *   current animation. If there is no current animation then this value is (0.0, 0.0). See
 *   \c SpriteAnimation::GetBoundingBox().
 */
class SpriteAnimationRenderComponent: public EntityComponent
{
public:
	SpriteAnimationRenderComponent();
	virtual ~SpriteAnimationRenderComponent();

	virtual void OnAdd(Entity *pEnt);
	
private:
	void UpdateSizeVars();
	
	void OnRender(VariantList *pVList);
	void OnFileNameChanged(Variant *pFileName);
	void OnScaleChanged(Variant *pDataObject);

	void OnAnimationNameChanged(Variant *pAnimationName);

	CL_Vec2f *m_pPos2d;
	CL_Vec2f *m_pSize2d;
	CL_Vec2f *m_pScale2d;
	CL_Vec2f *m_pFrameSize;
	float *m_pRotation;
	uint32 *m_pColor;
	uint32 *m_pColorMod;
	float *m_pAlpha;
	SpriteAnimationSet *m_pSpriteAnimationSet;
	SpriteSheetSurface *m_pSpriteSheet;
	string *m_pFileName;
	string *m_pAnimationName;
	float *m_pPhase;
	uint32 *m_pVisible;
	uint32 *m_pFlipX, *m_pFlipY;

	const SpriteAnimation *m_pCurrentAnimation;
	CL_Rectf m_pCurrentAnimBB;
	CL_Vec2f m_pCurrentAnimBBCenter;
};

#endif // SpriteAnimationRenderComponent_h__
