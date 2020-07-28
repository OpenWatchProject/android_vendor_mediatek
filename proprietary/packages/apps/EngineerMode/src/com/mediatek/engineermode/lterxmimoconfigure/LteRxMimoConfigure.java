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

package com.mediatek.engineermode.lterxmimoconfigure;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.R;


public class LteRxMimoConfigure extends Activity implements View.OnClickListener, CompoundButton
        .OnCheckedChangeListener {
    private static final String TAG = "LteRx/MimoConfigure";
    private static final int WARNING_MSG_REBOOT = 1;
    private static final int MSG_QUERY_CMD = 101;
    private static final int MSG_SET_CMD = 102;
    private static final int MSG_LTE_BAND_SETTING_4MIMO = 103;
    private static final int MSG_LTE_BAND_SETTING_4RX = 104;
    private CheckBox mRxMimoFeatureEnableCb;
    private CheckBox mRxMimo44MomoCb;
    private CheckBox mRxMimo44MomoUnderCCACb;
    private CheckBox mRxMimoRas4Rx2RxCb;
    private CheckBox mRxMimoRas4Rx2RxUnderCCACb;
    private CheckBox mRxMimoRas2Rx1RxCb;

    private Button mRxMimo44SingleBandSettingBt;
    private Button mRxMimoRas4RxSingleBandSettingBt;

    private Button mSetBt;

    private int mRxMimoFeatureEnableCurStatus = 0;
    private int mRxMimo44MomoCurStatus = 0;
    private int mRxMimo44MomoUnderCCACurStatuss = 0;
    private int mRxMimoRas4Rx2RxCurStatus = 0;
    private int mRxMimoRas4Rx2RxUnderCCACurStatus = 0;
    private int mRxMimoRas2Rx1RxCurStatus = 0;

    private Handler mCommandHander = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            AsyncResult asyncResult = (AsyncResult) msg.obj;
            switch (msg.what) {
                case MSG_QUERY_CMD:
                    final String[] result = (String[]) asyncResult.result;
                    if (result != null && result.length > 0) {
                        Elog.d(TAG, "LteRxMimo Query Succeed,result = " + result[0]);
                        EmUtils.showToast("LteRxMimo Query Succeed!");
                        parseCurrentLteMode(result[0]);
                    } else {
                        Elog.e(TAG, "LteRxMimo Query Failed!");
                        EmUtils.showToast("LteRxMimo Query Failed!");
                        //parseCurrentLteMode("123");
                    }
                    updateButtonStatus();
                    break;
                case MSG_SET_CMD:
                    if (null == asyncResult.exception) {
                        EmUtils.showToast("LteRxMimo Set Succeed!");
                        Elog.d(TAG, "LteRxMimo Set Succeed!");
                        showDialog(WARNING_MSG_REBOOT);
                    } else {
                        EmUtils.showToast("LteRxMimo Set failed!");
                        Elog.e(TAG, "LteRxMimo Set failed!");
                    }
                    break;
                case MSG_LTE_BAND_SETTING_4MIMO:
                    if (null == asyncResult.exception) {
                        EmUtils.showToast("4x4MIMO single band setting  Succeed!");
                        Elog.d(TAG, "4x4MIMO single band setting Succeed!");
                    } else {
                        EmUtils.showToast("4x4MIMO single band setting failed!");
                        Elog.e(TAG, "4x4MIMO single band setting failed!");
                    }
                    break;
                case MSG_LTE_BAND_SETTING_4RX:
                    if (null == asyncResult.exception) {
                        EmUtils.showToast("4RX single band setting Succeed!");
                        Elog.d(TAG, "4RX single band setting Succeed!");
                    } else {
                        EmUtils.showToast("4RX single band setting failed!");
                        Elog.e(TAG, "4RX single band setting failed!");
                    }
                    break;
                default:
                    break;
            }
        }
    };


    private void parseCurrentLteMode(String data) {
        try {
            String info[] = data.split(",");
            mRxMimoFeatureEnableCurStatus = Integer.valueOf(info[1].trim());
            mRxMimo44MomoCurStatus = Integer.valueOf(info[2].trim());
            mRxMimo44MomoUnderCCACurStatuss = Integer.valueOf(info[3].trim());
            mRxMimoRas4Rx2RxCurStatus = Integer.valueOf(info[4].trim());
            mRxMimoRas4Rx2RxUnderCCACurStatus = Integer.valueOf(info[5].trim());
            mRxMimoRas2Rx1RxCurStatus = Integer.valueOf(info[6].trim());
        } catch (NumberFormatException e) {
            Elog.e(TAG, "Wrong current mode format: " + data);
            return;
        }
    }


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.setContentView(R.layout.lte_rx_mimo_configure);
        mRxMimoFeatureEnableCb = (CheckBox) findViewById(R.id.lte_rx_mimo_feature_enable);
        mRxMimo44MomoCb = (CheckBox) findViewById(R.id.lte_rx_mimo_4_momo);
        mRxMimo44MomoUnderCCACb = (CheckBox) findViewById(R.id.lte_rx_mimo_4_momo_under_cca);
        mRxMimoRas4Rx2RxCb = (CheckBox) findViewById(R.id.lte_rx_mimo_ras_4rx_2rx);
        mRxMimoRas4Rx2RxUnderCCACb =
                (CheckBox) findViewById(R.id.lte_rx_mimo_ras_4rx_2rx_under_cca);
        mRxMimoRas2Rx1RxCb = (CheckBox) findViewById(R.id.lte_rx_mimo_ras_2rx_1rx);

        mRxMimo44SingleBandSettingBt = (Button) findViewById(R.id.lte_4x4MIMO_single_band_setting);
        mRxMimoRas4RxSingleBandSettingBt = (Button) findViewById(R.id.lte_4RX_single_band_setting);

        mSetBt = (Button) findViewById(R.id.lte_rx_mimo_set_button);
        mSetBt.setOnClickListener(this);
        mRxMimo44SingleBandSettingBt.setOnClickListener(this);
        mRxMimoRas4RxSingleBandSettingBt.setOnClickListener(this);
        mRxMimoFeatureEnableCb.setOnCheckedChangeListener(this);
        mRxMimo44MomoCb.setOnCheckedChangeListener(this);
        mRxMimoRas4Rx2RxCb.setOnCheckedChangeListener(this);

        queryStatus();
    }

    private void queryStatus() {
        sendCommand(new String[]{"AT+EGMC=0,\"rx_mimo_set\"", "+EGMC:"}, MSG_QUERY_CMD);
    }

    private void sendCommand(String[] command, int msg) {
        Elog.d(TAG, "sendCommand " + command[0]);
        EmUtils.invokeOemRilRequestStringsEm(command, mCommandHander.obtainMessage(msg));
    }

    void set4RxMimoConfigure() {
        String cmd = "";

        mRxMimoFeatureEnableCurStatus = mRxMimoFeatureEnableCb.isChecked() ? 1 : 0;
        if (mRxMimoFeatureEnableCurStatus == 1) {
            mRxMimo44MomoCurStatus = mRxMimo44MomoCb.isChecked() ? 1 : 0;
            if (mRxMimo44MomoCurStatus == 1)
                mRxMimo44MomoUnderCCACurStatuss = mRxMimo44MomoUnderCCACb.isChecked() ? 1 : 0;
            else
                mRxMimo44MomoUnderCCACurStatuss = 0;

            mRxMimoRas4Rx2RxCurStatus = mRxMimoRas4Rx2RxCb.isChecked() ? 1 : 0;
            if (mRxMimoRas4Rx2RxCurStatus == 1)
                mRxMimoRas4Rx2RxUnderCCACurStatus = mRxMimoRas4Rx2RxUnderCCACb.isChecked() ? 1 : 0;
            else
                mRxMimoRas4Rx2RxUnderCCACurStatus = 0;
            mRxMimoRas2Rx1RxCurStatus = mRxMimoRas2Rx1RxCb.isChecked() ? 1 : 0;
        } else {
            mRxMimo44MomoCurStatus = 0;
            mRxMimo44MomoUnderCCACurStatuss = 0;
            mRxMimoRas4Rx2RxCurStatus = 0;
            mRxMimoRas4Rx2RxUnderCCACurStatus = 0;
            mRxMimoRas2Rx1RxCurStatus = 0;
        }

        cmd = "AT+EGMC=1,\"rx_mimo_set\"," + mRxMimoFeatureEnableCurStatus + ","
                + mRxMimo44MomoCurStatus + ","
                + mRxMimo44MomoUnderCCACurStatuss + ","
                + mRxMimoRas4Rx2RxCurStatus + ","
                + mRxMimoRas4Rx2RxUnderCCACurStatus + ","
                + mRxMimoRas2Rx1RxCurStatus;
        sendCommand(new String[]{cmd, ""}, MSG_SET_CMD);
    }

    void setLteSingleBandSetting() {
        Elog.d(TAG, "LteBandSettingModeHistory = " +
                LteSingleBandSetting.LteBandSettingModeHistory);
        if (LteSingleBandSetting.LteBandSettingModeHistory == 0) {
            Elog.d(TAG, "skip LteSingleBandSetting");
        }
        if ((LteSingleBandSetting.LteBandSettingModeHistory & 0x1) == 0x1 &&
                mRxMimo44MomoCurStatus == 1) {
            sendCommand(new String[]{"AT+EGMC=1," + LteSingleBandSetting.CMD_SB_MIMO_SET +
                    LteSingleBandSetting.mSbMimoSetString[0], ""}, MSG_LTE_BAND_SETTING_4MIMO);
        }
        if ((LteSingleBandSetting.LteBandSettingModeHistory & 0x2) == 0x2 &&
                mRxMimoRas4Rx2RxCurStatus == 1) {
            sendCommand(new String[]{"AT+EGMC=1," + LteSingleBandSetting.CMD_SB_4RX_SET +
                    LteSingleBandSetting.mSbMimoSetString[1], ""}, MSG_LTE_BAND_SETTING_4RX);
        }
    }

    @Override
    public void onClick(View v) {
        Intent intent = new Intent();
        intent.setClass(this, LteSingleBandSetting.class);
        if (R.id.lte_rx_mimo_set_button == v.getId()) {
            set4RxMimoConfigure();
            setLteSingleBandSetting();
            Elog.d(TAG, "lte_rx_mimo_set_button click");
        } else if (R.id.lte_4x4MIMO_single_band_setting == v.getId()) {
            Elog.d(TAG, "4x4MIMO single band setting click");
            intent.putExtra("band_setting_type", LteSingleBandSetting
                    .LTE_BAND_SETTING_MODE_4X4MIMO);
            this.startActivity(intent);
        } else if (R.id.lte_4RX_single_band_setting == v.getId()) {
            Elog.d(TAG, "4RX single band setting click");
            intent.putExtra("band_setting_type", LteSingleBandSetting.LTE_BAND_SETTING_MODE_4RX);
            this.startActivity(intent);
        }
    }

    void updateButtonStatus() {
        Elog.d(TAG, "updateButtonStatus");
        mRxMimoFeatureEnableCb.setChecked(mRxMimoFeatureEnableCurStatus == 1);
        mRxMimo44MomoCb.setChecked(mRxMimo44MomoCurStatus == 1);
        mRxMimo44MomoUnderCCACb.setChecked(mRxMimo44MomoUnderCCACurStatuss == 1);
        mRxMimoRas4Rx2RxCb.setChecked(mRxMimoRas4Rx2RxCurStatus == 1);
        mRxMimoRas4Rx2RxUnderCCACb.setChecked(mRxMimoRas4Rx2RxUnderCCACurStatus == 1);
        mRxMimoRas2Rx1RxCb.setChecked(mRxMimoRas2Rx1RxCurStatus == 1);

        if (mRxMimoFeatureEnableCurStatus == 0) {
            mRxMimo44MomoCb.setEnabled(false);
            mRxMimo44MomoUnderCCACb.setEnabled(false);
            mRxMimoRas4Rx2RxCb.setEnabled(false);
            mRxMimoRas4Rx2RxUnderCCACb.setEnabled(false);
            mRxMimo44SingleBandSettingBt.setEnabled(false);
            mRxMimoRas2Rx1RxCb.setEnabled(false);
        } else {
            mRxMimo44MomoCb.setEnabled(true);
            mRxMimo44MomoUnderCCACb.setEnabled(true);
            mRxMimoRas4Rx2RxCb.setEnabled(true);
            mRxMimoRas4Rx2RxUnderCCACb.setEnabled(true);
            mRxMimoRas2Rx1RxCb.setEnabled(true);
            mRxMimo44SingleBandSettingBt.setEnabled(true);
        }

        if ((mRxMimo44MomoCurStatus == 1) && (mRxMimoFeatureEnableCurStatus == 1)) {
            mRxMimo44MomoUnderCCACb.setEnabled(true);
            mRxMimo44SingleBandSettingBt.setEnabled(true);
        } else {
            mRxMimo44MomoUnderCCACb.setEnabled(false);
            mRxMimo44SingleBandSettingBt.setEnabled(false);
        }
        if ((mRxMimoRas4Rx2RxCurStatus == 1) && (mRxMimoFeatureEnableCurStatus == 1)) {
            mRxMimoRas4Rx2RxUnderCCACb.setEnabled(true);
            mRxMimoRas4RxSingleBandSettingBt.setEnabled(true);
        } else {
            mRxMimoRas4Rx2RxUnderCCACb.setEnabled(false);
            mRxMimoRas4RxSingleBandSettingBt.setEnabled(false);
        }
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        Dialog dialog = null;
        switch (id) {
            case WARNING_MSG_REBOOT:
                return new AlertDialog.Builder(this).setTitle(
                        "LteRxMimoConfigure").setMessage("It needs to reboot modem to enable " +
                        "this setting\n")
                        .setPositiveButton("Confirm", new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                if (which == DialogInterface.BUTTON_POSITIVE) {
                                    EmUtils.rebootModem();
                                }
                                dialog.dismiss();
                            }
                        })
                        .setNegativeButton("Cancel", null).create();
        }
        return dialog;
    }

    @Override
    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        switch (buttonView.getId()) {
            case R.id.lte_rx_mimo_feature_enable:
                mRxMimoFeatureEnableCurStatus = isChecked ? 1 : 0;
                Elog.d(TAG, "feature_enable clicked");
                break;
            case R.id.lte_rx_mimo_4_momo:
                mRxMimo44MomoCurStatus = isChecked ? 1 : 0;
                Elog.d(TAG, "4X4 MIMO clicked");
                break;
            case R.id.lte_rx_mimo_ras_4rx_2rx:
                mRxMimoRas4Rx2RxCurStatus = isChecked ? 1 : 0;
                Elog.d(TAG, "RAS_4RX_2RX switching");
                break;
            default:
                break;
        }
        updateButtonStatus();
    }
}
