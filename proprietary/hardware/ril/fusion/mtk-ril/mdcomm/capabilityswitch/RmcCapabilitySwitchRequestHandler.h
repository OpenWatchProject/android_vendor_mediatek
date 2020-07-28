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

#ifndef __RMC_CAPABILITY_SWITCH_REQUEST_HANDLER_H__
#define __RMC_CAPABILITY_SWITCH_REQUEST_HANDLER_H__

#include "utils/Mutex.h"
#include "RfxBaseHandler.h"
#include "RfxIntsData.h"
#include "RfxStringData.h"
#include "RfxMessageId.h"

using ::android::Mutex;

class RmcCapabilitySwitchRequestHandler : public RfxBaseHandler {
    RFX_DECLARE_HANDLER_CLASS(RmcCapabilitySwitchRequestHandler);

  public:
    RmcCapabilitySwitchRequestHandler(int slot_id, int channel_id);
    virtual ~RmcCapabilitySwitchRequestHandler();
    static void switchChannel(int channel, int old_major_slot, int new_major_slot);
    static void switchChannelByRealId(int channel_id1, int channel_id2);
    static void lockRestartMutex(int channel_id);
    static void unlockRestartMutex(int channel_id);
    static int getSIMInitState();
    static void setSIMInitState(int val);
    static void notifySIMInitDone(int slot_id);

  protected:
    virtual void onHandleRequest(const sp<RfxMclMessage> &msg);

  private:
    void queryBearer();
    void queryActiveMode();
    int queryMainProtocol();
    void sendRadioCapabilityDoneIfNeeded();
    void requestGetRadioCapability(const sp<RfxMclMessage> &msg);
    void requestSetRadioCapability(const sp<RfxMclMessage> &msg);
    void setSimSwitchProp(int old_major_slot, int new_major_slot);
    void switchCapability(int old_major_slot, int new_major_slot);
    void resetRadio();
    bool isVsimEnabledByRid(int rid);
    bool isPersistVsim();
    bool isVsimEnabled();
    int getActiveMode();
    void queryTplusWSupport();
    void queryKeep3GMode();
    void sendEGRAT();
    void queryNoResetSupport();
    bool waitSIMInitDone();
    void shiftCapability(int old_major_slot, int new_major_slot);
    void switchFixedCapability(int slot_a, int slot_b);
    void setCapabilityByConfig(int slot, bool is_major_slot);

  private:
    char rat_properties[MAX_SIM_COUNT][RFX_PROPERTY_VALUE_MAX] = {
        "ro.vendor.mtk_protocol1_rat_config",
        "persist.vendor.radio.mtk_ps2_rat",
        "persist.vendor.radio.mtk_ps3_rat",
        "persist.vendor.radio.mtk_ps4_rat"};
    static Mutex s_sim_init_state_mutex;
    static int s_sim_init_state;
    static Mutex s_first_instance_mutex;
    static bool s_first_instance;
};

#endif
