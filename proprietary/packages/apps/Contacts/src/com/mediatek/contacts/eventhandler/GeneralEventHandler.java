package com.mediatek.contacts.eventhandler;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.text.TextUtils;

import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;

import com.mediatek.contacts.util.ContactsConstants;
import com.mediatek.contacts.util.Log;

import java.util.ArrayList;
import java.util.List;

/**
 * this class is used to handle base event.such as phb state change,
 * plug out/sd card.it is a base Event Receiver and dispatcher.
 */
public class GeneralEventHandler {
    private static final String TAG = "GeneralEventHandler";
    private static final String SDCARD_DATA_SCHEME = "file";

    public static final class SdCardState {
        public static final String SDSTEATE = "sdstate";
        public static final int SDCARD_ERROR = -1;
        public static final int SDCARD_REMOVED = 1;
        public static final int SDCARD_MOUNTED = 2;
    }

    public static final class PhbState {
        public static final String PHBREADY = "ready";
    }

    public static final class EventType {
        public static final String PHB_STATE_CHANGE_EVENT = "PhbChangeEvent";
        public static final String SD_STATE_CHANGE_EVENT = "SdStateChangeEvenet";
    }

    public interface Listener {

        /**
         * the callback to handle base event.
         *
         * @param eventType  receive event type
         * @param extraData  the related data in eventType
         */
        public void onReceiveEvent(String eventType, Intent extraData);
    }

    private List<Listener> mListeners = new ArrayList<Listener>();
    private Context mContext;
    private boolean mRegistered = false;
    private volatile static GeneralEventHandler uniqueInstance;

    /**
     * get the instance of the BaseEventHandler.
     * using double-checked locking for mutil-thread condition to spend
     * minimum time to get instance
     */
    public static GeneralEventHandler getInstance(Context context) {
        if (null == uniqueInstance ) {
            synchronized (GeneralEventHandler.class) {
                if (null == uniqueInstance) {
                    uniqueInstance = new GeneralEventHandler(context);
                }
            }
        }
        return uniqueInstance;
    }

    /**
     * register the listener.
     *
     * @param target the target register the listener.
     */
    public synchronized void register(Listener target) {
        Log.i(TAG, "[register] mContext: " + mContext + ",target: " + target +
            ",mRegistered = " + mRegistered);
        if (target != null && !mListeners.contains(target)) {
            mListeners.add(target);
        }
        if (!mRegistered) {
            registerBaseEventListener();
            mRegistered = true;
        }
    }

    /**
     * unRegister the listener.
     *
     * @param target
     *            the target unRegister the listener.
     */
    public synchronized void unRegister(Listener target) {
        Log.i(TAG, "[unRegister]target: " + target + ",mRegistered = " + mRegistered);
        if (target != null && mListeners.contains(target)) {
            mListeners.remove(target);
        }
        if (mListeners.isEmpty() && mRegistered) {
            unRegisterBaseEventListener();
            mRegistered = false;
        }
    }

    //private constructor. only called once
    private GeneralEventHandler(Context context) {
        mContext = context.getApplicationContext();
    }

    // for phb state change
    private BroadcastReceiver mPhbStateListener = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            final String action = intent.getAction();
            boolean isPhbReady = intent.getBooleanExtra(PhbState.PHBREADY, false);
            int subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY,
                    ContactsConstants.ERROR_SUB_ID);
            Log.i(TAG, "[PhbChangeState_onReceive]action: " + action + ",subId:" + subId +
                    ",phbReady: " + isPhbReady);
            if (TelephonyIntents.ACTION_PHB_STATE_CHANGED.equals(action)) {
                for (Listener listener : mListeners) {
                    if (listener != null) {
                        listener.onReceiveEvent(EventType.PHB_STATE_CHANGE_EVENT, intent);
                    }
                }
            }
        }
    };

    // for Sdcard state change
    private BroadcastReceiver mSdCardStateReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action == null) {
                Log.e(TAG, "[SdCardState_onReceive] get action is null,return");
                return;
            }
            Log.i(TAG, "[SdCardState_onReceive] action = " + action);
            int sdState = getSdCardMountedState(action);
            Intent extraData = new Intent();
            extraData.putExtra(SdCardState.SDSTEATE, sdState);
            for (Listener listener : mListeners) {
                if (listener != null) {
                    listener.onReceiveEvent(EventType.SD_STATE_CHANGE_EVENT, extraData);
                }
            }
        }
    };

    private void registerBaseEventListener() {
        //register sd state change listener
        IntentFilter sdcardFilter = new IntentFilter();
        sdcardFilter.addAction(Intent.ACTION_MEDIA_EJECT);
        sdcardFilter.addAction(Intent.ACTION_MEDIA_MOUNTED);
        sdcardFilter.addDataScheme(SDCARD_DATA_SCHEME);
        mContext.registerReceiver(mSdCardStateReceiver, sdcardFilter);

        //register phb state change listener
        mContext.registerReceiver(mPhbStateListener, new IntentFilter(
                TelephonyIntents.ACTION_PHB_STATE_CHANGED));
    }

    private void unRegisterBaseEventListener() {
        mContext.unregisterReceiver(mSdCardStateReceiver);
        mContext.unregisterReceiver(mPhbStateListener);
    }

    private int getSdCardMountedState(String action) {
        int rst = SdCardState.SDCARD_ERROR;
        if (TextUtils.isEmpty(action)) {
            Log.e(TAG, "[getSdCardMountedState] get action is null,return");
            return rst;
        }
        if (Intent.ACTION_MEDIA_EJECT.equals(action)) {
            rst = SdCardState.SDCARD_REMOVED;
        } else if (Intent.ACTION_MEDIA_MOUNTED.equals(action)) {
            rst = SdCardState.SDCARD_MOUNTED;
        }
        Log.i(TAG, "[getSdCardMountedState] rst: " + rst);
        return rst;
    }
}
