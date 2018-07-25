#ifndef TOUCHDEVICEEMULATORPOINTEREVENTHANDLER_H
#define TOUCHDEVICEEMULATORPOINTEREVENTHANDLER_H

#include "PointerEventHandler.h"
#include "Manager/MessageManager.h"
#include "ClanLib-2.0/Sources/API/Core/Math/vec2.h"

/**
 * Emulates a multitouch device with a single pointer device (such as a mouse on a desktop computer).
 *
 * Handles two pointer ids specially (set in the constructor). When the secondary pointer
 * is pressed down the position of the pointer becomes a center position for a future
 * multitouch gesture. Afterwards when the primary pointer is pressed down (and the
 * secondary pointer is still down) a two-touchpoint gesture is started. The second
 * touch point is on the opposite side of the gesture center point. Pinch-zoom and
 * two-finger rotate gestures can be performed with this system.
 *
 * This class also draws a small marker on the place of the emulated touch point.
 */
class TouchDeviceEmulatorPointerEventHandler : public PointerEventHandler
{
public:
	/**
	 * \param primaryPointerId sets a value that is used for the primary pointer.
	 * \param secondaryPointerId sets a value that is used for the secondary pointer.
	 */
	TouchDeviceEmulatorPointerEventHandler(unsigned int primaryPointerId, unsigned int secondaryPointerId);
	virtual ~TouchDeviceEmulatorPointerEventHandler();

	virtual void handlePointerDownEvent(float x, float y, unsigned int pointerId);
	virtual void handlePointerMoveEvent(float x, float y, unsigned int pointerId);
	virtual void handlePointerUpEvent(float x, float y, unsigned int pointerId);

private:
	enum Mode {
		NONE,
		NORMAL_TOUCH,
		ANTICIPATING_GESTURE,
		EMULATED_GESTURE
	};

	CL_Vec2f getFakeTouchPoint(float x, float y) const;
	void sendTouchPointEventsInRandomOrder(eMessageType msgType, float x, float y) const;

	const unsigned int mPrimaryPointerId;
	const unsigned int mSecondaryPointerId;
	Mode mMode;
	CL_Vec2f mFakeGestureCenter;
	bool mSecondaryButtonDown;

};

#endif // TOUCHDEVICEEMULATORPOINTEREVENTHANDLER_H
