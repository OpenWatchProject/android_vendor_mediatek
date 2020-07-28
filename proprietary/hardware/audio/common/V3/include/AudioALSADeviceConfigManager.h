#ifndef ANDROID_AUDIO_MTK_DEVICE_CONFIG_MANAGER_H
#define ANDROID_AUDIO_MTK_DEVICE_CONFIG_MANAGER_H

#include <stdint.h>
#include <sys/types.h>
#include <utils/Mutex.h>
#include <utils/String8.h>
#include "AudioType.h"
#include <utils/Log.h>
#include <cutils/properties.h>
#include <sys/types.h>
#include <cutils/config_utils.h>
#include <cutils/misc.h>
#include <utils/Errors.h>
#include <utils/KeyedVector.h>
#include <tinyxml.h>

#define AUDIO_DEVICE_TURNON                   "turnon"
#define AUDIO_DEVICE_TURNOFF                 "turnoff"
#define AUDIO_DEVICE_SETTING                  "setting"

#define AUDIO_DEVICE_HEADPHONE           "headphone_output"
#if defined(MTK_HIFIAUDIO_SUPPORT)
#define AUDIO_DEVICE_HIFI_DAC    "hifi_dac_output"
#endif
#define AUDIO_DEVICE_SPEAKER                 "speaker_output"
#define AUDIO_DEVICE_2IN1_SPEAKER       "two_in_one_speaker_output"
#define AUDIO_DEVICE_RECEIVER                "receiver_output"
#define AUDIO_DEVICE_EARPHONE_SPEAKER "headphoneSpeaker_output"
#define AUDIO_DEVICE_EXT_SPEAKER         "ext_speaker_output"

#define AUDIO_DEVICE_MAXIM_SPEAKER          "maxim_speaker_output"
#define AUDIO_DEVICE_MAXIM_EARPHONE_SPEAKER "maxim_headphoneSpeaker_output"

#define AUDIO_LOOPBACK_USE_SINGLE_MIC_L_CH "loopback_use_single_mic_l_ch"
#define AUDIO_LOOPBACK_USE_SINGLE_MIC_R_CH "loopback_use_single_mic_r_ch"

#define AUDIO_DEVICE_BUILTIN_MIC_MIC1  "builtin_Mic_Mic1"
#define AUDIO_DEVICE_BUILTIN_MIC_MIC2  "builtin_Mic_Mic2"
#define AUDIO_DEVICE_BUILTIN_MIC_MIC3  "builtin_Mic_Mic3"
#define AUDIO_DEVICE_BUILTIN_MIC_MIC4  "builtin_Mic_Mic4"
#define AUDIO_DEVICE_BUILTIN_MIC_MIC5  "builtin_Mic_Mic5"
#define AUDIO_DEVICE_BUILTIN_MIC_VOW_MIC  "builtin_Mic_Vow_Mic"
#define AUDIO_DEVICE_BUILTIN_MIC_VOW_MIC2  "builtin_Mic_Vow_Mic2"
#define AUDIO_DEVICE_BUILTIN_MIC_VOW_DUAL_MIC  "builtin_Mic_Vow_DualMic"

#define AUDIO_DEVICE_BUILTIN_MIC_MIC1_INVERSE  "builtin_Mic_Mic1_Inverse"
#define AUDIO_DEVICE_BUILTIN_MIC_MIC2_INVERSE  "builtin_Mic_Mic2_Inverse"
#define AUDIO_DEVICE_BUILTIN_MIC_MIC3_INVERSE  "builtin_Mic_Mic3_Inverse"


#define AUDIO_DEVICE_BUILTIN_SINGLE_MIC  "builtin_Mic_SingleMic"
#define AUDIO_DEVICE_BUILTIN_DUAL_MIC  "builtin_Mic_DualMic"
#define AUDIO_DEVICE_BUILTIN_TRIPLE_MIC  "builtin_Mic_TripleMic"
#define AUDIO_DEVICE_BUILTIN_BACK_MIC  "builtin_Mic_BackMic"
#define AUDIO_DEVICE_BUILTIN_BACK_MIC_INVERSE  "builtin_Mic_BackMic_Inverse"

#define AUDIO_DEVICE_HEADSET_MIC        "headset_mic_input"
#define AUDIO_DEVICE_SIDETONE        "sidetone_switch"
#define AUDIO_DEVICE_HEADSET_VOW_MIC     "headset_vow_input"


#define AUDIOMIC1_TYPE_ACCMODE                    "Mic1TypeACCMode"
#define AUDIOMIC1_TYPE_DCCMODE                    "Mic1TypeDCCMode"
#define AUDIOMIC1_TYPE_DMICMODE                  "Mic1TypeDMICMode"
#define AUDIOMIC1_TYPE_DCCECMDIFFMODE    "Mic1TypeDCCECMDIFFMode"
#define AUDIOMIC1_TYPE_DCCECMSINGLEMODE    "Mic1TypeDCCECMSINGLEMode"

#define AUDIOMIC2_TYPE_ACCMODE                       "Mic2TypeACCMode"
#define AUDIOMIC2_TYPE_DCCMODE                       "Mic2TypeDCCMode"
#define AUDIOMIC2_TYPE_DMICMODE                     "Mic2TypeDMICMode"
#define AUDIOMIC2_TYPE_DCCECMDIFFMODE       "Mic2TypeDCCECMDIFFMode"
#define AUDIOMIC2_TYPE_DCCECMSINGLEMODE    "Mic2TypeDCCECMSINGLEMode"

#define AUDIOMIC3_TYPE_ACCMODE                       "Mic3TypeACCMode"
#define AUDIOMIC3_TYPE_DCCMODE                       "Mic3TypeDCCMode"
#define AUDIOMIC3_TYPE_DMICMODE                     "Mic3TypeDMICMode"
#define AUDIOMIC3_TYPE_DCCECMDIFFMODE       "Mic3TypeDCCECMDIFFMode"
#define AUDIOMIC3_TYPE_DCCECMSINGLEMODE    "Mic3TypeDCCECMSINGLEMode"

#define AUDIOMIC4_TYPE_ACCMODE                       "Mic4TypeACCMode"
#define AUDIOMIC4_TYPE_DCCMODE                       "Mic4TypeDCCMode"
#define AUDIOMIC4_TYPE_DMICMODE                     "Mic4TypeDMICMode"
#define AUDIOMIC4_TYPE_DCCECMDIFFMODE       "Mic4TypeDCCECMDIFFMode"
#define AUDIOMIC4_TYPE_DCCECMSINGLEMODE    "Mic4TypeDCCECMSINGLEMode"

#define VOWMIC1_TYPE_ACCMODE                      "Mic1TypeVOWACCMode"
#define VOWMIC1_TYPE_DCCMODE                      "Mic1TypeVOWDCCMode"
#define VOWMIC1_TYPE_DMICMODE                    "Mic1TypeVOWDMICMode"
#define VOWMIC1_TYPE_DMICLPMODE                "Mic1TypeVOWDMICLPMode"
#define VOWMIC1_TYPE_DCCECMDIFFMODE        "Mic1TypeVOWDCCECMDIFFMode"
#define VOWMIC1_TYPE_DCCECMSINGLEMODE    "Mic1TypeVOWDCCECMSINGLEMode"

#define VOWMIC2_TYPE_ACCMODE                      "Mic2TypeVOWACCMode"
#define VOWMIC2_TYPE_DCCMODE                      "Mic2TypeVOWDCCMode"
#define VOWMIC2_TYPE_DCCECMDIFFMODE        "Mic2TypeVOWDCCECMDIFFMode"
#define VOWMIC2_TYPE_DCCECMSINGLEMODE    "Mic2TypeVOWDCCECMSINGLEMode"

#define VOWMIC3_TYPE_ACCMODE                      "Mic3TypeVOWACCMode"
#define VOWMIC3_TYPE_DCCMODE                      "Mic3TypeVOWDCCMode"
#define VOWMIC3_TYPE_DMICMODE                    "Mic3TypeVOWDMICMode"
#define VOWMIC3_TYPE_DMICLPMODE                "Mic3TypeVOWDMICLPMode"
#define VOWMIC3_TYPE_DCCECMDIFFMODE        "Mic3TypeVOWDCCECMDIFFMode"
#define VOWMIC3_TYPE_DCCECMSINGLEMODE    "Mic3TypeVOWDCCECMSINGLEMode"

#define AUDIO_MIC_INVERSE                                      "Mic_Setting_Inverse"
#define AUDIO_MIC_NOINVERSE                                 "Mic_Setting_NoInverse"

#if defined(MTK_AUDIO_KS)
// playback
#define AUDIO_CTL_PLAYBACK1 "PLAYBACK1_TO_"
#define AUDIO_CTL_PLAYBACK2 "PLAYBACK2_TO_"
#define AUDIO_CTL_PLAYBACK3 "PLAYBACK3_TO_"
#define AUDIO_CTL_PLAYBACK4 "PLAYBACK4_TO_"
#define AUDIO_CTL_PLAYBACK5 "PLAYBACK5_TO_"
#define AUDIO_CTL_PLAYBACK6 "PLAYBACK6_TO_"
#define AUDIO_CTL_PLAYBACK12 "PLAYBACK12_TO_"

#define AUDIO_CTL_PLAYBACK1_TO_MD1_CH4 "PLAYBACK1_TO_MD1_CH4"
#define AUDIO_CTL_PLAYBACK1_TO_MD2_CH4 "PLAYBACK1_TO_MD2_CH4"
#define AUDIO_CTL_PLAYBACK2_TO_MD1 "PLAYBACK2_TO_MD1"
#define AUDIO_CTL_PLAYBACK2_TO_MD2 "PLAYBACK2_TO_MD2"
// capture
#define AUDIO_CTL_ADDA_TO_CAPTURE1 "ADDA_TO_CAPTURE1"
#define AUDIO_CTL_ADDA_TO_CAPTURE1_4CH "ADDA_TO_CAPTURE1_4CH"
#define AUDIO_CTL_ADDA_TO_CAPTURE7 "ADDA_TO_CAPTURE7"
#define AUDIO_CTL_DL_MEMIF_TO_CAPTURE2 "DL_MEMIF_TO_CAPTURE2"
#define AUDIO_CTL_DL_MEMIF_TO_CAPTURE2_SPEAKER_HIFI3 "DL_MEMIF_TO_CAPTURE2_SPEAKER_HIFI3"
#define AUDIO_CTL_DL_PLAYBACK_TO_CAPTURE2_NON_SPEAKER_HIFI3 "DL_PLAYBACK_TO_CAPTURE2_NON_SPEAKER_HIFI3"
#define AUDIO_CTL_DL_MEMIF_TO_CAPTURE4 "DL_MEMIF_TO_CAPTURE4"
#define AUDIO_CTL_DL_MEMIF_TO_CAPTURE4_SPEAKER_HIFI3 "DL_MEMIF_TO_CAPTURE4_SPEAKER_HIFI3"
#define AUDIO_CTL_DL_MEMIF_TO_CAPTURE4_NON_SPEAKER_HIFI3 "DL_MEMIF_TO_CAPTURE4_NON_SPEAKER_HIFI3"
#define AUDIO_CTL_I2S0_TO_CAPTURE2 "I2S0_TO_CAPTURE2"
#define AUDIO_CTL_I2S2_TO_CAPTURE2 "I2S2_TO_CAPTURE2"
#define AUDIO_CTL_MD1_TO_CAPTURE2 "MD1_TO_CAPTURE2"
#define AUDIO_CTL_MD2_TO_CAPTURE2 "MD2_TO_CAPTURE2"
#define AUDIO_CTL_CONNSYS_TO_CAPTURE3 "CONNSYS_TO_CAPTURE3"
#define AUDIO_CTL_MD1_TO_CAPTURE_MONO_1 "MD1_TO_CAPTURE_MONO_1"
#define AUDIO_CTL_MD2_TO_CAPTURE_MONO_1 "MD2_TO_CAPTURE_MONO_1"
#define AUDIO_CTL_PLAYBACK1_TO_CAPTURE6 "PLAYBACK1_TO_CAPTURE6"
#define AUDIO_CTL_PLAYBACK2_TO_CAPTURE6 "PLAYBACK2_TO_CAPTURE6"
#define AUDIO_CTL_PLAYBACK12_TO_CAPTURE6 "PLAYBACK12_TO_CAPTURE6"
#define AUDIO_CTL_PLAYBACK3_TO_CAPTURE6 "PLAYBACK3_TO_CAPTURE6"
#define AUDIO_CTL_PLAYBACK4_TO_CAPTURE6 "PLAYBACK4_TO_CAPTURE6"
#define AUDIO_CTL_PLAYBACK6_TO_CAPTURE6 "PLAYBACK6_TO_CAPTURE6"
#define AUDIO_CTL_MD1_TO_CAPTURE6 "MD1_TO_CAPTURE6"
#define AUDIO_CTL_MD2_TO_CAPTURE6 "MD2_TO_CAPTURE6"
#define AUDIO_CTL_I2S_TO_CAPTURE4 "_TO_CAPTURE4"
#define AUDIO_CTL_TO_CAPTURE4 "_TO_CAPTURE4"
#define AUDIO_CTL_MIC_TO_CAPTURE8 "MIC_TO_CAPTURE8"
#define AUDIO_CTL_PLAYBACK8_TO_ADDA_DL "PLAYBACK8_TO_ADDA_DL"

// hostless
#define AUDIO_CTL_ADDA_UL "ADDA_UL_TO_"
#define AUDIO_CTL_CONNSYS "CONNSYS_TO_"
#define AUDIO_CTL_MRG "MRG_TO_"

// MD1
#define AUDIO_CTL_MD1 "MD1_TO_"
#define AUDIO_CTL_DL4_TO_MD1 "DL4_TO_MD1"

// MD2
#define AUDIO_CTL_MD2 "MD2_TO_"
#define AUDIO_CTL_DL4_TO_MD2 "DL4_TO_MD2"

// DSP DL
#define AUDIO_CTL_PLAYBACK1_TO_DSP "PLAYBACK1_TO_DSPDL"
#define AUDIO_CTL_PLAYBACK2_TO_DSP "PLAYBACK2_TO_DSPDL"
#define AUDIO_CTL_PLAYBACK3_TO_DSP "PLAYBACK3_TO_DSPDL"
#define AUDIO_CTL_PLAYBACK4_TO_DSP "PLAYBACK4_TO_DSPDL"
#define AUDIO_CTL_PLAYBACK6_TO_DSP "PLAYBACK6_TO_DSPDL"
#define AUDIO_CTL_PLAYBACK12_TO_DSP "PLAYBACK12_TO_DSPDL"
#define AUDIO_CTL_DSPDL "DSPDL"

#define AUDIO_CTL_ADDA_UL_TO_MD1 "ADDA_UL_TO_MD1"
#define AUDIO_CTL_ADDA_UL_TO_MD2 "ADDA_UL_TO_MD2"
#define AUDIO_CTL_HP_IMPEDANCE "HP_IMPEDANCE"
#define AUDIO_CTL_SPK_INIT "SPK_INIT"
#define AUDIO_CTL_MD1_ECHO_REF_I2S0_ON "MD1_ECHO_REF_I2S0_ON"
#define AUDIO_CTL_MD1_ECHO_REF_I2S0_OFF "MD1_ECHO_REF_I2S0_OFF"
#define AUDIO_CTL_MD1_ECHO_REF_I2S2_ON "MD1_ECHO_REF_I2S2_ON"
#define AUDIO_CTL_MD1_ECHO_REF_I2S2_OFF "MD1_ECHO_REF_I2S2_OFF"
#define AUDIO_CTL_MD2_ECHO_REF_I2S0_ON "MD2_ECHO_REF_I2S0_ON"
#define AUDIO_CTL_MD2_ECHO_REF_I2S0_OFF "MD2_ECHO_REF_I2S0_OFF"
#define AUDIO_CTL_MD2_ECHO_REF_I2S2_ON "MD2_ECHO_REF_I2S2_ON"
#define AUDIO_CTL_MD2_ECHO_REF_I2S2_OFF "MD2_ECHO_REF_I2S2_OFF"
#define AUDIO_CTL_VOW_BARGE_IN_ECHO "VOW_BARGE_IN_ECHO"
#define AUDIO_CTL_VOW_BARGE_IN_ECHO_SPEAKER_HIFI3 "VOW_BARGE_IN_ECHO_SPEAKER_HIFI3"
#define AUDIO_CTL_VOW_BARGE_IN_ECHO_DSP_SMARTPA "VOW_BARGE_IN_ECHO_DSP_SMARTPA"
// usb call
#define AUDIO_CTL_USB_ECHO_REF_DEBUG "USB_ECHO_REF_DEBUG"
#define AUDIO_CTL_USB_CALL_DEBUG_LOOPBACK "USB_CALL_DEBUG_LOOPBACK"
// i2s
#define AUDIO_CTL_I2S0_HD_ON "I2S0_HD_ON"
#define AUDIO_CTL_I2S0_HD_OFF "I2S0_HD_OFF"
#define AUDIO_CTL_I2S1_HD_ON "I2S1_HD_ON"
#define AUDIO_CTL_I2S1_HD_OFF "I2S1_HD_OFF"
#define AUDIO_CTL_I2S2_HD_ON "I2S2_HD_ON"
#define AUDIO_CTL_I2S2_HD_OFF "I2S2_HD_OFF"
#define AUDIO_CTL_I2S3_HD_ON "I2S3_HD_ON"
#define AUDIO_CTL_I2S3_HD_OFF "I2S3_HD_OFF"
#define AUDIO_CTL_I2S5_HD_ON "I2S5_HD_ON"
#define AUDIO_CTL_I2S5_HD_OFF "I2S5_HD_OFF"

#if !defined(AUDIO_CTL_ADSP_UL)
#define AUDIO_CTL_ADSP_UL "CAPTURE6"
#endif

// device
#define AUDIO_CTL_DEVICE_PRE_PMU "_DEVICE_CTRL_PRE_PMU"
#define AUDIO_CTL_CUST_RCV_OUT "RCV_OUT"
#define AUDIO_CTL_CUST_HP_OUT "HP_OUT"
#define AUDIO_CTL_CUST_SPK_OUT "SPK_OUT"
#define AUDIO_CTL_DEFAULT_DEVICE_OUT "ADDA_DL"

#define AUDIO_CTL_BUILT_IN_MIC "builtin_Mic"
#define AUDIO_CTL_HEADSET_MIC "headset_mic_input"
#define AUDIO_CTL_SINGLE_MIC "SingleMic"
#define AUDIO_CTL_DUAL_MIC "DualMic"
#define AUDIO_CTL_THREE_MIC "ThreeMic"
#define AUDIO_CTL_MIC1 "Mic1"
#define AUDIO_CTL_MIC2 "Mic2"
#define AUDIO_CTL_MIC3 "Mic3"
#define AUDIO_CTL_BACK_MIC "BackMic"
#define AUDIO_CTL_BACK_MIC_3 "BackMic_3"

#define AUDIO_CTL_MIC_INVERSE "Inverse"

// reset
#define AUDIO_CTL_RESET_DEVICE "reset_device_setting"
#else
// not MTK_AUDIO_KS
#define AUDIO_CTL_ADSP_UL ""
#define AUDIO_CTL_DSPDL ""
#endif

namespace android {

class DeviceCtlDescriptor {
public:
    DeviceCtlDescriptor();
    String8 mDevicename;
    Vector<String8> mDeviceCltonVector;
    Vector<String8> mDeviceCltoffVector;
    Vector<String8> mDeviceCltsettingVector;
    int DeviceStatusCounter;
};

class DeviceCtlControlSeq {
public:
    Vector<String8> mDeviceCltNameVector;
    Vector<String8> mDeviceCltValueVector;
};

class AudioALSADeviceConfigManager {
public:
    static AudioALSADeviceConfigManager *getInstance();

    /**
      * LoadAudioConfig file
      */
    status_t LoadAudioConfig(const char *path);

    /**
      * config file related function
      */
    bool  SupportConfigFile(void);


    /**
      *  check device path exist
      */
    bool CheckDeviceExist(const char *path);
    String8 CheckCustDevPathExist(const char *devicename);

    /**
      * dump for all config file content
      */
    void dump(void);

    /**
    * set mixer ctl by type
    */
    int setMixerByteCtl(struct mixer_ctl *ctl, char **values, unsigned int numValues);
    int setMixerCtlValue(String8 cltName, String8 cltNalue);
    int setMixerCtl(String8 cltName, String8 cltNalue);

    /**
    * apply turn on / off sequence by string
    */
    status_t ApplyDeviceTurnonSequenceByName(const char *DeviceName);
    status_t ApplyDeviceTurnoffSequenceByName(const char *DeviceName);

    /**
    * apply setting sequence
    */
    status_t ApplyDeviceSettingByName(const char *DeviceName) ;
private:
    static AudioALSADeviceConfigManager *UniqueAlsaDeviceConfigParserInstance;
    AudioALSADeviceConfigManager();

    /**
      *  load config file
      */
    status_t GetVersion(TiXmlElement *root);
    status_t ParseInitSequence(TiXmlElement *root);
    String8 ParseDeviceSequence(TiXmlElement *root);
    int parseCustDevicePathSeq(TiXmlElement *root);

    /**
      *     query for device
      */
    DeviceCtlDescriptor *GetDeviceDescriptorbyname(const char *devicename);

    Vector<DeviceCtlDescriptor *> mDeviceVector;
    DeviceCtlControlSeq mDeviceCtlSeq;
    DeviceCtlControlSeq mDevCustPathSeq;
    String8 VersionControl;
    bool mConfigsupport;
    bool mInit;

    /**
     * mixer controller
     */
    struct mixer *mMixer;
    /*
     * flag of dynamic enable verbose/debug log
     */
    int mLogEnable;
};

}

#endif
