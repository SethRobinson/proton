//This is shared between all android p+ projects so be careful!
//Thanks to Phil Hassey for his help and code

package com.rtsoft.shared;
import com.rtsoft.shared.SharedMultiTouchInput;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.app.Activity;
import android.os.SystemClock;
import android.text.ClipboardManager;
import android.app.AlertDialog;
import android.app.Dialog;
import android.os.Build;
import android.os.Environment;

import android.content.DialogInterface;
import android.widget.Button;
import android.widget.TextView;
import android.provider.Settings.Secure;

import android.content.Context;
import android.view.KeyEvent;
import android.view.inputmethod.InputMethodManager;
import android.opengl.GLSurfaceView;
import android.graphics.PixelFormat;

import android.os.Bundle;
import android.os.Vibrator;
import android.view.MotionEvent;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.IOException;
import android.util.Log;
import android.content.pm.ActivityInfo;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.AssetFileDescriptor;
import android.content.res.AssetManager;

// Licensing Server
import com.android.vending.licensing.AESObfuscator;
import com.android.vending.licensing.LicenseChecker;
import com.android.vending.licensing.LicenseCheckerCallback;
import com.android.vending.licensing.ServerManagedPolicy;
import com.android.vending.licensing.StrictPolicy;

import java.util.List;
import java.io.File;
import java.io.FileOutputStream;

import android.media.MediaPlayer;
import android.content.res.AssetFileDescriptor;
import android.media.SoundPool;
import android.media.AudioManager;
import java.util.Locale;

import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;


public class SharedActivity extends Activity implements SensorEventListener
{

	//********** THESE WILL BE OVERRIDDEN IN YOUR Main.java file **************
	public static String PackageName= "com.rtsoft.something";
	public static String dllname= "rtsomething";
	public static boolean securityEnabled = false; //if false, it won't try to use the online license stuff
	public static boolean bIsShuttingDown = false;
	//************************************************************************


	public static SharedActivity app; //a global to use in static functions with JNI
	
	//for the accelerometer
	private static float accelHzSave     = 0;
    private static Sensor sensor;
    private static SensorManager sensorManager;
	private static float m_lastMusicVol = 1;
	public static int apiVersion;
	
    ////////////////////////////////////////////////////////////////////////////
    // Licensing Server code
    ////////////////////////////////////////////////////////////////////////////
        public boolean is_demo = false;
        public String BASE64_PUBLIC_KEY = "this will be set in your app's Main.java";
    
        //20 random bytes.  You can override these in your own Main.java
        public byte[] SALT = new byte[] {
            24, -96, 16, 91, 65, -86, -54, -73, -101, 12, -84, -90, -53, -68, 20, -67, 45, 35, 85, 17
        };
    
        private LicenseCheckerCallback mLicenseCheckerCallback;
        private LicenseChecker mChecker;
        
        private class MyLicenseCheckerCallback implements LicenseCheckerCallback
		{
            public void allow() 
			{
                Log.v("allow()","Allow the user access");
                
                if (isFinishing()) 
				{
                    // Don't update UI if Activity is finishing.
                    return;
                }
                // Should allow user access.
//                 displayResult(getString(R.string.allow));
            }
    
            public void dontAllow() 
			{
                Log.v("dontAllow()","Don't allow the user access");
                
                is_demo = true;
                
                if (isFinishing()) 
				{
                    // Don't update UI if Activity is finishing.
                    return;
                }
//                 displayResult(getString(R.string.dont_allow));
                // Should not allow access. In most cases, the app should assume
                // the user has access unless it encounters this. If it does,
                // the app should inform the user of their unlicensed ways
                // and then either shut down the app or limit the user to a
                // restricted set of features.
                // In this example, we show a dialog that takes the user to Market.
                showDialog(0);
            }
    
            public void applicationError(ApplicationErrorCode errorCode)
			{
                String result = String.format("Application error: %1$s", errorCode);
                Log.v("applicationError",result);
                
                dontAllow();
            
                if (isFinishing()) 
				{
                    // Don't update UI if Activity is finishing.
                    return;
                }
                // This is a polite way of saying the developer made a mistake
                // while setting up or calling the license checker library.
                // Please examine the error code and fix the error.
//                 displayResult(result);
            }
        }
        
        protected Dialog onCreateDialog(int id) 
		{
            // We have only one dialog.
            return new AlertDialog.Builder(this)
                .setTitle("Application not licensed")
                .setMessage("This application is not licensed.  Please purchase it from Android Market.\n\nTip: if you have purchased this application, press Retry a few times.  It may take a minute to connect to the licensing server.  If that does not work, try rebooting your phone.")
                .setPositiveButton("Buy app", new DialogInterface.OnClickListener() 
				{
                    public void onClick(DialogInterface dialog, int which)
					{
                        Intent marketIntent = new Intent(Intent.ACTION_VIEW, Uri.parse(
                            "http://market.android.com/details?id=" + getPackageName()));
                        startActivity(marketIntent);
                     
						app.finish();
						android.os.Process.killProcess(android.os.Process.myPid());
                    }
                })
                .setNegativeButton("Exit", new DialogInterface.OnClickListener()
				{
                    public void onClick(DialogInterface dialog, int which)
					{
                      
					app.finish();
					android.os.Process.killProcess(android.os.Process.myPid());
                    }
                })
                .setNeutralButton("Retry", new DialogInterface.OnClickListener()
				{
                    public void onClick(DialogInterface dialog, int which)
					{
                        is_demo = false;
                        doCheck();
                    }
                })
                .create();
        }
    
        private void doCheck() 
		{
    //         mCheckLicenseButton.setEnabled(false);
    //         setProgressBarIndeterminateVisibility(true);
    //         mStatusText.setText(R.string.checking_license);
            mChecker.checkAccess(mLicenseCheckerCallback);
        }
        
        private void license_init() 
		{
                // Try to use more data here. ANDROID_ID is a single point of attack.
            String deviceId = Secure.getString(getContentResolver(), Secure.ANDROID_ID);
    
            // Library calls this when it's done.
            mLicenseCheckerCallback = new MyLicenseCheckerCallback();
            // Construct the LicenseChecker with a policy.
            mChecker = new LicenseChecker(
                this,
                new ServerManagedPolicy(this,new AESObfuscator(SALT, getPackageName(), deviceId)),
//                 new StrictPolicy(),
                BASE64_PUBLIC_KEY);
            doCheck();
        }
    ////////////////////////////////////////////////////////////////////////////



	@Override
    protected void onCreate(Bundle savedInstanceState) 
	{
        app = this;
		apiVersion = Build.VERSION.SDK_INT;
		
		Log.d(PackageName, "SDK version: " + apiVersion);
				
		System.loadLibrary(dllname);
				
		super.onCreate(savedInstanceState);
        mGLView = new AppGLSurfaceView(this, this);
     	
		setContentView(mGLView);
		setVolumeControlStream(AudioManager.STREAM_MUSIC);
		if (securityEnabled) this.license_init();
    }

    @Override
    protected synchronized void onPause()
	{
		float hzTemp = accelHzSave;
	    setup_accel(0); //disable accelerator messages to save on CPU/battery
		accelHzSave = hzTemp;
        mGLView.onPause();
        super.onPause();
		//_sounds.autoPause();
	
    }

    @Override
	protected synchronized void onResume()
	{
		//_sounds.autoResume();
	
		music_set_volume(m_lastMusicVol); //android forgets the audio level, need to reset it
        mGLView.onResume();
		setup_accel(accelHzSave);
		super.onResume();
  
	}
 
  // JNI used to get Save data dir
    public static String get_docdir() 
	{
		File fdir = app.getFilesDir();
     	return fdir.getAbsolutePath();
    }
 	
	public static String get_externaldir()
	{
		//directory of external storage
	boolean mExternalStorageAvailable = false;
	boolean mExternalStorageWriteable = false;

		String state = Environment.getExternalStorageState();
    if (Environment.MEDIA_MOUNTED.equals(state))
	{
        mExternalStorageAvailable = mExternalStorageWriteable = true;
    } else if (Environment.MEDIA_MOUNTED_READ_ONLY.equals(state)) 
	{
        mExternalStorageAvailable = true;
        mExternalStorageWriteable = false;
    } else 
	{
       // mExternalStorageAvailable = mExternalStorageWriteable = false;
    }

	if (mExternalStorageWriteable == false) return "";


	return Environment.getExternalStorageDirectory().toString();
	}

	// JNI used to get Save data dir
    public static String get_apkFileName() 
	{
		String apkFilePath = null;
			ApplicationInfo appInfo = null;
			PackageManager packMgmr = app.getPackageManager();
			try {
			appInfo = packMgmr.getApplicationInfo(PackageName, 0);
		} catch (NameNotFoundException e) 
		{
			e.printStackTrace();
			throw new RuntimeException("Unable to locate assets, aborting...");
		}
			return appInfo.sourceDir;	
	 }

	public static String get_region()
	{
		//will return region in the format "en_us"
		Locale l = java.util.Locale.getDefault();    
		return (l.getLanguage()+"_"+l.getCountry()).toLowerCase();
	}

	public static String get_clipboard()
	{
		ClipboardManager clipboard = (ClipboardManager) app.getSystemService(CLIPBOARD_SERVICE);
		String data = clipboard.getText().toString();
		return data;
	}
	public static String get_deviceID()
	{
		String m_szDevIDShort = "35" + //we make this look like a valid IMEI
            Build.BOARD.length()%10+ Build.BRAND.length()%10 +
            Build.CPU_ABI.length()%10 + Build.DEVICE.length()%10 +
            Build.DISPLAY.length()%10 + Build.HOST.length()%10 +
            Build.ID.length()%10 + Build.MANUFACTURER.length()%10 +
            Build.MODEL.length()%10 + Build.PRODUCT.length()%10 +
            Build.TAGS.length()%10 + Build.TYPE.length()%10 +
            Build.USER.length()%10 ; //13 digits

if (app.checkCallingOrSelfPermission("android.permission.READ_PHONE_STATE") == PackageManager.PERMISSION_GRANTED)
{


TelephonyManager tm = (TelephonyManager) app.getSystemService(Context.TELEPHONY_SERVICE);
final String DeviceId, SerialNum;
DeviceId = tm.getDeviceId();
SerialNum = tm.getSimSerialNumber();
return m_szDevIDShort + DeviceId + SerialNum;
} else
{
return m_szDevIDShort;
}
	}

  @Override
	public void onSensorChanged(SensorEvent event) 
	{
     		
        switch (event.sensor.getType())
		{
            case Sensor.TYPE_ACCELEROMETER:
				if (event.values.length < 3) return; //who knows what this is
				//		Log.d(PackageName, "Accel: " + "x:"+Float.toString(event.values[0]) + " y:"+Float.toString(event.values[1]) + " z:"+Float.toString(event.values[2]));
   				nativeOnAccelerometerUpdate(event.values[0], event.values[1], event.values[2]);
				break;

	        case Sensor.TYPE_ORIENTATION:
			
			//Log.d(PackageName, "Orientation: " + "x:"+Float.toString(event.values[0]));
			break;
        }
    }
	
  @Override
	public void onAccuracyChanged(Sensor sensor, int accuracy)
	{
    	//Log.d(PackageName,"onAccuracyChanged: " + sensor + ", accuracy: " + accuracy);
    }

	public void setup_accel(float hz) //0 to disable
	{
	  	accelHzSave = hz;
		sensorManager = (SensorManager) app.getSystemService(Context.SENSOR_SERVICE);
		
		sensorManager.unregisterListener(this);

		if (hz > 0)
		{
			sensorManager.registerListener(app, sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER), sensorManager.SENSOR_DELAY_GAME);
		}
		
		//if we ever need orientation data, do this:
		//sensorManager.registerListener(app, sensorManager.getDefaultSensor(Sensor.TYPE_ORIENTATION), sensorManager.SENSOR_DELAY_GAME);
	
	}
	
	/**
     * The listener that listen to events from the accelerometer listener
     */
    
	    // JNI to open_url
    public static void LaunchURL(String url)
	{
        Intent intent = new Intent(Intent.ACTION_VIEW);
        intent.setData(Uri.parse(url));
        app.startActivity(intent);
    }

	public static void create_dir_recursively(String basepath, String path)
	{
     //Log.v("create_dir_recursively", "base: "+basepath+" path: "+path);
		String finalPath = basepath+path;
		File myDir = new File(finalPath);
		myDir.mkdirs();
	}

	  public void toggle_keyboard(boolean show) 
	  {
        Log.v("Msg", "Toggling keyboard");
		 InputMethodManager mgr = (InputMethodManager)app.getSystemService(Context.INPUT_METHOD_SERVICE);
        mgr.hideSoftInputFromWindow (mGLView.getWindowToken(),0);
        if (show) 
		{
            Log.v("Msg","Enabling keyboard");
			mgr.toggleSoftInput(InputMethodManager.SHOW_FORCED,0);
            // On the Nexus One, SHOW_FORCED makes it impossible
            // to manually dismiss the keyboard.
            // On the Droid SHOW_IMPLICIT doesn't bring up the keyboard.
        }
    }

	//from MessageManager.h
	final static int VIRTUAL_KEY_BACK = 500000;
	final static int VIRTUAL_KEY_PROPERTIES = 500001;
	final static int VIRTUAL_KEY_HOME = 500002;
	final static int VIRTUAL_KEY_SEARCH = 500003;
	final static int VIRTUAL_KEY_DIR_UP = 500004;
	final static int VIRTUAL_KEY_DIR_DOWN = 500005;
	final static int VIRTUAL_KEY_DIR_LEFT = 500006;
	final static int VIRTUAL_KEY_DIR_RIGHT = 500007;
	final static int VIRTUAL_KEY_DIR_CENTER = 500008;
	final static int VIRTUAL_KEY_VOLUME_UP = 500009;
	final static int VIRTUAL_KEY_VOLUME_DOWN = 500010;
	final static int VIRTUAL_KEY_SHIFT = 500011;
	

	
	final static int VIRTUAL_KEY_TRACKBALL_DOWN = 500035;
	final static int VIRTUAL_DPAD_BUTTON_LEFT = 500036; //square on xperia
	final static int VIRTUAL_DPAD_BUTTON_UP = 500037; //triangle on xperia
	final static int VIRTUAL_DPAD_BUTTON_RIGHT = 500038; //O
	final static int VIRTUAL_DPAD_BUTTON_DOWN = 500039; //X
	final static int VIRTUAL_DPAD_SELECT = 500040;
	final static int VIRTUAL_DPAD_START = 500041;
	final static int VIRTUAL_DPAD_LBUTTON = 500042;
	final static int VIRTUAL_DPAD_RBUTTON = 500043;

//messages we could call on Proton using nativeSendGUIEx:
	final static int MESSAGE_TYPE_GUI_CLICK_START = 0;
	final static int MESSAGE_TYPE_GUI_CLICK_END = 1;
	final static int MESSAGE_TYPE_GUI_CLICK_MOVE = 2; //only send when button/finger is held down
	final static int MESSAGE_TYPE_GUI_CLICK_MOVE_RAW = 3; //win only, the raw mouse move messages
	final static int MESSAGE_TYPE_GUI_ACCELEROMETER = 4;
	final static int MESSAGE_TYPE_GUI_TRACKBALL = 5;
	final static int MESSAGE_TYPE_GUI_CHAR = 6; //the input box uses it on windows since we don't have a virtual keyboard
	final static int MESSAGE_TYPE_GUI_COPY = 7;
	final static int MESSAGE_TYPE_GUI_PASTE = 8;
	final static int MESSAGE_TYPE_GUI_TOGGLE_FULLSCREEN = 9;

	final static int MESSAGE_TYPE_SET_ENTITY_VARIANT = 10;
	final static int MESSAGE_TYPE_CALL_ENTITY_FUNCTION = 11;
	final static int MESSAGE_TYPE_CALL_COMPONENT_FUNCTION_BY_NAME = 12;
	final static int MESSAGE_TYPE_PLAY_SOUND = 13;
	final static int MESSAGE_TYPE_VIBRATE = 14;
	final static int MESSAGE_TYPE_REMOVE_COMPONENT = 15;
	final static int MESSAGE_TYPE_ADD_COMPONENT = 16;
	final static int MESSAGE_TYPE_OS_CONNECTION_CHECKED = 17; //sent by macOS, will send an eOSSTreamEvent as parm1
	final static int MESSAGE_TYPE_PLAY_MUSIC = 18;
	final static int MESSAGE_TYPE_UNKNOWN = 19;
	final static int MESSAGE_TYPE_PRELOAD_SOUND = 20;
	final static int MESSAGE_TYPE_GUI_CHAR_RAW = 21;
	final static int MESSAGE_TYPE_SET_SOUND_ENABLED = 22;
	
	final static int MESSAGE_TYPE_TAPJOY_AD_READY = 23;
	final static int MESSAGE_TYPE_TAPJOY_FEATURED_APP_READY = 24;
	final static int MESSAGE_TYPE_TAPJOY_MOVIE_AD_READY = 25;

	//GOOGLE BILLING
	final static int MESSAGE_TYPE_IAP_RESULT = 26;
	final static int MESSAGE_TYPE_IAP_ITEM_STATE = 27;

	//more tapjoy stuff
	final static int MESSAGE_TYPE_TAPJOY_TAP_POINTS_RETURN = 28;
	final static int MESSAGE_TYPE_TAPJOY_TAP_POINTS_RETURN_ERROR = 29;
	final static int MESSAGE_TYPE_TAPJOY_SPEND_TAP_POINTS_RETURN = 30;
	final static int MESSAGE_TYPE_TAPJOY_SPEND_TAP_POINTS_RETURN_ERROR = 31;
	final static int MESSAGE_TYPE_TAPJOY_AWARD_TAP_POINTS_RETURN = 32;
	final static int MESSAGE_TYPE_TAPJOY_AWARD_TAP_POINTS_RETURN_ERROR = 33;
	final static int MESSAGE_TYPE_TAPJOY_EARNED_TAP_POINTS = 34;
	
	final static int MESSAGE_USER = 1000; //send your own messages after this #
	
	

	public int TranslateKeycodeToProtonVirtualKey(int keycode)
	{

		switch (keycode)
		{
			case KeyEvent.KEYCODE_BACK:
				keycode = VIRTUAL_KEY_BACK;
				break;
			case KeyEvent.KEYCODE_MENU:
				keycode = VIRTUAL_KEY_PROPERTIES;
				break;
			case KeyEvent.KEYCODE_SEARCH:
				keycode = VIRTUAL_KEY_SEARCH;
				break;
			case KeyEvent.KEYCODE_DPAD_DOWN:
				keycode = VIRTUAL_KEY_DIR_DOWN;
				break;
			case KeyEvent.KEYCODE_DPAD_UP:
				keycode = VIRTUAL_KEY_DIR_UP;
				break;
			case KeyEvent.KEYCODE_DPAD_LEFT:
				keycode = VIRTUAL_KEY_DIR_LEFT;
				break;
			case KeyEvent.KEYCODE_DPAD_RIGHT:
				keycode = VIRTUAL_KEY_DIR_RIGHT;
				break;
			case KeyEvent.KEYCODE_DPAD_CENTER:
				keycode = VIRTUAL_KEY_DIR_CENTER;
				break;
			case 0:
				keycode = VIRTUAL_KEY_SHIFT;
				break;

			case KeyEvent.KEYCODE_VOLUME_UP:
				keycode = VIRTUAL_KEY_VOLUME_UP;
				break;
			case KeyEvent.KEYCODE_VOLUME_DOWN:
				keycode = VIRTUAL_KEY_VOLUME_DOWN;
				break;
		}

		return keycode;
		
	}
	  // single touch version, works starting API4/??
    public boolean onTrackballEvent (MotionEvent e)
	{
		if (e.getAction() == MotionEvent.ACTION_MOVE)
	  {
   		nativeOnTrackball(e.getX(),e.getY());
	 	//Log.d("Hey", "trackball x rel: "+e.getX()+" y rel: "+e.getY());
	    return true; //signal that we handled it, so its messages don't show up as normal directional presses
	  } else if (e.getAction() == MotionEvent.ACTION_DOWN)
	{
		  //they pushed the button
		//Log.d("Hey", "Trackball button pushed");
		  nativeOnKey(1, VIRTUAL_KEY_TRACKBALL_DOWN, VIRTUAL_KEY_TRACKBALL_DOWN); 
	  }
   		
		return false; 
	}

	 public boolean onKeyDown(int keycode, KeyEvent e) 
	 {
  //     Log.v("onKeyDown","Keydown Got "+keycode+" "+Character.toString(Character.toChars(e.getUnicodeChar())[0]));
   
	
		if (e.getRepeatCount() > 0) return super.onKeyDown(keycode, e);

		if(e.isAltPressed() && keycode == KeyEvent.KEYCODE_BACK) 
		{
			//XPeria's O button, not the back button!
			//Log.v("onKeyDown","Sending xperia back key");
			nativeOnKey(1, VIRTUAL_DPAD_BUTTON_RIGHT, e.getUnicodeChar()); 
		
			return true; //signal that we handled it
		}
		
        switch (keycode)
		{
			case KeyEvent.KEYCODE_BACK:
			{
				//Log.v("onKeyDown","Sending virtual back");
				nativeOnKey(1, VIRTUAL_KEY_BACK, e.getUnicodeChar()); 
				return true; //signal that we handled it
			}
		}
		
		int vKey = TranslateKeycodeToProtonVirtualKey(keycode);
		nativeOnKey(1, vKey,e.getUnicodeChar()); //1  means keydown
        return super.onKeyDown(keycode, e);
    }

    public boolean onKeyUp(int keycode, KeyEvent e)
	{
    //    Log.v("onKeyUp","Keyup Got "+keycode+" "+Character.toString(Character.toChars(e.getUnicodeChar())[0]));
       
       	if(e.isAltPressed() && keycode == KeyEvent.KEYCODE_BACK) 
		{
			//XPeria's O button, not the back button!
			//Log.v("onKeyDown","Sending xperia back key");
			nativeOnKey(0, VIRTUAL_DPAD_BUTTON_DOWN, e.getUnicodeChar()); 
		
			return true; //signal that we handled it
		}
       
		switch (keycode)
		{
			case KeyEvent.KEYCODE_BACK:
			{
				nativeOnKey(0, VIRTUAL_KEY_BACK, e.getUnicodeChar()); //0 is type keyup
				return true; //signal that we handled it
			}
		}

      	int vKey = TranslateKeycodeToProtonVirtualKey(keycode);
	
      	nativeOnKey(0, vKey,e.getUnicodeChar()); //0 is type keyup
        return super.onKeyUp(keycode, e);
    }
    
	//************** SOUND STUFF *************

	// JNI to play music, etc
	public MediaPlayer _music = null;
	
	private static class MusicFadeOutThread extends Thread {
		private final int m_duration;

		public MusicFadeOutThread(int duration)
		{
			super();
			m_duration = duration;
		}

		public void run()
		{
			final int volumeChangeInterval = 100;  // change volume every this amount of ms
			final int totalSteps = m_duration / volumeChangeInterval;
			int remainingSteps = totalSteps;
			
			while (remainingSteps > 0)
			{
				synchronized (app._music)
				{
					float phase = (float)remainingSteps / (float)totalSteps;
					app._music.setVolume(phase * m_lastMusicVol, phase * m_lastMusicVol);
					remainingSteps--;
				}
				
				try {
					Thread.sleep(volumeChangeInterval);
				} catch (InterruptedException ie)
				{
					return;
				}
			}

			synchronized (app._music)
			{
				app._music.stop();
				app._music.setVolume(m_lastMusicVol, m_lastMusicVol);
			}
		}
	}
	
	private MusicFadeOutThread musicFadeOutThread = null;

	public synchronized static void music_play(String fname, boolean looping)
	{
		if (app._music != null)
		{
			app._music.reset();
		} else
		{
			app._music = new MediaPlayer();
		}
		
		if (fname.charAt(0) == '/')
		{
			//load as raw, not an asset
			try { 
				app._music.setDataSource(fname);
				app._music.setLooping(looping);
				app._music.prepare();
				music_set_volume(m_lastMusicVol);
				app._music.start();
			} catch(IOException e) 
			{ 
				Log.v("Can't load music (raw)", fname);
			}
			catch(IllegalStateException e) 
			{ 
				Log.v("Can't load music (raw), illegal state", fname);
				app._music.reset();
			}
			return;
		}

		AssetManager am = app.getAssets();
		try { 
			AssetFileDescriptor fd = am.openFd(fname);
			app._music.setDataSource(fd.getFileDescriptor(),fd.getStartOffset(),fd.getLength());
			fd.close();
			app._music.setLooping(looping);
			app._music.prepare();
			music_set_volume(m_lastMusicVol);
			app._music.start();
		} catch(IOException e) 
		{ 
			Log.v("Can't load music", fname);
		}
		catch(IllegalStateException e) 
		{ 
			Log.v("Can't load music, illegal state", fname);
			app._music.reset();
		}
	}

	public synchronized static void music_stop() 
	{
		if (app._music == null) { return; }
		
		if (app.musicFadeOutThread != null && app.musicFadeOutThread.isAlive()) {
			try {
				app.musicFadeOutThread.interrupt();
				app.musicFadeOutThread.join();
			} catch (InterruptedException ie)
			{
			}
		}
		
		app._music.stop();
	}
	
	public synchronized static void music_fadeout(int duration)
	{
		if (app._music == null || !app._music.isPlaying())
		{
			return;
		}
		
		if (duration <= 0)
		{
			music_stop();
		} else if (app.musicFadeOutThread == null || !app.musicFadeOutThread.isAlive())
		{
			app.musicFadeOutThread = new MusicFadeOutThread(duration);
			app.musicFadeOutThread.start();
		}
	}

	public synchronized static void music_set_volume(float v) 
	{
		if (app._music == null) { return; }
		m_lastMusicVol = v;
		app._music.setVolume(v,v);
	}

    public synchronized static void vibrate(int vibMS) 
	{
        Vibrator v = (Vibrator) app.getSystemService(Context.VIBRATOR_SERVICE);
		v.vibrate(vibMS);
    }

    public synchronized static int music_get_pos() 
	{
        if (app._music == null) { return 0; }
        return app._music.getCurrentPosition();
    }
  
	public synchronized static boolean music_is_playing() 
	{
        if (app._music == null) { return false; }
        return app._music.isPlaying();
    }

    public synchronized static void music_set_pos(int positionMS) 
	{
        if (app._music == null) 
		{
			Log.v("warning: music_set_position:", "no music playing, can't set position");
			return; 
		}
		app._music.seekTo(positionMS);
    }

	// JNI to play sounds
    public SoundPool _sounds = new SoundPool(8,AudioManager.STREAM_MUSIC,0);
    
	public synchronized static void sound_init()
	{
		if (app._sounds == null)
		{
			app._sounds = new SoundPool(8,AudioManager.STREAM_MUSIC,0);
		}
	}

	public synchronized static void sound_destroy()
	{
		if (app._sounds != null)
		{
			app._sounds.release();
			app._sounds = null;
		}
	}

	public synchronized static int sound_load(String fname) 
	{
       // Log.v("sound_load",fname);
       
		if (fname.charAt(0) == '/')
		{
			//must be a raw file on the disc, not in the assets.  load differently
			int sid = app._sounds.load(fname, 1);
			return sid;
	   }
		
		AssetManager am = app.getAssets();
        try { 
            AssetFileDescriptor fd = am.openFd(fname);
            int sid = app._sounds.load(fd.getFileDescriptor(),fd.getStartOffset(),fd.getLength(),1);
            return sid;
        } catch(IOException e)
		{
			Log.v("Can't load sound", fname);
		}
        return 0;
    }

    public synchronized static int sound_play(int soundID, float leftVol, float rightVol, int priority, int loop, float speedMod ) 
	{
	//	Log.v("MSG", "Playing sound: "+soundID);
	//	Log.v("Sound vol:", ""+leftVol);
        return app._sounds.play(soundID,leftVol,rightVol,priority,loop,speedMod);
    }
  
	public static void sound_kill(int soundID ) 
	{
	//	Log.v("MSG", "killing sound: "+soundID);
       app._sounds.unload(soundID);
    }

	  public static void sound_stop(int streamID ) 
	{
		//Log.v("MSG", "stopping sound: "+streamID);
		//Log.v("Sound vol:", ""+leftVol);
        app._sounds.stop(streamID);
    }

	public static void sound_set_vol(int streamID, float left, float right ) 
	{
		//Log.v("MSG", "setting sound vol: "+streamID);
		//Log.v("Sound vol:", ""+left);
		app._sounds.setVolume(streamID, left, right);
	}
	
	public static void sound_set_rate(int streamID, float rate ) 
	{
		//Log.v("MSG", "Playing sound: "+soundID);
		//Log.v("Sound vol:", ""+leftVol);
		app._sounds.setRate(streamID, rate);
	}

	//****************************************

	public GLSurfaceView mGLView;
	private static native void nativeOnKey(int type, int keycode, int c);
	private static native void nativeOnTrackball(float x, float y);
	private static native void nativeLaunchURL();
	private static native void nativeOnAccelerometerUpdate(float x, float y, float z);

	static 
    {
//        System.loadLibrary(dllname);
    }
}

class AppGLSurfaceView extends GLSurfaceView
{
    public AppGLSurfaceView(Context context, SharedActivity _app) 
	{
    	super(context);
        app = _app;
	
		//setEGLConfigChooser(8, 8, 8, 8, 16, 0);
		//getHolder().setFormat(PixelFormat.TRANSLUCENT);
		
		mRenderer = new AppRenderer(_app);
   
		setRenderer(mRenderer);
	
		  /* establish whether the "new" class is available to us */
  
	 try
	   {
		   WrapSharedMultiTouchInput.checkAvailable(app);
           mMultiTouchClassAvailable = true;
       } catch (Throwable t) 
	   {
           mMultiTouchClassAvailable = false;
       }
	}
    	
	public void onPause() 
    {
        super.onPause();

		if (app.bIsShuttingDown == false)
		{
			nativePause();   
		}
    }
    
	public void onResume() 
    {
        super.onResume();

		if (app.bIsShuttingDown == false)
		{
			nativeResume();   
		}
   }
   
		
    // single touch version, works starting API4/??
    public synchronized boolean onTouchEvent(final MotionEvent e)
	{
      	
		if (app.is_demo)
		{
			app.showDialog(0);
		}
		 
		if (mMultiTouchClassAvailable) 
		{
			return WrapSharedMultiTouchInput.OnInput(e);
		} else
		{
			float x = e.getX(); float y = e.getY();
     		int finger = 0; //planning ahead for multi touch
			nativeOnTouch(e.getAction(), x,y,finger);
		}
		return true;
    }
    

	AppRenderer mRenderer;

	private static native void nativePause();
	private static native void nativeResume();
	public static native void nativeOnTouch(int action, float x, float y, int finger);
	public SharedActivity app;
	
	private static boolean mMultiTouchClassAvailable;

	
  
 
}


class WrapSharedMultiTouchInput
{
	private SharedMultiTouchInput mInstance;

	/* class initialization fails when this throws an exception */
   static 
   {
       try
	   {
          Class.forName("com.rtsoft.shared.SharedMultiTouchInput");
       } catch (Exception ex) 
	   {
           throw new RuntimeException(ex);
       }
   }

   /* calling here forces class initialization */
   public static void checkAvailable(SharedActivity _app)
   {
	   SharedMultiTouchInput.init(_app);
   }


   public static boolean OnInput(final MotionEvent e)
   {
       return SharedMultiTouchInput.OnInput(e);
   }

}

class AppRenderer implements GLSurfaceView.Renderer 
{
    
	public AppRenderer(SharedActivity _app)
	{
		app = _app;
	}

	public void onSurfaceCreated(GL10 gl, EGLConfig config)
    {
    }

    public void onSurfaceChanged(GL10 gl, int w, int h) 
    {
        //gl.glViewport(0, 0, w, h);
        nativeResize(w, h);
        nativeInit();
    }
	
	final static int MESSAGE_NONE = 0;
	final static int MESSAGE_OPEN_TEXT_BOX = 1;
	final static int MESSAGE_CLOSE_TEXT_BOX = 2;
	final static int MESSAGE_CHECK_CONNECTION = 3;
	final static int MESSAGE_SET_FPS_LIMIT = 4;
	final static int MESSAGE_SET_ACCELEROMETER_UPDATE_HZ = 5;
	final static int MESSAGE_FINISH_APP = 6; //only respected by windows and android right now.  webos and iphone don't really need it
	final static int MESSAGE_SET_VIDEO_MODE = 7; 

	final static int MESSAGE_TAPJOY_GET_FEATURED_APP = 8; 
	final static int MESSAGE_TAPJOY_GET_AD = 9; 
	final static int MESSAGE_TAPJOY_GET_MOVIE = 10; 

	final static int MESSAGE_TAPJOY_SHOW_FEATURED_APP = 11; 
	final static int MESSAGE_TAPJOY_SHOW_AD = 12; 
	final static int MESSAGE_TAPJOY_SHOW_MOVIE = 13; 
	
	final static int MESSAGE_IAP_PURCHASE = 14;
	
	final static int MESSAGE_IAP_GET_PURCHASED_LIST = 15;


	static long m_gameTimer = 0;
	static int m_timerLoopMS = 0; //every this MS, the loop runs.  0 for no fps limit

    public synchronized void onDrawFrame(GL10 gl)
    {
		if (m_timerLoopMS != 0)
		{
			while (m_gameTimer > SystemClock.uptimeMillis() || m_gameTimer > SystemClock.uptimeMillis()+m_timerLoopMS+1)
			{
		
				//wait a bit - no exception catch needed for the SystemClock version of sleep
				SystemClock.sleep(1); 
			} 
		
			m_gameTimer = SystemClock.uptimeMillis()+m_timerLoopMS;
		}
		
	
		nativeUpdate(); //maybe later we'll want to adjust this for performance reasons..
        nativeRender();

		//let's process OS messages sent from the app if any exist
		int type = MESSAGE_NONE;

		while ( (type = nativeOSMessageGet()) != 0) //it returns 0 if none is available
		{
			switch (type)
			{
				case MESSAGE_OPEN_TEXT_BOX: //open text box
					app.toggle_keyboard(true);
					break;

				case MESSAGE_CLOSE_TEXT_BOX: //close text box
					app.toggle_keyboard(false);
					break;
		
				case MESSAGE_SET_ACCELEROMETER_UPDATE_HZ: 
					app.setup_accel(nativeGetLastOSMessageX());
				break;

				case MESSAGE_SET_FPS_LIMIT: 
					if (nativeGetLastOSMessageX() == 0)
					{
						//disable it, and avoid a div by 0
						m_timerLoopMS = 0;
					} else
					{
						m_timerLoopMS = (int) (1000.0f/nativeGetLastOSMessageX());
					}
				
				//app.setup_accel(nativeGetLastOSMessageX());
				break;

				case MESSAGE_FINISH_APP: 
					Log.v(app.PackageName, "Finishing app from java side");
					app.bIsShuttingDown = true;
					nativeDone();
					app.finish();
					android.os.Process.killProcess(android.os.Process.myPid());
				break;

				default:

					Log.v("Unhandled","Unhandled OS message");
			}
			 
		}

    }

    private static native void nativeInit();
    private static native void nativeResize(int w, int h);
    private static native void nativeUpdate();
    private static native void nativeRender();
    private static native void nativeDone();
	
	//yes, I probably should do this as a Java class and init it from C++ and send that over but..

	private static native int nativeOSMessageGet();
	private static native int nativeGetLastOSMessageParm1();
	private static native float nativeGetLastOSMessageX();
	private static native String nativeGetLastOSMessageString();
	public SharedActivity app;


}
