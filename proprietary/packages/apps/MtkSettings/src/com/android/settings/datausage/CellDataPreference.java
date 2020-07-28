/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the
 * License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

package com.android.settings.datausage;

import android.app.settings.SettingsEnums;
/// M: Add for receiving the broadcasts.
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
/// M: Add for receiving the broadcasts. @{
import android.content.Intent;
import android.content.IntentFilter;
/// @}
import android.database.ContentObserver;
import android.net.NetworkTemplate;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.os.Parcel;
import android.os.Parcelable;
import android.provider.Settings.Global;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;
import android.widget.Checkable;
/// M: Add for showing toast.
import android.widget.Toast;

import androidx.annotation.VisibleForTesting;
import androidx.appcompat.app.AlertDialog.Builder;
import androidx.core.content.res.TypedArrayUtils;
import androidx.preference.PreferenceViewHolder;

/// M: Add for receiving the broadcasts.
import com.android.internal.telephony.TelephonyIntents;
import com.android.settings.R;
import com.android.settings.Utils;
import com.android.settings.overlay.FeatureFactory;
import com.android.settingslib.CustomDialogPreferenceCompat;

/// M: Add for printing the CTA logs. @{
import com.mediatek.cta.CtaManager;
import com.mediatek.cta.CtaManagerFactory;
/// @}
/// M: Add for data usage plugin. @{
import com.mediatek.settings.UtilsExt;
import com.mediatek.settings.ext.IDataUsageSummaryExt;
/// @}
/// M: Add SIM utility.
import com.mediatek.settings.sim.TelephonyUtils;

import java.util.List;

public class CellDataPreference extends CustomDialogPreferenceCompat implements TemplatePreference {

    private static final String TAG = "CellDataPreference";

    public int mSubId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;
    public boolean mChecked;
    public boolean mMultiSimDialog;
    private TelephonyManager mTelephonyManager;
    @VisibleForTesting
    SubscriptionManager mSubscriptionManager;

    /// M: Disable the preference in airplane mode.
    private boolean mIsAirplaneModeOn = false;
    /// M: Add for data usage plugin.
    private IDataUsageSummaryExt mDataUsageSummaryExt;

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
            } else if (action.equals(TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_DONE)
                    || action.equals(TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_FAILED)) {
                updateScreenEnableState();
            /// M: Add for SIM Lock feature. @{
            } else if (action.equals(TelephonyIntents.ACTION_SIM_SLOT_LOCK_POLICY_INFORMATION)) {
                updateScreenEnableState();
            /// @}
            /// M: Add for data usage plugin. @{
            } else if (mDataUsageSummaryExt.customDualReceiver(action)) {
                updateScreenEnableState();
            /// @}
            }
        }
    };
    /// @}

    public CellDataPreference(Context context, AttributeSet attrs) {
        super(context, attrs, TypedArrayUtils.getAttr(context,
                androidx.preference.R.attr.switchPreferenceStyle,
                android.R.attr.switchPreferenceStyle));

        /// M: Add for data usage plugin.
        mDataUsageSummaryExt = UtilsExt.getDataUsageSummaryExt(context);
    }

    @Override
    protected void onRestoreInstanceState(Parcelable s) {
        CellDataState state = (CellDataState) s;
        super.onRestoreInstanceState(state.getSuperState());
        mTelephonyManager = TelephonyManager.from(getContext());
        mChecked = state.mChecked;
        mMultiSimDialog = state.mMultiSimDialog;
        if (mSubId == SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
            mSubId = state.mSubId;
            setKey(getKey() + mSubId);
        }
        notifyChanged();
    }

    @Override
    protected Parcelable onSaveInstanceState() {
        CellDataState state = new CellDataState(super.onSaveInstanceState());
        state.mChecked = mChecked;
        state.mMultiSimDialog = mMultiSimDialog;
        state.mSubId = mSubId;
        return state;
    }

    @Override
    public void onAttached() {
        log("onAttached");
        super.onAttached();

        final Context context = getContext();
        mDataStateListener.setListener(true, mSubId, context);
        if (mSubscriptionManager!= null) {
            mSubscriptionManager.addOnSubscriptionsChangedListener(mOnSubscriptionsChangeListener);
        }

        /// M: Disable preference in airplane mode.
        mIsAirplaneModeOn = TelephonyUtils.isAirplaneModeOn(context);

        /// M: Update the preference when receiving the broadcasts. @{
        IntentFilter intentFilter = new IntentFilter(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        intentFilter.addAction(TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_DONE);
        intentFilter.addAction(TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_FAILED);
        intentFilter.addAction(TelephonyIntents.ACTION_DEFAULT_DATA_SUBSCRIPTION_CHANGED);
        /// M: Add for SIM Lock feature.
        intentFilter.addAction(TelephonyIntents.ACTION_SIM_SLOT_LOCK_POLICY_INFORMATION);
        /// @}

        /// M: Add for data usage plugin.
        mDataUsageSummaryExt.customReceiver(intentFilter);

        /// M: Update the preference when receiving the broadcasts.
        context.registerReceiver(mReceiver, intentFilter);

        /// M: Update the enabled state with more conditions.
        updateScreenEnableState();

        /// M: Add for data usage plugin.
        mDataUsageSummaryExt.setPreferenceSummary(this);
    }

    @Override
    public void onDetached() {
        log("onDetached");
        final Context context = getContext();
        mDataStateListener.setListener(false, mSubId, context);
        if (mSubscriptionManager!= null) {
            mSubscriptionManager.removeOnSubscriptionsChangedListener(
                    mOnSubscriptionsChangeListener);
        }
        /// M: Update the preference when receiving the broadcasts.
        context.unregisterReceiver(mReceiver);

        super.onDetached();
    }

    @Override
    public void setTemplate(NetworkTemplate template, int subId, NetworkServices services) {
        if (subId == SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
            throw new IllegalArgumentException("CellDataPreference needs a SubscriptionInfo");
        }
        mSubscriptionManager = SubscriptionManager.from(getContext());
        mTelephonyManager = TelephonyManager.from(getContext());

        mSubscriptionManager.addOnSubscriptionsChangedListener(mOnSubscriptionsChangeListener);

        if (mSubId == SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
            mSubId = subId;
            setKey(getKey() + subId);
        }
        /// M: Update the enabled state with more conditions.
        updateScreenEnableState();
        updateChecked();
    }

    private void updateChecked() {
        setChecked(mTelephonyManager.getDataEnabled(mSubId));
    }

    private void updateEnabled() {
        // If this subscription is not active, for example, SIM card is taken out, we disable
        // the button.
        setEnabled(mSubscriptionManager.getActiveSubscriptionInfo(mSubId) != null);
    }

    @Override
    protected void performClick(View view) {
        final Context context = getContext();
        FeatureFactory.getFactory(context).getMetricsFeatureProvider()
                .action(context, SettingsEnums.ACTION_CELL_DATA_TOGGLE, !mChecked);
        final SubscriptionInfo currentSir = mSubscriptionManager.getActiveSubscriptionInfo(
                mSubId);
        final SubscriptionInfo nextSir = mSubscriptionManager.getDefaultDataSubscriptionInfo();
        log("performClick, currentSir=" + currentSir + ", nextSir=" + nextSir);
        if (mChecked) {
            // If the device is single SIM or is enabling data on the active data SIM then forgo
            // the pop-up.
            if (!Utils.showSimCardTile(getContext()) ||
                    (nextSir != null && currentSir != null &&
                            currentSir.getSubscriptionId() == nextSir.getSubscriptionId())) {
                setMobileDataEnabled(false);
                if (nextSir != null && currentSir != null &&
                        currentSir.getSubscriptionId() == nextSir.getSubscriptionId()
                        /// M: Add for data usage plugin.
                        && mDataUsageSummaryExt.isAllowDataDisableForOtherSubscription()) {
                    disableDataForOtherSubscriptions(mSubId);
                }
                return;
            }
            // disabling data; show confirmation dialog which eventually
            // calls setMobileDataEnabled() once user confirms.
            mMultiSimDialog = false;
            super.performClick(view);
        } else {
            // If we are showing the Sim Card tile then we are a Multi-Sim device.
            if (Utils.showSimCardTile(getContext())) {
                mMultiSimDialog = true;
                if (nextSir != null && currentSir != null &&
                        currentSir.getSubscriptionId() == nextSir.getSubscriptionId()) {
                    setMobileDataEnabled(true);
                    /// M: Add for data usage plugin.
                    if (mDataUsageSummaryExt.isAllowDataDisableForOtherSubscription()) {
                        disableDataForOtherSubscriptions(mSubId);
                    }
                    return;
                }
                super.performClick(view);
            } else {
                setMobileDataEnabled(true);
            }
        }
    }

    private void setMobileDataEnabled(boolean enabled) {
        if (DataUsageSummary.LOGD) Log.d(TAG, "setMobileDataEnabled(" + enabled + ","
                + mSubId + ")");
        mTelephonyManager.setDataEnabled(mSubId, enabled);
        setChecked(enabled);

        /// M: Add for printing the CTA logs. @{
        if (enabled) {
            CtaManager ctaManager = CtaManagerFactory.getInstance().makeCtaManager();
            ctaManager.printCtaInfor(getContext(),
                    CtaManager.KeywordType.NETWORKCONNECT,
                    "CellDataPreference.setMobileDataEnabled",
                    CtaManager.ActionType.ENABLE_MOBILE_NETWORKCONNECT,
                    "mobiledata");
        }
        /// @}
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

        /// M: Add for data usage plugin. @{
        mDataUsageSummaryExt.onBindViewHolder(getContext(), holder.itemView,
                new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                performClick(v);
            }
        });
        /// @}
    }

    @Override
    protected void onPrepareDialogBuilder(Builder builder,
            DialogInterface.OnClickListener listener) {
        if (mMultiSimDialog) {
            showMultiSimDialog(builder, listener);
        } else {
            /// M: Add for data usage plugin.
            if (mDataUsageSummaryExt.onDisablingData(mSubId)) {
                showDisableDialog(builder, listener);
            }
        }
    }

    private void showDisableDialog(Builder builder,
            DialogInterface.OnClickListener listener) {
        builder.setTitle(null)
                .setMessage(R.string.data_usage_disable_mobile)
                .setPositiveButton(android.R.string.ok, listener)
                .setNegativeButton(android.R.string.cancel, null);
    }

    private void showMultiSimDialog(Builder builder,
            DialogInterface.OnClickListener listener) {
        final SubscriptionInfo currentSir = mSubscriptionManager.getActiveSubscriptionInfo(mSubId);
        final SubscriptionInfo nextSir = mSubscriptionManager.getDefaultDataSubscriptionInfo();

        final String previousName = (nextSir == null)
            ? getContext().getResources().getString(R.string.sim_selection_required_pref)
            : nextSir.getDisplayName().toString();

        /// M: Revise for filling the carrier name in dialog title and message. @{
        final String newName = (currentSir == null)
                ? getContext().getResources().getString(
                        R.string.sim_selection_required_pref)
                : currentSir.getDisplayName().toString();

        builder.setTitle(getContext().getString(R.string.sim_change_data_title, newName));
        builder.setMessage(getContext().getString(R.string.sim_change_data_message,
                newName, previousName));
        /// @}

        builder.setPositiveButton(R.string.okay, listener);
        builder.setNegativeButton(R.string.cancel, null);
    }

    private void disableDataForOtherSubscriptions(int subId) {
        List<SubscriptionInfo> subInfoList = mSubscriptionManager
                .getActiveSubscriptionInfoList(true);
        if (subInfoList != null) {
            for (SubscriptionInfo subInfo : subInfoList) {
                if (subInfo.getSubscriptionId() != subId) {
                    mTelephonyManager.setDataEnabled(subInfo.getSubscriptionId(), false);
                }
            }
        }
    }

    @Override
    protected void onClick(DialogInterface dialog, int which) {
        if (which != DialogInterface.BUTTON_POSITIVE) {
            return;
        }

        log("onClick, multiSimDialog=" + mMultiSimDialog);
        if (mMultiSimDialog) {
            /// M: Cann't switch data enabled state when in call. @{
            if (TelephonyUtils.isInCall()) {
                Toast.makeText(getContext(), R.string.default_data_switch_err_msg1,
                        Toast.LENGTH_SHORT).show();
                log("onClick, don't switch data state when in call.");
                return;
            }
            /// @}

            mSubscriptionManager.setDefaultDataSubId(mSubId);
            setMobileDataEnabled(true);
            /// M: Add for data usage plugin.
            if (mDataUsageSummaryExt.isAllowDataDisableForOtherSubscription()) {
                disableDataForOtherSubscriptions(mSubId);
            }
        } else {
            // TODO: extend to modify policy enabled flag.
            setMobileDataEnabled(false);
        }
    }

    @VisibleForTesting
    final SubscriptionManager.OnSubscriptionsChangedListener mOnSubscriptionsChangeListener
            = new SubscriptionManager.OnSubscriptionsChangedListener() {
        @Override
        public void onSubscriptionsChanged() {
            if (DataUsageSummary.LOGD) {
                Log.d(TAG, "onSubscriptionsChanged");
            }
            /// M: Update the enabled state with more conditions.
            updateScreenEnableState();
        }
    };

    private final DataStateListener mDataStateListener = new DataStateListener() {
        @Override
        public void onChange(boolean selfChange) {
            log("onChange, selfChange=" + selfChange);
            updateChecked();
        }
    };

    public abstract static class DataStateListener extends ContentObserver {
        public DataStateListener() {
            super(new Handler(Looper.getMainLooper()));
        }

        public void setListener(boolean listening, int subId, Context context) {
            if (listening) {
                Uri uri = Global.getUriFor(Global.MOBILE_DATA);
                if (TelephonyManager.getDefault().getSimCount() != 1) {
                    uri = Global.getUriFor(Global.MOBILE_DATA + subId);
                }
                context.getContentResolver().registerContentObserver(uri, false, this);
            } else {
                context.getContentResolver().unregisterContentObserver(this);
            }
        }
    }

    public static class CellDataState extends BaseSavedState {
        public int mSubId;
        public boolean mChecked;
        public boolean mMultiSimDialog;

        public CellDataState(Parcelable base) {
            super(base);
        }

        public CellDataState(Parcel source) {
            super(source);
            mChecked = source.readByte() != 0;
            mMultiSimDialog = source.readByte() != 0;
            mSubId = source.readInt();
        }

        @Override
        public void writeToParcel(Parcel dest, int flags) {
            super.writeToParcel(dest, flags);
            dest.writeByte((byte) (mChecked ? 1 : 0));
            dest.writeByte((byte) (mMultiSimDialog ? 1 : 0));
            dest.writeInt(mSubId);
        }

        public static final Creator<CellDataState> CREATOR = new Creator<CellDataState>() {
            @Override
            public CellDataState createFromParcel(Parcel source) {
                return new CellDataState(source);
            }

            @Override
            public CellDataState[] newArray(int size) {
                return new CellDataState[size];
            }
        };
    }

    /// M: Update the enabled state with more conditions. @{
    private void updateScreenEnableState() {
        /// M: Disable preference when capability switching.
        boolean capaSwitching = TelephonyUtils.isCapabilitySwitching();
        /// M: Add for SIM Lock feature.
        boolean enabledForSimLock = shouldEnableCellDataPrefForSimLock();
        log("updateScreenEnableState, airplaneMode=" + mIsAirplaneModeOn
                + ", capaSwitching=" + capaSwitching
                + ", enabledForSimLock=" + enabledForSimLock);
        boolean enabled = (!mIsAirplaneModeOn && !capaSwitching && enabledForSimLock);
        if (mDataUsageSummaryExt != null) {
            enabled &= mDataUsageSummaryExt.isAllowDataEnable(mSubId);
        }
    }
    /// @}

    /// M: Add for SIM Lock feature. @{
    private boolean shouldEnableCellDataPrefForSimLock() {
        int policy = TelephonyUtils.getSimLockPolicy();
        int slot = SubscriptionManager.getSlotIndex(mSubId);

        boolean enabledForSimLock = true;
        switch (policy) {
            case TelephonyUtils.SIM_LOCK_POLICY_UNKNOWN:
            case TelephonyUtils.SIM_LOCK_POLICY_SLOT1_ONLY:
            case TelephonyUtils.SIM_LOCK_POLICY_SLOT2_ONLY:
            case TelephonyUtils.SIM_LOCK_POLICY_SLOT_ALL:
            case TelephonyUtils.SIM_LOCK_POLICY_SLOT1_LINKED:
            case TelephonyUtils.SIM_LOCK_POLICY_SLOT2_LINKED:
            case TelephonyUtils.SIM_LOCK_POLICY_SLOT_ALL_LINKED:
            case TelephonyUtils.SIM_LOCK_POLICY_SLOT_ALL_LINKED_WITH_CS:
            case TelephonyUtils.SIM_LOCK_POLICY_SLOT_ALL_LINKED_REVERSE:
                int simCapa = TelephonyUtils.getSimLockSimCapability(slot);
                if (simCapa != TelephonyUtils.SIM_LOCK_SIM_CAPABILITY_FULL
                        && simCapa != TelephonyUtils.SIM_LOCK_SIM_CAPABILITY_PS_ONLY) {
                    enabledForSimLock = false;
                }
                break;

            case TelephonyUtils.SIM_LOCK_POLICY_SLOT_ALL_LINKED_WITH_ECC:
                enabledForSimLock = isValidSimExistForSimLock();
                break;

            default:
                break;
        }

        if (!enabledForSimLock) {
            log("shouldEnableCellDataPrefForSimLock, policy=" + policy
                    + ", enabled=" + enabledForSimLock);
        }
        return enabledForSimLock;
    }

    private boolean isValidSimExistForSimLock() {
        List<SubscriptionInfo> subInfoList =
                SubscriptionManager.from(getContext()).getActiveSubscriptionInfoList(true);

        if (subInfoList != null) {
            for (SubscriptionInfo info : subInfoList) {
                int slotId = info.getSimSlotIndex();
                int simValid = TelephonyUtils.getSimLockSimValid(slotId);
                if (simValid == TelephonyUtils.SIM_LOCK_SIM_VALID_YES) {
                    log("isValidSimExistForSimLock, SIM" + slotId
                            + "=" + info.getSubscriptionId() + " is valid.");
                    return true;
                }
            }
        }

        log("isValidSimExistForSimLock, no valid SIM.");
        return false;
    }
    /// @}

    private void log(String msg) {
        Log.d(TAG + "[" + mSubId + "]", msg);
    }
}
