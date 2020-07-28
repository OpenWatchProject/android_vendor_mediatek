
package com.mediatek.ims.internal;

import static android.net.ConnectivityManager.NETID_UNSET;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.ConnectivityManager.NetworkCallback;
import android.net.LinkProperties;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkInfo;
import android.net.NetworkRequest;
import android.net.NetworkRequest.Builder;
import android.os.Looper;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.telephony.Rlog;
import android.util.SparseArray;

import com.android.internal.telephony.PhoneConstants;
import com.android.internal.util.State;
import com.android.internal.util.StateMachine;

import com.mediatek.ims.common.SubscriptionManagerHelper;
import com.mediatek.ims.ImsAdapter;
import com.mediatek.ims.ImsAdapter.VaEvent;
import com.mediatek.ims.ImsEventDispatcher;
import com.mediatek.ims.VaConstants;
import com.mediatek.ims.internal.DataDispatcherUtil.ImsBearerRequest;

import java.util.Arrays;
import java.util.HashMap;

import vendor.mediatek.hardware.netdagent.V1_0.INetdagent;

public class DataDispatcher implements ImsEventDispatcher.VaEventDispatcher {
    private static final String TAG = DataDispatcher.class.getSimpleName();

    private Context mContext;
    private int mPhoneId, mSubId;
    private ImsDataTracker mTracker;
    private boolean mIsEnable;
    private Object mLock = new Object();

    private DataDispatcherUtil mDataDispatcherUtil;
    private HandlerThread mDcHandlerThread;
    private DataConnection mImsConnection, mEmcConnection;

    //Internal State Events
    static final int MSG_ID_WRAP_IMSM_IMSPA_PDN_ACT_COMPLETED = 800001;
    static final int MSG_ID_WRAP_IMSM_IMSPA_PDN_DEACT_COMPLETED = 800002;
    static final int MSG_ID_WRAP_IMSM_IMSPA_PDN_ACT_FAIL = 800003;
    static final int MSG_ID_WRAP_IMSM_IMSPA_PDN_ABORT = 800004;

    static final int MSG_ON_NOTIFY_ACTIVE_DATA_TIMEOUT = 800005;
    static final int MAX_NETWORK_ACTIVE_TIMEOUT_MS = 20000;

    private final int MSG_ID_IMSA_DISABLE_SERVICE = 700001;

    private final int MSG_ID_NOTIFY_SUBCRIPTION_CHANAGED = 600001;

    private static final String FAILCAUSE_NONE = "";
    private static final String FAILCAUSE_UNKNOWN = "UNKNOWN";
    private static final String FAILCAUSE_LOST_CONNECTION = "LOST_CONNECTION";

    private HashMap<String, Integer> mFailCauses = new HashMap<String, Integer>() {
        private static final long serialVersionUID = 1L;
        {
            put("", 0);
            put("OPERATOR_BARRED", 0x08);
            put("NAS_SIGNALLING", 0x0E);
            put("MBMS_CAPABILITIES_INSUFFICIENT", 0x18);
            put("LLC_SNDCP", 0x19);
            put("INSUFFICIENT_RESOURCES", 0x1A);
            put("MISSING_UNKNOWN_APN", 0x1B);
            put("UNKNOWN_PDP_ADDRESS_TYPE", 0x1C);
            put("USER_AUTHENTICATION", 0x1D);
            put("ACTIVATION_REJECT_GGSN", 0x1E);
            put("ACTIVATION_REJECT_UNSPECIFIED", 0x1F);
            put("SERVICE_OPTION_NOT_SUPPORTED", 0x20);
            put("SERVICE_OPTION_NOT_SUBSCRIBED", 0x21);
            put("SERVICE_OPTION_OUT_OF_ORDER", 0x22);
            put("NSAPI_IN_USE", 0x23);
            put("REGULAR_DEACTIVATION", 0x24);
            put("QOS_NOT_ACCEPTED", 0x25);
            put("NETWORK_FAILURE", 0x26);
            put("UMTS_REACTIVATION_REQ", 0x27);
            put("FEATURE_NOT_SUPP", 0x28);
            put("TFT_SEMANTIC_ERROR", 0x29);
            put("TFT_SYTAX_ERROR", 0x2A);
            put("UNKNOWN_PDP_CONTEXT", 0x2B);
            put("FILTER_SEMANTIC_ERROR", 0x2C);
            put("FILTER_SYTAX_ERROR", 0x2D);
            put("PDP_WITHOUT_ACTIVE_TFT", 0x2E);
            put("MULTICAST_GROUP_MEMBERSHIP_TIMEOUT", 0x2F);
            put("BCM_VIOLATION", 0x30);
            put("LAST_PDN_DISC_NOT_ALLOWED", 0x31);
            put("ONLY_IPV4_ALLOWED", 0x32);
            put("ONLY_IPV6_ALLOWED", 0x33);
            put("ONLY_SINGLE_BEARER_ALLOWED", 0x34);
            put("ESM_INFO_NOT_RECEIVED", 0x35);
            put("PDN_CONN_DOES_NOT_EXIST", 0x36);
            put("MULTI_CONN_TO_SAME_PDN_NOT_ALLOWED", 0x037);
            put("COLLISION_WITH_NW_INITIATED_REQUEST", 0x38);
            put("UNSUPPORTED_QCI_VALUE", 0x3B);
            put("BEARER_HANDLING_NOT_SUPPORT", 0x3C);
            put("MAX_ACTIVE_PDP_CONTEXT_REACHED", 0x41);
            put("UNSUPPORTED_APN_IN_CURRENT_PLMN", 0x42);
            put("INVALID_TRANSACTION_ID", 0x51);
            put("MESSAGE_INCORRECT_SEMANTIC", 0x5F);
            put("INVALID_MANDATORY_INFO", 0x60);
            put("MESSAGE_TYPE_UNSUPPORTED", 0x61);
            put("MSG_TYPE_NONCOMPATIBLE_STATE", 0x62);
            put("UNKNOWN_INFO_ELEMENT", 0x63);
            put("CONDITIONAL_IE_ERROR", 0x64);
            put("MSG_AND_PROTOCOL_STATE_UNCOMPATIBLE", 0x65);
            put("PROTOCOL_ERRORS", 0x6F);
            put("APN_TYPE_CONFLICT", 0x70);

            // Local errors generated by Vendor RIL
            // specified in ril.h
            put("REGISTRATION_FAIL", -1);
            put("GPRS_REGISTRATION_FAIL", -2);
            put("SIGNAL_LOST", -3);
            put("PREF_RADIO_TECH_CHANGED", -4);
            put("RADIO_POWER_OFF", -5);
            put("TETHERED_CALL_ACTIVE", -6);
            put("PDP_FAIL_ROUTER_ADVERTISEMENT", -7);

            put("PDP_FAIL_FALLBACK_RETRY", -1000);
            put("INSUFFICIENT_LOCAL_RESOURCES", 0xFFFFE);
            put("ERROR_UNSPECIFIED", 0xFFFF);

            // Errors generated by the Framework
            put("UNKNOWN", 0x10000);
            put("RADIO_NOT_AVAILABLE", 0x10001);
            put("UNACCEPTABLE_NETWORK_PARAMETER", 0x10002);
            put("CONNECTION_TO_DATACONNECTIONAC_BROKEN", 0x10003);
            put("LOST_CONNECTION", 0x10004);
            put("RESET_BY_FRAMEWORK", 0x10005);

            put("PAM_ATT_PDN_ACCESS_REJECT_IMS_PDN_BLOCK_TEMP", 0x1402);
            put("TCM_ESM_TIMER_TIMEOUT", 0x1502A);
            put("MTK_TCM_ESM_TIMER_TIMEOUT", 0x0F46);

            put("DUE_TO_REACH_RETRY_COUNTER", 0x0E0F);
            put("OEM_DCFAILCAUSE_12", 0x100C);
        }
    };

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();

            if (action.equalsIgnoreCase(
                    TelephonyManager.ACTION_PRECISE_DATA_CONNECTION_STATE_CHANGED)) {
                String type = intent.getStringExtra(PhoneConstants.DATA_APN_TYPE_KEY);
                String failure = intent.getStringExtra(PhoneConstants.DATA_FAILURE_CAUSE_KEY);
                //int subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY,
                //        SubscriptionManager.INVALID_SUBSCRIPTION_ID);
                int phoneId = intent.getIntExtra(PhoneConstants.PHONE_KEY,
                                                 SubscriptionManager.INVALID_PHONE_INDEX);
                int mCurrentPhoneId = SubscriptionManager.getPhoneId(mSubId);
                logd("ACTION_PRECISE_DATA_CONNECTION_STATE_CHANGED phoneId: " + phoneId +
                     ", mCurrentPhoneId: " + mCurrentPhoneId);

                if (phoneId == mCurrentPhoneId && failure != null && failure.length() > 0) {
                    logd("onReceive, intent action is " + intent.getAction());
                    logd("APN: " + type + " failCause: " + failure);
                    switch(type) {
                        case PhoneConstants.APN_TYPE_IMS:
                            Handler imsHandle = mImsConnection.getHandler();
                            imsHandle.sendMessage(
                                imsHandle.obtainMessage(MSG_ID_WRAP_IMSM_IMSPA_PDN_ACT_FAIL, failure));
                            mHandler.removeMessages(MSG_ON_NOTIFY_ACTIVE_DATA_TIMEOUT);
                            break;
                        case PhoneConstants.APN_TYPE_EMERGENCY:
                            Handler emcHandle = mEmcConnection.getHandler();
                            emcHandle.sendMessage(
                                emcHandle.obtainMessage(MSG_ID_WRAP_IMSM_IMSPA_PDN_ACT_FAIL, failure));
                            break;
                        default:
                            loge("UnKnown APN: " + type);
                            break;
                    }
                }
            }
        }
    };

    public DataDispatcher(Context context, ImsDataTracker tracker, int phoneId) {
        logd("constructor");
        mContext = context;
        mPhoneId = phoneId;
        mSubId = SubscriptionManagerHelper.getSubIdUsingPhoneId(mPhoneId);
        mTracker = tracker;
        mHandlerThread.start();
        mDcHandlerThread = new HandlerThread("DcHandlerThread");
        mDcHandlerThread.start();
        mImsConnection = new DataConnection(PhoneConstants.APN_TYPE_IMS,
                                            new Handler(mDcHandlerThread.getLooper()),
                                            NetworkCapabilities.NET_CAPABILITY_IMS);
        mEmcConnection = new DataConnection(PhoneConstants.APN_TYPE_EMERGENCY,
                                            new Handler(mDcHandlerThread.getLooper()),
                                            NetworkCapabilities.NET_CAPABILITY_EIMS);
        mDataDispatcherUtil = new DataDispatcherUtil();
    }

    public void enableRequest(int phoneId) {
        logi("receive enableRequest");
        synchronized (mLock) {
            mIsEnable = true;

            IntentFilter filter = new IntentFilter();
            filter.addAction(TelephonyManager.ACTION_PRECISE_DATA_CONNECTION_STATE_CHANGED);
            mContext.registerReceiver(mReceiver, filter);
        }
    }

    public void disableRequest(int phoneId) {
        logi("receive disableRequest");
        synchronized (mLock) {
            mIsEnable = false;
            mHandler.removeMessages(MSG_ON_NOTIFY_ACTIVE_DATA_TIMEOUT);
            mContext.unregisterReceiver(mReceiver);
            mImsConnection.disable();
            mEmcConnection.disable();
        }
    }

    /**
     * Handle Data related event from IMCB.
     *
     * @param VaEvent event from IMCB.
     */
    public void vaEventCallback(VaEvent event) {
        if (mIsEnable == true) {
            ImsBearerRequest request = mDataDispatcherUtil.deCodeEvent(event);
            logi("got request: " + request);

            //always reject pcscf address discovery by IMCB
            if (request.getRequestID() == VaConstants.MSG_ID_REQUEST_PCSCF_DISCOVERY) {
                StringBuilder builder = new StringBuilder();
                builder.append(request.getTransId() + ",");
                builder.append(mPhoneId + ",");
                builder.append(mFailCauses.get(FAILCAUSE_UNKNOWN) + "");
                sendVaEvent(mDataDispatcherUtil.enCodeEvent(
                        VaConstants.MSG_ID_REJECT_PCSCF_DISCOVERY, builder.toString()));
                return;
            }

            switch(request.getCapability()) {
                case PhoneConstants.APN_TYPE_IMS:
                    Handler imsHandle = mImsConnection.getHandler();
                    mImsConnection.putRequest(request);
                    imsHandle.sendMessage(imsHandle.obtainMessage(request.getRequestID()));
                    break;
                case PhoneConstants.APN_TYPE_EMERGENCY:
                    Handler emcHandle = mEmcConnection.getHandler();
                    mEmcConnection.putRequest(request);
                    emcHandle.sendMessage(emcHandle.obtainMessage(request.getRequestID()));
                    break;
                default:
                    loge("not support capbility: " + request.getCapability());
                    break;
            }
        } else {
            loge("ims service not be enabled");
        }
    }

    public void sendVaEvent(VaEvent event) {
        if (mIsEnable == true) {
            logi("send event [" + event.getRequestID() + ", " + event.getDataLen() + "]");
            mTracker.sendVaEvent(event);
        } else {
            loge("ims service not be enabled");
        }
    }

    public void onSubscriptionsChanged() {
        //update subId
        int newSubId = SubscriptionManagerHelper.getSubIdUsingPhoneId(mPhoneId);
        logd("onSubscriptionsChanged: subId: " + mSubId + ", newSubId: " + newSubId);
        if (mSubId == newSubId) {
            return;
        }
        mSubId = newSubId;

        mImsConnection.onSubscriptionsChanged();
        mEmcConnection.onSubscriptionsChanged();
    }

    public void logd(String s) {
        Rlog.d(TAG, "[" + mPhoneId + "]" + s);
    }

    public void logi(String s) {
        Rlog.i(TAG, "[" + mPhoneId + "]" + s);
    }

    public void loge(String s) {
        Rlog.e(TAG, "[" + mPhoneId + "]" + s);
    }

    private Handler mHandler;
    private Thread mHandlerThread = new Thread() {
        @Override
        public void run() {
            Looper.prepare();
            mHandler = new Handler() { // create handler here
                @Override
                synchronized public void handleMessage(Message msg) {
                    logd("receives request [" + msg.what + "]");

                    switch (msg.what) {
                        case MSG_ON_NOTIFY_ACTIVE_DATA_TIMEOUT:
                            mImsConnection.onImsRequestTimeout();
                            break;
                        default:
                            logd("receives unhandled message [" + msg.what + "]");
                    }
                }
            };
            Looper.loop();
        }
    };

    public class DataConnection extends StateMachine {

        private String TAG = "DC-";
        private int mCapabiliy;

        private ConnectivityManager mConnectivityManager;
        private SparseArray<ImsBearerRequest> mImsNetworkRequests =
                    new SparseArray<ImsBearerRequest>();

        private NwAvailableCallback mNwAvailableCallback;
        private NwLostCallback mNwLostCallback;

        private int mNetworkId;
        private long mNetworkHandle = NETID_UNSET;
        private String mInterface = "";
        private String mFwInterface = "";

        private DefaultState mDefaultState = new DefaultState();
        private InactiveState mInactiveState = new InactiveState();
        private ActivatingState mActivatingState = new ActivatingState();
        private ActiveState mActiveState = new ActiveState();
        private DisconnectingState mDisconnectingState = new DisconnectingState();

        public DataConnection(String name, Handler mHandler, int capability) {
            super(name, mHandler);
            mCapabiliy = capability;
            mConnectivityManager = (ConnectivityManager)
                mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
            mNwAvailableCallback = new NwAvailableCallback(this);
            mNwLostCallback = new NwLostCallback(this);

            addState(mDefaultState);
            addState(mInactiveState, mDefaultState);
            addState(mActivatingState, mDefaultState);
            addState(mActiveState, mDefaultState);
            addState(mDisconnectingState, mDefaultState);
            setInitialState(mInactiveState);
            start();
        }

        private class DefaultState extends State {
            @Override
            public void enter() {
                logd("DefaultState: enter");
            }

            @Override
            public void exit() {
                logd("DefaultState: exit");
            }

            @Override
            public boolean processMessage(Message msg) {
                boolean retVal = HANDLED;
                logd("DefaultState msg: " + msgToString(msg.what));
                switch (msg.what) {
                    case VaConstants.MSG_ID_WRAP_IMSM_IMSPA_PDN_ACT_REQ:
                        rejectNetworkRequest(mFailCauses.get(FAILCAUSE_NONE));
                        break;
                    case VaConstants.MSG_ID_WRAP_IMSM_IMSPA_PDN_DEACT_REQ:
                        rejectNetworkReleased(mFailCauses.get(FAILCAUSE_NONE));
                        break;
                    default:
                        loge("DefaultState not handled request: " + msgToString(msg.what));
                        break;
                }

                return retVal;
            }
        }

        private class InactiveState extends State {
            @Override
            public void enter() {
                logd("InactiveState: enter");
            }

            @Override
            public void exit() {
                logd("InactiveState: exit");
            }

            @Override
            public boolean processMessage(Message msg) {
                boolean retVal = HANDLED;
                logd("InactiveState msg: " + msgToString(msg.what));
                switch (msg.what) {
                    case VaConstants.MSG_ID_WRAP_IMSM_IMSPA_PDN_ACT_REQ:
                    case MSG_ID_NOTIFY_SUBCRIPTION_CHANAGED:
                        if (requestNetwork() ) {
                            transitionTo(mActivatingState);
                        }
                        break;
                    case MSG_ID_WRAP_IMSM_IMSPA_PDN_ABORT:
                        rejectNetworkRequest(mFailCauses.get(FAILCAUSE_UNKNOWN));
                        releaseNetwork();
                        onAbortNetworkCompleted();
                        break;
                    case VaConstants.MSG_ID_WRAP_IMSM_IMSPA_PDN_DEACT_REQ:
                        releaseNetwork();
                    case MSG_ID_WRAP_IMSM_IMSPA_PDN_DEACT_COMPLETED:
                        onReleaseNetworkCompleted();
                        break;
                    case MSG_ID_IMSA_DISABLE_SERVICE:
                        clearNwInfo(true);
                        break;
                    default:
                        loge("InactiveState not handled request: " + msgToString(msg.what));
                        retVal = NOT_HANDLED;
                        break;
                }

                return retVal;
            }
        }

        private class ActivatingState extends State {
            @Override
            public void enter() {
                logd("ActivatingState: enter");
            }

            @Override
            public void exit() {
                logd("ActivatingState: exit");
            }

            @Override
            public boolean processMessage(Message msg) {
                boolean retVal = HANDLED;
                logd("ActivatingState msg: " + msgToString(msg.what));
                switch (msg.what) {
                    case MSG_ID_NOTIFY_SUBCRIPTION_CHANAGED:
                        releaseNetwork();
                        rejectNetworkRequest(mFailCauses.get(FAILCAUSE_UNKNOWN));
                        transitionTo(mInactiveState);
                        break;
                    case MSG_ID_WRAP_IMSM_IMSPA_PDN_ACT_FAIL:
                        releaseNetwork();
                        rejectNetworkRequest(mFailCauses.get((String)msg.obj));
                        transitionTo(mInactiveState);
                        break;
                    case MSG_ID_WRAP_IMSM_IMSPA_PDN_ACT_COMPLETED:
                        deferMessage(msg);
                        transitionTo(mActiveState);
                        break;
                    case MSG_ID_WRAP_IMSM_IMSPA_PDN_ABORT:
                        deferMessage(msg);
                        transitionTo(mInactiveState);
                        break;
                    case MSG_ID_IMSA_DISABLE_SERVICE:
                        releaseNetwork();
                        transitionTo(mInactiveState);
                        clearNwInfo(true);
                        break;
                    default:
                        loge("ActivatingState not handled request: " + msgToString(msg.what));
                        retVal = NOT_HANDLED;
                        break;
                }

                return retVal;
            }
        }

        private class ActiveState extends State {
            @Override
            public void enter() {
                logd("ActiveState: enter");
                setFirewallInterfaceChain(true);
                mFwInterface = mInterface;
            }

            @Override
            public void exit() {
                logd("ActiveState: exit");
                setFirewallInterfaceChain(false);
            }

            @Override
            public boolean processMessage(Message msg) {
                boolean retVal = HANDLED;

                logd("ActiveState msg: " + msgToString(msg.what));
                switch (msg.what) {
                    case VaConstants.MSG_ID_WRAP_IMSM_IMSPA_PDN_ACT_REQ:
                        deferMessage(msg);
                        releaseNetwork();
                        transitionTo(mDisconnectingState);
                        break;
                    case MSG_ID_WRAP_IMSM_IMSPA_PDN_ACT_COMPLETED:
                        onRequestNetworkCompleted();
                        break;
                    case VaConstants.MSG_ID_WRAP_IMSM_IMSPA_PDN_DEACT_REQ:
                        releaseNetwork();
                        transitionTo(mDisconnectingState);
                        break;
                    case MSG_ID_WRAP_IMSM_IMSPA_PDN_DEACT_COMPLETED:
                        notifyNetworkLosted();
                        transitionTo(mInactiveState);
                        break;
                    case MSG_ID_WRAP_IMSM_IMSPA_PDN_ABORT:
                        deferMessage(msg);
                        transitionTo(mInactiveState);
                        break;
                    case MSG_ID_IMSA_DISABLE_SERVICE:
                        releaseNetwork();
                        transitionTo(mInactiveState);
                        clearNwInfo(true);
                        break;
                    default:
                        loge("ActiveState not handled request: " + msgToString(msg.what));
                        retVal = NOT_HANDLED;
                        break;
                }

                return retVal;
            }
        }

        private class DisconnectingState extends State {
            @Override
            public void enter() {
                logd("DisconnectingState: enter");
            }

            @Override
            public void exit() {
                logd("DisconnectingState: exit");
            }

            @Override
            public boolean processMessage(Message msg) {
                boolean retVal = HANDLED;
                logd("DisconnectingState msg: " + msgToString(msg.what));
                switch (msg.what) {
                    case VaConstants.MSG_ID_WRAP_IMSM_IMSPA_PDN_ACT_REQ:
                        deferMessage(msg);
                        break;
                    case MSG_ID_WRAP_IMSM_IMSPA_PDN_DEACT_COMPLETED:
                    case MSG_ID_NOTIFY_SUBCRIPTION_CHANAGED:
                        onReleaseNetworkCompleted();
                        transitionTo(mInactiveState);
                        break;
                    case MSG_ID_IMSA_DISABLE_SERVICE:
                        transitionTo(mInactiveState);
                        clearNwInfo(true);
                        break;
                    default:
                        loge("DisconnectingState not handled request: " + msgToString(msg.what));
                        retVal = NOT_HANDLED;
                        break;
                }

                return retVal;
            }
        }

        private boolean requestNetwork() {
            logd("requestNetwork");
            if (!SubscriptionManager.isValidSubscriptionId(mSubId)) {
                if (mCapabiliy != NetworkCapabilities.NET_CAPABILITY_EIMS) {
                    loge("inValid subId: " + mSubId);
                    return false;
                }
            }

            ImsBearerRequest n = mImsNetworkRequests.get(
                    VaConstants.MSG_ID_WRAP_IMSM_IMSPA_PDN_ACT_REQ);

            if (n == null) {
                loge("ImsBearerRequest is NULL");
                return false;
            }

            Builder builder = new NetworkRequest.Builder();
            builder.addCapability(mCapabiliy);
            builder.addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR);
            builder.setNetworkSpecifier(String.valueOf(mSubId));
            NetworkRequest nwRequest = builder.build();

            refreshNwLostCallBack(nwRequest);
            if (mCapabiliy == NetworkCapabilities.NET_CAPABILITY_IMS) {
                mHandler.removeMessages(MSG_ON_NOTIFY_ACTIVE_DATA_TIMEOUT);
                mHandler.sendMessageDelayed(mHandler.obtainMessage(
                      MSG_ON_NOTIFY_ACTIVE_DATA_TIMEOUT),
                      MAX_NETWORK_ACTIVE_TIMEOUT_MS);
            }
            logd("start requestNetwork for " + getName());
            mConnectivityManager.requestNetwork(nwRequest, mNwAvailableCallback);
            return true;
        }

        private void rejectNetworkRequest(int cause) {
            logd("rejectNetworkRequest cause: " + cause);
            ImsBearerRequest n = mImsNetworkRequests.get(
                    VaConstants.MSG_ID_WRAP_IMSM_IMSPA_PDN_ACT_REQ);
            for (int i = 0; i < mImsNetworkRequests.size(); i++) {
                logd("found Req: " + mImsNetworkRequests.valueAt(i));
            }
            if (n != null) {
                mHandler.removeMessages(MSG_ON_NOTIFY_ACTIVE_DATA_TIMEOUT);
                StringBuilder builder = new StringBuilder();
                builder.append(n.getTransId() + ",");
                builder.append(mPhoneId + ",");
                builder.append(cause + "");
                sendVaEvent(mDataDispatcherUtil.enCodeEvent(
                        VaConstants.MSG_ID_WRAP_IMSPA_IMSM_PDN_ACT_REJ_RESP, builder.toString()));
                clearNwInfo(false);
                mImsNetworkRequests.remove(n.getRequestID());
            }
        }

        private void onRequestNetworkCompleted() {
            logd("onRequestNetworkComplete");
            ImsBearerRequest n = mImsNetworkRequests.get(
                    VaConstants.MSG_ID_WRAP_IMSM_IMSPA_PDN_ACT_REQ);
            for (int i = 0; i < mImsNetworkRequests.size(); i++) {
                logd("found Req: " + mImsNetworkRequests.valueAt(i));
            }
            if (n != null) {
                // HIDL: notify netHandle to IMCB before PDN ACT response
                notifyNetworkHandle();

                StringBuilder builder = new StringBuilder();
                builder.append(n.getTransId() + ",");
                builder.append(mPhoneId + ",");
                builder.append(mNetworkId + ",");
                builder.append(mInterface + "");
                logd("netId:" + mNetworkId + " IfaceName:" + mInterface);

                sendVaEvent(mDataDispatcherUtil.enCodeEvent(
                        VaConstants.MSG_ID_WRAP_IMSPA_IMSM_PDN_ACT_ACK_RESP,builder.toString()));
                mImsNetworkRequests.remove(n.getRequestID());
            }
        }

        private void notifyNetworkHandle() {
            log("notifyNetworkHandle() netHandle: " + mNetworkHandle);
            StringBuilder builder = new StringBuilder();
            builder.append(mPhoneId + ",");
            builder.append(mNetworkHandle);
            sendVaEvent(mDataDispatcherUtil.enCodeEvent(
                    VaConstants.MSG_ID_WRAP_IMSPA_IMSM_PDN_NETWORK_HANDLE_NOTIFY,
                    builder.toString()));
        }

        private void releaseNetwork() {
            logd("releaseNetwork");
            ImsBearerRequest n = mImsNetworkRequests.get(
                        VaConstants.MSG_ID_WRAP_IMSM_IMSPA_PDN_DEACT_REQ);
            mHandler.removeMessages(MSG_ON_NOTIFY_ACTIVE_DATA_TIMEOUT);
            try {
                mConnectivityManager.unregisterNetworkCallback(mNwAvailableCallback);
            } catch (IllegalArgumentException ex) {
                loge("cb already has been released!!");
            }
        }

        private void rejectNetworkReleased(int cause) {
            logd("rejectNetworkReleased cause: " + cause);
            ImsBearerRequest n = mImsNetworkRequests.get(
                    VaConstants.MSG_ID_WRAP_IMSM_IMSPA_PDN_DEACT_REQ);
            for (int i = 0; i < mImsNetworkRequests.size(); i++) {
                logd("found Req: " + mImsNetworkRequests.valueAt(i));
            }
            if (n != null) {
                StringBuilder builder = new StringBuilder();
                builder.append(n.getTransId() + ",");
                builder.append(mPhoneId + ",");
                builder.append(cause + "");
                sendVaEvent(mDataDispatcherUtil.enCodeEvent(
                        VaConstants.MSG_ID_WRAP_IMSPA_IMSM_PDN_DEACT_REJ_RESP,builder.toString()));
                mImsNetworkRequests.remove(n.getRequestID());
            }
        }

        private void onReleaseNetworkCompleted() {
            logd("onReleaseNetworkCompleted");
            ImsBearerRequest n = mImsNetworkRequests.get(
                    VaConstants.MSG_ID_WRAP_IMSM_IMSPA_PDN_DEACT_REQ);
            for (int i = 0; i < mImsNetworkRequests.size(); i++) {
                logd("found Req: " + mImsNetworkRequests.valueAt(i));
            }
            if (n != null) {
                StringBuilder builder = new StringBuilder();
                builder.append(n.getTransId() + ",");
                builder.append(mPhoneId + ",");
                builder.append(mFailCauses.get(FAILCAUSE_UNKNOWN) + ",");
                builder.append(mInterface + "");
                sendVaEvent(mDataDispatcherUtil.enCodeEvent(
                        VaConstants.MSG_ID_WRAP_IMSPA_IMSM_PDN_DEACT_ACK_RESP,builder.toString()));
                mImsNetworkRequests.remove(n.getRequestID());
            }
            clearNwInfo(false);
        }

        private void onAbortNetworkCompleted() {
            logd("onAbortNetworkCompleted");
            ImsBearerRequest n = mImsNetworkRequests.get(MSG_ID_WRAP_IMSM_IMSPA_PDN_ABORT);
            for (int i = 0; i < mImsNetworkRequests.size(); i++) {
                logd("found Req: " + mImsNetworkRequests.valueAt(i));
            }
            if (n != null) {
                StringBuilder builder = new StringBuilder();
                builder.append(n.getTransId() + ",");
                builder.append(mPhoneId + ",");
                builder.append(mFailCauses.get(FAILCAUSE_UNKNOWN) + ",");
                builder.append(mInterface + "");
                sendVaEvent(mDataDispatcherUtil.enCodeEvent(
                        VaConstants.MSG_ID_WRAP_IMSPA_IMSM_PDN_DEACT_ACK_RESP,builder.toString()));
                mImsNetworkRequests.remove(n.getRequestID());
            }
            clearNwInfo(false);
        }

        private void notifyNetworkLosted() {
            logd("notifyNetworkLosted");

            try {
                mConnectivityManager.unregisterNetworkCallback(mNwAvailableCallback);
            } catch (IllegalArgumentException ex) {
                loge("cb already has been released!!");
            }

            StringBuilder builder = new StringBuilder();
            builder.append(mPhoneId + ",");
            builder.append(mFailCauses.get(FAILCAUSE_LOST_CONNECTION) + ",");
            builder.append(mInterface + "");
            sendVaEvent(mDataDispatcherUtil.enCodeEvent(
                    VaConstants.MSG_ID_WRAP_IMSPA_IMSM_PDN_DEACT_IND,builder.toString()));
            clearNwInfo(false);
        }

        private void disable() {
            sendMessage(obtainMessage(MSG_ID_IMSA_DISABLE_SERVICE));
        }

        private void clearNwInfo(boolean disable) {
            logd("clearNwInfo");
            if (disable) {
                mImsNetworkRequests.clear();
            }
            mNetworkId = 0;
            mNetworkHandle = NETID_UNSET;
            mInterface = "";
        }

        public void putRequest(ImsBearerRequest request) {
           ImsBearerRequest n = mImsNetworkRequests.get(request.getRequestID());
           if (n == null) {
               mImsNetworkRequests.put(request.getRequestID(), request);
           } else {
               loge("request already exist: " + request);
           }
        }

        public void onSubscriptionsChanged() {
            logd("onSubscriptionsChanged");
            ImsBearerRequest n1 = mImsNetworkRequests.get(
                    VaConstants.MSG_ID_WRAP_IMSM_IMSPA_PDN_ACT_REQ);
            ImsBearerRequest n2 = mImsNetworkRequests.get(
                    VaConstants.MSG_ID_WRAP_IMSM_IMSPA_PDN_DEACT_REQ);
            if (n1 != null || n2 != null) {
                sendMessage(obtainMessage(MSG_ID_NOTIFY_SUBCRIPTION_CHANAGED));
            }
        }

        public void onImsRequestTimeout() {
            logd("onImsRequestTimeout");
            ImsBearerRequest n1 = mImsNetworkRequests.get(
                    VaConstants.MSG_ID_WRAP_IMSM_IMSPA_PDN_ACT_REQ);
            if (n1 != null) {
                logd("get request type " + n1.getCapability());
                if(n1.getCapability() == PhoneConstants.APN_TYPE_IMS) {
                    Handler imsHandle = mImsConnection.getHandler();
                    imsHandle.sendMessage(
                       imsHandle.obtainMessage(MSG_ID_WRAP_IMSM_IMSPA_PDN_ACT_FAIL, FAILCAUSE_UNKNOWN));
                    mHandler.removeMessages(MSG_ON_NOTIFY_ACTIVE_DATA_TIMEOUT);
                }
            }
        }

        private void refreshNwLostCallBack(NetworkRequest nwRequest) {
            logd("refreshNwLostCallBack nwRequest: " + nwRequest);

            try {
                mConnectivityManager.unregisterNetworkCallback(mNwLostCallback);
            } catch (IllegalArgumentException ex) {
                loge("cb already has been released!!");
            }
            mConnectivityManager.registerNetworkCallback(nwRequest, mNwLostCallback);
        }


        private class NwAvailableCallback extends NetworkCallback {
            private DataConnection mConn;

            public NwAvailableCallback(DataConnection conn) {
                mConn = conn;
            }

            @Override
            public void onAvailable(Network network) {
                if (network == null) {
                    loge("onAvailable: network is null");
                    return;
                }
                mHandler.removeMessages(MSG_ON_NOTIFY_ACTIVE_DATA_TIMEOUT);
                LinkProperties mLink = mConnectivityManager.getLinkProperties(network);
                if (mLink == null) {
                    loge("LinkProperties is null");
                    return;
                }

                NetworkInfo netInfo = mConnectivityManager.getNetworkInfo(network);
                logd("onAvailable: networInfo: " + netInfo);

                mNetworkId = network.netId;
                mNetworkHandle = network.getNetworkHandle();
                mInterface = mLink.getInterfaceName();
                mConn.sendMessage(mConn.obtainMessage(
                                  MSG_ID_WRAP_IMSM_IMSPA_PDN_ACT_COMPLETED));
            }
        }

        private class NwLostCallback extends NetworkCallback {
            private DataConnection mConn;

            public NwLostCallback(DataConnection conn) {
                mConn = conn;
            }

            @Override
            public void onLost(Network network) {
                mHandler.removeMessages(MSG_ON_NOTIFY_ACTIVE_DATA_TIMEOUT);
                NetworkInfo netInfo = mConnectivityManager.getNetworkInfo(network);
                logd("onLost: networInfo: " + netInfo);
                mConn.sendMessage(mConn.obtainMessage(
                                  MSG_ID_WRAP_IMSM_IMSPA_PDN_DEACT_COMPLETED));
            }
        }

        private String msgToString(int msg) {
            switch(msg) {
                case VaConstants.MSG_ID_WRAP_IMSM_IMSPA_PDN_ACT_REQ:
                    return "MSG_ID_WRAP_IMSM_IMSPA_PDN_ACT_REQ";
                case VaConstants.MSG_ID_WRAP_IMSM_IMSPA_PDN_DEACT_REQ:
                    return "MSG_ID_WRAP_IMSM_IMSPA_PDN_DEACT_REQ";
                case VaConstants.MSG_ID_REQUEST_PCSCF_DISCOVERY:
                    return "MSG_ID_REQUEST_PCSCF_DISCOVERY";
                case MSG_ID_WRAP_IMSM_IMSPA_PDN_ACT_COMPLETED:
                    return "MSG_ID_WRAP_IMSM_IMSPA_PDN_ACT_COMPLETED";
                case MSG_ID_WRAP_IMSM_IMSPA_PDN_DEACT_COMPLETED:
                    return "MSG_ID_WRAP_IMSM_IMSPA_PDN_DEACT_COMPLETED";
                case MSG_ID_WRAP_IMSM_IMSPA_PDN_ACT_FAIL:
                    return "MSG_ID_WRAP_IMSM_IMSPA_PDN_ACT_FAIL";
                case MSG_ID_IMSA_DISABLE_SERVICE:
                    return "MSG_ID_IMSA_DISABLE_SERVICE";
                case MSG_ID_WRAP_IMSM_IMSPA_PDN_ABORT:
                    return "MSG_ID_WRAP_IMSM_IMSPA_PDN_ABORT";
                default: return "" + msg;
            }
        }

        public void logd(String s) {
            Rlog.d(TAG + getName(), "[" + mPhoneId + "] " + s);
        }

        public void logi(String s) {
            Rlog.i(TAG + getName(), "[" + mPhoneId + "] " + s);
        }

        public void loge(String s) {
            Rlog.e(TAG + getName(), "[" + mPhoneId + "] " + s);
        }

        private void setFirewallInterfaceChain(boolean isAdded) {
            logd("setFirewallInterfaceChain:" + isAdded);
            Thread thread = new Thread("setFirewallInterfaceChain") {
                public void run() {
                    try {
                        INetdagent agent = INetdagent.getService();
                        if (agent == null) {
                            loge("agnet is null");
                            return;
                        }
                        final String rule = isAdded ? "allow" : "deny";
                        String cmd = String.format(
                                "netdagent firewall set_interface_for_chain_rule %s dozable %s",
                                mFwInterface, rule);
                        logd("cmd:" + cmd);
                        agent.dispatchNetdagentCmd(cmd);
                    } catch (Exception e) {
                        loge("setFirewallInterfaceChain:" + e);
                    }
                }
            };
            thread.start();
        }
    }
}
