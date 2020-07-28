package com.mediatek.ims.internal;

import android.content.Context;
import android.telephony.Rlog;
import android.telephony.TelephonyManager;
import android.telephony.SubscriptionManager;
import android.telephony.SubscriptionManager.OnSubscriptionsChangedListener;
import android.os.AsyncResult;
import android.os.Looper;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.net.NetworkCapabilities;
import android.net.NetworkInfo;
import android.os.SystemProperties;

import com.mediatek.ims.ImsEventDispatcher;
import com.mediatek.ims.ImsAdapter.VaEvent;
import com.mediatek.ims.ImsAdapter.VaSocketIO;
import com.mediatek.ims.ril.ImsRILAdapter;
import com.mediatek.ims.ril.ImsCommandsInterface;
import com.mediatek.ims.internal.ImsDataSynchronizer.ImsBearerRequest;
import com.android.internal.telephony.PhoneConstants;

import java.util.Arrays;

public class ImsDataTracker implements ImsEventDispatcher.VaEventDispatcher {

    private String TAG = ImsDataTracker.class.getSimpleName();
    //private boolean mIsMdAutoActivate = false;
    private Context mContext;
    private int mPhoneNum;
    private MdCapability mMdCapability;
    public enum MdCapability {
        LEGACY,
        AUTOSETUPIMS
    }

    //92MD
    private DataDispatcher[] mDispatchers;
    private VaSocketIO mSocket;

    public ImsDataTracker(Context context, VaSocketIO IO) {
        mPhoneNum = TelephonyManager.getDefault().getPhoneCount();
        mDispatchers = new DataDispatcher[mPhoneNum];
        for(int i = 0; i < mPhoneNum; i++){
            mDispatchers[i] = new DataDispatcher(context, this, i);
        }
        mSocket = IO;
        mMdCapability = MdCapability.LEGACY;
        SubscriptionManager.from(context).addOnSubscriptionsChangedListener(
                mOnSubscriptionsChangedListener);
    }

    public void enableRequest(int phoneId) {
        logd("receive enableRequest on phone: " + phoneId);
        mDispatchers[phoneId].enableRequest(phoneId);
    }

    public void disableRequest(int phoneId) {
        logd("receive disableRequest on phone: " + phoneId);
        mDispatchers[phoneId].disableRequest(phoneId);
    }

    /**
     * Handle Data related event from IMCB.
     *
     * @param VaEvent event from IMCB.
     */
    public void vaEventCallback(VaEvent event) {
        logd("send event" + event.getRequestID() + " to phone " + event.getPhoneId());
        mDispatchers[event.getPhoneId()].vaEventCallback(event);
    }

    public void sendVaEvent(VaEvent event) {
        mSocket.writeEvent(event);
    }

    //93MD
    private ImsDataSynchronizer[] mSynchronizers;
    private ImsCommandsInterface [] mImsRILAdapters = null;
    private int[] mModemImsPdnState;
    private int[] mModemEmergencyPdnState;
    private final Object mPdnStateLock = new Object();


    /* Customized IMS and Emergency(EIMS) PDN notify/process rule support
       0 : DO NOT notify both IMS/EIMS PDN and do NOT request/releaseNetwork in FWK
       1 or Unset : Notify both IMS/EIMS PDN and do request/releaseNetwork in FWK (Default rule)
       2 : Notify both IMS/EIMS PDN but do NOT request/releaseNetwork in FWK
       3 : DO notify IMS PDN and DO request/releaseNetwork in FWK
           DO NOT notify EIMS PDN and DO NOT request/releaseNetwork in FWK*/

    private int mIsBearerNotify = 1;

    public ImsDataTracker(Context context, ImsCommandsInterface [] adapters) {
        mPhoneNum = TelephonyManager.getDefault().getPhoneCount();
        mSynchronizers = new ImsDataSynchronizer[mPhoneNum];
        mModemImsPdnState = new int[mPhoneNum];
        mModemEmergencyPdnState = new int[mPhoneNum];
        mContext = context;
        mImsRILAdapters = adapters;

        getImsPdnNotifyRule();

        for(int i = 0; i < mPhoneNum; i++){
            mSynchronizers[i] = new ImsDataSynchronizer(context, this, i);
            mImsRILAdapters[i].registerForBearerState(
                    mdHander, ImsDataSynchronizer.EVENT_BEARER_STATE_CHANGED, null);
            mImsRILAdapters[i].registerForBearerInit(
                    mdHander, ImsDataSynchronizer.EVENT_MD_RESTART, null);
            mImsRILAdapters[i].registerForImsDataInfoNotify(
                    mdHander, ImsDataSynchronizer.EVENT_IMS_DATA_INFO, null);
            mModemImsPdnState[i] = NetworkInfo.State.UNKNOWN.ordinal();
            mModemEmergencyPdnState[i] = NetworkInfo.State.UNKNOWN.ordinal();
            setImsBearerNotification(i, mIsBearerNotify);
        }
        mMdCapability = MdCapability.AUTOSETUPIMS;
        SubscriptionManager.from(mContext).addOnSubscriptionsChangedListener(
                mOnSubscriptionsChangedListener);
    }

    private Handler mdHander = new Handler() {
        @Override
        synchronized public void handleMessage(Message msg) {
            switch (msg.what) {
                case ImsDataSynchronizer.EVENT_BEARER_STATE_CHANGED:
                    onImsBearerChanged((AsyncResult) msg.obj);
                    break;
                case ImsDataSynchronizer.EVENT_MD_RESTART:
                    onMdRestart((AsyncResult) msg.obj);
                    break;
                case ImsDataSynchronizer.EVENT_IMS_DATA_INFO:
                    onImsDataInfo((AsyncResult) msg.obj);
                    break;
                default:
                    loge("not handle the message: " + msg.what);
                    break;
            }
        }
    };

    /* Customized IMS and Emergency(EIMS) PDN notify/process rule support
       0 : DO NOT notify both IMS/EIMS PDN and do NOT request/releaseNetwork in FWK
       1 or Unset : Notify both IMS/EIMS PDN and do request/releaseNetwork in FWK (Default rule)
       2 : Notify both IMS/EIMS PDN but do NOT request/releaseNetwork in FWK
       3 : DO notify IMS PDN and DO request/releaseNetwork in FWK
           DO NOT notify EIMS PDN and DO NOT request/releaseNetwork in FWK*/
    private void getImsPdnNotifyRule(){
        mIsBearerNotify = SystemProperties.getInt("persist.vendor.radio.ims.pdn.notify", 1);
        logd("mIsBearerNotify rule set to " + mIsBearerNotify);
    }

    private void onImsBearerChanged(AsyncResult ar) {
        logd("onImsBearerChanged");
        String[] bearerInfo = (String[]) ar.result;
        if(bearerInfo != null) {
            if(bearerInfo.length == 4) {
                logd(Arrays.toString(bearerInfo));
                int phoneId = Integer.parseInt(bearerInfo[0]);
                int aid = Integer.parseInt(bearerInfo[1]);
                int action = Integer.parseInt(bearerInfo[2]);
                String capability = bearerInfo[3];

                int event = -1;
                if (action == ImsDataSynchronizer.ACTION_ACTIVATION) {
                    event = ImsDataSynchronizer.EVENT_CONNECT;
                } else if (action == ImsDataSynchronizer.ACTION_DEACTIVATION) {
                    event = ImsDataSynchronizer.EVENT_DISCONNECT;
                } else {
                    loge("unknown action: " + action);
                }

                if (event >= 0) {
                    updateModemPdnState(phoneId, capability, event);
                    if((mIsBearerNotify == 1) || (mIsBearerNotify == 3)) {
                        mSynchronizers[phoneId].notifyMdRequest(
                                new ImsBearerRequest(aid, action, phoneId, event, capability));
                    }
                }
            } else {
                loge("parameter format error: " + Arrays.toString(bearerInfo));
            }
        } else {
            loge("parameter is NULL");
        }
    }

    private void onMdRestart(AsyncResult ar) {
        logd("onMdRestart");
        int phoneId = ((int[]) ar.result)[0];
        logd("onMdRestart, reset phone = " + phoneId + " connection state");
        mSynchronizers[phoneId].notifyMdRestart();
        clearModemPdnState();
        setImsBearerNotification(phoneId, mIsBearerNotify);
    }

    private void onImsDataInfo(AsyncResult ar) {
        String[] bearerInfo = (String[]) ar.result;
        if(bearerInfo != null) {
            if(bearerInfo.length == 4) {
                logd("onImsDataInfo: " + Arrays.toString(bearerInfo));
                int cap;
                int phoneId = Integer.parseInt(bearerInfo[0]);
                String capability = bearerInfo[1];
                String event = bearerInfo[2];

                // Check this IMS data info is for Eemergency or IMS PDN
                if (capability.equals(PhoneConstants.APN_TYPE_EMERGENCY)) {
                    cap = NetworkCapabilities.NET_CAPABILITY_EIMS;
                } else {
                    cap = NetworkCapabilities.NET_CAPABILITY_IMS;
                }

                // extra info for event "ClearCodes" is the ClearCodes cause value
                if("ClearCodes".equals(event)){
                    int cause = Integer.parseInt(bearerInfo[3]);
                    mSynchronizers[phoneId].notifyClearCodesEvent(cause,cap);
                }
            } else {
                loge("parameter format error: " + Arrays.toString(bearerInfo));
            }
        } else {
            loge("parameter is NULL");
        }
    }

    public void responseBearerConfirm(int event, int aid, int action, int status, int phoneId) {
        logd("send to MD, aid:" + aid + ", action:" + action + ", status:" + status + ", phoneId:" + phoneId);
        switch (event) {
            case ImsDataSynchronizer.EVENT_CONNECT:
                mImsRILAdapters[phoneId].responseBearerStateConfirm(aid, action, status, null);
                break;
            case ImsDataSynchronizer.EVENT_DISCONNECT:
                mImsRILAdapters[phoneId].responseBearerStateConfirm(aid, action, status, null);
                break;
        }
    }

    private void setImsBearerNotification(int phoneId, int enable) {
        logd("setImsBearerNotification enable: " +enable );
        mImsRILAdapters[phoneId].setImsBearerNotification(enable, null);
    }

    private final OnSubscriptionsChangedListener mOnSubscriptionsChangedListener =
            new OnSubscriptionsChangedListener() {
        @Override
        public void onSubscriptionsChanged() {
            logd("onSubscriptionsChanged");
            switch(mMdCapability) {
                case LEGACY:
                    for(int i = 0; i < mPhoneNum; i++) {
                        mDispatchers[i].onSubscriptionsChanged();
                    }
                    break;
                case AUTOSETUPIMS:
                    for(int i = 0; i < mPhoneNum; i++) {
                        mSynchronizers[i].onSubscriptionsChanged();
                    }
                    break;
            }
        }
    };

    public int[] getImsNetworkState(int capability) {
        logd("capability/mPhoneNum/imsPdnState/emergencyPdnState are : "
            + capability + "/"
            + mPhoneNum + "/"
            + Arrays.toString(mModemImsPdnState)
            + Arrays.toString(mModemEmergencyPdnState));
        if (capability == NetworkCapabilities.NET_CAPABILITY_IMS) {
            synchronized (mPdnStateLock) {
                return mModemImsPdnState;
            }
        } else if (capability == NetworkCapabilities.NET_CAPABILITY_EIMS) {
            synchronized (mPdnStateLock) {
                return mModemEmergencyPdnState;
            }
        } else {
            int[] pdnState = new int[mPhoneNum];
            Arrays.fill(pdnState, NetworkInfo.State.UNKNOWN.ordinal());
            loge("getImsNetworkState failed becase of invalid capability : " + capability);
            return pdnState;
        }
    }

    private void updateModemPdnState(int phoneId, String capability, int event) {
        if (event == ImsDataSynchronizer.EVENT_CONNECT) {
            if (capability.equals(PhoneConstants.APN_TYPE_IMS)) {
                mModemImsPdnState[phoneId] = NetworkInfo.State.CONNECTED.ordinal();
            } else if (capability.equals(PhoneConstants.APN_TYPE_EMERGENCY)) {
                mModemEmergencyPdnState[phoneId] = NetworkInfo.State.CONNECTED.ordinal();
            } else {
                loge("Not handle the capability: " + capability);
            }
        } else if (event == ImsDataSynchronizer.EVENT_DISCONNECT) {
            if (capability.equals(PhoneConstants.APN_TYPE_IMS)) {
                mModemImsPdnState[phoneId] = NetworkInfo.State.DISCONNECTED.ordinal();
            } else if (capability.equals(PhoneConstants.APN_TYPE_EMERGENCY)) {
                mModemEmergencyPdnState[phoneId] = NetworkInfo.State.DISCONNECTED.ordinal();
            } else {
                loge("Not handle the capability: " + capability);
            }
        } else {
            loge("Not handle the event: " + event);
        }
    }

    private void clearModemPdnState() {
        synchronized (mPdnStateLock) {
            for (int i = 0; i < mPhoneNum; ++i) {
                mModemImsPdnState[i] = NetworkInfo.State.UNKNOWN.ordinal();
                mModemEmergencyPdnState[i] = NetworkInfo.State.UNKNOWN.ordinal();
            }
        }
    }

    private void logd(String s) {
        Rlog.d(TAG, s);
    }

    private void logi(String s) {
        Rlog.i(TAG, s);
    }

    private void loge(String s) {
        Rlog.e(TAG, s);
    }
}
