/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
#ifndef VT_DEF_H
#define VT_DEF_H

// for common usage
#include <android/log.h>                                        /* for __android_log_print usage */
#include <utils/RefBase.h>
#include <utils/Vector.h>
#include <Surface.h>

// for 4G VT
#include "ImsMa.h"
#include "imsvt_imcb_sap.h"
#include <media/stagefright/foundation/ALooper.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/AHandler.h>

// for HiDL
#include <vendor/mediatek/hardware/videotelephony/1.0/IVideoTelephony.h>

using namespace android;

// for HiDL
using ::android::hidl::base::V1_0::DebugInfo;
using ::android::hidl::base::V1_0::IBase;
using ::vendor::mediatek::hardware::videotelephony::V1_0::IVideoTelephony;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;

namespace VTService {

class VTCore;
class MsgHandler;
class VTMALStub;
class VTAVSync;
class VideoProfile;

#define MIN(a, b)                                               ((a) < (b) ? (a) : (b))
#define MAX(a, b)                                               ((a) < (b) ? (b) : (a))

#define HAS_ANDROID_OS                                          1
#define VTSurface                                               Surface

#define VT_IMCB_CAPIND                                          vt_srv_imcb_msg_get_cap_ind_struct
#define VT_IMCB_CAP                                             vt_srv_imcb_msg_get_cap_rsp_struct
#define VT_IMCB_BW                                              vt_srv_imcb_msg_mod_bw_req_t

#define VT_IMCB_CONFIG                                          vt_srv_imcb_msg_param_t
#define VT_IMCB_INIT                                            vt_srv_imcb_msg_config_init_ind_struct
#define VT_IMCB_UPD                                             vt_srv_imcb_msg_config_update_ind_struct
#define VT_IMCB_DEINIT                                          vt_srv_imcb_msg_config_deinit_ind_struct

#define VT_IMCB_REQ                                             vt_srv_imcb_msg_session_modify_req_struct
#define VT_IMCB_IND                                             vt_srv_imcb_msg_session_modify_ind_struct
#define VT_IMCB_RSP                                             vt_srv_imcb_msg_session_modify_rsp_struct
#define VT_IMCB_CNF                                             vt_srv_imcb_msg_session_modify_cnf_struct

#define VT_IMCB_CANCEL_REQ                                      vt_srv_imcb_msg_session_modify_cancel_req_struct
#define VT_IMCB_CANCEL_IND                                      vt_srv_imcb_msg_session_modify_cancel_ind_struct
#define VT_IMCB_CANCEL_CNF                                      vt_srv_imcb_msg_session_modify_cancel_cnf_struct
#define VT_IMCB_UPGRADE_BW_READY_IND                            vt_srv_imcb_msg_session_modify_local_bw_ready_ind_struct

#define VT_IMCB_HOSTART                                         vt_srv_imcb_handover_start_ind_struct
#define VT_IMCB_HOSTOP                                          vt_srv_imcb_handover_stop_ind_struct
#define VT_IMCB_PDN_HOBEGIN                                     vt_srv_imcb_pdn_handover_begin_ind_struct
#define VT_IMCB_PDN_HOEND                                       vt_srv_imcb_pdn_handover_end_ind_struct

#define VT_IMCB_CALL_PROGRESS_IND                               vt_srv_imcb_msg_call_progress_ind_struct

#define VT_IVD                                                  (-1)
#define VT_SRV_ZERO                                             (0)
#define VT_SRV_MA_NR                                            (5)
#define VT_SRV_SIM_NR                                           (5)
#define VT_SRV_STR_LEN                                          (1024)
#define VT_SRV_DEBUG_ON                                         (1)
#define VT_RETRY_COUNT                                          (256)

#define VT_DIR_INACTIVE                                         (0)
#define VT_DIR_SEND_ONLY                                        (1)
#define VT_DIR_RECV_ONLY                                        (2)
#define VT_DIR_SEND_RECV                                        (3)

#define MA_DOWNGRADE                                            (4)

#define TAG_VILTE_MOBILE                                        (0xFF000000)
#define TAG_VILTE_WIFI                                          (0xFF100000)

#define RFX_UNUSED(x)                                           ((void)(x))

#define POSTEVENT_IGNORE_ID                                     (-10)

#define VT_HIDL_READ_WRITE_WAITTING_TIME                        (0) // o mean wait forever

#define VT_SIM_ID_ABSENT                                        (-1)
#define VT_DEFAULT_OP_ID                                        (8)
#define VT_NO_MATCHED_OP_ID                                     (-999)

#define VT_INVALID_NETWORK_ID                                   (0)
#define VT_INVALID_INDEX                                        (-1)

// notify msg enum
const int VT_SRV_NOTIFY_RECEIVE_FIRSTFRAME                      = 1001;
const int VT_SRV_NOTIFY_SNAPSHOT_DONE                           = 1002;
const int VT_SRV_NOTIFY_REC_INFO_UNKNOWN                        = 1003;
const int VT_SRV_NOTIFY_REC_INFO_REACH_MAX_DURATION             = 1004;
const int VT_SRV_NOTIFY_REC_INFO_REACH_MAX_FILESIZE             = 1005;
const int VT_SRV_NOTIFY_REC_INFO_NO_I_FRAME                     = 1006;
const int VT_SRV_NOTIFY_REC_INFO_COMPLETE                       = 1007;
const int VT_SRV_NOTIFY_CALL_END                                = 1008;

/* 3G part start *//* no use but keep */
const int VT_SRV_NOTIFY_CALL_ABNORMAL_END                       = 1009;
const int VT_SRV_START_COUNTER                                  = 1010;
const int VT_SRV_PEER_CAMERA_OPEN                               = 1011;
const int VT_SRV_PEER_CAMERA_CLOSE                              = 1012;
/* 3G part end */

const int VT_SRV_NOTIFY_BW_READY_IND                            = 1013;


const int VT_SRV_NOTIFY_RECV_SESSION_CONFIG_REQ                 = 4001;
const int VT_SRV_NOTIFY_RECV_SESSION_CONFIG_RSP                 = 4002;
const int VT_SRV_NOTIFY_HANDLE_CALL_SESSION_EVT                 = 4003;
const int VT_SRV_NOTIFY_PEER_SIZE_CHANGED                       = 4004;
const int VT_SRV_NOTIFY_LOCAL_SIZE_CHANGED                      = 4005;
const int VT_SRV_NOTIFY_DATA_USAGE_CHANGED                      = 4006;
const int VT_SRV_NOTIFY_CAM_CAP_CHANGED                         = 4007;
const int VT_SRV_NOTIFY_BAD_DATA_BITRATE                        = 4008;
const int VT_SRV_NOTIFY_DATA_BITRATE_RECOVER                    = 4009;
const int VT_SRV_NOTIFY_RECV_ENHANCE_SESSION_IND                = 4010;
const int VT_SRV_NOTIFY_DATA_CHANGE_PAUSE                       = 4011;
const int VT_SRV_NOTIFY_DATA_CHANGE_RESUME                      = 4012;
const int VT_SRV_NOTIFY_DEFAULT_LOCAL_SIZE                      = 4013;
const int VT_SRV_NOTIFY_GET_CAP                                 = 4014;
const int VT_SRV_NOTIFY_LOCAL_BUFFER                            = 4015;
const int VT_SRV_NOTIFY_UPLINK_STATE_CHANGE                     = 4016;
const int VT_SRV_NOTIFY_RESTART_CAMERA                          = 4017;
const int VT_SRV_NOTIFY_GET_SENSOR_INFO                         = 4018;
const int VT_SRV_NOTIFY_GET_CAP_WITH_SIM                        = 4019;
const int VT_SRV_NOTIFY_PACKET_LOSS_RATE                        = 4020;

const int VT_SRV_ERROR_BASE                                     = 8000;
const int VT_SRV_ERROR_SERVICE                                  = 8001;
const int VT_SRV_ERROR_SERVER_DIED                              = 8002;
const int VT_SRV_ERROR_CAMERA_CRASHED                           = 8003;
const int VT_SRV_ERROR_CODEC                                    = 8004;
const int VT_SRV_ERROR_REC                                      = 8005;
const int VT_SRV_ERROR_CAMERA_SET_IGNORED                       = 8006;
const int VT_SRV_ERROR_BIND_PORT                                = 8007;

// only for native layer event, not to Provider
const int VT_SRV_NOTIFY_AVPF_TMMBR_MBR_DL                       = 9001;

const int VT_SRV_NOTIFY_SET_ANBR                                = 9002;

//msg between VTS and L4
const int MSG_ID_MD_BEGIN                                       = MSG_ID_WRAP_IMSVT_IMCB_BEGIN + 1000;
const int MSG_ID_MD_ANBR_CONFIG_UPDATE_IND                      = MSG_ID_MD_BEGIN + 1;
const int MSG_ID_MD_ANBR_REPORT_REQ                             = MSG_ID_MD_BEGIN + 2;
const int MSG_ID_MD_INTER_RAT_STATUS_IND                        = MSG_ID_MD_BEGIN + 3;

// log function define
#define VT_LOGD(...) __android_log_print(ANDROID_LOG_DEBUG  , "VT", __VA_ARGS__)
#define VT_LOGV(...) //__android_log_print(ANDROID_LOG_VERBOSE, "VT", __VA_ARGS__)
#define VT_LOGI(...) __android_log_print(ANDROID_LOG_INFO   , "VT", __VA_ARGS__)
#define VT_LOGW(...) __android_log_print(ANDROID_LOG_WARN   , "VT", __VA_ARGS__)
#define VT_LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , "VT", __VA_ARGS__)

// Get SIM Id / Call Id for support dual IMS
// For the construct id (int), first 2 bytes are SIM Id, the last 2 bytes are call id
#define CONSTRUCT_SIM_CALL_ID(SID,CID)                          ((SID) << 16 | (CID))
#define GET_SIM_ID(ID)                                          (((ID) >> 16) & 0xffff)
#define GET_CALL_ID(ID)                                         ((ID) & 0xffff)

// =====================================================================================
// Enum for Common
// ====================================================================================
typedef enum {
    VT_SRV_LOG_E,
    VT_SRV_LOG_W,
    VT_SRV_LOG_I,
    VT_SRV_LOG_V,
    VT_SRV_LOG_D,
}VT_SRV_LOG_LV;

typedef enum {
    VT_SRV_RET_OK                       = 0,
    VT_SRV_RET_ERR_NO_MA                = 1,
    VT_SRV_RET_ERR_STATE                = 2,
    VT_SRV_RET_ERR_FULL                 = 3,
    VT_SRV_RET_ERR_EMPTY                = 4,
    VT_SRV_RET_ERR_ALREADY_EXIST        = 5,
    VT_SRV_RET_ERR_NOT_EXIST            = 6,
    VT_SRV_RET_ERR_INVALID_PARAM        = 7,
    VT_SRV_RET_ERR_NO_CLIENT            = 8,
    VT_SRV_RET_ERR_MA                   = 9,
    VT_SRV_RET_ERR_BIND_PORT            = 10,
    VT_SRV_RET_ERR                      = 11,
    VT_SRV_RET_END
} VT_SRV_RET;

typedef enum {
    VT_FALSE = 0,
    VT_TRUE
} VT_BOOL;

typedef enum {
    VT_SRV_CALL_NONE,
    VT_SRV_CALL_3G,
    VT_SRV_CALL_4G,
    VT_SRV_CALL_END
} VT_SRV_CALL_MODE;

typedef enum {
    VT_SRV_SNAPSHOT_LOCAL,
    VT_SRV_SNAPSHOT_PEER,
    VT_SRV_SNAPSHOT_LOCAL_PEER,
    VT_SRV_SNAPSHOT_END
}VT_SRV_SNAPSHOT_TYPE;

typedef enum {
    VT_SRV_RECORD_VIDEO,
    VT_SRV_RECORD_VIDEO_AUDIO,
    VT_SRV_RECORD_END
}VT_SRV_RECORD_TYPE;

typedef enum {
    VT_SRV_HIDE_ME_DISABLE,
    VT_SRV_HIDE_ME_ENABLE,
    VT_SRV_HIDE_ME_FREEZE,
    VT_SRV_HIDE_ME_PICTURE,
    VT_SRV_HIDE_ME_END
}VT_SRV_HIDE_ME_TYPE;

typedef enum {
    VT_SRV_HIDE_YOU_DISABLE,
    VT_SRV_HIDE_YOU_ENABLE,
    VT_SRV_HIDE_YOU_END
}VT_SRV_HIDE_YOU_TYPE;

typedef enum {
    VT_SRV_UI_MODE_FG,
    VT_SRV_UI_MODE_BG,
    VT_SRV_UI_MODE_FULL_SCREEN,
    VT_SRV_UI_MODE_NORMAL_SCREEN,
    VT_SRV_UI_MODE_END
}VT_SRV_UI_MODE;

typedef enum {
    VT_SRV_VIDEO_STATE_AUDIO_ONLY       = 0x0,
    VT_SRV_VIDEO_STATE_TX_ENABLED       = 0x1,
    VT_SRV_VIDEO_STATE_RX_ENABLED       = 0x2,
    VT_SRV_VIDEO_STATE_BIDIRECTIONAL    = 0x3,
    VT_SRV_VIDEO_STATE_PAUSED           = 0x4,
    VT_SRV_VIDEO_STATE_CANCEL           = 0x00010000,
    VT_SRV_VIDEO_STATE_CANCEL_TIMEOUT   = 0x00010000 << 1,
    VT_SRV_VIDEO_STATE_END
}VT_SRV_VIDEO_STATE;

typedef enum {
    VT_SRV_VIDEO_QTY_HIGH               = 1,
    VT_SRV_VIDEO_QTY_MEDIUM             = 2,
    VT_SRV_VIDEO_QTY_LOW                = 3,
    VT_SRV_VIDEO_QTY_DEFAULT            = 4,
    VT_SRV_VIDEO_QTY_END
}VT_SRV_VIDEO_QTY;

typedef enum {
    VT_SRV_SURFACE_STATE_INIT           = 0x0,
    VT_SRV_SURFACE_STATE_LOCAL          = 0x1,
    VT_SRV_SURFACE_STATE_PEER           = 0x2,
    VT_SRV_SURFACE_STATE_PEER_LOCAL     = 0x3,
    VT_SRV_SURFACE_STATE_END,
}VT_SRV_SURFACE_STATE;

typedef enum {
    VT_SRV_MA_STATE_STOPED,    /* 0 */
    VT_SRV_MA_STATE_OPENED,    /* 1 */
    VT_SRV_MA_STATE_INITED,    /* 2 */
    VT_SRV_MA_STATE_PRE_START, /* 3 */
    VT_SRV_MA_STATE_START_IN,  /* 4 */
    VT_SRV_MA_STATE_START_NI,  /* 5 */
    VT_SRV_MA_STATE_START_ID,  /* 6 */
    VT_SRV_MA_STATE_START_UI,  /* 7 */
    VT_SRV_MA_STATE_START_NN,  /* 8 */
    VT_SRV_MA_STATE_START_ND,  /* 9 */
    VT_SRV_MA_STATE_START_UN,  /* 10 */
    VT_SRV_MA_STATE_START_UD,  /* 11 */
    VT_SRV_MA_STATE_PRE_STOP,  /* 12 */
    VT_SRV_MA_STATE_STOPED_U,  /* 13 */
    VT_SRV_MA_STATE_STOPED_D,  /* 14 */
    VT_SRV_MA_STATE_VOICE_STOPED_U,  /* 15 */
    VT_SRV_MA_STATE_VOICE_STOPED_D,  /* 16 */
    VT_SRV_MA_STATE_END,
}VT_SRV_MA_STATE;

typedef enum {
	VT_SRV_MAL_STATE_OPENED,
	VT_SRV_MAL_STATE_INITED,
	VT_SRV_MAL_STATE_STARTED,
	VT_SRV_MAL_STATE_ERROR
}VT_SRV_MAL_STATE;

typedef enum {
    VT_SRV_DATA_PATH_NONE            = 0x0,
    VT_SRV_DATA_PATH_SOURCE          = 0x1,
    VT_SRV_DATA_PATH_SINK            = 0x2,
    VT_SRV_DATA_PATH_SOURCE_SINK     = 0x3,
    VT_SRV_DATA_PATH_END
}VT_SRV_DATA_PATH;

typedef enum {
    VT_SRV_PARAM_CALL_MODE,
    VT_SRV_PARAM_UA_CONFIG,
    VT_SRV_PARAM_MA_CONFIG,
    VT_SRV_PARAM_SURFACE_STATE,
    VT_SRV_PARAM_SESSION_REQ,
    VT_SRV_PARAM_REMOTE_SESSION_REQ,
    VT_SRV_PARAM_PEER_UI_CONFIG,
    VT_SRV_PARAM_LOCAL_UI_CONFIG,
    VT_SRV_PARAM_IS_FORCE_STOP,
    VT_SRV_PARAM_IS_MA_CRASH,
    VT_SRV_PARAM_UPDATE_INFO,
    VT_SRV_PARAM_CANCEL_SESSION_REQ,
    VT_SRV_PARAM_IS_FORCE_CANCEL,
    VT_SRV_PARAM_IS_RECV_UPDATE,
    VT_SRV_PARAM_CALL_STATE,
    VT_SRV_PARAM_IS_DURING_EARLY_MEDIA,
    VT_SRV_PARAM_END
}VT_SRV_PARAM;

typedef enum {
    VT_SRV_SESSION_MODIFY_SUCCESS = 1,
    VT_SRV_SESSION_MODIFY_FAIL = 2,
    VT_SRV_SESSION_MODIFY_INVALID = 3,
    VT_SRV_SESSION_MODIFY_TIMED_OUT = 4,
    VT_SRV_SESSION_MODIFY_REJECTED_BY_REMOTE = 5,
    VT_SRV_SESSION_MODIFY_END
}VT_SRV_SESSION_CNF;

typedef enum {
    VT_SRV_MESSAGE_DISPATCH = 0,
    VT_SRV_MESSAGE_HANDLING = 1,
    VT_SRV_MESSAGE_END
}VT_SRV_NESSAGE_TYPE;

typedef enum {
    VT_SRV_VTCAM_STATE_STOP_REC             = 0x0,
    VT_SRV_VTCAM_STATE_START_REC            = 0x1,
    VT_SRV_VTCAM_STATE_PAUSE_REC            = 0x2,
    VT_SRV_VTCAM_STATE_RESUME_REC           = 0x3,
    VT_SRV_VTCAM_STATE_STOP_REC_AND_PREVIEW = 0x4,
}VT_SRV_VTCAM_STATE;

//define same to ECPI value
typedef enum {
    VT_CALL_STATE_UNKNOWN          = -1,
    VT_CALL_STATE_SETUP            = 0,
    VT_CALL_STATE_ALERT            = 2,
    VT_CALL_STATE_CONNECTED        = 6,
    VT_CALL_STATE_HELD             = 131,
    VT_CALL_STATE_ACTIVE           = 132,
    VT_CALL_STATE_DISCONNETED      = 133,
    VT_CALL_STATE_MO_DISCONNECTING = 134,
    VT_CALL_STATE_REMOTE_HOLD      = 135,
    VT_CALL_STATE_REMOTE_RESUME    = 136,
}VT_SRV_CALL_STATE;

typedef enum {
    VT_SRV_SESSION_IND_CANCEL      = 0,
    VT_SRV_SESSION_IND_EARLY_MEDIA = 1,
} VT_SRV_ENHANCE_SESSION_IND_TYPE;

// =====================================================================================
// Structure for Common
// =====================================================================================
typedef struct {
    int                                                 mWidth;
    int                                                 mHeight;
    int                                                 mRotation;
}vt_srv_call_ui_config_struct;

typedef struct {
    int                                                 mNetworkId;
    char                                                mIfName[VT_MAX_IF_NAME_LENGTH];
}vt_srv_network_info_struct;

typedef struct {
    int                                                 mIsHold;
    int                                                 mIsResume;
    int                                                 mIsHeld;
    int                                                 mIsResumed;

    int                                                 mIsTxInPause;
    int                                                 mIsRxInPause;

    int                                                 mVdoRtpPort;
    int                                                 mVdoRtcpPort;
}vt_srv_call_update_info_struct;

typedef struct {
    int                                                 mId;
    int                                                 mSimId;
    sp<VTMALStub>                                       mPtr;
    VT_SRV_CALL_MODE                                    mMode;

    VT_SRV_MA_STATE                                     mState;

    VT_IMCB_CONFIG*                                     mUaVideoConfig;

    media_config_t*                                     mMaConfig;

    VT_SRV_SURFACE_STATE                                mSetSurfaceState;

    VT_IMCB_REQ                                         mSessionReq;
    sp<VideoProfile>                                    mSessionReqProfile;
    VT_IMCB_IND                                         mRemoteSessionReq;
    VT_IMCB_CANCEL_REQ                                  mSessionCancelReq;

    VT_BOOL                                             mIsForceCancel;
    vt_srv_call_ui_config_struct                        mPeerUiConfig;
    vt_srv_call_ui_config_struct                        mLocalUiConfig;

    bool                                                mIsForceStop;
    bool                                                mIsMaCrash;

    vt_srv_call_update_info_struct                      mUpdateInfo;

    VT_BOOL                                             mHasReceiveInit;

    VT_BOOL                                             mIsRecvUpdate;

    int                                                 mCallState;

    VT_BOOL                                             mIsDuringEarlyMedia;
}vt_srv_call_table_entry_struct;

typedef struct {
    int                                                 mUsedSize;
    vt_srv_call_table_entry_struct                      mTable[VT_SRV_MA_NR];
    int                                                 mUsed[VT_SRV_MA_NR];
    int                                                 mDefaultLocalW;
    int                                                 mDefaultLocalH;
    int                                                 mSimOpTable[VT_SRV_SIM_NR];
    int                                                 mSimHandoverTable[VT_SRV_SIM_NR];
    bool                                                mIsSetSensorInfo;
    Vector<vt_srv_network_info_struct>                  mNetworkInfoTable;
}vt_srv_call_table_struct;

typedef struct {
    int config;
    int ebi;
    int is_ul;
    int bitrate;
    int bearer_id;
    int pdu_session_id;
    int ext_param;
}vt_srv_anbr_struct;

typedef struct {
    int call_id;
    int sim_slot_id;
    vt_srv_anbr_struct anbr_config;
}vt_srv_ril_msg_anbr_struct;

#define VT_ANBR_CONFIG                                          vt_srv_anbr_struct
#define VT_ANBR_REQ                                             vt_srv_ril_msg_anbr_struct

typedef struct {
    int sim_slot_id;
    int irat_status;
    int is_successful;
}vt_srv_irat_struct;

#define VT_IRAT_STRUCT                                          vt_srv_irat_struct

typedef struct {
    sp<VTCore>                                          core;

    sp<VTAVSync>                                        avSync;
    sp<IVideoTelephony>                                 hidlService;

    VT_IMCB_BW                                          ma_bw;
}vt_srv_cntx_struct;

/* =========================================================================== */
/* VT / VT HiDL connection related interface                               */
/* =========================================================================== */
typedef void (*VT_Callback) (int type, void *data, int len);
int VT_Connect(VT_Callback cb);
int VT_Send(int target, int type, void *data, int len);
/* =========================================================================== */

}  // namespace VTService

#endif
