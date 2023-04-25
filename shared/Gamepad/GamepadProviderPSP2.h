#ifndef GamepadProviderVita_h__
#define GamepadProviderVita_h__

#include "GamepadProvider.h"

class GamepadProviderPSP2 : public GamepadProvider
{
    public:

        GamepadProviderPSP2();
        virtual ~GamepadProviderPSP2();

        virtual string GetName() { return "Playstation Vita"; }
        virtual bool Init();
	    virtual void Kill();
	    virtual void Update();
};

#endif