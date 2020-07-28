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

#ifndef __RFX_CHANNEL_MANAGER_H__
#define __RFX_CHANNEL_MANAGER_H__

#include "RfxLog.h"
#include "rfx_properties.h"
#include "RfxDefs.h"
#include "RfxChannel.h"
#include <telephony/mtk_ril.h>

#define RIL_SUPPORT_CHANNELS_MAX_NAME_LEN 32

#define RIL_TTY_CHANNEL_IMS "dev/ccci_vts"

class RfxChannelManager {
    private:

        RfxChannelManager();

    public:

        static void init();
        static RfxSender* getSender(int channelId);
        static RfxChannel* getChannel(int channelId);
        static int getSupportChannels();
        static const char* channelIdToString(int channelId);
        static const char* proxyIdToString(int proxyId);
        static void setChannelFdForGT(int* channelFd);
        static int getChannelFdForGT(int channelId);
        static void urcRegisterDone();
        static int getMainChannelSim();

    private:
        void initMuxPath();
        void switchMuxPath(int majorSim);
        void initOtherChannel();

    private:
        static RfxChannelManager *sSelf;
        RfxChannel* mChannels[RIL_SUPPORT_CHANNELS];
        static int sFdsForGt[RIL_SUPPORT_CHANNELS];
        char muxPath[RIL_SUPPORT_CHANNELS][RIL_SUPPORT_CHANNELS_MAX_NAME_LEN];
        char muxPathInit[RIL_SUPPORT_CHANNELS][RIL_SUPPORT_CHANNELS_MAX_NAME_LEN] = {
                "/dev/radio/pttynoti",
                "/dev/radio/pttycmd1",
                "/dev/radio/pttycmd2",
                "/dev/radio/pttycmd3",
                "/dev/radio/pttycmd4",
                "/dev/radio/atci1",
                "/dev/radio/pttycmd7",
                "/dev/radio/pttycmd8",
                "/dev/radio/pttycmd9",
                "/dev/radio/pttycmd10",
                "/dev/radio/pttycmd11",
                "/dev/radio/pttyims",

                "/dev/radio/ptty2noti",
                "/dev/radio/ptty2cmd1",
                "/dev/radio/ptty2cmd2",
                "/dev/radio/ptty2cmd3",
                "/dev/radio/ptty2cmd4",
                "/dev/radio/atci2",
                "/dev/radio/ptty2cmd7",
                "/dev/radio/ptty2cmd8",
                "/dev/radio/ptty2cmd9",
                "/dev/radio/ptty2cmd10",
                "/dev/radio/ptty2cmd11",
                "/dev/radio/ptty2ims",

                "/dev/radio/ptty3noti",
                "/dev/radio/ptty3cmd1",
                "/dev/radio/ptty3cmd2",
                "/dev/radio/ptty3cmd3",
                "/dev/radio/ptty3cmd4",
                "/dev/radio/atci3",
                "/dev/radio/ptty3cmd7",
                "/dev/radio/ptty3cmd8",
                "/dev/radio/ptty3cmd9",
                "/dev/radio/ptty3cmd10",
                "/dev/radio/ptty3cmd11",
                "/dev/radio/ptty3ims",

                "/dev/radio/ptty4noti",
                "/dev/radio/ptty4cmd1",
                "/dev/radio/ptty4cmd2",
                "/dev/radio/ptty4cmd3",
                "/dev/radio/ptty4cmd4",
                "/dev/radio/atci4",
                "/dev/radio/ptty4cmd7",
                "/dev/radio/ptty4cmd8",
                "/dev/radio/ptty4cmd9",
                "/dev/radio/ptty4cmd10",
                "/dev/radio/ptty4cmd11",
                "/dev/radio/ptty4ims"
        };
        Mutex mUrcMutex;
        static int sMainSim;
        int mUrcCount;
    };

#endif
