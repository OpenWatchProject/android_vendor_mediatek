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

#ifndef __RTC_MODE_SWITCH_CONTROLLER_H__
#define __RTC_MODE_SWITCH_CONTROLLER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/

#include "RfxController.h"
#include "RtcCardTypeReadyController.h"
#include "RtcModeDefs.h"
#include "utils/Vector.h"

using namespace android;
/*****************************************************************************
 * Class RtcModeSwitchController
 *****************************************************************************/
typedef enum {
    STATE_START_MODE_SWITCH,
    STATE_UPDATE_RADIO_CAPABILITY,
    STATE_SWITCH_RADIO_TECHNOLOGY,
    STATE_FINISH_MODE_SWITCH
} ModeSwitchState;

typedef struct {
        int card_type[MAX_RFX_SLOT_ID];
        int card_state[MAX_RFX_SLOT_ID];
        bool isCt3GDualMode[MAX_RFX_SLOT_ID];
        int rat_mode[MAX_RFX_SLOT_ID];
        Vector<int> switchQueue;
        int ct3gStatus[MAX_RFX_SLOT_ID];
} ModeSwitchInfo;

typedef struct {
        int card_type;
        int card_state;
        bool isCt3GDualMode;
        int rat_mode;
        /* Indicate who triggers to switch: AP or GMSS
         * and switching to which card type: SIM or RUIM.
         * value type: int
         *  -1: default
         *  1: AP_TRIGGER_SWITCH_SIM
         *  2: GMSS_TRIGGER_SWITCH_SIM
         *  3: AP_TRIGGER_SWITCH_RUIM
         *  4: GMSS_TRIGGER_SWITCH_RUIM
         */
        int ct3gStatus;
} RatSwitchInfo;

class RtcModeSwitchController : public RfxController {
    RFX_DECLARE_CLASS(RtcModeSwitchController);

public:
    RtcModeSwitchController();
    virtual ~RtcModeSwitchController();
    virtual void onInit();
    virtual void onDeinit();
    void onCardTypeReady(int* card_type, int* card_state, int slotNum);
    int getCCapabilitySlotId();

public:
    virtual bool onHandleResponse(const sp<RfxMessage>& message);
    virtual bool onHandleRequest(const sp<RfxMessage>& message);
    virtual bool onCheckIfRejectMessage(const sp<RfxMessage>& message, bool isModemPowerOff,
            int radioState);

private:
    class PendingSwitchRecord {
        public:
            PendingSwitchRecord();
            void cancel();
            void save(int* card_type, int* card_state, bool* isCt3GDualMode,
                    int* rat_mode, int slotNum, int* ct3gStatus);
            bool isPendingState();
            int updateCardStateIfRecordCovered(int card_state, int slotId);

        public:
            bool m_hasPendingRecord;
            int m_pending_card_type[MAX_RFX_SLOT_ID];
            int m_pending_card_state[MAX_RFX_SLOT_ID];
            bool m_pending_isCt3gDualMode[MAX_RFX_SLOT_ID];
            int m_pending_rat_mode[MAX_RFX_SLOT_ID];
            int m_pending_ct3gStatus[MAX_RFX_SLOT_ID];
    };

private:
    bool isEnableSwitchMode(int* card_type, int* card_state, bool* is_ct3g_dualmode,
            int* rat_mode, int slotNum, int* ct3gStatus);
    void onModemOffStateChanged(int slotId, RfxStatusKeyEnum key, RfxVariant old_value,
            RfxVariant value);
    void onCapabilityReported(int slotId, RfxStatusKeyEnum key, RfxVariant old_value,
            RfxVariant value);
    void onCallCountChanged(int slotId, RfxStatusKeyEnum key, RfxVariant old_value,
            RfxVariant value);
    void onIccidChanged(int slotId, RfxStatusKeyEnum key, RfxVariant oldValue,
            RfxVariant value);
    int getCallingSlot();
    int getCallingSlotWithCChangeCase();
    void handlePendedByCall(int slotId);
    void enterModeSwitch(int* card_type, int* card_state,  bool* is_ct3g_dualmode, int* rat_mode,
            int slotNum, int* ct3gStatus);
    void startSwitchMode();
    void finishSwitchMode();
    void setupSwitchQueue();
    void doSwitchRadioTech();
    void switchRadioTechnology(int slotId);
    void onRatSwitchDone(int slotId, int error);
    void handleSendEratFailByCall(int slotId);
    bool hasCErat(int type);
    bool hasCRadioCapability(int radioCapability);
    bool needSwtichCardType();
    void switchModeForECC(const sp<RfxMessage>& message);
    void enqueuePendingRequest();
    void dealPendedModeSwitch();
    void applyPendingRecord();
    void updateRadioCapability();
    int calculateCCapabilitySlot();
    bool containsCdma(int cardType);
    bool isCt3GDualMode(int slotId, int cardType);
    void setCCapabilitySlotId(int slotId);
    void initCCapabilitySlotId();
    const char* printSwitchState(ModeSwitchState state);
    void switchState(ModeSwitchState state);
    const char* printModemOffState(int state);
    bool hasCall();
    void handleIccidNotReady(int slotId);
    void handlePendedByIccid(int slotId);
    int getIccidNotReadySlot();
    int getEMCSSlot();
    void handlePendedByEMCS(int slotId);
    void onEMCSChanged(int slotId, RfxStatusKeyEnum key, RfxVariant oldValue, RfxVariant value);

private:
    // Update mCCapabilitySlot must update its system property.
    int mCCapabilitySlot;
    int mOldCCapabilitySlot;
    // Mode switch need wait all slot`s radio capability ready, it is for count slot num.
    int mCapabilityReportedCount;
    ModeSwitchInfo* mSwitchInfo;
    PendingSwitchRecord* mPendingRecord;
    RtcCardTypeReadyController* mCardTypeReadyController;
};

#endif /* __RTC_MODE_SWITCH_CONTROLLER_H__ */
