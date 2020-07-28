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

#ifndef WPAF_DRIVER_UTILIS_H
#define WPAF_DRIVER_UTILIS_H

#include <termios.h>
// #include "utils/Mutex.h"
#include "Mutex.h"

#include <mtk_log.h>

#include "WpfaDriverMessage.h"
#include "WpfaCcciReader.h"
#include "WpfaCcciSender.h"



using ::android::Mutex;

/*
 * =============================================================================
 *                     Defines
 * =============================================================================
 */
#define WPFA_DRIVER_TEST_MODE_ENABLE (0)

#define TEST_EVENT_NONE (0)
#define TEST_EVENT_PKT_SEND_TO_KERNEL_SUCCESS (1)


#define FAKE_MD_TID_START (100)
#define FAKE_MD_TID_END   (500)


class WpfaDriverUtilis {
public:
    WpfaDriverUtilis();
    virtual ~WpfaDriverUtilis();
    static WpfaDriverUtilis *getInstance();
    void init();

    void setCcciReader(WpfaCcciReader *reader) {
        mReader = reader;
    }

    void setCcciSender(WpfaCcciSender *sender){
        mSender = sender;
    }

    void notifyEvent(int eventId){
        mEventId = eventId;
    }

    void testStartNormal(const sp<WpfaDriverMessage>& msg);

    //int sendMsgToControlMsgDispatcher(const sp<WpfaDriverMessage>& message);
    //int sendMsgToFilterRuleReqHandler(const sp<WpfaDriverMessage>& message);
    //int sendMsgToShmReadMsgHandler(const sp<WpfaDriverMessage>& message);
    //int sendMsgToShmWriteMsgHandler(const sp<WpfaDriverMessage>& message);


private:

    void fake_M2A_WPFA_VERSION(const sp<WpfaDriverMessage>& msg);
    void fake_M2A_REG_DL_FILTER(int testingFilterStrId);
    void fake_M2A_DEREG_DL_FILTER(uint32_t filterId);
    void fake_M2A_UL_IP_PKT(int IpPktId);

    uint16_t fake_MD_Tid();

    /**
     * singleton pattern
     */
    static WpfaDriverUtilis *sInstance;
    static Mutex sWpfaDriverUtilisInitMutex;

    WpfaCcciReader *mReader;
    WpfaCcciSender *mSender;

    uint16_t mFakeMdTid;
    int mEventId;
};

#endif /* end of WPAF_DRIVER_UTILIS_H */

