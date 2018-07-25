#include "PassThroughPointerEventHandler.h"

#include "BaseApp.h"

PassThroughPointerEventHandler::PassThroughPointerEventHandler()
{
}

PassThroughPointerEventHandler::~PassThroughPointerEventHandler()
{
}

void PassThroughPointerEventHandler::handlePointerDownEvent(float x, float y, unsigned int pointerId)
{
	GetMessageManager()->SendGUIEx(MESSAGE_TYPE_GUI_CLICK_START, x, y, pointerId);
}

void PassThroughPointerEventHandler::handlePointerMoveEvent(float x, float y, unsigned int pointerId)
{
	GetMessageManager()->SendGUIEx(MESSAGE_TYPE_GUI_CLICK_MOVE, x, y, pointerId);
}

void PassThroughPointerEventHandler::handlePointerUpEvent(float x, float y, unsigned int pointerId)
{
	GetMessageManager()->SendGUIEx(MESSAGE_TYPE_GUI_CLICK_END, x, y, pointerId);
}
