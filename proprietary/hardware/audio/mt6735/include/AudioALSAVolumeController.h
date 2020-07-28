#ifndef ANDROID_AUDIO_ALSA_VOLUME_CONTROLLER_H
#define ANDROID_AUDIO_ALSA_VOLUME_CONTROLLER_H

#include "AudioVolumeInterface.h"
#include "AudioType.h"
#include <utils/Log.h>
#include <utils/String16.h>
#include <cutils/properties.h>

#include "AudioCustParam.h"
#include "audio_custom_exp.h"
#include <tinyalsa/asoundlib.h>
#include "AudioSpeechEnhanceInfo.h"
#include "AudioALSASpeechPhoneCallController.h"

namespace android {
#define NUM_AUDIO_ROUTE  (17)

/****************************************************
* Define Volume Range of  sound & Voice.
*****************************************************/
#define DEVICE_AUDIO_BUFFER_MAX_VOLUME           (9)
#define DEVICE_AUDIO_BUFFER_MIN_VOLUME           (-5)
#define DEVICE_AMP_MAX_VOLUME     (15)
#ifndef DEVICE_AMP_MIN_VOLUME   //reDefine @Audio_custom_exp.h
#define DEVICE_AMP_MIN_VOLUME     (6)
#endif
#define DEVICE_IV_BUFFER_MAX_VOLUME     (5)
#define DEVICE_IV_BUFFER_MIN_VOLUME     (-2)
#define DEVICE_LINEIN_PLAY_MAX_VOLUME  (20)

#define AUDIO_VOLUME_MAX       (160)
#define AUDIO_ONEDB_STEP         (4)
#define VOICE_VOLUME_MAX       (160)
#define VOICE_ONEDB_STEP         (4)
#define AMP_VOLUME_MAX           (180)
#define AMP_ONEDB_STEP             (4)
#define UPLINK_GAIN_MAX            (252)
#define UPLINK_ONEDB_STEP         (4)
#define SIDETONE_GAIN_MAX            (240)
#define SIDETONE_ONEDB_STEP         (4)

#define HW_DIGITAL_GAIN_MAX (252)
#define HW_DIGITAL_GAIN_STEP (4)   // 2==> 0.5 dB ,4==>1dB

#define AUDIO_SYSTEM_UL_GAIN_MAX            (45)
#define MIN_PGA_GAIN                                         (0)
#define MAX_PGA_GAIN_RANGE                          (30)
#define AUDIO_UL_PGA_STEP                               (6)
#define SW_AGC_GAIN_MAX                                (25)
#define TRANSPARENT_AGC_GAIN_OFFSET       (33)                        //for digital microphone


typedef enum {
    Idle_Normal_Record = 0,
    Idle_Headset_Record,
    Voice_Rec_Mic_Handset,
    Voice_Rec_Mic_Headset,
    Idle_Video_Record_Handset,
    Idle_Video_Record_Headset,
    Normal_Mic,
    Headset_Mic,
    Handfree_Mic,
    VOIP_Normal_Mic,
    VOIP_Headset_Mic,
    VOIP_Handfree_Mic,
    TTY_CTM_Mic,
    Level_Shift_Buffer_Gain,
    Analog_PLay_Gain,
    Voice_UnLock_Mic_Handset,
    Voice_UnLock_Mic_Headset,
    Customization1_Mic_Handset,
    Customization1_Mic_Headset,
    Customization2_Mic_Handset,
    Customization2_Mic_Headset,
    Customization3_Mic_Handset,
    Customization3_Mic_Headset,
    Individual_2ndSet_Mic,
    Unprocessed_Mic,
    Num_Mic_Gain
} MIC_GAIN_MODE;

typedef enum {
    EarPiece_SideTone_Gain = 0,
    Headset_SideTone_Gain,
    LoudSpk_SideTone_Gain,
    Num_Side_Tone_Gain
} SIDETOEN_GAIN_MODE;

typedef enum {
    Audio_Earpiece = 0,
    Audio_Headset,
    Audio_Headphone,
    Audio_Speaker,
    Audio_DualMode_Earpiece,
    Audio_DualMode_Headset,
    Audio_DualMode_Headphone,
    Audio_DualMode_speaker,
    Ringtone_Earpiece,
    Ringtone_Headset,
    Ringtone_Headphone,
    Ringtone_Speaker,
    Sipcall_Earpiece,
    Sipcall_Headset,
    Sipcall_Headphone,
    Sipcall_Speaker,
    Num_of_Audio_gain
} AUDIO_GAIN_MODE;

typedef enum {
    DRC_VERSION_1  = 0,
    DRC_VERSION_2 = 1,
} DRC_VERSION;

typedef enum {
    HEADPHONE_IMPEDANCE_16  = 16,
    HEADPHONE_IMPEDANCE_32 =  32,
    HEADPHONE_IMPEDANCE_64 =  64,
    HEADPHONE_IMPEDANCE_128 = 128,
    HEADPHONE_IMPEDANCE_256 = 256,
} AUDIO_HEADPHONE_IMPEDANCE;

class AudioALSAHardwareResourceManager;

class AudioALSAVolumeController : public AudioVolumeInterface {
public:
    static float linearToLog(int volume);
    static int logToLinear(float volume);

    static AudioALSAVolumeController *getInstance();
    ~AudioALSAVolumeController() {};
    /**
     * check to see if the audio hardware interface has been initialized.
     */
    virtual status_t    initCheck();
    virtual status_t    initVolumeController();

    virtual status_t setMasterVolume(float v, audio_mode_t mode, uint32_t devices);
    virtual status_t setMasterVolume(void);
    virtual float getMasterVolume();
    virtual audio_devices_t getNewDeviceForTty(audio_devices_t OutputDevice, TtyModeType tty_mode);
    virtual status_t setVoiceVolume(float v, audio_mode_t mode, uint32_t devices);
    virtual status_t setVoiceVolume(int MapVolume, uint32_t device);
    virtual status_t Apply2in1VoiceGain(int degradeDb, audio_mode_t mode, uint32_t device);
    virtual status_t ApplyVoiceGain(int degradeDb, audio_mode_t mode, uint32_t device);
    virtual float getVoiceVolume(void);
#ifdef MTK_AUDIO_GAIN_TABLE
    virtual status_t setAnalogVolume(int stream, int device, int index, audio_mode_t mode);
    virtual status_t speechBandChange(bool nb);
    virtual bool isNbSpeechBand(void);
    virtual status_t setBtVolumeCapability(bool support);
#endif

    // here only valid stream can be set .
    virtual status_t setStreamVolume(int stream, float v);
    virtual status_t setStreamMute(int stream, bool mute);
    virtual float getStreamVolume(int stream);

    virtual status_t SetMicGain(uint32_t Mode, uint32_t devices);
    virtual status_t SetULTotalGain(uint32_t Mode, unsigned char Volume);

    virtual status_t SetSideTone(uint32_t Mode, uint32_t devices);
    virtual status_t SetMicGainTuning(uint32_t Mode, uint32_t gain);

    /**
    * volume controller GetSideToneGain
    * base on output device get sidetone gain
    * @param device
    * @return gain
    */
    virtual uint32_t GetSideToneGain(uint32_t device);

    virtual status_t ApplySideTone(uint32_t Mode);

    /**
    * volume controller ApplyMicGain
    * base on mode gain and MicType to decide mic gain
    * @param MicType
    * @param Mode
    */
    virtual status_t ApplyMicGain(uint32_t MicType, int mode);

    /**
        * volume controller SetCaptureGain
        * base on mode gain and MicType to decide mic gain
        * @param Mode
        * @param Input Source
        * @param Input device
        * @param Output devices
        */
    virtual status_t SetCaptureGain(audio_mode_t mode, audio_source_t source, audio_devices_t input_device, audio_devices_t output_devices);
    virtual status_t SetCaptureGainForTty(audio_mode_t mode);

    /**
    * volume controller SetDigitalHwGain
    * base on mode gain and route to set digital HW gain
    * @param Mode
    * @param Gain
    * @param routes
    * @return status_t
    */
    virtual status_t SetDigitalHwGain(uint32_t Mode, uint32_t Gain, uint32_t routes);

    /**
    * volume controller Set modem DL gain
    * @param Gain
    * @return status_t
    */
    virtual void ApplyMdDlGain(int  Gain);

    /**
    * volume controller Set modem DL Ehn gain
    * @param Gain
    * @return status_t
    */
    virtual void ApplyMdDlEhn1Gain(int32_t Gain);

    /**
    * volume controller Set modem Ul gain
    * @param Gain
    * @return status_t
    */
    virtual void ApplyMdUlGain(int  Gain);

    /**
    * volume controller map volume to digital gain
    * @param Gain
    * @return digital gain
    */
    virtual uint16_t MappingToDigitalGain(unsigned char Gain);

    /**
    * volume controller map volume to PGA gain
    * @param Gain
    * @return PGA gain
    */
    virtual uint16_t MappingToPGAGain(unsigned char Gain);

    /**
    *  volume controller GetSWMICGain
    * get MIC software digital gain for HD record library
    */
    virtual short GetSWMICGain() {return mSwAgcGain;}

    /**
    *  volume controller GetULTotalGain
    * get MIC software digital gain for BesRecord library
    */
    virtual uint8_t GetULTotalGain() {return mULTotalGain;}


    /**
    *  set FM Volume (using HW_GAIN2), volume: 0.0 ~ 1.0
    */
    virtual status_t setFmVolume(const float fm_volume);

    /**
    *  Audio Tuning tool set playback gain
    */
    virtual int ApplyAudioGainTuning(int Gain, uint32_t mode, uint32_t device);

    /**
    *  apply hp impedance setting
    */
    int ApplyHpimpedanceGain(int DegradedBGain);

    /**
    *  get hp impedance setting
    */
    bool GetHeadPhoneImpedance(void);

    virtual uint32_t GetOffloadGain(float vol_f);



private:
    static AudioALSAVolumeController *UniqueVolumeInstance;
    AudioALSAVolumeController();
    AudioALSAVolumeController(const AudioALSAVolumeController &);             // intentionally undefined
    AudioALSAVolumeController &operator=(const AudioALSAVolumeController &);  // intentionally undefined
    void GetDefaultVolumeParameters(AUDIO_VER1_CUSTOM_VOLUME_STRUCT *volume_param);

    // cal and set and set analog gainQuant
    void ApplyAudioGain(int Gain, uint32_t mode, uint32_t device);
    void ApplyAmpGain(int Gain, uint32_t mode, uint32_t device);
    void ApplyExtAmpHeadPhoneGain(int Gain, uint32_t mode, uint32_t device);
    void ApplyDualmodeGain(int Gain, uint32_t mode, uint32_t device);
    bool SetVolumeRange(uint32_t mode, int32_t MaxVolume, int32_t MinVolume, int32_t VolumeRange);
    uint32_t SortHeadPhoneImpedance(uint32_t Impedance);
    int  MapHeadPhoneImpedance(void);
    bool GetHeadPhoneImpedanceEnable(void);

    bool IsHeadsetMicInput(uint32_t device);
    uint16_t UpdateSidetone(int DL_PGA_Gain, int  Sidetone_Volume, uint8_t SW_AGC_Ul_Gain);

    bool CheckMicUsageWithMode(uint32_t MicType, int mode);
    uint32_t GetSideToneGainType(uint32_t devices);

    bool ModeSetVoiceVolume(int mode);
    uint32_t GetDRCVersion(uint32_t device);
    // alsa driver set pga gain function
    void SetReceiverGain(int DrgradeDb);
    void SetHeadPhoneRGain(int DrgradeDb);
    void SetHeadPhoneLGain(int DrgradeDb);
    void SetLinoutRGain(int DrgradeDb);
    void SetLinoutLGain(int DrgradeDb);
    void SetSpeakerGain(int DegradedBGain);
    void SetAdcPga1(int DrgradeDb);
    void SetAdcPga2(int DrgradeDb);
    void SetAdcPga3(int DrgradeDb);
    void SetAdcPga4(int DrgradeDb);
    void SetInternalSpkGain(int degradeDb);
    void SetExternalSpkGain(int degradeDb);
    void Set2IN1InternalSpkGain(int degradeDb);
    void Set2IN1ExternalSpkGain(int degradeDb);

    int GetReceiverGain(void);
    int GetHeadphoneRGain(void);
    int GetHeadphoneLGain(void);
    int GetLineOutphoneRGain(void);
    int GetLineOutphoneLGain(void);
    int GetSPKGain(void);

    /**
    * volume controller mapping gain into DB
    * base on Gain and return Degrade in DB
    * @param Gain
    * @return uint32_t
    */
    uint32_t MapDigitalHwGain(uint32_t Gain);

    // objects
    AudioALSAHardwareResourceManager *mHardwareResourceManager;

    float mVoiceVolume;
    audio_mode_t mMode;
    uint32_t mOutputDevices;
    uint8_t mSwAgcGain;
    uint8_t mULTotalGain;
    float mMasterVolume;
    float mStreamVolume[AUDIO_STREAM_CNT];

    uint32_t mSpeechDrcType ;
    bool mInitDone;

    // data strcutre for volume
    AUDIO_VER1_CUSTOM_VOLUME_STRUCT mVolumeParam;
    AUDIO_CUSTOM_PARAM_STRUCT mSphParamNB;
    AUDIO_CUSTOM_WB_PARAM_STRUCT mSphParamWB;
    AUDIO_CUSTOM_HAC_PARAM_STRUCT mHacParam;
    int32_t mVolumeMax[Num_of_Audio_gain];
    int32_t mVolumeMin[Num_of_Audio_gain];
    int32_t mVolumeRange[Num_of_Audio_gain];
    int mMicGain[Num_Mic_Gain];
    uint8_t mULTotalGainTable[Num_Mic_Gain];
    int mSideTone[Num_Side_Tone_Gain];
    uint32_t mHeadPhoneImpedence;
    bool mHeadPhoneImpedenceEnable;

    //ALSA relate control
    struct mixer *mMixer;
    const uint32_t kAudioSoundCardIndex = 0;

    //for audio tuning tool
    /**
     * Speech EnhanceInfo Instance
     */
    AudioSpeechEnhanceInfo *mAudioSpeechEnhanceInfoInstance;
};

}

#endif

