/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2019. All rights reserved.
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

#ifndef __SUPL_MSG_DISPATCHER_H__
#define __SUPL_MSG_DISPATCHER_H__
#define HAL_MNL_INTERFACE_VERSION   1

/*****************************************************************************
 * Include
 *****************************************************************************/
#include <string>
#include <list>
#include "RfxObject.h"
#include "SmsParserUtils.h"

#define SUPL_EXISTENCE_UNKNOWN      2
#define SUPL_EXISTENCE_YES          1
#define SUPL_EXISTENCE_NO           0

#define SUPL_EXISTENCE_MAX_DETECTION 3

#define HAL_MNL_BUFF_SIZE           (16 * 1024)
#define HAL2MNL_NI_MESSAGE          401
#define HAL_MNL_INTERFACE_VERSION   1
#define MTK_HAL2MNL                 "mtk_hal2mnl"

class ConcentratedSms;
class SmsMessage;
class WappushMessage;

/*****************************************************************************
 * Class SuplMsgDispatcher
 *****************************************************************************/
class SuplMsgDispatcher : public RfxObject {
    // Required: declare this class
    RFX_DECLARE_CLASS(SuplMsgDispatcher);
public:
    void dispatchSuplMsg(string content);
    bool doesSuplExist(void);

// Override
protected:
    virtual void onDeinit();


private:
    bool addToExistingConcSms(SmsMessage* msg);
    ConcentratedSms* findConcSms(int ref);
    void onConcSmsTimeout(int ref);
    void hal2mnl_ni_message(char* msg, int len);
    void put_byte(char* buff, int* offset, const char input);
    void put_short(char* buff, int* offset, const short input);
    void put_int(char* buff, int* offset, const int input);
    void put_binary(char* buff, int* offset, const char* input, const int len);
    int safe_sendto(const char* path, const char* buff, int len);
    bool notifyConcMsg2Mnl(ConcentratedSms* msg);
    bool notifySms2Mnl(SmsMessage *msg);
    bool notifyWappush2Mnl(WappushMessage *msg);

private:
    list<ConcentratedSms*> mConcSmsList;
    static int sSuplExistenceState;
    static int sSuplExistenceDetectionCount;
};

#endif /* __SUPL_MSG_DISPATCHER_H__ */
