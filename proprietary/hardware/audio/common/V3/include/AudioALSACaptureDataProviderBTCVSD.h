#ifndef ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_BT_CVSD_H
#define ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_BT_CVSD_H

#include "AudioALSACaptureDataProviderBase.h"

namespace android {

class WCNChipController;
class AudioBTCVSDControl;

class AudioALSACaptureDataProviderBTCVSD : public AudioALSACaptureDataProviderBase {
public:
    virtual ~AudioALSACaptureDataProviderBTCVSD();

    static AudioALSACaptureDataProviderBTCVSD *getInstance();

    /**
     * open/close pcm interface when 1st attach & the last detach
     */
    status_t open();
    status_t close();



protected:
    AudioALSACaptureDataProviderBTCVSD();


    uint32_t readDataFromBTCVSD(void *linear_buffer, bool *isMuteData, struct timespec *bufferTimeStamp);

    /**
     * Bli SRC
     */
    status_t initBliSrc();
    status_t deinitBliSrc();
    status_t doBliSrc(void *pInBuffer, uint32_t inBytes, void **ppOutBuffer, uint32_t *pOutBytes);

    /**
     * Timestamp related
     */
    status_t getKernelTimeStamp(struct timespec *captureStartTime);

    struct timespec getCurrentTimeStamp();

    status_t updateStartTimeStamp(struct timespec timeStamp);

    status_t updateCaptureTimeStampByStartTime(uint32_t bufferSize);

    bool isBufferTimeStampMatchEstimated(struct timespec timeStamp);

    uint32_t getLatencyTime();

    struct timespec  mCaptureStartTime;

    struct timespec mEstimatedBufferTimeStamp;

private:
    /**
     * singleton pattern
     */
    static AudioALSACaptureDataProviderBTCVSD *mAudioALSACaptureDataProviderBTCVSD;

    WCNChipController *mWCNChipController;
    AudioBTCVSDControl *mAudioBTCVSDControl;
    struct mixer *mMixer;
    bool mBTIrqReceived;

    /**
     * pcm read thread
     */
    static void *readThread(void *arg);
    pthread_t hReadThread;

    uint32_t mReadBufferSize;

    MtkAudioSrcBase *mBliSrc;
    char *mBliSrcOutputBuffer;

    int mFd2;

    struct timespec mNewtime, mOldtime, mOldCVSDReadTime;
    double mTimerec[5];

};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_BT_CVSD_H
