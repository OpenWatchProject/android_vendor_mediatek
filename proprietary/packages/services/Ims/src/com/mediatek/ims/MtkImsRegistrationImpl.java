/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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


package com.mediatek.ims;

import android.annotation.IntDef;
import android.annotation.SystemApi;
import android.net.Uri;
import android.os.RemoteException;
import android.telephony.Rlog;

import android.telephony.ims.ImsReasonInfo;

import com.android.internal.annotations.VisibleForTesting;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;

import android.telephony.ims.stub.ImsRegistrationImplBase;

import com.mediatek.ims.ImsService;

/**
 * Controls IMS registration for this ImsService and notifies the framework when the IMS
 * registration for this ImsService has changed status.
 * @hide
 */
@SystemApi
public class MtkImsRegistrationImpl extends ImsRegistrationImplBase {

    private static final String LOG_TAG = "MtkImsRegImpl";

    private int mSlotId = -1;

    // Defines the underlying IMS registration state.
    @IntDef(flag = true,
            value = {
                    REGISTRATION_STATE_UNKNOWN,
                    REGISTRATION_STATE_REGISTERING,
                    REGISTRATION_STATE_REGISTERED,
                    REGISTRATION_STATE_DEREGISTERED
            })
    @Retention(RetentionPolicy.SOURCE)
    public @interface ImsRegistrationState {}

    public static final int REGISTRATION_STATE_UNKNOWN = 0;
    public static final int REGISTRATION_STATE_REGISTERING = 1;
    public static final int REGISTRATION_STATE_REGISTERED = 2;
    public static final int REGISTRATION_STATE_DEREGISTERED = 3;

    private ImsService mImsServiceImpl = null;

    // Delay between ImsService queries.
    private static final int DELAY_IMS_SERVICE_IMPL_QUERY_MS = 5000;
    private static final int MAXMUIM_IMS_SERVICE_IMPL_RETRY = 3;

    public MtkImsRegistrationImpl(int slotId) {
        mSlotId = slotId;

        int retry = 0;
        while ((mImsServiceImpl == null) && (retry < MAXMUIM_IMS_SERVICE_IMPL_RETRY)) {
            mImsServiceImpl = ImsService.getInstance(null);
            try {
                if (mImsServiceImpl == null) {
                    log("ImsService is not initialized yet. Query later - " + retry);
                    Thread.sleep(DELAY_IMS_SERVICE_IMPL_QUERY_MS);
                    retry++;
                }
            } catch (InterruptedException er) {
                loge("Fail to get ImsService " + er);
            }
        }
        if (mImsServiceImpl != null) {
            mImsServiceImpl.setImsRegistration(mSlotId, this);
        }
        log("[" + mSlotId + "] MtkImsRegistrationImpl created");
    }

    public void close() {
        if (mImsServiceImpl != null) {
            mImsServiceImpl.setImsRegistration(mSlotId, null);
        }
        logi("[" + mSlotId + "] MtkImsRegistrationImpl closed");
    }

    private static void log(String msg) {
        Rlog.d(LOG_TAG, msg);
    }

    private void logi(String msg) {
        Rlog.i(LOG_TAG, msg);
    }

    private static void loge(String msg) {
        Rlog.e(LOG_TAG, msg);
    }
}
