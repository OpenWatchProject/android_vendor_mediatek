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

package com.mediatek.ims;

import android.app.Application;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.os.UserHandle;
import android.telephony.Rlog;
import android.content.Intent;

import com.mediatek.ims.internal.ImsVTProviderUtil;
import com.mediatek.ims.MtkImsConstants;

/**
 * Top-level Application class for the Phone app.
 */
public class ImsApp extends Application {
    private static final String TAG = "ImsApp";
    private static final String IMS_SERVICE = "ims";

    public ImsApp() {
    }

    @Override
    public void onCreate() {
        if (SystemProperties.get("persist.vendor.ims_support").equals("0")) {
            Rlog.w(TAG, "IMS not support, do not init ImsService");
            return;
        }
        if (UserHandle.myUserId() == 0) {
            Rlog.i(TAG, "ImsApp onCreate begin");
            // We are running as the primary user, so should bring up the
            // global phone state.


            ImsService imsService = ImsService.getInstance(this);
            boolean isDynamicBinding =  getApplicationContext().getResources().getBoolean(
                    com.android.internal.R.bool.config_dynamic_bind_ims);
            Rlog.w(TAG, "isDynamicBinding? " + isDynamicBinding);
            if (!isDynamicBinding) {
                ServiceManager.addService(IMS_SERVICE, imsService.getBinder(), true);
            }
            ImsVTProviderUtil.getInstance().setContextAndInitRefVTP(this);

            MtkImsService mtkImsService = new MtkImsService(this, imsService);
            ServiceManager.addService(MtkImsConstants.MTK_IMS_SERVICE, mtkImsService.asBinder(),
                    true);

            Rlog.i(TAG, "ImsApp onCreate end");
            // Notify the MTK IMS service is created
            Intent intent = new Intent(MtkImsConstants.ACTION_MTK_IMS_SERVICE_UP);;
            getApplicationContext().sendBroadcast(intent);
        }
    }
}
