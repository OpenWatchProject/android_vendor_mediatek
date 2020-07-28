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

package com.mediatek.ims;

import android.os.Build;
import android.os.SystemProperties;
import android.telephony.Rlog;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;

import com.mediatek.ims.MtkImsConstants;

public class ImsCommonUtil {

    public static final boolean ENGLOAD = "eng".equals(Build.TYPE);

    private static final String LOG_TAG = "ImsCommonUtil";

    /**
     * Utility function to convert byte array to hex string.
     *
     * @param bytes the byte array value.
     * @return the hex string value.
     * @hide
     */
    public static String bytesToHex(byte[] bytes) {
        char[] hexArray = "0123456789abcdef".toCharArray();
        char[] hexChars = new char[bytes.length * 2];
        for (int j = 0; j < bytes.length; j++) {
            int v = bytes[j] & 0xFF;
            hexChars[j * 2] = hexArray[v >>> 4];
            hexChars[j * 2 + 1] = hexArray[v & 0x0F];
        }
        return new String(hexChars);
    }

    /**
     * Utility function to convert hex string to byte array.
     *
     * @param inputString the hex string value.
     * @return the byte array.
     * @hide
     */
    public static byte[] hexToBytes(String inputString) {
        if (inputString == null) {
            return null;
        }
        int len = inputString.length();
        Rlog.d(LOG_TAG, "hexToBytes: inputLen = " + len);
        byte[] result = new byte[len / 2];
        int[] temp = new int[2];
        for (int i = 0; i < len / 2; i++) {
            temp[0] = inputString.charAt(i * 2);
            temp[1] = inputString.charAt(i * 2 + 1);
            for (int j = 0; j < 2; j++) {
                if (temp[j] >= 'A' && temp[j] <= 'F') {
                    temp[j] = temp[j] - 'A' + 10;
                } else if (temp[j] >= 'a' && temp[j] <= 'f') {
                    temp[j] = temp[j] - 'a' + 10;
                } else if (temp[j] >= '0' && temp[j] <= '9') {
                    temp[j] = temp[j] - '0';
                } else {
                    return null;
                }
            }
            result[i] = (byte) (temp[0] << 4);
            result[i] |= temp[1];
        }
        return result;
    }

    /**
     * to get main capability phone id.
     *
     * @return The phone id with highest capability.
     * @hide
     */
    public static int getMainCapabilityPhoneId() {
        int phoneId = SystemProperties.getInt(ImsConstants.PROPERTY_CAPABILITY_SWITCH, 1) - 1;
        if (phoneId < 0 || phoneId >= TelephonyManager.getDefault().getPhoneCount()) {
            phoneId = SubscriptionManager.INVALID_PHONE_INDEX;
        }
        if(ENGLOAD) Rlog.d(LOG_TAG, "getMainCapabilityPhoneId = " + phoneId);
        return phoneId;
    }

    public static boolean isDssNoResetSupport() {
        if (SystemProperties.get("vendor.ril.simswitch.no_reset_support").equals("1")) {
            if(ENGLOAD) Rlog.d(LOG_TAG, "return true for isDssNoResetSupport");
            return true;
        }
        if(ENGLOAD) Rlog.d(LOG_TAG, "return false for isDssNoResetSupport");
            return false;
    }

    public static boolean supportMdAutoSetupIms() {
        if (SystemProperties.get(ImsConstants.SYS_PROP_MD_AUTO_SETUP_IMS).equals("1")) {
            return true;
        }
        return false;
    }

    /**
     * Check whether MIMS is supported.
     * #TODO remvoe every usage of this API and replace with MtkImsManager instead
     *
     * @return true if support MIMS, false for single IMS
     * @hide
     */
    public static boolean supportMims() {
        return (SystemProperties.getInt(MtkImsConstants.MULTI_IMS_SUPPORT, 1) > 1);
    }

    public static int getMainPhoneIdForSingleIms() {
        // duplication from RadioCapabilitySwitchUtil
        int phoneId = 0;
        phoneId = SystemProperties.getInt(ImsConstants.PROPERTY_CAPABILITY_SWITCH, 1) - 1;
        Rlog.d(LOG_TAG, "[getMainPhoneIdForSingleIms] : " + phoneId);
        return phoneId;
    }

    // Get the protocol stack id by slot id, the slot is 0-based, protocol stack id is 1-based
    public static int getProtocolStackId(int slot) {
        // The major sim is mapped to PS1, from 93 modem which supports dynamic sim switch without
        // modem reset, the other sims are mapped to PS2~PS4 in ascending order; For the modem
        // before 93, the major sim is switched protocol stack with SIM1.
        int majorSlot = getMainCapabilityPhoneId();
        if (slot == majorSlot) {
            return 1;
        }
        if (isDssNoResetSupport()) {
            if (slot < majorSlot) {
                return slot + 2;
            }
        } else if (slot == 0){
            return majorSlot + 1;
        }
        return slot + 1;
    }

    public static boolean isPhoneIdSupportIms(int phoneId) {
        boolean result = false;

        int isImsSupport = SystemProperties.getInt(MtkImsConstants.PROPERTY_IMS_SUPPORT, 0);
        int mimsCount = SystemProperties.getInt(MtkImsConstants.MULTI_IMS_SUPPORT, 1);

        if (isImsSupport == 0 || !SubscriptionManager.isValidPhoneId(phoneId)) {
            Rlog.d(LOG_TAG, "isPhoneIdSupportIms(), not support IMS, phoneId:" + phoneId);
            return result;
        }

        if (mimsCount == 1) {
            if (getMainCapabilityPhoneId() == phoneId) {
                result = true;
            }
        } else {
            int protocalStackId = getProtocolStackId(phoneId);

            if (protocalStackId <= mimsCount) {
                result = true;
            } else {
                Rlog.d(LOG_TAG, "isPhoneIdSupportIms(), mimsCount:" + mimsCount + ", phoneId:" +
                        phoneId  + ", protocalStackId:" + protocalStackId +
                        ", MainCapabilityPhoneId:" + getMainCapabilityPhoneId());
            }
        }
        return result;
    }

}
