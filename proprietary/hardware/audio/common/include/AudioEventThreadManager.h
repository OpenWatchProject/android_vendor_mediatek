#ifndef AUDIO_EVENT_THREAD_MANAGER_H
#define AUDIO_EVENT_THREAD_MANAGER_H

#include <vector>
#include <pthread.h>
#include <AudioLock.h>

namespace android {

/*
 * =============================================================================
 *                     typedef
 * =============================================================================
 */
enum AudioEventType {
    AUDIO_EVENT_PHONECALL_REOPEN,
    AUDIO_EVENT_SPEECH_PARAM_CHANGE,
    NUM_AUDIO_EVENT_PHONE
};

/**
 * callback function prototype
 */
typedef void(*CallbackFunc)(int, void *, void *);

/*
 * =============================================================================
 *                     ref struct
 * =============================================================================
 */
class AudioEventThread;

/*
 * =============================================================================
 *                     public function
 * =============================================================================
 */

class AudioEventThreadManager {
public:
    /**
    * get instance's pointer
    */
    static AudioEventThreadManager *getInstance();

    int registerCallback(AudioEventType audioEventType, CallbackFunc callbackFunc, void *regArg);
    int unregisterCallback(AudioEventType audioEventType, CallbackFunc callbackFunc, void *regArg);
    int unregisterCallback(AudioEventType audioEventType);
    int notifyCallback(AudioEventType audioEventType, void *arg);

private:
    AudioEventThreadManager() {};
    virtual ~AudioEventThreadManager();
    /**
    * singleton pattern
    */
    static AudioEventThreadManager *uniqueAudioEventThreadManager;

    std::vector<AudioEventThread *> mEventThreads;
    AudioEventThread *getAudioEventThread(AudioEventType audioEventType);
    bool hasAudioEventThread(AudioEventType audioEventType);
};



} /* end of namespace android */

#endif /* end of AUDIO_EVENT_THREAD_MANAGER_H */

