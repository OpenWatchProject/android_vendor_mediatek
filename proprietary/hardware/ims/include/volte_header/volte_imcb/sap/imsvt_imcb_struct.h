#ifndef __IMSVT_IMCB_STRUCT_H__
#define __IMSVT_IMCB_STRUCT_H__

#if __IMCF_NO_UA__

#define VOLTE_MAX_SDP_PARAMETER_SET_LENGTH              (256) // H.264, Spros parameter sets, must align with LTE_VT_SRV_UA_PARAMETER_SIZE_MAX.
#define VOLTE_MAX_SUB_PARAM_LENGTH  (128)
#define VOLTE_MAX_SDP_FMTP_TOT_VALUE_LENGTH             (32)// The maximum length of total fmtp specific parameter key and value, must align with LTE_VT_SRV_UA_FMTP_SIZE_MAX.
#define VOLTE_MAX_SDP_EGBYTE_LENGTH                     (8)
#define VOLTE_IPADDR_LENGTH                             (16) // IP address in binary
#define IMC_MAX_IF_NAME_LENGTH          (16)
#define VOLTE_MAX_IF_NAME_LENGTH                        (IMC_MAX_IF_NAME_LENGTH)

#define VOLTE_MAX_IMAGE_NUM                             (4)
#define VOLTE_MAX_RTCP_FB_SIZE                          (8)
#define VOLTE_MAX_VIDEO_CAP_NUM                         (4)
#define VOLTE_MAX_PS_NUM                                (32)
#define VOLTE_MAX_PROFILELEVEL_SIZE                     (32)
#define VOLTE_MAX_EXTMAP_URI_SIZE                       (256)
#define VOLTE_MAX_EXTMAP_NUM                            (4) // RFC 5285

typedef char      imcf_int8 ;
typedef short int imcf_int16;
typedef       int imcf_int32;

typedef unsigned char       imcf_uint8 ;
typedef unsigned short int  imcf_uint16;
typedef unsigned       int  imcf_uint32;

typedef enum imsvt_imcb_msg_id_e {

  /* Type4: IMSVT-IMCB TV { */
  MSG_ID_WRAP_IMSVT_IMCB_BEGIN                  = 101501,
    MSG_ID_WRAP_IMSVT_IMCB_LOOP_TEST_IND        = 101502,
    MSG_ID_WRAP_IMSVT_IMCB_LOOP_TEST_RSP        = 101503,
    MSG_ID_WRAP_IMSVT_IMCB_COMM_ERROR           = 101504,

    MSG_ID_WRAP_IMSVT_IMCB_GET_CAP_IND          = 101505,
    MSG_ID_WRAP_IMSVT_IMCB_GET_CAP_RSP          = 101506,

    MSG_ID_WRAP_IMSVT_IMCB_CONFIG_INIT_IND      = 101507,
    MSG_ID_WRAP_IMSVT_IMCB_CONFIG_UPDATE_IND    = 101508,
    MSG_ID_WRAP_IMSVT_IMCB_CONFIG_DEINIT_IND    = 101509,

    MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_REQ   = 101510,
    MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_CNF   = 101511,
    MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_IND   = 101512,
    MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_RSP   = 101513,

    /*for vt upgrade cancel*/
    /* old vt interface */
    MSG_ID_WRAP_IMSVT_IMCB_CONFIG_INIT               = 101514,
    MSG_ID_WRAP_IMSVT_IMCB_CONFIG_UPDATE             = 101515,
    MSG_ID_WRAP_IMSVT_IMCB_GET_CAP_REQ               = 101516,

    MSG_ID_WRAP_IMSVT_IMCB_MOD_BW_REQ                = 101517,

    MSG_ID_WRAP_IMSVT_IMCB_HANDOVER_START_IND        = 101518,
    MSG_ID_WRAP_IMSVT_IMCB_HANDOVER_STOP_IND         = 101519,

    MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_CANCEL_REQ = 101520,
    MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_CANCEL_CNF = 101521,
    MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_CANCEL_IND = 101522,
    MSG_ID_WRAP_IMSVT_IMCB_EVENT_LOCAL_BW_READY_IND  = 101523,
    MSG_ID_WRAP_IMSVT_IMCB_CALL_PROGRESS_IND         = 101524,
    MSG_ID_WRAP_IMSVT_IMCB_PDN_HANDOVER_BEGIN_IND    = 101525,
    MSG_ID_WRAP_IMSVT_IMCB_PDN_HANDOVER_END_IND      = 101526,

  MSG_ID_WRAP_IMSVT_IMCB_END,
  /* } Type4: IMSVT-IMCB TV */
} imsvt_imcb_msg_id_t;

#else
#include "volte_vt_event.h"
#include "volte_def.h"
#endif

#define VT_MAX_SDP_PARAMETER_SET_LENGTH             VOLTE_MAX_SDP_PARAMETER_SET_LENGTH
#define VT_MAX_SUB_SDP_PARAMETER_SET_LENGTH         VOLTE_MAX_SUB_PARAM_LENGTH
#define VT_MAX_SDP_FMTP_TOT_VALUE_LENGTH            VOLTE_MAX_SDP_FMTP_TOT_VALUE_LENGTH
#define VT_MAX_SDP_EGBYTE_LENGTH                    VOLTE_MAX_SDP_EGBYTE_LENGTH
#define VT_MAX_IMAGE_NUM                            VOLTE_MAX_IMAGE_NUM
#define VT_MAX_RTCP_FB_SIZE                         VOLTE_MAX_RTCP_FB_SIZE
#define VT_MAX_VIDEO_CAP_NUM                        VOLTE_MAX_VIDEO_CAP_NUM
#define VT_ADDR_LENGTH                              VOLTE_IPADDR_LENGTH
#define VT_MAX_PS_NUM                               VOLTE_MAX_PS_NUM
#define VT_MAX_LEVEL_SIZE                           VOLTE_MAX_PROFILELEVEL_SIZE
#define VT_MAX_EXTMAP_URI_SIZE                      VOLTE_MAX_EXTMAP_URI_SIZE
#define VT_MAX_EXTMAP_NUM                           VOLTE_MAX_EXTMAP_NUM
#define VT_MAX_IF_NAME_LENGTH                       VOLTE_MAX_IF_NAME_LENGTH

typedef enum vt_srv_imcb_session_modify_type_e {
    VT_SRV_IMCB_SESSION_MODIFY_TYPE_CAMERA,
    VT_SRV_IMCB_SESSION_MODIFY_TYPE_CALL_TPYE,
    VT_SRV_IMCB_SESSION_MODIFY_TYPE_RTP,
    VT_SRV_IMCB_SESSION_MODIFY_TYPE_END
} vt_srv_imcb_session_modify_type_t ;

typedef enum vt_srv_imcb_session_modify_result_e {
    VT_SRV_IMCB_SESSION_MODIFY_OK = 0,
    VT_SRV_IMCB_SESSION_MODIFY_WrongVideoDir,           /*wrong video direction cause up/downgrade failed*/
    VT_SRV_IMCB_SESSION_MODIFY_InternalError,           /*acct or session object broken*/
    VT_SRV_IMCB_SESSION_MODIFY_Result_BW_ModifyFailed,  /*bandwidth modify require failed*/
    VT_SRV_IMCB_SESSION_MODIFY_NotActiveState,          /*call session not in active state*/
    VT_SRV_IMCB_SESSION_MODIFY_LocalRel,                /*local release:Ex.SRVCC,Hungup,call refresh timer timeout,no rpt packets*/
    VT_SRV_IMCB_SESSION_MODIFY_IsHold,                  /*the call at hold state*/
    VT_SRV_IMCB_SESSION_MODIFY_NoNeed,                  /*acorrding the video action state, not need up/downgrade */
    VT_SRV_IMCB_SESSION_MODIFY_InvalidPara,             /*input parameter invalid*/
    VT_SRV_IMCB_SESSION_MODIFY_ReqTimeout,              /*SIPTX error,transaction timeout*/
    VT_SRV_IMCB_SESSION_MODIFY_RejectByRemote,          /*reject by remote*/
    VT_SRV_IMCB_SESSION_MODIFY_Canceled,                 /*canceled by user*/
}vt_srv_imcb_session_modify_result_t;

enum vt_srv_imcb_session_modify_cancel_cause_e {
    VT_SRV_IMCB_SESSION_MODIFY_CANCEL_CAUSE_cancel= 0,
    VT_SRV_IMCB_SESSION_MODIFY_CANCEL_CAUSE_timeout,
};

enum vt_srv_imcb_session_modify_cancel_result_e {
    VT_SRV_IMCB_SESSION_MODIFY_CANCEL_success = 0,
    VT_SRV_IMCB_SESSION_MODIFY_CANCEL_fail_downgrade,
    VT_SRV_IMCB_SESSION_MODIFY_CANCEL_fail_normal,
    VT_SRV_IMCB_SESSION_MODIFY_CANCEL_fail_disable,
    VT_SRV_IMCB_SESSION_MODIFY_CANCEL_fail_remote_reject,         /*remote reject the upgrade while local cancel*/
    VT_SRV_IMCB_SESSION_MODIFY_CANCEL_fail_internal_error,
    VT_SRV_IMCB_SESSION_MODIFY_CANCEL_fail_local_rel,
    VT_SRV_IMCB_SESSION_MODIFY_CANCEL_fail_bw_modify_fail,
    VT_SRV_IMCB_SESSION_MODIFY_CANCEL_fail_req_timeout,
};


typedef struct rtcp_fb_param_type_s {
    imcf_uint16 rtcp_fb_id;                  // such as "ack","nack","trr-int","ccm","app"
    imcf_uint16 rtcp_fb_param;               // such as pli"
    imcf_int8   rtcp_fb_sub_param[VT_MAX_SUB_SDP_PARAMETER_SET_LENGTH]; //such as smaxpr=120 of tmmbr
}vt_srv_rtcp_fb_param_type_t;

typedef struct vt_srv_image_s {
    imcf_uint32     x;    ///< RFC6236 image attributes
    imcf_uint32     y;
    imcf_int8       sar[VT_MAX_SDP_EGBYTE_LENGTH];
    imcf_int8       par[VT_MAX_SDP_EGBYTE_LENGTH];
    imcf_int8       q[VT_MAX_SDP_EGBYTE_LENGTH];

} vt_srv_image_t;

typedef struct vt_srv_imcb_msg_get_cap_ind_s {
    imcf_uint8 sim_slot_id; /* start from 0 */
    imcf_uint8 pad;
    imcf_uint16 operator_code;
} vt_srv_imcb_msg_get_cap_ind_struct, imsvt_imcb_get_cap_ind_struct;

/*******************
** VT_CAP_SETTING **
********************/

typedef struct vt_srv_h264_cap_cfg_s
{    /* Codec part */
    imcf_uint32                                  profile_level_id;       ///< H.264 profile level id. construct by 3 parts

    imcf_uint16                                  max_recv_level;         ///< require 2 bytes, reserved.
    imcf_uint8                                   redundant_pic_cap;     ///< 0, 1. 0: we do not support yet.
    imcf_uint8                                   pading;

    imcf_int8                                    sprop_parameter_sets[VT_MAX_SDP_PARAMETER_SET_LENGTH];
    imcf_int8                                    sprop_level_parameter_sets[VT_MAX_SDP_PARAMETER_SET_LENGTH];
    imcf_uint32                                  max_mbps;               ///< reserved. VT service provides default value
    imcf_uint32                                  max_smbps;              ///< reserved. VT service provides default value
    imcf_uint32                                  max_fs;                 ///< reserved. VT service provides default value
    imcf_uint32                                  max_cpb;                ///< reserved. VT service provides default value
    imcf_uint32                                  max_dpb;                ///< reserved. VT service provides default value
    imcf_uint32                                  max_br;                 ///< reserved. VT service provides default value
    imcf_uint32                                  max_rcmd_nalu_size;     ///< reserved. VT service provides default value

    imcf_uint8                                   sar_understood;         ///< 0~255, reserved. VT service provides default value
    imcf_uint8                                   sar_supported;          ///< 0~255, reserved. VT service provides default value
    imcf_uint8                                   in_band_parameter_sets; //0, 1
    imcf_uint8                                   level_asymmetry_allowed; //0, 1
    imcf_uint32                                  framerate;
    imcf_uint32                                  frame_width;
    imcf_uint32                                  frame_height;
    vt_srv_image_t                               image_send[VT_MAX_IMAGE_NUM];
    vt_srv_image_t                               image_recv[VT_MAX_IMAGE_NUM];
} vt_srv_h264_cap_cfg_t;

typedef struct vt_srv_hevc_cap_cfg_s {

    imcf_uint8                                    profile_space;
    imcf_uint8                                    profile_id;
    imcf_uint8                                    tier_flag;
    imcf_uint8                                    level_id;

    imcf_uint8                                    interop_constraints[6];
    imcf_uint8                                    pad[2];

    imcf_uint8                                    profile_comp_ind[4];

    imcf_uint8                                    sprop_sub_layer_id;
    imcf_uint8                                    recv_sub_layer_id;
    imcf_uint8                                    max_recv_level_id;
    imcf_uint8                                    tx_mode;  //"SRST", "MRST" or "MRMT"

    imcf_int8                                     sprop_vps[VOLTE_MAX_SDP_PARAMETER_SET_LENGTH];
    imcf_int8                                     sprop_sps[VOLTE_MAX_SDP_PARAMETER_SET_LENGTH];
    imcf_int8                                     sprop_pps[VOLTE_MAX_SDP_PARAMETER_SET_LENGTH];
    imcf_int8                                     sprop_sei[VOLTE_MAX_SDP_PARAMETER_SET_LENGTH];

    imcf_uint32                                   max_lsr;
    imcf_uint32                                   max_lps;
    imcf_uint32                                   max_cpb;
    imcf_uint32                                   max_dpb;
    imcf_uint32                                   max_br;
    imcf_uint32                                   max_tr;
    imcf_uint32                                   max_tc;
    imcf_uint32                                   max_fps;
    imcf_uint32                                   sprop_max_don_diff;
    imcf_uint32                                   sprop_depack_buf_nalus;
    imcf_uint32                                   sprop_depack_buf_bytes;
    imcf_uint32                                   depack_buf_cap;
    imcf_uint32                                   sprop_seg_id;
    imcf_uint32                                   sprop_spatial_seg_idc;

    imcf_int8                                     dec_parallel_cap[VOLTE_MAX_SDP_PARAMETER_SET_LENGTH];
    imcf_int8                                     include_dph[VOLTE_MAX_SDP_PARAMETER_SET_LENGTH];
    imcf_uint32                                   framerate;
    imcf_uint32                                   frame_width;
    imcf_uint32                                   frame_height;
    vt_srv_image_t                                image_send[VT_MAX_IMAGE_NUM];
    vt_srv_image_t                                image_recv[VT_MAX_IMAGE_NUM];

} vt_srv_hevc_cap_cfg_t;

typedef union vt_srv_video_codec_cap_cfg_u {
    vt_srv_h264_cap_cfg_t    h264_codec;
    vt_srv_hevc_cap_cfg_t    hevc_codec;
} vt_srv_video_codec_cap_cfg_t;

typedef struct vt_srv_video_media_bitrate_s {
    imcf_uint32 format;        ///<H264 or H265
    imcf_uint32 profile;
    imcf_uint32 level;
    imcf_uint32 minbitrate;
    imcf_uint32 bitrate;
} vt_srv_video_media_bitrate_t;

typedef struct vt_srv_rtp_ext_map_s {
    imcf_uint8 ext_id;
    imcf_uint8 direction;
    imcf_uint8 reserved[2];
    imcf_int8  ext_uri[VT_MAX_EXTMAP_URI_SIZE]; // size 256
} vt_srv_rtp_ext_map_t;

typedef struct vt_srv_h264_ps_s {
    imcf_uint32 profile_level_id;
    imcf_int8   sprop_parameter_sets [VT_MAX_SDP_PARAMETER_SET_LENGTH];
} vt_srv_h264_ps_t;

typedef struct vt_srv_hevc_ps_s {
    imcf_uint8  profile_id;             ///< Range: 0~31, 1 if not present.
    imcf_uint8  level_id;               ///< Range: 0~255, 93(level 3.1) if not present.
    imcf_uint8  pad[2];
    imcf_int8   sprop_vps[VT_MAX_SDP_PARAMETER_SET_LENGTH];
    imcf_int8   sprop_sps[VT_MAX_SDP_PARAMETER_SET_LENGTH];
    imcf_int8   sprop_pps[VT_MAX_SDP_PARAMETER_SET_LENGTH];
    imcf_int8   sprop_sei[VT_MAX_SDP_PARAMETER_SET_LENGTH];
} vt_srv_hevc_ps_t;

typedef struct vt_srv_video_capability_s {
    /* old start */
    imcf_uint32                                  profile_level_id;       ///< H.264 profile level id. construct by 3 parts

    imcf_uint16                                  max_recv_level;         ///< require 2 bytes, reserved.
    imcf_uint8                                   packetization_mode;     ///< VoLTE_Event_Packetize_Mode_e
    imcf_uint8                                   redundant_pic_cap;    ///< 0, 1. 0: we do not support yet.

    imcf_int8                                    sprop_parameter_sets [VT_MAX_SDP_PARAMETER_SET_LENGTH];
    imcf_int8                                    sprop_level_parameter_sets [VT_MAX_SDP_PARAMETER_SET_LENGTH];
    imcf_uint32                                  max_mbps;               ///< reserved. VT service provides default value
    imcf_uint32                                  max_smbps;              ///< reserved. VT service provides default value
    imcf_uint32                                  max_fs ;                ///< reserved. VT service provides default value
    imcf_uint32                                  max_cpb ;               ///< reserved. VT service provides default value
    imcf_uint32                                  max_dpb ;               ///< reserved. VT service provides default value
    imcf_uint32                                  max_br ;                ///< reserved. VT service provides default value
    imcf_uint32                                  max_rcmd_nalu_size;     ///< reserved. VT service provides default value

    imcf_uint8                                   sar_understood;         ///< 0~255, reserved. VT service provides default value
    imcf_uint8                                   sar_supported;          ///< 0~255, reserved. VT service provides default value
    imcf_uint8                                   in_band_parameter_sets; //0, 1
    imcf_uint8                                   level_asymmetry_allowed; //0, 1
    /* old end */

    vt_srv_video_codec_cap_cfg_t    codec_cap;  // Use mime_Type to judge the codec cap.

    /* rtp_rtcp part */
    imcf_uint8  media_type; //such as video/audio
    imcf_uint8  mime_Type; // such as H.264
    //imcf_uint8  video_payload_type;// num such as: 97,98, 100
    imcf_uint8  rtp_profile;//supported rtp_profile for special media such as RTP/AVP,RTP/AVPF
    imcf_uint8  reserved;
    imcf_uint32 sample_rate;//timescale, which use to calculte rtp timestamp

    /*@ rtp packet transmission param
    *@ contain b=AS,PT,packetization params related to the codec type
    *@ ----------------------------------------------------------------------
    *@consider the negotiated params also can set through rtp_rtcp_capability
    *@ rtp_rtcp_capability must be can for one explicit item
    *@ then should not contain alternatives in rtp_packetize_params,such as "packetization-moe=1;packetization-mode=0"
    *@ ----------------------------------------------------------------------
    */
    //imcf_uint32 video_b_as; //b=AS in kb/s
    imcf_uint32 packetize_mode;
    imcf_uint32 extmap_num;
    vt_srv_rtp_ext_map_t extmap[VT_MAX_EXTMAP_NUM]; //now only CVO support

    //imcf_uint32 video_b_rs; //b=RS in bps
    //imcf_uint32 video_b_rr; //b=RR in bps
    imcf_uint32 rtcp_rsize; //a=rtcp_rsize

    /*@ rtcp packet transmission param
     *@ contain b=RS,b=RR,and feedback message type
     *@ may be serveral feedback message supported like:
     *@ ---------sdp--------------
     *@ a=rtcp-fb:98 ccm tstr
     *@ a=rtcp-fb:98 ccm fir
     *@ a=rtcp-fb:* ccm tmmbr smaxpr=120
     *@ ---------------------------
     *@ so need a array of rtcp_fb_type_t to represent all thest feedback messages
     *@ rtcp_fb_param_num represent the size of the array
     */
    imcf_uint32 rtcp_fb_param_num;
    vt_srv_rtcp_fb_param_type_t  rtcp_fb_type[VT_MAX_RTCP_FB_SIZE]; // SIZE  5

} vt_srv_video_capability_t;

typedef struct vt_srv_imcb_msg_vt_cap_setting_s {
    imcf_int32                  accout_id;
    imcf_int32                  video_cap_num;                   ///< number of cap.
    vt_srv_video_capability_t   video_cap[VT_MAX_VIDEO_CAP_NUM]; ///< video capability from MA
    vt_srv_h264_ps_t            h264_ps[VT_MAX_PS_NUM];                 // SIZE  32
    vt_srv_hevc_ps_t            hevc_ps[VT_MAX_PS_NUM];                 // SIZE  32
    vt_srv_video_media_bitrate_t  bitrate_info[VT_MAX_LEVEL_SIZE]; // SIZE  20

    /* do not copy to ua */
    imcf_uint8   sim_slot_id; /* start from 0 */
    imcf_uint8   pad2;
    imcf_uint16  operator_code;

    /*CAUTION: please review imcb_set_vt_cap_setting_to_ua_para() when you want to add elements here */
} vt_srv_imcb_msg_get_cap_rsp_struct, imsvt_imcb_get_cap_rsp_struct,
vt_srv_vt_cap_setting_t;

typedef struct vt_srv_imcb_msg_config_s {
    // required by RTP and RTCP session
    imcf_uint8                           call_id;        // identify this call
    imcf_uint8                           video_payload_type;    // RTP payload type
    imcf_uint8                           remote_addr_type;    ///< VoLTE_Event_IP_Version_e
    imcf_uint8                           rtp_direction;         ///< VoLTE_Event_RTP_DIRECTION_e

    imcf_uint16                          remote_rtp_port;
    imcf_uint16                          remote_rtcp_port;
    imcf_uint16                          local_rtcp_port;     ///< rtcp port number. we may configure it not rtp_port+1
    imcf_uint16                          local_rtp_port;      ///< rtp port number for media attribute in SDP message
    imcf_int8                            remote_rtp_address[VT_ADDR_LENGTH];
    imcf_int8                            remote_rtcp_address[VT_ADDR_LENGTH];
    imcf_int8                            local_address[VT_ADDR_LENGTH];
    imcf_uint32                          video_b_as;               // RTP bandwidth
    imcf_uint32                          video_b_rs;               // RTCP sender bandwidth
    imcf_uint32                          video_b_rr;               // RTCP receiver bandwidth

    imcf_uint8                           media_type; //such as video/audio
    imcf_uint8                           mime_Type; // such as H.264
    imcf_uint16                          rtcp_rsize;
    imcf_uint32                          packetize_mode;
    imcf_uint32                          sample_rate;//timescale, which use to calculte rtp timestamp
    imcf_uint32                          rtcp_fb_param_num;
    vt_srv_rtcp_fb_param_type_t          rtcp_fb_type[VT_MAX_RTCP_FB_SIZE]; // SIZE? 32

    imcf_uint8                           rtp_profile;//supported rtp_profile for special media such as RTP/AVP,RTP/AVPF
    imcf_uint8                           camera_direction;
    imcf_uint8                           bHold; //local hold
    imcf_uint8                           bHeld; //remote hold

    vt_srv_rtp_ext_map_t extmap;

    /* WFC */
    imcf_uint32                          video_dscp;
    imcf_uint32                          video_soc_priority;

    /*------ old start --------------*/
    imcf_uint16                                  pad_rtcp_rsize; /* move to new one*/
    imcf_uint16                                  rtcpfb_type; //bitmap, VoLTE_Event_RTCP_FB_Type_e
    imcf_int8                                    id[VT_MAX_SDP_FMTP_TOT_VALUE_LENGTH];
    imcf_int8                                    trr_int[VT_MAX_SDP_FMTP_TOT_VALUE_LENGTH];
    imcf_int8                                    id_app_string[VT_MAX_SDP_FMTP_TOT_VALUE_LENGTH];
    imcf_int8                                    ack_app_string[VT_MAX_SDP_FMTP_TOT_VALUE_LENGTH];
    imcf_int8                                    nack_app_string[VT_MAX_SDP_FMTP_TOT_VALUE_LENGTH];
    imcf_int8                                    tmmbr_string[VT_MAX_SDP_FMTP_TOT_VALUE_LENGTH];
    imcf_int8                                    vbcm_string[VT_MAX_SDP_FMTP_TOT_VALUE_LENGTH];
    /*------ old end --------------*/

    // require by H.264 or H265
    vt_srv_video_codec_cap_cfg_t         codec_cfg;

    /*------ old start --------------*/
    // required by H.264 codec, RFC 6184
    imcf_uint32                                  profile_level_id;       ///< H.264 profile level id. construct by 3 parts

    imcf_uint16                                  max_recv_level;         ///< require 2 bytes, reserved.
    imcf_uint8                                   packetization_mode;     ///< VoLTE_Event_Packetize_Mode_e
    imcf_uint8                                   redundant_pic_cap;    ///< 0, 1. 0: we do not support yet.

    imcf_int8                                    sprop_parameter_sets [VOLTE_MAX_SDP_PARAMETER_SET_LENGTH];
    imcf_int8                                    sprop_level_parameter_sets[VOLTE_MAX_SDP_PARAMETER_SET_LENGTH];
    imcf_uint32                                  max_mbps;               ///< reserved. VT service provides default value
    imcf_uint32                                  max_smbps;              ///< reserved. VT service provides default value
    imcf_uint32                                  max_fs ;                ///< reserved. VT service provides default value
    imcf_uint32                                  max_cpb ;               ///< reserved. VT service provides default value
    imcf_uint32                                  max_dpb ;               ///< reserved. VT service provides default value
    imcf_uint32                                  max_br ;                ///< reserved. VT service provides default value
    imcf_uint32                                  max_rcmd_nalu_size;     ///< reserved. VT service provides default value, 0: SDP will not present the value

    imcf_uint8                                   sar_understood;         ///< 0~255, reserved. VT service provides default value, 0: SDP will not present the value
    imcf_uint8                                   sar_supported;          ///< 0~255, reserved. VT service provides default value, 0: SDP will not present the value
    imcf_uint8                                   in_band_parameter_sets; ///< 0, 1. we only support 0: support SPS, PPS in SDP
    imcf_uint8                                   level_asymmetry_allowed; ///<0, 1. we only support 0: symmetry communication
    /*------ old end --------------*/
} vt_srv_imcb_msg_config_t;

typedef struct vt_srv_imcb_msg_setting {
    // 0 : inactive (VoLTE)
    // 1 : active (ViLTE)
    imcf_uint32     mode;

    imcf_uint8      ebi;
    imcf_uint8      video_type;
    imcf_uint8      pad[2];
    imcf_uint32     network_id; /*drop on 93 MD */

    // 0 : inactive
    // 1 : send_only
    // 2 : recv_only
    // 3 : send_recv

    imcf_uint32    early_media_direction;

    /*kbps, please ignore when Wifi access RAT */
    /*0: is for un-specified,  please DO NOT use this element */
    imcf_uint32  nw_assigned_ul_bw;
    imcf_uint32  nw_assigned_dl_bw;

    imcf_uint8   sim_slot_id; /* start from 0 */
    imcf_uint8   pad2[3];

    /* 196 - 16 = 180 */
    imcf_uint8   if_name[VT_MAX_IF_NAME_LENGTH];

    imcf_uint8   bearer_id;
    imcf_uint8   pdu_session_id;

    /* 180 - 2 = 178 */
    imcf_uint8   extension[178];
} vt_srv_imcb_msg_setting_t;

typedef struct vt_srv_imcb_msg_param_s {
    vt_srv_imcb_msg_setting_t   setting;
    vt_srv_imcb_msg_config_t    config;
}
  vt_srv_imcb_msg_config_init_ind_struct,   imsvt_imcb_config_init_ind_struct,
  vt_srv_imcb_msg_config_update_ind_struct, imsvt_imcb_config_update_ind_struct,
  vt_srv_imcb_msg_config_deinit_ind_struct, imsvt_imcb_config_deinit_ind_struct,
  vt_srv_imcb_msg_param_t
;

typedef struct vt_srv_imcb_msg_session_modify_s {
    imcf_uint8      call_id;    /* identify this call */

    // 0 : inactive (VoLTE)
    // 1 : active (ViLTE)
    imcf_uint8     mode;

    //  0 : inactive
    //  1 : send_only
    //  2 : recv_only
    //  3 : send_recv
    imcf_uint8     camera_direction;

    imcf_uint8     result;    /* vt_srv_imcb_session_modify_result_t */

    /* remeber to do offset to UA, UA do not contain this one */
    imcf_uint8 sim_slot_id; /* start from 0 */
    imcf_uint8 pad[3];
}

/*
    VT -- req --> UA  .... UA -- ind --> VT
    VT <- cnf --- UA  .... UA <- rsp --  VT

*/
  vt_srv_imcb_msg_session_modify_req_struct,  imsvt_imcb_modify_session_req_struct,
  vt_srv_imcb_msg_session_modify_ind_struct,  imsvt_imcb_modify_session_ind_struct,
  vt_srv_imcb_msg_session_modify_rsp_struct,  imsvt_imcb_modify_session_rsp_struct,
  vt_srv_imcb_msg_session_modify_cnf_struct,  imsvt_imcb_modify_session_cnf_struct;

typedef struct vt_srv_imcb_msg_session_modify_cancel_req_s {
    imcf_uint8      call_id;    /* identify this call */
    /* remeber to do offset to UA, UA do not contain this one */
    imcf_uint8 sim_slot_id; /* start from 0 */
    imcf_uint8 cause;
    imcf_uint8 pad[5];
}vt_srv_imcb_msg_session_modify_cancel_req_struct,imsvt_imcb_modify_session_cancel_req_struct;

typedef struct vt_srv_imcb_msg_session_modify_cancel_cnf_s {
    imcf_uint8      call_id;    /* identify this call */
    /* remeber to do offset to UA, UA do not contain this one */
    imcf_uint8 sim_slot_id; /* start from 0 */
    imcf_uint8 result;
    imcf_uint8 pad[5];
}vt_srv_imcb_msg_session_modify_cancel_cnf_struct,imsvt_imcb_modify_session_cancel_cnf_struct;

typedef struct vt_srv_imcb_msg_session_modify_cancel_s {
    imcf_uint8      call_id;    /* identify this call */
    /* remeber to do offset to UA, UA do not contain this one */
    imcf_uint8 sim_slot_id; /* start from 0 */
    imcf_uint8 pad[2];
}
vt_srv_imcb_msg_session_modify_cancel_ind_struct,imsvt_imcb_modify_session_cancel_ind_struct,
vt_srv_imcb_msg_session_modify_local_bw_ready_ind_struct,imsvt_imcb_modify_session_local_bw_ready_ind_struct;


typedef struct vt_srv_imcb_msg_mod_bw_req_s {
    imcf_uint8 call_id;    /* identify this call */
    imcf_uint8 pad[3];

    imcf_uint32 video_bandwidth;

    imcf_uint8 sim_slot_id; /* start from 0 */
    imcf_uint8 pad2[3];
} vt_srv_imcb_msg_mod_bw_req_t, imsvt_imcb_mod_bw_req_struct;

typedef struct vt_srv_imcb_msg_call_progress_ind_s {
    imcf_uint8 call_id;    /* identify this call */
    /* remeber to do offset to UA, UA do not contain this one */
    imcf_uint8 sim_slot_id; /* start from 0 */
    imcf_uint8 call_mode;
    imcf_uint8 pad1;

    imcf_uint32 ecpi;
    imcf_uint32 reserved;
} vt_srv_imcb_msg_call_progress_ind_struct, imsvt_imcb_call_progress_ind_struct;

typedef struct {
    imcf_uint32 error_num;
} imsvt_imcb_comm_error_struct;

typedef struct {
    imcf_uint8 token;
    imcf_uint8 pad[3];
} imsvt_imcb_loop_test_ind_struct;

typedef struct {
    imcf_uint8 token;
    imcf_uint8 pad[3];
} imsvt_imcb_loop_test_rsp_struct;

typedef enum vt_srv_imcb_rat_type_e {
    VT_SRV_IMCB_RAT_TYPE_NONE = 0,
    VT_SRV_IMCB_RAT_TYPE_2G = VT_SRV_IMCB_RAT_TYPE_NONE,
    VT_SRV_IMCB_RAT_TYPE_3G_FDD,
    VT_SRV_IMCB_RAT_TYPE_3G_TDD,
    VT_SRV_IMCB_RAT_TYPE_4G_FDD,
    VT_SRV_IMCB_RAT_TYPE_4G_TDD,

    /* WFC */
    VT_SRV_IMCB_RAT_TYPE_802_11,
    VT_SRV_IMCB_RAT_TYPE_802_11a,
    VT_SRV_IMCB_RAT_TYPE_802_11b,
    VT_SRV_IMCB_RAT_TYPE_802_11g,
    VT_SRV_IMCB_RAT_TYPE_802_11n,
    VT_SRV_IMCB_RAT_TYPE_802_11ac,

    /* C2K */
    VT_SRV_IMCB_RAT_TYPE_1xRTT,
    VT_SRV_IMCB_RAT_TYPE_HRPD,
    VT_SRV_IMCB_RAT_TYPE_eHRPD,

    VT_SRV_IMCB_RAT_TYPE_MAX = VT_SRV_IMCB_RAT_TYPE_eHRPD,
    VT_SRV_IMCB_RAT_TYPE_UNSPEC = 0x80
} vt_srv_imcb_rat_type;


typedef struct {
    imcf_uint8           source_rat_type;  /* vt_srv_imcb_rat_type enum */
    imcf_uint8           target_rat_type;  /* vt_srv_imcb_rat_type enum */
    imcf_uint8           pad[2];

    imcf_uint8           sim_slot_id; /* start from 0 */
    imcf_uint8           pad2[3];
} vt_srv_imcb_handover_start_ind_struct, imsvt_imcb_handover_start_ind_struct;

typedef struct {
    imcf_uint8            source_rat_type;  /* vt_srv_imcb_rat_type enum */
    imcf_uint8            target_rat_type;  /* vt_srv_imcb_rat_type enum */
    imcf_uint8            is_success;
    imcf_uint8            pad[1];

    imcf_uint8            sim_slot_id; /* start from 0 */
    imcf_uint8            pad2[3];
} vt_srv_imcb_handover_stop_ind_struct, imsvt_imcb_handover_stop_ind_struct;

typedef struct {
    imcf_uint8 source_rat_type; /* vt_srv_imcb_rat_type enum */
    imcf_uint8 target_rat_type; /* vt_srv_imcb_rat_type enum */
    imcf_uint8 sim_slot_id; /* start from 0 */
    imcf_uint8 pad;
    imcf_uint32 network_id; /* drop on 93MD */
} vt_srv_imcb_pdn_handover_begin_ind_struct, imsvt_imcb_pdn_handover_begin_ind_struct;

typedef struct {
    imcf_uint8 source_rat_type; /* vt_srv_imcb_rat_type enum */
    imcf_uint8 target_rat_type; /* vt_srv_imcb_rat_type enum */
    imcf_uint8 is_success;
    imcf_uint8 sim_slot_id; /* start from 0 */
    imcf_uint32 network_id; /* drop on 93MD */
} vt_srv_imcb_pdn_handover_end_ind_struct, imsvt_imcb_pdn_handover_end_ind_struct;

#endif
