#ifndef ANDROID_AUDIO_ALSA_SPEECH_LOOPBACK_CONTROLLER_H
#define ANDROID_AUDIO_ALSA_SPEECH_LOOPBACK_CONTROLLER_H

#include <tinyalsa/asoundlib.h> // TODO(Harvey): move it

#include "AudioType.h"
#include "SpeechType.h"

#include <AudioLock.h>
#include "LoopbackManager.h"

namespace android {

class AudioALSAHardwareResourceManager;
class SpeechDriverFactory;

class AudioALSASpeechLoopbackController {
public:
    virtual ~AudioALSASpeechLoopbackController();

    static AudioALSASpeechLoopbackController *getInstance();

    virtual status_t        open(const audio_devices_t output_devices, const audio_devices_t input_device, loopback_t loopbackType);
    virtual status_t        close();
    virtual status_t        SetModemBTCodec(bool enable_codec);
    virtual status_t        OpenModemLoopbackControlFlow(const audio_devices_t input_device, const audio_devices_t output_device);
    virtual status_t        CloseModemLoopbackControlFlow(void);

protected:
    AudioALSASpeechLoopbackController();

    AudioALSAHardwareResourceManager *mHardwareResourceManager;

    SpeechDriverFactory    *mSpeechDriverFactory;


    AudioLock               mLock;

    struct pcm_config mConfig;

    struct pcm *mPcmUL;
    struct pcm *mPcmDL;
    String8 mApTurnOnSequence;
    String8 mApTurnOnSequence2;
    String8 mTurnOnSeqCustDev1;
    String8 mTurnOnSeqCustDev2;

private:
    static AudioALSASpeechLoopbackController *mSpeechLoopbackController; // singleton

    //for BT SW BT CVSD loopback test
    bool mUseBtCodec;

};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_SPEECH_LOOPBACK_CONTROLLER_H
