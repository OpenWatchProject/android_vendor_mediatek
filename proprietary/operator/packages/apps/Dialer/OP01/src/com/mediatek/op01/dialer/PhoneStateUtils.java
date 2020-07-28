package com.mediatek.op01.dialer;

import android.app.Activity;
import android.content.Context;
import android.telecom.PhoneAccount;
import android.telecom.PhoneAccountHandle;
import android.telecom.TelecomManager;
import android.telephony.TelephonyManager;
import android.telephony.PhoneStateListener;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.SubscriptionManager.OnSubscriptionsChangedListener;
import android.util.Log;

import java.lang.ref.WeakReference;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

public class PhoneStateUtils {
    private static final String TAG = "PhoneStateUtils";

    private TelephonyManager               mDefaultTelephonyManager;
    //Listen for subinfo change, like sim hot plugin-out
    private SubscriptionManager            mSubscriptionManager;
    private OnSubscriptionsChangedListener mSubscriptionsChangedListener;

    private final Set<OnChangedListener> mListeners = Collections.newSetFromMap(
            new ConcurrentHashMap<OnChangedListener, Boolean>(5, 0.9f, 1));

    private HashMap<Integer, OP01PhoneStateListener> mPhoneStateListeners =
            new HashMap<Integer, OP01PhoneStateListener>();
    private static HashMap<Integer, Integer> mPhoneStateMap =
            new HashMap<Integer, Integer>();

    private static TelecomManager mTelecommManager;
    private static PhoneStateUtils mPhoneStateUtils;

    private static Context mContext;

    public static PhoneStateUtils getInstance(Context cnx) {
        if (mPhoneStateUtils == null) {
            mPhoneStateUtils = new PhoneStateUtils(cnx);
        }
        return mPhoneStateUtils;
    }

    private PhoneStateUtils(Context cnx) {
        mContext = cnx;
        init(cnx);
        registerStateListener();
        mTelecommManager = (TelecomManager) mContext.getSystemService(Context.TELECOM_SERVICE);
    }

    //Interface for CallStatusServicePresenter
    public interface OnChangedListener {
        public void onCallStatusChange(int state);
    }

    public void addPhoneStateListener(OnChangedListener listener) {
        mListeners.add(listener);
    }

    public void removePhoneStateListener(OnChangedListener listener) {
      mListeners.remove(listener);
    }

    private void updateState(final int state) {
        for (OnChangedListener listener : mListeners) {
            listener.onCallStatusChange(state);
        }
    }

    private void init(Context cnx) {
        mDefaultTelephonyManager = (TelephonyManager) mContext
                .getSystemService(Context.TELEPHONY_SERVICE);
        mSubscriptionManager = SubscriptionManager.from(cnx);

        mSubscriptionsChangedListener = new OnSubscriptionsChangedListener() {
                                                @Override
                                                public void onSubscriptionsChanged() {
                                                    updateStateListener();
                                                }
                                            };
    }

    private void registerStateListener() {
        Log.d(TAG, "registerStateListener");
        List<SubscriptionInfo> infos =
                mSubscriptionManager.getActiveSubscriptionInfoList();
        if (infos == null) {
            return;
        }

        for (SubscriptionInfo info : infos) {
            int subId = info.getSubscriptionId();
            if (!mPhoneStateListeners.containsKey(subId)) {
                TelephonyManager telephonyManager =
                        mDefaultTelephonyManager.createForSubscriptionId(subId);
                OP01PhoneStateListener listener =
                        new OP01PhoneStateListener(subId, telephonyManager);
                mPhoneStateListeners.put(subId, listener);
                mPhoneStateMap.put(subId, TelephonyManager.CALL_STATE_IDLE);
                telephonyManager.listen(listener, PhoneStateListener.LISTEN_CALL_STATE);
            }
        }

        //Listen for sim changed.
        mSubscriptionManager.addOnSubscriptionsChangedListener(mSubscriptionsChangedListener);
    }

    private void updateStateListener() {
        Log.d(TAG, "updateStateListener");
        List<SubscriptionInfo> subInfos = mSubscriptionManager.getActiveSubscriptionInfoList();
        if (subInfos == null) {
            return;
        }

        //We should remove all the item in Listener, and then add the listener again.
        for (Integer id : mPhoneStateListeners.keySet()) {
            int subId = id.intValue();
            OP01PhoneStateListener listener = mPhoneStateListeners.get(id);
            listener.getTelephonyManager().listen(listener, PhoneStateListener.LISTEN_NONE);
        }

        mPhoneStateListeners.clear();
        mPhoneStateMap.clear();
        for (int i = 0; i < subInfos.size(); i++) {
            int subId = subInfos.get(i).getSubscriptionId();
            if (!mPhoneStateListeners.containsKey(subId)) {
                TelephonyManager telephonyManager =
                        mDefaultTelephonyManager.createForSubscriptionId(subId);
                OP01PhoneStateListener listener =
                        new OP01PhoneStateListener(subId, telephonyManager);

                mPhoneStateListeners.put(subId, listener);
                mPhoneStateMap.put(subId, TelephonyManager.CALL_STATE_IDLE);
                telephonyManager.listen(listener,
                        PhoneStateListener.LISTEN_CALL_STATE);
            }
        }
    }

    public static boolean canStartVideoCall() {
        return hasVideoCapability() &&
          ((getPhoneState() == TelephonyManager.CALL_STATE_IDLE));
    }

    public static boolean hasVideoCapability() {
      if (mTelecommManager == null) {
          mTelecommManager = (TelecomManager) mContext.getSystemService(Context.TELECOM_SERVICE);
      }

        List<PhoneAccountHandle> accountHandles = mTelecommManager.getCallCapablePhoneAccounts();
        for (PhoneAccountHandle accountHandle : accountHandles) {
            PhoneAccount account = mTelecommManager.getPhoneAccount(accountHandle);
            if (account != null && account.hasCapabilities(
                    PhoneAccount.CAPABILITY_VIDEO_CALLING)) {
                return true;
            }
        }
        return false;
    }

    public static int getPhoneState() {
        /*int state = TelephonyManager.CALL_STATE_IDLE;

        if (mTelecommManager == null) {
            mTelecommManager = (TelecomManager) mContext
                    .getSystemService(Context.TELECOM_SERVICE);
        }

        if (mTelecommManager != null) {
            state = mTelecommManager.getCallState();
        }

        Log.d(TAG, "getPhoneState, state = " + state);*/
        int state = TelephonyManager.CALL_STATE_IDLE;
        for (Integer phoneState : mPhoneStateMap.values()) {
            if (phoneState != TelephonyManager.CALL_STATE_IDLE) {
                state = phoneState;
                break;
            }
        }
        return state;
    }

    private class OP01PhoneStateListener extends PhoneStateListener {
        private int mSubId;
        private TelephonyManager mTelephonyManager;
        public OP01PhoneStateListener(int subId, TelephonyManager telephonyManager) {
            super();
            Log.d(TAG, "OP01PhoneStateListener subId:" + subId);
            mSubId = subId;
            mTelephonyManager = telephonyManager;
        }

        @Override
        public void onCallStateChanged(int state, String number) {
            mPhoneStateMap.put(mSubId, state);
            if (hasVideoCapability()) {
                updateState(state);
            }
        }

        public TelephonyManager getTelephonyManager() {
            return mTelephonyManager;
        }
    }
}
