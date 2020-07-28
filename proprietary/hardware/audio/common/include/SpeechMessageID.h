#ifndef ANDROID_SPEECH_MESSAGE_ID_H
#define ANDROID_SPEECH_MESSAGE_ID_H

#include <stdint.h>
#include <stdbool.h>

namespace android {


/**
 * =============================================================================
 *                     ref
 * =============================================================================
 */

struct sph_msg_t;


/**
 * =============================================================================
 *                     Speech Message ID
 * =============================================================================
 */

typedef uint16_t sph_msg_id_t;

enum { /* sph_msg_id_t */
    /**
     * =========================================================================
     *                     AP to Modem
     * =========================================================================
     */
    MSG_A2M_SPH_DL_DIGIT_VOLUME                 = 0x2F00,
    MSG_A2M_SPH_UL_DIGIT_VOLUME                 = 0x2F01,
    MSG_A2M_MUTE_SPH_UL                         = 0x2F02,
    MSG_A2M_MUTE_SPH_DL                         = 0x2F03,
    MSG_A2M_SIDETONE_VOLUME                     = 0x2F04, // 93 removed
    MSG_A2M_SPH_DL_ENH_REF_DIGIT_VOLUME         = 0x2F05,
    MSG_A2M_SIDETONE_CONFIG                     = 0x2F06, // 93 removed // Using modem SW STF or not
    //MSG_A2M_MUTE_SPH_UL_ENH_RESULT            = 0x2F07, // not used
    MSG_A2M_MUTE_SPH_UL_SOURCE                  = 0x2F08,
    MSG_A2M_MUTE_SPH_DL_CODEC                   = 0x2F09,

    MSG_A2M_SET_SAMPLE_RATE                     = 0x2F10, // 93 removed
    //MSG_A2M_SET_DUAL_MIC                      = 0x2F11, // 93 removed
    MSG_A2M_SET_BT_DELAY_TIME                   = 0x2F12, // 93 removed
    MSG_A2M_SET_LPBK_POINT_DVT                  = 0x2F13,

    MSG_A2M_SPH_ON                              = 0x2F20,
    MSG_A2M_SPH_OFF                             = 0x2F21,
    MSG_A2M_SET_SPH_MODE                        = 0x2F22, // 93 removed, but may comeback with customize
    MSG_A2M_CTRL_SPH_ENH                        = 0x2F23,
    MSG_A2M_CONFIG_SPH_ENH                      = 0x2F24, // 93 removed
    MSG_A2M_SET_ACOUSTIC_LOOPBACK               = 0x2F25, // 93 removed
    //MSG_A2M_PRINT_SPH_PARAM                   = 0x2F26, // 93 removed
    //MSG_A2M_SPH_ON_FOR_HOLD_CALL              = 0x2F27, // 93 removed // speech on with mute, for call hold use, no any other application can be turn on
    //MSG_A2M_SPH_ON_FOR_DACA                   = 0x2F28, // 93 removed
    MSG_A2M_SPH_ROUTER_ON                       = 0x2F29, // 93 removed // PCM wrouter on for enhancement and other application path.
    MSG_A2M_SPH_ENCRYPTION                      = 0x2F2A,
    MSG_A2M_SPH_DEV_CHANGE                      = 0x2F2B,
    MSG_A2M_ENH_CTRL_SUPPORT                    = 0x2F2C,

    MSG_A2M_PNW_ON                              = 0x2F30,
    MSG_A2M_PNW_OFF                             = 0x2F31,
    MSG_A2M_RECORD_ON                           = 0x2F32, // 93 removed
    MSG_A2M_RECORD_OFF                          = 0x2F33, // 93 removed
    //MSG_A2M_DMNR_RECPLAY_ON                   = 0x2F34, // 93 removed // not use in MT6582
    //MSG_A2M_DMNR_RECPLAY_OFF                  = 0x2F35, // 93 removed // not use in MT6582
    //MSG_A2M_DMNR_REC_ONLY_ON                  = 0x2F36, // 93 removed // not use in MT6582
    //MSG_A2M_DMNR_REC_ONLY_OFF                 = 0x2F37, // 93 removed // not use in MT6582
    MSG_A2M_PCM_REC_ON                          = 0x2F38, // 93 removed
    MSG_A2M_PCM_REC_OFF                         = 0x2F39, // 93 removed
    MSG_A2M_VM_REC_ON                           = 0x2F3A,
    MSG_A2M_VM_REC_OFF                          = 0x2F3B,
    MSG_A2M_RECORD_RAW_PCM_ON                   = 0x2F3C,
    MSG_A2M_RECORD_RAW_PCM_OFF                  = 0x2F3D,
    MSG_A2M_VOIP_RX_ON                          = 0x2F3E,
    MSG_A2M_VOIP_RX_OFF                         = 0x2F3F,

    MSG_A2M_CTM_ON                              = 0x2F40,
    MSG_A2M_CTM_OFF                             = 0x2F41,
    MSG_A2M_CTM_DUMP_DEBUG_FILE                 = 0x2F42,
    MSG_A2M_BGSND_ON                            = 0x2F43,
    MSG_A2M_BGSND_OFF                           = 0x2F44,
    MSG_A2M_BGSND_CONFIG                        = 0x2F45,
    MSG_A2M_RTT_CONFIG                          = 0x2F46,
    MSG_A2M_VOIP_RX_CONFIG                      = 0x2F47,
    MSG_A2M_TELEPHONY_TX_ON                     = 0x2F48,
    MSG_A2M_TELEPHONY_TX_OFF                    = 0x2F49,
    MSG_A2M_TELEPHONY_TX_CONFIG                 = 0x2F4A,

    MSG_A2M_PNW_DL_DATA_NOTIFY                  = 0x2F50,
    MSG_A2M_BGSND_DATA_NOTIFY                   = 0x2F51,
    MSG_A2M_CTM_DATA_NOTIFY                     = 0x2F52, // ?? removed
    //MSG_A2M_DACA_UL_DATA_NOTIFY               = 0x2F53, // 93 removed
    //MSG_A2M_ECALL_MSD                         = 0x2F54, // ecall for car
    MSG_A2M_SPH_UL_ENCRYPTION                   = 0x2F55,
    MSG_A2M_SPH_DL_DECRYPTION                   = 0x2F56,
    MSG_A2M_VOIP_RX_DL_DATA_NOTIFY              = 0x2F57,
    MSG_A2M_VOIP_RX_UL_DATA_NOTIFY              = 0x2F58,
    MSG_A2M_TELEPHONY_TX_UL_DATA_NOTIFY         = 0x2F59,

    MSG_A2M_PNW_UL_DATA_READ_ACK                = 0x2F60,
    MSG_A2M_REC_DATA_READ_ACK                   = 0x2F61, // 93 removed
    MSG_A2M_CTM_DEBUG_DATA_READ_ACK             = 0x2F62,
    MSG_A2M_PCM_REC_DATA_READ_ACK               = 0x2F63, // 93 removed
    MSG_A2M_VM_REC_DATA_READ_ACK                = 0x2F64,
    //MSG_A2M_DACA_DL_DATA_READ_ACK             = 0x2F65, // 93 removed
    MSG_A2M_RAW_PCM_REC_DATA_READ_ACK           = 0x2F66,
    MSG_A2M_CUST_DUMP_READ_ACK                  = 0x2F67,

    MSG_A2M_EM_DATA_REQUEST_ACK                 = 0x2F70, // 93 removed, MD no need
    MSG_A2M_EM_NB                               = 0x2F71, // 91 removed
    MSG_A2M_EM_DMNR                             = 0x2F72, // 91 removed
    MSG_A2M_EM_WB                               = 0x2F73, // 91 removed
    MSG_A2M_EM_MAGICON                          = 0x2F74, // 91 removed
    MSG_A2M_NETWORK_STATUS_ACK                  = 0x2F75, // modem no need
    //MSG_A2M_QUERY_RF_INFO                     = 0x2F76, // only for 6595/6795 Async FIFO
    MSG_A2M_EM_HAC                              = 0x2F77, // 91 removed
    MSG_A2M_EPOF_ACK                            = 0x2F78,
    MSG_A2M_EM_DYNAMIC_SPH                      = 0x2F79,
    MSG_A2M_SPH_ENH_CORE                        = 0x2F7A, // 93 removed // opendsp 1, speech_dsp 2
    MSG_A2M_DYNAMIC_PAR_IN_STRUCT_SHM           = 0x2F7B,

    MSG_A2M_VIBSPK_PARAMETER                    = 0x2F80,
    MSG_A2M_SMARTPA_PARAMETER                   = 0x2F81, // 93 removed

    MSG_A2M_NW_CODEC_INFO_READ_ACK              = 0x2F90,

    MSG_A2M_MD_ALIVE_ACK_BACK                   = 0x2FA0,



    /**
     * =========================================================================
     *                     Modem to AP
     * =========================================================================
     */
    //MSG_M2A_SPH_DL_DIGIT_VOLUME_ACK           = 0x8000 | MSG_A2M_SPH_DL_DIGIT_VOLUME, // AP no need
    //MSG_M2A_SPH_UL_DIGIT_VOLUME_ACK           = 0x8000 | MSG_A2M_SPH_UL_DIGIT_VOLUME, // AP no need
    MSG_M2A_MUTE_SPH_UL_ACK                     = 0x8000 | MSG_A2M_MUTE_SPH_UL,
    MSG_M2A_MUTE_SPH_DL_ACK                     = 0x8000 | MSG_A2M_MUTE_SPH_DL,
    //MSG_M2A_SIDETONE_VOLUME_ACK               = 0x8000 | MSG_A2M_SIDETONE_VOLUME, // AP no need
    //MSG_M2A_SPH_DL_ENH_REF_DIGIT_VOLUME_ACK   = 0x8000 | MSG_A2M_SPH_DL_ENH_REF_DIGIT_VOLUME, // AP no need
    //MSG_M2A_SIDETONE_CONFIG_ACK               = 0x8000 | MSG_A2M_SIDETONE_CONFIG, // AP no need
    //MSG_M2A_MUTE_SPH_UL_ENH_RESULT_ACK        = 0x8000 | MSG_A2M_MUTE_SPH_UL_ENH_RESULT, // AP no need
    MSG_M2A_MUTE_SPH_UL_SOURCE_ACK              = 0x8000 | MSG_A2M_MUTE_SPH_UL_SOURCE,
    //MSG_M2A_MUTE_SPH_DL_CODEC_ACK             = 0x8000 | MSG_A2M_MUTE_SPH_DL_CODEC, // AP no need


    //MSG_M2A_SET_SAMPLE_RATE_ACK               = 0x8000 | MSG_A2M_SET_SAMPLE_RATE, // AP no need
    //MSG_M2A_SET_DUAL_MIC_ACK                  = 0x8000 | MSG_A2M_SET_DUAL_MIC, // AP no need
    //MSG_M2A_SET_BT_DELAY_TIME_ACK             = 0x8000 | MSG_A2M_SET_BT_DELAY_TIME, // AP no need
    //MSG_M2A_SET_LPBK_POINT_DVT_ACK            = 0x8000 | MSG_A2M_SET_LPBK_POINT_DVT, // AP no need


    MSG_M2A_SPH_ON_ACK                          = 0x8000 | MSG_A2M_SPH_ON,
    MSG_M2A_SPH_OFF_ACK                         = 0x8000 | MSG_A2M_SPH_OFF,
    MSG_M2A_SET_SPH_MODE_ACK                    = 0x8000 | MSG_A2M_SET_SPH_MODE,
    //MSG_M2A_CTRL_SPH_ENH_ACK                  = 0x8000 | MSG_A2M_CTRL_SPH_ENH, // AP no need
    //MSG_M2A_CONFIG_SPH_ENH_ACK                = 0x8000 | MSG_A2M_CONFIG_SPH_ENH, // AP no need
    MSG_M2A_SET_ACOUSTIC_LOOPBACK_ACK           = 0x8000 | MSG_A2M_SET_ACOUSTIC_LOOPBACK,
    //MSG_M2A_PRINT_SPH_PARAM_ACK               = 0x8000 | MSG_A2M_PRINT_SPH_PARAM,
    //MSG_M2A_SPH_ON_FOR_HOLD_CALL_ACK          = 0x8000 | MSG_A2M_SPH_ON_FOR_HOLD_CALL,
    //MSG_M2A_SPH_ON_FOR_DACA_ACK               = 0x8000 | MSG_A2M_SPH_ON_FOR_DACA,
    MSG_M2A_SPH_ROUTER_ON_ACK                   = 0x8000 | MSG_A2M_SPH_ROUTER_ON,
    MSG_M2A_SPH_ENCRYPTION_ACK                  = 0x8000 | MSG_A2M_SPH_ENCRYPTION,
    MSG_M2A_SPH_DEV_CHANGE_ACK                  = 0x8000 | MSG_A2M_SPH_DEV_CHANGE,
    //MSG_M2A_ENH_CTRL_SUPPORT                  = 0x8000 | MSG_A2M_ENH_CTRL_SUPPORT, // AP no need


    MSG_M2A_PNW_ON_ACK                          = 0x8000 | MSG_A2M_PNW_ON,
    MSG_M2A_PNW_OFF_ACK                         = 0x8000 | MSG_A2M_PNW_OFF,
    MSG_M2A_RECORD_ON_ACK                       = 0x8000 | MSG_A2M_RECORD_ON,
    MSG_M2A_RECORD_OFF_ACK                      = 0x8000 | MSG_A2M_RECORD_OFF,
    //MSG_M2A_DMNR_RECPLAY_ON_ACK               = 0x8000 | MSG_A2M_DMNR_RECPLAY_ON,
    //MSG_M2A_DMNR_RECPLAY_OFF_ACK              = 0x8000 | MSG_A2M_DMNR_RECPLAY_OFF,
    //MSG_M2A_DMNR_REC_ONLY_ON_ACK              = 0x8000 | MSG_A2M_DMNR_REC_ONLY_ON,
    //MSG_M2A_DMNR_REC_ONLY_OFF_ACK             = 0x8000 | MSG_A2M_DMNR_REC_ONLY_OFF,
    MSG_M2A_PCM_REC_ON_ACK                      = 0x8000 | MSG_A2M_PCM_REC_ON,
    MSG_M2A_PCM_REC_OFF_ACK                     = 0x8000 | MSG_A2M_PCM_REC_OFF,
    MSG_M2A_VM_REC_ON_ACK                       = 0x8000 | MSG_A2M_VM_REC_ON,
    MSG_M2A_VM_REC_OFF_ACK                      = 0x8000 | MSG_A2M_VM_REC_OFF,
    MSG_M2A_RECORD_RAW_PCM_ON_ACK               = 0x8000 | MSG_A2M_RECORD_RAW_PCM_ON,
    MSG_M2A_RECORD_RAW_PCM_OFF_ACK              = 0x8000 | MSG_A2M_RECORD_RAW_PCM_OFF,
    MSG_M2A_VOIP_RX_ON_ACK                      = 0x8000 | MSG_A2M_VOIP_RX_ON,
    MSG_M2A_VOIP_RX_OFF_ACK                     = 0x8000 | MSG_A2M_VOIP_RX_OFF,

    MSG_M2A_CTM_ON_ACK                          = 0x8000 | MSG_A2M_CTM_ON,
    MSG_M2A_CTM_OFF_ACK                         = 0x8000 | MSG_A2M_CTM_OFF,
    //MSG_M2A_CTM_DUMP_DEBUG_FILE_ACK           = 0x8000 | MSG_A2M_CTM_DUMP_DEBUG_FILE, // AP no need
    MSG_M2A_BGSND_ON_ACK                        = 0x8000 | MSG_A2M_BGSND_ON,
    MSG_M2A_BGSND_OFF_ACK                       = 0x8000 | MSG_A2M_BGSND_OFF,
    //MSG_M2A_BGSND_CONFIG_ACK                  = 0x8000 | MSG_A2M_BGSND_CONFIG,
    //MSG_M2A_RTT_CONFIG_ACK                    = 0x8000 | MSG_A2M_RTT_CONFIG, // AP no need
    MSG_M2A_VOIP_RX_CONFIG_ACK                  = 0x8000 | MSG_A2M_VOIP_RX_CONFIG,
    MSG_M2A_TELEPHONY_TX_ON_ACK                 = 0x8000 | MSG_A2M_TELEPHONY_TX_ON,
    MSG_M2A_TELEPHONY_TX_OFF_ACK                = 0x8000 | MSG_A2M_TELEPHONY_TX_OFF,
    MSG_M2A_TELEPHONY_TX_CONFIG_ACK             = 0x8000 | MSG_A2M_TELEPHONY_TX_CONFIG,

    MSG_M2A_PNW_DL_DATA_REQUEST                 = 0x8000 | MSG_A2M_PNW_DL_DATA_NOTIFY,
    MSG_M2A_BGSND_DATA_REQUEST                  = 0x8000 | MSG_A2M_BGSND_DATA_NOTIFY,
    MSG_M2A_CTM_DATA_REQUEST                    = 0x8000 | MSG_A2M_CTM_DATA_NOTIFY,
    //MSG_M2A_DACA_UL_DATA_REQUEST              = 0x8000 | MSG_A2M_DACA_UL_DATA_NOTIFY,
    //MSG_M2A_ECALL_MSD                         = 0x8000 | MSG_A2M_ECALL_MSD, // ecall for car
    MSG_M2A_SPH_UL_ENCRYPTION                   = 0x8000 | MSG_A2M_SPH_UL_ENCRYPTION,
    MSG_M2A_SPH_DL_DECRYPTION                   = 0x8000 | MSG_A2M_SPH_DL_DECRYPTION,
    MSG_M2A_VOIP_RX_DL_DATA_REQUEST             = 0x8000 | MSG_A2M_VOIP_RX_DL_DATA_NOTIFY,
    MSG_M2A_VOIP_RX_UL_DATA_REQUEST             = 0x8000 | MSG_A2M_VOIP_RX_UL_DATA_NOTIFY,
    MSG_M2A_TELEPHONY_TX_UL_DATA_REQUEST        = 0x8000 | MSG_A2M_TELEPHONY_TX_UL_DATA_NOTIFY,

    MSG_M2A_PNW_UL_DATA_NOTIFY                  = 0x8000 | MSG_A2M_PNW_UL_DATA_READ_ACK,
    //MSG_M2A_REC_DATA_NOTIFY                   = 0x8000 | MSG_A2M_REC_DATA_READ_ACK,
    MSG_M2A_CTM_DEBUG_DATA_NOTIFY               = 0x8000 | MSG_A2M_CTM_DEBUG_DATA_READ_ACK,
    MSG_M2A_PCM_REC_DATA_NOTIFY                 = 0x8000 | MSG_A2M_PCM_REC_DATA_READ_ACK,
    MSG_M2A_VM_REC_DATA_NOTIFY                  = 0x8000 | MSG_A2M_VM_REC_DATA_READ_ACK,
    //MSG_M2A_DACA_DL_DATA_NOTIFY               = 0x8000 | MSG_A2M_DACA_DL_DATA_READ_ACK,
    MSG_M2A_RAW_PCM_REC_DATA_NOTIFY             = 0x8000 | MSG_A2M_RAW_PCM_REC_DATA_READ_ACK,
    MSG_M2A_CUST_DUMP_NOTIFY                    = 0x8000 | MSG_A2M_CUST_DUMP_READ_ACK,

    MSG_M2A_EM_DATA_REQUEST                     = 0x8000 | MSG_A2M_EM_DATA_REQUEST_ACK,
    MSG_M2A_EM_NB_ACK                           = 0x8000 | MSG_A2M_EM_NB,
    MSG_M2A_EM_DMNR_ACK                         = 0x8000 | MSG_A2M_EM_DMNR,
    MSG_M2A_EM_WB_ACK                           = 0x8000 | MSG_A2M_EM_WB,
    MSG_M2A_EM_MAGICON_ACK                      = 0x8000 | MSG_A2M_EM_MAGICON,
    MSG_M2A_NETWORK_STATUS_NOTIFY               = 0x8000 | MSG_A2M_NETWORK_STATUS_ACK,
    //MSG_M2A_QUERY_RF_INFO_ACK                 = 0x8000 | MSG_A2M_QUERY_RF_INFO,
    MSG_M2A_EM_HAC_ACK                          = 0x8000 | MSG_A2M_EM_HAC,
    MSG_M2A_EPOF_NOTIFY                         = 0x8000 | MSG_A2M_EPOF_ACK,
    MSG_M2A_EM_DYNAMIC_SPH_ACK                  = 0x8000 | MSG_A2M_EM_DYNAMIC_SPH,
    MSG_M2A_SPH_ENH_CORE_ACK                    = 0x8000 | MSG_A2M_SPH_ENH_CORE,
    MSG_M2A_DYNAMIC_PAR_IN_STRUCT_SHM_ACK       = 0x8000 | MSG_A2M_DYNAMIC_PAR_IN_STRUCT_SHM,

    MSG_M2A_VIBSPK_PARAMETER_ACK                = 0x8000 | MSG_A2M_VIBSPK_PARAMETER,
    MSG_M2A_SMARTPA_PARAMETER_ACK               = 0x8000 | MSG_A2M_SMARTPA_PARAMETER,

    MSG_M2A_NW_CODEC_INFO_NOTIFY                = 0x8000 | MSG_A2M_NW_CODEC_INFO_READ_ACK,

    MSG_M2A_MD_ALIVE                            = 0x8000 | MSG_A2M_MD_ALIVE_ACK_BACK,
};



/**
 * =============================================================================
 *                     sph_msg_handshake_t
 * =============================================================================
 */

typedef uint8_t sph_msg_handshake_t;

enum { /* sph_msg_handshake_t */
    /* AP -> Modem */
    SPH_MSG_HANDSHAKE_AP_CTRL_BYPASS_ACK   = 0x00, /* wait in quque(v), block queue(x) */
    SPH_MSG_HANDSHAKE_AP_CTRL_NEED_ACK     = 0x01, /* wait in quque(v), block queue(v) */

    SPH_MSG_HANDSHAKE_AP_ACK_BACK_MD_CTRL  = 0x11, /* ack to modem */

    SPH_MSG_HANDSHAKE_AP_NOTIFY_DATA       = 0x20, /* write ap data to modem */
    SPH_MSG_HANDSHAKE_AP_READ_DATA_DONE    = 0x21, /* read  md data done */


    /* MD -> AP */
    SPH_MSG_HANDSHAKE_MD_ACK_BACK_AP_CTRL  = 0x81, /* ack to ap */

    SPH_MSG_HANDSHAKE_MD_CTRL_BYPASS_ACK   = 0x90,
    SPH_MSG_HANDSHAKE_MD_CTRL_NEED_ACK     = 0x91,


    SPH_MSG_HANDSHAKE_MD_REQUEST_DATA      = 0xA0, /* modem need  ap data */
    SPH_MSG_HANDSHAKE_MD_NOTIFY_DATA       = 0xA1, /* modem offer md data */

    /* default */
    SPH_MSG_HANDSHAKE_INVALID              = 0xFF
};


uint16_t getSyncType(const uint16_t msg_id);


/**
 * =============================================================================
 *                     utility
 * =============================================================================
 */

bool isApMsgBypassQueue(const struct sph_msg_t *p_sph_msg);

bool isApNeedAck(const struct sph_msg_t *p_sph_msg);
bool isMdAckBack(const struct sph_msg_t *p_sph_msg);


bool isApMsg(const struct sph_msg_t *p_sph_msg);

bool isMdMsg(const struct sph_msg_t *p_sph_msg);

int  makeFakeMdAckMsgFromApMsg(struct sph_msg_t *p_sph_msg);

bool isAckMessageInPair(const struct sph_msg_t *p_sph_msg,
                        const struct sph_msg_t *p_sph_msg_ack);

bool isAckMessageInPairByID(const uint16_t ap_msg_id, const uint16_t md_msg_id);

uint32_t getBlockThreadTimeMsByID(struct sph_msg_t *p_sph_msg);

bool isNeedDumpMsg(const struct sph_msg_t *p_sph_msg);


} /* end namespace android */

#endif /* end of ANDROID_SPEECH_MESSAGE_ID_H */

