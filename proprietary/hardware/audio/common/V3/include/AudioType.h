#ifndef __AUDIO_TYPE_H__
#define __AUDIO_TYPE_H__

#include <stdint.h>
#include <sys/types.h>

#include <utils/Errors.h>
#include <utils/Vector.h>
#include <utils/String16.h>
#include <utils/String8.h>

#include <hardware/audio_effect.h>

#include "AudioAssert.h"


#ifndef int32
typedef signed int          int32;
#endif

#ifndef uint32
typedef unsigned int        uint32;
#endif

using android::status_t;

#define VM_FILE_NAME_LEN_MAX (128)
#define SCENE_NAME_LEN_MAX (64)

#define MAX_PREPROCESSORS 3 /* maximum one AGC + one NS + one AEC per input stream */
#ifndef UPLINK_LOW_LATENCY  //no need to drop data
#define CAPTURE_DROP_MS (120)    //drop 120ms record data in carture data normal provider due to hardware pulse
#else
#define CAPTURE_DROP_MS (0)    //drop 120ms record data in carture data normal provider due to hardware pulse
#endif
#define UPLINK_NORMAL_LATENCY_MS (20)

#ifdef UPLINK_LOW_LATENCY_CHIP_MS
#define UPLINK_LOW_LATENCY_MS (UPLINK_LOW_LATENCY_CHIP_MS)   // audio low latency param - record - interrupt rate
#else
#define UPLINK_LOW_LATENCY_MS (5)   // audio low latency param - record - interrupt rate
#endif

#define MMAP_DL_PERIOD_SIZE 96
#define MMAP_UL_PERIOD_SIZE 96

#define MIN_MMAP_PERIOD_COUNT    4
#define MAX_MMAP_FRAME_COUNT    4096     // max hw buffer size of share mode
#define MAX_MMAP_HW_BUFFER_SIZE  8192    // max hw buffer size of exclusive mode


#ifdef AUDIO_UTIL_PULSE_LEVEL
#define PULSE_LEVEL (AUDIO_UTIL_PULSE_LEVEL)
#else
#define PULSE_LEVEL (800)
#endif

// when call I2S start , need parameters for I2STYPE
typedef enum {
    MATV,                         //I2S Input For ATV
    FMRX,                         //I2S Input For FMRX
    FMRX_32K,                 //I2S Input For FMRX_32K
    FMRX_48K,                 //I2S Input For FMRX_48K
    I2S0OUTPUT,             //   I2S0 output
    I2S1OUTPUT,             //   I2S1 output
    HOA_SAMPLERATE,   //   use for HQA support
    NUM_OF_I2S
} I2STYPE;

#define AUDIO_LOCK_TIMEOUT_VALUE_MS (5000)  //The same with ANR

#if 1 //HP switch
//#define HIFIDAC_SWITCH
//#define SWITCH_BEFORE_HPAMP
//#define HIFI_SWITCH_BY_AUDENH

//#define EXTDAC_PMIC_MUTE
//#define RINGTONE_USE_PMIC
#endif

// TODO(Harvey): move it to somewhere else
/**
 * Playback handler types
 */
enum playback_handler_t {
    PLAYBACK_HANDLER_BASE = -1,
    PLAYBACK_HANDLER_NORMAL,
    PLAYBACK_HANDLER_BT_SCO,
    PLAYBACK_HANDLER_FM_TX,
    PLAYBACK_HANDLER_HDMI,
    PLAYBACK_HANDLER_VOICE,
    PLAYBACK_HANDLER_BT_CVSD,
    PLAYBACK_HANDLER_OFFLOAD,  //doug
    PLAYBACK_HANDLER_FAST,
    PLAYBACK_HANDLER_SPEAKERPROTECTION,
    PLAYBACK_HANDLER_USB,
    PLAYBACK_HANDLER_DEEP_BUFFER,
    PLAYBACK_HANDLER_MIXER,
    PLAYBACK_HANDLER_VOIP,
    PLAYBACK_HANDLER_MAX,
};

/**
 * Capture handler types
 */
enum capture_handler_t {
    CAPTURE_HANDLER_BASE      = (1 << 0),
    CAPTURE_HANDLER_NORMAL    = (1 << 1),
    CAPTURE_HANDLER_VOICE     = (1 << 2),
    CAPTURE_HANDLER_FM_RADIO  = (1 << 3),
    CAPTURE_HANDLER_SPK_FEED  = (1 << 4),
    CAPTURE_HANDLER_ANC       = (1 << 5),
    CAPTURE_HANDLER_BT        = (1 << 6),
    CAPTURE_HANDLER_AEC       = (1 << 7),
    CAPTURE_HANDLER_TDM       = (1 << 8),
    CAPTURE_HANDLER_MODEM_DAI = (1 << 9),
    CAPTURE_HANDLER_SYNCIO    = (1 << 10),
    CAPTURE_HANDLER_DSP       = (1 << 11),
    CAPTURE_HANDLER_ALL       = 0xFFFFFFFF,
};


/**
 * Capture Data Provider types
 */
enum capture_provider_t {
    CAPTURE_PROVIDER_BASE = -1,
    CAPTURE_PROVIDER_NORMAL,
    CAPTURE_PROVIDER_VOICE,
    CAPTURE_PROVIDER_FM_RADIO,
    CAPTURE_PROVIDER_SPK_FEED,
    CAPTURE_PROVIDER_ANC,
    CAPTURE_PROVIDER_ECHOREF,
    CAPTURE_PROVIDER_ECHOREF_BTSCO,
    CAPTURE_PROVIDER_ECHOREF_BTCVSD,
    CAPTURE_PROVIDER_ECHOREF_EXT,
    CAPTURE_PROVIDER_TDM_RECORD,
    CAPTURE_PROVIDER_EXTERNAL,
    CAPTURE_PROVIDER_BT_SCO,
    CAPTURE_PROVIDER_BT_CVSD,
    CAPTURE_PROVIDER_VOW,
    CAPTURE_PROVIDER_AP_VOICE_DL,
    CAPTURE_PROVIDER_USB_BASE,
    CAPTURE_PROVIDER_USB,
    CAPTURE_PROVIDER_ECHOREF_USB,
    CAPTURE_PROVIDER_DSP,
    CAPTURE_PROVIDER_MAX
};

/**
 * speech enhancement function dynamic mask
 * This is the dynamic switch to decided the enhancment output.
 */
enum voip_sph_enh_dynamic_mask_t {
    VOIP_SPH_ENH_DYNAMIC_MASK_DMNR      = (1 << 0), // for receiver
    VOIP_SPH_ENH_DYNAMIC_MASK_VCE       = (1 << 1),
    VOIP_SPH_ENH_DYNAMIC_MASK_BWE       = (1 << 2),
    VOIP_SPH_ENH_DYNAMIC_MASK_LSPK_DMNR = (1 << 5), // for loud speaker
    VOIP_SPH_ENH_DYNAMIC_MASK_ALL       = 0xFFFFFFFF
};

enum {
    VOIP_RX_TYPE_MIX,
    VOIP_RX_TYPE_REPLACE
};

typedef struct {
    uint32_t dynamic_func; // DMNR,VCE,BWE,
} voip_sph_enh_mask_struct_t;

typedef struct {
    bool    besrecord_enable;
    int32_t besrecord_scene;
    bool    besrecord_voip_enable;
    //for VoIP dymanic mask
    voip_sph_enh_mask_struct_t besrecord_dynamic_mask;
    //for besrecord tuning
    bool    besrecord_tuningEnable;
    char    besrecord_VMFileName[VM_FILE_NAME_LEN_MAX];
    bool    besrecord_tuning16K;
    //for engineer mode
    bool    besrecord_ForceMagiASREnable;
    bool    besrecord_ForceAECRecEnable;
    //for AP DMNR tunning
    bool    besrecord_dmnr_tuningEnable;
    int32_t besrecord_dmnr_tuningMode;
    //for temp using
    bool    besrecord_bypass_dualmicprocess;
} besrecord_info_struct_t;

typedef struct {
    bool    PreProcessEffect_Update;
    int PreProcessEffect_Count;
    bool PreProcessEffect_AECOn;
    bool PreProcessEffect_NSOn;
    bool PreProcessEffect_AGCOn;
    effect_handle_t PreProcessEffect_Record[MAX_PREPROCESSORS];

} native_preprocess_info_struct_t;


typedef struct {
    timespec timestamp_get;
    unsigned int frameInfo_get; //for input: remains data(frame) in kernel buffer.  for output: empty data(frame) in kernel buffer size
    unsigned int buffer_per_time; //for input: frames read per read. For output: kernel buffer size(frame)
    unsigned int total_frames_readed; //for input: total frames readed
    unsigned long kernelbuffer_ns;   //calculated kernel buffer time
    int halQueuedFrame;
} time_info_struct_t;

struct afe_offload_codec_t {
    unsigned int codec_samplerate;
    unsigned int codec_bitrate;
    unsigned int disable_codec;
};

struct stream_attribute_t {
    audio_format_t       audio_format;
    int       audio_offload_format;
    audio_channel_mask_t audio_channel_mask;
    union {
        audio_output_flags_t mAudioOutputFlags;
        audio_input_flags_t mAudioInputFlags;
    };

    afe_offload_codec_t offload_codec_info;

    audio_devices_t      output_devices;

    audio_devices_t      input_device;
    audio_source_t       input_source;

    uint32_t             num_channels;
    uint32_t             sample_rate;

    uint32_t             buffer_size;
    size_t               frame_count;
    uint32_t             latency;
    uint32_t             interrupt_samples;

    audio_in_acoustics_t acoustics_mask;

    bool                 digital_mic_flag;

    bool                 bBypassPostProcessDL;
    bool                 micmute;
#if defined(MTK_SPEAKER_MONITOR_SPEECH_SUPPORT)
    bool                 bFixedRouting;
    bool                 bModemDai_Input;
#endif
    audio_mode_t         audio_mode;
    audio_stream_type_t  stream_type;

    uint32_t             mStreamOutIndex;  // AudioALSAStreamOut pass to AudioALSAStreamManager

    char mCustScene[SCENE_NAME_LEN_MAX];
    besrecord_info_struct_t BesRecord_Info;

    native_preprocess_info_struct_t NativePreprocess_Info;
    time_info_struct_t Time_Info;

    double mInterrupt;
    bool mVoIPEnable;

    // sync device info with audio policy
    bool usePolicyDevice;
    audio_devices_t policyDevice;

    bool mPowerHalEnable;   // for PowerHAL debug

    bool isMixerOut;

    unsigned int dspLatency;
};


enum sgen_mode_t {
    SGEN_MODE_I00_I01           = 0,
    SGEN_MODE_I02               = 1,
    SGEN_MODE_I03_I04           = 2,
    SGEN_MODE_I05_I06           = 3,
    SGEN_MODE_I07_I08           = 4,
    SGEN_MODE_I09               = 5,
    SGEN_MODE_I10_I11           = 6,
    SGEN_MODE_I12_I13           = 7,
    SGEN_MODE_I14               = 8,
    SGEN_MODE_I15_I16           = 9,
    SGEN_MODE_I17_I18           = 10,
    SGEN_MODE_I19_I20           = 11,
    SGEN_MODE_I21_I22           = 12,
    SGEN_MODE_O00_O01           = 13,
    SGEN_MODE_O02               = 14,
    SGEN_MODE_O03_O04           = 15,
    SGEN_MODE_O05_O06           = 16,
    SGEN_MODE_O07_O08           = 17,
    SGEN_MODE_O09_O10           = 18,
    SGEN_MODE_O11               = 19,
    SGEN_MODE_O12               = 20,
    SGEN_MODE_O13_O14           = 21,
    SGEN_MODE_O15_O16           = 22,
    SGEN_MODE_O17_O18           = 23,
    SGEN_MODE_O19_O20           = 24,
    SGEN_MODE_O21_O22           = 25,
    SGEN_MODE_O23_O24           = 26,
    SGEN_MODE_DISABLE           = 27,
    SGEN_MODE_O03               = 28,   // sgen to o3 o4, but mute o4
    SGEN_MODE_O04               = 29,   // sgen to o3 o4, but mute o3
    SGEN_MODE_I25_I26           = 30,
    SGEN_MODE_O25               = 31,
    SGEN_MODE_O28_O29           = 32,
    SGEN_MODE_I23_I24           = 33,
    SGEN_MODE_O32_O33           = 34,
    SGEN_MODE_I34_I35           = 35
};

enum sgen_mode_samplerate_t {
    SGEN_MODE_SAMPLERATE_8000HZ  = 0,
    SGEN_MODE_SAMPLERATE_11025HZ = 1,
    SGEN_MODE_SAMPLERATE_12000HZ = 2,
    SGEN_MODE_SAMPLERATE_16000HZ = 3,
    SGEN_MODE_SAMPLERATE_22050HZ = 4,
    SGEN_MODE_SAMPLERATE_24000HZ = 5,
    SGEN_MODE_SAMPLERATE_32000HZ = 6,
    SGEN_MODE_SAMPLERATE_44100HZ = 7,
    SGEN_MODE_SAMPLERATE_48000HZ = 8
};

typedef enum {
    AUDIO_MIC_MODE_ACC = 1,
    AUDIO_MIC_MODE_DCC,
    AUDIO_MIC_MODE_DMIC,
    AUDIO_MIC_MODE_DMIC_LP,
    AUDIO_MIC_MODE_DCCECMDIFF,
    AUDIO_MIC_MODE_DCCECMSINGLE,
    AUDIO_MIC_MODE_DMIC_VENDOR01
} AUDIO_MIC_MODE;

#endif
