package com.mediatek.entitlement.o2;

import android.content.Context;
import android.os.Build;
import android.os.Bundle;
import android.os.SystemProperties;
import android.provider.Settings;
import android.util.Log;

import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.TimeZone;
import java.text.DateFormat;
import java.text.SimpleDateFormat;

import org.json.JSONObject;
import org.json.JSONArray;
import org.json.JSONException;

class EntitlementHandling {
    private static final String TAG = "EntitlementHandling";
    private static final boolean DEBUG = true;
    private static final String PROP_FORCE_DEBUG_KEY = "persist.vendor.log.tel_dbg";
    private static final boolean ENG = "eng".equals(Build.TYPE);
    private static final boolean TELDBG = (SystemProperties.getInt(PROP_FORCE_DEBUG_KEY, 0) == 1);

    public interface Listener {
        /**
         * Notify state change.
         */
        void onStateChange(EntitlementHandling handling, int state, String service, int slotId);

        /**
         * Websheet callback.
         */
        void onWebsheetPost(String url, String serverData, int slotId);

        /**
         * To pass information discovered during the entitlement process.
         */
        void onInfo(Bundle info, int slotId);
    }

    private final List<Listener> mListeners = new ArrayList<>();

    private final EntitlementServerApi mApi;

    static final int STATE_INITIAL = 0;
    static final int STATE_ACTIVATING = 10;
    static final int STATE_ENTITLED = 20;
    static final int STATE_ENTITLED_FAIL = 30;
    static final int STATE_NOT_ENTITLED = 40;

    //private int mState = STATE_INITIAL;
    private static int mPollInterval = 0;

    private Context mContext;
    private int mSlotId;
    //private String mService;
    private ArrayList<String> mServiceList = new ArrayList<String>();
    private final Map<String, Integer> mStates = new HashMap<>();

    private static final String DEVICE_GROUP_MEDIATEK = "Mediatek";

    private String mMSISDN; // Get from device_activation.
    private String mServiceFingerprint; // Get from device_activation, used for location_registration.
    private final String SES_URL = "https://ses.o2.co.uk:443/generic_devices";
    private final String TEST_SES_URL = "https://ses-test.o2.co.uk:443/generic_devices";

    EntitlementHandling(Context context, Listener listener, int slotId) {
        //mServiceList = serviceList;
        mContext = context;
        mListeners.add(listener);
        mSlotId = slotId;
        //for (String service : mServiceList) {
            //mStates.put(service, STATE_INITIAL);
        //}
        mApi = new EntitlementServerApi(mContext, getEntitlementApiEndpoint(), getDeviceId(mContext), slotId);
    }

    // These 'public' methods will be called by any thread:

    void clearListeners() {
        synchronized (mListeners) {
            mListeners.clear();
        }
    }

    synchronized void startEntitlementCheck(ArrayList<String> serviceList) {
        if (isinActiveState()) {
            return;
        }
        mServiceList = serviceList;
        for (String service : mServiceList) {
            updateState(service, STATE_ACTIVATING);
        }
        doServiceStatusCheck();
    }

    synchronized void stopEntitlementCheck() {
        log("in stopEntitlementCheck(): " + mStates);
        if (mStates != null) {
            for (String service : mServiceList) {
                int state = mStates.get(service);
                log("stopEntitlementCheck, current state = " + state + " for service = " + service);
                if (state != STATE_ACTIVATING) {
                    continue;
                } else {
                    mStates.put(service, STATE_ENTITLED_FAIL);
                }
            }
        }
    }

    private void doServiceStatusCheck() {
        mApi.createMethod()
                .addServiceEntitlementStatus(mServiceList.toArray(new String[mServiceList.size()]))
                .addGetMSISDN()
                .execute(new EntitlementServerApi.Callback() {
                    @Override
                    public void callback(EntitlementServerApi.Response rsp) {
                        onServiceStatusCheckResponse(rsp);
                    }
                });
    }

    private synchronized void onServiceStatusCheckResponse(EntitlementServerApi.Response rsp) {
        if (!isinActiveState()) {
            log("Currently not in active state, return from onServiceStatusCheckResponse");
            return;
        }
        boolean isEntitled;
        if (rsp == null || !rsp.isSuccessful(EntitlementServerApi.METHOD_CHECK_ENTITLEMENT) ||
            !rsp.isSuccessful(EntitlementServerApi.METHOD_GET_MSISDN)) {
            Log.e(TAG, "In onServiceStatusCheckResponse, Receive failed callback");
            for (String service : mServiceList) {
                updateState(service, STATE_ENTITLED_FAIL);
            }
        } else {
            mMSISDN = rsp.getString(EntitlementServerApi.METHOD_GET_MSISDN, "msisdn");
            mServiceFingerprint = rsp.getString(EntitlementServerApi.METHOD_GET_MSISDN, "service-fingerprint");
            mPollInterval = rsp.getInt(EntitlementServerApi.METHOD_CHECK_ENTITLEMENT, "poll-interval");
            logTel("MSISDN: " + mMSISDN + ", " + mServiceFingerprint + "Poll = " + mPollInterval);
            for (String service : mServiceList) {
                isEntitled = rsp.getEntitlementState(EntitlementServerApi.METHOD_CHECK_ENTITLEMENT, service);
                log("Entitlement result: " + isEntitled);
                if (isEntitled) {
                    log("Poll interval = " + mPollInterval);
                    updateState(service, STATE_ENTITLED);
                } else {
                    updateState(service, STATE_NOT_ENTITLED);
                }
            }
        }
    }

    private synchronized void updateState(String service, int state) {
        log("updateState for:  " + service + " to " + stateToString(state));
        mStates.put(service, state);

        synchronized (mListeners) {
            for (Listener l : mListeners) {
                l.onStateChange(this, state, service, mSlotId);
            }
        }
    }

    public int getPollInterval() {
        return mPollInterval;
    }

    private static String getDeviceId(Context context) {
        return Settings.Secure.getString(context.getContentResolver(), Settings.Secure.ANDROID_ID);
    }

    private static String stateToString(int state) {
        switch (state) {
            case STATE_ACTIVATING:
                return "STATE_ACTIVATING";
            case STATE_NOT_ENTITLED:
                return "STATE_NOT_ENTITLED";
            case STATE_ENTITLED:
                return "STATE_ENTITLED";
            case STATE_ENTITLED_FAIL:
                return "STATE_ENTITLED_FAIL";
            case STATE_INITIAL:
                return "STATE_INITIAL";
            default:
                return "UNKNOWN state";
        }
    }

    private String getEntitlementApiEndpoint() {
        // Possible endpoints:
        // - AT&T Lab SES Server: https://sentitlement2.npc.mobilephone.net/WFC
        // - AT&T live Network SES Server: https://sentitlement2.mobile.att.net/WFC
        // - Ericson old SES Testing server: http://ses.ericsson-magic.net:10090/generic_devices
        // - Ericson new SES Testing server: http://ses.ericsson-magic.net:8080/generic_devices
        //
        // By default, connect to AT&T live network SES server.

        String sesUrl = SystemProperties.get("persist.vendor.entitlement.sesurl",
                            getDefaultSesUrl(
                            "https://ses.o2.co.uk:443/generic_devices"));
        logTel("sesUrl = "+sesUrl);
        return sesUrl;
    }

    public String getDefaultSesUrl(String url) {
        return SES_URL;
    }

    private void log(String s) {
        if (DEBUG) Log.d(TAG, "[" + mSlotId + "]" + s);
    }

    private void loge(String s) {
        Log.e(TAG, "[" + mSlotId + "]" + s);
    }

    private void logTel(String s) {
        if (ENG || TELDBG) {
            Log.d(TAG,  "[" + mSlotId + "]" + s.replaceAll("[0-9a-fA-F]{7}", "***"));
        }
    }

    public boolean isinActiveState() {
        boolean ret = false;
        if (mStates != null) {
            for (String service : mServiceList) {
                int state = mStates.get(service);
                log("isinActiveState,current state = " + state + " for service = " + service);
                if (state == STATE_ACTIVATING) {
                    ret = true;
                }
            }
        }
        return ret;
    }
}
