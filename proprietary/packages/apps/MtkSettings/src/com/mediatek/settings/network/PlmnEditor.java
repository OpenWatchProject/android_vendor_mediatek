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

package com.mediatek.settings.network;

import android.app.Activity;
import android.app.Dialog;
import android.app.settings.SettingsEnums;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.DialogInterface.OnShowListener;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.telephony.PhoneStateListener;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.Editable;
import android.text.InputFilter;
import android.text.InputType;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.EditText;

import androidx.appcompat.app.AlertDialog;
import androidx.preference.MultiSelectListPreference;
import androidx.preference.Preference;
import androidx.preference.Preference.OnPreferenceChangeListener;

import com.android.internal.telephony.PhoneConstants;
import com.android.settings.R;
import com.android.settings.SettingsPreferenceFragment;
/// M: Add for checking the phone/SIM states.
import com.android.settings.network.telephony.MobileNetworkUtils;
import com.android.settingslib.core.instrumentation.Instrumentable;

import com.mediatek.internal.telephony.NetworkInfoWithAcT;
import com.mediatek.settings.sim.SimHotSwapHandler;
import com.mediatek.settings.sim.SimHotSwapHandler.OnSimHotSwapListener;
import com.mediatek.settings.sim.TelephonyUtils;

import java.util.HashSet;
import java.util.Set;

public class PlmnEditor extends SettingsPreferenceFragment
        implements OnPreferenceChangeListener, TextWatcher {

    public static final String TAG = "PlmnEditor";

    public static final String ACTION_PLMN_INSERT = "mediatek.settings.PLMN_INSERT";
    public static final String ACTION_PLMN_EDIT = "mediatek.settings.PLMN_EDIT";

    public static final String PLMN_ALPHA_NAME = "alpha_name";
    public static final String PLMN_NUMERIC = "numeric";
    public static final String PLMN_ACT = "act";
    public static final String PLMN_PRIORITY = "priority";
    public static final String PLMN_SUB = "sub_id";

    public static final int RESULT_ADD = 100;
    public static final int RESULT_MODIFY = 200;
    public static final int RESULT_DELETE = 300;
    public static final int RESULT_CANCEL = 400;

    public static final int RIL_NONE = 0x0;
    public static final int RIL_NONE2 = 0x2;
    public static final int RIL_2G = 0x1;
    public static final int RIL_3G = 0x4;
    public static final int RIL_4G = 0x8;
    public static final int RIL_2G_3G = 0x5;
    public static final int RIL_2G_4G = 0x9;
    public static final int RIL_3G_4G = 0xC;
    public static final int RIL_2G_3G_4G = 0xD;

    private static final String PREF_KEY_NETWORK_ID = "plmn_network_id";
    private static final String PREF_KEY_NETWORK_TYPE = "plmn_network_type";

    private static final int MENU_DELETE = Menu.FIRST;
    private static final int MENU_SAVE = Menu.FIRST + 1;
    private static final int MENU_CANCEL = Menu.FIRST + 2;

    private static final int DIALOG_NETWORK_ID = 1;

    private static final int DEFAULT_ACT = 0;
    private static final int DEFAULT_PRIORITY = 0;

    private String mNotSet;

    private Preference mNetworkId;
    private EditText mNetworkIdEditText = null;
    private AlertDialog mNetworkIdDialog = null;
    private String mNetworkIdValue = null;
    private MultiSelectListPreference mNetworkType;

    private int mSubId;
    private int mPhoneId;
    private String mInitNumeric;
    private int mInitAct;
    private String mAlphaName;
    private String mNumeric;
    private int mAct;
    private int mPriority;

    private boolean mNewPlmn = false;
    private boolean mRegistered = false;
    private boolean m4gEnabled;

    private TelephonyManager mTelephonyManager;

    private IntentFilter mIntentFilter;

    private boolean mEnabled = false;
    private boolean mRadioOn = false;

    private SimHotSwapHandler mSimHotSwapHandler = null;
    private boolean mSimOnOffEnabled = false;
    private int mCurSimState = TelephonyUtils.SIM_ONOFF_STATE_ON;

    private PhoneChangeReceiver mPhoneChangeReceiver = new PhoneChangeReceiver();
    private PhoneStateListener mPhoneStateListener = new PhoneStateListener() {
        @Override
        public void onRadioPowerStateChanged(int state) {
            Log.d(TAG, "PhoneStateListener.onRadioPowerStateChanged, state=" + state
                    + ", subId=" + mSubId);
            boolean radioOn = (state == TelephonyManager.RADIO_POWER_ON);
            if (radioOn != mRadioOn) {
                mRadioOn = radioOn;
                updateScreenStatus();
            }
        }
    };

    private OnClickListener mNetworkIdClickListener = new OnClickListener() {
        @Override
        public void onClick(DialogInterface dialog, int which) {
            if (which == DialogInterface.BUTTON_POSITIVE) {
                mNetworkIdValue = mNetworkIdEditText.getText().toString();
                mNetworkId.setSummary(checkNull(mNetworkIdValue));
                getActivity().invalidateOptionsMenu();
            }
        }
    };

    private OnShowListener mNetworkIdShowListener = new OnShowListener() {
        @Override
        public void onShow(DialogInterface dialog) {
            validateNetworkId();
        }
    };

    @Override
    public int getMetricsCategory() {
        return SettingsEnums.MOBILE_NETWORK_RENAME_DIALOG;
    }

    @Override
    public int getDialogMetricsCategory(int dialogId) {
        if (dialogId == DIALOG_NETWORK_ID) {
            return SettingsEnums.MOBILE_NETWORK_RENAME_DIALOG;
        }
        return Instrumentable.METRICS_CATEGORY_UNKNOWN;
    }

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        addPreferencesFromResource(R.xml.plmn_editor);

        mNotSet = getResources().getString(R.string.apn_not_set);

        mNetworkId = findPreference(PREF_KEY_NETWORK_ID);
        mNetworkType = (MultiSelectListPreference) findPreference(PREF_KEY_NETWORK_TYPE);

        final Intent intent = getIntent();
        final String action = intent.getAction();

        mSubId = intent.getIntExtra(PLMN_SUB,
                SubscriptionManager.INVALID_SUBSCRIPTION_ID);
        mPhoneId = SubscriptionManager.getPhoneId(mSubId);
        mAlphaName = intent.getStringExtra(PLMN_ALPHA_NAME);
        mNumeric = intent.getStringExtra(PLMN_NUMERIC);
        mAct = updateAct(intent.getIntExtra(PLMN_ACT, DEFAULT_ACT));
        mPriority = intent.getIntExtra(PLMN_PRIORITY, DEFAULT_PRIORITY);

        mInitNumeric = mNumeric;
        mInitAct = mAct;

        if (!SubscriptionManager.isValidSubscriptionId(mSubId)) {
            Log.e(TAG, "onCreate, invalid subId=" + mSubId);
            finish();
            return;
        }

        mTelephonyManager = TelephonyManager.from(getActivity()).createForSubscriptionId(mSubId);
        mRadioOn = TelephonyUtils.isRadioOn(mSubId, getActivity());

        if (action.equals(ACTION_PLMN_INSERT)) {
            mNewPlmn = true;
        } else if (action.equals(ACTION_PLMN_EDIT)) {
        } else {
            Log.d(TAG, "onCreate, invalid action=" + action);
            finish();
            return;
        }

        m4gEnabled = TelephonyUtils.isMtkLteSupported() && MobileNetworkUtils.isUsim(mSubId, mPhoneId);

        registerPhoneChangeReceiver();

        mSimHotSwapHandler = new SimHotSwapHandler(getActivity());
        mSimHotSwapHandler.registerOnSimHotSwap(new OnSimHotSwapListener() {
            @Override
            public void onSimHotSwap() {
                if (getActivity() != null) {
                    Log.d(TAG, "onSimHotSwap, finish activity.");
                    getActivity().finish();
                }
            }
        });

        mSimOnOffEnabled = TelephonyUtils.isSimOnOffEnabled();
        int slotId = SubscriptionManager.getSlotIndex(mSubId);
        mCurSimState = TelephonyUtils.getSimOnOffState(slotId);
        Log.d(TAG, "onCreate, simOnOffEnabled=" + mSimOnOffEnabled
                + ", slotId=" + slotId + ", subId=" + mSubId
                + ", simState=" + mCurSimState);

        for (int i = 0; i < getPreferenceScreen().getPreferenceCount(); i++) {
            Preference preference = getPreferenceScreen().getPreference(i);
            preference.setOnPreferenceChangeListener(this);
        }

        fillUI();
    }

    private void fillUI() {
        mNetworkIdValue = mNumeric;
        mNetworkId.setSummary(checkNull(mNetworkIdValue));

        HashSet<String> acts = new HashSet<String>();
        if ((mAct & RIL_2G) != 0) {
            acts.add("" + RIL_2G);
        }
        if ((mAct & RIL_3G) != 0) {
            acts.add("" + RIL_3G);
        }
        if ((mAct & RIL_4G) != 0) {
            acts.add("" + RIL_4G);
        }
        mNetworkType.setValues(acts);
        String type = buildNetworkTypeDescription(acts);
        mNetworkType.setSummary(type);
    }

    private String buildNetworkTypeDescription(Set<String> acts) {
        String[] values = getResources().getStringArray(R.array.plmn_network_type_entries);
        StringBuilder retVal = new StringBuilder();
        boolean first = true;
        for (String act : acts) {
            int index = mNetworkType.findIndexOfValue(act);
            try {
                if (first) {
                    retVal.append(values[index]);
                    first = false;
                } else {
                    retVal.append("/" + values[index]);
                }
            } catch (ArrayIndexOutOfBoundsException e) {
                // ignore
            }
        }
        String val = retVal.toString();
        if (!TextUtils.isEmpty(val)) {
            return val;
        }
        return null;
    }

    private int buildNetworkTypeAct(Set<String> acts) {
        String[] values = getResources().getStringArray(R.array.plmn_network_type_values);
        int result = 0;
        for (String act : acts) {
            int index = mNetworkType.findIndexOfValue(act);
            try {
                int actValue = Integer.valueOf(values[index]);
                result += actValue;
            } catch (NumberFormatException e) {
                // ignore
            }
        }
        return result;
    }

    private int updateAct(int act) {
        if (act == RIL_NONE || act == RIL_NONE2) {
            act = RIL_2G;
        }
        return act;
    }

    private String checkNull(String value) {
        return TextUtils.isEmpty(value) ? mNotSet : value;
    }

    private String checkNotSet(String value) {
        return mNotSet.equals(value) ? null : value;
    }

    @Override
    public boolean onPreferenceTreeClick(Preference preference) {
        String key = preference.getKey();
        Log.d(TAG, "onPreferenceClick, key=" + key);
        if (PREF_KEY_NETWORK_ID.equals(key)) {
            removeDialog(DIALOG_NETWORK_ID);
            showDialog(DIALOG_NETWORK_ID);
            return true;
        }
        return false;
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        String key = preference.getKey();
        Log.d(TAG, "onPreferenceChange, key=" + key);
        if (PREF_KEY_NETWORK_ID.equals(key)) {
            mNetworkId.setSummary(checkNull(
                    newValue != null ? String.valueOf(newValue) : null));
        } else if (PREF_KEY_NETWORK_TYPE.equals(key)) {
            String type = buildNetworkTypeDescription((Set<String>) newValue);
            if (type == null) {
                return false;
            }
            mNetworkType.setValues((Set<String>) newValue);
            mNetworkType.setSummary(type);
        }
        return true;
    }

    @Override
    public Dialog onCreateDialog(int id) {
        if (id == DIALOG_NETWORK_ID) {
            final Activity activity = getActivity();
            final LayoutInflater inflater = LayoutInflater.from(activity);
            final View view = inflater.inflate(R.layout.dialog_edittext, null);
            mNetworkIdEditText = (EditText) view.findViewById(R.id.edittext);
            if (!mNotSet.equals(mNetworkId.getSummary())) {
                mNetworkIdEditText.setText(mNetworkId.getSummary());
                int len = mNetworkIdEditText.getText().toString().length();
                mNetworkIdEditText.setSelection(len);
            }
            mNetworkIdEditText.addTextChangedListener(this);
            mNetworkIdEditText.setInputType(InputType.TYPE_CLASS_NUMBER);
            mNetworkIdEditText.setFilters(new InputFilter[] {new InputFilter.LengthFilter(6)});
            mNetworkIdEditText.setFocusable(true);
            mNetworkIdEditText.setFocusableInTouchMode(true);
            mNetworkIdEditText.requestFocus();
            mNetworkIdDialog = new AlertDialog.Builder(activity)
                    .setTitle(activity.getResources().getString(R.string.plmn_network_id))
                    .setView(view)
                    .setPositiveButton(activity.getResources().getString(
                            com.android.internal.R.string.ok), mNetworkIdClickListener)
                    .setNegativeButton(activity.getResources().getString(
                            com.android.internal.R.string.cancel), null)
                    .create();
            mNetworkIdDialog.setOnShowListener(mNetworkIdShowListener);
            return mNetworkIdDialog;
        }

        return null;
    }

    @Override
    public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
        super.onCreateOptionsMenu(menu, inflater);

        if (!mNewPlmn) {
            menu.add(0, MENU_DELETE, 0, com.android.internal.R.string.delete)
                    .setIcon(R.drawable.ic_delete);
        }
        menu.add(0, MENU_SAVE, 0, R.string.menu_save)
            .setIcon(android.R.drawable.ic_menu_save);
        menu.add(0, MENU_CANCEL, 0, R.string.menu_cancel)
            .setIcon(android.R.drawable.ic_menu_close_clear_cancel);
    }

    @Override
    public void onPrepareOptionsMenu(Menu menu) {
        super.onPrepareOptionsMenu(menu);

        final Activity activity = getActivity();
        boolean inCall = TelephonyUtils.isInCall();
        boolean enabled = !inCall && mRadioOn
                && (mCurSimState == TelephonyUtils.SIM_ONOFF_STATE_ON);
        boolean isEmpty = mNotSet.equals(mNetworkId.getSummary());
        if (menu != null) {
            menu.setGroupEnabled(0, enabled);
            menu.findItem(MENU_SAVE).setEnabled(enabled && !isEmpty);
        }
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case MENU_DELETE:
                deletePlmn();
                finish();
                return true;

            case MENU_SAVE:
                if (validateAndSavePlmnData()) {
                    finish();
                }
                return true;

            case MENU_CANCEL:
                cancelPlmn();
                finish();
                return true;

            default:
                return super.onOptionsItemSelected(item);
        }
    }

    @Override
    public void onDestroy() {
        unregisterPhoneChangeReceiver();

        if (mSimHotSwapHandler != null) {
            mSimHotSwapHandler.unregisterOnSimHotSwap();
            mSimHotSwapHandler = null;
        }

        super.onDestroy();
    }

    @Override
    public void afterTextChanged(Editable s) {
        validateNetworkId();
    }

    @Override
    public void beforeTextChanged(CharSequence s, int start, int count,
              int after) {
        // work done in afterTextChanged
    }

    @Override
    public void onTextChanged(CharSequence s, int start, int before, int count) {
        // work done in afterTextChanged
    }

    public void validateNetworkId() {
        int len = mNetworkIdEditText.getText().toString().length();
        mNetworkIdEditText.setSelection(len);
        if (mNetworkIdDialog != null) {
            mNetworkIdDialog.getButton(DialogInterface.BUTTON_POSITIVE).setEnabled(len >= 5);
        }
    }

    private void genNetworkInfo(Intent intent) {
        intent.putExtra(PlmnEditor.PLMN_ALPHA_NAME, mAlphaName);
        intent.putExtra(PlmnEditor.PLMN_NUMERIC, mNetworkIdValue);
        intent.putExtra(PlmnEditor.PLMN_PRIORITY, mPriority);
        intent.putExtra(PlmnEditor.PLMN_ACT,
                buildNetworkTypeAct(mNetworkType.getValues()));
    }

    private void deletePlmn() {
        Intent intent = new Intent();
        genNetworkInfo(intent);
        setResult(RESULT_DELETE, intent);
    }

    private boolean validateAndSavePlmnData() {
        Intent intent = new Intent();
        genNetworkInfo(intent);
        if (mNewPlmn) {
            setResult(RESULT_ADD, intent);
        } else {
            setResult(RESULT_MODIFY, intent);
        }
        return true;
    }

    private void cancelPlmn() {
        Intent intent = new Intent();
        setResult(RESULT_CANCEL, intent);
    }

    private void registerPhoneChangeReceiver() {
        final IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        intentFilter.addAction(TelephonyManager.ACTION_PHONE_STATE_CHANGED);
        intentFilter.addAction(TelephonyManager.ACTION_SIM_APPLICATION_STATE_CHANGED);

        getActivity().registerReceiver(mPhoneChangeReceiver, intentFilter);

        mTelephonyManager.listen(mPhoneStateListener,
                PhoneStateListener.LISTEN_RADIO_POWER_STATE_CHANGED);

        mRegistered = true;
    }

    private void unregisterPhoneChangeReceiver() {
        if (mRegistered) {
            getActivity().unregisterReceiver(mPhoneChangeReceiver);
            mTelephonyManager.listen(mPhoneStateListener, PhoneStateListener.LISTEN_NONE);
            mRegistered = false;
        }
    }

    private void updateScreenStatus() {
        final Activity activity = getActivity();
        boolean inCall = TelephonyUtils.isInCall();
        boolean enabled = !inCall && mRadioOn
                && (mCurSimState == TelephonyUtils.SIM_ONOFF_STATE_ON);
        Log.d(TAG, "updateScreenStatus, subId=" + mSubId
                + ", enabled=" + enabled + ", inCall=" + inCall
                + ", radioOn=" + mRadioOn + ", simState=" + mCurSimState);

        if (enabled != mEnabled) {
            getPreferenceScreen().setEnabled(enabled);
            activity.invalidateOptionsMenu();
        }

        mEnabled = enabled;
    }

    private void handleSimApplicationStateChange(Intent intent) {
        if (!mSimOnOffEnabled) {
            Log.d(TAG, "handleSimApplicationStateChange, SIM On/Off is not enabled.");
            return;
        }

        Bundle extra = intent.getExtras();
        if (extra == null) {
            Log.d(TAG, "handleSimApplicationStateChange, extra=null");
            return;
        }

        int slotId = extra.getInt(PhoneConstants.SLOT_KEY,
                SubscriptionManager.INVALID_SIM_SLOT_INDEX);
        int subId = extra.getInt(PhoneConstants.SUBSCRIPTION_KEY,
                SubscriptionManager.INVALID_SIM_SLOT_INDEX);
        int state = TelephonyUtils.getSimOnOffState(slotId);
        if (SubscriptionManager.isValidSubscriptionId(subId)
                && (mSubId == subId) && (mCurSimState != state)) {
            Log.d(TAG, "handleSimApplicationStateChange, subId=" + subId
                    + ", preState=" + mCurSimState + ", curState=" + state);
            mCurSimState = state;
            updateScreenStatus();
        }
    }

    private class PhoneChangeReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.d(TAG, "onReceive, action=" + action);

            if (action.equals(Intent.ACTION_AIRPLANE_MODE_CHANGED)) {
                finish();
            } else if (action.equals(TelephonyManager.ACTION_PHONE_STATE_CHANGED)) {
                updateScreenStatus();
            } else if (action.equals(TelephonyManager.ACTION_SIM_APPLICATION_STATE_CHANGED)) {
                handleSimApplicationStateChange(intent);
            }
        }
    }
}
