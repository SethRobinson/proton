
#include <PlatformSetup.h>
#include <jni.h>
#include <BaseApp.h>

extern "C"
{

	JNIEXPORT void JNICALL Java_com_rtsoft_RTAndroidApp_SharedActivity_nativeOnKey(JNIEnv* env, jobject thiz, jint type, jint keycode, jint c)
	{
		AppOnKey(env, thiz, type, keycode, c);
	}
	JNIEXPORT void JNICALL Java_com_rtsoft_RTAndroidApp_SharedActivity_nativeSendGUIEx(JNIEnv* env, jobject thiz, jint messageType, jint parm1, jint parm2, jint finger)
	{
		AppOnSendGUIEx(env, thiz, messageType, parm1, parm2, finger);
	}
	JNIEXPORT void JNICALL Java_com_rtsoft_RTAndroidApp_SharedActivity_nativeSendGUIStringEx(JNIEnv* env, jobject thiz, jint messageType, jint parm1, jint parm2, jint finger, jstring s)
	{
		AppOnSendGUIStringEx(env, thiz, messageType, parm1, parm2, finger, s);
	}
	JNIEXPORT void JNICALL Java_com_rtsoft_RTAndroidApp_SharedActivity_nativeOnAccelerometerUpdate(JNIEnv* env, jobject thiz, jfloat x, jfloat y, jfloat z)
	{
		AppOnAccelerometerUpdate(env, thiz, x, y, z);
	}
	JNIEXPORT void JNICALL Java_com_rtsoft_RTAndroidApp_SharedActivity_nativeOnTrackball(JNIEnv* env, jobject thiz, jfloat x, jfloat y)
	{
		AppOnTrackball(env, thiz, x, y);
	}
	// JAKE ADDED - DO NOT REMOVE - Machineworks requires this
	JNIEXPORT void JNICALL Java_com_rtsoft_RTAndroidApp_Main_nativeOnJoyPadButtons(JNIEnv* env, jobject thiz, jint key, jint value)
	{
		AppOnJoypadButtons(env, thiz, key, value);
	}
	JNIEXPORT void JNICALL Java_com_rtsoft_RTAndroidApp_Main_nativeOnJoyPad(JNIEnv* env, jobject thiz, jfloat xL, jfloat yL, jfloat xR, jfloat yR)
	{
		AppOnJoypad(env, thiz, xL, yL, xR, yR);
	}
	JNIEXPORT void JNICALL Java_com_rtsoft_RTAndroidApp_Main_nativeOnJoyPadConnection(JNIEnv* env, jobject thiz, jint xL)
	{
		AppOnJoypadConnection(env, thiz, xL);
	}
	// JAKE END
	JNIEXPORT void JNICALL Java_com_rtsoft_RTAndroidApp_AppGLSurfaceView_nativePause(JNIEnv* env)
	{
		AppPause(env);
	}
	JNIEXPORT void JNICALL Java_com_rtsoft_RTAndroidApp_AppGLSurfaceView_nativeResume(JNIEnv* env)
	{
		AppResume(env);
	}
	JNIEXPORT void JNICALL Java_com_rtsoft_RTAndroidApp_AppGLSurfaceView_nativeOnTouch(JNIEnv* env, jobject thiz, jint action, jfloat x, jfloat y, jint finger)
	{
		AppOnTouch(env, thiz, action, x, y, finger);
	}

	JNIEXPORT int Java_com_rtsoft_RTAndroidApp_AppRenderer_nativeOSMessageGet(JNIEnv* env)
	{
		return AppOSMessageGet(env);
	}
	JNIEXPORT float Java_com_rtsoft_RTAndroidApp_AppRenderer_nativeGetLastOSMessageX(JNIEnv* env)
	{
		return AppGetLastOSMessageX(env);
	}

	JNIEXPORT float Java_com_rtsoft_RTAndroidApp_AppRenderer_nativeGetLastOSMessageY(JNIEnv* env)
	{
		return AppGetLastOSMessageY(env);
	}


	JNIEXPORT int Java_com_rtsoft_RTAndroidApp_AppRenderer_nativeGetLastOSMessageParm1(JNIEnv* env)
	{
		return AppGetLastOSMessageParm1(env);
	}
	
	JNIEXPORT int Java_com_rtsoft_RTAndroidApp_AppRenderer_nativeGetLastOSMessageParm2(JNIEnv* env)
	{
		return AppGetLastOSMessageParm2(env);
	}

	JNIEXPORT jstring Java_com_rtsoft_RTAndroidApp_AppRenderer_nativeGetLastOSMessageString(JNIEnv* env)
	{
		return AppGetLastOSMessageString(env);
	}

	JNIEXPORT jstring Java_com_rtsoft_RTAndroidApp_AppRenderer_nativeGetLastOSMessageString2(JNIEnv* env)
	{
		return AppGetLastOSMessageString2(env);
	}
	JNIEXPORT jstring Java_com_rtsoft_RTAndroidApp_AppRenderer_nativeGetLastOSMessageString3(JNIEnv* env)
	{
		return AppGetLastOSMessageString3(env);
	}

	/* Call to initialize the graphics state */
	JNIEXPORT void JNICALL Java_com_rtsoft_RTAndroidApp_AppRenderer_nativeInit(JNIEnv* env)
	{
		AppInit(env);
	}

	JNIEXPORT void JNICALL Java_com_rtsoft_RTAndroidApp_AppRenderer_nativeResize(JNIEnv* env, jobject  thiz, jint w, jint h)
	{
		AppResize(env, thiz, w, h);
	}

	/* Call to finalize the graphics state */
	JNIEXPORT void JNICALL Java_com_rtsoft_RTAndroidApp_AppRenderer_nativeDone(JNIEnv* env)
	{
		AppDone(env);
	}

	JNIEXPORT void JNICALL Java_com_rtsoft_RTAndroidApp_AppRenderer_nativeUpdate(JNIEnv* env)
	{
		AppUpdate(env);
	}
	JNIEXPORT void JNICALL Java_com_rtsoft_RTAndroidApp_AppRenderer_nativeRender(JNIEnv* env)
	{
		AppRender(env);
	}

}