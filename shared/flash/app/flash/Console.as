/*
Main entry file for the flash side of Proton.  Handles input and system type stuff, and creates GLES1Adaptor and AudioManager objects to handle those parts.


(c) Seth A. Robinson: all rights reserved

*/

package com.adobe.flascc
{
	import flash.display.*;
	import flash.display3D.*;
	import flash.display3D.textures.Texture;
	import flash.text.TextField;
	import flash.net.LocalConnection;
	import flash.net.URLRequest;
	import flash.geom.*;
	import flash.utils.*;
	import flash.events.*;
	import flash.geom.Matrix3D;
	import flash.geom.Rectangle;
	import com.adobe.utils.*;
	import flash.system.*;
	import com.rtsoft.*;

	import com.adobe.flascc.vfs.HTTPBackingStore;
	import com.adobe.flascc.vfs.ISpecialFile;
	import com.adobe.flascc.vfs.LSOBackingStore;
	import com.adobe.flascc.vfs.RootFSBackingStore;
  
	public class Console extends Sprite implements ISpecialFile
	{
		public static var current:Console;
		
		public static var screenWidth:int;
		public static var screenHeight:int;
		
		// Stage3D related members
		public var context:Context3D;
	
		//temps used for passing vars, because I don't know a better way
		public var m_parmInt1:int = 0;
		public var m_parmInt2:int = 0;
		public var m_parmInt3:int = 0;
		public var m_parmFloat1:Number = 0;
		public var m_parmFloat2:Number = 0;
		public var m_audioManager:AudioManager;
		public var m_graphics:GLES1Adaptor;
		public var m_debug:Boolean = false;
		
	    private var webfs:HTTPBackingStore = null

		public function Console(container:DisplayObjectContainer = null)
		{
  		  CModule.rootSprite = container ? container.root : this

		    if(CModule.runningAsWorker()) 
		    {
				trace("Why are we running as a worker?!");
				return;
		    }

			 Console.current = this
		
			if (container)
			{
				container.addChild(this)
				initG(null);
			}
			else
			{
				addEventListener(Event.ADDED_TO_STAGE, initG);
			}
		}
		
		private function initG(e:Event):void
		{
			removeEventListener(Event.ADDED_TO_STAGE, initG);
			stage.frameRate = 60;
			stage.quality = "best";
			
		    CModule.vfs.console = this
			CModule.vfs.addBackingStore(new RootFSBackingStore(), null)
   			CModule.vfs.addDirectory("/local")

			screenWidth = stage.stageWidth;
			screenHeight = stage.stageHeight;
			
			trace("Setting screen to " + screenWidth + ", " + screenHeight);
			// wait for Stage3D to provide us a Context3D
			stage.stage3Ds[0].addEventListener(Event.CONTEXT3D_CREATE, __onCreate);
			stage.stage3Ds[0].requestContext3D();
		}
		
		
		private function __onCreate(event:Event):void
		{
			// // // CREATE CONTEXT // //
			context = stage.stage3Ds[0].context3D;
					
			// Configure the back buffer, in width and height. You can also specify the antialiasing
			// The backbuffer is the memory space where your final image is rendered.
			context.configureBackBuffer(screenWidth, screenHeight, 4, true);
			m_graphics = new GLES1Adaptor(context);
	
			m_audioManager = new AudioManager;
			
			trace("Telling system it's ok to init the Proton part");
			CModule.startAsync(this)
 	
			//m_native_SendGUIEx = CModule.getPublicSymbol("Native_sendGUIEx");
			//trace("Public sym got" + m_native_sendGUIEx);
			
			// start the rendering loop
			addEventListener(Event.ENTER_FRAME, render);
			stage.addEventListener(MouseEvent.MOUSE_DOWN, OnMouseDown);
			stage.addEventListener(MouseEvent.MOUSE_UP, OnMouseUp);
			stage.addEventListener(MouseEvent.MOUSE_MOVE, OnMouseMove);
			stage.addEventListener(KeyboardEvent.KEY_DOWN, OnKeyDown);
			stage.addEventListener(KeyboardEvent.KEY_UP, OnKeyUp);
			stage.addEventListener(Event.DEACTIVATE, lostFocus);
			stage.addEventListener(Event.ACTIVATE, gainFocus);
		}
		
		private static const MESSAGE_TYPE_GUI_CLICK_START:int = 0;
		private static const MESSAGE_TYPE_GUI_CLICK_END:int = 1;
		private static const MESSAGE_TYPE_GUI_CLICK_MOVE:int = 2; //only send when button/finger is held down
		private static const MESSAGE_TYPE_GUI_CLICK_MOVE_RAW:int = 3; //the raw mouse move messages
		private static const MESSAGE_TYPE_GUI_ACCELEROMETER:int = 4;
		private static const MESSAGE_TYPE_GUI_TRACKBALL:int = 5;
		private static const MESSAGE_TYPE_GUI_CHAR:int = 6;
		
		private static const MESSAGE_TYPE_GUI_COPY:int = 7;
		private static const MESSAGE_TYPE_GUI_PASTE:int = 8;
		private static const MESSAGE_TYPE_GUI_TOGGLE_FULLSCREEN:int = 9;
		private static const MESSAGE_TYPE_SET_ENTITY_VARIANT:int = 10;
		private static const MESSAGE_TYPE_CALL_ENTITY_FUNCTION:int = 11;
		private static const MESSAGE_TYPE_CALL_COMPONENT_FUNCTION_BY_NAME:int = 12;
		private static const MESSAGE_TYPE_PLAY_SOUND:int = 13;
		private static const MESSAGE_TYPE_VIBRATE:int = 14;
		private static const MESSAGE_TYPE_REMOVE_COMPONENT:int = 15;
		private static const MESSAGE_TYPE_ADD_COMPONENT:int = 16;
		private static const MESSAGE_TYPE_OS_CONNECTION_CHECKED:int = 17;
		private static const MESSAGE_TYPE_PLAY_MUSIC:int = 18;
		private static const MESSAGE_TYPE_UNKNOWN:int = 19;
		private static const MESSAGE_TYPE_PRELOAD_SOUND:int = 20;
		private static const MESSAGE_TYPE_GUI_CHAR_RAW:int = 21;
		
		public function LaunchURL(urlToLaunch:String): void
		{
			var url:URLRequest = new URLRequest(urlToLaunch);
			navigateToURL(url, "_blank");
		}

		private function SendGUIEx(msg:int, parm1:Number, parm2:Number, finger:int):void
		{
			var args:Vector.<int> = new Vector.<int>;
			m_parmInt1 = msg;
			m_parmInt2 = 0; //touchID
			m_parmFloat1 = parm1;
			m_parmFloat2 = parm2;
			
			CModule.callI(CModule.getPublicSymbol("Native_SendGUIEx"), args);
		}
		
		/**
    * The callback to call when FlasCC code calls the posix exit() function. Leave null to exit silently.
    * @private
    */
    public var exitHook:Function;

    /**
    * The PlayerKernel implementation will use this function to handle
    * C process exit requests
    */
    public function exit(code:int):Boolean
    {
      // default to unhandled
      return exitHook ? exitHook(code) : false;
    }


		public function SetDebugMode(debug:Boolean)
		{
			if (m_debug != debug)
			{
				m_debug = debug;
				m_audioManager.SetDebugMode(m_debug);
				m_graphics.SetDebugMode(m_debug);
			}
		}
		private function OnMouseDown(e:MouseEvent):void
		{
			SendGUIEx(MESSAGE_TYPE_GUI_CLICK_START, e.stageX, e.stageY, 0);
		}
		
		private function OnMouseUp(e:MouseEvent):void
		{
			SendGUIEx(MESSAGE_TYPE_GUI_CLICK_END, e.stageX, e.stageY, 0);
		}
		
		private function OnMouseMove(e:MouseEvent):void
		{
			SendGUIEx(MESSAGE_TYPE_GUI_CLICK_MOVE_RAW, e.stageX, e.stageY, 0);
			
			if (e.buttonDown)
			{
				SendGUIEx(MESSAGE_TYPE_GUI_CLICK_MOVE, e.stageX, e.stageY, 0);
			}
		}
		
		public function calculateUsedBytes():int
		{
			return System.totalMemory;
		}
		
		private function OnKeyDown(event:KeyboardEvent):void
		{
			//trace("keyPressed " + event.keyCode);
			SendGUIEx(MESSAGE_TYPE_GUI_CHAR_RAW, event.keyCode, 1, 0);
			//also send as normal keystroke
			SendGUIEx(MESSAGE_TYPE_GUI_CHAR, event.charCode, 1, 0);
		}
		
		private function OnKeyUp(event:KeyboardEvent):void
		{
			SendGUIEx(MESSAGE_TYPE_GUI_CHAR_RAW, event.keyCode, 0, 0);
		}
		
		private function gainFocus(event:Event):void
		{
			trace("Game received keyboard focus.");
			var args:Vector.<int> = new Vector.<int>;
			CModule.callI(CModule.getPublicSymbol("Native_OnGotFocus"), args);
		}
		
		private function lostFocus(event:Event):void
		{
			trace("Game lost keyboard focus.");
			var args:Vector.<int> = new Vector.<int>;
			CModule.callI(CModule.getPublicSymbol("Native_OnLostFocus"), args);
		}
		
		private function render(event:Event):void
		{
			CModule.serviceUIRequests()
			var args:Vector.<int> = new Vector.<int>;
			CModule.callI(CModule.getPublicSymbol("Native_Update"), args);
			CModule.callI(CModule.getPublicSymbol("Native_Render"), args);
			context.present(); // render the backbuffer on screen.
		}
	
    /**
    * The PlayerKernel implementation will use this function to handle
    * C IO write requests to the file "/dev/tty" (e.g. output from
    * printf will pass through this function). See the ISpecialFile
    * documentation for more information about the arguments and return value.
    */
    public function write(fd:int, bufPtr:int, nbyte:int, errnoPtr:int):int
    {
      var str:String = CModule.readString(bufPtr, nbyte)
      trace(str)
      return nbyte
    }

    /**
    * The PlayerKernel implementation will use this function to handle
    * C IO read requests to the file "/dev/tty" (e.g. reads from stdin
    * will expect this function to provide the data). See the ISpecialFile
    * documentation for more information about the arguments and return value.
    */
    public function read(fd:int, bufPtr:int, nbyte:int, errnoPtr:int):int
    {
      return 0
    }

    /**
    * The PlayerKernel implementation will use this function to handle
    * C fcntl requests to the file "/dev/tty" 
    * See the ISpecialFile documentation for more information about the
    * arguments and return value.
    */
    public function fcntl(fd:int, com:int, data:int, errnoPtr:int):int
    {
      return 0
    }

    /**
    * The PlayerKernel implementation will use this function to handle
    * C ioctl requests to the file "/dev/tty" 
    * See the ISpecialFile documentation for more information about the
    * arguments and return value.
    */
    public function ioctl(fd:int, com:int, data:int, errnoPtr:int):int
    {
      return 0
    }

	
	}
}
