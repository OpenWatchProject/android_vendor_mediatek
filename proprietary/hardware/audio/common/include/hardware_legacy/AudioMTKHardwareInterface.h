#ifndef ANDROID_AUDIO_MTK_HARDWARE_INTERFACE_H
#define ANDROID_AUDIO_MTK_HARDWARE_INTERFACE_H

//#include <SpeechControlInterface.h>

//!  AudioMTKHardwareInterface interface
/*!
  this class is hold extension of android default hardwareinterface
*/
#include <utils/Errors.h>
#include <utils/Vector.h>
#include <utils/String16.h>
#include <utils/String8.h>
#include <utils/RefBase.h>

#include <hardware/audio.h>

#include <hardware/audio_mtk.h>
#include <android/hardware/bluetooth/a2dp/1.0/IBluetoothAudioOffload.h>

using android::status_t;
using namespace android;

class AudioMTKStreamOutInterface {
public:
    virtual             ~AudioMTKStreamOutInterface() = 0;

    /** return audio sampling rate in hz - eg. 44100 */
    virtual uint32_t    sampleRate() const = 0;

    /** returns size of output buffer - eg. 4800 */
    virtual size_t      bufferSize() const = 0;

    /**
     * returns the output channel mask
     */
    virtual uint32_t    channels() const = 0;

    /**
     * return audio format in 8bit or 16bit PCM format -
     * eg. AUDIO_FORMAT_PCM_16_BIT
     */
    virtual int         format() const = 0;

    /**
     * return the frame size (number of bytes per sample).
     */
    uint32_t    frameSize() const {
        return audio_channel_count_from_out_mask(channels()) *
               ((format() == AUDIO_FORMAT_PCM_16_BIT) ? sizeof(int16_t) : sizeof(int8_t));
    }

    /**
     * return the audio hardware driver latency in milli seconds.
     */
    virtual uint32_t    latency() = 0;

    /**
     * Use this method in situations where audio mixing is done in the
     * hardware. This method serves as a direct interface with hardware,
     * allowing you to directly set the volume as apposed to via the framework.
     * This method might produce multiple PCM outputs or hardware accelerated
     * codecs, such as MP3 or AAC.
     */
    virtual status_t    setVolume(float left, float right) = 0;

    /** write audio buffer to driver. Returns number of bytes written */
    virtual ssize_t     write(const void *buffer, size_t bytes) = 0;

    /**
     * Put the audio hardware output into standby mode. Returns
     * status based on include/utils/Errors.h
     */
    // "standby" is used by framework to close stream out.
    virtual status_t    standby(bool halRequest = true) = 0;
    // "standbyStreamOut" is used by HAL to close stream out.
    virtual status_t    standbyStreamOut(bool halRequest = true) = 0;

    /** dump the state of the audio output device */
    virtual status_t dump(int fd, const android::Vector<android::String16> &args) = 0;

    // set/get audio output parameters. The function accepts a list of parameters
    // key value pairs in the form: key1=value1;key2=value2;...
    // Some keys are reserved for standard parameters (See AudioParameter class).
    // If the implementation does not accept a parameter change while the output is
    // active but the parameter is acceptable otherwise, it must return INVALID_OPERATION.
    // The audio flinger will put the output in standby and then change the parameter value.
    virtual status_t    setParameters(const android::String8 &keyValuePairs) = 0;
    virtual android::String8     getParameters(const android::String8 &keys) = 0;

    // return the number of audio frames written by the audio dsp to DAC since
    // the output has exited standby
    virtual status_t    getRenderPosition(uint32_t *dspFrames) = 0;

    /**
     * get the local time at which the next write to the audio driver will be
     * presented
     */
    virtual status_t    getNextWriteTimestamp(int64_t *timestamp) = 0;

    /**
     * Return a recent count of the number of audio frames presented to an external observer.
     */
    virtual status_t    getPresentationPosition(uint64_t *frames, struct timespec *timestamp) = 0;

    /**
     * Called when the metadata of the stream's source has been changed.
     * @param source_metadata Description of the audio that is played by the clients.
     */
    virtual void updateSourceMetadata(const struct source_metadata* source_metadata) = 0;

    virtual status_t    setCallBack(stream_callback_t callback, void *cookie) = 0;
    virtual int    pause() = 0;
    virtual int    resume() = 0;
    virtual int    drain(audio_drain_type_t type) = 0;

    virtual int    flush() = 0;

    /**
     * AAudio MMAP
     */
    virtual status_t    start() = 0;
    virtual status_t    stop() = 0;
    virtual status_t    createMmapBuffer(int32_t min_size_frames,
                                  struct audio_mmap_buffer_info *info) = 0;
    virtual status_t    getMmapPosition(struct audio_mmap_position *position) = 0;
};

class AudioMTKStreamInInterface {
public:
    virtual             ~AudioMTKStreamInInterface() = 0;

    /** return audio sampling rate in hz - eg. 44100 */
    virtual uint32_t    sampleRate() const = 0;

    /** return the input buffer size allowed by audio driver */
    virtual size_t      bufferSize() const = 0;

    /** return input channel mask */
    virtual uint32_t    channels() const = 0;

    /**
     * return audio format in 8bit or 16bit PCM format -
     * eg. AUDIO_FORMAT_PCM_16_BIT
     */
    virtual int         format() const = 0;

    /**
     * return the frame size (number of bytes per sample).
     */
    uint32_t    frameSize() const {
        return audio_channel_count_from_in_mask(channels()) *
               ((format() == AUDIO_FORMAT_PCM_16_BIT) ? sizeof(int16_t) : sizeof(int8_t));
    }

    /** set the input gain for the audio driver. This method is for
     *  for future use */
    virtual status_t    setGain(float gain) = 0;

    /** read audio buffer in from audio driver */
    virtual ssize_t     read(void *buffer, ssize_t bytes) = 0;

    /** dump the state of the audio input device */
    virtual status_t dump(int fd, const android::Vector<android::String16> &args) = 0;

    /**
     * Put the audio hardware input into standby mode. Returns
     * status based on include/utils/Errors.h
     */
    virtual status_t    standby(bool halRequest = true) = 0;

    // set/get audio input parameters. The function accepts a list of parameters
    // key value pairs in the form: key1=value1;key2=value2;...
    // Some keys are reserved for standard parameters (See AudioParameter class).
    // If the implementation does not accept a parameter change while the output is
    // active but the parameter is acceptable otherwise, it must return INVALID_OPERATION.
    // The audio flinger will put the input in standby and then change the parameter value.
    virtual status_t    setParameters(const android::String8 &keyValuePairs) = 0;
    virtual android::String8     getParameters(const android::String8 &keys) = 0;


    // Return the number of input frames lost in the audio driver since the last call of this function.
    // Audio driver is expected to reset the value to 0 and restart counting upon returning the current value by this function call.
    // Such loss typically occurs when the user space process is blocked longer than the capacity of audio driver buffers.
    // Unit: the number of input audio frames
    virtual unsigned int  getInputFramesLost() const = 0;

    virtual status_t addAudioEffect(effect_handle_t effect) = 0;
    virtual status_t removeAudioEffect(effect_handle_t effect) = 0;

    virtual int         getCapturePosition(int64_t *frames, int64_t *time) = 0;

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
    virtual int getActiveMicrophones(struct audio_microphone_characteristic_t *mic_array, size_t *mic_count) = 0;

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
    virtual int setMicrophoneDirection(audio_microphone_direction_t direction) = 0;

    /**
     * Called by the framework to specify to the HAL the desired zoom factor for the selected
     * microphone(s).
     *
     * \param[in] zoom      the zoom factor.
     * \return OK if the call is successful, an error code otherwise.
     */
    virtual int setMicrophoneFieldDimension(float zoom) = 0;

    /**
     * Called when the metadata of the stream's sink has been changed.
     * @param sink_metadata Description of the audio that is recorded by the clients.
     */
    virtual void updateSinkMetadata(const struct sink_metadata *sink_metadata) = 0;


    /**
     * AAudio MMAP
     */
    virtual status_t    start() = 0;
    virtual status_t    stop() = 0;
    virtual status_t    createMmapBuffer(int32_t min_size_frames,
                                  struct audio_mmap_buffer_info *info) = 0;
    virtual status_t    getMmapPosition(struct audio_mmap_position *position) = 0;

};

class AudioMTKHardwareInterface {
public:
    virtual             ~AudioMTKHardwareInterface() { }

    /**
     * check to see if the audio hardware interface has been initialized.
     * return status based on values defined in include/utils/Errors.h
     */
    virtual status_t    initCheck() = 0;

    /** set the audio volume of a voice call. Range is between 0.0 and 1.0 */
    virtual status_t    setVoiceVolume(float volume) = 0;

    /**
     * set the audio volume for all audio activities other than voice call.
     * Range between 0.0 and 1.0. If any value other than NO_ERROR is returned,
     * the software mixer will emulate this capability.
     */
    virtual status_t    setMasterVolume(float volume) = 0;

    /**
     * Get the current master volume value for the HAL, if the HAL supports
     * master volume control.  AudioFlinger will query this value from the
     * primary audio HAL when the service starts and use the value for setting
     * the initial master volume across all HALs.
     */
    virtual status_t    getMasterVolume(float *volume) = 0;

    /**
     * setMode is called when the audio mode changes. NORMAL mode is for
     * standard audio playback, RINGTONE when a ringtone is playing, and IN_CALL
     * when a call is in progress.
     */
    virtual status_t    setMode(int mode) = 0;

    // mic mute
    virtual status_t    setMicMute(bool state) = 0;
    virtual status_t    getMicMute(bool *state) = 0;

    // set/get global audio parameters
    virtual status_t    setParameters(const android::String8 &keyValuePairs) = 0;
    virtual android::String8     getParameters(const android::String8 &keys) = 0;

    // Returns audio input buffer size according to parameters passed or 0 if one of the
    // parameters is not supported
    virtual size_t    getInputBufferSize(uint32_t sampleRate, int format, int channelCount) = 0;

    /** This method creates and opens the audio hardware output stream */
    virtual AudioMTKStreamOutInterface *openOutputStream(uint32_t devices,
                                                         int *format = 0,
                                                         uint32_t *channels = 0,
                                                         uint32_t *sampleRate = 0,
                                                         status_t *status = 0) = 0;
    virtual AudioMTKStreamOutInterface *openOutputStreamWithFlags(uint32_t devices,
                                                                  audio_output_flags_t flags = (audio_output_flags_t)0,
                                                                  int *format = 0,
                                                                  uint32_t *channels = 0,
                                                                  uint32_t *sampleRate = 0,
                                                                  status_t *status = 0) = 0;
    virtual void closeOutputStream(AudioMTKStreamOutInterface *out) = 0;

    /** This method creates and opens the audio hardware input stream */
    virtual AudioMTKStreamInInterface *openInputStream(
        uint32_t devices,
        int *format,
        uint32_t *channels,
        uint32_t *sampleRate,
        status_t *status,
        audio_in_acoustics_t acoustics) = 0;

    // for open input stream with flag
    virtual AudioMTKStreamInInterface *openInputStreamWithFlags(
        uint32_t devices,
        int *format,
        uint32_t *channels,
        uint32_t *sampleRate,
        status_t *status,
        audio_in_acoustics_t acoustics,
        audio_input_flags_t flags = (audio_input_flags_t)0) {
        ALOGV("%s(), flags %d", __FUNCTION__, flags);
        return openInputStream(devices, format, channels, sampleRate, status, acoustics);
    }

    virtual    void        closeInputStream(AudioMTKStreamInInterface *in) = 0;

    /**
     * Called by the framework to read available microphones characteristics.
     *
     * \param[in] dev the hw_device object.
     * \param[out] mic_array Pointer to first element on array with microphone info
     * \param[out] mic_count When called, this holds the value of the max number of elements
     *                       allowed in the mic_array. The actual number of elements written
     *                       is returned here.
     *                       if mic_count is passed as zero, mic_array will not be populated,
     *                       and mic_count will return the actual number of microphones in the
     *                       system.
     *
     * \return 0 if the microphone array is successfully filled.
     *         -ENOSYS if there is an error filling the data
     */
    virtual int getMicrophones(struct audio_microphone_characteristic_t *mic_array, size_t *mic_count) = 0;

    /**This method dumps the state of the audio hardware */
    virtual status_t dumpState(int fd, const android::Vector<android::String16> &args) = 0;

    virtual status_t setMasterMute(bool muted) = 0;

    virtual int createAudioPatch(unsigned int num_sources,
                                 const struct audio_port_config *sources,
                                 unsigned int num_sinks,
                                 const struct audio_port_config *sinks,
                                 audio_patch_handle_t *handle) = 0;

    virtual int releaseAudioPatch(audio_patch_handle_t handle) = 0;

    virtual int getAudioPort(struct audio_port *port) = 0;

    virtual int setAudioPortConfig(const struct audio_port_config *config) = 0;

    /**
    *  SetEMParamete, set em parameters to audioahrdware
    * @param ptr
    * @param len
    * @return status
    */
    virtual status_t SetEMParameter(void *ptr, int len) = 0;

    /**
    *  GetEMParameter, get em parameters to audioahrdware
    * @param ptr
    * @param len
    * @return status
    */
    virtual status_t GetEMParameter(void *ptr, int len) = 0;

    /**
    *  SetAudioCommand, base on par1 and par2
    * @param par1
    * @param par2
    * @return status
    */
    virtual status_t SetAudioCommand(int par1, int par2) = 0;

    /**
    *  GetAudioCommand, base on par1
    * @param par1
    * @return status
    */
    virtual status_t GetAudioCommand(int par1) = 0;

    /**
    *  SetAudioData
    * @param par1
    * @param len
    * @param ptr
    * @return status
    */
    virtual status_t SetAudioData(int par1, size_t len, void *ptr) = 0;

    /**
    *  GetAudioData
    * @param par1
    * @param len
    * @param ptr
    * @return status
    */
    virtual status_t GetAudioData(int par1, size_t len, void *ptr) = 0;

    /**
    *  set ACF Preview parameter , thoiis function only temporary replace coefficient
    * @param ptr
    * @param len
    * @return status
    */
    virtual status_t SetACFPreviewParameter(void *ptr, int len) = 0;

    /**
    *  set HCF Preview parameter , thoiis function only temporary replace coefficient
    * @param ptr
    * @param len
    * @return status
    */
    virtual status_t SetHCFPreviewParameter(void *ptr, int len) = 0;

    /////////////////////////////////////////////////////////////////////////
    //    for PCMxWay Interface API
    /////////////////////////////////////////////////////////////////////////
    virtual int xWayPlay_Start(int sample_rate) = 0;
    virtual int xWayPlay_Stop(void) = 0;
    virtual int xWayPlay_Write(void *buffer, int size_bytes) = 0;
    virtual int xWayPlay_GetFreeBufferCount(void) = 0;
    virtual int xWayRec_Start(int sample_rate) = 0;
    virtual int xWayRec_Stop(void) = 0;
    virtual int xWayRec_Read(void *buffer, int size_bytes) = 0;

    static AudioMTKHardwareInterface *create();

    virtual int setupParametersCallback(device_parameters_callback_t callback, void *cookie) = 0;

    virtual int setAudioParameterChangedCallback(device_audio_parameter_changed_callback_t callback, void *cookie) = 0;

    virtual int clearAudioParameterChangedCallback(void *cookie) = 0;
    virtual void setBluetoothAudioOffloadParam(const sp<::android::hardware::bluetooth::a2dp::V1_0::IBluetoothAudioHost>& hostIf,
                                               const ::android::hardware::bluetooth::a2dp::V1_0::CodecConfiguration& codecConfig,
                                               bool on) = 0;
    virtual void setA2dpSuspendStatus(int status) = 0;

protected:
    virtual status_t dump(int fd, const android::Vector<android::String16> &args) = 0;

};

extern "C" AudioMTKHardwareInterface *createMTKAudioHardware(void);

typedef AudioMTKHardwareInterface *create_AudioMTKHw(void);

#endif
