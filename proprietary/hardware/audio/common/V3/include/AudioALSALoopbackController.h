#ifndef ANDROID_AUDIO_ALSA_LOOPBACK_CONTROLLER_H
#define ANDROID_AUDIO_ALSA_LOOPBACK_CONTROLLER_H

#include <tinyalsa/asoundlib.h> // TODO(Harvey): move it

#include "AudioType.h"
#include <AudioLock.h>
#include "AudioUtility.h"
#include "LoopbackManager.h"

namespace android {

class AudioALSAHardwareResourceManager;

class AudioALSALoopbackController {
public:
    virtual ~AudioALSALoopbackController();

    static AudioALSALoopbackController *getInstance();

    virtual status_t open(const audio_devices_t output_devices, const audio_devices_t input_device, loopback_t loopbackType);
    virtual status_t close();
    virtual status_t SetApBTCodec(bool enable_codec);
    virtual bool IsAPBTLoopbackWithCodec(void);
    virtual status_t OpenAudioLoopbackControlFlow(const audio_devices_t input_device, const audio_devices_t output_device);
    virtual status_t CloseAudioLoopbackControlFlow(void);
    //#if defined(BTCVSD_LOOPBACK_WITH_CODEC)
#if 1 //0902
    class AudioMTKLoopbackThread : public Thread {
    public:
        AudioMTKLoopbackThread();
        virtual ~AudioMTKLoopbackThread();
        virtual status_t    readyToRun();
        virtual void        onFirstRef();
    private:
        String8 mName;
        virtual bool threadLoop();
    };
#endif

protected:
    AudioALSALoopbackController();

    void setLoopbackUseLCh(bool enable);

    AudioALSAHardwareResourceManager *mHardwareResourceManager;

    AudioLock mLock;

    struct pcm_config mConfig;

    struct pcm *mPcmDL;
    struct pcm *mPcmUL;
    String8 mApTurnOnSequence;
    String8 mApTurnOnSequence2;
    String8 mTurnOnSeqCustDev1;
    String8 mTurnOnSeqCustDev2;

    struct mixer *mMixer;

private:
    /**
     * singleton pattern
     */
    static AudioALSALoopbackController *mAudioALSALoopbackController;

    int mFd2;
    bool mBtLoopbackWithCodec;
    bool mBtLoopbackWithoutCodec;
    //#if defined(BTCVSD_LOOPBACK_WITH_CODEC)
#if 1 //0902
    sp<AudioMTKLoopbackThread>  mBTCVSDLoopbackThread;
#endif
    //for BT SW BT CVSD loopback test
    bool mUseBtCodec;

};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_LOOPBACK_CONTROLLER_H
