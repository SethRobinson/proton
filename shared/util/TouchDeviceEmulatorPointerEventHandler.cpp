#include "TouchDeviceEmulatorPointerEventHandler.h"

#include "BaseApp.h"
#include "Entity/RectRenderComponent.h"
#include "Entity/FocusRenderComponent.h"

TouchDeviceEmulatorPointerEventHandler::TouchDeviceEmulatorPointerEventHandler(unsigned int primaryPointerId, unsigned int secondaryPointerId) :
	mPrimaryPointerId(primaryPointerId),
	mSecondaryPointerId(secondaryPointerId),
	mMode(NONE),
	mSecondaryButtonDown(false)
{
}

TouchDeviceEmulatorPointerEventHandler::~TouchDeviceEmulatorPointerEventHandler()
{
}

void TouchDeviceEmulatorPointerEventHandler::handlePointerDownEvent(float x, float y, unsigned int pointerId)
{
	switch (mMode) {
	case NONE:
		if (pointerId == mPrimaryPointerId) {
			mMode = NORMAL_TOUCH;
			GetMessageManager()->SendGUIEx(MESSAGE_TYPE_GUI_CLICK_START, x, y, mPrimaryPointerId);
		} else if (pointerId == mSecondaryPointerId) {
			mMode = ANTICIPATING_GESTURE;
			mFakeGestureCenter.x = x;
			mFakeGestureCenter.y = y;
		}
		break;

	case ANTICIPATING_GESTURE:
		if (pointerId == mPrimaryPointerId) {
			mMode = EMULATED_GESTURE;

			GetMessageManager()->SendGUIEx(MESSAGE_TYPE_GUI_CLICK_START, x, y, mPrimaryPointerId);
			CL_Vec2f fakePoint(getFakeTouchPoint(x, y));
			GetMessageManager()->SendGUIEx(MESSAGE_TYPE_GUI_CLICK_START, fakePoint.x, fakePoint.y, mSecondaryPointerId);

			Entity* marker = GetBaseApp()->GetEntityRoot()->AddEntity(new Entity("FakeTouchMarker"));
			marker->AddComponent(new RectRenderComponent);
			marker->GetVar("pos2d")->Set(fakePoint);
			marker->GetVar("size2d")->Set(CL_Vec2f(10, 10));
			marker->GetVar("color")->Set(MAKE_RGBA(150, 240, 150, 128));
			marker->AddComponent(new FocusRenderComponent);
		}
		break;
	}

	if (pointerId == mSecondaryPointerId) {
		mSecondaryButtonDown = true;
	}

	if (pointerId != mPrimaryPointerId && pointerId != mSecondaryPointerId) {
		GetMessageManager()->SendGUIEx(MESSAGE_TYPE_GUI_CLICK_START, x, y, pointerId);
	}
}

void TouchDeviceEmulatorPointerEventHandler::handlePointerMoveEvent(float x, float y, unsigned int pointerId)
{
	switch (mMode) {
	case NORMAL_TOUCH:
		GetMessageManager()->SendGUIEx(MESSAGE_TYPE_GUI_CLICK_MOVE, x, y, mPrimaryPointerId);
		break;

	case EMULATED_GESTURE:
		sendTouchPointEventsInRandomOrder(MESSAGE_TYPE_GUI_CLICK_MOVE, x, y);

		GetBaseApp()->GetEntityRoot()->GetEntityByName("FakeTouchMarker")->GetVar("pos2d")->Set(getFakeTouchPoint(x, y));
		break;
	}
}

void TouchDeviceEmulatorPointerEventHandler::handlePointerUpEvent(float x, float y, unsigned int pointerId)
{
	switch (mMode) {
	case NORMAL_TOUCH:
		if (pointerId == mPrimaryPointerId) {
			mMode = NONE;
			GetMessageManager()->SendGUIEx(MESSAGE_TYPE_GUI_CLICK_END, x, y, mPrimaryPointerId);
		}
		break;

	case EMULATED_GESTURE:
		if (pointerId == mPrimaryPointerId) {
			sendTouchPointEventsInRandomOrder(MESSAGE_TYPE_GUI_CLICK_END, x, y);

			GetBaseApp()->GetEntityRoot()->RemoveEntityByName("FakeTouchMarker");

			if (mSecondaryButtonDown) {
				mMode = ANTICIPATING_GESTURE;
			} else {
				mMode = NONE;
			}
		}
		break;

	case ANTICIPATING_GESTURE:
		if (pointerId == mSecondaryPointerId) {
			mMode = NONE;
		}
	}

	if (pointerId == mSecondaryPointerId) {
		mSecondaryButtonDown = false;
	}

	if (pointerId != mPrimaryPointerId && pointerId != mSecondaryPointerId) {
		GetMessageManager()->SendGUIEx(MESSAGE_TYPE_GUI_CLICK_END, x, y, pointerId);
	}
}

CL_Vec2f TouchDeviceEmulatorPointerEventHandler::getFakeTouchPoint(float x, float y) const
{
	CL_Vec2f realPoint(x, y);
	return CL_Vec2f(realPoint + (mFakeGestureCenter - realPoint) * 2);
}

void TouchDeviceEmulatorPointerEventHandler::sendTouchPointEventsInRandomOrder(eMessageType msgType, float x, float y) const
{
	CL_Vec2f fakePoint(getFakeTouchPoint(x, y));

	if (GetBaseApp()->GetTick() & 1) {
		GetMessageManager()->SendGUIEx(msgType, x, y, mPrimaryPointerId);
		GetMessageManager()->SendGUIEx(msgType, fakePoint.x, fakePoint.y, mSecondaryPointerId);
	} else {
		GetMessageManager()->SendGUIEx(msgType, fakePoint.x, fakePoint.y, mSecondaryPointerId);
		GetMessageManager()->SendGUIEx(msgType, x, y, mPrimaryPointerId);
	}
}
