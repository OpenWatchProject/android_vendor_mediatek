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
/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RmcCdmaBcConfigGet.h"

/*****************************************************************************
 * Register Data Class
 *****************************************************************************/
RFX_REGISTER_DATA_TO_REQUEST_ID(
        RmcCdmaGetBcConfigReq, RmcCdmaGetBcConfigRsp,
        RFX_MSG_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG);

/*****************************************************************************
 * Class RmcCdmaGetBcConfigReq
 *****************************************************************************/
RFX_IMPLEMENT_DATA_CLASS(RmcCdmaGetBcConfigReq);
RmcCdmaGetBcConfigReq::RmcCdmaGetBcConfigReq(void *data, int length)
        :RmcMultiAtReq(data, length), m_mode(-1) {
}

RmcCdmaGetBcConfigReq::~RmcCdmaGetBcConfigReq() {
}

RmcAtSendInfo* RmcCdmaGetBcConfigReq::onGetFirstAtInfo(RfxBaseHandler *h) {
    RFX_UNUSED(h);
    String8 cmd("AT+ECSCB?");
    String8 responsePrefix("+ECSCB:");
    return new RmcSingleLineAtSendInfo(cmd, responsePrefix);
}

RmcAtSendInfo* RmcCdmaGetBcConfigReq::onGetNextAtInfo(
        const String8 & cmd, RfxBaseHandler * h) {
    RFX_UNUSED(h);
    if (cmd == String8("AT+ECSCB?")) {
        String8 cmd("AT+ECSCBCHA?");
        String8 responsePrefix("+ECSCBCHA:");
        return new RmcSingleLineAtSendInfo(cmd, responsePrefix);
    } else if (cmd == String8("AT+ECSCBCHA?")) {
        String8 cmd("AT+ECSCBLAN?");
        String8 responsePrefix("+ECSCBLAN:");
        return new RmcSingleLineAtSendInfo(cmd, responsePrefix);
    }
    return NULL;
}

bool RmcCdmaGetBcConfigReq::onHandleIntermediates(
        const String8 & cmd,RfxAtLine * line,RfxBaseHandler * h) {
    RFX_UNUSED(h);
    int err;
    int mode = line->atTokNextint(&err);
    if (err < 0 || mode < 0 || mode > 1) {
        setError(RIL_E_SYSTEM_ERR);
        return false;
    }

    if (cmd == String8("AT+ECSCB?")) {
        m_mode = mode;
        if (mode == 0) {
            return false;
        }
    } else if (cmd == String8("AT+ECSCBCHA?") ||
        cmd == String8("AT+ECSCBLAN?")) {
        String8 rangeStr;
        Range ranges[MAX_RANGE];
        int num = 0;
        if (mode == 1) {
            char *str = line->atTokNextstr(&err);
            if (err < 0) {
                setError(RIL_E_SYSTEM_ERR);
                return false;
            }
            rangeStr.setTo(str);
            num = RmcCdmaBcRangeParser::getRangeFromModem((char *)rangeStr.string(), ranges);
        }
        if (cmd == String8("AT+ECSCBCHA?")) {
            for (int i = 0; i < num; i++) {
                m_channels.push(ranges[i]);
            }
        } else {
            for (int i = 0; i < num; i++) {
                m_languages.push(ranges[i]);
            }
        }
    }
    return true;
}


/*****************************************************************************
 * Class RmcCdmaGetBcConfigRsp
 *****************************************************************************/
RFX_IMPLEMENT_DATA_CLASS(RmcCdmaGetBcConfigRsp);
RmcCdmaGetBcConfigRsp::RmcCdmaGetBcConfigRsp(void *data, int length)
        : RmcVoidRsp(data, length) {
    if (data != NULL) {
        RIL_CDMA_BroadcastSmsConfigInfo **p_cur =
                   (RIL_CDMA_BroadcastSmsConfigInfo **) data;

        int num = length / sizeof (RIL_CDMA_BroadcastSmsConfigInfo *);
        for (int i = 0; i < num; i++) {
            m_infos.push(*(p_cur[i]));
        }
        Vector<RIL_CDMA_BroadcastSmsConfigInfo>::iterator it;
        int i = 0;
        for (it = m_infos.begin(); it != m_infos.end(); it++, i++) {
            m_pInfos.push(it);
        }
        m_data = (void *)m_pInfos.array();
        m_length = sizeof(RIL_CDMA_BroadcastSmsConfigInfo*) * m_pInfos.size();
    }
}

RmcCdmaGetBcConfigRsp::RmcCdmaGetBcConfigRsp(
        Vector<RIL_CDMA_BroadcastSmsConfigInfo> infos, RIL_Errno e)
        : RmcVoidRsp(e), m_infos(infos) {
    if (e == RIL_E_SUCCESS) {
        Vector<RIL_CDMA_BroadcastSmsConfigInfo>::iterator it;
        int i = 0;
        for (it = m_infos.begin(); it != m_infos.end(); it++, i++) {
            m_pInfos.push(it);
        }
        m_data = (void *)m_pInfos.array();
        m_length = sizeof(RIL_CDMA_BroadcastSmsConfigInfo*) * m_pInfos.size();
    }
}

RmcCdmaGetBcConfigRsp::~RmcCdmaGetBcConfigRsp() {
}

/*****************************************************************************
 * Class RmcCdmaBcGetConfigHdlr
 *****************************************************************************/
RmcBaseRspData *RmcCdmaBcGetConfigHdlr::onGetRspData(RmcBaseReqData *req) {
    RmcCdmaGetBcConfigReq *bcConfigReq = (RmcCdmaGetBcConfigReq *)((void* )req);
    RIL_Errno e = RIL_E_SUCCESS;
    Vector<RIL_CDMA_BroadcastSmsConfigInfo> infos;
    if (bcConfigReq->isBcActivate()) {
        const Vector<Range> & channels = bcConfigReq->getChannels();
        const Vector<Range> & languages = bcConfigReq->getLanguages();
        if ((channels.size() + languages.size()) == 0) {
            RIL_CDMA_BroadcastSmsConfigInfo info;
            memset(&info, 0, sizeof(info));
            infos.push(info);
        } else {
            Vector<Range>::const_iterator it;
            for (it = channels.begin(); it != channels.end(); it++) {
                for (int i = it->start; i <= it->end; i++) {
                    RIL_CDMA_BroadcastSmsConfigInfo info;
                    info.service_category = i;
                    info.language = 0;
                    info.selected = 1;
                    infos.push(info);
                }
            }
            for (it = languages.begin(); it != languages.end(); it++) {
                for (int i = it->start; i <= it->end; i++) {
                    RIL_CDMA_BroadcastSmsConfigInfo info;
                    info.service_category = 0;
                    info.language = i;
                    info.selected = 1;
                    infos.push(info);
                }
            }
        }
    } else {
        RIL_CDMA_BroadcastSmsConfigInfo info;
        memset(&info, 0, sizeof(info));
        infos.push(info);
    }
    return new RmcCdmaGetBcConfigRsp(infos, e);
}
