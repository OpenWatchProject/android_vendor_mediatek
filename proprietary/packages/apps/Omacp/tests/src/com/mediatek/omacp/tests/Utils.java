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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.omacp.tests;

import java.util.HashMap;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

import com.mediatek.omacp.provider.OmacpProviderDatabase;

import android.content.BroadcastReceiver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.IntentFilter.MalformedMimeTypeException;
import android.content.SharedPreferences.Editor;
import android.net.Uri;
import android.util.Log;
import android.app.Instrumentation;

public class Utils {

    private static final String TAG = "OmacpFunctionalTest";

    private static CountDownLatch mLatch = new CountDownLatch(1);

    private final static int MESSAGE_RECEIVE_TIMEOUT = 150000; // 150 seconds, 10 more seconds than install timeout

    private static Context mContext;

    public Utils(Context context) {
        mContext = context;
    }

    public static void DeleteLatch() {
        if (mLatch.getCount() != 1) {
            Log.w(TAG, "Expecting latch to be 1, but it's not! It is : " + mLatch.getCount());
        } else {
            mLatch.countDown();
        }
    }

    public static void SetLatch() throws MalformedMimeTypeException {
        mLatch = new CountDownLatch(1);
    }

    public static void CreateOneOmacpMessage(String content) throws InterruptedException,
            MalformedMimeTypeException {
        SetLatch();

        IntentFilter filter = new IntentFilter();
        filter.addAction("android.provider.Telephony.WAP_PUSH_RECEIVED");
        filter.addDataType("application/vnd.wap.connectivity-wbxml");
        filter.addDataType("text/vnd.wap.connectivity-xml");
        mContext.registerReceiver(mMessageReceiver, filter);

        SendOmacpMessage.sendOmacpMessage(mContext, content);
        boolean timedout = !mLatch.await(MESSAGE_RECEIVE_TIMEOUT, TimeUnit.MILLISECONDS);
        if (timedout) {
            Log.e(TAG, "CreateOneOmacpMessage failed.");
        }
    }

    public static void CreateWbXmlMessage(int eventType, byte[] content) throws InterruptedException,
            MalformedMimeTypeException {
        SetLatch();

        IntentFilter filter = new IntentFilter();
        filter.addAction("android.provider.Telephony.WAP_PUSH_RECEIVED");
        filter.addDataType("application/vnd.wap.connectivity-wbxml");
        filter.addDataType("text/vnd.wap.connectivity-xml");
        mContext.registerReceiver(mMessageReceiver, filter);

        SendOmacpMessage.sendOmacpMessageWithPin(eventType, mContext, content);
        boolean timedout = !mLatch.await(MESSAGE_RECEIVE_TIMEOUT, TimeUnit.MILLISECONDS);
        if (timedout) {
            Log.e(TAG, "CreateOneOmacpMessage failed.");
        }
    }

    private static BroadcastReceiver mMessageReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context arg0, Intent intent) {
            // TODO Auto-generated method stub
            if (intent.getAction().equals("android.provider.Telephony.WAP_PUSH_RECEIVED")) {
                DeleteLatch();
                mContext.unregisterReceiver(mMessageReceiver);
            }
        }
    };

    public static void insertOneMessage(String data, String summary) {
        ContentValues value = new ContentValues();
        value.put("sim_id", 0);
        value.put("sender", "+8613612345678");
        value.put("service_center", "13812345678");
        value.put("seen", 0);
        value.put("read", 0);
        int currentTime = (int) (System.currentTimeMillis() / 1000);
        value.put("date", currentTime);
        value.put("installed", 0);
        value.put("pin_unlock", 1);
        value.put("sec", "");
        value.put("mac", "");
        value.put("title", "Configuration Message");
        value.put("summary", summary);
        value.put("body",
                (SendOmacpMessage.DOCUMENT_START + data + SendOmacpMessage.DOCUMENT_END).getBytes());
        value.put("context", "Settings");
        value.put("mime_type", "text/vnd.wap.connectivity-xml");

        Uri uri = mContext.getContentResolver().insert(OmacpProviderDatabase.CONTENT_URI, value);
        if (uri != null) {
            SharedPreferences sh = mContext.getSharedPreferences("omacp",
                    mContext.MODE_PRIVATE);
            Editor editor = sh.edit();
            editor.putBoolean("configuration_msg_exist", true);
            editor.commit();
        }
    }

    public static void DeleteAllMessages() {
        mContext.getContentResolver().delete(OmacpProviderDatabase.CONTENT_URI, null, null);
    }

    public static void InstallOneSetting(String appId, Instrumentation inst)
            throws InterruptedException, MalformedMimeTypeException {
        // if receive the result early than this function is called, then return
        // directly
        if (appId.equals(mResultMap.get("appId"))) {
            if (mResultMap.get("result") != null) {
                Log.w(TAG, "InstallOneSetting done before await.");
                return;
            }
        }

        // receive result latter than called, this is the normal case
        SetLatch();
        boolean timedout = !mLatch.await(MESSAGE_RECEIVE_TIMEOUT, TimeUnit.MILLISECONDS);
        if (timedout) {
            Log.e(TAG, "InstallOneSetting timeout.");
        } else {
            Log.i(TAG, "InstallOneSetting done.");
        }
    }

    public static void registerResultReceiver() {
        IntentFilter filter = new IntentFilter();
        filter.addAction("com.mediatek.omacp.settings.result");
        mContext.registerReceiver(mResultReceiver, filter);
    }

    public static void unregisterResultReceiver() {
        mContext.unregisterReceiver(mResultReceiver);
    }

    private static BroadcastReceiver mResultReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent.getAction().equals("com.mediatek.omacp.settings.result")) {
                String appId = intent.getStringExtra("appId");
                boolean result = intent.getBooleanExtra("result", false);

                Log.i(TAG, "mResultMap receive appId is : " + appId + " result is: " + result);
                mResultMap.clear();
                mResultMap.put("appId", appId);
                mResultMap.put("result", result);
                if (mLatch.getCount() == 1) {
                    DeleteLatch();
                }
            }
        }
    };

    public static HashMap mResultMap = new HashMap();

}
