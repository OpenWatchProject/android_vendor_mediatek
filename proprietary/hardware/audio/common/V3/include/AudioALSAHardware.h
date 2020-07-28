#ifndef ANDROID_AUDIO_MTK_HARDWARE_H
#define ANDROID_AUDIO_MTK_HARDWARE_H

#include <stdint.h>
#include <sys/types.h>

#include <utils/Mutex.h>
#include <utils/String8.h>

#include "AudioType.h"
#include "AudioSpeechEnhanceInfo.h"
#include <hardware_legacy/AudioMTKHardwareInterface.h>
#include <AudioCustomVolume.h>
#include "AudioVolumeInterface.h"
#include "AudioCustParamClient.h"

#include "AudioTypeExt.h"

#include "AudioParamParser.h"

#include <hardware/audio_mtk.h>
#include <android/hardware/bluetooth/a2dp/1.0/IBluetoothAudioOffload.h>

namespace android {

class AudioALSAStreamManager;
class AudioALSAVolumeController;
class AudioALSAParamTuner;
class AudioALSASpeechPhoneCallController;
class AudioALSADeviceParser;
class AudioALSAANCController;
class AudioParameterChangedHidlCallback;
class AudioMessengerIPI;

class AudioALSAHardware : public AudioMTKHardwareInterface {
public:
    AudioALSAHardware();
    virtual             ~AudioALSAHardware();

    /**
     * check to see if the audio hardware interface has been initialized.
     * return status based on values defined in include/utils/Errors.h
     */
    virtual status_t    initCheck();

    /** set the audio volume of a voice call. Range is between 0.0 and 1.0 */
    virtual status_t    setVoiceVolume(float volume);

    /**
     * set the audio volume for all audio activities other than voice call.
     * Range between 0.0 and 1.0. If any value other than NO_ERROR is returned,
     * the software mixer will emulate this capability.
     */
    virtual status_t    setMasterVolume(float volume);

    /**
    * Get the current master volume value for the HAL, if the HAL supports
    * master volume control.  AudioFlinger will query this value from the
    * primary audio HAL when the service starts and use the value for setting
    * the initial master volume across all HALs.
    */
    virtual status_t    getMasterVolume(float *volume);

    /**
     * setMode is called when the audio mode changes. NORMAL mode is for
     * standard audio playback, RINGTONE when a ringtone is playing, and IN_CALL
     * when a call is in progress.
     */
    virtual status_t    setMode(int mode);

    // mic mute
    virtual status_t    setMicMute(bool state);
    virtual status_t    getMicMute(bool *state);

#ifdef  MTK_HIFIAUDIO_SUPPORT
    // hifi audio
    virtual status_t setHiFiStateToXML(bool value);
    virtual int getHiFiStateFromXML();
#endif

    // set/get global audio parameters
    virtual status_t    setParameters(const String8 &keyValuePairs);
    virtual String8     getParameters(const String8 &keys);

    // Returns audio input buffer size according to parameters passed or 0 if one of the
    // parameters is not supported
    virtual size_t    getInputBufferSize(uint32_t sampleRate, int format, int channelCount);

    /** This method creates and opens the audio hardware output stream */
    virtual AudioMTKStreamOutInterface *openOutputStream(
        uint32_t devices,
        int *format = 0,
        uint32_t *channels = 0,
        uint32_t *sampleRate = 0,
        status_t *status = 0);
    virtual AudioMTKStreamOutInterface *openOutputStreamWithFlags(uint32_t devices,
                                                                  audio_output_flags_t flags = (audio_output_flags_t)0,
                                                                  int *format = 0,
                                                                  uint32_t *channels = 0,
                                                                  uint32_t *sampleRate = 0,
                                                                  status_t *status = 0);
    virtual    void        closeOutputStream(AudioMTKStreamOutInterface *out);

    /** This method creates and opens the audio hardware input stream */
    virtual AudioMTKStreamInInterface *openInputStream(
        uint32_t devices,
        int *format,
        uint32_t *channels,
        uint32_t *sampleRate,
        status_t *status,
        audio_in_acoustics_t acoustics);

    virtual AudioMTKStreamInInterface *openInputStreamWithFlags(
        uint32_t devices,
        int *format,
        uint32_t *channels,
        uint32_t *sampleRate,
        status_t *status,
        audio_in_acoustics_t acoustics,
        audio_input_flags_t flags = (audio_input_flags_t)0);

    virtual    void        closeInputStream(AudioMTKStreamInInterface *in);

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
    virtual int getMicrophones(struct audio_microphone_characteristic_t *mic_array, size_t *mic_count);

    /**This method dumps the state of the audio hardware */
    virtual status_t dumpState(int fd, const Vector<String16> &args);
    //#ifdef MTK_AUDIO
    // TDM loopback I0I1 record
    virtual status_t SetTDMrecordEnable(uint32_t sampleRate);
    virtual status_t SetTDMrecordDisable(void);
    virtual status_t setTDMRecord(int samplerate);


    // add EM parameter or general purpose commands

    virtual status_t SetEMParameter(void *ptr, int len) ;
    virtual status_t GetEMParameter(void *ptr, int len) ;
    virtual status_t SetAudioCommand(int par1, int par2);
    virtual status_t GetAudioCommand(int par1);
    virtual status_t SetAudioData(int par1, size_t len, void *ptr);
    virtual status_t GetAudioData(int par1, size_t len, void *ptr);
    virtual String8 GetAudioEncodedBuffer(int TypeAudioData, size_t ByteAudioData);

    // ACF Preview parameter
    virtual status_t SetACFPreviewParameter(void *ptr, int len);
    virtual status_t SetHCFPreviewParameter(void *ptr, int len);

#ifdef MTK_SPEAKER_MONITOR_SPEECH_SUPPORT
    status_t CheckDisableSpeechStream(audio_mode_t new_mode);
    status_t CheckEnableSpeechStream(audio_mode_t mode, status_t status_change);
#endif

    // for PCMxWay Interface API
    virtual int xWayPlay_Start(int sample_rate);
    virtual int xWayPlay_Stop(void);
    virtual int xWayPlay_Write(void *buffer, int size_bytes);
    virtual int xWayPlay_GetFreeBufferCount(void);
    virtual int xWayRec_Start(int sample_rate);
    virtual int xWayRec_Stop(void);
    virtual int xWayRec_Read(void *buffer, int size_bytes);

    status_t GetAudioCommonData(int par1, size_t len, void *ptr);
    status_t SetAudioCommonData(int par1, size_t len, void *ptr);
    void setScreenState(bool mode);
    void setBluetoothAudioOffloadParam(const sp<::android::hardware::bluetooth::a2dp::V1_0::IBluetoothAudioHost>& hostIf,
                                       const ::android::hardware::bluetooth::a2dp::V1_0::CodecConfiguration& codecConfig,
                                       bool on);
    void setA2dpSuspendStatus(int status);

    //#endif
    // update speech fir
    bool UpdateOutputFIR(int mode, int index);

    virtual status_t setMasterMute(bool muted);

    virtual int createAudioPatch(unsigned int num_sources,
                                 const struct audio_port_config *sources,
                                 unsigned int num_sinks,
                                 const struct audio_port_config *sinks,
                                 audio_patch_handle_t *handle);

    virtual int releaseAudioPatch(audio_patch_handle_t handle);

    virtual int getAudioPort(struct audio_port *port);

    virtual int setAudioPortConfig(const struct audio_port_config *config);
    class AudioHalPatch {
    public:
        AudioHalPatch(audio_patch_handle_t newHalHandle) :
            num_sources(0),
            num_sinks(0),
            mHalHandle(newHalHandle) {
            memset((void *)sources, 0x00, sizeof(struct audio_port_config)*AUDIO_PATCH_PORTS_MAX);
            memset((void *)sinks, 0x00, sizeof(struct audio_port_config)*AUDIO_PATCH_PORTS_MAX);
        }

        unsigned int      num_sources;      /* number of sources in following array */
        struct audio_port_config sources[AUDIO_PATCH_PORTS_MAX];
        unsigned int      num_sinks;        /* number of sinks in following array */
        struct audio_port_config sinks[AUDIO_PATCH_PORTS_MAX];
        audio_patch_handle_t mHalHandle;
    };

    static AudioALSAHardware *GetInstance();
    virtual int setupParametersCallback(device_parameters_callback_t callback, void *cookie);
    static int setParametersByCallback(const String8 &keyValuePairs);
    virtual int setAudioParameterChangedCallback(device_audio_parameter_changed_callback_t callback, void *cookie);
    virtual int clearAudioParameterChangedCallback(void *cookie);
    bool getBtscoOn() { return mBtscoOn; }
protected:
    virtual status_t dump(int fd, const Vector<String16> &args);

    AudioMessengerIPI *mAudioMessengerIPI;
    AudioALSAStreamManager *mStreamManager;
    AudioSpeechEnhanceInfo *mAudioSpeechEnhanceInfoInstance;
    AudioVolumeInterface *mAudioALSAVolumeController;
    AudioALSAParamTuner *mAudioALSAParamTunerInstance;
    AudioALSASpeechPhoneCallController *mSpeechPhoneCallController;
    AudioALSADeviceParser *mAudioAlsaDeviceInstance;
    AudioALSAANCController *mANCController;

private:
    static AudioALSAHardware *mAudioALSAHardware; // singleton
    bool             mFmTxEnable;
    bool             mUseTuningVolume;
    AUDIO_CUSTOM_VOLUME_STRUCT VolCache;
    volatile int32_t mNextUniqueId;
    bool             mUseAudioPatchForFm;
    SortedVector <AudioHalPatch *> mAudioHalPatchVector;
    float MappingFMVolofOutputDev(int Gain, audio_devices_t eOutput);
    //KeyedVector<audio_patch_handle_t, AudioHalPatch *> mAudioHalPatchVector;
    int  valAudioCmd;
    char *pAudioBuffer;

    // for bybpass audio hw
    bool             mAudioHWBypass;
    static device_parameters_callback_t mHwParametersCbk;
    static void *mHwParametersCbkCookie;

    /**
     * AudioCustParamClient
     */
    AudioCustParamClient *mAudioCustParamClient;

    /**
     * For AudioParameterChanged hidl callback
     */
    AudioLock                                   mAudioParameterChangedHidlCallbackListLock;
    Vector<AudioParameterChangedHidlCallback *>  mAudioParameterChangedHidlCallbackList;
    void onAudioParameterChangedCallback(const char *audioType);
#ifdef MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT
    static void onXmlChangedCallback(AppHandle *appHandle, const char *audioType);
#endif

    ::android::hardware::bluetooth::a2dp::V1_0::CodecConfiguration mBluetoothAudioOffloadCodecConfig;
    /*
     * flag of dynamic enable verbose/debug log
     */
    int mLogEnable;
    bool mAudioHalBtscoWB;
    bool mBtscoOn;
};

}

#endif
