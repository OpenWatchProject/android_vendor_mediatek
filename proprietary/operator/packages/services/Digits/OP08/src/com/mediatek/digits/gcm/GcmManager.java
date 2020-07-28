package com.mediatek.digits.gcm;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;

import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.support.v4.content.LocalBroadcastManager;
import android.util.Log;

import org.json.JSONArray;
import org.json.JSONObject;
import org.json.JSONArray;
import org.json.JSONException;

public class GcmManager {


    private static final String TAG = "GcmManager";

    // key: EVENT_KEY_MSISDN
    public static final int EVENT_VOWIFI_SUBSCRIPTION_REMOVED_CUSTOMER_CARE = 0;

    // key: EVENT_KEY_MSISDN
    public static final int EVENT_VOWIFI_SUBSCRIPTION_REMOVED = 1;

    // key: EVENT_KEY_MSISDN
    // key: EVENT_KEY_NAME
    public static final int EVENT_LINE_NAME_UPDATED = 2;

    // key: EVENT_KEY_MSISDN
    public static final int EVENT_NEW_LINE_INSTANCE_ACTIVATION = 3;

    // key: EVENT_KEY_IMEI
    public static final int EVENT_CALL_ANSWERED = 4;

    // key: EVENT_KEY_MSISDN
    // key: EVENT_KEY_CHANGE_TYPE
    public static final int EVENT_MSISDN_AUTH_CHANGED = 5;

    public static final int EVENT_MUTUALLY_AGREE_LOGOUT = 6;

    public static final int EVENT_RESULT_SUCCEED = 0;
    public static final int EVENT_RESULT_FAIL = 1;

    public static final String EVENT_KEY_DEVICE_ID = "device_id";
    public static final String EVENT_KEY_MSISDN = "msisdn";
    public static final String EVENT_KEY_NAME = "name";
    public static final String EVENT_KEY_IMSI = "imsi";
    public static final String EVENT_KEY_IMEI = "imei";
    public static final String EVENT_KEY_CHANGE_TYPE = "change_type";

    private static final String DIGITS_GCM_MESSAGE = "com.mediatek.digits.gcmmessage";

    private Context mContext;
    private GcmListener mGcmListener;
    private Thread mThread;

    /** Interface for callback to notify GCM event */
    public interface GcmListener {
        /**
         * callback when any events happened.
         */
        void onGcmEvent(int event, Bundle bundle);
    }


    public GcmManager(Context context, GcmListener listener) {
        mContext = context;
        mGcmListener = listener;

        registerIntentReceiver();
    }

    private void registerIntentReceiver() {
        IntentFilter filter = new IntentFilter();
        filter.addAction(DIGITS_GCM_MESSAGE);
        LocalBroadcastManager.getInstance(mContext).registerReceiver(mGcmReceiver, filter);
        //testGcmManager();
    }

    private BroadcastReceiver mGcmReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context contex, Intent intent) {
            Bundle extras = intent.getExtras();

            String channel = extras.getString("channel");
            String pnsType = extras.getString("pns-type");
            String serviceName = extras.getString("serviceName");

            Bundle bundle = new Bundle();

            Log.d(TAG, "channel: " + channel +
                  ", pnsType: " + pnsType +
                  ", serviceName: " + serviceName);

            if (pnsType.equals("ENA")) {
                String pnsSubtype = extras.getString("pns-subtype");
                Log.d(TAG, "pnsSubtype: " + pnsSubtype);
                String message = extras.getString("message");
                JSONObject jMsg;
                Log.d(TAG, " message: " + message);
                try {
                    jMsg = new JSONObject(message);
                } catch (JSONException e) {
                    Log.e(TAG, "Parsing GCM message JSON object conversion error: ", e);
                    return;
                }

                if (pnsSubtype.equals("vowifi-subscription-removed-customer-care")) {
                    bundle.putString(EVENT_KEY_MSISDN, jMsg.optString("msisdn", null));
                    bundle.putString(EVENT_KEY_DEVICE_ID, jMsg.optString("device-id", null));
                    mThread = new Thread() {
                        public void run() {
                            mGcmListener.onGcmEvent(EVENT_VOWIFI_SUBSCRIPTION_REMOVED_CUSTOMER_CARE,
                                                    bundle);
                        }
                    };
                    mThread.start();
                } else if (pnsSubtype.equals("vowifi-subscription-removed")) {
                    bundle.putString(EVENT_KEY_MSISDN, jMsg.optString("msisdn", null));
                    bundle.putString(EVENT_KEY_DEVICE_ID, jMsg.optString("device-id", null));
                    mThread = new Thread() {
                        public void run() {
                            mGcmListener.onGcmEvent(EVENT_VOWIFI_SUBSCRIPTION_REMOVED, bundle);
                        }
                    };
                    mThread.start();
                }  else if (pnsSubtype.equals("friendly-name-updated")) {
                    bundle.putString(EVENT_KEY_MSISDN, jMsg.optString("msisdn", null));
                    bundle.putString(EVENT_KEY_NAME, jMsg.optString("friendly-name", null));
                    mThread = new Thread() {
                        public void run() {
                            mGcmListener.onGcmEvent(EVENT_LINE_NAME_UPDATED, bundle);
                        }
                    };
                    mThread.start();
                }
            } else if (pnsType.equals("conn_mgr")) {
                String eventType = extras.getString("event-type");
                String eventBody = extras.getString("event-body");
                String subServicName = extras.getString("service-name");
                String deviceName = extras.getString("device-name");
                String msisdn = extras.getString("msisdn");

                Log.d(TAG, "eventType: " + eventType +
                      ", eventBody: " + eventBody +
                      ", subServicName: " + subServicName +
                      ", deviceName: " + deviceName +
                      ", msisdn: " + msisdn);
                if (eventType.equals("service-activated") &&
                        subServicName.equals("vowifi")) {
                    bundle.putString(EVENT_KEY_MSISDN, msisdn);
                    mThread = new Thread() {
                        public void run() {
                            mGcmListener.onGcmEvent(EVENT_NEW_LINE_INSTANCE_ACTIVATION, bundle);
                        }
                    };
                    mThread.start();
                }
            } else if (pnsType.equals("Call")) {
                String pnsSubtype = extras.getString("pns-subtype");
                Log.d(TAG, "pnsSubtype: " + pnsSubtype);
                if (pnsSubtype.equals("History")) {
                    String status = extras.getString("status");
                    if (status.contains("ANSWERED")) {
                        String participatingDevice = extras.getString("participating-device");
                        Log.d(TAG, "participatingDevice: " + participatingDevice);
                        if (participatingDevice != null) {
                            String imei=participatingDevice.replace("-", "").replace("urn:gsma:imei:", "");
                            Log.d(TAG, "imei: " + imei);
                            bundle.putString(EVENT_KEY_IMEI, imei);
                        }
                        mThread = new Thread() {
                            public void run() {
                                mGcmListener.onGcmEvent(EVENT_CALL_ANSWERED, bundle);
                            }
                        };
                        mThread.start();
                    }
                }
            } else if (pnsType.equals("IAM")) {
                String pnsSubtype = extras.getString("pns-subtype");
                Log.d(TAG, "pnsSubtype: " + pnsSubtype);
                if (pnsSubtype.equals("Permission")) {
                    String message = extras.getString("message");
                    if (message != null) {
                        Log.d(TAG, " message: " + message);
                        JSONObject jMsg;
                        try {
                            jMsg = new JSONObject(message);
                            String permission = jMsg.optString("permission", null);
                            Log.d(TAG, " permission: " + permission);
                            if (permission != null) {
                                JSONObject jPermission = new JSONObject(permission);
                                String changeType = jPermission.optString("change-type", null);
                                Log.d(TAG, " changeType: " + changeType);
                                if (changeType.equals("GRANT") || changeType.equals("REVOKE")) {
                                    String msisdn = jPermission.optString("account", null);
                                    bundle.putString(EVENT_KEY_MSISDN, msisdn);
                                    bundle.putString(EVENT_KEY_CHANGE_TYPE, changeType);
                                    Log.d(TAG, "account: " + msisdn);
                                    mThread = new Thread() {
                                        public void run() {
                                            mGcmListener.onGcmEvent(EVENT_MSISDN_AUTH_CHANGED, bundle);
                                        }
                                    };
                                    mThread.start();
                                }
                            }
                        } catch (JSONException e) {
                            Log.e(TAG, "Parsing GCM message JSON object conversion error: ", e);
                        }
                    }
                } else if (pnsSubtype.equals("password-changePN") ||
                           pnsSubtype.equals("global-logoutPN") ||
                           pnsSubtype.equals("account-lockPN") ||
                           pnsSubtype.equals("password-resetPN")) {
                    Log.d(TAG, "IAM mutually agree force logout");
                    mThread = new Thread() {
                        public void run() {
                            mGcmListener.onGcmEvent(EVENT_MUTUALLY_AGREE_LOGOUT, bundle);
                        }
                    };
                    mThread.start();
                }
            } else if (pnsType.equals("vowifi")) {
                String eventType = extras.getString("event-type");
                String eventBody = extras.getString("event-body");
                String friendlyName = extras.getString("friendly-name");
                String msisdn = extras.getString("msisdn");

                Log.d(TAG, "eventType: " + eventType +
                      ", eventBody: " + eventBody +
                      ", friendlyName: " + friendlyName +
                      ", msisdn: " + msisdn);
                if (eventType.equals("msisdn-name-update")) {
                    bundle.putString(EVENT_KEY_MSISDN, msisdn);
                    bundle.putString(EVENT_KEY_NAME, friendlyName);
                    mThread = new Thread() {
                        public void run() {
                            mGcmListener.onGcmEvent(EVENT_LINE_NAME_UPDATED, bundle);
                        }
                    };
                    mThread.start();
                }
            }
        }
    };

    public static String eventToString(int event) {
        switch(event) {
            case EVENT_VOWIFI_SUBSCRIPTION_REMOVED_CUSTOMER_CARE:
                return "EVENT_VOWIFI_SUBSCRIPTION_REMOVED_CUSTOMER_CARE";
            case EVENT_VOWIFI_SUBSCRIPTION_REMOVED:
                return "EVENT_VOWIFI_SUBSCRIPTION_REMOVED";
            case EVENT_LINE_NAME_UPDATED:
                return "EVENT_LINE_NAME_UPDATED";
            case EVENT_NEW_LINE_INSTANCE_ACTIVATION:
                return "EVENT_NEW_LINE_INSTANCE_ACTIVATION";
            case EVENT_CALL_ANSWERED:
                return "EVENT_CALL_ANSWERED";
            case EVENT_MSISDN_AUTH_CHANGED:
                return "EVENT_MSISDN_AUTH_CHANGED";
            case EVENT_MUTUALLY_AGREE_LOGOUT:
                return "EVENT_MUTUALLY_AGREE_LOGOUT";
            default:
                return "UNKNOWN_EVENT(" + event + ")";
        }
    }

    /*private void testGcmManager() {
        Bundle bundle = new Bundle();
        bundle.putString("pns-type", "IAM");
        bundle.putString("pns-subtype", "Permission");
        bundle.putString("message", "{\"permission\":{\"grantor-uid\":\"U-80a20c1b-9218-4bef-8799-f434b743ca06\",\"data\":{\"end-date\":\"\",\"service\":\"vowifi\",\"start-date\":\"20171206084135Z\",\"privilege\":\"Limited\",\"status\":\"Enabled\"},\"change-type\":\"REVOKE\",\"grantee-uid\":\"U-43b01bd5-e8c4-46ed-acad-9cb04b370b27\",\"grantee\":\"wendy-st.lin@mediatek.com\",\"grantor-msisdn\":\"7023251480\",\"owners\":[{\"owner\":\"fangju.wang@mediatek.com\",\"owner-role\":\"AO\",\"owner-msisdn\":\"7023251480\"},{\"owner\":\"fangju.wang@mediatek.com\",\"owner-role\":\"AO\",\"owner-msisdn\":\"7023251480\"}],\"grantor\":\"fangju.wang@mediatek.com\",\"text\":\"\",\"grantee-msisdn\":\"4258372190\",\"account\":\"2107897239\"}}");

        Intent intent = new Intent();
        intent.setAction("com.mediatek.digits.gcmmessage");
        intent.putExtras(bundle);
        LocalBroadcastManager.getInstance(mContext).sendBroadcast(intent);

        Bundle bundle1 = new Bundle();
        bundle1.putString("pns-type", "IAM");
        bundle1.putString("pns-subtype", "Permission");
        bundle1.putString("message", "{\"permission\":{\"grantor-uid\":\"U-80a20c1b-9218-4bef-8799-f434b743ca06\",\"data\":{\"end-date\":\"\",\"service\":\"vowifi\",\"start-date\":\"20171206091201Z\",\"privilege\":\"Limited\",\"status\":\"Enabled\"},\"change-type\":\"GRANT\",\"grantee-uid\":\"U-43b01bd5-e8c4-46ed-acad-9cb04b370b27\",\"grantee\":\"wendy-st.lin@mediatek.com\",\"grantor-msisdn\":\"7023251480\",\"owners\":[{\"owner\":\"fangju.wang@mediatek.com\",\"owner-role\":\"AO\",\"owner-msisdn\":\"7023251480\"},{\"owner\":\"fangju.wang@mediatek.com\",\"owner-role\":\"AO\",\"owner-msisdn\":\"7023251480\"}],\"grantor\":\"fangju.wang@mediatek.com\",\"text\":\"\",\"grantee-msisdn\":\"4258372190\",\"account\":\"7023251480\"}}");
        Intent intent1 = new Intent();

        intent1.putExtras(bundle1);
        LocalBroadcastManager.getInstance(mContext).sendBroadcast(intent1);
    }*/

}
