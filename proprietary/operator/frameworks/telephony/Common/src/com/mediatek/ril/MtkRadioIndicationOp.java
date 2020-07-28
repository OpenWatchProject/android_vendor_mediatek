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

package com.mediatek.opcommon.telephony;

import android.os.AsyncResult;
import android.os.RemoteException;

import java.util.ArrayList;
import android.telephony.Rlog;
import android.text.TextUtils;
import android.util.Log;
import android.os.Build;
import android.os.AsyncResult;
import android.os.Registrant;
import android.os.RegistrantList;

import com.mediatek.opcommon.telephony.MtkRILConstantsOp;
import static com.mediatek.internal.telephony.MtkRILConstants.*;

import vendor.mediatek.hardware.radio_op.V2_0.IRadioIndicationOp;

public class MtkRadioIndicationOp extends IRadioIndicationOp.Stub  {
    static final String TAG = "MtkRadioIndicationOp";
    private MtkRilOp mMtkRilOp;
    private static final boolean ENG = "eng".equals(Build.TYPE);

    MtkRadioIndicationOp(MtkRilOp ril) {
        mMtkRilOp = ril;
        mMtkRilOp.log("MtkRadioIndicationOp constructor");
    }

    // MTK-START: SIM TMO RSU
    public void onSimMeLockEvent(int indicationType, int eventId) {
        mMtkRilOp.processIndication(indicationType);
        mMtkRilOp.log("onSimMeLockEvent eventId " + eventId);

        if (ENG) mMtkRilOp.unsljLog(MtkRILConstantsOp.
                RIL_UNSOL_MELOCK_NOTIFICATION);

        int response[] = new int[1];
        response[0] = eventId;

        if (mMtkRilOp.mMelockRegistrants != null) {
            mMtkRilOp.mMelockRegistrants.notifyRegistrants(
                    new AsyncResult(null, response, null));
        }
    }
    // MTK-END

    public void log(String text) {
        Rlog.d(TAG, text);
    }

    public void responseModulationInfoInd(int indicationType,
            ArrayList<Integer> data) {
        mMtkRilOp.processIndication(indicationType);

        Object ret = null;
        int[] response= new int[data.size()];

        for (int i = 0; i < data.size(); i++) {
             response[i] = data.get(i);
        }
        ret = response;

        mMtkRilOp.unsljLogRet(RIL_UNSOL_MODULATION_INFO, ret);

        if (mMtkRilOp.mModulationRegistrants.size()  !=  0) {
            mMtkRilOp.mModulationRegistrants.notifyRegistrants(new AsyncResult(null, ret, null));
        }
    }

    public void enterSCBMInd(int indicationType) {
        mMtkRilOp.processIndication(indicationType);

        if (ENG) mMtkRilOp.unsljLog(MtkRILConstantsOp.RIL_UNSOL_ENTER_SCBM);

        if (mMtkRilOp.mEnterSCBMRegistrant != null) {
            mMtkRilOp.mEnterSCBMRegistrant.notifyRegistrant();
        }
    }

    public void exitSCBMInd(int indicationType) {
        mMtkRilOp.processIndication(indicationType);

        if (ENG) mMtkRilOp.unsljLog(MtkRILConstantsOp.RIL_UNSOL_EXIT_SCBM);

        mMtkRilOp.mExitSCBMRegistrants.notifyRegistrants();
    }

    public void onRsuEvent(int indicationType, int eventId, String eventString) {
        mMtkRilOp.processIndication(indicationType);

        if (ENG) mMtkRilOp.unsljLog(MtkRILConstantsOp.RIL_UNSOL_RSU_EVENT);

        mMtkRilOp.mRsuEventRegistrants.notifyRegistrants(
                new AsyncResult(null,
                        new String[] {Integer.toString(eventId), eventString},
                        null));
    }

}
