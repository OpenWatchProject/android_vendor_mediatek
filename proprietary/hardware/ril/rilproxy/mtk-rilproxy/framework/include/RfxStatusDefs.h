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
/*
 * File name:  RfxStatusDef.h
 * Author: Jun Liu (MTK80064)
 * Description:
 *  Define the keys of shared status.
 */

#ifndef __RFX_STATUS_DEFS_H__
#define __RFX_STATUS_DEFS_H__

/*****************************************************************************
 * Enum
 *****************************************************************************/

enum RfxStatusKeyEnum {
    RFX_STATUS_KEY_START,
    /*Please add your keys below this line*/

/**
 * The card type of SIM card.
 * value type : int
 * RFX_CARD_TYPE_SIM  0x01
 * RFX_CARD_TYPE_USIM 0x02
 * RFX_CARD_TYPE_CSIM 0x04
 * RFX_CARD_TYPE_RUIM 0x08
 */
    RFX_STATUS_KEY_CARD_TYPE,
/**
 * The card type of CDMA card.
 * value type : int
 * #define UIM_CARD  1  // cdma only card but not CT card
 * #define UIM_SIM_CARD  3  // cdma dual mode card but not CT card
 * #define UNKOWN_CARD  4  //unkonw card
 * #define CT_3G_UIM_CARD  5  // CT sigle mode card
 * #define CT_UIM_SIM_CARD  6  // CT dual mode card
 * #define NEED_TO_INPUT_PIN  7  // pin locked card
 * #define CT_4G_UICC_CARD  8  // CT 4G dual mode card
 * #define NOT_CT_UICC_CARD  9  // 4G dual mode card but not CT card
 * #define LOCKED_CARD  18  // card is locked by modem
 * #define IVSR_LOST  19  // invalid sim recovery
 * #define CARD_NOT_INSERTED  255  // no card inserted
 */
    RFX_STATUS_KEY_CDMA_CARD_TYPE,
/**
 * CDMA card ready after C2K modem sends ciev 109.
 * value type : bool
 * false : cdma card is not ready. It is default value.
 * true : cdma card is ready.
 */
    RFX_STATUS_KEY_CDMA_CARD_READY,
/**
 * Ready to read CDMA card file after C2K modem sends ciev 107.
 * value type : bool
 * false : cdma card file is not ready to read. It is default value.
 * true : cdma card file is ready to read.
 */
    RFX_STATUS_KEY_CDMA_FILE_READY,
/**
 * Ct 3g dualmode card flag.
 * value type : bool
 * false : it is not a ct 3g dualmode card. It is default value.
 * true : it is a ct 3g dualmode card.
 */
    RFX_STATUS_KEY_CT3G_DUALMODE_CARD,
/**
 * Report uicc application list in OP09 A project for slot2.
 * value type : int
 * uicc_app_list = is_csim_exist | is_usim_exist | is_isim_exist (currently isim always 0)
 * is_usim_exist:2 is_csim_exist:4 (is_csim_exist | is_usim_exist): 6
 * For icc card uicc_app_list:0
 */
    RFX_STATUS_KEY_ESIMIND_APPLIST,
    RFX_STATUS_KEY_SERVICE_STATE,
    RFX_STATUS_KEY_VOICE_TYPE,
    RFX_STATUS_KEY_DATA_TYPE,
/**
 * This is used to check the modem SIM task is ready or not.
 * NOTE: It is not SIM_STATE_CHANGED READY!!
 * value type : bool
 * false: modem SIM task is not ready. It is also default value
 * true: modem SIM task is ready.
 */
    RFX_STATUS_KEY_MODEM_SIM_TASK_READY,
    RFX_STATUS_KEY_SUGGESTED_RADIO_CAPABILITY,
    RFX_STATUS_KEY_RADIO_CAPABILITY,
    RFX_STATUS_KEY_RADIO_STATE,
    RFX_STATUS_KEY_RADIO_STATE_C2K,
    RFX_STATUS_KEY_REQUEST_RADIO_POWER,
    RFX_STATUS_KEY_MODEM_POWER_OFF,
    RFX_STATUS_KEY_MODEM_VERSION,
/**
 * Active cdmalte mode slot id.
 * NOTE: Belong to non slot controller, use getNonSlotScopeStatusManager().
 * value type : int
 * CSFB_ON_SLOT
 * CDMALTE_ON_SLOT_0
 * CDMALTE_ON_SLOT_1
 */
    RFX_STATUS_KEY_ACTIVE_CDMALTE_MODE_SLOT,
/**
 * Slot id which connected to c2k rild.
 * NOTE: Belong to non slot controller, use getNonSlotScopeStatusManager().
 * value type : int
 * SIM_ID_1
 * SIM_ID_2
 */
    RFX_STATUS_KEY_CDMA_SOCKET_SLOT,

    /**
     * mode controller switch mode state.
     * NOTE: Belong to non slot controller, use getNonSlotScopeStatusManager().
     * value type : int
     * MODE_SWITCH_START
     * MODE_SWITCH_FINISH
     */
    RFX_STATUS_KEY_MODE_SWITCH,

 /**
 * State if CDMALTE mode slot ready.
 * NOTE: Belong to non slot controller, use getNonSlotScopeStatusManager().
 * value type : int
 * CDMALTE_MODE_INIT
 * CDMALTE_MODE_NOT_READY
 * CDMALTE_MODE_READY
 */
    RFX_STATUS_KEY_CDMALTE_MODE_SLOT_READY,
  /**
   * Modem off state.
   * NOTE: Belong to non slot controller, use getNonSlotScopeStatusManager().
   * value type : int
   * MODEM_OFF_IN_IDLE
   * MODEM_OFF_BY_MODE_SWITCH
   * MODEM_OFF_BY_POWER_OFF
   * MODEM_OFF_BY_SIM_SWITCH
   */
    RFX_STATUS_KEY_MODEM_OFF_STATE,
    RFX_STATUS_KEY_RAT_SWITCH_DONE,
    /**
     * rat controller  preferred network type.
     * value type : int
     * PREF_NET_TYPE_GSM_WCDMA                   GSM/WCDMA (WCDMA preferred)
     * PREF_NET_TYPE_GSM_ONLY                    GSM only
     * PREF_NET_TYPE_WCDMA                       WCDMA
     * PREF_NET_TYPE_GSM_WCDMA_AUTO              GSM/WCDMA (auto mode, according to PRL)
     * PREF_NET_TYPE_CDMA_EVDO_AUTO              CDMA and EvDo (auto mode, according to PRL)
     * PREF_NET_TYPE_CDMA_ONLY                   CDMA only
     * PREF_NET_TYPE_EVDO_ONLY                   EvDo only
     * PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO    GSM/WCDMA, CDMA, and EvDo (auto mode, according to PRL)
     * PREF_NET_TYPE_LTE_CDMA_EVDO               LTE, CDMA and EvDo
     * PREF_NET_TYPE_LTE_GSM_WCDMA               LTE, GSM/WCDMA
     * PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA     LTE, CDMA, EvDo, GSM/WCDMA
     * PREF_NET_TYPE_LTE_ONLY                    LTE only
     * PREF_NET_TYPE_LTE_WCDMA                   LTE/WCDMA
     * PREF_NET_TYPE_LTE_GSM                     LTE/GSM
     * PREF_NET_TYPE_LTE_TDD_ONLY                LTE only
     */
    RFX_STATUS_KEY_PREFERRED_NW_TYPE,

/**
 * rat controller  mode state.
 * value type : int
 *  NWS_MODE_UNKNOWN
 *  NWS_MODE_CDMALTE
 *  NWS_MODE_CSFB
 */
    RFX_STATUS_KEY_NWS_MODE,

    /**
     * Indicate the voice call count
     */
    RFX_STATUS_KEY_VOICE_CALL_COUNT,

    /**
     * Indicate is in IMS call
     *  value type: boolean
     *  true:  In emergency mode
     *  false: Not in emergency mode
     */
    RFX_STATUS_KEY_IMS_IN_CALL,

/**
 * The SIM state. It will be updated upon receiving the response of GET_SIM_STATUS.
 * And be set as NOT_READY when RADIO_UNAVAILABLE.
 * value type : int
 * RFX_SIM_STATE_NOT_READY  0
 * RFX_SIM_STATE_READY 1
 * RFX_SIM_STATE_LOCKED 2
 * RFX_SIM_STATE_ABSENT 3
 */
    RFX_STATUS_KEY_SIM_STATE,
/**
 * The IMSI of GSM application.
 * value type : String8
 * "" or IMSI
 */
    RFX_STATUS_KEY_GSM_IMSI,
/**
 * The IMSI of CDMA application.
 * value type : String8
 * "" or IMSI
 */
    RFX_STATUS_KEY_C2K_IMSI,

    /**
     * Get socket connected state
     * value type: RfxSocketState (reference to RfxSocketState.h)
     */
    RFX_STATUS_KEY_SOCKET_STATE,

    /**
     * The restrict mode state
     * value type : int
     * Not in restrict mode:   0
     * In restrict mode     :   1
     */
    RFX_STATUS_KEY_RESTRICT_MODE_STATE,

    /**
     * The raido acess family for each slot
     * value type : int
     * Return the networktype like RAF_LTE+RAF_UMTS+RAF_GSM
     */
    RFX_STATUS_KEY_SLOT_CAPABILITY,

    /**
     * The main capability slot id
     * value type : int
     * Return the main capability slot Id
     */
    RFX_STATUS_KEY_MAIN_CAPABILITY_SLOT,

    /**
    * The key will identify the c slot will be changed or not.
    * value type: int
    * WILL NOT BE CHANGED: 0
    * WILL BE CHANGED: 1
    * CHANGED DONE: 2
    */
    RFX_STATUS_KEY_CSLOT_CHANGED_STATUS,

    /**
     * The uicc subscription changed status
     * value type : int
     * Init: -1
     * Deactivate: 0
     * Activate: 1
     */
    RFX_STATUS_KEY_UICC_SUB_CHANGED_STATUS,

    /**
     * The CDMA OTA provsison state
     * value type : int
     * programming started                       1
     * service programming lock unlocked         2
     * NAM parameters downloaded successfully    3
     * MDN downloaded successfully               4
     * IMSI downloaded successfully              5
     * PRL downloaded successfully               6
     * commit successfully                       7
     * programming successfully                  8
     * programming unsuccessfully                9
     * verify SPC failed                         10
     * a key exchanged                           11
     * SSD updated                               12
     * OTAPA started                             13
     * OTAPA stopped                             14
     * Owner: network
     */
    RFX_STATUS_KEY_OTA_STATUS,

    /**
    * This is used to check if rat swithing now
    * value type : int
    * not switching : 0
    * switching : 1
    * Owner: network
    */
    RFX_STATUS_KEY_IS_RAT_MODE_SWITCHING,

    /**
      * In CDMA ECBM slot id
      * value type : int
      */
    RFX_STATUS_KEY_IN_CDMA_ECBM_SLOTID,

    /**
     * This is used to check SIM CARD is CT or not.
     * Now, it is only for Volte feature.
     * Other features mustn't use it.
     * value type : int
     * RFX_CDMA_CARD_UNKNOWN        0
     * RFX_CDMA_CARD_NO            1
     * RFX_CDMA_CARD_3G             2
     * RFX_CDMA_CARD_CT_NO3G           3
     * RFX_CDMA_CARD_ABSENT           4
     */
    RFX_STATUS_KEY_CT_CARD,

    /**
     * For set and get data settings:
     * Data enabled, data roaming and default data selected.
     */
    RFX_STATUS_KEY_DATA_SETTINGS,

    /**
    * Store default data SIM.
    * -1: Unset
    * 0:  Slot 0
    * 1:  Slot 1
    */
    RFX_STATUS_KEY_DEFAULT_DATA_SIM,

    /**
     * Slot data allow status
     * value type: int
     *  0: MD's status is allow 0
     *  1: MD's status is allow 1
     */
    RFX_STATUS_KEY_SLOT_ALLOW,

    /**
     * PDN deactivation failure's error cause
     * value type: int
     *  CME_LAST_PDN_NOT_ALLOW (151)
     */
    RFX_STATUS_KEY_PDN_DEACT_ERR_CAUSE,

    /**
     * The world mode switching state
     * value type : int
     * Switching:   0
     * Switch done: 1
     * Switch done but modem failure: -1
     */
    RFX_STATUS_KEY_WORLD_MODE_STATE,

    /**
     * The GSM world mode switching state
     * value type : int
     * Switching:   0
     * Switch done: 1
     */
    RFX_STATUS_KEY_GSM_WORLD_MODE_STATE,

    /**
     * The C2k world mode switching state
     * value type : int
     * Switching:   0
     * Switch done: 1
     */
    RFX_STATUS_KEY_CDMA_WORLD_MODE_STATE,

    /**
     * VoLTE enabled state
     * value type: integer
     *  1:  VoLTE enabled
     *  0: VoLTE disabled
     *  -1: invalid state
     */
    RFX_STATUS_KEY_VOLTE_STATE,

    /**
     * Slot data connection status is changed.
     * status will be notified only if status is changed.
     * value type : int
     * DISCONNECTED: no active data connection exist.
     * CONNECTED: at least one data connection exist.
     */
    RFX_STATUS_KEY_DATA_CONNECTION,

    /**
     * Store PCO status for radio manager.
     * value type: String
     */
    RFX_STATUS_KEY_PCO_STATUS,

    /*
     * CDMA_MO_SMS_SENDING(0)
     * CDMA_MO_SMS_SENDED(1)
     */
    RFX_STATUS_KEY_CDMA_MO_SMS_STATE,

    /*
     * Indicate whether the UE is in emergency mdoe
     * value type: boolean
     *  true:  In emergency mode
     *  false: Not in emergency mode
     */
    RFX_STATUS_KEY_EMERGENCY_MODE,

    /**
     * Emergency callback mode
     * value type: integer
     *  0: Not in emergency callback mode
     *  1: in C2K emergency callback mode
     *  2: in LWG emergency callback mode
     */
    RFX_STATUS_KEY_EMERGENCY_CALLBACK_MODE,

    /**
     * Indicate: the current cmd(at+ERAT) is success or failed.
     *  0: failed.
     *  1: success.
     */
    RFX_STATUS_KEY_RAT_SWITCH_STATUS,

    /**
     * Indicate the capability switch states
     * value type: CapabilitySwitchState enum
     */
    RFX_STATUS_KEY_CAPABILITY_SWITCH_STATE,

    /*
     * Indicate whether the UE is in switch card type flow
     * value type: boolean
     *  true:  In switch card type flow
     *  false: Not in switch card type flow
     */
    RFX_STATUS_KEY_SWITCH_CARD_TYPE_STATUS,

    /**

         * For Telephony-assistant.
         * 1) User should use interface(sendRequestStrings) to send user data.
         * 2) The first string data must be "STATUS_SYNC".
         * 3) The sencond string data should be user data.
         * 4) RILD will update RFX_STATUS key: RFX_STATUS_KEY_TELEPHONY_ASSISTANT_STATUS
         * 5) Module should register to listen status(RFX_STATUS_KEY_TELEPHONY_ASSISTANT_STATUS)
         *    changed, if want to handle it.
         * value type: String
         */
    RFX_STATUS_KEY_TELEPHONY_ASSISTANT_STATUS,

    /**
     * Indicate the sim slot lock policy value
     * value type : int
     * SML_SLOT_LOCK_POLICY_UNKNOWN        -1
     * SML_SLOT_LOCK_POLICY_NONE        0
     * SML_SLOT_LOCK_POLICY_ONLY_SLOT1        1
     * SML_SLOT_LOCK_POLICY_ONLY_SLOT2        2
     * SML_SLOT_LOCK_POLICY_ALL_SLOTS_INDIVIDUAL        3
     * SML_SLOT_LOCK_POLICY_LK_SLOT1        4
     * SML_SLOT_LOCK_POLICY_LK_SLOT2        5
     * SML_SLOT_LOCK_POLICY_LK_SLOTA        6
     * SML_SLOT_LOCK_POLICY_LK_SLOTA_RESTRICT_INVALID_CS        7
     * SML_SLOT_LOCK_POLICY_LK_SLOTA_RESTRICT_REVERSE        8
     * SML_SLOT_LOCK_POLICY_LK_SLOTA_RESTRICT_INVALID_ECC_FOR_VALID_NO_SERVICE        9
     * SML_SLOT_LOCK_POLICY_LK_SLOTA_RESTRICT_INVALID_VOICE        10
     * SML_SLOT_LOCK_POLICY_ALL_SLOTS_INDIVIDUAL_AND_RSU_VZW    11
     * SML_SLOT_LOCK_POLICY_LEGACY        255
     */
    RFX_STATUS_KEY_SML_SLOT_LOCK_POLICY,

    /**
     * Indicate the sim slot lock state value
     * value type : int
     * SML_SLOT_LOCK_POLICY_LOCK_STATE_UNKNOWN        -1
     * SML_SLOT_LOCK_POLICY_LOCK_STATE_YES        0
     * SML_SLOT_LOCK_POLICY_LOCK_STATE_NO         1
     */
    RFX_STATUS_KEY_SML_SLOT_LOCK_POLICY_LOCK_STATE,

    /**
     * Indicate the sim slot lock service capability
     * value type : int
     * SML_SLOT_LOCK_POLICY_SERVICE_CAPABILITY_UNKNOWN        -1
     * SML_SLOT_LOCK_POLICY_SERVICE_CAPABILITY_FULL         0
     * SML_SLOT_LOCK_POLICY_SERVICE_CAPABILITY_CS_ONLY        1
     * SML_SLOT_LOCK_POLICY_SERVICE_CAPABILITY_PS_ONLY        2
     * SML_SLOT_LOCK_POLICY_SERVICE_CAPABILITY_ECC_ONLY        3
     * SML_SLOT_LOCK_POLICY_SERVICE_CAPABILITY_NO_SERVICE        4
     */
    RFX_STATUS_KEY_SML_SLOT_LOCK_POLICY_SERVICE_CAPABILITY,

    /**
     * Indicate the sim slot lock valid card value
     * value type : int
     * SML_SLOT_LOCK_POLICY_VALID_CARD_UNKNOWN         -1
     * SML_SLOT_LOCK_POLICY_VALID_CARD_YES          0
     * SML_SLOT_LOCK_POLICY_VALID_CARD_NO         1
     * SML_SLOT_LOCK_POLICY_VALID_CARD_ABSENT         2
     */
    RFX_STATUS_KEY_SML_SLOT_LOCK_POLICY_VALID_CARD,

    /*
    * indicate socket state betwwen RILD and RILJ
    */
    RFX_STATUS_CONNECTION_STATE,

    /**
     * Store Preferred data SIM. Which is same as FWK's value
     * -1: Unset
     * 0:  Slot 0
     * 1:  Slot 1
     */
    RFX_STATUS_KEY_PREFERRED_DATA_SIM,

    /**
     * For USSD session control.
     * value type: Bool
     */
    RFX_STATUS_KEY_USSD_SESSION_ONGOING,

    /*Please add your keys above this line*/
    RFX_STATUS_KEY_END_OF_ENUM
};

#define RFX_CARD_TYPE_SIM  0x01
#define RFX_CARD_TYPE_USIM 0x02
#define RFX_CARD_TYPE_CSIM 0x04
#define RFX_CARD_TYPE_RUIM 0x08

#define RFX_SIM_STATE_NOT_READY 0
#define RFX_SIM_STATE_READY 1
#define RFX_SIM_STATE_LOCKED 2
#define RFX_SIM_STATE_ABSENT 3

#define RFX_CDMA_CARD_UNKNOWN     0
#define RFX_CDMA_CARD_NO          1
#define RFX_CDMA_CARD_3G          2
#define RFX_CDMA_CARD_CT_NO3G     3
#define RFX_CDMA_CARD_ABSENT      4

#define RFX_UICC_APPLIST_UNKNOWN  -1
#define RFX_UICC_APPLIST_NONE     0
#define RFX_UICC_APPLIST_USIM     2
#define RFX_UICC_APPLIST_CSIM     4


/* CDMA card type */
#define UIM_CARD  1
#define SIM_CARD  2
#define UIM_SIM_CARD  3
#define UNKOWN_CARD  4
#define CT_3G_UIM_CARD  5
#define CT_UIM_SIM_CARD  6
#define NEED_TO_INPUT_PIN  7
#define CT_4G_UICC_CARD  8
#define NOT_CT_UICC_CARD  9
#define CT_EXCEL_GG_CARD  10
#define LOCKED_CARD  18
#define IVSR_LOST  19
#define CARD_NOT_INSERTED  255

/* SIM_ID*/
#define SIM_ID_1 (0)
#define SIM_ID_2 (1)
#define SIM_ID_3 (2)
#define SIM_ID_4 (3)

/// M:[c2k 6M][NW] For RAT SWITCH*/
typedef enum {
    RAT_SWITCH_UNKNOWN = -1,
    /* Rat switch for mode controller */
    RAT_SWITCH_INIT = 0,
    /* RAT switch done for NWS */
    RAT_SWITCH_NWS = 1,
    /* RAT switch done for RIL Request and signal */
    RAT_SWITCH_NORMAL = 2,
    /* Rat switch for some restricted mode. ex: ECC redial */
    RAT_SWITCH_RESTRICT = 3,
    /* Rat switch for CT VoLTE */
    RAT_SWITCH_VOLTE = 4
} RatSwitchCaller;

/*RFX_STATUS_KEY_SML_SLOT_LOCK_POLICY value*/
typedef enum {
    SML_SLOT_LOCK_POLICY_UNKNOWN = -1,
    SML_SLOT_LOCK_POLICY_NONE = 0,
    SML_SLOT_LOCK_POLICY_ONLY_SLOT1 = 1,
    SML_SLOT_LOCK_POLICY_ONLY_SLOT2 = 2,
    SML_SLOT_LOCK_POLICY_ALL_SLOTS_INDIVIDUAL = 3,
    SML_SLOT_LOCK_POLICY_LK_SLOT1 = 4,
    SML_SLOT_LOCK_POLICY_LK_SLOT2 = 5,
    SML_SLOT_LOCK_POLICY_LK_SLOTA = 6,
    SML_SLOT_LOCK_POLICY_LK_SLOTA_RESTRICT_INVALID_CS = 7,
    SML_SLOT_LOCK_POLICY_LK_SLOTA_RESTRICT_REVERSE = 8,
    SML_SLOT_LOCK_POLICY_LK_SLOTA_RESTRICT_INVALID_ECC_FOR_VALID_NO_SERVICE = 9,
    SML_SLOT_LOCK_POLICY_LK_SLOTA_RESTRICT_INVALID_VOICE = 10,
    SML_SLOT_LOCK_POLICY_ALL_SLOTS_INDIVIDUAL_AND_RSU_VZW = 11,
    SML_SLOT_LOCK_POLICY_LEGACY = 255
} SmlSlotLockPolicy;

/*RFX_STATUS_KEY_SML_SLOT_LOCK_POLICY_LOCK_STATE value*/
typedef enum {
    SML_SLOT_LOCK_POLICY_LOCK_STATE_UNKNOWN = -1,
    SML_SLOT_LOCK_POLICY_LOCK_STATE_YES = 0,
    SML_SLOT_LOCK_POLICY_LOCK_STATE_NO = 1
} SmlSlotLockState;

/*RFX_STATUS_KEY_SML_SLOT_LOCK_POLICY_SERVICE_CAPABILITY value*/
typedef enum {
    SML_SLOT_LOCK_POLICY_SERVICE_CAPABILITY_UNKNOWN = -1,
    SML_SLOT_LOCK_POLICY_SERVICE_CAPABILITY_FULL = 0,
    SML_SLOT_LOCK_POLICY_SERVICE_CAPABILITY_CS_ONLY = 1,
    SML_SLOT_LOCK_POLICY_SERVICE_CAPABILITY_PS_ONLY = 2,
    SML_SLOT_LOCK_POLICY_SERVICE_CAPABILITY_ECC_ONLY = 3,
    SML_SLOT_LOCK_POLICY_SERVICE_CAPABILITY_NO_SERVICE = 4
} SmlSlotLockServiceCapability;

/*RFX_STATUS_KEY_SML_SLOT_LOCK_POLICY_VALID_CARD value*/
typedef enum {
    SML_SLOT_LOCK_POLICY_VALID_CARD_UNKNOWN = -1,
    SML_SLOT_LOCK_POLICY_VALID_CARD_YES = 0,
    SML_SLOT_LOCK_POLICY_VALID_CARD_NO = 1,
    SML_SLOT_LOCK_POLICY_VALID_CARD_ABSENT = 2
} SmlSlotLockValidCard;

/* MODEM_OFF_STATE*/
#define MODEM_OFF_IN_IDLE             (0)
#define MODEM_OFF_BY_MODE_SWITCH      (1)
#define MODEM_OFF_BY_POWER_OFF        (2)
#define MODEM_OFF_BY_SIM_SWITCH       (3)
#define MODEM_OFF_BY_RESET_RADIO      (4)

/* MODE_SWITCH_STATE*/
#define MODE_SWITCH_START        (1)
#define MODE_SWITCH_FINISH       (0)

/*Restrict mode state*/
#define RESTRICT_MODE_STATE_OFF (0)
#define RESTRICT_MODE_STATE_ON (1)

/*CSLOT will be changed or not*/
#define CSLOT_WILL_NOT_CHANGED (0)
#define CSLOT_WILL_BE_CHANGED (1)
#define CSLOT_WILL_CHANGED_DONE (2)

/* CDMALTE_MODE_SLOT_READY_STATE*/
#define CDMALTE_MODE_INIT        (-1)
#define CDMALTE_MODE_NOT_READY       (0)
#define CDMALTE_MODE_READY       (1)

/* ACTIVE_CDMAMODE_SLOT_ID*/
#define CSFB_ON_SLOT             (-1)
#define CDMALTE_ON_SLOT_0        SIM_ID_1
#define CDMALTE_ON_SLOT_1        SIM_ID_2

/* DATA CONNECTION STATE*/
#define DATA_STATE_DISCONNECTED        (0)
#define DATA_STATE_CONNECTED           (1)

/*Rat mode for ECC*/
#define RAT_MODE_INVALID (-1)
#define RAT_MODE_CDMA_EVDO_AUTO PREF_NET_TYPE_CDMA_EVDO_AUTO
#define RAT_MODE_LTE_WCDMA_GSM PREF_NET_TYPE_LTE_GSM_WCDMA

#define CDMA_MO_SMS_SENDING (0)
#define CDMA_MO_SMS_SENT (1)

/* RFX_STATUS_KEY_CAPABILITY_SWITCH_STATE */
typedef enum {
    CAPABILITY_SWITCH_STATE_IDLE = 0,
    CAPABILITY_SWITCH_STATE_START = 1,
} CapabilitySwitchState;

#define RFX_STATUS_DEFAULT_VALUE_ENTRY(key, value) {key, #key, value}

#define RFX_STATUS_DEFAULT_VALUE_TABLE_BEGIN                       \
        const RfxStatusManager::StatusDefaultValueEntry RfxStatusManager::s_default_value_table[] = {

#define RFX_STATUS_DEFAULT_VALUE_TABLE_END                         \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_END_OF_ENUM, RfxVariant())}

#define RFX_STATUS_DECLARE_DEFAULT_VALUE_TABLE                     \
        static const StatusDefaultValueEntry s_default_value_table[]

#define RFX_STATUS_IMPLEMENT_DEFAULT_VALUE_TABLE                                                    \
    RFX_STATUS_DEFAULT_VALUE_TABLE_BEGIN                                                            \
        /*Please add your default value below this line*/                                           \
        /*NOTE. below every line should be ended by "\" */                                          \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_CARD_TYPE, RfxVariant(-1)),                     \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_CDMA_CARD_TYPE, RfxVariant(-1)),               \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_CDMA_CARD_READY, RfxVariant(false)),               \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_CDMA_FILE_READY, RfxVariant(false)),               \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_CT3G_DUALMODE_CARD, RfxVariant(false)),               \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_ESIMIND_APPLIST, RfxVariant(-1)),               \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_SERVICE_STATE, RfxVariant()),                 \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_VOICE_TYPE, RfxVariant()),                    \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_DATA_TYPE, RfxVariant()),                     \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_MODEM_SIM_TASK_READY, RfxVariant(false)),          \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_SUGGESTED_RADIO_CAPABILITY, RfxVariant()),    \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_RADIO_CAPABILITY, RfxVariant(0)),              \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_RADIO_STATE, RfxVariant()),                   \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_RADIO_STATE_C2K, RfxVariant()),                   \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_REQUEST_RADIO_POWER, RfxVariant(false)),      \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_MODEM_POWER_OFF, RfxVariant(false)),      \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_MODEM_VERSION, RfxVariant(-1)),      \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_ACTIVE_CDMALTE_MODE_SLOT, RfxVariant(0)),      \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_MODE_SWITCH, RfxVariant(0)), \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_RESTRICT_MODE_STATE, RfxVariant(0)), \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_CDMALTE_MODE_SLOT_READY, RfxVariant(-1)),      \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_CDMA_SOCKET_SLOT, RfxVariant(0)),              \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_MODEM_OFF_STATE, RfxVariant(0)),      \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_RAT_SWITCH_DONE, RfxVariant()),               \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_SIM_STATE, RfxVariant(0)),               \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_GSM_IMSI, RfxVariant(String8(""))),      \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_C2K_IMSI, RfxVariant(String8(""))),      \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_SOCKET_STATE, RfxVariant(RfxSocketState())),  \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_NWS_MODE, RfxVariant(RfxVariant(0))),  \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_PREFERRED_NW_TYPE, RfxVariant(RfxVariant(-1))),  \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_SLOT_CAPABILITY, RfxVariant(0)), \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_UICC_SUB_CHANGED_STATUS, RfxVariant(-1)), \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_OTA_STATUS, RfxVariant(-1)), \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_CT_CARD, RfxVariant(0)), \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_DATA_SETTINGS, RfxVariant(RfxDataSettings())), \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_DEFAULT_DATA_SIM, RfxVariant(-1)), \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_PREFERRED_DATA_SIM, RfxVariant(-1)), \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_SLOT_ALLOW, RfxVariant(0)), \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_PDN_DEACT_ERR_CAUSE, RfxVariant(0)), \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_WORLD_MODE_STATE, RfxVariant(1)), \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_GSM_WORLD_MODE_STATE, RfxVariant(1)), \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_CDMA_WORLD_MODE_STATE, RfxVariant(1)), \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_VOLTE_STATE, RfxVariant(-1)), \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_DATA_CONNECTION, RfxVariant(0)),  \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_PCO_STATUS, RfxVariant(String8(""))), \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_VOICE_CALL_COUNT, RfxVariant(0)), \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_IMS_IN_CALL, RfxVariant(false)), \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_CDMA_MO_SMS_STATE, RfxVariant(1)),\
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_EMERGENCY_MODE, RfxVariant(false)), \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_EMERGENCY_CALLBACK_MODE, RfxVariant(0)), \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_CAPABILITY_SWITCH_STATE, RfxVariant(CAPABILITY_SWITCH_STATE_IDLE)), \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_SWITCH_CARD_TYPE_STATUS, RfxVariant(false)), \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_SML_SLOT_LOCK_POLICY, RfxVariant(0)), \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_SML_SLOT_LOCK_POLICY_LOCK_STATE, RfxVariant(1)), \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_SML_SLOT_LOCK_POLICY_SERVICE_CAPABILITY, RfxVariant(0)), \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_SML_SLOT_LOCK_POLICY_VALID_CARD, RfxVariant(0)), \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_CONNECTION_STATE, RfxVariant(false)), \
        RFX_STATUS_DEFAULT_VALUE_ENTRY(RFX_STATUS_KEY_USSD_SESSION_ONGOING, RfxVariant(false)), \
        /*Please add your default value above this line*/                                           \
    RFX_STATUS_DEFAULT_VALUE_TABLE_END

#endif /* __RFX_STATUS_DEFS_H__ */
