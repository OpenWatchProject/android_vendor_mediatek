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

#include "RfxChannelManager.h"
#include "RfxRilUtils.h"

#define RFX_LOG_TAG "RfxChannelMgr"

RfxChannelManager *RfxChannelManager::sSelf = NULL;
int RfxChannelManager::sFdsForGt[RIL_SUPPORT_CHANNELS];
int RfxChannelManager::sMainSim = 1;

RfxChannelManager::RfxChannelManager() {
    mUrcCount = 0;
    initMuxPath();
    // init channel
    for (int i = 0; i < getSupportChannels(); i++) {
        mChannels[i] = new RfxChannel(i, muxPath[i]);
    }
}

void RfxChannelManager::setChannelFdForGT(int* testChannelFd) {
    RFX_LOG_D(RFX_LOG_TAG, "setChannelFdForGT");
    for (int index = 0; index < getSupportChannels(); index++) {
        sFdsForGt[index] = testChannelFd[index];
        RFX_LOG_D(RFX_LOG_TAG, "setChannelFdForGT, id:%d, fd:%d", index, sFdsForGt[index]);
    }
}

int RfxChannelManager::getChannelFdForGT(int channelId) {
    return sFdsForGt[channelId];
}

void RfxChannelManager::init() {
    sSelf = new RfxChannelManager();

    // run each RfxSender & RfxReader
    for (int i = 0; i < getSupportChannels(); i++) {
        if ((i % RIL_CHANNEL_OFFSET) == 0) {
            RFX_LOG_E(RFX_LOG_TAG, "init urc first");
            sSelf->mChannels[i]->run();
        }
    }
}

RfxSender* RfxChannelManager::getSender(int channelId) {
   RFX_ASSERT(channelId >= 0 && channelId < getSupportChannels());
   return sSelf->mChannels[channelId]->getSender();
}

RfxChannel* RfxChannelManager::getChannel(int channelId) {
    return sSelf->mChannels[channelId];
}

int RfxChannelManager::getSupportChannels() {
    int supportChannels = 0;
    switch(RfxRilUtils::rfxGetSimCount()) {
        case 2:
            supportChannels = RIL_CHANNEL_SET3_OFFSET;
            break;
        case 3:
            supportChannels = RIL_CHANNEL_SET4_OFFSET;
            break;
        case 4:
            supportChannels = RIL_SUPPORT_CHANNELS;
            break;
        case 1:
        default:
            supportChannels = RIL_CHANNEL_OFFSET;
            break;
    }
    return supportChannels;
}

void RfxChannelManager::initMuxPath() {
    char prop_value[RFX_PROPERTY_VALUE_MAX] = {0};

    RFX_LOG_D(RFX_LOG_TAG, "initMuxPath start");
    // translate IMS channel
#ifdef MTK_IMS_CHANNEL_SUPPORT
    int isSubmarine = 0;
    rfx_property_get("persist.vendor.ims.submarine", prop_value, "0");
    isSubmarine = atoi(prop_value);
    if (isSubmarine) {
        RFX_LOG_D(RFX_LOG_TAG, "enable submarine");
        int fd = open(RIL_TTY_CHANNEL_IMS, O_RDWR);
        if (fd == -1) {
            RFX_LOG_E(RFX_LOG_TAG, "IMS TTY channel doesn't exist! %s", strerror(errno));
        } else {
            RFX_LOG_I(RFX_LOG_TAG, "IMS TTY channel exists!");
            strncpy(muxPathInit[RIL_CMD_IMS], RIL_TTY_CHANNEL_IMS,
                    RIL_SUPPORT_CHANNELS_MAX_NAME_LEN - 1);
            strncpy(muxPathInit[RIL_CMD2_IMS], "", RIL_SUPPORT_CHANNELS_MAX_NAME_LEN - 1);
            strncpy(muxPathInit[RIL_CMD3_IMS], "", RIL_SUPPORT_CHANNELS_MAX_NAME_LEN - 1);
            strncpy(muxPathInit[RIL_CMD4_IMS], "", RIL_SUPPORT_CHANNELS_MAX_NAME_LEN - 1);
            close(fd);
        }
    }
#endif
    memcpy(muxPath, muxPathInit, RIL_SUPPORT_CHANNELS*RIL_SUPPORT_CHANNELS_MAX_NAME_LEN);
    if (RFX_SLOT_COUNT >= 2) {
        int targetSim = 0;

        rfx_property_get(PROPERTY_3G_SIM, prop_value, "1");
        targetSim = atoi(prop_value);
        if (targetSim == 0) {
            targetSim = 1;
            rfx_property_set(PROPERTY_3G_SIM, "1");
        }
        RFX_LOG_D(RFX_LOG_TAG, "targetSim : %d", targetSim);
        sMainSim = targetSim;

        if(targetSim*RIL_CHANNEL_OFFSET > getSupportChannels()) {
            RFX_LOG_E(RFX_LOG_TAG, "!!!! targetSim*RIL_CHANNEL_OFFSET > RIL_SUPPORT_CHANNELS");
            RFX_ASSERT(0);
        } else if (targetSim != 1) {
            switchMuxPath(targetSim);
        }
    }
    for(int i = 0; i < getSupportChannels(); i++) {
        RFX_LOG_D(RFX_LOG_TAG, "mux_path[%d] = %s", i, muxPath[i]);
    }
    RFX_LOG_D(RFX_LOG_TAG, "initMuxPath end");
}

void RfxChannelManager::switchMuxPath(int majorSim) {
    if (RfxRilUtils::rfxGetSimCount() < 3) {
        char mux_path_tmp[RIL_CHANNEL_OFFSET][RIL_SUPPORT_CHANNELS_MAX_NAME_LEN] = {{0}};
        // exchange mux channel for SIM switch
        // -1 is special handle for ViLTE IMS channel, they do not need to switch path
        for(int i = 0; i < RIL_CHANNEL_OFFSET-1; i++) {
            strncpy(mux_path_tmp[i], muxPath[(majorSim-1)*RIL_CHANNEL_OFFSET+i],
                    RIL_SUPPORT_CHANNELS_MAX_NAME_LEN-1);
            strncpy(muxPath[(majorSim-1)*RIL_CHANNEL_OFFSET+i], muxPath[i],
                    RIL_SUPPORT_CHANNELS_MAX_NAME_LEN-1);
            strncpy(muxPath[i], mux_path_tmp[i],
                    RIL_SUPPORT_CHANNELS_MAX_NAME_LEN-1);
        }
    } else {
        int supportChannels = getSupportChannels();
        int j = RIL_CHANNEL_OFFSET;
        for(int i = 0; i < supportChannels; i++) {
            if (i / RIL_CHANNEL_OFFSET == majorSim - 1) {  // copy major sim
                if (i % RIL_CHANNEL_OFFSET != RIL_CMD_IMS) {  // skip ViLTE IMS channel
                    strncpy(muxPath[i], muxPathInit[i % RIL_CHANNEL_OFFSET],
                            RIL_SUPPORT_CHANNELS_MAX_NAME_LEN-1);
                }
            } else {  // copy others
                if (j % RIL_CHANNEL_OFFSET != RIL_CMD_IMS) {  // skip ViLTE IMS channel
                    strncpy(muxPath[i], muxPathInit[j], RIL_SUPPORT_CHANNELS_MAX_NAME_LEN-1);
                }
                j++;
            }
        }
    }
}

const char* RfxChannelManager::channelIdToString(int channelId) {
    switch (channelId) {
        case RIL_URC:
            return "RIL_URC_READER";
        case RIL_CMD_1:
            return "RIL_CMD_READER_1";
        case RIL_CMD_2:
            return "RIL_CMD_READER_2";
        case RIL_CMD_3:
            return "RIL_CMD_READER_3";
        case RIL_CMD_4:
            return "RIL_CMD_READER_4";
        case RIL_ATCI:
            return "RIL_ATCI_READER";
        case RIL_CMD_7:
            return "RIL_CMD_RT_7";
        case RIL_CMD_8:
            return "RIL_CMD_NRT_8";
        case RIL_CMD_9:
            return "RIL_CMD_RADIO_9";
        case RIL_CMD_10:
            return "RIL_CMD_DATA2_10";
        case RIL_CMD_11:
            return "RIL_CMD_SIM2_11";
        case RIL_CMD_IMS:
            return "RIL_CMD_IMS";

        case RIL_URC2:
            return "RIL_URC2_READER";
        case RIL_CMD2_1:
            return "RIL_CMD2_READER_1";
        case RIL_CMD2_2:
            return "RIL_CMD2_READER_2";
        case RIL_CMD2_3:
            return "RIL_CMD2_READER_3";
        case RIL_CMD2_4:
            return "RIL_CMD2_READER_4";
        case RIL_ATCI2:
            return "RIL_ATCI2_READER";
        case RIL_CMD2_7:
            return "RIL_CMD2_RT_7";
        case RIL_CMD2_8:
            return "RIL_CMD2_NRT_8";
        case RIL_CMD2_9:
            return "RIL_CMD2_RADIO_9";
        case RIL_CMD2_10:
            return "RIL_CMD2_DATA2_10";
        case RIL_CMD2_11:
            return "RIL_CMD2_SIM2_11";
        case RIL_CMD2_IMS:
            return "RIL_CMD2_IMS";

        case RIL_URC3:
            return "RIL_URC3_READER";
        case RIL_CMD3_1:
            return "RIL_CMD3_READER_1";
        case RIL_CMD3_2:
            return "RIL_CMD3_READER_2";
        case RIL_CMD3_3:
            return "RIL_CMD3_READER_3";
        case RIL_CMD3_4:
            return "RIL_CMD3_READER_4";
        case RIL_ATCI3:
            return "RIL_ATCI3_READER";
        case RIL_CMD3_7:
            return "RIL_CMD3_RT_7";
        case RIL_CMD3_8:
            return "RIL_CMD3_NRT_8";
        case RIL_CMD3_9:
            return "RIL_CMD3_RADIO_9";
        case RIL_CMD3_10:
            return "RIL_CMD3_DATA2_10";
        case RIL_CMD3_11:
            return "RIL_CMD3_SIM2_11";
        case RIL_CMD3_IMS:
            return "RIL_CMD3_IMS";

        case RIL_URC4:
            return "RIL_URC4_READER";
        case RIL_CMD4_1:
            return "RIL_CMD4_READER_1";
        case RIL_CMD4_2:
            return "RIL_CMD4_READER_2";
        case RIL_CMD4_3:
            return "RIL_CMD4_READER_3";
        case RIL_CMD4_4:
            return "RIL_CMD4_READER_4";
        case RIL_ATCI4:
            return "RIL_ATCI4_READER";
        case RIL_CMD4_7:
            return "RIL_CMD4_RT_7";
        case RIL_CMD4_8:
            return "RIL_CMD4_NRT_8";
        case RIL_CMD4_9:
            return "RIL_CMD4_RADIO_9";
        case RIL_CMD4_10:
            return "RIL_CMD4_DATA2_10";
        case RIL_CMD4_11:
            return "RIL_CMD4_SIM2_11";
        case RIL_CMD4_IMS:
            return "RIL_CMD4_IMS";

        default:
            return "<unknown proxy>";
    }
}

const char* RfxChannelManager::proxyIdToString(int proxyId) {
    switch (proxyId) {
        case RIL_CMD_PROXY_1:
            return "RIL_CMD_PROXY_1";
        case RIL_CMD_PROXY_2:
            return "RIL_CMD_PROXY_2";
        case RIL_CMD_PROXY_3:
            return "RIL_CMD_PROXY_3";
        case RIL_CMD_PROXY_URC:
            return "RIL_URC_PROXY";
        case RIL_CMD_PROXY_5:
            return "RIL_CMD_PROXY_5";
        case RIL_CMD_PROXY_6:
            return "RIL_ATCI_PROXY";
        case RIL_CMD_PROXY_7:
            return "RIL_CMD_RT_7";
        case RIL_CMD_PROXY_8:
            return "RIL_CMD_NRT_8";
        case RIL_CMD_PROXY_9:
            return "RIL_CMD_RADIO_9";
        case RIL_CMD_PROXY_10:
            return "RIL_CMD_DATA2_10";
        case RIL_CMD_11:
            return "RIL_CMD_SIM2_11";
        case RIL_CMD_PROXY_IMS:
            return "RIL_CMD_IMS";

        case RIL_CMD2_PROXY_1:
            return "RIL_CMD2_PROXY_1";
        case RIL_CMD2_PROXY_2:
            return "RIL_CMD2_PROXY_2";
        case RIL_CMD2_PROXY_3:
            return "RIL_CMD2_PROXY_3";
        case RIL_CMD2_PROXY_URC:
            return "RIL_URC2_PROXY";
        case RIL_CMD2_PROXY_5:
            return "RIL_CMD2_PROXY_5";
        case RIL_CMD2_PROXY_6:
            return "RIL_ATCI2_PROXY";
        case RIL_CMD2_PROXY_7:
            return "RIL_CMD2_RT_7";
        case RIL_CMD2_PROXY_8:
            return "RIL_CMD2_NRT_8";
        case RIL_CMD2_PROXY_9:
            return "RIL_CMD2_RADIO_9";
        case RIL_CMD2_PROXY_10:
            return "RIL_CMD2_DATA2_10";
        case RIL_CMD2_11:
            return "RIL_CMD2_SIM2_11";
        case RIL_CMD2_PROXY_IMS:
            return "RIL_CMD2_IMS";

        case RIL_CMD3_PROXY_1:
            return "RIL_CMD3_PROXY_1";
        case RIL_CMD3_PROXY_2:
            return "RIL_CMD3_PROXY_2";
        case RIL_CMD3_PROXY_3:
            return "RIL_CMD3_PROXY_3";
        case RIL_CMD3_PROXY_URC:
            return "RIL_URC3_PROXY";
        case RIL_CMD3_PROXY_5:
            return "RIL_CMD3_PROXY_5";
        case RIL_CMD3_PROXY_6:
            return "RIL_ATCI3_PROXY";
        case RIL_CMD3_PROXY_7:
            return "RIL_CMD3_RT_7";
        case RIL_CMD3_PROXY_8:
            return "RIL_CMD3_NRT_8";
        case RIL_CMD3_PROXY_9:
            return "RIL_CMD3_RADIO_9";
        case RIL_CMD3_PROXY_10:
            return "RIL_CMD3_DATA2_10";
        case RIL_CMD3_11:
            return "RIL_CMD3_SIM2_11";
        case RIL_CMD3_PROXY_IMS:
            return "RIL_CMD3_IMS";

        case RIL_CMD4_PROXY_1:
            return "RIL_CMD4_PROXY_1";
        case RIL_CMD4_PROXY_2:
            return "RIL_CMD4_PROXY_2";
        case RIL_CMD4_PROXY_3:
            return "RIL_CMD4_PROXY_3";
        case RIL_CMD4_PROXY_URC:
            return "RIL_URC4_PROXY";
        case RIL_CMD4_PROXY_5:
            return "RIL_CMD4_PROXY_5";
        case RIL_CMD4_PROXY_6:
            return "RIL_ATCI4_PROXY";
        case RIL_CMD4_PROXY_7:
            return "RIL_CMD4_RT_7";
        case RIL_CMD4_PROXY_8:
            return "RIL_CMD4_NRT_8";
        case RIL_CMD4_PROXY_9:
            return "RIL_CMD4_RADIO_9";
        case RIL_CMD4_PROXY_10:
            return "RIL_CMD4_DATA2_10";
        case RIL_CMD4_11:
            return "RIL_CMD4_SIM2_11";
        case RIL_CMD4_PROXY_IMS:
            return "RIL_CMD4_IMS";

        default:
            return "<unknown proxy>";
    }
}

void RfxChannelManager::urcRegisterDone() {
    Mutex::Autolock autoLock(sSelf->mUrcMutex);
    sSelf->mUrcCount++;
    if (RfxRilUtils::rfxGetSimCount() == sSelf->mUrcCount) {
        RFX_LOG_E(RFX_LOG_TAG, "urcRegisterDone");
        sSelf->initOtherChannel();
    }
}

void RfxChannelManager::initOtherChannel() {
    RFX_LOG_E(RFX_LOG_TAG, "init other channel");
    for (int i = 0; i < getSupportChannels(); i++) {
        if ((i % RIL_CHANNEL_OFFSET) != 0) {
            sSelf->mChannels[i]->run();
        }
    }
}

int RfxChannelManager::getMainChannelSim() {
    return sMainSim;
}