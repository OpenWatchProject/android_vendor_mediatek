#ifndef ANDROID_AUDIO_ALSA_STREAM_IN_H
#define ANDROID_AUDIO_ALSA_STREAM_IN_H

#include <utils/Mutex.h>

#include <hardware_legacy/AudioMTKHardwareInterface.h>
#include <media/AudioParameter.h>

#include "AudioType.h"
#include <AudioLock.h>
#include "AudioSpeechEnhanceInfo.h"
#include "AudioALSACaptureHandlerBase.h"

namespace android {

class AudioALSAStreamManager;
class AudioALSACaptureHandlerBase;

class AudioALSAStreamIn : public AudioMTKStreamInInterface {
public:
    AudioALSAStreamIn();
    virtual ~AudioALSAStreamIn();

    virtual status_t    set(uint32_t devices, int *format, uint32_t *channels, uint32_t *sampleRate,
                            status_t *status, audio_in_acoustics_t acoustics, uint32_t flags = 0);

    /** return audio sampling rate in hz - eg. 44100 */
    virtual uint32_t    sampleRate() const;

    /** return the input buffer size allowed by audio driver */
    virtual size_t      bufferSize() const;

    /** return input channel mask */
    virtual uint32_t    channels() const;

    /**
     * return audio format in 8bit or 16bit PCM format -
     * eg. AUDIO_FORMAT_PCM_16_BIT
     */
    virtual int         format() const;

    /** set the input gain for the audio driver. This method is for
     *  for future use */
    virtual status_t    setGain(float gain);

    /** read audio buffer in from audio driver */
    virtual ssize_t     read(void *buffer, ssize_t bytes);

    /** dump the state of the audio input device */
    virtual status_t    dump(int fd, const Vector<String16> &args);

    /**
     * Put the audio hardware input into standby mode. Returns
     * status based on include/utils/Errors.h
     */
    virtual status_t    standby(bool halRequest = true);
    virtual bool        getStandby();

    // set/get audio input parameters. The function accepts a list of parameters
    // key value pairs in the form: key1=value1;key2=value2;...
    // Some keys are reserved for standard parameters (See AudioParameter class).
    // If the implementation does not accept a parameter change while the output is
    // active but the parameter is acceptable otherwise, it must return INVALID_OPERATION.
    // The audio flinger will put the input in standby and then change the parameter value.
    virtual status_t    setParameters(const String8 &keyValuePairs);
    virtual String8     getParameters(const String8 &keys);


    // Return the amount of input frames lost in the audio driver since the last call of this function.
    // Audio driver is expected to reset the value to 0 and restart counting upon returning the current value by this function call.
    // Such loss typically occurs when the user space process is blocked longer than the capacity of audio driver buffers.
    // Unit: the number of input audio frames
    virtual unsigned int getInputFramesLost() const;
    unsigned int getInputFlags() const;

    /**
      * Return a recent count of the number of audio frames received and
      * the clock time associated with that frame count.
      *
      * frames is the total frame count received. This should be as early in
      *     the capture pipeline as possible. In general,
      *     frames should be non-negative and should not go "backwards".
      *
      * time is the clock MONOTONIC time when frames was measured. In general,
      *     time should be a positive quantity and should not go "backwards".
      *
      * The status returned is 0 on success, -ENOSYS if the device is not
      * ready/available, or -EINVAL if the arguments are null or otherwise invalid.
      */
    virtual int getCapturePosition(int64_t *frames, int64_t *time);

    /**
     * Called by the framework to read active microphones
     *
     * \param[out] mic_array Pointer to first element on array with microphone info
     * \param[out] mic_count When called, this holds the value of the max number of elements
     *                       allowed in the mic_array. The actual number of elements written
     *                       is returned here.
     *                       if mic_count is passed as zero, mic_array will not be populated,
     *                       and mic_count will return the actual number of active microphones.
     *
     * \return 0 if the microphone array is successfully filled.
     *         -ENOSYS if there is an error filling the data
     */
    virtual int getActiveMicrophones(struct audio_microphone_characteristic_t *mic_array, size_t *mic_count);

    /**
     * Called by the framework to instruct the HAL to optimize the capture stream in the
     * specified direction.
     *
     * \param[in] direction The direction constant (from audio-base.h)
     *   MIC_DIRECTION_UNSPECIFIED  Don't do any directionality processing of the
     *      activated microphone(s).
     *   MIC_DIRECTION_FRONT        Optimize capture for audio coming from the screen-side
     *      of the device.
     *   MIC_DIRECTION_BACK         Optimize capture for audio coming from the side of the
     *      device opposite the screen.
     *   MIC_DIRECTION_EXTERNAL     Optimize capture for audio coming from an off-device
     *      microphone.
     * \return OK if the call is successful, an error code otherwise.
     */
    virtual int setMicrophoneDirection(audio_microphone_direction_t direction);

    /**
     * Called by the framework to specify to the HAL the desired zoom factor for the selected
     * microphone(s).
     *
     * \param[in] zoom      the zoom factor.
     * \return OK if the call is successful, an error code otherwise.
     */
    virtual int setMicrophoneFieldDimension(float zoom);

    /**
     * Called when the metadata of the stream's sink has been changed.
     * @param sink_metadata Description of the audio that is recorded by the clients.
     */
    virtual void updateSinkMetadata(const struct sink_metadata *sink_metadata);

    virtual status_t    addAudioEffect(effect_handle_t effect);
    virtual status_t    removeAudioEffect(effect_handle_t effect);


    /**
     * set stream in index
     */
    inline void         setIdentity(const uint32_t identity) { mIdentity = identity; }
    inline uint32_t     getIdentity() const { return mIdentity; }


    /**
     * open/close stream in related audio hardware
     */
    virtual status_t    open();
    virtual status_t    close();


    /**
     * routing
     */
    status_t routing(audio_devices_t input_device);
    status_t setPhonecallRouting(audio_devices_t input_device);
    bool getDeviceConnectionState(audio_devices_t device);
    void updateDeviceConnectionState(audio_devices_t device, bool connect);


    /**
     * reopen
     */
    void setReopenState(bool state) { mStreamInReopen = state; }
    bool getReopenState() { return mStreamInReopen; }


    /**
     * suspend/resume
     */
    status_t setSuspend(const bool suspend_on);


    /**
     * get stream attribute
     */
    virtual const stream_attribute_t *getStreamAttribute() const { return &mStreamAttributeTarget; }

    /**
     * low latency
     */
    status_t setLowLatencyMode(bool mode);


    /**
     * update output routing
     */
    status_t updateOutputDeviceInfoForInputStream(audio_devices_t output_devices);

    /**
     * Set Input mute
     */
    void SetInputMute(bool bEnable);

    /**
     * Update Dynamic function mask
     */
    void UpdateDynamicFunctionMask(void);

    /**
     * Query if the stream in can run in Call Mode
     */
    bool isSupportConcurrencyInCall(void);

    AudioALSACaptureHandlerBase *getStreamInCaptureHandler(void);

    capture_handler_t getCaptureHandlerType();


    /**
     * AAudio MMAP
     */
    virtual status_t    start();
    virtual status_t    stop();
    virtual status_t    createMmapBuffer(int32_t min_size_frames,
                                  struct audio_mmap_buffer_info *info);
    virtual status_t    getMmapPosition(struct audio_mmap_position *position);


protected:
    AudioALSAStreamManager         *mStreamManager;
    AudioALSACaptureHandlerBase    *mCaptureHandler;

    /**
     * low latency
     */
    status_t setLowLatencyMode_l() {
        status_t ret = NO_ERROR;
        if (NULL != mCaptureHandler) {
            ret = mCaptureHandler->setLowLatencyMode(mLowLatencyMode, mKernelBuffersize, mReduceBuffersize);
        }
        return ret;
    }

    /**
     * check open input stream parameters
     */
    virtual bool checkOpenStreamFormat(int *format);
    virtual bool checkOpenStreamChannels(uint32_t *channels);
    virtual bool checkOpenStreamSampleRate(const audio_devices_t devices, uint32_t *sampleRate);



private:
    AudioLock           mLock;
    volatile int32_t    mLockCount;
    volatile int32_t    mSuspendLockCount;
    AudioLock           mSuspendLock;
    AudioLock           mStandbyLock;


    uint32_t            mIdentity; // key for mStreamInVector

    uint32_t            mSuspendCount;
    int64_t             mStandbyFrameCount;
    bool                mStandby;
    stream_attribute_t  mStreamAttributeTarget;
    bool                mStreamInReopen;
    bool                mUpdateStandbyFrameCount;
    audio_devices_t     mNewInputDevice;
    effect_handle_t     mPreProcessEffectBackup[MAX_PREPROCESSORS];
    int                 mPreProcessEffectBackupCount;
    bool                mLowLatencyMode;
    uint32_t            mKernelBuffersize;
    size_t              mReduceBuffersize;
    uint64_t            mReadBytesSum;
    uint64_t            mOldReadBytesSum;
    int64_t             mOldTime;
    int64_t             mOldFrames;
    audio_input_flags_t mOldAudioInputFlags;
    uint32_t            mOldBuffersize;

    /**
    * Speech EnhanceInfo Instance
    */
    AudioSpeechEnhanceInfo *mAudioSpeechEnhanceInfoInstance;

    void CheckBesRecordInfo();

    /**
     * for debug PCM dump
     */
    void  OpenPCMDump(void);
    void  ClosePCMDump(void);
    void  WritePcmDumpData(void *buffer, ssize_t bytes);
    String8 mDumpFileName;
    FILE *mPCMDumpFile;
    static int mDumpFileNum;

    /**
     * Update standby frame count for standby/input source changed usage
     */
    void setUpdateStandByFrameCount(bool update) { mUpdateStandbyFrameCount = update; }
    bool getUpdateStandByFrameCount() { return mUpdateStandbyFrameCount; }
    void updateStandByFrameCount();
    int calculateCapturePosition(int64_t *frames, int64_t *time);

    /*
     * Device Available flag
     */
    uint32_t mAvailableInputDevices;

    /*
     * flag of dynamic enable verbose/debug log
     */
    int mLogEnable;

    /**
     * AAudio MMAP
     */
    bool                mStart;
    bool                mDestroy;
};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_STREAM_IN_H
