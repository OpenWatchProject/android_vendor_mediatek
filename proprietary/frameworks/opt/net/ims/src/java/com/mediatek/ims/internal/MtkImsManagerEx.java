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

package com.mediatek.ims.internal;

import android.os.IBinder;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.telephony.Rlog;
import android.telephony.ims.ImsReasonInfo;

import com.android.ims.ImsException;

import com.mediatek.internal.telephony.MtkPhoneConstants;

import android.annotation.ProductApi;

/**
 * Provides access for MTK IMS services or none AOSP APIs.
 */
public class MtkImsManagerEx {

    /**
     * For accessing the Mediatek IMS related service.
     * Internal use only.
     * @hide
     */
    public static final String MTK_IMS_SERVICE = "mtkIms";

    private static final String TAG = "MtkImsManagerEx";
    private static final boolean DBG = true;
    private IMtkImsService mMtkImsService = null;
    private MtkImsServiceDeathRecipient mMtkDeathRecipient = new MtkImsServiceDeathRecipient();

    private MtkImsManagerEx() {
        bindMtkImsService(true);
    }

    private static MtkImsManagerEx sInstance = new MtkImsManagerEx();

    /**
     * Return the static instance of MtkImsManagerEx
     * @return return the static instance of MtkImsManagerEx
     * @hide
     */
    @ProductApi
    public static MtkImsManagerEx getInstance() {
        return sInstance;
    }

    /**
     * Binds the IMS service only if the service is not created.
     */
    private void checkAndThrowExceptionIfServiceUnavailable()
            throws ImsException {
        if (mMtkImsService == null) {
            bindMtkImsService(true);

            if (mMtkImsService == null) {
                throw new ImsException("MtkImsService is unavailable",
                        ImsReasonInfo.CODE_LOCAL_IMS_SERVICE_DOWN);
            }
        }
    }

    private static String getMtkImsServiceName() {
        return MTK_IMS_SERVICE;
    }

    /**
     * Binds the IMS service to make/receive the call.
     */
    private void bindMtkImsService(boolean checkService) {
        if (checkService) {
            IBinder binder = ServiceManager.checkService(getMtkImsServiceName());

            if (binder == null) {
                loge("bindMtkImsService binder is null");
                return;
            }
        }

        IBinder b = ServiceManager.getService(getMtkImsServiceName());

        if (b != null) {
            try {
                b.linkToDeath(mMtkDeathRecipient, 0);
            } catch (RemoteException e) {
                // no-op
            }
        }

        mMtkImsService = IMtkImsService.Stub.asInterface(b);
        if (DBG) log("mMtkImsService = " + mMtkImsService);
    }

    private static void log(String s) {
        Rlog.d(TAG, s);
    }

    private static void logw(String s) {
        Rlog.w(TAG, s);
    }

    private static void loge(String s) {
        Rlog.e(TAG, s);
    }

    private static void loge(String s, Throwable t) {
        Rlog.e(TAG, s, t);
    }

    /**
     * Death recipient class for monitoring IMS service.
     */
    private class MtkImsServiceDeathRecipient implements IBinder.DeathRecipient {
        @Override
        public void binderDied() {
            mMtkImsService = null;
        }
    }

    /**
     * To get IMS state.
     *
     * @param phoneId Indicates which phone to query
     * @return ims state - possible value are :
     *      IMS_STATE_DISABLED = 0;
     *      IMS_STATE_ENABLE = 1;
     *      IMS_STATE_ENABLING = 2;
     *      IMS_STATE_DISABLING = 3;
     * @throws ImsException if getting the ims status result in an error.
     * @hide
     */
    @ProductApi
    public int getImsState(int phoneId) throws ImsException {
        int imsState = MtkPhoneConstants.IMS_STATE_DISABLED;

        checkAndThrowExceptionIfServiceUnavailable();

        try {
            imsState = mMtkImsService.getImsState(phoneId);
        } catch (RemoteException e) {
            throw new ImsException("getImsState()", e, ImsReasonInfo.CODE_LOCAL_IMS_SERVICE_DOWN);
        }
        log("getImsState=" + imsState + " for phoneId=" + phoneId);
        return imsState;
    }

    /*
     * To get IMS active call count.
     * @throws ImsException if getting the ims status result in an error.
     * @hide
     */
    @ProductApi
    public int getCurrentCallCount(int phoneId) throws ImsException {
        int callCount = 0;
        checkAndThrowExceptionIfServiceUnavailable();

        try {
            callCount = mMtkImsService.getCurrentCallCount(phoneId);
        } catch (RemoteException e) {
            throw new ImsException(
                    "getCurrentCallCount()", e, ImsReasonInfo.CODE_LOCAL_IMS_SERVICE_DOWN);
        }
        log("getCurrentCallCount, phoneId: " + phoneId + " callCount: " + callCount);
        return callCount;
    }
}
