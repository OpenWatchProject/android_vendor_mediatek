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

package com.mediatek.op08.telephony;

import android.content.Context;
import android.os.Bundle;
import android.telephony.Rlog;
import android.text.TextUtils;

import com.android.internal.telephony.Connection;
import com.mediatek.internal.telephony.MtkGsmCdmaCallTrackerExt;

import mediatek.telecom.MtkTelecomManager;

public class Op08GsmCdmaCallTrackerExt extends MtkGsmCdmaCallTrackerExt {
    static final String TAG = "Op08GsmCdmaCallTrackerExt";

    public Op08GsmCdmaCallTrackerExt(Context context) {
        super(context);
    }

    @Override
    public String convertDialString(Bundle intentExtras, String destination) {
        String source = null;
        if (intentExtras != null) {
            if (intentExtras.containsKey(MtkTelecomManager.EXTRA_VIRTUAL_LINE_NUMBER)) {
                source = intentExtras.getString(MtkTelecomManager.EXTRA_VIRTUAL_LINE_NUMBER);
            }
        }

        if (source != null) {
            return source + "_to_" + destination;
        }
        return null;
    }

    @Override
    public String convertAddress(String formatNumber) {
        String[] entry;
        entry = formatNumber.split("_to_");
        if (entry != null && entry.length > 1) {
            return entry[0];  // source
        }
        return null;
    }

    @Override
    public boolean isAddressChanged(boolean converted, String dcNumber,
            String address, String convertedNumber) {
        if (!equalsHandlesNulls(address, dcNumber) && !converted) {
            return true;
        }
        return false;
    }

    @Override
    public boolean isAddressChanged(boolean converted, String dcAddress,
            String address) {
        if (!equalsHandlesNulls(address, dcAddress) && !converted) {
            return true;
        }
        return false;
    }

    @Override
    public Bundle getAddressExtras(String formatNumber) {
        String[] entry;
        entry = formatNumber.split("_to_");
        if (entry != null && entry.length > 1) {
            String destination = entry[1];
            Bundle intentExtras = new Bundle();
            intentExtras.putString(MtkTelecomManager.EXTRA_VIRTUAL_LINE_NUMBER, destination);
            return intentExtras;
        }
        return null;
    }

    private String getVirtualLineNumber(Connection connection) {
        if (connection == null) {
            return "";
        }
        Bundle extras = connection.getConnectionExtras();
        if (extras == null) {
            return "";
        }
        return extras.getString(MtkTelecomManager.EXTRA_VIRTUAL_LINE_NUMBER);
    }


    @Override
    public boolean areConnectionsInSameLine(Connection[] connections) {
        String prevLineNumber = null;
        boolean result = true;
        int index = 0;
        for (Connection connection: connections) {
            ++index;
            if (connection == null) {
                //Rlog.d(TAG, "connection[" + index + "] null, skip. ");
                continue;
            }
            // get virtual line number from connection for MT
            String virtualLineNumber = getVirtualLineNumber(connection);
            if (!TextUtils.isEmpty(virtualLineNumber)) {
                Rlog.d(TAG, "connection[" + index + "] virtualLineNumber: " + virtualLineNumber);
            }

            // start compare when prevLineNumber be init.
            if (prevLineNumber != null) {
                if (TextUtils.isEmpty(prevLineNumber) && TextUtils.isEmpty(virtualLineNumber)) {
                    continue;
                }

                // different line appears
                if (TextUtils.equals(prevLineNumber, virtualLineNumber) == false) {
                    result = false;
                    break;
                }
            }
            prevLineNumber = virtualLineNumber;
        }

        return result;
    }
}
