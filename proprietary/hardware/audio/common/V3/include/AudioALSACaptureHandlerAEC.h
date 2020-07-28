#ifndef ANDROID_AUDIO_ALSA_CAPTURE_HANDLER_AEC_H
#define ANDROID_AUDIO_ALSA_CAPTURE_HANDLER_AEC_H

#include "AudioALSACaptureHandlerBase.h"

namespace android {
class AudioALSACaptureHandlerAEC : public AudioALSACaptureHandlerBase {
public:
    AudioALSACaptureHandlerAEC(stream_attribute_t *stream_attribute_target);
    virtual ~AudioALSACaptureHandlerAEC();

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
     * Update BesRecord Parameters
     */
    virtual status_t UpdateBesRecParam();

protected:
    /**
     * init audio hardware
     */
    virtual status_t init();



private:

    stream_attribute_t mStreamAttributeTargetEchoRef; // to stream in EchoRef

};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_CAPTURE_HANDLER_AEC_H
