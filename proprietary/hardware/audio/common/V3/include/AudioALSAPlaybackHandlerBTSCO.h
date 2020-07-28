#ifndef ANDROID_AUDIO_ALSA_PLAYBACK_HANDLER_BT_SCO_H
#define ANDROID_AUDIO_ALSA_PLAYBACK_HANDLER_BT_SCO_H

#include "AudioALSAPlaybackHandlerBase.h"

namespace android {

class WCNChipController;

class AudioALSAPlaybackHandlerBTSCO : public AudioALSAPlaybackHandlerBase {
public:
    AudioALSAPlaybackHandlerBTSCO(const stream_attribute_t *stream_attribute_source);
    virtual ~AudioALSAPlaybackHandlerBTSCO();


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

    struct timespec mNewtime, mOldtime;
    double latencyTime[3];
};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_PLAYBACK_HANDLER_BT_SCO_H
