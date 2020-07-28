package com.mediatek.systemui.op01;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.IBinder;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Process;
import android.os.SystemProperties;
import android.telephony.PhoneStateListener;
import android.telephony.PreciseDataConnectionState;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.SubscriptionManager.OnSubscriptionsChangedListener;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;

import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.HashMap;
import java.util.List;

/**
 * M: Op01 PhoneState Listener Controller.
 */
public class PhoneStateListenerController {
    private static final String TAG = "OP01-PhoneStateListenerController";

    private TelephonyManager mTelephonyManager;
    private SubscriptionManager mSubscriptionManager;
    private OnSubscriptionsChangedListener mSubscriptionsChangedListener;
    private boolean registered;
    private Context mContext;

    private static final boolean DEBUG = !isUserLoad();
    private static HashMap<Integer, ArrayList> sUpdateActivityIconMap =
            new HashMap<Integer, ArrayList>();
    private Handler mUiHandler = new Handler(Looper.getMainLooper());

    private static PhoneStateListenerController mPhoneStateController;

    private static HashMap<Integer, ArrayList> mPreciseDataStateMap =
            new HashMap<Integer, ArrayList>();
    private static HashMap<Integer, Boolean> mAllDisconnMap =
            new HashMap<Integer, Boolean>();
    private HandlerThread mHandlerThread;
    // Handler that all broadcasts are received on.
    private final Handler mReceiverHandler;
    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(TelephonyIntents.ACTION_ANY_DATA_CONNECTION_STATE_CHANGED)) {
                handleIntent(intent);
            }
        }
    };

    public static PhoneStateListenerController getInstance(Context context) {
        if (mPhoneStateController != null) {
            return mPhoneStateController;
        } else {
            mPhoneStateController = new PhoneStateListenerController(context);
            return mPhoneStateController;
        }
    }

    public PhoneStateListenerController(Context context) {
        Log.v(TAG, "OP01 PhoneStateListenerService onCreate");
        mHandlerThread = new HandlerThread(TAG, Process.THREAD_PRIORITY_BACKGROUND);
        mHandlerThread.start();
        mReceiverHandler = new Handler(mHandlerThread.getLooper());

        mContext = context;
        mTelephonyManager = TelephonyManager.from(mContext);
        mSubscriptionManager = SubscriptionManager.from(mContext);

        IntentFilter intentFilter =  new IntentFilter(
                TelephonyIntents.ACTION_ANY_DATA_CONNECTION_STATE_CHANGED);
        mContext.registerReceiver(mReceiver, intentFilter, null, mReceiverHandler);

        mSubscriptionsChangedListener = new OnSubscriptionsChangedListener() {
            @Override
            public void onSubscriptionsChanged() {
                updatePreciseDataState();
            }
        };
        mSubscriptionManager.addOnSubscriptionsChangedListener(mSubscriptionsChangedListener);
    }

    private void handleIntent(Intent intent) {
        String state = intent.getStringExtra(PhoneConstants.STATE_KEY);
        PhoneConstants.DataState dataState;
        if (state != null) {
            dataState = Enum.valueOf(PhoneConstants.DataState.class, state);
        } else {
            dataState = PhoneConstants.DataState.DISCONNECTED;
        }
        String apnType = intent.getStringExtra(PhoneConstants.DATA_APN_TYPE_KEY);
        int subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY, -1);
        if (DEBUG) {
            Log.d(TAG, "handle precise data Intent, subId: " + subId + ",apnType: " + apnType +
                    ",dataState: " + dataState);
        }

        if (!SubscriptionManager.isValidSubscriptionId(subId)) {
            return;
        }

        synchronized (mPreciseDataStateMap) {
            ArrayList<String> apnTypeList;
            if (mPreciseDataStateMap.containsKey(subId)) {
                apnTypeList = mPreciseDataStateMap.get(subId);
            } else {
                apnTypeList = new ArrayList<>();
            }
            boolean allDisconn;
            if (mAllDisconnMap.containsKey(subId)) {
                allDisconn = mAllDisconnMap.get(subId);
            } else {
                allDisconn = true;
            }
            if (dataState == PhoneConstants.DataState.CONNECTED) {
                if (!apnTypeList.contains(apnType)) {
                    apnTypeList.add(apnType);
                    allDisconn = false;
                    if (DEBUG) {
                        Log.d(TAG, "subId: " + subId + ", put apnType: " + apnType +
                                " to cache, mAllDisconn: " + allDisconn);
                    }
                }
            } else {
                if (apnTypeList.contains(apnType)) {
                    apnTypeList.remove(apnType);
                    if (apnTypeList.size() == 0) {
                        allDisconn = true;
                    }
                    if (DEBUG) {
                        Log.d(TAG, "subId: " + subId + ", remove apnType: " + apnType
                                + " from cache, mAllDisconn: " + allDisconn);
                    }
                }
            }
            mPreciseDataStateMap.put(subId, apnTypeList);
            mAllDisconnMap.put(subId, allDisconn);
        }

        // update data activity icon
        mUiHandler.post(new Runnable() {
            @Override
            public void run() {
                ArrayList<ImageView> viewsList = sUpdateActivityIconMap.get(subId);
                if (viewsList != null) {
                    for (ImageView view : viewsList) {
                        // sync witch status bar ext, just filter ims case in data connected
                        if (isDefaultApnConnected(subId)) {
                            if (isShowDataActyIconByIntent(subId)) {
                                view.setVisibility(View.VISIBLE);
                            } else {
                                view.setVisibility(View.GONE);
                            }
                        } else {
                            view.setVisibility(View.GONE);
                        }
                    }
                }
            }
        });
    }

    public static final boolean isUserLoad() {
        return SystemProperties.get("ro.build.type").equals("user") ||
                    SystemProperties.get("ro.build.type").equals("userdebug");
    }

    public void addDataActivityView(int subId, ImageView view) {
        ArrayList<ImageView> viewsList;
        if (sUpdateActivityIconMap.containsKey(subId)) {
            viewsList = sUpdateActivityIconMap.get(subId);
        } else {
            viewsList = new ArrayList<>();
        }

        if (!viewsList.contains(view)) {
            viewsList.add(view);
            if (DEBUG) {
                Log.d(TAG, "subId: " + subId + ",addDataActivityView to list, view: " + view);
            }
        }
        sUpdateActivityIconMap.put(subId, viewsList);
        viewsList = null;
    }

    public boolean isShowDataActyIconByIntent(int subId) {
        synchronized (mPreciseDataStateMap) {
            boolean isShow = true;
            boolean allDisconn;
            if (mAllDisconnMap.containsKey(subId)) {
                allDisconn = mAllDisconnMap.get(subId);
            } else {
                allDisconn = true;
            }
            ArrayList<String> apnTypeList = mPreciseDataStateMap.get(subId);
            if (apnTypeList != null && apnTypeList.size() == 2) {
                if (apnTypeList.contains(PhoneConstants.APN_TYPE_IMS)
                        && apnTypeList.contains(PhoneConstants.APN_TYPE_EMERGENCY)) {
                    isShow = false;
                    if (DEBUG) {
                        Log.d(TAG, "subId: " + subId
                                + " isShowDataActyIconByIntent(), contain ims and emergency,"
                                + "return false");
                    }
                }
            } else if (apnTypeList != null && apnTypeList.size() == 1) {
                if (apnTypeList.contains(PhoneConstants.APN_TYPE_IMS)
                        || apnTypeList.contains(PhoneConstants.APN_TYPE_EMERGENCY)) {
                    isShow = false;
                    if (DEBUG) {
                        Log.d(TAG, "subId: " + subId +
                                " isShowDataActyIconByIntent(), contain ims or emergency, " +
                                "return false");
                    }
                }
            } else if (allDisconn) {
                isShow = false;
                if (DEBUG) {
                    Log.d(TAG, "subId: " + subId +
                            " isShowDataActyIconByIntent(), allDisconn is ture, return false");
                }
            }
            return isShow;
        }
    }

    private boolean isDefaultApnConnected(int subId) {
        synchronized (mPreciseDataStateMap) {
            ArrayList<String> apnTypeList = mPreciseDataStateMap.get(subId);
            if (apnTypeList != null && apnTypeList.contains(PhoneConstants.APN_TYPE_DEFAULT)) {
                return true;
            } else {
                return false;
            }
        }
    }

    private void updatePreciseDataState() {
        if (DEBUG) {
            Log.d(TAG, "updatePreciseDataState");
        }
        List<SubscriptionInfo> subInfos = mSubscriptionManager.getActiveSubscriptionInfoList();
        if (subInfos == null) {
            return;
        }

        synchronized (mPreciseDataStateMap) {
            if (subInfos.size() == 0) {
                mPreciseDataStateMap.clear();
            } else {
                ArrayList<Integer> removeSubs = new ArrayList<>();
                for (int subId : mPreciseDataStateMap.keySet()) {
                    boolean found = false;
                    for (SubscriptionInfo info : subInfos) {
                        int id = info.getSubscriptionId();
                        if (subId == id) {
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        removeSubs.add(subId);
                    }
                }
                for (int subId : removeSubs) {
                    mPreciseDataStateMap.remove(subId);
                    Log.d(TAG, "remove invalid subId: " + subId + " from mPreciseDataStateMap");
                }
            }
        }
    }
}
