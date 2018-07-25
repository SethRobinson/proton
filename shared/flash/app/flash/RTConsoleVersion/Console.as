/*
Main entry file for the flash side of Proton.  Handles input and system type stuff.

Doesn't include the audiomanager or Seth's GLAdaptor, this is for console only stuff, as
demonstrated in the RTConsole example.  (Can only print boring old text)

(c) Seth A. Robinson: all rights reserved
*/

package com.adobe.flascc
{
	import flash.display.DisplayObjectContainer;
	import flash.display.Sprite;
	import flash.display.StageScaleMode;
	import flash.events.Event;
	import flash.net.LocalConnection;
	import flash.net.URLRequest;
	import flash.text.TextField;
	import flash.utils.ByteArray;

	import com.adobe.flascc.vfs.HTTPBackingStore;
	import com.adobe.flascc.vfs.ISpecialFile;
	import com.adobe.flascc.vfs.LSOBackingStore;
	import com.adobe.flascc.vfs.RootFSBackingStore;
 
	public class Console extends Sprite implements ISpecialFile
	{
		public static var current:Console;
		public static var screenWidth:int;
		public static var screenHeight:int;
		
		//temps used for passing vars, because I don't know a better way
		public var m_parmInt1:int = 0;
		public var m_parmInt2:int = 0;
		public var m_parmInt3:int = 0;
		public var m_parmFloat1:Number = 0;
		public var m_parmFloat2:Number = 0;
		private var enableConsole:Boolean = true
		private var _tf:TextField
	    private var inputContainer:DisplayObjectContainer

	    private var alclogo:String = "FlasCC"
	    private var webfs:HTTPBackingStore = null

		public var m_debug:Boolean = false;
		
		public function Console(container:DisplayObjectContainer = null)
		{
		
			  Console.current = this
			  CModule.rootSprite = container ? container.root : this

			  if(container)
			   {
				container.addChild(this)
				init(null)
			  } else 
			  {
				addEventListener(Event.ADDED_TO_STAGE, init)
			  }
		}
		
		private function init(e:Event):void
		{
		  inputContainer = new Sprite()
		  addChild(inputContainer)
	
		  addEventListener(Event.ENTER_FRAME, enterFrame)

		  stage.frameRate = 60
		  stage.scaleMode = StageScaleMode.NO_SCALE

		  if(enableConsole) 
		  {
			_tf = new TextField
			_tf.multiline = true
			_tf.width = stage.stageWidth
			_tf.height = stage.stageHeight 
			inputContainer.addChild(_tf)
		  }

		  // PlayerKernel will delegate read/write requests to the "/dev/tty"
		  // file to the object specified with this API.
		  CModule.vfs.console = this
		  CModule.vfs.addBackingStore(new RootFSBackingStore(), null)
		 
		  CModule.vfs.addDirectory("/local")

		 /*
		  CModule.vfs.addBackingStore(
		  new LSOBackingStore("samplevfsLSO"), "/local")
		  var ba:ByteArray = new ByteArray()
		  ba.writeUTFBytes(alclogo)
		
		  webfs = new HTTPBackingStore()
		  webfs.addEventListener(Event.COMPLETE, onComplete)
		  trace("Finished init");
	    */
		
		//instead of the web download thing, let's assume we have it all now.
		 CModule.startAsync(this)
 		}
		
	  private function onComplete(e:Event):void 
	  {
	  //not actually used, as this event isn't triggered
      CModule.vfs.addDirectory("/web")
      CModule.vfs.addBackingStore(webfs, "/web")
      CModule.startAsync(this)
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
			
			CModule.callFun(CModule.getPublicSym("Native_SendGUIEx"), args);
		}
		
		public function SetDebugMode(debug:Boolean)
		{
			if (m_debug != debug)
			{
				m_debug = debug;
			}
		}
	
		public function calculateUsedBytes():int
		{
			return System.totalMemory;
		}
	
		private function gainFocus(event:Event):void
		{
			trace("Game received keyboard focus.");
			var args:Vector.<int> = new Vector.<int>;
			CModule.callFun(CModule.getPublicSym("Native_OnGotFocus"), args);
		}
		
		private function lostFocus(event:Event):void
		{
			trace("Game lost keyboard focus.");
			var args:Vector.<int> = new Vector.<int>;
			CModule.callFun(CModule.getPublicSym("Native_OnLostFocus"), args);
		}
		
		private function render(event:Event):void
		{
			var args:Vector.<int> = new Vector.<int>;
			CModule.callFun(CModule.getPublicSym("Native_Update"), args);
			CModule.callFun(CModule.getPublicSym("Native_Render"), args);
			
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
      consoleWrite(str)
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

    /**
    * Helper function that traces to the flashlog text file and also
    * displays output in the on-screen textfield Console.
    */
    protected function consoleWrite(s:String):void
    {
     // trace(s)
      if(enableConsole) {
        _tf.appendText(s)
        _tf.scrollV = _tf.maxScrollV
      }
    }

    /**
    * The enterFrame callback will be run once every frame. UI thunk requests should be handled
    * here by calling CModule.serviceUIRequests() (see CModule ASdocs for more information on the UI thunking functionality).
    */
    protected function enterFrame(e:Event):void
    {
      CModule.serviceUIRequests()
    }

    /**
    * Provide a way to get the TextField's text.
    */
    public function get consoleText():String
    {
        var txt:String = null;

        if(_tf != null){
            txt = _tf.text;
        }
        
        return txt;
    }
	}
}
