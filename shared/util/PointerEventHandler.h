#ifndef POINTEREVENTHANDLER_H
#define POINTEREVENTHANDLER_H

class PointerEventHandler
{
public:
	virtual ~PointerEventHandler() {}

	virtual void handlePointerDownEvent(float x, float y, unsigned int pointerId) = 0;
	virtual void handlePointerMoveEvent(float x, float y, unsigned int pointerId) = 0;
	virtual void handlePointerUpEvent(float x, float y, unsigned int pointerId) = 0;
};

#endif // POINTEREVENTHANDLER_H
