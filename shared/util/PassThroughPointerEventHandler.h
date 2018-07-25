#ifndef PASSTHROUGHPOINTEREVENTHANDLER_H
#define PASSTHROUGHPOINTEREVENTHANDLER_H

#include "PointerEventHandler.h"

class PassThroughPointerEventHandler : public PointerEventHandler
{
public:
    PassThroughPointerEventHandler();
	virtual ~PassThroughPointerEventHandler();

	virtual void handlePointerDownEvent(float x, float y, unsigned int pointerId);
	virtual void handlePointerMoveEvent(float x, float y, unsigned int pointerId);
	virtual void handlePointerUpEvent(float x, float y, unsigned int pointerId);
};

#endif // PASSTHROUGHPOINTEREVENTHANDLER_H
