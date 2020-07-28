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

package com.mediatek.engineermode.worldmode;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.CountDownTimer;
import android.os.Handler;
import android.os.Message;
import android.telephony.PhoneStateListener;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;

import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.TextView;
import android.widget.Toast;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.ModemCategory;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.WorldModeUtil;
import com.mediatek.engineermode.RatConfiguration;

/**
 * Set world mode.
 */
public class WorldModeActivity extends Activity implements View.OnClickListener {
     private final static String TAG = "WorldModeActivity";
    private static final int BAND_GSM_INDEX = 0;
    private static final int BAND_UTMS_INDEX = 1;
    private static final int BAND_LTEFDD_INDEX = 2;
    private static final int BAND_LTETDD_INDEX = 3;

    private static final int MASK_LTEFDD = (1 << 4);
    private static final int MASK_LTETDD = (1 << 3);
    private static final int MASK_WCDMA = (1 << 2);
    private static final int MASK_TDSCDMA = (1 << 1);
    private static final int MASK_GSM = (1);

    private static final int WORLD_MODE_AUTO = 10;
    private static final int WORLD_MODE_LWCG = 11;
    private static final int C2K_WORLD_MODE_AUTO = 12;
    private static final int WORLD_MODE_LTG = 8;
    private static final int WORLD_MODE_LWG = 9;
    private static final int WORLD_MODE_LTTG = 13;
    private static final int WORLD_MODE_LFWG = 14;
    private static final int WORLD_MODE_LTCTG = 17;
    private static final String ACTION_WORLD_MODE_CHANGED =
            WorldModeUtil.ACTION_WORLD_MODE_CHANGED;
    private static final String ACTION_SERVICE_STATE_CHANGED =
            TelephonyIntents.ACTION_SERVICE_STATE_CHANGED;

    private static final String ACTION_SET_RADIO_CAPABILITY_DONE =
            TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_DONE;
    private static final int EVENT_QUERY_CURRENT = 0;

    private static final int WORLD_MODE_RESULT_SUCCESS = 100;
    private static final int WORLD_MODE_RESULT_ERROR = 101;
    private static final int WORLD_MODE_RESULT_WM_ID_NOT_SUPPORT = 102;

    private static final int REBOOT_RF = 0;
    private static final int DIALOG_WAIT = 1;
    private static final int UNSUPPORT = 2;
    private static final int ERROR = 3;
    private static final int INDEX_BAND_MAX = 5;
    private static final boolean IS_NR_SUPPORT = RatConfiguration.isNrSupported();
    private static int mMode = 0;
    private static int mOldMode = 0;
    private static int mSetWorldMode = 0;
    private static int mBandMode = 0;
    private final int WAIT_TIME = 15;
    private CountDownTimer timer;
    private ProgressDialog mProgressDialog;
    private String[] mWorldMode = {"unknown", "lwg", "lttg"};
    private String[] mWorldModeFor6M = {"unknown", "LWCG", "LtCTG"};
    private RadioButton mRadioLttg;
    private RadioButton mRadioLtctg;
    private RadioButton mRadioLwg;
    private RadioButton mRadioLwcg;
    private RadioButton mRadioAuto;
    private RadioGroup mRadioGroup;
    private TextView mTextCurrentMode;
    private TextView mTextMainProtocol;
    private TextView mTextWorldModeHint;
    private EditText mValue;
    private Button mButtonSet;
    private Button mButtonSetValue;
    private Toast mToast;
    private boolean mStateFlag = false;

    private final Handler mResponseHander = new Handler() {
        public void handleMessage(final Message msg) {
            AsyncResult asyncResult;
            switch (msg.what) {
                case EVENT_QUERY_CURRENT:
                    asyncResult = (AsyncResult) msg.obj;
                    if (asyncResult.exception != null) {
                        EmUtils.showToast("Failed to query band");
                        Elog.e(TAG, "Failed to query band");
                    } else {
                        final String[] result = (String[]) asyncResult.result;
                        final String splitString = result[0].substring(7);
                        final String[] getDigitalVal = splitString.split(",");
                        long[] values = new long[INDEX_BAND_MAX];
                        if (getDigitalVal != null && getDigitalVal.length > 1) {
                            for (int i = 0; i < values.length; i++) {
                                if (getDigitalVal.length <= i || getDigitalVal[i] == null) {
                                    values[i] = 0;
                                    continue;
                                }
                                try {
                                    values[i] = Long.valueOf(getDigitalVal[i].trim());
                                } catch (NumberFormatException e) {
                                    values[i] = 0;
                                }
                            }
                        }
                        if (values[BAND_GSM_INDEX] > 0) {
                            mBandMode += MASK_GSM;
                        }
                        //wcdma
                        if ((values[BAND_UTMS_INDEX] & 0x3F) > 0) {
                            mBandMode += MASK_TDSCDMA;
                        }
                        //tdscdma
                        if ((values[BAND_UTMS_INDEX] & 0x3C0) > 0) {
                            mBandMode += MASK_WCDMA;
                        }
                        if (values[BAND_LTEFDD_INDEX] > 0) {
                            mBandMode += MASK_LTEFDD;
                        }
                        if (values[BAND_LTETDD_INDEX] > 0) {
                            mBandMode += MASK_LTETDD;
                        }
                        Elog.d(TAG, "mBandMode = " + mBandMode);
                        showDialog(REBOOT_RF);
                    }
                    break;
                default:
                    break;
            }
        }
    };

    private TelephonyManager mTelephonyManager;
    private PhoneStateListener mPhoneStateListener = new PhoneStateListener() {
        @Override
        public void onRadioPowerStateChanged(
                @TelephonyManager.RadioPowerState int state) {
            Elog.d(TAG, "RadioPowerState:" + state);
            if (state == TelephonyManager.RADIO_POWER_ON) {
                updateUi();
            }
        }
    };

    private final BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Elog.d(TAG, "[Receiver]+" + action);
            if (ACTION_SET_RADIO_CAPABILITY_DONE.equals(action)) {
                updateUi();
            } else if (ACTION_WORLD_MODE_CHANGED.equals(action)) {
                int worldModeState = intent.getIntExtra(
                        WorldModeUtil.EXTRA_WORLD_MODE_CHANGE_STATE, -1);
                if (worldModeState == WorldModeUtil.MD_WM_CHANGED_END) {
                    updateUi();
                }
            }
            Elog.d(TAG, "[Receiver]-");
        }
    };

   private final BroadcastReceiver
           mBroadcastReceiverServiceStateChanged= new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Elog.d(TAG, "[Receiver]+");
            ServiceState ss = ServiceState.newFromBundle(intent.getExtras());
            Elog.d(TAG, "ss.getState(): " + ss.getState() + ",mStateFlag = " + mStateFlag);
            Elog.d(TAG, "[Receiver ACTION_SERVICE_STATE_CHANGED]");
            if (ss.getState() != ServiceState.STATE_POWER_OFF) {
                if ((mSetWorldMode == 1) && (mStateFlag)) {
                    updateUi();
                    mStateFlag = false;
                    if (timer != null) {
                        timer.cancel();
                        timer = null;
                    }
                    if (mProgressDialog != null) {
                        mProgressDialog.dismiss();
                        mSetWorldMode = 0;
                    }
                }
            } else {
                if ((mSetWorldMode == 1) && (!mStateFlag)) {
                    mStateFlag = true;
                }
            }
            Elog.d(TAG, "[Receiver]-");
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.world_mode);
        mRadioLtctg = (RadioButton) findViewById(R.id.world_mode_ltctg);
        mRadioLttg = (RadioButton) findViewById(R.id.world_mode_lttg);
        mRadioLwg = (RadioButton) findViewById(R.id.world_mode_lwg);
        mRadioLwcg = (RadioButton) findViewById(R.id.world_mode_lwcg);
        mRadioAuto = (RadioButton) findViewById(R.id.world_mode_auto_switch);
        mRadioGroup = (RadioGroup) findViewById(R.id.world_mode_radio_group);
        mButtonSet = (Button) findViewById(R.id.world_mode_set);
        mTextCurrentMode = (TextView) findViewById(R.id.world_mode_current_value);
        mTextMainProtocol = (TextView) findViewById(R.id.world_mode_main_protocol);
        mTextWorldModeHint = (TextView) findViewById(R.id.world_mode_hint);
        mTextWorldModeHint.setText(R.string.world_mode_hint_aosp);
        mValue = (EditText) findViewById(R.id.world_mode_value);
        mButtonSetValue = (Button) findViewById(R.id.world_mode_set_value);

        if (is6m()) {
            mRadioLwg.setVisibility(View.GONE);
            mRadioLttg.setVisibility(View.GONE);
        } else {
            mRadioLtctg.setVisibility(View.GONE);
            mRadioLwcg.setVisibility(View.GONE);
        }

        if (true == IS_NR_SUPPORT) {
            mRadioLtctg.setVisibility(View.GONE);
            mRadioLttg.setVisibility(View.GONE);
            mRadioLwg.setVisibility(View.GONE);
            mRadioLttg.setVisibility(View.GONE);
            mRadioLwcg.setVisibility(View.GONE);
            mRadioAuto.setVisibility(View.GONE);
            mButtonSet.setVisibility(View.GONE);
            mTextWorldModeHint.setVisibility(View.GONE);
        }

        IntentFilter intentFilter = new IntentFilter();

        //1.world mode changed by oneself.read the new ID
        intentFilter.addAction(ACTION_WORLD_MODE_CHANGED);

        //2.sim switch lead to world mode change,read the new ID
        intentFilter.addAction(ACTION_SET_RADIO_CAPABILITY_DONE);

        registerReceiver(mBroadcastReceiver, intentFilter);

        //3.when change the world mode by click this ui,ervice status changed, read the new ID
        IntentFilter intentFilterServiceStateChanged = new IntentFilter();

        intentFilterServiceStateChanged.addAction(ACTION_SERVICE_STATE_CHANGED);

        registerReceiver(mBroadcastReceiverServiceStateChanged, intentFilterServiceStateChanged);

        mTelephonyManager = (TelephonyManager) getSystemService(TELEPHONY_SERVICE);
        mTelephonyManager.listen(mPhoneStateListener, PhoneStateListener
                .LISTEN_RADIO_POWER_STATE_CHANGED);

        updateUi();
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (ModemCategory.getCapabilitySim() == PhoneConstants.SIM_ID_1) {
            mTextMainProtocol.setText(String.format("%sSIM1", getResources().getString(R.string
                    .world_mode_main_protocol)));
        } else if (ModemCategory.getCapabilitySim() == PhoneConstants.SIM_ID_2) {
            mTextMainProtocol.setText(String.format("%sSIM2", getResources().getString(R.string
                    .world_mode_main_protocol)));
        }
    }

    @Override
    protected void onDestroy() {
        Elog.d(TAG, "onDestroy()");
        unregisterReceiver(mBroadcastReceiver);
        unregisterReceiver(mBroadcastReceiverServiceStateChanged);
        mResponseHander.removeCallbacksAndMessages(null);
        mTelephonyManager.listen(mPhoneStateListener, PhoneStateListener.LISTEN_NONE);
        super.onDestroy();
    }

    private void updateUi() {
        int mode = getWorldMode();
        int currentMode = WorldModeUtil.get3GDivisionDuplexMode();
        Elog.d(TAG, "MTK Get world mode: " + mode);
        String modeString = "";
        if (true == IS_NR_SUPPORT) {
            modeString = WorldModeUtil.worldModeIdToString(mode);
        } else if ((mode == WORLD_MODE_AUTO) || (mode == C2K_WORLD_MODE_AUTO)) {
            modeString = getString(R.string.world_mode_auto);
            if (is6m()) {
                modeString = modeString + "(" + mWorldModeFor6M[currentMode] + ")";
            } else {
                modeString = modeString + "(" + mWorldMode[currentMode] + ")";
            }
            mRadioAuto.setChecked(true);
        } else if (mode == WORLD_MODE_LTG) {
            modeString = getString(R.string.world_mode_ltg);
            mRadioGroup.check(-1);
        } else if (mode == WORLD_MODE_LWG) {
            modeString = getString(R.string.world_mode_lwg);
            mRadioLwg.setChecked(true);
        } else if (mode == WORLD_MODE_LTTG) {
            modeString = getString(R.string.world_mode_lttg);
            mRadioLttg.setChecked(true);
        } else if (mode == WORLD_MODE_LTCTG) {
            modeString = getString(R.string.world_mode_ltctg);
            mRadioLtctg.setChecked(true);
        } else if (mode == WORLD_MODE_LWCG) {
            modeString = getString(R.string.world_mode_lwcg);
            mRadioLwcg.setChecked(true);
        } else if (mode == WORLD_MODE_LFWG) {
            modeString = getString(R.string.world_mode_lfwg);
            mRadioGroup.check(-1);
        } else {
            modeString = mWorldMode[0];
            mRadioGroup.check(-1);
        }
        mTextCurrentMode.setText(getString(R.string.world_mode_current_value) + modeString);
        mValue.setText(String.valueOf(mode));
        mOldMode = mode;
    }

    @Override
    public void onClick(View v) {
        if (v == mButtonSetValue) {
            int mode = 0;
            try {
                mMode = Integer.parseInt(mValue.getText().toString());
            } catch (NumberFormatException e) {
                Elog.w(TAG, "Invalid format: " + mValue.getText());
                showToast("Invalid value");
            }
        } else if (v == mButtonSet) {
            if (mRadioLwg.isChecked()) {
                mMode = WORLD_MODE_LWG;
            } else if (mRadioLttg.isChecked()) {
                mMode = WORLD_MODE_LTTG;
            } else if (mRadioAuto.isChecked()) {
                if (is6m()) {
                    mMode = C2K_WORLD_MODE_AUTO;
                } else {
                    mMode = WORLD_MODE_AUTO;
                }
            } else if (mRadioLtctg.isChecked()) {
                mMode = WORLD_MODE_LTCTG;
            } else if (mRadioLwcg.isChecked()) {
                mMode = WORLD_MODE_LWCG;
            } else {
                return;
            }
        }
        if (mOldMode != mMode) {
            mBandMode = 0;
            queryCurrentBandModeAndSetNewWorldMode();
        } else {
            Elog.i(TAG, "The mode is not changed!");
        }
    }

    public void startTimer() {
        timer = new CountDownTimer(WAIT_TIME * 1000, 1000) {
            @Override
            public void onTick(long millisUntilFinishied) {
                Elog.d(TAG, "WAIT_TIME = " + WAIT_TIME);
                Elog.d(TAG, "millisUntilFinishied = " + millisUntilFinishied);
            }

            @Override
            public void onFinish() {
                Elog.d(TAG, "millisUntilFinishied finish");
                mProgressDialog.dismiss();
                timer.cancel();
                timer = null;
            }
        };
        timer.start();
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        switch (id) {
            case REBOOT_RF:
                return new AlertDialog.Builder(this).setTitle("Hint")
                        .setMessage("This will cost several seconds,\n" +
                                "UI will hang during reboot RF!")
                        .setPositiveButton("Confirm", new OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                if (which == DialogInterface.BUTTON_POSITIVE) {
                                    int ret = WORLD_MODE_RESULT_ERROR;
                                    int radioState = mTelephonyManager.getRadioPowerState();
                                    if (radioState == TelephonyManager.RADIO_POWER_UNAVAILABLE) {
                                        // radio unavailable
                                    } else {
                                        ret = WorldModeUtil.setWorldModeWithBand(mMode, mBandMode);
                                    }
                                    Elog.i(TAG, "set world mode radioState:" + radioState +
                                            ", ret:" + ret);
                                    if (ret == WORLD_MODE_RESULT_SUCCESS) {
                                        mSetWorldMode = 1;
                                        startTimer();
                                        showDialog(DIALOG_WAIT);
                                    } else if (ret == WORLD_MODE_RESULT_WM_ID_NOT_SUPPORT) {
                                        showDialog(UNSUPPORT);
                                    } else if (ret == WORLD_MODE_RESULT_ERROR) {
                                        showDialog(ERROR);
                                    }
                                }
                                dialog.dismiss();
                            }
                        })
                        .setNegativeButton("Cancel", null)
                        .create();

            case DIALOG_WAIT:
                mProgressDialog = new ProgressDialog(this);
                mProgressDialog.setTitle("Waiting");
                mProgressDialog.setMessage("Waiting rf reboot");
                mProgressDialog.setCancelable(false);
                mProgressDialog.setIndeterminate(true);
                Elog.d(TAG, "After timer.start(");
                return mProgressDialog;
            case UNSUPPORT:
                return new AlertDialog.Builder(this).setTitle("Unsupport")
                        .setMessage("The band is not support the world mode id!")
                        .setPositiveButton("Confirm", null)
                        .create();
            case ERROR:
                return new AlertDialog.Builder(this).setTitle("Error")
                        .setMessage("The world mode is set error!")
                        .setPositiveButton("Confirm", null)
                        .create();
        }
        return super.onCreateDialog(id);
    }

    private void queryCurrentBandModeAndSetNewWorldMode() {
        final String[] modeString = {"AT+EPBSE?", "+EPBSE:"};
        Elog.d(TAG, "AT String:" + modeString[0]);
        sendATCommand(modeString, EVENT_QUERY_CURRENT);
    }

    private void sendATCommand(String[] atCommand, int msg) {
        EmUtils.invokeOemRilRequestStringsEm(atCommand, mResponseHander.obtainMessage(msg));
    }

    private int getWorldMode() {
        return WorldModeUtil.getWorldModeId();
    }

    private void showToast(String msg) {
        if (mToast != null) {
            mToast.cancel();
        }
        mToast = Toast.makeText(this, msg, Toast.LENGTH_SHORT);
        mToast.show();
    }

    private boolean is6m() {
        boolean result = false;
        if (ModemCategory.isCdma()) {
            result = true;
        }
        return result;
    }
}
