/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.engineermode;

import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.telephony.RadioAccessFamily;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;

import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;

public class ModemCategory {

    public static final int MODEM_FDD = 1;
    public static final int MODEM_TD = 2;
    public static final int MODEM_NO3G = 3;
    public static final String FK_SIM_SWITCH = "persist.vendor.radio.simswitch";
    public static final String FK_CDMA_SLOT = "persist.vendor.radio.cdma_slot";
    public static final String FK_NR_SLOT = "persist.vendor.radio.nrslot";
    private static final String ModemType[] = {"none", "FDD", "TDD", "No3G"};
    private static final String TAG = "ModemCategory";
    /**
     * The property is used to check if the card is cdma 3G dual mode card in the slot.
     */
    private static final String[] PROPERTY_RIL_CT3G = {
            "vendor.gsm.ril.ct3g",
            "vendor.gsm.ril.ct3g.2",
            "vendor.gsm.ril.ct3g.3",
            "vendor.gsm.ril.ct3g.4",
    };
    /**
     * The property is used to get supported card type of each SIM card in the slot.
     */
    private static final String[] PROPERTY_RIL_FULL_UICC_TYPE = {
            "vendor.gsm.ril.fulluicctype",
            "vendor.gsm.ril.fulluicctype.2",
            "vendor.gsm.ril.fulluicctype.3",
            "vendor.gsm.ril.fulluicctype.4"
    };

    public static int getModemType() {
        int mode = MODEM_NO3G;
        int mask = WorldModeUtil.get3GDivisionDuplexMode();
        if ((1 == mask) || (2 == mask)) {
            mode = mask;
        }
        Elog.v(TAG, "mode = " + mode + "(" + ModemType[mode] + ")");
        return mode;
    }

    public static boolean isCdma() {
        return RatConfiguration.isC2kSupported();
    }

    public static boolean isLteSupport() {
        return (RatConfiguration.isLteFddSupported() || RatConfiguration.isLteTddSupported());
    }

    public static boolean isGsmSupport() {
        return (RatConfiguration.isGsmSupported());
    }

    public static boolean isWcdmaSupport() {
        return (RatConfiguration.isWcdmaSupported());
    }

    public static boolean isTdscdmaSupport() {
        return (RatConfiguration.isTdscdmaSupported());
    }

    public static int getCapabilitySim() {
        int phoneid = PhoneConstants.SIM_ID_1;
        String mPhoneId = SystemProperties.get(FK_SIM_SWITCH, "1");
        if ("1".equals(mPhoneId)) {
            phoneid = PhoneConstants.SIM_ID_1;
        } else if ("2".equals(mPhoneId)) {
            phoneid = PhoneConstants.SIM_ID_2;
        } else if ("3".equals(mPhoneId)) {
            phoneid = PhoneConstants.SIM_ID_3;
        } else {
            Elog.w(TAG, "read phone id error");
            return -1;
        }
        Elog.v(TAG, "main card phoneid = " + phoneid);
        return phoneid;
    }

    public static Phone getCdmaPhone() {
        Phone mPhone = null;
        try {
            if (!FeatureSupport.is93Modem()) {
                String sCdmaSlotId = SystemProperties.get(FK_CDMA_SLOT, "1");
                Elog.d(TAG, "sCdmaSlotId = " + sCdmaSlotId);
                int iCdmaSlotId = Integer.parseInt(sCdmaSlotId);
                mPhone = PhoneFactory.getPhone(iCdmaSlotId - 1);
            } else {
                mPhone = PhoneFactory.getPhone(getCapabilitySim());
            }
        } catch (Exception e) {
            Elog.e(TAG, e.getMessage());
        }
        return mPhone;
    }

    public static int getCdmaPhoneId() {
        int iCdmaSlotId = 0;
        try {
            if (!FeatureSupport.is93Modem()) {
                String sCdmaSlotId = SystemProperties.get(FK_CDMA_SLOT, "1");
                Elog.d(TAG, "sCdmaSlotId = " + sCdmaSlotId);
                iCdmaSlotId = Integer.parseInt(sCdmaSlotId) - 1;
            } else {
                iCdmaSlotId = getCapabilitySim();
            }
        } catch (Exception e) {
            Elog.e(TAG, e.getMessage());
        }
        return iCdmaSlotId;
    }

    public static boolean CheckViceSimCdmaCapability(int simtype) {
        int cdmaid = getCdmaPhoneId();
        Elog.d(TAG, "sCdmaSlotId = " + cdmaid);
        return (simtype == cdmaid ) ? true : false;
    }

    public static String[] getCdmaCmdArr(String[] cmdArray) {
        if (!FeatureSupport.is93Modem()) {
            return cmdArray;
        } else {
            String[] cmdArrayNew = new String[2];
            cmdArrayNew[0] = cmdArray[0];
            cmdArrayNew[1] = cmdArray[1];
            return cmdArrayNew;
        }
    }

    public static boolean CheckViceSimWCapability(int simtype) {
        TelephonyManager telephonyManager = TelephonyManager.getDefault();
        ITelephony iTelephony =
                ITelephony.Stub.asInterface(ServiceManager.getService("phone"));
        if (iTelephony == null || telephonyManager == null
                || telephonyManager.getSimCount() <= 1) {
            return false;
        }
        try {
            if ((iTelephony.getRadioAccessFamily(simtype, "engineermode") &
                    RadioAccessFamily.RAF_UMTS) > 0) {
                Elog.d(TAG, "SIM has W capability ");
                return true;
            }
        } catch (RemoteException e) {
            Elog.e(TAG, e.getMessage());
        }
        Elog.d(TAG, "SIM has no W capability ");
        return false;
    }

    public static boolean checkViceSimCapability(int simType, int capability) {
        TelephonyManager telephonyManager = TelephonyManager.getDefault();
        ITelephony iTelephony =
                ITelephony.Stub.asInterface(ServiceManager.getService("phone"));
        if (iTelephony == null || telephonyManager == null
                || telephonyManager.getSimCount() <= 1) {
            return false;
        }
        try {
            if ((iTelephony.getRadioAccessFamily(simType, "engineermode") &
                    capability) > 0) {
                Elog.d(TAG, "SIM has checked capability ");
                return true;
            }
        } catch (RemoteException e) {
            Elog.e(TAG, e.getMessage());
        }
        Elog.v(TAG, "SIM has no checked capability ");
        return false;
    }

    public static boolean isCapabilitySim(int mSimType) {
        int mainCard = getCapabilitySim();
        boolean isCapability = (mSimType == mainCard) ? true : false;
        Elog.v(TAG, "The card: " + mSimType + " is main card = " + isCapability);
        return isCapability;
    }

    /**
     * Check if the specified slot is CT 3G dual mode card.
     *
     * @param slotId slot ID
     * @return if it's CT 3G dual mode card
     */
    public static boolean isCt3gDualMode(int slotId) {
        if (slotId < 0 || slotId >= PROPERTY_RIL_CT3G.length) {
            Elog.e(TAG, "isCt3gDualMode: invalid slotId " + slotId);
            return false;
        }
        String result = SystemProperties.get(PROPERTY_RIL_CT3G[slotId], "");
        Elog.v(TAG, "isCt3gDualMode:  " + result);
        return ("1".equals(result));
    }


    /**
     * Return the supported card type of the SIM card in the slot.
     *
     * @param slotId the given slot id.
     * @return supported card type array.
     */
    public static String[] getSupportCardType(int slotId) {
        String property = null;
        String prop = null;
        String values[] = null;

        if (slotId < 0 || slotId >= PROPERTY_RIL_FULL_UICC_TYPE.length) {
            Elog.e(TAG, "getSupportCardType: invalid slotId " + slotId);
            return null;
        }
        prop = SystemProperties.get(PROPERTY_RIL_FULL_UICC_TYPE[slotId], "");
        if ((!prop.equals("")) && (prop.length() > 0)) {
            values = prop.split(",");
        }
        Elog.v(TAG, "getSupportCardType slotId " + slotId + ", prop value= " + prop +
                ", size= " + ((values != null) ? values.length : 0));
        return values;
    }


    public static int getSubIdBySlot(int slot) {
        int[] subId = SubscriptionManager.getSubId(slot);
        if (subId != null) {
            for (int i = 0; i < subId.length; i++) {
                Elog.v(TAG, "subId[" + i + "]: " + subId[i]);
            }
        }
        if (subId == null || subId.length == 0) {
            Elog.w(TAG, "the subid is empty");
            return -1;
        } else {
            Elog.v(TAG, "subId = " + subId[0]);
            return subId[0];
        }
    }


    public static boolean isSimReady(int slotId) {
        TelephonyManager telephonyManager = (TelephonyManager) EmApplication.getContext()
                .getSystemService(EmApplication.getContext().TELEPHONY_SERVICE);
        int status;
        if (slotId < 0) {
            status = telephonyManager.getSimState();
        } else {
            status = telephonyManager.getSimState(slotId);
        }
        Elog.v(TAG, "slotId = " + slotId + ",simStatus = " + status);
        if (status == TelephonyManager.SIM_STATE_ABSENT ||
                status == TelephonyManager.SIM_STATE_UNKNOWN)
            return false;
        return true;
    }


    public static boolean CheckViceSimNRCapability(int simtype) {
        int NRSlotId;
        String sNRSlotId = SystemProperties.get(FK_NR_SLOT, "0");
        if (!RatConfiguration.isNrSupported()) {
            Elog.d(TAG, "project not support NR");
            return false;
        } else {
            Elog.d(TAG, "sNRSlotId = " + sNRSlotId);
            NRSlotId = Integer.parseInt(sNRSlotId) - 1;
            if (NRSlotId == -1) {
                Elog.d(TAG, "both sim support NR");
                return true;
            } else {
                Elog.d(TAG, "sim " + NRSlotId + " support NR");
                return (simtype == NRSlotId) ? true : false;
            }
        }
    }
}