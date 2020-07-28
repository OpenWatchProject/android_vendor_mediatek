#ifndef ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_BASE_H
#define ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_BASE_H

#include <utils/KeyedVector.h>

#include <tinyalsa/asoundlib.h>

#include <pthread.h>


#include "AudioType.h"
#include <AudioLock.h>
#include "AudioUtility.h"
#include "AudioALSADeviceParser.h"

#ifdef MTK_AUDIO_SCP_SUPPORT
struct ipi_msg_t;
#endif

#ifdef MTK_AUDIODSP_SUPPORT
#include "AudioDspType.h"
#endif

typedef int (*audio_pcm_read_wrapper_fp_t)(struct pcm *pcm, void *data, unsigned int count);

namespace android {

class IAudioALSACaptureDataClient;
class AudioALSAHardwareResourceManager;
class AudioMessengerIPI;

/// Observer pattern: Subject
class AudioALSACaptureDataProviderBase {
public:
    virtual ~AudioALSACaptureDataProviderBase();

    /**
     * attach/detach client to capture data provider
     */
    void     attach(IAudioALSACaptureDataClient *pCaptureDataClient);
    void     detach(IAudioALSACaptureDataClient *pCaptureDataClient);

    void                      configStreamAttribute(const stream_attribute_t *attribute);
    const stream_attribute_t *getStreamAttributeSource() { return &mStreamAttributeSource; }
    stream_attribute_t getStreamAttributeTargetDSP() { return mStreamAttributeTargetDSP; }

    bool isEnable() { return mEnable; }

    int getCapturePosition(int64_t *frames, int64_t *time);

    capture_provider_t getCaptureDataProviderType() { return mCaptureDataProviderType; }

    virtual uint32_t getLatencyTime() { return mlatency; }

    virtual bool getReadThreadReady() { return mReadThreadReady; }
    void signalPcmStart();
    status_t writeData(const char *echoRefData, uint32_t dataSize, struct timespec *timestamp);

    virtual status_t getPcmStatus();

    /**
     * AAudio MMAP
     */
    virtual status_t    start();
    virtual status_t    stop();
    virtual status_t    createMmapBuffer(int32_t min_size_frames,
                                  struct audio_mmap_buffer_info *info);
    virtual status_t    getMmapPosition(struct audio_mmap_position *position);
    virtual ssize_t doBcvProcess(void *buffer, ssize_t bytes);



protected:
    AudioALSACaptureDataProviderBase();
    AudioMessengerIPI *mAudioMessengerIPI;
    AudioALSAHardwareResourceManager *mHardwareResourceManager;

    /**
     * pcm driver open/close
     */
    status_t         openPcmDriver(const unsigned int device); // TODO(Harvey): Query device by string
    status_t         openPcmDriverWithFlag(const unsigned int device, unsigned int flag);
    status_t         closePcmDriver();

    /**
     * provide captrue data to all attached clients
     */
    void     provideCaptureDataToAllClients(const uint32_t open_index);


    //echoref+++
    /**
     * provide captrue data to all attached clients
     */
    void     provideEchoRefCaptureDataToAllClients(const uint32_t open_index);
    //echoref---


    /**
       * pcm write function
       */
    int pcmRead(struct pcm *mpcm, void *data, unsigned int count);


    /**
     * open/close pcm interface when 1st attach & the last detach
     */
    virtual status_t open() = 0;
    virtual status_t close() = 0;

    /**
     * open/close pmic interface when attach & detach
     */
    void enablePmicInputDevice(bool enable);

    /**
     * pcm read time stamp
     */
    status_t GetCaptureTimeStamp(time_info_struct_t *Time_Info, unsigned int read_size);

    /**
     * calculate time stamp instead of pcm_get_htimestamp
     */
    status_t calculateCaptureTimeStamp(time_info_struct_t *Time_Info, unsigned int read_size);

    /**
     * update start time stamp by monotonic time
     */
    status_t updateStartTimeStamp();

    int64_t mCaptureFramesReaded;
    timespec mCaptureTimeStamp;
    bool mCaptureTimeStampSupport;

    struct timespec  mCaptureStartTime;
    uint32_t mTotalReadBytes;

    /**
     * check if any attached clients has low latency requirement
     */
    bool HasLowLatencyCapture(void);

    /**
     * Set the thread priority
     */
     void setThreadPriority(void);

    /**
     * enable state
     */
    bool mEnable;
    static status_t mPcmStatus;

    /**
     * Provider Index
     */
    uint32_t mOpenIndex;


    /**
     * latency time
     */
    uint32_t mlatency;


    /**
     * lock
     */
    AudioLock mTimeStampLock;  // Protect timestamp

    /**
     * client vector
     */
    KeyedVector<void *, IAudioALSACaptureDataClient *> mCaptureDataClientVector;

    /**
     * local ring buffer
     */
    RingBuf             mPcmReadBuf;


    /**
     * tinyalsa pcm interface
     */
    struct pcm_config mConfig;
    struct pcm *mPcm;
    String8 mApTurnOnSequence;
    bool isNeedSyncPcmStart();
    void waitPcmStart();


    /**
     * pcm start
     */
    AudioLock       mStartLock; // first
    bool            mStart;
    bool            mReadThreadReady;


    /**
     * pcm read attribute
     */
    stream_attribute_t mStreamAttributeSource;
    stream_attribute_t mStreamAttributeTargetDSP;

    capture_provider_t mCaptureDataProviderType;

    unsigned int mPcmflag;
    audio_pcm_read_wrapper_fp_t audio_pcm_read_wrapper_fp;

    void  OpenPCMDump(const char *class_name);
    void  ClosePCMDump(void);
    void  WritePcmDumpData(void);
    String8 mDumpFileName;
    FILE *mPCMDumpFile;
    FILE *mPCMDumpFile4ch;

#ifdef MTK_AUDIODSP_SUPPORT
    void  OpenPCMDumpDsp(const char *class_name);
    void  ClosePCMDumpDsp(void);
    void  WritePcmDumpDataDsp(void);
    String8 mDumpFileNameDsp;
    FILE *mPCMDumpFileDsp;
    FILE *pcmin_dump_array[DEBUG_PCMDUMP_NUM];
    static uint32_t mDumpFileNumDsp;

    /* dsp pcm dump */
    void get_task_pcmdump_info(int task_id, int param1, void **pcm_dump);
    void set_task_pcmdump_info(int task_id, int param1, void *pcm_dump);
    void processDmaMsg(struct ipi_msg_t *msg,
                       void *buf,
                       uint32_t size);
    static void processDmaMsgWrapper(struct ipi_msg_t *msg,
                                     void *buf,
                                     uint32_t size,
                                     void *arg);
#endif

private:
    /**
     * lock
     */
    AudioLock    mEnableLock; // first
    AudioLock    mClientLock; // second

    static int      mDumpFileNum;
};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_BASE_H
