//  ***************************************************************
//  AndroidUtils - Creation date: 09/12/2010
//  -------------------------------------------------------------
//  Robinson Technologies - Check license.txt for license info.
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef AndroidUtils_h__
#define AndroidUtils_h__

#include <jni.h>
#include <string>

std::string GetAPKFile();

JNIEnv * GetJavaEnv();
char * GetAndroidMainClassName();
void AppResize( JNIEnv*  env, jobject  thiz, jint w, jint h );
void AppUpdate(JNIEnv*  env);
void AppRender(JNIEnv*  env);
void AppDone(JNIEnv* env);
void AppPause(JNIEnv* env);
void AppInit(JNIEnv* env);
void AppResume(JNIEnv* env);
void AppOnTouch( JNIEnv* env, jobject jobj,jint action, jfloat x, jfloat y, jint finger);
void AppOnKey( JNIEnv* env, jobject jobj, jint type, jint keycode, jint c);
int AppOSMessageGet(JNIEnv* env);
float AppGetLastOSMessageX(JNIEnv* env);
float AppGetLastOSMessageY(JNIEnv* env);
void AppOnAccelerometerUpdate(JNIEnv* env, jobject jobj, jfloat x, jfloat y, jfloat z);
void AppOnTrackball(JNIEnv* env, jobject jobj, jfloat x, jfloat y);
// JAKE ADDED - MACHINE WORKS REQUIRES THIS PLEASE LEAVE
void AppOnJoypad(JNIEnv* env, jobject jobj, jfloat xL, jfloat yL, jfloat xR, jfloat yR);
void AppOnJoypadButtons(JNIEnv* env, jobject jobj, jint key, jint value);
void AppOnJoypadConnection(JNIEnv* env, jobject jobj, jint connect);
// JAKE END
void AppOnSendGUIEx(JNIEnv*  env, jobject thiz,jint messageType, jint parm1, jint parm2, jint finger );
jstring AppGetLastOSMessageString(JNIEnv* env);
jstring AppGetLastOSMessageString2(JNIEnv* env);
jstring AppGetLastOSMessageString3(JNIEnv* env);
void AppOnSendGUIStringEx(JNIEnv* env, jobject thiz,jint messageType, jint parm1, jint parm2, jint finger, jstring s );
float AppGetLastOSMessageParm1(JNIEnv* env);
#endif // AndroidUtils_h__
