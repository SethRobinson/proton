//  ***************************************************************
//  GamepadProvider - Creation date: 01/27/2012
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2012 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef GamepadProvider_h__
#define GamepadProvider_h__

class GamepadProvider: public boost::signals::trackable
{
public:
	GamepadProvider();
	virtual ~GamepadProvider();
	virtual string GetName()=0;
	virtual bool Init()=0;
	virtual void Kill()=0;
	virtual void Update()=0;

	boost::signal<void (GamepadProvider*)> m_sig_failed_to_connect; //only used by iCade at the moment
	boost::signal<void (GamepadProvider*, VariantList*)> m_sig_status; //could be useful for sending pad plugged in/unplugged messages.  Not used yet

protected:
	

private:
};

#endif // GamepadProvider_h__