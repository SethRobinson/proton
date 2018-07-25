//This is shared between all android p+ projects so be careful!

package ${PACKAGE_NAME};
import ${PACKAGE_NAME}.SharedActivity;
import ${PACKAGE_NAME}.AppGLSurfaceView;

import android.os.Build;
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
import java.util.List;
import java.util.LinkedList;
import java.util.ListIterator;
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


//tricks for being compatible with Android 1.5 but still being able to use new features of 2.2
public class SharedMultiTouchInput
{

	static class TouchInfo
	{
	  public int pointerID;
	  int fingerID;
	}

	public static SharedActivity app;
	static LinkedList <TouchInfo> listTouches;
	public static void init(SharedActivity _app)
	{
		app = _app;
		listTouches =  new LinkedList<TouchInfo>();
	}

	public static int GetNextAvailableFingerID()
	{
		int fingerID = 0;
		
		while ( fingerID <  12)
		{
			Boolean bOk = true;
			ListIterator<TouchInfo> iterator = listTouches.listIterator(); 
	
			while (iterator.hasNext())
			{
				TouchInfo t = iterator.next();
				if (fingerID == t.fingerID)
				{
					bOk = false;
					break;
				}
			}
			
			if (bOk)
			{
				return fingerID;
			}
			
			//guess we failed, try again
			fingerID ++;
		}
		
		
		return fingerID;
	}

	public static int GetFingerByPointerID(int pointerID)
	{
		ListIterator<TouchInfo> iterator = listTouches.listIterator(); 
		
		while (iterator.hasNext())
		{
			TouchInfo t = iterator.next();
			if (pointerID == t.pointerID)
			{
			//	Log.d("", "Returning..."+t.fingerID);
				return t.fingerID;
			}
		}
		
		//Log.d("", "Adding "+pointerID+" (finger "+listTouches.size());
		//add it?
		final TouchInfo t = new TouchInfo();
		t.pointerID = pointerID;
		t.fingerID = GetNextAvailableFingerID();
		
		listTouches.add(t);
		return t.fingerID;
	}
	
	public static void RemoveFinger(int pointerID)
	{
		ListIterator<TouchInfo> iterator = listTouches.listIterator(); 
		
		while (iterator.hasNext())
		{
			TouchInfo t = iterator.next();
			if (pointerID == t.pointerID)
			{
			//	Log.d("", "removing "+t.pointerID+" touch: "+t.fingerID);
				iterator.remove();
				return;
			}
		}
	
	   //Log.d("", "Failed to remove "+pointerID);
	}
	
	public static void processMouse(int msg, float x, float y, int id)
	{
	//we can't just send the id, it cannot be used as a "fingerID" as it could be 100 or more in certain circumstances on an
	//xperia.  We'll do our own finger track abstraction here before we send it to Proton
	
		int fingerID = GetFingerByPointerID(id);
		
		if (msg == MotionEvent.ACTION_UP)
		{
				RemoveFinger(id);
		}
	
		AppGLSurfaceView.nativeOnTouch(msg, x, y, fingerID);
	}


//Based on code from http://stackoverflow.com/questions/5860879/android-motionevent-getactionindex-and-multitouch

    public static boolean OnInput(final MotionEvent event)
	{
	 int p = event.getActionIndex();
     switch(event.getActionMasked())
     {
        case MotionEvent.ACTION_DOWN:
        case MotionEvent.ACTION_POINTER_DOWN:
            processMouse(MotionEvent.ACTION_DOWN, event.getX(p), event.getY(p), event.getPointerId(p));                
        break;              
        case MotionEvent.ACTION_POINTER_UP:                 
        case MotionEvent.ACTION_UP:
            processMouse(MotionEvent.ACTION_UP, event.getX(p), event.getY(p), event.getPointerId(p));
            break;
        case MotionEvent.ACTION_MOVE:
         
         
            final int pointerCount = event.getPointerCount();
         
         /*
         //if we wanted to buffer all movement, we should add a way to enable this.. I disable it because at low FPS it would
         //cause horrible lag of events we wouldn't care about, but for a paint program you'd want it. -Seth
            final int historySize = event.getHistorySize();
            for (int h = 0; h < historySize; h++)
             {
                 for (int p1 = 0; p1 < pointerCount; p1++)
                  {
                    processMouse(MotionEvent.ACTION_MOVE, event.getHistoricalX(p1, h), event.getHistoricalY(p1, h), event.getPointerId(p1));
                 }
            }
           
           */
            for (int p1 = 0; p1 < event.getPointerCount(); p1++)
             {

                processMouse(MotionEvent.ACTION_MOVE, event.getX(p1), event.getY(p1), event.getPointerId(p1));
            }
            break;
            
            case MotionEvent.ACTION_CANCEL:
			{
			//Log.d(":", " ACTION_CANCEL!");
            //This almost never happens... but really, I guess we should look at our active touch list and send button ups fr
            //each one before destroying the list
			listTouches.clear();
			}
			break;
	
    }
    return true;
 }

}
