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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

package com.mediatek.internal.telephony;

import android.content.Context;
import android.os.Bundle;
import android.telephony.Rlog;

import com.android.internal.telephony.Connection;
import com.mediatek.internal.telephony.MtkGsmCdmaCallTracker;

public class MtkGsmCdmaCallTrackerExt implements IMtkGsmCdmaCallTrackerExt {
    static final String TAG = "GsmCdmaCallTkrExt";
    protected Context mContext;

    public MtkGsmCdmaCallTrackerExt() {
    }

    public MtkGsmCdmaCallTrackerExt(Context context) {
        mContext = context;
    }

    public void log(String text) {
        Rlog.d(TAG, text);
    }

    /**
     * Convert dialString from extras
     *
     * @param intentExtras extra carrying source virtual line info
     * @param destination original dialString
     *
     * @return converted dialString
     */
    public String convertDialString(Bundle intentExtras, String destination) {
        return null;
    }

    /**
     * Convert driver call number by certain format
     *
     * @param formatNumber number with certain format
     * @return original number
     */
    public String convertAddress(String formatNumber) {
        return null;
    }

    protected static boolean equalsHandlesNulls(Object a, Object b) {
        return (a == null) ? (b == null) : a.equals(b);
    }

    protected static boolean
    equalsBaseDialString (String a, String b) {
        return (a == null) ? (b == null) : (b != null && a.startsWith (b));
    }
    /**
     * Check if address changed by comparing mAddress to dc Number
     * version 1
     *
     * @param converted local flag to indicate whether number with certain format
     * @param dcNumber number returned by RIL
     * @param address the cached number
     * @param convertedNumber the cached converted number
     * @return true if address is changed
     */
    public boolean isAddressChanged(boolean converted, String dcNumber,
            String address, String convertedNumber) {
        if (!equalsBaseDialString(address, dcNumber) && (!converted
                || !equalsBaseDialString(convertedNumber, dcNumber))) {
            return true;
        }
        return false;
    }

    /**
     * Check if address changed by comparing mAddress to dc Number
     * version 2
     *
     * @param converted local flag to indicate whether number with certain format
     * @param dcAddress number returned by RIL and add "+" sign if TOA=international
     * @param address the cahced number
     * @return true if address is changed
     */
    public boolean isAddressChanged(boolean converted, String dcAddress,
            String address) {
        if (!equalsHandlesNulls(address, dcAddress)) {
            return true;
        }
        return false;
    }

    /**
     * Get Address Extras from formatted number
     *
     * @param formatNumber number with certain format
     * @return extras contain target information
     */
    public Bundle getAddressExtras(String formatNumber) {
        return null;
    }

    /**
     * Check all connections belongs to same line for multiline feature.
     *
     * @param connections available connections.
     * @return true if all connections belongs to same line.
     */
    public boolean areConnectionsInSameLine(Connection[] connections) {
        return true;
    }
}
