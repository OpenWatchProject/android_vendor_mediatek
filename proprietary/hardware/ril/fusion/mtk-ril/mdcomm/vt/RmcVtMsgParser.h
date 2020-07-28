/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#ifndef __RMC_VT_MSG_PARSER_H__
#define __RMC_VT_MSG_PARSER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "imsvt_imcb_sap.h"

/*****************************************************************************
 * Define
 *****************************************************************************/

const int MSG_ID_WRAP_IMSVT_MD_BEGIN                            = MSG_ID_WRAP_IMSVT_IMCB_BEGIN + 1000;
const int MSG_ID_WRAP_IMSVT_MD_ANBR_CONFIG_UPDATE_IND           = MSG_ID_WRAP_IMSVT_MD_BEGIN + 1;
const int MSG_ID_WRAP_IMSVT_MD_ANBR_REPORT_REQ                  = MSG_ID_WRAP_IMSVT_MD_BEGIN + 2;
const int MSG_ID_WRAP_IMSVT_MD_INTER_RAT_STATUS_IND             = MSG_ID_WRAP_IMSVT_MD_BEGIN + 3;

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


#define VT_IMCB_CAPIND                              vt_srv_imcb_msg_get_cap_ind_struct
#define VT_IMCB_CAP                                 vt_srv_imcb_msg_get_cap_rsp_struct
#define VT_IMCB_BW                                  vt_srv_imcb_msg_mod_bw_req_t

#define VT_IMCB_CONFIG                              vt_srv_imcb_msg_param_t
#define VT_IMCB_INIT                                vt_srv_imcb_msg_config_init_ind_struct
#define VT_IMCB_UPD                                 vt_srv_imcb_msg_config_update_ind_struct
#define VT_IMCB_DEINIT                              vt_srv_imcb_msg_config_deinit_ind_struct

#define VT_IMCB_REQ                                 vt_srv_imcb_msg_session_modify_req_struct
#define VT_IMCB_IND                                 vt_srv_imcb_msg_session_modify_ind_struct
#define VT_IMCB_RSP                                 vt_srv_imcb_msg_session_modify_rsp_struct
#define VT_IMCB_CNF                                 vt_srv_imcb_msg_session_modify_cnf_struct

#define VT_IMCB_CANCEL_REQ                          vt_srv_imcb_msg_session_modify_cancel_req_struct
#define VT_IMCB_CANCEL_IND                          vt_srv_imcb_msg_session_modify_cancel_ind_struct
#define VT_IMCB_CANCEL_CNF                          vt_srv_imcb_msg_session_modify_cancel_cnf_struct

#define VT_IMCB_HOSTART                             vt_srv_imcb_handover_start_ind_struct
#define VT_IMCB_HOSTOP                              vt_srv_imcb_handover_stop_ind_struct
#define VT_IMCB_PDN_HOBEGIN                         vt_srv_imcb_pdn_handover_begin_ind_struct
#define VT_IMCB_PDN_HOEND                           vt_srv_imcb_pdn_handover_end_ind_struct

#define VT_ANBR_REQ                                 vt_srv_ril_msg_anbr_struct
#define VT_ANBR_CONFIG                              vt_srv_anbr_struct

class RmcVtMsgParser {

    private:
        RmcVtMsgParser();

    public:
        virtual ~RmcVtMsgParser();

        static RmcVtMsgParser* getInstance();
        void startSession(char* input, int type);
        void stopSession();
        int getSlot();

        static RmcVtMsgParser          *s_RmcVtMsgParser;

    private:

        int                      mType;

        VT_IMCB_CAPIND          *mCapIndStruct;
        VT_IMCB_CAP             *mCapRspStruct;
        VT_IMCB_BW              *mBwReqStruct;

        VT_IMCB_INIT            *mInitIndStruct;
        VT_IMCB_UPD             *mUpdateIndStruct;
        VT_IMCB_DEINIT          *mDeinitIndStruct;

        VT_IMCB_REQ             *mSessionModifyReqStruct;
        VT_IMCB_IND             *mSessionModifyIndStruct;
        VT_IMCB_RSP             *mSessionModifyRspStruct;
        VT_IMCB_CNF             *mSessionModifyCnfStruct;

        VT_IMCB_CANCEL_REQ      *mSessionModifyCancelReqStruct;
        VT_IMCB_CANCEL_IND      *mSessionModifyCancelIndStruct;
        VT_IMCB_CANCEL_CNF      *mSessionModifyCancelCnfStruct;

        VT_IMCB_HOSTART         *mHOStartIndStruct;
        VT_IMCB_HOSTOP          *mHOStopIndStruct;
        VT_IMCB_PDN_HOBEGIN     *mHOPDNBeginIndStruct;
        VT_IMCB_PDN_HOEND       *mHOPDNEndIndStruct;
        VT_ANBR_REQ             *mANBRRequestStruct;
};

#endif /* __RMC_MSIM_REQ_HANDLER_H__ */
