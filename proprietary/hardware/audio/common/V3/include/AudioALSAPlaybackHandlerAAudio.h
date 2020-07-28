#ifndef ANDROID_AUDIO_ALSA_PLAYBACK_HANDLER_AAUDIO_H
#define ANDROID_AUDIO_ALSA_PLAYBACK_HANDLER_AAUDIO_H

#include "AudioALSAPlaybackHandlerBase.h"


namespace android {

class AudioALSAPlaybackHandlerAAudio : public AudioALSAPlaybackHandlerBase {
public:
    AudioALSAPlaybackHandlerAAudio(const stream_attribute_t *stream_attribute_source);
    virtual ~AudioALSAPlaybackHandlerAAudio();


    /**
     * open/close audio hardware
     */
    virtual status_t open();
    virtual status_t close();
    virtual status_t routing(const audio_devices_t output_devices);


    /**
     * write data to audio hardware
     */
    virtual ssize_t  write(const void *buffer, size_t bytes);

    virtual status_t setFilterMng(AudioMTKFilterManager *pFilterMng);


    /**
     * low latency
     */
    virtual status_t setScreenState(bool mode, size_t buffer_size, size_t reduceInterruptSize, bool bforce = false);


    /**
     * AAudio MMAP
     */
    virtual status_t    start();
    virtual status_t    stop();
    virtual status_t    createMmapBuffer(int32_t min_size_frames,
                                  struct audio_mmap_buffer_info *info);
    virtual status_t    getMmapPosition(struct audio_mmap_position *position);

private:

    /**
     * AAudio MMAP
     */
    int64_t mTime_nanoseconds;
    int32_t mPosition_frames;
    int32_t mMin_size_frames;

};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_PLAYBACK_HANDLER_AAUDIO_H
