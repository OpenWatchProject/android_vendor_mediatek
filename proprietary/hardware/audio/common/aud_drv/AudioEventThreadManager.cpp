#include <AudioEventThreadManager.h>
#include <AudioAssert.h>
#include <utils/threads.h>
#include <SpeechUtility.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioEventThreadManager"

namespace android {

/*
 * =============================================================================
 *                     typedef
 * =============================================================================
 */
/**
 * callback function structure
 */
struct CallbackStruc {
    CallbackFunc cb;
    void *regArg;
    void *arg;
};

/*
 * =============================================================================
 *                     AudioEventThread
 * =============================================================================
 */
class AudioEventThread {
public:
    AudioEventThread(AudioEventType audioEventType);
    virtual ~AudioEventThread();

    pthread_t mThreadHandle;
    AudioEventType mAudioEventType;
    std::vector<CallbackStruc> mCallbackStrucs;
    void *mArg;
    bool mEventThreadEnable;

    void signal(void *arg);
    int addCallback(CallbackFunc callbackFunc, void *regArg);

private:
    static void    *eventThread(void *arg);
    AudioLock    mLock;
};

AudioEventThread::AudioEventThread(AudioEventType audioEventType) {
    int retval = 0;
    ALOGD("%s(), audioEventType = %d", __FUNCTION__, audioEventType);
    mAudioEventType = audioEventType;
    mArg = NULL;
    mThreadHandle = 0;

    retval = pthread_create(&mThreadHandle, NULL,
                            AudioEventThread::eventThread,
                            (void *)this);
    mEventThreadEnable = true;
    ASSERT(retval == 0);

}

AudioEventThread::~AudioEventThread() {
    ALOGD("%s()", __FUNCTION__);
    mEventThreadEnable = false;
    signal(NULL);
    pthread_join(mThreadHandle, NULL);
}

void AudioEventThread::signal(void *arg) {
    AL_LOCK(mLock);
    mArg = arg;
    AL_SIGNAL(mLock);
    AL_UNLOCK(mLock);
}

int AudioEventThread::addCallback(CallbackFunc callbackFunc, void *regArg) {
    struct CallbackStruc callback;
    callback.cb = callbackFunc;
    callback.regArg = regArg;
    mCallbackStrucs.push_back(callback);
    return 0;
}

void *AudioEventThread::eventThread(void *arg) {
    char thread_name[128];

    AudioEventThread *pAudioEventThread = NULL;
    CONFIG_THREAD(thread_name, ANDROID_PRIORITY_AUDIO);

    pAudioEventThread = static_cast<AudioEventThread *>(arg);
    if (pAudioEventThread == NULL) {
        ALOGE("%s(), NULL!! pAudioEventThread %p", __FUNCTION__, pAudioEventThread);
        goto PROCESS_EVENT_THREAD_DONE;
    }

    AL_LOCK(pAudioEventThread->mLock);
    while (pAudioEventThread->mEventThreadEnable == true) {
        // sleep until signal comes
        AL_WAIT_NO_TIMEOUT(pAudioEventThread->mLock);
        ALOGV("%s(), signal comes, mEventThreadEnable=%d", __FUNCTION__, pAudioEventThread->mEventThreadEnable);

        //signal callbacks
        for (int idxCallback = 0; idxCallback < (int) pAudioEventThread->mCallbackStrucs.size(); idxCallback ++) {
            pAudioEventThread->mCallbackStrucs.at(idxCallback).arg = pAudioEventThread->mArg;

            (*pAudioEventThread->mCallbackStrucs.at(idxCallback).cb)
            ((int)pAudioEventThread->mAudioEventType,
             (void *)pAudioEventThread->mCallbackStrucs.at(idxCallback).regArg,
             (void *)pAudioEventThread->mCallbackStrucs.at(idxCallback).arg);
        }
    }
    AL_UNLOCK(pAudioEventThread->mLock);

PROCESS_EVENT_THREAD_DONE:
    ALOGD("%s terminated", thread_name);
    pthread_exit(NULL);
    return NULL;
}

/*
 * =============================================================================
 *                     Singleton Pattern
 * =============================================================================
 */
AudioEventThreadManager *AudioEventThreadManager::uniqueAudioEventThreadManager = NULL;

AudioEventThreadManager *AudioEventThreadManager::getInstance() {
    static AudioLock getInstanceLock;
    AL_AUTOLOCK(getInstanceLock);

    if (uniqueAudioEventThreadManager == NULL) {
        uniqueAudioEventThreadManager = new AudioEventThreadManager();
    }
    ASSERT(uniqueAudioEventThreadManager != NULL);
    return uniqueAudioEventThreadManager;
}

/*
 * =============================================================================
 *                     AudioEventThreadManager
 * =============================================================================
 */
AudioEventThreadManager::~AudioEventThreadManager() {
    ALOGD("%s()", __FUNCTION__);
    if (!mEventThreads.empty()) {
        mEventThreads.clear();
    }
}

int  AudioEventThreadManager::registerCallback(AudioEventType audioEventType,
                                               CallbackFunc callbackFunc, void *regArg) {
    ALOGD("%s(), audioEventType=%d, callbackFunc = %p", __FUNCTION__, audioEventType, callbackFunc);
    bool isCallbackFound = false;
    if (hasAudioEventThread(audioEventType)) {
        AudioEventThread *audioEvnetHandler = getAudioEventThread(audioEventType);
        for (int idxCallback = 0; idxCallback < (int)audioEvnetHandler->mCallbackStrucs.size(); idxCallback ++) {
            if (callbackFunc == audioEvnetHandler->mCallbackStrucs.at(idxCallback).cb &&
                regArg == audioEvnetHandler->mCallbackStrucs.at(idxCallback).regArg) {
                ALOGW("%s(), callback already existed. return audioEventType=%d, callbackFunc = %p",
                      __FUNCTION__, audioEventType, callbackFunc);
                return 1;
            }
        }
        audioEvnetHandler->addCallback(callbackFunc, regArg);
        ALOGD("%s(), add callbackFunc(%p) to audioEventType(%d), ", __FUNCTION__, callbackFunc, audioEventType);
    } else {
        AudioEventThread *audioEvnetHandler = new AudioEventThread(audioEventType);
        audioEvnetHandler->addCallback(callbackFunc, regArg);
        mEventThreads.push_back(audioEvnetHandler);
        ALOGD("%s(), add new audioEventType=%d, callbackFunc = %p", __FUNCTION__, audioEventType, callbackFunc);
    }
    return 0;
}

int AudioEventThreadManager::unregisterCallback(AudioEventType audioEventType, CallbackFunc callbackFunc, void *regArg) {
    ALOGD("+%s(), audioEventType=%d, callbackFunc = %p", __FUNCTION__, audioEventType, callbackFunc);
    bool isCallbackFound = false;

    for (int idxEvent = 0; idxEvent < (int) mEventThreads.size(); idxEvent ++) {
        AudioEventThread *audioEvnetHandler = mEventThreads.at(idxEvent);
        if (audioEventType == audioEvnetHandler->mAudioEventType) {
            for (int idxCallback = 0; idxCallback < (int)audioEvnetHandler->mCallbackStrucs.size(); idxCallback ++) {
                if (callbackFunc == audioEvnetHandler->mCallbackStrucs.at(idxCallback).cb &&
                    regArg == audioEvnetHandler->mCallbackStrucs.at(idxCallback).regArg) {
                    isCallbackFound = true;
                    audioEvnetHandler->mCallbackStrucs.erase(audioEvnetHandler->mCallbackStrucs.begin() + idxCallback);
                    if (audioEvnetHandler->mCallbackStrucs.size() == 0) {
                        mEventThreads.at(idxEvent)->mEventThreadEnable = false;
                        mEventThreads.at(idxEvent)->signal(NULL);
                        pthread_join(audioEvnetHandler->mThreadHandle, NULL);
                        mEventThreads.erase(mEventThreads.begin() + idxEvent);
                    }
                    break;
                }
            }
        }
    }
    if (isCallbackFound) {
        ALOGD("-%s(), audioEventType=%d, callbackFunc = %p", __FUNCTION__, audioEventType, callbackFunc);
        return 0;
    } else {
        ALOGW("-%s(), can not find callbackFunc(%p) to audioEventType(%d), return",
              __FUNCTION__, callbackFunc, audioEventType);
        return 1;
    }
}

int AudioEventThreadManager::unregisterCallback(AudioEventType audioEventType) {
    ALOGD("+%s(), audioEventType=%d", __FUNCTION__, audioEventType);
    for (int idxEvent = 0; idxEvent < (int) mEventThreads.size(); idxEvent ++) {
        if (audioEventType == mEventThreads.at(idxEvent)->mAudioEventType) {
            mEventThreads.at(idxEvent)->mEventThreadEnable = false;
            mEventThreads.at(idxEvent)->signal(NULL);
            mEventThreads.at(idxEvent)->mCallbackStrucs.clear();
            pthread_join(mEventThreads.at(idxEvent)->mThreadHandle, NULL);
            mEventThreads.erase(mEventThreads.begin() + idxEvent);
            break;
        }
    }
    ALOGD("-%s(), audioEventType=%d", __FUNCTION__, audioEventType);
    return 0;
}

int AudioEventThreadManager::notifyCallback(AudioEventType audioEventType, void *arg) {
    int retval = 0;
    int idxEvent = 0;
    bool isThreadFound = false;

    for (idxEvent = 0; idxEvent < (int) mEventThreads.size(); idxEvent ++) {
        if (audioEventType == mEventThreads.at(idxEvent)->mAudioEventType) {
            isThreadFound = true;
            break;
        }
    }
    if (isThreadFound) {
        mEventThreads.at(idxEvent)->signal(arg);
        ALOGD("%s(), audioEventType(0x%x), idxEvent=%d", __FUNCTION__, audioEventType, idxEvent);
    } else {
        ALOGW("%s(), audioEventType(0x%x), arg(%p), no event callback registered. skip",
              __FUNCTION__, audioEventType, arg);
    }
    return 0;
}

bool AudioEventThreadManager::hasAudioEventThread(AudioEventType audioEventType) {
    bool isThreadExisted = false;
    for (int idxEvent = 0; idxEvent < (int) mEventThreads.size(); idxEvent ++) {
        if (audioEventType == mEventThreads.at(idxEvent)->mAudioEventType) {
            isThreadExisted = true;
            break;
        }
    }
    return isThreadExisted;
}

AudioEventThread *AudioEventThreadManager::getAudioEventThread(AudioEventType audioEventType) {
    AudioEventThread *audioEvnetHandler = NULL;
    for (int idxEvent = 0; idxEvent < (int) mEventThreads.size(); idxEvent ++) {
        if (audioEventType == mEventThreads.at(idxEvent)->mAudioEventType) {
            audioEvnetHandler = mEventThreads.at(idxEvent);
            break;
        }
    }
    return audioEvnetHandler;
}


} /* end of namespace android */

