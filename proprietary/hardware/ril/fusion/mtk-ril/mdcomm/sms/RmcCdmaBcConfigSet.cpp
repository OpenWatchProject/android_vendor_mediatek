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
#include "RmcCdmaBcConfigSet.h"


/*****************************************************************************
 * Register Data Class
 *****************************************************************************/
RFX_REGISTER_DATA_TO_REQUEST_ID(
        RmcCdmaSetBcConfigReq, RmcVoidRsp, RFX_MSG_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG);


/*****************************************************************************
 * Class RmcCdmaSetBcConfigReq
 *****************************************************************************/
RFX_IMPLEMENT_DATA_CLASS(RmcCdmaSetBcConfigReq);
RmcCdmaSetBcConfigReq::RmcCdmaSetBcConfigReq(void *data, int length) :
        RmcMultiAtReq(data, length),
        m_channel(-1),
        m_lan(-1),
        m_mode(-1),
        m_channel_md(-1),
        m_lans_md(-1) {
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

RmcCdmaSetBcConfigReq::~RmcCdmaSetBcConfigReq() {
}


RmcAtSendInfo* RmcCdmaSetBcConfigReq::onGetFirstAtInfo(RfxBaseHandler *h) {
   RFX_UNUSED(h);
   if (!sortCategoryAndLanguage()) {
      return NULL;
   }
   RmcCdmaBcRangeParser::getRange(m_channels, m_categorys);
   RmcCdmaBcRangeParser::getRange(m_lans, m_languages);
   m_channel = 0;
   m_lan = 0;
   if (isSelected()) {
        String8 cmd("AT+ECSCBCHA?");
        String8 responsePrefix("+ECSCBCHA:");
        return new RmcSingleLineAtSendInfo(cmd, responsePrefix);
   } else {
      String8 cmd = String8::format("AT+ECSCBCHA=%d,\"%d-%d\"",
        0, m_channels[m_channel].start, m_channels[m_channel].end);
      m_channel++;
      return new RmcNoLineAtSendInfo(cmd);
   }
   return NULL;
}


RmcAtSendInfo* RmcCdmaSetBcConfigReq::onGetNextAtInfo(const String8 & cmd,RfxBaseHandler * h) {
    RFX_UNUSED(h);
    if (isSelected()) {
        if (cmd == String8("AT+ECSCBCHA?")) {
            m_channel_md = 0;
            m_lans_md = 0;
            m_channel = 0;
            m_lan = 0;
            String8 cmd("AT+ECSCBLAN?");
            String8 responsePrefix("+ECSCBLAN:");
            return new RmcSingleLineAtSendInfo(cmd, responsePrefix);
        } else {
            if (m_channel_md< m_channels_md.size()) {
                String8 cmd = String8::format("AT+ECSCBCHA=%d,\"%d-%d\"",
                  0, m_channels_md[m_channel_md].start, m_channels_md[m_channel_md].end);
                m_channel_md++;
                return new RmcNoLineAtSendInfo(cmd);
            } else if (m_lans_md < m_languages_md.size()){
                String8 cmd = String8::format("AT+ECSCBLAN=%d,\"%d-%d\"",
                  0, m_languages_md[m_lans_md].start, m_languages_md[m_lans_md].end);
                m_lans_md++;
                return new RmcNoLineAtSendInfo(cmd);
            } else if (m_channel < m_channels.size()) {
                String8 cmd = String8::format("AT+ECSCBCHA=%d,\"%d-%d\"",
                  1, m_channels[m_channel].start, m_channels[m_channel].end);
                m_channel++;
                return new RmcNoLineAtSendInfo(cmd);
            } else if (m_lan < m_languages.size()){
                String8 cmd = String8::format("AT+ECSCBLAN=%d,\"%d-%d\"",
                  1, m_lans[m_lan].start, m_lans[m_lan].end);
                m_lan++;
                return new RmcNoLineAtSendInfo(cmd);
            }
        }

    } else {
        if (m_channel < m_channels.size()) {
            String8 cmd = String8::format("AT+ECSCBCHA=%d,\"%d-%d\"",
              0, m_channels[m_channel].start, m_channels[m_channel].end);
            m_channel++;
            return new RmcNoLineAtSendInfo(cmd);
        } else if (m_lan < m_languages.size()){
            String8 cmd = String8::format("AT+ECSCBLAN=%d,\"%d-%d\"",
              0, m_lans[m_lan].start, m_lans[m_lan].end);
            m_lan++;
            return new RmcNoLineAtSendInfo(cmd);
        }
    }
    return NULL;
}


bool RmcCdmaSetBcConfigReq::onHandleIntermediates(
        const String8 & cmd,RfxAtLine * line,RfxBaseHandler * h) {
    RFX_UNUSED(h);
    if (isSelected()) {
        if (cmd == String8("AT+ECSCBCHA?") ||
                    cmd == String8("AT+ECSCBLAN?")) {
            int err;
            int mode = line->atTokNextint(&err);
            if (err < 0 || mode < 0 || mode > 1) {
                setError(RIL_E_SYSTEM_ERR);
                return false;
            }
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
                    m_channels_md.push(ranges[i]);
                }

            } else {
                for (int i = 0; i < num; i++) {
                    m_languages_md.push(ranges[i]);
                }
            }
        }
    }
    return true;
}

bool RmcCdmaSetBcConfigReq::sortCategoryAndLanguage() {
    Vector<RIL_CDMA_BroadcastSmsConfigInfo>::iterator it;
    for (it = m_infos.begin(); it != m_infos.end(); it++) {
        if (it->selected != isSelected()) {
            return false;
        }
        m_categorys.add(it->service_category);
        m_languages.add(it->language);
    }

    return true;
}

