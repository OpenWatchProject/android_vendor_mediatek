/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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


package com.mediatek.op07.settings;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.CountDownTimer;
import android.os.IBinder;
import android.os.RemoteException;
import android.preference.Preference.OnPreferenceClickListener;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.SwitchPreference;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TabHost;
import android.widget.Toast;

import com.android.ims.ImsConfig;
import com.android.ims.ImsException;
import com.android.ims.ImsManager;

import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;

import com.mediatek.entitlement.ISesService;
import com.mediatek.entitlement.ISesServiceListener;
import com.mediatek.ims.internal.MtkImsManager;
import com.mediatek.internal.telephony.MtkPhoneConstants;
import com.mediatek.ims.internal.MtkImsManagerEx;

/**
 * Main WFC UI including switch and emergency address menu
 */
public class WiFiCallingFragmentSecond extends PreferenceFragment
        implements Preference.OnPreferenceChangeListener {

    private static final String TAG = "Op07WiFiCallingFragment";

    private static final String WIFICALLING_SWITCH_KEY = "toggle_wificalling";
    private static final String EMPTY_VIEW_KEY = "empty_view";
    private static final String EMERGENCY_ADDRESS_KEY = "emergency_address";
    private static final String SERVICE_PKG_NAME = "com.mediatek.entitlement";
    private static final String SERVICE_NAME = "com.mediatek.entitlement.EntitlementService";
    private final boolean DISABLE_SWITCH = false;
    private final boolean ENABLE_SWITCH = true;

    public static int CALL_STATE_OVER_WFC = 0;
    public static int CALL_STATE_IDLE = 1;
    // Number of active Subscriptions to show tabs
    private static final int TAB_THRESHOLD = 2;
    public static final String CALL_STATE = "call_state";
    public static final String NOTIFY_CALL_STATE = "OP07:call_state_Change";

    private SwitchPreference mWificallingSwitch;
    private Preference mEmergencyAddress;

    private ProgressDialog mLoadingDialog;

    private WfcSettingsActivitySecond mWfcSettingsActivity = null;
    private SubscriptionManager mSubscriptionManager;
    private List<SubscriptionInfo> mActiveSubInfos;
    private TabHost mTabHost;
    private int mCurrentTab = 0;
    private ImsManager mImsMgr;
    private TelephonyManager mTelephonyManager;
    private ISesService mSesService = null;
    private int mSlotId = 0;
    public static final int STATE_NOT_ENTITLED = 0;
    public static final int STATE_PENDING = 1;
    public static final int STATE_ENTITlED = 2;
    public static final int STATE_ENTITLEMENT_FAILED = 3;
    private static int sCallState = -1;
    private boolean mShouldShowWebShet []= {false, false};
    private boolean mIsActivityVisible[]= {false, false};
    private String mUrl = null;
    private String mPostData = null;
    private static final String UX_APP_PATH =
             "com.mediatek.op07.settings/com.mediatek.op07.settings.EntitlementActivity";
    private static final int REQUEST_UPDATE_WFC_EMERGENCY_ADDRESS = 1;
    public static final int LAUNCH_UPDATE_ONLY = 2;
    public final String EXTRA_LAUNCH_CARRIER_APP = "EXTRA_LAUNCH_CARRIER_APP";
    public final String EXTRA_LAUNCH_E911_URL = "EXTRA_LAUNCH_E911_URL";
    public final String EXTRA_LAUNCH_E911_POSTDATA = "EXTRA_LAUNCH_E911_POSTDATA";
    public final String EXTRA_LAUNCH_SLOTID = "EXTRA_LAUNCH_SLOTID";

    private int mEntitlementState = STATE_NOT_ENTITLED;
    private enum TabState {
        NO_TABS, UPDATE, DO_NOTHING
    }

    private final OnPreferenceClickListener mClickListener = new OnPreferenceClickListener() {
        @Override
        public boolean onPreferenceClick(Preference preference) {
            Log.d(TAG, "onPreferenceClick, pref:" + preference);
            if (preference == mEmergencyAddress) {
                updateLocationAndTc(mSlotId);
                return true;
            }
            return false;
        }
    };
    public void updateLocationAndTc(int slotId) {
        showProgressDialog(R.string.loading_message);
        try {
            mSesService.updateLocationAndTc(slotId);
        } catch (RemoteException e) {
            Log.e(TAG, "updateLocationAndTc :Exception happened! " + e.getMessage());
        }
    }

    private final BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            Log.d(TAG, "action: " + intent.getAction());
            if (!ImsManager.isWfcEnabledByPlatform(context)) {
                Log.d(TAG, "isWfcEnabledByPlatform: false");
                return;
            }
            if (NOTIFY_CALL_STATE.equals(intent.getAction())) {
                int callState = intent.getIntExtra(CALL_STATE, CALL_STATE_IDLE);
                Log.v(TAG, "br call_satte: " + callState);
                if (callState == CALL_STATE_OVER_WFC) {
                    //check if wificalling is registered
                    updateState(DISABLE_SWITCH);
                } else {
                    updateState(ENABLE_SWITCH);
                }
            }
        }
    };

    /** Constructor.
     */
    public WiFiCallingFragmentSecond(WfcSettingsActivitySecond parent) {
        mWfcSettingsActivity = parent;
    }

    public WiFiCallingFragmentSecond() {
        Log.d(TAG, "Empty constructor");
    }

    @Override
    public void onAttach(Activity activity) {
        super.onAttach(activity);
        Log.d(TAG, "onAttach activity = " + activity);
        if (activity instanceof WfcSettingsActivitySecond) {
            if (mWfcSettingsActivity == null) {
                mWfcSettingsActivity = (WfcSettingsActivitySecond) activity;
            }
        }

    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.d(TAG, "onCreate sCallState");
        addPreferencesFromResource(R.layout.wificalling_settings);

        mSubscriptionManager = SubscriptionManager.from(mWfcSettingsActivity);
        // Initialize mActiveSubInfo
        int max = mSubscriptionManager.getActiveSubscriptionInfoCountMax();
        mActiveSubInfos = new ArrayList<SubscriptionInfo>(max);
        mWificallingSwitch = (SwitchPreference) findPreference(WIFICALLING_SWITCH_KEY);
        mEmergencyAddress = findPreference(EMERGENCY_ADDRESS_KEY);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        return inflater.inflate(com.android.internal.R.layout.common_tab_settings,
                container, false);
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        initializeSubscriptions();
        ///get slotid
        mSlotId = convertTabToSlot(mCurrentTab);
        bindWithService();
    }

    private void initializeSubscriptions() {
        int currentTab = 0;
        Log.d(TAG, "initializeSubscriptions");
        List<SubscriptionInfo> sil = mSubscriptionManager.getActiveSubscriptionInfoList();
        TabState state = isUpdateTabsNeeded(sil);
        mActiveSubInfos.clear();
        if (sil != null) {
            mActiveSubInfos.addAll(sil);
        }

        switch (state) {
            case UPDATE: {
                Log.d(TAG, "initializeSubscriptions: UPDATE");
                currentTab = mTabHost != null ? mTabHost.getCurrentTab() : mCurrentTab;
                if (mTabHost != null) {
                    mTabHost.clearAllTabs();
                    Log.d(TAG, "TabHost Clear.");
                }
                mTabHost = (TabHost) getActivity().findViewById(android.R.id.tabhost);
                mTabHost.setup();
                for (int index = 0; index  < mActiveSubInfos.size(); index++) {
                    String tabName = String.valueOf(mActiveSubInfos.get(index).
                            getDisplayName());
                    Log.d(TAG, "initializeSubscriptions:tab=" + index + " name=" + tabName);
                    mTabHost.addTab(buildTabSpec(String.valueOf(index), tabName));
                }

                mTabHost.setOnTabChangedListener(mTabListener);
                mTabHost.setCurrentTab(currentTab);
                break;
            }
            case NO_TABS: {
                Log.d(TAG, "initializeSubscriptions: NO_TABS");
                if (mTabHost != null) {
                    mTabHost.clearAllTabs();
                    mTabHost = null;
                }
                break;
            }
            case DO_NOTHING: {
                Log.d(TAG, "initializeSubscriptions: DO_NOTHING");

                if (mTabHost != null) {
                    currentTab = mTabHost.getCurrentTab();
                }
                break;
            }
        }
    }

    private TabHost.OnTabChangeListener mTabListener = new TabHost.OnTabChangeListener() {
        @Override
        public void onTabChanged(String tabId) {
            Log.d(TAG, "onTabChanged...:");
            // The User has changed tab; update the body.
            updatePhone(convertTabToSlot(Integer.parseInt(tabId)));
            mCurrentTab = Integer.parseInt(tabId);
            ///get slotid
            mSlotId = convertTabToSlot(mCurrentTab);
            bindWithService();
            updateUI(mSlotId, mWificallingSwitch.isChecked());

        }
    };

    private void updatePhone(int slotId) {
        final SubscriptionInfo sir = mSubscriptionManager
                .getActiveSubscriptionInfoForSimSlotIndex(slotId);
        int phoneId = -1;
        if (sir != null) {
            phoneId = SubscriptionManager.getPhoneId(sir.getSubscriptionId());
        } else {
            Log.i(TAG, "sir = null phoneid = -1");
        }
        Log.i(TAG, "updatePhone:- slotId=" + slotId + " sir=" + sir);

        mImsMgr = ImsManager.getInstance(mWfcSettingsActivity, phoneId);
        mTelephonyManager = new TelephonyManager(mWfcSettingsActivity, phoneId);
        if (mImsMgr == null) {
            Log.i(TAG, "updatePhone :: Could not get ImsManager instance!");
        } else {
            Log.i(TAG, "updatePhone :: mImsMgr=" + mImsMgr);
        }

        //mPhoneStateListener.updatePhone();
    }
    private int convertTabToSlot(int currentTab) {
        mSlotId  = mActiveSubInfos.size() > currentTab ?
                mActiveSubInfos.get(currentTab).getSimSlotIndex() : 0;

        Log.i(TAG, "convertTabToSlot: info size=" + mActiveSubInfos.size() +
                    " currentTab=" + currentTab + " mSlotId=" + mSlotId);

        return mSlotId;
    }

    private TabHost.TabContentFactory mEmptyTabContent = new TabHost.TabContentFactory() {
        @Override
        public View createTabContent(String tag) {
            return new View(mTabHost.getContext());
        }
    };

    private TabHost.TabSpec buildTabSpec(String tag, String title) {
        return mTabHost.newTabSpec(tag).setIndicator(title).setContent(
                mEmptyTabContent);
    }

    private TabState isUpdateTabsNeeded(List<SubscriptionInfo> newSil) {
        TabState state = TabState.DO_NOTHING;
        if (newSil == null) {
            if (mActiveSubInfos.size() >= TAB_THRESHOLD) {
                Log.i(TAG, "isUpdateTabsNeeded: NO_TABS, size unknown and was tabbed");
                state = TabState.NO_TABS;
            }
        } else if (newSil.size() < TAB_THRESHOLD && mActiveSubInfos.size() >= TAB_THRESHOLD) {
            Log.i(TAG, "isUpdateTabsNeeded: NO_TABS, size went to small");
            state = TabState.NO_TABS;
        } else if (newSil.size() >= TAB_THRESHOLD && mActiveSubInfos.size() < TAB_THRESHOLD) {
            Log.i(TAG, "isUpdateTabsNeeded: UPDATE, size changed");
            state = TabState.UPDATE;
        } else if (newSil.size() >= TAB_THRESHOLD) {
            Iterator<SubscriptionInfo> siIterator = mActiveSubInfos.iterator();
            for(SubscriptionInfo newSi : newSil) {
                SubscriptionInfo curSi = siIterator.next();
                if (!newSi.getDisplayName().equals(curSi.getDisplayName())) {
                    Log.i(TAG, "isUpdateTabsNeeded: UPDATE, new name="
                            + newSi.getDisplayName());
                    state = TabState.UPDATE;
                    break;
                }
            }
        }
        Log.i(TAG, "isUpdateTabsNeeded:- " + state
                + " newSil.size()=" + ((newSil != null) ? newSil.size() : 0)
                + " mActiveSubInfos.size()=" + mActiveSubInfos.size());
        return state;
    }
    public boolean isServiceConnected() {
        return mSesService != null ? true : false;
    }

    public void cancelEntitlementCheck(int slotId) {
        if (mSesService != null) {
            try {
                mSesService.stopEntitlementCheck(slotId);
            } catch (RemoteException e) {
                Log.e(TAG, "cancelEntitlementCheck :Exception happened! " + e.getMessage());
            }
        }
    }

    private void showProgressDialog(int message) {
        if (mLoadingDialog == null) {
            mLoadingDialog = ProgressDialog.show(mWfcSettingsActivity, null,
                    getResources().getString(message), true,
                    true,
                    new DialogInterface.OnCancelListener() {
                        @Override
                        public void onCancel(DialogInterface dialog) {
                        // TODO:: Should do the cancel dialog handling here
                            Log.d(TAG, "OnCancel");
                            mLoadingDialog = null;
                            cancelEntitlementCheck(mSlotId);
                            if (!isServiceConnected()) {
                                mWfcSettingsActivity.finish();
                            }
                        }
                    });
            mLoadingDialog.setCanceledOnTouchOutside(false);
            mLoadingDialog.setInverseBackgroundForced(false);
        }
    }

    @Override
    public void onResume() {
        super.onResume();

        ImsManager imsManager = ImsManager.getInstance(mWfcSettingsActivity, mSlotId);
        imsManager.setWfcMode(ImsConfig.WfcModeFeatureValueConstants.CELLULAR_PREFERRED, false);

        mIsActivityVisible[mSlotId] = true;

        if (!isServiceConnected()) {
            showProgressDialog(R.string.loading_message);
        } else if (mShouldShowWebShet[mSlotId]) {
            showEmergencyFragment(mSlotId);
            mShouldShowWebShet[mSlotId] = false;
        }
        mWificallingSwitch.setOnPreferenceChangeListener(this);
        mWificallingSwitch.setChecked(mImsMgr.isWfcEnabledByUser(getActivity()));
        mWificallingSwitch.setEnabled((sCallState != CALL_STATE_OVER_WFC));

        mEmergencyAddress.setOnPreferenceClickListener(mClickListener);

        mSubscriptionManager.addOnSubscriptionsChangedListener(mOnSubscriptionsChangeListener);

        if (mWfcSettingsActivity != null) {
            IntentFilter filter = new IntentFilter();
            filter.addAction(WiFiCallingFragmentSecond.NOTIFY_CALL_STATE);
            Log.d(TAG, "Register receiver for call state");
            mWfcSettingsActivity.registerReceiver(mBroadcastReceiver, filter);
        }

        Log.d(TAG, "ON:" + mWificallingSwitch.isChecked());
        updateUI(mSlotId, mWificallingSwitch.isChecked());
    }

    public void showEmergencyFragment(int slotId) {
        ComponentName componentName = ComponentName.unflattenFromString(UX_APP_PATH);
        // Build and return intent
        Intent intent = new Intent();
        Log.i(TAG, "showEmergencyFragment slotId = " + slotId);
        intent.setComponent(componentName);
        intent.putExtra(EXTRA_LAUNCH_SLOTID, slotId);
        intent.putExtra(EXTRA_LAUNCH_CARRIER_APP, LAUNCH_UPDATE_ONLY);
        intent.putExtra(EXTRA_LAUNCH_E911_URL, mUrl);
        intent.putExtra(EXTRA_LAUNCH_E911_POSTDATA, mPostData);
        startActivityForResult(intent, REQUEST_UPDATE_WFC_EMERGENCY_ADDRESS + slotId);
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (mSlotId == requestCode -REQUEST_UPDATE_WFC_EMERGENCY_ADDRESS) {
            if (requestCode == REQUEST_UPDATE_WFC_EMERGENCY_ADDRESS + mSlotId) {
                Log.d(TAG, "WFC emergency address activity result = " + resultCode);

                if (resultCode == Activity.RESULT_OK) {
                    boolean updateResult = data.getBooleanExtra("isComplete", false);
                    if (updateResult) {
                        turnWfcOn(mSlotId);
                    } else {
                        startEntitlement(mSlotId, true);
                    }
                } else {
                    if (mEntitlementState != STATE_ENTITlED) {
                        cancelEntitlementCheck(mSlotId);
                    }
                }
            }
        }
    }

    public void startEntitlement(int slotId, boolean retry) {

        if (mSesService != null) {
           try {
               if (retry) {
                   mSesService.startEntitlementCheck(slotId, 30, 2);
               } else {
                   mSesService.startEntitlementCheck(slotId, 0, 0);
               }
           } catch (RemoteException e) {
               Log.e(TAG, "startEntitlementCheck :Exception happened! "
                       + e.getMessage());
           }
       }
   }

    public boolean turnWfcOn(int slotId) {
        // Show toast for time being & wait for service callback to enable switch
        Log.i(TAG, "turnWfcOn do not set imsmanger.setwfcsetting fasle slotid = " + slotId);
        startEntitlement(slotId, false);
        //updateWifiCalling(slotId, false);
        // Wifi not connected means Alert dialog is being displayed.
        // Display progress dialog once user takes action on it
        showProgressDialog(R.string.SES_ongoing_string);

        return false;
    }
    private void bindWithService() {
        if (isPackageExist(SERVICE_PKG_NAME)) {
            Intent intent = new Intent();
            intent.setClassName(SERVICE_PKG_NAME, SERVICE_NAME);
            boolean b = mWfcSettingsActivity.bindService(
                    intent, mServiceConnection, Context.BIND_AUTO_CREATE);
            Log.d(TAG, "bindWithService:" + b);
        }
    }

    private void dismissProgressDialog() {
        if (mLoadingDialog != null && mLoadingDialog.isShowing()) {
            Log.d(TAG, "Dismiss dialog");
            mLoadingDialog.dismiss();
            mLoadingDialog = null;
        }
    }

    private ServiceConnection mServiceConnection = new ServiceConnection() {

        @Override
        public void onServiceConnected(ComponentName name, IBinder binder) {
            Log.d(TAG, "SES service Connected");
            mSesService = ISesService.Stub.asInterface(binder);
            dismissProgressDialog();
            try {
                for (int i = 0; i < mActiveSubInfos.size(); i ++) {
                    int slotId = mActiveSubInfos.get(i).getSimSlotIndex();
                    mSesService.registerListener(slotId, mSesStateListener);
                }
            } catch (RemoteException e) {
                Log.e(TAG, "Exception happened! " + e.getMessage());
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            Log.d(TAG, "SES service DisConnected");
            try {
                for (int i = 0; i < mActiveSubInfos.size(); i ++) {
                    int slotId = mActiveSubInfos.get(i).getSimSlotIndex();
                    mSesService.unregisterListener(slotId, mSesStateListener);
                }
            } catch (RemoteException e) {
                Log.e(TAG, "Exception happened! " + e.getMessage());
            }
            mSesService = null;
        }
    };

    private int translateToInternalState(String internalState) {
        if (internalState == null) {
            return STATE_NOT_ENTITLED;
        }
        switch (internalState) {
            case "not-entitled":
                return STATE_NOT_ENTITLED;
            case "failed":
                return STATE_ENTITLEMENT_FAILED;
            case "pending":
                return STATE_PENDING;
            case "entitled":
                return STATE_ENTITlED;
            default:
                Log.e(TAG, "Unhandled state: " + internalState);
                return STATE_NOT_ENTITLED;
        }
    }

    private ISesServiceListener.Stub mSesStateListener = new ISesServiceListener.Stub() {
        @Override
        public void onEntitlementEvent(int slotId, String event, Bundle extras) {
            Log.d(TAG, "onEntitlementEvent: " + slotId + ", "
                    + event + ", " + extras
                    + " mSlotId = "  + mSlotId);
            if (mSlotId == slotId) {
                mWfcSettingsActivity.runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        Log.d(TAG, "onEntitlementEvent: " + slotId + ", " + event + ", " + extras);
                        mEntitlementState = translateToInternalState(event);
                        dismissProgressDialog();
                        switch (event) {
                            case "entitled":
                                updateWifiCalling(slotId, true);
                                updateUI(slotId, true);
                                Toast.makeText(mWfcSettingsActivity,
                                        "provisioning successfully completed",
                                        Toast.LENGTH_SHORT).show();
                                break;
                            case "failed":
                                updateWifiCalling(slotId, false);
                                updateUI(slotId, false);
                                showToast("provisioning failed," +
                                        " please try again later");
                                break;
                            case "not-entitled":
                                updateWifiCalling(slotId, false);
                                updateUI(slotId, false);
                                Toast.makeText(mWfcSettingsActivity,
                                        "Not provisioning",
                                        Toast.LENGTH_SHORT).show();
                                break;
                            case "pending":
                                Toast.makeText(mWfcSettingsActivity,
                                        "provisioning in progress",
                                        Toast.LENGTH_SHORT).show();
                                break;
                            case "remove_sim":
                            case "new_sim":
                                updateWifiCalling(slotId, false);
                                updateUI(slotId, false);
                                Toast.makeText(mWfcSettingsActivity,
                                        "New/Remove SIM",
                                        Toast.LENGTH_SHORT).show();
                                break;
                        }
                    }
                });
            }
        }

        @Override
        public void onWebsheetPost(int slotId, String url, String postData) {
            Log.d(TAG, "onWebsheetPost: slotId:" + slotId
                    + ", url: " + url
                    + ", postData: " + postData
                    + " mSlotId = " + mSlotId);
            if (mSlotId == slotId){
                mWfcSettingsActivity.runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        Log.d(TAG, "onWebsheetPost: slotId:" + slotId
                                + ", url: " + url + ", postData: " + postData);
                        mUrl = url;
                        mPostData = postData;
                        if (mIsActivityVisible[mSlotId]) {
                            dismissProgressDialog();
                            showEmergencyFragment(mSlotId);
                        } else {
                            mShouldShowWebShet[mSlotId] = true;
                        }
                    }
                });
            }
        }
    };

    /**
     * Unbinds with Ses Service.
     * @param context context
     * @param serviceConnection serviceConnection
     * @return
     */
    private void unbindWithService() {
        if (mSesService != null) {
            Log.d(TAG, "unbindWithService");
            try {
                for (int i = 0; i < mActiveSubInfos.size(); i ++) {
                    int slotId = mActiveSubInfos.get(i).getSimSlotIndex();
                    mSesService.unregisterListener(slotId, mSesStateListener);
                }
            } catch (RemoteException e) {
                Log.e(TAG, "Exception happened! " + e.getMessage());
            }
            mWfcSettingsActivity.unbindService(mServiceConnection);
        }
    }

    private void showToast(String message) {
        Toast toastToShow;
        // Set the toast and duration
        int toastDurationInMilliSeconds = 10000;
        toastToShow = Toast.makeText(mWfcSettingsActivity, message, Toast.LENGTH_LONG);
        Log.d(TAG, "Start to show toast for " + message);
        // Set the countdown to display the toast
        CountDownTimer toastCountDown;
        toastCountDown = new CountDownTimer(toastDurationInMilliSeconds, 1000 /*Tick duration*/) {
            public void onTick(long millisUntilFinished) {
                Log.d(TAG, "onTick leftTime = " + millisUntilFinished);
                toastToShow.show();
            }
            public void onFinish() {
                Log.d(TAG, "onFinish cancel toast");
                toastToShow.cancel();
            }
        };
        // Show the toast and starts the countdown
        toastToShow.show();
        toastCountDown.start();
    }

    public boolean updateWifiCalling(int slotId, boolean enabled) {
        Log.d(TAG, "enabled:" + enabled);

        if (enabled) {
            if (isInSwitchProcess()) {
                Log.d(TAG, "Switching process ongoing");
                Toast.makeText(mWfcSettingsActivity, R.string.Switch_not_in_use_string, Toast.LENGTH_SHORT)
                        .show();
                return false;
            }
        }
        ImsManager imsManager = ImsManager.getInstance(mWfcSettingsActivity, slotId);
        imsManager.setWfcSetting(enabled);
        return enabled;
    }

    private boolean isInSwitchProcess() {
        int imsState = MtkPhoneConstants.IMS_STATE_DISABLED;
        try {
            imsState = MtkImsManagerEx.getInstance()
                .getImsState(mSlotId);
        } catch (ImsException e) {
           return false;
        }
        Log.d("@M_" + TAG, "isInSwitchProcess , imsState = " + imsState);
        return imsState == MtkPhoneConstants.IMS_STATE_DISABLING
                || imsState == MtkPhoneConstants.IMS_STATE_ENABLING;
    }

    private boolean isPackageExist(String packageName) {
        try {
            mWfcSettingsActivity.getPackageManager().getPackageInfo(packageName,
                    PackageManager.GET_SERVICES);
        } catch (PackageManager.NameNotFoundException  e) {
            Log.d(TAG, "package exist: false");
            return false;
        }
        Log.d(TAG, "package exist: true");
        return true;
    }

    @Override
    public void onPause() {
        super.onPause();
        mIsActivityVisible[mSlotId] = false;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        if (mWfcSettingsActivity != null) {
            mWfcSettingsActivity.unregisterReceiver(mBroadcastReceiver);
        }
        if (mSubscriptionManager != null) {
            mSubscriptionManager
                    .removeOnSubscriptionsChangedListener(mOnSubscriptionsChangeListener);
        }
        unbindWithService();
    }

    private final SubscriptionManager.OnSubscriptionsChangedListener
            mOnSubscriptionsChangeListener
            = new SubscriptionManager.OnSubscriptionsChangedListener() {
        @Override
        public void onSubscriptionsChanged() {
            Log.i(TAG, "onSubscriptionsChanged:");
            List<SubscriptionInfo> newSil = mSubscriptionManager.getActiveSubscriptionInfoList();
            /// M: add for hot swap @{
            if (isHotSwapHanppened(
                        mActiveSubInfos, newSil)) {
                Log.i(TAG, "onSubscriptionsChanged:hot swap hanppened");
                final Activity activity = getActivity();
                if (activity != null) {
                    activity.finish();
                }
                return;
            }
            /// @}
        }
    };
    /**
     * Implemented to support onPreferenceChangeListener to look for preference
     * changes.
     *
     * @param preference is the preference to be changed
     * @param objValue should be the value of the selection, NOT its localized
     * display value.
     * @return boolean
     */
    @Override
    public boolean onPreferenceChange(Preference preference, Object objValue) {
        Log.d(TAG, "onPreferenceChange:" + preference + ", changed to " + objValue);
        if (preference == mWificallingSwitch) {
            // TODO: do not change the switch on user click. check with service
            // if service returns true, enable else let it be off until response from
            // service is not received
            // if turn ON, enable wificallingMode & emergency preference

            boolean wfcOn = (boolean) objValue;
            Log.d(TAG, "Wfc ON:" + wfcOn);
            if (!wfcOn) {
                //Switch is turned OFF
                boolean result = updateWifiCalling(mSlotId, wfcOn);
                deactivateService(mSlotId);
                updateUI(mSlotId, result);
            } else {
               showLocationAlert(mSlotId);
               return false;
            }
        }
        return true;
    }

    protected boolean isHotSwapHanppened(
            List<SubscriptionInfo> activeSubInfos,
            List<SubscriptionInfo> newSil) {
        boolean result = false;
        if (null == activeSubInfos) {
            Log.i(TAG, "isHotSwapHanppened null == activeSubInfos");
            return false;
        }
        if (null == newSil && activeSubInfos.size() > 0) {
            Log.i(TAG, "isHotSwapHanppened null == newSil && activeSubInfos.size() > 0");
            return true;
        } else if (null == newSil) {
            Log.i(TAG, "isHotSwapHanppened null == newSil");
            return false;
        }
        if (activeSubInfos.size() != newSil.size()) {
            return true;
        }
        for (int i = 0; i < newSil.size(); i++) {
            SubscriptionInfo currentSubInfo = newSil.get(i);
            SubscriptionInfo originalSubInfo = activeSubInfos.get(i);
            if (!(currentSubInfo.getIccId()).equals(originalSubInfo.getIccId())) {
                result = true;
                break;
            } else {
                result = false;
            }
        }

        Log.i(TAG, "isHotSwapHanppened : " + result);
        return result;
    }

    public void deactivateService(int slotId) {
        if (mSesService != null) {
            try {
                mSesService.deactivateService(slotId);
            } catch (RemoteException e) {
                Log.e(TAG, "deactivateService :Exception happened! " + e.getMessage());
            }
        } else {
            Log.d(TAG, "deactivateService mSesService is null");
        }
    }

    public void updateUI(int slotId, final boolean enabled) {
        Log.d(TAG, "enabled:" + enabled);
        if (mSlotId == slotId) {
            getActivity().runOnUiThread(new Runnable() {
                @Override
                public void run() {

                    if (enabled) {
                        getPreferenceScreen().addPreference(mEmergencyAddress);
                    }
                    else {
                        getPreferenceScreen().removePreference(mEmergencyAddress);
                    }
                    mWificallingSwitch.setChecked(enabled);
                    mWificallingSwitch.setTitle(enabled ? R.string.On : R.string.Off);
                    mWificallingSwitch.setSummaryOn(getSummary());
                    mWificallingSwitch.setSummaryOff(getSummary());
                    mWificallingSwitch.setEnabled((sCallState != CALL_STATE_OVER_WFC));
                   }
                });
        }
    }
    /**
     * Gets wfc summary on basis of entitlement status.
     * @param service service
     * @return int
     */
    public int getSummary() {
        int resId = R.string.empty_message;

        switch (mEntitlementState) {
 
            case STATE_NOT_ENTITLED:
               break;

            case STATE_PENDING:
                resId = R.string.prov_pending_message;
                break;

            case STATE_ENTITlED:
                resId = R.string.prov_complete_message;
                break;

            case STATE_ENTITLEMENT_FAILED:
                resId = R.string.service_not_allowed;
                break;

            default:
                Log.d(TAG, "invalid state:" + mEntitlementState);
                break;
        }
        return resId;
    }
    private void showWifiAlert() {
        AlertDialog.Builder builder = new AlertDialog.Builder(mWfcSettingsActivity);
        builder.setMessage(R.string.wifi_off_message)
                .setTitle(R.string.wifi_off_title)
                .setPositiveButton(android.R.string.ok,
                        new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                Toast.makeText(mWfcSettingsActivity,
                                        R.string.location_usage_info, Toast.LENGTH_SHORT).show();
                            }
                        });
        AlertDialog dialog = builder.create();
        dialog.show();
    }

    public void showLocationAlert(int slotId) {
        final int islotId = slotId;
        AlertDialog.Builder builder = new AlertDialog.Builder(mWfcSettingsActivity);
        builder.setTitle(R.string.location_usage_info)
                .setPositiveButton(android.R.string.ok,
                        new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                if (!mWfcSettingsActivity.isWifiConnected(mWfcSettingsActivity)) {
                                    Toast.makeText(mWfcSettingsActivity,
                                    R.string.wifi_off_message,
                                    Toast.LENGTH_SHORT).show();
                                }
                                boolean result = turnWfcOn(islotId);
                                updateUI(islotId, result);
                            }
                        })
                .setNegativeButton(android.R.string.cancel, null);
        AlertDialog dialog = builder.create();
        dialog.show();
    }

    private final void updateState(boolean switchState) {
        mWificallingSwitch.setEnabled(switchState);
    }

    public static void setCallState(int callState) {
        sCallState = callState;

    }
}
