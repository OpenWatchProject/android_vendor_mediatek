#ifndef ANDROID_AUDIO_ALSA_PLAYBACK_HANDLER_FM_TRANSMITTER_H
#define ANDROID_AUDIO_ALSA_PLAYBACK_HANDLER_FM_TRANSMITTER_H

#include "AudioALSAPlaybackHandlerBase.h"

namespace android {

class WCNChipController;

class AudioALSAPlaybackHandlerFMTransmitter : public AudioALSAPlaybackHandlerBase {
public:
    AudioALSAPlaybackHandlerFMTransmitter(const stream_attribute_t *stream_attribute_source);
    virtual ~AudioALSAPlaybackHandlerFMTransmitter();


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



private:
    WCNChipController *mWCNChipController;

};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_PLAYBACK_HANDLER_FM_TRANSMITTER_H
