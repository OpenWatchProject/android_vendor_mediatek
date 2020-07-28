#ifndef ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_ECHOREF__BTCVSD_H
#define ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_ECHOREF__BTCVSD_H

#include "AudioALSACaptureDataProviderBase.h"

#include <AudioLock.h>
#include <pthread.h>

namespace android {

class WCNChipController;

class AudioALSACaptureDataProviderEchoRefBTCVSD : public AudioALSACaptureDataProviderBase {
public:
    virtual ~AudioALSACaptureDataProviderEchoRefBTCVSD();

    static AudioALSACaptureDataProviderEchoRefBTCVSD *getInstance();

    /**
     * open/close pcm interface when 1st attach & the last detach
     */
    status_t open();
    status_t close();

    /**
     * PlaybackHandler use this API to provide echo ref data
     */
    status_t writeData(const char *echoRefData, uint32_t dataSize, struct timespec *timestamp);

protected:
    AudioALSACaptureDataProviderEchoRefBTCVSD();



private:
    /**
     * For echo ref SW implementation
     */
    RingBuf          mDataRingBuf;
    AudioLock        mDataBufLock;
    struct timespec  mCaptureStartTime;
    struct timespec  mEstimateTimeStamp;
    struct timespec  mOldEstimateTimeStamp;
    uint32_t         mTotalCaptureBufSize;

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
    static AudioALSACaptureDataProviderEchoRefBTCVSD *mAudioALSACaptureDataProviderEchoRefBTCVSD;

    WCNChipController *mWCNChipController;

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
