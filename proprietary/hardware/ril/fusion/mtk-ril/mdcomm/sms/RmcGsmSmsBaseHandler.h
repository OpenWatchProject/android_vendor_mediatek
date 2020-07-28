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

#ifndef __RMC_GSM_SMS_BASE_HANDLER_H__
#define __RMC_GSM_SMS_BASE_HANDLER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxBaseHandler.h"
#include "RmcGsmSmsDefs.h"

/*****************************************************************************
 * Class RpSimController
 *****************************************************************************/

class RmcGsmSmsBaseHandler : public RfxBaseHandler {
RFX_DECLARE_HANDLER_CLASS(RmcGsmSmsBaseHandler);

public:
    RmcGsmSmsBaseHandler(int slot_id, int channel_id);

protected:
    // Utility functions
    int smsPackPdu(const char* smsc, const char *tpdu, char *pdu, bool check);
    bool smsFoCheck(int fo);
    bool smsPidCheck(int pid);
    bool smsDcsCheck(int dcs);
    void smsDecodeDcs(int dcs, SMS_ENCODING_ENUM *alphabet_type,
            SMS_MESSAGE_CLASS_ENUM *msg_class, bool *is_compress);
    bool smsIsLenIn8bit(int dcs);
    unsigned int smsMsgLenInOctet(int dcs, int len);
    int smsHexCharToDecInt(char *hex, int length);
    bool smsSubmitPduCheck(int pdu_len, char *pdu_ptr, TPDU_ERROR_CAUSE_ENUM  *error_cause);
    bool smsDeliverPduCheck(int pdu_len, char *pdu, TPDU_ERROR_CAUSE_ENUM *error_cause);
    int atGetCmsError(const sp<RfxAtResponse> p_response);
    void showCurrIncomingSmsType();
    int smsCheckReceivedPdu(int length, char* pdu, bool check);
    int hexCharToInt(char c);
    void hexStringToBytes(const char *in, int inLength, char *out, int outLength);

private:
    void setTag(String8 tag) {mTag = tag;}

private:
    String8 mTag;
};
#endif /* __RMC_GSM_SMS_BASE_HANDLER_H__ */

