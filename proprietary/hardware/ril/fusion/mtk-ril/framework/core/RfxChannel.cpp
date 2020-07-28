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

#include "RfxChannel.h"
#include "RfxChannelManager.h"

#define RFX_LOG_TAG "RfxChannel"

RfxChannel::RfxChannel(int channelId, char *muxPath) :
        mChannelId(channelId), mMuxPath(muxPath) {
    if (RfxRilUtils::getRilRunMode() == RIL_RUN_MODE_MOCK) {
        RFX_LOG_D(RFX_LOG_TAG, "RfxChannel, change ChannelFd For GT");
        mFd = RfxChannelManager::getChannelFdForGT(channelId);
    } else {
        if (strlen(mMuxPath) > 0) {
            do {
                mFd = open(mMuxPath, O_RDWR);
                if (mFd < 0) {
                    perror("opening AT interface. retrying...");
                    RFX_LOG_E(RFX_LOG_TAG, "could not connect to %s: %s", mMuxPath, strerror(errno));
                    sleep(10);
                    /* never returns */
                } else {
                    struct termios ios;
                    tcgetattr(mFd, &ios);
                    ios.c_lflag = 0; /* disable ECHO, ICANON, etc... */
                    ios.c_iflag = 0;
                    tcsetattr(mFd, TCSANOW, &ios);
                }
            } while (mFd < 0);
        } else {
            mFd = -1;
            RFX_LOG_E(RFX_LOG_TAG, "path is empty, set fd to -1");
        }
    }

    mContext = new RfxChannelContext();
    mReader = new RfxReader(mFd, mChannelId, mContext);
    mSender = new RfxSender(mFd, mChannelId, mContext);
}

void RfxChannel::run() {
    mReader->run(String8::format("RfxReader_%d", mChannelId).string());
    mSender->run(String8::format("RfxSender_%d", mChannelId).string());
}

void RfxChannel::enqueueMessage(const sp<RfxMclMessage>& msg) {
    mSender->enqueueMessage(msg);
}

void RfxChannel::enqueueMessageFront(const sp<RfxMclMessage>& msg) {
    mSender->enqueueMessageFront(msg);
}