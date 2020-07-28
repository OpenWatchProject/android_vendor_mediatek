#ifndef ANDROID_SPEECH_TYPE_H
#define ANDROID_SPEECH_TYPE_H

#include <stdint.h>
#include <stdbool.h>
#include "SpeechFeatureDef.h"

namespace android {

#ifdef SLOGV
#undef SLOGV
#endif

#if 0 // for speech debug usage
#define SLOGV(x...) ALOGD(x)
#else
#define SLOGV(x...) ALOGV(x)
#endif



enum modem_index_t {
    MODEM_1   = 0,
    MODEM_2   = 1,
    MODEM_EXTERNAL   = 2,
    NUM_MODEM
};

enum phone_id_t {
    PHONE_ID_INVALID   = -1,
    PHONE_ID_0   = 0,
    PHONE_ID_1   = 1,
    NUM_PHONE_ID
};

/** CCCI modem status */
typedef uint8_t modem_status_t;

enum { /* modem_status_t */
    MODEM_STATUS_INVALID = 0, // Boot stage 0 -> Means MD Does NOT run
    MODEM_STATUS_INIT    = 1, // Boot stage 1 -> Means MD begin to run, but not ready
    MODEM_STATUS_READY   = 2, // Boot stage 2 -> Means MD is ready
    MODEM_STATUS_EXPT    = 3  // MD exception -> Means EE occur
};

enum modem_status_mask_t {
    SPEECH_STATUS_MASK          = (1 << 0),
    RECORD_STATUS_MASK          = (1 << 1),
    BGS_STATUS_MASK             = (1 << 2),
    P2W_STATUS_MASK             = (1 << 3),
    TTY_STATUS_MASK             = (1 << 4),
    VT_STATUS_MASK              = (1 << 5),
    LOOPBACK_STATUS_MASK        = (1 << 6),
    VM_RECORD_STATUS_MASK       = (1 << 7),
    SPEECH_ROUTER_STATUS_MASK   = (1 << 8),
    RAW_RECORD_STATUS_MASK      = (1 << 9),
    PCM_MIXER_STATUS_MASK       = (1 << 10),
    TELEPHONY_TX_STATUS_MASK    = (1 << 11),
};

enum speech_mode_t {
    SPEECH_MODE_NORMAL          = 0,
    SPEECH_MODE_EARPHONE        = 1,
    SPEECH_MODE_LOUD_SPEAKER    = 2,
    SPEECH_MODE_BT_EARPHONE     = 3,
    SPEECH_MODE_BT_CORDLESS     = 4,
    SPEECH_MODE_BT_CARKIT       = 5,
    SPEECH_MODE_MAGIC_CON_CALL  = 6,
    SPEECH_MODE_PRESERVED_2     = 7,
    SPEECH_MODE_HAC             = 8,
    SPEECH_MODE_USB_AUDIO       = 9,
    SPEECH_MODE_NO_CONNECT      = 10
};

enum phone_call_mode_t {
    RAT_2G_MODE     = 0, // 2G phone call
    RAT_3G_MODE     = 1, // 3G phone call // for both 2G/3G phone call, set mode as 2G. Modem side can query 2G/3G phone call.
    RAT_3G324M_MODE = 2, // VT phone call
};

enum rf_mode_t {
    RF_2G_MODE     = (0xF << 0),  // 2G RF index
    RF_3G_MODE     = (0xF << 4),  // 3G RF index
    RF_4G_MODE     = (0xF << 8),  // 4G RF index
    RF_5G_MODE     = (0xF << 12),  // 5G RF index
};

enum phone_call_network_t {
    NETWORK_GSM       = 0,
    NETWORK_WCDMA_TDD = 1,
    NETWORK_WCDMA_FDD = 2,
    NETWORK_VOLTE     = 3,
    NETWORK_C2K       = 4,
};

enum ctm_interface_t { // L1 CTM Interface
    DIRECT_MODE = 0,
    BAUDOT_MODE = 1
};


enum record_format_t {
    RECORD_FORMAT_PCM         = 0,
    RECORD_FORMAT_VM          = 1,
    RECORD_FORMAT_DUAL_MIC_VM = 2,
    RECORD_FORMAT_CTM_4WAY    = 3,
};

enum record_sample_rate_t {
    RECORD_SAMPLE_RATE_08K = 0,
    RECORD_SAMPLE_RATE_16K = 1,
    RECORD_SAMPLE_RATE_32K = 2,
    RECORD_SAMPLE_RATE_48K = 3
};

enum record_channel_t {
    RECORD_CHANNEL_MONO = 0,
    RECORD_CHANNEL_STEREO = 1
};

enum RecordType {
    RECORD_TYPE_UL = 0,
    RECORD_TYPE_DL = 1,
    RECORD_TYPE_MIX = 2,
    RECORD_TYPE_MAX
};

// downlink record position in dsp, default 0
enum RecordPositionDlType {
    RECORD_POS_DL_END = 0,
    RECORD_POS_DL_AFTER_ENH = 1,
    RECORD_POS_DL_HEAD = 2,
    RECORD_POS_DL_MAX
};

struct SpcRecordTypeStruct {
    RecordType direction;// record type
    RecordPositionDlType dlPosition;// dl record position
};

// define for dual mic pcm2way format
enum dualmic_pcm2way_format_t {
    P2W_FORMAT_NORMAL = 0,
    P2W_FORMAT_VOIP   = 1,
    P2W_FORMAT_NB_CAL = 2, // NB calibration
    P2W_FORMAT_WB_CAL = 3, // WB calibration
};

enum pcmnway_format_t {
    SPC_PNW_MSG_BUFFER_SE  = (1 << 0), // Bit 0, PCM4WAY_PutToSE
    SPC_PNW_MSG_BUFFER_SPK = (1 << 1), // Bit 1, PCM4WAY_PutToSpk
    SPC_PNW_MSG_BUFFER_MIC = (1 << 2), // Bit 2, PCM4WAY_GetFromMic
    SPC_PNW_MSG_BUFFER_SD  = (1 << 3), // Bit 3, PCM4WAY_GetFromSD
};

// speech enhancement function mask(before 92 modem)
// This is the power on/off setting of enhancement. Most of the case, it should be totally on.
enum sph_enh_main_mask_t {
    //SPH_ENH_MAIN_MASK_ES  = (1 << 0),
    SPH_ENH_MAIN_MASK_AEC   = (1 << 1),
    SPH_ENH_MAIN_MASK_EES   = (1 << 2),
    SPH_ENH_MAIN_MASK_ULNR  = (1 << 3), // VCE depends on this
    SPH_ENH_MAIN_MASK_DLNR  = (1 << 4), // VCE depends on this
    SPH_ENH_MAIN_MASK_TDNC  = (1 << 5),
    SPH_ENH_MAIN_MASK_DMNR  = (1 << 6), // Enable only when phone with dual mic
    SPH_ENH_MAIN_MASK_AGC   = (1 << 7), //SPH_ENH_MAIN_MASK_SIDETONE = (1 << 7),
    SPH_ENH_MAIN_MASK_ALL   = 0xFFFF
};

// speech enhancement function dynamic mask(before 92 modem)
// This is the dynamic switch to decided the enhancment output.
enum sph_enh_dynamic_mask_t {
    SPH_ENH_DYNAMIC_MASK_DMNR           = (1 << 0), // for receiver
    SPH_ENH_DYNAMIC_MASK_VCE            = (1 << 1),
    SPH_ENH_DYNAMIC_MASK_BWE            = (1 << 2),
    SPH_ENH_DYNAMIC_MASK_DLNR           = (1 << 3), // ==> SAL_ENH_DYNAMIC_DLNR_MUX, bit 4
    SPH_ENH_DYNAMIC_MASK_ULNR           = (1 << 4), // ==> SAL_ENH_DYNAMIC_DLNR_MUX, bit 5
    SPH_ENH_DYNAMIC_MASK_LSPK_DMNR      = (1 << 5), // for loud SPEAKER_AMP
    SPH_ENH_DYNAMIC_MASK_SIDEKEY_DGAIN  = (1 << 6), // ==> SAL_ENH_DYNAMIC_SIDEKEYCTRL_DGAIN_MUX, bit 7
    SPH_ENH_DYNAMIC_MASK_DLNR_INIT_CTRL = (1 << 7), // ==> SAL_ENH_DYNAMIC_DL_NR_INIT_CTRL_MUX, bit 8
    SPH_ENH_DYNAMIC_MASK_AEC            = (1 << 8), // ==> SAL_ENH_DYNAMIC_AEC_MUX, bit 9
    SPH_ENH_DYNAMIC_MASK_ALL            = 0xFFFFFFFF
};

typedef struct {
    uint16_t main_func;    // ES,AEC,EES,ULNR,DLNR,TDNC,DMNR,SIDETONE, ...
    uint32_t dynamic_func; // DMNR,VCE,BWE,
} sph_enh_mask_struct_t;



// speech enhancement function dynamic mask(after 93 modem)
enum { /* sph_enh_dynamic_ctrl_t */
    SPH_ENH_DYNAMIC_CTRL_MASK_DMNR              = (1 << 0),
    SPH_ENH_DYNAMIC_CTRL_MASK_TDNC              = (1 << 1),
    SPH_ENH_DYNAMIC_CTRL_MASK_MAGIC_CONFERENCE  = (1 << 2),

    /* max 16 bit !! */
    SPH_ENH_DYNAMIC_CTRL_MASK_MAX               = (1 << 15)
};

typedef uint16_t sph_enh_dynamic_ctrl_t;



typedef struct spcRAWPCMBufInfoStruct {
    //UL sample rate, please refer PCM_REC_SAMPLE_RATE_IDX
    uint16_t u16ULFreq;
    //UL length in byte
    uint16_t u16ULLength;
    //DL sample rate, please refer PCM_REC_SAMPLE_RATE_IDX
    uint16_t u16DLFreq;
    //DL length in byte
    uint16_t u16DLLength;
} spcRAWPCMBufInfo;

typedef struct spcApRAWPCMBufHdrStruct {
    uint16_t u16SyncWord;
    uint16_t u16RawPcmDir;
    uint16_t u16Freq;
    uint16_t u16Length;
    uint16_t u16Channel;
    uint16_t u16BitFormat;
} spcApRAWPCMBufHdr;

#define SPC_PROP_CODEC_LEN 92

typedef struct {
    char codecInfo[SPC_PROP_CODEC_LEN];
    char codecOp[SPC_PROP_CODEC_LEN];
} spcCodecInfoStruct;

enum share_buff_data_type_t {
    SHARE_BUFF_DATA_TYPE_PCM_FillSE                     = 0,
    SHARE_BUFF_DATA_TYPE_PCM_FillSpk                    = 1,
    SHARE_BUFF_DATA_TYPE_PCM_GetFromMic                 = 2,
    SHARE_BUFF_DATA_TYPE_PCM_GetfromSD                  = 3,
    SHARE_BUFF_DATA_TYPE_CCCI_VM_TYPE                   = 4,
    SHARE_BUFF_DATA_TYPE_CCCI_PCM_TYPE                  = 5,
    SHARE_BUFF_DATA_TYPE_CCCI_BGS_TYPE                  = 6,
    SHARE_BUFF_DATA_TYPE_CCCI_EM_PARAM                  = 7,
    SHARE_BUFF_DATA_TYPE_CCCI_CTM_UL_IN                 = 8,
    SHARE_BUFF_DATA_TYPE_CCCI_CTM_DL_IN                 = 9,
    SHARE_BUFF_DATA_TYPE_CCCI_CTM_UL_OUT                = 10,
    SHARE_BUFF_DATA_TYPE_CCCI_CTM_DL_OUT                = 11,
    SHARE_BUFF_DATA_TYPE_CCCI_VIBSPK_PARAM              = 12,

    SHARE_BUFF_DATA_TYPE_CCCI_SMARTPA_PARAM             = 15,
    SHARE_BUFF_DATA_TYPE_CCCI_MAGICON_PARAM             = 16,
    SHARE_BUFF_DATA_TYPE_CCCI_HAC_PARAM                 = 17,
    SHARE_BUFF_DATA_TYPE_CCCI_RAW_PCM_TYPE              = 18,
    SHARE_BUFF_DATA_TYPE_CCCI_DYNAMIC_PARAM_TYPE        = 19,
    SHARE_BUFF_DATA_TYPE_CCCI_NW_CODEC_INFO             = 20,
    SHARE_BUFF_DATA_TYPE_CCCI_STRMBUF_TYPE_CUST_DUMP    = 21,
    SHARE_BUFF_DATA_TYPE_CCCI_UL_ENC                    = 22,
    SHARE_BUFF_DATA_TYPE_CCCI_DL_DEC                    = 23,
    SHARE_BUFF_DATA_TYPE_CCCI_ECALL_MSD_TYPE            = 24,
    SHARE_BUFF_DATA_TYPE_CCCI_SPH_INFO                  = 25,
    SHARE_BUFF_DATA_TYPE_CCCI_MD_ALIVE_INFO             = 26,
    SHARE_BUFF_DATA_TYPE_CCCI_VOIP_RX                   = 27,
    SHARE_BUFF_DATA_TYPE_CCCI_TELEPHONY_TX              = 28,
    SHARE_BUFF_DATA_TYPE_CCCI_MAX_TYPE
};


typedef struct _AUDIO_CUSTOM_HAC_SPEECH_PARAM_STRUCT {
    /* speech enhancement */
    uint16_t speech_hac_mode_nb_para[16];
    /* WB speech enhancement */
    uint16_t speech_hac_mode_wb_para[16];
    /* speech input FIR */
    int16_t sph_hac_in_fir[45];
    /* speech output FIR */
    int16_t sph_hac_out_fir[45];

    /* WB speech input FIR */
    int16_t sph_hac_wb_in_fir[90];
    /* WB speech output FIR */
    int16_t sph_hac_wb_out_fir[90];
    /* mic volume setting */

} AUDIO_CUSTOM_HAC_SPEECH_PARAM_STRUCT;


/*
 * =============================================================================
 *                     speech on/routing/tuning payload
 * =============================================================================
 */

enum {
    SPH_APPLICATION_NORMAL,
    SPH_APPLICATION_VT_CALL,
    SPH_APPLICATION_LOOPBACK,
    SPH_APPLICATION_ROUTER,
    SPH_APPLICATION_DACA,
    SPH_APPLICATION_HOLD_CALL,
    SPH_APPLICATION_ENCRYPTION,

    SPH_APPLICATION_INVALID = 0xFF
};

typedef uint8_t sph_application_t;


enum {
    SPH_BT_OFF,
    SPH_BT_PCM,         /* via Modem PCM hardware */
    SPH_BT_CVSD_MSBC,   /* not use now, keep for MD legacy */
    SPH_BT_CVSD,        /* via software BT CVSD codec */
    SPH_BT_MSBC         /* via software BT MSBC codec */
};

typedef uint8_t sph_bt_info_t;


enum {
    SPH_SAMPLE_RATE_08K,
    SPH_SAMPLE_RATE_16K,
    SPH_SAMPLE_RATE_32K,
    SPH_SAMPLE_RATE_48K
};

typedef uint8_t sph_sample_rate_t;


enum {
    SPH_PARAM_VIA_PAYLOAD,       /* speech param is written in payload (0x2F79) */
    SPH_PARAM_VIA_SHM_CCCI,      /* speech param is written in share memory from ccci */
    SPH_PARAM_VIA_SHM_USIP       /* speech param is written in share memory for ap/md/usip accessed */
};

typedef uint8_t sph_param_path_t;


enum {
    SPH_EXT_DEV_INFO_DEFULAT = 0,
    SPH_EXT_DEV_INFO_VIBRATION_RECEIVER,        /* vibration receiver (3-in-1) */
    SPH_EXT_DEV_INFO_VIBRATION_SPEAKER,         /* vibration speaker (2-in-1 or 3-in-1) */
    SPH_EXT_DEV_INFO_SMARTPA_SPEAKER,           /* speaker with smart-PA which need echo reference path */
    SPH_EXT_DEV_INFO_SMARTPA_VIBRATION_SPEAKER, /* vibration speaker with smart-PA which need echo reference path */
    SPH_EXT_DEV_INFO_USB_AUDIO,                 /* usb audio device, which needs echo reference path */
    SPH_EXT_DEV_INFO_EARPHONE,                  /* earphone audio device */
    SPH_EXT_DEV_INFO_DUALSMARTPA_SPEAKER,        /* speaker with 2 smart-PA which need echo reference path */
    SPH_EXT_DEV_INFO_DUALSMARTPA_VIBRATION_SPEAKER, /* vibration speaker with 2smart-PA which need echo reference path */
};

typedef uint16_t sph_ext_dev_info_t;


enum {
    SPH_LOOPBACK_INFO_FLAG_DISABLE_BT_CODEC = (1 << 0), /* 0'b == BT Loopback with BT codec / 1'b == BT Loopback without BT Codec, only support when SPH_BTINFO_BT_CVSD_MSBC */
    SPH_LOOPBACK_INFO_FLAG_DELAY_SETTING    = (1 << 1), /* delay setting for normal loopback, i.e. speech mode is not BT cases. 0==Use modem default delay value/ 1== use AP gives delay value in loopback_delay */
};

typedef uint8_t sph_loopback_info_flag_t;

enum {
    SPH_PARAM_INVALID = 0,
    SPH_PARAM_VALID,
    SPH_PARAM_PREVIOUS_VALID
};

enum {
    SPH_ON_MUTE_BIT_UL = (1 << 0),
    SPH_ON_MUTE_BIT_DL = (1 << 1),
    SPH_ON_MUTE_BIT_UL_ENH_RESULT = (1 << 2),
    SPH_ON_MUTE_BIT_UL_SOURCE = (1 << 3),
    SPH_ON_MUTE_BIT_DL_CODEC = (1 << 4)
};


typedef struct sph_info_t {
    uint8_t  application;             /* sph_application_t */
    uint8_t  bt_info;                 /* sph_bt_info_t */
    uint8_t  sample_rate_enum;        /* sph_sample_rate_t */
    uint8_t  opendsp_flag;            /* 0: internal SPE, 1: OpenDSP */

    uint8_t  sph_param_path;          /* sph_param_path_t */
    uint8_t  sph_param_valid;         /* SPH_PARAM_VIA_SHM only. 0: invalid, 1: valid, 2: previous valid */
    uint16_t sph_param_length;        /* Max 65535 bytes */
    uint16_t sph_param_index;         /* Max 65535 bytes */

    uint16_t ext_dev_info;            /* sph_ext_dev_info_t */

    uint8_t  loopback_flag;           /* sph_loopback_info_flag_t */
    uint8_t  loopback_delay;          /* unit is 20ms */

    uint16_t echo_ref_delay_ms;       /* echo reference data delay, unit is ms */ /* SPH_BT_CVSD_MSBC only */
    uint8_t  mic_delay_ms;            /* UL0 data delay, unit is ms */ /* SPH_EXT_DEV_INFO_SMARTPA_SPEAKER & SPH_EXT_DEV_INFO_SMARTPA_VIBRATION_SPEAKER only */

    uint8_t  mute_info;               /* mute status bit wise info */
    uint16_t enh_dynamic_ctrl;        /* sph_enh_dynamic_ctrl_t */
    uint32_t sph_param_usip_length;   /* Max  4294967295 bytes */
    uint32_t sph_param_usip_index;    /* Max  4294967295 bytes */
    uint16_t drv_common_param[12];    /* param for driver/scheduler */
    uint16_t drv_debug_info[16];      /* param for driver/scheduler */

    uint8_t  num_smart_pa;
    //uint8_t  mic_delay_ms_2;
    //uint16_t echo_ref_delay_ms_2;

    uint8_t  __reserve_at_128byte[43];
} sph_info_t; /* 128 bytes */

#define SIZE_OF_SPH_INFO (128)


typedef struct MdAliveInfo {
    uint16_t  headerMdVersion;             /* header type for ap/md header sync */
    uint16_t  mdVersion;                   /* MD version */

    uint8_t  __reserve_at_32byte[28];
} MdAliveInfo; /* 32 bytes */

#define SIZE_OF_MD_ALIVE_INFO (32)

enum SphShmType {
    SPH_SHM_AP_DATA = 1,
    SPH_SHM_MD_DATA = 2,
    SPH_SHM_SPEECH_PARAM = 3
};


/*
 * =============================================================================
 *                     share memory
 * =============================================================================
 */

typedef uint8_t sph_msg_buffer_t;

enum { /* sph_msg_buffer_t */
    SPH_MSG_BUFFER_TYPE_MAILBOX,
    SPH_MSG_BUFFER_TYPE_PAYLOAD
};


typedef struct sph_msg_t {
    uint8_t  buffer_type;       /* sph_msg_buffer_t */
    uint16_t msg_id;            /* sph_msg_id_t */

    /* only for SPH_MSG_BUFFER_TYPE_MAILBOX */
    union {
        uint16_t param_16bit;
        uint16_t length;        /* the data length in share memory xxx region */
    };

    union {
        uint32_t param_32bit;
        uint32_t rw_index;      /* the read/write index in share memory xxx region */
    };


    /* only for SPH_MSG_BUFFER_TYPE_PAYLOAD */
    uint16_t payload_data_type;         /* share_buff_data_type_t */
    uint16_t payload_data_size;         /* data size without header */
    void    *payload_data_addr;         /* must free at the end */
    uint16_t payload_data_idx;          /* only for modem payload */
    uint16_t payload_data_total_idx;    /* only for modem payload */
} sph_msg_t;


} // end namespace android

#endif

