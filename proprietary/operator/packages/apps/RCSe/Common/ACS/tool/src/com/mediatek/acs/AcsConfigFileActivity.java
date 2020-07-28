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

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.ActivityInfo;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.v4.content.LocalBroadcastManager;
import android.util.Log;
import android.widget.EditText;

import com.mediatek.ims.rcsua.AcsConfiguration;
import com.mediatek.ims.rcsua.RcsUaService;

public class AcsConfigFileActivity extends Activity {
    private Context mContext;
    private AcsUtils mAcsUtils = null;
    private EditText mConfigFileTV = null;
    private static final String TAG = "[ACSTOOL] AcsConfigFileActivity";

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.i(TAG, "onCreate");
        mContext = this;
        mAcsUtils = AcsUtils.getInstance(mContext);
        // Set layout
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
        setContentView(R.layout.acs_file_content);
        registerReceiver();

        mConfigFileTV = (EditText)this.findViewById(R.id.AcsConfiFile);
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.i(TAG, "onResume");
        update(AcsUtils.SCONNECTION_CHGNAGE);
    }

    /**
     * Describe <code>onDestroy</code> method here.
     *
     */
    public final void onDestroy() {
        super.onDestroy();
        Log.i(TAG, "onDestroy");
        unregisterReceiver();
    }

    private LocalBroadcastManager bm;
    private void registerReceiver() {
        bm = LocalBroadcastManager.getInstance(mContext);
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(AcsUtils.LOCAL_ACTION);
        bm.registerReceiver(mReceiver, intentFilter);
    }

    private void unregisterReceiver() {
        if (bm != null) {
            bm.unregisterReceiver(mReceiver);
        }
    }

    private static final int MESSAGE_CONFIG_CHANGE = 1;
    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                Log.d(TAG, "BroadcastReceiver:" + intent);
                String change = intent.getStringExtra("change");
                int type = intent.getIntExtra("type", -1);
                if ("yes".equals(change)) {
                    mHandler.obtainMessage(MESSAGE_CONFIG_CHANGE, type).sendToTarget();
                }
            };
        };

    private Handler mHandler = new Handler() {
            @Override
            public void handleMessage(Message msg) {
                int type = (int)msg.obj;
                Log.d(TAG, "receive msg:" + type);
                switch(msg.what) {
                case MESSAGE_CONFIG_CHANGE:
                    update(type);
                    break;

                default:
                    break;
                }
            }
        };

    private void update(int type) {
        RcsUaService uaService = mAcsUtils.getRcsUaService();
        if (uaService != null) {
            AcsConfiguration configinfo;
            try {
                configinfo = uaService.getAcsConfiguration();
            } catch (Exception e) {
                e.printStackTrace();
                return;
            }

            if (configinfo == null) {
                mConfigFileTV.setText("null");
                return;
            }

            String config =  configinfo.readXmlData();
            mConfigFileTV.setText(config);
            // Log.i(TAG, "file_content:" + config);
        }
    }
}
