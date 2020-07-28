#ifndef ANDROID_AUDIO_ALSA_PLAYBACK_HANDLER_BASE_H
#define ANDROID_AUDIO_ALSA_PLAYBACK_HANDLER_BASE_H

#include <tinyalsa/asoundlib.h> // TODO(Harvey): move it
#include "sound/compress_params.h"
#include <tinycompress/tinycompress.h>
#include <sound/asound.h>
#include "sound/compress_offload.h"

#include "AudioType.h"
#include "AudioALSADeviceParser.h"
#include <hardware/audio.h>
#include "AudioALSANLEController.h"
#include "MtkAudioComponent.h"


#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
struct aurisys_lib_manager_t;
struct aurisys_lib_manager_config_t;
struct audio_pool_buf_t;
struct data_buf_t;
#endif

typedef int (*audio_pcm_write_wrapper_fp_t)(struct pcm *pcm, const void *data, unsigned int count);


#ifdef MTK_AUDIO_SCP_SUPPORT
struct ipi_msg_t;
#endif


namespace android {

class AudioALSADataProcessor;

class AudioALSAHardwareResourceManager;

class AudioMTKFilterManager;

class AudioALSAHyBridNLEManager;

class AudioMessengerIPI;

struct WriteSmoother;



class AudioALSAPlaybackHandlerBase {

public:
    virtual ~AudioALSAPlaybackHandlerBase();


    /**
     * set handler index
     */
    inline void         setIdentity(const uint32_t identity) { mIdentity = identity; }
    inline uint32_t     getIdentity() const { return mIdentity; }


    /**
     * open/close audio hardware
     */
    virtual status_t open() = 0;
    virtual status_t close() = 0;
    virtual status_t routing(const audio_devices_t output_devices) = 0;

    virtual int pause() { return -ENODATA; }
    virtual int resume() { return -ENODATA; }
    virtual int flush() { return 0; }
    virtual int drain(audio_drain_type_t type __unused) { return 0; }
    virtual status_t setVolume(uint32_t vol __unused) { return INVALID_OPERATION; }

    virtual int getLatency();

    virtual int setSuspend(bool suspend __unused) { return 0; }
    virtual String8 getPlaybackTurnOnSequence(unsigned int turnOnSeqType,
                                              const char *playbackSeq);
    /**
     * write data to audio hardware
     */
    virtual ssize_t  write(const void *buffer, size_t bytes) = 0;
    virtual int preWriteOperation(const void *buffer, size_t bytes);

    /**
     * Post processing
     */
    virtual status_t setFilterMng(AudioMTKFilterManager *pFilterMng);


    /**
     * low latency
     */
    virtual status_t setScreenState(bool mode, size_t buffer_size, size_t reduceInterruptSize, bool bforce = false);


    /**
     * get hardware buffer info (framecount)
     */
    virtual status_t getHardwareBufferInfo(time_info_struct_t *HWBuffer_Time_Info);

    virtual status_t get_timeStamp(unsigned long *frames, unsigned int *samplerate);

    virtual status_t updateHardwareBufferInfo(size_t sourceWrittenBytes, uint32_t targetWrittenBytes);

    playback_handler_t getPlaybackHandlerType();

    /**
     * get stream attribute for kernel buffer
     */
    const stream_attribute_t *getStreamAttributeTarget() { return &mStreamAttributeTarget; }

    status_t setComprCallback(stream_callback_t StreamCbk, void *CbkCookie);

    /**
     * set first write
     */
    virtual void setFirstDataWriteFlag(bool bFirstDataWrite) { mFirstDataWrite = bFirstDataWrite; }

    virtual uint64_t getBytesWriteKernel() { return mBytesWriteKernel; }

    virtual void resetBytesWriteKernel() {  mBytesWriteKernel = 0; }

    /**
     * update mode
     */
    virtual int updateAudioMode(audio_mode_t mode);

#ifdef MTK_AUDIODSP_SUPPORT
    /* dsp pcm dump */
    static void processDmaMsgWrapper(struct ipi_msg_t *msg,
                                     void *buf,
                                     uint32_t size,
                                     void *arg);

    void get_task_pcmdump_info(int task_id, int param1, void **pcm_dump);
    void set_task_pcmdump_info(int task_id, int param1, void *pcm_dump);
    int setDspDumpWakelock(bool condition);
    void processDmaMsg(struct ipi_msg_t *msg,
                       void *buf,
                       uint32_t size);

    void  OpenPCMDumpDSP(const char *className, uint8_t task_scene);
    void  ClosePCMDumpDSP(uint8_t task_scene);
#endif
    /**
     * update offload routing status
     */
    virtual bool setOffloadRoutingFlag(bool enable) { return enable;}


    /**
     * write smoother
     */
    virtual void updateSmootherTime(const uint64_t bufferTimeUs);

    /**
     * AAudio MMAP
     */
    virtual status_t    start();
    virtual status_t    stop();
    virtual status_t    createMmapBuffer(int32_t min_size_frames,
                                  struct audio_mmap_buffer_info *info);
    virtual status_t    getMmapPosition(struct audio_mmap_position *position);


protected:
    AudioALSAPlaybackHandlerBase(const stream_attribute_t *stream_attribute_source);

    AudioMessengerIPI *mAudioMessengerIPI;


    /**
     * pcm driver open/close
     */
    status_t         openPcmDriver(const unsigned int device); // TODO(Harvey): Query device by string
    status_t         openPcmDriverWithFlag(const unsigned int device, unsigned int flag);
    status_t         openComprDriver(const unsigned int device);


    status_t         closePcmDriver();
    status_t         closeComprDriver();

    /**
        * pcm driver list
        */
    status_t         ListPcmDriver(const unsigned int card, const unsigned int device);

    /**
        * pcm write function
        */
    status_t         pcmWrite(struct pcm *pcm, const void *data, unsigned int count);

    /**
        * stereo to mono for speaker
        */
    status_t doStereoToMonoConversionIfNeed(void *buffer, size_t bytes);


    /**
     * Post processing
     */
    status_t         initPostProcessing();
    status_t         deinitPostProcessing();
    status_t         doPostProcessing(void *pInBuffer, uint32_t inBytes, void **ppOutBuffer, uint32_t *pOutBytes);

    /**
     * Dc Removal
     */
    int32           initDcRemoval();
    int32           deinitDcRemoval();
    int32           doDcRemoval(void *pInBuffer, uint32_t inBytes, void **ppOutBuffer, uint32_t *pOutBytes);

    /**
     * Bli SRC
     */
    status_t         initBliSrc();
    status_t         deinitBliSrc();
    status_t         doBliSrc(void *pInBuffer, uint32_t inBytes, void **ppOutBuffer, uint32_t *pOutBytes);


    /**
     * Bit Converter
     */
    pcm_format       transferAudioFormatToPcmFormat(const audio_format_t audio_format) const;
    status_t         initBitConverter();
    status_t         deinitBitConverter();
    status_t         doBitConversion(void *pInBuffer, uint32_t inBytes, void **ppOutBuffer, uint32_t *pOutBytes);

    status_t         initDataPending();
    status_t         DeinitDataPending();
    status_t         dodataPending(void *pInBuffer, uint32_t inBytes, void **ppOutBuffer, uint32_t *pOutBytes);


    /**
     * NLE processing
     */
    status_t         initNLEProcessing();
    status_t         deinitNLEProcessing();
    status_t         doNLEProcessing(void *pInBuffer, size_t inBytes);
    playback_handler_t mPlaybackHandlerType;

    AudioALSAHardwareResourceManager *mHardwareResourceManager;

    const stream_attribute_t *mStreamAttributeSource; // from stream out
    stream_attribute_t        mStreamAttributeTarget; // to audio hw
    bool IsVoIPEnable(void) { return (mStreamAttributeSource->mVoIPEnable || mStreamAttributeTarget.mVoIPEnable); }

    struct pcm_config mConfig; // TODO(Harvey): move it to AudioALSAHardwareResourceManager later
    struct pcm *mPcm; // TODO(Harvey): move it to AudioALSAHardwareResourceManager & AudioALSAPlaybackDataDispatcher later
    String8 mApTurnOnSequence;
    String8 mApTurnOnSequence2;
    String8 mApTurnOnSequenceDsp;
    String8 mApTurnOnSequence3;
    String8 mTurnOnSeqCustDev1;
    String8 mTurnOnSeqCustDev2;

    struct compr_config mComprConfig;
    struct compress *mComprStream;
    stream_callback_t mStreamCbk;
    void *mCbkCookie;

    /**
     * Post processing
     */
    AudioMTKFilterManager *mAudioFilterManagerHandler;
    char                  *mPostProcessingOutputBuffer;
    uint32_t               mPostProcessingOutputBufferSize;
    bool mFirstDataWrite;

    /**
    * DcRemoval
    */
    MtkAudioDcRemoveBase   *mDcRemove;
    char        *mDcRemoveWorkBuffer;
    uint32_t    mDcRemoveBufferSize;
    /**
     * Bli SRC
     */
    MtkAudioSrcBase *mBliSrc;
    char        *mBliSrcOutputBuffer;


    /**
     * Bit Converter
     */
    MtkAudioBitConverterBase *mBitConverter;
    char                 *mBitConverterOutputBuffer;

    /**
     * data Pending
     */
    char                  *mdataPendingOutputBuffer;
    char                  *mdataPendingTempBuffer;
    uint32_t             mdataPendingOutputBufferSize;
    uint32_t             mdataPendingRemindBufferSize;
    uint32_t             mDataAlignedSize;
    bool                 mDataPendingForceUse;
    /**
     * NLE Control
     */
    AudioALSAHyBridNLEManager *mNLEMnger;
    /**
     * for debug PCM dump
     */
    void  OpenPCMDump(const char *class_name);
    void  ClosePCMDump(void);
    void  WritePcmDumpData(const void *buffer, ssize_t bytes);
    FILE *mPCMDumpFile;
    static uint32_t mDumpFileNum;

#ifdef MTK_AUDIODSP_SUPPORT
    FILE *mPCMDumpFileDSP;
    static uint32_t mDumpFileNumDSP;
#endif
    struct mixer *mMixer;
    uint64_t mBytesWriteKernel;
    bool mTimeStampValid;

    int mHalQueuedFrame;


    /*
     * =============================================================================
     *                     Aurisys Framework 2.0
     * =============================================================================
     */
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    void CreateAurisysLibManager();
    uint32_t GetAurisysScenario();
    void InitArsiTaskConfig(struct aurisys_lib_manager_config_t *pManagerConfig);
    void InitBufferConfig(struct aurisys_lib_manager_t *manager);
    void DestroyAurisysLibManager();
    uint32_t GetTransferredBufferSize(uint32_t sourceBytes,
                                      const stream_attribute_t *source,
                                      const stream_attribute_t *target);

    struct aurisys_lib_manager_t *mAurisysLibManager;
    struct aurisys_lib_manager_config_t *mManagerConfig;
    uint32_t mAurisysScenario;

    audio_pool_buf_t *mAudioPoolBufUlIn;
    audio_pool_buf_t *mAudioPoolBufUlOut;
    audio_pool_buf_t *mAudioPoolBufDlIn;
    audio_pool_buf_t *mAudioPoolBufDlOut;

    uint32_t mTransferredBufferSize;

    struct data_buf_t *mLinearOut;
#endif

    bool mIsNeedUpdateLib;
    uint32_t mUpdateLibCount;

    struct WriteSmoother *mSmoother;



private:
    AudioALSADataProcessor *mDataProcessor;

    uint32_t mIdentity; // key for mPlaybackHandlerVector
    unsigned int mPcmflag;
    audio_pcm_write_wrapper_fp_t audio_pcm_write_wrapper_fp;

};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_PLAYBACK_HANDLER_BASE_H
