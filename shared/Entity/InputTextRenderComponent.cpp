#include "PlatformPrecomp.h"

#include "InputTextRenderComponent.h"
#include "BaseApp.h"

#ifdef PLATFORM_HTML5
#include "html5/SharedJSLIB.h";
int GetTouchesReceived();
#endif

InputTextRenderComponent::InputTextRenderComponent()
{
	SetName("InputTextRender");
}

InputTextRenderComponent::~InputTextRenderComponent()
{
	//BigB's change
	if (GetEntityWithNativeUIFocus() == GetParent())
	{
		GetFunction("CloseKeyboard")->sig_function(NULL);
	} else
	{
		if (GetIsUsingNativeUI())
		{
			LogMsg("Keyboard is active, but proton isn't closing it because it doesn't look like this InputTextRenderComponent has focus.");
		}
	}
}


void InputTextRenderComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);

	//shared with the rest of the entity
	m_pPos2d = &GetParent()->GetVar("pos2d")->GetVector2();
	m_pSize2d = &GetParent()->GetVar("size2d")->GetVector2();
	m_pScale2d = &GetParent()->GetVarWithDefault("scale2d", Variant(1.0f, 1.0f))->GetVector2();
	m_pAlignment = &GetParent()->GetVar("alignment")->GetUINT32();
	m_pColor = &GetParent()->GetVarWithDefault("color", Variant(MAKE_RGBA(255,255,255,255)))->GetUINT32();
	m_pColorMod = &GetParent()->GetVarWithDefault("colorMod", Variant(MAKE_RGBA(255,255,255,255)))->GetUINT32();
	m_pAlpha = &GetParent()->GetVarWithDefault("alpha", Variant(1.0f))->GetFloat();

	GetParent()->GetFunction("OnTouchEnd")->sig_function.connect(1, boost::bind(&InputTextRenderComponent::OnTouchEnd, this, _1));
	GetParent()->GetFunction("OnTouchStart")->sig_function.connect(1, boost::bind(&InputTextRenderComponent::OnTouchStart, this, _1));
	GetParent()->GetFunction("OnInput")->sig_function.connect(1, boost::bind(&InputTextRenderComponent::OnInput, this, _1)); //used for keyboard keys on Win
	GetParent()->GetFunction("OnLosingNativeGUIFocus")->sig_function.connect(1, boost::bind(&InputTextRenderComponent::OnLosingNativeGUIFocus, this, _1)); 
	GetFunction("ActivateKeyboard")->sig_function.connect(1, boost::bind(&InputTextRenderComponent::ActivateKeyboard, this, _1)); 
	GetFunction("CloseKeyboard")->sig_function.connect(1, boost::bind(&InputTextRenderComponent::CloseKeyboard, this, _1)); 
	m_pVisible = &GetParent()->GetVarWithDefault("visible", uint32(1))->GetUINT32();
	m_pGetFocusOnEnter = &GetVarWithDefault("getFocusOnEnter", uint32(0))->GetUINT32();
	GetParent()->GetVar("visible")->GetSigOnChanged()->connect(1, boost::bind(&InputTextRenderComponent::OnVisibilityChanged, this, _1));
	m_pHasFocus = &GetVar("hasFocus")->GetUINT32();

	//our own stuff
	m_pDisabled = &GetVarWithDefault("disabled", uint32(0))->GetUINT32();
	GetVar("disabled")->GetSigOnChanged()->connect(1, boost::bind(&InputTextRenderComponent::OnDisabledChanged, this, _1));
	m_pVisualStyle = &GetVarWithDefault("visualStyle", Variant(uint32(STYLE_NORMAL)))->GetUINT32();
	GetVar("visualStyle")->GetSigOnChanged()->connect(1, boost::bind(&InputTextRenderComponent::OnVisualStyleChanged, this, _1));


	m_pTextSize2d = &GetVar("textSize2d")->GetVector2();
	m_pTextOffsetPos2d = &GetVarWithDefault("textOffsetPos2d", Variant(3,3))->GetVector2();
	m_pCursorColor = &GetVarWithDefault("cursorColor", Variant(MAKE_RGBA(209,181,137,255)))->GetUINT32();
	m_pInputLengthMax = &GetVarWithDefault("inputLengthMax", Variant(uint32(10)))->GetUINT32();
	m_pBorderColor = &GetVarWithDefault("borderColor", Variant(MAKE_RGBA(255,255,255,255)))->GetUINT32();
	m_pInputType = &GetVarWithDefault("inputType", Variant(uint32(INPUT_TYPE_ASCII)))->GetUINT32();
	m_pFiltering = &GetVarWithDefault("filtering", Variant(uint32(FILTERING_STRICT)))->GetUINT32();
	m_pTruncateTextIfNeeded = &GetVarWithDefault("truncateTextIfNeeded", Variant(uint32(0)))->GetUINT32();
	GetVar("truncateTextIfNeeded")->GetSigOnChanged()->connect(1, boost::bind(&InputTextRenderComponent::OnTruncateTextIfNeededChanged, this, _1));

	m_pText = &GetVar("text")->GetString(); //local to us
	GetVar("text")->GetSigOnChanged()->connect(1, boost::bind(&InputTextRenderComponent::OnTextChanged, this, _1));

	m_pPlaceHolderText = &GetVar("placeHolderText")->GetString(); //local to us

	m_pFontID = &GetVarWithDefault("font", uint32(FONT_SMALL))->GetUINT32();
	GetVar("font")->GetSigOnChanged()->connect(1, boost::bind(&InputTextRenderComponent::OnFontChanged, this, _1));
	GetParent()->GetVar("scale2d")->GetSigOnChanged()->connect(1, boost::bind(&InputTextRenderComponent::OnScaleChanged, this, _1));

	//register ourselves to render if the parent does
	GetParent()->GetFunction("OnRender")->sig_function.connect(1, boost::bind(&InputTextRenderComponent::OnRender, this, _1));
	GetParent()->GetFunction("OnUpdate")->sig_function.connect(1, boost::bind(&InputTextRenderComponent::OnUpdate, this, _1));

	//on android we might need to reinit the soft keyboard

	GetBaseApp()->m_sig_enterforeground.connect(1, boost::bind(&InputTextRenderComponent::OnEnterForeground, this, _1));
	GetBaseApp()->m_sig_enterbackground.connect(1, boost::bind(&InputTextRenderComponent::OnEnterBackground, this, _1));

}



void InputTextRenderComponent::OnVisibilityChanged(Variant *pDataObject)
{
	if (pDataObject->GetUINT32() == 0)
	{
		//we're no longer visible.  If we have focus and the keyboard is up, we should really put it down
		CloseKeyboard(NULL);
	}
}


void InputTextRenderComponent::OnDisabledChanged(Variant *pDataObject)
{
	if (pDataObject->GetUINT32() == 1)
	{
		//we should lose focus if we had it, otherwise it will look weird with the blinking carrot on a dimmed out control
		CloseKeyboard(NULL);
	}
}

void InputTextRenderComponent::OnTextChanged(Variant *pDataObject)
{
	rtRectf rt;

	switch(*m_pVisualStyle)
	{
	case STYLE_NORMAL:
	if (*m_pTruncateTextIfNeeded != 0)
	{
		m_displayText = TrimText(m_pText);
		//also set the real thing
		*m_pText = m_displayText;
	} else
	{
		m_displayText = *m_pText;
	}

	break;

	case STYLE_PASSWORD:
		m_displayText.clear();
		m_displayText.resize(m_pText->size(), '*');
		break;
	}

	GetBaseApp()->GetFont(eFont(*m_pFontID))->MeasureText(&rt, m_displayText, m_pScale2d->x);
	
	*m_pTextSize2d = CL_Vec2f(rt.GetWidth(), rt.GetHeight());

	if (*m_pHasFocus)
	{
		SetLastStringInput(*m_pText);
	}

	//oh, fix the border around us to fit if needed
	m_pSize2d->y = GetBaseApp()->GetFont(eFont(*m_pFontID))->GetLineHeight(m_pScale2d->y)+6.0f;
}

void InputTextRenderComponent::OnScaleChanged(Variant *pDataObject)
{
	OnTextChanged(NULL);
}

void InputTextRenderComponent::OnVisualStyleChanged(Variant *pDataObject)
{
	OnTextChanged(NULL);
}


void InputTextRenderComponent::OnFontChanged(Variant *pDataObject)
{
	OnTextChanged(NULL);
}

void InputTextRenderComponent::ActivateKeyboard(VariantList *pVList)
{


	if (GetIsUsingNativeUI())
	{
		if (GetEntityWithNativeUIFocus() == GetParent())
		{
			return;
		} else
		{
			SetIsUsingNativeUI(false); //prepare to accept focus here and unfocus whatever had it
			GetMessageManager()->CallComponentFunction(this, 1, "ActivateKeyboard");
			return;
		}
	}
	

	string name = "Unknown";
	if (GetParent()) name = GetParent()->GetName();
#ifdef _DEBUG
	LogMsg("InputTextRenderComponent::ActivateKeyboard sending MESSAGE_OPEN_TEXT_BOX from %s", name.c_str());
#endif

	OSMessage o;
	o.m_type = OSMessage::MESSAGE_OPEN_TEXT_BOX;
	o.m_string = *m_pText;
	SetLastStringInput(*m_pText);
	o.m_x = -1000;
	o.m_y = -1000; 
	o.m_parm1 = *m_pInputLengthMax; //no longer used...
	o.m_fontSize = 30.0f;
	o.m_sizeX = 217;
	o.m_sizeY = 40;

	switch (*m_pInputType)
	{
	case INPUT_TYPE_NUMBERS:
		o.m_parm2 = OSMessage::PARM_KEYBOARD_TYPE_NUMBERS;
		break;

	case INPUT_TYPE_ASCII:
		o.m_parm2 = OSMessage::PARM_KEYBOARD_TYPE_ASCII;
		break;

	case INPUT_TYPE_URL:
		o.m_parm2 = OSMessage::PARM_KEYBOARD_TYPE_URL;
		break;

	case INPUT_TYPE_ASCII_FULL:
		o.m_parm2 = OSMessage::PARM_KEYBOARD_TYPE_ASCII_FULL;
		break;
	
	case INPUT_TYPE_EMAIL:
		o.m_parm2 = OSMessage::PARM_KEYBOARD_TYPE_EMAIL;
		break;
	}

	GetBaseApp()->AddOSMessage(o);
	GetVar("hasFocus")->Set(uint32(1));
	SetIsUsingNativeUI(true);
	SetEntityWithNativeUIFocus(GetParent());

    //send message to whoever cares, iCade gamepad on iOS cares about this
    VariantList v;
    v.Get(0).Set((float)MESSAGE_TYPE_HW_KEYBOARD_INPUT_STARTING);
    GetBaseApp()->m_sig_hardware(&v);
    


#ifdef PLATFORM_HTML5
	
	//do we use the annoying javascript "prompt" command to grab text?  Only if we know this is a touch screen...
	if (GetTouchesReceived() > 0)
	{
		char *pInput = JLIB_EnterString("Enter text:", m_pText->c_str());

		if (pInput)
		{
			string temp = pInput;

			TruncateString(temp, *m_pInputLengthMax);
			SetLastStringInput(temp);
			free(pInput);
		} else
		{
			LogMsg("ignoring bad input");
		}
		
		
		SetIsUsingNativeUI(false);
	}
#endif


}

void InputTextRenderComponent::OnLosingNativeGUIFocus(VariantList *pVList)
{
	string name = "Unknown";
	if (GetParent()) name = GetParent()->GetName();

	LogMsg("Item %s losing focus, closing keyboard", name.c_str());
    VariantList vList(this);
	GetFunction("CloseKeyboard")->sig_function(&vList);
}

void InputTextRenderComponent::OnEnterForeground(VariantList *pVList)
{
	if (GetEntityWithNativeUIFocus() == GetParent())
	{
	//added for android


#ifdef _DEBUG
		LogMsg("InputTextRenderComponent::OnEnterForeground - Re-opening on keyboard");
#endif
		//re init our keyboard just in case, android needs this
		GetFunction("ActivateKeyboard")->sig_function(NULL);
	}
	
}

void InputTextRenderComponent::OnEnterBackground(VariantList *pVList)
{
	
	//added for Android
	if (GetEntityWithNativeUIFocus() == GetParent())
	{

#ifdef _DEBUG
		LogMsg("InputTextRenderComponent::OnEnterBackground - Running CloseKeyboard");
#endif
	
		VariantList vList(this);
		GetFunction("CloseKeyboard")->sig_function(&vList);
	}

}


void InputTextRenderComponent::CloseKeyboard( VariantList *pVList )
{
	string name = "Unknown";
	if (GetParent()) name = GetParent()->GetName();

	if (GetEntityWithNativeUIFocus() == GetParent())
	{
#ifdef _DEBUG
		LogMsg("InputTextRenderComponent::CloseKeyboard - setting NativeUIFocus to zero from %s", name.c_str());
#endif
		SetEntityWithNativeUIFocus(NULL, true);
	}

	if (!*m_pHasFocus) return;

#ifdef _DEBUG
	LogMsg("Sending MESSAGE_CLOSE_TEXT_BOX from %s", name.c_str());
#endif

	GetVar("hasFocus")->Set(uint32(0));

	OSMessage o;
	o.m_type = OSMessage::MESSAGE_CLOSE_TEXT_BOX;
	GetBaseApp()->AddOSMessage(o);

    //send message to people who care, iCade gamepad on iOS cares about this...
    VariantList v;
    v.Get(0).Set((float)MESSAGE_TYPE_HW_KEYBOARD_INPUT_ENDING);
    GetBaseApp()->m_sig_hardware(&v);
    
}


void InputTextRenderComponent::OnTouchEnd(VariantList *pVList)
{
	/*
	if (*m_pVisible == 0) return;

	TouchTrackInfo *pTouch = GetBaseApp()->GetTouch(pVList->Get(2).GetUINT32());
	if (pTouch->WasHandled()) return;
	pTouch->SetWasHandled(true, GetParent());

	if (*m_pDisabled == false)
	{
		ActivateKeyboard(NULL);
	}
	*/
}


void InputTextRenderComponent::OnTouchStart(VariantList *pVList)
{
	if (*m_pVisible == 0) return;

	TouchTrackInfo *pTouch = GetBaseApp()->GetTouch(pVList->Get(2).GetUINT32());
	if (pTouch->WasHandled()) return;
	pTouch->SetWasHandled(true, GetParent());

	if (*m_pDisabled == false)
	{
		ActivateKeyboard(NULL);
	}
}
void InputTextRenderComponent::OnRemove()
{
	/*
	if (GetEntityWithNativeUIFocus() == GetParent())
	{
		SetEntityWithNativeUIFocus(NULL);
	}
	//Don't do this, otherwise we won't kill the keyboard properly in the deconstructor
	*/

	EntityComponent::OnRemove();
}

void InputTextRenderComponent::OnUpdate(VariantList *pVList)
{

	if (*m_pHasFocus)
	{

	
	//	string curString = FilterToValidAscii(GetLastStringInput(), *m_pFiltering == FILTERING_STRICT);
				
		string curString = GetLastStringInput(); //don't have to filter, already was
		
		if (*m_pText != curString)
		{
			if (curString.size() > m_pText->size())
			{
				//well, a character was added.  send it in case anybody cares
                VariantList vList( this, uint32(curString[curString.size()-1]));
				GetFunction("OnChar")->sig_function(&vList);
			} else
			{
				if (curString.size() < m_pText->size())
				{
                    VariantList vList( this, uint32(8) );
                    
					GetFunction("OnChar")->sig_function(&vList); //send a delete char
				}
			}
		
			GetVar("text")->Set(curString);
		}
	
	
		if (!GetIsUsingNativeUI())
		{
			//the keyboard is closed, we're done
			GetVar("hasFocus")->Set(uint32(0));
			GetFunction("CloseKeyboard")->sig_function(NULL);
		}
	}
}

void InputTextRenderComponent::OnRender(VariantList *pVList)
{
	if (*m_pAlpha <= 0) return;

	if (*m_pVisible == 0) return;

	CL_Vec2f vFinalPos = pVList->m_variant[0].GetVector2()+*m_pPos2d;
	//vFinalPos -= GetAlignmentOffset(*m_pSize2d, eAlignment(*m_pAlignment));

	if (vFinalPos.y < -m_pSize2d->y) return;
	if (vFinalPos.y > GetOrthoRenderSizeYf()) return;

	float alpha;

	if (*m_pDisabled)
	{
		alpha = rt_min(*m_pAlpha, 0.5f);
	} else
	{
		alpha = *m_pAlpha;
	}
	
	uint32 color = ColorCombine(*m_pColor, *m_pColorMod, alpha);

	string *pTextToDraw = &m_displayText;

	if (!*m_pHasFocus && !m_pPlaceHolderText->empty())
	{
		pTextToDraw = m_pPlaceHolderText;
	}

	GetBaseApp()->GetFont(eFont(*m_pFontID))->DrawScaled(vFinalPos.x+m_pTextOffsetPos2d->x* m_pScale2d->x, vFinalPos.y+m_pTextOffsetPos2d->y* m_pScale2d->y, *pTextToDraw, m_pScale2d->x, color);
	

	uint32 borderCol = ColorCombine(*m_pBorderColor, MAKE_RGBA(255,255,255,255), alpha);
	if (GET_ALPHA(borderCol) > 0)
	{
		DrawRect(vFinalPos, *m_pSize2d, borderCol);
	}

	if (*m_pHasFocus)
	{
		//draw the blinking cursor too
		float height = GetBaseApp()->GetFont(eFont(*m_pFontID))->GetLineHeight(m_pScale2d->x);
				
		CL_Vec2f vCursorPos(vFinalPos+*m_pTextOffsetPos2d);
		vCursorPos.x += m_pTextSize2d->x;
		
		CL_Rectf r = CL_Rectf(vCursorPos.x, vCursorPos.y, vCursorPos.x+2, vCursorPos.y+height); 
		float blinkAlpha = ((SinPulseByMS(500)+1)/2);
		uint32 color = ColorCombine(*m_pCursorColor, *m_pColorMod, *m_pAlpha*blinkAlpha);
		
		if (GET_ALPHA(color) > 0)
		DrawFilledRect(r, color);
	}
}

void InputTextRenderComponent::OnInput( VariantList *pVList )
{
	if (*m_pVisible == 0) return;


	eMessageType messageType = eMessageType( int(pVList->Get(0).GetFloat()));

	if (!*m_pHasFocus)
	{
		//well, there is one exception to a keypress we'll process, if nobody else focus, and it's an enter, and we're
		//instructed to give ourselves focus if enter is hit, then ok.
		if (messageType == MESSAGE_TYPE_GUI_CHAR)
		{
			char c = (char)pVList->Get(2).GetUINT32();
			
			if (c == 13)
			{
				if ( *m_pGetFocusOnEnter && GetEntityWithNativeUIFocus() == NULL)
				{
					//they hit enter, give ourselves focus!
					GetFunction("ActivateKeyboard")->sig_function(NULL);
				}
			}
		}
		
		return; //don't process text if we're not active
	}

	//0 = message type, 1 = parent coordinate offset, 2 = char, 3 reserved for filtering control messages
	
	switch (messageType)
	{
	case MESSAGE_TYPE_GUI_PASTE:
		{
			string paste = pVList->Get(2).GetString();
			string input = *m_pText;

			input += paste;

			//but is it too long for our input box?
			TruncateString(input, *m_pInputLengthMax);
			input = FilterToValidAscii(input, *m_pFiltering == FILTERING_STRICT);
			if (*m_pHasFocus)
			{
				SetLastStringInput( input);
			} else
			{
				GetVar("text")->Set(input); //so everybody receives notifications that it has changed
			}
		}
		break;
	
	case MESSAGE_TYPE_GUI_CHAR:
#ifdef _DEBUG		
		//LogMsg("InputTextRender: Got key %u (%c)", pVList->Get(2).GetUINT32(), (char)pVList->Get(2).GetUINT32());
#endif
		if (pVList->Get(2).GetUINT32() > 255)
		{
			//a proton virtual key, like f1 or something. Ignore
			break;
		}


		char c = (char)pVList->Get(2).GetUINT32();
		
		string input = GetLastStringInput();

		switch (c)
		{
		case 13:
		{
			//enter
			VariantList vList(this);
			GetFunction("CloseKeyboard")->sig_function(&vList);
		}
			break;
		
		case 8:
			//backspace
			if (input.length() > 0)
			{
				input.erase(input.length()-1, 1);
			}
			break;

			
		break;

		default:

			if (input.size() < *m_pInputLengthMax)
			{
				input += c;
			} 
		}
	
		SetLastStringInput( FilterToValidAscii(input, *m_pFiltering == FILTERING_STRICT));
		break;
	}	
}

void InputTextRenderComponent::OnTruncateTextIfNeededChanged(Variant *pDataObject)
{
	if (pDataObject->GetUINT32() != 0)
	{
		//apply it to any existing text
		GetVar("text")->Set(*m_pText);
	}
}

string InputTextRenderComponent::TrimText(string *pText)
{

	CL_Vec2f vTextSize = GetBaseApp()->GetFont(eFont(*m_pFontID))->MeasureText(*pText, m_pScale2d->x);

	if (vTextSize.x > m_pSize2d->x)
	{
		//trim that shit
		int charsThatFitInArea = GetBaseApp()->GetFont(eFont(*m_pFontID))->CountCharsThatFitX(m_pSize2d->x, *pText, m_pScale2d->x);
		string temp = *m_pText;
		TruncateString(temp, charsThatFitInArea);
		return temp;
		
	}
	return *m_pText;
}
