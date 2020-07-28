#ifndef ANDROID_SPEECH_PCM_MIXER_TELEPHONYTX_H
#define ANDROID_SPEECH_PCM_MIXER_TELEPHONYTX_H

#include <SpeechPcmMixerBase.h>

namespace android {

class SpeechPcmMixerTelephonyTx : public SpeechPcmMixerBase {

public:
    virtual ~SpeechPcmMixerTelephonyTx() {}
    static SpeechPcmMixerTelephonyTx    *GetInstance();
    virtual status_t                    pcmMixerOn(SpeechDriverInterface *pSpeechDriver);
    virtual status_t                    pcmMixerOff(SpeechDriverInterface *pSpeechDriver);
    virtual uint32_t                    getPcmMixerType() { return PCM_MIXER_TYPE_TELEPHONYTX; }
    static  void                        setPcmMixerDlMute(const bool mute_on);
    static  void                        setPcmMixerUlMute(const bool mute_on);

protected:
    SpeechPcmMixerTelephonyTx() {}

private:
    static SpeechPcmMixerTelephonyTx    *mTelephonyTx; // singleton
    static uint8_t                      mPcmMixerDlGain;
    static uint8_t                      mPcmMixerUlGain;

};
} // end namespace android

#endif //ANDROID_SPEECH_PCM_MIXER_TELEPHONYTX_H