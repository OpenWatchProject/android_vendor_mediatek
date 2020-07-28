#ifndef ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_ECHOREF__USB_H
#define ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_ECHOREF__USB_H

#include "AudioALSACaptureDataProviderBase.h"
#include "AudioALSAPlaybackHandlerUsb.h"

#include <AudioLock.h>

namespace android {
class IAudioALSACaptureDataClient;
class AudioALSACaptureDataProviderEchoRefUsb : public AudioALSACaptureDataProviderBase {
public:
    virtual ~AudioALSACaptureDataProviderEchoRefUsb();

    static AudioALSACaptureDataProviderEchoRefUsb *getInstance();

    /**
     * open/close pcm interface when 1st attach & the last detach
     */
    status_t open();
    status_t close();
    virtual void initEchoRefInfo(stream_attribute_t stream_attribute_source_usb);

    /**
     * PlaybackHandler use this API to provide echo ref data
     */
    status_t writeData(const char *echoRefData, uint32_t dataSize, struct timespec *timestamp);
protected:
    AudioALSACaptureDataProviderEchoRefUsb();



private:

    /**
     * For echo ref SW implementation
     */
    RingBuf          mDataRingBuf;
    AudioLock       mDataBufLock;
    struct timespec  mCaptureStartTime;
    struct timespec  mEstimateTimeStamp;
    uint32_t         mTotalCaptureBufSize;
    AudioALSAPlaybackHandlerUsb *mPlaybackHandlerUsb;

    void             initDataRingBuf(uint32_t size);
    void             deinitDataRingBuf();
    status_t         readData(char *buffer, uint32_t size);
    void             signalDataWaiting();
    status_t         GetCaptureTimeStampByStartTime(time_info_struct_t *Time_Info);
    void             updateStartTimeStamp(struct timespec *timeStamp);
    void             updateTotalCaptureBufSize(uint32_t captureSize);
    void             resetTimeStampInfo();
    uint32_t         compensateSilenceData(uint32_t msec, RingBuf *ringBuf);

    /**
     * singleton pattern
     */
    static AudioALSACaptureDataProviderEchoRefUsb *mAudioALSACaptureDataProviderEchoRefUsb;


    /**
     * pcm read thread
     */
    static void *readThread(void *arg);
    pthread_t hReadThread;

    struct timespec mNewtime, mOldtime; //for calculate latency
    double timerec[3]; //0=>threadloop, 1=>kernel delay, 2=>process delay
};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_ECHOREF__BTCVSD_H
