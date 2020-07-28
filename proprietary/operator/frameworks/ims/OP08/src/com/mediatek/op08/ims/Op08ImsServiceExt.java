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
 * MediaTek Inc. (C) 2014. All rights reserved.
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

package com.mediatek.op08.ims;

import android.content.Context;
import android.os.Message;
import android.util.Log;


import com.mediatek.ims.ext.ImsServiceExt;

/**
 * Plugin implementation for ImsService.
 */
public class Op08ImsServiceExt extends ImsServiceExt {

    private static final String TAG = "Op08ImsServiceExt";

    public static final int EVENT_IMS_DISABLED_URC   = 5;
    public static final int EVENT_IMS_ENABLING_URC   = 10;
    public static final int EVENT_IMS_NOTIFICATION_INIT = 39;

    private Context mPluginContext;

    private boolean mIsImsnRegistered = false;

    /**
     * Constructor of plugin.
     * @param context context
     */
    public Op08ImsServiceExt(Context context) {
        super(context);
        mPluginContext = context;
    }

    /**
     * Notify about IMS service events.
     * @param phoneId phoneId
     * @param context context
     * @param msg message received at ImsService
     */
    @Override
    public void notifyImsServiceEvent(int phoneId, Context context, Message msg) {
        Log.d(TAG, "ImsService event: " + msg.what);
        switch (msg.what) {
            case EVENT_IMS_ENABLING_URC:
                mIsImsnRegistered = true;
                Log.d(TAG, "IMSN registered");
                break;
            case EVENT_IMS_DISABLED_URC:
                mIsImsnRegistered = false;
                break;
            case EVENT_IMS_NOTIFICATION_INIT: {
                ImsNotificationController imsNotiCtrl =
                        ImsNotificationController.getInstance(context, mPluginContext, phoneId);
                // Phone id is not used in ImsNotificationController.
                imsNotiCtrl.updateContext(context, mPluginContext, phoneId);
                imsNotiCtrl.start();
                break;
            }
            default:
                break;
        }
    }

    /**
     * Check wheter operator support transcode SIP error to WFC error cause.
     * @return true/false
     */
    @Override
    public boolean isWfcRegErrorCauseSupported() {
        Log.d(TAG, "isWfcRegErrorCauseSupported return true");
        return true;
    }
}
