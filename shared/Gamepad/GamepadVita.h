#ifndef GamepadVita_h__
#define GamepadVita_h__

#include <psp2/ctrl.h>

#include "Gamepad.h"
#include "GamepadProviderVita.h"

class GamepadVita : public Gamepad
{
    public:

        GamepadVita();
        virtual ~GamepadVita();

        virtual bool Init();
        virtual void Kill();
        virtual void Update();
    
    protected:

        void PressButton(int mask, int id);
        float ConvertToProtonStickWithDeadZone(float stick);

        SceCtrlData m_state;
};

#endif