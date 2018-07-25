#include "PlatformPrecomp.h"
#include "MessageManager.h"
#include "BaseApp.h"


void Message::SetDeliveryTime( int deliveryTimeMS )
{
	if (m_timerMethod == TIMER_GAME)
	{
		m_deliveryTime = deliveryTimeMS + GetBaseApp()->GetGameTick();
	} else m_deliveryTime = deliveryTimeMS + GetBaseApp()->GetTick();
}

void Message::SetTargetEntity( Entity *pEnt )
{
	 m_pTargetEntity = pEnt;
	 m_pTargetEntity->sig_onRemoved.connect(1, boost::bind(&Message::OnEntityDestroyed, this, _1));
}


void Message::OnEntityDestroyed( Entity *pEnt )
{
	//LogMsg("Entity destroyed (%s)", pEnt->GetName().c_str());
	if (GetType() == MESSAGE_TYPE_ADD_COMPONENT)
	{
		//we're holding on to a component, delete it from memory now
		SAFE_DELETE(m_pComponent);
	}
	
	m_pTargetEntity = NULL;
	m_pComponent = NULL;

}

void Message::OnComponentDestroyed( VariantList *pVList )
{
	m_pTargetEntity = NULL;
	m_pComponent = NULL;
}

void Message::SetTargetComponent( EntityComponent *pComp )
{
	assert(!m_pTargetEntity);
	m_pComponent = pComp;
	if (!pComp) return;
	m_pComponent->GetFunction("OnDelete")->sig_function.connect(1, boost::bind(&Message::OnComponentDestroyed, this, _1));
}

void Message::SetComponentToAdd( EntityComponent *pComp )
{
	assert(pComp && !pComp->GetParent());

	m_pComponent = pComp;
	m_pTargetEntity->sig_onRemoved.connect(1, boost::bind(&Message::OnEntityDestroyed, this, _1));
}

MessageManager::MessageManager()
{
}

MessageManager::~MessageManager()
{
	DeleteAllMessages();
}



void MessageManager::SendGame( eMessageType type, const string msg, int deliverTimeMS, eTimingSystem timing)
{
	Message *m = new Message(MESSAGE_CLASS_GAME, timing, type);
	m->SetVarName(msg);
	m->SetDeliveryTime(deliverTimeMS);
	Send(m);
}

void MessageManager::SendGame( eMessageType type, const Variant &v, int deliverTimeMS, eTimingSystem timing)
{
	Message *m = new Message(MESSAGE_CLASS_GAME, timing, type);
	m->Set(v);
	m->SetDeliveryTime(deliverTimeMS);
	Send(m);
}
void MessageManager::SendGUI( eMessageType type, float parm1, float parm2, int deliverTimeMS, eTimingSystem timing)
{
	Message *m = new Message(MESSAGE_CLASS_GUI, timing, type);
	m->SetParm1(parm1);
	m->SetParm2(parm2);
	m->SetParm3(0); //finger id unknown so..
	m->SetParm4(0); //alt/shift/etc unknown so
	m->SetDeliveryTime(deliverTimeMS);
	Send(m);
}

void MessageManager::SendGUIEx( eMessageType type, float parm1, float parm2, int finger, int deliverTimeMS, eTimingSystem timing)
{
	Message *m = new Message(MESSAGE_CLASS_GUI, timing, type);
	m->SetParm1(parm1);
	m->SetParm2(parm2);
	m->SetParm3(finger);
	m->SetDeliveryTime(deliverTimeMS);
	Send(m);
}

void MessageManager::SendGUIEx2( eMessageType type, float parm1, float parm2, int finger, uint32 modifiers, int deliverTimeMS, eTimingSystem timing)
{
	Message *m = new Message(MESSAGE_CLASS_GUI, timing, type);
	m->SetParm1(parm1);
	m->SetParm2(parm2);
	m->SetParm3(finger);
	m->SetParm4(modifiers);
	m->SetDeliveryTime(deliverTimeMS);
	Send(m);
}
void MessageManager::SendGUIStringEx( eMessageType type, float parm1, float parm2, int finger, string s, int deliverTimeMS, eTimingSystem timing)
{
	Message *m = new Message(MESSAGE_CLASS_GUI, timing, type);
	m->SetStringParm(s);
	m->SetParm1(parm1);
	m->SetParm2(parm2);
	m->SetParm3(finger);
	m->SetDeliveryTime(deliverTimeMS);
	Send(m);
}

void MessageManager::SendGUI( eMessageType type, const Variant &v, int deliverTimeMS)
{
	Message *m = new Message(MESSAGE_CLASS_GUI, TIMER_SYSTEM, type);
	m->Set(v);
	m->SetDeliveryTime(deliverTimeMS);
	Send(m);
}

void MessageManager::SendGUI( eMessageType type, const VariantList &vList, int deliverTimeMS /*= 0*/ )
{
	Message *m = new Message(MESSAGE_CLASS_GUI, TIMER_SYSTEM, type);
	m->Set(&vList);
	m->SetDeliveryTime(deliverTimeMS);
	Send(m);
}

void MessageManager::AddMessageToList(list <Message*> &messageList, Message *m)
{
	//insert sorted by delivery time
	list <Message*>::reverse_iterator itor = messageList.rbegin();
	while (itor != messageList.rend())
	{
		if ( (*itor)->GetDeliveryTime() > m->GetDeliveryTime())
		{
			itor++;
		} else
		{
			break;
		}
	}

	//add it
	messageList.insert(itor.base(), m);

	
}

void MessageManager::Send(Message *m)
{
/*
	if (m->GetType() != MESSAGE_TYPE_GUI_CLICK_MOVE_RAW)
	{
		LogMsg("Sending msg %d", m->GetType());
	}
	*/

	if (m->GetTimingMethod() == TIMER_GAME)
	{
		AddMessageToList(m_gameMessages, m);
		//LogMsg("Game messages: %d", m_gameMessages.size());
	} else
	{
		AddMessageToList(m_systemMessages, m);
		//LogMsg("System messages: %d", m_gameMessages.size());
	}
}


void MessageManager::DumpMessagesInList(list<Message*> m)
{
	list<Message*>::iterator itor = m.begin();

	string s;

	while (itor != m.end())
	{
		s.clear();
		if ( (*itor)->GetTimingMethod() == TIMER_GAME)
		{
			s += "Game: ";
		} else
		{
			s += "System: ";
		}

		s += " Delivery: " + toString( (*itor)->GetDeliveryTime());
		s += " Type: " + toString( (*itor)->GetType());
		s += " Parm1: " + toString( (*itor)->GetParm1());
		s += " Parm2: " + toString( (*itor)->GetParm2());

		LogMsg(s.c_str());
		itor++;
	}
}

//thing used for debugging
void MessageManager::DumpMessages()
{
	LogMsg("Dumping system messages...");
	DumpMessagesInList(m_gameMessages);
	LogMsg("Dumping game messages...");
	DumpMessagesInList(m_systemMessages);

}


void MessageManager::Deliver(Message *m)
{
	
	if (m->GetClass() == MESSAGE_CLASS_ENTITY)
	{
			if (m->GetTargetComponent())
			{
				switch (m->GetType())
				{
				case MESSAGE_TYPE_SET_ENTITY_VARIANT:
					m->GetTargetComponent()->GetVar(m->GetVarName())->Set(m->Get());
					break;

				case MESSAGE_TYPE_CALL_ENTITY_FUNCTION:
					m->GetTargetComponent()->GetShared()->CallFunctionIfExists(m->GetVarName(), &m->GetVariantList());
					break;

				case MESSAGE_TYPE_REMOVE_COMPONENT:
					//actually, we're targeting an entity but adding this component...
					m->GetTargetEntity()->AddComponent(m->GetTargetComponent());
					m->ClearComponent();

					break;
				default:
					LogError("Message delivery error");
					assert(0);
				}
		
			} else if (m->GetTargetEntity())
			{
				switch (m->GetType())
				{
			
				case MESSAGE_TYPE_CALL_COMPONENT_FUNCTION_BY_NAME:
					{

						EntityComponent *pComp = m->GetTargetEntity()->GetComponentByName(m->GetStringParm());
						if (pComp)
						{
							pComp->GetFunction(m->GetVarName())->sig_function(&m->GetVariantList());
						} else
						{
							LogMsg("Warning: Entity %s doesn't have a component named %s to call %s on", m->GetTargetEntity()->GetName().c_str(),
								m->GetStringParm().c_str(), m->GetVarName().c_str());
						}
					}
					break;

				
				case MESSAGE_TYPE_SET_ENTITY_VARIANT:
					m->GetTargetEntity()->GetVar(m->GetVarName())->Set(m->Get());
					break;

				case MESSAGE_TYPE_CALL_ENTITY_FUNCTION:
					m->GetTargetEntity()->GetFunction(m->GetVarName())->sig_function(&m->GetVariantList());
					break;

				case MESSAGE_TYPE_CALL_ENTITY_FUNCTION_RECURSIVELY:

					if (m->GetType() == MESSAGE_TYPE_GUI_CLICK_START)
					{
						//this function is sort of a hack to fake a mouse click, used with automation stuff for stress tests

						//fake out our touch tracker, will override the 11th finger touch..
						GetBaseApp()->GetTouch(C_MAX_TOUCHES_AT_ONCE-1)->SetWasHandled(false);
						GetBaseApp()->GetTouch(C_MAX_TOUCHES_AT_ONCE-1)->SetWasPreHandled(false);
						GetBaseApp()->GetTouch(C_MAX_TOUCHES_AT_ONCE-1)->SetIsDown(true);
						GetBaseApp()->GetTouch(C_MAX_TOUCHES_AT_ONCE-1)->SetPos(m->GetVariantList().Get(1).GetVector2());
					}

					m->GetTargetEntity()->CallFunctionRecursively(m->GetVarName(), &m->GetVariantList());

					break;

				case MESSAGE_TYPE_REMOVE_COMPONENT:
					m->GetTargetEntity()->RemoveComponentByName(m->GetVarName());
					break;
				default:
					LogError("Message delivery error");
					assert(0);
				}	//entity's shared space
			} else
			{
				switch (m->GetType())
				{
					case MESSAGE_TYPE_CALL_STATIC_FUNCTION:
						(m->GetFunctionPointer())(&m->GetVariantList());
						break;
				
					default:
						//LogError("Message delivery error");
						;
						//Not actually an error, if an entity is killed while a message is in transit, it will set the entity
						//to null and some messages may end up here as they are undeliverable
				}
			}
	} else
	{
		GetBaseApp()->OnMessage(*m);

	}
}

void MessageManager::Update()
{

	while (!m_systemMessages.empty() &&  (*m_systemMessages.begin())->GetDeliveryTime() <= GetBaseApp()->GetTick())
	{
		Message *m = *m_systemMessages.begin();
		m_systemMessages.pop_front();
		Deliver(m);
		delete m;
	}


	while (!m_gameMessages.empty() && (*m_gameMessages.begin())->GetDeliveryTime() <= GetBaseApp()->GetGameTick())
	{
		Message *m = *m_gameMessages.begin();
		m_gameMessages.pop_front();
		Deliver(m);
		delete m;
	}
}

void MessageManager::DeleteAllMessages()
{
	while (!m_systemMessages.empty())
	{
		Message *m = *m_systemMessages.begin();
		m_systemMessages.pop_front();
		delete m;
	}

	while (!m_gameMessages.empty())
	{
		Message *m = *m_gameMessages.begin();
		m_gameMessages.pop_front();
		delete m;
	}
}

void MessageManager::SetEntityVariable( Entity *pEnt, int timeMS, const string &varName, const Variant &v, eTimingSystem timing )
{
	Message *m = new Message(MESSAGE_CLASS_ENTITY, timing, MESSAGE_TYPE_SET_ENTITY_VARIANT);
	m->Set(v);
	m->SetVarName(varName);
	m->SetTargetEntity(pEnt);
	m->SetDeliveryTime(timeMS);
	Send(m);
}

void MessageManager::SetComponentVariable( EntityComponent *pComp, int timeMS, const string &varName,  const Variant &v, eTimingSystem timing)
{
	Message *m = new Message(MESSAGE_CLASS_ENTITY, timing, MESSAGE_TYPE_SET_ENTITY_VARIANT);
	m->Set(v);
	m->SetVarName(varName);
	m->SetTargetComponent(pComp);
	m->SetDeliveryTime(timeMS);
	Send(m);
}

void MessageManager::CallEntityFunction( Entity *pEnt, int timeMS, const string &funcName, const VariantList *v, eTimingSystem timing )
{
	Message *m = new Message(MESSAGE_CLASS_ENTITY, timing, MESSAGE_TYPE_CALL_ENTITY_FUNCTION);
	m->Set(v);
	m->SetVarName(funcName);
	m->SetTargetEntity(pEnt);
	m->SetDeliveryTime(timeMS);
	Send(m);
}

void MessageManager::CallStaticFunction(PtrFuncVarList pFunctionWithVList, int timeMS,  const VariantList *v, eTimingSystem timing )
{
	Message *m = new Message(MESSAGE_CLASS_ENTITY, timing, MESSAGE_TYPE_CALL_STATIC_FUNCTION);
	m->Set(v);
	m->SetFunctionPointer(pFunctionWithVList);
	m->SetDeliveryTime(timeMS);
	Send(m);
}

void MessageManager::CallEntityFunctionRecursively( Entity *pEnt, int timeMS, const string &funcName, const VariantList *v, eTimingSystem timing )
{
	Message *m = new Message(MESSAGE_CLASS_ENTITY, timing, MESSAGE_TYPE_CALL_ENTITY_FUNCTION);
	m->Set(v);
	m->SetVarName(funcName);
	m->SetTargetEntity(pEnt);
	m->SetDeliveryTime(timeMS);
	Send(m);
}

void MessageManager::CallComponentFunction( EntityComponent *pComp, int timeMS, const string &funcName, const VariantList *v, eTimingSystem timing )
{
	Message *m = new Message(MESSAGE_CLASS_ENTITY, timing, MESSAGE_TYPE_CALL_ENTITY_FUNCTION);
	m->Set(v);
	m->SetVarName(funcName);
	m->SetTargetComponent(pComp);
	m->SetDeliveryTime(timeMS);
	Send(m);
}


void MessageManager::CallComponentFunction( Entity *pEnt, const string &compName, int timeMS,const string &funcName, const VariantList *v, eTimingSystem timing )
{
	Message *m = new Message(MESSAGE_CLASS_ENTITY, timing, MESSAGE_TYPE_CALL_COMPONENT_FUNCTION_BY_NAME);

	m->SetVarName(funcName);
	m->Set(v);
	m->SetTargetEntity(pEnt);
	m->SetStringParm(compName);
	m->SetDeliveryTime(timeMS);
	Send(m);
}

void MessageManager::RemoveComponentByName( Entity *pEnt, int timeMS, const string &compName, eTimingSystem timing )
{
	Message *m = new Message(MESSAGE_CLASS_ENTITY, timing, MESSAGE_TYPE_REMOVE_COMPONENT);
	
	m->SetVarName(compName);
	m->SetTargetEntity(pEnt);
	m->SetDeliveryTime(timeMS);
	Send(m);
}

void MessageManager::AddComponent(Entity *pEnt, int timeMS, EntityComponent *pComp, eTimingSystem timing)
{
	Message *m = new Message(MESSAGE_CLASS_ENTITY, timing, MESSAGE_TYPE_REMOVE_COMPONENT);

	m->SetTargetEntity(pEnt);
	m->SetComponentToAdd(pComp);
	m->SetDeliveryTime(timeMS);
	Send(m);

}

void MessageManager::DeleteMessagesByFunctionCallName( const string &name, eTimingSystem timing )
{
	list<Message*> *pList = &m_gameMessages;
	
	if (timing == TIMER_SYSTEM) pList = &m_systemMessages;

	list<Message*>::iterator itor = pList->begin();

	while (itor != pList->end())
	{
		if ( (*itor)->GetVarName() == name)
		{
			delete (*itor);
			itor = pList->erase(itor);
			continue;
		}
		itor++;
	}
}


void MessageManager::DeleteMessagesToComponent( EntityComponent *pComponent)
{
	list<Message*> *pList = &m_gameMessages;

	list<Message*>::iterator itor = pList->begin();
	while (itor != pList->end())
	{
		if ( (*itor)->GetTargetComponent() == pComponent)
		{
			delete (*itor);
			itor = pList->erase(itor);
			continue;
		}
		itor++;
	}

	//also messages using the system timer
	pList = &m_systemMessages;

	itor = pList->begin();
	while (itor != pList->end())
	{
		if ( (*itor)->GetTargetComponent() == pComponent)
		{
			delete (*itor);
			itor = pList->erase(itor);
			continue;
		}
		itor++;
	}

}


void MessageManager::DeleteMessagesToEntity( Entity *pEntity)
{
	list<Message*> *pList = &m_gameMessages;

	list<Message*>::iterator itor = pList->begin();
	while (itor != pList->end())
	{
		if ( (*itor)->GetTargetEntity() == pEntity)
		{
			delete (*itor);
			itor = pList->erase(itor);
			continue;
		}
		itor++;
	}

	//also messages using the system timer
	pList = &m_systemMessages;

	itor = pList->begin();
	while (itor != pList->end())
	{
		if ( (*itor)->GetTargetEntity() == pEntity)
		{
			delete (*itor);
			itor = pList->erase(itor);
			continue;
		}
		itor++;
	}

}

void MessageManager::DeleteMessagesByType( eMessageType type, eTimingSystem timing  )
{
	list<Message*> *pList = &m_gameMessages;

	if (timing == TIMER_SYSTEM) pList = &m_systemMessages;

	list<Message*>::iterator itor = pList->begin();

	while (itor != pList->end())
	{
		if ( (*itor)->GetType() == type)
		{
			delete (*itor);
			itor = pList->erase(itor);
			continue;
		}
		itor++;
	}
}
