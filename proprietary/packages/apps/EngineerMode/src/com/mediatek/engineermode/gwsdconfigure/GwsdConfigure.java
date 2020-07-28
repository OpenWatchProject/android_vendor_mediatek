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

package com.mediatek.engineermode.gwsdconfigure;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.SharedPreferences;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.EditText;
import android.widget.RelativeLayout;
import android.widget.Switch;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneFactory;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.ModemCategory;
import com.mediatek.engineermode.R;

import com.mediatek.gwsd.GwsdStatus;
import com.mediatek.gwsd.GwsdListener;
import com.mediatek.telephony.MtkTelephonyManagerEx;


public class GwsdConfigure extends Activity implements OnCheckedChangeListener{
    private static final String TAG = "GwsdConfigure";

    //UI Related
    Switch mCapSwitch, mSilenceSwitch, mDualSimSwitch;
    private ProgressDialog mProDialog;
    EditText editCallValid, editIgnoreNumber;
    Button btnCallValid, btnIngoreNumber;

    //Message definition
    private final int MSG_SWITCH_CAPABILITY = 0;
    private final int MSG_SILENCE_MODE = 1;
    private final int MSG_STATUS_CHANGED_START = 2;
    private final int MSG_STATUS_CHANGED_END = 3;
    private final int MSG_SET_CALL_VALID_TIMER = 4;
    private final int MSG_IGNORE_SAME_CALL_INTERVAL = 5;
    private final int MSG_STATUS_MODEM_RESET = 6;

    //SharePreference
    SharedPreferences mPref;
    SharedPreferences.Editor mPrefEditor;

    String mPrefCapabiltiy = "gwsd_capability";
    String mPrefSilence = "gwsd_silence";
    String mPrefDualCapabiltiy = "gwsd_dual_capability";
    String mPrefCallValid = "gwsd_call_valid";
    String mPrefIgnoreInterval = "gwsd_ignore_interval";
    boolean mLastCapStatus, mLastSilenceStatus, mLastDualCapStatus;
    int mLastCallValid, mLastIgnoreInterval;

    private MtkTelephonyManagerEx mMtkTelephonyManagerEx;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.setContentView(R.layout.gaming_gwsd);

        mPref = getSharedPreferences(TAG, Context.MODE_PRIVATE);
        mPrefEditor = mPref.edit();

        mLastCapStatus = mPref.getBoolean(mPrefCapabiltiy, false);
        mCapSwitch = (Switch)findViewById(R.id.gwsd_switch);
        mCapSwitch.setChecked(mLastCapStatus);
        mCapSwitch.setOnCheckedChangeListener(this);

        mLastSilenceStatus = mPref.getBoolean(mPrefSilence, false);
        mSilenceSwitch = (Switch)findViewById(R.id.gwsd_auto_reject_switch);
        mSilenceSwitch.setChecked(mLastSilenceStatus);
        mSilenceSwitch.setOnCheckedChangeListener(this);

        mLastDualCapStatus = mPref.getBoolean(mPrefDualCapabiltiy, false);
        mDualSimSwitch = (Switch)findViewById(R.id.gwsd_dual_sim_switch);
        mDualSimSwitch.setChecked(mLastDualCapStatus);
        mDualSimSwitch.setOnCheckedChangeListener(this);

        if (mLastCapStatus == false) {
            mSilenceSwitch.setEnabled(false);
        }

        mProDialog = new ProgressDialog(this);
        mProDialog.setTitle(TAG);
        mProDialog.setMessage(getString(R.string.gwsd_processing));
        mProDialog.setCancelable(false);
        mProDialog.setIndeterminate(true);

        mMtkTelephonyManagerEx = MtkTelephonyManagerEx.getDefault();
        mMtkTelephonyManagerEx.removeGwsdListener();
        mMtkTelephonyManagerEx.addGwsdListener(mGwsdListenerImpl);
        mMtkTelephonyManagerEx.syncGwsdInfo(mLastCapStatus, mLastSilenceStatus);

        editCallValid = (EditText)findViewById(R.id.gwsd_call_valid_edit);
        mLastCallValid = mPref.getInt(mPrefCallValid, 7);
        editCallValid.setText(String.valueOf(mLastCallValid));
        btnCallValid = (Button)findViewById(R.id.gwsd_call_valid_btn);
        btnCallValid.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Elog.d(TAG, "setCallValidTimer, timer: " + editCallValid.getText().toString());
                mPrefEditor.putInt(mPrefCallValid,
                        Integer.valueOf(editCallValid.getText().toString()));
                mPrefEditor.commit();
                mMtkTelephonyManagerEx.setCallValidTimer(
                        Integer.valueOf(editCallValid.getText().toString()));
            }
        });

        editIgnoreNumber = (EditText)findViewById(R.id.gwsd_ignore_same_number_edit);
        mLastIgnoreInterval = mPref.getInt(mPrefIgnoreInterval, 3);
        editIgnoreNumber.setText(String.valueOf(mLastIgnoreInterval));
        btnIngoreNumber = (Button)findViewById(R.id.gwsd_ignore_same_number_btn);
        btnIngoreNumber.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mPrefEditor.putInt(mPrefIgnoreInterval, Integer.valueOf(
                        editIgnoreNumber.getText().toString()));
                mPrefEditor.commit();
                Elog.d(TAG, "setIgnoreSameNumberInterval, interval: "
                        + editIgnoreNumber.getText().toString());
                mMtkTelephonyManagerEx.setIgnoreSameNumberInterval(
                        Integer.valueOf(editIgnoreNumber.getText().toString()));
            }
        });
    }

    @Override
    public void onCheckedChanged(CompoundButton view, boolean isChecked) {
        String[] cmds = new String[2];

        if (view == mCapSwitch) {
            Elog.d(TAG, "setGwsdEnabled, action: " + isChecked);
            mMtkTelephonyManagerEx.setGwsdEnabled(isChecked);
            mProDialog.show();
        } else if (view == mSilenceSwitch) {
            Elog.d(TAG, "setGwsdAutoRejectEnabled, action: " + isChecked);
            mMtkTelephonyManagerEx.setGwsdAutoRejectEnabled(isChecked);
            mProDialog.show();
        } else if (view == mDualSimSwitch) {
            Elog.d(TAG, "setGwsdDualSim, action: " + isChecked);
            if (isChecked == true) {
                Elog.d(TAG, "setGwsdDualSim, mode 2");
                mPrefEditor.putBoolean(mPrefDualCapabiltiy, true);
                mPrefEditor.commit();
                mMtkTelephonyManagerEx.setGwsdDualSimEnabled(true);
            } else if (isChecked == false) {
                Elog.d(TAG, "setGwsdDualSim, mode 0");
                mPrefEditor.putBoolean(mPrefDualCapabiltiy, false);
                mPrefEditor.commit();
                mMtkTelephonyManagerEx.setGwsdDualSimEnabled(false);
            }
        }
    }

    private GwsdListenerImpl mGwsdListenerImpl  = new GwsdListenerImpl();
    public class GwsdListenerImpl extends GwsdListener {
        @Override
        public void onAddListenered(int status, String reason) {
            Elog.d(TAG, "onAddListenered, status: " + status + " reason: " + reason);
        }

        @Override
        public void onUserSelectionModeChanged(int status, String reason) {
            Elog.d(TAG, "onUserSelectionModeChanged, status: " + status + " reason: " + reason);
            mProDialog.dismiss();

            Message msg = mHander.obtainMessage(MSG_SWITCH_CAPABILITY);
            msg.arg1 = status;
            mHander.sendMessage (msg);
        }

        @Override
        public void onAutoRejectModeChanged(int status, String reason) {
            Elog.d(TAG, "onAutoRejectModeChanged, status: " + status + " reason: " + reason);
            mProDialog.dismiss();

            Message msg = mHander.obtainMessage(MSG_SILENCE_MODE);
            msg.arg1 = status;
            mHander.sendMessage (msg);
        }

        @Override
        public void onCallValidTimerChanged(int status, String reason) {
            Elog.d(TAG, "onCallValidTimerChanged, status: " + status + " reason: " + reason);
            Message msg = mHander.obtainMessage(MSG_SET_CALL_VALID_TIMER);
            msg.arg1 = status;
            mHander.sendMessage (msg);
        }

        @Override
        public void onIgnoreSameNumberIntervalChanged(int status, String reason) {
            Elog.d(TAG, "onIgnoreSameNumberIntervalChanged, status: "
                    + status + " reason: " + reason);
            Message msg = mHander.obtainMessage(MSG_IGNORE_SAME_CALL_INTERVAL);
            msg.arg1 = status;
            mHander.sendMessage (msg);
        }

        @Override
        public void onSyncGwsdInfoFinished(int status, String reason) {
            Elog.d(TAG, "onSyncInfoToServiceFinished, status: " + status + " reason: " + reason);
        }

        @Override
        public void onSystemStateChanged(int state) {
            Message msg;
            switch (state) {
                case GwsdStatus.SYSTEM_STATE_DEFAULT_DATA_SWITCHED_START:
                    Elog.d(TAG, "onSystemStateChanged: start for default data switched");
                    msg = mHander.obtainMessage(MSG_STATUS_CHANGED_START);
                    mHander.sendMessage (msg);
                    break;
                case GwsdStatus.SYSTEM_STATE_DEFAULT_DATA_SWITCHED_END:
                    Elog.d(TAG, "onSystemStateChanged: end for start default data switched");
                    msg = mHander.obtainMessage(MSG_STATUS_CHANGED_END);
                    mHander.sendMessage (msg);
                    break;
                case GwsdStatus.SYSTEM_STATE_MODEM_RESET:
                    Elog.d(TAG, "onSystemStateChanged: modem crash");
                    msg = mHander.obtainMessage(MSG_STATUS_MODEM_RESET);
                    mHander.sendMessage (msg);
                    break;
            }
        }
    }

    private Handler mHander = new Handler() {
        @Override
        public void handleMessage(final Message msg) {
            AsyncResult asyncResult = (AsyncResult) msg.obj;
            switch (msg.what) {
                case MSG_SWITCH_CAPABILITY:
                    if (msg.arg1 == GwsdStatus.STATUS_SUCCESS) {
                        EmUtils.showToast("AT cmd success.");
                        if (mLastCapStatus == true) {
                            mLastCapStatus = false;
                            mPrefEditor.putBoolean(mPrefCapabiltiy, false);

                            mLastSilenceStatus = false;
                            mPrefEditor.putBoolean(mPrefSilence, false);
                            mSilenceSwitch.setOnCheckedChangeListener(null);
                            mSilenceSwitch.setChecked(false);
                            mSilenceSwitch.setOnCheckedChangeListener(GwsdConfigure.this);
                            mSilenceSwitch.setEnabled(false);
                        } else {
                            mLastCapStatus = true;
                            mPrefEditor.putBoolean(mPrefCapabiltiy, true);
                            mSilenceSwitch.setEnabled(true);
                        }
                        mPrefEditor.commit();
                    } else {
                        EmUtils.showToast("AT cmd fail.");
                        //changed capability fail, reset status of switch
                        if (mLastCapStatus == true) {
                            mCapSwitch.setOnCheckedChangeListener(null);
                            mCapSwitch.setChecked(true);
                            mCapSwitch.setOnCheckedChangeListener(GwsdConfigure.this);
                        } else {
                            mCapSwitch.setOnCheckedChangeListener(null);
                            mCapSwitch.setChecked(false);
                            mCapSwitch.setOnCheckedChangeListener(GwsdConfigure.this);
                        }
                    }
                    break;
                case MSG_SILENCE_MODE:
                    if (msg.arg1 == GwsdStatus.STATUS_SUCCESS) {
                        EmUtils.showToast("AT cmd success.");
                        if (mLastSilenceStatus == true) {
                            mLastSilenceStatus = false;
                            mPrefEditor.putBoolean(mPrefSilence, false);
                        } else {
                            mLastSilenceStatus = true;
                             mPrefEditor.putBoolean(mPrefSilence, true);
                        }
                        mPrefEditor.commit();
                    } else {
                        EmUtils.showToast("AT cmd fail.");
                        //changed silence fail, reset status of switch
                        if (mLastSilenceStatus == true) {
                            mSilenceSwitch.setOnCheckedChangeListener(null);
                            mSilenceSwitch.setChecked(true);
                            mSilenceSwitch.setOnCheckedChangeListener(GwsdConfigure.this);
                        } else {
                            mSilenceSwitch.setOnCheckedChangeListener(null);
                            mSilenceSwitch.setChecked(false);
                            mSilenceSwitch.setOnCheckedChangeListener(GwsdConfigure.this);
                        }
                    }
                    break;
                case MSG_STATUS_CHANGED_START:
                    mCapSwitch.setEnabled(false);
                    mSilenceSwitch.setEnabled(false);
                    break;
                case MSG_STATUS_CHANGED_END:
                    mCapSwitch.setEnabled(true);

                    mCapSwitch.setOnCheckedChangeListener(null);
                    mCapSwitch.setChecked(false);
                    mCapSwitch.setOnCheckedChangeListener(GwsdConfigure.this);
                    mLastCapStatus = false;
                    mPrefEditor.putBoolean(mPrefCapabiltiy, false);

                    mSilenceSwitch.setOnCheckedChangeListener(null);
                    mSilenceSwitch.setChecked(false);
                    mSilenceSwitch.setOnCheckedChangeListener(GwsdConfigure.this);
                    mLastSilenceStatus = false;
                    mPrefEditor.putBoolean(mPrefSilence, false);

                    mPrefEditor.commit();
                    break;
                case MSG_SET_CALL_VALID_TIMER:
                case MSG_IGNORE_SAME_CALL_INTERVAL:
                    if (msg.arg1 == GwsdStatus.STATUS_SUCCESS) {
                        EmUtils.showToast("configuration success");
                    } else {
                        EmUtils.showToast("configuration fail");
                    }
                    break;
                case MSG_STATUS_MODEM_RESET:
                    mDualSimSwitch.setChecked(false);
                    mPrefEditor.putBoolean(mPrefDualCapabiltiy, false);
                    mPrefEditor.commit();
                    Elog.d(TAG, "Reset mPrefDualCapabiltiy done");
                    break;
                default:
                    break;
            }
        }
    };
}
