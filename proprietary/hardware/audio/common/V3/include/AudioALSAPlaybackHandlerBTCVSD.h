#ifndef ANDROID_AUDIO_ALSA_PLAYBACK_HANDLER_BT_CVSD_H
#define ANDROID_AUDIO_ALSA_PLAYBACK_HANDLER_BT_CVSD_H

#include "AudioALSAPlaybackHandlerBase.h"

namespace android {

class WCNChipController;
class AudioALSACaptureDataProviderEchoRefBTCVSD;
class AudioBTCVSDControl;

class AudioALSAPlaybackHandlerBTCVSD : public AudioALSAPlaybackHandlerBase {
public:
    AudioALSAPlaybackHandlerBTCVSD(const stream_attribute_t *stream_attribute_source);
    virtual ~AudioALSAPlaybackHandlerBTCVSD();


    /**
     * open/close audio hardware
     */
    virtual status_t open();
    virtual status_t close();
    virtual status_t routing(const audio_devices_t output_devices);

    virtual int getLatency();

    /**
     * write data to audio hardware
     */
    virtual ssize_t  write(const void *buffer, size_t bytes);


    /**
     * get hardware buffer info (framecount)
     */
    virtual status_t getHardwareBufferInfo(time_info_struct_t *HWBuffer_Time_Info __unused) { return INVALID_OPERATION; }


private:
    /**
     * For echo ref SW implementation
     */
    bool writeEchoRefDataToDataProvider(AudioALSACaptureDataProviderEchoRefBTCVSD *dataProvider, const char *echoRefData, uint32_t dataSize);
    status_t updateStartTimeStamp();

    stream_attribute_t mStreamAttributeTargetEchoRef; // to echoref
    struct timespec mEchoRefStartTime;
    uint32_t mTotalEchoRefBufSize;

    AudioALSACaptureDataProviderEchoRefBTCVSD *mDataProviderEchoRefBTCVSD;
    WCNChipController *mWCNChipController;
    AudioBTCVSDControl *mAudioBTCVSDControl;
    struct mixer *mMixer;
    bool mInitWrite;
    unsigned int mWrittenFrame;

    int mFd2;

    struct timespec mNewtime, mOldtime;
    double latencyTime[4];
};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_PLAYBACK_HANDLER_BT_CVSD_H
