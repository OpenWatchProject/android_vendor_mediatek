#ifndef ANDROID_AUDIO_FTM_BASE_H
#define ANDROID_AUDIO_FTM_BASE_H

#include <sys/types.h>

namespace android {

enum FMTX_Command {
    FREQ_NONE = 0,
    FREQ_1K_HZ,
    FREQ_2K_HZ,
    FREQ_3K_HZ,
    FREQ_4K_HZ,
    FREQ_5K_HZ,
    FREQ_6K_HZ,
    FREQ_7K_HZ,
    FREQ_8K_HZ,
    FREQ_9K_HZ,
    FREQ_10K_HZ,
    FREQ_11K_HZ,
    FREQ_12K_HZ,
    FREQ_13K_HZ,
    FREQ_14K_HZ,
    FREQ_15K_HZ
};


enum UL_SAMPLERATE_INDEX {
    UPLINK8K = 0,
    UPLINK16K,
    UPLINK32K,
    UPLINK48K,
    UPLINK_UNDEF
};

enum select_mic_type {
    SELECT_MIC_DEFAULT,
    SELECT_MIC1,
    SELECT_MIC2,
    SELECT_MIC3,
    SELECT_MIC4,
    SELECT_MIC5,
    SELECT_MIC_TYPE_MAX,
};

// for afe loopback
#define MIC1_OFF  0
#define MIC1_ON   1
#define MIC2_OFF  2
#define MIC2_ON   3
#define MIC3_OFF  4
#define MIC3_ON   5
#define MIC4_OFF  6
#define MIC4_ON   7


// for acoustic loopback
#define ACOUSTIC_STATUS   -1
#define DUAL_MIC_WITHOUT_DMNR_ACS_OFF 0
#define DUAL_MIC_WITHOUT_DMNR_ACS_ON  1
#define DUAL_MIC_WITH_DMNR_ACS_OFF   2
#define DUAL_MIC_WITH_DMNR_ACS_ON    3


class AudioFtmBaseVirtual {
public:
    virtual ~AudioFtmBaseVirtual() {};
    /// Codec
    virtual void Audio_Set_Speaker_Vol(int level) = 0;
    virtual void Audio_Set_Speaker_On(int Channel) = 0;
    virtual void Audio_Set_Speaker_Off(int Channel) = 0;
    virtual void Audio_Set_HeadPhone_On(int Channel) = 0;
    virtual void Audio_Set_HeadPhone_Off(int Channel) = 0;
    virtual void Audio_Set_Earpiece_On() = 0;
    virtual void Audio_Set_Earpiece_Off() = 0;


    /// for factory mode & Meta mode (Analog part)
    virtual void FTM_AnaLpk_on(void) = 0;
    virtual void FTM_AnaLpk_off(void) = 0;

    virtual int SineGenTest(char sinegen_test) = 0;

    /// Output device test
    virtual int RecieverTest(char receiver_test) = 0;
    virtual int LouderSPKTest(char left_channel, char right_channel) = 0;
    virtual int EarphoneTest(char bEnable) = 0;
    virtual int EarphoneTestLR(char bLR) = 0;

    /// Input device test
    virtual int SpecificBuildInMicTest(char type) = 0;

    /// Speaker over current test
    virtual int Audio_READ_SPK_OC_STA(void) = 0;
    virtual int LouderSPKOCTest(char left_channel, char right_channel) = 0;


    /// Loopback // TODO: Add in platform!!!
    virtual int PhoneMic_Receiver_Loopback(char echoflag) = 0;
    virtual int PhoneMic_EarphoneLR_Loopback(char echoflag) = 0;
    virtual int PhoneMic_SpkLR_Loopback(char echoflag) = 0;
    virtual int HeadsetMic_EarphoneLR_Loopback(char bEnable, char bHeadsetMic) = 0;
    virtual int HeadsetMic_SpkLR_Loopback(char echoflag) = 0;
    virtual int HeadsetMic_Receiver_Loopback(char bEnable, char bHeadsetMic) = 0;

    virtual int PhoneMic_Receiver_Acoustic_Loopback(int Acoustic_Type, int *Acoustic_Status_Flag, int bHeadset_Output) = 0;


    /// FM / mATV
    virtual int FMLoopbackTest(char bEnable) = 0;

    virtual int Audio_FM_I2S_Play(char bEnable) = 0;
    virtual int Audio_MATV_I2S_Play(int enable_flag) = 0;
    virtual int Audio_FMTX_Play(bool Enable, unsigned int Freq) = 0;

    virtual int ATV_AudPlay_On(void) = 0;
    virtual int ATV_AudPlay_Off(void) = 0;
    virtual unsigned int ATV_AudioWrite(void *buffer, unsigned int bytes) = 0;


    /// HDMI
    virtual int HDMI_SineGenPlayback(bool bEnable, int dSamplingRate) = 0;


    /// Vibration Speaker // MTK_VIBSPK_SUPPORT??
    virtual int      SetVibSpkCalibrationParam(void *cali_param) = 0;
    virtual uint32_t GetVibSpkCalibrationStatus() = 0;
    virtual void     SetVibSpkEnable(bool enable, uint32_t freq) = 0;
    virtual void     SetVibSpkRampControl(uint8_t rampcontrol) = 0;

    virtual bool     ReadAuxadcData(int channel, int *value) = 0;
    virtual void     SetStreamOutPostProcessBypass(bool flag) = 0;

    // speaker calibration for SmartPa
    virtual bool     SpeakerCalibration(int calibStage) = 0;
};


class AudioFtmBase: public AudioFtmBaseVirtual {
public:
    virtual ~AudioFtmBase();
    static AudioFtmBase *createAudioFtmInstance();

    /// Codec
    virtual void Audio_Set_Speaker_Vol(int level);
    virtual void Audio_Set_Speaker_On(int Channel);
    virtual void Audio_Set_Speaker_Off(int Channel);
    virtual void Audio_Set_HeadPhone_On(int Channel);
    virtual void Audio_Set_HeadPhone_Off(int Channel);
    virtual void Audio_Set_Earpiece_On();
    virtual void Audio_Set_Earpiece_Off();


    /// for factory mode & Meta mode (Analog part)
    virtual void FTM_AnaLpk_on(void);
    virtual void FTM_AnaLpk_off(void);


    virtual int SineGenTest(char sinegen_test);

    /// Output device test
    virtual int RecieverTest(char receiver_test);
    virtual int LouderSPKTest(char left_channel, char right_channel);
    virtual int EarphoneTest(char bEnable);
    virtual int EarphoneTestLR(char bLR);

    /// Input device test
    virtual int SpecificBuildInMicTest(char type);

    /// Speaker over current test
    virtual int Audio_READ_SPK_OC_STA(void);
    virtual int LouderSPKOCTest(char left_channel, char right_channel);


    /// Loopback // TODO: Add in platform!!!
    virtual int PhoneMic_Receiver_Loopback(char echoflag);
    virtual int PhoneMic_EarphoneLR_Loopback(char echoflag);
    virtual int PhoneMic_SpkLR_Loopback(char echoflag);
    virtual int HeadsetMic_EarphoneLR_Loopback(char bEnable, char bHeadsetMic);
    virtual int HeadsetMic_SpkLR_Loopback(char echoflag);
    virtual int HeadsetMic_Receiver_Loopback(char bEnable, char bHeadsetMic);

    virtual int PhoneMic_Receiver_Acoustic_Loopback(int Acoustic_Type, int *Acoustic_Status_Flag, int bHeadset_Output);


    /// FM / mATV
    virtual int FMLoopbackTest(char bEnable);

    virtual int Audio_FM_I2S_Play(char bEnable);
    virtual int Audio_MATV_I2S_Play(int enable_flag);
    virtual int Audio_FMTX_Play(bool Enable, unsigned int Freq);

    virtual int ATV_AudPlay_On(void);
    virtual int ATV_AudPlay_Off(void);
    virtual unsigned int ATV_AudioWrite(void *buffer, unsigned int bytes);


    /// HDMI
    virtual int HDMI_SineGenPlayback(bool bEnable, int dSamplingRate);


    /// Vibration Speaker // MTK_VIBSPK_SUPPORT??
    virtual int      SetVibSpkCalibrationParam(void *cali_param);
    virtual uint32_t GetVibSpkCalibrationStatus();
    virtual void     SetVibSpkEnable(bool enable, uint32_t freq);
    virtual void     SetVibSpkRampControl(uint8_t rampcontrol);

    virtual bool     ReadAuxadcData(int channel, int *value);
    virtual void     SetStreamOutPostProcessBypass(bool flag);

    // speaker calibration for SmartPa
    virtual bool     SpeakerCalibration(int calibStage);

protected:
    AudioFtmBase();

};

}; // end namespace android

#ifdef __cplusplus
extern "C" {
#endif
android::AudioFtmBaseVirtual *pfuncGetAudioFtmByDL(void);
#ifdef __cplusplus
}
#endif

typedef android::AudioFtmBaseVirtual *create_AudioFtm(void);

#endif // end of ANDROID_AUDIO_FTM_BASE_H
