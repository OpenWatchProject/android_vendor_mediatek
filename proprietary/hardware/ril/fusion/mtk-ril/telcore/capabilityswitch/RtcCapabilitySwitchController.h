/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#ifndef __RTC_CAPABILITY_SWITCH_CONTROLLER_H__
#define __RTC_CAPABILITY_SWITCH_CONTROLLER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxController.h"
#include "RfxVoidData.h"
#include "RfxStringData.h"
#include "RfxStringsData.h"
#include "RfxIntsData.h"
#include "RtcCapabilitySwitchChecker.h"
#include <telephony/mtk_ril.h>

/*****************************************************************************
 * Class RtcCapabilitySwitchController
 *****************************************************************************/
class RtcCapabilitySwitchController : public RfxController {
    // Required: declare this class
    RFX_DECLARE_CLASS(RtcCapabilitySwitchController);

  public:
    RtcCapabilitySwitchController();
    virtual ~RtcCapabilitySwitchController();
    void notifySetRatDone();

// Override
  protected:
    virtual bool onHandleRequest(const sp<RfxMessage> &message);
    virtual void onInit();
    virtual bool onHandleResponse(const sp<RfxMessage> &message);
    virtual bool onCheckIfRejectMessage(const sp<RfxMessage>& message,
            bool isModemPowerOff,int radioState);
    virtual bool onPreviewMessage(const sp<RfxMessage>& message);
    virtual bool onCheckIfResumeMessage(const sp<RfxMessage>& message);

  private:
    void calculateNewMainSlot(int capability, int slot, char* modemId);
    void processSetMajorSimResponse(const sp<RfxMessage> &message);
    bool isReadyForMessage(const sp<RfxMessage>& message, bool log);
    void powerOffRadio();
    void onRequestRadioOffDone(int slotId);
    void requestSetMajorSim();
    void handleRequestSwitchCapability();
    void setAllLocks();
    void setLock(int slot_id, RfxStatusKeyEnum key);
    int getLockState(int slot_id, RfxStatusKeyEnum key);
    void resetLock(int slot_id, RfxStatusKeyEnum key);
    void backupRadioPower();
    void powerOnRadio();
    void onDefaultDataChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant new_value);
    void onVolteStateChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant new_value);
    void registerStatusKeys();
    void unregisterStatusKeys();
    void onStatusKeyChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant new_value);
    void checkAndSwitchCapability();
    void updateRadioCapability();
    bool isPendingUndoneSwitch();
    int getUndoneSwitch();

  private:
    int m_request_count;
    int m_max_capability;
    int m_new_main_slot;
    int m_close_radio_count;
    int m_new_main_slot_by_modem_id;
    bool m_backup_radio_power[MAX_SIM_COUNT];
    bool m_is_started;
    sp<RfxMessage> m_pending_request;
    RtcCapabilitySwitchChecker* m_checker_controller;
};

#endif /* __RTC_CAPABILITY_SWITCH_CONTROLLER_H__ */
