package com.mediatek.setting;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.provider.Settings;
import android.telephony.ServiceState;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;
import com.android.mms.MmsApp;
import com.android.mms.util.MmsLog;
import mediatek.telephony.MtkTelephony;

import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * Used for setting activities, receive SIM state related intents, activities will re-fresh
 * when receive any one of these action:
 * TelephonyIntents.ACTION_SIM_STATE_CHANGED
 * TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED
 * Intent.ACTION_AIRPLANE_MODE_CHANGED
 * TelephonyIntents.ACTION_SERVICE_STATE_CHANGED
 * Telephony.Sms.Intents.SMS_STATE_CHANGED_ACTION
 */
public class SimStateMonitor {

    private Set<SimStateListener> mListeners = new HashSet<SimStateMonitor.SimStateListener>();
    private static SimStateMonitor sSimStateMonitor = null;
    private List<SubscriptionInfo> mSubInfoList;
    private int mSubCount = 0;
    private static boolean mIsAirPlaneMode = false;
    private static final String TAG = "SimStateMonitor";
    private static int[] mServiceState = {-1,-1};

    private BroadcastReceiver mSimStateReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (SimStateMonitor.getInstance().refreshData(intent)) {
                for (SimStateListener listener : SimStateMonitor.getInstance().mListeners) {
                    listener.onSimStateChanged();
                }
            }
        }
    };

    public static SimStateMonitor getInstance() {
        if (sSimStateMonitor == null) {
            synchronized (SimStateMonitor.class) {
                if (sSimStateMonitor == null) {
                    sSimStateMonitor = new SimStateMonitor();
                }
            }
        }
        return sSimStateMonitor;
    }

    public interface SimStateListener {
        public void onSimStateChanged();
    }

    private SimStateMonitor() {
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(TelephonyIntents.ACTION_SIM_STATE_CHANGED);
        intentFilter.addAction(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        intentFilter.addAction(MtkTelephony.SMS_STATE_CHANGED_ACTION);
        intentFilter.addAction(TelephonyIntents.ACTION_SERVICE_STATE_CHANGED);
        Context context = MmsApp.getApplication().getApplicationContext();
        context.registerReceiver(mSimStateReceiver, intentFilter);
        refreshData(null);
    }

    public void addListener(SimStateListener listener) {
        mListeners.add(listener);
    }

    public void removeListener(SimStateListener listener) {
        mListeners.remove(listener);
    }

    private boolean refreshData(Intent refreshIntent) {
        String action = refreshIntent != null ? refreshIntent.getAction() : null;
        MmsLog.d(TAG, "onReceive action : " + action);
        mSubInfoList = SubscriptionManager.from(MmsApp.getApplication())
                .getActiveSubscriptionInfoList();
        if (mSubInfoList == null) {
            mSubInfoList = new CopyOnWriteArrayList();
        }
        boolean hasChanged = false;
        if (mSubCount != mSubInfoList.size()) {
            hasChanged = true;
        }
        mSubCount = mSubInfoList.size();
        boolean currentAirPlaneMode = Settings.System.getInt(
            MmsApp.getApplication().getContentResolver(), Settings.System.AIRPLANE_MODE_ON, 0) == 1;
        if (mIsAirPlaneMode != currentAirPlaneMode) {
            hasChanged = true;
        }
        mIsAirPlaneMode = currentAirPlaneMode;

        if (action != null && action.equals(TelephonyIntents.ACTION_SERVICE_STATE_CHANGED)) {
            ServiceState serviceState = ServiceState.newFromBundle(refreshIntent.getExtras());
            if (serviceState != null) {
                int newState = serviceState.getState();
                int slotId = refreshIntent.getIntExtra(PhoneConstants.SLOT_KEY, 0);
                if (slotId == 0 || slotId == 1) {
                    int oldState = mServiceState[slotId];
                    if (newState != oldState) {
                        MmsLog.d(TAG, "Service state changed! " + newState +
                            " Full: " + serviceState + " Current state=" + oldState);
                        mServiceState[slotId] = newState;
                        hasChanged = true;
                    }
                }
            }
        }

        if (action != null && action.equals(MtkTelephony.SMS_STATE_CHANGED_ACTION)) {
            hasChanged = true;
        }
        return hasChanged;
    }

    public List<SubscriptionInfo> getSubInfoList() {
        return mSubInfoList;
    }

    public int getSubCount() {
        return mSubCount;
    }

    public boolean getAirplaneMode() {
        return mIsAirPlaneMode;
    }

}
