#ifndef ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_AAUDIO_H
#define ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_AAUDIO_H

#include "AudioALSACaptureDataProviderBase.h"

namespace android {

class AudioALSACaptureDataProviderAAudio : public AudioALSACaptureDataProviderBase {
public:
    virtual ~AudioALSACaptureDataProviderAAudio();

    static AudioALSACaptureDataProviderAAudio *getInstance();

    /**
     * open/close pcm interface when 1st attach & the last detach
     */
    status_t open();
    status_t close();

    /**
     * AAudio MMAP
     */
    // Used in pairs
    static AudioALSACaptureDataProviderAAudio *requestInstance();
    static void freeInstance();

    // MMAP API
    virtual status_t    start();
    virtual status_t    stop();
    virtual status_t    createMmapBuffer(int32_t min_size_frames,
                                  struct audio_mmap_buffer_info *info);
    virtual status_t    getMmapPosition(struct audio_mmap_position *position);


protected:
    AudioALSACaptureDataProviderAAudio();



private:
    /**
     * singleton pattern
     */
    static AudioALSACaptureDataProviderAAudio *mAudioALSACaptureDataProviderAAudio;
    struct mixer *mMixer;


    /**
     * AAudio MMAP
     */
    static AudioLock           mLock;
    static int          mUsageCount;
    int64_t mTime_nanoseconds;
    int32_t mPosition_frames;
    int32_t mMin_size_frames;
    struct timespec     mCreateMmapTime;
    bool mPmicEnable;

};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_AAUDIO_H