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

package com.mediatek.engineermode.power;

import android.app.Activity;
//import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
//import android.widget.ArrayAdapter;
import android.widget.Spinner;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.FeatureSupport;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.ShellExe;

import java.io.IOException;

public class LowPowerDebug extends Activity {
    private static final String TAG = "Power/LowPowerDebug";
    private static final String[] STOP_STRS_ARRAY = {"0", "1"};
    private static final String[] UT_STRS_ARRAY = {"0", "1", "2"};

    private static final String INFO_PATH = "/sys/bus/platform/devices/mt-pmic/";
    private static final String LP_PROTECT_STOP = "low_battery_protect_stop";
    private static final String LP_PROTECT_UT = "low_battery_protect_ut";

    private Spinner mStopSpinner;
    private Spinner mUtSpinner;

    private final OnItemSelectedListener mSpinnerListener = new OnItemSelectedListener() {
        public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2,
                long arg3) {
            String cmd = null;
            switch (arg0.getId()) {
                case R.id.low_power_stop_select :
                    cmd = "echo " + arg2 + " > " + INFO_PATH + LP_PROTECT_STOP;
                    break;
                case R.id.low_power_ut_select :
                    cmd = "echo " + arg2 + " > " + INFO_PATH + LP_PROTECT_UT;
                    break;
                default:
                    break;
            }
            if (cmd != null) {
                if (setCommand(cmd) != 0) {
                    Toast.makeText(LowPowerDebug.this, "Set failed!", Toast.LENGTH_LONG)
                        .show();
                }
            }
        }
        public void onNothingSelected(AdapterView<?> arg0) {
            Elog.v(TAG, "Spinner nothing selected");
        }
    };
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.low_power_debug);
        if (!FeatureSupport.isSupportedEmSrv()) {
            Toast.makeText(this, R.string.notice_wo_emsvr,
                    Toast.LENGTH_LONG).show();
            finish();
            return;
        }
        mStopSpinner = (Spinner) findViewById(R.id.low_power_stop_select);
        mUtSpinner = (Spinner) findViewById(R.id.low_power_ut_select);
        mStopSpinner.setOnItemSelectedListener(mSpinnerListener);
        mUtSpinner.setOnItemSelectedListener(mSpinnerListener);

    }

    @Override
    protected void onResume() {
        super.onResume();
        initSpinner();
    }

    private void initSpinner() {
        String cmd = "cat " + INFO_PATH + LP_PROTECT_STOP;
        String result = getCommand(cmd);
        boolean getResult = true;
        if (result != null) {
            int index = Integer.valueOf(result);
            mStopSpinner.setSelection(index);
        } else {
            getResult = false;
        }
        cmd = "cat " + INFO_PATH + LP_PROTECT_UT;
        result = getCommand(cmd);
        if (result != null) {
            int index = Integer.valueOf(result);
            mUtSpinner.setSelection(index);
        } else {
            getResult = false;
        }
        if (getResult == false) {
            Toast.makeText(this, "Get status failed!", Toast.LENGTH_LONG)
                        .show();
        }
    }

    private String getCommand(String cmd) {
         int ret = -1;
         Elog.v(TAG, "[cmd]:" + cmd);
         try {
             ret = ShellExe.execCommand(cmd);
         } catch (IOException e) {
             Elog.e(TAG, "IOException: " + e.getMessage());
         }
         if (ret == 0) {
             String outStr = ShellExe.getOutput();
             Elog.d(TAG, "[output]: " + outStr);
             return outStr;
         }
         return null;
     }

    private int setCommand(String cmd) {
        int ret = -1;
        Elog.v(TAG, "[cmd]:" + cmd);
        try {
            ret = ShellExe.execCommand(cmd);
        } catch (IOException e) {
            Elog.e(TAG, "IOException: " + e.getMessage());
        }
        return ret;
    }
}
