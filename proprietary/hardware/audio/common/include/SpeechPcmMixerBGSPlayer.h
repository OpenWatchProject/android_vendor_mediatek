#ifndef ANDROID_SPEECH_PCM_MIXER_BGSPLAYER_H
#define ANDROID_SPEECH_PCM_MIXER_BGSPLAYER_H

#include <SpeechPcmMixerBase.h>

namespace android {

class SpeechPcmMixerBGSPlayer : public SpeechPcmMixerBase {

public:
    virtual ~SpeechPcmMixerBGSPlayer() {}
    static SpeechPcmMixerBGSPlayer      *GetInstance();
    virtual status_t                    pcmMixerOn(SpeechDriverInterface *pSpeechDriver);
    virtual status_t                    pcmMixerOff(SpeechDriverInterface *pSpeechDriver);
    virtual uint32_t                    getPcmMixerType() { return PCM_MIXER_TYPE_BGS; }
    static  void                        setPcmMixerDlMute(const bool mute_on);
    static  void                        setPcmMixerUlMute(const bool mute_on);

protected:
    SpeechPcmMixerBGSPlayer() {}

private:
    static SpeechPcmMixerBGSPlayer     *mBGSPlayer; // singleton
    static uint8_t                      mPcmMixerDlGain;
    static uint8_t                      mPcmMixerUlGain;
};

} // end namespace android

#endif //ANDROID_SPEECH_PCM_MIXER_BGSPLAYER_H