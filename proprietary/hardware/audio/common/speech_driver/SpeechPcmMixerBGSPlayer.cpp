#include <SpeechPcmMixerBGSPlayer.h>
#include <SpeechDriverInterface.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "SpeechPcmMixerBGSPlayer"

namespace android {

uint8_t SpeechPcmMixerBGSPlayer::mPcmMixerDlGain = 0xFF;
uint8_t SpeechPcmMixerBGSPlayer::mPcmMixerUlGain = 0x0;

SpeechPcmMixerBGSPlayer *SpeechPcmMixerBGSPlayer::mBGSPlayer = NULL;
SpeechPcmMixerBGSPlayer *SpeechPcmMixerBGSPlayer::GetInstance() {
    static Mutex mGetInstanceLock;
    Mutex::Autolock _l(mGetInstanceLock);

    if (mBGSPlayer == NULL) {
        mBGSPlayer = new SpeechPcmMixerBGSPlayer();
    }
    ASSERT(mBGSPlayer != NULL);
    return mBGSPlayer;
}

status_t SpeechPcmMixerBGSPlayer::pcmMixerOn(SpeechDriverInterface *pSpeechDriver) {
    pSpeechDriver->BGSoundOn();

    //recover the UL gain
    //backup Background Sound UL and DL gain
    //bcs we set them to zero when normal recording
    //we need to set it back when phone call recording
    pSpeechDriver->BGSoundConfig(mPcmMixerUlGain, mPcmMixerDlGain);
    return NO_ERROR;
}

status_t SpeechPcmMixerBGSPlayer::pcmMixerOff(SpeechDriverInterface *pSpeechDriver) {
    pSpeechDriver->BGSoundOff();
    return NO_ERROR;
}

void SpeechPcmMixerBGSPlayer::setPcmMixerDlMute(const bool mute_on) {
    if (mute_on == 1) {
        mPcmMixerDlGain = 0x0;
    } else {
        mPcmMixerDlGain = 0xFF;
    }
}

void SpeechPcmMixerBGSPlayer::setPcmMixerUlMute(const bool mute_on) {
    if (mute_on == 1) {
        mPcmMixerUlGain = 0x0;
    } else {
        mPcmMixerUlGain = 0xFF;
    }
}

}; // namespace android
