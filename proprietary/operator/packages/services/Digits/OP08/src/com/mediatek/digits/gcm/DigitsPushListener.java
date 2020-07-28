package com.mediatek.digits.gcm;

import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.support.v4.content.LocalBroadcastManager;
import android.util.Log;

import com.google.android.gms.gcm.GcmListenerService;

import org.json.JSONArray;
import org.json.JSONObject;
import org.json.JSONArray;
import org.json.JSONException;

public class DigitsPushListener extends GcmListenerService {

    private static final String TAG = "DigitsPushListener";

    private static final String DIGITS_GCM_MESSAGE = "com.mediatek.digits.gcmmessage";

    private static final String INTENT_PUSH_MESSAGE = "com.mediatek.digits.intent.PUSH_MESSAGE";
    private static final String EXTRA_KEY_DATA = "data";
    private static final String EXTRA_KEY_FROM = "from";

    /**
     * Called when message is received.
     *
     * @param from SenderID of the sender.
     * @param data Data bundle containing message data as key/value pairs.
     *             For Set of keys use data.keySet().
     */
    // [START receive_message]
    @Override
    public void onMessageReceived(String from, Bundle data) {
        Log.d(TAG, "From: " + from);
        Log.d(TAG, "Data: " + data);
        boolean processMsg = false;

        String pushMsg = data.getString("push-message");
        if(pushMsg != null) {
            try {
                Bundle bundle = new Bundle();
                Log.d(TAG, "push-message : " + pushMsg);
                JSONObject jPushMsg = new JSONObject(pushMsg);
                String channel = jPushMsg.optString("channel", null);
                String pnsType = jPushMsg.optString("pns-type", null);
                String serviceName = jPushMsg.optString("serviceName", null);

                bundle.putString("channel", channel);
                bundle.putString("pns-type", pnsType);
                bundle.putString("serviceName", serviceName);
                Log.d(TAG, "channel: " + channel +
                      ", pnsType: " + pnsType +
                      ", serviceName: " + serviceName);

                if (pnsType.equals("ENA")) {
                    String pnsSubtype = jPushMsg.optString("pns-subtype", null);
                    bundle.putString("pns-subtype", pnsSubtype);

                    String message = jPushMsg.optString("message", null);
                    bundle.putString("message", message);
                    processMsg = true;
                } else if (pnsType.equals("conn_mgr")) {
                    String message = jPushMsg.optString("message", null);
                    JSONObject jMsg = new JSONObject(message);
                    bundle.putString("message", message);
                    String eventType = jMsg.optString("event-type", null);
                    String eventBody = jMsg.optString("event-body", null);
                    JSONObject jEventBody = new JSONObject(eventBody);
                    String subServicName = jEventBody.optString("service-name", null);
                    String deviceName = jEventBody.optString("device-name", null);
                    String msisdn = jEventBody.optString("msisdn", null);
                    bundle.putString("event-type", eventType);
                    bundle.putString("event-body", eventBody);
                    bundle.putString("service-name", subServicName);
                    bundle.putString("device-name", deviceName);
                    bundle.putString("msisdn", msisdn);
                    Log.d(TAG, "eventType: " + eventType +
                          ", eventBody: " + eventBody +
                          ", subServicName: " + subServicName +
                          ", deviceName: " + deviceName +
                          ", msisdn: " + msisdn);
                    processMsg = true;
                } else if (pnsType.equals("Call")) {
                    String pnsSubtype = jPushMsg.optString("pns-subtype", null);
                    bundle.putString("pns-subtype", pnsSubtype);
                    Log.d(TAG, "pns-subtype: " + pnsSubtype);
                    if (pnsSubtype.equals("History")) {
                        String nmsEventList = jPushMsg.optString("nmsEventList", null);
                        JSONObject jNmsEventList = new JSONObject(nmsEventList);
                        String nmsEvent = jNmsEventList.optString("nmsEvent", null);
                        JSONArray jNmsEvents = new JSONArray(nmsEvent);
                        for (int j = 0; j < jNmsEvents.length(); j++) {
                            JSONObject jNmsEvent = jNmsEvents.getJSONObject(j);
                            String changedObject = jNmsEvent.optString("changedObject", null);
                            if (changedObject != null) {
                                JSONObject jChangedObject = new JSONObject(changedObject);
                                String message = jChangedObject.optString("message", null);

                                if (message != null) {
                                    JSONObject jMsg = new JSONObject(message);
                                    String participatingDevice = jMsg.optString("participating-device", null);
                                    String status = jMsg.optString("status", null);
                                    String direction = jMsg.optString("direction", null);
                                    Log.d(TAG, "participatingDevice: " + participatingDevice + ", status: " + status);
                                    if (status != null && status.contains("ANSWERED") &&
                                            direction != null && direction.equals("In")) {
                                        bundle.putString("status", status);
                                        bundle.putString("participating-device", participatingDevice);
                                        processMsg = true;
                                        break;
                                    }
                                }
                            }
                        }

                    }
                } else if (pnsType.equals("IAM")) {
                    String pnsSubtype = jPushMsg.optString("pns-subtype", null);
                    bundle.putString("pns-subtype", pnsSubtype);
                    if (pnsSubtype.equals("Permission")) {
                        String message = jPushMsg.optString("message", null);
                        bundle.putString("message", message);
                        processMsg = true;
                    } else if (pnsSubtype.equals("password-changePN") ||
                               pnsSubtype.equals("global-logoutPN") ||
                               pnsSubtype.equals("account-lockPN") ||
                               pnsSubtype.equals("password-resetPN")) {
                        processMsg = true;
                    }
                } else if (pnsType.equals("vowifi")) {
                    String message = jPushMsg.optString("message", null);
                    JSONObject jMsg = new JSONObject(message);
                    bundle.putString("message", message);
                    String eventType = jMsg.optString("event-type", null);
                    String eventBody = jMsg.optString("event-body", null);
                    JSONObject jEventBody = new JSONObject(eventBody);
                    String friendlyName = jEventBody.optString("friendly-name", null);
                    String msisdn = jEventBody.optString("msisdn", null);
                    bundle.putString("event-type", eventType);
                    bundle.putString("event-body", eventBody);
                    bundle.putString("friendly-name", friendlyName);
                    bundle.putString("msisdn", msisdn);
                    Log.d(TAG, "eventType: " + eventType +
                          ", eventBody: " + eventBody +
                          ", friendlyName: " + friendlyName +
                          ", msisdn: " + msisdn);
                    processMsg = true;
                }

                if (processMsg) {
                    Intent intent = new Intent();
                    intent.setAction(DIGITS_GCM_MESSAGE);
                    intent.putExtras(bundle);
                    LocalBroadcastManager.getInstance(getApplicationContext()).sendBroadcast(intent);
                }

                // Notify application for all push
                Intent pushIntent = new Intent(INTENT_PUSH_MESSAGE);
                pushIntent.putExtra(EXTRA_KEY_FROM, from);
                pushIntent.putExtra(EXTRA_KEY_DATA, data);
                getApplicationContext().sendBroadcast(pushIntent);

            } catch (JSONException e) {
                Log.e(TAG, "JSON object conversion error: ", e);
            }
        }
    }
}
