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
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.UserManager;
import android.telephony.PhoneStateListener;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;

import androidx.preference.Preference;
import androidx.preference.PreferenceScreen;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.settings.R;
import com.android.settings.RestrictedSettingsFragment;
/// M: Add for checking the phone/SIM states.
import com.android.settings.network.telephony.MobileNetworkUtils;
import com.android.settingslib.core.instrumentation.Instrumentable;

import com.mediatek.internal.telephony.MtkGsmCdmaPhone;
import com.mediatek.internal.telephony.NetworkInfoWithAcT;
import com.mediatek.settings.sim.SimHotSwapHandler;
import com.mediatek.settings.sim.SimHotSwapHandler.OnSimHotSwapListener;
import com.mediatek.settings.sim.SimOnOffSwitchHandler;
import com.mediatek.settings.sim.SimOnOffSwitchHandler.OnSimOnOffSwitchListener;
import com.mediatek.settings.sim.TelephonyUtils;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashSet;
import java.util.Iterator;

public class PlmnSettings extends RestrictedSettingsFragment
        implements Preference.OnPreferenceChangeListener {

    public static final String TAG = "PlmnSettings";

    public static final String SUB_ID = "sub_id";

    private static final int REQUEST_ADD = 100;
    private static final int REQUEST_EDIT = 200;

    private static final int MENU_NEW = Menu.FIRST;

    private static final int MESSAGE_GET_PLMN_LIST = 0;
    private static final int MESSAGE_SET_PLMN_LIST = 1;

    private static final String PREF_KEY_PLMN_SETTINGS = "plmn_settings";

    private int mSubId;
    private int mPhoneId;
    private MtkGsmCdmaPhone mPhone;
    private TelephonyManager mTelephonyManager;
    private boolean mEnabled = false;
    private boolean mRadioOn = false;

    private MyHandler mHandler = new MyHandler();
    private IntentFilter mIntentFilter;

    private boolean mRegistered = false;
    private boolean m4gEnabled;

    private PreferenceScreen mPlmnListScreen;
    private View mProgressHeader;

    private ArrayList<NetworkInfoWithAcT> mPlmnList = new ArrayList<NetworkInfoWithAcT>();
    private int mPlmnNum;

    private SimHotSwapHandler mSimHotSwapHandler = null;
    private boolean mSimOnOffEnabled;
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

    public PlmnSettings() {
        super(UserManager.DISALLOW_CONFIG_MOBILE_NETWORKS);
    }

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.d(TAG, "onReceive, action=" + action);
            if (action.equals(Intent.ACTION_AIRPLANE_MODE_CHANGED)) {
                final Activity activity = getActivity();
                if (activity != null) {
                    Log.d(TAG, "onReceive, finish Activity for airplane mode.");
                    activity.finish();
                }
            }
        }
    };

    @Override
    public int getMetricsCategory() {
        return Instrumentable.METRICS_CATEGORY_UNKNOWN;
    }

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        final Activity activity = getActivity();
        mSubId = activity.getIntent().getIntExtra(SUB_ID,
                SubscriptionManager.INVALID_SUBSCRIPTION_ID);
        mPhoneId = SubscriptionManager.getPhoneId(mSubId);

        if (!SubscriptionManager.isValidSubscriptionId(mSubId)) {
            Log.e(TAG, "onCreate, invalid subId=" + mSubId);
            activity.finish();
            return;
        }

        Phone phone = PhoneFactory.getPhone(SubscriptionManager.getPhoneId(mSubId));
        if (phone == null || !(phone instanceof MtkGsmCdmaPhone)) {
            Log.e(TAG, "onCreate, invalid phone=" + (phone == null ? "null" : phone));
            activity.finish();
            return;
        }
        mPhone = (MtkGsmCdmaPhone) phone;
        mTelephonyManager = TelephonyManager.from(activity).createForSubscriptionId(mSubId);
        mRadioOn = TelephonyUtils.isRadioOn(mSubId, activity);

        m4gEnabled = TelephonyUtils.isMtkLteSupported() && MobileNetworkUtils.isUsim(mSubId, mPhoneId);

        registerPhoneChangeReceiver();

        mPlmnListScreen = (PreferenceScreen) findPreference(PREF_KEY_PLMN_SETTINGS);

        mSimHotSwapHandler = new SimHotSwapHandler(activity);
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
    }

    @Override
    public void onViewCreated(View view, Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);

        final Activity activity = getActivity();
        if (activity != null) {
            mProgressHeader = setPinnedHeaderView(R.layout.progress_header)
                    .findViewById(R.id.progress_bar_animation);
            setProgressBarVisible(false);
        }
    }

    @Override
    public void onStart() {
        super.onStart();

        setProgressBarVisible(true);

        mPhone.getPol(mHandler.obtainMessage(MESSAGE_GET_PLMN_LIST));
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
    protected int getPreferenceScreenResId() {
        return R.xml.plmn_settings;
    }

    @Override
    public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
        menu.add(0, MENU_NEW, 0,
                getResources().getString(R.string.menu_new))
                .setIcon(R.drawable.ic_add_24dp)
                .setShowAsAction(MenuItem.SHOW_AS_ACTION_IF_ROOM);

        super.onCreateOptionsMenu(menu, inflater);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case MENU_NEW:
                addNewPlmn();
                return true;

            default:
                break;
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    public boolean onPreferenceTreeClick(Preference preference) {
        int index = preference.getOrder();
        NetworkInfoWithAcT info = mPlmnList.get(index);
        Intent intent = getIntentFromNetworkInfo(info);
        startActivityForResult(intent, REQUEST_EDIT);
        return true;
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        return true;
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent intent) {
        if (mPlmnList != null && intent != null
                && TelephonyUtils.isSimStateReady(mSubId)) {
            NetworkInfoWithAcT info = createNetworkInfo(intent);
            switch (resultCode) {
                case PlmnEditor.RESULT_ADD:
                    handlePlmnAdd(info);
                    break;
                case PlmnEditor.RESULT_MODIFY:
                    handlePlmnEdit(info);
                    break;
                case PlmnEditor.RESULT_DELETE:
                    handlePlmnDelete(info);
                    break;
                case PlmnEditor.RESULT_CANCEL:
                    break;
                default:
                    break;
            }
        }
    }

    private void adjustPriority(ArrayList<NetworkInfoWithAcT> list) {
        int priority = 0;
        for (NetworkInfoWithAcT info : list) {
            info.setPriority(priority++);
        }
    }

    private NetworkInfoWithAcT createNetworkInfo(Intent intent) {
        String alphaName = intent.getStringExtra(PlmnEditor.PLMN_ALPHA_NAME);
        String numeric = intent.getStringExtra(PlmnEditor.PLMN_NUMERIC);
        int priority = intent.getIntExtra(PlmnEditor.PLMN_PRIORITY, 0);
        int act = intent.getIntExtra(PlmnEditor.PLMN_ACT, 0);
        return new NetworkInfoWithAcT(alphaName, numeric, act, priority);
    }

    private void handlePlmnAdd(NetworkInfoWithAcT info) {
        Log.d(TAG, "handlePlmnAdd, info=" + info);
        mPlmnList.add(0, info);
        adjustPriority(mPlmnList);
        setPlmn(mPlmnList);
    }

    private void handlePlmnEdit(NetworkInfoWithAcT info) {
        Log.d(TAG, "handlePlmnEdit, info=" + info);
        NetworkInfoWithAcT tempInfo = mPlmnList.get(info.getPriority());
        tempInfo.setOperatorAlphaName(info.getOperatorAlphaName());
        tempInfo.setOperatorNumeric(info.getOperatorNumeric());
        tempInfo.setAccessTechnology(info.getAccessTechnology());
        setPlmn(mPlmnList);
    }

    private void handlePlmnDelete(NetworkInfoWithAcT info) {
        Log.d(TAG, "handlePlmnDelete, info=" + info);

        int oldLen = mPlmnList.size();
        mPlmnList.remove(info.getPriority());
        mPlmnList.add(new NetworkInfoWithAcT("", "", 1, mPlmnList.size()));
        adjustPriority(mPlmnList);
        setPlmn(mPlmnList);
    }

    private void setPlmn(ArrayList<NetworkInfoWithAcT> list) {
        mPlmnNum = list.size();
        for (int i = 0; i < list.size(); i++) {
            NetworkInfoWithAcT act = list.get(i);
            Log.d(TAG, "setPLMN, act[" + i + "]=" + act);
            mPhone.setPolEntry(act, mHandler.obtainMessage(
                    MESSAGE_SET_PLMN_LIST));
        }
        setProgressBarVisible(true);
    }

    private void setProgressBarVisible(boolean visible) {
        if (mProgressHeader != null) {
            mProgressHeader.setVisibility(visible ? View.VISIBLE : View.GONE);
        }
    }

    private void addNewPlmn() {
        Log.d(TAG, "addNewPlmn");

        Intent intent = new Intent(PlmnEditor.ACTION_PLMN_INSERT);
        intent.putExtra(SUB_ID, mSubId);
        startActivityForResult(intent, REQUEST_ADD);
    }

    private Intent getIntentFromNetworkInfo(NetworkInfoWithAcT info) {
        Intent intent = new Intent(PlmnEditor.ACTION_PLMN_EDIT);
        intent.putExtra(PlmnEditor.PLMN_ALPHA_NAME, info.getOperatorAlphaName());
        intent.putExtra(PlmnEditor.PLMN_NUMERIC, info.getOperatorNumeric());
        intent.putExtra(PlmnEditor.PLMN_ACT, info.getAccessTechnology());
        intent.putExtra(PlmnEditor.PLMN_PRIORITY, info.getPriority());
        intent.putExtra(PlmnEditor.PLMN_SUB, mSubId);
        return intent;
    }

    public int findActIndexOfValue(String[] values, String value) {
        if (values != null && value != null) {
            for (int i = values.length - 1; i >= 0; i--) {
                if (values[i].equals(value)) {
                    return i;
                }
            }
        }
        return -1;
    }

    private String buildNetworkTypeDescription(int act) {
        HashSet<String> acts = new HashSet<String>();
        if ((act & PlmnEditor.RIL_2G) != 0) {
            acts.add("" + PlmnEditor.RIL_2G);
        }
        if ((act & PlmnEditor.RIL_3G) != 0) {
            acts.add("" + PlmnEditor.RIL_3G);
        }
        if ((act & PlmnEditor.RIL_4G) != 0) {
            acts.add("" + PlmnEditor.RIL_4G);
        }

        String[] entries = getResources().getStringArray(R.array.plmn_network_type_entries);
        String[] values = getResources().getStringArray(R.array.plmn_network_type_values);
        StringBuilder retVal = new StringBuilder();
        boolean first = true;
        for (String entry : acts) {
            int index = findActIndexOfValue(values, entry);
            try {
                if (first) {
                    retVal.append(entries[index]);
                    first = false;
                } else {
                    retVal.append("/" + entries[index]);
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

    private String getPlmnPreferenceName(NetworkInfoWithAcT info) {
        int act = info.getAccessTechnology();
        String plmnName = info.getOperatorAlphaName();
        String actName = buildNetworkTypeDescription(act);
        String prefName;
        if (actName != null && !actName.isEmpty()) {
            if (plmnName != null && !plmnName.isEmpty()) {
                prefName = plmnName + " (" + actName + ")";
            } else {
                prefName = "(" + actName + ")";
            }
        } else {
            prefName = (plmnName == null ? "" : plmnName);
        }
        return prefName;
    }

    private ArrayList<NetworkInfoWithAcT> filterPlmnList(
            ArrayList<NetworkInfoWithAcT> plmnList) {
        final boolean lteSupported = TelephonyUtils.isMtkLteSupported();
        Iterator itr = plmnList.iterator();
        while (itr.hasNext()) {
            NetworkInfoWithAcT info = (NetworkInfoWithAcT) itr.next();
            String forbiddenNetwork = info.getOperatorNumeric();
            if (forbiddenNetwork != null && forbiddenNetwork.equals("46020")) {
                itr.remove();
                continue;
            }
            int act = info.getAccessTechnology();
            if (!lteSupported && (act & PlmnEditor.RIL_4G) != 0) {
                itr.remove();
                continue;
            }
        }

        Collections.sort(plmnList, new NetworkInfoCompare());

        return plmnList;
    }

    private void updatePreference() {
        mPlmnListScreen.removeAll();
        for (int index = 0; index < mPlmnList.size(); index++) {
            Preference pref = new Preference(getPrefContext());
            pref.setTitle(getPlmnPreferenceName(mPlmnList.get(index)));
            pref.setOrder(index);
            mPlmnListScreen.addPreference(pref);
        }
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

    /**
     * Network Compartor class to allign the network based on priority.
     */
    class NetworkInfoCompare implements Comparator<NetworkInfoWithAcT> {
        @Override
        public int compare(NetworkInfoWithAcT object1, NetworkInfoWithAcT object2) {
            return (object1.getPriority() - object2.getPriority());
        }
    }

    /**
     * Handler class to query plmn list.
     */
    private class MyHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            final Activity activity = getActivity();
            if (activity == null || activity.isFinishing()
                    || activity.isDestroyed()) {
                Log.d(TAG, "handleMessage, activity is finished.");
                return;
            }

            switch (msg.what) {
                case MESSAGE_GET_PLMN_LIST:
                    handleGetPlmnResponse(msg);
                    break;

                case MESSAGE_SET_PLMN_LIST:
                    handleSetPlmnResponse(msg);
                    break;

                default:
                    break;
            }
        }

        public void handleGetPlmnResponse(Message msg) {
            Log.d(TAG, "handleGetPlmnResponse");

            ArrayList<NetworkInfoWithAcT> plmnList = null;
            AsyncResult ar = (AsyncResult) msg.obj;

            if (ar.exception != null) {
                Log.d(TAG, "handleGetPlmnResponse, exception=" + ar.exception);
            } else {
                plmnList = (ArrayList<NetworkInfoWithAcT>) ar.result;
            }
            
            mPlmnList.clear();
            if (plmnList != null && !plmnList.isEmpty()) {
                mPlmnList.addAll(filterPlmnList(plmnList));
                adjustPriority(mPlmnList);
            }

            setProgressBarVisible(false);

            updatePreference();
        }

        public void handleSetPlmnResponse(Message msg) {
            Log.d(TAG, "handleSetPlmnResponse, num=" + mPlmnNum);
            mPlmnNum--;

            AsyncResult ar = (AsyncResult) msg.obj;
            boolean isUserException = false;
            if (ar.exception != null) {
                Log.d(TAG, "handleSetPlmnResponse, exception=" + ar.exception);
            }

            if (mPlmnNum == 0) {
                mPhone.getPol(mHandler.obtainMessage(MESSAGE_GET_PLMN_LIST));
            }
        }
    }
}
