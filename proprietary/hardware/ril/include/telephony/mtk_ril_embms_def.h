/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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


#ifndef MTK_RIL_EMBMS_DEF_H
#define MTK_RIL_EMBMS_DEF_H 1

#ifdef __cplusplus
extern "C" {
#endif

/// M: eMBMS feature
#define EMBMS_IFACE_ID  7+1
#define EMBMS_MAX_NUM_EMSLUI      435
#define EMBMS_MAX_LEN_INTERFACE 10
#define EMBMS_MAX_LEN_TMGI 12  // 12 digits plain text according to 3GPP 23.003
#define EMBMS_LEN_SESSION_IDX 2
#define EMBMS_MAX_BYTES_TMGI 6  // 6 bytes according to 3GPP 23.246
#define EMBMS_MAX_NUM_SAI   64
#define EMBMS_MAX_NUM_FREQ  5
#define EMBMS_MAX_NUM_NEIGHBOR_FREQ  8
#define EMBMS_MAX_NUM_SESSIONINFO  29

typedef struct {
    uint32_t tmgi_len;
    uint8_t tmgi[EMBMS_MAX_BYTES_TMGI];
}embms_tmgi_info_type;

typedef struct RIL_EMBMS_LocalEnableResp {
    int32_t trans_id;
    int32_t response;
    uint8_t interface_index_valid;
    uint32_t interface_index;
    char    interface_name[EMBMS_MAX_LEN_INTERFACE + 1];
}RIL_EMBMS_LocalEnableResp;

typedef struct RIL_EMBMS_LocalDisableResp {
    int32_t trans_id;
    int32_t response;
}RIL_EMBMS_LocalDisableResp;

typedef struct RIL_EMBMS_LocalGetTimeResp {
    int32_t     trans_id;
    int32_t     response;
    uint64_t    milli_sec;
    uint8_t     day_light_saving_valid;
    uint8_t     day_light_saving;
    uint8_t     leap_seconds_valid;
    int8_t      leap_seconds;
    uint8_t     local_time_offset_valid;
    int8_t      local_time_offset;
} RIL_EMBMS_LocalGetTimeResp;

typedef struct RIL_EMBMS_LocalStartSessionReq {
    int32_t     trans_id;
    char        tmgi[EMBMS_MAX_LEN_TMGI+1];
    int32_t     session_id;
    uint32_t    earfcnlist_count;
    int32_t     earfcnlist[EMBMS_MAX_NUM_FREQ];
    uint32_t    saiList_count;
    int32_t     saiList[EMBMS_MAX_NUM_SAI];
} RIL_EMBMS_LocalStartSessionReq;

typedef struct RIL_EMBMS_LocalStopSessionReq {
    int32_t     stop_type;
    int32_t     trans_id;
    char        tmgi[EMBMS_MAX_LEN_TMGI+1];
    int32_t     session_id;
} RIL_EMBMS_LocalStopSessionReq;

typedef struct RIL_EMBMS_LocalStartSessionResp {
    int32_t trans_id;
    int32_t response;
    uint8_t tmgi_info_valid;
    char    tmgi[EMBMS_MAX_LEN_TMGI+1];
} RIL_EMBMS_LocalStartSessionResp;

typedef struct RIL_EMBMS_LocalOosNotify {
    uint32_t reason;
    uint32_t tmgi_info_count;
    char    tmgix[EMBMS_MAX_NUM_SESSIONINFO][EMBMS_MAX_LEN_TMGI+1];
} RIL_EMBMS_LocalOosNotify;

typedef struct RIL_EMBMS_LocalSaiNotify {
    uint32_t cf_total;
    int32_t curFreqData[EMBMS_MAX_NUM_NEIGHBOR_FREQ];
    uint32_t csai_total;
    int32_t curSaiData[EMBMS_MAX_NUM_SAI];
    uint32_t nf_total;
    int32_t neiFreqData[EMBMS_MAX_NUM_NEIGHBOR_FREQ];
    uint32_t nsai_count_per_group[EMBMS_MAX_NUM_NEIGHBOR_FREQ];
    uint32_t nsai_total;
    int32_t neiSaiData[EMBMS_MAX_NUM_SAI];
}RIL_EMBMS_LocalSaiNotify;

typedef struct RIL_EMBMS_LocalSessionNotify {
    uint32_t tmgi_info_count;
    char    tmgix[EMBMS_MAX_NUM_SESSIONINFO][EMBMS_MAX_LEN_TMGI+1];
    char    session_idx[EMBMS_MAX_NUM_SESSIONINFO][EMBMS_LEN_SESSION_IDX+1];
} RIL_EMBMS_LocalSessionNotify;
/*****************************************************************************
 * also used by rilproxy
 *****************************************************************************/

typedef struct RIL_EMBMS_EnableResp {
    int32_t trans_id;
    int32_t response;
    uint8_t interface_index_valid;
    uint32_t interface_index;
}RIL_EMBMS_EnableResp;

typedef struct RIL_EMBMS_DisableResp {
    int32_t trans_id;
    int32_t response;
}RIL_EMBMS_DisableResp;

typedef struct RIL_EMBMS_StartSessionReq {
    int32_t     trans_id;
    embms_tmgi_info_type    tmgi_info;
    uint32_t    earfcnlist_count;
    int32_t     earfcnlist[EMBMS_MAX_NUM_FREQ];
    uint8_t     saiList_valid;
    uint32_t    saiList_count;
    int32_t     saiList[EMBMS_MAX_NUM_SAI];
} RIL_EMBMS_StartSessionReq;

typedef struct RIL_EMBMS_StopSessionReq {
    int32_t     trans_id;
    embms_tmgi_info_type    tmgi_info;
} RIL_EMBMS_StopSessionReq;

typedef struct RIL_EMBMS_StartSessionResp {
    int32_t trans_id;
    int32_t response;
    uint8_t tmgi_info_valid;
    embms_tmgi_info_type tmgi_info;
} RIL_EMBMS_StartSessionResp;

typedef struct RIL_EMBMS_GetCoverageResp {
    int32_t trans_id;
    int32_t response;
    uint8_t coverage_state_valid;
    uint32_t coverage_state;
} RIL_EMBMS_GetCoverageResp;

typedef struct RIL_EMBMS_GetTimeResp {
    int32_t     trans_id;
    int32_t     response;
    uint64_t    milli_sec;
    uint8_t     day_light_saving_valid;
    uint8_t     day_light_saving;
    uint8_t     leap_seconds_valid;
    int8_t      leap_seconds;
    uint8_t     local_time_offset_valid;
    int8_t      local_time_offset;
} RIL_EMBMS_GetTimeResp;

typedef struct RIL_EMBMS_OosNotify {
    int32_t trans_id;
    uint32_t reason;
    uint32_t tmgi_info_count;
    embms_tmgi_info_type tmgi_info[EMBMS_MAX_NUM_SESSIONINFO];
} RIL_EMBMS_OosNotify;

typedef struct RIL_EMBMS_SaiNotify {
    int32_t trans_id;
    uint32_t cf_total;
    int32_t curFreqData[EMBMS_MAX_NUM_NEIGHBOR_FREQ];
    int32_t csai_count_per_group[EMBMS_MAX_NUM_NEIGHBOR_FREQ];
    uint32_t csai_total;
    int32_t curSaiData[EMBMS_MAX_NUM_SAI];
    uint32_t nf_total;
    int32_t neiFreqData[EMBMS_MAX_NUM_NEIGHBOR_FREQ];
    uint32_t nsai_count_per_group[EMBMS_MAX_NUM_NEIGHBOR_FREQ];
    uint32_t nsai_total;
    int32_t neiSaiData[EMBMS_MAX_NUM_SAI];
} RIL_EMBMS_SaiNotify;

typedef struct RIL_EMBMS_CellInfoNotify {
    int32_t trans_id;
    int32_t cell_id;
} RIL_EMBMS_CellInfoNotify;

typedef struct RIL_EMBMS_ModemEeNotify {
    int32_t trans_id;
    int32_t state;
} RIL_EMBMS_ModemEeNotify;

typedef struct RIL_EMBMS_ActiveSessionNotify {
    int32_t trans_id;
    uint32_t tmgi_info_count;
    embms_tmgi_info_type tmgi_info[EMBMS_MAX_NUM_SESSIONINFO];
} RIL_EMBMS_ActiveSessionNotify;

/*****************************************************************************
 * ENUM, also used by rild
 *****************************************************************************/

#define INVALID_EMBMS_SESSION_ID -1

typedef enum {
    EMBMS_GENERAL_SUCCESS = 0x00,
    EMBMS_GENERAL_FAIL = 0x01,
}embms_general_error;

typedef enum {
    EMBMS_COMMAND_ATCI = 0,
    EMBMS_COMMAND_RIL = 1,
}embms_command_enum;

typedef enum {
    EMBMS_DEACTIVE_SESSION = 0,
    EMBMS_ACTIVE_SESSION = 1,
    EMBMS_DEACTIVE_ALL_SESSION = 2,
}embms_at_activesession_enum;

typedef enum {
    EMBMS_SESSION_SUCCESS = 0x00,
    EMBMS_SESSION_UNKNOWN_ERROR = 0x01,
    EMBMS_SESSION_ALREADY_ACTIVATED = 0x02,
    EMBMS_SESSION_NOT_ALLOWED_DISABLED = 0x03,
    EMBMS_SESSION_MISSING_CONTROL_INFO = 0x04,
    EMBMS_SESSION_MISSING_TMGI = 0x05,
    EMBMS_SESSION_OUT_OF_COVERAGE = 0x06,
    EMBMS_SESSION_OUT_OF_SERVICE = 0x07,
    EMBMS_SESSION_FREQ_CONFLICT = 0x08,
    EMBMS_SESSION_MAX_NUM = 0x09,
}embms_localstart_session_error;

typedef enum {
    EMBMS_GET_TIME_SIB16 = 0x00,
    EMBMS_GET_TIME_ERROR = 0x01,
    EMBMS_GET_TIME_NITZ = 0x02,
}embms_local_get_time_error;


typedef enum {
    EMBMS_LOCAL_SRV_OOS = 0x00,
    EMBMS_LOCAL_SRV_UNITCAST = 0x01,
    EMBMS_LOCAL_SRV_EMBMS = 0x02,
    EMBMS_LOCAL_SRV_E911 = 0x03,
    EMBMS_LOCAL_SRV_HVOLTE = 0x04,
    EMBMS_LOCAL_SRV_FLIGHT_MODE = 0x05,
}embms_local_service_state;

typedef enum {
    EMBMS_UNICAST_OOS = 0x00,
    EMBMS_MULTICAST_OOS = 0x01,
    EMBMS_EXIT_OOS = 0x02
}embms_oos_state;
/// M: eMBMS end

#ifdef __cplusplus
}
#endif

#endif /*MTK_RIL_EMBMS_DEF_H*/
