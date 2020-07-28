#ifndef ANDROID_ALSA_AUDIO_FM_CONTROLLER_H
#define ANDROID_ALSA_AUDIO_FM_CONTROLLER_H

#include "AudioType.h"
#include <AudioLock.h>
#include "AudioVolumeInterface.h"

#include <tinyalsa/asoundlib.h>


namespace android {

// FMAudioPlayer.cpp also need this structure!!
typedef struct _AUDIO_DEVICE_CHANGE_CALLBACK_STRUCT {
    void (*callback)(void *data);
} AUDIO_DEVICE_CHANGE_CALLBACK_STRUCT;


class AudioALSAHardwareResourceManager;
class AudioALSAVolumeController;


class AudioALSAFMController {
public:
    virtual ~AudioALSAFMController();

    static AudioALSAFMController *getInstance();

    virtual bool     getFmEnable();
    virtual status_t setFmEnable(const bool enable, const audio_devices_t output_device, bool bForceControl = false, bool bForce2DirectConn = false, bool bNeedSyncVolume = false); // TODO(Harvey)

    virtual uint32_t getFmUplinkSamplingRate() const;
    virtual uint32_t getFmDownlinkSamplingRate() const;

    virtual status_t routing(const audio_devices_t pre_device, const audio_devices_t new_device);

    virtual status_t setFmVolume(const float fm_volume);

    virtual bool     getFmChipPowerInfo();
    virtual void     setFmDeviceCallback(const AUDIO_DEVICE_CHANGE_CALLBACK_STRUCT *callback_data);

    virtual void     setUseFmDirectConnectionMode(const bool use_direct_mode) { mUseFmDirectConnectionMode = use_direct_mode; }

    virtual bool      checkFmNeedUseDirectConnectionMode();
    virtual float getFmVolume();
    virtual bool     getFmMute();
    virtual status_t setFmMute(const bool mute);
    virtual bool     getFmMode();
    virtual status_t setFmMode(const bool on);
protected:
    AudioALSAFMController();

    virtual status_t setFmDirectConnection_l(const bool enable, const bool bforce, audio_devices_t output_device);

    void (*mFmDeviceCallback)(void *data);
    virtual status_t doDeviceChangeCallback();
    bool isPreferredSampleRate(uint32_t rate) const;
    virtual status_t setFmVolume_l(const float fm_volume);

    AudioALSAHardwareResourceManager *mHardwareResourceManager;
    AudioVolumeInterface        *mAudioALSAVolumeController;


    AudioLock mLock; // TODO(Harvey): could remove it later...

    bool mFmEnable;
    bool mIsFmDirectConnectionMode;
    bool mUseFmDirectConnectionMode;

    float mFmVolume;

    struct pcm *mPcm;
    struct pcm *mPcmUL;
    String8 mApTurnOnSequence;
    String8 mApTurnOnSequence2;
    String8 mTurnOnSeqCustDev1;
    String8 mTurnOnSeqCustDev2;

    audio_devices_t mOuput_device;
    bool mHyBridNLERegister;
    struct pcm_config mConfig;
    bool mFmMuted;
    bool mFmMode;
private:
    static AudioALSAFMController *mAudioALSAFMController; // singleton
    /*
     * flag of dynamic enable verbose/debug log
     */
    int mLogEnable;

};

} // end namespace android

#endif // end of ANDROID_ALSA_AUDIO_FM_CONTROLLER_H
