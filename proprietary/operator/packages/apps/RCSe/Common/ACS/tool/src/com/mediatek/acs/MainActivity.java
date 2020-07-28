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

package com.mediatek.acs;

import android.app.TabActivity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.os.Bundle;
import android.util.Log;
import android.widget.TabHost;

import com.mediatek.ims.rcsua.AcsEventCallback;
import com.mediatek.ims.rcsua.RcsUaService;

public class MainActivity extends TabActivity
{
    private Context mContext;
    AcsUtils mAcsUtils = null;
    private static final String TAG = "[ACSTOOL] MainActivity";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.i(TAG, "onCreate");
        mContext = this;
        mAcsUtils = AcsUtils.getInstance(mContext);

        // Set layout
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);

        // Set tabs
        final TabHost tabHost = getTabHost();
        tabHost.addTab(tabHost.newTabSpec("profile")
                .setIndicator("Profile", null)
                .setContent(new Intent(this, AcsConfigParamActivity.class)));
        tabHost.addTab(tabHost.newTabSpec("config_content")
                .setIndicator("Config content", null)
                .setContent(new Intent(this, AcsConfigFileActivity.class)));

        // tabHost.addTab(tabHost.newTabSpec("stack")
        //         .setIndicator("Stack", null)
        //         .setContent(new Intent(this, StackProvisioning.class)));
        // tabHost.addTab(tabHost.newTabSpec("ui")
        //         .setIndicator("Service", null)
        //         .setContent(new Intent(this, ServiceProvisioning.class)));
        // tabHost.addTab(tabHost.newTabSpec("logger")
        //         .setIndicator("Logger", null)
        //         .setContent(new Intent(this, LoggerProvisioning.class)));

        RcsUaService.startService(mContext, mAcsCallback);
    }

    /**
     * Describe <code>onDestroy</code> method here.
     *
     */
    public final void onDestroy() {
        Log.i(TAG, "onDestroy");

        if (mRcsUaService != null) {
            mRcsUaService.unregisterAcsEventCallback(mAcsEventCallback);
        }


        super.onDestroy();
    }

    // ==================================================
    private AcsEventCallback mAcsEventCallback = new AcsEventCallback() {
            public void onConfigurationStatusChanged(boolean valid, int version) {
                Log.i(TAG, "onAcsConfigChange,valid:" + valid + ", version:" + version);
                mAcsUtils.notifyConfigChange(AcsUtils.SCONFIG_CHGNAGE);
            }
            public void onAcsConnected() {
                Log.i(TAG, "onAcsConnected");
                mAcsUtils.notifyConfigChange(AcsUtils.SSERVICE_CONNECTED);
            }
            public void onAcsDisconnected() {
                Log.i(TAG, "onAcsDisconnected");
                mAcsUtils.notifyConfigChange(AcsUtils.SSERVICE_CONNECTED);
            }
        };

    RcsUaService mRcsUaService = null;
    RcsUaService.Callback mAcsCallback = new RcsUaService.Callback() {
            public void serviceConnected(RcsUaService service) {
                Log.d(TAG, "RcsUaService serviceConnected");
                mRcsUaService = service;
                mAcsUtils.setRcsUaService(mRcsUaService);
                mRcsUaService.registerAcsEventCallback(mAcsEventCallback);
                mAcsUtils.notifyConfigChange(AcsUtils.SSERVICE_CONNECTED);
            }

            public void serviceDisconnected(RcsUaService service) {
                Log.d(TAG, "RcsUaService serviceDisconnected");
                mRcsUaService = null;
                mAcsUtils.setRcsUaService(null);
                mAcsUtils.notifyConfigChange(AcsUtils.SSERVICE_CONNECTED);
            }
        };
}