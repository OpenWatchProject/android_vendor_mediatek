package com.mediatek.entitlement;

import android.app.ActivityManager;
import android.app.AlarmManager;
import android.app.IActivityManager;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.pm.PackageManager;
import android.net.ConnectivityManager;
import android.net.Network;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteCallbackList;
import android.os.RemoteException;
import android.os.SystemProperties;
import android.os.UserManager;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;
import com.android.ims.ImsManager;
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.PhoneConstants;
import java.util.HashMap;
import java.util.Map;

interface Consumer<T> {
    void accept(T t);
}

public class EntitlementService extends Service {
    private static final String TAG = "EntitlementService";
    private static final boolean DEBUG = true;

    private static final int PENDING_INTENT_REQUEST_CODE_ENTITLEMENT_CHECK = 0;
    private static final int PENDING_INTENT_REQUEST_CODE_PROVISIONING = 2;

    private static final String INTENT_KEY_SLOT_ID = "SLOT_ID";

    // Key for Share Prefernece + slot id
    static private final String PREF_KEY_MANUALLY_TURN_OFF_WFC = "MANUALLY_TURN_OFF_WFC_";

    private Context mContext;
    private SharedPreferences mPref;

    private int mSimCount;
    private EntitlementHandling[] mEntitlementHandling;
    private Provisioning[] mProvisioning;
    private EntitlementCheck[] mEntitlementCheck;

    private RemoteCallbackList<ISesServiceListener>[] mListener;

    // FCMService --> EntitlementService
    private static final String FCM_MESSAGE_RECEIVED_INTENT = "com.mediatek.entitlement.fcm.MESSAGE_RECEIVED";
    private static final String FCM_NEW_TOKEN_INTENT = "com.mediatek.entitlement.fcm.NEW_TOKEN";

    private static final String FCM_BROADCAST_PERMISSION = "com.mediatek.permission.entitlement.FCM_BROADCAST";

    // EntitlementService --> FCMService
    private static final String FCM_REQUEST_TOKEN_INTENT = "com.mediatek.entitlement.ACTION_REQUEST_FCM_TOKEN";

    private static final String FCM_PACKAGE_NAME = "com.mediatek.entitlement.fcm";
    private static final String FCM_SERVICE_NAME = "com.mediatek.entitlement.fcm.FcmService";

    public static final String FCM_INTENT_KEY_DATA = "DATA";
    public static final String FCM_INTENT_KEY_TOKEN = "TOKEN";

    static final String ACTION_PROVISIONING_PENDING = "com.mediatek.entitlement.ACTION_PROVISIONING_PENDING";
    static final String ACTION_ENTITLEMENT_CHECK = "com.mediatek.entitlement.ACTION_ENTITLEMENT_CHECK";

    private String mFcmToken;

    static private final int AUTONOMOUS_STATUS_OFF = 0;
    static private final int AUTONOMOUS_STATUS_ON = 1;
    static private final int AUTONOMOUS_STATUS_CHECK = 2;
    private int[] mAutonomousCheck;

    private boolean mNetworkAvailable;
    private boolean[] mSimLoaded;
    private int mSubId[];

    // Periodically check whether a service is still pending.
    private class Provisioning {
        private final int mSlotId;
        private final int mAlarmSec;
        private int mRetryTimes;
        private boolean mSilence;

        private Intent mIntent;

        Provisioning(int slotId, int alarmSec, int retryTimesMax, boolean silence) {
            log(slotId, "new Provisioning(), alarmSec:" + alarmSec +
                ", times:" + retryTimesMax + ", silence:" + silence);

            mSlotId = slotId;
            mAlarmSec = alarmSec;
            mRetryTimes = retryTimesMax;
            mSilence = silence;

            // Prepare pending intent
            mIntent = new Intent(ACTION_PROVISIONING_PENDING);
            mIntent.putExtra(INTENT_KEY_SLOT_ID, mSlotId);

            setAlarm();
        }

        synchronized boolean shouldRetry() {
            return mRetryTimes > 0;
        }

        synchronized boolean retry() {
            if (!shouldRetry()) {
                return false;
            }

            mRetryTimes--;
            getHandling(mSlotId).startEntitlementCheck(mSilence);
            return true;
        }

        void setAlarm() {
            log(mSlotId, "Provisioning setAlarm(), alarmSec:" + mAlarmSec +
                ", times:" + mRetryTimes);

            PendingIntent pi = PendingIntent.getBroadcast(
                           mContext,
                           PENDING_INTENT_REQUEST_CODE_PROVISIONING + mSlotId,
                           mIntent,
                           PendingIntent.FLAG_UPDATE_CURRENT);

            AlarmManager am = (AlarmManager) getSystemService(Context.ALARM_SERVICE);
            am.setExactAndAllowWhileIdle(AlarmManager.RTC_WAKEUP, System.currentTimeMillis() + 1000 * mAlarmSec, pi);
        }

        void cancelAlarm() {
            log(mSlotId, "Provisioning, cancelAlarm()");
            PendingIntent pi = PendingIntent.getBroadcast(
                           mContext,
                           PENDING_INTENT_REQUEST_CODE_PROVISIONING + mSlotId,
                           mIntent,
                           PendingIntent.FLAG_CANCEL_CURRENT);

            AlarmManager alarmManager = (AlarmManager) getSystemService(Context.ALARM_SERVICE);
            alarmManager.cancel(pi);
        }
    }

    // Once a service is entitled, check the entitlement daily
    private class EntitlementCheck {

        private final int mSlotId;

        private Intent mIntent;

        EntitlementCheck(int slotId) {
            log(slotId, "new EntitlementCheck()");

            mSlotId = slotId;

            // Prepare pending intent
            mIntent = new Intent(ACTION_ENTITLEMENT_CHECK);
            mIntent.putExtra(INTENT_KEY_SLOT_ID, mSlotId);
        }

        void setAlarm() {
            log(mSlotId, "EntitlementCheck, setAlarm()");
            PendingIntent pi = PendingIntent.getBroadcast(
                           mContext,
                           PENDING_INTENT_REQUEST_CODE_ENTITLEMENT_CHECK + mSlotId,
                           mIntent,
                           PendingIntent.FLAG_UPDATE_CURRENT);

            AlarmManager am = (AlarmManager) getSystemService(Context.ALARM_SERVICE);

            // LTE-BTR-5-7106: 48h
            am.setRepeating(
                AlarmManager.RTC_WAKEUP,
                System.currentTimeMillis() + AlarmManager.INTERVAL_DAY * 2,
                AlarmManager.INTERVAL_DAY * 2, pi);

            // LTE-BTR-5-7106: 1min (For IT)
            /*
            am.setRepeating(
                AlarmManager.RTC_WAKEUP,
                System.currentTimeMillis() + 60000,
                60000, pi);
            */
        }

        void cancelAlarm() {
            log(mSlotId, "EntitlementCheck, cancelAlarm()");
            PendingIntent pi = PendingIntent.getBroadcast(
                           mContext,
                           PENDING_INTENT_REQUEST_CODE_ENTITLEMENT_CHECK + mSlotId,
                           mIntent,
                           PendingIntent.FLAG_CANCEL_CURRENT);

            AlarmManager alarmManager = (AlarmManager) getSystemService(Context.ALARM_SERVICE);
            alarmManager.cancel(pi);
        }
    }

    private final BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {

            if (!isEntitlementEnabled()) {
                Log.d(TAG, "onReceive(), isEntitlementEnabled is false, return directly");
                return;
            }

            if (!isOp07()) {
                Log.d(TAG, "onReceive(), isOp07() is false, return directly");
                return;
            }

            String action = intent.getAction();
            Log.d(TAG, "onReceive(), action:" + action);

            if (TelephonyIntents.ACTION_SIM_STATE_CHANGED.equals(action)) {
                int slotId = intent.getIntExtra(PhoneConstants.PHONE_KEY, -1);

                if (slotId < 0 || slotId >= mSimCount) {
                    Log.e(TAG, "SIM_STATE_CHANGED, invalid slotId:" + slotId);
                    return;
                }

                String state = intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);

                log(slotId, "SIM_STATE_CHANGED, state:" + state);

                // when radio off, sim state becomes UNKNOWN. Avoid to disable WFC
                if (state.equals(IccCardConstants.INTENT_VALUE_ICC_UNKNOWN)) {

                    log(slotId, "Not handle unknown state");
                    return;
                }

                boolean loaded = state.equals(IccCardConstants.INTENT_VALUE_ICC_LOADED) ? true : false;

                // new sim or remove sim
                if (mSimLoaded[slotId] != loaded) {
                    notifySimChanged(slotId, loaded);
                }
                mSimLoaded[slotId] = loaded;

                if (mSimLoaded[slotId]) {

                    int subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY,
                        SubscriptionManager.INVALID_SUBSCRIPTION_ID);
                    if (SubscriptionManager.isValidSubscriptionId(subId)) {
                        mSubId[slotId] = subId;
                    } else {
                        Log.e(TAG, "SIM_STATE_CHANGED, invalid subId: " + subId);
                    }

                    startAutonomousCheck(slotId);
                }

            } else if (intent.getAction().equals(Intent.ACTION_USER_UNLOCKED)) {

                // SharedPreference only can be accessed after USER UNLOCKED
                mPref = mContext.getSharedPreferences(TAG, Context.MODE_PRIVATE);

                for (int i = 0; i < mSimCount; i ++) {
                    startAutonomousCheck(i);
                }

            } else if (Intent.ACTION_AIRPLANE_MODE_CHANGED.equals(action)) {
                boolean isAirplaneModeOn = intent.getBooleanExtra("state", false);

                // CDR-WiFi-1120: check AID expire time
                if (mNetworkAvailable && !isAirplaneModeOn) {

                    for (int i = 0; i < mSimCount; i++) {
                        getHandling(i).validateAid();
                    }
                }
            } else if (ACTION_ENTITLEMENT_CHECK.equals(action)) {

                int slotId = intent.getIntExtra(INTENT_KEY_SLOT_ID, -1);

                if (slotId < 0 || slotId >= mSimCount) {
                    Log.e(TAG, "ACTION_ENTITLEMENT_CHECK, invalid slotId:" + slotId);
                    return;
                }

                getHandling(slotId).startEntitlementCheck(true);

                // LTE-BTR-5-7256 Step5: retry the SES entitlement check in 5 mins
                initProvisioning(slotId, 5 * 60, 1, true);

            } else if (ACTION_PROVISIONING_PENDING.equals(action)) {

                int slotId = intent.getIntExtra(INTENT_KEY_SLOT_ID, -1);

                if (slotId < 0 || slotId >= mSimCount) {
                    Log.e(TAG, "ACTION_PROVISIONING_PENDING, invalid slotId:" + slotId);
                    return;
                }

                if (mProvisioning[slotId] != null) {
                    mProvisioning[slotId].retry();
                }
            }

        }
    };

    private final BroadcastReceiver mFcmReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {

            if (!isEntitlementEnabled()) {
                Log.d(TAG, "mFcmReceiver onReceive(), isEntitlementEnabled is false");
                return;
            }

            if (!isOp07()) {
                Log.d(TAG, "mFcmReceiver onReceive(), isOp07() is false");
                return;
            }

            String action = intent.getAction();
            Log.d(TAG, "onReceive(), action:" + action);

            if (FCM_NEW_TOKEN_INTENT.equals(action)) {

                // Store token in service to avoid the null EntitlementHandling loses it
                String token = intent.getStringExtra(FCM_INTENT_KEY_TOKEN);

                if (token == null) {
                    Log.e(TAG, "FCM_INTENT_KEY_TOKEN, token is null");
                    return;
                }

                mFcmToken = token;

                for (int i = 0; i < mSimCount; i ++) {
                    startAutonomousCheck(i);
                }

                for (int i = 0; i < mSimCount; i++) {
                    getHandling(i).updateFcmToken(mFcmToken);
                }
            } else if (FCM_MESSAGE_RECEIVED_INTENT.equals(action)) {

                HashMap<String, String> map = (HashMap)intent.getExtras().getSerializable(FCM_INTENT_KEY_DATA);
                String payload = map.get("message");
                for (int i = 0; i < mSimCount; i++) {
                    getHandling(i).handleFcmNotification(payload);
                }

            }
        }
    };

    @Override
    public void onCreate() {
        super.onCreate();

        Log.d(TAG, "onCreate()");

        if (!isEntitlementEnabled()) {
            Log.d(TAG, "onCreate(), isEntitlementEnabled is false, return directly");
            return;
        }

        initParameters();

        final IntentFilter filter = new IntentFilter();
        filter.addAction(ACTION_ENTITLEMENT_CHECK);
        filter.addAction(ACTION_PROVISIONING_PENDING);
        filter.addAction(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        filter.addAction(Intent.ACTION_USER_UNLOCKED);
        filter.addAction(TelephonyIntents.ACTION_SIM_STATE_CHANGED);
        registerReceiver(mBroadcastReceiver, filter);

        final IntentFilter fcmFilter = new IntentFilter();
        fcmFilter.addAction(FCM_MESSAGE_RECEIVED_INTENT);
        fcmFilter.addAction(FCM_NEW_TOKEN_INTENT);
        registerReceiver(mFcmReceiver, fcmFilter, FCM_BROADCAST_PERMISSION, null);

        registerDefaultNetwork();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();

        Log.d(TAG, "onDestroy(), check why ?");

        if (!isEntitlementEnabled()) {
            Log.d(TAG, "onDestroy(), isEntitlementEnabled is false, return directly");
            return;
        }
        unregisterReceiver(mBroadcastReceiver);
    }

    private void initParameters() {

        mContext = getApplicationContext();

        TelephonyManager tm = (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);
        if (tm == null) {
            Log.e(TAG, "initParameters() fail !!! telephony is null");
        }

        mSimCount = tm.getSimCount();
        mAutonomousCheck = new int[mSimCount];
        mEntitlementHandling = new EntitlementHandling[mSimCount];
        mProvisioning = new Provisioning[mSimCount];
        mEntitlementCheck = new EntitlementCheck[mSimCount];
        mSimLoaded = new boolean[mSimCount];
        mSubId = new int[mSimCount];
        mListener = new RemoteCallbackList[mSimCount];

        for (int i = 0; i < mSimCount; i++) {

            mAutonomousCheck[i] = AUTONOMOUS_STATUS_ON;
            mProvisioning[i] = null;
            mEntitlementCheck[i] = new EntitlementCheck(i);
            mSimLoaded[i] = false;
            mSubId[i] = SubscriptionManager.INVALID_SUBSCRIPTION_ID;
            mListener[i] = new RemoteCallbackList<ISesServiceListener>();
        }
    }

    private EntitlementHandling getHandling(int slotId) {

        if (mEntitlementHandling[slotId] == null) {
            mEntitlementHandling[slotId] = new EntitlementHandling(slotId, mContext,
                mEntitlementHandlingListener);

            // Update FCM token
            if (mFcmToken != null) {
                mEntitlementHandling[slotId].updateFcmToken(mFcmToken);
            }
        }

        if (mFcmToken == null) {
            requestFcmToken();
        }

        return mEntitlementHandling[slotId];
    }

    private void requestFcmToken() {
        Log.d(TAG, "requestFcmToken(), start FCM service");

        Intent intent = new Intent(FCM_REQUEST_TOKEN_INTENT);
        intent.setComponent(new ComponentName(FCM_PACKAGE_NAME, FCM_SERVICE_NAME));

        ComponentName component = mContext.startForegroundService(intent);
        Log.d(TAG, "component:" + component);
    }


    // One listener for dual SIM
    EntitlementHandling.Listener mEntitlementHandlingListener = new EntitlementHandling.Listener() {
        @Override
        public synchronized void onStateChange(final int slotId, int state, boolean silence) {
            log(slotId, "onStateChange(), state:" + EntitlementHandling.stateToString(state) +
                ", silence:" + silence);

            boolean retryFail = false;

            if (state == EntitlementHandling.STATE_ENTITLED) {
                mEntitlementCheck[slotId].setAlarm();

                // no need to retry in 30/60mins or 2mins (autonomous check/48h)
                removeProvisioning(slotId);

                //CDR-WiFi-1080: 1. toggle WFC on automatically
                if (mAutonomousCheck[slotId] != AUTONOMOUS_STATUS_OFF) {
                    checkIfEnableWfc(slotId);
                }
            } else {
                mEntitlementCheck[slotId].cancelAlarm();

                if (state == EntitlementHandling.STATE_NOT_ENTITLED ||
                    state == EntitlementHandling.STATE_PENDING_WITH_NO_TC ||
                    state == EntitlementHandling.STATE_PENDING_WITH_NO_ADDRESS) {

                    if (mProvisioning[slotId] != null) {
                        if (mProvisioning[slotId].shouldRetry()) {
                            mProvisioning[slotId].setAlarm();
                        } else {
                            // provisioning failed
                            mProvisioning[slotId] = null;
                            log(slotId, "Retry done. Notify entitlement failed");
                            // For LTE-BTR-5-7112, show "provisioning failed, please try again later"
                            // LTE-BTR-5-7112, retrying stops on STATE_PENDING_WITH_ADDRESS
                            retryFail = true;
                        }
                    }

                }

                //CDR-WiFi-1080: 2.b: toggle wfc off if no E911AID
                if (state == EntitlementHandling.STATE_PENDING_WITH_NO_TC ||
                    state == EntitlementHandling.STATE_PENDING_WITH_NO_ADDRESS) {

                    disableWfc(slotId, silence);
                }

                if (state == EntitlementHandling.STATE_NOT_ENTITLED) {

                    //CDR-WiFi-1080:
                    // 2: turn Wi-Fi Calling toggle off for 1063
                    // 4: leave the Wi-Fi Calling toggle unchanged
                    if (mAutonomousCheck[slotId] == AUTONOMOUS_STATUS_OFF) {

                        disableWfc(slotId, silence);
                    }
                }
            }

            if (!silence) {
                final boolean failed = retryFail;
                notifyListeners(slotId, new Consumer<ISesServiceListener>() {
                    @Override
                    public void accept(ISesServiceListener l) {
                        try {
                            if (failed) {
                                l.onEntitlementEvent(slotId, "failed", null);
                            } else {
                                l.onEntitlementEvent(slotId, translateToExternalState(state), null);
                            }
                        } catch (RemoteException ignored) {
                        }
                    }
                });
            }
        }

        @Override
        public synchronized void onWebsheetPost(final int slotId, String url, String serverData) {
            log(slotId, "onWebsheetPost(), url: " + url);

            notifyListeners(slotId, new Consumer<ISesServiceListener>() {
                @Override
                public void accept(ISesServiceListener l) {
                    try {
                        l.onWebsheetPost(slotId, url, serverData);
                    } catch (RemoteException ignored) {
                    }
                }
            });
        }

        @Override
        public synchronized void onInfo(final int slotId, Bundle info) {
            log(slotId, "Info discovered: " + info);

            // Handle internal info first
            boolean handled = handleInternalInfo(slotId, info);
            if (handled) {
                return;
            }

            notifyListeners(slotId, new Consumer<ISesServiceListener>() {
                @Override
                public void accept(ISesServiceListener l) {
                    try {
                        l.onEntitlementEvent(slotId, "info", info);
                    } catch (RemoteException ignored) {
                    }
                }
            });
        }
    };

    private boolean handleInternalInfo(int slotId, Bundle info) {

        if (!info.containsKey(EntitlementHandling.ENTITLEMENT_CHECK_RETRY_TIMES) ||
            !info.containsKey(EntitlementHandling.ENTITLEMENT_CHECK_RETRY_SILENCE)) {
            return false;
        }

        int times = info.getInt(EntitlementHandling.ENTITLEMENT_CHECK_RETRY_TIMES);

        boolean silence =
            info.getInt(EntitlementHandling.ENTITLEMENT_CHECK_RETRY_SILENCE) == 1 ? true : false;

        log(slotId, "ENTITLEMENT_CHECK_RETRY_TIMES times: " + times + " silence:" + silence);

        // LTE_BTR-5-7250: step2, 1063 no retry
        if (times == 0) {
            removeProvisioning(slotId);
        }
        // LTE_BTR-5-7250: step4, retry 4 times with 30s interval
        else if (times == 4) {
            removeProvisioning(slotId);
            initProvisioning(slotId, 30, times, silence);
        }
        return true;

    }

    private void notifySimChanged(final int slotId, boolean loaded) {
        log(slotId, "notifySimChanged(), loaded: " + loaded);

        if (loaded) {

            // Notify UI
            notifyListeners(slotId, new Consumer<ISesServiceListener>() {
                @Override
                public void accept(ISesServiceListener l) {
                    try {
                        l.onEntitlementEvent(slotId, "new_sim", null);
                    } catch (RemoteException ignored) {
                    }
                }
            });
        } else {

            // Notify UI
            notifyListeners(slotId, new Consumer<ISesServiceListener>() {
                @Override
                public void accept(ISesServiceListener l) {
                    try {
                        l.onEntitlementEvent(slotId, "remove_sim", null);
                    } catch (RemoteException ignored) {
                    }
                }
            });

            log(slotId, "REMOVE_SIM, reset all state and set wfc disabled via storage");

            // Destroy handling and re-create again if need when sim switch
            mEntitlementHandling[slotId] = null;

            // in case of reset state, turn off vowifi
            //ImsManager imsManager = ImsManager.getInstance(mContext, slotId);
            //imsManager.setWfcSetting(false);

            // Disable WFC when no SIM
            SubscriptionManager.setSubscriptionProperty(
                mSubId[slotId], SubscriptionManager.WFC_IMS_ENABLED, "0");

            // When a new SIM is inserted, autonomous check won't auto enable wfc
            setManuallyTurnOffWfc(slotId, true);
        }

    }

    private void notifyListeners(int slotId, Consumer<ISesServiceListener> consumer) {
        synchronized (mListener[slotId]) {
            int i = mListener[slotId].beginBroadcast();
            while (i > 0) {
                i--;
                consumer.accept(mListener[slotId].getBroadcastItem(i));
            }
            mListener[slotId].finishBroadcast();
        }
    }

    private synchronized void initProvisioning(
        int slotId, int retryTimer, int retryTimes, boolean silence) {

        // For LTE-BTR-5-7112, avoid websheet reset retry counter
        if (mProvisioning[slotId] == null || !mProvisioning[slotId].shouldRetry()) {
            mProvisioning[slotId] = new Provisioning(slotId, retryTimer, retryTimes, silence);
        }

    }

    private synchronized void removeProvisioning(int slotId) {

        if (mProvisioning[slotId] != null) {
            log(slotId, "removeProvisioning(), cancel alarm");
            mProvisioning[slotId].cancelAlarm();
            mProvisioning[slotId] = null;
        }
    }


    @Override
    public IBinder onBind(Intent intent) {
        Log.d(TAG, "onBind: " + intent);
        return new ISesService.Stub() {
            @Override
            public void startEntitlementCheck(int slotId, int retryTimer, int retryTimes) {
                log(slotId, "[EXT]startEntitlementCheck()" +
                    "retryTimer:" + retryTimer + ", retryTimes:" + retryTimes +
                    ", mAutonomousCheck:" + mAutonomousCheck[slotId]);

                // Cancel autonomous task
                removeProvisioning(slotId);
                mAutonomousCheck[slotId] = AUTONOMOUS_STATUS_OFF;

                setManuallyTurnOffWfc(slotId, false);

                if (retryTimer > 0) {
                    initProvisioning(slotId, retryTimer * 60, retryTimes, false);
                } else {
                    getHandling(slotId).startEntitlementCheck(false);
                }
            }

            @Override
            public void stopEntitlementCheck(int slotId) {
                log(slotId, "[EXT]stopEntitlementCheck()");

                getHandling(slotId).stopEntitlementCheck();
            }

            @Override
            public void updateLocationAndTc(int slotId) {
                log(slotId, "[EXT]updateLocationAndTc()");

                mAutonomousCheck[slotId] = AUTONOMOUS_STATUS_OFF;

                getHandling(slotId).updateLocationAndTc(true);
            }

            @Override
            public String getCurrentEntitlementState(int slotId) {
                log(slotId, "[EXT]getCurrentEntitlementState()");

                return translateToExternalState(getHandling(slotId).getState());
            }

            @Override
            public void registerListener(int slotId, ISesServiceListener listener) {
                log(slotId, "[EXT]registerStateListener");
                if (listener != null) {
                    synchronized (mListener[slotId]) {
                        log(slotId, "registerStateListener " + listener.getClass().toString());
                        mListener[slotId].register(listener);
                    }
                }
            }

            @Override
            public void unregisterListener(int slotId, ISesServiceListener listener) {
                if (listener != null) {
                    synchronized (mListener[slotId]) {
                        log(slotId, "[EXT]unregisterStateListener " + listener.getClass().toString());
                        mListener[slotId].unregister(listener);
                    }
                }
            }

            @Override
            public int getLastErrorCode(int slotId) {
                int errCode = ErrorCodes.getErrorCode(slotId);
                log(slotId, "[EXT]getErrorCode = " + errCode);
                return errCode;
            }

            @Override
            public void deactivateService(int slotId) {
                log(slotId, "[EXT]deactivateService");

                setManuallyTurnOffWfc(slotId, true);

                getHandling(slotId).deactivateService(false);
            }
        };
    }

    private String translateToExternalState(int internalState) {
        switch (internalState) {
            case EntitlementHandling.STATE_NOT_ENTITLED:
                return "not-entitled";
            case EntitlementHandling.STATE_ACTIVATING:
            case EntitlementHandling.STATE_ENTITLEMENT_CHECKING:
            case EntitlementHandling.STATE_PENDING_WITH_NO_TC:
            case EntitlementHandling.STATE_PENDING_WITH_NO_ADDRESS:
            case EntitlementHandling.STATE_DEACTIVATING:
            case EntitlementHandling.STATE_DEACTIVATING_PENDING_CHECK:
                return "pending";
            case EntitlementHandling.STATE_ENTITLED:
                return "entitled";
            default:
                Log.e(TAG, "Unhandled state: " + internalState);
                return "Unhandled";
        }
    }

    private void checkIfEnableWfc(int slotId) {

        ImsManager imsManager = ImsManager.getInstance(mContext, slotId);
        boolean wfcEnabled = imsManager.isWfcEnabledByUser();
        boolean manuallyTurnOffWfc = getManuallyTurnOffWfc(slotId);

        log(slotId, "checkIfEnableWfc(), manuallyTurnOffWfc:" + manuallyTurnOffWfc +
            ", wfcEnabled:" + wfcEnabled);

        // CDR-WiFi-1080: wfc toggle was not turned off manually by the user
        if (!manuallyTurnOffWfc && !wfcEnabled) {

            log(slotId, "Enable WFC automatically");
            imsManager.setWfcSetting(true);
        }
    }

    private void disableWfc(int slotId, boolean silence) {

        ImsManager imsManager = ImsManager.getInstance(mContext, slotId);
        boolean wfcEnabled = imsManager.isWfcEnabledByUser();

        log(slotId, "disableWfc(), wfcEnabled:" + wfcEnabled + ", silence:" + silence);

        if (wfcEnabled) {

            // Perform push token removal
            getHandling(slotId).deactivateService(silence);

            log(slotId, "toggle wfc off automatically");
            imsManager.setWfcSetting(false);
        }
    }

    private synchronized void startAutonomousCheck(int slotId) {

        UserManager userManager = (UserManager) getSystemService(Context.USER_SERVICE);
        boolean unlocked = userManager.isUserUnlocked();

        log(slotId, "startAutonomousCheck(), mAutonomousCheck:" + mAutonomousCheck[slotId] +
            ", mSimLoaded:" + mSimLoaded[slotId] + ", mNetworkAvailable:" + mNetworkAvailable +
            ", unlocked:" + unlocked + ", fcmTokenReady:" + (mFcmToken != null));

        if (isOp07() && unlocked && mNetworkAvailable && mSimLoaded[slotId] &&
            mAutonomousCheck[slotId] == AUTONOMOUS_STATUS_ON) {

            if (mFcmToken == null) {
                requestFcmToken();

            } else {

                mAutonomousCheck[slotId] = AUTONOMOUS_STATUS_CHECK;
                getHandling(slotId).startEntitlementCheck(true);

                // CDR-WiFi-1080: device should retry once in 5 mins
                // CDR-WiFi-1080: no open websheet URL when autonomous entitlement check
                initProvisioning(slotId, 5 * 60, 1, true);
            }
        }
    }

    private void registerDefaultNetwork() {

        ConnectivityManager cm = ConnectivityManager.from(mContext);
        cm.registerDefaultNetworkCallback(new ConnectivityManager.NetworkCallback() {
            @Override
            public void onAvailable(Network network) {
                Log.d(TAG, "NetworkCallback.onAvailable()");

                mNetworkAvailable = true;

                for (int i = 0; i < mSimCount; i ++) {
                    startAutonomousCheck(i);
                }
            }

            @Override
            public void onLost(Network network) {
                Log.d(TAG, "NetworkCallback.onLost()");

                mNetworkAvailable = false;

            }
        });
    }

    private void setManuallyTurnOffWfc(int slotId, boolean value) {
        log(slotId, "setManuallyTurnOffWfc(), value:" + value);

        if (mPref != null) {
            Editor editor = mPref.edit();
            editor.putBoolean(PREF_KEY_MANUALLY_TURN_OFF_WFC + slotId, value);
            editor.apply();
        }
    }

    private boolean getManuallyTurnOffWfc(int slotId) {

        if (mPref != null) {
            return mPref.getBoolean(PREF_KEY_MANUALLY_TURN_OFF_WFC + slotId, false);
        } else {
            return false;
        }
    }

    private boolean isEntitlementEnabled() {
        boolean isEntitlementEnabled = (1 == SystemProperties.getInt
                ("persist.vendor.entitlement_enabled", 1) ? true : false);
        return isEntitlementEnabled;
    }

    private boolean isOp07() {

        String optr = SystemProperties.get("persist.vendor.operator.optr");

        if (null != optr && optr.equals("OP07")) {
            return true;
        } else {
            Log.d(TAG, "isOp07() is false !!! optr:" + optr);
            return false;
        }
    }

    private static void log(int slotId, String s) {
        if (DEBUG) Log.d(TAG,  "[" + slotId + "]" + s);
    }

    private void loge(int slotId, String s) {
        Log.e(TAG,  "[" + slotId + "]" + s);
    }
}
