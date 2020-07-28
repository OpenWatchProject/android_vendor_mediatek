#ifndef ANDROID_AUDIO_ALSA_SPEECH_STREAM_CONTROLLER_H
#define ANDROID_AUDIO_ALSA_SPEECH_STREAM_CONTROLLER_H

#include <utils/threads.h>
#include "AudioType.h"
#include <AudioLock.h>
#include "AudioALSAStreamManager.h"

namespace android {
class AudioALSAHardwareResourceManager;
class AudioALSAStreamOut;
class AudioALSAStreamIn;

class AudioALSASpeechStreamController {
public:
    virtual ~AudioALSASpeechStreamController();
    status_t EnableSpeechStreamThread(bool enable);
    AudioALSAStreamManager *getStreamManager() {return mAudioMtkStreamManager;}
    bool IsSpeechStreamThreadEnable(void);
    pthread_mutex_t mSpeechStreamMutex;
    pthread_cond_t mSphStream_Cond, mSpkMonitorActivate_Cond;
    bool m_bThreadExit;
    bool m_bEnabled;
    static AudioALSASpeechStreamController *getInstance();
    audio_devices_t GetStreamOutputDevice(void);
    status_t SetStreamOutputDevice(audio_devices_t OutputDevices);

protected:
    AudioALSASpeechStreamController();

    AudioALSAHardwareResourceManager *mHardwareResourceManager;

    AudioLock               mLock;

    audio_mode_t            mAudioMode;

    audio_devices_t         mRoutingForTty;

    struct pcm *mPcmIn;
    struct pcm *mPcmOut;

private:
    AudioALSAStreamManager *mAudioMtkStreamManager;
    pthread_t mSpeechStreamThreadID;
    static AudioALSASpeechStreamController *UniqueInstance; // singleton
    static void *SpeechStreamThread(void *arg);
    audio_devices_t mOutputDevices;

};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_SPEECH_PHONE_CALL_CONTROLLER_H
