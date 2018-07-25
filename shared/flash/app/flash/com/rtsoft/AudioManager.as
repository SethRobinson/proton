package com.rtsoft
{
	/*

   AudioManager.as

   Driven by the C++ AudioManagerFlash, this uses Flash's native audio system to play mp3s.

   When sounds are not available yet, it handles loading them and retroactively making the soundID assigned valid, to stop or control the sound instance later.

   (c) Seth A. Robinson 2012:  All Rights reserved

 */
 
	import flash.media.SoundChannel;
	import flash.media.Sound;
	import flash.net.*;
	
	import flash.utils.ByteArray
	import flash.events.Event;
	import C_Run.ram;
	import flash.utils.*;
	import flash.geom.Matrix3D;
	import com.adobe.utils.*;
	import flash.system.*;
	import org.audiofx.mp3.MP3FileReferenceLoader;
	import org.audiofx.mp3.MP3SoundEvent;
		
	
	public class AudioManager
	{
		
		private var m_AudioDict:Dictionary = new Dictionary(); //keep track of the loaded Sound objects
		private var m_AudioGenCounter:int = 0;
		
		private var m_AudioInstanceDict:Dictionary = new Dictionary(); //keep track of the individual SoundChannels that are used to play
		private var m_AudioInstanceCounter:int = 0;
		private var m_AudioInstanceLookupDict:Dictionary = new Dictionary(); //use the key of the SoundChannel to locate the m_AudioInstanceDict num key
		
		public static var current:AudioManager;
		public var m_debug:Boolean = false;
		
		public function AudioManager()
		{
			AudioManager.current = this
		}
		
		public function GenAudio():int
		{
			m_AudioGenCounter++;
			//	trace("Creating audio buffer " +m_AudioGenCounter);
			
			return m_AudioGenCounter;
		}
		
		public function DeleteSoundInstance(soundInstance:int):void
		{
			if (m_AudioInstanceDict[soundInstance])
			{
				m_AudioInstanceDict[soundInstance].stop();
				delete m_AudioInstanceLookupDict[m_AudioInstanceDict[soundInstance]];
				
				//		trace("Deleting soundinstance " + soundInstance);
				delete m_AudioInstanceDict[soundInstance];
			}
			else
			{
				//		trace("Can't stop/kill sound instance " + soundInstance + ", alread dead");
			}
		
		}
		
		public function OnSoundChannelAssign(chanID:int, chan:SoundChannel):void
		{
			//trace("OnSoundChannelAssign Got " + chanID);
			m_AudioInstanceDict[m_AudioInstanceCounter] = chan;
			m_AudioInstanceLookupDict[chan] = chanID;
			chan.addEventListener(Event.SOUND_COMPLETE, soundCompleteHandler);
		}
		
		public function LoadAudioWav(audioID:int, audioPtr:int, byteSize:int, audioType:int):int
		{
			//	trace("Flash: Loading audio into buffer " +audioID);
			ram.position = audioPtr;
			var tempWave:ByteArray = new ByteArray;
			tempWave.length = byteSize;
			tempWave.writeBytes(ram, audioPtr, byteSize);
			m_AudioDict[audioID] = new MP3FileReferenceLoader();
			
			try
			{
				m_AudioDict[audioID].getSoundBA(tempWave);
			}
			catch (errObject:Error)
			{
				trace("Error loading audio file (Are you sure it's a 44khz stereo mp3?)  : " + errObject.message);
			}
			
			return 1;
		}
		
		public function PlayAudio(audioID:int, pos:int, loopCount:int):int
		{
			m_AudioInstanceCounter++;
			
			m_AudioInstanceDict[m_AudioInstanceCounter] = m_AudioDict[audioID].play(pos, loopCount, null, m_AudioInstanceCounter);
			
			if (m_AudioInstanceDict[m_AudioInstanceCounter])
			{
				m_AudioInstanceLookupDict[m_AudioInstanceDict[m_AudioInstanceCounter]] = m_AudioInstanceCounter;
				m_AudioInstanceDict[m_AudioInstanceCounter].addEventListener(Event.SOUND_COMPLETE, soundCompleteHandler);
				
			}
			else
			{
				//	trace("Sound " + audioID + " not loaded yet, can't return channel");
				m_AudioDict[audioID].SetChannelAssignCallback(OnSoundChannelAssign);
			}
			
			//	trace("Created sound instance " + m_AudioInstanceCounter);
			return m_AudioInstanceCounter;
		}
		
		private function soundCompleteHandler(event:Event):void
		{
			var soundChan:SoundChannel = SoundChannel(event.target);
			var instanceID:int = m_AudioInstanceLookupDict[soundChan];
			//	trace("soundCompleteHandler: " + event + " instance id is " + instanceID);
			DeleteSoundInstance(instanceID);
		}
		
		public function StopAudio(audioID:int)
		{
			m_AudioDict[audioID].stop();
		}
		
		public function DeleteAudio(audioID:int)
		{
			trace("Flash uncaching audio sample " + audioID);
			delete m_AudioDict[audioID];
		}

		public function SetDebugMode(debug:Boolean)
		{
			m_debug = debug;
		}	
	}
}
