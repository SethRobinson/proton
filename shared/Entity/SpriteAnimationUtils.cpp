#include "SpriteAnimationUtils.h"

#include "BaseApp.h"
#include "Entity.h"
#include "SpriteAnimationRenderComponent.h"

void SpriteAnimationUtils::SetupAnimationEntity(Entity* pEntity, const std::string& fileName, const std::string& animationName)
{
	EntityComponent *spriteAnimComp = pEntity->AddComponent(new SpriteAnimationRenderComponent);
	spriteAnimComp->GetVar("fileName")->Set(fileName);

	if (!animationName.empty())
	{
		spriteAnimComp->GetVar("animationName")->Set(animationName);
	}
}

void SpriteAnimationUtils::SetAnimationName(Entity* pEntity, const std::string& animationName)
{
	if (!animationName.empty())
	{
		EntityComponent* spriteAnimComp = pEntity->GetComponentByName("SpriteAnimationRender");

#ifdef _DEBUG
		if (!spriteAnimComp)
		{
			LogError("No sprite animation component found from '%s'. Can't set animation name.", pEntity->GetName().c_str());
			assert(spriteAnimComp);
		}
#endif

		spriteAnimComp->GetVar("animationName")->Set(animationName);
	}
}

void SpriteAnimationUtils::StartAnimationEntity(Entity *pEntity, const std::string& animationName, unsigned int animationDuration, unsigned int delayToStartMS, InterpolateComponent::eOnFinish finishType)
{
	EntityComponent* spriteAnimComp = pEntity->GetComponentByName("SpriteAnimationRender");

#ifdef _DEBUG
	if (!spriteAnimComp)
	{
		LogError("No sprite animation component found from '%s'. Can't start animation", pEntity->GetName().c_str());
		assert(spriteAnimComp);
	}
#endif

	EntityComponent *animator = pEntity->GetComponentByName("ic_sprite_animation");
	if (!animator)
	{
		animator = pEntity->AddComponent(new InterpolateComponent);
		animator->SetName("ic_sprite_animation");
		animator->GetVar("component_name")->Set("SpriteAnimationRender");
		animator->GetVar("var_name")->Set("phase");
		animator->GetVar("interpolation")->Set(uint32(INTERPOLATE_LINEAR));
		animator->GetVar("on_finish")->Set(uint32(finishType));
		animator->GetVar("target")->Set(1.0f);
	}

	if (delayToStartMS == 0)
	{
		spriteAnimComp->GetVar("animationName")->Set(animationName);
		animator->GetVar("duration_ms")->Set(uint32(animationDuration));
	} else
	{
		GetMessageManager()->SetComponentVariable(spriteAnimComp, delayToStartMS, "animationName", Variant(animationName));
		GetMessageManager()->SetComponentVariable(animator, delayToStartMS, "duration_ms", Variant(uint32(animationDuration)));
	}
}
