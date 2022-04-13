//This is the only app-specific java file for the android project.  The real work is done in
//shared/android/v3_src/java.  When editing that, be careful, as it is used by all Proton projects.
//Thanks to Phil Hassey for his help and code


//the idea here is you can override values or functions in this project-specific file if needed without
// messing with the SharedActivity.java which is shared between projects.
// 
// You don't actually have to set anything here though.


package com.rtsoft.RTAndroidApp;
import com.rtsoft.RTAndroidApp.SharedActivity;
import android.os.Bundle;

public class Main extends SharedActivity
{
	@Override
    protected void onCreate(Bundle savedInstanceState) 
	{
		securityEnabled = false;  //no longer used
   		IAPEnabled = false; //no longer used
		HookedEnabled = false; //no longer used

		dllname= "RTAndroidApp"; //this doesn't need to be changed

		System.loadLibrary(dllname);
		super.onCreate(savedInstanceState);
    }
}
	
