#ifndef ANDROID_AUDIO_ALSA_STREAM_MANAGER_H
#define ANDROID_AUDIO_ALSA_STREAM_MANAGER_H

#include <utils/threads.h>
#include <utils/KeyedVector.h>
#include "AudioTypeExt.h"

#include <hardware_legacy/AudioMTKHardwareInterface.h>


#include "AudioType.h"
#include <AudioLock.h>
#include "AudioMTKFilter.h"
#include "AudioPolicyParameters.h"
#include "AudioSpeechEnhanceInfo.h"
#include "AudioVolumeInterface.h"
#include <AudioCustParamClient.h>
#include <android/hardware/bluetooth/a2dp/1.0/IBluetoothAudioOffload.h>

#ifndef KERNEL_BUFFER_SIZE_DL1_DATA2
#define KERNEL_BUFFER_SIZE_DL1_DATA2         KERNEL_BUFFER_SIZE_DL1
#endif

#ifdef PLAYBACK_USE_24BITS_ONLY
#define KERNEL_BUFFER_SIZE_DL1_NORMAL KERNEL_BUFFER_SIZE_DL1
#define KERNEL_BUFFER_SIZE_DL1_DATA2_NORMAL KERNEL_BUFFER_SIZE_DL1_DATA2
#else
#define KERNEL_BUFFER_SIZE_DL1_NORMAL (KERNEL_BUFFER_SIZE_DL1 / 2)
#define KERNEL_BUFFER_SIZE_DL1_DATA2_NORMAL (KERNEL_BUFFER_SIZE_DL1_DATA2 / 2)
#endif
#define KERNEL_BUFFER_SIZE_DL1_HIFI_96K (KERNEL_BUFFER_SIZE_DL1_NORMAL * 2)
#define KERNEL_BUFFER_SIZE_DL1_HIFI_192K (KERNEL_BUFFER_SIZE_DL1_NORMAL * 4)

#define KERNEL_BUFFER_SIZE_DL1_DATA2_HIFI_96K (KERNEL_BUFFER_SIZE_DL1_DATA2_NORMAL * 2)
#define KERNEL_BUFFER_SIZE_DL1_DATA2_HIFI_192K (KERNEL_BUFFER_SIZE_DL1_DATA2_NORMAL * 4)

#define KERNEL_BUFFER_FRAME_COUNT_REMAIN 1024

namespace android {

class AudioALSAStreamOut;
class AudioALSAStreamIn;

class AudioALSAPlaybackHandlerBase;
class AudioALSACaptureHandlerBase;

class AudioALSASpeechPhoneCallController;
class AudioSmartPaController;

class AudioALSAFMController;

class AudioALSAVolumeController;

class AudioALSAVoiceWakeUpController;

class SpeechDriverFactory;

class AudioALSAStreamManager {
public:
    virtual ~AudioALSAStreamManager();
    static AudioALSAStreamManager *getInstance();


    /**
     * open/close ALSA output stream
     */
    AudioMTKStreamOutInterface *openOutputStream(
        uint32_t devices,
        int *format,
        uint32_t *channels,
        uint32_t *sampleRate,
        status_t *status,
        uint32_t output_flag = 0);

    void closeOutputStream(AudioMTKStreamOutInterface *out);


    /**
     * open/close ALSA input stream
     */
    AudioMTKStreamInInterface *openInputStream(
        uint32_t devices,
        int *format,
        uint32_t *channels,
        uint32_t *sampleRate,
        status_t *status,
        audio_in_acoustics_t acoustics,
        uint32_t input_flag = 0);

    void closeInputStream(AudioMTKStreamInInterface *in);


    /**
     * create/destroy ALSA playback/capture handler
     */
    AudioALSAPlaybackHandlerBase *createPlaybackHandler(stream_attribute_t *stream_attribute_source);
    AudioALSACaptureHandlerBase  *createCaptureHandler(stream_attribute_t *stream_attribute_target);

    status_t destroyPlaybackHandler(AudioALSAPlaybackHandlerBase *pPlaybackHandler);
    status_t destroyCaptureHandler(AudioALSACaptureHandlerBase   *pCaptureHandler);


    /**
     * volume related functions
     */
    status_t setVoiceVolume(float volume);
    float getMasterVolume(void);
    status_t setMasterVolume(float volume, uint32_t iohandle = 0);
    status_t setHeadsetVolumeMax(void);
    status_t setFmVolume(float volume);

    status_t setMicMute(bool state);
    bool     getMicMute();
    uint32_t GetOffloadGain(float vol_f);

#ifdef MTK_AUDIO_GAIN_TABLE
    status_t setAnalogVolume(int stream, int device, int index, bool force_incall);
    int SetCaptureGain(void);
#endif

    /**
     * mode / routing related functions
     */
    status_t setMode(audio_mode_t new_mode);
    void updateAudioModePolicy(const audio_mode_t new_mode);

    status_t routingOutputDevice(AudioALSAStreamOut *pAudioALSAStreamOut, const audio_devices_t current_output_devices, audio_devices_t output_devices);
    status_t routingInputDevice(AudioALSAStreamIn *pAudioALSAStreamIn, const audio_devices_t current_input_device, audio_devices_t input_device);
    audio_devices_t CheckInputDevicePriority(audio_devices_t input_device);
    uint32_t setUsedDevice(const audio_devices_t used_device);
    bool CheckStreaminPhonecallRouting(audio_devices_t new_phonecall_device, bool checkrouting = false);
    audio_mode_t getMode();
    status_t syncSharedOutDevice(audio_devices_t routingSharedDevice, AudioALSAStreamOut *currentStreamOut);
    bool isOutputNeedRouting(AudioALSAStreamOut *eachStreamOut, AudioALSAStreamOut *currentStreamOut,
                             audio_devices_t routingSharedOutDevice);

    // check if headset has changed
    bool CheckHeadsetChange(const audio_devices_t current_input_device, audio_devices_t input_device);


    status_t DeviceNoneUpdate(void);

    /**
     * FM radio related opeation // TODO(Harvey): move to FM Controller later
     */
    status_t setFmEnable(const bool enable, bool bForceControl = false, bool bForce2DirectConn = false, audio_devices_t output_device = AUDIO_DEVICE_NONE);
    bool     getFmEnable();

    // TODO(Harvey): move to Loopback Controller later
    status_t setLoopbackEnable(const bool enable);

    status_t setHdmiEnable(const bool enable);

    /**
     * suspend/resume all input/output stream
     */
    status_t setAllOutputStreamsSuspend(const bool suspend_on, const bool setModeRequest = false);
    status_t setAllInputStreamsSuspend(const bool suspend_on, const bool setModeRequest = false, const capture_handler_t caphandler = CAPTURE_HANDLER_ALL);
    status_t setAllStreamsSuspend(const bool suspend_on, const bool setModeRequest = false);


    /**
     * standby all input/output stream
     */
    status_t standbyAllOutputStreams(const bool setModeRequest = false);
    status_t standbyAllInputStreams(const bool setModeRequest = false, const capture_handler_t caphandler = CAPTURE_HANDLER_ALL);
    status_t standbyAllStreams(const bool setModeRequest = false);


    /**
     * audio mode status
     */
    bool isPhoneCallOpen();
    inline bool isModeInPhoneCall() { return isModeInPhoneCall(mAudioMode); }
    inline bool isModeInVoipCall()  { return isModeInVoipCall(mAudioMode); }
    inline bool isModeInRingtone()  { return isModeInRingtone(mAudioMode); }
    inline audio_mode_t getAudioMode() { return mAudioMode; }
    inline audio_mode_t getModeForGain() { return isPhoneCallOpen() ? AUDIO_MODE_IN_CALL : mAudioMode; }

    // TODO(Harvey): test code, remove it later
    inline uint32_t getStreamOutVectorSize()  { return mStreamOutVector.size(); }
    inline AudioALSAStreamOut *getStreamOut(const size_t i)  { return mStreamOutVector[i]; }
    inline AudioALSAStreamIn *getStreamIn(const size_t i)  { return mStreamInVector[i]; }

#if defined(MTK_HIFIAUDIO_SUPPORT)
    /**
     * Hi-Fi Mode
     */
    int setAllStreamHiFi(AudioALSAStreamOut *pAudioALSAStreamOut, uint32_t sampleRate);
#endif

    /**
     * stream in related
     */
    virtual size_t getInputBufferSize(uint32_t sampleRate, audio_format_t format, uint32_t channelCount);
    status_t updateOutputDeviceForAllStreamIn(audio_devices_t outputDevices);
    status_t updateOutputDeviceForAllStreamIn_l(audio_devices_t outputDevices);

    status_t SetMusicPlusStatus(bool bEnable);
    bool GetMusicPlusStatus();
    status_t UpdateACFHCF(int value);
    status_t SetACFPreviewParameter(void *ptr, int len);
    status_t SetHCFPreviewParameter(void *ptr, int len);
    status_t SetBesLoudnessStatus(bool bEnable);
    bool GetBesLoudnessStatus();
    status_t SetBesLoudnessControlCallback(const BESLOUDNESS_CONTROL_CALLBACK_STRUCT *callback_data);

    status_t SetEMParameter(AUDIO_CUSTOM_PARAM_STRUCT *pSphParamNB);
    status_t updateSpeechNVRAMParam(const int speech_band);
    status_t UpdateDualMicParams();
    status_t UpdateMagiConParams();
    status_t UpdateHACParams();
    status_t UpdateSpeechMode();
    status_t UpdateSpeechVolume();
    status_t SetVCEEnable(bool bEnable);
    status_t UpdateSpeechLpbkParams();

    status_t Enable_DualMicSettng(sph_enh_dynamic_mask_t sphMask, bool bEnable);
    status_t Set_LSPK_DlMNR_Enable(sph_enh_dynamic_mask_t sphMask, bool bEnable);
    status_t setSpkOutputGain(int32_t gain, uint32_t ramp_sample_cnt);
    status_t setSpkFilterParam(uint32_t fc, uint32_t bw, int32_t th);
    status_t setVtNeedOn(const bool vt_on);
    bool EnableBesRecord(void);
    bool getPhoncallOutputDevice();

    /**
     * Magic Conference Call
     */
    status_t SetMagiConCallEnable(bool bEnable);
    bool GetMagiConCallEnable(void);

    /**
     * HAC
     */
    status_t SetHACEnable(bool bEnable);
    bool GetHACEnable(void);

    /**
     * volume index
     */
    int setVolumeIndex(int stream, int device, int index);
    int getVolumeIndex() { return mVolumeIndex; }

    /**
     * reopen Phone Call audio path according to RIL mapped modem notify
     */
    int phoneCallRefreshModem(const char *rilMappedMDName);

    /**
     * reopen Phone Call audio path
     */
    int phoneCallReopen();

    /**
     * update Phone Call phone id
     */
    int phoneCallUpdatePhoneId(const phone_id_t phoneId);

    /**
     * BT headset name
     */
    status_t SetBtHeadsetName(const char *btHeadsetName);
    const char *GetBtHeadsetName();

    /**
     * BT NREC
     */
    status_t SetBtHeadsetNrec(bool bEnable);
    bool GetBtHeadsetNrecStatus(void);

    /**
     * voice wake up
     */
    status_t setVoiceWakeUpNeedOn(const bool enable);
    bool     getVoiceWakeUpNeedOn();

    /**
     * VoIP dynamic function
     */
    void UpdateDynamicFunctionMask(void);


    /**
     * low latency
     */
    status_t setScreenState(bool mode);

    /**
     * a2dp offload
     */
    void setBluetoothAudioOffloadParam(const sp<::android::hardware::bluetooth::a2dp::V1_0::IBluetoothAudioHost> &hostIf,
                                       const ::android::hardware::bluetooth::a2dp::V1_0::CodecConfiguration &codecConfig,
                                       bool on);
    void setA2dpSuspend(bool on);
    void setA2dpSuspendStatus(int status);
    int getA2dpSuspendStatus(void);
    void setA2dpPlaybackPaused(bool status);
    bool getA2dpPlaybackPaused(void);

    /**
     * Bypass DL Post Process Flag
     */
    status_t setBypassDLProcess(bool flag);


    /**
     * [TMP] stream out routing related // TODO(Harvey)
     */
    virtual status_t setParametersToStreamOut(const String8 &keyValuePairs);
    virtual status_t setParameters(const String8 &keyValuePairs, int IOport);

    /**
     * Enable/Disable speech Strm
     */
    status_t DisableSphStrm(const audio_mode_t new_mode);
    status_t EnableSphStrm(const audio_mode_t new_mode);
    status_t DisableSphStrmByDevice(audio_devices_t output_devices);
    status_t EnableSphStrmByDevice(audio_devices_t output_devices);
    bool isModeInPhoneCallSupportEchoRef(const audio_mode_t audio_mode);
    bool IsSphStrmSupport(void);
    void updateDeviceConnectionState(audio_devices_t device, bool connect);
    bool getDeviceConnectionState(audio_devices_t device);

    /**
     * Check for VOIP and FM not concurrent
     */
    bool     isEchoRefUsing();

    /**
     * Scene APIs
     */
    void setCustScene(const String8 scene);
    String8 getCustScene() { return mCustScene; }

    sp<::android::hardware::bluetooth::a2dp::V1_0::IBluetoothAudioHost> getBluetoothAudioHostIf() {
        return mBluetoothAudioOffloadHostIf;
    }
    int getBluetoothAudioSession() {
        return mBluetoothAudioOffloadSession;
    }
    void *getBluetoothAudioCodecInfo(void);
    uint32_t getBluetoothAudioCodecType(void);

    /*
     * VoIP independent path
     */
    bool needEnableVoip(const stream_attribute_t *streamAttribute);

protected:
    AudioALSAStreamManager();

    inline bool isModeInPhoneCall(const audio_mode_t audio_mode) {
        return (audio_mode == AUDIO_MODE_IN_CALL);
    }

    inline bool isModeInVoipCall(const audio_mode_t audio_mode) {
        return (audio_mode == AUDIO_MODE_IN_COMMUNICATION);
    }

    inline bool isModeInRingtone(const audio_mode_t audio_mode) {
        return (audio_mode == AUDIO_MODE_RINGTONE);
    }

    inline bool isModeInNormal(const audio_mode_t audio_mode) {
        return (audio_mode == AUDIO_MODE_NORMAL);
    }

    void SetInputMute(bool bEnable);
    bool enableHifi(uint32_t type, uint32_t typeValue);

private:
    /**
     * singleton pattern
     */
    static AudioALSAStreamManager *mStreamManager;


    /**
     * stream manager lock
     */
    AudioLock mStreamVectorLock; // used in setMode & open/close input/output stream
    AudioLock mPlaybackHandlerVectorLock;
    AudioLock mLock;
    AudioLock mAudioModeLock;
    AudioLock mA2dpSuspendStatusLock;
    AudioLock mA2dpPlaybackPausedLock;
    AudioLock mVowLock;

    /**
     * stream in/out vector
     */
    KeyedVector<uint32_t, AudioALSAStreamOut *> mStreamOutVector;
    KeyedVector<uint32_t, AudioALSAStreamIn *>  mStreamInVector;
    uint32_t mStreamOutIndex;
    uint32_t mStreamInIndex;


    /**
     * stream playback/capture handler vector
     */
    KeyedVector<uint32_t, AudioALSAPlaybackHandlerBase *> mPlaybackHandlerVector;
    KeyedVector<uint32_t, AudioALSACaptureHandlerBase *>  mCaptureHandlerVector;
    uint32_t mPlaybackHandlerIndex;
    uint32_t mCaptureHandlerIndex;


    /**
     * speech phone call controller
     */
    AudioALSASpeechPhoneCallController *mSpeechPhoneCallController;

    /**
     * SmartPA controller
     */

    AudioSmartPaController *mSmartPaController;

    /**
     * FM radio
     */
    AudioALSAFMController *mFMController;


    /**
     * volume controller
     */
    AudioVolumeInterface *mAudioALSAVolumeController;
    SpeechDriverFactory *mSpeechDriverFactory;


    /**
     * volume related variables
     */
    bool mMicMute;


    /**
     * audio mode
     */
    audio_mode_t mAudioMode;
    audio_mode_t mAudioModePolicy;
    bool mEnterPhoneCallMode;
    bool mPhoneCallControllerStatusPolicy;
    bool mResumeAllStreamsAtRouting;
    bool mIsNeedResumeStreamOut;

    /**
     * Loopback related
     */
    bool mLoopbackEnable; // TODO(Harvey): move to Loopback Controller later

    bool mHdmiEnable; // TODO(Harvey): move to Loopback Controller later
    /**
     * stream in/out vector
     */
    KeyedVector<uint32_t, AudioMTKFilterManager *> mFilterManagerVector;

    bool mBesLoudnessStatus;

    void (*mBesLoudnessControlCallback)(void *data);

    /**
     * Speech EnhanceInfo Instance
     */
    AudioSpeechEnhanceInfo *mAudioSpeechEnhanceInfoInstance;

    /**
     * headphone change flag
     */
    bool mHeadsetChange;

    /**
     * voice wake up
     */
    AudioALSAVoiceWakeUpController *mAudioALSAVoiceWakeUpController;
    bool mVoiceWakeUpNeedOn;
    bool mForceDisableVoiceWakeUpForSetMode;

    /**
    * Bypass DL Post Process Flag
    */
    bool mBypassPostProcessDL;

    /**
    * Bypass UL DMNR Pre Process Flag
    */
    bool mBypassDualMICProcessUL;

    /**
     * BT device info
     */
    const char *mBtHeadsetName;

    /**
     * AudioCustParamClient
     */
    AudioCustParamClient *mAudioCustParamClient;

    /*
     * Device Available flag
     */
    uint32_t mAvailableOutputDevices;

    /*
     * flag of dynamic enable verbose/debug log
     */
    int mLogEnable;

    /**
     * Customized scenario information
     */
    String8 mCustScene;

    sp<::android::hardware::bluetooth::a2dp::V1_0::IBluetoothAudioHost> mBluetoothAudioOffloadHostIf;
    int mBluetoothAudioOffloadSession;
    int mA2dpSuspend;
    bool mA2dpPlaybackPaused;

    /**
     * Prevent setMode during destructing
     */
    bool mStreamManagerDestructing;

    /**
     * volume index
     */
    int mVolumeIndex;

    /**
     * ADSP
     */
    static void audioDspStopWrap(void *arg);
    static void audioDspReadyWrap(void *arg);
    void audioDspStatusUpdate(const bool adspReady);

    AudioALSAStreamOut *mOutputStreamForCall;
    audio_devices_t mCurrentOutputDevicesForCall;
    audio_devices_t mOutputDevicesForCall;

    /*
     * Output available flags
     */
    uint32_t mAvailableOutputFlags;

    /*
     * Record offload query
     */
    bool isCaptureOffload(stream_attribute_t *stream_attribute_target);

};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_STREAM_MANAGER_H
