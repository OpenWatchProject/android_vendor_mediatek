#ifndef ANDROID_ALSA_AUDIO_VOICE_WAKE_UP_CONTROLLER_H
#define ANDROID_ALSA_AUDIO_VOICE_WAKE_UP_CONTROLLER_H

#include <tinyalsa/asoundlib.h>

#include "AudioType.h"
#include "AudioParamParser.h"
#include <AudioLock.h>

#include "AudioALSACaptureHandlerBase.h"
#include "AudioALSACaptureHandlerVOW.h"


namespace android {
//class AudioALSACaptureDataProviderBase;

class AudioALSADeviceConfigManager;
class AudioALSACaptureDataProviderVOW;

class AudioALSAVoiceWakeUpController {
public:
    virtual ~AudioALSAVoiceWakeUpController();

    static AudioALSAVoiceWakeUpController *getInstance();

    virtual status_t setVoiceWakeUpEnable(const bool enable);
    virtual bool     getVoiceWakeUpEnable();

    virtual status_t updateDeviceInfoForVoiceWakeUp();

    virtual status_t updateVOWCustParam();
    virtual bool    getVoiceWakeUpStateFromKernel();
    virtual unsigned int getVOWMicType();
    virtual void setBargeInBypass(const bool enable);

    virtual status_t SeamlessRecordEnable();

    bool updateSpeakerPlaybackStatus(bool isSpeakerPlaying);

protected:
    AudioALSAVoiceWakeUpController();

    virtual status_t updateParamToKernel();
    virtual bool setBargeInEnable(const bool enable);

private:
    /**
     * singleton pattern
     */
    static AudioALSAVoiceWakeUpController *mAudioALSAVoiceWakeUpController;

    AudioALSACaptureDataProviderVOW *mVOWCaptureDataProvider;
    AudioALSACaptureHandlerBase *mCaptureHandler;
    stream_attribute_t *stream_attribute_target;
    status_t setVoiceWakeUpDebugDumpEnable(const bool enable);
    bool mDebug_Enable;

    struct mixer *mMixer; // TODO(Harvey): move it to AudioALSAHardwareResourceManager later
    struct pcm *mPcm;

    AudioLock mLock;

    bool mEnable;
    bool mWakeupEnableOngoing;

    bool mBargeInEnable;
    bool mBargeInEnableOngoing;
    bool mBargeInBypass;
    struct pcm *mBargeInPcm;
    struct pcm *mPcmHostlessUl;
    struct pcm *mPcmHostlessDl;
    String8 mBargeInTurnOnSequence;

    bool mIsUseHeadsetMic;
    AudioALSAHardwareResourceManager *mHardwareResourceManager;
    bool mIsNeedToUpdateParamToKernel;


    uint32_t mHandsetMicMode;
    uint32_t mHeadsetMicMode;
    AudioALSADeviceConfigManager *mDeviceConfigManager;
    AppHandle *mAppHandle;

    static void *dumyReadThread(void *arg);
    pthread_t hDumyReadThread;
    bool mDumpReadStart;
    AudioLock mDebugDumpLock;
    int mFd_dnn;
    AudioLock mSeamlessLock;
    int mFd_vow;
    bool mIsSpeakerPlaying;

    struct pcm_config mSrcDlConfig;
    struct pcm_config mSrcUlConfig;
    unsigned int mVowChannel;
};

} // end namespace android

#endif // end of ANDROID_ALSA_AUDIO_VOICE_WAKE_UP_CONTROLLER_H
