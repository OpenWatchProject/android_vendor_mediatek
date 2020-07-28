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

#ifndef __RP_CDMALTE_MODE_CONTROLLER_H__
#define __RP_CDMALTE_MODE_CONTROLLER_H__

/***************************************************************************** 
 * Include
 *****************************************************************************/

#include <telephony/mtk_ril.h>
#include "nw/RpNwDefs.h"
#include "RfxController.h"
#include "RpCardTypeReadyController.h"
#include "RpCdmaLteDefs.h"
#include "utils/Vector.h"
#include "RfxRootController.h"
#include "RfxAction.h"

using namespace android;
/***************************************************************************** 
 * Class RfxController
 *****************************************************************************/
typedef enum {
    STATE_PREPARE_FOR_SWITCH,
    STATE_TRIGGER_TRM,
    STATE_SWITCH_CARD_TYPE,
    STATE_START_MODE_SWITCH,
    STATE_ENTER_RESTRICT_MODE,
    STATE_RF_OFF,
    STATE_CONFIG_MODEM_STATUS,
    STATE_SWITCH_RADIO_TECHNOLOGY,
    STATE_LEAVE_RESTRICT_MODE,
    STATE_RF_ON,
    STATE_FINISH_MODE_SWITCH
} ModeSwitchState;

typedef struct {
        int card_types[MAX_RFX_SLOT_COUNT];
        int card_states[MAX_RFX_SLOT_COUNT];
        int card_modes[MAX_RFX_SLOT_COUNT];
        int old_card_modes[MAX_RFX_SLOT_COUNT];
        bool isCt3GDualMode[MAX_RFX_SLOT_COUNT];
        int cardTypeBeSwitch[MAX_RFX_SLOT_COUNT];
        int closeRadioCount;
        bool isEccCase;
        Vector<int> switchQueue;
        int rat_mode[MAX_RFX_SLOT_COUNT];
        int mOldCdmaSocketSlotId;
} ModeSwitchInfo;

typedef struct {
        AppFamilyType app_family_type;
        int network_type;
        int card_state;
        sp<RfxAction> action;
        int rat_mode;
        int card_type;
        bool isCt3GDualMode;
} RatSwitchInfo;

class RpCdmaLteModeController : public RfxController {
    RFX_DECLARE_CLASS(RpCdmaLteModeController);

public:
    RpCdmaLteModeController();
    virtual void onInit();
    virtual void onDeinit();
    virtual ~RpCdmaLteModeController();
    void onCardTypeReady(int* card_types, int* card_state, int slotNum,
        CardTypeReadyReason ctrr);
    int getDefaultNetworkType(int slotId);
    static int getActiveCdmaLteModeSlotId();
    static int getCdmaSocketSlotId();
    static void setCdmaSocketSlotId(int slotId);
    static bool isCt3GDualMode(int slotId, int cardType);

public:
    virtual bool onHandleResponse(const sp<RfxMessage>& message);
    virtual bool onHandleRequest(const sp<RfxMessage>& message);

private:
    class PendingSwitchRecord {
        public:
            PendingSwitchRecord() : m_hasPendingRecord(false) {
                for (int slotId = 0; slotId < RFX_SLOT_COUNT; slotId++) {
                    m_pending_card_types[slotId] = CARD_TYPE_INVALID;
                    m_pending_card_states[slotId] = CARD_STATE_INVALID;
                    m_is_3g_dual_mode[slotId] = false;
                }
            }

            void cancel() {
                m_hasPendingRecord = false;
            }

            void save(int* card_types, int* card_status, bool* ct3gDualMode, int slotNum) {
                assert(slotNum != RFX_SLOT_COUNT);
                m_hasPendingRecord = true;
                for (int slotId = 0; slotId < RFX_SLOT_COUNT; slotId++) {
                    m_pending_card_types[slotId] = card_types[slotId];
                    m_pending_card_states[slotId] = card_status[slotId];
                    m_is_3g_dual_mode[slotId] = ct3gDualMode[slotId];
                }
            }

            bool isPendingState() {
                return m_hasPendingRecord;
            }

        public:
            bool m_hasPendingRecord;
            int m_pending_card_types[MAX_RFX_SLOT_COUNT];
            int m_pending_card_states[MAX_RFX_SLOT_COUNT];
            bool m_is_3g_dual_mode[MAX_RFX_SLOT_COUNT];
    };

private:
    static int isSvlteSupport();
    void onRatSwitchDone(int slotId);
    void setActiveCdmaLteModeSlotId();
    void setupCdmaLteMode();
    void initCardModes();
    void startSwitchMode();
    void finishSwitchMode();
    void calculateCardMode();
    void configModemStatus(int* card_types, int slotNum = 1);
    int getCurrentNetworkType(int slotId);
    void setupSwitchQueue(int old_cdma_socket_slot);
    void doSwitchRadioTech();
    void onC2kSocketConnected(int slotId, RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);
    void switchRadioTechnology(int slotId);
    bool is4GCdmaCard(int cardType);
    bool containsCdma(int cardType);
    bool containsGsm(int cardType);
    bool containsUsim(int cardType);
    bool isUsimOnlyCard(int cardType);
    AppFamilyType getAppFamilyType(int networkType);
    bool needSwtichCardType();
    void triggerTrm();
    void getUiccType(int slotId, char* uiccType, int uiccTypeLength);
    bool isCdmaCard(char* uicc_type);
    int getFirstCCardSlot();
    int getCCardCount();
    int getRealCCardCount();
    bool waitC2KSocketConnectDone();
    int getRealCardType(int slotId);
    void enterModeSwitch(int* card_types, int* card_state, bool* ct3gDualMode,
        int slotNum);
    bool isEnableSwitchMode(int* card_types, int* card_state, bool* ct3gDualMode,
            int slotNum);
    int getNewCdmaSocketSlotId();
    void enterRestrictMode();
    void requestRadioOff();
    void onRequestRadioOffDone(int slotId);
    void leaveRestrictMode();
    void dealPendedModeSwitch();
    void responseTimedoutCallBack(const sp<RfxMessage>& message);

    bool isEmdStatusChanged(int* card_types, int slotNum = 1);
    void enqueuePendingRequest();
    void onModemOffStateChanged(int slotId, RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value);
    void onConfigModemStatusFinished(int cSlotId);
    void onWorldModeStateChange(int slotId, RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value);
    void applyPendingRecord();
    void switchCardType();
    const char* printSwitchState(ModeSwitchState state);
    void switchState(ModeSwitchState state);
    void requestRadioOn();
    bool isRemoteSimProtocolChanged(int* card_types, int slotNum);
    void switchModeForECC(const sp<RfxMessage>& message);
    void onCallCountChanged(int slotId, RfxStatusKeyEnum key,
            RfxVariant old_value, RfxVariant value);
    void handlePendedByCall();
    void updatePendingRequestInfo(int* card_types,
            bool* ct3gDualMode, bool useNew);
    void rollbackCardModeAndCdmaSlot(int oldCdmaSocketSlotId);
    bool checkEratReslut(int slotId);
    void handleSendEratFailByCall(int slotId);
    void cancelListeningCallStatus();
    bool isCommontSlotSupport();

private:
    static int sCdmaSocketSlotId;
    static int sIsSvlteSupport;
    int mCdmaLteModeSlotId;
    ModeSwitchInfo* mSwitchInfo;
    PendingSwitchRecord* mPendingRecord;
    RpCardTypeReadyController* mCardTypeReadyController;
    int mIsListeningCall;
};

#endif /* __RP_CDMALTE_MODE_CONTROLLER_H__ */
