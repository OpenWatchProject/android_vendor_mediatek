#ifndef ANDROID_AUDIO_ALSA_CAPTURE_HANDLER_AAUDIO_H
#define ANDROID_AUDIO_ALSA_CAPTURE_HANDLER_AAUDIO_H

#include "AudioALSACaptureHandlerBase.h"

namespace android {
class AudioALSACaptureHandlerAAudio : public AudioALSACaptureHandlerBase {
public:
    AudioALSACaptureHandlerAAudio(stream_attribute_t *stream_attribute_target);
    virtual ~AudioALSACaptureHandlerAAudio();

    /**
     * open/close audio hardware
     */
    virtual status_t open();
    virtual status_t close();
    virtual status_t routing(const audio_devices_t input_device);


    /**
     * read data from audio hardware
     */
    virtual ssize_t  read(void *buffer, ssize_t bytes);

    /**
     * AAudio MMAP
     */
    class AudioALSACaptureDataProviderAAudio *mDataProvider;
    virtual status_t    start();
    virtual status_t    stop();
    virtual status_t    createMmapBuffer(int32_t min_size_frames,
                                  struct audio_mmap_buffer_info *info);
    virtual status_t    getMmapPosition(struct audio_mmap_position *position);

protected:
    /**
     * init audio hardware
     */
    virtual status_t init();



private:

};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_CAPTURE_HANDLER_AAUDIO_H
