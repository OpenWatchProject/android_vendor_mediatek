#ifndef ANDROID_SPEECH_PCM_MIXER_VOIPRX_H
#define ANDROID_SPEECH_PCM_MIXER_VOIPRX_H

#include <SpeechPcmMixerBase.h>

namespace android {

enum {
    PCM_DIRECTION_DOWNLINK,
    PCM_DIRECTION_UPLINK
};

class SpeechPcmMixerVoipRx : public SpeechPcmMixerBase {

public:
    virtual ~SpeechPcmMixerVoipRx() {}
    static SpeechPcmMixerVoipRx         *GetInstance();
    virtual status_t                    pcmMixerOn(SpeechDriverInterface *pSpeechDriver);
    virtual status_t                    pcmMixerOff(SpeechDriverInterface *pSpeechDriver);
    virtual uint32_t                    getPcmMixerType() { return PCM_MIXER_TYPE_VOIPRX; }
    status_t                            configMixTypeDl(uint8_t mixerType);

protected:
    SpeechPcmMixerVoipRx();

private:
    static SpeechPcmMixerVoipRx         *mVoipRx; // singleton
    uint8_t                             mMixTypeDl;
};
} // end namespace android
#endif //ANDROID_SPEECH_PCM_MIXER_VOIPRX_H