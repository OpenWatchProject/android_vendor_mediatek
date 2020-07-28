/*
 * Copyright (C) 2014 The Android Open Source Project
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

package com.android.settings.sim;

import android.app.Activity;
import android.app.Dialog;
/// M: Dismiss the dialog when turning on airplane mode.
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
/// M: Dismiss the dialog when turning on airplane mode. @{
import android.content.Intent;
import android.content.IntentFilter;
/// @}
/// M: Finish the activity when the dialog is dismissed.
import android.content.DialogInterface.OnDismissListener;
import android.content.res.Resources;
import android.os.Bundle;
import android.telecom.PhoneAccount;
import android.telecom.PhoneAccountHandle;
import android.telecom.TelecomManager;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.ListAdapter;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AlertDialog;

import com.android.settings.R;

/// M: Add for printing the CTA logs. @{
import com.mediatek.cta.CtaManager;
import com.mediatek.cta.CtaManagerFactory;
/// @}
/// M: Add for SIM settings plugin.
import com.mediatek.settings.UtilsExt;
/// M: Get the default SMS SIM's sub id.
import com.mediatek.internal.telephony.MtkDefaultSmsSimSettings;
/// M: Add for CDMA SIM settings.
import com.mediatek.settings.cdma.CdmaUtils;
/// M: Add for SIM settings plugin.
import com.mediatek.settings.ext.ISimManagementExt;
import com.mediatek.settings.sim.TelephonyUtils;
/// M: Add for supporting SIM hot swap. @{
import com.mediatek.settings.sim.SimHotSwapHandler;
import com.mediatek.settings.sim.SimHotSwapHandler.OnSimHotSwapListener;
/// @}

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

public class SimDialogActivity extends Activity {
    private static String TAG = "SimDialogActivity";

    public static String PREFERRED_SIM = "preferred_sim";
    public static String DIALOG_TYPE_KEY = "dialog_type";
    // sub ID returned from startActivityForResult
    public static String RESULT_SUB_ID = "result_sub_id";
    public static final int INVALID_PICK = -1;
    public static final int DATA_PICK = 0;
    public static final int CALLS_PICK = 1;
    public static final int SMS_PICK = 2;
    public static final int PREFERRED_PICK = 3;
    // Show the "select SMS subscription" dialog, but don't save as default, just return a result
    public static final int SMS_PICK_FOR_MESSAGE = 4;

    /// M: Add for SIM settings plugin.
    private static ISimManagementExt mSimManagementExt;
    /// M: Add for supporting SIM hot swap. @{
    private SimHotSwapHandler mSimHotSwapHandler;
    private OnSimHotSwapListener mSimHotSwapListener = new OnSimHotSwapListener() {
        @Override
        public void onSimHotSwap() {
            Log.d(TAG, "onSimHotSwap, finish Activity");
            dismissSimDialog();
            finish();
        }
    };
    /// @}
    /// M: Record the dialog to avoid the window leak.
    private Dialog mDialog;
    /// M: Record the new data SIM id to avoid the screen flash.
    private int mNewDataSubId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;

    /// M: Dismiss the dialog when entering airplane mode. @{
    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.d(TAG, "onReceive, action=" + action);
            dismissSimDialog();
            finish();
        }
    };
    /// @}

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        /// M: Check the SIM related state change.
        setSimStateCheck();

        /// M: Add for SIM settings plugin.
        mSimManagementExt = UtilsExt.getSimManagementExt(getApplicationContext());

        final int dialogType = getIntent().getIntExtra(DIALOG_TYPE_KEY, INVALID_PICK);

        Log.d(TAG, "onCreate, dialogType=" + dialogType);

        /// M: When there is no active subscription, finish activity directly. @{
        final int activeSubCount = SubscriptionManager.from(getApplicationContext())
                .getActiveSubscriptionInfoCount();
        if (activeSubCount == 0) {
            Log.e(TAG, "onCreate, no active sub, finish activity directly.");
            finish();
            return;
        }
        /// @}

        /// M: Add for showing the SIM dialog.
        showSimDialog(dialogType);
    }


    /// M: Add for showing the SIM dialog. @{
    // In some case, the SIM dialog should be updated in onNewIntent().
    private void showSimDialog(int dialogType) {
        switch (dialogType) {
            case DATA_PICK:
            case CALLS_PICK:
            case SMS_PICK:
                /// M: For ALPS02463456, activity state is in chaos. @{
                if (isFinishing()) {
                    Log.e(TAG, "Activity is finishing.");
                }
                /// @}

                /// M: For AlPS02113443, avoid window leak. @{
                // Record the dialog to avoid the window leak.
                mDialog = createDialog(this, dialogType);
                mDialog.show();
                Log.d(TAG, "show selection dialog=" + mDialog);
                /// @}
                break;
            case PREFERRED_PICK:
                /// M: For ALPS02423087, hot plug SIM timing issue. @{
                // The sub list may be already changed.
                List<SubscriptionInfo> subs = SubscriptionManager.from(this)
                        .getActiveSubscriptionInfoList(true);
                if (subs == null || subs.size() != 1) {
                    Log.w(TAG, "Subscription count is not 1, skip preferred SIM dialog");
                    finish();
                    return;
                }
                /// @}

                displayPreferredDialog(getIntent().getIntExtra(PREFERRED_SIM, 0));
                break;
            case SMS_PICK_FOR_MESSAGE:
                /// M: Not support now.
                break;
            default:
                throw new IllegalArgumentException("Invalid dialog type " + dialogType + " sent.");
        }

    }
    /// @}

    private void displayPreferredDialog(final int slotId) {
        final Resources res = getResources();
        final Context context = getApplicationContext();
        final SubscriptionInfo sir = SubscriptionManager.from(context)
                .getActiveSubscriptionInfoForSimSlotIndex(slotId);

        if (sir != null) {
            AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(this);
            alertDialogBuilder.setTitle(R.string.sim_preferred_title);
            alertDialogBuilder.setMessage(res.getString(
                        R.string.sim_preferred_message, sir.getDisplayName()));

            alertDialogBuilder.setPositiveButton(R.string.yes, new
                    DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int id) {
                    final int subId = sir.getSubscriptionId();
                    PhoneAccountHandle phoneAccountHandle =
                            subscriptionIdToPhoneAccountHandle(subId);
                    setDefaultDataSubId(context, subId);
                    setDefaultSmsSubId(context, subId);
                    setUserSelectedOutgoingPhoneAccount(phoneAccountHandle);

                    /// M: Dismiss the dialog before finishing activity to void screen flash.
                    dismissSimDialog();

                    finish();
                }
            });
            alertDialogBuilder.setNegativeButton(R.string.no, new
                    DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog,int id) {
                    /// M: Dismiss the dialog before finishing activity to void screen flash.
                    dismissSimDialog();

                    finish();
                }
            });

            /// M: Finish the activity when the dialog is dismissed. @{
            alertDialogBuilder.setOnDismissListener(new OnDismissListener() {
                @Override
                public void onDismiss(DialogInterface dialog) {
                    finish();
                }
            });
            /// @}

            /// M: For ALPS02422990, avoid window leak. @{
            // Record the dialog to avoid the window leak.
            mDialog = alertDialogBuilder.create();
            mDialog.show();
            Log.d(TAG, "show preferred dialog=" + mDialog);
            /// @}
        } else {
            finish();
        }
    }

    private void setDefaultDataSubId(final Context context, final int subId) {
        Log.d(TAG, "setDefaultDataSubId, sub=" + subId);
        final SubscriptionManager subscriptionManager = SubscriptionManager.from(context);
        final TelephonyManager telephonyManager = TelephonyManager.from(context)
                .createForSubscriptionId(subId);

        /// M: Add for SIM settings plugin. @{
        // Need to call before setDefaultDataSubId.
        mSimManagementExt.setDataState(subId);
        /// @}

        subscriptionManager.setDefaultDataSubId(subId);
        telephonyManager.setDataEnabled(true);

        /// M: Add for printing the CTA logs. @{
        CtaManager ctaManager = CtaManagerFactory.getInstance().makeCtaManager();
        ctaManager.printCtaInfor(context,
                CtaManager.KeywordType.NETWORKCONNECT,
                "SimDialogActivity.setDefaultDataSubId",
                CtaManager.ActionType.ENABLE_MOBILE_NETWORKCONNECT,
                "mobiledata");
        /// @}

        /// M: Add for SIM settings plugin. @{
        // Need to call after setDefaultDataSubId.
        mSimManagementExt.setDataStateEnable(subId);
        /// @}

        /// M: Record the new data SIM id to avoid the screen flash. @{
        // Don't show data switch toast here.
        mNewDataSubId = subId;
        /// @}
    }

    private static void setDefaultSmsSubId(final Context context, final int subId) {
        Log.d(TAG, "setDefaultSmsSubId, sub=" + subId);
        final SubscriptionManager subscriptionManager = SubscriptionManager.from(context);
        subscriptionManager.setDefaultSmsSubId(subId);
    }

    private void setUserSelectedOutgoingPhoneAccount(PhoneAccountHandle phoneAccount) {
        Log.d(TAG, "setUserSelectedOutgoingPhoneAccount, phoneAccount=" + phoneAccount);
        final TelecomManager telecomManager = TelecomManager.from(this);
        telecomManager.setUserSelectedOutgoingPhoneAccount(phoneAccount);
    }

    private PhoneAccountHandle subscriptionIdToPhoneAccountHandle(final int subId) {
        final TelecomManager telecomManager = TelecomManager.from(this);
        final TelephonyManager telephonyManager = TelephonyManager.from(this);
        final Iterator<PhoneAccountHandle> phoneAccounts =
                telecomManager.getCallCapablePhoneAccounts().listIterator();

        Log.d(TAG, "Match phone account, subId=" + subId
                + ", phone account list exist=" + phoneAccounts.hasNext());

        while (phoneAccounts.hasNext()) {
            final PhoneAccountHandle phoneAccountHandle = phoneAccounts.next();
            final PhoneAccount phoneAccount = telecomManager.getPhoneAccount(phoneAccountHandle);
            final int phoneAccountSubId = telephonyManager.getSubIdForPhoneAccount(phoneAccount);
            Log.d(TAG, "Match phone account, phoneAccountSubId=" + phoneAccountSubId
                    + ", phoneAccount=" + phoneAccount);
            if (subId == phoneAccountSubId) {
                return phoneAccountHandle;
            }
        }

        return null;
    }

    public Dialog createDialog(final Context context, final int id) {
        final ArrayList<String> list = new ArrayList<String>();
        final SubscriptionManager subscriptionManager = SubscriptionManager.from(context);
        final List<SubscriptionInfo> subInfoList =
            subscriptionManager.getActiveSubscriptionInfoList(true);
        final int selectableSubInfoLength = subInfoList == null ? 0 : subInfoList.size();

        final DialogInterface.OnClickListener selectionListener =
                new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int value) {
                        /// M: Add for SIM settings plugin. @{
                        // Handle the click event in plugin
                        if (mSimManagementExt.simDialogOnClick(id, value, context)) {
                            Log.d(TAG, "Handle the click event in plugin.");
                            dismissSimDialog();
                            finish();
                            return;
                        }
                        /// @}

                        final SubscriptionInfo sir;

                        switch (id) {
                            case DATA_PICK:
                                sir = subInfoList.get(value);

                                /// M: Add for checking CDMA card competion. @{
                                int targetSub = (sir == null
                                        ? SubscriptionManager.INVALID_SUBSCRIPTION_ID
                                        : sir.getSubscriptionId());
                                if (CdmaUtils.isCdmaCardCompetionForData(context)) {
                                    int defaultId = SubscriptionManager
                                            .getDefaultDataSubscriptionId();
                                    Log.d(TAG, "currnt default subId=" + defaultId
                                            + ", targetId=" + targetSub);

                                    if (defaultId != targetSub) {
                                        if (TelephonyUtils.isInCall()) {
                                            Toast.makeText(context,
                                                    R.string.default_data_switch_err_msg1,
                                                    Toast.LENGTH_SHORT).show();
                                        } else {
                                            setDefaultDataSubId(context, targetSub);
                                        }
                                    }
                                } else {
                                    setDefaultDataSubId(context, targetSub);
                                }
                                /// @}

                                break;
                            case CALLS_PICK:
                                final TelecomManager telecomManager =
                                        TelecomManager.from(context);
                                final List<PhoneAccountHandle> phoneAccountsList =
                                        telecomManager.getCallCapablePhoneAccounts();

                                Log.d(TAG, "value=" + value + ", phoneAccountsList="
                                        + phoneAccountsList);

                                /// M: For ALPS02320816, phone account may be changed. @{
                                // When the phone account is changed in background,
                                // the value may be larger than the phone account size.
                                if (value > phoneAccountsList.size()) {
                                    Log.w(TAG, "phone account changed, do noting."
                                            + " value=" + value + ", phone account size="
                                            + phoneAccountsList.size());
                                    break;
                                }
                                /// @}

                                setUserSelectedOutgoingPhoneAccount(
                                        value < 1 ? null : phoneAccountsList.get(value - 1));
                                break;
                            case SMS_PICK:
                                /// M: Add for supporting SMS always ask. @{
                                int subId = getPickSmsDefaultSub(subInfoList, value);
                                setDefaultSmsSubId(context, subId);
                                /// @}

                                break;
                            default:
                                throw new IllegalArgumentException("Invalid dialog type "
                                        + id + " in SIM dialog.");
                        }

                        /// M: Dismiss the dialog before finishing activity to void screen flash.
                        dismissSimDialog();

                        finish();
                    }
                };

        Dialog.OnKeyListener keyListener = new Dialog.OnKeyListener() {
            @Override
            public boolean onKey(DialogInterface arg0, int keyCode,
                    KeyEvent event) {
                    if (keyCode == KeyEvent.KEYCODE_BACK) {
                        finish();
                    }
                    return true;
                }
            };

        ArrayList<SubscriptionInfo> callsSubInfoList = new ArrayList<SubscriptionInfo>();

        /// M: Add for supporting SMS always ask.
        ArrayList<SubscriptionInfo> smsSubInfoList = new ArrayList<SubscriptionInfo>();

        if (id == CALLS_PICK) {
            final TelecomManager telecomManager = TelecomManager.from(context);
            final TelephonyManager telephonyManager = TelephonyManager.from(context);
            final Iterator<PhoneAccountHandle> phoneAccounts =
                    telecomManager.getCallCapablePhoneAccounts().listIterator();

            /// M: Add for SIM settings plugin. @{
            int accountSize = telecomManager.getCallCapablePhoneAccounts().size();
            mSimManagementExt.updateList(list, callsSubInfoList, accountSize);
            Log.d(TAG, "phone account size=" + accountSize);

            if (accountSize > 1) {
                list.add(getResources().getString(R.string.sim_calls_ask_first_prefs_title));
                callsSubInfoList.add(null);
            }
            /// @}

            while (phoneAccounts.hasNext()) {
                final PhoneAccount phoneAccount =
                        telecomManager.getPhoneAccount(phoneAccounts.next());

                /// M: For ALPS02362894, phoneAccount is null. @{
                // This phone account may be unregistered in the background.
                if (phoneAccount == null) {
                    Log.d(TAG, "phoneAccount is null");
                    continue;
                }
                /// @}

                list.add((String)phoneAccount.getLabel());
                int subId = telephonyManager.getSubIdForPhoneAccount(phoneAccount);
                Log.d(TAG, "phoneAccount label=" + phoneAccount.getLabel()
                        + ", subId=" + subId);

                if (subId != SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
                    final SubscriptionInfo sir = SubscriptionManager.from(context)
                            .getActiveSubscriptionInfo(subId);
                    callsSubInfoList.add(sir);
                } else {
                    callsSubInfoList.add(null);
                }
            }
            Log.d(TAG, "callsSubInfoList=" + callsSubInfoList + ", list=" + list);
        /// M: Add for supporting SMS always ask. @{
        } else if (id == SMS_PICK) {
            setupSmsSubInfoList(list, subInfoList, selectableSubInfoLength, smsSubInfoList);
        /// @}
        } else {
            for (int i = 0; i < selectableSubInfoLength; ++i) {
                final SubscriptionInfo sir = subInfoList.get(i);
                CharSequence displayName = sir.getDisplayName();
                if (displayName == null) {
                    displayName = "";
                }
                list.add(displayName.toString());
            }
        }

        String[] arr = list.toArray(new String[0]);

        AlertDialog.Builder builder = new AlertDialog.Builder(context);

        ListAdapter adapter = new SelectAccountListAdapter(
                /// M: Add for supporting SMS always ask. @{
                getAdapterData(id, subInfoList, callsSubInfoList, smsSubInfoList),
                /// @}
                builder.getContext(),
                R.layout.select_account_list_item,
                arr, id);

        switch (id) {
            case DATA_PICK:
                builder.setTitle(R.string.select_sim_for_data);
                break;
            case CALLS_PICK:
                builder.setTitle(R.string.select_sim_for_calls);
                break;
            case SMS_PICK:
                builder.setTitle(R.string.select_sim_for_sms);
                break;
            case SMS_PICK_FOR_MESSAGE:
                /// M: Not support now.
                break;
            default:
                throw new IllegalArgumentException("Invalid dialog type "
                        + id + " in SIM dialog.");
        }

        Dialog dialog = builder.setAdapter(adapter, selectionListener).create();
        dialog.setOnKeyListener(keyListener);

        dialog.setOnCancelListener(new DialogInterface.OnCancelListener() {
            @Override
            public void onCancel(DialogInterface dialogInterface) {
                finish();
            }
        });

        return dialog;

    }

    private class SelectAccountListAdapter extends ArrayAdapter<String> {
        private Context mContext;
        private int mResId;
        private int mDialogId;
        private final float OPACITY = 0.54f;
        private List<SubscriptionInfo> mSubInfoList;

        public SelectAccountListAdapter(List<SubscriptionInfo> subInfoList,
                Context context, int resource, String[] arr, int dialogId) {
            super(context, resource, arr);
            mContext = context;
            mResId = resource;
            mDialogId = dialogId;
            mSubInfoList = subInfoList;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            LayoutInflater inflater = (LayoutInflater)
                    mContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            View rowView;
            final ViewHolder holder;

            if (convertView == null) {
                // Cache views for faster scrolling
                rowView = inflater.inflate(mResId, null);
                holder = new ViewHolder();
                holder.title = (TextView) rowView.findViewById(R.id.title);
                holder.summary = (TextView) rowView.findViewById(R.id.summary);
                holder.icon = (ImageView) rowView.findViewById(R.id.icon);
                rowView.setTag(holder);
            } else {
                rowView = convertView;
                holder = (ViewHolder) rowView.getTag();
            }

            final SubscriptionInfo sir = mSubInfoList.get(position);
            if (sir == null) {
                holder.title.setText(getItem(position));
                holder.summary.setText("");

                /// M: Display icon for non-sub accounts. @{
                if (mDialogId == CALLS_PICK) {
                    setPhoneAccountIcon(holder, position);
                } else {
                    holder.icon.setImageDrawable(
                            getResources().getDrawable(R.drawable.ic_feedback_24dp));
                }
                /// @}

                /// M: Add for SIM settings plugin. @{
                mSimManagementExt.setCurrNetworkIcon(holder.icon, mDialogId, position);
                /// @}

                holder.icon.setAlpha(OPACITY);
            } else {
                holder.title.setText(sir.getDisplayName());
                holder.summary.setText(sir.getNumber());
                holder.icon.setImageBitmap(sir.createIconBitmap(mContext));

                /// M: When item size is over the screen, should set alpha 1.0f.
                holder.icon.setAlpha(1.0f);
            }
            return rowView;
        }

        private class ViewHolder {
            TextView title;
            TextView summary;
            ImageView icon;
        }

        /// M: Display icon for non-sub accounts. @{
        private void setPhoneAccountIcon(ViewHolder holder, int location) {
            Log.d(TAG, "setPhoneAccountIcon, location=" + location);
            String askFirst = getResources().getString(R.string.sim_calls_ask_first_prefs_title);
            String lableString = getItem(location);
            final TelecomManager telecomManager = TelecomManager.from(mContext);
            List<PhoneAccountHandle> phoneAccountHandles =
                    telecomManager.getCallCapablePhoneAccounts();

            if (!askFirst.equals(lableString)) {
                if (phoneAccountHandles.size() > 1) {
                    location--;
                }

                PhoneAccount phoneAccount = null;
                if (location >= 0 && location < phoneAccountHandles.size()) {
                    phoneAccount = telecomManager.getPhoneAccount(
                            phoneAccountHandles.get(location));
                }
                Log.d(TAG, "setPhoneAccountIcon(), location=" + location
                        + ", account="  + phoneAccount);
                /// M: ALPS04017179, PhoneAccountUtils in CtsVerifier does not add icon
                if (phoneAccount != null && phoneAccount.getIcon() != null) {
                    holder.icon.setImageDrawable(
                            phoneAccount.getIcon().loadDrawable(mContext));
                }
            } else {
                holder.icon.setImageDrawable(
                        getResources().getDrawable(R.drawable.ic_feedback_24dp));
            }
        }
        /// @}
    }

    /// M: Check the SIM related state change. @{
    private void setSimStateCheck() {
        // Add for supporting SIM hot swap. @{
        mSimHotSwapHandler = new SimHotSwapHandler(getApplicationContext());
        mSimHotSwapHandler.registerOnSimHotSwap(mSimHotSwapListener);
        /// @}

        // Dismiss the dialog when entering airplane mode.
        IntentFilter itentFilter = new IntentFilter(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        registerReceiver(mReceiver, itentFilter);
    }

    private void unsetSimStateCheck() {
        // Add for supporting SIM hot swap. @{
        mSimHotSwapHandler.unregisterOnSimHotSwap();
        mSimHotSwapHandler = null;
        /// @}

        // Dismiss the dialog when entering airplane mode.
        unregisterReceiver(mReceiver);
    }
    /// @}

    /// M: Update the intent to handle the dialog type changed case. @{
    @Override
    protected void onNewIntent(Intent intent) {
        final int dialogType = intent.getIntExtra(DIALOG_TYPE_KEY, INVALID_PICK);
        Log.d(TAG, "onNewIntent, dialogType=" + dialogType);
        setIntent(intent);
        if (mDialog != null && mDialog.isShowing()) {
            // Clean the cancel and dismiss listener, then dismiss the dialog.
            mDialog.setOnCancelListener(null);
            mDialog.setOnDismissListener(null);
            mDialog.dismiss();
            mDialog = null;

            // Unregister the previous SIM hot swap handler,
            // then create the new handler and register the listener again.
            mSimHotSwapHandler.unregisterOnSimHotSwap();
            mSimHotSwapHandler = new SimHotSwapHandler(getApplicationContext());
            mSimHotSwapHandler.registerOnSimHotSwap(mSimHotSwapListener);

            // Show the dialog with the new type again.
            showSimDialog(dialogType);

            Log.d(TAG, "onNewIntent, recreate the SIM dialog.");
        }
    }
    /// @}

    @Override
    protected void onDestroy() {
        Log.d(TAG, "onDestroy");

        unsetSimStateCheck();

        /// M: For AlPS02113443, avoid window leak.
        dismissSimDialog();

        /// M: Show data switch toast to avoid the screen flash. @{
        if (mNewDataSubId != SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
            Toast.makeText(this, R.string.data_switch_started, Toast.LENGTH_LONG).show();
        }
        /// @}

        super.onDestroy();
    }

    /// M: Add for supporting SMS always ask. @{
    private int getPickSmsDefaultSub(final List<SubscriptionInfo> subInfoList, int value) {
        int subId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;
        if (value < 1) {
            int length = subInfoList == null ? 0 : subInfoList.size();
            if (length == 1) {
                subId = subInfoList.get(value).getSubscriptionId();
            } else {
                subId = MtkDefaultSmsSimSettings.ASK_USER_SUB_ID;
            }
        } else if (value >= 1 && value < subInfoList.size() + 1) {
            subId = subInfoList.get(value - 1).getSubscriptionId();
        }
        subId = mSimManagementExt.getDefaultSmsClickContentExt(subInfoList, value, subId);
        Log.d(TAG, "getPickSmsDefaultSub, value=" + value + ", subId=" + subId);
        return subId;
    }

    private void setupSmsSubInfoList(final ArrayList<String> list,
            final List<SubscriptionInfo> subInfoList,
            final int selectableSubInfoLength,
            ArrayList<SubscriptionInfo> smsSubInfoList) {
        // Add for SIM settings plugin.
        mSimManagementExt.updateList(list, smsSubInfoList,  selectableSubInfoLength);

        if ((selectableSubInfoLength > 1)
                && (mSimManagementExt.isNeedAskFirstItemForSms())) {
            list.add(getResources().getString(R.string.sim_calls_ask_first_prefs_title));
            smsSubInfoList.add(null);
        }
        for (int i = 0; i < selectableSubInfoLength; ++i) {
            final SubscriptionInfo sir = subInfoList.get(i);
            smsSubInfoList.add(sir);
            CharSequence displayName = sir.getDisplayName();
            if (displayName == null) {
                displayName = "";
            }
            list.add(displayName.toString());
        }
    }

    private List<SubscriptionInfo> getAdapterData(final int id,
            final List<SubscriptionInfo> subInfoList,
            ArrayList<SubscriptionInfo> callsSubInfoList,
            ArrayList<SubscriptionInfo> smsSubInfoList) {
        List<SubscriptionInfo> listForAdpter = null;
        switch (id) {
            case DATA_PICK:
                listForAdpter = subInfoList;
                break;
            case CALLS_PICK:
                listForAdpter = callsSubInfoList;
                break;
            case SMS_PICK:
                listForAdpter = smsSubInfoList;
                break;
            default:
                Log.e(TAG, "Invalid dialog type=" + id);
                break;
        }
        return listForAdpter;
    }
    /// @}

    /// M: Dismiss the dialog before finishing activity to void screen flash. @{
    private void dismissSimDialog() {
        if (mDialog != null && mDialog.isShowing()) {
            mDialog.dismiss();
            mDialog = null;
        }
    }
    /// @}
}
