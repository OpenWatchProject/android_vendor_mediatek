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
 * MediaTek Inc. (C) 2018. All rights reserved.
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

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.RIL;
import com.android.internal.telephony.RILRequest;

import android.hardware.radio.V1_0.RadioError;
import android.hardware.radio.V1_0.RadioResponseInfo;

import android.os.AsyncResult;
import android.os.Message;
import android.os.SystemClock;
import android.telephony.ServiceState;

import java.util.ArrayList;

/*
* This class is used to boost message priority
*/
public class MtkMessageBoost {

    private static MtkMessageBoost sMtkMessageBoost;

    /* Performance part */
    protected int mPriorityFlag = 0;

    /* Module adds priority */
    // public static final int BOOST_MESSAGE_PRIORITY_MTKSST = 0x1;

    protected static final int MESSAGE_BOOT_TIME_MSEC = 10000;

    MtkRIL mMtkRil;
    public static MtkMessageBoost init(MtkRIL mtkRIL) {
        synchronized (MtkMessageBoost.class) {
            if (sMtkMessageBoost == null) {
                sMtkMessageBoost =  new MtkMessageBoost(mtkRIL);
            }
            return sMtkMessageBoost;
        }
    }

    /**
     * get singleton instance of MtkMessageBoost
     * @return MtkMessageBoost
     */
    public static MtkMessageBoost getInstance() {
        synchronized (MtkMessageBoost.class) {
            return sMtkMessageBoost;
        }
    }

    public MtkMessageBoost (MtkRIL mtkRIL) {
        // Backup MtkRIL once constructor was invoked.
        // RIL API required when processing response.
        mMtkRil = mtkRIL;
    }

    public void setPriorityFlag(int flag, Phone phone) {
        mPriorityFlag |= flag;
    }

    public void clearPriorityFlag(int flag) {
        // for performance use
        mPriorityFlag &= ~flag;
    }

    public int getPriorityFlag(int flag) {
        // for performance use
        return mPriorityFlag & flag;
    }

    /**
     * Helper function to send response msg
     * @param msg Response message to be sent
     * @param ret Return object to be included in the response message
     */
    public static void sendMessageResponseWithPriority(Message msg, Object ret) {
        if (msg != null) {
            long boost_time
                    = SystemClock.uptimeMillis() - MESSAGE_BOOT_TIME_MSEC;
            AsyncResult.forMessage(msg, ret, null);
            msg.getTarget().sendMessageAtTime(msg, boost_time);
        }
    }

    public void responseStringsWithPriority(RadioResponseInfo responseInfo, String ...str) {
        ArrayList<String> strings = new ArrayList<>();
        for (int i = 0; i < str.length; i++) {
            strings.add(str[i]);
        }
        responseStringArrayListWithPriority(mMtkRil, responseInfo, strings);
    }

    public void responseStringArrayListWithPriority(RIL ril, RadioResponseInfo responseInfo,
            ArrayList<String> strings) {
        RILRequest rr = ril.processResponse(responseInfo);

        if (rr != null) {
            String[] ret = new String[strings.size()];
            for (int i = 0; i < strings.size(); i++) {
                ret[i] = strings.get(i);
            }
            if (responseInfo.error == RadioError.NONE) {
                sendMessageResponseWithPriority(rr.mResult, ret);
            }
            ril.processResponseDone(rr, responseInfo, ret);
        }
    }

    public void responseIntsWithPriority(RadioResponseInfo responseInfo, int ...var) {
        final ArrayList<Integer> ints = new ArrayList<>();
        for (int i = 0; i < var.length; i++) {
            ints.add(var[i]);
        }
        responseIntArrayListWithPriority(responseInfo, ints);
    }

    public void responseIntArrayListWithPriority(RadioResponseInfo responseInfo,
            ArrayList<Integer> var) {
        RILRequest rr = mMtkRil.processResponse(responseInfo);

        if (rr != null) {
            int[] ret = new int[var.size()];
            for (int i = 0; i < var.size(); i++) {
                ret[i] = var.get(i);
            }
            if (responseInfo.error == RadioError.NONE) {
                sendMessageResponseWithPriority(rr.mResult, ret);
            }
            mMtkRil.processResponseDone(rr, responseInfo, ret);
        }
    }
}
