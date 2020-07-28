package com.mediatek.entitlement.fcm;

import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import com.google.firebase.messaging.FirebaseMessagingService;
import com.google.firebase.messaging.RemoteMessage;
import java.util.HashMap;

public class FcmListener extends FirebaseMessagingService {

    private static final String TAG = "FcmListener";

    public static final String ENTITLEMENT_PACKAGE_NAME = "com.mediatek.entitlement";

    public static final String FCM_MESSAGE_RECEIVED_INTENT = "com.mediatek.entitlement.fcm.MESSAGE_RECEIVED";
    public static final String FCM_NEW_TOKEN_INTENT = "com.mediatek.entitlement.fcm.NEW_TOKEN";

    public static final String FCM_BROADCAST_PERMISSION = "com.mediatek.permission.entitlement.FCM_BROADCAST";

    public static final String FCM_INTENT_KEY_DATA = "DATA";
    public static final String FCM_INTENT_KEY_TOKEN = "TOKEN";
    /**
     * Called when message is received.
     *
     * @param from SenderID of the sender.
     * @param data Data bundle containing message data as key/value pairs.
     *             For Set of keys use data.keySet().
     */
    // [START receive_message]
    @Override
    public void onMessageReceived(RemoteMessage message){
        String from = message.getFrom();
        HashMap<String, String> data = new HashMap<String, String>(message.getData());

        Log.d(TAG, "onMessageReceived(), From: " + from);
        Log.d(TAG, "onMessageReceived(), Data: " + data);

        Intent intent = new Intent(FCM_MESSAGE_RECEIVED_INTENT);
        intent.setPackage(ENTITLEMENT_PACKAGE_NAME);

        Bundle bundle = new Bundle();
        bundle.putSerializable(FCM_INTENT_KEY_DATA, data);
        intent.putExtras(bundle);
        getApplicationContext().sendBroadcast(intent, FCM_BROADCAST_PERMISSION);
    }

    @Override
    public void onNewToken(String token) {
        Log.d(TAG, "onNewToken(): " + token);

        Intent intent = new Intent(FCM_NEW_TOKEN_INTENT);
        intent.setPackage(ENTITLEMENT_PACKAGE_NAME);

        intent.putExtra(FCM_INTENT_KEY_TOKEN, token);
        getApplicationContext().sendBroadcast(intent, FCM_BROADCAST_PERMISSION);
    }

}
