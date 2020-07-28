package com.mediatek.ims.internal;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.ConnectivityManager.NetworkCallback;
import android.net.LinkProperties;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkInfo;
import android.net.NetworkRequest;
import android.net.NetworkRequest.Builder;
import android.telephony.Rlog;
import android.os.Looper;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.os.SystemProperties;
import android.util.SparseArray;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.util.State;
import com.android.internal.util.StateMachine;

import com.mediatek.ims.common.SubscriptionManagerHelper;

import java.io.Serializable;

public class ImsDataSynchronizer {

    private String TAG = ImsDataSynchronizer.class.getSimpleName();
    private int mPhoneId, mSubId;
    private Context mContext;
    private ImsDataTracker mDataTracker;
    private HandlerThread mImsDcHandlerThread;
    private DataConnection mImsDataConnection, mEmcDataConnection;

    public static final int EVENT_CONNECT          = 0;
    public static final int EVENT_CONNECT_DONE     = 1;
    public static final int EVENT_DISCONNECT       = 2;
    public static final int EVENT_DISCONNECT_DONE  = 3;
    public static final int EVENT_SUBSCRIPTIONS_CHANGED  = 4;
    public static final int EVENT_MD_RESTART  = 5;
    public static final int EVENT_SET_BEARER_NOTIFICATION_DONE  = 6;
    public static final int EVENT_IMS_DATA_INFO  = 7;
    public static final int EVENT_BEARER_STATE_CHANGED = 8;

    public static final int ACTION_ACTIVATION = 1;
    public static final int ACTION_DEACTIVATION = 0;

    public ImsDataSynchronizer(Context context, ImsDataTracker dataTracker, int phoneId) {
        mContext = context;
        mDataTracker = dataTracker;
        mPhoneId = phoneId;
        mSubId = SubscriptionManagerHelper.getSubIdUsingPhoneId(mPhoneId);
        mImsDcHandlerThread = new HandlerThread("ImsDcHandlerThread");
        mImsDcHandlerThread.start();
        mImsDataConnection = new DataConnection(PhoneConstants.APN_TYPE_IMS,
                new Handler(mImsDcHandlerThread.getLooper()),
                NetworkCapabilities.NET_CAPABILITY_IMS);
        mEmcDataConnection = new DataConnection(PhoneConstants.APN_TYPE_EMERGENCY,
                new Handler(mImsDcHandlerThread.getLooper()),
                NetworkCapabilities.NET_CAPABILITY_EIMS);
    }

    public void notifyMdRequest(ImsBearerRequest request) {
        logd("got request: " + request);
        switch(request.getCapability()) {
            case PhoneConstants.APN_TYPE_IMS:
                Handler imsHandle = mImsDataConnection.getHandler();
                mImsDataConnection.putRequest(request);
                imsHandle.sendMessage(imsHandle.obtainMessage(request.getRequest()));
                break;
            case PhoneConstants.APN_TYPE_EMERGENCY:
                Handler emcHandle = mEmcDataConnection.getHandler();
                mEmcDataConnection.putRequest(request);
                emcHandle.sendMessage(emcHandle.obtainMessage(request.getRequest()));
                break;
            default:
                loge("not support capability: " + request.getCapability());
                break;
        }
    }

    public void notifyMdRestart(){
        logd("notifyMdRestart");
        Handler imsHandle = mImsDataConnection.getHandler();
        imsHandle.sendMessage(imsHandle.obtainMessage(EVENT_MD_RESTART));
        Handler emcHandle = mEmcDataConnection.getHandler();
        emcHandle.sendMessage(emcHandle.obtainMessage(EVENT_MD_RESTART));
    }

    public void notifyClearCodesEvent(int cause , int capability){
        /* For CC33 UI prompt support
           This function will be invoked when IMS/ECC PDN activation was
           rejected with cause 33/29. Check mPhoneId to distinguish
           the event come from SIM1 or SIM2.
           cause : 33 or 29
           capability : IMS PDN (NetworkCapabilities.NET_CAPABILITY_IMS) or
                        Emergency PDN (NetworkCapabilities.NET_CAPABILITY_EIMS)
        */
        logd("notifyClearCodesEvent,cause= " + cause + " capability= " + capability);
    }

    public void onSubscriptionsChanged() {
        //update subId
        int newSubId = SubscriptionManagerHelper.getSubIdUsingPhoneId(mPhoneId);
        if(mSubId == newSubId) {
            return;
        }
        logd("onSubscriptionsChanged: subId: " + mSubId + ", newSubId: " + newSubId);
        mSubId = newSubId;

        mImsDataConnection.onSubscriptionsChanged();
        mEmcDataConnection.onSubscriptionsChanged();
    }

    private boolean earlyConfirmReqNetworkToMd() {
        // M: ALPS03861864, do the confirm procedure after setup IMS pdn
        // To pass OP07 testcases
        if ("OP07".equals(SystemProperties.get("persist.vendor.operator.optr", "")))  {
            return false;
        }
        // M: Common behavior, response confirm to MD right after receving notify
        // to enhance performance of VoLTE registration
        // Also match to the solution of ALPS03902669 for passing OP12 testcases
        return true;
    }

    public void logd(String s) {
        Rlog.d(TAG, s);
    }

    public void logi(String s) {
        Rlog.i(TAG, s);
    }

    public void loge(String s) {
        Rlog.e(TAG, s);
    }

    public class DataConnection extends StateMachine {

        private String TAG = "DC-";
        private int mCapability;
        private ConnectivityManager mConnectivityManager;
        private SparseArray<ImsBearerRequest> mImsNetworkRequests =
                new SparseArray<ImsBearerRequest>();
        private DataConnection mConn = this;

        private NetworkAvailableCallback mAvailableListener;
        private NetworkLostCallback mLostListener;

        private String mPdnSatate = "DefaultState";
        private DefaultState mDefaultState = new DefaultState();
        private InactiveState mInactiveState = new InactiveState();
        private ActivatingState mActivatingState = new ActivatingState();
        private ActiveState mActiveState = new ActiveState();
        private DisconnectingState mDisconnectingState = new DisconnectingState();

        /// Status Code {
        private static final int STATUS_SUCCESS = 0;
        private static final int STATUS_ABORT = 1;
        ///}

        public DataConnection(String name, Handler mHandler, int capability) {
            super(name, mHandler);
            mCapability = capability;
            mConnectivityManager = (ConnectivityManager)
                mContext.getSystemService(Context.CONNECTIVITY_SERVICE);

            mAvailableListener = new NetworkAvailableCallback();
            mLostListener = new NetworkLostCallback();

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
                mPdnSatate = "DefaultState";
                logd("enter");
            }

            @Override
            public void exit() {
                logd("exit");
            }

            @Override
            public boolean processMessage(Message msg) {
                boolean retVal = HANDLED;
                switch (msg.what) {
                    case EVENT_CONNECT:
                    case EVENT_CONNECT_DONE:
                    case EVENT_DISCONNECT:
                    case EVENT_DISCONNECT_DONE:
                    case EVENT_SUBSCRIPTIONS_CHANGED:
                    case EVENT_MD_RESTART:
                        break;
                }

                return retVal;
            }
        }

        private class InactiveState extends State {
            @Override
            public void enter() {
                mPdnSatate = "InactiveState";
                refreshNetworkLostListener();
                logd(" enter");
            }

            @Override
            public void exit() {
                logd("exit");
            }

            @Override
            public boolean processMessage(Message msg) {
                boolean retVal;
                logd("msg=" + msgToString(msg.what));
                switch (msg.what) {
                    case EVENT_CONNECT:
                    case EVENT_SUBSCRIPTIONS_CHANGED:
                        if(requestNetwork()){
                            if(earlyConfirmReqNetworkToMd()) {
                                confirmRequestNetworkToMd(EVENT_CONNECT, STATUS_SUCCESS);
                            }
                            transitionTo(mActivatingState);
                        }
                        retVal = HANDLED;
                        break;
                    case EVENT_CONNECT_DONE:
                        transitionTo(mActiveState);
                        retVal = HANDLED;
                        break;
                    case EVENT_DISCONNECT:
                        // M: ALPS03364695 adding abort pdn logic at InactiveState
                        if (mImsNetworkRequests.get(EVENT_CONNECT) != null) {
                            logd("handle pdn abort requested");
                            confirmRequestNetworkToMd(EVENT_CONNECT, STATUS_ABORT);
                        }
                        releaseNetwork();
                        confirmReleaseNetworkToMd(EVENT_DISCONNECT);
                        retVal = HANDLED;
                        break;
                    case EVENT_MD_RESTART:
                        mConn.clear();
                        releaseNetwork();
                        retVal = HANDLED;
                        break;
                    default:
                        loge("not handle the messag " + msgToString(msg.what));
                        retVal = NOT_HANDLED;
                        break;
                }

                return retVal;
            }
        }

        private class ActivatingState extends State {
            @Override
            public void enter() {
                mPdnSatate = "ActivatingState";
                logd("enter");
            }

            @Override
            public void exit() {
                logd("exit");
            }

            @Override
            public boolean processMessage(Message msg) {
                boolean retVal;
                logd("msg=" + msgToString(msg.what));
                switch (msg.what) {
                    case EVENT_CONNECT_DONE:
                        if(!earlyConfirmReqNetworkToMd()) {
                            confirmRequestNetworkToMd(EVENT_CONNECT, STATUS_SUCCESS);
                        }
                        transitionTo(mActiveState);
                        retVal = HANDLED;
                        break;
                    case EVENT_DISCONNECT:
                        confirmRequestNetworkToMd(EVENT_CONNECT, STATUS_ABORT);
                        deferMessage(msg);
                        transitionTo(mInactiveState);
                        retVal = HANDLED;
                        break;
                    case EVENT_MD_RESTART:
                        mConn.clear();
                        releaseNetwork();
                        transitionTo(mInactiveState);
                        retVal = HANDLED;
                        break;
                    default:
                        loge("not handle the message " + msgToString(msg.what));
                        retVal = NOT_HANDLED;
                        break;
                }
                return retVal;
            }
        }

        private class ActiveState extends State {
            @Override
            public void enter() {
                mPdnSatate = "ActiveState";
                logd("enter");
            }

            @Override
            public void exit() {
                logd("exit");
            }

            @Override
            public boolean processMessage(Message msg) {
                boolean retVal;
                logd("msg=" + msgToString(msg.what));
                switch (msg.what) {
                    case EVENT_DISCONNECT:
                        releaseNetwork();
                        transitionTo(mDisconnectingState);
                        retVal = HANDLED;
                        break;
                    case EVENT_DISCONNECT_DONE:
                        transitionTo(mInactiveState);
                        retVal = HANDLED;
                        break;
                    case EVENT_MD_RESTART:
                        mConn.clear();
                        releaseNetwork();
                        transitionTo(mInactiveState);
                        retVal = HANDLED;
                        break;

                    default:
                        loge("not handle the message " + msgToString(msg.what));
                        retVal = NOT_HANDLED;
                        break;
                }

                return retVal;
            }
        }

        private class DisconnectingState extends State {
            @Override
            public void enter() {
                mPdnSatate = "DisconnectingState";
                logd("enter");
            }

            @Override
            public void exit() {
                logd("exit");
            }

            @Override
            public boolean processMessage(Message msg) {
                boolean retVal;
                logd("msg=" + msgToString(msg.what));
                switch (msg.what) {
                    case EVENT_DISCONNECT_DONE:
                        confirmReleaseNetworkToMd(EVENT_DISCONNECT);
                        transitionTo(mInactiveState);
                        retVal = HANDLED;
                        break;
                    case EVENT_MD_RESTART:
                        mConn.clear();
                        transitionTo(mInactiveState);
                        retVal = HANDLED;
                        break;
                    default:
                        loge("not handle the message " + msgToString(msg.what));
                        retVal = NOT_HANDLED;
                        break;
                }

                return retVal;
            }
        }

        private boolean requestNetwork() {
            logd("requestNetwork");

            if(mSubId < 0) {
                if (mCapability != NetworkCapabilities.NET_CAPABILITY_EIMS) {
                    loge("inValid subId: " + mSubId);
                    return false;
                }
            }

            ImsBearerRequest n = mImsNetworkRequests.get(EVENT_CONNECT);
            if(n == null) {
                loge("ImsBearerRequest is NULL");
                return false;
            }

            Builder builder = new NetworkRequest.Builder();
            builder.addCapability(mCapability);
            builder.addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR);
            builder.setNetworkSpecifier(String.valueOf(mSubId));
            NetworkRequest nwRequest = builder.build();

            logd("start requestNetwork for " + getName());
            mAvailableListener.setNetworkCallbackRegistered(true);
            mConnectivityManager.requestNetwork(nwRequest, mAvailableListener);

            return true;
        }

        private void refreshNetworkLostListener() {
            logd("refreshNetworkLostListener");

            if ((mPdnSatate.equals("ActivatingState")) ||
                    (mPdnSatate.equals("ActiveState")) ||
                    (mPdnSatate.equals("DisconnectingState"))) {
               loge("inValid state: " + mPdnSatate);
               return;
            }

            try {
                mConnectivityManager.unregisterNetworkCallback(mLostListener);
            } catch (IllegalArgumentException ex) {
                loge("cb already has been released!!");
            }

            Builder builder = new NetworkRequest.Builder();
            builder.addCapability(mCapability);
            builder.addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR);
            builder.setNetworkSpecifier(String.valueOf(mSubId));
            NetworkRequest nwRequest = builder.build();
            mConnectivityManager.registerNetworkCallback(nwRequest, mLostListener);
        }

        private void confirmRequestNetworkToMd(int reqId, int status) {
            logd("confirmRequestNetworkToMd");
            ImsBearerRequest n = mImsNetworkRequests.get(reqId);
            for (int i = 0; i < mImsNetworkRequests.size(); i++) {
                logd("found Req: " + mImsNetworkRequests.valueAt(i));
            }
            if(n != null) {
                mImsNetworkRequests.remove(n.getRequest());
                mDataTracker.responseBearerConfirm(n.getRequest(), n.getAid(), n.getAction(), status, mPhoneId);
            }
        }

        private void releaseNetwork() {
            logd("releaseNetwork");

            try {
                mConnectivityManager.unregisterNetworkCallback(mAvailableListener);
                mAvailableListener.setNetworkCallbackRegistered(false);
            } catch (IllegalArgumentException ex) {
                loge("cb already has been released!!");
            }
        }

        private void confirmReleaseNetworkToMd(int reqId) {
            logd("confirmReleaseNetworkToMd");
            ImsBearerRequest n = mImsNetworkRequests.get(reqId);
            for (int i = 0; i < mImsNetworkRequests.size(); i++) {
                logd("found Req: " + mImsNetworkRequests.valueAt(i));
            }
            if(n != null) {
                mImsNetworkRequests.remove(n.getRequest());
                mDataTracker.responseBearerConfirm(
                    n.getRequest(), n.getAid(), n.getAction(), STATUS_SUCCESS, mPhoneId);
            }
        }

        public void putRequest(ImsBearerRequest request) {
            ImsBearerRequest n = mImsNetworkRequests.get(request.getRequest());
            if(n == null) {
                mImsNetworkRequests.put(request.getRequest(), request);
            } else {
                loge("request already exist: " + request);
            }
        }

        public void clear() {
            logd("clear");
            mImsNetworkRequests.clear();
        }

        private class NetworkAvailableCallback extends NetworkCallback {
            private boolean mNetworkCallbackRegistered = false;

            public NetworkAvailableCallback() {
            }

            @Override
            public void onAvailable(Network network) {
                NetworkInfo netInfo = mConnectivityManager.getNetworkInfo(network);
                logd("onAvailable: networInfo: " + netInfo + " mNetworkCallbackRegistered "
                        + mNetworkCallbackRegistered);
                if (mNetworkCallbackRegistered) {
                    mConn.sendMessage(mConn.obtainMessage(EVENT_CONNECT_DONE));
                }
            }

            public void setNetworkCallbackRegistered(boolean regState) {
                logd("setNetworkCallbackRegistered regState= " + regState);
                mNetworkCallbackRegistered = regState;
            }
        }

        private class NetworkLostCallback extends NetworkCallback {

            public NetworkLostCallback() {
            }

            @Override
            public void onLost(Network network) {
                NetworkInfo netInfo = mConnectivityManager.getNetworkInfo(network);
                logd("onLost: networInfo: " + netInfo);
                mConn.sendMessage(mConn.obtainMessage(EVENT_DISCONNECT_DONE));
            }
        }

        public void onSubscriptionsChanged() {
            logd("onSubscriptionsChanged");
            refreshNetworkLostListener();
            ImsBearerRequest n = mImsNetworkRequests.get(EVENT_CONNECT);
            if(n != null) {
                DataConnection.this.sendMessage(
                    DataConnection.this.obtainMessage(EVENT_SUBSCRIPTIONS_CHANGED));
            }
        }

        private String msgToString(int msg) {
            switch(msg) {
                case EVENT_CONNECT:
                    return "EVENT_CONNECT";
                case EVENT_CONNECT_DONE:
                    return "EVENT_CONNECT_DONE";
                case EVENT_DISCONNECT:
                    return "EVENT_DISCONNECT";
                case EVENT_DISCONNECT_DONE:
                    return "EVENT_DISCONNECT_DONE";
                case EVENT_SUBSCRIPTIONS_CHANGED:
                    return "EVENT_SUBSCRIPTIONS_CHANGED";
                case EVENT_MD_RESTART:
                    return "EVENT_MD_RESTART";
                default: return "<unknown request>";
            }
        }

        public void logd(String s) {
            Rlog.d(TAG + getName() +"[" + mPhoneId + "]", mPdnSatate +": " + s);
        }

        public void logi(String s) {
            Rlog.i(TAG + getName() +"[" + mPhoneId + "]", mPdnSatate +": " + s);
        }

        public void loge(String s) {
            Rlog.e(TAG + getName() +"[" + mPhoneId + "]", mPdnSatate +": " + s);
        }
    }

    public static class ImsBearerRequest implements Serializable {

        private static final long serialVersionUID = -5053412967314724078L;

        private int     mAid;
        private int     mAction;
        private int     mPhoneId;
        private int     mRequest;
        private String  mCapability;

        public ImsBearerRequest(int aid, int action, int phoneId, int request, String capability) {
            mAid = aid;
            mAction = action;
            mPhoneId = phoneId;
            mRequest = request;
            mCapability = capability;
        }

        public int getAid() {
            return mAid;
        }

        public int getAction() {
            return mAction;
        }

        public int getPhoneId() {
            return mPhoneId;
        }

        public int getRequest() {
            return mRequest;
        }

        public String getCapability() {
            return mCapability;
        }

        public String toString() {
            StringBuilder builder = new StringBuilder();
            builder.append("aid: " + mAid);
            builder.append(" action: " + mAction);
            builder.append(" phoneId: " + mPhoneId);
            switch(mRequest){
                case EVENT_CONNECT:
                    builder.append(" Request: EVENT_CONNECT");
                    break;
                case EVENT_DISCONNECT:
                    builder.append(" Request: EVENT_DISCONNECT");
                    break;
            }
            builder.append(" Capability: " + mCapability + " }");
            return builder.toString();
        }
    }
}
