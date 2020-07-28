/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2011. All rights reserved.
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
 */

package com.mediatek.dialer.compat;

import android.content.Context;
import android.telecom.TelecomManager;
import android.util.Log;

/**
 * [portable]Dialer new features compatible
 */
public class DialerCompatEx {
    private static final String TAG = DialerCompatEx.class.getSimpleName();

    //[MTK SIM Contacts feature] INDICATE_PHONE_SIM,IS_SDN_CONTACT
    private static final String COMPAT_CLASS_MTKCONTACTSCONTRACT =
            "com.mediatek.provider.MtkContactsContract";
    private static final String COMPAT_FIELD_INDICATE_PHONE_SIM= "INDICATE_PHONE_SIM";
    private static Boolean sSimContactsCompat = null;

    public static boolean isSimContactsCompat() {
        if (sSimContactsCompat == null) {
            sSimContactsCompat = DialerCompatExUtils.isClassExits(COMPAT_CLASS_MTKCONTACTSCONTRACT);
            Log.d(TAG, "init isSimContactsCompat got " + sSimContactsCompat);
        }
        return sSimContactsCompat;
    }

    /* package */static void setSimContactsCompat(Boolean supported) {
        Log.d(TAG, "setSimContactsCompat supported: " + supported);
        sSimContactsCompat = supported;
    }

    // [VoLTE ConfCall] Whether the VoLTE enhanced conference call (Launch
    // conference call directly from dialer) supported.
    // here use carrier config in telecom
    private static final String COMPAT_CLASS_MTK_CARRIER_CONFIG =
            "mediatek.telephony.MtkCarrierConfigManager";
    private static final String COMPAT_FIELD_CAPABILITY_VOLTE_CONFERENCE_ENHANCED =
            "MTK_KEY_VOLTE_CONFERENCE_ENHANCED_ENABLE_BOOL";
    private static Boolean sVolteEnhancedConfCallCompat = null;

    public static boolean isVolteEnhancedConfCallCompat() {
        if (sVolteEnhancedConfCallCompat == null) {
            sVolteEnhancedConfCallCompat = DialerCompatExUtils.isFieldAvailable(
                COMPAT_CLASS_MTK_CARRIER_CONFIG, COMPAT_FIELD_CAPABILITY_VOLTE_CONFERENCE_ENHANCED);
            Log.d(TAG, "init isVolteEnhancedConfCallCompat got " + sVolteEnhancedConfCallCompat);
        }
        return sVolteEnhancedConfCallCompat;
    }

    private static final String COMPAT_CLASS_TMEX = "com.mediatek.telephony.MtkTelephonyManagerEx";
    private static final String COMPAT_METHOD_WFC = "isWifiCallingEnabled";
    private static Boolean sWfcCompat = null;
    public static boolean isWfcCompat() {
        if (sWfcCompat == null) {
            sWfcCompat = DialerCompatExUtils.isMethodAvailable(COMPAT_CLASS_TMEX, COMPAT_METHOD_WFC,
                int.class);
            Log.d(TAG, "init isWfcCompat:" + sWfcCompat);
        }
        return sWfcCompat;
    }
//    /**
//     * Blocked Number Permission check for portable. corresponding to
//     * BlockedNumberProvider, only default or system dialer can read/write its db.
//     */
//    public static boolean isDefaultOrSystemDialer(Context context) {
//        String self = context.getApplicationInfo().packageName;
//        final TelecomManager telecom = context.getSystemService(TelecomManager.class);
//        if (self.equals(telecom.getDefaultDialerPackage())
//                || self.equals(telecom.getSystemDialerPackage())) {
//            return true;
//        }
//        Log.d(TAG, "isDefaultOrSystemDialer, return false");
//        return false;
//    }
}
