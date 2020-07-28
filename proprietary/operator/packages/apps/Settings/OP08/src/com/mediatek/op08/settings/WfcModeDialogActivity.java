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


package com.mediatek.op08.settings;

import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.telephony.PhoneStateListener;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.View;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.RadioGroup.OnCheckedChangeListener;
import android.widget.TextView;

import com.android.ims.ImsConfig;
import com.android.ims.ImsManager;
import com.android.internal.app.AlertActivity;
import com.android.internal.app.AlertController;
import com.mediatek.ims.internal.MtkImsManager;
import com.mediatek.internal.telephony.RadioCapabilitySwitchUtil;


/**
 * Dialog to be shown on first time wifi enabling.
 */
public class WfcModeDialogActivity extends AlertActivity implements View.OnClickListener {

    private static final String TAG = "Op08WfcModeDialogActivity";

    // For preference dialog
    private RadioButton mWifiPreferredButton;
    private RadioButton mCellularPreferredButton;
    private RadioButton mWifiOnlyButton;

    private PhoneStateListener mPhoneStateListener = new PhoneStateListener() {
        @Override
        public void onCallStateChanged(int state, String incomingNumber) {
            // TODO Auto-generated method stub
            Log.d(TAG, "Phone state:" + state);
            switch (state) {
                case TelephonyManager.CALL_STATE_OFFHOOK:
                case TelephonyManager.CALL_STATE_RINGING:
                    /* Dismiss the alert during call */
                    dismiss();
                    break;
                default:
                    break;
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        createWfcWifiDialog();
    }

    @Override
    public void onResume() {
        super.onResume();
        ((TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE))
                .listen(mPhoneStateListener, PhoneStateListener.LISTEN_CALL_STATE);
    }

    @Override
    public void onPause() {
        super.onPause();
        ((TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE))
                .listen(mPhoneStateListener, PhoneStateListener.LISTEN_NONE);
    }

    private void createWfcWifiDialog() {
        final AlertController.AlertParams p = mAlertParams;
        p.mTitle = getString(R.string.connection_preference);
        p.mView = getLayoutInflater().inflate(R.layout.wfc_preference_dlg_layout, null);
        p.mNegativeButtonText = getString(R.string.cancel);
        p.mNegativeButtonListener = new OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                switch (which) {
                case DialogInterface.BUTTON_NEGATIVE:
                    Log.d("@M_" + TAG, "negative clicked, activity finish");
                default:
                    dialog.dismiss();
                    break;
                }
            }
        };
        p.mCancelable = true;
        handleRadioGroup(p.mView);
        setupAlert();
    }

    @Override
    public void onBackPressed() {
        try {
            super.onBackPressed();
        } catch (java.lang.IllegalStateException e) {
            Log.e("@M_" + TAG, e.getMessage());
        }
    }

    @Override
    public void onClick(View v) {
        Log.d("@M_" + TAG, "in View onClick");
        /* Note here, radioGroup.check(radioButtonId) is not being used to check/uncheck the
               * buttons because Android calls thrice the onCheckedChanged listener on setting
               * buttons with this method. Instead RadioButton.setChecked(true) is being used,
               * as it calls listener only once.
               */
        if (v instanceof TextView) {
            switch (v.getId()) {
            case R.id.summary_wifi_preferred:
                Log.d("@M_" + TAG, "first textview pressed");
                mWifiPreferredButton.setChecked(true);
                break;
            case R.id.summary_cellular_network_preferred:
                Log.d("@M_" + TAG, "second textview pressed");
                mCellularPreferredButton.setChecked(true);
                break;
            case R.id.summary_never_use_cellular_network:
                Log.d("@M_" + TAG, "third textview pressed");
                mWifiOnlyButton.setChecked(true);
                break;
            default:
                Log.d("@M_" + TAG, "case in default");
                break;
            }
        }
    }

    private void handleRadioGroup(View dlgView) {
        RadioGroup wfcPreferenceRadioGroup = (RadioGroup) dlgView
                .findViewById(R.id.wfc_preference_button_group);
        TextView wifiPreferredSummary = (TextView) dlgView
                .findViewById(R.id.summary_wifi_preferred);
        wifiPreferredSummary.setOnClickListener(this);
        TextView cellularPreferredSummary = (TextView) dlgView
                .findViewById(R.id.summary_cellular_network_preferred);
        cellularPreferredSummary.setOnClickListener(this);
        TextView wifiOnlySummary = (TextView) dlgView
                .findViewById(R.id.summary_never_use_cellular_network);
        wifiOnlySummary.setOnClickListener(this);
        mWifiPreferredButton = (RadioButton) dlgView.findViewById(R.id.wifi_preferred);
        mCellularPreferredButton = (RadioButton) dlgView
                .findViewById(R.id.cellular_network_preferred);
        mWifiOnlyButton = (RadioButton) dlgView.findViewById(R.id.never_use_cellular_network);
        setModeButtonChecked(ImsManager.getWfcMode(this));
        wfcPreferenceRadioGroup.setOnCheckedChangeListener(new OnCheckedChangeListener() {

            @Override
            public void onCheckedChanged(RadioGroup group, int checkedId) {
                // TODO Auto-generated method stub
                Log.d("@M_" + TAG, "in checkChangeListener, checkedId:" + checkedId);
                int selected = ImsManager.getWfcMode(WfcModeDialogActivity.this);
                switch (checkedId) {
                    case R.id.wifi_preferred:
                        Log.d("@M_" + TAG, "first button clicked");
                        selected = ImsConfig.WfcModeFeatureValueConstants.WIFI_PREFERRED;
                        break;
                    case R.id.cellular_network_preferred:
                        Log.d("@M_" + TAG, "second button clicked");
                        selected = ImsConfig.WfcModeFeatureValueConstants.CELLULAR_PREFERRED;
                        break;
                    case R.id.never_use_cellular_network:
                        Log.d("@M_" + TAG, "third button clicked");
                        selected = ImsConfig.WfcModeFeatureValueConstants.WIFI_ONLY;
                        break;
                    default:
                        Log.d("@M_" + TAG, "in default:" + checkedId);
                        break;
                }
                handleWfcPreferenceChange(selected);
                Log.d("@M_" + TAG, "Preference selection done return back screen");
                WfcModeDialogActivity.this.dismiss();
            }
        });
    }

    private void handleWfcPreferenceChange(int wfcPreferenceSelected) {
        int phoneId = RadioCapabilitySwitchUtil.getMainCapabilityPhoneId();
        int currentWfcMode = ImsManager.getInstance(this, phoneId).getWfcMode(false);
        if (wfcPreferenceSelected != currentWfcMode) {
            ImsManager.getInstance(this, phoneId).setWfcMode(wfcPreferenceSelected, false);
        }
        int currentWfcRoamingMode = ImsManager.getInstance(this, phoneId).getWfcMode(true);
        if (wfcPreferenceSelected != currentWfcRoamingMode) {
            ImsManager.getInstance(this, phoneId).setWfcMode(wfcPreferenceSelected, true);
        }
    }

    private void setModeButtonChecked(int wfcMode) {
        Log.d("@M_" + TAG, "in setModeButtonChecked, mode:" + wfcMode);
        switch (wfcMode) {
            case ImsConfig.WfcModeFeatureValueConstants.CELLULAR_PREFERRED:
                mCellularPreferredButton.setChecked(true);
                break;

            case ImsConfig.WfcModeFeatureValueConstants.WIFI_ONLY:
                mWifiOnlyButton.setChecked(true);
                break;

            case ImsConfig.WfcModeFeatureValueConstants.WIFI_PREFERRED:
            default:
                mWifiPreferredButton.setChecked(true);
                break;
        }
    }
}
