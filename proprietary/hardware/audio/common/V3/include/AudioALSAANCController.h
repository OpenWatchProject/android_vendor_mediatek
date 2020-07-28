#ifndef ANDROID_ALSA_AUDIO_ANC_CONTROLLER_H
#define ANDROID_ALSA_AUDIO_ANC_CONTROLLER_H

#include "AudioType.h"
#include <AudioLock.h>

#include <tinyalsa/asoundlib.h>

#define AUD_DRV_ANC_IOC_MAGIC 'C'
//ANC Controlaa
#define SET_ANC_CONTROL          _IOW(AUD_DRV_ANC_IOC_MAGIC, 0x1, int)
#define SET_ANC_PARAMETER        _IOW(AUD_DRV_ANC_IOC_MAGIC, 0x2, int)
#define GET_ANC_PARAMETER        _IOW(AUD_DRV_ANC_IOC_MAGIC, 0x3, int)

#define MAX_TABS 68
#define PROPERTY_ANC_SWITCH "persist.vendor.audiohal.anc_switch"
namespace android {

static char const *const kANCDevName = "/dev/ancservice";

class AudioALSAHardwareResourceManager;
//class AudioMTKGainController;

class AudioALSAANCController {
public:
    virtual ~AudioALSAANCController();

    static AudioALSAANCController *getInstance();

    virtual void download_binary();
    virtual void load_coef_l();
    virtual void upload_coef_l();

    virtual bool getANCEnable();
    virtual bool setANCSwitch(bool bSwitch);
    virtual bool getANCSwitch();
    virtual bool setFivePole(bool isFivePole);
    virtual bool getFivePole();

    virtual bool setReceiverEnabled(bool bEnabled);
    virtual bool setSpeakerEnabled(bool bEnabled);
    virtual bool setHeadphoneSpeakerEnabled(bool bEnabled);

    int setCMD(int cmd);

protected:
    AudioALSAANCController();

    AudioALSAHardwareResourceManager *mHardwareResourceManager;
    //AudioMTKGainController *mAudioMTKGainController;
    //AudioALSAVolumeController        *mAudioALSAVolumeController;


    AudioLock mLock; // TODO(Harvey): could remove it later...

    bool mEnable;
    bool mSwitch;
    bool mIsFivePole;
    //Mutex   mMutex;
    int     mFd;

    struct pcm_config mConfig;
    struct pcm *mPcm;

private:
    static AudioALSAANCController *mAudioALSAANCController; // singleton
    int anc_coef[MAX_TABS];

    void setHWEnable(bool enable);

    void setANCEnable(bool enable);
    void setANCEnable_l(bool enable);

    bool RefreshEnabledDecision_l();

    //ALSA relate control
    struct mixer *mMixer;

    bool mSpeakerEnabled;
    bool mReceiverEnabled;
    bool mHeadphoneSpeakerEnabled;
};

} // end namespace android

#endif // end of ANDROID_ALSA_AUDIO_ANC_CONTROLLER_H
