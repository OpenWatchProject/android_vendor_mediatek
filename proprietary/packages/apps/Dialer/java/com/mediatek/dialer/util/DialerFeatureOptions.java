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

package com.mediatek.dialer.util;

import com.mediatek.dialer.compat.DialerCompatEx;

import android.os.SystemProperties;

public class DialerFeatureOptions {
    // [Union Query] this feature will make a union query on Calls table and data view
    // while query the call log. So that the query result would contain contacts info.
    // and no need to query contacts info again in CallLogAdapter. It improve the call
    // log performance.
    public static final boolean CALL_LOG_UNION_QUERY = false;

    // [Dialer Global Search] Support search call log from quick search box.
    public static final boolean DIALER_GLOBAL_SEARCH = true;

    // For dynamic control the test case
    public static boolean sIsRunTestCase = false;

    /**
     * [MTK Dialer Search] whether DialerSearch feature enabled on this device
     * @return ture if allowed to enable
     */
    public static boolean isDialerSearchEnabled() {
        return true;
    }

    public static final boolean MTK_IMS_SUPPORT = SystemProperties.get(
            "persist.vendor.ims_support").equals("1");
    public static final boolean MTK_VOLTE_SUPPORT = SystemProperties.get(
            "persist.vendor.volte_support").equals("1");
    //[VoLTE ConfCall] Whether the VoLTE enhanced conference call (Launch
    //conference call directly from dialer) supported.
    public static final boolean MTK_ENHANCE_VOLTE_CONF_CALL = true;
    // Add for auto testing
    public static boolean sEnableVolteConfForTest = false;
    public static void setVolteConfForTest(boolean enable) {
        sEnableVolteConfForTest = enable;
    }

    /**
     * [VoLTE ConfCall] Whether the VoLTE enhanced conference call (Launch
     * conference call directly from dialer) supported.
     *
     * @return true if the VoLTE enhanced conference call supported
     */
    public static boolean isVolteEnhancedConfCallSupport() {
        if (sEnableVolteConfForTest) {
            return true;
        } else {
            return MTK_ENHANCE_VOLTE_CONF_CALL && MTK_IMS_SUPPORT && MTK_VOLTE_SUPPORT
                    && DialerCompatEx.isVolteEnhancedConfCallCompat();
        }
    }
//
//    /**
//     * [IMS Call] Whether the IMS call supported
//     * @return true if the IMS call supported
//     */
//    public static boolean isImsCallSupport() {
//        return MTK_IMS_SUPPORT && MTK_VOLTE_SUPPORT;
//    }


    /**
     * Whether the Light cust support is supported
     * @return true if the Light cust supported
     */
    public static boolean isOpLightCustSupport() {
        return SystemProperties.get("ro.cmcc_light_cust_support").equals("1") &&
                SystemProperties.get("ro.mtk_c2k_support").equals("1");
    }

    private static final boolean SIM_CONTACTS_FEATURE_OPTION = true;
    /**
     * [MTK SIM Contacts feature] Whether the SIM contact indicator support supported
     * @return true if the SIM contact indicator supported
     */
    public static boolean isSimContactsSupport() {
        return SIM_CONTACTS_FEATURE_OPTION && DialerCompatEx.isSimContactsCompat();
    }
}
