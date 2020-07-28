/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.settings.network.telephony.gsm;

import java.util.ArrayList;
import java.util.List;

import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.AsyncResult;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;
import android.widget.Toast;

import androidx.preference.Preference;
import androidx.preference.PreferenceScreen;


import com.android.settings.R;
import com.android.settings.network.telephony.TelephonyBasePreferenceController;
import com.android.settingslib.core.lifecycle.LifecycleObserver;

import com.mediatek.internal.telephony.FemtoCellInfo;
import com.mediatek.settings.UtilsExt;
import com.mediatek.telephony.MtkTelephonyManagerEx;

/**
 * Preference controller for "Open network select"
 */
public class CustomizeATTManualFemtoCellSelectionPreferenceController
        extends TelephonyBasePreferenceController
        implements LifecycleObserver {

    private static final String TAG = "CustomizeATTManualFemtoCellSelectionPreferenceController";

    private TelephonyManager mTelephonyManager;
    private Preference mPreference;
    private static final String LOG_TAG = "CustomizeATTManualFemtoCellSelectionPreferenceController";

    private final int FEMTO_SELECT_STATE_NONE = 0;
    private final int FEMTO_SELECT_STATE_FETCHING = 1;
    private final int FEMTO_SELECT_STATE_SELECTING = 2;
    private final int FEMTO_SELECT_STATE_DONE = 3;
    private final int FEMTO_SELECT_STATE_FAILED = 4;

    private int mPhoneId;
    private ProgressDialog mDialog = null;
    private ArrayList<FemtoCellInfo> mFemtoList = null;
    private int mFemtoCount = 0;
    private int mFemtoIndex = 0;
    private int mFemtoSelectState = FEMTO_SELECT_STATE_NONE;
    private MtkTelephonyManagerEx mTelephonyManagerEx;


    public CustomizeATTManualFemtoCellSelectionPreferenceController(Context context, String key) {
        super(context, key);
    }

    public CustomizeATTManualFemtoCellSelectionPreferenceController init(int subId) {
        mSubId = subId;
        mPhoneId = SubscriptionManager.getSlotIndex(subId);
        mTelephonyManager = TelephonyManager.from(mContext).createForSubscriptionId(mSubId);
        mTelephonyManagerEx = MtkTelephonyManagerEx.getDefault();
        return this;
    }

    @Override
    public int getAvailabilityStatus(int subId) {
        boolean visible;
        if (subId == SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
            Log.d(TAG, "getAvailabilityStatus, subId is invalid.");
            visible = false;
        } else {
            visible = UtilsExt.getMobileNetworkSettingsExt(mContext)
                    .customizeATTManualFemtoCellSelectionPreference(subId);
        }
        return visible ? AVAILABLE : CONDITIONALLY_UNAVAILABLE;
    }

    @Override
    public void displayPreference(PreferenceScreen screen) {
        super.displayPreference(screen);
        mPreference = screen.findPreference(getPreferenceKey());
    }

    @Override
    public void updateState(Preference preference) {
        super.updateState(preference);
    }

    @Override
    public boolean handlePreferenceTreeClick(Preference preference) {
        if (TextUtils.equals(preference.getKey(), getPreferenceKey())) {
            if (mFemtoSelectState == FEMTO_SELECT_STATE_NONE) {
                Log.d(TAG, "onClick");
                showProgressDialog();
                mFemtoSelectState = FEMTO_SELECT_STATE_FETCHING;
                new AsyncTask<Void, Void, List<FemtoCellInfo>>() {
                    @Override
                    protected List<FemtoCellInfo> doInBackground(Void... voids) {
                        Log.d(TAG, "getFemtoCellList doInBackground  phoneId = " + mPhoneId);
                        List<FemtoCellInfo> listFemtoCellInfo = mTelephonyManagerEx.getFemtoCellList(mPhoneId);
                        return listFemtoCellInfo;
                    }

                    @Override
                    protected void onPostExecute(List<FemtoCellInfo> result) {
                        Log.d(TAG, "onPostExecute getFemtoCellList = " + result);
                        if (null == result) {
                            Log.d(TAG, "handleGetFemtocellList, result= null");
                            showFailToast();
                        } else {
                            mFemtoList = (ArrayList)result;
                            mFemtoCount = mFemtoList != null ? mFemtoList.size() : 0;
                            mFemtoIndex = 0;
                            Log.d(TAG, "handleGetFemtocellList, femtoCount=" + mFemtoCount);
                            if (mFemtoCount > 0) {
                                mFemtoSelectState = FEMTO_SELECT_STATE_SELECTING;
                                startFemtocellSelection();
                            } else {
                                showFailToast();
                            }
                        }
                    }
                }.execute();
            } else {
                Log.d(TAG, "onClick is continuous, femtoSelectState="
                        + mFemtoSelectState);
            }
            return true;
        }
        return false;
    }

    private void startFemtocellSelection() {
        Log.d(LOG_TAG, "startFemtocellSelection, femtoSelectState="
                + mFemtoSelectState + ", femtoIndex=" + mFemtoIndex);
        if (mFemtoSelectState == FEMTO_SELECT_STATE_SELECTING) {
            if (mFemtoIndex >= mFemtoCount) {
                showFailToast();
                return;
            }

            final FemtoCellInfo femtoCell = mFemtoList.get(mFemtoIndex);
            new AsyncTask<Void, Void, Boolean>() {
                @Override
                protected Boolean doInBackground(Void... voids) {
                    Log.d(TAG, "selectFemtoCell doInBackground  phoneId = " + mPhoneId);
                    Boolean isSucess = mTelephonyManagerEx.selectFemtoCell(mPhoneId, femtoCell);
                    return isSucess;
                }

                @Override
                protected void onPostExecute(Boolean result) {
                    Log.d(TAG, "onPostExecute selectFemtoCell = " + result);
                    if (result) {
                        Log.d(TAG, "handleSelectFemtoCell sussess, femtoIndex=" + mFemtoIndex);
                        showPassToast();
                    } else {
                        Log.d(TAG, "handleSelectFemtoCell, femtoIndex=" + mFemtoIndex);
                        mFemtoIndex++;
                        startFemtocellSelection();
                    }
                }
            }.execute();
        }
    }

    private void showFailToast() {
        mFemtoSelectState = FEMTO_SELECT_STATE_FAILED;
        if (mDialog != null && mDialog.isShowing()) {
            mDialog.dismiss();
        }
        Toast.makeText(mContext,
                mContext.getString(R.string.femto_fail), Toast.LENGTH_LONG).show();
    }

    private void showPassToast() {
        mFemtoSelectState = FEMTO_SELECT_STATE_DONE;
        if (mDialog != null && mDialog.isShowing()) {
            mDialog.dismiss();
        }
        Toast.makeText(mContext,
                mContext.getString(R.string.femto_pass), Toast.LENGTH_LONG).show();
    }

    private void showProgressDialog() {
        // TODO Auto-generated method stub
        Log.d(TAG, "showProgressDialog");
        mDialog = new ProgressDialog(mContext);
        mDialog.setMessage(mContext.getText(R.string.fetching_femtocell_list));
        mDialog.setCancelable(true);
        mDialog.setCanceledOnTouchOutside(false);
        mDialog.setOnDismissListener(new DialogInterface.OnDismissListener() {
            @Override
            public void onDismiss(DialogInterface dialog) {
                Log.d(LOG_TAG, "onDismiss, femtoSelectState=" + mFemtoSelectState);
                if (mFemtoSelectState == FEMTO_SELECT_STATE_FETCHING) {
                    new AsyncTask<Void, Void, Boolean>() {
                        @Override
                        protected Boolean doInBackground(Void... voids) {
                            Log.d(TAG, "abortFemtoCellList doInBackground  phoneId = " + mPhoneId);
                            Boolean isSucess = mTelephonyManagerEx.abortFemtoCellList(mPhoneId);
                            return isSucess;
                        }

                        @Override
                        protected void onPostExecute(Boolean result) {
                            Log.d(TAG, "onPostExecute abortFemtoCellList = " + result);
                            if (result) {
                                Log.d(TAG, "abortFemtoCellList true");
                            } else {
                                Log.d(TAG, "abortFemtoCellList false");
                            }
                        }
                    }.execute();
                }
                mFemtoSelectState = FEMTO_SELECT_STATE_NONE;
                if (mDialog != null) {
                    mDialog = null;
                }
            }
        });
        mDialog.setIndeterminate(true);
        mDialog.show();
    }
}