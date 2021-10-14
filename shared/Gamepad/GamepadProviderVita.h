#ifndef GamepadProviderVita_h__
#define GamepadProviderVita_h__

#include "GamepadProvider.h"

class GamepadProviderVita : public GamepadProvider
{
    public:

        GamepadProviderVita();
        virtual ~GamepadProviderVita();

        virtual string GetName() { return "Vita"; }
        virtual bool Init();
	    virtual void Kill();
	    virtual void Update();
};

#endif