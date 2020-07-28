#include "AudioALSAANCController.h"

#include <sys/ioctl.h>
#include <cutils/properties.h>

#include <AudioLock.h>

#include "AudioALSAHardwareResourceManager.h"
#include "AudioALSADriverUtility.h"

#include "AudioVolumeFactory.h"
#include "AudioUtility.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioALSAANCController"

namespace android {

/*==============================================================================
 *                     Property keys
 *============================================================================*/


/*==============================================================================
 *                     Const Value
 *============================================================================*/

/*==============================================================================
 *                     Enumerator
 *============================================================================*/

/*==============================================================================
 *                     Singleton Pattern
 *============================================================================*/

AudioALSAANCController *AudioALSAANCController::mAudioALSAANCController = NULL;

AudioALSAANCController *AudioALSAANCController::getInstance() {
    static AudioLock mGetInstanceLock;
    AL_AUTOLOCK(mGetInstanceLock);

    if (mAudioALSAANCController == NULL) {
        mAudioALSAANCController = new AudioALSAANCController();
    }
    ASSERT(mAudioALSAANCController != NULL);
    return mAudioALSAANCController;
}

/*==============================================================================
 *                     Constructor / Destructor / Init / Deinit
 *============================================================================*/

AudioALSAANCController::AudioALSAANCController() :
    mHardwareResourceManager(AudioALSAHardwareResourceManager::getInstance()),
    //mAudioALSAVolumeController(AudioALSAVolumeController::getInstance()),
    mEnable(false),
    mIsFivePole(false),
    mPcm(NULL),
    mSpeakerEnabled(false),
    mReceiverEnabled(false),
    mHeadphoneSpeakerEnabled(false) {
    ALOGD("%s()", __FUNCTION__);

    mFd = ::open(kANCDevName, O_RDWR);

    if (mFd < 0) {
        ALOGE("%s() fail to open %s", __FUNCTION__, kANCDevName);
    } else {
        ALOGD("%s() open %s success!", __FUNCTION__, kANCDevName);

        ::ioctl(mFd, SET_ANC_CONTROL, 0);
    }

    mMixer = AudioALSADriverUtility::getInstance()->getMixer();
    ALOGD("mMixer = %p", mMixer);
    ASSERT(mMixer != NULL);


    // Use System propery
    char property_value[PROPERTY_VALUE_MAX];
    property_get(PROPERTY_ANC_SWITCH, property_value, "1");
    mSwitch = (bool)atoi(property_value);

    RefreshEnabledDecision_l();
}

AudioALSAANCController::~AudioALSAANCController() {
    ALOGD("%s()", __FUNCTION__);
    if (mFd) {
        ::close(mFd);
        mFd = 0;
    }
}

bool AudioALSAANCController::setANCSwitch(bool bSwitch) {
    AL_AUTOLOCK(mLock);
    ALOGD("%s() bSwitch=%d", __FUNCTION__, bSwitch);
    mSwitch = bSwitch;
    property_set(PROPERTY_ANC_SWITCH, mSwitch ? "1" : "0");

    return RefreshEnabledDecision_l();
}

bool AudioALSAANCController::getANCSwitch() {
    AL_AUTOLOCK(mLock);
    ALOGD("%s() mSwitch=%d", __FUNCTION__, mSwitch);

    return mSwitch;
}

bool AudioALSAANCController::setFivePole(bool isFivePole) {
    AL_AUTOLOCK(mLock);
    ALOGD("%s() isFivePole=%d", __FUNCTION__, isFivePole);
    mIsFivePole = isFivePole;

    return RefreshEnabledDecision_l();
}

bool AudioALSAANCController::setReceiverEnabled(bool bEnabled) {
    AL_AUTOLOCK(mLock);
    ALOGD("%s() bEnabled=%d", __FUNCTION__, bEnabled);
    mReceiverEnabled = bEnabled;

    return RefreshEnabledDecision_l();
}

bool AudioALSAANCController::setSpeakerEnabled(bool bEnabled) {
    AL_AUTOLOCK(mLock);
    ALOGD("%s() isFivePole=%d", __FUNCTION__, bEnabled);
    mSpeakerEnabled = bEnabled;

    return RefreshEnabledDecision_l();
}

bool AudioALSAANCController::setHeadphoneSpeakerEnabled(bool bEnabled) {
    AL_AUTOLOCK(mLock);
    ALOGD("%s() isFivePole=%d", __FUNCTION__, bEnabled);
    mHeadphoneSpeakerEnabled = bEnabled;

    return RefreshEnabledDecision_l();
}

bool AudioALSAANCController::getFivePole() {
    ALOGD("%s() mIsFivePole=%d", __FUNCTION__, mIsFivePole);
    AL_AUTOLOCK(mLock);

    return mIsFivePole;
}

void AudioALSAANCController::setANCEnable(bool enable) {
    ALOGD("%s() enable=%d", __FUNCTION__, enable);
    AL_AUTOLOCK(mLock);

    setANCEnable_l(enable);
}

void AudioALSAANCController::setANCEnable_l(bool enable) {
    ALOGD("%s() enable=%d", __FUNCTION__, enable);
    int kernelResult = -1;
    if (enable) {
        mHardwareResourceManager->startInputDevice(AUDIO_DEVICE_IN_WIRED_HEADSET);
        ALOGD("%s(), mHardwareResourceManager->startInputDevice(AUDIO_DEVICE_IN_WIRED_HEADSET)", __FUNCTION__);

#ifdef MTK_NEW_VOL_CONTROL
        AudioMTKGainController::getInstance()->setANCEnable(true);
#endif
        AudioVolumeInterface *mAudioALSAVolumeController = AudioVolumeFactory::CreateAudioVolumeController();
        mAudioALSAVolumeController->setMasterVolume(mAudioALSAVolumeController->getMasterVolume(),
                                                    AUDIO_MODE_CURRENT,
                                                    AUDIO_DEVICE_OUT_WIRED_HEADSET);
        mAudioALSAVolumeController->SetCaptureGain(AUDIO_MODE_CURRENT, AUDIO_SOURCE_MIC, AUDIO_DEVICE_IN_WIRED_HEADSET, AUDIO_DEVICE_OUT_WIRED_HEADSET);

        load_coef_l();
        upload_coef_l();
        kernelResult = ::ioctl(mFd, SET_ANC_CONTROL, 81);
        setHWEnable(true);
        ALOGD("result %d", kernelResult);
    } else {
#ifdef MTK_NEW_VOL_CONTROL
        AudioMTKGainController::getInstance()->setANCEnable(false);
#endif
        setHWEnable(false);
        mHardwareResourceManager->stopInputDevice(AUDIO_DEVICE_IN_WIRED_HEADSET);
        ALOGD("%s(), mHardwareResourceManager->stopInputDevice(AUDIO_DEVICE_IN_WIRED_HEADSET)", __FUNCTION__);

        kernelResult = ::ioctl(mFd, SET_ANC_CONTROL, 82);
        ALOGD("result %d", kernelResult);
    }
}


bool AudioALSAANCController::RefreshEnabledDecision_l() {
    ALOGD("%s() mEnable=%d mIsFivePole=%d mSwitch=%d reveiver=%d speaker=%d headphonespeaker=%d", __FUNCTION__,
          mEnable, mIsFivePole, mSwitch, mReceiverEnabled, mSpeakerEnabled, mHeadphoneSpeakerEnabled);
    bool newEnable = false;
    bool isOtherDeviceEnabled = (mReceiverEnabled || mSpeakerEnabled || mHeadphoneSpeakerEnabled);

    newEnable = (mIsFivePole && mSwitch);
    newEnable = newEnable && !isOtherDeviceEnabled;

    if (newEnable != mEnable) {
        setANCEnable_l(newEnable);
    }

    mEnable = newEnable;
    return mEnable;
}

void AudioALSAANCController::setHWEnable(bool enable) {
    ALOGD("%s() enable=%d", __FUNCTION__, enable);
    struct mixer_ctl *ctl;
    enum mixer_ctl_type type;
    unsigned int num_values, i ;
    ctl = mixer_get_ctl_by_name(mMixer, "Audio_ANC_Switch");
    type = mixer_ctl_get_type(ctl);
    num_values = mixer_ctl_get_num_values(ctl);

    if (mixer_ctl_set_enum_by_string(ctl, enable ? "On" : "Off")) {
        ALOGE("Error: SetSpeakerGain invalid value");
    }
}

/*==============================================================================
 *                     ANC Control
 *============================================================================*/

int AudioALSAANCController::setCMD(int cmd) {
    AL_AUTOLOCK(mLock);
    int result;

    ALOGD("%s(), CMD = %d", __FUNCTION__, cmd);
    ALOGD("mFd=%d", mFd);

    switch (cmd) {
    case 90:
        memset(anc_coef, 0x0, MAX_TABS * 4);
        upload_coef_l();
        cmd = 91;
        break;
    case 91:
        load_coef_l();
        upload_coef_l();
        break;
    case 810:
        setANCEnable_l(true);
        return 0;

        break;
    case 820:
        setANCEnable_l(false);
        return 0;

        break;
    }

    ALOGD("%s(), CMD = %d", __FUNCTION__, cmd);
    result = ::ioctl(mFd, SET_ANC_CONTROL, cmd);
    ALOGD("result %d", result);
    return 0;
}

bool AudioALSAANCController::getANCEnable() {
    AL_AUTOLOCK(mLock);
    ALOGD("+%s(), mEnable = %d =>", __FUNCTION__, mEnable);
    return mEnable;
}

void AudioALSAANCController::download_binary() {
    AL_AUTOLOCK(mLock);
    ALOGD("+%s()", __FUNCTION__);
    ALOGD("-%s()", __FUNCTION__);
}

void AudioALSAANCController::load_coef_l() {
    ALOGD("+%s()", __FUNCTION__);
    int tmp_int;
    FILE *fPtr;
    fPtr = fopen("/etc/audio_anc_parm.txt", "r");
    if (fPtr) {
        for (int i = 0 ; i < MAX_TABS; i++) {
            if (fscanf(fPtr, "%d", &tmp_int) > 0) {
                anc_coef[i] = (int)tmp_int;
            }
        }
        fclose(fPtr);
    }
    ALOGD("parameter: read done");
    char tmp[1000];
    char *cur = (char *)tmp;
    for (int i = 0 ; i < MAX_TABS; i++) {
        cur += sprintf((char *)cur, "%d, ", (int)anc_coef[i]);
    }
    cur[0] = '\0';

    ALOGD("get parameter: %s", tmp);
    ALOGD("-%s()", __FUNCTION__);
}

void AudioALSAANCController::upload_coef_l() {
    ALOGD("+%s()", __FUNCTION__);

    FILE *fPtr;
    fPtr = fopen(kANCDevName, "w");
    if (fPtr) {
        fwrite(anc_coef, 1, MAX_TABS * 4, fPtr);
        fclose(fPtr);
    }
    ALOGD("-%s()", __FUNCTION__);
}

} // end of namespace android
