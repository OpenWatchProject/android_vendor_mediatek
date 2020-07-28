#ifndef ANDROID_AUDIO_ALSA_PLAYBACK_HANDLER_I2SHDMI_H
#define ANDROID_AUDIO_ALSA_PLAYBACK_HANDLER_I2SHDMI_H

#include "AudioALSAPlaybackHandlerBase.h"

namespace android {

class AudioALSAPlaybackHandlerHDMI : public AudioALSAPlaybackHandlerBase {
public:
    AudioALSAPlaybackHandlerHDMI(const stream_attribute_t *stream_attribute_source);
    virtual ~AudioALSAPlaybackHandlerHDMI();
    virtual status_t SetMHLChipParameter(int channels, int bits, int samplerate);

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
    virtual status_t setScreenState(bool mode, size_t buffer_size, size_t reduceInterruptSize, bool bforce = false);

private:
    struct timespec mNewtime, mOldtime;
    status_t SetMHLChipEnable(int);
    double latencyTime[3];
};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_PLAYBACK_HANDLER_NORMAL_H
