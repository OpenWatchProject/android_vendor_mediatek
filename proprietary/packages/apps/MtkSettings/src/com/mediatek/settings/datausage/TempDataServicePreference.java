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

package com.mediatek.settings.datausage;

import android.app.settings.SettingsEnums;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.ContentObserver;
import android.net.NetworkTemplate;
import android.os.Handler;
import android.provider.Settings;
import android.telephony.PhoneStateListener;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;
import android.widget.Checkable;
import android.widget.Toast;

import androidx.appcompat.app.AlertDialog.Builder;
import androidx.core.content.res.TypedArrayUtils;
import androidx.preference.PreferenceViewHolder;

import com.android.internal.telephony.TelephonyIntents;
import com.android.settings.R;
import com.android.settings.datausage.TemplatePreference;
import com.android.settings.datausage.TemplatePreference.NetworkServices;
import com.android.settingslib.CustomDialogPreferenceCompat;

import com.mediatek.provider.MtkSettingsExt;
import com.mediatek.settings.UtilsExt;
import com.mediatek.settings.ext.IDataUsageSummaryExt;
import com.mediatek.settings.sim.TelephonyUtils;

import java.util.List;

public class TempDataServicePreference extends CustomDialogPreferenceCompat implements TemplatePreference {

    private static final String TAG = "TempDataServicePreference";

    private static final String DATA_SERVICE_ENABLED = MtkSettingsExt.Global.DATA_SERVICE_ENABLED;

    public int mSubId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;
    public boolean mChecked;
    private TelephonyManager mTelephonyManager;
    private SubscriptionManager mSubscriptionManager;
    private IDataUsageSummaryExt mDataUsageSummaryExt;
    private static final int TYPE_TEMP_DATA_SERVICE_SUMMARY = 0;
    private static final int TYPE_TEMP_DATA_SERVICE_MESSAGE = 1;

    /// M: Disable the preference in airplane mode.
    private boolean mIsAirplaneModeOn = false;

    /// M: Update the preference when receiving the broadcasts. @{
    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            log("onReceive, action=" + action);

            if (action.equals(Intent.ACTION_AIRPLANE_MODE_CHANGED)) {
                mIsAirplaneModeOn = intent.getBooleanExtra("state", false);
                updateScreenEnableState();
            } else if (action.equals(
                    TelephonyIntents.ACTION_DEFAULT_DATA_SUBSCRIPTION_CHANGED)) {
                updateScreenEnableState();
            } else if (TelephonyManager.ACTION_PHONE_STATE_CHANGED.equals(action)){
                updateScreenEnableState();
            }
        }
    };
    /// @}

    public TempDataServicePreference(Context context, AttributeSet attrs) {
        super(context, attrs, TypedArrayUtils.getAttr(context,
                androidx.preference.R.attr.switchPreferenceStyle,
                android.R.attr.switchPreferenceStyle));
        /// M: Add for data usage plugin.
        mDataUsageSummaryExt = UtilsExt.getDataUsageSummaryExt(
                context.getApplicationContext());
    }

    @Override
    public void onAttached() {
        log("onAttached.");
        super.onAttached();

        final Context context = getContext();

        mIsAirplaneModeOn = TelephonyUtils.isAirplaneModeOn(context);

        IntentFilter intentFilter = new IntentFilter(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        intentFilter.addAction(TelephonyIntents.ACTION_DEFAULT_DATA_SUBSCRIPTION_CHANGED);
        intentFilter.addAction(TelephonyManager.ACTION_PHONE_STATE_CHANGED);

        context.registerReceiver(mReceiver, intentFilter);

        context.getContentResolver().registerContentObserver(
                Settings.Global.getUriFor(DATA_SERVICE_ENABLED), false, mContentObserver);
        updateScreenEnableState();
    }

    @Override
    public void onDetached() {
        log("onDetached");

        final Context context = getContext();
        context.unregisterReceiver(mReceiver);
        context.getContentResolver().unregisterContentObserver(mContentObserver);
        super.onDetached();
    }

    @Override
    public void setTemplate(NetworkTemplate template, int subId, NetworkServices services) {
        if (subId == SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
            throw new IllegalArgumentException("CellDataPreference needs a SubscriptionInfo");
        }
        mSubscriptionManager = SubscriptionManager.from(getContext());
        mTelephonyManager = TelephonyManager.from(getContext());

        int tempSubId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;
        List<SubscriptionInfo> subList =
                mSubscriptionManager.getActiveSubscriptionInfoList(true);
        if (subList != null && subList.size() == 2) {
            for (SubscriptionInfo info : subList) {
                if (info.getSubscriptionId() != subId) {
                    tempSubId = info.getSubscriptionId();
                    break;
                }
            }
        }

        if (mSubId == SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
            mSubId = tempSubId;
        }

        if (mSubId == SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
            return;
        }

        updateScreenEnableState();
        updateChecked();
    }

    private void updateChecked() {
        boolean checked = isDataServiceEnabled();
        setChecked(checked);
    }

    @Override
    protected void performClick(View view) {
        final Context context = getContext();
        log("performClick, checked=" + mChecked);
        if (mChecked) {
            setDataServiceEnabled(0);
            setChecked(false);
        } else {
            super.performClick(view);
            setEnabled(false);
        }
    }

    private void setChecked(boolean checked) {
        log("setChecked, currChecked=" + mChecked + ", newChecked=" + checked);
        if (mChecked == checked) return;
        mChecked = checked;
        notifyChanged();
    }

    @Override
    public void onBindViewHolder(PreferenceViewHolder holder) {
        super.onBindViewHolder(holder);
        View switchView = holder.findViewById(android.R.id.switch_widget);
        switchView.setClickable(false);
        ((Checkable) switchView).setChecked(mChecked);
    }

    @Override
    protected void onPrepareDialogBuilder(Builder builder,
            DialogInterface.OnClickListener listener) {
        String customerDialogString = mDataUsageSummaryExt.customTempDataSummary(
                getContext().getString(R.string.temp_data_service_prompt),
                TYPE_TEMP_DATA_SERVICE_MESSAGE);
        builder.setTitle(null)
                .setMessage(customerDialogString)
                .setPositiveButton(android.R.string.ok, listener)
                .setNegativeButton(android.R.string.cancel, listener);
    }

    @Override
    protected void onClick(DialogInterface dialog, int which) {
        log("onClick, which=" + which);

        updateScreenEnableState();

        if (which != DialogInterface.BUTTON_POSITIVE) {
            return;
        }

        setDataServiceEnabled(1);
        setChecked(true);
    }

    @Override
    protected void onDialogClosed(boolean positiveResult) {
        log("onDialogClosed");

        updateScreenEnableState();
    }

    private boolean isDataServiceEnabled() {
        int dataService = Settings.Global.getInt(getContext().getContentResolver(),
                    DATA_SERVICE_ENABLED, 0);
        log("isDataServiceEnabled, dataService=" + dataService);
        return dataService == 0 ? false : true;
    }

    private void setDataServiceEnabled(int value) {
        log("setDataServiceEnabled, value=" + value);
        Settings.Global.putInt(getContext().getContentResolver(),
                DATA_SERVICE_ENABLED, value);
    }

    private ContentObserver mContentObserver = new ContentObserver(new Handler()) {
        @Override
        public void onChange(boolean selfChange) {
            Log.d(TAG, "onChange");
            updateChecked();
        }
    };

    private void updateScreenEnableState() {
        boolean enabled = (!mIsAirplaneModeOn
                && !mDataUsageSummaryExt.customTempdata());
        log("updateScreenEnableState, enabled=" + enabled
                + ", airplaneMode=" + mIsAirplaneModeOn);
        setEnabled(enabled);
    }

    private void log(String msg) {
        Log.d(TAG + "[" + mSubId + "]", msg);
    }

}