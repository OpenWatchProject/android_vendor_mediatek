#ifndef ANDROID_AUDIO_ALSA_CAPTURE_DATA_CLIENT_AURISYS_NORMAL_H
#define ANDROID_AUDIO_ALSA_CAPTURE_DATA_CLIENT_AURISYS_NORMAL_H

#include "IAudioALSACaptureDataClient.h"

#include <pthread.h>


#include "AudioType.h"
#include "AudioUtility.h"

#include <audio_ringbuf.h>

#include <AudioLock.h>


// Aurisys Framework
struct aurisys_lib_manager_t;
struct aurisys_lib_manager_config_t;
struct audio_pool_buf_t;
struct data_buf_t;


class AudioVolumeInterface;

namespace android {

class AudioALSACaptureDataProviderBase;
class AudioALSACaptureDataProviderEchoRef;

//Android Native Preprocess effect +++
class AudioPreProcess;
//Android Native Preprocess effect ---


/// Observer pattern: Observer
class AudioALSACaptureDataClientAurisysNormal : public IAudioALSACaptureDataClient {
public:
    AudioALSACaptureDataClientAurisysNormal(
        AudioALSACaptureDataProviderBase *pCaptureDataProvider,
        stream_attribute_t *stream_attribute_target,
        AudioALSACaptureDataProviderBase *pCaptureDataProviderEchoRef);

    virtual ~AudioALSACaptureDataClientAurisysNormal();


    /**
     * set client index
     */
    inline void        *getIdentity() const { return (void *)this; }

    /**
     * set/get raw frame count from hardware
     */
    inline void setRawStartFrameCount(int64_t frameCount) { mRawStartFrameCount = frameCount * mStreamAttributeTarget->sample_rate / mStreamAttributeSource->sample_rate; }
    virtual inline int64_t getRawStartFrameCount() { return mRawStartFrameCount; }

    /**
     * let provider copy raw data to client
     */
    virtual uint32_t    copyCaptureDataToClient(RingBuf pcm_read_buf); // called by capture data provider


    /**
     * let handler read processed data from client
     */
    virtual ssize_t     read(void *buffer, ssize_t bytes); // called by capture handler


    /**
     * check if the attached client has low latency requirement
     */
    bool    IsLowLatencyCapture(void);


    /**
     * Query captured frames & time stamp
     */
    int getCapturePosition(int64_t *frames, int64_t *time);


    /**
     * EchoRef
     */
    void AddEchoRefDataProvider(AudioALSACaptureDataProviderBase *pCaptureDataProvider, stream_attribute_t *stream_attribute_target);
    uint32_t copyEchoRefCaptureDataToClient(RingBuf pcm_read_buf);


    /**
     * Update BesRecord Parameters
     */
    status_t UpdateBesRecParam() { return INVALID_OPERATION; } /* TODO: remove it */


    /**
     * Attribute
     */
    const stream_attribute_t *getStreamAttributeSource() { return mStreamAttributeSource; }


    /**
     * sync pcm start
     */
    bool isNeedSyncPcmStart() { return IsAECEnable(); }


    /**
     * StreamIn reopen
     */
    virtual bool getStreamInReopen() { return mStreamInReopen; }
    virtual void setStreamInReopen(bool state) { mStreamInReopen = state; }


private:
    AudioALSACaptureDataClientAurisysNormal() {}


    /**
     * stream attribute
     */
    const stream_attribute_t *mStreamAttributeSource; // from audio hw
    stream_attribute_t       *mStreamAttributeTarget; // to stream in
    bool IsVoIPEnable(void) { return mStreamAttributeTarget->mVoIPEnable; }


    /**
     * for data provider
     */
    AudioALSACaptureDataProviderBase *mCaptureDataProvider;
    bool getPcmStatus(void);
    int64_t mRawStartFrameCount;


    /**
     * for StreamIn reopen
     */
    void handleLockTimeout();


    /**
     * gain control
     */
    bool IsNeedApplyVolume();
    status_t ApplyVolume(void *Buffer, uint32_t BufferSize);
    bool mMicMute;
    bool mMuteTransition;
    AudioVolumeInterface *mAudioALSAVolumeController;


    /**
     * process raw data to processed data
     */
    bool            mEnable;

    bool            mStreamInReopen;

    static void    *processThread(void *arg);
    pthread_t       hProcessThread;
    bool            mProcessThreadLaunched;

    uint32_t        mLatency;

    audio_ringbuf_t mRawDataBuf;
    AudioLock       mRawDataBufLock;
    struct timespec mRawDataBufTimeStamp;
    uint32_t        mRawDataPeriodBufSize;

    audio_ringbuf_t mProcessedDataBuf;
    AudioLock       mProcessedDataBufLock;
    uint32_t        mProcessedDataPeriodBufSize;

    /**
     * EchoRef
     */
    bool IsAECEnable(void) { return (mCaptureDataProviderEchoRef != NULL); }
    void syncEchoRefData(const uint8_t data_buf_type, audio_ringbuf_t *rb_data); /* data_buf_type_t */

    AudioALSACaptureDataProviderBase *mCaptureDataProviderEchoRef;
    const stream_attribute_t *mStreamAttributeSourceEchoRef; // from audio hw, need the same as DL1 stream out used

    audio_ringbuf_t mEchoRefDataBuf;
    struct timespec mEchoRefDataBufTimeStamp;
    uint32_t        mEchoRefDataPeriodBufSize;

    bool            mIsEchoRefDataSync;


    /**
     * Aurisys Framework
     */
    void CreateAurisysLibManager();
    void InitArsiTaskConfig(struct aurisys_lib_manager_config_t *pManagerConfig);
    void InitBufferConfig(struct aurisys_lib_manager_t *manager);
    void DestroyAurisysLibManager();

    struct aurisys_lib_manager_t *mAurisysLibManager;
    struct aurisys_lib_manager_config_t *mManagerConfig;
    uint32_t mAurisysScenario;

    audio_pool_buf_t *mAudioPoolBufUlIn;
    audio_pool_buf_t *mAudioPoolBufUlOut;
    audio_pool_buf_t *mAudioPoolBufUlAec;
    audio_pool_buf_t *mAudioPoolBufDlIn;
    audio_pool_buf_t *mAudioPoolBufDlOut;
    struct data_buf_t *mLinearOut;


    /**
     * Depop
     */
    uint32_t mDropPopSize;


    //Android Native Preprocess effect +++
    AudioPreProcess *mAudioPreProcessEffect;
    uint32_t NativePreprocess(void *buffer, uint32_t bytes);
    void CheckNativeEffect(void);
    //Android Native Preprocess effect ---




};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_CAPTURE_DATA_CLIENT_AURISYS_NORMAL_H

