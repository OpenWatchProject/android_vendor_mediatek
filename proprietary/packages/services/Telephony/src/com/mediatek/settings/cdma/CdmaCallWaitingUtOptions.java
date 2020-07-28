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
package com.mediatek.settings.cdma;

import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.DialogInterface.OnDismissListener;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.preference.Preference;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.RadioGroup;
import android.widget.TextView;

import com.android.ims.ImsManager;
import com.android.internal.telephony.CommandException;
import com.android.internal.telephony.Phone;
import com.android.phone.PhoneGlobals;
import com.android.phone.R;
import com.android.phone.SubscriptionInfoHelper;
import com.android.phone.TimeConsumingPreferenceActivity;

import com.mediatek.ims.internal.MtkImsManager;
import com.mediatek.internal.telephony.ratconfiguration.RatConfiguration;
import com.mediatek.settings.CallSettingUtils;

import static com.android.phone.TimeConsumingPreferenceActivity.RESPONSE_ERROR;

public class CdmaCallWaitingUtOptions extends TimeConsumingPreferenceActivity implements
        PhoneGlobals.SubInfoUpdateListener {

    private final static String TAG = "CdmaCallWaitingUtOptions";
    private Phone mPhone;
    private SubscriptionInfoHelper mSubscriptionInfoHelper;
    private Preference mDummyPreference;
    private final static String DUMMY_PREFERENCE_KEY = "dummy_preference_key";

    private final MyHandler mHandler = new MyHandler();

    private RadioGroup mRadioGroup;
    private Dialog mDialog;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mSubscriptionInfoHelper = new SubscriptionInfoHelper(this, getIntent());
        mPhone = mSubscriptionInfoHelper.getPhone();
        mDummyPreference = new Preference(this);
        mDummyPreference.setKey(DUMMY_PREFERENCE_KEY);
        PhoneGlobals.getInstance().addSubInfoUpdateListener(this);
    }

    @Override
    public void onResume() {
        super.onResume();
        mDialog = createDialog();
        mDialog.show();
        initForUt();
    }

    private void initForUt() {
        Log.d(TAG, "init...");
        onStarted(mDummyPreference, true);
        mPhone.getCallWaiting(mHandler.obtainMessage(MyHandler.MESSAGE_GET_CALL_WAITING,
                MyHandler.MESSAGE_GET_CALL_WAITING, MyHandler.MESSAGE_GET_CALL_WAITING));
    }

    /**
     * Create the call wait setting dialog.
     *
     * @return the created dialog object.
     */
    public Dialog createDialog() {
        final Dialog dialog = new Dialog(this, R.style.CWDialogTheme);
        dialog.setContentView(R.layout.mtk_cdma_cf_dialog);
        dialog.setTitle(R.string.labelCW);
        dialog.setOnCancelListener(new DialogInterface.OnCancelListener() {
            @Override
            public void onCancel(DialogInterface dialog) {
                Log.d(TAG, "Dialog Cancels, so finish the activity");
                dialog.dismiss();
                finish();
            }
        });

        final RadioGroup radioGroup = (RadioGroup) dialog.findViewById(R.id.group);

        final TextView textView = (TextView) dialog.findViewById(R.id.dialog_sum);
        if (textView != null) {
            textView.setVisibility(View.GONE);
        } else {
            Log.d(TAG, "--------------[text view is null]---------------");
        }

        EditText editText = (EditText) dialog.findViewById(R.id.EditNumber);
        if (editText != null) {
            editText.setVisibility(View.GONE);
        }

        ImageButton addContactBtn = (ImageButton) dialog.findViewById(R.id.select_contact);
        if (addContactBtn != null) {
            addContactBtn.setVisibility(View.GONE);
        }

        Button dialogSaveBtn = (Button) dialog.findViewById(R.id.save);
        if (dialogSaveBtn != null) {
            dialogSaveBtn.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View v) {
                    if (radioGroup.getCheckedRadioButtonId() != -1) {
                        boolean enable = radioGroup.getCheckedRadioButtonId() == R.id.enable;
                        mPhone.setCallWaiting(enable, mHandler
                                .obtainMessage(MyHandler.MESSAGE_SET_CALL_WAITING));
                    }
                    dialog.dismiss();
                    finish();
                }
            });
        }

        Button dialogCancelBtn = (Button) dialog.findViewById(R.id.cancel);
        if (dialogCancelBtn != null) {
                dialogCancelBtn.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View v) {
                    dialog.dismiss();
                    finish();
                }
            });
        }
        mRadioGroup = radioGroup;
        return dialog;
    }

    @Override
    public void handleSubInfoUpdate() {
        Log.d(TAG, "handleSubInfoUpdate...");
        if (mDialog != null) {
            mDialog.dismiss();
        }
        finish();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        PhoneGlobals.getInstance().removeSubInfoUpdateListener(this);
    }

    @Override
    public void onPause() {
        Log.d(TAG, "onPause...");
        super.onPause();
        if (mDialog != null) {
            mDialog.dismiss();
        }
        finish();
    }

    private class MyHandler extends Handler {
        static final int MESSAGE_GET_CALL_WAITING = 0;
        static final int MESSAGE_SET_CALL_WAITING = 1;

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MESSAGE_GET_CALL_WAITING:
                    handleGetCallWaitingResponse(msg);
                    break;
                case MESSAGE_SET_CALL_WAITING:
                    handleSetCallWaitingResponse(msg);
                    break;
            }
        }

        private boolean isUtError(CommandException.Error er) {
            boolean result = false;
            if (RatConfiguration.isC2kSupported()) {
                result = (er == CommandException.Error.OPERATION_NOT_ALLOWED
                        || er == CommandException.Error.OEM_ERROR_2
                        || er == CommandException.Error.OEM_ERROR_3);
            }
            Log.d(TAG, "Has Ut Error: " + result);
            return result;
        }

        private void handleGetCallWaitingResponse(Message msg) {
            AsyncResult ar = (AsyncResult) msg.obj;

            if (msg.arg2 == MESSAGE_SET_CALL_WAITING) {
                onFinished(mDummyPreference, false);
            } else {
                onFinished(mDummyPreference, true);
            }

            if (ar.exception instanceof CommandException) {
                Log.d(TAG, "handleGetCallWaitingResponse: CommandException=" +
                            ar.exception);
                handleCommandException((CommandException) ar.exception);
            } else if (ar.userObj instanceof Throwable || ar.exception != null) {
                // Still an error case but just not a CommandException.
                Log.d(TAG, "handleGetCallWaitingResponse: Exception" + ar.exception);
                if (mDialog != null) {
                    mDialog.dismiss();
                }
                onError(mDummyPreference, RESPONSE_ERROR);
            } else {
                Log.d(TAG, "handleGetCallWaitingResponse: CW state successfully queried.");
                int[] cwArray = (int[]) ar.result;
                // If cwArray[0] is = 1, then cwArray[1] must follow,
                // with the TS 27.007 service class bit vector of services
                // for which call waiting is enabled.
                try {
                    boolean enabled = ((cwArray[0] == 1) && ((cwArray[1] & 0x01) == 0x01));
                    mRadioGroup.check(enabled ? R.id.enable : R.id.disable);
                    Log.d(TAG, "handleGetCallWaitingResponse, enabled: " + enabled
                            + ", cwArray[0]:cwArray[1] = "
                            + cwArray[0] + ":" + cwArray[1]);
                } catch (ArrayIndexOutOfBoundsException e) {
                    Log.e(TAG, "handleGetCallWaitingResponse: improper result: err ="
                            + e.getMessage());
                }
            }
        }

        private void handleSetCallWaitingResponse(Message msg) {
            final AsyncResult ar = (AsyncResult) msg.obj;

            if (ar.exception != null) {
                Log.d(TAG, "handleSetCallWaitingResponse: ar.exception=" + ar.exception);
                //setEnabled(false);
                /// M: reset the checkbox if set fail.
                boolean enabled = mRadioGroup.getCheckedRadioButtonId() == R.id.enable;
                mRadioGroup.check(enabled ? R.id.disable : R.id.enable);
            }
            Log.d(TAG, "handleSetCallWaitingResponse: re get start");
            /// M: modem has limitation that if query result immediately set, will
            //  not get the right result, so we need wait 1s to query. just AP workaround @{
            Runnable runnable = new Runnable() {
                @Override
                public void run() {
                    Log.d(TAG, "handleSetCallWaitingResponse: re get");
                    mPhone.getCallWaiting(obtainMessage(MESSAGE_GET_CALL_WAITING,
                            MESSAGE_SET_CALL_WAITING, MESSAGE_SET_CALL_WAITING, ar.exception));
                }
            };
            postDelayed(runnable, 1000);
            // @}
        }

        private void handleCommandException(CommandException exception) {
            if (isUtError(exception.getCommandError())) {
                Log.d(TAG, "403 received, path to CS...");
                ImsManager imsManager = ImsManager.getInstance(
                    CdmaCallWaitingUtOptions.this, mPhone.getPhoneId());
                if (!CallSettingUtils.isCtVolteMix() && imsManager != null &&
                        imsManager.isEnhanced4gLteModeSettingEnabledByUser() &&
                        (TelephonyUtilsEx.isCapabilityPhone(mPhone) ||
                        MtkImsManager.isSupportMims())) {
                    Log.d(TAG, "volte enabled, show alert...");
                    if (mDialog != null) {
                        mDialog.dismiss();
                    }
                    AlertDialog.Builder b = new AlertDialog.Builder(
                            CdmaCallWaitingUtOptions.this);
                    b.setMessage(R.string.alert_turn_off_volte);
                    b.setCancelable(false);
                    b.setPositiveButton(R.string.alert_dialog_ok,
                            new DialogInterface.OnClickListener() {
                                @Override
                                public void onClick(DialogInterface dialog, int which) {
                                    if (mDialog != null) {
                                        mDialog.dismiss();
                                    }
                                    finish();
                                }
                            });
                    AlertDialog dialog = b.create();
                    // make the dialog more obvious by bluring the
                    // background.
                    dialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_BLUR_BEHIND);
                    if (!isFinishing()) {
                       dialog.show();
                    }
                    mDialog = dialog;
                } else {
                    if (mDialog != null) {
                        mDialog.dismiss();
                    }
                    CdmaCallWaitOptions cwOptions = new CdmaCallWaitOptions(
                            CdmaCallWaitingUtOptions.this, mPhone);
                    mDialog = cwOptions.createDialog();
                    mDialog.setOnDismissListener(new OnDismissListener() {
                        @Override
                        public void onDismiss(DialogInterface dialog) {
                            finish();
                        }
                    });
                    if (!isFinishing()) {
                       mDialog.show();
                    }
                }
            } else {
                if (mDialog != null) {
                    mDialog.dismiss();
                }
                onException(mDummyPreference, exception);
            }
        }
    }
}
