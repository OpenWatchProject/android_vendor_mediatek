#include <SpeechPcmMixerVoipRx.h>
#include <SpeechDriverInterface.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "SpeechPcmMixerVoipRx"

namespace android {

SpeechPcmMixerVoipRx *SpeechPcmMixerVoipRx::mVoipRx = NULL;
SpeechPcmMixerVoipRx *SpeechPcmMixerVoipRx::GetInstance() {
    static Mutex mGetInstanceLock;
    Mutex::Autolock _l(mGetInstanceLock);

    if (mVoipRx == NULL) {
        mVoipRx = new SpeechPcmMixerVoipRx();
    }
    ASSERT(mVoipRx != NULL);
    return mVoipRx;
}

SpeechPcmMixerVoipRx::SpeechPcmMixerVoipRx() {
    mMixTypeDl = VOIP_RX_TYPE_REPLACE;
}

status_t SpeechPcmMixerVoipRx::pcmMixerOn(SpeechDriverInterface *pSpeechDriver) {
    pSpeechDriver->VoipRxConfig(PCM_DIRECTION_DOWNLINK, mMixTypeDl);
    //turn on voip rx
    pSpeechDriver->VoipRxOn();
    return NO_ERROR;
}

status_t SpeechPcmMixerVoipRx::pcmMixerOff(SpeechDriverInterface *pSpeechDriver) {
    pSpeechDriver->VoipRxOff();
    return NO_ERROR;
}

status_t SpeechPcmMixerVoipRx::configMixTypeDl(uint8_t mixerType) {
    mMixTypeDl = mixerType;
    return NO_ERROR;
}

}; // namespace android
