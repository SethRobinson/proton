//This is shared between all android p+ projects so be careful!
//Thanks to Phil Hassey for his help and code

package ${PACKAGE_NAME};
import ${PACKAGE_NAME}.SharedMultiTouchInput;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.app.Activity;
import android.os.SystemClock;
import android.text.ClipboardManager;
import android.app.AlertDialog;
import android.app.Dialog;
import android.os.Build;
import android.os.Environment;
import java.util.Map;
import java.util.HashMap;
import java.util.ArrayList;
import org.json.JSONObject;
import android.content.DialogInterface;
import android.widget.Button;
import android.widget.TextView;
import android.widget.EditText;
import android.provider.Settings.Secure;
import android.view.ViewGroup;
import android.content.Context;
import android.view.KeyEvent;
import android.view.inputmethod.InputMethodManager;
import android.opengl.GLSurfaceView;
import android.graphics.PixelFormat;

import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.view.View.OnKeyListener;
import android.text.InputType;
import android.view.inputmethod.BaseInputConnection;
import android.text.Editable;
import android.text.Selection;
import android.view.KeyCharacterMap;
import android.view.ActionMode;
import android.view.Menu;
import android.text.InputFilter;
import android.view.MenuItem;
import android.text.TextWatcher;
import android.view.View.OnFocusChangeListener;
import android.graphics.Color;


import android.os.Bundle;
import android.os.Vibrator;
import android.view.MotionEvent;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import java.io.File;
import java.io.FileInputStream;
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
import android.content.pm.PackageInfo;


//#if defined(RT_GOOGLE_SERVICES_SUPPORT)
import com.google.android.gms.ads.identifier.AdvertisingIdClient;
import com.google.android.gms.ads.identifier.AdvertisingIdClient.Info;
import com.google.android.gms.auth.GooglePlayServicesAvailabilityException;
import com.google.android.gms.common.GooglePlayServicesNotAvailableException;
import com.google.android.gms.common.GooglePlayServicesRepairableException;
//#endif

// Wifi
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;

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

import android.os.Handler;
import android.view.View;
import android.view.ViewGroup.LayoutParams;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;

//#if defined(RT_TAPJOY_SUPPORT)
	
//tapjoy publisher V10.02 supported, the new .jar format, not the old raw source version
import com.tapjoy.TapjoyAwardPointsNotifier;
import com.tapjoy.TapjoyConnect;
import com.tapjoy.TapjoyConnectFlag;
import com.tapjoy.TapjoyConstants;
import com.tapjoy.TapjoyDisplayAdNotifier;
import com.tapjoy.TapjoyEarnedPointsNotifier;
import com.tapjoy.TapjoyFullScreenAdNotifier;
import com.tapjoy.TapjoyLog;
import com.tapjoy.TapjoyNotifier;
import com.tapjoy.TapjoySpendPointsNotifier;
import com.tapjoy.TapjoyVideoNotifier;
import com.tapjoy.TapjoyVideoStatus;
import com.tapjoy.TapjoyViewNotifier;
import com.tapjoy.TapjoyViewType;

//#endif

//#if defined(RT_CHARTBOOST_SUPPORT)
import com.chartboost.sdk.Chartboost;
import com.chartboost.sdk.ChartboostDelegate;
import com.chartboost.sdk.Analytics.CBAnalytics;
//#endif

//#if defined(RT_FLURRY_SUPPORT)
import com.flurry.android.FlurryAgent;
//#endif

//#if defined(RT_DISABLE_IAP_SUPPORT)
//#else
//Android in app billing
import ${PACKAGE_NAME}.util.IabHelper;
import ${PACKAGE_NAME}.util.IabResult;
import ${PACKAGE_NAME}.util.Inventory;
import ${PACKAGE_NAME}.util.Purchase;
//#endif

//#if defined(RT_HOOKED_SUPPORT)
//for hooked/wasabi
	import com.hookedmediagroup.wasabi.WasabiApi;
//#endif

import android.view.View.OnClickListener;
import android.net.ConnectivityManager;


//#if defined(RT_APPSFLYER_ENABLED)
// Adding appsflyer support.
import com.appsflyer.AppsFlyerLib;
import com.appsflyer.AFInAppEventParameterName;
import com.appsflyer.AFInAppEventType;
//#endif


//#if defined(RT_TAPJOY_SUPPORT)
	public class SharedActivity extends Activity implements SensorEventListener,  TapjoyNotifier, TapjoyFullScreenAdNotifier, TapjoySpendPointsNotifier, TapjoyDisplayAdNotifier, TapjoyAwardPointsNotifier, TapjoyEarnedPointsNotifier, TapjoyVideoNotifier 
//#else
	public class SharedActivity extends Activity implements SensorEventListener
//#endif
{

	//********** THESE WILL BE OVERRIDDEN IN YOUR Main.java file **************
	public static String PackageName= "com.rtsoft.something";
	public static String dllname= "rtsomething";
	public static boolean securityEnabled = false; //if false, it won't try to use the online license stuff
	public static boolean bIsShuttingDown = false;
	public static boolean IAPEnabled = false; //if false, IAB won't be initted.  I call it IAP because I'm used to it from iOS land
	
	public static String tapBannerSize = ""; //tapjoy banner size text, set in Main.cpp, or by AdManager calls
	public static int adBannerWidth = 0;
	public static int adBannerHeight = 0;
	
	public static String m_iap_asap = "";
	public static String m_iap_sync_purchases_asap = "";
	public static String m_iap_consume_asap = "";
	public static String m_iap_developerdata = "";
	
	public static String m_advertiserID = ""; //only will be set it Google Services is added to the project.   This is a big hassle for Proton projects,
	//I ended up just copying over the res and jar manually because android.library.reference.1= in project.properties didn't seem to let the manifest access the .res and.. argh.
	public static boolean m_limitAdTracking = false;

	public static Inventory m_iap_inventory = null;
	public static boolean m_focusOnKeyboard = false;
	public static boolean m_focusOffKeyboard = false;
	
//#if defined(RT_FLURRY_SUPPORT)
	public static String m_flurryAPIKey = "";	
//#endif

	public static boolean HookedEnabled = false;
	//************************************************************************
    static final int RC_REQUEST = 10001;


	public static SharedActivity app = null; //a global to use in static functions with JNI
	
	//for the accelerometer
	private static float accelHzSave     = 0;
    private static Sensor sensor;
    private static SensorManager sensorManager;
	private static float m_lastMusicVol = 1;
	public static int apiVersion;
	
	//TAPJOY
	
	public static View adView;
	public static RelativeLayout adLinearLayout;
	public static EditText m_editText;
	public static String m_before="";
	public static int m_text_max_length = 20;
	public static String m_text_default = "";
	public static boolean update_display_ad;
	public static boolean run_hooked;
	public static int tapjoy_ad_show; //0 for don't shot, 1 for show
	
//#if defined(RT_CHARTBOOST_SUPPORT)
	public static boolean cb_cacheInterstitial = false;
	public static boolean cb_showInterstitial = false;
	// Create the Chartboost object
    public static Chartboost cb;
	public static String m_chartBoostAppID = "";	
	public static String m_chartBoostAppSig = "";	
	public static boolean cb_performLogon = false;

//#endif
	
	public static boolean set_allow_dimming_asap = false;
	public static boolean set_disallow_dimming_asap = false;

//#if defined(RT_DISABLE_IAP_SUPPORT)
//#else	
	//GOOGLE IAB
   // The helper object
   IabHelper mHelper;
   
 
	
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
				//displayResult(getString(R.string.allow));
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
				//displayResult(getString(R.string.dont_allow));
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
				//displayResult(result);
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
			//mCheckLicenseButton.setEnabled(false);
			//setProgressBarIndeterminateVisibility(true);
			//mStatusText.setText(R.string.checking_license);
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
//										new StrictPolicy(),
										BASE64_PUBLIC_KEY);
            doCheck();
        }
    ////////////////////////////////////////////////////////////////////////////
	final Handler mMainThreadHandler = new Handler();

	@Override
	protected void onDestroy()
     {
     	Log.d(PackageName, "Destroying...");
		
        super.onDestroy();
//#if defined(RT_DISABLE_IAP_SUPPORT)
//#else
         Log.d(PackageName, "Destroying helper.");
        if (mHelper != null) mHelper.dispose();
        mHelper = null;
//#endif
    }


    
    @Override
    protected void onStart() 
    {
        super.onStart();
//#if defined(RT_CHARTBOOST_SUPPORT)
		this.cb.onStart(this);
//#endif

    }



    @Override
    protected void onStop()
    {
        super.onStop();

//#if defined(RT_CHARTBOOST_SUPPORT)
	 this.cb.onStop(this);
//#endif
    }
	
	
	@Override
	public void onBackPressed() 
{
	//#if defined(RT_CHARTBOOST_SUPPORT)
    // If an interstitial is on screen, close it. Otherwise continue as normal.
    
    if (this.cb.onBackPressed())
        return;
    else
    //#endif
    
        super.onBackPressed();
}

    void alert(String message)
     {
        AlertDialog.Builder bld = new AlertDialog.Builder(this);
        bld.setMessage(message);
        bld.setNeutralButton("OK", null);
        Log.d(PackageName, "Showing alert dialog: " + message);
        bld.create().show();
    }
	 void complain(String message)
	 {
        Log.e(PackageName, "Initialization error: " + message);
        alert("Error: " + message);
     }
    
	@Override
    protected void onCreate(Bundle savedInstanceState) 
	{
        app = this;
		apiVersion = Build.VERSION.SDK_INT;
	    Log.d(PackageName, "***********************************************************************");
		Log.d(PackageName, "API Level: " + apiVersion);
				
		super.onCreate(savedInstanceState);
        mGLView = new AppGLSurfaceView(this, this);
    	
		setContentView(mGLView);
	  
	  //if (apiVersion > 15) //use new input system because Google sucks.  Using on all not just newer now
	  {
	  
	   Log.d(app.PackageName, "Setting up new input system");
      
	  m_editText = new EditText(this);
	  m_editText.setText("");
      m_editText.setImeOptions(EditorInfo.IME_FLAG_NO_FULLSCREEN|EditorInfo.IME_FLAG_NO_EXTRACT_UI| EditorInfo.IME_FLAG_FORCE_ASCII);
	  
    m_editText.setOnFocusChangeListener(new OnFocusChangeListener() {          
 @Override
        public void onFocusChange(View v, boolean hasFocus) {
            if(!hasFocus)
            {
              // Log.d(app.PackageName, "Edittext lost focus");
            } else
            {
              // Log.d(app.PackageName, "Edittext got focus");
      
            }
        }
    });
       
   
	m_editText.setInputType(InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS | InputType.TYPE_TEXT_VARIATION_VISIBLE_PASSWORD);
 	
	try
		{
        	m_editText.setCustomSelectionActionModeCallback(new ActionMode.Callback() {
 @Override
    public boolean onCreateActionMode(ActionMode actionMode, Menu menu) {
        return false;
    }

    public boolean onPrepareActionMode(ActionMode actionMode, Menu menu) {
        return false;
    }

    public boolean onActionItemClicked(ActionMode actionMode, MenuItem item) {
        return false;
    }

    public void onDestroyActionMode(ActionMode actionMode) {
    }
});
	
		} 
		catch (NoClassDefFoundError ex) 
		{
           //	Log.d(PackageName, "setCustomSelectionActionModeCallback(> Avoided crash. "+ex);
		}
        	
       	//Log.d(PackageName, "Passed setCustomSelectionActionModeCallback");
	
	try
		{
m_editText.setOnEditorActionListener(new EditText.OnEditorActionListener() {
    @Override
    public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
        if (actionId == EditorInfo.IME_ACTION_SEARCH || actionId == EditorInfo.IME_ACTION_DONE) 
        {
          	InputMethodManager mgr = (InputMethodManager)app.getSystemService(Context.INPUT_METHOD_SERVICE);
		    mgr.hideSoftInputFromWindow (mGLView.getWindowToken(),0);
      
            Log.d(app.PackageName, "editor action says we're done editing text");
           	SharedActivity.app.nativeOnKey(1, 13, 13); //fake enter key
            
        	mGLView.requestFocus();
			m_editText.setText("");
            return true;
        }
        return false;
    }
});

	} 
		catch (NoClassDefFoundError ex) 
		{
           	Log.d(PackageName, "setOnEditorActionListener(> Avoided crash. "+ex);
		}
		
m_editText.addTextChangedListener(new TextWatcher() 
{

          public void afterTextChanged(Editable s) 
          {
          
          
          }

          public void beforeTextChanged(CharSequence s, int start, int count, int after) 
          {
	       //  Log.d(app.PackageName, "beforeTextChanged: "+count+" chars changed. start is "+start+" and After: "+after+" String: "+s);
      
          }

          public void onTextChanged(CharSequence s, int start, int before, int count) 
          {
          //grab the last char entered and send to Proton
         // Log.d(app.PackageName, "onTextChanged: "+count+" chars changed. start is "+start+" and before is "+before+". String: "+s);
          	
          		if (m_before.length() > s.length())
          		{
          			//string is now smaller, either a cut, clear or delete.  Let's assume delete for Proton's simple native input
          			SharedActivity.app.m_before = s.toString();
		  			SharedActivity.app.nativeOnKey(1, 67, 0);
          			return;
          		} else
          		
          		if (s.length() > m_before.length())
          		{
          			if (s.length() > start)
				  {
          				char changedChar = s.charAt(start);
          				SharedActivity.app.nativeOnKey(1, 0, changedChar);
            			SharedActivity.app.nativeOnKey(0, 0, changedChar);
          		  }
          		} else
          		{
          			//nothing changed.  Must be del?
          		}
          		
			SharedActivity.app.m_before = s.toString();
			
	
          }
       });
	
		m_editText.setLongClickable(false);
		
		try
		{
		m_editText.setTextIsSelectable(false);
		} 
		catch (NoSuchMethodError ex) 
		{
           	//Log.d(PackageName, "setTextIsSelectable(> Avoided crash. "+ex);
		}
		m_editText.setBackgroundColor(Color.TRANSPARENT);
		m_editText.setTextColor(Color.TRANSPARENT);

		addContentView(m_editText, new ViewGroup.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT));
		mGLView.requestFocus();
	//end new input stuff
  }
	
		setVolumeControlStream(AudioManager.STREAM_MUSIC);
		if (securityEnabled) 
			this.license_init();
		
		//#if defined(RT_CHARTBOOST_SUPPORT)

		this.cb = Chartboost.sharedChartboost();
	
		
		//#endif
		
		//#if defined(RT_FLURRY_SUPPORT)
		Log.d(app.PackageName, "Flurry initializing");
		// configure Flurry
        FlurryAgent.setLogEnabled(false);
 
        // init Flurry
        FlurryAgent.init(this, m_flurryAPIKey);
		//#endif

		
		// Adding Appsflyer Support
//#if defined(RT_APPSFLYER_ENABLED)
		try{
			Log.d("Appsflyer", "Starting Appsflyer Tracking");
			AppsFlyerLib.getInstance().setCollectIMEI(false);
			AppsFlyerLib.getInstance().setCollectAndroidID(false);
			AppsFlyerLib.getInstance().startTracking(this.getApplication(),"");
			Log.d("Appsflyer", "Appsflyer Tracking Successfull!");
		}
		catch(Exception e){
			Log.e("Appsflyer", "Couldn't initialize appsflyer!");
			Log.e("Appsflyer", e.getMessage());
		}
//#endif


//#if defined(RT_TAPJOY_SUPPORT)
			
		//Create dummy tapjoy view overlay we'll show ads on
		adLinearLayout = new RelativeLayout(this);
		RelativeLayout.LayoutParams l = new RelativeLayout.LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT);
		//l.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM);
   		Log.d(PackageName, "Tapjoy enabled - setting up adview overlay");
		addContentView(adLinearLayout, l);
//#endif
		
		Log.d(PackageName, "Setting IAB...");

		update_display_ad = false;
		run_hooked = false;
		tapjoy_ad_show = 0;
//#if defined(RT_DISABLE_IAP_SUPPORT)
//#else	  		
		if (IAPEnabled)	
		{
			// Log.d(PackageName, "Creating IAB helper.");
			 mHelper = new IabHelper(this, BASE64_PUBLIC_KEY);
        
			 // enable debug logging (for a production application, you should set this to false).
			 //mHelper.enableDebugLogging(true);
				
			 //Log.d(PackageName, "Setting up IAB helper");
			 mHelper.startSetup(new IabHelper.OnIabSetupFinishedListener() 
			 {
				 public void onIabSetupFinished(IabResult result) 
				 {
					if (!result.isSuccess()) 
					{
						// Oh noes, there was a problem.
						complain("Problem setting up in-app billing (do you have the latest version of the market installed?) : " + result);
						//return;
					}

					// Hooray, IAB is fully set up. Now, let's get an inventory of stuff we own.
					//Log.d(PackageName, "Setup successful.");
					//mHelper.queryInventoryAsync(mGotInventoryListener);
				}
           });

		}
//#endif
		sendVersionDetails();
    }

    // Listener that's called when we finish querying the items and subscriptions we own
    IabHelper.QueryInventoryFinishedListener mGotInventoryListener = new IabHelper.QueryInventoryFinishedListener()
     {
        public void onQueryInventoryFinished(IabResult result, Inventory inventory)
         {
            
            // Have we been disposed of in the meantime? If so, quit.
            if (mHelper == null) return;


            if (result.isFailure())
             {
                complain("Failed to query inventory: " + result);
     		    SharedActivity.nativeSendGUIEx(SharedActivity.app.MESSAGE_TYPE_IAP_PURCHASED_LIST_STATE, -1,0,0);
	            return;
            }

			SharedActivity.app.m_iap_inventory = inventory; //cache for later
	        
            /*
             * Check for items we own. Notice that for each purchase, we check
             * the developer payload to see if it's correct! See
             * verifyDeveloperPayload().
             */
           
  
			   for(Purchase purchase : inventory.getAllPurchases())
				{
										//Log.d(PackageName, "Get all purchases Purchase: " + purchase.getSku());
					//Log.d(PackageName, "json: " + purchase.getOriginalJson());
					//Log.d(PackageName, "json: " + purchase.getSignature());
					nativeSendGUIStringEx(SharedActivity.app.MESSAGE_TYPE_IAP_PURCHASED_LIST_STATE, 0,0,0, purchase.getSku()+"|"+purchase.getOriginalJson()+"|"+purchase.getSignature()); //0 means PURCHASED
				}
    
            //Log.d(PackageName, "Initial inventory query finished; enabling main UI.");
      	    SharedActivity.nativeSendGUIEx(SharedActivity.app.MESSAGE_TYPE_IAP_PURCHASED_LIST_STATE, -1,0,0); //-1 means END OF LIST
	  
        }
    };
 					
					
   IabHelper.OnIabPurchaseFinishedListener mPurchaseFinishedListener = new IabHelper.OnIabPurchaseFinishedListener()
     {
        public void onIabPurchaseFinished(IabResult result, Purchase purchase) 
        {
           // Log.d(PackageName, "Purchase finished: " + result + ", purchase: " + purchase);
            
                // if we were disposed of in the meantime, quit.
            if (mHelper == null) return;

            if (result.isFailure()) 
            {
               nativeSendGUIEx(MESSAGE_TYPE_IAP_RESULT, result.getResponse() ,0,0);
		       return;
            }
           
           // Log.d(PackageName, "Purchase successful: "+purchase.getOriginalJson());

			nativeSendGUIStringEx(MESSAGE_TYPE_IAP_RESULT, result.getResponse(),0,0, purchase.getOriginalJson()+"|"+purchase.getSignature());
        }
    };
    
    
    // Called when consumption is complete
    IabHelper.OnConsumeFinishedListener mConsumeFinishedListener = new IabHelper.OnConsumeFinishedListener() 
    {
        public void onConsumeFinished(Purchase purchase, IabResult result) 
        {
           // Log.d(PackageName, "Consumption finished. Purchase: " + purchase + ", result: " + result);
            // if we were disposed of in the meantime, quit.
            if (mHelper == null) return;

            if (result.isSuccess()) 
            {
                // successfully consumed, so we apply the effects of the item in our
                // game world's logic, which in our case means filling the gas tank a bit
                Log.d(PackageName, "Consumption successful. Provisioning.");
				if (m_iap_inventory != null)
				{
					m_iap_inventory.erasePurchase(purchase.getSku());
				}
            }
            else 
            {
                complain("Error while consuming: " + result);
            }
          
           // Log.d(PackageName, "End consumption flow.");
        }
    };


      @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data)
     {
       // Log.d(PackageName, "****");

       // Log.d(PackageName, "onActivityResult(" + requestCode + "," + resultCode + "," + data);

        // Pass on the activity result to the helper for handling
        if (!mHelper.handleActivityResult(requestCode, resultCode, data)) 
        {
            // not handled, so handle it ourselves (here's where you'd
            // perform any handling of activity results not related to in-app
            // billing...
            super.onActivityResult(requestCode, resultCode, data);
        }
        else 
        {
       //     Log.d(PackageName, "onActivityResult handled by IABUtil.");
        }
    }
    
//#endif


    @Override
    protected synchronized void onPause()
	{
		float hzTemp = accelHzSave;
	    setup_accel(0); //disable accelerator messages to save on CPU/battery
		accelHzSave = hzTemp;
        mGLView.onPause();
        super.onPause();
		//_sounds.autoPause();
		
//#if defined(RT_FLURRY_SUPPORT)
		Log.d(app.PackageName, "Flurry finishing session");
		FlurryAgent.onEndSession(app);
//#endif
	
    }

    @Override
	protected synchronized void onResume()
	{
		//_sounds.autoResume();
	
		music_set_volume(m_lastMusicVol); //android forgets the audio level, need to reset it
        mGLView.onResume();
		setup_accel(accelHzSave);
		super.onResume();

		
//#if defined(RT_FLURRY_SUPPORT)
		if (!m_flurryAPIKey.isEmpty())
		{
			Log.d(app.PackageName, "Flurry re-starting session");
			FlurryAgent.onStartSession(app, m_flurryAPIKey);
		}
//#endif	
					
	}
	
	// Create runnable for posting
	final Runnable mUpdateMainThread = new Runnable() 
	{
		public void run() 
		{
			if (app.bIsShuttingDown)
			{
				//Log.d(PackageName, "Finished app in  main thread");
				app.finish();
//#if defined(RT_DISABLE_IAP_SUPPORT)
//#else				
				if (IAPEnabled)
				{
					  //this happens in the destroy(), right?
					 //if (mHelper != null) mHelper.dispose();
					 //mHelper = null;
  				}
//#endif			
				android.os.Process.killProcess(android.os.Process.myPid());
				return;
			}
			updateResultsInUi();
		}
	};
 
 
	private void updateResultsInUi() 
	{
	
		if (set_allow_dimming_asap)
		{
			set_allow_dimming_asap = false;
			Log.d(app.PackageName, "Allowing screen dimming.");
			mGLView.setKeepScreenOn(false);
		}

		if (set_disallow_dimming_asap)
		{
			set_allow_dimming_asap = false;
			Log.d(app.PackageName, "Disabling screen dimming.");
			mGLView.setKeepScreenOn(true);
		}
		
		if (m_focusOnKeyboard)
		{
			m_focusOnKeyboard = false;
			int maxLength = m_text_max_length;
		
			if (m_editText != null)
			{
				/*
				InputFilter[] FilterArray = new InputFilter[1];
				FilterArray[0] = new InputFilter.LengthFilter(maxLength);
				m_editText.setFilters(FilterArray);
				*/
				m_editText.setText(m_text_default);
				m_editText.setSelection(m_editText.getText().length()); 
				m_editText.requestFocus();
			}
		}
		
		if (m_focusOffKeyboard)
		{
			m_focusOffKeyboard = false;
	
			if (m_editText != null)
			{
				mGLView.requestFocus();
			}
		}

//#if defined(RT_DISABLE_IAP_SUPPORT)
//#else	

	boolean bWaiting = false;

	if (!m_iap_sync_purchases_asap.equals(""))
	{
		if (!mHelper.isBusy())
		{
			mHelper.queryInventoryAsync(mGotInventoryListener);
			m_iap_sync_purchases_asap = ""; //clear it so we don't do it again
		} else
		{
				bWaiting = true;
		}
	}
	
	if (!m_iap_asap.equals(""))
	{
		if (!mHelper.isBusy())
		{
			mHelper.launchPurchaseFlow(app, m_iap_asap, RC_REQUEST, mPurchaseFinishedListener, m_iap_developerdata);
			m_iap_asap = ""; //clear it so we don't do it again
			m_iap_developerdata = "";
		} else
		{
			bWaiting = true;
		}
	}

	if (!m_iap_consume_asap.equals(""))
	{
		if (!mHelper.isBusy())
		{

			if (m_iap_inventory != null)
			{
	  			Log.d(app.PackageName, "Initiating consume of"+m_iap_consume_asap);
		
				if (m_iap_inventory.getPurchase(m_iap_consume_asap) == null)
				{
					Log.d(app.PackageName, "Ignoring consume, customer hasn't bought "+m_iap_consume_asap);
				} else
				{
					mHelper.consumeAsync(m_iap_inventory.getPurchase(m_iap_consume_asap), mConsumeFinishedListener);
				}
			} else
			{
		  			Log.d(app.PackageName, "IAP inventory null, can't consume");
			}
			m_iap_consume_asap = ""; //clear it so we don't do it again
		} else
		{
			bWaiting = true;
		}
	} 

	if (bWaiting)
	{
		//I have no idea what I'm doing
		SharedActivity.app.mMainThreadHandler.post(SharedActivity.app.mUpdateMainThread);
	}



//#endif	

//#if defined(RT_CHARTBOOST_SUPPORT)


	    if (cb_performLogon)
	    {
	    	Log.d(app.PackageName, "CB startSession");
		
			cb.onCreate(this, m_chartBoostAppID, m_chartBoostAppSig, null);
			cb.startSession();
			cb_performLogon = false; //don't want it to happen again					
	    }
	    
	    
		if (cb_cacheInterstitial)
		{
			Log.d(app.PackageName, "Caching CB interstitial");
			this.cb.cacheInterstitial();
			cb_cacheInterstitial = false;
		}
		
		if (cb_showInterstitial)
		{

			Log.d(app.PackageName, "Showing CB interstitial");
			//String location = nativeGetLastOSMessageString();
			cb_showInterstitial = false;
			//ChartBoost _cb = Chartboost.sharedChartboost();
			this.cb.showInterstitial();
		}
		
//#endif

		
		if (run_hooked && HookedEnabled)
		{
			Log.d(PackageName, "Lauching Hooked (wasabi) dialog");
			run_hooked = false;
//#if defined(RT_HOOKED_SUPPORT)
			WasabiApi.handleLogoViewClick(app);
//#endif
		}
		if (update_display_ad)
		{
			Log.d(PackageName, "Updating view in main  thread");
			update_display_ad = false;    
	
			adLinearLayout.removeAllViews();
			
			if (tapjoy_ad_show == 1)
			{
				adLinearLayout.addView(adView);
			}
  		}
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
		} 
		else if (Environment.MEDIA_MOUNTED_READ_ONLY.equals(state)) 
		{
			mExternalStorageAvailable = true;
			mExternalStorageWriteable = false;
		} 
		else 
		{
		   // mExternalStorageAvailable = mExternalStorageWriteable = false;
		}

		if (mExternalStorageWriteable == false) 
			return "";

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
		} 
		catch (NameNotFoundException e) 
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
	
		public static int is_app_installed(String packageName)
	{
		//will return 1 if the app is installed
		try
		{
			 ApplicationInfo info = app.getPackageManager().getApplicationInfo(packageName, 0 );
			return 1;
		} catch( PackageManager.NameNotFoundException e )
		{
			return 0;
		}
		
	}

	public static String get_clipboard()
	{
		//Note: On Honeycomb this appears to cause a crash because it has to be done in the main thread, which isn't active when
		//JNI invokes this.  So we have to do a callback and send back the answer later? Argh.  For now, I'll try/catch the crash, it
		//will just be a no-op.
	
		String data = "Thread error, sorry, paste can't be used here.";
	
		try
		{
        	ClipboardManager clipboard = (ClipboardManager) app.getSystemService(CLIPBOARD_SERVICE);
			data = clipboard.getText().toString();
		} 
		catch (Exception ex) 
		{
           	Log.d(PackageName, "get_clipboard> Avoided crash. "+ex);
		}
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
		} 
		else
		{
			return m_szDevIDShort;
		}
	}

public static String get_macAddress()
	{
	 WifiManager wimanager = (WifiManager) app.getSystemService(Context.WIFI_SERVICE);
     String macAddress = wimanager.getConnectionInfo().getMacAddress();
     
     if (macAddress == null) 
		{
		    macAddress = ""; //blank to signal we couldn't get it
	    }
    
    return macAddress;
	}
	
	
  private static boolean hasSuperuserApk()
  {
    return new File("/system/app/Superuser.apk").exists();
  }
  private static int isTestKeyBuild()
  {
    String str = Build.TAGS;
    if ((str != null) && (str.contains("test-keys")));
    for (int i = 1; ; i = 0)
      return i;
  }
  
public static String get_advertisingIdentifier()
	{
		return app.m_advertiserID;
	}
	
	public static String get_cantSupportTrees()
	{
		if (
		hasSuperuserApk() ||
		is_app_installed("com.noshufou.android.su") == 1 ||
		is_app_installed("com.thirdparty.superuser") == 1 ||
		is_app_installed("eu.chainfire.supersu") == 1 ||
		is_app_installed("com.koushikdutta.superuser") == 1 ||
		is_app_installed("com.zachspong.temprootremovejb") == 1 ||
		is_app_installed("com.ramdroid.appquarantine") == 1 ||
		is_app_installed("cyanogenmod.superuser") == 1 ||
		is_app_installed("com.devadvance.rootcloakplus") == 1
		
		
		
		) return "0";
		return "4322";
	}

public static String get_getNetworkType()
	{
		ConnectivityManager connManager = (ConnectivityManager)app.getSystemService(Context.CONNECTIVITY_SERVICE);
		try{
			if (connManager.getNetworkInfo(ConnectivityManager.TYPE_WIFI).isConnected()) {
			    // Wifi is connected
				return "wifi";
			}
			else if(connManager.getNetworkInfo(ConnectivityManager.TYPE_MOBILE).isConnected()){
				// Mobile connection available
				return "mobile";
			}else{
				// No connection available
				return "none";
			}	
		}
		catch(Exception e){
			Log.d("DeviceNetwork", e.getMessage());
		}
		return "none";
	}
	
  @Override
	public void onSensorChanged(SensorEvent event) 
	{
        switch (event.sensor.getType())
		{
            case Sensor.TYPE_ACCELEROMETER:
				if (event.values.length < 3) 
					return; //who knows what this is
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
	
	// Achievement handler (called from C++ class, and the actual handler is overriden in Main.java to do the actual app specific API call
	public void FireAchievement(String achievement)
	{
		Log.v("Achievement", "Firing in Wrong instance");
	}
	 
	// JNI to talk to Kiip
    public static void HandleAchievement(String achievement)
	{
        Log.v("Achievement", "Unlocked value: "+achievement);
		app.FireAchievement(achievement);
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
		InputMethodManager mgr = (InputMethodManager)app.getSystemService(Context.INPUT_METHOD_SERVICE);
        mgr.hideSoftInputFromWindow (mGLView.getWindowToken(),0);
        if (show) 
		{
            Log.d("Msg","Enabling keyboard");
			mgr.toggleSoftInput(InputMethodManager.SHOW_FORCED,0);
            m_focusOnKeyboard = true;
             
            // On the Nexus One, SHOW_FORCED makes it impossible
            // to manually dismiss the keyboard.
            // On the Droid SHOW_IMPLICIT doesn't bring up the keyboard.
        } 
		else
        {
            m_focusOnKeyboard = false;
			Log.d("Msg", "Disabling keyboard");
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
	
	//some tapjoy stuff
	final static int MESSAGE_TYPE_TAPJOY_AD_READY = 23;
	final static int MESSAGE_TYPE_TAPJOY_FEATURED_APP_READY = 24;
	final static int MESSAGE_TYPE_TAPJOY_MOVIE_AD_READY = 25;

	//GOOGLE BILLING
	final static int MESSAGE_TYPE_IAP_RESULT = 26;
	final static int MESSAGE_TYPE_IAP_ITEM_STATE = 27;
	final static int MESSAGE_TYPE_IAP_ITEM_INFO_RESULT = 52;
	
	//more tapjoy stuff
	final static int MESSAGE_TYPE_TAPJOY_TAP_POINTS_RETURN = 28;
	final static int MESSAGE_TYPE_TAPJOY_TAP_POINTS_RETURN_ERROR = 29;
	final static int MESSAGE_TYPE_TAPJOY_SPEND_TAP_POINTS_RETURN = 30;
	final static int MESSAGE_TYPE_TAPJOY_SPEND_TAP_POINTS_RETURN_ERROR = 31;
	final static int MESSAGE_TYPE_TAPJOY_AWARD_TAP_POINTS_RETURN = 32;
	final static int MESSAGE_TYPE_TAPJOY_AWARD_TAP_POINTS_RETURN_ERROR = 33;
	final static int MESSAGE_TYPE_TAPJOY_EARNED_TAP_POINTS = 34;

	final static int MESSAGE_TYPE_GUI_JOYPAD_BUTTONS = 35; //For Jake's android gamepad input
	final static int MESSAGE_TYPE_GUI_JOYPAD = 36; //For Jake's android gamepad input
	final static int MESSAGE_TYPE_GUI_JOYPAD_CONNECT = 37; // For Jakes android gamepad input
	final static int MESSAGE_TYPE_CALL_ENTITY_FUNCTION_RECURSIVELY = 38; //used to schedule fake clicks, helps me with debugging
	
	final static int MESSAGE_TYPE_HW_TOUCH_KEYBOARD_WILL_SHOW = 39; //ios only, when not using external keyboard
	final static int MESSAGE_TYPE_HW_TOUCH_KEYBOARD_WILL_HIDE = 40; //ios only, when not using external keyboard
	final static int MESSAGE_TYPE_HW_KEYBOARD_INPUT_ENDING = 41; //proton is done with input and requesting that the keyboard hid
	final static int MESSAGE_TYPE_HW_KEYBOARD_INPUT_STARTING = 42; //proton is asking for the keyboard to open
   
	//GOOGLE BILLING again
	final static int MESSAGE_TYPE_IAP_PURCHASED_LIST_STATE = 43; //for sending back lists of items we've already purchased

	final static int MESSAGE_TYPE_CALL_STATIC_FUNCTION = 44; // use by other platforms, but this value needs to be reserved by those platforms.

	// for sending through version values
	final static int MESSAGE_TYPE_APP_VERSION = 45;

	final static int MESSAGE_USER = 1000; //send your own messages after this #
	
	//IAP RESPONSE CODES for Proton
	final static int RESULT_OK = 0;
	final static int RESULT_USER_CANCELED = 1;
	final static int RESULT_SERVICE_UNAVAILABLE = 2;
	final static int RESULT_BILLING_UNAVAILABLE = 3;
	final static int RESULT_ITEM_UNAVAILABLE = 4;
	final static int RESULT_DEVELOPER_ERROR = 5;
	final static int RESULT_ERROR = 6;
    final static int RESULT_OK_ALREADY_PURCHASED = 7;
	
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
		} 
		else if (e.getAction() == MotionEvent.ACTION_DOWN)
		{
			//they pushed the button
			//Log.d("Hey", "Trackball button pushed");
			nativeOnKey(1, VIRTUAL_KEY_TRACKBALL_DOWN, VIRTUAL_KEY_TRACKBALL_DOWN); 
		}
		return false; 
	}

@Override
		public boolean onKeyMultiple (int keyCode, 
                int count, 
                KeyEvent event)
                {
        			//Log.v("***** onKeyMultiple",count+" keys pressed! First was "+keyCode+" "+Character.toString(Character.toChars(event.getUnicodeChar())[0]));
        			return super.onKeyMultiple(keyCode, count, event);
                }
@Override
	public boolean onKeyDown(int keycode, KeyEvent e) 
	{
		//Log.v("onKeyDown","Keydown Got "+keycode+" "+Character.toString(Character.toChars(e.getUnicodeChar())[0]));
  
		if (keycode == 67)
		{
			//Log.v("onKeyUp", "Detected delete, ignoring, we handle it a different way now");
			return true;			
		}
		if (e.getRepeatCount() > 0) 
			return super.onKeyDown(keycode, e);

		if(e.isAltPressed() && keycode == KeyEvent.KEYCODE_BACK) 
		{
			//XPeria's O button, not the back button!
			//Log.v("onKeyDown","Sending xperia back key");
			nativeOnKey(1, VIRTUAL_DPAD_BUTTON_RIGHT, e.getUnicodeChar()); 
			return true; //signal that we handled it
		}
		
		//do we want this?  Read somewhere it helps with some issues relating to foreign keyboard input..
		//if(keycode==KeyEvent.KEYCODE_ALT_LEFT || keycode==KeyEvent.KEYCODE_ALT_RIGHT || keycode==KeyEvent.KEYCODE_SHIFT_LEFT || keycode==KeyEvent.KEYCODE_SHIFT_RIGHT) return true;

        switch (keycode)
		{
			case KeyEvent.KEYCODE_BACK:
		{
				//#if defined(RT_CHARTBOOST_SUPPORT)
				// If an interstitial is on screen, close it. Otherwise continue as normal.
				//	 Log.d("onKeyDown","Sending virtual back");
				
				if (this.cb.onBackPressed())
				{
				//	 Log.d("onKeyDown","CB handling back");
					 return true; //ignore the key
				}
				//#endif
				
				nativeOnKey(1, VIRTUAL_KEY_BACK, e.getUnicodeChar()); 
				return true; //signal that we handled it
			}
		}
		
		int vKey = TranslateKeycodeToProtonVirtualKey(keycode);
		nativeOnKey(1, vKey, (char)e.getUnicodeChar()); //1  means keydown
        return super.onKeyDown(keycode, e);
    }
@Override
    public boolean onKeyUp(int keycode, KeyEvent e)
	{
		//Log.v("onKeyUp","Keyup Got "+keycode+" "+Character.toString(Character.toChars(e.getUnicodeChar())[0]));
     	if (keycode == 67)
		{
			//Log.v("onKeyUp", "Detected delete, ignoring, we handle it a different way now");
			return true;			
		}

       	if(e.isAltPressed() && keycode == KeyEvent.KEYCODE_BACK) 
		{
			//XPeria's O button, not the back button!
			//Log.v("onKeyUp","Sending xperia back key");
			nativeOnKey(0, VIRTUAL_DPAD_BUTTON_RIGHT, e.getUnicodeChar()); 
			return true; //signal that we handled it
		}
		
		switch (keycode)
		{
			case KeyEvent.KEYCODE_BACK:
			{
				//#if defined(RT_CHARTBOOST_SUPPORT)
				// If an interstitial is on screen, close it. Otherwise continue as normal.
				if (this.cb.onBackPressed()) 
				{
			//	    Log.d("onKeyUp","CB handling back");
					return true; //ignore the key
				}
				//#endif
    
				nativeOnKey(0, VIRTUAL_KEY_BACK, e.getUnicodeChar()); //0 is type keyup
				return true; //signal that we handled it
			}
		}

      	int vKey = TranslateKeycodeToProtonVirtualKey(keycode);
	
      	nativeOnKey(0, vKey,(char)e.getUnicodeChar()); //0 is type keyup
		return super.onKeyUp(keycode, e);
    }
   

	// straight version
	public void sendVersionDetails()
	{
		//Log.v(app.PackageName, "Attempting to send app version");
		PackageInfo pInfo;
		try {
			pInfo = getPackageManager().getPackageInfo(getPackageName(), 0);
			//Log.v(app.PackageName, "Sending version");
			nativeSendGUIStringEx(MESSAGE_TYPE_APP_VERSION, 0,0,0, pInfo.versionName);
		}
		catch (NameNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			Log.d(app.PackageName, "Cannot load App Version!");
		}
	}
  
  
	//TAPJOY
//#if defined(RT_TAPJOY_SUPPORT)
   
   	// Notifier when a TapjoyConnect.getFullScreenAd is successful.
	public void getFullScreenAdResponse() 
	{
		Log.i(app.PackageName, "Displaying Full Screen Ad..");
		TapjoyConnect.getTapjoyConnectInstance().showFullScreenAd();
	}

	// Notifier when a TapjoyConnect.getFullScreenAd request fails.
	public void getFullScreenAdResponseFailed(int error) 
	{
		Log.i(app.PackageName, "No Full Screen Ad to display: " + error);
		
		//update_text = true;
		//displayText = "No Full Screen Ad to display.";
		
		// We must use a handler since we cannot update UI elements from a different thread.
		//mHandler.post(mUpdateResults);
	}
	

	public void getDisplayAdResponse(View view)
	{
		app.adView = view;

		int ad_width = app.adBannerWidth;
		int ad_height = app.adBannerHeight;

		if (ad_width == 0) 
			ad_width = app.adView.getLayoutParams().width;
		if (ad_height == 0) 
			ad_height = app.adView.getLayoutParams().height;
		
		Log.d(app.PackageName,  "adView dimensions: " + ad_width + "x" + ad_height);
		
		int desired_width = app.mGLView.getMeasuredWidth();
		Log.d(app.PackageName,  "mGLView width is " + desired_width);
				
		if (desired_width > ad_width)
			desired_width = ad_width;
		
		// Resize banner to desired width and keep aspect ratio.
		RelativeLayout.LayoutParams layout = new RelativeLayout.LayoutParams(desired_width, (desired_width*ad_height)/ad_width);
	
		 //RelativeLayout.LayoutParams l = new RelativeLayout.LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT);
		layout.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM);
		layout.addRule(RelativeLayout.CENTER_HORIZONTAL);
		
		app.adView.setLayoutParams(layout);
		Log.v(app.PackageName, "adLinearLayout dimensions: " + app.mGLView.getMeasuredWidth() + "x" + app.mGLView.getMeasuredHeight());
		nativeSendGUIEx(MESSAGE_TYPE_TAPJOY_AD_READY, (int)1,0,0);
	}

	public void getDisplayAdResponseFailed(String error)
	{
		Log.d(app.PackageName, "getDisplayAd error: " + error);
		
		//update_text = true;
		//displayText = "Display Ads: " + error;
		nativeSendGUIEx(MESSAGE_TYPE_TAPJOY_AD_READY, 0,0,0);
			
		// We must use a handler since we cannot update UI elements from a different thread.
		//mMainThreadHandler.post(mUpdateResults);
	}

	// This method must be implemented if using the TapjoyConnect.getTapPoints() method.
	// It is the callback method which contains the currency and points data.
	public void getUpdatePoints(String currencyName, int pointTotal)
	{
		//Log.i("${SMALL_PACKAGE_NAME}", "currencyName: " + currencyName);
		//Log.i("${SMALL_PACKAGE_NAME}", "pointTotal: " + pointTotal);
		nativeSendGUIStringEx(MESSAGE_TYPE_TAPJOY_TAP_POINTS_RETURN, pointTotal,0,0, currencyName);
	}
	
	// This method must be implemented if using the TapjoyConnect.getTapPoints() method.
	// It is the callback method which contains the currency and points data.
	public void getUpdatePointsFailed(String error)
	{
		Log.i("${SMALL_PACKAGE_NAME}", "getTapPoints error: " + error);
	
		nativeSendGUIStringEx(MESSAGE_TYPE_TAPJOY_TAP_POINTS_RETURN_ERROR, 0,0,0, error);
	}
	
	// Notifier for when spending virtual currency succeeds.
	public void getSpendPointsResponse(String currencyName, int pointTotal)
	{
		//Log.i("${SMALL_PACKAGE_NAME}", "currencyName: " + currencyName);
		//Log.i("${SMALL_PACKAGE_NAME}", "pointTotal: " + pointTotal);
		
		nativeSendGUIStringEx(MESSAGE_TYPE_TAPJOY_SPEND_TAP_POINTS_RETURN, pointTotal,0,0, currencyName);
	}
	
	// Notifier for when spending virtual currency fails.
	public void getSpendPointsResponseFailed(String error)
	{
		Log.i("${SMALL_PACKAGE_NAME}", "spendTapPoints error: " + error);
		nativeSendGUIStringEx(MESSAGE_TYPE_TAPJOY_SPEND_TAP_POINTS_RETURN_ERROR, 0,0,0, error);
	}
	
	public void getAwardPointsResponse(String currencyName, int pointTotal)
	{
		//Log.i("${SMALL_PACKAGE_NAME}", "getAwardPointsResponse: " + currencyName);
		nativeSendGUIStringEx(MESSAGE_TYPE_TAPJOY_AWARD_TAP_POINTS_RETURN, pointTotal,0,0, currencyName);
	}

	public void getAwardPointsResponseFailed(String error)
	{
		Log.i("${SMALL_PACKAGE_NAME}", "getAwardPointsResponseFailed: " + error);
		nativeSendGUIStringEx(MESSAGE_TYPE_TAPJOY_AWARD_TAP_POINTS_RETURN_ERROR, 0,0,0, error);
	}

	public void earnedTapPoints(int amount)
	{
		//Log.i("${SMALL_PACKAGE_NAME}", "earnedTapPoints: " + amount);
		nativeSendGUIStringEx(MESSAGE_TYPE_TAPJOY_EARNED_TAP_POINTS, amount,0,0, "");
	}
	
	public void videoReady()
	{
		Log.i("${SMALL_PACKAGE_NAME}", "VIDEO READY");
		nativeSendGUIStringEx(MESSAGE_TYPE_TAPJOY_MOVIE_AD_READY, 1,0,0, "");
	}
	
		// Notifier when a video ad starts.
	public void videoStart()
	{
		Log.i("${SMALL_PACKAGE_NAME}", "VIDEO START");
	}

	public void videoError(int statusCode)
	{
		String displayText = "Error";
		 
		switch (statusCode)
		{
			case TapjoyVideoStatus.STATUS_MEDIA_STORAGE_UNAVAILABLE:
				displayText = "VIDEO ERROR: No SD card or external media storage mounted on device";
				break;
			case TapjoyVideoStatus.STATUS_NETWORK_ERROR_ON_INIT_VIDEOS:
				displayText = "VIDEO ERROR: Network error on init videos";
				break;
			case TapjoyVideoStatus.STATUS_UNABLE_TO_PLAY_VIDEO:
				displayText = "VIDEO ERROR: Error playing video";
				break;
		}
		
		Log.i("${SMALL_PACKAGE_NAME}", "VIDEO ERROR: " + statusCode+" " + displayText);
		nativeSendGUIStringEx(MESSAGE_TYPE_TAPJOY_MOVIE_AD_READY, 0,statusCode,0, displayText);
	}
	
	public void videoComplete()
	{
		Log.i("${SMALL_PACKAGE_NAME}", "VIDEO COMPLETE");
		nativeSendGUIStringEx(MESSAGE_TYPE_TAPJOY_MOVIE_AD_READY, 2,0,0, "");
	}
	
//#endif
	//***************************
    
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
				} 
				catch (InterruptedException ie)
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
		} 
		else
		{
			app._music = new MediaPlayer();
		}
		
		if (fname.charAt(0) == '/')
		{
			//load as raw, not an asset
			try {
				File file = new File(fname);
				FileInputStream is = new FileInputStream(file);
				app._music.setDataSource(is.getFD());
				is.close();
				app._music.setLooping(looping);
				app._music.prepare();
				music_set_volume(m_lastMusicVol);
				app._music.start();
			} 
			catch(IOException e) 
			{ 
				Log.d("Can't load music (raw)", fname, e);
			}
			catch(IllegalStateException e) 
			{ 
				Log.d("Can't load music (raw), illegal state", fname);
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
		} 
		catch(IOException e) 
		{ 
			Log.d("Can't load music", fname);
		}
		catch(IllegalStateException e) 
		{ 
			Log.d("Can't load music, illegal state", fname);
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
			} 
			catch (InterruptedException ie)
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
		} 
		else if (app.musicFadeOutThread == null || !app.musicFadeOutThread.isAlive())
		{
			app.musicFadeOutThread = new MusicFadeOutThread(duration);
			app.musicFadeOutThread.start();
		}
	}

	public synchronized static void music_set_volume(float v) 
	{
		if (app._music == null) 
		{ 
			return; 
		}
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
        if (app._music == null) 
		{ 
			return 0; 
		}
        return app._music.getCurrentPosition();
    }
  
	public synchronized static boolean music_is_playing() 
	{
        if (app._music == null) 
		{ 
			return false; 
		}
        return app._music.isPlaying();
    }

    public synchronized static void music_set_pos(int positionMS) 
	{
        if (app._music == null) 
		{
			Log.d("warning: music_set_position:", "no music playing, can't set position");
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
        } 
		catch(IOException e)
		{
			Log.d("Can't load sound", fname);
		}
        return 0;
    }

    public synchronized static int sound_play(int soundID, float leftVol, float rightVol, int priority, int loop, float speedMod ) 
	{
		//Log.v("MSG", "Playing sound: "+soundID);
		//Log.v("Sound vol:", ""+leftVol);
        return app._sounds.play(soundID,leftVol,rightVol,priority,loop,speedMod);
    }
  
	public static void sound_kill(int soundID ) 
	{
		//Log.v("MSG", "killing sound: "+soundID);
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
	public static native void nativeOnKey(int type, int keycode, int c);
	public static native void nativeOnTrackball(float x, float y);
	public static native void nativeLaunchURL();
	public static native void nativeOnAccelerometerUpdate(float x, float y, float z);
	public static native void nativeSendGUIEx(int messageType, int parm1, int parm2, int finger);
	public static native void nativeSendGUIStringEx(int messageType, int parm1, int parm2, int finger, String s);
	static 
    {
		//System.loadLibrary(dllname);
    }
}

class AppGLSurfaceView extends GLSurfaceView
{

    public AppGLSurfaceView(Context context, SharedActivity _app) 
	{
    	super(context);
        app = _app;
		
		if (app.m_editText != null)
		{
			Log.d(app.PackageName, "Setting focus options...");
			this.setFocusable(true);
			this.setFocusableInTouchMode(true);
			this.requestFocus();
		}
		//Log.d(app.PackageName, "Setup focus");

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
		} 
		else
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
          Class.forName("${PACKAGE_NAME}.SharedMultiTouchInput");
       } 
	   catch (Exception ex) 
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
	 
	 if (app.m_advertiserID == "")
	 {
	 
//#if defined(RT_GOOGLE_SERVICES_SUPPORT)
	 
//Stuff to get the advertiserID
Thread thr = new Thread(new Runnable() {
        @Override
        public void run()
         {
                   AdvertisingIdClient.Info adInfo = null;
        try {
                  Context ctx = SharedActivity.app;
			      adInfo = AdvertisingIdClient.getAdvertisingIdInfo(ctx);
               
            } catch (IllegalStateException e) 
            {
			// Unrecoverable error connecting to Google Play services (e.g.,
			// the old version of the service doesn't support getting AdvertisingId).
			Log.d(app.PackageName, "IllegalStateException: Unrecoverable error connecting to Google Play services");
			 } catch (GooglePlayServicesNotAvailableException e) 
			{
  			Log.d(app.PackageName, "Google Play services is not available entirely.");
			// Google Play services is not available entirely.
		    }  catch (GooglePlayServicesRepairableException e) 
			{
  			Log.d(app.PackageName, "GooglePlayServicesRepairableException");
		    } catch (IOException e) 
            {
			// Unrecoverable error connecting to Google Play services (e.g.,
			Log.d(app.PackageName, "Getting AID: IOException");
			 } 

           if (adInfo != null)
           {
             app.m_advertiserID = adInfo.getId();
			 app.m_limitAdTracking = adInfo.isLimitAdTrackingEnabled();
		   	Log.d(app.PackageName, "------------ Got A-ID: "+app.m_advertiserID+" Tracking: "+app.m_limitAdTracking);
		
           } else
           {
             	Log.d(app.PackageName, "---------- Unable to get A-ID info");
				app.m_advertiserID = "";
           }
        }
    });

    thr.start();
    //#endif
    }


    }

    public void onSurfaceChanged(GL10 gl, int w, int h) 
    {
        //gl.glViewport(0, 0, w, h);
        nativeResize(w, h);
        nativeInit();
    }
	
	//don't change the order of these defines, they match the ones in Proton!
		
	//messages that might be sent to us from Proton's C++ side
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
	final static int MESSAGE_TAPJOY_SHOW_MOVIE_AD = 13; 
	
	final static int MESSAGE_IAP_PURCHASE = 14;
	final static int MESSAGE_IAP_GET_PURCHASED_LIST = 15;
	final static int MESSAGE_IAP_ITEM_DETAILS = 39;
	
	final static int MESSAGE_TAPJOY_GET_TAP_POINTS = 16;
	final static int MESSAGE_TAPJOY_SPEND_TAP_POINTS = 17;
	final static int MESSAGE_TAPJOY_AWARD_TAP_POINTS = 18;
	final static int MESSAGE_TAPJOY_SHOW_OFFERS = 19;
	final static int MESSAGE_HOOKED_SHOW_RATE_DIALOG = 20;
	final static int MESSAGE_ALLOW_SCREEN_DIMMING = 21;
	final static int MESSAGE_REQUEST_AD_SIZE = 22;

	//CHARTBOOST STUFF

	final static int MESSAGE_CHARTBOOST_CACHE_INTERSTITIAL = 23;
	final static int MESSAGE_CHARTBOOST_SHOW_INTERSTITIAL = 24;
	final static int MESSAGE_CHARTBOOST_CACHE_MORE_APPS = 25;
	final static int MESSAGE_CHARTBOOST_SHOW_MORE_APPS = 26;
	final static int MESSAGE_CHARTBOOST_SETUP = 27;
	final static int MESSAGE_CHARTBOOST_NOTIFY_INSTALL = 28;
	final static int MESSAGE_CHARTBOOST_RESERVED1 = 29;
	final static int MESSAGE_CHARTBOOST_RESERVED2 = 30;

	//FLURRY
	final static int MESSAGE_FLURRY_SETUP = 31;
	final static int MESSAGE_FLURRY_ON_PAGE_VIEW = 32;
	final static int MESSAGE_FLURRY_LOG_EVENT = 33;
	
	final static int MESSAGE_SUSPEND_TO_HOME_SCREEN = 34;

	//TJ AGAIN
	final static int MESSAGE_TAPJOY_INIT_MAIN = 35;
	final static int MESSAGE_TAPJOY_INIT_PAID_APP_WITH_ACTIONID = 36;
	final static int MESSAGE_TAPJOY_SET_USERID = 37;

	//IAP again
	final static int MESSAGE_IAP_CONSUME_ITEM = 38;

	static long m_gameTimer = 0;
	static int m_timerLoopMS = 0; //every this MS, the loop runs.  0 for no fps limit
	final static int MESSAGE_FLURRY_START_TIMED_EVENT = 1001;
	final static int MESSAGE_FLURRY_STOP_TIMED_EVENT = 1002;

	// Appsflyer logging puchase
	final static int MESSAGE_APPSFLYER_LOG_PURCHASE = 40;
	
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
	
		if (!app.bIsShuttingDown)
		{
			nativeUpdate(); //maybe later we'll want to adjust this for performance reasons..
			nativeRender();
        }

		//let's process OS messages sent from the app if any exist
		int type = MESSAGE_NONE;

		while ( (type = nativeOSMessageGet()) != 0) //it returns 0 if none is available
		{
			switch (type)
			{
				case MESSAGE_OPEN_TEXT_BOX: //open text box
					
					app.m_text_max_length = nativeGetLastOSMessageParm1();
					app.m_text_default = nativeGetLastOSMessageString();
					app.m_before =  nativeGetLastOSMessageString();
					app.toggle_keyboard(true);
					app.mMainThreadHandler.post(app.mUpdateMainThread);
					break;

				case MESSAGE_CLOSE_TEXT_BOX: //close text box
					app.toggle_keyboard(false);
					app.mMainThreadHandler.post(app.mUpdateMainThread);

					break;
		
				case MESSAGE_SET_ACCELEROMETER_UPDATE_HZ: 
					app.setup_accel(nativeGetLastOSMessageX());
					break;

				case MESSAGE_ALLOW_SCREEN_DIMMING: 
					if (nativeGetLastOSMessageX() == 0)
					{
						//disable screen dimming
						app.set_disallow_dimming_asap = true; //must do it in the UI thread
						app.mMainThreadHandler.post(app.mUpdateMainThread);

					} else
					{
						Log.v(app.PackageName, "Allowing screen dimming.");
						app.set_allow_dimming_asap = true; //must do it in the UI thread
						app.mMainThreadHandler.post(app.mUpdateMainThread);

					}
					break;

				case MESSAGE_SET_FPS_LIMIT: 
					if (nativeGetLastOSMessageX() == 0)
					{
						//disable it, and avoid a div by 0
						m_timerLoopMS = 0;
					} 
					else
					{
						m_timerLoopMS = (int) (1000.0f/nativeGetLastOSMessageX());
					}
				
					//app.setup_accel(nativeGetLastOSMessageX());
					break;

				case MESSAGE_FINISH_APP: 
					Log.v(app.PackageName, "Finishing app from java side");
					app.bIsShuttingDown = true;
					nativeDone();
					Log.v(app.PackageName, "Native shutdown");
			
					//app.finish() will get called in the update handler called below, don't need to do it now
					app.mMainThreadHandler.post(app.mUpdateMainThread);
					break;
				
				case MESSAGE_SUSPEND_TO_HOME_SCREEN:
					Log.v(app.PackageName, "Suspending to home screen");
					
					Intent i = new Intent();
					i.setAction(Intent.ACTION_MAIN);
			        i.addCategory(Intent.CATEGORY_HOME);
					app.startActivity(i);
					break;

				case MESSAGE_TAPJOY_GET_AD: 
//#if defined(RT_TAPJOY_SUPPORT)
						Log.v(app.PackageName, "banner ads no longer supported in TJ 10");
						//TapjoyConnect.getTapjoyConnectInstance().setBannerAdSize(app.tapBannerSize);
						TapjoyConnect.getTapjoyConnectInstance().getDisplayAd(app);
//#else
						Log.v(app.PackageName, "ERROR: RT_TAPJOY_SUPPORT isn't defined in Java project, you can't use it!");
//#endif
					break;
	
	
	
				case MESSAGE_FLURRY_SETUP:
				{
//#if defined(RT_FLURRY_SUPPORT)

					app.m_flurryAPIKey = nativeGetLastOSMessageString();
					//Log.v(app.PackageName, "Setting up flurry: "+app.m_flurryAPIKey);
					FlurryAgent.onStartSession(app, app.m_flurryAPIKey);		
//#else
					Log.v(app.PackageName, "ERROR: RT_FLURRY_SUPPORT isn't defined in Main.java, you can't use it!");
//#endif
				}
					break;
	
//#if defined(RT_FLURRY_SUPPORT)
	
	
				case MESSAGE_FLURRY_ON_PAGE_VIEW:
				{
					//Log.v(app.PackageName, "MESSAGE_FLURRY_ON_PAGE_VIEW> Incrementing page view");
					FlurryAgent.onPageView();
				}
					break;

				case MESSAGE_FLURRY_LOG_EVENT:
				{
					String event = nativeGetLastOSMessageString();
					String key = nativeGetLastOSMessageString2();
					String data = nativeGetLastOSMessageString3();

					Log.v(app.PackageName, "MESSAGE_FLURRY_LOG_EVENT: Event + key/data: "+event+" key: "+key+", data: "+data);

					if (!data.isEmpty())
					{
						// Event with parameters
						//Log.v(app.PackageName, "MESSAGE_FLURRY_LOG_EVENT: Event + key/data: "+event+" key: "+key+", data: "+data);
						// data contains pipe seperated string. split it into HashMap

						//String text2 = "numsession|5\ntimeplayed|3\nIAP|5000\nLastWorld|START";
						Map<String, String> map = new HashMap<String, String>();
	
						for(String keyValue : data.split("\n")) {
   							String[] pairs = keyValue.split("\\|", 2);
   							map.put(pairs[0], pairs.length == 1 ? "" : pairs[1]);
						}

						//Map<String,String> m=new HashMap<String, String>();
						//m.put(key, data);
						FlurryAgent.logEvent(event, map);
					} 
					else
					{
						//Log.v(app.PackageName, "MESSAGE_FLURRY_LOG_EVENT: Event: "+event);
						FlurryAgent.logEvent(event);
					}
				}
					break;
				case MESSAGE_FLURRY_START_TIMED_EVENT:
				{
					String event = nativeGetLastOSMessageString();
					String key = nativeGetLastOSMessageString2();
					String data = nativeGetLastOSMessageString3();

					Log.v(app.PackageName, "MESSAGE_FLURRY_START_TIMED_EVENT: Event + key/data: "+event+" key: "+key+", data: "+data);

					if (!data.isEmpty())
					{
						// Timed event with parameters
						//Log.v(app.PackageName, "MESSAGE_FLURRY_LOG_EVENT: Event + key/data: "+event+" key: "+key+", data: "+data);
						// data contains pipe seperated string. split it into HashMap

						//String text2 = "numsession|5\ntimeplayed|3\nIAP|5000\nLastWorld|START";
						Map<String, String> map = new HashMap<String, String>();
	
						for(String keyValue : data.split("\n")) {
   							String[] pairs = keyValue.split("\\|", 2);
   							map.put(pairs[0], pairs.length == 1 ? "" : pairs[1]);
						}

						//Map<String,String> m=new HashMap<String, String>();
						//m.put(key, data);
						FlurryAgent.logEvent(event, map, true);
					} 
					else
					{
						//Log.v(app.PackageName, "MESSAGE_FLURRY_LOG_EVENT: Event: "+event);
						FlurryAgent.logEvent(event, true);
					}
				}
					break;
				case MESSAGE_FLURRY_STOP_TIMED_EVENT:
				{
					String event = nativeGetLastOSMessageString();
					String key = nativeGetLastOSMessageString2();
					String data = nativeGetLastOSMessageString3();

					Log.v(app.PackageName, "MESSAGE_FLURRY_STOP_TIMED_EVENT: Event + key/data: "+event+" key: "+key+", data: "+data);

					if (!data.isEmpty())
					{
						// STOP Timed event with parameters
						//Log.v(app.PackageName, "MESSAGE_FLURRY_LOG_EVENT: Event + key/data: "+event+" key: "+key+", data: "+data);
						// data contains pipe seperated string. split it into HashMap

						//String text2 = "numsession|5\ntimeplayed|3\nIAP|5000\nLastWorld|START";
						Map<String, String> map = new HashMap<String, String>();
	
						for(String keyValue : data.split("\n")) {
   							String[] pairs = keyValue.split("\\|", 2);
   							map.put(pairs[0], pairs.length == 1 ? "" : pairs[1]);
						}

						//Map<String,String> m=new HashMap<String, String>();
						//m.put(key, data);
						FlurryAgent.endTimedEvent(event, map);

					} 
					else
					{
						//Log.v(app.PackageName, "MESSAGE_FLURRY_LOG_EVENT: Event: "+event);
						FlurryAgent.endTimedEvent(event);
					}
				}
					break;
				
	
//#endif
				case MESSAGE_CHARTBOOST_SETUP:
				{				
//#if defined(RT_CHARTBOOST_SUPPORT)

					//ChartBoost stuff - we can't initialize it here, it must be done in the GUI thread.  So
					//we'll sort of just set some flags, causing it to happen next pass, before any other CB
					//stuff.

					app.m_chartBoostAppID = nativeGetLastOSMessageString();
					app.m_chartBoostAppSig = nativeGetLastOSMessageString2();
					app.cb_performLogon = true; //so it triggers
//#else
					Log.v(app.PackageName, "ERROR: RT_CHARTBOOST_SUPPORT isn't defined in Main.java, you can't use it!");
//#endif
				}
					break;
	
//#if defined(RT_CHARTBOOST_SUPPORT)

				case MESSAGE_CHARTBOOST_CACHE_INTERSTITIAL:
				{
					//must wait and do it in UI thread
					app.cb_cacheInterstitial = true;
					app.mMainThreadHandler.post(app.mUpdateMainThread);
				}
					break;
					
				case MESSAGE_CHARTBOOST_SHOW_INTERSTITIAL:
				{
					//must wait and do it in UI thread
					app.cb_showInterstitial = true;
					app.mMainThreadHandler.post(app.mUpdateMainThread);

				}	
					break;
		
		//TODO:  Handle the other messages...
		
//#endif
	

//#if defined(RT_TAPJOY_SUPPORT)
	
				case MESSAGE_TAPJOY_SET_USERID:
					Log.v(app.PackageName, "Setting userID: " +nativeGetLastOSMessageString());
					TapjoyConnect.getTapjoyConnectInstance().setUserID(nativeGetLastOSMessageString());
					break;
		
		
				case MESSAGE_TAPJOY_GET_FEATURED_APP:
					//re-purposing to show videos
					Log.v(app.PackageName, "Asking tj for fullscreen ad");
					//TapjoyConnect.getTapjoyConnectInstance().getFeaturedApp(app);
					if (nativeGetLastOSMessageString().length() > 0)
					{
						TapjoyConnect.getTapjoyConnectInstance().getFullScreenAdWithCurrencyID(nativeGetLastOSMessageString(), app); 
					} else
					{
						TapjoyConnect.getTapjoyConnectInstance().getFullScreenAd(app); 
					}
					break;

				case MESSAGE_TAPJOY_SHOW_FEATURED_APP:
					//Log.v(app.PackageName, "show tapjoy ap");
					TapjoyConnect.getTapjoyConnectInstance().showFullScreenAd();
					break;
		
				case MESSAGE_TAPJOY_GET_TAP_POINTS:
					//Log.v(app.PackageName, "Getting tapjoy points");
					TapjoyConnect.getTapjoyConnectInstance().getTapPoints(app);
					break;
			
				case MESSAGE_TAPJOY_SPEND_TAP_POINTS:
					//Log.v(app.PackageName, "Spending tappoints: " + nativeGetLastOSMessageParm1());
					TapjoyConnect.getTapjoyConnectInstance().spendTapPoints(nativeGetLastOSMessageParm1(), app);
					break;	
				
				case MESSAGE_TAPJOY_AWARD_TAP_POINTS:
					TapjoyConnect.getTapjoyConnectInstance().awardTapPoints(nativeGetLastOSMessageParm1(), app);
					break;
				
				case MESSAGE_TAPJOY_SHOW_OFFERS:
						// This will show Offers web view from where you can download the latest offers.
						// Note: If you want to provide your own publisher id then use following method to show offer web view:
						//		TapjoyOffers.getTapjoyOffersInstance().showOffers(this, "provide here publisher id");
					TapjoyConnect.getTapjoyConnectInstance().showOffers();
					break;
				
				case MESSAGE_TAPJOY_SHOW_AD:
					
					app.tapjoy_ad_show = (int)nativeGetLastOSMessageX();
					Log.v(app.PackageName, "Tapjoy banner ads no longer supported in SDK 10, parm is: " + app.tapjoy_ad_show);
					//TapjoyConnect.getTapjoyConnectInstance().enableBannerAdAutoRefresh(app.tapjoy_ad_show != 0);
				
					app.update_display_ad = true;
					// We must use a handler since we cannot update UI elements from a different thread.
					app.mMainThreadHandler.post(app.mUpdateMainThread);
					break;
		
				case MESSAGE_REQUEST_AD_SIZE:
					app.adBannerWidth = (int)nativeGetLastOSMessageX();
					app.adBannerHeight = (int)nativeGetLastOSMessageY();
				
					app.adBannerWidth = 480;
					app.adBannerHeight = 72;
				
					app.tapBannerSize = app.adBannerWidth+"x"+app.adBannerHeight;		
					Log.v(app.PackageName, "Setting tapjoy banner size to " + app.tapBannerSize);
					break;
	
//#endif
//#if defined(RT_DISABLE_IAP_SUPPORT)
//#else			
				case MESSAGE_IAP_PURCHASE:
					
					String parm = nativeGetLastOSMessageString();
					String payload =  nativeGetLastOSMessageString2();
					
					//Log.v(app.PackageName, "Buying "+parm);
					
					if (!SharedActivity.IAPEnabled)
					{
						   Log.d(app.PackageName, "requestPurchase>> Um, you'll need to change IAPEnabled to true in Main.java!");
				
					} else
					{
					app.m_iap_asap = parm;
					app.m_iap_developerdata = payload;
					
					app.mMainThreadHandler.post(app.mUpdateMainThread);

                
						/*
						if (!app.mBillingService.requestPurchase(parm, "")) 
						{
						   Log.d(app.PackageName, "requestPurchase>> Billing not supported?!");
						}
						*/
						   //SharedActivity.nativeSendGUIEx(app.MESSAGE_TYPE_IAP_RESULT, ResponseCode.RESULT_BILLING_UNAVAILABLE.ordinal(),0,0);
					}
					break;
				
				case MESSAGE_IAP_GET_PURCHASED_LIST:
					
					if (!SharedActivity.IAPEnabled)
					{
						   Log.d(app.PackageName, "requestPurchase>> Um, you'll need to change IAPEnabled to true in Main.java!");
				
					} else
					{
					  // Log.d(app.PackageName, "Get purchased list");
					   app.m_iap_sync_purchases_asap = "true";
						app.mMainThreadHandler.post(app.mUpdateMainThread);

					}
					break;
					
				case MESSAGE_IAP_CONSUME_ITEM:
					Log.d(app.PackageName, "Consume");
					app.m_iap_consume_asap = nativeGetLastOSMessageString();
					app.mMainThreadHandler.post(app.mUpdateMainThread);
					break;
				
				case MESSAGE_IAP_ITEM_DETAILS:
					// Call the IAP currency info result
					try{
						//Log.d("Appsflyer", "Trying to get item info");
						String item = /*"rt_grope_gem_bag"*/nativeGetLastOSMessageString();
						//Log.d("Appsflyer", "Getting item info for : " + item);
						if(item != null && item != ""){
							String info = "";
							
							// Get the information we need about the SKU
							ArrayList skuList = new ArrayList();
							skuList.add(item);
							Bundle querySkus = new Bundle();
							querySkus.putStringArrayList("ITEM_ID_LIST", skuList);

							Bundle skuDetails = app.mHelper.getIInAppBillingService().getSkuDetails(3, app.getPackageName(), "inapp", querySkus);

							int response = skuDetails.getInt("RESPONSE_CODE");
							if (response == 0) {
								ArrayList<String> responseList = skuDetails.getStringArrayList("DETAILS_LIST");

								for (String thisResponse : responseList) {
									JSONObject object 	= new JSONObject(thisResponse);
									String currency 	= object.getString("price_currency_code");
									
									// Replace all non numeric characters from price and get the price
									String price 		= object.getString("price").replaceAll("[A-Za-z]", "");
									info 				= item+","+currency+","+price;
								}
							}
							
							//Log.d("Appsflyer", "Got item info : " + info);
							if(info != null && info != ""){
								SharedActivity.nativeSendGUIStringEx(app.MESSAGE_TYPE_IAP_ITEM_INFO_RESULT, 0,0,0, info);								
								//Log.d("Appsflyer", "Sent OS Message for Item Info");	
							}
							else{
								//Log.d("Appsflyer", "Item info is not right : "+info);	
							}
						}						
					}
					catch(Exception e){
						Log.d("Get Item Info", "Failed : "+e.getMessage());
					}				
					break;
//#endif			
				case MESSAGE_HOOKED_SHOW_RATE_DIALOG:
					Log.v(app.PackageName, "Launching hooked");
					app.run_hooked = true;
					app.mMainThreadHandler.post(app.mUpdateMainThread);
					break;
				
				case MESSAGE_APPSFLYER_LOG_PURCHASE:
					// Send tracking to Appsflyer
					try{
						

						//#if defined(RT_APPSFLYER_ENABLED)

						// Get the information we need about the SKU
						String item 		= nativeGetLastOSMessageString();
						String currency 	= nativeGetLastOSMessageString2();
						String price 		= nativeGetLastOSMessageString3();
						
						//Log.d("Appsflyer", "Starting purchase tracking.");
						//Log.d("Appsflyer", "Item : "+item);
						//Log.d("Appsflyer", "Currency :"+currency);
						//Log.d("Appsflyer", "Price : "+price);
						Map<String, Object> eventValue = new HashMap<String, Object>();
						eventValue.put(AFInAppEventParameterName.CONTENT_ID, item);
						eventValue.put(AFInAppEventParameterName.CURRENCY, currency);
						eventValue.put(AFInAppEventParameterName.REVENUE, price);
						AppsFlyerLib.getInstance().trackEvent(app.getApplicationContext(),AFInAppEventType.PURCHASE,eventValue);
						//Log.d("Appsflyer", "Tracking sent successfully.");
						//#endif
					}
					catch(Exception e){
						Log.d("Appsflyer", "Tracking failed : "+e.getMessage());
					}
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
	private static native float nativeGetLastOSMessageY();
	private static native String nativeGetLastOSMessageString();
	private static native String nativeGetLastOSMessageString2();
	private static native String nativeGetLastOSMessageString3();
	public SharedActivity app;
}
