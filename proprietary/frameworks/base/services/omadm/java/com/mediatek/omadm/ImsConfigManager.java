/* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein
* is confidential and proprietary to MediaTek Inc. and/or its licensors.
* Without the prior written permission of MediaTek inc. and/or its licensors,
* any reproduction, modification, use or disclosure of MediaTek Software,
* and information contained herein, in whole or in part, shall be strictly prohibited.
*
* MediaTek Inc. (C) 2018. All rights reserved.
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

package com.mediatek.omadm;

import android.content.Context;
import android.os.SystemProperties;
import android.telephony.SubscriptionManager;
import android.telephony.CarrierConfigManager;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Slog;

import com.android.ims.ImsConfig;
import com.android.ims.ImsException;
import com.android.ims.ImsManager;

//import com.mediatek.ims.internal.MtkImsManager;
//import com.mediatek.ims.internal.MtkImsManagerEx;
//import com.mediatek.internal.telephony.MtkPhoneConstants;

import com.mediatek.dm.DmManager;
//import com.mediatek.internal.telephony.MtkPhoneConstants;

/**
 * Helper for interaction with ImsConfig
 * @hide
 */
public class ImsConfigManager {

    public static boolean DEBUG = true;

    private static final String TAG = ImsConfigManager.class.getSimpleName();
    private final Context mContext;
    private DmManager mDmManager = null;

    private int imsRslt_PalRslt(int imsStatus) {
        switch(imsStatus) {
            case ImsConfig.OperationStatusConstants.FAILED:
                return PalConstDefs.RET_ERR;
            case ImsConfig.OperationStatusConstants.SUCCESS:
                return PalConstDefs.RET_SUCC;
            case ImsConfig.OperationStatusConstants.UNSUPPORTED_CAUSE_DISABLED:
                return PalConstDefs.RET_ERR_STATE;
            case ImsConfig.OperationStatusConstants.UNSUPPORTED_CAUSE_NONE:
            case ImsConfig.OperationStatusConstants.UNSUPPORTED_CAUSE_RAT:
                return PalConstDefs.RET_ERR_NORES;
            default:
                return PalConstDefs.RET_ERR_UNDEF;
        }
    }

    private ImsConfig getImsConfig() {
        ImsConfig imsConfig = null;
        try {
            ImsManager imsManager = ImsManager.getInstance(mContext, getMainCapabilityPhoneId());
            imsConfig = imsManager.getConfigInterface();
        } catch (ImsException e) {
            e.printStackTrace();
            PalConstDefs.throwEcxeption(PalConstDefs.RET_ERR);
        }

        if (imsConfig == null) {
            PalConstDefs.throwEcxeption(PalConstDefs.
                    RET_ERR_NORES);
        }
        return imsConfig;
    }

    private int getMainCapabilityPhoneId() {
        int phoneId = SystemProperties.getInt("persist.radio.simswitch", 1) - 1;
            if (phoneId < 0 || phoneId >= TelephonyManager.getDefault().getPhoneCount()) {
                phoneId = SubscriptionManager.INVALID_PHONE_INDEX;
            }
        Slog.d(TAG, "getMainCapabilityPhoneId = " + phoneId);
        return phoneId;
    }

    public ImsConfigManager(Context context) {
        mContext = context;
        mDmManager = DmManager.getDefaultDmManager(mContext);
    }

    public void setProvisionedStringValue(int item, String value) {
        if (DEBUG) Slog.d(TAG, "setProvisionedStringValue, item = " + item + ", value = " + value);
        int status;

        try {
            status = imsRslt_PalRslt(getImsConfig().setProvisionedStringValue(item, value));
        } catch (ImsException e) {
            e.printStackTrace();
            status = PalConstDefs.RET_ERR_STATE;
        }
        PalConstDefs.throwEcxeption(status);
    }

    public String getProvisionedStringValue(int item) {
        String result = null;
        int status = PalConstDefs.RET_SUCC;

        try {
            result = getImsConfig().getProvisionedStringValue(item);
            if (DEBUG) Slog.d(TAG, "getProvisionedStringValue, item = "+item+ ", result = "+result);
            if (result == null || result.length() == 0) {
                status = PalConstDefs.RET_ERR_STATE;
            }
        } catch (ImsException e) {
            e.printStackTrace();
            status = PalConstDefs.RET_ERR_STATE;
        }

        PalConstDefs.throwEcxeption(status);
        return result;
    }

    public void setProvisionedIntValue(int item, int value) {
        if (DEBUG) Slog.d(TAG, "setProvisionedIntValue, item = " + item + ", value = " + value);
        int status;
        try {
            status = imsRslt_PalRslt(getImsConfig().setProvisionedValue(item, value));
        } catch (ImsException e) {
            e.printStackTrace();
            status = PalConstDefs.RET_ERR_STATE;
        }
        PalConstDefs.throwEcxeption(status);
    }

    public int getProvisionedIntValue(int item) {
        if (DEBUG) Slog.d(TAG, "getProvisionedIntValue, item = " + item);
        int result = -1;
        int status = PalConstDefs.RET_SUCC;

        try {
            result = getImsConfig().getProvisionedValue(item);
            if (DEBUG) Slog.d(TAG, "getProvisionedStringValue, item = "+item+", result = "+result);
        } catch (ImsException e) {
            e.printStackTrace();
            status = PalConstDefs.RET_ERR_STATE;
        }

        PalConstDefs.throwEcxeption(status);
        return result;
    }

    public int isImcPvsInfoSupport(){
        Slog.d(TAG, "check isImcPvsInfoSupport");
        int status = 0;
        if(mDmManager == null){
            mDmManager = DmManager.getDefaultDmManager(mContext);
        }

        if(mDmManager != null) {
            status = mDmManager.getDmSupported();
        }
        else {
            Slog.d(TAG, "setImcProvisioned, failed since cant get  mDmManager");
            PalConstDefs.throwEcxeption(status);
        }
        return status;
    }

     public void setImcProvisioned(int item, int value) {
        Slog.d(TAG, "setImcProvisioned, item = " + item + ", value = " + value);
        int status= PalConstDefs.RET_SUCC;
        if(mDmManager == null){
            mDmManager = DmManager.getDefaultDmManager(mContext);
        }

        if(mDmManager != null) {
            mDmManager.setImcProvision(getMainCapabilityPhoneId(), item, value);
        }
        else {
            Slog.d(TAG, "setImcProvisioned, failed since cant get  mDmManager");
            PalConstDefs.throwEcxeption(status);
        }
    }

    public int getImcProvisioned(int item) {
        Slog.d(TAG, "getImcProvisioned, item = " + item);
        int result = -1;
        int status = PalConstDefs.RET_SUCC;

        if(mDmManager == null){
            mDmManager = DmManager.getDefaultDmManager(mContext);
        }

        if(mDmManager != null){
            result = (mDmManager.getImcProvision(getMainCapabilityPhoneId(), item)==true)? 1 : 0;
            Slog.d(TAG, "getImcProvisioned, item = " + item + ", result = " + result);
        }
        else {
            Slog.d(TAG, "getImcProvisioned, failed since cant get  mDmManager");
            PalConstDefs.throwEcxeption(status);
        }
        return result;
    }

}
