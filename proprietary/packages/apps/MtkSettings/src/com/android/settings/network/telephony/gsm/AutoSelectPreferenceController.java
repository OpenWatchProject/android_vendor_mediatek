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

import android.app.ProgressDialog;
import android.app.settings.SettingsEnums;
import android.content.Context;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.PersistableBundle;
import android.os.SystemClock;
import android.provider.Settings;
import android.telephony.CarrierConfigManager;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;

import androidx.annotation.VisibleForTesting;
import androidx.preference.Preference;
import androidx.preference.PreferenceScreen;
import androidx.preference.SwitchPreference;

import com.android.settings.R;
import com.android.settings.core.SubSettingLauncher;
import com.android.settings.network.telephony.MobileNetworkUtils;
import com.android.settings.network.telephony.NetworkSelectSettings;
import com.android.settings.network.telephony.TelephonyTogglePreferenceController;
import com.android.settingslib.utils.ThreadUtils;

/// M: Add more condition for enabled state.
import com.mediatek.settings.sim.TelephonyUtils;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.TimeUnit;

/**
 * Preference controller for "Auto Select Network"
 */
public class AutoSelectPreferenceController extends TelephonyTogglePreferenceController {

    private static final String TAG = "AutoSelectPreferenceController";

    private static final long MINIMUM_DIALOG_TIME_MILLIS = TimeUnit.SECONDS.toMillis(1);

    private final Handler mUiHandler;
    private TelephonyManager mTelephonyManager;
    private boolean mOnlyAutoSelectInHome;
    private List<OnNetworkSelectModeListener> mListeners;
    @VisibleForTesting
    ProgressDialog mProgressDialog;
    @VisibleForTesting
    SwitchPreference mSwitchPreference;

    /// M: Add for getting/setting selection mode async. @{
    private int mSelectMode = TelephonyManager.NETWORK_SELECTION_MODE_UNKNOWN;
    private boolean mIsGettingMode = false;
    private boolean mIsSettingMode = false;
    private boolean mIsInProgress = false;
    /// @}

    public AutoSelectPreferenceController(Context context, String key) {
        super(context, key);
        mTelephonyManager = context.getSystemService(TelephonyManager.class);
        mSubId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;
        mListeners = new ArrayList<>();
        mUiHandler = new Handler(Looper.getMainLooper());
    }

    @Override
    public int getAvailabilityStatus(int subId) {
        /// M: Add for log.
        final boolean enabled =
                MobileNetworkUtils.shouldDisplayNetworkSelectOptions(mContext, subId);

        Log.i(TAG, "getAvailabilityStatus, subId=" + subId + ", enabled=" + enabled);

        return enabled ? AVAILABLE : CONDITIONALLY_UNAVAILABLE;
    }

    @Override
    public void displayPreference(PreferenceScreen screen) {
        super.displayPreference(screen);
        mSwitchPreference = screen.findPreference(getPreferenceKey());
    }

    @Override
    public boolean isChecked() {
        /// M: Add for getting selection mode async. @{
        Log.d(TAG, "isChecked, phoneId=" + mPhoneId + ", isSettingMode=" + mIsSettingMode
                + ", isGettingMode=" + mIsGettingMode + ", isInProgress=" + mIsInProgress);
        if (mIsSettingMode) {
            return true;
        }

        if (mIsInProgress) {
            return mSelectMode == TelephonyManager.NETWORK_SELECTION_MODE_AUTO;
        }

        if (!mIsGettingMode) {
            mIsGettingMode = true;
            mIsInProgress = true;
            final long startMillis = SystemClock.elapsedRealtime();
            mSwitchPreference.setEnabled(false);
            ThreadUtils.postOnBackgroundThread(() -> {
                Log.d(TAG, "postOnBackgroundThread, phoneId=" + mPhoneId
                        + ", isGettingMode=" + mIsGettingMode);
                if (!mIsGettingMode || mSwitchPreference == null) {
                    return;
                }

                // Get network selection mode in background
                mSelectMode = mTelephonyManager.getNetworkSelectionMode();

                // Update UI in UI thread
                final long durationMillis = SystemClock.elapsedRealtime() - startMillis;
                mUiHandler.postDelayed(() -> {
                            Log.d(TAG, "postDelayed, phoneId=" + mPhoneId
                                    + ", isGettingMode=" + mIsGettingMode);
                            if (!mIsGettingMode || mSwitchPreference == null) {
                                return;
                            }

                            mIsGettingMode = false;
                            mSwitchPreference.setChecked(
                                    mSelectMode == TelephonyManager.NETWORK_SELECTION_MODE_AUTO);
                            updateState(mSwitchPreference);
                            mIsInProgress = false;
                        }, 0);
            });
        }

        return mSelectMode == TelephonyManager.NETWORK_SELECTION_MODE_AUTO;
        /// @}
    }

    @Override
    public void updateState(Preference preference) {
        super.updateState(preference);

        preference.setSummary(null);
        /// M: Add more condition for enabled state.
        Log.d(TAG, "updateState, phoneId=" + mPhoneId
                + ", isGettingMode=" + mIsGettingMode
                + ", isSettingMode=" + mIsSettingMode);
        boolean enabled = !mIsGettingMode && !mIsSettingMode
                && MobileNetworkUtils.shouldEnableMobileNetworkOption(mPhoneId);
        /// M: The service state may be null.
        if (mTelephonyManager.getServiceState() != null
                && mTelephonyManager.getServiceState().getRoaming()) {
            /// M: Add more condition for enabled state.
            preference.setEnabled(enabled);
        } else {
            /// M: Add more condition for enabled state.
            preference.setEnabled(!mOnlyAutoSelectInHome && enabled);
            if (mOnlyAutoSelectInHome) {
                preference.setSummary(mContext.getString(
                        R.string.manual_mode_disallowed_summary,
                        mTelephonyManager.getSimOperatorName()));
            }
        }
    }

    @Override
    public boolean setChecked(boolean isChecked) {
        Log.d(TAG, "setChecked, phoneId=" + mPhoneId
                + ", isGettingMode=" + mIsGettingMode + ", isInProgress=" + mIsInProgress);
        /// M: Add for setting selection mode async. @{
        if (mIsSettingMode || mIsGettingMode || mIsInProgress) {
            return false;
        }
        /// @}

        if (isChecked) {
            /// M: Add for setting selection mode async.
            mIsSettingMode = true;

            final long startMillis = SystemClock.elapsedRealtime();
            showAutoSelectProgressBar();
            mSwitchPreference.setEnabled(false);
            ThreadUtils.postOnBackgroundThread(() -> {
                /// M: Add for setting selection mode async.
                if (!mIsSettingMode || mSwitchPreference == null) {
                    return;
                }
                /// @}

                // set network selection mode in background
                mTelephonyManager.setNetworkSelectionModeAutomatic();
                /// M: Add for setting selection mode async.
                mSelectMode = mTelephonyManager.getNetworkSelectionMode();

                //Update UI in UI thread
                final long durationMillis = SystemClock.elapsedRealtime() - startMillis;
                mUiHandler.postDelayed(() -> {
                            /// M: Add for setting selection mode async. @{
                            if (!mIsSettingMode || mSwitchPreference == null) {
                                return;
                            }

                            mIsSettingMode = false;
                            updateState(mSwitchPreference);
                            /// @}

                            mSwitchPreference.setChecked(
                                    mSelectMode == TelephonyManager.NETWORK_SELECTION_MODE_AUTO);
                            for (OnNetworkSelectModeListener lsn : mListeners) {
                                lsn.onNetworkSelectModeChanged();
                            }
                            dismissProgressBar();
                        },
                        Math.max(MINIMUM_DIALOG_TIME_MILLIS - durationMillis, 0));
            });
            return false;
        } else {
            final Bundle bundle = new Bundle();
            bundle.putInt(Settings.EXTRA_SUB_ID, mSubId);
            new SubSettingLauncher(mContext)
                    .setDestination(NetworkSelectSettings.class.getName())
                    .setSourceMetricsCategory(SettingsEnums.MOBILE_NETWORK_SELECT)
                    .setTitleRes(R.string.choose_network_title)
                    .setArguments(bundle)
                    .launch();
            return false;
        }
    }

    public AutoSelectPreferenceController init(int subId) {
        mSubId = subId;
        /// M: Add for recording the phone Id.
        mPhoneId = SubscriptionManager.getPhoneId(subId);
        mTelephonyManager = TelephonyManager.from(mContext).createForSubscriptionId(mSubId);
        final PersistableBundle carrierConfig = mContext.getSystemService(
                CarrierConfigManager.class).getConfigForSubId(mSubId);
        mOnlyAutoSelectInHome = carrierConfig != null
                ? carrierConfig.getBoolean(
                CarrierConfigManager.KEY_ONLY_AUTO_SELECT_IN_HOME_NETWORK_BOOL)
                : false;

        /// M: Add for getting/setting selection mode async. @{
        mUiHandler.removeCallbacksAndMessages(null);
        mSelectMode = TelephonyManager.NETWORK_SELECTION_MODE_UNKNOWN;
        mIsGettingMode = false;
        mIsSettingMode = false;
        mIsInProgress = false;
        /// @}

        return this;
    }

    public AutoSelectPreferenceController addListener(OnNetworkSelectModeListener lsn) {
        mListeners.add(lsn);

        return this;
    }

    private void showAutoSelectProgressBar() {
        if (mProgressDialog == null) {
            mProgressDialog = new ProgressDialog(mContext);
            mProgressDialog.setMessage(
                    mContext.getResources().getString(R.string.register_automatically));
            mProgressDialog.setCanceledOnTouchOutside(false);
            mProgressDialog.setCancelable(false);
            mProgressDialog.setIndeterminate(true);
        }
        mProgressDialog.show();
    }

    private void dismissProgressBar() {
        if (mProgressDialog != null && mProgressDialog.isShowing()) {
            mProgressDialog.dismiss();
        }
    }

    /**
     * Callback when network select mode is changed
     *
     * @see TelephonyManager#getNetworkSelectionMode()
     */
    public interface OnNetworkSelectModeListener {
        void onNetworkSelectModeChanged();
    }
}
