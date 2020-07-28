package com.mediatek.entitlement.o2;

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkInfo;
import android.net.NetworkRequest;
import android.net.wifi.WifiManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.SystemProperties;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;
import android.util.SparseArray;
import com.android.ims.ImsConfig;
import com.android.ims.ImsException;
import com.android.ims.ImsManager;
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Vector;

interface Consumer<T> {
    void accept(T t);
}

public class EntitlementService extends Service {
    private static final String TAG = "EntitlementService";
    private static final boolean DEBUG = true;
    private static final String PROP_FORCE_DEBUG_KEY = "persist.vendor.log.tel_dbg";
    private static final boolean ENG = "eng".equals(Build.TYPE);
    private static final boolean TELDBG = (SystemProperties.getInt(PROP_FORCE_DEBUG_KEY, 0) == 1);

    private static final String SERVICE_STATUS_PREF = "EntitlementPref";
    //private static final String VLT_SETTING_SYTEM_PROPERTY = "persist.vendor.ril.vt.setting.support";
    //private static final String WFC_SETTING_SYTEM_PROPERTY = "persist.vendor.ril.wfc.setting.support";
    private static final String VOWIFI_SERVICE = "VoWiFi"; //"vowifi";
    private static final String VOLTE_SERVICE = "VoLTE"; //"volte";
    private static final String ICCID_KEY = "iccid_key";
    private static final String INTENT_KEY_SLOT_ID = "SLOT_ID";
    private static final String INTENT_KEY_SERVICE_TYPE = "service";
    private static final String MULTI_IMS_SUPPORT = "persist.vendor.mims_support";
    private static final String PROPERTY_CAPABILITY_SWITCH = "persist.vendor.radio.simswitch";
    private static final String ACTION_PROVISIONING_PENDING =
            "com.mediatek.entitlement.o2.ACTION_PROVISIONING_PENDING";
    private static final String ACTION_ENTITLEMENT_CHECK =
            "com.mediatek.entitlement.o2.ACTION_ENTITLEMENT_CHECK";
    private static final int WAIT_DEFAULT_NETWORK_MILLIS = 30 * 1000;
    private static final int RETRY_TIMER = 30; //minutes.
    private static final int RETRY_COUNT = 5;
    private static final int EVENT_REQUEST_SEND = 1;
    private static final int EVENT_REQUEST_RESPONSE = 2;
    private static final int EVENT_REQUEST_RESET = 3;
    private static final int EVENT_RETRY_GET_IMS_SERVICE_1 = 400;
    private static final int EVENT_RETRY_GET_IMS_SERVICE_2 = 401;
    private static final int EVENT_RETRY_GET_IMS_SERVICE_3 = 402;
    private static final int EVENT_RETRY_GET_IMS_SERVICE_4 = 403;

    private static final int TIMEOUT_RETRY_GET_IMS_SERVICE = 500;

    private static final List<String> OP15_MCCMNC_LIST = Arrays.asList("23410");

    private Context mContext;
    private Context mDeviceContext;
    private ConnectivityManager mConnectivityManager;
    private int mSimCount;
    private boolean[] mSimLoaded;
    //private final Map<String, EntitlementHandling> mServices = new HashMap<>();
    private EntitlementHandling[] mHandling;
    private Map<String, Provisioning>[] mProvisionings;
    private EntitlementCheck[] mEntitlementCheck;
    private boolean[] mEntitlementStarted;
    private final SparseArray<EntitlementNetworkManager> mNetworkManagers = new SparseArray<>();
    private Network mNetwork;
    private int mDefaultNetworkState = -1;
    private ConnectivityManager.NetworkCallback mDefaultNetworkCallback;
    private HandlerThread mRequestThread;
    private EntitlementRequestHandler mRequestHandler;
    private final Object mLock = new Object();

    private static String[] mProvisionedService;
    /* Static objects to do synchronized */
    private static Object pausedSync = new Object();
    private static int mRefCount;

    // Periodically check whether a service is still pending.
    private class Provisioning {
        private final String mService;
        private final int mAlarmInterval;
        private final int mSlotId;
        private int mRetryTimes;

        Provisioning(String service, int alarmInterval, int retryTimesMax, int slotId) {
            mService = service;
            mAlarmInterval = alarmInterval;
            mSlotId = slotId;
            mRetryTimes = retryTimesMax;
            updateRetryStatus(service, mRetryTimes, mSlotId);
            setAlarm();
        }

        synchronized boolean shouldRetry() {
            log(mSlotId, "In shouldRetry(), mRetryTimes = " + mRetryTimes);
            return mRetryTimes > 0;
        }

        synchronized boolean retry() {
            if (!shouldRetry()) {
                return false;
            }
            log(mSlotId, "Start retry Mechanism for service = " + mService);
            mRetryTimes--;
            updateRetryStatus(mService, mRetryTimes, mSlotId);
            //getHandling(mService).startEntitlementCheck();
            //startEntitlement(mServiceList); // populate list.
            return true;
        }

        void setAlarm() {
            Intent i = new Intent(ACTION_PROVISIONING_PENDING);
            i.putExtra(INTENT_KEY_SERVICE_TYPE, mService);
            i.putExtra(INTENT_KEY_SLOT_ID, mSlotId);
            log(mSlotId, "Setting Retry alarm for Service = " + mService);
            PendingIntent pi = PendingIntent.getBroadcast(
                    mContext, 10 + mSlotId, i, PendingIntent.FLAG_UPDATE_CURRENT);
            AlarmManager am = (AlarmManager) getSystemService(Context.ALARM_SERVICE);
            am.set(AlarmManager.RTC_WAKEUP,
                    System.currentTimeMillis() + 1000 * 60 * mAlarmInterval, pi);
        }

        void cancelAlarm() {
            Intent i = new Intent(ACTION_PROVISIONING_PENDING);
            i.putExtra(INTENT_KEY_SERVICE_TYPE, mProvisionedService[mSlotId]);
            i.putExtra(INTENT_KEY_SLOT_ID, mSlotId);
            log(mSlotId, "Cancel Retry alarm srv = " + mService +
                    ", sent= " + mProvisionedService[mSlotId]);
            mProvisionedService[mSlotId] = null;
            PendingIntent pi = PendingIntent.getBroadcast(
                    mContext, 10 + mSlotId, i, PendingIntent.FLAG_CANCEL_CURRENT);
            AlarmManager alarmManager = (AlarmManager) getSystemService(Context.ALARM_SERVICE);
            alarmManager.cancel(pi);
        }
    }

    // Once a service is entitled, check the entitlement daily
    private class EntitlementCheck {
        private final int mSlotId;
        private final HashMap<String, Boolean> mServices = new HashMap<String, Boolean>();

        EntitlementCheck(int slotId) {
            log(slotId, "new EntitlementCheck()");
            mSlotId = slotId;
        }

        void setAlarm(String service) {
            if (mServices.get(service) == null) {
                mServices.put(service, true);
            }
            int alarmInterval = mHandling[mSlotId].getPollInterval();
            log(mSlotId, "Received alarm interval = " + alarmInterval + "service:" + service);
            Intent i = new Intent(ACTION_ENTITLEMENT_CHECK);
            i.putExtra(INTENT_KEY_SERVICE_TYPE, service);
            i.putExtra(INTENT_KEY_SLOT_ID, mSlotId);
            PendingIntent pi = PendingIntent.getBroadcast(
                    mContext, mSlotId, i, PendingIntent.FLAG_UPDATE_CURRENT);
            AlarmManager am = (AlarmManager) getSystemService(Context.ALARM_SERVICE);
            am.setRepeating(AlarmManager.RTC_WAKEUP,
                    System.currentTimeMillis() + 1000 * 60 * 60 * alarmInterval,
                    1000 * 60 * 60 * alarmInterval/*AlarmManager.INTERVAL_DAY*/, pi);
        }

        void cancelAlarm(String service) {
            log(mSlotId, "EntitlementCheck: cancelAlarm for service:" + service);
            if (mServices.get(service) != null) {
                mServices.remove(service);
            }
            if (mServices.size() == 0) {
                Intent i = new Intent(ACTION_ENTITLEMENT_CHECK);
                i.setAction(ACTION_ENTITLEMENT_CHECK);
                i.putExtra(INTENT_KEY_SERVICE_TYPE, service);
                i.putExtra(INTENT_KEY_SLOT_ID, mSlotId);
                PendingIntent pi = PendingIntent.getBroadcast(
                        mContext, mSlotId, i, PendingIntent.FLAG_CANCEL_CURRENT);
                AlarmManager alarmManager = (AlarmManager) getSystemService(Context.ALARM_SERVICE);
                alarmManager.cancel(pi);
            }
        }
    }

    class EntitlementRequestHandler extends Handler implements Runnable {
        private Vector<Message> requestBuffer = new Vector<Message>(); /* Message Queue Buffer */
        private boolean paused = false;  /* Flag indicating the pause state of handler*/
        private int mRequestSim = -1;

        public EntitlementRequestHandler(Looper looper) {
            super(looper);
        }

        //***** Runnable implementation
        public void
            run() {
            //setup if needed
        }

        @Override
        public void handleMessage(Message msg) {
            log(msg.arg1, "handleMessage(), msg.what = " + msg.what + " , paused = " + paused);
            switch (msg.what) {
                case EVENT_REQUEST_SEND:
                    synchronized (pausedSync) {
                        if (paused) {
                            log("A request ongoing, add it into the queue");
                            Message msgCopy = new Message();
                            msgCopy.copyFrom(msg);
                            requestBuffer.add(msgCopy);
                        } else {
                            /* process the request by passing SuppSrvRequest object and phoneId */
                            mRequestSim = msg.arg1;
                            processRequest(msg.obj, msg.arg1);
                            paused = true;
                        }
                    }
                    break;
                case EVENT_REQUEST_RESPONSE:
                    synchronized (pausedSync) {
                        if (mRequestSim != msg.arg1) {
                            break;
                        }
                        mRequestSim = -1;
                        processResponse(msg.arg1);
                        paused = false;
                        if (requestBuffer.size() > 0) {
                            Message request = requestBuffer.elementAt(0);
                            requestBuffer.removeElementAt(0);
                            sendMessage(request);
                        }
                    }
                    break;
                case EVENT_REQUEST_RESET:
                    synchronized (pausedSync) {
                        processReset(msg.arg1);
                        for (Message request : requestBuffer) {
                            if (request.arg1 == msg.arg1) {
                                requestBuffer.remove(request);
                            }
                        }
                    }
                    break;
                case EVENT_RETRY_GET_IMS_SERVICE_1:
                case EVENT_RETRY_GET_IMS_SERVICE_2:
                case EVENT_RETRY_GET_IMS_SERVICE_3:
                case EVENT_RETRY_GET_IMS_SERVICE_4:
                    if (!getImsServiceDone(msg.arg1, msg.obj)) {
                        retryGetImsService(msg.arg1, msg.obj);
                    }
                    break;
                default:
                    log("EntitlementRequestHandler:msg.what=" + msg.what);
            }
        }

        private void processRequest(Object obj, int slotId) {
            ArrayList<String> serviceList = (ArrayList<String>) obj;
            mRefCount = serviceList.size();
            //for (String service : serviceList) {
                //deProvisionService(service, slotId);
            //}
            //mHandling[slotId] = null;
            if ((needRequestNetwork() && mNetwork == null) ||
                    (getSubIdUsingPhoneId(slotId) !=
                     SubscriptionManager.getDefaultDataSubscriptionId())) {
                requestNetwork(slotId);
            }
            getHandling(slotId).startEntitlementCheck(serviceList);
        }

        private void processResponse(int slotId) {
            releaseNetwork(slotId);
        }

        private void processReset(int slotId) {
            synchronized (mLock) {
                ImsManager imsManager = ImsManager.getInstance(mContext, slotId);
                if (imsManager.isServiceReady()) {
                    if (!isMccMncReady(slotId) || isOp15Card(slotId)) {
                        deProvisionServices(slotId);
                    }
                    // in case of reset state, turn off vowifi
                    //ImsManager.getInstance(mContext, slotId).setWfcSetting(false);
                } else {
                    ImsRequest request =
                            new ImsRequest(slotId, ImsRequest.IMS_REQUEST_DEPROVISION_SERVICES);
                    retryGetImsService(slotId, request);
                }
                if (mHandling[slotId] != null) {
                    mHandling[slotId].stopEntitlementCheck();
                }
                mHandling[slotId] = null;
                synchronized (mProvisionings[slotId]) {
                    if (mProvisionedService[slotId] != null) {
                        if (mProvisionings[slotId] != null) {
                            Provisioning provisioning =
                                    mProvisionings[slotId].get(mProvisionedService[slotId]);
                            if (provisioning != null) {
                                provisioning.cancelAlarm();
                            }
                            mProvisionedService[slotId] = null;
                        }
                    }
                    mProvisionings[slotId].clear();
                }
                mEntitlementCheck[slotId].cancelAlarm(VOLTE_SERVICE);
                mEntitlementCheck[slotId].cancelAlarm(VOWIFI_SERVICE);
                updateServiceStatus(VOLTE_SERVICE, EntitlementHandling.STATE_INITIAL, slotId);
                updateServiceStatus(
                        VOWIFI_SERVICE, EntitlementHandling.STATE_INITIAL, slotId);
                updateRetryStatus(VOLTE_SERVICE, 0, slotId);
                updateRetryStatus(VOWIFI_SERVICE, 0, slotId);
                if (mRequestSim == slotId) {
                    sendResponse(slotId);
                    mRefCount = 0;
                }
            }
        }
    }

    private void addRequest(ArrayList<String> serviceList, int slotId) {
        if (mRequestHandler != null) {
            Message msg = mRequestHandler.obtainMessage(EVENT_REQUEST_SEND, slotId, 0, serviceList);
            msg.sendToTarget();
        }
    }

    private void sendResponse(int slotId) {
        if (mRequestHandler != null) {
            Message msg = mRequestHandler.obtainMessage(EVENT_REQUEST_RESPONSE, slotId, 0);
            msg.sendToTarget();
        }
    }

    private void sendReset(int slotId) {
        if (mRequestHandler != null) {
            Message msg = mRequestHandler.obtainMessage(EVENT_REQUEST_RESET, slotId, 0);
            msg.sendToTarget();
        }
    }

    private final BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            log("onReceive(), action:" + action);

            if (TelephonyIntents.ACTION_SIM_STATE_CHANGED.equals(action)) {
                int slotId = intent.getIntExtra(PhoneConstants.PHONE_KEY, -1);

                if (slotId < 0 || slotId >= mSimCount) {
                    loge("SIM_STATE_CHANGED, invalid slotId:" + slotId);
                    return;
                }

                String simStatus = intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);
                log(slotId, "SIM_STATE_CHANGED, simStatus:" + simStatus);

                if (IccCardConstants.INTENT_VALUE_ICC_ABSENT.equals(simStatus)) {
                    mEntitlementStarted[slotId] = false;
                    mSimLoaded[slotId] = false;
                    saveIccId("", slotId);
                    sendReset(slotId);
                } else if (IccCardConstants.INTENT_VALUE_ICC_LOADED.equals(simStatus)) {
                    mSimLoaded[slotId] = true;
                    if (mHandling[slotId] != null) {
                        if (mHandling[slotId].isinActiveState()) {
                            log(slotId, "In New SIM, Currently Entitlement is ongoing, return");
                            return;
                        }
                    }
                    if (isSimChanged(slotId)) {
                        mEntitlementStarted[slotId] = false;
                        sendReset(slotId);
                    }

                    waitDefaultNetwork();
                    ArrayList<String> list = new ArrayList<String>();
                    list.add(VOLTE_SERVICE);
                    list.add(VOWIFI_SERVICE);
                    startEntitlement(list, slotId);
                } else if (IccCardConstants.INTENT_VALUE_ICC_READY.equals(simStatus)) {
                    if (isSimReady(slotId) && isSimChanged(slotId)) {
                        mEntitlementStarted[slotId] = false;
                        sendReset(slotId);
                    }
                }
            } else if (TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_DONE.equals(action)) {
                int slotId = getMainPhoneId();
                mEntitlementStarted[slotId] = false;
                ArrayList<String> list = new ArrayList<String>();
                list.add(VOLTE_SERVICE);
                list.add(VOWIFI_SERVICE);
                startEntitlement(list, slotId);
            } else if (ACTION_ENTITLEMENT_CHECK.equals(action)) {
                int slotId = intent.getIntExtra(INTENT_KEY_SLOT_ID, -1);

                if (slotId < 0 || slotId >= mSimCount) {
                    loge("ACTION_ENTITLEMENT_CHECK, invalid slotId:" + slotId);
                    return;
                }
                mEntitlementStarted[slotId] = false;
                ArrayList<String> list = new ArrayList<String>();
                list.add(VOLTE_SERVICE);
                list.add(VOWIFI_SERVICE);
                startEntitlement(list, slotId);
            } else if (ACTION_PROVISIONING_PENDING.equals(action)) {
                int slotId = intent.getIntExtra(INTENT_KEY_SLOT_ID, -1);

                if (slotId < 0 || slotId >= mSimCount) {
                    loge("ACTION_PROVISIONING_PENDING, invalid slotId:" + slotId);
                    return;
                }
                mEntitlementStarted[slotId] = false;
                String service = intent.getStringExtra("service");
                mProvisionedService[slotId] = service;
                //mServiceList.clear();
                log(slotId, "Received Retry alarm for service = " + service);
                ArrayList<String> list = new ArrayList<String>();
                synchronized (mProvisionings[slotId]) {
                    for (String retryService : mProvisionings[slotId].keySet()) {
                        Provisioning provisioning = mProvisionings[slotId].get(retryService);
                        if (provisioning != null) {
                            if (!provisioning.retry()) {
                                continue;
                            }
                        }
                        list.add(retryService);
                    }
                }
                if (list != null) {
                    startEntitlement(list, slotId);
                }
            }
        }
    };

    class ImsRequest {
        public static final int IMS_REQUEST_DEPROVISION_SERVICES = 1;
        private static final int MAX_RETRY_COUNT = 40;
        private int mPhoneId;
        private int mRequestCode;
        private int mRetryCount = 0;

        private ImsRequest(int phoneId, int requestCode) {
            mPhoneId = phoneId;
            mRequestCode = requestCode;
        }

        public int getRequestCode() {
            return mRequestCode;
        }

        public boolean isNeedRetry() {
            mRetryCount ++;
            return mRetryCount <= MAX_RETRY_COUNT;
        }
    }

    @Override
    public void onCreate() {
        super.onCreate();

        if (!isEntitlementEnabled()) {
            log("onCreate(), isEntitlementEnabled is false, return directly");
            return;
        }

        mContext = getApplicationContext();
        mDeviceContext = mContext.createDeviceProtectedStorageContext();
        if (mDeviceContext == null) {
            mDeviceContext = mContext;
            loge("replace mContext to mDeviceContext");
        }
        mConnectivityManager =
                (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
        TelephonyManager telephonyManager =
                (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);
        if (telephonyManager == null) {
            loge("get telephony service fail !!! telephony is null");
        }
        mSimCount = telephonyManager.getSimCount();
        mHandling = new EntitlementHandling[mSimCount];
        mProvisionings = new HashMap[mSimCount];
        mEntitlementCheck = new EntitlementCheck[mSimCount];
        mSimLoaded = new boolean[mSimCount];
        mProvisionedService = new String[mSimCount];
        mEntitlementStarted = new boolean[mSimCount];

        for (int i = 0; i < mSimCount; i++) {
            mHandling[i] = null;
            mProvisionings[i] = new HashMap<>();
            mEntitlementCheck[i] = new EntitlementCheck(i);
            mSimLoaded[i] = false;
            mProvisionedService[i] = null;
            mEntitlementStarted[i] = false;
        }

        final IntentFilter filter = new IntentFilter();
        filter.addAction(TelephonyIntents.ACTION_SIM_STATE_CHANGED);
        if (!isSupportMims()) {
            filter.addAction(TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_DONE);
        }
        filter.addAction(ACTION_ENTITLEMENT_CHECK);
        filter.addAction(ACTION_PROVISIONING_PENDING);
        registerReceiver(mBroadcastReceiver, filter);
        registerDefaultNetwork();

        mRequestThread = new HandlerThread("EntitlementHandler");
        mRequestThread.start();
        Looper looper = mRequestThread.getLooper();
        mRequestHandler = new EntitlementRequestHandler(looper);
    }

    private EntitlementHandling getHandling(int slotId) {
        /* synchronized (mServices) {
            EntitlementHandling handling = mServices.get(service); */
         if (mHandling[slotId] == null) {
             log(slotId, "Init Handling");
             mHandling[slotId] = new EntitlementHandling(mContext,
                     new EntitlementHandling.Listener() {
                 @Override
                 public void onStateChange(EntitlementHandling handling,
                     int state, String service, int slotId) {
                     //  Check state and set alarm
                     if (mHandling[slotId] == null) {
                         return;
                     }
                     synchronized (mLock) {
                         if (mHandling[slotId] == null) {
                             return;
                         }
                         updateServiceStatus(service, state, slotId);

                         if (state == EntitlementHandling.STATE_ENTITLED) {
                             mEntitlementCheck[slotId].setAlarm(service);
                             mRefCount--;
                             synchronized (mProvisionings[slotId]) {
                                 Provisioning provisioning = mProvisionings[slotId].get(service);
                                 log(slotId, "In sucess, Stored provisioning = " + provisioning);
                                 if (provisioning != null) {
                                     if (mProvisionings[slotId].size() == 1) {
                                         provisioning.cancelAlarm();
                                     }
                                     mProvisionings[slotId].remove(service);
                                     updateRetryStatus(service, 0, slotId);
                                 }
                             }
                             provisionService(service, slotId);
                         } else {
                             mEntitlementCheck[slotId].cancelAlarm(service);
                             if (state == EntitlementHandling.STATE_ENTITLED_FAIL) {
                                 //ImsManager.getInstance(mContext, slotId).setWfcSetting(false);
                                 synchronized (mProvisionings[slotId]) {
                                     Provisioning provisioning =
                                             mProvisionings[slotId].get(service);
                                     log(slotId, "Stored provisioning = " + provisioning);
                                     if (provisioning != null) {
                                         if (provisioning.shouldRetry()) {
                                             provisioning.setAlarm();
                                         } else {
                                             // provisioning failed after 30/60min retry.
                                             if (mProvisionings[slotId].size() == 1) {
                                                 provisioning.cancelAlarm();
                                             }
                                             mProvisionings[slotId].remove(service);
                                         }
                                     } else {
                                         mProvisionings[slotId].put(service,
                                             new Provisioning(
                                             service, RETRY_TIMER, RETRY_COUNT, slotId));
                                     }
                                 }
                                 mRefCount--;
                             } else if (state == EntitlementHandling.STATE_NOT_ENTITLED) {
                                 //ImsManager.getInstance(mContext, slotId).setWfcSetting(false);
                                 deProvisionService(service, slotId);
                                 mRefCount--;
                             }
                         }
                         if (mRefCount == 0) {
                             sendResponse(slotId);
                         }
                     }
                 }

                 @Override
                 public void onWebsheetPost(String url, String serverData, int slotId) {
                 }

                 @Override
                 public void onInfo(Bundle info, int slotId) {
                     Log.d(TAG, "Info discovered: " + info);
                 }
             },
             slotId);
             //mServices.put(service, handling);
         }
         return mHandling[slotId];
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        log("onStartCommand: " + intent + " " + startId);
        if (intent == null) {
            log("Service Re-created by system. Start Entitlement.");
            for (int i = 0; i < mSimCount; i++) {
                int simState = SubscriptionManager.getSimStateForSlotIndex(i);
                if (simState == TelephonyManager.SIM_STATE_LOADED) {
                    mSimLoaded[i] = true;
                }
                mDefaultNetworkState = 0;
                ArrayList<String> list = new ArrayList<String>();
                if (needEntitlementCheck(list, i)) {
                    startEntitlement(list, i);
                } else {
                    mEntitlementStarted[i] = true;
                }
            }
        }
        return START_STICKY;
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        log("onDestroy()");
        if (!isEntitlementEnabled()) {
            log("onDestroy(), isEntitlementEnabled is false, return directly");
            return;
        }
        unregisterReceiver(mBroadcastReceiver);
        mConnectivityManager.unregisterNetworkCallback(mDefaultNetworkCallback);
    }

    private void startEntitlementAll() {
        log("startEntitlementAll");
        for (int i = 0; i < mSimCount; i++) {
            ArrayList<String> list = new ArrayList<String>();
            list.add(VOLTE_SERVICE);
            list.add(VOWIFI_SERVICE);
            startEntitlement(list, i);
        }
    }

    private void startEntitlement(ArrayList<String> serviceList, int slotId) {
        log(slotId, "startEntitlement");
        log("Just log: data id" + SubscriptionManager.getDefaultDataSubscriptionId() +
                "main id: " + getMainPhoneId());
        if (serviceList != null) {
            if (checkEntitlementCriteria(slotId)) {
                mEntitlementStarted[slotId] = true;
                addRequest(serviceList, slotId);
            }
        }
    }

    private boolean checkEntitlementCriteria(int slotId) {
        if (!mSimLoaded[slotId]) {
            return false;
        }
        if (!isOp15Card(slotId)) {
            return false;
        }
        if (!isSupportMims() && getMainPhoneId() != slotId) {
            return false;
        }
        if (mEntitlementStarted[slotId]) {
            log(slotId, "checkEntitlementCriteria: my entitlement started");
            return false;
        }
        if (mDefaultNetworkState == -1 && !isNetworkAvailable() &&
                (isDataEnabled() || isWifiEnabled())) {
            log(slotId, "checkEntitlementCriteria: need to wait default network");
            return false;
        }
        return true;
    }

    private void waitDefaultNetwork() {
        if (mDefaultNetworkState == -1 && !isNetworkAvailable() &&
                (isDataEnabled() || isWifiEnabled())) {
            new Handler(Looper.getMainLooper()).postDelayed(new Runnable() {
                @Override
                public void run() {
                    if (mDefaultNetworkState == -1) {
                        mDefaultNetworkState = 0;
                        startEntitlementAll();
                    }
                    log("Wait default network done");
                }
            }, WAIT_DEFAULT_NETWORK_MILLIS);
        }
    }

    private EntitlementNetworkManager getNetworkManager(int phoneId) {
        EntitlementNetworkManager manager = mNetworkManagers.get(phoneId);
        if (manager == null) {
            manager = new EntitlementNetworkManager(mContext, phoneId);
            mNetworkManagers.put(phoneId, manager);
        }
        return manager;
    }

    private void requestNetwork(int phoneId) {
        mNetwork = getNetworkManager(phoneId).acquireNetwork();
        if (mNetwork != null && mConnectivityManager != null) {
            boolean result = mConnectivityManager.bindProcessToNetwork(mNetwork);
            log(phoneId, "Try to bind network " + mNetwork + ", result = " + result);
        }
    }

    private void releaseNetwork(int phoneId) {
        log(phoneId, "releaseNetwork");
        if (mNetwork != null) {
            getNetworkManager(phoneId).releaseNetwork();
            mNetwork = null;
        }
    }

    private void registerDefaultNetwork() {
        mDefaultNetworkCallback = new ConnectivityManager.NetworkCallback() {
            @Override
            public void onAvailable(Network network) {
                log("NetworkCallback.onAvailable()");
                mDefaultNetworkState = 1;
                startEntitlementAll();
            }

            @Override
            public void onLost(Network network) {
                log("NetworkCallback.onLost()");
                mDefaultNetworkState = 0;
            }
        };
        mConnectivityManager.registerDefaultNetworkCallback(mDefaultNetworkCallback);
    }

    public boolean needRequestNetwork() {
        if (!isNetworkAvailable()) {
            return true;
        }
        log("Network already available, no need Request");
        return false;
    }

    /***Telefonica Request: Silently enable data to complete entitlement***/
    private boolean isNetworkAvailable() {
        ConnectivityManager connectivity
              = (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo activeNetworkInfo = connectivity.getActiveNetworkInfo();
        if (activeNetworkInfo != null) {
            log("Active network state: " + activeNetworkInfo.getState());
        }
        return activeNetworkInfo != null && activeNetworkInfo.isConnected();
    }

    private boolean isNetworkConnecting() {
        ConnectivityManager connectivity
              = (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo activeNetworkInfo = connectivity.getActiveNetworkInfo();
        boolean isConnecting = activeNetworkInfo != null &&
                (activeNetworkInfo.getState() == NetworkInfo.State.CONNECTING);
        if (activeNetworkInfo != null) {
            log("Active network state: " + activeNetworkInfo.getState());
        }
        return isConnecting;
    }

    private boolean getImsServiceDone(int slotId, Object object) {
        ImsManager imsManager = ImsManager.getInstance(mContext, slotId);
        if (imsManager.isServiceReady()) {
            if (object != null && object instanceof ImsRequest) {
                ImsRequest request = (ImsRequest) object;
                switch (request.getRequestCode()) {
                    case ImsRequest.IMS_REQUEST_DEPROVISION_SERVICES:
                        if (!isMccMncReady(slotId) || isOp15Card(slotId)) {
                            deProvisionServices(slotId);
                        }
                        break;
                    default:
                        log(slotId, "retryGetImsService: error code:" + request.mRequestCode);
                }
            }
            return true;
        }
        return false;
    }

    private void retryGetImsService(int slotId, Object object) {
        if (object != null && object instanceof ImsRequest) {
            ImsRequest request = (ImsRequest) object;
            if (!request.isNeedRetry()) {
                return;
            }
        }
        ImsManager imsManager = ImsManager.getInstance(mContext, slotId);
        if (imsManager.isServiceReady()) {
            return;
        }
        if (mRequestHandler != null) {
            Message msg = mRequestHandler.obtainMessage(
                    EVENT_RETRY_GET_IMS_SERVICE_1 + slotId, slotId, 0, object);
            mRequestHandler.sendMessageDelayed(msg, TIMEOUT_RETRY_GET_IMS_SERVICE);
        }
    }

    private void stopGetImsService(int slotId) {
        if (mRequestHandler != null) {
            mRequestHandler.removeMessages(EVENT_RETRY_GET_IMS_SERVICE_1 + slotId);
        }
    }

    private void deProvisionServices(int slotId) {
        log(slotId, "De-provision both VoLTE & VoWifi");
        //SystemProperties.set(VLT_SETTING_SYTEM_PROPERTY + slotId, "0");
        //SystemProperties.set(WFC_SETTING_SYTEM_PROPERTY + slotId, "0");
        try {
            ImsManager imsManager = ImsManager.getInstance(mContext, slotId);
            int result = imsManager.getConfigInterface().setProvisionedValue(
                    ImsConfig.ConfigConstants.VOICE_OVER_WIFI_SETTING_ENABLED, 0);
            //log("WFC deprovision result = " + result);
            imsManager.getConfigInterface().setProvisionedValue(
                    ImsConfig.ConfigConstants.VLT_SETTING_ENABLED, 0);
            //log("VoLTE deprovision result = " + result);
        } catch (ImsException e) {
            loge("Exception happened! " + e.getMessage());
        }
    }

    private void provisionService(String service, int slotId) {
        log(slotId, "Restore provisioning for service = " + service);
        String sysProperty = null;
        ImsManager imsManager = ImsManager.getInstance(mContext, slotId);
        int setting = 0;
        if (service.equalsIgnoreCase(VOLTE_SERVICE)) {
            setting = ImsConfig.ConfigConstants.VLT_SETTING_ENABLED;
            //SystemProperties.set(VLT_SETTING_SYTEM_PROPERTY + slotId, "1");
            //log(slotId, "sys prop set = " + SystemProperties.get(VLT_SETTING_SYTEM_PROPERTY));
        } else if (service.equalsIgnoreCase(VOWIFI_SERVICE)) {
            setting = ImsConfig.ConfigConstants.VOICE_OVER_WIFI_SETTING_ENABLED;
            //SystemProperties.set(WFC_SETTING_SYTEM_PROPERTY + slotId, "1");
        } else {
            loge("Invalid service!!!");
            return;
        }
        stopGetImsService(slotId);
        try {
            int result = imsManager.getConfigInterface().setProvisionedValue(
                    setting, 1);
            imsManager.updateImsServiceConfig(true);
            log(slotId, "For service, " + service + "setting:" + setting +
                    "Provision result = " + result);
        } catch (ImsException e) {
            loge("Exception happened! " + e.getMessage());
        }
    }

    private void deProvisionService(String service, int slotId) {
        log(slotId, "Remove provisioning for service = " + service);
        ImsManager imsManager = ImsManager.getInstance(mContext, slotId);
        int setting = 0;
        if (service.equalsIgnoreCase(VOLTE_SERVICE)) {
            setting = ImsConfig.ConfigConstants.VLT_SETTING_ENABLED;
            //SystemProperties.set(VLT_SETTING_SYTEM_PROPERTY + slotId, "0");
            //log(slotId, "sys prop reset = " + SystemProperties.get(VLT_SETTING_SYTEM_PROPERTY));
        } else if (service.equalsIgnoreCase(VOWIFI_SERVICE)) {
            setting = ImsConfig.ConfigConstants.VOICE_OVER_WIFI_SETTING_ENABLED;
            //SystemProperties.set(WFC_SETTING_SYTEM_PROPERTY + slotId, "0");
        } else {
            loge("Invalid service!!!");
            return;
        }
        try {
            int result = imsManager.getConfigInterface().setProvisionedValue(
                    setting, 0);
            log(slotId, "For service, " + service + "De-Provisioning result = " + result);
        } catch (ImsException e) {
            loge("Exception happened! " + e.getMessage());
        }
    }

    private boolean needEntitlementCheck(ArrayList<String> list, int slotId) {
        SharedPreferences prefs = mDeviceContext.getSharedPreferences(
                SERVICE_STATUS_PREF, MODE_PRIVATE);
        int serviceStatus = prefs.getInt("volte" + slotId, 0);
        log(slotId, "from pref, volte status = " + serviceStatus);
        if (serviceStatus == EntitlementHandling.STATE_INITIAL ||
                serviceStatus == EntitlementHandling.STATE_ACTIVATING) {
            list.add(VOLTE_SERVICE);
        } else if (serviceStatus == EntitlementHandling.STATE_ENTITLED_FAIL) {
            int retriesleft = prefs.getInt("volteretry" + slotId, 0);
            if (retriesleft > 0) {
                list.add(VOLTE_SERVICE);
            }
        }
        serviceStatus = prefs.getInt("vowifi" + slotId, 0);
        log(slotId, "from pref, vowifi status = " + serviceStatus);
        if (serviceStatus == EntitlementHandling.STATE_INITIAL ||
                serviceStatus == EntitlementHandling.STATE_ACTIVATING) {
            list.add(VOWIFI_SERVICE);
        } else if (serviceStatus == EntitlementHandling.STATE_ENTITLED_FAIL) {
            int retriesleft = prefs.getInt("vowifiretry" + slotId, 0);
            if (retriesleft > 0) {
                list.add(VOWIFI_SERVICE);
            }
        }
        if (list != null) {
            return true;
        } else {
            return false;
        }
    }

    private void updateServiceStatus(String service, int status, int slotId) {
        log(slotId, "Update shared pref for service = " + service + " to status = " + status);
        SharedPreferences.Editor editor = mDeviceContext.getSharedPreferences(
                SERVICE_STATUS_PREF, MODE_PRIVATE).edit();
        if (service.equalsIgnoreCase(VOLTE_SERVICE)) {
            editor.putInt("volte" + slotId, status);
        } else if (service.equalsIgnoreCase(VOWIFI_SERVICE)) {
            editor.putInt("vowifi" + slotId, status);
        }
        editor.commit();
    }

    private void updateRetryStatus(String service, int retriedTimes, int slotId) {
        log(slotId, "Update shared pref for service = " + service +
                " ,Retry times = " + retriedTimes);
        SharedPreferences.Editor editor = mDeviceContext.getSharedPreferences(
                SERVICE_STATUS_PREF, MODE_PRIVATE).edit();
        if (service.equalsIgnoreCase(VOLTE_SERVICE)) {
            editor.putInt("volteretry" + slotId, retriedTimes);
        } else if (service.equalsIgnoreCase(VOWIFI_SERVICE)) {
            editor.putInt("vowifiretry" + slotId, retriedTimes);
        }
        editor.commit();
    }

    private boolean isSimReady(int slotId) {
        SubscriptionManager subMgr = SubscriptionManager.from(mContext);
        SubscriptionInfo subInfo = null;
        if (subMgr != null) {
            int subId = getSubIdUsingPhoneId(slotId);
            subInfo = subMgr.getActiveSubscriptionInfo(subId);
        }
        if ((subInfo != null) && (subInfo.getIccId() != null)) {
            return true;
        }
        return false;
    }

    private boolean isSimChanged(int slotId) {
        String iccId = null;
        /*TelephonyManager telephonyManager = (TelephonyManager) mContext
                .getSystemService(Context.TELEPHONY_SERVICE);

        int subId = getSubIdUsingPhoneId(slotId);
        if (telephonyManager != null) {
            iccId = telephonyManager.getSimSerialNumber(subId);
        }*/
        SubscriptionManager subMgr = SubscriptionManager.from(mContext);
        SubscriptionInfo subInfo = null;
        if (subMgr != null) {
            int subId = getSubIdUsingPhoneId(slotId);
            subInfo = subMgr.getActiveSubscriptionInfo(subId);
        }
        if ((subInfo != null) && (subInfo.getIccId() != null)) {
            iccId = subInfo.getIccId();
        }
        if (TextUtils.isEmpty(iccId)) {
            loge(slotId, "isSimChanged: iccId is null or empty.");
            return false;
        }
        String oldIccId = getIccId(slotId);
        String hashIccId = getHashCode(iccId);
        if (oldIccId.equals(hashIccId)) {
            return false;
        }
        saveIccId(hashIccId, slotId);
        return true;
    }

    private void saveIccId(String iccId, int slotId) {
        logTel(slotId, "saveIccId: " + iccId);
        SharedPreferences.Editor editor = mDeviceContext.getSharedPreferences(
                SERVICE_STATUS_PREF, MODE_PRIVATE).edit();
        editor.putString(ICCID_KEY + slotId, iccId);
        if (!editor.commit()) {
            loge(slotId, "Failed to commit iccId preference");
        }
    }

    private String getIccId(int slotId) {
        SharedPreferences prefs = mDeviceContext.getSharedPreferences(
                SERVICE_STATUS_PREF, MODE_PRIVATE);
        String iccId = prefs.getString(ICCID_KEY + slotId, "");
        logTel(slotId, "getIccId: " + iccId);
        return iccId;
    }

    private static String getHashCode(String input) {
        if (input == null) {
            return input;
        }
        try {
            MessageDigest messageDigest = MessageDigest.getInstance("SHA-256");
            messageDigest.update(input.getBytes());
            byte[] result = messageDigest.digest();
            return bytesToHex(result);
        } catch (NoSuchAlgorithmException e) {
            return input;
        }
    }

    private static String bytesToHex(byte[] bytes) {
        StringBuffer sb = new StringBuffer(bytes.length * 2);
        for (int i = 0; i < bytes.length; i++) {
            int intValue = bytes[i] & 0xff;
            String hex = Integer.toString(intValue, 16);
            if (hex.length() < 2) {
                sb.append("0");
            }
            sb.append(hex);
        }
        return sb.toString();
    }

    private boolean isDataEnabled() {
        boolean enabled = true;
        TelephonyManager telephonyManager = (TelephonyManager) mContext
                .getSystemService(Context.TELEPHONY_SERVICE);

        int subId = getSubIdUsingPhoneId(getMainPhoneId());
        if (SubscriptionManager.getDefaultDataSubscriptionId() !=
                SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
            subId = SubscriptionManager.getDefaultDataSubscriptionId();
        }
        if (telephonyManager != null) {
            enabled = telephonyManager.getDataEnabled(subId);
            log("isDataEnabled: " + enabled);
        }
        return enabled;
    }

    private boolean isWifiEnabled() {
        boolean enabled = false;
        WifiManager wifiMngr = (WifiManager)mContext.getSystemService(Context.WIFI_SERVICE);
        if (wifiMngr != null) {
            enabled = wifiMngr.isWifiEnabled();
            log("isWifiEnabled: " + enabled + "getWifiState:" + wifiMngr.getWifiState());
        }
        return enabled;
    }

    private static int getMainPhoneId() {
        int phoneId = 0;
        phoneId = SystemProperties.getInt(PROPERTY_CAPABILITY_SWITCH, 1) - 1;
        log("[getMainPhoneId] : " + phoneId);
        return phoneId;
    }

    private static int getSubIdUsingPhoneId(int phoneId) {
        int [] values = SubscriptionManager.getSubId(phoneId);
        if(values == null || values.length <= 0) {
            return SubscriptionManager.getDefaultSubscriptionId();
        }
        else {
            logTel(phoneId, "getSubIdUsingPhoneId:" + values[0]);
            return values[0];
        }
    }

    private static boolean isSupportMims() {
        return (SystemProperties.getInt(MULTI_IMS_SUPPORT, 1) > 1);
    }

    private static boolean isOp15Card(int phoneId) {
        String mccMnc = getSimOperatorNumericForPhone(phoneId);
        logTel(phoneId, "isOp15Card(): mccMnc is " + mccMnc);
        return OP15_MCCMNC_LIST.contains(mccMnc);
    }

    private static boolean isMccMncReady(int phoneId) {
        String mccMnc = getSimOperatorNumericForPhone(phoneId);
        if (TextUtils.isEmpty(mccMnc)) {
            Log.d(TAG, "MccMnc is empty.");
            return false;
        }
        return true;
    }

    private static String getSimOperatorNumericForPhone(int phoneId) {
        String mccMncPropertyName = "";
        String mccMnc = "";
        int phoneType = 0;
        int subId = getSubIdUsingPhoneId(phoneId);
        if (!SubscriptionManager.isValidSubscriptionId(subId)) {
            log(phoneId, "Is Invalid Subscription id.");
            return mccMnc;
        }
        phoneType = TelephonyManager.getDefault().getCurrentPhoneType(subId);
        if (phoneType == PhoneConstants.PHONE_TYPE_CDMA) {
            if (phoneId == 0) {
                mccMncPropertyName = "vendor.cdma.ril.uicc.mccmnc";
            } else {
                mccMncPropertyName = "vendor.cdma.ril.uicc.mccmnc." + phoneId;
            }
        } else {
            if (phoneId == 0) {
                mccMncPropertyName = "vendor.gsm.ril.uicc.mccmnc";
            } else {
                mccMncPropertyName = "vendor.gsm.ril.uicc.mccmnc." + phoneId;
            }
        }
        mccMnc = SystemProperties.get(mccMncPropertyName, "");
        logTel(phoneId, "getMccMnc, mccMnc value:" + mccMnc);
        return mccMnc;
    }

    private static boolean isEntitlementEnabled() {
        boolean isEntitlementEnabled = (1 == SystemProperties.getInt
                ("persist.vendor.entitlement_enabled", 1) ? true : false);
        log("In EntitlementService, isEntitlementEnabled:" + isEntitlementEnabled);
        return isEntitlementEnabled;
    }

    private static void log(String s) {
        if (DEBUG) Log.d(TAG, s);
    }

    private static void log(int slotId, String s) {
        if (DEBUG) Log.d(TAG,  "[" + slotId + "]" + s);
    }

    private static void loge(String s) {
        if (DEBUG) Log.e(TAG, s);
    }

    private static void loge(int slotId, String s) {
        if (DEBUG) Log.e(TAG,  "[" + slotId + "]" + s);
    }

    private static void logTel(int slotId, String s) {
        if (ENG || TELDBG) {
            Log.d(TAG,  "[" + slotId + "]" + s.replaceAll("[0-9a-fA-F]{7}", "***"));
        }
    }
}
