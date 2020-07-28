#ifndef ANDROID_AUDIO_ALSA_HARDWARE_RESOURCE_MANAGER_H
#define ANDROID_AUDIO_ALSA_HARDWARE_RESOURCE_MANAGER_H
#include "AudioTypeExt.h"

#include <tinyalsa/asoundlib.h>

#include "AudioType.h"
#include <AudioLock.h>
#include "AudioUtility.h"
#include "LoopbackManager.h"

namespace android {

typedef enum {
    AUDIO_SPEAKER_MODE_D = 0,
    AUDIO_SPEAKER_MODE_AB = 1,

} AUDIO_SPEAKER_MODE;

enum builtin_mic_specific_type {
    BUILTIN_MIC_DEFAULT,
    BUILTIN_MIC_MIC1_ONLY,
    BUILTIN_MIC_MIC2_ONLY,
    BUILTIN_MIC_MIC3_ONLY,
    BUILTIN_MIC_MIC4_ONLY,
    BUILTIN_MIC_MIC5_ONLY,
};

enum DeviceStatus {
    DEVICE_STATUS_ON,
    DEVICE_STATUS_OFF,
};

typedef bool (*SpeakerStatusChangeCb)(const DeviceStatus);

class AudioALSADeviceConfigManager;
class AudioSmartPaController;
class AudioALSAHardwareResourceManager {
public:
    virtual ~AudioALSAHardwareResourceManager();
    static AudioALSAHardwareResourceManager *getInstance();
    virtual status_t ResetDevice(void);

    /**
     * output devices
     */
    virtual status_t setOutputDevice(const audio_devices_t new_devices, const uint32_t sample_rate);
    virtual status_t startOutputDevice(const audio_devices_t new_devices, const uint32_t SampleRate);
    virtual status_t stopOutputDevice();
    virtual status_t changeOutputDevice(const audio_devices_t new_devices);
    virtual bool isSharedOutDevice(audio_devices_t devices);
    virtual audio_devices_t getOutputDevice();

    /**
     * output devices - Before using them, we should keep the mlock
     */
    virtual status_t startOutputDevice_l(const audio_devices_t new_devices, const uint32_t SampleRate);
    virtual status_t stopOutputDevice_l();
    virtual status_t changeOutputDevice_l(const audio_devices_t new_devices, const uint32_t SampleRate);

    /**
     * input devices
     */
    virtual status_t setInputDevice(const audio_devices_t new_device);
    virtual status_t startInputDevice(const audio_devices_t new_device);
    virtual status_t stopInputDevice(const audio_devices_t stop_device);
    virtual status_t changeInputDevice(const audio_devices_t new_device);
    virtual audio_devices_t getInputDevice();


    /**
     * HW Gain2
     */
    virtual status_t setHWGain2DigitalGain(const uint32_t gain);


    /**
     * Interrupt Rate
     */
    virtual status_t setInterruptRate(const audio_output_flags_t flag, const uint32_t rate);
    virtual status_t setInterruptRate2(const uint32_t rate);
    virtual status_t setULInterruptRate(const uint32_t rate);


    /**
     * sgen
     */
    virtual status_t setSgenMode(const sgen_mode_t sgen_mode);
    virtual status_t setSgenSampleRate(const sgen_mode_samplerate_t sample_rate);
    virtual status_t setSgenMute(int channel, bool mute);
    virtual status_t setSgenFreqDiv(int channel, int freqDiv);
    virtual status_t openAddaOutput(const uint32_t sample_rate);
    virtual status_t closeAddaOutput();

    /**
    * sidetone
    */
    virtual status_t EnableSideToneFilter(const bool enable);
    /**
    *   Current Sensing
    */
    virtual status_t setSPKCurrentSensor(bool bSwitch);

    virtual status_t setSPKCurrentSensorPeakDetectorReset(bool bSwitch);

    /**
     * Get Phone MIC Mode by HW config
     */
    virtual uint32_t updatePhoneMicMode();
    virtual uint32_t getPhoneMicMode(void) { return mPhoneMicMode; }
    virtual uint32_t getHeadsetMicMode(void) { return mHeadsetMicMode; }

    /**
     * MIC inverse
     */
    virtual status_t setMicInverse(bool bMicInverse);
    virtual bool getMicInverse(void);

    virtual void setMIC1Mode(bool isHeadsetMic);
    virtual void setMIC2Mode(bool isHeadsetMic);
    virtual void setMIC3Mode(bool isHeadsetMic);

    /**
    * acquire pmic clk
    */
    virtual void     EnableAudBufClk(bool bEanble);
    virtual void     EnableAudBufClk_l(bool bEanble);

    /**
     * Headphone Change information
     */
    virtual void     setHeadPhoneChange(bool bchange) { mHeadchange = bchange; }
    virtual bool     getHeadPhoneChange(void) { return mHeadchange ; }

    /**
     * Headphone Change information
     */
    virtual void     setNumOfHeadsetPole(int pole) { mNumHSPole = pole; }
    virtual int      getNumOfHeadsetPole(void) { return mNumHSPole; }

    /**
     * debug dump register & DAC I2S Sgen
     */
    virtual void     setAudioDebug(const bool enable);

    /**
     * build in mic specific type
     */
    void setBuiltInMicSpecificType(const builtin_mic_specific_type type) { mBuiltInMicSpecificType = type; }

    int  getNumPhoneMicSupport(void) { return mNumPhoneMicSupport; }
    void HpImpeDanceDetect(void);
    void setDPDModule(bool enable);
    void setHeadphoneLowPowerMode(bool enable);

    int setNonSmartPAType();
    int getNonSmartPAType();
    bool setSpeakerStatusChangeCb(const SpeakerStatusChangeCb cb);


    status_t EnableLowLatencyDebug(uint32_t bFlag) {
        ALOGD("+%s() flag %d, Audio_LowLatency_Debug", __FUNCTION__, bFlag);

        struct mixer_ctl *ctl = mixer_get_ctl_by_name(mMixer, "Audio_LowLatency_Debug");
        if (ctl == NULL) {
            ALOGD("+%s() don't support Audio_LowLatency_Debug", __FUNCTION__);
            return NO_ERROR;
        }

        int retval = mixer_ctl_set_value(ctl, 0, bFlag);
        ALOGD("+%s() retval %d", __FUNCTION__, retval);

        return NO_ERROR;
    }

    // Latency unit : 0.1ms
    void setA2dpDeviceLatency(int latency) { mA2dpDeviceLatency = latency / 10; }
    int getA2dpDeviceLatency() { return mA2dpDeviceLatency; }
    void resetA2dpDeviceLatency(void);

    // BT FW latency
    void setA2dpFwLatency(int latency) { mA2dpFwLatency = latency; }
    int getA2dpFwLatency() { return mA2dpFwLatency; }
    void resetA2dpFwLatency() {  mA2dpFwLatency = 0; }

    int getA2dpLatency() { return mA2dpFwLatency + mA2dpDeviceLatency; }

#if defined(MTK_HIFIAUDIO_SUPPORT)
    void setHiFiStatus(const bool enable) { mHiFiState = enable; }
    bool getHiFiStatus() { return mHiFiState; }
    //int setHiFiDACStatus(bool enable);
    bool GetHiFiDACStateProperty();
#endif

    void setCodecSampleRate(const uint32_t sample_rate);

    status_t AssignDRAM(uint32_t memoryInterface) {
        struct mixer_ctl *ctl = mixer_get_ctl_by_name(mMixer, "Audio_Assign_DRAM");
        if (ctl == NULL) {
            ALOGD("-%s() don't support Audio_Assign_DRAM", __FUNCTION__);
            return NO_ERROR;
        }

        int retval = mixer_ctl_set_value(ctl, 0, memoryInterface);
        ALOGV("-%s() retval %d", __FUNCTION__, retval);

        return NO_ERROR;
    }

    void setCustOutputDevTurnOnSeq(audio_devices_t outputDevice,
                                   String8 &devTurnOnSeq1,
                                   String8 &devTurnOnSeq2);
    String8 getOutputTurnOnSeq(audio_devices_t outputDevice,
                               bool isSpkwithOtherOutputDevice,
                               const char *prefixStr);
    status_t enableTurnOnSequence(String8 sequence);
    status_t disableTurnOnSequence(String8 &sequence);
    String8 getMicRecordPath(unsigned int channels);
    String8 getSingleMicLoopbackPath(audio_devices_t outputDevice,
                                     bool isMultiOutputDevice,
                                     loopback_t loopbackType);

    int setI2sSequenceHD(bool enable, const char *i2sStr);
protected:
    AudioALSAHardwareResourceManager();



private:
    /**
     * singleton pattern
     */
    static AudioALSAHardwareResourceManager *mAudioALSAHardwareResourceManager;

    status_t setDeviceSeqence();

    /**
     *  low level for device open close
     */
    virtual status_t OpenReceiverPath(const uint32_t SampleRate);
    virtual status_t CloseReceiverPath();
    virtual status_t OpenHeadphonePath(const uint32_t SampleRate);
    virtual status_t CloseHeadphonePath();
    virtual status_t OpenSpeakerPath(const uint32_t SampleRate);
    virtual status_t CloseSpeakerPath();
    virtual status_t OpenHeadphoneSpeakerPath(const uint32_t SampleRate);
    virtual status_t CloseHeadphoneSpeakerPath();
    virtual status_t OpenBuiltInMicPath();
    virtual status_t CloseBuiltInMicPath();
    virtual status_t OpenBackMicPath();
    virtual status_t CloseBackMicPath();
    virtual status_t OpenWiredHeadsetMicPath();
    virtual status_t CloseWiredHeadsetMicPath();

    status_t setMicType(void);
    status_t SetExtDacGpioEnable(bool bEnable);
    bool GetExtDacPropertyEnable(void);

    bool notifyOutputDeviceStatusChange(const audio_devices_t device, const DeviceStatus status);

    String8 getDevCustPathSeqence(audio_devices_t device);
    String8 getDevCustCtrlSeqence(audio_devices_t device);

    AudioLock mLock;
    AudioLock mLockAudClkBuf;

    struct mixer *mMixer;
    struct pcm *mPcmDL;
    AudioALSADeviceConfigManager  *mDeviceConfigManager;

    audio_devices_t mOutputDevices;
    audio_devices_t mInputDevice;

    uint32_t        mOutputDeviceSampleRate;
    uint32_t        mInputDeviceSampleRate;

    bool mIsChangingInputDevice;
    int32_t mAudBufClkCount;
    int32_t mStartOutputDevicesCount;
    int32_t mStartInputDeviceCount;
    String8 mStartInputDeviceSeqeunce;
    String8 mSpkOutPathSeq;
    String8 mSpkOutCustCtrlSeq;
    String8 mHpOutPathSeq;
    String8 mHpOutCustCtrlSeq;
    String8 mRcvOutPathSeq;
    String8 mRcvOutCustCtrlSeq;
    SpeakerStatusChangeCb mSpeakerStatusChangeCb;

    int mNumPhoneMicSupport;
    bool mMicInverse;
    builtin_mic_specific_type mBuiltInMicSpecificType;
#if defined(MTK_HIFIAUDIO_SUPPORT)
    AudioLock mHiFiLock;
    bool mHiFiState;
    int mHiFiDACStatusCount;
#endif

    int mNumHSPole;
    bool mHeadchange;
    uint32_t mPhoneMicMode;
    uint32_t mHeadsetMicMode;
    struct pcm_config mHpImpedanceConfig;
    struct pcm *mHpImpeDancePcm;

    AudioSmartPaController *mSmartPaController;

    int mA2dpDeviceLatency;
    int mA2dpFwLatency;

    /*
     * flag of dynamic enable verbose/debug log
     */
    int mLogEnable;

    int nonSmartPAType;
};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_HARDWARE_RESOURCE_MANAGER_H
