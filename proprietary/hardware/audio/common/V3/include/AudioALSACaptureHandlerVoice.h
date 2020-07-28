#ifndef ANDROID_AUDIO_ALSA_CAPTURE_HANDLER_VOICE_H
#define ANDROID_AUDIO_ALSA_CAPTURE_HANDLER_VOICE_H

#include "AudioALSACaptureHandlerBase.h"

namespace android {

class AudioALSACaptureHandlerVoice : public AudioALSACaptureHandlerBase {
public:
    AudioALSACaptureHandlerVoice(stream_attribute_t *stream_attribute_target);
    virtual ~AudioALSACaptureHandlerVoice();

    /**
     * open/close speech driver
     */
    virtual status_t open();
    virtual status_t close();
    virtual status_t routing(const audio_devices_t input_device);


    /**
     * read data from speech driver
     */
    virtual ssize_t  read(void *buffer, ssize_t bytes);


protected:
    /**
     * init audio speech driver
     */
    virtual status_t init();
};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_CAPTURE_HANDLER_VOICE_H
