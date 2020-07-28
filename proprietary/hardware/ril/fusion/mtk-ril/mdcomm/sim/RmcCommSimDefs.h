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

#ifndef __RMC_COMM_SIM_DEF_H__
#define __RMC_COMM_SIM_DEF_H__

/*****************************************************************************
 * Include
 *****************************************************************************/


/*****************************************************************************
 * Enum
 *****************************************************************************/
typedef enum {
    UICC_ABSENT = 0,
    UICC_NOT_READY = 1,
    UICC_READY = 2, /* SIM_READY means the radio state is RADIO_STATE_SIM_READY */
    UICC_PIN = 3,
    UICC_PUK = 4,
    UICC_NETWORK_PERSONALIZATION = 5,
    UICC_BUSY = 9,
    UICC_NP = 10,
    UICC_NSP = 11,
    UICC_SP = 12,
    UICC_CP = 13,
    UICC_SIMP = 14,
    UICC_PERM_BLOCKED = 15, // PERM_DISABLED
    // MTK-START: AOSP SIM PLUG IN/OUT
    UICC_NO_INIT = 16,
    // MTK-END
    UICC_NP_PUK = 17,
    UICC_NSP_PUK = 18,
    UICC_SP_PUK = 19,
    UICC_CP_PUK = 20,
    UICC_SIMP_PUK = 21,
    UICC_CARD_REBOOT = 22,
    UICC_SIM_RESTRICTED = 23,
    UICC_USIM_RESTRICTED = 24,
    UICC_NS_SP = 25,
    UICC_NS_SP_PUK = 26,
    UICC_SIM_C = 27,
    UICC_SIM_C_PUK = 28,
    UICC_EMPTY_EUICC = 29,
} UICC_Status;

typedef enum {
    UICC_APP_ISIM = 0,
    UICC_APP_USIM = 1,
    UICC_APP_CSIM = 2, /* SIM_READY means the radio state is RADIO_STATE_SIM_READY */
    UICC_APP_SIM = 3,
    UICC_APP_RUIM = 4,

    UICC_APP_ID_END
} App_Id;

typedef enum {
    ENTER_PIN1,
    ENTER_PIN2,
    ENTER_PUK1,
    ENTER_PUK2,
    CHANGE_PIN1,
    CHANGE_PIN2
} UICC_Security_Operation;

// MTK-START: SIM SLOT LOCK
// SIM SLOT LOCK MD RULE
typedef enum {
    SML_NO_RULE = 0,
    SML_SIM1_SLOT_LOCK_ONLY = 1,
    SML_SIM2_SLOT_LOCK_ONLY = 2,
    SML_ALL_SIM_LOCK = 3,
    SML_LINK_LOCK_WITH_SIM1_SLOT = 4,
    SML_LINK_LOCK_WITH_SIM2_SLOT = 5,
    SML_LINK_LOCK_WITH_ANY_SLOT = 6,
    SML_SLOTA_RESTRICT_INVALID_VOICE = 7,
    SML_SLOTA_RESTRICT_REVERSE = 8,
    SML_RESTRICT_INVALID_ECC_FOR_VALID_NO_SERVICE = 9,
    SML_ALL_SIM_LOCK_AND_RSU_VZW = 10,
    SML_SLOTA_RSU_KDDI = 11,
    SML_SLOTA_RSU_TMO = 12, //OP08 RSU modem-based solution
    SML_LEGACY_MODE = 255
} SML_MD_RULE;

// SIM SLOT LOCK MD SUB-RULE
typedef enum {
    SML_SLOT_LOCK_USE_SAME_CONTEXT = 0,
    SML_SLOT_LOCK_NOT_USE_SAME_CONTEXT = 1
} SML_SLOT_LOCK_RULE;

// SIM SLOT LOCK MD INVALID SIM CAPABILITY
typedef enum {
    SML_INVALID_SIM_FULL_SERVICE = 0,
    SML_INVALID_SIM_CS_ONLY = 1,
    SML_INVALID_VOICE = 2
} SML_INVALID_SIM_CAPABILITY;

typedef enum {
    SML_LOCK_OR_UNLOCK = 3
} SML_LOCK_OPERATION;

typedef enum {
    SML_LOCK_DEVICE = 0,
    SML_UNLOCK_DEVICE = 1
} SML_LOCK_NEW_STATE;
// MTK-END

typedef enum {
    SIM_POWER_STATE_SUCCESS = 0,
    SIM_POWER_STATE_GENERIC_FAILURE = -1,
    SIM_POWER_STATE_REQUEST_IN_PROCESS = -3
} SIM_POWER_STATE;
/*****************************************************************************
 * Define
 *****************************************************************************/

#define MAX_AUTH_RSP   (256*2+27)
#define MAX_SIM_ME_LOCK_CAT_NUM 7
#define PROPERTY_GSM_GCF_TEST_MODE  "vendor.gsm.gcf.testmode"
#define PROPERTY_ICCID_PREIFX "vendor.ril.iccid.sim"
#define PROPERTY_COMMON_SLOT_SUPPORT "ro.vendor.mtk_sim_hot_swap_common_slot"
#define PROPERTY_FULL_UICC_TYPE "vendor.gsm.ril.fulluicctype"
#define PROPERTY_EXTERNAL_SIM_ENABLED "vendor.gsm.external.sim.enabled"
// MTK-START: AOSP SIM PLUG IN/OUT
#define ESIMS_CAUSE_SIM_NO_INIT 26
// MTK-END
// MTK-START: SIM SLOT LOCK
#define SML_BITMASK_SLOT_LOCK_CONTEXT    (0x01)
#define SML_BITMASK_INVALID_SIM_CAP      (0x0E)

static const char PROPERTY_SIM_SLOT_LOCK_POLICY[35] = "vendor.gsm.sim.slot.lock.policy";
static const char PROPERTY_SIM_SLOT_LOCK_STATE[35] = "vendor.gsm.sim.slot.lock.state";
static const char PROPERTY_SIM_SLOT_LOCK_DEVICE_LOCK_REMAIN_COUNT[55] =
    "vendor.gsm.sim.slot.lock.device.lock.remain.count";
static const char PROPERTY_SIM_SLOT_LOCK_SERVICE_CAPABILITY[4][50] = {
    "vendor.gsm.sim.slot.lock.service.capability",
    "vendor.gsm.sim.slot.lock.service.capability.2",
    "vendor.gsm.sim.slot.lock.service.capability.3",
    "vendor.gsm.sim.slot.lock.service.capability.4",
};
static const char PROPERTY_SIM_SLOT_LOCK_CARD_VALID[4][40] = {
    "vendor.gsm.sim.slot.lock.card.valid",
    "vendor.gsm.sim.slot.lock.card.valid.2",
    "vendor.gsm.sim.slot.lock.card.valid.3",
    "vendor.gsm.sim.slot.lock.card.valid.4",
};
// MTK-END

// Feature option value. 0: No on/off feature, NULL or 1: radio on/off, 2: SIM on/off
#define PROPERTY_SIM_CARD_ONOFF "ro.vendor.mtk_sim_card_onoff"
// Record if modem support SIM on/off feature. 0: not support, 1: support
#define PROPERTY_SIM_ONOFF_SUPPORT "vendor.ril.sim.onoff.support"
// SIM on/off state. -1: unknown, 10: SIM off, 11: SIM on
#define PROPERTY_SIM_ONOFF_STATE "vendor.ril.sim.onoff.state"

/* AOSP defines card power state.
enum CardPowerState : int32_t {
    POWER_DOWN,
    POWER_UP,
    POWER_UP_PASS_THROUGH,
};
MTK uses at+esimpower=x to set the power state. To extend sim on/off state and be compatible
with AOSP, value starts from 10 and leaves a margin for following AOSP updates.
*/
// SIM on/off state.
#define SIM_POWER_STATE_SIM_OFF 10
#define SIM_POWER_STATE_SIM_ON 11
#define SIM_POWER_STATE_SIM_SWITCHING_OFF 12
#define SIM_POWER_STATE_SIM_SWITCHING_ON 13

class RmcSimPinPukCount {

    public:
        int pin1;
        int pin2;
        int puk1;
        int puk2;
};

class RmcSimSlotLockDeviceLockInfo {

    public:
        int lock_state;
        int algo;
        int maximum;
        int remain;
};
#endif /* __RMC_COMM_SIM_DEF_H__ */

