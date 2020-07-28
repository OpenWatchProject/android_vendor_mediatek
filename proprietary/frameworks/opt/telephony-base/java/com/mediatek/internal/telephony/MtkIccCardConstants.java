/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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

package com.mediatek.internal.telephony;

import android.annotation.ProductApi;
import android.telephony.TelephonyManager;
import com.android.internal.telephony.IccCardConstants;

public class MtkIccCardConstants extends IccCardConstants{

    // MTK-START: SIM ME LOCK
    // Added by M begin
    /**
     * NETWORK_SUBSET means ICC is locked on NETWORK SUBSET PERSONALIZATION.
     * @internal
     */
    public static final String INTENT_VALUE_LOCKED_NETWORK_SUBSET = "NETWORK_SUBSET";
    /**
     * CORPORATE means ICC is locked on CORPORATE PERSONALIZATION.
     * @internal
     */
    public static final String INTENT_VALUE_LOCKED_CORPORATE = "CORPORATE";
    /**
     * SERVICE_PROVIDER means ICC is locked on SERVICE_PROVIDER PERSONALIZATION.
     * @internal
     */
    public static final String INTENT_VALUE_LOCKED_SERVICE_PROVIDER = "SERVICE_PROVIDER";
    /**
     * SIM means ICC is locked on SIM PERSONALIZATION.
     * @internal
     */
    public static final String INTENT_VALUE_LOCKED_SIM = "SIM";
    /**
     * NETWORK_PUK means ICC is locked on NETWORK_PUK PERSONALIZATION.
     * @internal
     */
    public static final String INTENT_VALUE_LOCKED_NETWORK_PUK = "NETWORK_PUK";
    /**
     * NETWORK_SUBSET_PUK means ICC is locked on NETWORK SUBSET_PUK PERSONALIZATION.
     * @internal
     */
    public static final String INTENT_VALUE_LOCKED_NETWORK_SUBSET_PUK = "NETWORK_SUBSET_PUK";
    /**
     * CORPORATE_PUK means ICC is locked on CORPORATE_PUK PERSONALIZATION.
     * @internal
     */
    public static final String INTENT_VALUE_LOCKED_CORPORATE_PUK = "CORPORATE_PUK";
    /**
     * SERVICE_PROVIDER_PUK means ICC is locked on SERVICE_PROVIDER_PUK PERSONALIZATION.
     * @internal
     */
    public static final String INTENT_VALUE_LOCKED_SERVICE_PROVIDER_PUK = "SERVICE_PROVIDER_PUK";
    /**
     * SIM_PUK means ICC is locked on SIM_PUK PERSONALIZATION.
     * @internal
     */
    public static final String INTENT_VALUE_LOCKED_SIM_PUK = "SIM_PUK";

    /**
     * NS_SP means ICC is locked on NS_SP PERSONALIZATION.
     * @internal
     */
    public static final String INTENT_VALUE_LOCKED_NS_SP = "NS_SP";

    /**
     * NS_SP_PUK means ICC is locked on NS_SP_PUK PERSONALIZATION.
     * @internal
     */
    public static final String INTENT_VALUE_LOCKED_NS_SP_PUK = "NS_SP_PUK";

    /**
     * SIM_C means ICC is locked on SIM_C PERSONALIZATION.
     * @internal
     */
    public static final String INTENT_VALUE_LOCKED_SIM_C = "SIM_C";

    /**
     * SIM_C_PUK means ICC is locked on SIM_C_PUK PERSONALIZATION.
     * @internal
     */
    public static final String INTENT_VALUE_LOCKED_SIM_C_PUK = "SIM_C_PUK";

    // Added by M end
    // SIM ME LOCK - Start
    // lock policy
    public static final int SML_SLOT_LOCK_POLICY_UNKNOWN = -1;
    public static final int SML_SLOT_LOCK_POLICY_NONE = 0;
    public static final int SML_SLOT_LOCK_POLICY_ONLY_SLOT1 = 1;
    public static final int SML_SLOT_LOCK_POLICY_ONLY_SLOT2 = 2;
    public static final int SML_SLOT_LOCK_POLICY_ALL_SLOTS_INDIVIDUAL = 3;
    public static final int SML_SLOT_LOCK_POLICY_LK_SLOT1 = 4;
    public static final int SML_SLOT_LOCK_POLICY_LK_SLOT2 = 5;
    public static final int SML_SLOT_LOCK_POLICY_LK_SLOTA = 6;
    public static final int SML_SLOT_LOCK_POLICY_LK_SLOTA_RESTRICT_INVALID_CS = 7;
    public static final int SML_SLOT_LOCK_POLICY_LK_SLOTA_RESTRICT_REVERSE = 8;
    public static final int SML_SLOT_LOCK_POLICY_LK_SLOTA_RESTRICT_INVALID_ECC_FOR_VALID_NO_SERVICE
            = 9;
    public static final int SML_SLOT_LOCK_POLICY_LK_SLOTA_RESTRICT_INVALID_VOICE = 10;
    public static final int SML_SLOT_LOCK_POLICY_ALL_SLOTS_INDIVIDUAL_AND_RSU_VZW = 11;
    public static final int SML_SLOT_LOCK_POLICY_LK_SLOTA_RSU_KDDI = 13;
    public static final int SML_SLOT_LOCK_POLICY_LK_SLOTA_RSU_TMO = 15;
    public static final int SML_SLOT_LOCK_POLICY_LEGACY = 255;
    // lock capability
    public static final int SML_SLOT_LOCK_POLICY_SERVICE_CAPABILITY_UNKNOWN = -1;
    public static final int SML_SLOT_LOCK_POLICY_SERVICE_CAPABILITY_FULL = 0;
    public static final int SML_SLOT_LOCK_POLICY_SERVICE_CAPABILITY_CS_ONLY = 1;
    public static final int SML_SLOT_LOCK_POLICY_SERVICE_CAPABILITY_PS_ONLY = 2;
    public static final int SML_SLOT_LOCK_POLICY_SERVICE_CAPABILITY_ECC_ONLY = 3;
    public static final int SML_SLOT_LOCK_POLICY_SERVICE_CAPABILITY_NO_SERVICE = 4;
    // valid card
    public static final int SML_SLOT_LOCK_POLICY_VALID_CARD_UNKNOWN = -1;
    public static final int SML_SLOT_LOCK_POLICY_VALID_CARD_YES = 0;
    public static final int SML_SLOT_LOCK_POLICY_VALID_CARD_NO = 1;
    public static final int SML_SLOT_LOCK_POLICY_VALID_CARD_ABSENT = 2;
    // lock state
    public static final int SML_SLOT_LOCK_POLICY_LOCK_STATE_UNKNOWN = -1;
    public static final int SML_SLOT_LOCK_POLICY_LOCK_STATE_YES = 0;
    public static final int SML_SLOT_LOCK_POLICY_LOCK_STATE_NO = 1;
    // unlock
    public static final int SML_SLOT_LOCK_POLICY_UNLOCK_UNKNOWN = -1;
    public static final int SML_SLOT_LOCK_POLICY_UNLOCK_NO_LOCK_POLICY = 0;
    public static final int SML_SLOT_LOCK_POLICY_UNLOCK_INCORRECT_PASSWORD = 1;
    public static final int SML_SLOT_LOCK_POLICY_UNLOCK_GENERAL_FAIL = 2;
    public static final int SML_SLOT_LOCK_POLICY_UNLOCK_SUCCESS = 3;

    /**
     * DEVICE_LOCK_POLICY means lock rule-by device.
     * @internal
     */
    public static final String INTENT_KEY_SML_SLOT_DEVICE_LOCK_POLICY = "DEVICE_LOCK_POLICY";

    /**
     * DEVICE_LOCK_STATE means lock state-by device.
     * @internal
     */
    public static final String INTENT_KEY_SML_SLOT_DEVICE_LOCK_STATE = "DEVICE_LOCK_STATE";

    /**
     * SIM_SERVICE_CAPABILITY means The service capability of the current slot sim card should have
     * –by slot.
     * @internal
     */
    public static final String INTENT_KEY_SML_SLOT_SIM_SERVICE_CAPABILITY= "SIM_SERVICE_CAPABILITY";

    /**
     * SIM_VALID means whether the current slot sim is valid or not–by slot.
     * @internal
     */
    public static final String INTENT_KEY_SML_SLOT_SIM_VALID = "SIM_VALID";

    /**
     * DETECTED_TYPE means the detected type.
     * @internal
     */
    public static final String INTENT_KEY_SML_SLOT_DETECTED_TYPE = "DETECTED_TYPE";

    /**
     * SML_SIM_COUNT means the SIM ME LOCK sim count.
     * @internal
     */
    public static final String INTENT_KEY_SML_SLOT_SIM_COUNT = "SML_SIM_COUNT";

    /**
     * SML_SIM1_VALID means the SIM ME LOCK SIM 1 valid or not.
     * @internal
     */
    public static final String INTENT_KEY_SML_SLOT_SIM1_VALID = "SML_SIM1_VALID";

    /**
     * SML_SIM2_VALID means the SIM ME LOCK SIM 2 valid or not.
     * @internal
     */
    public static final String INTENT_KEY_SML_SLOT_SIM2_VALID = "SML_SIM2_VALID";

    /**
     * SML_SIM3_VALID means the SIM ME LOCK SIM 3 valid or not.
     * @internal
     */
    public static final String INTENT_KEY_SML_SLOT_SIM3_VALID = "SML_SIM3_VALID";

    /**
     * SML_SIM4_VALID means the SIM ME LOCK SIM 4 valid or not.
     * @internal
     */
    public static final String INTENT_KEY_SML_SLOT_SIM4_VALID = "SML_SIM4_VALID";
    // SIM ME LOCK - End
    // MTK-END

    /**
     * This card type is report by CDMA(VIA) modem.
     * for CT requset to detect card type then give a warning
     * @deprecated - use IccCardType instead
     */
    @ProductApi
    @Deprecated public enum CardType {
        UIM_CARD(1),             //ICC structure, non CT UIM card
        SIM_CARD(2),             //ICC structure, non CT SIM card
        UIM_SIM_CARD(3),         //ICC structure, non CT dual mode card
        UNKNOW_CARD(4),          //card is present, but can't detect type
        CT_3G_UIM_CARD(5),       //ICC structure, CT 3G UIM card
        CT_UIM_SIM_CARD(6),      //ICC structure, CT 3G dual mode card
        PIN_LOCK_CARD(7),        //this card need PIN
        CT_4G_UICC_CARD(8),      //UICC structure, CT 4G dual mode UICC card
        NOT_CT_UICC_CARD(9),     //UICC structure, Non CT 4G dual mode UICC card
        CT_EXCEL_GG_CARD(10),    //UICC structure, CTEXCEL GG card
        LOCKED_CARD(18),         //card is locked
        CARD_NOT_INSERTED(255);  //card is not inserted

        private int mValue;

        public int getValue() {
            return mValue;
        }

        /**
         * Get CardType from integer.
         * ASSERT: Please DON'T directly use CardType.values(), otherwise JE will occur
         * @param cardTypeInt for cardType index.
         * @return CardType.
         */
        public static MtkIccCardConstants.CardType getCardTypeFromInt(int cardTypeInt) {
            CardType cardType = UNKNOW_CARD;
            CardType[] cardTypes = CardType.values();
            for (int i = 0; i < cardTypes.length; i++) {
                if (cardTypes[i].getValue() == cardTypeInt) {
                    cardType = cardTypes[i];
                    break;
                }
            }
            return cardType;
        }

        /**
         * Check if it is 4G card.
         * @return true if it is 4G card
         */
        public boolean is4GCard() {
            return ((this == CT_4G_UICC_CARD) || (this == NOT_CT_UICC_CARD));
        }

        private CardType(int value) {
            mValue = value;
        }
    }

    // External SIM [START]
    /**
     *  Use the detemine VSIM card type.
     */
    public enum VsimType {
        LOCAL_SIM,
        REMOTE_SIM,
        SOFT_AKA_SIM,
        PHYSICAL_AKA_SIM,
        PHYSICAL_SIM;

        public boolean isUserDataAllowed() {
            return ((this == SOFT_AKA_SIM) || (this == PHYSICAL_AKA_SIM));
        }

        public boolean isDataRoamingAllowed() {
            return ((this == SOFT_AKA_SIM) || (this == REMOTE_SIM));
        }

        public boolean isAllowVsimConnection() {
            return ((this == SOFT_AKA_SIM) || (this == PHYSICAL_AKA_SIM));
        }

        public boolean isAllowReqNonVsimNetwork() {
            return (this != SOFT_AKA_SIM);
        }

        public boolean isAllowOnlyVsimNetwork() {
            return (this == SOFT_AKA_SIM);
        }

        public boolean isVsimCard() {
            return ((this != PHYSICAL_SIM) && (this != PHYSICAL_AKA_SIM));
        }
    }
    // External SIM [END]
}
