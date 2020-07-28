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
* have been modified by MediaTek Inc. All revisions are subject to any receiver\'s
* applicable license agreements with MediaTek Inc.
*/

package com.mediatek.services.telephony;

import android.os.SystemProperties;
import android.telecom.ConnectionRequest;
import android.telecom.PhoneAccountHandle;
import android.telephony.TelephonyManager;

import com.android.internal.telephony.TelephonyDevController;
import com.android.phone.PhoneUtils;
import com.android.services.telephony.Log;
import com.mediatek.internal.telephony.MtkHardwareConfig;
/// M: CC: [ALPS04214959] Smart Lock: not allow empty slot for ECC @{
import com.mediatek.internal.telephony.MtkIccCardConstants;
import com.mediatek.telephony.MtkTelephonyManagerEx;
/// @}

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

/**
 * The emergency call retry handler.
 * Selected the proper Phone for setting up the ecc call.
 */
public class EmergencyRetryHandler {
    private static final boolean DBG = true;

    private static final boolean MTK_CT_VOLTE_SUPPORT
            = "1".equals(SystemProperties.get("persist.vendor.mtk_ct_volte_support", "0"));

    private static final int PROJECT_SIM_NUM = TelephonyManager.getDefault().getPhoneCount();

    private final int MAX_NUM_RETRIES = PROJECT_SIM_NUM - 1
            + (MTK_CT_VOLTE_SUPPORT && PROJECT_SIM_NUM == 1 && !hasC2kOverImsModem() ? 1 : 0);

    private ConnectionRequest mRequest = null;
    private int mNumRetriesSoFar = 0;
    private List<PhoneAccountHandle> mAttemptRecords;
    private Iterator<PhoneAccountHandle> mAttemptRecordIterator;
    private String mCallId = null;

    /**
     * Init the EmergencyRetryHandler.
     * @param request ConnectionRequest
     * @param initPhoneId PhoneId of the initial ECC
     */
    public EmergencyRetryHandler(ConnectionRequest request, int initPhoneId) {
        mRequest = request;
        mNumRetriesSoFar = 0;
        mAttemptRecords = new ArrayList<PhoneAccountHandle>();

        PhoneAccountHandle phoneAccountHandle;
        int num = 0;

        while (num <  MAX_NUM_RETRIES) {
            // 1. Add other phone rather than initPhone sequentially
            for (int i = 0; i < TelephonyManager.getDefault().getPhoneCount(); i++) {
                if (initPhoneId != i) {
                    /// M: CC: [ALPS04214959] Smart Lock: not allow empty slot for ECC @{
                    // Do SIM ME Lock permission check for ECC retry list.
                    if (checkSimMeLockPermForEccRetry(i) == false) {
                        Log.d(this, "Skip phone (id=" + i +
                                ") to ECC retry list (SIM ME lock check fail)");
                        continue;
                    }
                    /// @}
                    // If No SIM is inserted, the corresponding IccId will be null,
                    // so take phoneId as PhoneAccountHandle::mId which is IccId originally
                    phoneAccountHandle = PhoneUtils.makePstnPhoneAccountHandle(
                            Integer.toString(i));
                    mAttemptRecords.add(phoneAccountHandle);
                    num++;
                    Log.d(this, "Add #" + num + " to ECC retry list, " + phoneAccountHandle);
                }
            }

            // 2. Add initPhone at last
            phoneAccountHandle = PhoneUtils.makePstnPhoneAccountHandle(
                    Integer.toString(initPhoneId));
            mAttemptRecords.add(phoneAccountHandle);
            num++;
            Log.d(this, "Add #" + num + " to ECC retry list, " + phoneAccountHandle);
        }

        mAttemptRecordIterator = mAttemptRecords.iterator();
    }

    /// M: CC: [ALPS04214959] Smart Lock: not allow empty slot for ECC @{
    private boolean checkSimMeLockPermForEccRetry(int phoneId) {
        MtkTelephonyManagerEx telEx = MtkTelephonyManagerEx.getDefault();
        if (telEx != null
                && telEx.getSimLockPolicy() ==
                        MtkIccCardConstants.SML_SLOT_LOCK_POLICY_LK_SLOTA_RESTRICT_INVALID_CS
                && telEx.getShouldServiceCapability(phoneId) ==
                        MtkIccCardConstants.SML_SLOT_LOCK_POLICY_SERVICE_CAPABILITY_NO_SERVICE) {
            return false;
        }
        return true;
    }
    /// @}

    public void setCallId(String id) {
        Log.d(this, "setCallId, id=" + id);
        mCallId = id;
    }

    public String getCallId() {
        Log.d(this, "getCallId, id=" + mCallId);
        return mCallId;
    }

    public boolean isTimeout() {
        boolean isOut = (mNumRetriesSoFar >= MAX_NUM_RETRIES)
                || (!mAttemptRecordIterator.hasNext()); //[ALPS04214959]: may skip by smart lock
        Log.d(this, "isTimeout, timeout=" + isOut
                + ", mNumRetriesSoFar=" + mNumRetriesSoFar
                + ", mAttemptRecordIterator.hasNext==" + mAttemptRecordIterator.hasNext());
        return isOut;
    }

    public ConnectionRequest getRequest() {
        Log.d(this, "getRequest, request=" + mRequest);
        return mRequest;
    }

    public PhoneAccountHandle getNextAccountHandle() {
        if (mAttemptRecordIterator.hasNext()) {
            mNumRetriesSoFar++;
            Log.d(this, "getNextAccountHandle, next account handle exists");
            return mAttemptRecordIterator.next();
        }
        Log.d(this, "getNextAccountHandle, next account handle is null");
        return null;
    }

    private boolean hasC2kOverImsModem() {
        TelephonyDevController telDevController = TelephonyDevController.getInstance();
        if (telDevController != null && telDevController.getModem(0) != null
                && ((MtkHardwareConfig) telDevController.getModem(0)).hasC2kOverImsModem()) {
            return true;
        }
        return false;
    }
}
