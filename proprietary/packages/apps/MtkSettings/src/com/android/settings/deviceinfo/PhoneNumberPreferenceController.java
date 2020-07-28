/*
 * Copyright (C) 2017 The Android Open Source Project
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

package com.android.settings.deviceinfo;

import static android.content.Context.CLIPBOARD_SERVICE;

import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Context;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.BidiFormatter;
import android.text.TextDirectionHeuristics;
import android.text.TextUtils;
import android.util.Log;
import android.widget.Toast;

import androidx.annotation.VisibleForTesting;
import androidx.preference.Preference;
import androidx.preference.PreferenceScreen;

import com.android.settings.R;
import com.android.settings.core.BasePreferenceController;
import com.android.settingslib.DeviceInfoUtils;
/// M: Add for updating phone number. @{
import com.android.settingslib.core.lifecycle.LifecycleObserver;
import com.android.settingslib.core.lifecycle.events.OnStart;
import com.android.settingslib.core.lifecycle.events.OnStop;
import com.mediatek.settings.UtilsExt;
import com.mediatek.settings.ext.IStatusExt;
/// @}

import java.util.ArrayList;
import java.util.List;

/// M: Revise for updating phone number.
public class PhoneNumberPreferenceController extends BasePreferenceController
        implements LifecycleObserver, OnStart, OnStop {

    private static final String TAG = "PhoneNumberPreferenceController";

    private final static String KEY_PHONE_NUMBER = "phone_number";

    private final TelephonyManager mTelephonyManager;
    private final SubscriptionManager mSubscriptionManager;
    private final List<Preference> mPreferenceList = new ArrayList<>();
    private IStatusExt mStatusExt;

    public PhoneNumberPreferenceController(Context context, String key) {
        super(context, key);
        mTelephonyManager = mContext.getSystemService(TelephonyManager.class);
        mSubscriptionManager = mContext.getSystemService(SubscriptionManager.class);
        /// M: Add for SIM status plugin.
        mStatusExt = UtilsExt.getStatusExt(context);
    }

    @Override
    public int getAvailabilityStatus() {
        return mTelephonyManager.isVoiceCapable() ? AVAILABLE : UNSUPPORTED_ON_DEVICE;
    }

    @Override
    public CharSequence getSummary() {
        return getFirstPhoneNumber();
    }

    @Override
    public void displayPreference(PreferenceScreen screen) {
        super.displayPreference(screen);
        final Preference preference = screen.findPreference(getPreferenceKey());
        mPreferenceList.add(preference);

        final int phonePreferenceOrder = preference.getOrder();
        // Add additional preferences for each sim in the device
        for (int simSlotNumber = 1; simSlotNumber < mTelephonyManager.getPhoneCount();
                simSlotNumber++) {
            final Preference multiSimPreference = createNewPreference(screen.getContext());
            multiSimPreference.setOrder(phonePreferenceOrder + simSlotNumber);
            multiSimPreference.setKey(KEY_PHONE_NUMBER + simSlotNumber);
            /// M: Add for disable the other preference's selectable.
            multiSimPreference.setSelectable(false);
            screen.addPreference(multiSimPreference);
            mPreferenceList.add(multiSimPreference);
        }
    }

    @Override
    public void updateState(Preference preference) {
        for (int simSlotNumber = 0; simSlotNumber < mPreferenceList.size(); simSlotNumber++) {
            final Preference simStatusPreference = mPreferenceList.get(simSlotNumber);
            simStatusPreference.setTitle(getPreferenceTitle(simSlotNumber));
            simStatusPreference.setSummary(getPhoneNumber(simSlotNumber));
        }
    }

    @Override
    public boolean isSliceable() {
        return true;
    }

    @Override
    public boolean isCopyableSlice() {
        return true;
    }

    @Override
    public boolean useDynamicSliceSummary() {
        return true;
    }

    @Override
    public void copy() {
        final ClipboardManager clipboard = (ClipboardManager) mContext.getSystemService(
                CLIPBOARD_SERVICE);
        clipboard.setPrimaryClip(ClipData.newPlainText("text", getFirstPhoneNumber()));

        final String toast = mContext.getString(R.string.copyable_slice_toast,
                mContext.getText(R.string.status_number));
        Toast.makeText(mContext, toast, Toast.LENGTH_SHORT).show();
    }

    private CharSequence getFirstPhoneNumber() {
        final List<SubscriptionInfo> subscriptionInfoList =
                mSubscriptionManager.getActiveSubscriptionInfoList(true);
        if (subscriptionInfoList == null || subscriptionInfoList.isEmpty()) {
            return mContext.getText(R.string.device_info_default);
        }

        // For now, We only return first result for slice view.
        return getFormattedPhoneNumber(subscriptionInfoList.get(0));
    }

    private CharSequence getPhoneNumber(int simSlot) {
        final SubscriptionInfo subscriptionInfo = getSubscriptionInfo(simSlot);
        if (subscriptionInfo == null) {
            return mContext.getText(R.string.device_info_default);
        }

        String number = mStatusExt.updatePhoneNumber(
                getFormattedPhoneNumber(subscriptionInfo).toString(),
                subscriptionInfo.getSimSlotIndex(), mContext);
        return number;
    }

    private CharSequence getPreferenceTitle(int simSlot) {
        return mTelephonyManager.getPhoneCount() > 1 ? mContext.getString(
                R.string.status_number_sim_slot, simSlot + 1) : mContext.getString(
                R.string.status_number);
    }

    @VisibleForTesting
    SubscriptionInfo getSubscriptionInfo(int simSlot) {
        final List<SubscriptionInfo> subscriptionInfoList =
                mSubscriptionManager.getActiveSubscriptionInfoList(true);
        if (subscriptionInfoList != null) {
            for (SubscriptionInfo info : subscriptionInfoList) {
                if (info.getSimSlotIndex() == simSlot) {
                    return info;
                }
            }
        }
        return null;
    }

    @VisibleForTesting
    CharSequence getFormattedPhoneNumber(SubscriptionInfo subscriptionInfo) {
        final String phoneNumber = DeviceInfoUtils.getFormattedPhoneNumber(mContext,
                subscriptionInfo);
        return TextUtils.isEmpty(phoneNumber) ? mContext.getString(R.string.device_info_default)
                : BidiFormatter.getInstance().unicodeWrap(phoneNumber, TextDirectionHeuristics.LTR);
    }

    @VisibleForTesting
    Preference createNewPreference(Context context) {
        return new Preference(context);
    }

    /// M: Register listener for updating phone number. @{
    private final SubscriptionManager.OnSubscriptionsChangedListener mOnSubscriptionsChangeListener
            = new SubscriptionManager.OnSubscriptionsChangedListener() {
        @Override
        public void onSubscriptionsChanged() {
            Log.d(TAG, "onSubscriptionsChanged");
            updateState(null);
        }
    };

    @Override
    public void onStart() {
        // Register listener for updating phone number.
        mSubscriptionManager.addOnSubscriptionsChangedListener(mOnSubscriptionsChangeListener);
    }

    @Override
    public void onStop() {
        // Unregister listener.
        mSubscriptionManager.removeOnSubscriptionsChangedListener(mOnSubscriptionsChangeListener);
    }
    /// @}
}
