#include "PlatformPrecomp.h"
 
#include "EntityUtils.h"

Entity * CreateOverlayEntity(Entity *pParentEnt, string name, string fileName, float x, float y, bool bAddBasePath)
{
	Entity *pEnt = NULL;
	
	if (pParentEnt)
	{
		pEnt = pParentEnt->AddEntity(new Entity(name));
	} else
	{
		pEnt = new Entity(name);
	}
	
	if (!pEnt)
	{
		LogError("Failed creating entity");
		return NULL;
	}
	EntityComponent *pComp = pEnt->AddComponent(new OverlayRenderComponent());
	if (!bAddBasePath)
	{
		pComp->GetVar("dontAddBasePath")->Set((uint32)!bAddBasePath);
	}
	pComp->GetVar("fileName")->Set(fileName); //local to component
	pEnt->GetVar("pos2d")->Set(x,y);  //shared with whole entity

	return pEnt;
}

Entity * CreateOverlayButtonEntity(Entity *pParentEnt, string name, string fileName, float x, float y)
{
	
	Entity *pButtonEnt = CreateOverlayEntity(pParentEnt, name, fileName, x, y);
	pButtonEnt->AddComponent(new TouchHandlerComponent);
	pButtonEnt->AddComponent(new Button2DComponent);
	return pButtonEnt;
}

/**
 * Sets up an \c InterpolateComponent in an \c Entity.
 *
 * \param componentName if not empty tries to find a pre-existing \c InterpolateComponent
 * with the given name from the \c Entity. If no such component is found, a new \c InterpolateComponent
 * is created to the \c Entity and its name is set to this value. If this argument is an empty
 * string it is ignored.
 */
EntityComponent* SetupInterpolateComponent(Entity *pEnt, const string& componentName, const string& varName, const Variant& targetValue, int durationMS, int delayBeforeStartMS, eInterpolateType interpolationType = INTERPOLATE_SMOOTHSTEP, InterpolateComponent::eOnFinish onFinish = InterpolateComponent::ON_FINISH_DIE, eTimingSystem timing = GetTiming())
{

	if (!pEnt->GetShared()->GetVarIfExists(varName))
	{
		//no point, var doesn't exist
		return NULL;
	}

	EntityComponent *pComp = NULL;

	if (!componentName.empty())
	{
		pComp = pEnt->GetComponentByName(componentName);
	}

	if (!pComp)
	{
		//doesn't exist, create one
		pComp = pEnt->AddComponent(new InterpolateComponent);
		if (!componentName.empty()) {
			pComp->SetName(componentName);
		}
	}

#ifdef _DEBUG
	
	//too picky I guess
	/*
	if (!pComp->GetParent()->GetShared()->GetVarIfExists(varName))
	{

		LogMsg("Warning: SetupInterpolateComponent doesn't see a variable named %s in entity %s", varName.c_str(),
			pComp->GetParent()->GetName().c_str());
		assert(!"See log output for warning");
	}
	*/
#endif

	pComp->GetVar("var_name")->Set(varName);
	pComp->GetVar("timingSystem")->Set(uint32(timing));
	pComp->GetVar("target")->Set(const_cast<Variant&>(targetValue));
	pComp->GetVar("interpolation")->Set(uint32(interpolationType));
	pComp->GetVar("on_finish")->Set(uint32(onFinish));

	if (delayBeforeStartMS == 0)
	{
		pComp->GetVar("duration_ms")->Set(uint32(durationMS));
	} else
	{
		//trigger it to start later
		GetMessageManager()->SetComponentVariable(pComp, delayBeforeStartMS, "duration_ms", Variant(uint32(durationMS)), timing);
	}

	return pComp;
}

EntityComponent * SlideScreen(Entity *pEnt, bool bIn, int speedMS, int delayToStartMS)
{
	
	CL_Vec2f vEndPos;
	CL_Vec2f vOrigPos = pEnt->GetVar("pos2d")->GetVector2();

	if (bIn)
	{
		//move it off screen to start
		pEnt->GetVar("pos2d")->Set(CL_Vec2f( float(-GetScreenSizeX()+vOrigPos.x), vOrigPos.y));
		vEndPos = CL_Vec2f(vOrigPos.x,vOrigPos.y);
	} else
	{
		vEndPos = CL_Vec2f(GetScreenSizeXf(),vOrigPos.y);
	}

	return SetupInterpolateComponent(pEnt, "", "pos2d", vEndPos, speedMS, delayToStartMS);
}


EntityComponent * SlideScreenBackwards(Entity *pEnt, bool bIn, int speedMS, int delayToStartMS)
{

	CL_Vec2f vEndPos;
	CL_Vec2f vOrigPos = pEnt->GetVar("pos2d")->GetVector2();

	if (bIn)
	{
		//move it off screen to start
		pEnt->GetVar("pos2d")->Set(CL_Vec2f( float(GetScreenSizeX()+vOrigPos.x), vOrigPos.y));
		vEndPos = CL_Vec2f(vOrigPos.x,vOrigPos.y);
	} else
	{
		vEndPos = CL_Vec2f(-GetScreenSizeXf(),vOrigPos.y);
	}

	return SetupInterpolateComponent(pEnt, "", "pos2d", vEndPos, speedMS, delayToStartMS);
}

EntityComponent * SlideScreenVertical(Entity *pEnt, bool bIn, int speedMS, int delayToStartMS)
{
	CL_Vec2f vEndPos;
	CL_Vec2f vOrigPos = pEnt->GetVar("pos2d")->GetVector2();

	if (bIn)
	{
		//move it off screen to start
		pEnt->GetVar("pos2d")->Set(CL_Vec2f( vOrigPos.x, -GetScreenSizeYf()));
		vEndPos = CL_Vec2f(vOrigPos.x,0);
	} else
	{
		vEndPos = CL_Vec2f(vOrigPos.x, GetScreenSizeYf());
	}

	return SetupInterpolateComponent(pEnt, "", "pos2d", vEndPos, speedMS, delayToStartMS);
}

bool IsEntityBobbing(Entity *pEnt)
{
	if (!pEnt) return false;

	return pEnt->GetComponentByName("ic_bob") != NULL;
}

//bounces for ever
void BobEntity(Entity *pEnt, float bobAmount, int delayBeforeBob, int durationOfEachBobMS)
{
	if (!pEnt) return;

	CL_Vec2f vEndPos = GetPos2DEntity(pEnt);

	vEndPos.y += bobAmount;

	EntityComponent *pComp = SetupInterpolateComponent(pEnt, "ic_bob", "pos2d", vEndPos, durationOfEachBobMS, delayBeforeBob, INTERPOLATE_SMOOTHSTEP, InterpolateComponent::ON_FINISH_BOUNCE);

	//move it back to where it started if we ever stop bobbing
	pComp->GetVar("set_value_on_finish")->Set(GetPos2DEntity(pEnt));
}

void BobEntityStop(Entity *pEnt)
{
	if (!pEnt) return;
	pEnt->RemoveComponentByName("ic_bob");

}

void OneTimeBobEntity(Entity *pEnt, float bobAmount, int delayBeforeBob, int durationMS)
{
	if (pEnt->GetComponentByName("ic_bob"))
	{
		//well, we already have one of these active, don't trigger it again yet until it's dead
	} else
	{
		CL_Vec2f vEndPos = pEnt->GetVar("pos2d")->GetVector2();
		vEndPos.y += bobAmount;

		EntityComponent *pComp = SetupInterpolateComponent(pEnt, "ic_bob", "pos2d", vEndPos, durationMS, delayBeforeBob, INTERPOLATE_SMOOTHSTEP, InterpolateComponent::ON_FINISH_BOUNCE);
		pComp->GetVar("deleteAfterPlayCount")->Set(uint32(2));
	}
}

void AnimateStopEntity(Entity *pEnt, int delayToStartMS)
{
	EntityComponent *pComp = pEnt->GetComponentByName("ic_anim");
	if (pComp)
	{
		if (delayToStartMS == 0)
		{
			pComp->GetVar("duration_ms")->Set(uint32(0));
		} else
		{
			GetMessageManager()->SetComponentVariable(pComp, delayToStartMS, "duration_ms", Variant(uint32(0)));
		}
	}
}

void AnimateStopEntityAndSetFrame(Entity *pEnt, int delayToStartMS, int frameX, int frameY)
{
	EntityComponent *pComp = pEnt->GetComponentByName("ic_anim");
	if (pComp)
	{
		if (delayToStartMS == 0)
		{
			pComp->GetVar("duration_ms")->Set(uint32(0));

		} else
		{
			GetMessageManager()->SetComponentVariable(pComp, delayToStartMS, "duration_ms", Variant(uint32(0)));
		}
	}

	pComp = pEnt->GetComponentByName("OverlayRender");
	if (pComp)
	{
		if (delayToStartMS == 0)
		{
			pComp->GetVar("frameX")->Set(uint32(frameX));
			pComp->GetVar("frameY")->Set(uint32(frameY));
		} else
		{
			GetMessageManager()->SetComponentVariable(pComp, delayToStartMS, "frameX", Variant(uint32(frameX)));
			GetMessageManager()->SetComponentVariable(pComp, delayToStartMS, "frameY", Variant(uint32(frameY)));
		}
	}
}


EntityComponent * SetupAnimEntity(Entity *pEnt, uint32 frameCountX, uint32 frameCountY, int curFrameX, int curFrameY)
{
	EntityComponent *pComp = pEnt->GetComponentByName("OverlayRender");
	
	if (pComp)
	{
        VariantList vList(frameCountX, frameCountY);
		pComp->GetFunction("SetupAnim")->sig_function(&vList);

		if (curFrameX != -1)
		{
			//also set this..
			pComp->GetVar("frameX")->Set(uint32(curFrameX));
		} 
		
		if (curFrameY != -1)
		{
			pComp->GetVar("frameY")->Set(uint32(curFrameY));
		}

		return pComp; //return it just in case they want to do something else with it
	}

	assert("This anim doesn't even have an OverlayRender attached!");
	return NULL;
}

void AnimateEntitySetMirrorMode(Entity *pEnt, bool flipX, bool flipY)
{
	EntityComponent *overlayComp = pEnt->GetComponentByName("OverlayRender");
	assert(overlayComp && "You must add a OverlayRender component to use this");
	if (!overlayComp) return;

	overlayComp->GetVar("flipX")->Set(uint32(flipX));
	overlayComp->GetVar("flipY")->Set(uint32(flipY));

}

void AnimateEntity(Entity *pEnt, int startFrame, int endFrame, int animSpeedMS, InterpolateComponent::eOnFinish type, int delayToStartMS)
{
	EntityComponent *overlayComp = pEnt->GetComponentByName("OverlayRender");
	assert(overlayComp && "You must add a OverlayRender component to use this");
	if (!overlayComp) return;

	int totalFramesX = overlayComp->GetVar("totalFramesX")->GetUINT32();

	string frameName;

	if (totalFramesX > 1)
	{
		frameName = "frameX";
	} else
	{
		frameName = "frameY";
	}

	assert (type != InterpolateComponent::ON_FINISH_DIE && "You should use ON_FINISH_STOP so additional calls won't break stuff");
	EntityComponent *pComp = pEnt->GetComponentByName("ic_anim");
	if (!pComp)
	{
		//doesn't exist, create one
		pComp = pEnt->AddComponent(new InterpolateComponent);
		pComp->SetName("ic_anim");
	}

	uint32 totalTimeMS = animSpeedMS * (endFrame - startFrame + 1);
	
	if (delayToStartMS == 0)
	{
		//set it all now
		pComp->GetVar("component_name")->Set("OverlayRender");
		pComp->GetVar("var_name")->Set(frameName);
		overlayComp->GetVar(frameName)->Set(uint32(startFrame));
		pComp->GetVar("target")->Set(uint32(endFrame + 1));
		pComp->GetVar("set_value_on_finish")->Set(uint32(endFrame));
		pComp->GetVar("interpolation")->Set(uint32(INTERPOLATE_LINEAR));
		pComp->GetVar("on_finish")->Set(uint32(type));
		pComp->GetVar("duration_ms")->Set(uint32(totalTimeMS));
	} else
	{
		//otherwise..schedule it so we can modify an anim on the fly by completely replacing it later
		GetMessageManager()->SetComponentVariable(pComp, delayToStartMS, "component_name", Variant("OverlayRender"));
		GetMessageManager()->SetComponentVariable(pComp, delayToStartMS, "var_name", Variant(frameName));
		GetMessageManager()->SetComponentVariable(overlayComp, delayToStartMS, frameName, Variant(uint32(startFrame)));
		GetMessageManager()->SetComponentVariable(pComp, delayToStartMS, "target", Variant(uint32(endFrame + 1)));
		GetMessageManager()->SetComponentVariable(pComp, delayToStartMS, "set_value_on_finish", Variant(uint32(endFrame)));
		GetMessageManager()->SetComponentVariable(pComp, delayToStartMS, "interpolation", Variant(uint32(INTERPOLATE_LINEAR)));
		GetMessageManager()->SetComponentVariable(pComp, delayToStartMS, "on_finish", Variant(uint32(type)));
		GetMessageManager()->SetComponentVariable(pComp, delayToStartMS, "duration_ms", Variant(uint32(totalTimeMS)));
	}
}

EntityComponent * SetOverlayImageEntity(Entity *pEntWithOverlayComponent, string imageFileName, uint32 delayBeforeActionMS, bool bAddBasePath)
{
	if (!pEntWithOverlayComponent)
	{
		assert(!"SetOverlayImageEntity sent null entity");
		return NULL;
	}
	EntityComponent *pComp = pEntWithOverlayComponent->GetComponentByName("OverlayRender");
	if (!pComp)
	{
		assert(!"Only send entities with OverlayRenderComponents to this!  Ie, stuff created with CreateOverlayRenderEntity or such.");
		return NULL;
	}

	if (delayBeforeActionMS == 0)
	{
		//change it now
		pComp->GetVar("dontAddBasePath")->Set((uint32)!bAddBasePath);
		pComp->GetVar("fileName")->Set(imageFileName);
	} else
	{
		//schedule it to happen later
		GetMessageManager()->SetComponentVariable(pComp, delayBeforeActionMS, "fileName", imageFileName);
	}
	return pComp;
}

std::string GetOverlayImageEntity(Entity *pEntWithOverlayComponent)
{
	EntityComponent *pComp = pEntWithOverlayComponent->GetComponentByName("OverlayRender");
	if (!pComp)
	{
		return "";
	}
	Variant *var = pComp->GetVar("fileName");
	if (!var) {
		return "";
	}
	return var->GetString();
}

void FadeInEntity(Entity *pEnt, bool bRecursive, int timeMS, int delayBeforeFadingMS, float fadeTarget, eTimingSystem timing)
{
	pEnt->GetVar("alpha")->Set(0.0f); //so we can fade in

	SetupInterpolateComponent(pEnt, "", "alpha", fadeTarget, timeMS, delayBeforeFadingMS, INTERPOLATE_SMOOTHSTEP, InterpolateComponent::ON_FINISH_DIE, timing);

	if (!bRecursive) return;

	//also run this on all children
	EntityList *pChildren = pEnt->GetChildren();

	EntityList::iterator itor = pChildren->begin();
	while (itor != pChildren->end())
	{
		FadeInEntity( *itor, bRecursive, timeMS, delayBeforeFadingMS, fadeTarget, timing);
		itor++;
	}
}

EntityComponent * MorphToVec2Entity(Entity *pEnt, string targetVar, CL_Vec2f vTargetSize, unsigned int speedMS, eInterpolateType interpolateType,  int delayBeforeActionMS)
{
	return SetupInterpolateComponent(pEnt, "ic_" + targetVar, targetVar, vTargetSize, speedMS, delayBeforeActionMS, interpolateType);
}


EntityComponent * MorphToVec2EntityMulti(Entity *pEnt, string targetVar, CL_Vec2f vTargetSize, unsigned int speedMS, eInterpolateType interpolateType,  int delayBeforeActionMS)
{
	EntityComponent* pComp = SetupInterpolateComponent(pEnt, "", targetVar, vTargetSize, speedMS, delayBeforeActionMS, interpolateType);
	pComp->SetName("ic_"+targetVar+"_multi");

	return pComp;
}

EntityComponent * MorphToFloatEntity(Entity *pEnt, string targetVar, float target, unsigned int speedMS, eInterpolateType interpolateType,  int delayBeforeActionMS)
{
	return SetupInterpolateComponent(pEnt, "ic_" + targetVar, targetVar, target, speedMS, delayBeforeActionMS, interpolateType);
}

EntityComponent * MorphToSizeEntity(Entity *pEnt, CL_Vec2f vTargetSize, unsigned int speedMS, eInterpolateType interpolateType,  int delayBeforeActionMS)
{
	return MorphToVec2Entity(pEnt, "size2d", vTargetSize, speedMS, interpolateType, delayBeforeActionMS);
}


EntityComponent * MorphToFloatComponent(EntityComponent *pTargetComp, string targetVar, float target, unsigned int speedMS, eInterpolateType interpolateType,  int delayBeforeActionMS)
{
	EntityComponent* pComp = MorphToFloatEntity(pTargetComp->GetParent(), targetVar, target, speedMS, interpolateType, delayBeforeActionMS);
	pComp->GetVar("component_name")->Set(pTargetComp->GetName());
	return pComp;
}


EntityComponent * MorphToVec2Component(EntityComponent *pTargetComp, string targetVar, CL_Vec2f vTargetSize, unsigned int speedMS, eInterpolateType interpolateType,  int delayBeforeActionMS)
{
	EntityComponent* pComp = MorphToVec2Entity(pTargetComp->GetParent(), targetVar, vTargetSize, speedMS, interpolateType, delayBeforeActionMS);
	pComp->GetVar("component_name")->Set(pTargetComp->GetName());
	return pComp;
}

EntityComponent * ZoomToPositionFromThisOffsetEntity(Entity *pEnt, CL_Vec2f offset, unsigned int speedMS, eInterpolateType interpolateType,  int delayBeforeActionMS)
{
	CL_Vec2f vEndPos;

	vEndPos = pEnt->GetVar("pos2d")->GetVector2();
	pEnt->GetVar("pos2d")->Set(vEndPos+offset);

	return SetupInterpolateComponent(pEnt, "ic_pos", "pos2d", vEndPos, speedMS, delayBeforeActionMS, interpolateType);
}

EntityComponent * ZoomFromPositionEntity(Entity *pEnt, CL_Vec2f vPos, unsigned int speedMS, eInterpolateType interpolateType,  int delayBeforeActionMS)
{
	CL_Vec2f vEndPos = pEnt->GetVar("pos2d")->GetVector2();
	pEnt->GetVar("pos2d")->Set(vPos);
	
	return SetupInterpolateComponent(pEnt, "ic_pos", "pos2d", vEndPos, speedMS, delayBeforeActionMS, interpolateType);
}

EntityComponent * ZoomToPositionEntity(Entity *pEnt, CL_Vec2f vPos, unsigned int speedMS, eInterpolateType interpolateType, int delayBeforeActionMS)
{
	return MorphToVec2Entity(pEnt, "pos2d", vPos, speedMS, interpolateType, delayBeforeActionMS);
}

EntityComponent * ZoomToPositionOffsetEntity(Entity *pEnt, CL_Vec2f offset, unsigned int speedMS, eInterpolateType interpolateType, int delayBeforeActionMS)
{
	return MorphToVec2Entity(pEnt, "pos2d", pEnt->GetVar("pos2d")->GetVector2()+offset, speedMS, interpolateType, delayBeforeActionMS);
}

EntityComponent * ZoomToPositionOffsetEntityMulti(Entity *pEnt, CL_Vec2f offset, unsigned int speedMS, eInterpolateType interpolateType, int delayBeforeActionMS)
{
	return MorphToVec2EntityMulti(pEnt, "pos2d", pEnt->GetVar("pos2d")->GetVector2()+offset, speedMS, interpolateType, delayBeforeActionMS);
}

EntityComponent * ZoomToPositionEntityMulti(Entity *pEnt, CL_Vec2f vPos, unsigned int speedMS, eInterpolateType interpolateType, int delayBeforeActionMS)
{
	EntityComponent* pComp = SetupInterpolateComponent(pEnt, "", "pos2d", vPos, speedMS, delayBeforeActionMS, interpolateType);
	pComp->SetName("ic_pos_multi");
	return pComp;
}

EntityComponent * ZoomToScaleEntity(Entity *pEnt, CL_Vec2f vScale, unsigned int speedMS, eInterpolateType interpolateType, int delayBeforeActionMS)
{
	return MorphToVec2Entity(pEnt, "scale2d", vScale, speedMS, interpolateType, delayBeforeActionMS);
}

void MorphToColorEntity(Entity *pEnt, bool bRecursive, int timeMS, unsigned int color, int delayBeforeActionMS, bool bAllowMultipleAtOnce)
{
	EntityComponent* pComp = SetupInterpolateComponent(pEnt, bAllowMultipleAtOnce ? "" : "ic_color", "color", uint32(color), timeMS, delayBeforeActionMS, INTERPOLATE_SMOOTHSTEP_AS_COLOR);
	pComp->SetName("ic_color");

	if (!bRecursive) return;

	//also run this on all children
	EntityList *pChildren = pEnt->GetChildren();

	EntityList::iterator itor = pChildren->begin();
	while (itor != pChildren->end())
	{
		MorphToColorEntity( *itor, bRecursive, timeMS, color, delayBeforeActionMS);
		itor++;
	}
}


EntityComponent * PulsateColorEntity(Entity *pEnt, bool bRecursive, unsigned int color, unsigned int pulsateSpeedMS)
{
	EntityComponent* pComp = SetupInterpolateComponent(pEnt, "ColorModIC", "colorMod", uint32(color), pulsateSpeedMS, 0, INTERPOLATE_SMOOTHSTEP_AS_COLOR, InterpolateComponent::ON_FINISH_BOUNCE);

	if (!bRecursive) return pComp;

	//also run this on all children
	EntityList *pChildren = pEnt->GetChildren();

	EntityList::iterator itor = pChildren->begin();
	while (itor != pChildren->end())
	{
		PulsateColorEntity( *itor, bRecursive, color, pulsateSpeedMS);
		itor++;
	}

	return pComp;
}

bool IsDisabledEntity(Entity *pEnt)
{
	EntityComponent * pComp = pEnt->GetComponentByName("Button2D");
	if (!pComp) pComp = pEnt->GetComponentByName("TouchDrag");

	if (pComp)
	{
		return pComp->GetVar("disabled")->GetUINT32() != 0;
	}

	return false;
}

Entity * DisableEntityButtonByName(const string &entityName, Entity *pRootEntity)
{
	Entity *pEnt = pRootEntity->GetEntityByName(entityName);

	if (!pEnt) return NULL;

	EntityComponent * pComp = pEnt->GetComponentByName("Button2D");
	if (!pComp) pComp = pEnt->GetComponentByName("TouchDrag");

	if (pComp)
	{
		pComp->GetVar("disabled")->Set(uint32(1));
	}

	//also, check for a specific type of touch handlers that should also be disabled, so it doesn't mark taps as owned
	pComp = pEnt->GetComponentByName("TouchHandlerArcade");
	if (pComp)
	{
		pComp->GetVar("disabled")->Set(uint32(1));
	}

	pComp = pEnt->GetComponentByName("EmitVirtualKey");
	if (pComp)
	{
		pComp->GetVar("disabled")->Set(uint32(1));
	}

	
	return pEnt;

}

void DisableAllButtonsEntity(Entity *pEnt, bool bRecursive)
{
	EntityComponent * pComp = pEnt->GetComponentByName("Button2D");
	if (!pComp) pComp = pEnt->GetComponentByName("TouchDrag");
	if (!pComp) pComp = pEnt->GetComponentByName("EmitVirtualKey");

	if (pComp)
	{
		pComp->GetVar("disabled")->Set(uint32(1));
	}

	//also, check for a specific type of touch handler that should also be disabled, so it doesn't mark taps as owned
	pComp = pEnt->GetComponentByName("TouchHandlerArcade");
	if (pComp)
	{
		pComp->GetVar("disabled")->Set(uint32(1));
	}

	pComp = pEnt->GetComponentByName("EmitVirtualKey");
	if (pComp)
	{
		pComp->GetVar("disabled")->Set(uint32(1));
	}

	if (!bRecursive) return;

	//also run this on all children
	EntityList *pChildren = pEnt->GetChildren();

	EntityList::iterator itor = pChildren->begin();
	while (itor != pChildren->end())
	{
		DisableAllButtonsEntity( *itor, bRecursive);
		itor++;
	}
}

Entity * EnableEntityButtonByName(const string &entityName, Entity *pRootEntity)
{
	Entity *pEnt = pRootEntity->GetEntityByName(entityName);

	if (!pEnt) return NULL;

	EntityComponent * pComp = pEnt->GetComponentByName("Button2D");
	if (!pComp) pComp = pEnt->GetComponentByName("TouchDrag");

	if (pComp)
	{
		pComp->GetVar("disabled")->Set(uint32(0));
	}

	//also, check for a specific type of touch handler that should also be disabled, so it doesn't mark taps as owned
	pComp = pEnt->GetComponentByName("TouchHandlerArcade");
	if (pComp)
	{
		pComp->GetVar("disabled")->Set(uint32(0));
	}
	return pEnt;

}
void EnableAllButtonsEntity(Entity *pEnt, bool bRecursive, int delayBeforeActionMS, eTimingSystem timing)
{
	EntityComponent * pComp = pEnt->GetComponentByName("Button2D");
	if (!pComp) pComp = pEnt->GetComponentByName("TouchDrag");
	if (!pComp) pComp = pEnt->GetComponentByName("EmitVirtualKey");
	
	if (pComp)
	{
		if (delayBeforeActionMS == 0)
		{
			//do it now
			pComp->GetVar("disabled")->Set(uint32(0));
		} else
		{
			//schedule it instead
			GetMessageManager()->SetComponentVariable(pComp, delayBeforeActionMS,"disabled",uint32(0), timing);
		}
	}

	//also, check for a specific type of touch handler that should also be disabled, so it doesn't mark taps as owned
	pComp = pEnt->GetComponentByName("TouchHandlerArcade");
	if (pComp)
	{
		if (delayBeforeActionMS == 0)
		{
			//do it now
			pComp->GetVar("disabled")->Set(uint32(0));
		} else
		{
			//schedule it instead
			GetMessageManager()->SetComponentVariable(pComp, delayBeforeActionMS,"disabled",uint32(0), timing);
		}
	}

	if (!bRecursive) return;

	//also run this on all children
	EntityList *pChildren = pEnt->GetChildren();

	EntityList::iterator itor = pChildren->begin();
	while (itor != pChildren->end())
	{
		EnableAllButtonsEntity( *itor, bRecursive, delayBeforeActionMS, timing);
		itor++;
	}
}


void FlashStopEntity(Entity *pEnt)
{
	pEnt->RemoveComponentByName("ic_flash");
}

void FlashStartEntity(Entity *pEnt, int flashSpeedMS)
{
	FlashStopEntity(pEnt);

	EntityComponent* pComp = SetupInterpolateComponent(pEnt, "ic_flash", "alpha", 1.0f, flashSpeedMS / 2, 0, INTERPOLATE_SMOOTHSTEP, InterpolateComponent::ON_FINISH_BOUNCE);
	pComp->GetVar("set_value_on_finish")->Set(pEnt->GetVar("alpha")->GetFloat()); //grab a copy of the current alpha, to restore it later
}

void FlashOnceEntity(Entity *pEnt, int flashSpeedMS)
{
	pEnt->RemoveComponentByName("ic_flash");

	EntityComponent* pComp = SetupInterpolateComponent(pEnt, "ic_flash", "alpha", 1.0f, flashSpeedMS / 2, 0, INTERPOLATE_SMOOTHSTEP, InterpolateComponent::ON_FINISH_BOUNCE);
	pComp->GetVar("set_value_on_finish")->Set(pEnt->GetVar("alpha")->GetFloat()); //grab a copy of the current alpha, to restore it later
	pComp->GetVar("deleteAfterPlayCount")->Set(uint32(2));
}

void FadeEntity(Entity *pEnt, bool bRecursive, float alpha, int timeMS, int delayBeforeFadingMS,  bool bAllowMultipleFadesActiveAtOnce)
{
	if (!bAllowMultipleFadesActiveAtOnce)
	{
		while (pEnt->RemoveComponentByName("ic_fade"));
	}

	EntityComponent* pComp = SetupInterpolateComponent(pEnt, "", "alpha", alpha, timeMS, delayBeforeFadingMS);
	if (!pComp)
	{
		//entity doesn't have an "alpha" variable, so it probably isn't something we can actually fade!  However, it's children may have something...
		
	}
	else
	{
		pComp->SetName("ic_fade");
	}
	
	if (!bRecursive) return;

	//also run this on all children
	EntityList *pChildren = pEnt->GetChildren();

	EntityList::iterator itor = pChildren->begin();
	while (itor != pChildren->end())
	{
		FadeEntity( *itor, bRecursive, alpha, timeMS, delayBeforeFadingMS);
		itor++;
	}
}

void FadeOutEntity(Entity *pEnt, bool bRecursive, int timeMS, int delayBeforeFadingMS, eTimingSystem timing)
{
	SetupInterpolateComponent(pEnt, "", "alpha", 0.0f, timeMS, delayBeforeFadingMS, INTERPOLATE_SMOOTHSTEP, InterpolateComponent::ON_FINISH_DIE, timing);

	if (!bRecursive) return;

	//also run this on all children
	EntityList *pChildren = pEnt->GetChildren();

	EntityList::iterator itor = pChildren->begin();
	while (itor != pChildren->end())
	{
		FadeOutEntity( *itor, bRecursive, timeMS, delayBeforeFadingMS, timing);
		itor++;
	}
}

void FadeOutAndKillEntity(Entity *pEnt, bool bRecursive, int timeMS, int delayBeforeFadingMS, eTimingSystem timing)
{
	if (!pEnt) return;
	FadeOutEntity(pEnt, bRecursive, timeMS, delayBeforeFadingMS, timing);
	KillEntity(pEnt, timeMS+delayBeforeFadingMS, timing);
}

void FadeOutAndKillChildrenEntities(Entity *pEnt, int timeMS, int delayBeforeFadingMS, eTimingSystem timing)
{
	EntityList *pChildren = pEnt->GetChildren();

	EntityList::iterator itor = pChildren->begin();
	while (itor != pChildren->end())
	{
		FadeOutAndKillEntity(*itor, true, timeMS, delayBeforeFadingMS, timing);
		itor++;
	}
}


void ScaleEntity(Entity *pEnt, float scaleStart, float scaleEnd, int timeMS, int delayBeforeStartingMS, eInterpolateType interpolationType)
{
	ScaleEntity(pEnt, CL_Vec2f(scaleStart, scaleStart), CL_Vec2f(scaleEnd, scaleEnd), timeMS, delayBeforeStartingMS, interpolationType);
}

void ScaleEntity(Entity *pEnt, CL_Vec2f vScaleStart, CL_Vec2f vScaleEnd, int timeMS, int delayBeforeStartingMS, eInterpolateType interpolationType)
{
	pEnt->RemoveComponentByName("ic_scale");

	if (vScaleStart != CL_Vec2f(-1,-1))
	{
		pEnt->GetVar("scale2d")->Set(vScaleStart);
	}

	EntityComponent * pComp = SetupInterpolateComponent(pEnt, "", "scale2d", vScaleEnd, timeMS, delayBeforeStartingMS, interpolationType);
	pComp->SetName("ic_scale");
}

void KillEntity(Entity *pEnt, int timeMS, eTimingSystem timingSystem)
{
	if (!pEnt) return;

	if (timeMS == 0)
	{
		pEnt->SetTaggedForDeletion();
	} else
	{
        VariantList vList(pEnt);
		GetMessageManager()->CallEntityFunction(pEnt, timeMS, "OnDelete", &vList, timingSystem);
	}
}

Entity * CreateTextLabelEntity(Entity *pParentEnt, string name, float x, float y, string text)
{
		
	Entity *pButtonEnt = NULL;

	if (pParentEnt)
	{
		pButtonEnt = pParentEnt->AddEntity(new Entity(name));
	} else
	{
		pButtonEnt = new Entity(name);
	}

	EntityComponent *pComp  = pButtonEnt->AddComponent(new TextRenderComponent());
	pComp->GetVar("text")->Set(text); //local to component
	pButtonEnt->GetVar("pos2d")->Set(x, y);
	return pButtonEnt;
}


Entity * CreateInputTextEntity(Entity *pParentEnt, string name, float x, float y, string text, float sizeX, float sizeY)
{
	
	Entity *pButtonEnt = NULL;

	if (pParentEnt)
	{
		pButtonEnt = pParentEnt->AddEntity(new Entity(name));
	} else
	{
		pButtonEnt = new Entity(name);
	}

	EntityComponent *pComp  = pButtonEnt->AddComponent(new InputTextRenderComponent());
	pButtonEnt->AddComponent( new TouchHandlerComponent);
	pComp->GetVar("text")->Set(text); //local to component
	pButtonEnt->GetVar("pos2d")->Set(x, y);
	
	float fontHeight = GetBaseApp()->GetFont(FONT_SMALL)->GetLineHeight(1.0f);
	if (sizeX == 0) sizeX = fontHeight*10;
	if (sizeY == 0) sizeY = fontHeight+6;

	pButtonEnt->GetVar("size2d")->Set(sizeX, sizeY);
	
	return pButtonEnt;
}

Entity * CreateOverlayRectEntity(Entity *pParent, CL_Rectf posAndBoundsRect, uint32 color, RectRenderComponent::eVisualStyle style)
{
	Entity *pEnt;
	
	if (pParent)
	{
		pEnt = pParent->AddEntity(new Entity);
	} else
	{
		pEnt = new Entity;
	}
	EntityComponent *pComp = pEnt->AddComponent(new RectRenderComponent);
	pEnt->GetVar("pos2d")->Set(posAndBoundsRect.get_top_left());
	pEnt->GetVar("size2d")->Set(posAndBoundsRect.get_width(), posAndBoundsRect.get_height());
	pEnt->GetVar("color")->Set(color);
	if (style != RectRenderComponent::STYLE_NORMAL)
	{
		pComp->GetVar("visualStyle")->Set(uint32(style));
	}
	return pEnt;
}

Entity * CreateOverlayRectEntity(Entity *pParent, CL_Vec2f vPos, CL_Vec2f vBounds, uint32 color, RectRenderComponent::eVisualStyle style)
{
	return CreateOverlayRectEntity(pParent, CL_Rectf(vPos, *((CL_Sizef*)&vBounds) ), color, style);
}


Entity * CreateButtonHotspot(Entity *pParentEnt, string name, CL_Vec2f vPos, CL_Vec2f vBounds, Button2DComponent::eButtonStyle buttonStyle)
{
	Entity *pButtonEnt = CreateOverlayRectEntity(pParentEnt, vPos, vBounds, MAKE_RGBA(0,0,0,100));
	pButtonEnt->SetName(name);
	pButtonEnt->AddComponent(new TouchHandlerComponent);
	pButtonEnt->GetVar("touchPadding")->Set(CL_Rectf(0,0,0,0));

	EntityComponent *pButtonComp = pButtonEnt->AddComponent(new Button2DComponent);
	//pButtonEnt->GetVar("alpha")->Set(0.0f);
	//don't shot the hotspot rect
	pButtonComp->GetVar("buttonStyle")->Set(uint32(buttonStyle));
	pButtonComp->GetVar("visualStyle")->Set(uint32(Button2DComponent::STYLE_INVISIBLE_UNTIL_CLICKED));
	return pButtonEnt;
}

void SetButtonRepeatDelayMS(Entity *pEnt, uint32 delayMS)
{
	EntityComponent *pBut = pEnt->GetComponentByName("Button2D");
	if (pBut)
	{
		pBut->GetVar("repeatDelayMS")->Set(delayMS);
	} else
	{
		assert(!"Entity doesn't have a button component, so why call this?");
	}
}

void SetButtonClickSound(Entity *pEnt, string fileName)
{
	EntityComponent *pBut = pEnt->GetComponentByName("Button2D");
	if (pBut)
	{
		pBut->GetVar("onClickAudioFile")->Set(fileName);
	} else
	{
		assert(!"Entity doesn't have a button component, so why call this?");
	}
}

Entity * CreateTextButtonEntity(Entity *pParentEnt, string name, float x, float y, string text, bool bUnderline)
{
	Entity *pButtonEnt = CreateTextLabelEntity(pParentEnt, name, x, y, text);
	pButtonEnt->AddComponent(new TouchHandlerComponent);
	pButtonEnt->AddComponent(new Button2DComponent);
	if (bUnderline)
	{
		pButtonEnt->AddComponent(new UnderlineRenderComponent);
	}

	return pButtonEnt;
}


void RemoveFocusIfNeeded(Entity *pEnt)
{
	pEnt->RemoveComponentByName("FocusInput");
	pEnt->RemoveComponentByName("FocusRender");
	pEnt->RemoveComponentByName("FocusUpdate");
}

void RemoveInputFocusIfNeeded(Entity *pEnt)
{
	pEnt->RemoveComponentByName("FocusInput");
}

bool EntityHasInputFocus(Entity *pEnt)
{
	return pEnt->GetComponentByName("FocusInput", true) != NULL;
}

void AddFocusIfNeeded(Entity *pEnt, bool bAlsoLinkMoveMessages, int delayInputMS, int updateAndRenderDelay)
{
	EntityComponent *pComp = pEnt->GetComponentByName("FocusUpdate", true);
	
	if (!pComp)
	{
		if (updateAndRenderDelay == 0)
		{
			pEnt->AddComponent(new FocusUpdateComponent);
		} else
		{
			//schedule it
			GetMessageManager()->AddComponent(pEnt, updateAndRenderDelay, new FocusUpdateComponent);

		}
	}

	if (!pEnt->GetComponentByName("FocusRender", true))
	{
		if (updateAndRenderDelay == 0)
		{
			pEnt->AddComponent(new FocusRenderComponent);
		} else
		{
			//schedule it
			GetMessageManager()->AddComponent(pEnt, updateAndRenderDelay, new FocusRenderComponent);

		}
	}

	if (!pEnt->GetComponentByName("FocusInput", true))
	{
		if (delayInputMS == 0)
		{
			FocusInputComponent *pComp = (FocusInputComponent*)pEnt->AddComponent(new FocusInputComponent);
		
			if (bAlsoLinkMoveMessages)
			{
				pComp->GetFunction("LinkMoveMessages")->sig_function(NULL);
			}
		} else
		{
			//add the input focus, but wait a bit before doing it
			GetMessageManager()->AddComponent(pEnt, delayInputMS, new FocusInputComponent);
			//call a function on a component that doesn't exist yet, but will be added in 500 ms
			if (bAlsoLinkMoveMessages)
			{
				GetMessageManager()->CallComponentFunction(pEnt, "FocusInput", delayInputMS, "LinkMoveMessages");
			}

		}
	}
}

//adds input focus, but ONLY for touch movement, nothing else.  Useful if you're already receiving the other inputs by trickle down from
//another focus.

void AddInputMovementFocusIfNeeded(Entity *pEnt)
{
	if (!pEnt->GetComponentByName("FocusInput", false))
	{
			FocusInputComponent *pComp = new FocusInputComponent;
			
			//tell it not to wire anything
			pComp->GetVar("mode")->Set(uint32(FocusInputComponent::MODE_START_NONE));
			pEnt->AddComponent(pComp);
			pComp->GetFunction("LinkMoveMessages")->sig_function(NULL);
	}
}

void EnableRawMessageFocusInput(Entity *pEnt)
{
	EntityComponent *pInputComp = pEnt->GetComponentByName("FocusInput", true);

	if (pInputComp)
	{
		pInputComp->GetFunction("LinkRawMessages")->sig_function(NULL);
	}
}


Entity * FadeScreen( Entity *pParent, float defaultStartAlpha, float targetAlpha, int fadeDurationMS, bool bDeleteWhenDone )
{
	
	Entity *pEnt = pParent->GetEntityByName("black_overlay");
	
	if (!pEnt)
	{
		pEnt = pParent->AddEntity(new Entity("black_overlay"));
		pEnt->AddComponent(new RectRenderComponent);
		pEnt->GetVar("size2d")->Set(CL_Vec2f(GetScreenSizeXf(), GetScreenSizeYf()));
		pEnt->GetVar("color")->Set(MAKE_RGBA(0,0,0,255));
		pEnt->GetVar("alpha")->Set(defaultStartAlpha);

	}

	FadeEntity(pEnt, false, targetAlpha, fadeDurationMS, 0);

	if (bDeleteWhenDone)
	{
		pEnt->SetName("ic_delete"); //safety measure so if they call FadeScreen again fast this soon to be deleted versuion won't confuse it
		KillEntity(pEnt, fadeDurationMS);
	}

	return pEnt;
}

void FadeScreenUp( Entity *pParent, float targetAlpha, int fadeDurationMS, bool bDeleteWhenDone )
{

	Entity *pEnt = pParent->GetEntityByName("black_overlay");

	if (!pEnt)
	{
		pEnt = pParent->AddEntity(new Entity("black_overlay"));
		pEnt->AddComponent(new RectRenderComponent);
		pEnt->GetVar("size2d")->Set(CL_Vec2f(GetScreenSizeXf(), GetScreenSizeYf()));
		pEnt->GetVar("color")->Set(MAKE_RGBA(0,0,0,255));
		pEnt->GetVar("alpha")->Set(1.0f);
	}

	FadeEntity(pEnt, false, targetAlpha, fadeDurationMS, 0);

	if (bDeleteWhenDone)
	{
		KillEntity(pEnt, fadeDurationMS);
	}
}



CL_Rectf MeasureEntityAndChildren(Entity *pEnt, CL_Vec2f *pVStartingPos,  bool bFirst)
{
	CL_Vec2f vSize = pEnt->GetVar("size2d")->GetVector2();
	CL_Vec2f vPos = pEnt->GetVar("pos2d")->GetVector2();
	
	eAlignment align = eAlignment(pEnt->GetVar("alignment")->GetUINT32());
	if (align != ALIGNMENT_UPPER_LEFT)
	{
		vPos -= GetAlignmentOffset(vSize, align);
	}
	
	CL_Rectf r = CL_Rectf(0,0,vSize.x, vSize.y);
	if (!bFirst)
	r.translate(vPos.x, vPos.y);

	//also run this on all children
	EntityList *pChildren = pEnt->GetChildren();

	EntityList::iterator itor = pChildren->begin();
	while (itor != pChildren->end())
	{
		CL_Rectf childR = MeasureEntityAndChildren(*itor, pVStartingPos, false);

		if (r.get_width() == 0 && r.get_height() == 0)
		{
			r.bounding_rect(childR);
			
			CL_Vec2f vSize = CL_Vec2f(r.get_width(), r.get_height());

			r = childR;
			if (pVStartingPos)
			{
				pVStartingPos->x = vSize.x - r.get_width();
				pVStartingPos->y = vSize.y - r.get_height();

			}
		} else
		{
			r.bounding_rect(childR);
		}
		itor++;
	}

	return r;
}

void SetupTextEntity(Entity *pEnt, eFont fontID, float scale)
{
	EntityComponent *pComp = pEnt->GetComponentByName("TextRender");

	if (pComp)
	{
		//normal TextRender component
		if (scale != 0)
		{
			pEnt->GetVar("scale2d")->Set(CL_Vec2f(scale, scale));
		}
	}
	
	if (!pComp && ( (pComp = pEnt->GetComponentByName("TextBoxRender"))  != 0))
	{
		//we handle font scale a different way, so it doesn't affect the text input box size
		if (scale != 0)
		{
			pComp->GetVar("fontScale")->Set(scale);
		}
	} 

	if (!pComp && ( (pComp = pEnt->GetComponentByName("LogDisplay")) != 0))
	{
		//we handle font scale a different way, so it doesn't affect the text input box size
		if (scale != 0)
		{
			pComp->GetVar("fontScale")->Set(scale);
		}
	} 

	if (!pComp && ( (pComp = pEnt->GetComponentByName("InputTextRender")) != 0))
	{
		if (scale != 0)
		{
			pEnt->GetVar("scale2d")->Set(CL_Vec2f(scale, scale));
		}
	} 

	if (!pComp)
	{
		assert(!"Huh?");
		return;
	}

	pComp->GetVar("font")->Set(uint32(fontID));
}


void SetTextShadowColor(Entity *pEnt, uint32 color)
{
	EntityComponent *pComp = pEnt->GetComponentByName("TextRender");

	if (!pComp && ( (pComp = pEnt->GetComponentByName("TextBoxRender"))  != 0))
	{

	} 

	if (!pComp && ( (pComp = pEnt->GetComponentByName("LogDisplay")) != 0))
	{
	
	} 

	if (!pComp && ( (pComp = pEnt->GetComponentByName("InputTextRender")) != 0))
	{
	
	} 

	if (!pComp)
	{
		assert(!"Huh?");
		return;
	}

	pComp->GetVar("shadowColor")->Set(uint32(color));
}


void GetFontAndScaleToFitThisStringInWidthPixels(eFont *pFontIDOut, float *pFontScaleOut, string text, float desiredWidth)
{
	*pFontIDOut = FONT_SMALL;
	float sizeY = GetBaseApp()->GetFont(*pFontIDOut)->GetLineHeight(1.0f);

	*pFontScaleOut = 1.0f;

	CL_Vec2f vSize = GetBaseApp()->GetFont(*pFontIDOut)->MeasureText(text, *pFontScaleOut);

	*pFontScaleOut = desiredWidth/vSize.x;

	if (*pFontScaleOut <= 1.0f) return;

	*pFontScaleOut = 1.0f;

	//let's use the bigger font instead
	*pFontIDOut = FONT_LARGE;

	vSize = GetBaseApp()->GetFont(*pFontIDOut)->MeasureText(text, *pFontScaleOut);
	*pFontScaleOut = desiredWidth/vSize.x;
}

//Gets a good font and scale so the text will match this ratio of the screen height
void GetFontAndScaleToFitThisLinesPerScreenY(eFont *pFontIDOut, float *pFontScaleOut, float desiredLinesPerScreenY)
{
	*pFontIDOut = FONT_SMALL;
	float sizeY = GetBaseApp()->GetFont(*pFontIDOut)->GetLineHeight(1.0f);

	*pFontScaleOut = (GetScreenSizeYf()/desiredLinesPerScreenY)/sizeY;

	if (*pFontScaleOut <= 1.0f) return;

	//let's use the bigger font instead
	*pFontIDOut = FONT_LARGE;
	sizeY = GetBaseApp()->GetFont(*pFontIDOut)->GetLineHeight(1.0f);
	*pFontScaleOut = (GetScreenSizeYf()/desiredLinesPerScreenY)/sizeY;
}

//Gets a good font and scale so the text will match this height, in pixels
void GetFontAndScaleToFitThisPixelHeight(eFont *pFontIDOut, float *pFontScaleOut, float heightPixels)
{
	*pFontIDOut = FONT_SMALL;
	float sizeY = GetBaseApp()->GetFont(*pFontIDOut)->GetLineHeight(1.0f);

	*pFontScaleOut = heightPixels/sizeY;

	if (*pFontScaleOut <= 1.0f) return;

	//let's use the bigger font instead
	*pFontIDOut = FONT_LARGE;
	sizeY = GetBaseApp()->GetFont(*pFontIDOut)->GetLineHeight(1.0f);
	*pFontScaleOut = heightPixels/sizeY;

}



//if you have a giant font that you are afraid is going to be too big on some phone
//sizes, this is a way to auto-scale it so it works out
//Example: float fontScale = EnforceMinimumFontLineToScreenRatio(FONT_LARGE, 1.0f, 6.6f);

float EnforceMinimumFontLineToScreenRatio(eFont fontID, float fontScale, float minLineToScreenRatio)
{
	float fontSizeRating = GetScreenSizeYf() / GetBaseApp()->GetFont(fontID)->GetLineHeight(fontScale);

	if (fontSizeRating < minLineToScreenRatio)
	{
		//font is too big.
		fontScale =fontSizeRating/minLineToScreenRatio;
	}

	return fontScale;
}

float EnforceMinimumFontLineToScreenRatioAllowBig(eFont fontID, float fontScale, float minLineToScreenRatio)
{
	float sizeY = GetBaseApp()->GetFont(fontID)->GetLineHeight(1.0f);

	return (GetScreenSizeYf()/minLineToScreenRatio)/sizeY;
}


void SetAlignmentEntity(Entity *pEnt, eAlignment align)
{
	pEnt->GetVar("alignment")->Set(uint32(align));
}

eAlignment GetAlignmentEntity(Entity *pEnt)
{
	return eAlignment(pEnt->GetVar("alignment")->GetUINT32());
}

Entity * CreateTextBoxEntity(Entity *pParent, string entName, CL_Vec2f vPos, CL_Vec2f vTextAreaSize, string msg, float scale, eAlignment textAlignment)
{
	Entity *pText = pParent->AddEntity(new Entity(entName));
	
	EntityComponent *pTextComp = pText->AddComponent(new TextBoxRenderComponent);
	pText->GetVar("size2d")->Set(vTextAreaSize);
	pTextComp->GetVar("fontScale")->Set(scale);
	pTextComp->GetVar("text")->Set(msg);
	pTextComp->GetVar("textAlignment")->Set((uint32)textAlignment);
	pText->GetVar("pos2d")->Set(vPos);
	return pText;
}

EntityComponent * SetTextEntity(Entity *pEntWithTextComponent, const string &text)
{
	if (!pEntWithTextComponent)
	{
		assert(!"SetTextEntity sent a null pointer");
		return NULL;
	}
	EntityComponent *pComp = pEntWithTextComponent->GetComponentByName("TextRender");

	if (!pComp)
	{
		pComp = pEntWithTextComponent->GetComponentByName("TextBoxRender");
	}
	
	if (!pComp)
	{
		pComp = pEntWithTextComponent->GetComponentByName("InputTextRender");
	}


	if (!pComp)
	{
		assert(!"SetTextEntity failed - Send an entity with a TextBoxRender or TextRender component for crissake!");
		return NULL;
	}

	pComp->GetVar("text")->Set(text);
	return pComp;
}


void ResizeScrollerAfterTextboxChanges(VariantList *vList)
{
	VariantList v(vList->Get(0).GetEntity()->GetParent()->GetParent());
	ResizeScrollBounds(&v);
}

Entity * CreateScrollingTextBoxEntity(Entity *pParent, string entName, CL_Vec2f vPos, CL_Vec2f vTextAreaSize, string msg, float scale, eAlignment textAlignment)
{

	//first create an entity that will handle the scrollcomponent, and hold an entity that will contain all content that will be scrolled

	CL_Vec2f vTextAreaPos = vPos;
	CL_Vec2f vTextAreaBounds = vTextAreaSize;

	Entity *pScroll = pParent->AddEntity(new Entity("scroll"));
	pScroll->GetVar("pos2d")->Set(vTextAreaPos);
	pScroll->GetVar("size2d")->Set(vTextAreaBounds);
	pScroll->AddComponent(new TouchHandlerComponent);

	EntityComponent *pScrollComp = pScroll->AddComponent(new ScrollComponent);
	pScroll->AddComponent(new FilterInputComponent); //lock out taps that are not in our scroll area
	pScrollComp->GetVar("fingerTracking")->Set(uint32(1));
	EntityComponent *pScrollBarComp = pScroll->AddComponent(new ScrollBarRenderComponent); 	//also let's add a visual way to see the scroller position
	//pScroll->GetVar("color")->Set(MAKE_RGBA(61,155, 193, 255)); 
	Entity *pScrollChild = pScroll->AddEntity(new Entity("scroll_child"));

	pScroll->AddComponent(new RenderScissorComponent()); //so the text/etc won't get drawn outside our scroll box

	//now add the real content to pScrollChild

	Entity *pText = pScrollChild->AddEntity(new Entity(entName));
	vTextAreaSize -=iPhoneMap(2,10); //so the letters don't overlay the scroll bar

	EntityComponent *pTextComp = pText->AddComponent(new TextBoxRenderComponent);
	pText->GetVar("size2d")->Set(vTextAreaSize);
	pTextComp->GetVar("fontScale")->Set(scale);
	pTextComp->GetVar("text")->Set(msg);
	pTextComp->GetVar("textAlignment")->Set((uint32)textAlignment);

	//now let the scroll component know to resize itself in a sec

	pScroll->GetFunction("ResizeScrollBounds")->sig_function.connect(ResizeScrollBounds);

	//TODO: I schedule it to happen the next frame, otherwise it resizes incorrectly.  Figure out why later?
	VariantList vList(pScroll);
	GetMessageManager()->CallEntityFunction(pScroll, 1, "ResizeScrollBounds", &vList);

	//also, let's have it resize itself whenever the text changes
	pText->GetFunction("OnSizeChanged")->sig_function.connect(ResizeScrollerAfterTextboxChanges);
	return pText;
}

void RemovePaddingEntity(Entity *pEnt)
{
	SetTouchPaddingEntity(pEnt, CL_Rectf(0,0,0,0));
}

void SetTouchPaddingEntity( Entity *pEnt, CL_Rectf padding )
{
	pEnt->GetVar("touchPadding")->Set(padding);
}

EntityComponent * SetButtonVisualStyleEntity(Entity *pEnt, Button2DComponent::eVisualStyle style)
{
	EntityComponent *pComp = pEnt->GetComponentByName("Button2D");
	if (!pComp)
	{
		assert(!"Huh?");
		return NULL;
	}

	pComp->GetVar("visualStyle")->Set(uint32(style));
	return pComp;
}


EntityComponent * SetButtonStyleEntity(Entity *pEnt, Button2DComponent::eButtonStyle style)
{
	EntityComponent *pComp = pEnt->GetComponentByName("Button2D");
	if (!pComp)
	{
		assert(!"Huh?");
		return NULL;
	}

	pComp->GetVar("buttonStyle")->Set(uint32(style));
	return pComp;
}

EntityComponent * TypeTextLabelEntity( Entity *pEnt, int delayBeforeActionMS, uint32 textTypeSpeedMS,
									  TyperComponent::eMode mode, string textToAddByTyping)
{
	EntityComponent *pText = pEnt->GetComponentByName("TextRender");

	if (!pText)
	{
		pText = pEnt->GetComponentByName("TextBoxRender");
	}

	if (!pText)
	{
		assert("!This only works on an entity that has a TextRender or TextBoxRender component already!");
		return NULL;
	}

	EntityComponent *pTextTyper = pEnt->GetComponentByName("Typer");
	if (pTextTyper) pEnt->RemoveComponentByAddress(pTextTyper); //kill any existing ones

	pTextTyper = pEnt->AddComponent(new TyperComponent);
	pTextTyper->GetVar("mode")->Set(uint32(mode));
	pTextTyper->GetVar("speedMS")->Set(textTypeSpeedMS);
	string msg = pText->GetVar("text")->GetString();

	if (textToAddByTyping.empty())
	{
		//type the whole existing label, don't add anything to it
		pText->GetVar("text")->Set(""); //clear what was there, it will be added by the text typer
		pTextTyper->GetVar("text")->Set(msg);
	} else
	{
		//only type the new part
		pTextTyper->GetVar("text")->Set(textToAddByTyping);
	}

	if (delayBeforeActionMS != 0)
	{
		pTextTyper->GetVar("paused")->Set(uint32(1));
		GetMessageManager()->SetComponentVariable(pTextTyper, delayBeforeActionMS, "paused", Variant(uint32(0)));
	}
	return pTextTyper; //just in case they want to make more tweaks

}


void SetAlphaEntity( Entity *pEnt, float alpha )
{
	pEnt->GetVar("alpha")->Set(alpha);
}

void PreloadKeyboard(OSMessage::eParmKeyboardType keyboardType)
{
	if (GetEmulatedPlatformID() == PLATFORM_ID_ANDROID) return; //no point on this platform I don't think..

    
	OSMessage o;
	o.m_type = OSMessage::MESSAGE_OPEN_TEXT_BOX;
	o.m_string = "";
	SetLastStringInput("");
	o.m_x = -1000;
	o.m_y = -1000; 
	o.m_parm1 = 0;
	o.m_fontSize = 30.0f;
	o.m_sizeX = 217;
	o.m_sizeY = 40;
	o.m_parm2 = keyboardType;
	GetBaseApp()->AddOSMessage(o);

	o.m_type = OSMessage::MESSAGE_CLOSE_TEXT_BOX;
	GetBaseApp()->AddOSMessage(o);
    
    VariantList v;
    
    v.Get(0).Set((float)MESSAGE_TYPE_HW_KEYBOARD_INPUT_STARTING);
    GetBaseApp()->m_sig_hardware(&v);

    v.Get(0).Set((float)MESSAGE_TYPE_HW_KEYBOARD_INPUT_ENDING);
    GetBaseApp()->m_sig_hardware(&v);
    
}


void SendFakeInputMessageToEntity(Entity *pEnt, eMessageType msg, CL_Vec2f vClickPos, int delayBeforeStartingMS)
{
	
	VariantList v;
	v.Get(0).Set((float)msg);
	v.Get(1).Set(vClickPos);
	v.Get(2).Set(uint32(C_MAX_TOUCHES_AT_ONCE-1));

	if (delayBeforeStartingMS == 0)
	{
		//fake out our touch tracker, will override the 11th finger touch..
		GetBaseApp()->GetTouch(C_MAX_TOUCHES_AT_ONCE-1)->SetWasHandled(false);
		GetBaseApp()->GetTouch(C_MAX_TOUCHES_AT_ONCE-1)->SetIsDown(true);
		GetBaseApp()->GetTouch(C_MAX_TOUCHES_AT_ONCE-1)->SetPos(vClickPos);

		//do it now
		pEnt->CallFunctionRecursively("OnInput", &v);	
	} else
	{
		//schedule it to happen later
		GetMessageManager()->CallEntityFunction(pEnt, delayBeforeStartingMS, "OnInput", &v);
		
	}
	
}

void LightBarOnChange(VariantList *pVList)
{
	Entity *pEnt = pVList->Get(1).GetEntity();
	//LogMsg("Clicked %s", pEnt->GetName().c_str());
	string barName = pEnt->GetName().substr(0, pEnt->GetName().rfind("_")) + "_lightbar";
	//LogMsg("Bar name is %s", barName.c_str());

	Entity *pLightBar = pEnt->GetParent()->GetEntityByName(barName);
	if (!pLightBar)
	{
		assert(0);
		return;
	}

	CL_Vec2f vOffset(4,2);
	ZoomToPositionEntity(pLightBar, pEnt->GetVar("pos2d")->GetVector2()-vOffset, 300);
	MorphToSizeEntity(pLightBar, pEnt->GetVar("size2d")->GetVector2()+(vOffset*2), 300);	
}

void SendFakeButtonPushToEntity(Entity *pEntity, int timeMS)
{
	VariantList vList(CL_Vec2f(pEntity->GetVar("pos2d")->GetVector2()), pEntity);
	GetMessageManager()->CallEntityFunction(pEntity, timeMS, "OnButtonSelected", &vList);
}
void SetupLightBarSelect(Entity *pBG, string entNamePrefix, int defaultOption, uint32 color)
{
	//count how many buttons we can find

	int buttonCount = 0;
	Entity *pEnt = NULL;
	Entity *pHighlighted = NULL;

	for (int i=0;;i++)
	{
		pEnt = pBG->GetEntityByName(entNamePrefix+toString(i));

		if (pEnt)
		{
			if (defaultOption == buttonCount)
			{
				pHighlighted = pEnt;
			}

			SetButtonStyleEntity(pEnt, Button2DComponent::BUTTON_STYLE_CLICK_ON_TOUCH);
			//map buttons so we know when it is clicked
			pEnt->GetFunction("OnButtonSelected")->sig_function.connect(&LightBarOnChange);

			buttonCount++;
		} else
		{
			break;
		}
	}

	//LogMsg("Found %d buttons.", buttonCount);

	pEnt = CreateOverlayRectEntity(pBG, CL_Vec2f(0,0), CL_Vec2f(30, 10), color);
	pEnt->SetName(entNamePrefix+"lightbar");

	assert(pHighlighted && "Need to specify the default");

	if  (pHighlighted)
	{
		CL_Vec2f vOffset(4,2);
		pEnt->GetVar("pos2d")->Set(pHighlighted->GetVar("pos2d")->GetVector2()-vOffset);
		pEnt->GetVar("size2d")->Set(pHighlighted->GetVar("size2d")->GetVector2()+(vOffset*2));
	}

	pBG->MoveEntityToBottomByAddress(pEnt); //let this draw first
}

//assumes you've setup a scroll window the way I always do... see examples.
void ResizeScrollBounds(VariantList *pVList)
{
	Entity *pScroll = pVList->Get(0).GetEntity()->GetEntityByName("scroll");
	
	if (!pScroll)
	{
		LogError("This is sort of hardcoded to need the entity sent in to have an entity named scroll..");
		assert(!"No entity named scroll!");
		return;
	}
	Entity *pScrollChild = pScroll->GetEntityByName("scroll_child");
	if (!pScroll || !pScrollChild)
	{
		LogError("huh");
		return;
	}

	CL_Vec2f scrollSize = pScroll->GetVar("size2d")->GetVector2();

	CL_Rectf r = MeasureEntityAndChildren(pScrollChild);
	//	LogMsg("Resizing bounds to %s", PrintRect(r).c_str());
	CL_Rectf scrollRect = CL_Rectf( rt_min(0, (-r.get_width())+scrollSize.x), (-r.get_height())+scrollSize.y, 0, 0);

	pScroll->GetComponentByName("Scroll")->GetVar("boundsRect")->Set(scrollRect);
}

void DisableHorizontalScrolling(Entity *pEnt)
{
	Entity *pScroll = pEnt->GetEntityByName("scroll");
    if (!pScroll)
	{
		assert(!"This only works for an entity holding a ScrollComponent!");
		return;
	}

	pEnt->GetComponentByName("Scroll")->GetVar("boundsRect")->GetRect().left = 0;

}

void SetScrollProgressEntity(Entity *pEnt, const CL_Vec2f &progress)
{
	EntityComponent *pComp = pEnt->GetComponentByName("Scroll");
	if (!pComp)
	{
		assert(!"This only works for an entity holding a ScrollComponent!");
		return;
	}

	VariantList scrollProgress(progress);
	pComp->GetFunction("SetProgress")->sig_function(&scrollProgress);
}


CL_Vec2f GetScrollProgressEntity(Entity *pEnt)
{
	EntityComponent *pComp = pEnt->GetComponentByName("Scroll");
	if (!pComp)
	{
		assert(!"This only works for an entity holding a ScrollComponent!");
		return CL_Vec2f(0,0);
	}

	return pComp->GetVar("progress2d")->GetVector2();
}


void SetDisabledOnAllComponentsRecursively(Entity *pEnt, bool bDisabled)
{

	//disable all components in us
	ComponentList *pComponents = pEnt->GetComponents();

	ComponentList::iterator compItor = pComponents->begin();
	while (compItor != pComponents->end())
	{
		(*compItor)->GetVar("disabled")->Set((uint32)bDisabled);
		compItor++;
	}

	//also run this on all children
	EntityList *pChildren = pEnt->GetChildren();

	EntityList::iterator itor = pChildren->begin();
	while (itor != pChildren->end())
	{
		SetDisabledOnAllComponentsRecursively(*itor, bDisabled);
		itor++;
	}

}

EntityComponent * DisableComponentByName(Entity *pEnt, const string &compName, int delayBeforeActionMS)
{
	EntityComponent *pComp = pEnt->GetComponentByName(compName);
	if (!pComp)
	{
		assert(!"Unable to find component");
		return NULL;
	}

	if (delayBeforeActionMS != 0)
	{
		GetMessageManager()->SetComponentVariable(pComp, delayBeforeActionMS, "disabled", Variant(uint32(1)));
	} else
	{
		//set it now
		pComp->GetVar("disabled")->Set(uint32(1));
	}

	return pComp;
}

EntityComponent * EnableComponentByName(Entity *pEnt, const string &compName, int delayBeforeActionMS)
{
	EntityComponent *pComp = pEnt->GetComponentByName(compName);
	if (!pComp)
	{
		assert("!Unable to find component");
		return NULL;
	}

	if (delayBeforeActionMS != 0)
	{
		GetMessageManager()->SetComponentVariable(pComp, delayBeforeActionMS, "disabled", Variant(uint32(0)));
	} else
	{
		//set it now
		pComp->GetVar("disabled")->Set(uint32(0));
	}

	return pComp;
}

CL_Vec2f ConvertEntityClickToScreenCoords(CL_Vec2f pt, Entity *pEnt)
{
	//remove any offsets/adjustments and figure out the true screen coordinates of a click

	eAlignment align = eAlignment(pEnt->GetVar("alignment")->GetUINT32());
	if (align != ALIGNMENT_UPPER_LEFT)
	{
		pt -= GetAlignmentOffset(pEnt->GetVar("size2d")->GetVector2(), align);
	}

	//also add parents positions?
	//assert(pEnt->GetParent()->GetVar("pos2d")->GetVector2() == CL_Vec2f(0,0) && "Shouldn't you take this into account too?");

	return pt;
}

void GetUsedTextures(vector<string> &usedTextures, Entity *pEnt)
{

	//check components for anything we recognize that would use textures

	ComponentList *pComponents = pEnt->GetComponents();

	ComponentList::iterator compItor = pComponents->begin();
	while (compItor != pComponents->end())
	{
		EntityComponent *pComp = *compItor;

		if (pComp->GetName() == "OverlayRender")
		{
			string fName = pComp->GetVar("fileName")->GetString();

			if (!fName.empty()) usedTextures.push_back(fName);
		} else
		if (pComp->GetName() == "ScrollBarRender")
		{
			string fName = pComp->GetVar("fileName")->GetString();

			if (!fName.empty()) usedTextures.push_back(fName);
		}

		compItor++;
	}

	//also run this on all children
	EntityList *pChildren = pEnt->GetChildren();

	EntityList::iterator itor = pChildren->begin();
	while (itor != pChildren->end())
	{
		GetUsedTextures( usedTextures, *itor);
		itor++;
	}
}


void DestroyUnusedTextures()
{
	//first, make a list of everything that is used

	vector<string> usedTextures;

	GetUsedTextures(usedTextures, GetEntityRoot());
#ifdef _DEBUG
	LogMsg("Destroying unused textures");
#endif

	/*
	for (int i=0; i < usedTextures.size(); i++)
	{
		LogMsg("%d - %s", i, usedTextures[i].c_str());
	}
	*/

	GetBaseApp()->GetResourceManager()->RemoveTexturesNotInExclusionList(usedTextures);
}

bool EntityRetinaRemapIfNeeded(Entity *pEnt, bool bAdjustPosition, bool bAdjustScale,  bool bApplyToIpadAlso, bool bPerserveAspectRatio)
{
	/*
	LogMsg("Screen size is %s", PrintVector2(GetScreenSize()).c_str());
	
	if (IsIphone4Size) LogMsg("Is iphone4 sized");
	if (IsIPADSize) LogMsg("Is ipad sized");
	if (IsPixiSize) LogMsg("is pixi sized");
	*/

	if (!IsIphone4Size && !(bApplyToIpadAlso && IsIPADSize) && !IsPixiSize) return false;
	
	if (bAdjustPosition)
	{
		CL_Vec2f vPos = iPhoneMap(pEnt->GetVar("pos2d")->GetVector2());
		pEnt->GetVar("pos2d")->Set(vPos);
	}

	if (bAdjustScale)
	{
		CL_Vec2f vScale;

		if (!IsPixiSize)
		{
			vScale = pEnt->GetVar("scale2d")->GetVector2() * 2.0f;
		} else
		{
			if (bPerserveAspectRatio)
			{
				float scale = rt_min(GetScreenSizeXf()/480, GetScreenSizeYf()/320);
				vScale = CL_Vec2f(scale,scale);

			} else
			{
				vScale = CL_Vec2f(GetScreenSizeXf()/480, GetScreenSizeYf()/320);
			}
		}
		pEnt->GetVar("scale2d")->Set(vScale);

	}
	return true;
}

void EntityAdjustScaleSoPhysicalSizeMatches(Entity *pEnt, int ppiToMatch, float powerMult)
{
	
	int pixelsPerInch = GetDevicePixelsPerInchDiagonal();
	if (pixelsPerInch == ppiToMatch) return; //no change needed

	CL_Vec2f vScale = GetScale2DEntity(pEnt);

	float scaleMod = float(pixelsPerInch) / float(ppiToMatch);
	
	CL_Vec2f vOldScale = vScale;

	vScale.x = scaleMod*vScale.x;
	vScale.y = scaleMod*vScale.y;

	//weaken the effect if needed
	vScale = vOldScale + ((vScale-vOldScale)*powerMult);

	SetScale2DEntity(pEnt, vScale);
}

void EntitySetScaleBySize(Entity *pEnt, CL_Vec2f vDestSize, bool bPreserveAspectRatio, bool bPreserveOtherAxis)
{
	CL_Vec2f vOriginalScale = GetScale2DEntity(pEnt);

	CL_Vec2f vSize = pEnt->GetVar("size2d")->GetVector2();
	assert(vSize.x != 0 && vSize.y != 0);

	if (vSize.x == 0 || vSize.y == 0)
	{
		assert(!"Huh?");
		return; //avoid divide by 0
	}

	if (bPreserveAspectRatio)
	{
		float aspectRatio = vSize.x /vSize.y;
		
		//this is strange, but it's legacy, don't change

		if (bPreserveOtherAxis)
		{		//knock the X setting out and replace with aspect correct size
			if (aspectRatio >  1.0)	
			{
				//actually, lets do it the other way
				vDestSize.y = vDestSize.x * (1/aspectRatio);
			} else
			{
				vDestSize.x = vDestSize.y * aspectRatio;
			}

		} else
		{
			//knock the X setting out and replace with aspect correct size
			if (aspectRatio <  1.0)	
			{
				//actually, lets do it the other way
				vDestSize.y = vDestSize.x * (1/aspectRatio);
			} else
			{
				vDestSize.x = vDestSize.y * aspectRatio;
			}
		}

	}

	CL_Vec2f vFinalScale = CL_Vec2f( vDestSize.x / vSize.x, vDestSize.y / vSize.y);
	vFinalScale.x *= vOriginalScale.x;
	vFinalScale.y *= vOriginalScale.y;

	pEnt->GetVar("scale2d")->Set(vFinalScale);
}

void EntitySetScaleBySizeAndAspectMode(Entity *pEnt, CL_Vec2f vDestSize, eAspect aspectMode)
{
	CL_Vec2f vOriginalScale = GetScale2DEntity(pEnt);

	CL_Vec2f vSize = pEnt->GetVar("size2d")->GetVector2();
	assert(vSize.x != 0 && vSize.y != 0);

	if (vSize.x == 0 || vSize.y == 0)
	{
		assert(!"Huh?");
		return; //avoid divide by 0
	}
	float aspectRatio = vSize.x / vSize.y;

	switch (aspectMode)
	{
	case ASPECT_NONE:
	break;

	case ASPECT_FIT:
	{

		if (aspectRatio > 1.0f) //x is bigger, we go by that
		{
			float destYTemp = vDestSize.x * (1 / aspectRatio); //make Y match the aspect ratio
				
				if (destYTemp > vDestSize.y)
				{
					//well, it's too big.  Go reverse
					vDestSize.x = vDestSize.y * aspectRatio;
					//doesn't fit.  
				}
				else
				{
					vDestSize.y = destYTemp;
				}
		}
		else
		{
			float destXTemp = vDestSize.y * aspectRatio;

			if (destXTemp > vDestSize.x)
			{
				//well, it's too big.  Go reverse
				vDestSize.y = vDestSize.x * (1 / aspectRatio);
				//doesn't fit.  
			}
			else
			{
				vDestSize.x = destXTemp;
			}
		}

	} 
	break;

	default:
	
		if (aspectMode == ASPECT_WIDTH_CONTROLS_HEIGHT)
		{		//knock the Y setting out and replace with aspect correct size
				vDestSize.y = vDestSize.x * (1 / aspectRatio);
		}
		else
		{
				vDestSize.x = vDestSize.y * aspectRatio;
		}

	}

	CL_Vec2f vFinalScale = CL_Vec2f(vDestSize.x / vSize.x, vDestSize.y / vSize.y);
	vFinalScale.x *= vOriginalScale.x;
	vFinalScale.y *= vOriginalScale.y;

	pEnt->GetVar("scale2d")->Set(vFinalScale);
}

//On an ipad sized device it does nothing, on anything else it resizes the entity to match the device size
void EntityScaleiPad(Entity *pEnt, bool bPerserveAspectRatio)
{
	CL_Vec2f vSize = pEnt->GetVar("size2d")->GetVector2();
	assert(vSize.x != 0 && vSize.y != 0);

	if (vSize.x == 0 || vSize.y == 0)
	{
		assert(!"Huh?");
		return; //avoid divide by 0
	}
	
	CL_Vec2f vDestSize = vSize;

	CL_Vec2f vScale = pEnt->GetVar("scale2d")->GetVector2();

	//remove scaling from size temporarily
	vSize.x /= vScale.x;
	vSize.y /= vScale.y;

	//Note:  I think the X/Y has to be reverses if the orientation is portrait... todo... - SAR
	
		if (bPerserveAspectRatio)
		{
			float scale = rt_min(GetScreenSizeXf()/1024, GetScreenSizeYf()/768);
			vDestSize.x *= scale;
			vDestSize.y *= scale;

		} else
		{
			vDestSize.x *= GetScreenSizeXf()/1024;
			vDestSize.y *= GetScreenSizeYf()/768;
		}
	
	pEnt->GetVar("scale2d")->Set(CL_Vec2f( vDestSize.x / vSize.x, vDestSize.y / vSize.y));
}

EntityComponent * AddHotKeyToButton(Entity *pEnt, uint32 keycode)
{
	if (!pEnt)
	{
		assert(!"Serious error");
		return NULL;
	}
	EntityComponent *pComp = pEnt->AddComponent(new SelectButtonWithCustomInputComponent);
	pComp->GetVar("keycode")->Set(keycode);
	return pComp;
}

EntityComponent * MakeButtonEmitVirtualGameKey(Entity *pEnt, uint32 keycode)
{
	if (!pEnt)
	{
		assert(!"Serious error");
		return NULL;
	}
	EntityComponent *pComp = pEnt->AddComponent(new EmitVirtualKeyComponent);
	pComp->GetVar("keycode")->Set(keycode);
	return pComp;
}

EntityComponent * CreateSlider(Entity *pBG, float x, float y, float sizeX, string buttonFileName, string left, string middle, string right)
{
	//first the BG
	CreateOverlayRectEntity(pBG, CL_Vec2f(x, y), CL_Vec2f(sizeX,3), MAKE_RGBA(255,255,255,255));

	//the text descriptions

	float textY = y- (GetBaseApp()->GetFont(FONT_SMALL)->GetLineHeight(1)+iPhoneMapY2X(15));

	CreateTextLabelEntity(pBG, "txt", x, textY, left);

	Entity *pText = CreateTextLabelEntity(pBG, "txt", x+sizeX/2, textY, middle);
	SetAlignmentEntity(pText, ALIGNMENT_UPPER_CENTER);

	pText = CreateTextLabelEntity(pBG, "txt", x+sizeX, textY, right);
	SetAlignmentEntity(pText, ALIGNMENT_UPPER_RIGHT);

	//then the slider control
	Entity *pSliderEnt = pBG->AddEntity(new Entity("SliderEnt"));
	EntityComponent * pSliderComp = pSliderEnt->AddComponent(new SliderComponent);

	//the button we move around to slide
	Entity *pSliderButton = CreateOverlayButtonEntity(pSliderEnt, "sliderButton",  buttonFileName, 0, 0);

	CL_Vec2f vButtonScale = CL_Vec2f(0.7f, 0.7f);

	if (IsLargeScreen())
	{
		vButtonScale = CL_Vec2f(1, 1);
	}
	
	SetButtonStyleEntity(pSliderButton, Button2DComponent::BUTTON_STYLE_CLICK_ON_TOUCH);
	SetButtonVisualStyleEntity(pSliderButton, Button2DComponent::STYLE_NONE); //sliders are not really buttons, and the
	//default "alpha when hovering until clicked" effect doesn't look right, so we shut it off

	pSliderButton->GetVar("scale2d")->Set(vButtonScale);
	pSliderButton->GetComponentByName("Button2D")->GetVar("onClickAudioFile")->Set("");

	CL_Vec2f vImageSize = pSliderButton->GetVar("size2d")->GetVector2();
	pSliderEnt->GetVar("pos2d")->Set(CL_Vec2f(x+(vImageSize.x/2)*0.5f, y));
	pSliderEnt->GetVar("size2d")->Set(CL_Vec2f(sizeX- (vImageSize.x*0.5f),0));

	//SetTouchPaddingEntity(pSliderButton, CL_Rectf(0,0,0,0));
	SetAlignmentEntity(pSliderButton, ALIGNMENT_CENTER);
	pSliderComp->GetVar("sliderButton")->Set(pSliderButton);
	return pSliderComp;
}


void AdjustGUIElementForWindowView(Entity *pEnt, CL_Rectf r, float rotation)
{
	CL_Vec2f vPos = RotateGUIPoint( pEnt->GetVar("pos2d")->GetVector2(), r, rotation);
	pEnt->GetVar("pos2d")->Set(vPos);
}

void ManuallySetAlignmentEntity(Entity *pEnt, eAlignment alignment)
{
	float rotation = pEnt->GetVar("rotation")->GetFloat();

	CL_Vec2f pt = pEnt->GetVar("pos2d")->GetVector2();
	CL_Vec2f vSize = pEnt->GetVar("size2d")->GetVector2();
	CL_Vec2f vOffset = GetAlignmentOffset(vSize, alignment);

	float modx,mody;
	RotationToXYMod(rotation, &modx, &mody);

	if (modx) vOffset.x *= -1;
	if (mody) vOffset.y *= -1;

	if (rotation == 90 || rotation == 270)
	{
		swap(vOffset.x, vOffset.y);
	}

	pt -= vOffset;
	pEnt->GetVar("pos2d")->Set(pt);
}

bool FakeClickAnEntityByName(Entity *pEntitySearchRoot, const string name)
{
	Entity *pEnt = pEntitySearchRoot->GetEntityByName(name);
	if (pEnt)
	{
		FakeClickAnEntity(pEnt);
		return true;
	}

	return false; //couldn't find it
}

void FakeClickAnEntity(Entity *pEnt, int delayBeforeStartingMS)
{
	CL_Vec2f vClickPos = pEnt->GetVar("pos2d")->GetVector2();

	SendFakeInputMessageToEntity(pEnt, MESSAGE_TYPE_GUI_CLICK_START, vClickPos, delayBeforeStartingMS);
	SendFakeInputMessageToEntity(pEnt, MESSAGE_TYPE_GUI_CLICK_END, vClickPos, delayBeforeStartingMS);
}

bool IsCheckboxChecked(Entity *pEnt)
{
	if (!pEnt)
	{
		assert(!"No entity");
		return false;
	}

	return pEnt->GetVar("checked")->GetUINT32() != 0;
}

void SetCheckBoxChecked(Entity *pEnt, bool bChecked, bool bShowAnim)
{
	OneTimeBobEntity(pEnt);
	if (!bChecked)
	{
		if(pEnt->GetParent() && pEnt->GetParent()->GetVar("min_checks"))
		{
			uint32 minCheck=pEnt->GetParent()->GetVar("min_checks")->GetUINT32();
			if(minCheck>0)
			{	// don't let them check more than maxChecks items
				EntityList *kids=pEnt->GetParent()->GetChildren();
				uint32 checked=0;
				EntityList::iterator itor=kids->begin();
				while(itor!=kids->end())
				{
					if (StringFromStartMatches((*itor)->GetName(), "input_checkicon|") ||
						StringFromStartMatches((*itor)->GetName(), "input_checkbox|"))
					{
						if((*itor)->GetVar("checked")->GetUINT32()!=0)
							checked++;
					}
					itor++;
				}
				if(checked<=minCheck)
					return;	// bobbed already, but don't actually uncheck it
			}
		}
		//uncheck it
		pEnt->GetVar("checked")->Set(uint32(0));
		//the image too
		if (StringFromStartMatches(pEnt->GetName(), "input_checkicon|"))
			pEnt->GetVar("alpha")->Set(0.3f);
		else
			AnimateStopEntityAndSetFrame(pEnt, 0, 0, 0);
	} 
	else
	{
		if(pEnt->GetParent() && pEnt->GetParent()->GetVar("max_checks"))
		{
			uint32 maxCheck=pEnt->GetParent()->GetVar("max_checks")->GetUINT32();
			if(maxCheck>0)
			{	// don't let them check more than maxChecks items
				EntityList *kids=pEnt->GetParent()->GetChildren();
				uint32 checked=0;
				EntityList::iterator itor=kids->begin();
				while(itor!=kids->end())
				{
					if (StringFromStartMatches((*itor)->GetName(), "input_checkicon|") ||
						StringFromStartMatches((*itor)->GetName(), "input_checkbox|"))
					{
						if((*itor)->GetVar("checked")->GetUINT32()!=0)
							checked++;
					}
					itor++;
				}
				if(checked>=maxCheck)
					return;	// bobbed already, but don't actually check it
			}
		}
		//check it
		pEnt->GetVar("checked")->Set(uint32(1));
		//the image too
		if (StringFromStartMatches(pEnt->GetName(), "input_checkicon|"))
			pEnt->GetVar("alpha")->Set(1.0f);
		else
			AnimateStopEntityAndSetFrame(pEnt, 0, 1, 0);
	}
}

void OnCheckboxToggle(VariantList *pVList)
{
	Entity *pEntClicked = pVList->m_variant[1].GetEntity();
	SetCheckBoxChecked(pEntClicked, !IsCheckboxChecked(pEntClicked));
}

Entity * CreateCheckbox(Entity *pBG, string name, string text, float x, float y, bool bChecked, eFont fontID, float fontScale,bool unclickable)
{
	//harcoded a checkbox image here, it's 2 frames, horizontally.  First frame is unchecked

	SurfaceAnim *pAnim = GetResourceManager()->GetSurfaceAnim("interface/checkbox.rttex");

	if (pAnim)
	{
		pAnim->SetSmoothing(false);
	} else
	{
		LogError("Um, the couldn't be loaded.  Rebuild resources?  Filename is hardcoded to be interface/checkbox.rttex right now.");
	}

	Entity *pEnt;
	if(!unclickable)
		pEnt = CreateOverlayButtonEntity(pBG, name, "interface/checkbox.rttex", x, y);
	else
		pEnt = CreateOverlayEntity(pBG, name, "interface/checkbox.rttex", x, y);
	SetupAnimEntity(pEnt, 2); //let it know its two frames, will default to showing the first one
	pEnt->GetVar("checked")->Set(uint32(bChecked)); //default unchecked
	RemovePaddingEntity(pEnt); //have to click exactly in the image to do anything
	SetTouchPaddingEntity(pEnt, CL_Rectf(5, 5, 5, 5));

	float letterHeight = GetBaseApp()->GetFont(fontID)->MeasureText("W", fontScale).y;
	EntitySetScaleBySize(pEnt, CL_Vec2f(letterHeight*2.0f, letterHeight*2.0f));
		
	if (bChecked)
	{
		AnimateStopEntityAndSetFrame(pEnt, 0, 1, 0);
	}

	//if someone clicks it, toggle it...
	if(!unclickable)
		pEnt->GetFunction("OnButtonSelected")->sig_function.connect(&OnCheckboxToggle);

	//add the text off to the right
	CL_Vec2f vImageSize = pEnt->GetVar("size2d")->GetVector2();
	Entity *pTextEnt = CreateTextLabelEntity(pEnt, "_text"+name, vImageSize.x+iPhoneMapX(8), iPhoneMapY(3), text);
	SetupTextEntity(pTextEnt, fontID, fontScale);
	//pEnt->GetVar("scale2d")->Set(pEnt->GetVar("scale2d")->GetVector2()*fontScale);
	return pEnt;
}

Entity * SetTextEntityByName(const string &entityName, string text, Entity *pRootEntity)
{
	Entity *pEnt = pRootEntity->GetEntityByName(entityName);
	if (!pEnt) return NULL; //couldn't find it

	EntityComponent *pTextComp = pEnt->GetComponentByName("TextRender");
	
	if (!pTextComp)
	{
		//also try for another kind of component
		pTextComp = pEnt->GetComponentByName("TextBoxRender");
	}
	
	if (!pTextComp)
	{
		assert(!"Well, we found the entity but it doesn't have a TextRender or TextBoxRender component.  How the heck can we change its label?");
		return NULL;
	}

	pTextComp->GetVar("text")->Set(text);
	return pEnt; //changed it
}


bool EntityIsOnScreen(Entity *pEnt)
{
	//NOTE:  Doesn't take parents position into account, //todo?
	CL_Rectf r = GetScreenRect();
	CL_Rectf er(pEnt->GetVar("pos2d")->GetVector2(), pEnt->GetVar("size2d")->GetVector2());
	return r.is_overlapped(er);
}

CL_Vec2f GetSize2DEntity(Entity *pEnt) {return pEnt->GetVar("size2d")->GetVector2();}
void SetSize2DEntity(Entity *pEnt, const CL_Vec2f &vSize) { pEnt->GetVar("size2d")->Set(vSize);}


CL_Vec2f GetImageSize2DEntity(Entity *pEnt) 
{
	//this is tricky because we have to recognize known visual formats rather than read the final size
	EntityComponent *pComp = pEnt->GetComponentByName("OverlayRender");
	if (pComp)
	{
		return pComp->GetVar("frameSize2d")->GetVector2();
	} 

	//LogMsg("GetImageSize2DEntity says %s has no known visual component to read from, returning 0,0", pEnt->GetName().c_str());
	return CL_Vec2f(-1, -1);
}

CL_Vec2f GetPos2DEntity(Entity *pEnt) {return pEnt->GetVar("pos2d")->GetVector2();}
void SetPos2DEntity(Entity *pEnt, const CL_Vec2f &vPos) { pEnt->GetVar("pos2d")->Set(vPos);}

CL_Vec2f GetScale2DEntity(Entity *pEnt) {return pEnt->GetVar("scale2d")->GetVector2();}
void SetScale2DEntity(Entity *pEnt, const CL_Vec2f &vScale) { pEnt->GetVar("scale2d")->Set(vScale);}

void SetProgressBarPercent(Entity *pEnt, float progressPercent)
{
	EntityComponent *pComp = pEnt->GetComponentByName("ProgressBar");
	if (!pComp)
	{
		assert(!"Only send this an entity with a ProgressBarComponent component!");
		return;
	}

	pComp->GetVar("progress")->Set(progressPercent);
}

void SetVisibleEntity(Entity *pEnt, bool bVisible)
{
	if (!pEnt) return;

	pEnt->GetVar("visible")->Set(uint32(bVisible));
}

void CopyPropertiesToEntity(Entity *pToEnt, Entity *pFromEnt, const string varName1, const string varName2, const string varName3)
{
	pToEnt->GetVar(varName1)->Set(*pFromEnt->GetVar(varName1));

	if (!varName2.empty())
	{
		pToEnt->GetVar(varName2)->Set(*pFromEnt->GetVar(varName2));
	}

	if (!varName3.empty())
	{
		pToEnt->GetVar(varName3)->Set(*pFromEnt->GetVar(varName3));
	}
}


void MoveEntityToTop(VariantList *pVList)
{
	Entity *pEnt =pVList->Get(0).GetEntity();

	RemoveFocusIfNeeded(pEnt);
	AddFocusIfNeeded(pEnt);
}

void OnShowTextMessage(VariantList *pVList)
{
	string msg = pVList->Get(0).GetString();
	int timeMS = pVList->Get(1).GetUINT32();

	Entity *pEnt = CreateTextLabelEntity(NULL, "", 0,0, msg);
	SetupTextEntity(pEnt, FONT_LARGE, 0.66f);

	//now that we know the size of the text, let's create a black bg, then attach the text to it.
	Entity *pRect = CreateOverlayRectEntity(NULL, GetScreenSize()/2, GetSize2DEntity(pEnt), MAKE_RGBA(0,0,0,170) );
	SetAlignmentEntity(pRect, ALIGNMENT_UPPER_CENTER);
	pRect->AddEntity(pEnt);

	AddFocusIfNeeded(pRect);
	FadeOutAndKillEntity(pRect, true, 1000, timeMS);
	ZoomFromPositionEntity(pRect, CL_Vec2f( GetScreenSizeXf()/2, GetScreenSizeYf()), 600);

	//hack to make sure we're on the top of the screen.. maybe we should do it every frame..
	VariantList v(pRect);
	GetMessageManager()->CallEntityFunction(pRect, timeMS+1, "MoveToTop", &v, TIMER_SYSTEM);
	pRect->GetFunction("MoveToTop")->sig_function.connect(MoveEntityToTop);

}

void ShowTextMessage(string msg, int timeMS, int delayBeforeStartingMS)
{
	VariantList vList(msg, (uint32)timeMS);
	GetMessageManager()->CallStaticFunction(OnShowTextMessage, delayBeforeStartingMS, &vList, TIMER_SYSTEM);
}


void SetupEntityToEatInput(Entity *pEnt)
{
	if (!pEnt->GetComponentByName("TouchHandler"))
	{
		pEnt->AddComponent(new TouchHandlerComponent);
	}

	assert(pEnt->GetComponentByName("Button2D") == 0 && "This won't work, it's already a button!");
	
	pEnt->AddComponent(new Button2DComponent);

	SetButtonStyleEntity(pEnt, Button2DComponent::BUTTON_STYLE_CLICK_ON_TOUCH);
	SetButtonClickSound(pEnt, ""); //don't make a sound!
	SetButtonVisualStyleEntity(pEnt, Button2DComponent::STYLE_NONE);
}

void ActivateTextInputEntity(Entity *pEnt)
{
	if (!pEnt) return;

	EntityComponent *pInputComp = pEnt->GetComponentByName("InputTextRender");
	if (pEnt)
	{
		pInputComp->GetFunction("ActivateKeyboard")->sig_function(NULL);
	}
}

CL_Vec2f GetScreenPos2DEntity( Entity *pEnt, CL_Vec2f vRecursivePosToAdd )
{
	CL_Vec2f vPos = ConvertEntityClickToScreenCoords(GetPos2DEntity(pEnt), pEnt)+vRecursivePosToAdd;

	Entity *pParent = pEnt->GetParent();

	if (!pParent)
	{
		//we're done
		return vPos;
	}

	return GetScreenPos2DEntity(pParent, vPos);
}

void SetEntityOverlayToUnloadImageOnKill(Entity *pImage)
{
	if (!pImage) return;

	EntityComponent *pComp = pImage->GetComponentByName("OverlayRender");
	if (pComp)
	{
		pComp->GetVar("unloadImageAtOnKill")->Set(uint32(1));
	}
}