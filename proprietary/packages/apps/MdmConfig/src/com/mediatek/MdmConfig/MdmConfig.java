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
/**
 * Description: To set MDM config status
 *
 * @author mtk80144
 *
 */
package com.mediatek.mdmconfig;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.SystemProperties;
import android.os.SystemService;
import android.preference.Preference;
import android.preference.PreferenceScreen;
import android.preference.SwitchPreference;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.ProgressBar;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import java.util.concurrent.TimeoutException;
import java.lang.InterruptedException;

public class MdmConfig extends Activity {
    private static final String TAG = "MdmConfig";

    private static final String MDM_CONFIG_PROP = "persist.vendor.mdmmonitor";
    private static final String KEY_MD_MONITOR_SWITCH = "md_monitor_switch";
    private static final String SERVICE_NAME = "md_monitor";

    private static final int ACTION_START = 0;
    private static final int ACTION_STOP = 1;
    private static final int WAIT_TIMEOUT = 3000;
    private Button mSetBut;
    private Switch mSwtich;
    private ProgressBar  mProgressBar;

    public class UIUpdater implements Runnable {
        public void run () {
            boolean isrun = SystemService.isRunning(SERVICE_NAME);
            boolean isstop = SystemService.isStopped(SERVICE_NAME);
            Log.v(TAG, "updateServiceState isrun: " + isrun);
            if (isrun == true) {
                mProgressBar.setVisibility(View.INVISIBLE);
                mSetBut.setText(R.string.mdm_config_stop);
                mSetBut.setEnabled(true);
            } else if (isstop == true) {
                mProgressBar.setVisibility(View.INVISIBLE);
                mSetBut.setEnabled(true);
                mSetBut.setText(R.string.mdm_config_start);
            } else if (SystemService.State.RESTARTING == SystemService.getState(SERVICE_NAME)) {
                mProgressBar.setVisibility(View.VISIBLE);
                mSetBut.setText(R.string.mdm_config_stop);
                mSetBut.setEnabled(false);
                waitServiceSwitchDone(ACTION_STOP);
            } else if (SystemService.State.STOPPING == SystemService.getState(SERVICE_NAME)) {
                mProgressBar.setVisibility(View.VISIBLE);
                mSetBut.setEnabled(false);
                mSetBut.setText(R.string.mdm_config_start);
                waitServiceSwitchDone(ACTION_START);
            }else {
                Log.v(TAG, "updateServiceState sate = " + SystemService.getState(SERVICE_NAME));
            }
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.setContentView(R.layout.mdm_config_activity);
        mSetBut = (Button) findViewById(R.id.mdm_set_button);
        mSwtich = (Switch) findViewById(R.id.is_check);
        mProgressBar = (ProgressBar) findViewById(R.id.progressBar_Circle);
        mSwtich.setClickable(true);
        mSwtich.setEnabled(true);
        mProgressBar.setVisibility(View.INVISIBLE);

        mSwtich.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            public void onCheckedChanged(CompoundButton view, boolean isChecked) {
                if (isChecked) {
                    Log.d(TAG, "set enable");
                    SystemProperties.set(MDM_CONFIG_PROP, "1");
                    waitServiceSwitchDone(ACTION_START);
                } else {
                    Log.d(TAG, "set disable");
                    SystemProperties.set(MDM_CONFIG_PROP, "0");
                    waitServiceSwitchDone(ACTION_STOP);
                }
            }
        });

          mSetBut.setOnClickListener(new View.OnClickListener() {

            public void onClick(View v) {
                boolean isrun = SystemService.isRunning(SERVICE_NAME);
                Log.v(TAG, "setOnClickListener isrun state: " + isrun);
                mProgressBar.setVisibility(View.VISIBLE);
                mSetBut.setEnabled(false);
                if (isrun) {
                    setMDMConfig(ACTION_STOP);
                } else {
                    setMDMConfig(ACTION_START);
                }
            }
        });
    }

    protected void onResume() {
        super.onResume();
        updateConfigProp();
        runOnUiThread(new UIUpdater());
    }

    private void waitServiceSwitchDone(int action) {
        Log.v(TAG, "waitServiceSwitchDone action: " + action);
        try {
            if (ACTION_STOP == action) {
                SystemService.waitForState(SERVICE_NAME, SystemService.State.STOPPED,  WAIT_TIMEOUT);
                boolean isstop = SystemService.isStopped(SERVICE_NAME);
                if (isstop == false) {
                    Log.v(TAG, "waitServiceSwitchDone switch failed time out");
                }
                runOnUiThread(new UIUpdater());
            } else if (ACTION_START == action) {
                SystemService.waitForState(SERVICE_NAME, SystemService.State.RUNNING,  WAIT_TIMEOUT);
                boolean isrun = SystemService.isRunning(SERVICE_NAME);
                if (isrun == false) {
                    Log.v(TAG, "waitServiceSwitchDone switch failed time out");
                }
                runOnUiThread(new UIUpdater());
            }
        } catch (TimeoutException e) {
            e.printStackTrace();
        }
    }

    private void updateConfigProp() {
        String isAutoStart = SystemProperties.get(MDM_CONFIG_PROP);
        Log.v(TAG, "updateConfigProp is auto start = " + isAutoStart);
        if (isAutoStart.equals("1")) {
            mSwtich.setChecked(true);
        } else {
            mSwtich.setChecked(false);
        }
    }
    
    private void setMDMConfig(int action) {
        Log.v(TAG, "setMDMConfig action: " + action);
        if (ACTION_STOP == action) {
            SystemProperties.set("ctl.stop", "md_monitor");
            waitServiceSwitchDone(ACTION_STOP);
        } else if (ACTION_START == action) {
            SystemProperties.set("ctl.start", "md_monitor");
            waitServiceSwitchDone(ACTION_START);
        }
    }
}
