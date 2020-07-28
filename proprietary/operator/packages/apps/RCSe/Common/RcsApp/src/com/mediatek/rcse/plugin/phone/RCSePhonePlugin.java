/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
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
package com.mediatek.rcse.plugin.phone;

import android.app.Activity;
import android.content.Context;
import android.util.Log;

import com.android.internal.telephony.CallManager;
import com.mediatek.rcse.service.MediatekFactory;

/**
 * The Class RCSePhonePlugin.
 */
public class RCSePhonePlugin {
    /**
     * The Constant TAG.
     */
    private static final String TAG = "RCSePhonePlugin";
    /**
     * The m plugin context.
     */
    private Context mPluginContext;
    /**
     * The m in call screen activity.
     */
    private Activity mInCallScreenActivity;
    /**
     * The m cm.
     */
    private CallManager mCM;
    /**
     * Instantiates a new RC se phone plugin.
     *
     * @param context the context
     */
    public RCSePhonePlugin(Context context) {
        mPluginContext = context;
        //RCSeInCallUIExtension.initialize(context, this);
    }
    /* package *//**
                     * Sets the in call screen activity.
                     *
                     * @param activity the new in call screen activity
                     */
    void setInCallScreenActivity(Activity activity) {
        log("setInCallScreenActivity RCSe" + activity);
        mInCallScreenActivity = activity;
    }
    /* package *//**
                     * Gets the in call screen activity.
                     *
                     * @return the in call screen activity
                     */
    Activity getInCallScreenActivity() {
        return mInCallScreenActivity;
    }
    /* package *//**
                     * Sets the call manager.
                     *
                     * @param cm the new call manager
                     */
    void setCallManager(CallManager cm) {
        mCM = cm;
    }
    /* package *//**
                     * Gets the call manager.
                     *
                     * @return the call manager
                     */
    CallManager getCallManager() {
        return mCM;
    }
    /**
     * Log.
     *
     * @param msg the msg
     */
    public static void log(String msg) {
        Log.d(TAG, msg);
    }
    private static RCSePhonePlugin sInstance = null;

    public static void initialize(Context context) {
        MediatekFactory.setApplicationContext(context);
        if(sInstance == null)
        {
        	log("initialize RCSePhonePlugin null");
        	sInstance = new RCSePhonePlugin(context);
    }
        else
        	log("initialize RCSePhonePlugin not null");
    }

    public static RCSePhonePlugin getInstance() {
    	log("getInstance is " + sInstance);
        return sInstance;
    }
}
