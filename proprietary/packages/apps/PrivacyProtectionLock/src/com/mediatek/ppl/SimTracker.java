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

package com.mediatek.ppl;

import android.content.Context;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.os.SystemProperties;

import com.android.internal.telephony.IccCardConstants;
import com.mediatek.ppl.Utils;

import java.util.Arrays;

/**
 * Utility class to track SIM states.
 */
public class SimTracker {
    private static final String TAG = "PPL/SimTracker";

    public final int slotNumber;
    private Context mContext;
    boolean inserted[];
    String serialNumbers[];
    int states[];

    private String[] PROPERTY_ICCID_SIM = {
        "vendor.ril.iccid.sim1",
        "vendor.ril.iccid.sim2",
        "vendor.ril.iccid.sim3",
        "vendor.ril.iccid.sim4",
    };

    public SimTracker(int number, Context context) {
        slotNumber = number;
        mContext = context;
        inserted = new boolean[slotNumber];
        serialNumbers = new String[slotNumber];
        states = new int[slotNumber];
    }

    /**
     * Take a snapshot of the current SIM information in system and store it in this object.
     */
    public synchronized void takeSnapshot() {
        if (!PlatformManager.isTelephonyReady(mContext)) {
            return;
        }

        TelephonyManager tm = (TelephonyManager) mContext
                .getSystemService(Context.TELEPHONY_SERVICE);
        for (int i = 0; i < slotNumber; ++i) {
            if (tm.hasIccCard(i)) {
                // here inserted[i] is determined by hasIccCard(i) not by sim ABSENT state
                inserted[i] = true;
                serialNumbers[i] = SystemProperties.get(PROPERTY_ICCID_SIM[i]);
                if ("".equals(serialNumbers[i])) {
                    serialNumbers[i] = null;
                }
                states[i] = tm.getSimState(i);
                Log.d(TAG, "Slot[" + i + "] SSN=" + Utils.stringForSecureLog(serialNumbers[i])
                    + ", getSimState = " + states[i]);
            } else {
                inserted[i] = false;
                serialNumbers[i] = null;
                states[i] = tm.getSimState(i);
            }
        }
    }

    /**
     * Get a list of the IDs of current inserted SIM cards.
     *
     * @return
     */
    public synchronized int[] getInsertedSim() {
        int[] result = new int[inserted.length];
        int count = 0;
        for (int i = 0; i < inserted.length; ++i) {
            if (inserted[i]) {
                result[count++] = i;
            }
        }
        return Arrays.copyOf(result, count);
    }

    public synchronized boolean isAllSimReady() {

        TelephonyManager tm = (TelephonyManager) mContext
                .getSystemService(Context.TELEPHONY_SERVICE);
        for (int i = 0; i < inserted.length; ++i) {
            int state = tm.getSimState(i);
            Log.d(TAG, "isAllSimReady: Slot[" + i + "] state = " + state);
            if (IccCardConstants.State.UNKNOWN.ordinal() == state ||
                IccCardConstants.State.NOT_READY.ordinal() == state ||
                IccCardConstants.State.PIN_REQUIRED.ordinal() == state ||
                IccCardConstants.State.PUK_REQUIRED.ordinal() == state) {
                Log.d(TAG, "Slot[" + i + "] state not ready!");
                return false;
            }
        }
        return true;
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder("SimTracker: ");
        for (int i = 0; i < slotNumber; ++i) {
            sb.append("{")
              .append(inserted[i])
              .append(", ")
              .append(Utils.stringForSecureLog(serialNumbers[i]))
              .append(", ")
              .append(states[i])
              .append("}, ");
        }
        return sb.toString();
    }
}
