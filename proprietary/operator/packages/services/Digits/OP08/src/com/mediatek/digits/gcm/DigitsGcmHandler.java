package com.mediatek.digits.gcm;



import android.content.Context;
import android.util.Log;
import com.google.android.gms.gcm.GcmPubSub;
import com.google.android.gms.gcm.GoogleCloudMessaging;
import com.google.android.gms.iid.InstanceID;
import com.google.android.gms.iid.InstanceIDListenerService;

import java.io.IOException;

public class DigitsGcmHandler extends InstanceIDListenerService {

    private static final String TAG = "DigitsGcmHandler";
    private static final String[] TOPICS = {"global"};

    private Context mContext;
    private String mSenderId;
    private String mGcmToken;

    private static DigitsGcmHandler sInstance;

    // Private constructor
    private DigitsGcmHandler(Context context) {
        mContext = context;

    }

    public static DigitsGcmHandler getInstance(Context context) {
        if (sInstance == null) {
            sInstance = new DigitsGcmHandler(context);
        }
        return sInstance;
    }

    public void initGcmToken(String senderId) {
        Log.i(TAG, "initGcmToken(), senderId: " + senderId);
        mSenderId = senderId;
        registerGcmToken();
    }

    public String getGcmToken() {
        return mGcmToken;
    }

    /**
     * Called if InstanceID token is updated. This may occur if the security of
     * the previous token had been compromised. This call is initiated by the
     * InstanceID provider.
     */
    @Override
    public void onTokenRefresh() {
        // Fetch updated Instance ID token and notify it.
        registerGcmToken();
    }

    /*
     * trigger register for gcm, and get the token.
     *
     * @return GCM Token.
     */
    private String registerGcmToken() {
        try {
            // Initially this call goes out to the network to retrieve the token, subsequent calls
            // are local.
            // R.string.gcm_defaultSenderId (the Sender ID) is typically derived from google-services.json.
            // See https://developers.google.com/cloud-messaging/android/start for details on this file.
            InstanceID instanceID = InstanceID.getInstance(mContext);

            mGcmToken = instanceID.getToken(mSenderId,
                    GoogleCloudMessaging.INSTANCE_ID_SCOPE, null);

            Log.i(TAG, "registerGcmToken(), token: " + mGcmToken);

            // Subscribe to topic channels
            subscribeTopics(mGcmToken, mContext);

            return mGcmToken;
        } catch (Exception e) {
            Log.e(TAG, "Failed to complete token refresh", e);
        }

        return null;
    }

    /**
     * Subscribe to any GCM topics of interest, as defined by the TOPICS constant.
     *
     * @param token GCM token
     * @throws IOException if unable to reach the GCM PubSub service
     */
    private void subscribeTopics(String token, Context context) throws IOException {
        GcmPubSub pubSub = GcmPubSub.getInstance(context);
        for (String topic : TOPICS) {
            pubSub.subscribe(token, "/topics/" + topic, null);
        }
    }
}
