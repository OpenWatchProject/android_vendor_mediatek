/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.engineermode.power;

import android.app.Activity;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.widget.CompoundButton;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.ToggleButton;

import com.mediatek.engineermode.FeatureSupport;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.ShellExe;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;

public class ChargeBattery extends Activity {

    private static final String TAG = "Power/ChargeBattery";
    private static final String FILE_GM30 = "/sys/devices/platform/battery/FG_daemon_disable";
    private static final String FILE_NAFG = "/sys/devices/platform/battery/disable_nafg";
    private static final String ENABLED = "0";
    private static final String DISABLED = "1";

    private TextView mInfo = null;
    private ToggleButton mGm30Toggle;
    private ToggleButton mNafgToggle;
    private Toast mToast;

    private String mCmdString = null;
    private static final int EVENT_UPDATE = 1;
    private static final float FORMART_TEN = 10.0f;

    private final String[][] mFiles_old = {
    { "ADC_Charger_Voltage", "mV" }, { "Power_On_Voltage", "mV" }, { "Power_Off_Voltage", "mV" },
            { "Charger_TopOff_Value", "mV" }, { "FG_Battery_CurrentConsumption", "mA" },
            { "SEP", "" },
            { "ADC_Channel_0_Slope", "" }, { "ADC_Channel_1_Slope", "" },
            { "ADC_Channel_2_Slope", "" },{ "ADC_Channel_3_Slope", "" },
            { "ADC_Channel_4_Slope", "" }, { "ADC_Channel_5_Slope", "" },
            { "ADC_Channel_6_Slope", "" }, { "ADC_Channel_7_Slope", "" },
            { "ADC_Channel_8_Slope", "" },{ "ADC_Channel_9_Slope", "" },
            { "ADC_Channel_10_Slope", "" }, { "ADC_Channel_11_Slope", "" },
            { "ADC_Channel_12_Slope", "" }, { "ADC_Channel_13_Slope", "" },
            { "SEP", "" }, { "ADC_Channel_0_Offset", "" },
            { "ADC_Channel_1_Offset", "" }, { "ADC_Channel_2_Offset", "" },
            { "ADC_Channel_3_Offset", "" },{ "ADC_Channel_4_Offset", "" },
            { "ADC_Channel_5_Offset", "" },{ "ADC_Channel_6_Offset", "" },
            { "ADC_Channel_7_Offset", "" }, { "ADC_Channel_8_Offset", "" },
            { "ADC_Channel_9_Offset", "" },{ "ADC_Channel_10_Offset", "" },
            { "ADC_Channel_11_Offset", "" },{ "ADC_Channel_12_Offset", "" },
            { "ADC_Channel_13_Offset", "" } };

    private final String[][] mFiles_new = {
                                { "ADC_Charger_Voltage", "mV" ,"charger/" },
                                { "Power_On_Voltage", "mV" ,"battery/"},
                                { "Power_Off_Voltage", "mV" ,"battery/"},
                                { "FG_Battery_CurrentConsumption", "mA","battery/"}
                            };

    private  String[][] mFiles = mFiles_old;

    private boolean mRun = false;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.battery_charge);

        mInfo = (TextView) findViewById(R.id.battery_charge_info_text);

        //For chips after kibo/whitney, no need check cmd string by chip
        if(FeatureSupport.isGauge30Support()) {
            mFiles = mFiles_new;
            mCmdString = "cat /sys/devices/platform/";
        } else {
            mCmdString = "cat /sys/devices/platform/battery/";
        }

        if(!isSupportDisableGm30()) {
            findViewById(R.id.gm30_layout).setVisibility(View.GONE);
        } else {
            mGm30Toggle = (ToggleButton) findViewById(R.id.gm30_toggle);
            mNafgToggle = (ToggleButton) findViewById(R.id.nafg_toggle);
            boolean isGm30Disabled = isGm30Disabled();
            if(isGm30Disabled == true) {
                mGm30Toggle.setChecked(true);
                mGm30Toggle.setEnabled(false);
                mNafgToggle.setEnabled(false);
            } else {
                mGm30Toggle.setChecked(false);
                mNafgToggle.setChecked(isNafgDisabled());
                mGm30Toggle.setOnCheckedChangeListener(mListener);
                mNafgToggle.setOnCheckedChangeListener(mListener);
            }
        }
    }

    private String getInfo(String cmd) {
        String result = null;
        try {
            String[] cmdx = { "/system/bin/sh", "-c", cmd }; // file must
            int ret = ShellExe.execCommand(cmdx);
            if (0 == ret) {
                result = ShellExe.getOutput();
            } else {
                result = "ERR.JE";
            }
        } catch (IOException e) {
            Elog.i(TAG, e.toString());
            result = "ERR.JE";
        }
        return result;
    }

    private static final int UPDATE_INTERVAL = 500; // 0.5 sec

    public Handler mUpdateHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case EVENT_UPDATE:
                Bundle b = msg.getData();
                mInfo.setText(b.getString("INFO"));
                break;
            default:
                break;
            }
        }
    };

    private float getMeanBatteryVal(String filePath, int totalCount, int intervalMs) {
        float meanVal = 0.0f;
        if (totalCount <= 0) {
            return 0.0f;
        }
        for (int i = 0; i < totalCount; i++) {
            try {
                float f = Float.valueOf(getFileContent(filePath));
                meanVal += f / totalCount;
            } catch (NumberFormatException e) {
                Elog.e(TAG, "getMeanBatteryVal invalid result from cmd:" + filePath);
            }
            if (intervalMs <= 0) {
                continue;
            }
            try {
                Thread.sleep(intervalMs);
            } catch (InterruptedException e) {
                Elog.e(TAG, "Catch InterruptedException");
            }
        }
        return meanVal;
    }

    private static String getFileContent(String filePath) {
        if (filePath == null) {
            return null;
        }
        StringBuilder builder = new StringBuilder();
        BufferedReader reader = null;
        try {
            reader = new BufferedReader(new FileReader(filePath));
            char[] buffer = new char[500];
            int ret = -1;
            while ((ret = reader.read(buffer)) != -1) {
                builder.append(buffer, 0, ret);
            }
        } catch (IOException e) {
            Elog.e(TAG, "IOException:" + e.getMessage());
        } finally {
            if (reader != null) {
                try {
                    reader.close();
                } catch (IOException e) {
                    Elog.e(TAG, "IOException:" + e.getMessage());
                }
            }
        }
        String result = builder.toString();
        if (result != null) {
            result = result.trim();
        }
        return result;
    }

    class FunctionThread extends Thread {

        @Override
        public void run() {
            while (mRun) {
                StringBuilder text = new StringBuilder("");
                String cmd = "";
                for (int i = 0; i < mFiles.length; i++) {
                    if (mFiles[i][0].equalsIgnoreCase("SEP")) {
                        text.append("- - - - - - - - -\n");
                        continue;
                    }

                    if(FeatureSupport.isGauge30Support()){
                        cmd = mCmdString + mFiles[i][2]  + mFiles[i][0];
                    }
                    else{
                        cmd = mCmdString + mFiles[i][0];
                    }
                    String filePath = cmd;

                    if (filePath.startsWith("cat ")) {
                        filePath = filePath.substring(3).trim();
                    }
                    if (mFiles[i][1].equalsIgnoreCase("mA")) {
                        double f = 0.0f;
                        if (mFiles[i][0].equalsIgnoreCase("FG_Battery_CurrentConsumption")) {
                            f = getMeanBatteryVal(filePath, 5, 0) / FORMART_TEN;
                        } else {
                            try {
                                f = Float.valueOf(getFileContent(filePath)) / FORMART_TEN;
                            } catch (NumberFormatException e) {
                                Elog.e(TAG, "read file error " + mFiles[i][0]);
                            }
                        }
                        text.append(String.format("%1$-28s:[ %2$-6s ]%3$s\n", mFiles[i][0], f,
                                mFiles[i][1]));
                    } else {
                        text.append(String.format("%1$-28s: [ %2$-6s ]%3$s\n",
                                mFiles[i][0], getFileContent(filePath), mFiles[i][1]));
                    }
                }

                Bundle b = new Bundle();
                b.putString("INFO", text.toString());

                Message msg = new Message();
                msg.what = EVENT_UPDATE;
                msg.setData(b);

                mUpdateHandler.sendMessage(msg);
                try {
                    sleep(UPDATE_INTERVAL);
                } catch (InterruptedException e) {
                    Elog.e(TAG, "Catch InterruptedException");
                }
            }
        }
    }

    private CompoundButton.OnCheckedChangeListener mListener =
            new CompoundButton.OnCheckedChangeListener () {

        @Override
        public void onCheckedChanged(CompoundButton arg0, boolean arg1) {
            if(arg0.equals(mGm30Toggle) && arg1 == true) {
                boolean result = disalbeGm30();
                if(result == true) {
                    //NAFG is sub_function of GM30
                    //GM30 don't allow enabled
                    mGm30Toggle.setEnabled(false);
                    mNafgToggle.setEnabled(false);
                } else {
                    arg0.setChecked(!arg1);
                }
            } else if (arg0.equals(mNafgToggle)){
                    boolean result = disalbeNafg(arg1);
                    if(result == false) {
                        arg0.setChecked(!arg1);
                    }
            }
        }

    };

    private boolean isSupportDisableGm30() {
        return new File(FILE_GM30).exists();
    }

    private boolean isGm30Disabled() {
        String value = getInfo("cat " + FILE_GM30);
        return DISABLED.equals(value);
    }

    private boolean isNafgDisabled() {
        String value = getInfo("cat " + FILE_NAFG);
        return DISABLED.equals(value);
    }

    private boolean disalbeGm30() {
        String cmd = "echo 1 > " + FILE_GM30;
        String result = getInfo(cmd);
        if (null != result && result.equals("ERR.JE")) {
            showToast("Disalbe GM30 failed!");
            return false;
        }else{
            showToast("Disalbe GM30 succeed!");
            return true;
        }
    }

    private boolean disalbeNafg(boolean arg1) {
        String value = arg1? DISABLED : ENABLED;
        String msg = arg1? "Disable" : "Enable";
        String cmd = "echo " + value + " > " + FILE_NAFG;
        String result = getInfo(cmd);
        if (null != result && result.equals("ERR.JE")) {
            showToast(msg + " NAFG failed!");
            return false;
        } else {
            showToast(msg + " NAFG succeed!");
            return true;
        }
    }

    private void showToast(String msg) {
        if (mToast != null) {
            mToast.cancel();
        }
        mToast = Toast.makeText(this, msg, Toast.LENGTH_SHORT);
        mToast.show();
    }

    @Override
    public void onPause() {
        super.onPause();
        mRun = false;
    }

    @Override
    protected void onResume() {
        super.onResume();
        mRun = true;
        new FunctionThread().start();
    }

    @Override
    protected void onStop() {
        super.onStop();
    }

}
