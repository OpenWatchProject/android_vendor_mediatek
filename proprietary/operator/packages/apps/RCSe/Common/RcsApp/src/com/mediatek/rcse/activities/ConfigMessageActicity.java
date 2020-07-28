/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
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

package com.mediatek.rcse.activities;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.os.Bundle;
import android.os.CountDownTimer;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.RegistrationApi;
import com.mediatek.rcse.api.terms.TermsApiIntents;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.ApiManager.RcseComponentController;
import com.mediatek.rcse.service.CoreApplication;
import com.mediatek.rcs.R;
//import com.orangelabs.rcs.service.LauncherUtils;
import com.orangelabs.rcs.service.StartService;

import com.gsma.services.rcs.JoynServiceException;

/**
 * This class defined as a transparent activity to display the auto
 * configuration message dialog.
 */
public class ConfigMessageActicity extends Activity {

    private static final String TAG = "ConfigMessageActicity";
    public static final String CONFIG_DIALOG_TITLE = "subject";
    public static final String CONFIG_DIALOG_MESSAGE = "text";
    public static final String CONFIG_DIALOG_ID = "id";
    public static final String CONFIG_DIALOG_PIN = "pin";
    public static final String CONFIG_DIALOG_TIMEOUT = "timeout";
    public static final String CONFIG_DIALOG_TYPE = "type";
    public static final String CONFIG_DIALOG_TYPE_1 = "1";
    public static boolean sPinRequired = false;
    public static long sDialogTimeout;
    public String mType = "";
    public static final String CONFIG_DIALOG_ACCEPT_BUTTON = "accept_button";
    public static final String CONFIG_DIALOG_REJECT_BUTTON = "reject_button";
    private String mTitle = "";
    private String mMessage = "";
    private String mId = null;
    private boolean mAccept = true;
    private boolean mReject = false;
    View mTermsLayout = null;

    @Override
    protected void onCreate(Bundle arg0) {
        super.onCreate(arg0);
        this.setContentView(R.layout.auto_config_layout);
        Intent intent = this.getIntent();
        if (intent != null
                && intent.getAction().equals(TermsApiIntents.TERMS_SIP_REQUEST)) {
            mTitle = intent.getStringExtra(CONFIG_DIALOG_TITLE);
            mMessage = intent.getStringExtra(CONFIG_DIALOG_MESSAGE);
            mAccept = intent.getBooleanExtra(CONFIG_DIALOG_ACCEPT_BUTTON, true);
            mReject = intent.getBooleanExtra(CONFIG_DIALOG_REJECT_BUTTON, true);
            sPinRequired = intent.getBooleanExtra(CONFIG_DIALOG_PIN, true);
            sDialogTimeout = intent.getLongExtra(CONFIG_DIALOG_TIMEOUT, 5000);
            mType = intent.getStringExtra(CONFIG_DIALOG_TYPE);
            Logger.v(TAG, "ConfigMesssage Dioalog before () Type = " + mType);
            mId = intent.getStringExtra(CONFIG_DIALOG_ID);
            final ConfigMessageDialog termsDialog = new ConfigMessageDialog(ConfigMessageActicity.this);
            termsDialog.show(this.getFragmentManager(), TAG);
            if (mType.equals(CONFIG_DIALOG_TYPE_1)) {
                Logger.v(TAG, "ConfigMesssage Dioalog () Type = " + mType);
                new CountDownTimer(sDialogTimeout, 10000) {

                    public void onTick(long millisUntilFinished) {
                    }

                    public void onFinish() {
                        ConfigMessageActicity.this.finish();
                    }
                }.start();
            }
        } else if (intent != null
                && intent.getAction().equals(TermsApiIntents.TERMS_SIP_ACK)) {
            mTitle = intent.getStringExtra(CONFIG_DIALOG_TITLE);
            mMessage = intent.getStringExtra(CONFIG_DIALOG_MESSAGE);
            final AckMessageDialog ackDialog = new AckMessageDialog(ConfigMessageActicity.this);
            ackDialog.show(this.getFragmentManager(), "ACK_DIALOG");

        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    /**
     * Enable rcs service.
     */
    private void enableRcsService() {
        Logger.v(TAG, "enableRcsService()");
        /*LauncherUtils.launchRcsService(this.getApplicationContext(), false,
                false);*/
        
        Intent intent = new Intent();
        intent.setAction(CoreApplication.LAUNCH_SERVICE);
        //intent.putExtra(CORE_CONFIGURATION_STATUS, false);
        getApplicationContext().sendBroadcast(intent);
    }

    /**
     * This class defined to display the auto configuration message to user.
     */
    public static class ConfigMessageDialog extends DialogFragment {
        private ConfigMessageActicity mActivity = null;

        /**
         * Instantiates a new config message dialog.
         */
        public ConfigMessageDialog(ConfigMessageActicity activity) {
            this.mActivity = activity;
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            if (mActivity == null) {
                Logger.w(TAG, "onCreateDialog()-mActivity is null");
                return null;
            }
            Builder dialogBuilder = new AlertDialog.Builder(
                    mActivity, AlertDialog.THEME_HOLO_LIGHT);
            mActivity.mTermsLayout = LayoutInflater.from(mActivity).inflate(
                    R.layout.terms_dialog, null);
            dialogBuilder.setView(mActivity.mTermsLayout);
            TextView subject = (TextView) mActivity.mTermsLayout
                    .findViewById(R.id.message);
            final EditText pin = (EditText) mActivity.mTermsLayout.findViewById(R.id.pin);
            if (mActivity.mMessage != null) {
                subject.setText(mActivity.mMessage);
            } else {
                Logger.w(TAG, "onCreateDialog()-mMessage is null");
            }
            if (mActivity.mTitle != null) {
                dialogBuilder.setTitle(mActivity.mTitle);
            } else {
                Logger.w(TAG, "onCreateDialog()-mTitle is null");
            }
            if (sPinRequired) {
                pin.setVisibility(View.VISIBLE);
            } else {
                pin.setVisibility(View.GONE);
            }
            if (mActivity.mAccept) {
                dialogBuilder.setPositiveButton("Accept",
                        new OnClickListener() {
                            @Override
                            public void onClick(DialogInterface arg0, int arg1) {
                                RcseComponentController rcseComponentController = ApiManager
                                        .getInstance()
                                        .getRcseComponentController();
                                Logger.w(TAG, "onCreateDialog()-accept is true");
                                String userPin = pin.getText().toString();
                                if ((sPinRequired && !userPin.equals(""))
                                        || !sPinRequired) {
                                    if (rcseComponentController != null) {
                                        rcseComponentController
                                                .onServiceActiveStatusChanged(true);
                                    } else {
                                        Logger.e(
                                                TAG,
                                                "StartServiceTask doInBackground()"
                                                        + " ApiManager.getInstance()" +
                                                        ".getRcseComponentController() is null ");
                                    }
                                    RegistrationApi
                                            .setServiceActivationState(true);
                                    mActivity.enableRcsService();
                                    try {
                                        ApiManager.getInstance().getTermsApi()
                                                .acceptTerms(mActivity.mId, userPin);
                                    } catch (JoynServiceException e) {
                                        // TODO Auto-generated catch block
                                        e.printStackTrace();
                                    }
                                    if (mActivity != null){
                                        mActivity.finish();
                                    }
                                } else if (sPinRequired && userPin.equals("")) {
                                    Toast.makeText(mActivity,
                                            "Please enter your pin",
                                            Toast.LENGTH_LONG).show();
                                }
                            }
                        });
            } else {
                Logger.w(TAG, "onCreateDialog()-mAccept is null");
            }
            if (mActivity.mReject) {
                dialogBuilder.setNegativeButton("Reject",
                        new OnClickListener() {
                            @Override
                            public void onClick(DialogInterface arg0, int arg1) {
                                Logger.w(TAG,
                                        "onCreateDialog()-rejected is true");
                                /*LauncherUtils
                                        .resetRcsConfig(getApplicationContext());*/
                                try {
                                    ApiManager.getInstance().getTermsApi()
                                            .rejectTerms(mActivity.mId, null);
                                } catch (JoynServiceException e) {
                                    // TODO Auto-generated catch block
                                    e.printStackTrace();
                                }
                                if (mActivity != null){
                                    mActivity.finish();
                                }
                            }
                        });
            } else {
                Logger.w(TAG, "onCreateDialog()-mReject is null");
            }

            AlertDialog dialog = dialogBuilder.create();
            dialog.setCancelable(false);
            dialog.setCanceledOnTouchOutside(false);
            return dialog;
        }

        @Override
        public void onCancel(DialogInterface dialog) {
            super.onCancel(dialog);
            this.dismissAllowingStateLoss();
            if (mActivity != null){
                mActivity.finish();
            }
        }
    }

    /**
     * This class defined to display the auto configuration message to user.
     */
    public static class AckMessageDialog extends DialogFragment {
        private ConfigMessageActicity mActivity = null;

        /**
         * Instantiates a new ack message dialog.
         */
        public AckMessageDialog(ConfigMessageActicity activity) {
            this.mActivity = activity;
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            if (mActivity == null) {
                Logger.w(TAG, "onCreateDialog()-mActivity is null");
                return null;
            }
            Builder dialogBuilder = new AlertDialog.Builder(
                    mActivity, AlertDialog.THEME_HOLO_LIGHT);
            mActivity.mTermsLayout = LayoutInflater.from(mActivity).inflate(
                    R.layout.terms_dialog, null);
            dialogBuilder.setView(mActivity.mTermsLayout);
            TextView subject = (TextView) mActivity.mTermsLayout
                    .findViewById(R.id.message);
            final EditText pin = (EditText) mActivity.mTermsLayout.findViewById(R.id.pin);
            if (mActivity.mMessage != null) {
                subject.setText(mActivity.mMessage);
            } else {
                Logger.w(TAG, "onCreateDialog()-mMessage is null");
            }
            if (mActivity.mTitle != null) {
                dialogBuilder.setTitle(mActivity.mTitle);
            } else {
                Logger.w(TAG, "onCreateDialog()-mTitle is null");
            }

            pin.setVisibility(View.GONE);
            dialogBuilder.setPositiveButton("OK", new OnClickListener() {
                @Override
                public void onClick(DialogInterface arg0, int arg1) {
                    Logger.w(TAG, "onCreateDialog() AckDialog -OK is true");
                    if (mActivity != null){
                        mActivity.finish();
                    }
                }
            });

            AlertDialog dialog = dialogBuilder.create();
            dialog.setCancelable(false);
            dialog.setCanceledOnTouchOutside(false);
            return dialog;
        }

        @Override
        public void onCancel(DialogInterface dialog) {
            super.onCancel(dialog);
            this.dismissAllowingStateLoss();
            if (mActivity != null){
                mActivity.finish();
            }
        }
    }
}
