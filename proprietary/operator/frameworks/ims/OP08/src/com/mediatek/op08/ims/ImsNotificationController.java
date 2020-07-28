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

package com.mediatek.op08.ims;

import android.app.Activity;
import android.app.KeyguardManager;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.location.LocationManager;
import android.preference.PreferenceManager;
import android.database.ContentObserver;
import android.net.Uri;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.UserHandle;
import android.os.PersistableBundle;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.provider.Settings;
import android.telephony.CarrierConfigManager;
import android.telephony.ims.feature.MmTelFeature;
import android.telephony.ims.ImsMmTelManager;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.stub.ImsRegistrationImplBase;
import android.telephony.SubscriptionManager;
import android.telephony.PhoneStateListener;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.widget.Toast;

import com.android.ims.ImsConfig;
import com.android.ims.ImsConnectionStateListener;
import com.android.ims.ImsException;
import com.android.ims.ImsManager;
import com.android.ims.ImsServiceClass;
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.RILConstants;
import com.android.internal.telephony.TelephonyIntents;

import com.mediatek.ims.internal.MtkImsManager;
import com.mediatek.ims.internal.MtkImsManagerEx;
import com.mediatek.internal.telephony.MtkPhoneConstants;
import com.mediatek.internal.telephony.IMtkTelephonyEx;
import mediatek.telephony.MtkServiceState;

import android.net.NetworkInfo;

import java.util.HashMap;


/** Class to show WFC related notifications like registration & WFC call.
 */
public class ImsNotificationController {

    BroadcastReceiver mBr = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Log.d(TAG, "Intent action:" + intent.getAction());

            /* Restore screen lock state, even if intent received may not provide its effect */
            if (intent.getAction().equals(Intent.ACTION_SCREEN_OFF)) {
                mIsScreenLock = true;
                handleScreenOff();
            } else {
                mIsScreenLock = mKeyguardManager.isKeyguardLocked();
            }
            Log.d(TAG, "on receive:screen lock:" + mIsScreenLock);

            /* ALPS02260621: Need to save phone_type as there is no provision of getting it
                     * without having phone object.
                     */
       /* if (intent.getAction().equals(PhoneConstants.ACTION_SUBSCRIPTION_PHONE_STATE_CHANGED)) {
                String state = intent.getStringExtra(TelephonyManager.EXTRA_STATE);
                int phoneType = intent.getIntExtra(MtkPhoneConstants.PHONE_TYPE_KEY,
                        RILConstants.NO_PHONE);
                if (phoneType == RILConstants.IMS_PHONE) {
                    if (TelephonyManager.EXTRA_STATE_OFFHOOK.equals(state)
                            || TelephonyManager.EXTRA_STATE_RINGING.equals(state)) {
                        mPhoneType = RILConstants.IMS_PHONE;
                    } else {
                        mPhoneType = RILConstants.NO_PHONE;
                    }
                }
            }
            if (DBG) {
                Log.d(TAG, "mPhoneType:" + mPhoneType);
            }*/

            if (intent.getAction().equals(PhoneConstants
                    .ACTION_SUBSCRIPTION_PHONE_STATE_CHANGED)) {
                /* ALPS02260621: Need to save phone_type as there is no provision of getting it
                  * without having phone object.
                  */
                int phoneId = intent.getIntExtra(PhoneConstants.SLOT_KEY,
                        SubscriptionManager.INVALID_PHONE_INDEX);
                String state = intent.getStringExtra(TelephonyManager.EXTRA_STATE);
                Log.d(TAG, "phoneId: " + phoneId);
                int imsCallCount = 0;
                try {
                    imsCallCount = MtkImsManagerEx.getInstance().getCurrentCallCount(phoneId);
                } catch (ImsException e) {
                    Log.e(TAG, "getImsCallCount: " + e);
                }
                handleCallIntent(state, imsCallCount);
            } else if (intent.getAction().equals(ImsManager.ACTION_IMS_SERVICE_UP)) {
                start();
            } else if (intent.getAction().equals(ImsManager.ACTION_IMS_SERVICE_DOWN)) {
                stop();
            } else if (intent.getAction().equals(ImsManager.ACTION_IMS_REGISTRATION_ERROR)) {
                mLocationSetting = mLocationManager.isLocationEnabled();
                Log.i(TAG, "mLocationSetting: " + mLocationSetting);
                if (!mLocationSetting) {
                    // If the location dialog is show. There is no need to add
                    // notification in this case.
                    // In order to communicate to ImsPhone that it should
                    // not show notification, we are changing result code here.
                    setResultCode(Activity.RESULT_CANCELED);
                    showLocationDialog();
                }
            } else if (intent.getAction().equals(Intent.ACTION_SCREEN_ON)) {
                handleScreenOn();
            } else if (intent.getAction().equals(Intent.ACTION_USER_PRESENT)) {
                handleScreenUnlock();
            } else if (intent.getAction().equals(ACTION_ROAM_STATE_CACHE)) {
                int roam_state = intent.getIntExtra(ROAMING_STATE_KEY, IS_HOME);
                Log.d(TAG, "reset roam state cache to: " + roam_state);
                saveRoamingStateToSp(roam_state,ROAMING_STATE_KEY);
                getRoamingStateFromSp(ROAMING_STATE_KEY);
            }else if (intent.getAction().equals(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED)) {
                String imsEnabledKey = intent.getStringExtra(INTENT_KEY_PROP_KEY);
                if (SubscriptionManager.WFC_IMS_ENABLED.equals(imsEnabledKey)) {
                    handleRoamingStateChange();
                }
            }else if(intent.getAction().equals(WifiManager.NETWORK_STATE_CHANGED_ACTION)){
                NetworkInfo info = intent.getParcelableExtra(WifiManager.EXTRA_NETWORK_INFO);
                boolean oldWifiConnected = mWifiConnected;
                mWifiConnected = info.isConnected();
                Log.d(TAG, "mWifiConnected: " + mWifiConnected +
                        ", oldWifiConnected: " + oldWifiConnected);
                if (mWifiConnected != oldWifiConnected) {
                    handleRoamingStateChange();
                }
            } else if (intent.getAction().equals(TelephonyIntents.ACTION_SIM_STATE_CHANGED)) {
                String simState = intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);
                Log.d(TAG, "simState:" + simState);
                // Check Sim absent case
                if (simState.equals(IccCardConstants.INTENT_VALUE_ICC_ABSENT)) {
                    mIsSimPresent = false;
                    mIsISimAppPresent = false;
                    mIsGBAValid = false;
                    Log.d(TAG, "SIM not present");
                /* Reset sim present value on sim state:
                    1) NOT_READY: for normal sim insertion/removal cases
                    2) READY:for hot plugin case,in which state jumps directly from ABSENT to READY
                    3) LOADED: IMSN will be created in SIM_LOADED by OP08ImsServiceExt(cz first
                               (IMS_ENABLING event comes after LOADED state), when booted with SIM
                               inserted. So, NOT_READY/READY events will be missed.(ALPS03586962).
                 */
                } else if (simState.equals(IccCardConstants.INTENT_VALUE_ICC_NOT_READY)
                        || simState.equals(IccCardConstants.INTENT_VALUE_ICC_READY)) {
                    mIsSimPresent = true;
                    Log.d(TAG, "Remove sim error if present");
                    mNotificationManager.cancelAsUser(null, WFC_NOTIFICATION, UserHandle.ALL);
                } else if (simState.equals(IccCardConstants.INTENT_VALUE_ICC_LOADED)) {
                    mIsSimPresent = true;
                    mIsISimAppPresent = isIsimAppPresent();
                    mIsGBAValid = isGbaValid(context);
                    Log.d(TAG, "SIM is loaded, so remove SIM error if present");
                    mNotificationManager.cancelAsUser(null, WFC_NOTIFICATION, UserHandle.ALL);
                }
            }
            Log.d(TAG, "SIM present:" + mIsSimPresent + "\nISIM present:" + mIsISimAppPresent +
                       "\nGBA Valid:" + mIsGBAValid +
                       "\nWfc enabled by user:" + ImsManager.isWfcEnabledByUser(context));
            if (isWifiEnabled()) {
                // Show SIM error only if WFC is enabled by user & Wifi enabled
                if (ImsManager.isWfcEnabledByUser(context) && (!mIsSimPresent ||
                                                    !(mIsISimAppPresent && mIsGBAValid))) {
                    displayWfcErrorNotification();
                    return;
                }
            } else {
                if (!mIsSimPresent || !(mIsISimAppPresent && mIsGBAValid)) {
                    Log.d(TAG, "Wifi Disabled, remove no sim error noti, if present");
                    mNotificationManager.cancelAsUser(null, WFC_NOTIFICATION, UserHandle.ALL);
                }
            }
        }
    };

    // private final ImsConnectionStateListener mImsRegListener = new ImsConnectionStateListener() {
    //     @Override
    //     public void onFeatureCapabilityChanged(int serviceClass,
    //             int[] enabledFeatures, int[] disabledFeatures) {
    //         Log.d(TAG, "Receive IMS FeatureCapabilityChanged");
    //         handleImsStateChange(serviceClass, enabledFeatures, disabledFeatures);
    //     }
    // };

    private static final String TAG = "ImsNotificationController";

    /**
    * Wfc registration notification ID. This is
     * the ID of the Notification given to the NotificationManager.
     * Note: Id should be unique within APP.
     */
    private static final int WFC_NOTIFICATION = 0x10;
    private static final String WFC_CHANNEL_ID = "imsn_channel";

    private static final int WFC_REGISTERED_ICON = R.drawable.wfc_notify_registration_success;
    private static final int WFC_CALL_ICON = R.drawable.wfc_notify_ongoing_call;
    private static final int WFC_ERROR_ICON = R.drawable.wfc_notify_registration_error;

    private static final int WFC_REGISTERED_TITLE = R.string.success_notification_title;
    private static final int WFC_CALL_TITLE = R.string.ongoing_call_notification_title;
    private static final int WFC_ERROR_TITLE = R.string.network_error_notification_title;
    private static final int WFC_REGISTERED_SUMMARY = R.string.success_notification_summary;
    private static final int WFC_ERROR_SUMMARY = R.string.wfc_incorrect_sim_card_error;

    private static final int WFC_APP_TITLE = R.string.wfc_app_title;

    private static final int WFC_CHANNEL_NAME = R.string.wfc_app_title;
    private static final int WFC_CHANNEL_DESCRIPTION = R.string.wfc_app_title;
    private static final int WFC_TO_ROAMING_NOTIFICATION = R.string.wfc_notification_home_to_roaming;
    private static final int WFC_BACK_HOME_NOTIFICATION = R.string.wfc_notification_roaming_to_home;

    private static final String ACTION_LAUNCH_WFC_SETTINGS
                = "android.settings.WIFI_CALLING_SETTINGS";
    private static final String ACTION_LAUNCH_WFC_INVALID_SIM_ALERT
            = "mediatek.settings.WFC_INVALID_SIM_DIALOG_LAUNCH";

    private static final int CODE_WFC_DEFAULT = 0;
    private static final int CODE_WFC_SUCCESS = 1;

    private static final Bundle sNotificationBundle = new Bundle();

    // Current WFC state.
    // Can be: 1) Success: WFC registered (2) DEFAULT: WFC not registered
    //private int mImsState = CODE_WFC_DEFAULT;

    private boolean mWfcCapabilityPresent = false;
    private boolean mIsCapabilityTypeVoice = false;
    private boolean mWfcCallOngoing = false;
    private boolean mIsScreenLock = false;
    private boolean mIsSimPresent = false;
    private boolean isImsnRegistered = false;
    private boolean mIsISimAppPresent = false;
    private boolean mIsGBAValid = false;

    /*  Vars required for ImsNotificationController initialization */
    private Context mHostContext;
    private Context mPluginContext;
    private int mPhoneId;
    private int mPhoneType = RILConstants.NO_PHONE;

    private NotificationManager mNotificationManager;
    private KeyguardManager mKeyguardManager;
    private WifiManager mWifiManager;
    private ContentObserver mWfcSwitchContentObserver;
    private ImsManager mImsManager;
    private PhoneStateListener mPhoneStatelistener;
    private TelephonyManager mTelephonyManager;
    private LocationManager mLocationManager;

    private boolean mWifiConnected = false;
    private boolean mWfcRoamingNotified = false;
    private boolean mIsRoamingChange = false;
    private static final String INTENT_KEY_PROP_KEY = "simPropKey";

    private static final String ACTION_ROAM_STATE_CACHE = "com.mediatek.ims.ACTION_ROAM_STATE_CACHE";
    private static final String ROAMING_STATE_KEY = "roaming_state_key";
    private static final int IS_ROAMING = 2;
    private static final int IS_HOME    = 1;
    private static final int IS_UNKNOWN = 0;
    private int mCellularRoamingState = IS_UNKNOWN;

    private static HashMap<Integer, ImsNotificationController> mNotiCtrlMap = new HashMap();

    private int mImsRadioTech;
    private boolean mIsRegistered;

    private final Object mLock = new Object();

    private boolean mLocationSetting = false;

    /** Constructor.
     * @param hostContext hostContext
     * @param pluginContext pluginContext
     * @param phoneId phoneId
     */
    private ImsNotificationController(Context hostContext, Context pluginContext, int phoneId) {
        Log.d(TAG, "in constructor: phoneId:" + phoneId);
        mHostContext = hostContext;
        mPluginContext = pluginContext;
        mPhoneId = phoneId; // For multi-sim. Future use.
        mNotificationManager =
                (NotificationManager) mPluginContext.getSystemService(Context.NOTIFICATION_SERVICE);
        createChannel();
        mKeyguardManager = (KeyguardManager) mPluginContext
                                .getSystemService(Context.KEYGUARD_SERVICE);
        mIsScreenLock =  mKeyguardManager.isKeyguardLocked();
        mWifiManager = (WifiManager) mPluginContext.getSystemService(Context.WIFI_SERVICE);
        mImsManager = ImsManager.getInstance(pluginContext, getMainCapabilityPhoneId());
        mTelephonyManager = (TelephonyManager) mPluginContext.getSystemService(Context.TELEPHONY_SERVICE);
        mLocationManager = (LocationManager) mPluginContext.getSystemService(Context.LOCATION_SERVICE);
        mLocationSetting = mLocationManager.isLocationEnabled();
    }

    public static ImsNotificationController getInstance(Context hostContext, Context pluginContext,
            int phoneId) {
        if (mNotiCtrlMap.containsKey(phoneId)) {
            return mNotiCtrlMap.get(phoneId);
        }

        mNotiCtrlMap.put(phoneId, new ImsNotificationController(hostContext, pluginContext,
                    phoneId));

        return mNotiCtrlMap.get(phoneId);
    }

    public void updateContext(Context hostContext, Context pluginContext, int phoneId) {
        mHostContext = hostContext;
        mPluginContext = pluginContext;
        mPhoneId = phoneId;
    }

    /** Register receivers/observer/listeners.
     */
    public void start() {
        registerReceiver();
    }

    /** Stop the Imsnotification controller.  Dergister receivers etc.
     */
    public void stop() {
        Log.d(TAG, "in destroy Instance");
        unRegisterReceiver();
        /* Cancel visible notifications, if any */
        removeWfcNotification();
    }

    private void registerReceiver() {
        if (!isImsnRegistered) {
            IntentFilter filter = new IntentFilter(ImsManager.ACTION_IMS_SERVICE_DOWN);
            filter.addAction(ImsManager.ACTION_IMS_SERVICE_UP);
            filter.addAction(ImsManager.ACTION_IMS_REGISTRATION_ERROR);
            filter.addAction(PhoneConstants.ACTION_SUBSCRIPTION_PHONE_STATE_CHANGED);
            filter.addAction(Intent.ACTION_SCREEN_OFF);
            filter.addAction(Intent.ACTION_SCREEN_ON);
            filter.addAction(Intent.ACTION_USER_PRESENT);
            filter.addAction(TelephonyIntents.ACTION_SIM_STATE_CHANGED);
            filter.addAction(WifiManager.WIFI_STATE_CHANGED_ACTION);
            filter.addAction(WifiManager.NETWORK_STATE_CHANGED_ACTION);
            filter.addAction(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED);
            filter.addAction(ACTION_ROAM_STATE_CACHE);
            mPluginContext.registerReceiver(mBr, filter);
            registerForWfcSwitchChange();
            registerForImsStateChange();
            int subId = getSubIdUsingPhoneId(mPhoneId);
            Log.d(TAG, "registerReceiver(): subId " + subId);
            mPhoneStatelistener = new PhoneStateListener() {
                @Override
                public void onServiceStateChanged(ServiceState serviceState) {
                    if (serviceState == null) {
                        Log.e(TAG, "onServiceStateChanged-" + subId
                                + ": serviceState is null");
                    } else {
                        Log.d(TAG, "onServiceStateChanged(): " + serviceState);
                        if (isLTEOn(serviceState)) { // care in service state only
                            mIsRoamingChange = checkAndUpdateRoamingChange(serviceState);

                            Log.d(TAG, "onServiceStateChanged() subId: " + subId
                                    + " mIsRoamingChange: " + mIsRoamingChange);

                            handleRoamingStateChange();

                            if (mCellularRoamingState == IS_HOME) {
                                // if UE back to home, allow home -> roaming to notify again
                                resetToastWfcRoamingNotification();
                            }
                        } else {
                            Log.d(TAG, "onServiceStateChanged(), no service, skip");
                        }
                    }
                }
            };
            if (mTelephonyManager != null) {
                mTelephonyManager.listen(
                                mPhoneStatelistener, PhoneStateListener.LISTEN_SERVICE_STATE);
            }

            isImsnRegistered = true;
        }
    }

    private void saveRoamingStateToSp(int value, String key) {
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(
                mHostContext.createDeviceProtectedStorageContext());
        SharedPreferences.Editor editor = sp.edit();
        editor.putInt(key + getSubIdUsingPhoneId(mPhoneId), value);

        // Commit and log the result.
        if (!editor.commit()) {
            Log.e(TAG, "Failed to commit to preference");
        }
    }

    private int getRoamingStateFromSp(String key) {
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(
                mHostContext.createDeviceProtectedStorageContext());
        int value = sp.getInt(key + getSubIdUsingPhoneId(mPhoneId), IS_UNKNOWN);
        Log.d(TAG, "getFromSp: " + value);
        return value;
    }

    private void saveRoamingState(String key, int value) {
        TelephonyManager.setTelephonyProperty(mPhoneId,
                key, Integer.toString(value));
    }

    private int getPreviousRoamingState(String key) {
        String state = TelephonyManager.getTelephonyProperty(
                mPhoneId, key, String.valueOf(ServiceState.ROAMING_TYPE_UNKNOWN));
        // logd("getPreviousSubId: " + subId);

        if (state.isEmpty()) {
            return ServiceState.ROAMING_TYPE_UNKNOWN;
        }

        try {
            return Integer.parseInt(state);
        } catch (NumberFormatException e) {
            return ServiceState.ROAMING_TYPE_UNKNOWN;
        }
    }

    private void handleRoamingStateChange() {
        if (!mIsRoamingChange) {
            Log.d(TAG, "handleRoamingStateChange, mIsRoamingChange: " + mIsRoamingChange);
            return;
        }

        if (mCellularRoamingState == IS_ROAMING) {
            // home -> roaming
            if (needToToastWfcRoamingNotification()) {
                handleWfcRoamingNotification();
                mIsRoamingChange = false;
            }
        } else if (mCellularRoamingState == IS_HOME) {
            // roaming -> home
            if (needToToastWfcHomeNotification()) {
                handleWfcHomeNotification();
                mIsRoamingChange = false;
            }
        }
    }

    private void resetToastWfcRoamingNotification(){
        mWfcRoamingNotified = false; // reset
        Log.d(TAG, "resetToastWfcRoamingNotification mWfcRoamingNotified: "
                + mWfcRoamingNotified);
    }
    private boolean needToToastWfcRoamingNotification () {
        boolean needToast = false;
        int subId = getSubIdUsingPhoneId(mPhoneId);
        int currentWfcModeHome = ImsManager.getInstance(mPluginContext, mPhoneId).getWfcMode(false);
        int currentWfcModeRoaming = ImsManager.getInstance(mPluginContext, mPhoneId).getWfcMode(true);

        needToast = (isWifiEnabled() && !mWfcRoamingNotified &&
                mWifiConnected && isVoWifiOn(mPhoneId) &&
                ((currentWfcModeHome != ImsConfig.WfcModeFeatureValueConstants.WIFI_PREFERRED)
                || currentWfcModeRoaming != ImsConfig.WfcModeFeatureValueConstants.WIFI_PREFERRED));

        Log.d(TAG, "needToToastWfcRoamingNotification mWfcRoamingNotified: " + mWfcRoamingNotified
                // + " mIsCellularRoaming: " + mIsCellularRoaming
                + " WiFi connected: " + mWifiConnected
                + " isVoWifiOn: " + isVoWifiOn(mPhoneId)
                + " currentWfcModeHome: " + currentWfcModeHome
                + " currentWfcModeRoaming: " + currentWfcModeRoaming
                + " need toast: "+ needToast);
        return needToast;
    }

    private void handleWfcRoamingNotification() {
        toastWfcRoamingNotification();

        // Keep the same between home and roaming
        ImsManager.getInstance(mPluginContext, mPhoneId).setWfcMode(
                ImsConfig.WfcModeFeatureValueConstants.WIFI_PREFERRED, true);

        ImsManager.getInstance(mPluginContext, mPhoneId).setWfcMode(
                ImsConfig.WfcModeFeatureValueConstants.WIFI_PREFERRED, false);
    }

    private void handleWfcHomeNotification() {
        Toast.makeText(mPluginContext, WFC_BACK_HOME_NOTIFICATION,
                                            Toast.LENGTH_LONG).show();

        // Keep the same between home and roaming
        ImsManager.getInstance(mPluginContext, mPhoneId).setWfcMode(
                ImsConfig.WfcModeFeatureValueConstants.CELLULAR_PREFERRED, true);

        ImsManager.getInstance(mPluginContext, mPhoneId).setWfcMode(
                ImsConfig.WfcModeFeatureValueConstants.CELLULAR_PREFERRED, false);
    }

    private boolean needToToastWfcHomeNotification() {
        boolean needToast = false;
        int subId = getSubIdUsingPhoneId(mPhoneId);
        int currentWfcModeHome = ImsManager.getInstance(mPluginContext, mPhoneId).getWfcMode(false);
        int currentWfcModeRoaming = ImsManager.getInstance(mPluginContext, mPhoneId).getWfcMode(true);

        needToast = (isWifiEnabled() && isVoWifiOn(mPhoneId) &&
                ((currentWfcModeHome != ImsConfig.WfcModeFeatureValueConstants.CELLULAR_PREFERRED)
                || currentWfcModeRoaming != ImsConfig.WfcModeFeatureValueConstants.CELLULAR_PREFERRED));

        Log.d(TAG, "needToToastWfcHomeNotification "
                + " isVoWifiOn: " + isVoWifiOn(mPhoneId)
                + " currentWfcModeHome: " + currentWfcModeHome
                + " currentWfcModeRoaming: " + currentWfcModeRoaming
                + " need toast: "+ needToast);
        return needToast;
    }
    private void toastWfcRoamingNotification() {
        Toast.makeText(mPluginContext, WFC_TO_ROAMING_NOTIFICATION, Toast.LENGTH_LONG).show();
        mWfcRoamingNotified = true;
    }

    private void unRegisterReceiver() {
        if (isImsnRegistered) {
            mPluginContext.unregisterReceiver(mBr);
            unRegisterForWfcSwitchChange();
            unRegisterForImsStateChange();
            if (mTelephonyManager != null) {
                mTelephonyManager.listen(mPhoneStatelistener, 0);
            }
            mPhoneStatelistener = null;
            isImsnRegistered = false;
        }
    }

    private void createChannel() {
        NotificationChannel channel = new NotificationChannel(WFC_CHANNEL_ID,
                mPluginContext.getResources().getString(WFC_CHANNEL_NAME),
                NotificationManager.IMPORTANCE_LOW);
        channel.setDescription(mPluginContext.getResources().getString(WFC_CHANNEL_DESCRIPTION));
        mNotificationManager.createNotificationChannel(channel);
    }

    private void handleCallIntent(String state, int imsCallCount) {
        Log.d(TAG, "in handleCallIntent, phone state:" + state +
              "\nin handleCallIntent, imsCallCount:" + imsCallCount);
        if (imsCallCount > 0) {
            if (TelephonyManager.EXTRA_STATE_OFFHOOK.equals(state)
                    || TelephonyManager.EXTRA_STATE_RINGING.equals(state)) {
                mWfcCallOngoing = true;
                displayWfcCallNotification();
            } else {
                mWfcCallOngoing = false;
                displayWfcRegistrationNotification(false);
            }
        } else {
            mWfcCallOngoing = false;
            displayWfcRegistrationNotification(false);
        }
    }

    private void handleImsStateChange(int serviceClass,
                int[] enabledFeatures, int[] disabledFeatures) {
        if (serviceClass == ImsServiceClass.MMTEL) {
            Log.d(TAG, "wfc capability:" +
                    enabledFeatures[ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_WIFI]);
            if (enabledFeatures[ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_WIFI]
                    != ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_WIFI) {
                mWfcCapabilityPresent = false;
                /* Capabilities have been change to other than WIFI, so set wfc status as OFF */
                removeWfcNotification();
            } else {
                handleInStateService();
            }
        }
    }

    private void handleInStateService() {
        Log.d(TAG, "in handleInStateService");
        /*handle for registration icon*/
            mWfcCapabilityPresent = true;
            /* Capabilities have been change to WIFI, so set wfc status as Success.
             * It is done to cover handover cases in which IMS_STATE_CHANGE is not
             * received before capability_change intent
            */
            /* ALPS02187200: Query phone state to check whether UE is in Call
             * when capability change to Wifi.This case can happen during handover from
             * LTE to Wifi when call is ongoing.
             */
        TelephonyManager tm = (TelephonyManager) mPluginContext
                .getSystemService(Context.TELEPHONY_SERVICE);
        // TODO: for multiSim
        /* ALPS02260621: check phone_type before showing call icon*/
        if ((tm.getCallState() == TelephonyManager.CALL_STATE_OFFHOOK
                        || tm.getCallState() == TelephonyManager.CALL_STATE_RINGING)
                    && mPhoneType == RILConstants.IMS_PHONE) {
            mWfcCallOngoing = true;
            displayWfcCallNotification();
        } else {
            displayWfcRegistrationNotification(true);
        }
    }



    /* Listening screen off intent because no intent for screen lock present in SDK now
     * So, treating screen Off as screen lock
     * Remove notification, if screen off
     */
    private void handleScreenOff() {
        mNotificationManager.cancelAsUser(null, WFC_NOTIFICATION, UserHandle.ALL);
    }

    /* Screen on but check if screen is locked or not. If unlocked, show notification. */
    private void handleScreenOn() {
        if (!mIsScreenLock) {
            Log.d(TAG, "screen not locked & screen on, show notification");
            showNotification();
        }
    }

    /* Intent received when user unlocks. Show notification. */
    private void handleScreenUnlock() {
        showNotification();
    }

    private void showNotification() {
        Log.d(TAG, "[showNotification] mWfcCallOngoing:" + mWfcCallOngoing +
                      " mWfcCapabilityPresent:" + mWfcCapabilityPresent);
        if (mWfcCallOngoing) {
            displayWfcCallNotification();
        } else if (mWfcCapabilityPresent) {
            displayWfcRegistrationNotification(false);
        }
    }

    private void displayWfcCallNotification() {
        Log.d(TAG, "in call handling, screen lock:" + mIsScreenLock);
        if (!mIsScreenLock && mWfcCapabilityPresent) {
            // TODO: to handle fake SRVCC case(wfc registered but during call setup it goes on CS).
            //Need RAT type of call setup
            if (sNotificationBundle.isEmpty()) {
                sNotificationBundle.putString(Notification.EXTRA_SUBSTITUTE_APP_NAME,
                        mPluginContext.getResources().getString(WFC_APP_TITLE));
            }
            Notification noti = new Notification.Builder(mPluginContext)
                    .addExtras(sNotificationBundle)
                    .setContentTitle(mPluginContext.getResources().getString(WFC_CALL_TITLE))
                    .setSmallIcon(WFC_CALL_ICON)
                    .setOngoing(true)
                    .setVisibility(Notification.VISIBILITY_SECRET)
                    .setChannel(WFC_CHANNEL_ID)
                    .build();
            mNotificationManager.notifyAsUser(null, WFC_NOTIFICATION, noti, UserHandle.ALL);
            Log.d(TAG, "showing wfc call notification");
        }
    }

    private void displayWfcRegistrationNotification(boolean showTicker) {
        Log.d(TAG, "in registration handling, screen lock:" + mIsScreenLock);
        if (!mIsScreenLock && mWfcCapabilityPresent
            && mWfcCallOngoing == false) {
            if (sNotificationBundle.isEmpty()) {
                sNotificationBundle.putString(Notification.EXTRA_SUBSTITUTE_APP_NAME,
                        mPluginContext.getResources().getString(WFC_APP_TITLE));
            }
            Notification noti = new Notification.Builder(mPluginContext)
                    .addExtras(sNotificationBundle)
                    .setContentTitle(mPluginContext.getResources().getString(WFC_REGISTERED_TITLE))
                    .setContentText(mPluginContext.getResources().getString(WFC_REGISTERED_SUMMARY))
                    .setSmallIcon(WFC_REGISTERED_ICON)
                    .setOngoing(true)
                    .setVisibility(Notification.VISIBILITY_SECRET)
                    .setChannel(WFC_CHANNEL_ID)
                    .build();
            if (showTicker) {
                noti.tickerText = mPluginContext.getResources().getString(WFC_REGISTERED_TITLE);
            }
            Intent intent = new Intent(ACTION_LAUNCH_WFC_SETTINGS);
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TASK);
            noti.contentIntent = PendingIntent.getActivity(mHostContext, 0, intent, 0);
            noti.flags |= Notification.FLAG_NO_CLEAR;
            mNotificationManager.notifyAsUser(null, WFC_NOTIFICATION, noti, UserHandle.ALL);
            Log.d(TAG, "showing wfc registration notification");
        }
    }

    private void displayWfcErrorNotification() {
        Log.d(TAG, "in error handling, screen lock:" + mIsScreenLock);
        if (!mIsScreenLock) {
            if (sNotificationBundle.isEmpty()) {
                sNotificationBundle.putString(Notification.EXTRA_SUBSTITUTE_APP_NAME,
                        mPluginContext.getResources().getString(WFC_APP_TITLE));
            }
            Notification noti = new Notification.Builder(mPluginContext)
                       .addExtras(sNotificationBundle)
                       .setContentTitle(mPluginContext.getResources().getString(WFC_ERROR_TITLE))
                       .setContentText(mPluginContext.getResources().getString(WFC_ERROR_SUMMARY))
                       .setSmallIcon(WFC_ERROR_ICON)
                       .setOngoing(true)
                       .setVisibility(Notification.VISIBILITY_SECRET)
                       .setChannel(WFC_CHANNEL_ID)
                       .build();
            Intent intent;
            intent = new Intent(ACTION_LAUNCH_WFC_INVALID_SIM_ALERT);
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            /* Need host app context, cz IMSN plugin instance will run in ImsService process
             * i.e. phone process, while op08 plugin runs in system process. So notification
             * intent pending cannot be made for phone process's notification from other process's
             * context/package which here is system process/op08 package.
             * Was getting runtime exception: "Caused by: java.lang.SecurityException: Permission
             * Denial: getIntentSender() from pid=1517, uid=1001, (need uid=1000) is not allowed to
             * send as package com.mediatek.op08.plugin" where 1517 is phone process*/

            noti.contentIntent = PendingIntent.getActivity(mHostContext, 0, intent, 0);
            noti.flags |= Notification.FLAG_NO_CLEAR;
            mNotificationManager.notifyAsUser(null, WFC_NOTIFICATION, noti, UserHandle.ALL);
            Log.d(TAG, "showing sim error notification");
        }
    }

    private void removeWfcNotification() {
        Log.d(TAG, "removing wfc notification, if any");
        mNotificationManager.cancelAsUser(null, WFC_NOTIFICATION, UserHandle.ALL);
        mWfcCapabilityPresent = false;
        mWfcCallOngoing = false;
    }

    /* Observes WFC settings changes. Needed for cases when WFC is switch OFF but
         * state_changes intent is received. Ex: WFC error & user switches WCF OFF.
         */
    private void registerForWfcSwitchChange() {
        mWfcSwitchContentObserver = new ContentObserver(new Handler()) {

            @Override
            public void onChange(boolean selfChange) {
                this.onChange(selfChange, Settings.Global.getUriFor(Settings
                        .Global.WFC_IMS_ENABLED));
            }

            @Override
            public void onChange(boolean selfChange, Uri uri) {
                if (Settings.Global.getInt(mPluginContext.getContentResolver(),
                        Settings.Global.WFC_IMS_ENABLED,
                        ImsConfig.FeatureValueConstants.OFF)
                        == ImsConfig.FeatureValueConstants.OFF) {
                    Log.d(TAG, "contentObserver:WFC OFF");
                    if (!mIsSimPresent || !(mIsISimAppPresent && mIsGBAValid)) {
                        Log.d(TAG, "remove no sim error");
                        mNotificationManager.cancelAsUser(null, WFC_NOTIFICATION, UserHandle.ALL);
                    }
                } else {
                    Log.d(TAG, "contentObserver:WFC ON");
                    if (isWifiEnabled() && (!mIsSimPresent ||
                                                !(mIsISimAppPresent && mIsGBAValid))) {
                        Log.d(TAG, "show error notification");
                        displayWfcErrorNotification();
                    }
                }
            }
        };
        mPluginContext.getContentResolver().registerContentObserver(
                Settings.Global.getUriFor(Settings.Global.WFC_IMS_ENABLED),
               false, mWfcSwitchContentObserver);
    }

    private void registerForImsStateChange() {
        try {
            mImsManager.addRegistrationCallback(mRegistrationCallback);
            mImsManager.addCapabilitiesCallback(mCapabilityCallback);
        } catch (ImsException e) {
            Log.e(TAG, "addCapabilitiesCallback: " + e);
        }
    }

    private ImsMmTelManager.RegistrationCallback mRegistrationCallback =
            new ImsMmTelManager.RegistrationCallback() {
                @Override
                public void onRegistered(
                        @ImsRegistrationImplBase.ImsRegistrationTech int imsRadioTech) {
                    Log.d(TAG, "onImsConnected imsRadioTech=" + imsRadioTech);
                    synchronized (mLock) {
                        mIsRegistered = true;
                        mImsRadioTech = imsRadioTech;

                        handleCapabilityChange();
                    }
                }

                @Override
                public void onRegistering(
                        @ImsRegistrationImplBase.ImsRegistrationTech int imsRadioTech) {
                    Log.d(TAG, "onImsProgressing imsRadioTech=" + imsRadioTech);
                    synchronized (mLock) {
                        mIsRegistered = false;
                        mImsRadioTech = imsRadioTech;
                    }
                }

                @Override
                public void onUnregistered(ImsReasonInfo info) {
                    Log.d(TAG, "onImsDisconnected imsReasonInfo=" + info);
                    synchronized (mLock) {
                        mIsRegistered = false;
                        mImsRadioTech = ImsRegistrationImplBase.REGISTRATION_TECH_NONE;
                    }
                }
            };

    private ImsMmTelManager.CapabilityCallback mCapabilityCallback =
            new ImsMmTelManager.CapabilityCallback() {
                @Override
                public void onCapabilitiesStatusChanged(MmTelFeature.MmTelCapabilities config) {
                    synchronized (mLock) {
                        Log.d(TAG, "onCapabilitiesStatusChanged");
                        if (config.isCapable(
                                MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE)) {
                            Log.d(TAG, "onCapabilitiesStatusChanged: CAPABILITY_TYPE_VOICE");
                            mIsCapabilityTypeVoice = true;
                        }

                        handleCapabilityChange();
                    }
                }
            };

    private void handleCapabilityChange() {
        if (mIsCapabilityTypeVoice && mIsRegistered &&
                mImsRadioTech == ImsRegistrationImplBase.REGISTRATION_TECH_IWLAN) {
            handleInStateService();
        } else {
            mWfcCapabilityPresent = false;
            /* Capabilities have been change to other than WIFI,
                    so set wfc status as OFF */
            removeWfcNotification();
        }
    }

    private void unRegisterForWfcSwitchChange() {
        mPluginContext.getContentResolver().unregisterContentObserver(mWfcSwitchContentObserver);
        mWfcSwitchContentObserver = null;
    }

    private void unRegisterForImsStateChange() {
        try {
            mImsManager.removeRegistrationListener(mRegistrationCallback);
            mImsManager.removeCapabilitiesCallback(mCapabilityCallback);
        } catch (ImsException e) {
            Log.e(TAG, "removeRegistrationListener: " + e);
        }
    }

    private boolean isWifiEnabled() {
        int wifiState = mWifiManager.getWifiState();
        Log.d(TAG, "wifi state:" + wifiState);
        return (wifiState != WifiManager.WIFI_STATE_DISABLED);
    }

    private int getMainCapabilityPhoneId() {
        int phoneId = SystemProperties.getInt(MtkPhoneConstants.PROPERTY_CAPABILITY_SWITCH, 1) - 1;
            if (phoneId < 0 || phoneId >= TelephonyManager.getDefault().getPhoneCount()) {
                phoneId = SubscriptionManager.INVALID_PHONE_INDEX;
            }
        Log.d(TAG, "getMainCapabilityPhoneId = " + phoneId);
        return phoneId;
    }

    private boolean isGbaValid(Context context) {
        final TelephonyManager telephonyManager = TelephonyManager.getDefault();
        String efIst = telephonyManager.getIsimIst();
        if (efIst == null) {
            Log.d(TAG, "[isGbaValid]ISF is NULL");
            return true;
        }
        boolean result = efIst != null && efIst.length() > 1 &&
                                 (0x02 & (byte) efIst.charAt(1)) != 0;
        return result;
    }

    private boolean isIsimAppPresent() {
        IMtkTelephonyEx telEx = IMtkTelephonyEx.Stub.asInterface(ServiceManager
                   .getService("phoneEx"));
        boolean iSimPresent = false;
        try {
            if (telEx != null) {
                iSimPresent = telEx.isAppTypeSupported(SubscriptionManager.getDefaultVoicePhoneId(),
                       PhoneConstants.APPTYPE_ISIM);
            }
        } catch (RemoteException e) {
            Log.d(TAG, "[isIsimAppPresent]IMtkTelephonyEx exceptio:" + e);
        }
        return iSimPresent;
    }

    private int getSubIdUsingPhoneId(int phoneId) {
        int subId;
        int [] values = SubscriptionManager.getSubId(phoneId);
        if (values == null || values.length <= 0) {
            subId = SubscriptionManager.getDefaultSubscriptionId();
        }
        else {
            subId = values[0];
        }
        return subId;
    }

    private boolean checkAndUpdateRoamingChange(ServiceState ss) {
        int voiceRoamingType = ss.getVoiceRoamingType();
        boolean isCellularRoaming = ss.getRoaming();
        int cellularRoamingFromSP = getRoamingStateFromSp(ROAMING_STATE_KEY);
        int cellularRoamingState = IS_UNKNOWN;

        if (isCellularRoaming) {
            cellularRoamingState = IS_ROAMING;
        } else if (voiceRoamingType == ServiceState.ROAMING_TYPE_NOT_ROAMING) {
            cellularRoamingState = IS_HOME;
        }

        Log.d(TAG, "checkAndUpdateRoamingChange voiceRoamingType: " + voiceRoamingType +
                " cellularRoamingState: " + cellularRoamingState +
                " isCellularRoaming: " + isCellularRoaming +
                " cellularRoamingFromSP: " + cellularRoamingFromSP +
                " mCellularRoamingState: " + mCellularRoamingState);

        boolean ret = false;

        if (cellularRoamingState == IS_UNKNOWN) {
            Log.d(TAG, "Skip due to roaming state unknown.");
            return ret;
        }

        if (mCellularRoamingState == IS_UNKNOWN &&
            cellularRoamingFromSP == IS_UNKNOWN) {
            Log.d(TAG, "Could not get any previous roaming state, consider to roaming change.");
            ret = true;
        }

        if (mCellularRoamingState != IS_UNKNOWN && cellularRoamingState != mCellularRoamingState) {
            Log.d(TAG, "checkAndUpdateRoamingChange: Roaming type different from local memory.");
            ret = true;
        }

        if (cellularRoamingFromSP != IS_UNKNOWN && cellularRoamingState != cellularRoamingFromSP) {
            Log.d(TAG,
                    "checkAndUpdateRoamingChange: Roaming type different from SharedPreferences.");
            ret = true;
        }

        mCellularRoamingState = cellularRoamingState;
        saveRoamingStateToSp(cellularRoamingState, ROAMING_STATE_KEY);

        Log.d(TAG, "checkAndUpdateRoamingChange: " + ret);
        return ret;
    }

    private boolean isLTEOn(ServiceState ss) {
        int networkType;
        boolean ret = false;
        if (ss instanceof MtkServiceState) {
            MtkServiceState mtkSs = (MtkServiceState) ss;
            networkType = mtkSs.getCellularDataNetworkType();
            Log.d(TAG, "isLTEOn: getCellularDataNetworkType() = " + networkType);
            if (networkType == TelephonyManager.NETWORK_TYPE_LTE) {
                ret = true;
            }
        } else {
            networkType = ss.getDataNetworkType();
            Log.d(TAG, "isLTEOn: getDataNetworkType() = " + networkType);
            if (networkType == TelephonyManager.NETWORK_TYPE_IWLAN) {
                networkType = ss.getVoiceNetworkType();
                Log.d(TAG, "isLTEOn: getVoiceNetworkType() = " + networkType);
            }

            if (networkType == TelephonyManager.NETWORK_TYPE_LTE) {
                ret = true;
            }
        }
        Log.d(TAG, "isLTEOn: " + ret);
        return ret;
    }

    private boolean isVoWifiOn(int phoneId) {
        boolean ret;
        ret = ImsManager.getInstance(mPluginContext, mPhoneId).isWfcEnabledByUser();
        Log.d(TAG, "isVoWifiOn: " + ret);
        return ret;
    }

    private boolean getBooleanCarrierConfig(String key, int subId) {
        CarrierConfigManager configManager = (CarrierConfigManager) mPluginContext.getSystemService(
                Context.CARRIER_CONFIG_SERVICE);
        PersistableBundle b = null;
        if (configManager != null) {
            b = configManager.getConfigForSubId(subId);
        }

        boolean ret;
        if (b != null) {
            ret = b.getBoolean(key);
        } else {
            // Return static default defined in CarrierConfigManager.
            Log.d(TAG, "getBooleanCarrierConfig: get from default config");
            ret = CarrierConfigManager.getDefaultConfig().getBoolean(key);
        }

        Log.d(TAG, "getBooleanCarrierConfig sub: " + subId + " key: " + key + " ret: " + ret);
        return ret;
    }

    /// For show dialog let user select turnoff WFC or trun on location
    private void showLocationDialog(){
        Log.d(TAG, "showLocationDialog ");
        Intent i = new Intent(mPluginContext, WfcLocationDialogActivity.class);
        i.putExtra("ExtraPhoneId", mPhoneId);
        i.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        mPluginContext.startActivity(i);
    }
}

