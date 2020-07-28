#include "AudioFtmBase.h"

#include <utils/threads.h>
#include <utils/Log.h>

#include "AudioAssert.h"

#include "AudioFtm.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioFtmBase"

#ifdef __cplusplus
extern "C" {
#endif
android::AudioFtmBaseVirtual *pfuncGetAudioFtmByDL(void) {
    return android::AudioFtmBase::createAudioFtmInstance();
}
#ifdef __cplusplus
}
#endif


namespace android {

AudioFtmBase *AudioFtmBase::createAudioFtmInstance() {
    static Mutex mGetInstanceLock;
    Mutex::Autolock _l(mGetInstanceLock);

    return AudioFtm::getInstance();
}

AudioFtmBase::AudioFtmBase() {
    ALOGD("%s()", __FUNCTION__);
}

AudioFtmBase::~AudioFtmBase() {
    ALOGD("%s()", __FUNCTION__);
}


/// Codec
void AudioFtmBase::Audio_Set_Speaker_Vol(int level __unused) {
    ALOGW("%s()", __FUNCTION__);
}
void AudioFtmBase::Audio_Set_Speaker_On(int Channel __unused) {
    ALOGW("%s()", __FUNCTION__);
}
void AudioFtmBase::Audio_Set_Speaker_Off(int Channel __unused) {
    ALOGW("%s()", __FUNCTION__);
}
void AudioFtmBase::Audio_Set_HeadPhone_On(int Channel __unused) {
    ALOGW("%s()", __FUNCTION__);
}
void AudioFtmBase::Audio_Set_HeadPhone_Off(int Channel __unused) {
    ALOGW("%s()", __FUNCTION__);
}
void AudioFtmBase::Audio_Set_Earpiece_On() {
    ALOGW("%s()", __FUNCTION__);
}
void AudioFtmBase::Audio_Set_Earpiece_Off() {
    ALOGW("%s()", __FUNCTION__);
}


/// for factory mode & Meta mode (Analog part)
void AudioFtmBase::FTM_AnaLpk_on(void) {
    ALOGW("%s()", __FUNCTION__);
}

void AudioFtmBase::FTM_AnaLpk_off(void) {
    ALOGW("%s()", __FUNCTION__);
}

int AudioFtmBase::SineGenTest(char sinegen_test __unused) {
    ALOGW("%s()", __FUNCTION__);
    return true;
}

/// Output device test
int AudioFtmBase::RecieverTest(char receiver_test __unused) {
    ALOGW("%s()", __FUNCTION__);
    return true;
}

int AudioFtmBase::LouderSPKTest(char left_channel __unused, char right_channel __unused) {
    ALOGW("%s()", __FUNCTION__);
    return true;
}
int AudioFtmBase::EarphoneTest(char bEnable __unused) {
    ALOGW("%s()", __FUNCTION__);
    return true;
}
int AudioFtmBase::EarphoneTestLR(char bLR __unused) {
    ALOGW("%s()", __FUNCTION__);
    return true;
}

/// Input device test
int AudioFtmBase::SpecificBuildInMicTest(char type __unused) {
    ALOGW("%s()", __FUNCTION__);
    return true;
}

/// Speaker over current test
int AudioFtmBase::Audio_READ_SPK_OC_STA(void) {
    ALOGW("%s()", __FUNCTION__);
    return true;
}
int AudioFtmBase::LouderSPKOCTest(char left_channel __unused, char right_channel __unused) {
    ALOGW("%s()", __FUNCTION__);
    return true;
}


/// Loopback
int AudioFtmBase::PhoneMic_Receiver_Loopback(char echoflag __unused) {
    ALOGW("%s()", __FUNCTION__);
    return true;
}
int AudioFtmBase::PhoneMic_EarphoneLR_Loopback(char echoflag __unused) {
    ALOGW("%s()", __FUNCTION__);
    return true;
}
int AudioFtmBase::PhoneMic_SpkLR_Loopback(char echoflag __unused) {
    ALOGW("%s()", __FUNCTION__);
    return true;
}
int AudioFtmBase::HeadsetMic_EarphoneLR_Loopback(char bEnable __unused, char bHeadsetMic __unused) {
    ALOGW("%s()", __FUNCTION__);
    return true;
}
int AudioFtmBase::HeadsetMic_SpkLR_Loopback(char echoflag __unused) {
    ALOGW("%s()", __FUNCTION__);
    return true;
}

int AudioFtmBase::HeadsetMic_Receiver_Loopback(char bEnable __unused, char bHeadsetMic __unused) {
    ALOGW("%s()", __FUNCTION__);
    return true;
}

int AudioFtmBase::PhoneMic_Receiver_Acoustic_Loopback(int Acoustic_Type __unused, int *Acoustic_Status_Flag __unused, int bHeadset_Output __unused) {
    ALOGW("%s()", __FUNCTION__);
    return true;
}


/// FM / mATV
int AudioFtmBase::FMLoopbackTest(char bEnable __unused) {
    ALOGW("%s()", __FUNCTION__);
    return true;
}

int AudioFtmBase::Audio_FM_I2S_Play(char bEnable __unused) {
    ALOGW("%s()", __FUNCTION__);
    return true;
}
int AudioFtmBase::Audio_MATV_I2S_Play(int enable_flag __unused) {
    ALOGW("%s()", __FUNCTION__);
    return true;
}
int AudioFtmBase::Audio_FMTX_Play(bool Enable __unused, unsigned int Freq __unused) {
    ALOGW("%s()", __FUNCTION__);
    return true;
}

int AudioFtmBase::ATV_AudPlay_On(void) {
    ALOGW("%s()", __FUNCTION__);
    return true;
}
int AudioFtmBase::ATV_AudPlay_Off(void) {
    ALOGW("%s()", __FUNCTION__);
    return true;
}
unsigned int AudioFtmBase::ATV_AudioWrite(void *buffer __unused, unsigned int bytes __unused) {
    ALOGW("%s()", __FUNCTION__);
    return true;
}


/// HDMI
int AudioFtmBase::HDMI_SineGenPlayback(bool bEnable __unused, int dSamplingRate __unused) {
    ALOGE("%s() is not supported!!", __FUNCTION__);
    return false;
}


/// Vibration Speaker
int AudioFtmBase::SetVibSpkCalibrationParam(void *cali_param __unused) {
    ALOGW("%s()", __FUNCTION__);
    return 0;
}

uint32_t AudioFtmBase::GetVibSpkCalibrationStatus() {
    ALOGW("%s()", __FUNCTION__);
    return 0;
}

void AudioFtmBase::SetVibSpkEnable(bool enable __unused, uint32_t freq __unused) {
    ALOGW("%s()", __FUNCTION__);
}

void AudioFtmBase::SetVibSpkRampControl(uint8_t rampcontrol __unused) {
    ALOGW("%s()", __FUNCTION__);
}

bool AudioFtmBase::ReadAuxadcData(int channel __unused, int *value __unused) {
    ALOGW("%s()", __FUNCTION__);
    return false;
}

void AudioFtmBase::SetStreamOutPostProcessBypass(bool flag __unused) {
    ALOGW("%s()", __FUNCTION__);
    return;
}

bool AudioFtmBase::SpeakerCalibration(int calibrateStage __unused) {
    ALOGW("%s()", __FUNCTION__);
    return false;
}
} // end of namespace android
