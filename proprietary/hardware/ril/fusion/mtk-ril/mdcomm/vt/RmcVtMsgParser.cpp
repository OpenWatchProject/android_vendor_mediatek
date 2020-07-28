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
#include <mutex>
#include "RfxLog.h"
#include "RmcVtMsgParser.h"
#include "RmcVtReqHandler.h"

#define RFX_LOG_TAG "VT RIL PSR"

std::mutex mtxx;
RmcVtMsgParser* RmcVtMsgParser::s_RmcVtMsgParser;

RmcVtMsgParser::RmcVtMsgParser() {

    mCapIndStruct = NULL;
    mCapRspStruct = NULL;
    mBwReqStruct = NULL;

    mInitIndStruct = NULL;
    mUpdateIndStruct = NULL;
    mDeinitIndStruct = NULL;

    mSessionModifyReqStruct = NULL;
    mSessionModifyIndStruct = NULL;
    mSessionModifyRspStruct = NULL;
    mSessionModifyCnfStruct = NULL;

    mSessionModifyCancelReqStruct = NULL;
    mSessionModifyCancelIndStruct = NULL;
    mSessionModifyCancelCnfStruct = NULL;

    mHOStartIndStruct = NULL;
    mHOStopIndStruct = NULL;
    mHOPDNBeginIndStruct = NULL;
    mHOPDNEndIndStruct = NULL;

    mANBRRequestStruct = NULL;

    mType = 0;
}

RmcVtMsgParser::~RmcVtMsgParser() {
}

void RmcVtMsgParser::startSession(char* input, int type) {

    mtxx.lock();

    mCapIndStruct = NULL;
    mCapRspStruct = NULL;
    mBwReqStruct = NULL;

    mInitIndStruct = NULL;
    mUpdateIndStruct = NULL;
    mDeinitIndStruct = NULL;

    mSessionModifyReqStruct = NULL;
    mSessionModifyIndStruct = NULL;
    mSessionModifyRspStruct = NULL;
    mSessionModifyCnfStruct = NULL;

    mSessionModifyCancelReqStruct = NULL;
    mSessionModifyCancelIndStruct = NULL;
    mSessionModifyCancelCnfStruct = NULL;

    mHOStartIndStruct = NULL;
    mHOStopIndStruct = NULL;
    mANBRRequestStruct = NULL;

    mType = type;

    if (MSG_ID_WRAP_IMSVT_IMCB_GET_CAP_IND == mType) {

        RFX_LOG_I(RFX_LOG_TAG, "[VT  PARSER] MSG_ID_WRAP_IMSVT_IMCB_GET_CAP_IND");

        mCapIndStruct = (VT_IMCB_CAPIND* )input;

    } else if (MSG_ID_WRAP_IMSVT_IMCB_GET_CAP_RSP == mType) {

        RFX_LOG_I(RFX_LOG_TAG, "[VT  PARSER] MSG_ID_WRAP_IMSVT_IMCB_GET_CAP_RSP");

        mCapRspStruct = (VT_IMCB_CAP*)input;

        if (RmcVtReqHandler::isVTLogEnable()) {

            RFX_LOG_I(RFX_LOG_TAG, "[VT  PARSER] accout_id = %d", mCapRspStruct->accout_id);
            RFX_LOG_I(RFX_LOG_TAG, "[VT  PARSER] video_cap_num = %d", mCapRspStruct->video_cap_num);
            RFX_LOG_I(RFX_LOG_TAG, "[VT  PARSER] profile_level_id = %d", mCapRspStruct->video_cap[0].profile_level_id);
            RFX_LOG_I(RFX_LOG_TAG, "[VT  PARSER] max_recv_level = %d", mCapRspStruct->video_cap[0].max_recv_level);
            RFX_LOG_I(RFX_LOG_TAG, "[VT  PARSER] packetization_mode = %d", mCapRspStruct->video_cap[0].packetization_mode);

            RFX_LOG_I(RFX_LOG_TAG, "[VT  PARSER] bitrate_info format = %d", mCapRspStruct->bitrate_info[0].format);
            RFX_LOG_I(RFX_LOG_TAG, "[VT  PARSER] bitrate_info profile = %d", mCapRspStruct->bitrate_info[0].profile);
            RFX_LOG_I(RFX_LOG_TAG, "[VT  PARSER] bitrate_info level = %d", mCapRspStruct->bitrate_info[0].level);
            RFX_LOG_I(RFX_LOG_TAG, "[VT  PARSER] bitrate_info minbitrate = %d", mCapRspStruct->bitrate_info[0].minbitrate);
            RFX_LOG_I(RFX_LOG_TAG, "[VT  PARSER] bitrate_info bitrate = %d", mCapRspStruct->bitrate_info[0].bitrate);

        }

    } else if (MSG_ID_WRAP_IMSVT_IMCB_MOD_BW_REQ == mType) {

        RFX_LOG_I(RFX_LOG_TAG, "[VT  PARSER] MSG_ID_WRAP_IMSVT_IMCB_MOD_BW_REQ");

        mBwReqStruct = (VT_IMCB_BW*)input;

    } else if (MSG_ID_WRAP_IMSVT_IMCB_CONFIG_INIT_IND == mType) {

        RFX_LOG_I(RFX_LOG_TAG, "[VT  PARSER] MSG_ID_WRAP_IMSVT_IMCB_CONFIG_INIT_IND");

        mInitIndStruct = (VT_IMCB_INIT*)input;

    } else if (MSG_ID_WRAP_IMSVT_IMCB_CONFIG_UPDATE_IND == mType) {

        RFX_LOG_I(RFX_LOG_TAG, "[VT  PARSER] MSG_ID_WRAP_IMSVT_IMCB_CONFIG_UPDATE_IND");

        mUpdateIndStruct = (VT_IMCB_UPD*)input;

    } else if (MSG_ID_WRAP_IMSVT_IMCB_CONFIG_DEINIT_IND == mType) {

        RFX_LOG_I(RFX_LOG_TAG, "[VT  PARSER] MSG_ID_WRAP_IMSVT_IMCB_CONFIG_DEINIT_IND");

        mDeinitIndStruct = (VT_IMCB_DEINIT*)input;

    } else if (MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_REQ == mType) {

        RFX_LOG_I(RFX_LOG_TAG, "[VT  PARSER] MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_REQ");

        mSessionModifyReqStruct = (VT_IMCB_REQ*)input;

    } else if (MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_IND == mType) {

        RFX_LOG_I(RFX_LOG_TAG, "[VT  PARSER] MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_IND");

        mSessionModifyIndStruct = (VT_IMCB_IND*)input;

    } else if (MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_RSP == mType) {

        RFX_LOG_I(RFX_LOG_TAG, "[VT  PARSER] MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_RSP");

        mSessionModifyRspStruct = (VT_IMCB_RSP*)input;

    } else if (MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_CNF == mType) {

        RFX_LOG_I(RFX_LOG_TAG, "[VT  PARSER] MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_CNF");

        mSessionModifyCnfStruct = (VT_IMCB_CNF*)input;

    } else if (MSG_ID_WRAP_IMSVT_IMCB_HANDOVER_START_IND == mType) {

        RFX_LOG_I(RFX_LOG_TAG, "[VT  PARSER] MSG_ID_WRAP_IMSVT_IMCB_HANDOVER_START_IND");

        mHOStartIndStruct = (VT_IMCB_HOSTART*)input;

    } else if (MSG_ID_WRAP_IMSVT_IMCB_HANDOVER_STOP_IND == mType) {

        RFX_LOG_I(RFX_LOG_TAG, "[VT  PARSER] MSG_ID_WRAP_IMSVT_IMCB_HANDOVER_STOP_IND");

        mHOStopIndStruct = (VT_IMCB_HOSTOP*)input;

    } else if (MSG_ID_WRAP_IMSVT_IMCB_PDN_HANDOVER_BEGIN_IND == mType) {

        RFX_LOG_I(RFX_LOG_TAG, "[VT  PARSER] MSG_ID_WRAP_IMSVT_IMCB_PDN_HANDOVER_BEGIN_IND");

        mHOPDNBeginIndStruct = (VT_IMCB_PDN_HOBEGIN*)input;

    } else if (MSG_ID_WRAP_IMSVT_IMCB_PDN_HANDOVER_END_IND == mType) {

        RFX_LOG_I(RFX_LOG_TAG, "[VT  PARSER] MSG_ID_WRAP_IMSVT_IMCB_PDN_HANDOVER_END_IND");

        mHOPDNEndIndStruct = (VT_IMCB_PDN_HOEND*)input;

    } else if (MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_CANCEL_REQ == mType) {

        RFX_LOG_I(RFX_LOG_TAG, "[VT  PARSER] MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_CANCEL_REQ");

        mSessionModifyCancelReqStruct = (VT_IMCB_CANCEL_REQ*)input;

    } else if (MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_CANCEL_IND == mType) {

        RFX_LOG_I(RFX_LOG_TAG, "[VT  PARSER] MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_CANCEL_IND");

        mSessionModifyCancelIndStruct = (VT_IMCB_CANCEL_IND*)input;

    } else if (MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_CANCEL_CNF == mType) {

        RFX_LOG_I(RFX_LOG_TAG, "[VT  PARSER] MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_CANCEL_CNF");

        mSessionModifyCancelCnfStruct = (VT_IMCB_CANCEL_CNF*)input;

    } else if (MSG_ID_WRAP_IMSVT_MD_ANBR_REPORT_REQ == mType) {

        RFX_LOG_I(RFX_LOG_TAG, "[VT  PARSER] MSG_ID_VT_ANBR_REQ");
        mANBRRequestStruct = (VT_ANBR_REQ*)input;

    } else {

    }
}

void RmcVtMsgParser::stopSession() {

    mCapIndStruct = NULL;
    mCapRspStruct = NULL;
    mBwReqStruct = NULL;

    mInitIndStruct = NULL;
    mUpdateIndStruct = NULL;
    mDeinitIndStruct = NULL;

    mSessionModifyReqStruct = NULL;
    mSessionModifyIndStruct = NULL;
    mSessionModifyRspStruct = NULL;
    mSessionModifyCnfStruct = NULL;

    mSessionModifyCancelReqStruct = NULL;
    mSessionModifyCancelIndStruct = NULL;
    mSessionModifyCancelCnfStruct = NULL;

    mHOStartIndStruct = NULL;
    mHOStopIndStruct = NULL;

    mtxx.unlock();
}

int RmcVtMsgParser::getSlot() {

    if (MSG_ID_WRAP_IMSVT_IMCB_GET_CAP_IND == mType) {

        return mCapIndStruct->sim_slot_id;

    } else if (MSG_ID_WRAP_IMSVT_IMCB_GET_CAP_RSP == mType) {

        return mCapRspStruct->sim_slot_id;

    } else if (MSG_ID_WRAP_IMSVT_IMCB_MOD_BW_REQ == mType) {

        return mBwReqStruct->sim_slot_id;

    } else if (MSG_ID_WRAP_IMSVT_IMCB_CONFIG_INIT_IND == mType) {

        return mInitIndStruct->setting.sim_slot_id;

    } else if (MSG_ID_WRAP_IMSVT_IMCB_CONFIG_UPDATE_IND == mType) {

        return mUpdateIndStruct->setting.sim_slot_id;

    } else if (MSG_ID_WRAP_IMSVT_IMCB_CONFIG_DEINIT_IND == mType) {

        return mDeinitIndStruct->setting.sim_slot_id;

    } else if (MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_REQ == mType) {

        return mSessionModifyReqStruct->sim_slot_id;

    } else if (MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_IND == mType) {

        return mSessionModifyIndStruct->sim_slot_id;

    } else if (MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_RSP == mType) {

        return mSessionModifyRspStruct->sim_slot_id;

    } else if (MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_CNF == mType) {

        return mSessionModifyCnfStruct->sim_slot_id;

    } else if (MSG_ID_WRAP_IMSVT_IMCB_HANDOVER_START_IND == mType) {

        return mHOStartIndStruct->sim_slot_id;

    } else if (MSG_ID_WRAP_IMSVT_IMCB_HANDOVER_STOP_IND == mType) {

        return mHOStopIndStruct->sim_slot_id;

    } else if (MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_CANCEL_REQ == mType) {

        return mSessionModifyCancelReqStruct->sim_slot_id;

    } else if (MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_CANCEL_IND == mType) {

        return mSessionModifyCancelIndStruct->sim_slot_id;

    } else if (MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_CANCEL_CNF == mType) {

        return mSessionModifyCancelCnfStruct->sim_slot_id;

    } else if (MSG_ID_WRAP_IMSVT_MD_ANBR_REPORT_REQ == mType) {

        RFX_LOG_I(RFX_LOG_TAG, "[VT  PARSER] MSG_ID_WRAP_IMSVT_MD_ANBR_REPORT_REQ, sim_slot_id = %d", mANBRRequestStruct->sim_slot_id);
        return mANBRRequestStruct->sim_slot_id;

    } else {

        return 0;
    }

    return 0;
}

RmcVtMsgParser* RmcVtMsgParser::getInstance() {
    if (s_RmcVtMsgParser == NULL) {
        mtxx.lock();
        if (s_RmcVtMsgParser == NULL) {
            s_RmcVtMsgParser = new RmcVtMsgParser();
        }
        mtxx.unlock();
    }
    return s_RmcVtMsgParser;
}

