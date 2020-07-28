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

#ifndef __RMC_PHB_REQUEST_HANDLER_H__
#define __RMC_PHB_REQUEST_HANDLER_H__

#include "RfxBaseHandler.h"
#include <telephony/mtk_ril.h>
#include "RfxPhbEntriesData.h"
#include "RfxPhbEntryExtData.h"
#include "RfxPhbMemStorageData.h"

#define RFX_LOG_TAG "RmcPhbReq"

typedef enum {
   CPBW_ENCODE_IRA,
   CPBW_ENCODE_UCS2,
   CPBW_ENCODE_UCS2_81,
   CPBW_ENCODE_UCS2_82,
   CPBW_ENCODE_GSM7BIT,
   CPBW_ENCODE_MAX
}RilPhbCpbwEncode;

#define RIL_PHB_UCS2_81_MASK    0x7f80

#define RIL_MAX_PHB_NAME_LEN 40   // Max # of characters in the NAME
#define RIL_MAX_PHB_EMAIL_LEN 60
#define RIL_MAX_PHB_ENTRY 10

class RmcPhbRequestHandler : public RfxBaseHandler {
    RFX_DECLARE_HANDLER_CLASS(RmcPhbRequestHandler);

    public:
        RmcPhbRequestHandler(int slot_id, int channel_id);
        virtual ~RmcPhbRequestHandler();

    protected:
        virtual void onHandleRequest(const sp<RfxMclMessage>& msg);

        virtual void onHandleUrc(const sp<RfxMclMessage>& msg) {RFX_UNUSED(msg);}

        virtual void onHandleEvent(const sp<RfxMclMessage>& msg);

    private:
        int ril_max_phb_name_len = 40;
        int maxGrpNum = -1;
        int maxAnrNum = -1;
        int maxEmailNum = -1;
        int mIsUserLoad = -1;
        // int current_phb_storage[4] = {-1, -1, -1, -1};
        int current_phb_storage = -1;
        int selectPhbStorage(int type);
        char* getPhbStorageString(int type);
        void resetPhbStorage();
        void requestResetPhbStorage(const sp<RfxMclMessage>& msg);

        int bIsTc1();
        int hexCharToDecInt(char *hex, int length);
        int isMatchGsm7bit(char *src, int maxLen);
        int isGsm7bitExtension(char *src, int maxLen);
        int encodeUCS2_0x81(char *src, char *des, int maxLen);
        int encodeUCS2_0x82(char *src, char *des, int maxLen);
        char * ascii2UCS2(char * input);
        void requestQueryPhbInfo(const sp<RfxMclMessage>& msg);
        void requestClearPhbEntry(int index);
        void requestWritePhbEntry(const sp<RfxMclMessage>& msg);
        void requestReadPhbEntry(const sp<RfxMclMessage>& msg);
        void requestQueryUPBCapability(const sp<RfxMclMessage>& msg);
        void requestEditUPBEntry(const sp<RfxMclMessage>& msg);
        void requestDeleteUPBEntry(const sp<RfxMclMessage>& msg);
        void requestReadGasList(const sp<RfxMclMessage>& msg);
        void requestReadUpbGrpEntry(const sp<RfxMclMessage>& msg);
        void requestWriteUpbGrpEntry(const sp<RfxMclMessage>& msg);
        void requestGetPhoneBookStringsLength(const sp<RfxMclMessage>& msg);
        void requestGetPhoneBookMemStorage(const sp<RfxMclMessage>& msg);
        void requestSetPhoneBookMemStorage(const sp<RfxMclMessage>& msg);
        void loadUPBCapability();
        void requestReadPhoneBookEntryExt(const sp<RfxMclMessage>& msg);
        void requestWritePhoneBookEntryExt(const sp<RfxMclMessage>& msg);
        void requestQueryUPBAvailable(const sp<RfxMclMessage>& msg);
        void requestReadUPBEmail(const sp<RfxMclMessage>& msg);
        void requestReadUPBSne(const sp<RfxMclMessage>& msg);
        void requestReadUPBAnr(const sp<RfxMclMessage>& msg);
        void requestReadAasList(const sp<RfxMclMessage>& msg);
        void requestSetPhonebookReady(const sp<RfxMclMessage>& msg);
};

#endif
