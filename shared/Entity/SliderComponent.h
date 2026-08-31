//  ***************************************************************
//  SliderComponent - Creation date: 8/25/2010
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2010 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef SliderComponent_h__
#define SliderComponent_h__

#include "Entity/Component.h"
#include "Entity/Entity.h"

/*
A slider control.  Add it to the TRACK entity, whose pos2d/size2d are the
line the knob travels along (see CreateSlider in EntityUtils.cpp for the
usual setup).  The knob is a child entity found through the parent's
"sliderButton" var (or a child named "sliderButton"); the component only ever
moves its pos2d, so any render component works on it.  Clicking or dragging
anywhere on the track (padded 12 px across it) moves the knob there.

Component vars:
  "progress" (float, 0..1): the knob's position along the track.  Setting it
    from code moves the knob; connect to its GetSigOnChanged() to hear drags.
  "vertical" (uint32, default 0): 1 makes the knob travel along size2d.y
    (progress 0 = top) instead of size2d.x; the knob's other coordinate is
    left alone, so set it to center the knob on the track.
*/

class SliderComponent: public EntityComponent
{
public:
	SliderComponent();
	virtual ~SliderComponent();

	virtual void OnAdd(Entity *pEnt);
	virtual void OnRemove();
	void SetSliderPosition();
	void SetSliderPosition(float value);

private:

	void OnRender(VariantList *pVList);
	void OnUpdate(VariantList *pVList);
	void OnProgressChanged(Variant *pDataObject);
	Entity * GetSliderButton();

	void SetPositionWithMouseClick(CL_Vec2f pt);
	void OnInput( VariantList *pVList );
	void UpdatePositionByTouch(CL_Vec2f pt);
	float GetTrackLength();
	void SetKnobAlongTrack(float along); //moves the knob and updates progress

	CL_Vec2f *m_pPos2d;
	CL_Vec2f *m_pSize2d;
	float *m_pAlpha;
	uint32 *m_pColor;
	uint32 *m_pColorMod;
	uint32 *m_pBorderColor;
	float *m_pProgress;
	uint32 *m_pVertical;
	Entity * m_pSliderButton;
	CL_Vec2f m_pClickStartPos;
	bool	m_sliderButtonSelected;
	
};

#endif // SliderComponent_h__