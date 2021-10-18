#ifndef VitaTouch_h__
#define VitaTouch_h__

#include <psp2/touch.h>

class VitaTouch
{

    public:

        VitaTouch();
        virtual ~VitaTouch();
        virtual void Update(); //Polling...

    private:

        SceTouchData m_Touch;
        SceTouchData m_TouchPrevious;

};

#endif