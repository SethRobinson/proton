#ifndef GamepadVita_h__
#define GamepadVita_h__

#include <psp2/ctrl.h>

#include "Gamepad.h"
#include "GamepadProviderPSP2.h"

class GamepadPSP2 : public Gamepad
{
    public:

        GamepadPSP2();
        virtual ~GamepadPSP2();

        virtual bool Init();
        virtual void Kill();
        virtual void Update();
    
    protected:

        void PressButton(int mask, int id);

        SceCtrlData m_state;
};

#endif