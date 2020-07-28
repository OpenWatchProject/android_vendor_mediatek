#ifndef ANDROID_AUDIO_ALSA_CAPTURE_HANDLER_SYNCIO_H
#define ANDROID_AUDIO_ALSA_CAPTURE_HANDLER_SYNCIO_H

#include "AudioALSACaptureHandlerBase.h"

namespace android {
class AudioALSACaptureHandlerSyncIO : public AudioALSACaptureHandlerBase {
public:
    AudioALSACaptureHandlerSyncIO(stream_attribute_t *stream_attribute_target);
    virtual ~AudioALSACaptureHandlerSyncIO();

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

protected:
    /**
     * init audio hardware
     */
    virtual status_t init();



private:

};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_CAPTURE_HANDLER_SYNCIO_H