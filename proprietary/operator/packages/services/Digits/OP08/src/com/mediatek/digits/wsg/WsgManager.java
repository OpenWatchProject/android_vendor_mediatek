package com.mediatek.digits.wsg;

import android.content.Context;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;

public class WsgManager extends Handler {


    private static final String TAG = "WsgManager";
    private static final boolean DEBUG = true;

    // key: EVENT_KEY_MSISDN, value: msisdn
    public static final int EVENT_ADD_HUNT_GROUP_RESULT = 0;

    // key: EVENT_KEY_MSISDN, value: msisdn
    public static final int EVENT_REMOVE_HUNT_GROUP_RESULT = 1;

    // key: EVENT_KEY_FROM, value: from
    // key: EVENT_KEY_TO, value: to
    // key: EVENT_KEY_TRN, value: trn
    public static final int EVENT_GET_TRN_RESULT = 2;

    public static final String EVENT_KEY_TRN = "trn";
    public static final String EVENT_KEY_MSISDN = "msisdn";
    public static final String EVENT_KEY_FROM = "from";
    public static final String EVENT_KEY_TO = "to";

    public static final int EVENT_RESULT_SUCCEED = 0;
    public static final int EVENT_RESULT_FAIL = 1;

    private final WsgServerApi mApi;


    Context mContext;
    WsgListener mListener;

    /** Interface for callback to notify WSG event */
    public interface WsgListener {
        /**
         * callback when any events happened.
         */
        void onWsgEvent(int event, int result, Bundle bundle);
    }


    public WsgManager(Context context, Looper looper, WsgListener listener) {
        super(looper);

        mContext = context;
        mListener = listener;
        mApi = new WsgServerApi(mContext);
    }


    /**
     * Add line. It's an asynchronous API.
     *
     * @param msisdn the virtual line needs to add
     * @param nativeMsisdn the primary line
     * @param sit the service instance token

     */
    public void addHuntGroup(String msisdn, String nativeMsisdn, String sit) {
        mApi.createRequest()
            .addHuntGroup(msisdn, nativeMsisdn, sit)
            .execute(new WsgServerApi.Callback() {
                @Override
                public void callback(WsgServerApi.WsgResponse response) {
                // TODO Auto-generated method stub
                    int mResult =
                        (response.isHttpSuccess()) ? EVENT_RESULT_SUCCEED : EVENT_RESULT_FAIL;
                    String mMsisdn = response.getReqHttpUrl().pathSegments().get(4);
                    Bundle bundle = new Bundle();
                    bundle.putString(EVENT_KEY_MSISDN, mMsisdn);
                    mListener.onWsgEvent(EVENT_ADD_HUNT_GROUP_RESULT, mResult, bundle);
                    log("addHuntGroup statuscode:" + response.getRespStatusCode());
                    log("addHuntGroup msisdn:" + mMsisdn);
                    log("addHuntGroup result:" + ((response.isHttpSuccess())
                        ? "EVENT_RESULT_SUCCEED" : "EVENT_RESULT_FAIL"));
                }
            });
    }

    /**
     * Remove line. It's an asynchronous API.
     *
     * @param msisdn the virtual line needs to remove
     * @param nativeMsisdn the primary line
     * @param sit the service instance token
     */
    public void removeHuntGroup(String msisdn, String nativeMsisdn, String sit) {
        mApi.createRequest()
            .remHuntGroup(msisdn, nativeMsisdn, sit)
            .execute(new WsgServerApi.Callback() {
                @Override
                public void callback(WsgServerApi.WsgResponse response) {
                    int mResult =
                        (response.isHttpSuccess()) ? EVENT_RESULT_SUCCEED : EVENT_RESULT_FAIL;
                    String mMsisdn = response.getReqHttpUrl().pathSegments().get(4);
                    Bundle bundle = new Bundle();
                    bundle.putString(EVENT_KEY_MSISDN, mMsisdn);
                    mListener.onWsgEvent(EVENT_REMOVE_HUNT_GROUP_RESULT, mResult, bundle);
                    log("remHuntGroup statuscode:" + response.getRespStatusCode());
                    log("remHuntGroup msisdn:" + mMsisdn);
                    log("remHuntGroup result:" + ((response.isHttpSuccess())
                        ? "EVENT_RESULT_SUCCEED" : "EVENT_RESULT_FAIL"));
                }
            });
    }

    /**
     * Get TRN. It's an asynchronous API.
     *
     * @param fromMsisdn the from line
     * @param toMsisdn the to line
     * @param nativeMsisdn the primary line
     * @param sit the service instance token
     */
    public void getTrn(String fromMsisdn, String toMsisdn, String nativeMsisdn, String sit) {
        mApi.createRequest()
            .getTRN(fromMsisdn, toMsisdn, nativeMsisdn, sit)
            .execute(new WsgServerApi.Callback() {
                @Override
                public void callback(WsgServerApi.WsgResponse response) {
                    int mResult =
                        (response.isHttpSuccess()) ? EVENT_RESULT_SUCCEED : EVENT_RESULT_FAIL;
                    String mRequestBody = response.getReqBody();
                    String mFromMsisdn = mRequestBody.substring(
                        mRequestBody.indexOf("<assertedAddress>sip:")
                        + "<assertedAddress>sip:".length(),
                        mRequestBody.indexOf("</assertedAddress>"));
                    mFromMsisdn = mFromMsisdn.substring(0, mFromMsisdn.indexOf('@'));
                    String mToMsisdn = mRequestBody.substring(
                        mRequestBody.lastIndexOf("<participantAddress>sip:")
                        + "<participantAddress>sip:".length(),
                        mRequestBody.lastIndexOf("</participantAddress>"));
                    mToMsisdn = mToMsisdn.substring(0, mToMsisdn.indexOf('@'));
                    String mTrn = response.getRespHeaders().get("location");
                    if (mTrn != null) {
                        mTrn = mTrn.substring(mTrn.indexOf('+'));
                    } else {
                        mTrn = "";
                    }

                    Bundle bundle = new Bundle();
                    bundle.putString(EVENT_KEY_FROM, mFromMsisdn);
                    bundle.putString(EVENT_KEY_TO, mToMsisdn);
                    bundle.putString(EVENT_KEY_TRN, mTrn);
                    mListener.onWsgEvent(EVENT_GET_TRN_RESULT, mResult, bundle);
                    log("getTrn statuscode:" + response.getRespStatusCode());
                    log("getTrn FromMsisdn:" + mFromMsisdn);
                    log("getTrn ToMsisdn:" + mToMsisdn);
                    log("getTrn TRN:" + mTrn);
                    log("getTrn result:" + ((response.isHttpSuccess())
                        ? "EVENT_RESULT_SUCCEED" : "EVENT_RESULT_FAIL"));
                }
            });
    }

    private static void log(String s) {
        if (DEBUG) {
            Log.d(TAG, s);
        }
    }

    public static String eventToString(int event) {
        switch(event) {
            case EVENT_ADD_HUNT_GROUP_RESULT:
                return "EVENT_ADD_HUNT_GROUP_RESULT";
            case EVENT_REMOVE_HUNT_GROUP_RESULT:
                return "EVENT_REMOVE_HUNT_GROUP_RESULT";
            case EVENT_GET_TRN_RESULT:
                return "EVENT_GET_TRN_RESULT";
            default:
                return "UNKNOWN_EVENT(" + event + ")";
        }
    }
}

