#ifdef MTK_LOG_ENABLE
#undef MTK_LOG_ENABLE
#endif
#define MTK_LOG_ENABLE 1

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "voicecontrol/VRJNI"

#include "cutils/log.h"

#define LOGDEBUG_JNI_VOICECMD ALOGD("[JNIVOICECMD]@%s,@%d",__FUNCTION__,__LINE__);
#define LOGERR_JNI_VOICECMD ALOGE("[JNIVOICECMD]ERROR:@%s,@%d",__FUNCTION__,__LINE__);

#include "jni.h"
#include "nativehelper/JNIHelp.h"
#include "android_runtime/AndroidRuntime.h"
#include "android_os_Parcel.h"
#include "utils/Errors.h"
#include "utils/threads.h"
#include <utils/RefBase.h>

#include "../../src/voiceCmdRecognition/VoiceCmdRecognition.h"

using namespace android;

//---------------------------------------------------------------------------------------------------------------------------------------------------//
//for java

struct fields_t {
    JavaVM  *pVM;
    jfieldID    context;
    jmethodID   post_event;
};
static fields_t fields;

typedef struct
{
    const char*             pName;
    const char*             pType;
    jfieldID                fieldId;
} VoiceCmdJava_Field;


typedef struct
{
    const char*             pName;
    const char*             pType;
    jmethodID               methodId;
} VoiceCmdJava_Method;



#define VOICECMD_JAVA_FIELD_INIT(m_name, m_type)                           \
    { m_name,                                                               \
      m_type,                                                               \
      NULL }


#define VOICECMD_JAVA_METHOD_INIT(m_name, m_type)                          \
    { m_name,                                                               \
      m_type,                                                               \
      NULL }


#define VOICE_RECOGNITION_RESULT_CLASS_NAME "com/mediatek/voicecommand/adapter/RecognitionResult"

//------------------------------------------------------------------------------------------------------------------------------------------------------
//for native
static Mutex sLock;


typedef enum
{
    VRMESSAGE_NO_SUCCESS,
    VRMESSAGE_SUCCESS,
    VRMESSAGE_LARGE_NOISE,
    VRMESSAGE_SMALL_VOICE
} VRMessage;

enum voicecmd_event_type {

    VOICECMD_ERROR  = -1,
    VOICECMD_KEYWORD_RECOGNITION  = 0,
    VOICECMD_TRAINING = 1,
    VOICECMD_RECOGNITION  = 2,
};

class JNIVoiceCmdListener: public VoiceCmdRecognitionListener
{
public:
    JNIVoiceCmdListener(JNIEnv* env, jobject thiz, jobject weak_thiz);
    ~JNIVoiceCmdListener();
    virtual void notify(int msg, int ext1, int ext2, char **ext3 = NULL);//, const Parcel *obj = NULL);
private:
    JNIVoiceCmdListener();
    jclass      mClass;     // Reference to VoiceCmdRecognition class
    jobject     mObject;    // Weak ref to VoiceCmdRecognition Java object to call on
    jobject     vchandlerarcel;
};

JNIVoiceCmdListener::JNIVoiceCmdListener(JNIEnv* env, jobject thiz, jobject weak_thiz)
{

    // Hold onto the VoiceCmdRecognition class for use in calling the static method
    // that posts events to the application thread.
    jclass clazz = env->GetObjectClass(thiz);
    if (clazz == NULL) {
        ALOGE("Can't find android/media/VoiceCmdRecognition");
        jniThrowException(env, "java/lang/Exception", NULL);
        return;
    }
    mClass = (jclass)env->NewGlobalRef(clazz);

    // We use a weak reference so the VoiceCmdRecognition object can be garbage collected.
    // The reference is only used as a proxy for callbacks.
    mObject  = env->NewGlobalRef(weak_thiz);
    vchandlerarcel = env->NewGlobalRef(createJavaParcelObject(env));

}

JNIVoiceCmdListener::~JNIVoiceCmdListener()
{
    // remove global references
    JNIEnv *env = AndroidRuntime::getJNIEnv();
    env->DeleteGlobalRef(mObject);
    env->DeleteGlobalRef(mClass);

    recycleJavaParcelObject(env, vchandlerarcel);
    env->DeleteGlobalRef(vchandlerarcel);
}


void JNIVoiceCmdListener::notify(int msg, int ext1, int ext2, char **ext3)//, const Parcel *obj)
{
   ALOGD("[JNIVOICECMD]NOTIFY: MSG %d, EXT1 %d, EXT2 %d",msg,ext1,ext2);
   JNIEnv *env ;
   int status;
   bool isAttached = false;
   //(void)ext3;
   status = fields.pVM->GetEnv((void**) &env, JNI_VERSION_1_4);
   if(status<0)
   {
      ALOGD("notify:failed to get Java JNI environment!");
      status = fields.pVM->AttachCurrentThread(&env,NULL);
      if(status <0)
      {
         ALOGE("notify :failed to attach java thread");
         return;
      }
      isAttached = true;
   }

   if(mClass == NULL)
   {
      ALOGE("Java Class is NULL");
      return;
   }

   if(fields.post_event == NULL)
   {
      ALOGE("Java Object funtion is NULL");
      return;
   }


   if ((msg == VOICE_TRAINING) && ext3) {
       char **temp = ext3;
       int length = 2;
       for (int i = 0; i < length; i++) {
           ALOGI("notify:VOICE_TRAINING ext3:%s!",*temp++);
       }
       jobjectArray contactManeArray = 0;
       jstring str;
       contactManeArray = (jobjectArray)(env->NewObjectArray(length, env->FindClass("java/lang/String"), 0));
       for (int i = 0; i < length; i++) {
           str = env->NewStringUTF(*ext3++);
           env->SetObjectArrayElement(contactManeArray, i, str);
           env->DeleteLocalRef(str);
       }
       env->CallStaticVoidMethod(mClass, fields.post_event, mObject,
                                 msg, ext1, ext2, contactManeArray);
       env->DeleteLocalRef(contactManeArray);
   } else {
       ALOGV("notify:voice");
       env->CallStaticVoidMethod(mClass, fields.post_event, mObject,
                                msg, ext1, ext2, NULL);
   }

   if (env->ExceptionCheck())
   {
      ALOGW("An exception occurred while notifying an event.");
      env->ExceptionClear();
   }

   if(isAttached)
   {
      fields.pVM->DetachCurrentThread();
   }
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------//
static sp<VoiceCmdRecognition> getVoiceCmdRecognition(JNIEnv* env, jobject thiz)
{
   Mutex::Autolock l(sLock);
   VoiceCmdRecognition* const p = (VoiceCmdRecognition*)env->GetLongField(thiz, fields.context);
   return sp<VoiceCmdRecognition>(p);
}

static sp<VoiceCmdRecognition> setVoiceCmdRecognition(JNIEnv* env, jobject thiz, const sp<VoiceCmdRecognition>& player)
{
    Mutex::Autolock l(sLock);
    sp<VoiceCmdRecognition> old = (VoiceCmdRecognition*)env->GetLongField(thiz, fields.context);
    if (player.get()) {
        player->incStrong(thiz);
    }
    if (old != 0) {
        old->decStrong(thiz);
    }
    env->SetLongField(thiz, fields.context, (long)player.get());
    return old;
}

status_t VoiceCmdJava_getMethodIds(JNIEnv* env,jclass clazz,VoiceCmdJava_Method *methods,int count)
{
	status_t result = OK;

	// Loop over the class fields.
	for (int index = 0; index < count; index++)
	{
		// Look up the method id.
		jmethodID fieldId = env->GetMethodID(clazz, methods[index].pName, methods[index].pType);
		
		if (NULL == fieldId)
		{
			// Throw an exception.
			jniThrowException(env, "java/lang/NoSuchMethodException", "unable to locate method");
			result = INVALID_OPERATION;
			break;
		}

		methods[index].methodId = fieldId;				
	}
    return result;

}

status_t VoiceCmdJava_getFieldIds(JNIEnv* env,jclass clazz,VoiceCmdJava_Field *fields,int count)
{
	status_t result = OK;
	// Loop over the class fields.
	for (int index = 0; index < count; index++)
	{
		// Look up the method id.
		jfieldID fieldId = env->GetFieldID(clazz, fields[index].pName, fields[index].pType);
		
		if (NULL == fieldId)
		{
			// Throw an exception.
			jniThrowException(env, "java/lang/NoSuchMethodException", "unable to locate field");
			result = INVALID_OPERATION;
			break;
		}

		fields[index].fieldId = fieldId;				
	}
	return result;

}


// If exception is NULL and opStatus is not OK, this method sends an error
// event to the client application; otherwise, if exception is not NULL and
// opStatus is not OK, this method throws the given exception to the client
// application.
static void process_VoiceCmdRecognition_call(JNIEnv *env, jobject thiz, status_t opStatus, const char* exception, const char *message)
{

	if (exception == NULL) {  // Don't throw exception. Instead, send an event.
		if (opStatus != (status_t) OK) {
			sp<VoiceCmdRecognition> vchandler = getVoiceCmdRecognition(env, thiz);
			if (vchandler != 0) vchandler->notify(VOICECMD_ERROR, opStatus, 0);
		}
	} else {  // Throw exception!
		if ( opStatus == (status_t) INVALID_OPERATION ) {
			jniThrowException(env, "java/lang/IllegalStateException", NULL);
		} else if ( opStatus == (status_t) PERMISSION_DENIED ) {
			jniThrowException(env, "java/lang/SecurityException", NULL);
		} else if ( opStatus != (status_t) OK ) {
			if (strlen(message) > 230) {
			   // if the message is too long, don't bother displaying the status code
			   jniThrowException( env, exception, message);
			} else {
			   char msg[256];
				// append the status code to the message
			   sprintf(msg, "%s: status=0x%X", message, opStatus);
			   jniThrowException( env, exception, msg);
			}
		}
	}

}


//--------------------------------------------------------------------------------------------------------------------------------------------------------//

static void voice_cmd_init(JNIEnv* env)
{
	LOGDEBUG_JNI_VOICECMD

	jclass clazz;

    clazz = env->FindClass("com/mediatek/voicecommand/adapter/JNICommandAdapter");//java class location XXXXXX
    if (clazz == NULL) {
		LOGERR_JNI_VOICECMD
		jniThrowException(env, "java/lang/NoSuchMethodException", "unable to locate class");
        return;
    }

    fields.context = env->GetFieldID(clazz, "mNativeContext", "J");//java object field
    if (fields.context == NULL) {
		LOGERR_JNI_VOICECMD
		jniThrowException(env, "java/lang/NoSuchMethodException", "unable to locate field");
        return;
    }

    fields.post_event = env->GetStaticMethodID(clazz, "postEventFromNative",
                                               "(Ljava/lang/Object;IIILjava/lang/Object;)V");//java object method for callback function
    if (fields.post_event == NULL) {
		LOGERR_JNI_VOICECMD
		jniThrowException(env, "java/lang/NoSuchMethodException", "unable to locate method");
        return;
    }
	// Set the virtual machine.
	env->GetJavaVM(&(fields.pVM));

}


static void voice_cmd_setup(JNIEnv *env, jobject thiz, jobject weak_this)
{
   LOGDEBUG_JNI_VOICECMD
   sp<VoiceCmdRecognition> vchandler = new VoiceCmdRecognition();
   if (vchandler == NULL) {
      jniThrowException(env, "java/lang/RuntimeException", "Out of memory");
      return;
   }

   // create new listener and give it to VoiceCmdRecognition
   sp<JNIVoiceCmdListener> listener = new JNIVoiceCmdListener(env, thiz, weak_this);
   vchandler->setListener(listener);

   // Set our new C++ VoiceCmdRecognition in an opaque field in the Java object.
   setVoiceCmdRecognition(env, thiz, vchandler);
}

static void voice_cmd_setVoicePasswordFile(JNIEnv *env, jobject thiz,jobject fileDescriptor,jlong offset,jlong length)
{
   LOGDEBUG_JNI_VOICECMD
   sp<VoiceCmdRecognition> vchandler = getVoiceCmdRecognition(env, thiz);
   if (vchandler == NULL)
   {
      jniThrowException(env, "java/lang/IllegalStateException", NULL);
      return;
   }

   if (fileDescriptor == NULL) {
      jniThrowException(env, "java/lang/IllegalArgumentException", NULL);
      return;
   }

   int fd = jniGetFDFromFileDescriptor(env, fileDescriptor);
   ALOGV("setVoicePasswordFile: fd %d", fd);
   process_VoiceCmdRecognition_call( env, thiz, vchandler->setVoicePasswordFile(fd, offset, length), NULL, NULL);

}

static void voice_cmd_setVoicePatternFileFd(JNIEnv *env, jobject thiz,jobject fileDescriptor,jlong offset,jlong length)
{

   LOGDEBUG_JNI_VOICECMD
   sp<VoiceCmdRecognition> vchandler = getVoiceCmdRecognition(env, thiz);
   if (vchandler == NULL)
   {
      jniThrowException(env, "java/lang/IllegalStateException", NULL);
      return;
   }

   if (fileDescriptor == NULL) {
      jniThrowException(env, "java/lang/IllegalArgumentException", NULL);
      return;
   }

   int fd = jniGetFDFromFileDescriptor(env, fileDescriptor);
   ALOGV("setVoicePatternFile: fd %d", fd);
   process_VoiceCmdRecognition_call( env, thiz, vchandler->setVoicePatternFile(fd, offset, length), NULL, NULL);

}

static void voice_cmd_setVoicePatternFile(JNIEnv *env, jobject thiz,jstring str)
{

   LOGDEBUG_JNI_VOICECMD
   sp<VoiceCmdRecognition> vchandler = getVoiceCmdRecognition(env, thiz);
   if (vchandler == NULL ) {
      jniThrowException(env, "java/lang/IllegalStateException", NULL);
      return;
   }

   const  char *cStr = NULL;
   if (NULL != str) {
      cStr = env->GetStringUTFChars(str, NULL);
   if (cStr == NULL) {	// Out of memory
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return; //NO_MEMORY;
      }
   }
   ALOGV("setVoicePatternFile: %s",cStr ? cStr : "(null)");

   process_VoiceCmdRecognition_call(env, thiz, vchandler->setVoicePatternFile(cStr), NULL, NULL);

   if (NULL != str)
   {
      env->ReleaseStringUTFChars(str, cStr);
   }

}


static void voice_cmd_setVoiceFeatureFile(JNIEnv *env, jobject thiz,jobject fileDescriptor,jlong offset,jlong length)
{

   LOGDEBUG_JNI_VOICECMD
   sp<VoiceCmdRecognition> vchandler = getVoiceCmdRecognition(env, thiz);
   if (vchandler == NULL)
   {
      jniThrowException(env, "java/lang/IllegalStateException", NULL);
      return;
   }

   if (fileDescriptor == NULL) {
      jniThrowException(env, "java/lang/IllegalArgumentException", NULL);
      return;
   }

   int fd = jniGetFDFromFileDescriptor(env, fileDescriptor);
   ALOGV("setVoiceFeatureFile: fd %d", fd);
   process_VoiceCmdRecognition_call( env, thiz, vchandler->setVoiceFeatureFile(fd, offset, length),NULL,NULL); //"java/io/IOException", "setVoiceFeatureFile failed." );

}


static void voice_cmd_setVoicePatternPath(JNIEnv *env, jobject thiz,jstring str)
{
    (void)env;
    (void)thiz;
    (void)str;
}



static void voice_cmd_setCommandId(JNIEnv *env, jobject thiz,jint id)
{
   LOGDEBUG_JNI_VOICECMD

   sp<VoiceCmdRecognition> vchandler = getVoiceCmdRecognition(env, thiz);
   if (vchandler == NULL)
   {
      jniThrowException(env, "java/lang/IllegalStateException", NULL);
      return;
   }

   ALOGV("setCommandId: id %d", id);
   process_VoiceCmdRecognition_call( env, thiz, vchandler->setCommandId(id), NULL, NULL );
}


static void voice_cmd_startCaptureVoice(JNIEnv *env, jobject thiz,jint mode)
{
   LOGDEBUG_JNI_VOICECMD
   // 3 mode :voice keyword, training and recognize
   sp<VoiceCmdRecognition> vchandler = getVoiceCmdRecognition(env, thiz);
   if (vchandler == NULL)
   {
      jniThrowException(env, "java/lang/IllegalStateException", NULL);
      return;
   }

   ALOGV("startCaptureVoice: mode %d", mode);
   process_VoiceCmdRecognition_call( env, thiz, vchandler->startCaptureVoice(static_cast<uint32_t>(mode)), NULL,NULL);//"java/io/IOException", "startCaptureVoice failed." );
}

static void voice_cmd_stopCaptureVoice(JNIEnv *env, jobject thiz,jint mode)
{
   LOGDEBUG_JNI_VOICECMD
   sp<VoiceCmdRecognition> vchandler = getVoiceCmdRecognition(env, thiz);
   if (vchandler == NULL)
   {
      jniThrowException(env, "java/lang/IllegalStateException", NULL);
      return;
   }

   ALOGV("stopCaptureVoice: mode %d", mode);
   process_VoiceCmdRecognition_call( env, thiz, vchandler->stopCaptureVoice(static_cast<uint32_t>(mode)), NULL,NULL);//"java/io/IOException", "stopCaptureVoice failed." );

}

static jint voice_cmd_getVoiceIntensity(JNIEnv *env, jobject thiz)
{
   //LOGDEBUG_JNI_VOICECMD
   sp<VoiceCmdRecognition> vchandler = getVoiceCmdRecognition(env, thiz);
   if (vchandler == NULL)
   {
      jniThrowException(env, "java/lang/IllegalStateException", NULL);
      return 0;
   }
   int voice;
   process_VoiceCmdRecognition_call( env, thiz, vchandler->getVoiceIntensity(&voice), NULL, NULL);
   ALOGV("getVoiceIntensity: %d (voice)", voice);
   return voice;
}

static void voice_cmd_startVoiceTraining(JNIEnv *env, jobject thiz)
{
   LOGDEBUG_JNI_VOICECMD

   sp<VoiceCmdRecognition> vchandler = getVoiceCmdRecognition(env, thiz);
   if (vchandler == NULL)
   {
      jniThrowException(env, "java/lang/IllegalStateException", NULL);
      return;
   }

   process_VoiceCmdRecognition_call( env, thiz, vchandler->startVoiceTraining(), NULL, NULL);
}

static void voice_cmd_setVoiceModelRetrain(JNIEnv *env, jobject thiz)
{
   LOGDEBUG_JNI_VOICECMD

   sp<VoiceCmdRecognition> vchandler = getVoiceCmdRecognition(env, thiz);
   if (vchandler == NULL)
   {
      jniThrowException(env, "java/lang/IllegalStateException", NULL);
      return;
   }

   process_VoiceCmdRecognition_call( env, thiz, vchandler->setVoiceModelRetrain(), NULL, NULL);
}

static jobject voice_cmd_startVoicePWRecognition(JNIEnv *env, jobject thiz)
{
    (void)env;
    (void)thiz;
    return NULL;
}

static void voice_cmd_setVoiceUBMFile(JNIEnv *env, jobject thiz,jstring str)
{

   LOGDEBUG_JNI_VOICECMD

   sp<VoiceCmdRecognition> vchandler = getVoiceCmdRecognition(env, thiz);
   if (vchandler == NULL)
   {
      jniThrowException(env, "java/lang/IllegalStateException", NULL);
      return;
   }

   const  char *cStr = NULL;
   if (NULL != str) {
      cStr = env->GetStringUTFChars(str, NULL);
      if (cStr == NULL) {   // Out of memory
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return; //NO_MEMORY;
      }
   }
   ALOGV("setUBMFile: %s",cStr ? cStr : "(null)");

   process_VoiceCmdRecognition_call( env, thiz, vchandler->setVoiceUBMFile(cStr), NULL, NULL);

}

static void voice_cmd_setUpgradeVoiceUBMFile(JNIEnv *env, jobject thiz,jstring str)
{

   LOGDEBUG_JNI_VOICECMD

   sp<VoiceCmdRecognition> vchandler = getVoiceCmdRecognition(env, thiz);
   if (vchandler == NULL)
   {
      jniThrowException(env, "java/lang/IllegalStateException", NULL);
      return;
   }

   const  char *cStr = NULL;
   if (NULL != str) {
      cStr = env->GetStringUTFChars(str, NULL);
      if (cStr == NULL) {   // Out of memory
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return; //NO_MEMORY;
      }
   }
   ALOGV("setUpgradeUBMFile: %s",cStr ? cStr : "(null)");

   process_VoiceCmdRecognition_call( env, thiz, vchandler->setUpgradeVoiceUBMFile(cStr), NULL, NULL);

}

static void voice_cmd_setModelFile(JNIEnv *env, jobject thiz,jstring str)
{
    (void)env;
    (void)thiz;
    (void)str;
}

static void voice_cmd_setActiveAP(JNIEnv *env, jobject thiz,jint id,jboolean bEnable)
{
    (void)env;
    (void)thiz;
    (void)id;
    (void)bEnable;
}

static void voice_cmd_setActiveLanguage(JNIEnv *env, jobject thiz,jint idl)
{
    (void)env;
    (void)thiz;
    (void)idl;
}

static void voice_cmd_setInputMode(JNIEnv *env, jobject thiz,jint mode)
{
   LOGDEBUG_JNI_VOICECMD

   sp<VoiceCmdRecognition> vchandler = getVoiceCmdRecognition(env, thiz);
   if (vchandler == NULL)
   {
      jniThrowException(env, "java/lang/IllegalStateException", NULL);
      return;
   }
   ALOGV("setInputMode: mode %d", mode);

   process_VoiceCmdRecognition_call( env, thiz, vchandler->setInputMode(mode), NULL, NULL);
}

static void voice_cmd_setContactProModePath(JNIEnv *env, jobject thiz,jstring strContactProModePath, jstring strContactInfoPath)
{
    (void)env;
    (void)thiz;
    (void)strContactProModePath;
    (void)strContactInfoPath;
}

static void voice_cmd_setContactName(JNIEnv *env, jobject thiz, jobjectArray arrayArg)
{
    (void)env;
    (void)thiz;
    (void)arrayArg;
}

static void voice_cmd_setAPSelectedRlt(JNIEnv *env, jobject thiz, jstring str)
{
    (void)env;
    (void)thiz;
    (void)str;
}

static void voice_cmd_setSearchRltCnts(JNIEnv *env, jobject thiz,jint maxCnts)
{
    (void)env;
    (void)thiz;
    (void)maxCnts;
}

static void voice_cmd_setScreenOrientation(JNIEnv *env, jobject thiz,jint screen_orientation)
{
    (void)env;
    (void)thiz;
    (void)screen_orientation;
}

static void voice_cmd_voiceRecognitionEnable(JNIEnv *env, jobject thiz, jint mode, jint enable)
{
    (void)env;
    (void)thiz;
    (void)mode;
    (void)enable;
}

/*the following APIs is for voice wakeup feature*/
static void voice_cmd_setVoiceTrainingMode(JNIEnv *env, jobject thiz, jint mode)
{
   LOGDEBUG_JNI_VOICECMD

   sp<VoiceCmdRecognition> vchandler = getVoiceCmdRecognition(env, thiz);
   if (vchandler == NULL)
   {
      jniThrowException(env, "java/lang/IllegalStateException", NULL);
      return;
   }
   ALOGV("setRecognitionMode:  recognition mode: %d", mode);

   process_VoiceCmdRecognition_call( env, thiz, vchandler->setVoiceTrainingMode(mode), NULL, NULL);
}

static void voice_cmd_setVoiceWakeupInfoPath(JNIEnv *env, jobject thiz, jstring str)
{
   LOGDEBUG_JNI_VOICECMD

   sp<VoiceCmdRecognition> vchandler = getVoiceCmdRecognition(env, thiz);
   if (vchandler == NULL)
   {
      jniThrowException(env, "java/lang/IllegalStateException", NULL);
      return;
   }

   const char *cStr = NULL;
   if (str != NULL)
   {
      cStr = env->GetStringUTFChars(str, NULL);
      if (cStr == NULL)
      {// Out of memory
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return; //NO_MEMORY;
      }
   }
   ALOGV("setVoiceWakeupInfoPath: %s",cStr ? cStr : "(null)");

   process_VoiceCmdRecognition_call( env, thiz, vchandler->setVoiceWakeupInfoPath(cStr), NULL, NULL);

   if (str != NULL)
   {
      env->ReleaseStringUTFChars(str, cStr);
   }
}

static void voice_cmd_setVoiceWakeupPWPath(JNIEnv *env, jobject thiz,jint mode, jstring str)
{
    (void)env;
    (void)thiz;
    (void)mode;
    (void)str;
}

static void voice_cmd_setVoiceWakeupPatternPath(JNIEnv *env, jobject thiz,jint mode, jstring str)
{
    (void)env;
    (void)thiz;
    (void)mode;
    (void)str;
}

static void voice_cmd_setVoiceWakeupMode(JNIEnv *env, jobject thiz,jint mode)
{
   LOGDEBUG_JNI_VOICECMD

   sp<VoiceCmdRecognition> vchandler = getVoiceCmdRecognition(env, thiz);
   if (vchandler == NULL)
   {
      jniThrowException(env, "java/lang/IllegalStateException", NULL);
      return;
   }
   ALOGV("setVoiceWakeupMode: voice wakeup mode: %d", mode);

   process_VoiceCmdRecognition_call( env, thiz, vchandler->setVoiceWakeupMode(mode), NULL, NULL);
}

static void voice_cmd_setVoiceWakeupModel(JNIEnv *env, jobject thiz,jint cmdID, jstring str)
{
    (void)env;
    (void)thiz;
    (void)cmdID;
    (void)str;
}

static void voice_cmd_resetVoiceWakeupCmd(JNIEnv *env, jobject thiz,jint cmdID)
{
    (void)env;
    (void)thiz;
    (void)cmdID;
}

static void voice_cmd_continueVoiceTraining(JNIEnv *env, jobject thiz)
{
   LOGDEBUG_JNI_VOICECMD

   sp<VoiceCmdRecognition> vchandler = getVoiceCmdRecognition(env, thiz);
   if (vchandler == NULL)
   {
      jniThrowException(env, "java/lang/IllegalStateException", NULL);
      return;
   }

   process_VoiceCmdRecognition_call( env, thiz, vchandler->continueVoiceTraining(), NULL, NULL);
}

static void voice_cmd_pauseVoiceTraining(JNIEnv *env, jobject thiz)
{
   LOGDEBUG_JNI_VOICECMD

   sp<VoiceCmdRecognition> vchandler = getVoiceCmdRecognition(env, thiz);
   if (vchandler == NULL)
   {
      jniThrowException(env, "java/lang/IllegalStateException", NULL);
      return;
   }

   process_VoiceCmdRecognition_call( env, thiz, vchandler->pauseVoiceTraining(), NULL, NULL);
}

static jint voice_cmd_getTrainigTimes(JNIEnv *env, jobject thiz)
{
   //LOGDEBUG_JNI_VOICECMD
   sp<VoiceCmdRecognition> vchandler = getVoiceCmdRecognition(env, thiz);
   if (vchandler == NULL)
   {
      jniThrowException(env, "java/lang/IllegalStateException", NULL);
      return 0;
   }
   int times;
   process_VoiceCmdRecognition_call( env, thiz, vchandler->getTrainigTimes(&times), NULL, NULL);
   ALOGV("getTrainigTimes: %d (times)", times);
   return times;
}

static void voice_cmd_setUtteranceTrainingTimeout(JNIEnv *env, jobject thiz, jint msec)
{
   LOGDEBUG_JNI_VOICECMD

   sp<VoiceCmdRecognition> vchandler = getVoiceCmdRecognition(env, thiz);
   if (vchandler == NULL)
   {
      jniThrowException(env, "java/lang/IllegalStateException", NULL);
      return;
   }

   ALOGV("setUtteranceTrainingTimeout: msec %d", msec);
   process_VoiceCmdRecognition_call( env, thiz, vchandler->setUtteranceTrainingTimeout(static_cast<uint32_t>(msec)), NULL, NULL);
}

static void voice_cmd_setTrainingThreshold(JNIEnv *env, jobject thiz, jint threshold)
{
   LOGDEBUG_JNI_VOICECMD

   sp<VoiceCmdRecognition> vchandler = getVoiceCmdRecognition(env, thiz);
   if (vchandler == NULL)
   {
      jniThrowException(env, "java/lang/IllegalStateException", NULL);
      return;
   }

   ALOGV("setTrainingThreshold: threshold %d", threshold);
   process_VoiceCmdRecognition_call( env, thiz, vchandler->setTrainingThreshold(static_cast<uint32_t>(threshold)), NULL, NULL);
}

static jint voice_cmd_getTrainingThreshold(JNIEnv *env, jobject thiz)
{
   //LOGDEBUG_JNI_VOICECMD
   sp<VoiceCmdRecognition> vchandler = getVoiceCmdRecognition(env, thiz);
   if (vchandler == NULL)
   {
      jniThrowException(env, "java/lang/IllegalStateException", NULL);
      return 0;
   }
   int threshold;
   process_VoiceCmdRecognition_call( env, thiz, vchandler->getTrainingThreshold(&threshold), NULL, NULL);
   ALOGV("getTrainingThreshold: %d (threshold)", threshold);
   return threshold;
}

static void voice_cmd_release(JNIEnv *env, jobject thiz)
{
   LOGDEBUG_JNI_VOICECMD

   sp<VoiceCmdRecognition> vchandler = setVoiceCmdRecognition(env, thiz, 0);
   if (vchandler != NULL) {
      // this prevents native callbacks after the object is released
      vchandler->setListener(0);
      //vchandler->disconnect();
   }
}

/*
static void voice_cmd_native_finalize(JNIEnv *env, jobject thiz)
{
    LOGDEBUG_JNI_VOICECMD

    sp<VoiceCmdRecognition> vchandler = getVoiceCmdRecognition(env, thiz);
    if (vchandler != NULL) {
        ALOGW("Voice Cmd finalized without being released");
    }
    voice_cmd_release(env, thiz);
}
*/

//----------------------------------------------------------------------------------------------------------------------------------------------------------//
static JNINativeMethod gMethods[] = {

  { "native_setup","(Ljava/lang/Object;)V",(void*)voice_cmd_setup },
  { "native_init","()V",(void*)voice_cmd_init },
  { "setVoicePasswordFile","(Ljava/io/FileDescriptor;JJ)V" , (void*)voice_cmd_setVoicePasswordFile},
  { "setVoicePatternFile","(Ljava/io/FileDescriptor;JJ)V" , (void*)voice_cmd_setVoicePatternFileFd},
  { "setVoicePatternFile","(Ljava/lang/String;)V",(void*)voice_cmd_setVoicePatternFile},
  { "setVoiceFeatureFile","(Ljava/io/FileDescriptor;JJ)V",(void*)voice_cmd_setVoiceFeatureFile},
  { "setVoicePatternPath","(Ljava/lang/String;)V",(void*)voice_cmd_setVoicePatternPath},
  { "setCommandId","(I)V",(void*)voice_cmd_setCommandId},
  { "getVoiceIntensity","()I",(void*)voice_cmd_getVoiceIntensity},
  { "startCaptureVoice","(I)V", (void*)voice_cmd_startCaptureVoice},
  { "stopCaptureVoice","(I)V", (void*)voice_cmd_stopCaptureVoice},
  { "startVoiceTraining","()V",(void*)voice_cmd_startVoiceTraining},
  { "setVoiceModelRetrain","()V",(void*)voice_cmd_setVoiceModelRetrain},
  { "setVoiceUBMFile","(Ljava/lang/String;)V",(void*)voice_cmd_setVoiceUBMFile},
  { "setUpgradeVoiceUBMFile","(Ljava/lang/String;)V",(void*)voice_cmd_setUpgradeVoiceUBMFile},
  { "setModeIFile","(Ljava/lang/String;)V",(void*)voice_cmd_setModelFile},
  { "setActiveAP","(IZ)V",(void*)voice_cmd_setActiveAP},
  { "startVoicePWRecognition","()Lcom/mediatek/voicecommand/adapter/RecognitionResult;",(void*)voice_cmd_startVoicePWRecognition},
  { "setActiveLanguage","(I)V",(void*)voice_cmd_setActiveLanguage},
  { "setInputMode","(I)V",(void*)voice_cmd_setInputMode},
  { "setContactProModePath","(Ljava/lang/String;Ljava/lang/String;)V",(void*)voice_cmd_setContactProModePath},
  { "setContactName","([Ljava/lang/String;)V",(void*)voice_cmd_setContactName},
  { "setAPSelectedRlt","(Ljava/lang/String;)V",(void*)voice_cmd_setAPSelectedRlt},
  { "setSearchRltCnts","(I)V",(void*)voice_cmd_setSearchRltCnts},
  { "setScreenOrientation","(I)V",(void*)voice_cmd_setScreenOrientation},
  { "voiceRecognitionEnable","(II)V",(void*)voice_cmd_voiceRecognitionEnable},
  { "setVoiceTrainingMode","(I)V",(void*)voice_cmd_setVoiceTrainingMode},
  { "setVoiceWakeupInfoPath","(Ljava/lang/String;)V",(void*)voice_cmd_setVoiceWakeupInfoPath},
  { "setVoiceWakeupPWPath","(ILjava/lang/String;)V",(void*)voice_cmd_setVoiceWakeupPWPath},
  { "setVoiceWakeupPatternPath","(ILjava/lang/String;)V",(void*)voice_cmd_setVoiceWakeupPatternPath},
  { "setVoiceWakeupMode","(I)V",(void*)voice_cmd_setVoiceWakeupMode},
  { "setVoiceWakeupModel","(ILjava/lang/String;)V",(void*)voice_cmd_setVoiceWakeupModel},
  { "resetVoiceWakeupCmd","(I)V",(void*)voice_cmd_resetVoiceWakeupCmd},
  { "continueVoiceTraining","()V",(void*)voice_cmd_continueVoiceTraining},
  { "pauseVoiceTraining","()V",(void*)voice_cmd_pauseVoiceTraining},
  { "getTrainigTimes","()I",(void*)voice_cmd_getTrainigTimes},
  { "setUtteranceTrainingTimeout","(I)V",(void*)voice_cmd_setUtteranceTrainingTimeout},
  { "setTrainingThreshold","(I)V",(void*)voice_cmd_setTrainingThreshold},
  { "getTrainingThreshold","()I",(void*)voice_cmd_getTrainingThreshold},
  { "_release","()V",(void*)voice_cmd_release}
  //{ "native_finalize","()V", (void*)voice_cmd_native_finalize}
};


// This function only registers the native methods
static int register_android_voicecmd_VoiceRecognition(JNIEnv *env)
{
   LOGDEBUG_JNI_VOICECMD

   // Register native functions
   return AndroidRuntime::registerNativeMethods(env, "com/mediatek/voicecommand/adapter/JNICommandAdapter",
                                              gMethods, NELEM(gMethods));//for java class location
}

jint JNI_OnLoad(JavaVM* vm , void* reserved)
{
    LOGDEBUG_JNI_VOICECMD
    JNIEnv* env = NULL;
    jint result = -1;
    (void)reserved;

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        ALOGE("ERROR: GetEnv failed\n");
        goto bail;
    }
    assert(env != NULL);

    if (register_android_voicecmd_VoiceRecognition(env) < 0) {
        ALOGE("ERROR: Voice recognition native registration failed\n");
        goto bail;
    }

    /* success -- return valid version number */
    result = JNI_VERSION_1_4;

bail:
    return result;
}
