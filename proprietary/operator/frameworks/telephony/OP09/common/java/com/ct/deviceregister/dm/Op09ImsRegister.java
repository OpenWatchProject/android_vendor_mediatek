package com.ct.deviceregister.dm;

import android.app.Activity;
import android.app.AlarmManager.OnAlarmListener;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.telephony.PhoneStateListener;
import android.telephony.ServiceState;
import android.telephony.ims.ImsReasonInfo;
import android.util.Log;

import com.android.ims.ImsManager;
import com.ct.deviceregister.dm.utils.AgentProxyIms;
import com.ct.deviceregister.dm.utils.Utils;
import com.ct.deviceregister.dm.utils.PlatformManager;
import com.mediatek.ims.MtkImsConnectionStateListener;
import com.mediatek.internal.telephony.devreg.DeviceRegisterController;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class Op09ImsRegister {

    private static final String TAG = Const.TAG_PREFIX + "ImsRegister";

    private static final int MSG_SIM_STATE_CHANGED = 1;
    private static final int MSG_SIM_INSERVICE = 2;
    private static final int MSG_DO_REGISTER = 3;
    private static final int MSG_MESSAGE_SEND = 4;
    private static final int MSG_RETRY_REGISTER = 5;
    private static final int MSG_IMS_CONNECTED = 6;

    private static final int TIMES_MAX_WAIT = 3;
    private static final int TIMES_MAX_RETRY = 3;

    private boolean mHasInServiceAlarm = false;
    private int mWaitTimes = 0;
    private int mRetryTimes = 0;

    private int[] mSlotList;
    private String[] mImsiOnSim;
    private int[] mSubIdSnapShot;
    private CustomizedPhoneStateListener[] mPhoneStateListener;
    private CustomizedImsStateListener[] mImsStateListeners;

    private OnAlarmListener mRetryAlarmListener;

    private Context mContext;
    private AgentProxyIms mAgentProxyIms;
    private PlatformManager mPlatformManager;
    private SmsSendReceiver mSmsSendReceiver;
    private DeviceRegisterController mDeviceRegisterController;

    private Handler mHandler;
    private HandlerThread mHandlerThread;

    private List<Integer> mToRegisterLists = new ArrayList<>();

    private boolean[] mWaitForResponse;
    private long[] mSendMessageTime;
    private String[] mRegisterImsi;

    Op09ImsRegister(Context context, DeviceRegisterController controller) {
        mContext = context;
        mDeviceRegisterController = controller;
        init();
    }

    private void init() {
        Log.i(TAG, "init begin at " + System.currentTimeMillis());
        mPlatformManager = new PlatformManager(mContext);
        mAgentProxyIms = new AgentProxyIms(mContext);

        if (mPlatformManager.isSingleLoad()) {
            mSlotList = Const.SINGLE_SIM_SLOT;
        } else {
            mSlotList = Const.DUAL_SIM_SLOTS;
        }

        mPhoneStateListener = new CustomizedPhoneStateListener[mSlotList.length];
        mSubIdSnapShot = new int[mSlotList.length];
        mImsStateListeners = new CustomizedImsStateListener[mSlotList.length];

        mWaitForResponse = new boolean[mSlotList.length];
        mSendMessageTime = new long[mSlotList.length];
        mRegisterImsi = new String[mSlotList.length];

        for (int i = 0; i < mSlotList.length; ++i) {
            mPhoneStateListener[i] = null;
            mSubIdSnapShot[i] = Const.SUB_ID_INVALID;
            mWaitForResponse[i] = false;
            mSendMessageTime[i] = 0;
            mRegisterImsi[i] = "";
            mImsStateListeners[i] = null;
        }

        initHandler();
        Log.i(TAG, "init done at " + System.currentTimeMillis());
    }

    private void resetParameters() {
        Log.i(TAG, "resetParameters " + System.currentTimeMillis());
        unRegisterPhoneListeners();

        // not reset mRetryTimes
        mWaitTimes = 0;
    }

    /**
     * Invoke from DeviceRegister after boot complete
     */
    void onBootUp() {
        registerImsListener();
    }

    /**
     * Invoke from DeviceRegister after sim state change (absent or loaded)
     */
    void onSimStateChanged() {
        Log.i(TAG, "onSimStateChanged");
        if (!PlatformManager.isFeatureEnabled()) {
            Log.i(TAG, "Feature disabled, do nothing");
            resetParameters();
            return;
        }

        if (mHasInServiceAlarm) {
            Log.i(TAG, "Already find a sim IN_SERVICE, do nothing");
        } else {
            mHandler.sendEmptyMessage(MSG_SIM_STATE_CHANGED);
        }
    }

    private void initHandler() {
        mHandlerThread = new HandlerThread("ImsRegister");
        mHandlerThread.start();
        mHandler = new Handler(mHandlerThread.getLooper()) {

            @Override
            public void handleMessage(Message msg) {
                switch (msg.what) {
                    case MSG_SIM_STATE_CHANGED:
                    case MSG_IMS_CONNECTED:
                        registerPhoneListeners();
                        break;

                    case MSG_SIM_INSERVICE:
                        doAfterSimInService();
                        break;

                    case MSG_DO_REGISTER:
                        sendRegisterMessage(msg.arg1);
                        break;

                    case MSG_MESSAGE_SEND:
                        doAfterMessageSend(msg.arg1);
                        break;

                    case MSG_RETRY_REGISTER:
                        doAfterRetry();
                        break;

                    default:
                        break;
                }
            }
        };
    }

    private void doAfterSimInService() {
        mHasInServiceAlarm = false;

        Log.i(TAG, "doAfterSimInService, wait times " + mWaitTimes);
        if (!mPlatformManager.areSlotsInfoReady(mSlotList) && mWaitTimes < TIMES_MAX_WAIT) {
            long second = (mWaitTimes % 3 + 1) * 10 * Const.ONE_SECOND;
            setInServiceDelayAlarm(second);
            mWaitTimes++;
            return;
        }

        initIdentityInfo();

        // enqueue slot to register
        for (int i = 0; i < mSlotList.length; ++i) {
            if (!mToRegisterLists.contains(i) && needRegister(i)) {
                Log.i(TAG, "Add " + i + " to list");
                mToRegisterLists.add(i);
            }
        }

        if (mToRegisterLists.isEmpty()) {
            if (isWaitForResponse()) {
                Log.i(TAG, "Still wait for response.");
                unRegisterPhoneListeners();
                return;
            } else {
                Log.i(TAG, "Phone no need to register.");
                resetParameters();
                return;
            }
        }

        requestRegister();
    }

    private boolean isWaitForResponse() {
        for (int i = 0; i < mWaitForResponse.length; i++) {
            if (mWaitForResponse[i]) {
                return true;
            }
        }
        return false;
    }

    private void initIdentityInfo() {
        mImsiOnSim = new String[mSlotList.length];
        for (int i = 0; i < mSlotList.length; ++i) {
            mImsiOnSim[i] = mPlatformManager.getImsiInfo(mSlotList[i]);
        }
    }

    private boolean needRegister(int slotId) {
        if (!mPlatformManager.hasIccCard(slotId)) {
            Log.i(TAG, "[needRegister] slot " + slotId + " empty");
            return false;
        }

        if (mPlatformManager.isNetworkRoaming(slotId)) {
            Log.w(TAG, "[needRegister] slot " + slotId + " is roaming");
            return false;
        }

        // Scenario need to consider
        // 1. CT SIM's network change from LTE to CDMA (User disable or out of LTE region)
        // 2. CT SIM removed after IN_SERVICE & register LTE network
        if (!mPlatformManager.isValidUimForIms(slotId)) {
            Log.i(TAG, "[needRegister] slot " + slotId + " not a valid CT Sim");
            return false;
        }

        if (!mPlatformManager.isImsRegistered(slotId)) {
            Log.i(TAG, "[needRegister] slot " + slotId + " Ims not registered");
            return false;
        }

        if (mWaitForResponse[slotId]) {
            return isUimChangeOrTimeout(slotId);
        }

        if (mAgentProxyIms.isRegistered(slotId)) {
            Log.d(TAG, "[needRegister] registered before, check IMSI");

            // Only compare the register slot, not care whether other slot's SIM change or not
            if (isRegisterImsiSame(slotId)) {
                Log.i(TAG, "[needRegister] slot " + slotId + " imsi not change, no need");
                return false;
            }
        }
        Log.i(TAG, "[needRegister] slot " + slotId + " need register");
        return true;
    }

    private void requestRegister() {
        Log.i(TAG, "requestRegister");
        registerSendReceiver();
        long delay = Const.ONE_SECOND * 5;
        for (int i = 0; i < mToRegisterLists.size(); ++i) {
            int slot = mToRegisterLists.get(i);

            Message msg = Message.obtain(mHandler, MSG_DO_REGISTER, slot, 0);
            OnAlarmListener listener = new CustomizedAlarmListener(msg);
            PlatformManager.setElapsedAlarm(mContext, listener, delay * (i + 1));
        }
        mToRegisterLists.clear();
    }

    private boolean isRegisterImsiSame(int slotId) {
        String registeredImsi = mAgentProxyIms.getRegisterImsi(slotId);
        String currentImsi = mImsiOnSim[slotId];

        Log.i(TAG, "Registered/current imsi " + encryptMessage(registeredImsi)
                + "/" + encryptMessage(currentImsi));

        return registeredImsi.equals(currentImsi);
    }

    private boolean isImsiSame() {
        String[] imsiSaved = mAgentProxyIms.getSavedImsi();
        for (int i = 0; i < imsiSaved.length; ++i) {
            imsiSaved[i] = mPlatformManager.getProcessedImsi(imsiSaved[i]);
        }

        Log.i(TAG, "Saved imsi " + encryptMessage(imsiSaved));
        Log.i(TAG, "Current imsi " + encryptMessage(mImsiOnSim));

        return Utils.compareUnsortArray(mImsiOnSim, imsiSaved);
    }

    /**
     * Already send a message, whether uim changes or time out
     * @return true if Uim changes or last message sent > 5 minutes;
     *         false: same Uim and
     */
    private boolean isUimChangeOrTimeout(int slotId) {
        // Uim changes, do register
        if (!mRegisterImsi[slotId].equals(mImsiOnSim[slotId])) {
            Log.i(TAG, "[isUimChangeOrTimeout] Uim changes");
            return true;
        }

        // Uim not change, but not get response > 5 minutes, do register
        long interval = System.currentTimeMillis() - mSendMessageTime[slotId];
        Log.i(TAG, "Last register/current is " + mSendMessageTime[slotId] + "/"
                + System.currentTimeMillis() + ", interval is " + interval);
        if (interval >= Const.FIVE_MINUTES) {
            Log.i(TAG, "[isUimChangeOrTimeout] same Uim, response timeout (> 5 minutes)");
            return true;
        } else {
            Log.i(TAG, "[isUimChangeOrTimeout] same uim, not timeout");
            return false;
        }
    }

    private void sendRegisterMessage(int slotId) {
        RegisterMessageIms registerMessageIms = new RegisterMessageIms(mPlatformManager, slotId);
        byte[] data = registerMessageIms.getRegisterMessage();

        Intent intent = new Intent(Const.ACTION_MESSAGE_SEND_IMS);
        intent.putExtra(Const.EXTRA_REG_SLOT, slotId);
        PendingIntent pendingIntent = PendingIntent.getBroadcast(mContext, 0, intent,
                PendingIntent.FLAG_UPDATE_CURRENT);

        Log.i(TAG, "[sendRegisterMessage] slot: " + slotId + ", len: " + data.length
                + ", content (hide sensitive): " + registerMessageIms.encryptImeiImsi());

        try {
            PlatformManager.SmsWrapper.sendRegisterMessage(mDeviceRegisterController,
                    data, pendingIntent, slotId);

        } catch (UnsupportedOperationException | SecurityException  e) {
            Log.i(TAG, "Exception " + e);
            e.printStackTrace();
            setRetryAlarm(slotId);
        }
    }

    private void doAfterMessageSend(int slotId) {
        Log.i(TAG, "doAfterMessageSend " + slotId);

        if (slotId == Const.SLOT_ID_INVALID) {
            Log.i(TAG, "slot " + slotId + " is invalid");
            return;
        }

        mWaitForResponse[slotId] = true;
        mSendMessageTime[slotId] = System.currentTimeMillis();
        mRegisterImsi[slotId] = mImsiOnSim[slotId];

        unRegisterPhoneListeners();

        mAgentProxyIms.setRegisterFlag(slotId, false);
        mAgentProxyIms.setRegisterImsi(slotId, mImsiOnSim[slotId]);
        mAgentProxyIms.setSavedImsi(mImsiOnSim);
    }

    private void doAfterRetry() {
        Log.i(TAG, "doAfterRetry");
        mRetryAlarmListener = null;
        if (!mHasInServiceAlarm) {
            setInServiceDelayAlarm(Const.ONE_MINUTE);
        }
    }

    private void setRetryAlarm(int slotId) {
        Log.i(TAG, "setRetryAlarm " + slotId);

        if (PlatformManager.isSlotValid(slotId)) {
            if (mRetryTimes < TIMES_MAX_RETRY) {
                mRetryTimes++;
                Log.i(TAG, "Send failed, retry after 10 minutes ");
                mRetryAlarmListener = new CustomizedAlarmListener(MSG_RETRY_REGISTER);
                PlatformManager.setElapsedAlarm(mContext, mRetryAlarmListener, Const.TEN_MINUTES);
            } else {
                Log.i(TAG, "Send failed, reach limit " + mRetryTimes);
            }
        } else {
            Log.e(TAG, "setRetryAlarm, invalid slot");
        }
        resetParameters();
    }

    private void cancelRetryAlarm() {
        if (mRetryAlarmListener != null) {
            Log.i(TAG, "cancelRetryAlarm");
            PlatformManager.cancelAlarm(mContext, mRetryAlarmListener);
            mRetryAlarmListener = null;
        }
        mRetryTimes = 0;
    }

    private void setInServiceDelayAlarm() {
        setInServiceDelayAlarm(Const.ONE_MINUTE + 15 * Const.ONE_SECOND);
    }

    private void setInServiceDelayAlarm(long delay) {
        Log.i(TAG, "setInServiceDelayAlarm " + (delay / Const.ONE_SECOND) + "s");
        OnAlarmListener listener = new CustomizedAlarmListener(MSG_SIM_INSERVICE);
        PlatformManager.setElapsedAlarm(mContext, listener, delay);
        mHasInServiceAlarm = true;
    }

    /**
     * Invoke from DeviceRegisterExt, after get response from server
     */
    void onAutoRegMessage(int subId, String format, byte[] pdu) {
        Log.i(TAG, "onAutoRegMessage subId " + subId + ", format " + format + ", pdu "
                + Arrays.toString(pdu));

        int slotId = PlatformManager.getSlotId(subId);

        if (mWaitForResponse[slotId]) {
            boolean result = PlatformManager.SmsWrapper.checkRegisterResult(format, pdu);
            mAgentProxyIms.setRegisterFlag(slotId, result);
            if (result) {
                Log.i(TAG, "Register success!");
            } else {
                Log.i(TAG, "Register failed!");
            }
            resetAfterServerResponse(slotId);
        } else {
            Log.e(TAG, "Not wait for response, ignore");
        }
    }

    private void resetAfterServerResponse(int slotId) {
        Log.i(TAG, "resetAfterServerResponse " + slotId);
        mWaitForResponse[slotId] = false;

        for (int i = 0; i < mWaitForResponse.length; ++i) {
            if (mWaitForResponse[i]) {
                return;
            }
        }

        cancelRetryAlarm();
        resetParameters();
    }

    /**
     * Invoke from DeviceRegisterExt, after check fingerprint between system & SharedPreference
     */
    void resetRegisterFlag() {
        mAgentProxyIms.resetRegisterFlag();
    }

    private String encryptMessage(String[] array) {
        return PlatformManager.encryptMessage(array);
    }

    private String encryptMessage(String str) {
        return PlatformManager.encryptMessage(str);
    }

    private void registerPhoneListeners() {

        for (int i = 0; i < mSlotList.length; ++i) {

            int[] subId = PlatformManager.getSubId(mSlotList[i]);

            if (PlatformManager.isSubIdsValid(subId)) {

                if (subId[0] == mSubIdSnapShot[i]) {
                    Log.i(TAG, "[registerPhoneListeners] Slot " + i + " already" +
                            " registered.");
                } else {
                    Log.i(TAG, "[registerPhoneListeners] Slot " + i + " subId changed," +
                            " refresh");
                    unRegisterPhoneListener(mPhoneStateListener[i]);

                    mSubIdSnapShot[i] = subId[0];
                    mPhoneStateListener[i] = new CustomizedPhoneStateListener(i, subId[0]);
                    registerPhoneListener(mPhoneStateListener[i]);
                }

            } else {
                if (mPhoneStateListener[i] != null) {
                    Log.i(TAG, "[registerPhoneListeners] Slot " + i + " unplug, unregister");
                    unRegisterPhoneListener(mPhoneStateListener[i]);
                    mSubIdSnapShot[i] = Const.SUB_ID_INVALID;
                    mPhoneStateListener[i] = null;
                } else {
                    Log.i(TAG, "[registerPhoneListeners] No need for slot " + i);
                }

            }
        }
    }

    private void unRegisterPhoneListeners() {
        Log.i(TAG, "unRegisterPhoneListeners");

        for (int i = 0; i < mPhoneStateListener.length; ++i) {
            if (mPhoneStateListener[i] != null) {
                unRegisterPhoneListener(mPhoneStateListener[i]);
                mSubIdSnapShot[i] = Const.SUB_ID_INVALID;
                mPhoneStateListener[i] = null;
            }
        }
    }

    private void registerPhoneListener(CustomizedPhoneStateListener listener) {
        if (listener != null) {
            mPlatformManager.registerPhoneListener(listener, listener.mSubId);
        }
    }

    private void unRegisterPhoneListener(CustomizedPhoneStateListener listener) {
        if (listener != null) {
            mPlatformManager.unRegisterPhoneListener(listener, listener.mSubId);
        }
    }

    private void registerSendReceiver() {
        Log.i(TAG,"registerSendReceiver");
        if (mSmsSendReceiver == null) {
            mSmsSendReceiver = new SmsSendReceiver();
            IntentFilter filter = new IntentFilter();
            filter.addAction(Const.ACTION_MESSAGE_SEND_IMS);
            mContext.registerReceiver(mSmsSendReceiver, filter);
        }
    }

    public void registerImsListener() {
        for (int i = 0; i < mSlotList.length; ++i) {
            Log.i(TAG, "registerImsListener for phone " + i);
            ImsManager imsManager = ImsManager.getInstance(mContext, i);
            mImsStateListeners[i] = new CustomizedImsStateListener(i);
            mPlatformManager.addImsConnectionStateListener(imsManager, mImsStateListeners[i]);
        }
    }

    private class CustomizedImsStateListener extends MtkImsConnectionStateListener {

        private int mPhoneId;

        public CustomizedImsStateListener(int phoneId) {
            super();
            mPhoneId = phoneId;
        }

        @Override
        public void onImsConnected(int imsRadioTech) {
            Log.i(TAG, "onImsConnected " + mPhoneId);
            if (PlatformManager.isFeatureEnabled()) {
                mHandler.sendEmptyMessage(MSG_IMS_CONNECTED);
            } else {
                Log.i(TAG, "Feature disabled, do nothing");
            }
        }

        @Override
        public void onImsProgressing(int imsRadioTech) {
            Log.i(TAG, "onImsProgressing " + mPhoneId);
        }

        @Override
        public void onImsDisconnected(ImsReasonInfo imsReasonInfo) {
            Log.i(TAG, "onImsDisconnected " + mPhoneId);
        }
    }

    private class CustomizedAlarmListener implements OnAlarmListener {

        private int mMsgType;
        private Message mMessage;

        public CustomizedAlarmListener(int type) {
            mMsgType = type;
            mMessage = null;
        }

        public CustomizedAlarmListener(Message message) {
            mMsgType = 0;
            mMessage = message;
        }

        @Override
        public void onAlarm() {
            if (mMessage != null) {
                mHandler.sendMessage(mMessage);
            } else {
                mHandler.sendEmptyMessage(mMsgType);
            }
        }
    }

    private class CustomizedPhoneStateListener extends PhoneStateListener {

        private int mSlotId;
        private int mSubId;

        public CustomizedPhoneStateListener(int slotId, int subId) {
            mSlotId = slotId;
            mSubId = subId;
        }

        @Override
        public void onServiceStateChanged(ServiceState serviceState) {
            super.onServiceStateChanged(serviceState);

            if (mPlatformManager.isValidLteNetwork(serviceState)) {
                Log.i(TAG, "[onService " + mSlotId + "] voice/data in service");

                // As SIM info may not ready, do more check later
                if (!mHasInServiceAlarm) {
                    setInServiceDelayAlarm();
                }

            } else {
                Log.i(TAG, "[onService " + mSlotId + "] not in service");
            }
        }
    }

    private class SmsSendReceiver extends BroadcastReceiver {
        private static final String TAG = Const.TAG_PREFIX + "SmsSendReceiver";

        @Override
        public void onReceive(Context context, Intent intent) {
            Log.i(TAG, "onReceive " + intent);

            String action = intent.getAction();
            if (action.equalsIgnoreCase(Const.ACTION_MESSAGE_SEND_IMS)) {
                int resultCode = getResultCode();

                int slotId = intent.getIntExtra(Const.EXTRA_REG_SLOT, Const.SLOT_ID_INVALID);
                Log.i(TAG, "slot is " + slotId);

                if (resultCode == Activity.RESULT_OK) {

                    Message msg = Message.obtain(mHandler, MSG_MESSAGE_SEND, slotId, 0);
                    mHandler.sendMessage(msg);

                } else {
                    Log.i(TAG, "ResultCode: " + resultCode + ", failed.");
                    setRetryAlarm(slotId);
                }

            } else {
                Log.i(TAG, "action is not valid." + action);
            }
        }
    }
}
