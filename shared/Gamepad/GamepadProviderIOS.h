//Provide support for iOS's native support of bluetooth gamepads
#include "GamepadProvider.h"

#pragma once


class GamepadIOS;

class GamepadProviderIOS : public GamepadProvider
{
public:
	GamepadProviderIOS();
	virtual ~GamepadProviderIOS();

	virtual string GetName() { return "IOS"; }
	virtual bool Init();
	virtual void Kill();
	virtual void Update();

	//Used by our custom GamepadIOS class

	CL_Vec2f  GetLeftStickPos();
	CL_Vec2f  GetRightStickPos();

protected:

	void OnEnterBackground(VariantList* pVList);
	void OnEnterForeground(VariantList* pVList);

private:


	GamepadIOS* m_pPad;
};



