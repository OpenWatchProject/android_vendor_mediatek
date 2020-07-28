#include <SpeechPcmMixerTelephonyTx.h>
#include <SpeechDriverInterface.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "SpeechPcmMixerTelephonyTx"

namespace android {

uint8_t SpeechPcmMixerTelephonyTx::mPcmMixerDlGain = 0x0;
uint8_t SpeechPcmMixerTelephonyTx::mPcmMixerUlGain = 0xFF;

SpeechPcmMixerTelephonyTx *SpeechPcmMixerTelephonyTx::mTelephonyTx = NULL;
SpeechPcmMixerTelephonyTx *SpeechPcmMixerTelephonyTx::GetInstance() {
    static Mutex mGetInstanceLock;
    Mutex::Autolock _l(mGetInstanceLock);

    if (mTelephonyTx == NULL) {
        mTelephonyTx = new SpeechPcmMixerTelephonyTx();
    }
    ASSERT(mTelephonyTx != NULL);
    return mTelephonyTx;
}

status_t SpeechPcmMixerTelephonyTx::pcmMixerOn(SpeechDriverInterface *pSpeechDriver) {
    pSpeechDriver->TelephonyTxOn();
    // turn on telephony ty
    pSpeechDriver->TelephonyTxConfig(mPcmMixerUlGain, mPcmMixerDlGain);
    return NO_ERROR;
}

status_t SpeechPcmMixerTelephonyTx::pcmMixerOff(SpeechDriverInterface *pSpeechDriver) {
    pSpeechDriver->TelephonyTxOff();
    return NO_ERROR;
}

void SpeechPcmMixerTelephonyTx::setPcmMixerDlMute(const bool mute_on) {
    if (mute_on == 1) {
        mPcmMixerDlGain = 0x0;
    } else {
        mPcmMixerDlGain = 0xFF;
    }
}

void SpeechPcmMixerTelephonyTx::setPcmMixerUlMute(const bool mute_on) {
    if (mute_on == 1) {
        mPcmMixerUlGain = 0x0;
    } else {
        mPcmMixerUlGain = 0xFF;
    }
}

}; // namespace android
