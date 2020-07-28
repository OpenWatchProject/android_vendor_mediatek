/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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

package com.android.settings.network.telephony;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.os.AsyncResult;
import android.os.AsyncTask;
import android.os.Message;
import android.telephony.PhoneStateListener;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;
import androidx.preference.Preference;
import androidx.preference.PreferenceScreen;
import androidx.preference.SwitchPreference;

import com.android.settings.R;
import com.android.settingslib.core.lifecycle.LifecycleObserver;
import com.android.settingslib.core.lifecycle.events.OnStart;
import com.android.settingslib.core.lifecycle.events.OnStop;

import com.mediatek.internal.telephony.ratconfiguration.RatConfiguration;
import com.mediatek.settings.UtilsExt;
import com.mediatek.settings.sim.TelephonyUtils;

import com.mediatek.telephony.MtkTelephonyManagerEx;

public class Disable2gPreferenceController extends TelephonyTogglePreferenceController
        implements LifecycleObserver, OnStart, OnStop{
    private static final String TAG = "Disable2gPreferenceController";
    private SwitchPreference mPreference;
    private PreferenceScreen mPreferenceScreen;
    private boolean mIsDisable2G = false;
    private boolean mIsGetDisable2GState = false;
    private boolean mIsSetDisable2GState = false;
    private AlertDialog mDialog;
    private int mPhoneId;
    private TelephonyManager mTelephonyManager;
    private MtkTelephonyManagerEx mTelephonyManagerEx;

    private boolean mListeningPhoneState = false;

    public Disable2gPreferenceController(Context context, String key) {
        super(context, key);
    }

    @Override
    public int getAvailabilityStatus(int subId) {
        boolean isLteSupported = RatConfiguration.isLteFddSupported()
            || RatConfiguration.isLteTddSupported();
        if (isLteSupported) {
            boolean customerATTDisable2Gvisible
                    = UtilsExt.getMobileNetworkSettingsExt(mContext)
                            .customizeATTDisable2Gvisible(subId);
            return customerATTDisable2Gvisible ? AVAILABLE : CONDITIONALLY_UNAVAILABLE;
        } else {
            return CONDITIONALLY_UNAVAILABLE;  
        }
    }

    @Override
    public void displayPreference(PreferenceScreen screen) {
        super.displayPreference(screen);
        mPreferenceScreen = screen;
        mPreference = (SwitchPreference)screen.findPreference(getPreferenceKey());
    }

    public void init(int subId) {
        mSubId = subId;
        mPhoneId = SubscriptionManager.getSlotIndex(subId);
        mTelephonyManager = TelephonyManager.from(mContext).createForSubscriptionId(mSubId);
        mTelephonyManagerEx = MtkTelephonyManagerEx.getDefault();
        int visible = getAvailabilityStatus(subId);
        Log.i(TAG, "init, visible="+ visible);
        if (visible != AVAILABLE) {
            return;
        }
        ///need queryDisable2G or not
        mIsGetDisable2GState = true;
        new AsyncTask<Void, Void, Integer>() {
            @Override
            protected Integer doInBackground(Void... voids) {
                Log.d(TAG, "getDisable2G.doInBackground, phoneId=" + mPhoneId);
                int mode = mTelephonyManagerEx.getDisable2G(mPhoneId);
                Log.d(TAG, "getDisable2G.doInBackground, phoneId=" + mPhoneId + ", mode=" + mode);
                return mode;
            }

            @Override
            protected void onPostExecute(Integer result) {
                Log.d(TAG, "getDisable2G.onPostExecute, result=" + result);
                if (result == 0) {
                    mIsDisable2G = false;
                } else if (result == 1) {
                    mIsDisable2G = true;
                }
                mIsGetDisable2GState = false;
                updateState(mPreference);
            }
        }.execute();
    }

    @Override
    public void updateState(Preference preference) {
        SwitchPreference switchPreference = (SwitchPreference)preference;
        switchPreference.setChecked(mIsDisable2G);
        switchPreference.setEnabled(getDisable2GEnabledState());
    }

    private boolean getDisable2GEnabledState() {
        boolean inCall = TelephonyUtils.isInCall();
        Log.d(TAG, "getDisable2GEnabledState, inCall=" + inCall
                     + ", getState=" + mIsGetDisable2GState
                     + ", setState=" + mIsSetDisable2GState);
        return !inCall && !mIsGetDisable2GState && !mIsSetDisable2GState;
    }

    @Override
    public boolean handlePreferenceTreeClick(Preference preference) {
        if (getPreferenceKey().equals(preference.getKey())) {
            boolean isCheck = isChecked();
            Log.i(TAG, "handlePreferenceTreeClick, isCheck=" + isCheck);
            if (!isCheck) {
                Log.d(TAG, "handlePreferenceTreeClick, show alert dialog for disable 2G.");
                AlertDialog.Builder builder =
                        new AlertDialog.Builder(mContext);
                builder.setTitle(
                        mContext.getString(R.string.disable_2g_Alert_title_att));
                builder.setMessage(
                        mContext.getString(R.string.disable_2g_menu_on_alert_att));
                builder.setOnCancelListener(new DialogInterface.OnCancelListener() {
                    @Override
                    public void onCancel(DialogInterface dialog) {
                        Log.d(TAG, "onCancel");
                    }
                });
                builder.setOnDismissListener(new DialogInterface.OnDismissListener () {
                    @Override
                    public void onDismiss(DialogInterface dialog) {
                        Log.d(TAG, "onDismiss");
                    }
                });
                builder.setPositiveButton(
                        mContext.getString(R.string.disable_2g_alert_ok_att),
                        new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                Log.d(TAG, "onClick, OK.");
                                handleSwitchAction(true);
                                mPreference.setChecked(true);
                            }
                        });
                builder.setNegativeButton(
                        mContext.getString(R.string.disable_2g_alert_cancel_att),
                        new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                Log.d(TAG, "onClick, Cancel.");
//                                mPreference.setChecked(false);
                            }
                        });
                mDialog = builder.show();
            } else {
                Log.d(TAG, "handlePreferenceTreeClick, trun off disable 2G.");
                handleSwitchAction(false);
            }
            return true;
        }
        return false;
    }

    @Override
    public boolean setChecked(boolean isChecked) {
       return false;
    }

    @Override
    public boolean isChecked() {
        return mPreference.isChecked();
    }

    private boolean isSettingOn() {
        Log.d(TAG, "isSettingOn, isDisable2G=" + mIsDisable2G);
        return mIsDisable2G;
    }

    private void handleSwitchAction(final boolean switchValue) {
        Log.d(TAG, "handleSwitchAction, switchValue=" + switchValue);
        mIsSetDisable2GState = true;
        new AsyncTask<Void, Void, Boolean>() {
            @Override
            protected Boolean doInBackground(Void... voids) {
                Log.d(TAG, "setDisable2G.doInBackground, phoneId=" + mPhoneId
                        + ", switchValue=" + switchValue);
                boolean isSuccess = mTelephonyManagerEx.setDisable2G(mPhoneId, switchValue);
                if (isSuccess) {
                    mIsDisable2G = switchValue;
                }
                Log.d(TAG, "setDisable2G.doInBackground, phoneId=" + mPhoneId
                        + ", result=" + isSuccess + ", isDisable2G=" + mIsDisable2G);
                return isSuccess;
            }

            @Override
            protected void onPostExecute(Boolean result) {
                Log.d(TAG, "setDisable2G.onPostExecute, result=" + result);
                mIsSetDisable2GState = false;
                updateState(mPreference);
            }
        }.execute();
    }

    private PhoneStateListener mPhoneStateListener = new PhoneStateListener() {
        @Override
        public void onCallStateChanged(int state, String incomingNumber) {
            Log.d(TAG, "onCallStateChanged, state=" + state);

            mPreference.setEnabled(!isHandleDisable2G()
                    && state == TelephonyManager.CALL_STATE_IDLE);
            mPreference.setChecked(isSettingOn());

            if (state == TelephonyManager.CALL_STATE_RINGING
                    || state == TelephonyManager.CALL_STATE_OFFHOOK) {
                if (mDialog != null && mDialog.isShowing()) {
                    mDialog.dismiss();
                }
            }
        }
    };

    private boolean isHandleDisable2G() {
        Log.d(TAG, "isHandleDisable2G, getState=" + mIsGetDisable2GState
                + ", setState=" + mIsSetDisable2GState);
        return mIsGetDisable2GState || mIsSetDisable2GState;
    }

    @Override
    public void onStart() {
        mTelephonyManager.listen(mPhoneStateListener,
                PhoneStateListener.LISTEN_CALL_STATE);
        mListeningPhoneState = true;
    }

    @Override
    public void onStop() {
        if (mListeningPhoneState) {
            mTelephonyManager.listen(mPhoneStateListener, PhoneStateListener.LISTEN_NONE);
            mListeningPhoneState = false;
        }
    }
}

