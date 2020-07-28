package com.mediatek.apmonitor.proxy;

import com.mediatek.apmonitor.ApmServiceUtils;
import vendor.mediatek.hardware.apmonitor.V2_0.apmKpiHeader_t;
import vendor.mediatek.hardware.apmonitor.V2_0.apmResult_e;
import vendor.mediatek.hardware.apmonitor.V2_0.IApmControlCallback;
import vendor.mediatek.hardware.apmonitor.V2_0.IApmControlCallback.Stub;
import vendor.mediatek.hardware.apmonitor.V2_0.IApmService;

import android.os.HwBinder;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;
import java.util.ArrayList;
import java.util.NoSuchElementException;

public class ApmSession {
    private static final String TAG = "APM-SessionJ";
    private static final String APM_SERVICE_NAME = "apm_hidl_service";
    private static final int INVALID_SESSION_ID = 0;
    private int mSID = INVALID_SESSION_ID;
    private long mSessionParam;
    private ArrayList<Short> mSubscribedMessages = new ArrayList<Short>();
    private ApmControlcallback mApmControlCallback;
    private ApmDeathRecipient mApmDeathCallback;
    private boolean mApmSessionReady = false;
    private IApmSessionCallback mSessionCallback = null;

    // APM HIDL service connection.
    private IApmService mApmConnectionBase_V2_0 = null;

    public ApmSession() {
        mApmControlCallback = new ApmControlcallback(this);
        mApmDeathCallback = new ApmDeathRecipient(this);
    }

    public int getApmSessionId() {
        return isApmSessionReady()? mSID: INVALID_SESSION_ID;
    }

    public long getApmSessionParam() {
        if (!isApmSessionReady()) {
            Log.e(TAG, "[" + mSID + "]getApmSessionParam() APM session not ready!");
            return 0;
        }
        return mSessionParam;
    }

    public boolean isApmSessionReady() {
        return mApmSessionReady;
    }

    public void registControlCallback(IApmSessionCallback callback) {
        mSessionCallback = callback;
    }

    public boolean shouldSubmitKpi(Short msgId) {
        if (!tryCreateApmSession(true)) {
            return false;
        }
        return mSubscribedMessages.contains(msgId);
    }

    public boolean apmSubmitKpiST(Short msgId, byte simId, long timestampMs, int len, byte[] payload) {
        Log.d(TAG, "[" + mSID + "]apmSubmitKpiST(" +
                msgId + "), simId = " + simId + ", timestamp = " + timestampMs);
        if (!tryCreateApmSession(true)) {
            Log.e(TAG, "[" + mSID + "]tryCreateApmSession() false");
            return false;
        }

        // Submit the KPI only when it being subscribed.
        if (!shouldSubmitKpi(msgId)) {
            Log.e(TAG, "[" + mSID + "]shouldSubmitKpi(" + msgId + ") false");
            return false;
        }

        apmKpiHeader_t kpiHeader = new apmKpiHeader_t();
        kpiHeader.msgId = msgId;
        kpiHeader.simId = simId;
        kpiHeader.timestampMs = timestampMs;

        // Covert byte[] to ArrayList<Byte>
        ArrayList<Byte> raw = new ArrayList<Byte>();
        for (byte b: payload) {
            raw.add(b);
        }

        int result = apmResult_e.APM_FAILED;
        try {
            mApmConnectionBase_V2_0.submitRawEvent(
                    mSID, kpiHeader, len, raw);
        } catch (RemoteException e) {
            e.printStackTrace();
            return false;
        }
        return (result == apmResult_e.APM_SUCCESS)? true : false;
    }

    public boolean tryCreateApmSession(boolean bCheckApmActive) {
        boolean bSessionCreated = false;
        if (mSessionCallback == null) {
            Log.e(TAG, "[" + mSID + "]Create APM session failed! call registControlCallback() first");
            return false;
        }
        try {
            if (isApmSessionReady()) {
                // Need to double confirm APM was activated.
                if (!ApmServiceUtils.isApmActive()) {
                    Log.w(TAG, "[" + mSID + "]APM service is not activated, reset APM client.");
                    // APM is inactivated, close session.
                    mSubscribedMessages.clear();
                    mApmSessionReady = false;
                    bSessionCreated = true;
                    mSessionParam = 0;
                    dumpSubscribedMsgList("APM inactivated");
                    mSessionCallback.onKpiSubscriptionStateChanged(mSubscribedMessages);
                    return false;
                }
                return true;
            }

            if (bCheckApmActive && !ApmServiceUtils.isApmActive()) {
                Log.d(TAG, "[" + mSID + "]APM service is not activated");
                return false;
            }

            if (!tryGetApmService()) {
                Log.w(TAG, "[" + mSID + "]APM service is not ready!");
                return false;
            }

            // Create APM session
            mApmConnectionBase_V2_0.createSession(
                new IApmService.createSessionCallback() {
                    @Override
                    public void onValues(int result, int sessionId, long param) {
                        if (result != apmResult_e.APM_SUCCESS) {
                            Log.e(TAG, "[" + mSID + "]Create APM session failed! result = " + result);
                            return;
                        }
                        if (sessionId <= INVALID_SESSION_ID) {
                            Log.e(TAG, "[" + mSID + "]Create APM session with invalid SID = " + sessionId);
                            return;
                        }
                        mSID = sessionId;
                        mSessionParam = param;
                    }
                }
            );
            bSessionCreated = true;

            // Get subscribed messages
            mApmConnectionBase_V2_0.getSubscribedMessages(mSID,
                new IApmService.getSubscribedMessagesCallback() {
                    @Override
                    public void onValues(int result, ArrayList<Short> subscribedMessages) {
                        if (result != apmResult_e.APM_SUCCESS) {
                            Log.e(TAG, "[" + mSID + "]getSubscribedMessages() failed with SID = " +
                                    mSID + ", result = " + result);
                            return;
                        }
                        for (Short id : subscribedMessages) {
                            mSubscribedMessages.add(id);
                        }
                        dumpSubscribedMsgList("getSubscribedMessages");
                        mSessionCallback.onKpiSubscriptionStateChanged(mSubscribedMessages);
                    }
                }
            );
            // Subscribe APM control callback
            mApmConnectionBase_V2_0.registerControlCallback(mSID, mApmControlCallback);

            // Death notify for DMC HIDL server
            mApmConnectionBase_V2_0.linkToDeath(mApmDeathCallback, mSID);
            mApmSessionReady = true;
        } catch (RemoteException e) {
            Log.e(TAG, "[" + mSID + "]Create APM session failed! " + e.toString());
        } finally {
            if (!isApmSessionReady()) {
                Log.e(TAG, "[" + mSID + "]Create APM session failed!");
                if (bSessionCreated) {
                    try {
                        Log.e(TAG, "[" + mSID + "]Reset APM client");
                        mApmConnectionBase_V2_0.closeSession(mSID);
                    } catch (RemoteException e) {
                        Log.e(TAG, e.toString());
                    }
                }
                mSID = INVALID_SESSION_ID;
                mApmConnectionBase_V2_0 = null;
                mSessionParam = 0;
                return false;
            }
        }
        Log.d(TAG, "[" + mSID + "]Create APM session success! param = " + mSessionParam);
        return true;
    }

    private boolean tryGetApmService() {
        try {
            mApmConnectionBase_V2_0 = IApmService.getService(APM_SERVICE_NAME, true);
        } catch (RemoteException e) {
            mApmConnectionBase_V2_0 = null;
            Log.e(TAG, "[" + mSID + "]tryGetApmService() failed! " + e.toString());
            return false;
        } catch (NoSuchElementException e) {
            mApmConnectionBase_V2_0 = null;
            Log.e(TAG, "[" + mSID + "]tryGetApmService() failed! " + e.toString());
            return false;
        }
        return true;
    }

    private void dumpSubscribedMsgList(String cause) {
        if (mSubscribedMessages.size() == 0) {
            Log.d(TAG, "[" + mSID + "]dumpSubscribedMsgList(" + cause + "): No APM KPI being subscribed!");
        } else {
            StringBuilder out = new StringBuilder();
            out.append("[" + mSID + "]dumpSubscribedMsgList(" + cause + "): {");
            int i = 0;
            for (Short id : mSubscribedMessages) {
                if (i == 0) {
                    out.append(id);
                } else {
                    out.append(", " + id);
                }
                i++;
            }
            out.append("}");
            Log.d(TAG, out.toString());
        }
    }

    public class ApmControlcallback extends IApmControlCallback.Stub {
        private ApmSession mSession;

        public ApmControlcallback(ApmSession client) {
            mSession = client;
        }

        @Override
        public int onUpdateKpiSubscriptionState(ArrayList<Short> subscribedMessages) {
            mSession.mSubscribedMessages.clear();
            for (Short id : subscribedMessages) {
                mSession.mSubscribedMessages.add(id);
            }
            dumpSubscribedMsgList("onUpdateKpiSubscriptionState");
            mSessionCallback.onKpiSubscriptionStateChanged(mSubscribedMessages);
            return apmResult_e.APM_SUCCESS;
        }

        @Override
        public int onQueryKpi(short msgId) {
            return apmResult_e.APM_SUCCESS;
        }
    }

    class ApmDeathRecipient implements HwBinder.DeathRecipient{
        private ApmSession mSession;

        public ApmDeathRecipient(ApmSession client) {
            mSession = client;
        }

        @Override
        public void serviceDied(long cookie) {
            Log.w(TAG, "[" + cookie + "]APM service died!");
            mSession.mSubscribedMessages.clear();
            mSession.mApmSessionReady = false;
            mSession.mApmConnectionBase_V2_0 = null;
            mSession.mSID = INVALID_SESSION_ID;
            mSession.mSessionParam = 0;
            dumpSubscribedMsgList("APM serviceDied");
            mSessionCallback.onKpiSubscriptionStateChanged(mSubscribedMessages);
        }
    }
}
