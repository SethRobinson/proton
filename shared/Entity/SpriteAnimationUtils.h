#ifndef SPRITEANIMATIONUTILS_H
#define SPRITEANIMATIONUTILS_H

#include <string>
#include "InterpolateComponent.h"

/**
 * Various utility functions for dealing with sprites and sprite animations.
 */
class SpriteAnimationUtils
{
public:
	/**
	 * Sets up \a pEntity to be used with a sprite animation.
	 * The file from which the animation data is loaded is set by \a fileName.
	 * If an initial animation name should be set it can be given with
	 * \a animationName.
	 */
	static void SetupAnimationEntity(Entity* pEntity, const std::string& fileName, const std::string& animationName = std::string());

	/**
	 * Sets a sprite animation name to \a animationName.
	 * The argument \a pEntity must already contain a \c SpriteAnimationRenderComponent.
	 * This method does not create one.
	 */
	static void SetAnimationName(Entity* pEntity, const std::string& animationName);

	/**
	 * Starts a sprite animation called \a animationName on \a pEntity.
	 * The duration of the animation is set by \a animationDuration in milliseconds.
	 * It's expected that \a pEntity has been prepared with \c SetupSpriteAnimationEntity()
	 * before calling this function.
	 *
	 * If \a delayToStartMS is greater than 0 then the animation is only started after
	 * this many milliseconds has passed.
	 *
	 * When the animation ends by default the animation stops and the last frame of the
	 * animation is left visible. This behaviour can be changed by setting the
	 * \a finishType. For example the animation can be set to repeat itself from the start.
	 */
	static void StartAnimationEntity(Entity *pEntity, const std::string& animationName, unsigned int animationDuration, unsigned int delayToStartMS = 0, InterpolateComponent::eOnFinish finishType = InterpolateComponent::ON_FINISH_STOP);
};

#endif // SPRITEANIMATIONUTILS_H
