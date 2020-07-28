#include <jni.h>
/* Header for class com_mediatek_usp_UspServiceImpl */

#ifndef _Included_usp_native
#define _Included_usp_native
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_mediatek_usp_UspServiceImpl
 * Method:    freeFrame
 * Signature: ()I;
 */

JNIEXPORT jint
Java_com_mediatek_usp_UspServiceImpl_freezeFrame(JNIEnv* env, jobject thiz);

/*
 * Class:     com_mediatek_usp_UspServiceImpl
 * Method:    unfreeFrame
 * Signature: ()I;
 */
JNIEXPORT jint
Java_com_mediatek_usp_UspServiceImpl_unfreezeFrame(JNIEnv* env, jobject thiz);

/*
 * Class:     com_mediatek_usp_UspServiceImpl
 * Method:    ccciEnterDeepFlight
 * Signature: ()I;
 */
JNIEXPORT jint
Java_com_mediatek_usp_UspServiceImpl_ccciEnterDeepFlight(JNIEnv* env, jobject thiz);

/*
 * Class:     com_mediatek_usp_UspServiceImpl
 * Method:    ccciLeaveDeepFlight
 * Signature: ()I;
 */
JNIEXPORT jint
Java_com_mediatek_usp_UspServiceImpl_ccciLeaveDeepFlight(JNIEnv* env, jobject thiz);

#ifdef __cplusplus
}
#endif
#endif
