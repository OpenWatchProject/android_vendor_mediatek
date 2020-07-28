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

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RmcSimUrcEntryHandler.h"
#include <telephony/mtk_ril.h>
#include "RfxDefs.h"
#include "RfxOpUtils.h"

/*****************************************************************************
 * Class RfxController
 *****************************************************************************/
RFX_IMPLEMENT_HANDLER_CLASS(RmcSimUrcEntryHandler, RIL_CMD_PROXY_URC);

RmcSimUrcEntryHandler::RmcSimUrcEntryHandler(int slot_id, int channel_id) :
        RmcSimBaseHandler(slot_id, channel_id) {
    setTag(String8("RmcSimUrcEntry"));
    // Create Gsm SIM Controller and C2K SIM Controller
    if (RfxOpUtils::getOpHandler() != NULL) {
        mGsmUrcHandler = (RmcGsmSimUrcHandler*)RfxOpUtils::getSimOpHandler(MTK_RIL_SIM_GSM_URC,
                slot_id, channel_id);
        mCdmaUrcHandler = (RmcCdmaSimUrcHandler*)RfxOpUtils::getSimOpHandler(MTK_RIL_SIM_CDMA_URC,
                slot_id, channel_id);
        mCommUrcHandler = (RmcCommSimUrcHandler*)RfxOpUtils::getSimOpHandler(MTK_RIL_SIM_COMM_URC,
                slot_id, channel_id);
    } else {
        RFX_HANDLER_CREATE(mGsmUrcHandler, RmcGsmSimUrcHandler, (slot_id, channel_id));
        RFX_HANDLER_CREATE(mCdmaUrcHandler, RmcCdmaSimUrcHandler, (slot_id, channel_id));
        RFX_HANDLER_CREATE(mCommUrcHandler, RmcCommSimUrcHandler, (slot_id, channel_id));
    }
    // External SIM [Start]
#ifdef MTK_EXTERNAL_SIM_SUPPORT
    if (RfxRilUtils::isExternalSimSupport()) {
        RFX_HANDLER_CREATE(mVsimUrcHandler, RmcVsimUrcHandler, (slot_id, channel_id));
    } else {
        mVsimUrcHandler = NULL;
    }
#else
    mVsimUrcHandler = NULL;
#endif
    // External SIM [End]
    // register request
    int record_num = 0;
    if (mGsmUrcHandler != NULL) {
        const char** gsm_urc_list = mGsmUrcHandler->queryUrcTable(&record_num);
        if (gsm_urc_list != NULL) {
            registerToHandleURC(gsm_urc_list, record_num);
        }
        if (RfxOpUtils::getOpHandler() != NULL && NULL != gsm_urc_list) {
            for (int i = 0; i < record_num; i++) {
                free((void*)gsm_urc_list[i]);
            }
            free((void*)gsm_urc_list);
        }
    }

    record_num = 0;
    if (mCdmaUrcHandler != NULL) {
        const char** cdma_urc_list = mCdmaUrcHandler->queryUrcTable(&record_num);
        if (cdma_urc_list != NULL) {
            registerToHandleURC(cdma_urc_list, record_num);
        }
    }

    record_num = 0;
    if (mCommUrcHandler != NULL) {
        const char** comm_urc_list = mCommUrcHandler->queryUrcTable(&record_num);
        if (comm_urc_list != NULL) {
            registerToHandleURC(comm_urc_list, record_num);
        }
        if (RfxOpUtils::getOpHandler() != NULL && NULL != comm_urc_list) {
            for (int i = 0; i < record_num; i++) {
                free((void*)comm_urc_list[i]);
            }
            free((void*)comm_urc_list);
        }
    }
    // External SIM [Start]
#ifdef MTK_EXTERNAL_SIM_SUPPORT
    if (mVsimUrcHandler != NULL) {
        record_num = 0;
        const char** vsim_urc_list = mVsimUrcHandler->queryUrcTable(&record_num);
        if (vsim_urc_list != NULL) {
            registerToHandleURC(vsim_urc_list, record_num);
        }
    }
#endif
    // External SIM [End]

    // register request, urc
}

RmcSimUrcEntryHandler::~RmcSimUrcEntryHandler() {
}

void RmcSimUrcEntryHandler::onHandleUrc(const sp<RfxMclMessage>& msg) {
    bool isHandled = false;
    RfxAtLine *urc = new RfxAtLine(msg->getRawUrc()->getLine(), NULL);

    if (mCommUrcHandler->needHandle(msg) == RmcSimBaseHandler::RESULT_NEED) {
        isHandled = true;
        mCommUrcHandler->handleUrc(msg, urc);
    }

    if (urc != NULL) {
        delete(urc);
    }

    urc = new RfxAtLine(msg->getRawUrc()->getLine(), NULL);
    if (mGsmUrcHandler->needHandle(msg) == RmcSimBaseHandler::RESULT_NEED) {
        isHandled = true;
        mGsmUrcHandler->handleUrc(msg, urc);
    } else if (mCdmaUrcHandler->needHandle(msg) == RmcSimBaseHandler::RESULT_NEED) {
        isHandled = true;
        mCdmaUrcHandler->handleUrc(msg, urc);
    // External SIM [Start]
#ifdef MTK_EXTERNAL_SIM_SUPPORT
    } else if (mVsimUrcHandler != NULL &&
               mVsimUrcHandler->needHandle(msg) == RmcSimBaseHandler::RESULT_NEED) {
        isHandled = true;
        mVsimUrcHandler->handleUrc(msg, urc);
#endif
    // External SIM [End]
    }
    if (urc != NULL) {
        delete(urc);
    }

    if (!isHandled) {
        // No one want to handle the URC
        logE(mTag, "No one handle the request!");
    }
}


