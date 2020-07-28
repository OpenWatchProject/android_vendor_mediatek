/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.location.agps.apn;

import android.app.AlertDialog;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.location.LocationManager;
import android.os.Handler;
import android.os.Message;
import android.os.SystemClock;
import android.os.UserHandle;
import android.telecom.TelecomManager;
import android.telephony.PhoneNumberUtils;
import android.telephony.PhoneStateListener;
import android.telephony.TelephonyManager;
import android.view.WindowManager;
import android.util.Log;

import com.android.internal.R;
import com.android.internal.notification.SystemNotificationChannels;
import com.android.internal.telephony.GsmAlphabet;
import com.mediatek.location.agps.apn.NetInitiatedListener;

import java.io.UnsupportedEncodingException;
import java.util.concurrent.TimeUnit;

public class GpsNetInitiatedHandler {

    private static final String TAG = "GpsNetInitiatedHandler";

    private static final boolean DEBUG = true;

    // these need to match GpsNiType constants in gps_ni.h
    public static final int GPS_NI_TYPE_VOICE = 1;
    public static final int GPS_NI_TYPE_UMTS_SUPL = 2;
    public static final int GPS_NI_TYPE_UMTS_CTRL_PLANE = 3;
    public static final int GPS_NI_TYPE_EMERGENCY_SUPL = 4;

    // these need to match GpsUserResponseType constants in gps_ni.h
    public static final int GPS_NI_RESPONSE_ACCEPT = 1;
    public static final int GPS_NI_RESPONSE_DENY = 2;
    public static final int GPS_NI_RESPONSE_NORESP = 3;
    public static final int GPS_NI_RESPONSE_IGNORE = 4;

    // these need to match GpsNiNotifyFlags constants in gps_ni.h
    public static final int GPS_NI_NEED_NOTIFY = 0x0001;
    public static final int GPS_NI_NEED_VERIFY = 0x0002;
    public static final int GPS_NI_PRIVACY_OVERRIDE = 0x0004;

    // these need to match GpsNiEncodingType in gps_ni.h
    public static final int GPS_ENC_NONE = 0;
    public static final int GPS_ENC_SUPL_GSM_DEFAULT = 1;
    public static final int GPS_ENC_SUPL_UTF8 = 2;
    public static final int GPS_ENC_SUPL_UCS2 = 3;
    public static final int GPS_ENC_SUPL_ASCII = 4;
    public static final int GPS_ENC_UNKNOWN = -1;

    private static final int GPS_NO_RESPONSE_TIME_OUT = 1;

    private final Context mContext;
    private final TelephonyManager mTelephonyManager;
    //private final PhoneStateListener mPhoneStateListener;
    private final TelecomManager mTelecom;

    // parent gps location provider
    private final LocationManager mLocationManager;

    // configuration of notificaiton behavior
    private boolean mPlaySounds = false;
    private boolean mPopupImmediately = true;

    // read the SUPL_ES form gps.conf
    private volatile boolean mIsSuplEsEnabled;

    // Set to true if the phone is having emergency call.
//    private volatile boolean mIsInEmergencyCall;

    // If Location function is enabled.
    private volatile boolean mIsLocationEnabled = false;

    private final NetInitiatedListener mNetInitiatedListener;

    // Set to true if string from HAL is encoded as Hex, e.g., "3F0039"
    static private boolean mIsHexInput = true;

    // End time of emergency call, and extension, if set
    private volatile long mCallEndElapsedRealtimeMillis = 0;
    private volatile long mEmergencyExtensionMillis = 0;

    AlertDialog mAlertDialog;
    GpsNiNotification mCurNotif;

    public static class GpsNiNotification
    {
        public int notificationId;
        public int niType;
        public boolean needNotify;
        public boolean needVerify;
        public boolean privacyOverride;
        public int timeout;
        public int defaultResponse;
        public String requestorId;
        public String text;
        public int requestorIdEncoding;
        public int textEncoding;
    };

    public static class GpsNiResponse {
        /* User response, one of the values in GpsUserResponseType */
        int userResponse;
    };

    private final BroadcastReceiver mBroadcastReciever = new BroadcastReceiver() {

        @Override public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(LocationManager.MODE_CHANGED_ACTION)) {
                updateLocationMode();
                if (DEBUG) Log.d(TAG, "location enabled :" + getLocationEnabled());
            }
        }
    };

    /**
     * The notification that is shown when a network-initiated notification
     * (and verification) event is received.
     * <p>
     * This is lazily created, so use {@link #setNINotification()}.
     */
    private Notification.Builder mNiNotificationBuilder;

    public GpsNetInitiatedHandler(Context context,
                                  NetInitiatedListener netInitiatedListener,
                                  boolean isSuplEsEnabled) {
        mContext = context;

        if (netInitiatedListener == null) {
            throw new IllegalArgumentException("netInitiatedListener is null");
        } else {
            mNetInitiatedListener = netInitiatedListener;
        }

        setSuplEsEnabled(isSuplEsEnabled);
        mLocationManager = (LocationManager)context.getSystemService(Context.LOCATION_SERVICE);
        updateLocationMode();
        mTelephonyManager =
            (TelephonyManager)context.getSystemService(Context.TELEPHONY_SERVICE);
        mTelecom = (TelecomManager) mContext.getSystemService(Context.TELECOM_SERVICE);

        /*mPhoneStateListener = new PhoneStateListener() {
            @Override
            public void onCallStateChanged(int state, String incomingNumber) {
                if (DEBUG) Log.d(TAG, "onCallStateChanged(): state is "+ state);
                // listening for emergency call ends
                if (state == TelephonyManager.CALL_STATE_IDLE) {
                    if (mTelecom.isInEmergencyCall()) {
                        mCallEndElapsedRealtimeMillis = SystemClock.elapsedRealtime();
                    }
                }
            }
        };
        mTelephonyManager.listen(mPhoneStateListener, PhoneStateListener.LISTEN_CALL_STATE);
        */

        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(LocationManager.MODE_CHANGED_ACTION);
        mContext.registerReceiver(mBroadcastReciever, intentFilter);
    }

    public void setSuplEsEnabled(boolean isEnabled) {
        mIsSuplEsEnabled = isEnabled;
    }

    public boolean getSuplEsEnabled() {
        return mIsSuplEsEnabled;
    }

    /**
     * Updates Location enabler based on location setting.
     */
    public void updateLocationMode() {
        mIsLocationEnabled = mLocationManager.isProviderEnabled(LocationManager.GPS_PROVIDER);
    }

    /**
     * Checks if user agreed to use location.
     */
    public boolean getLocationEnabled() {
        return mIsLocationEnabled;
    }

    /**
     * Determines whether device is in user-initiated emergency session based on the following
     * 1. If the user is making an emergency call, this is provided by actively
     *    monitoring the outgoing phone number;
     * 2. If the user has recently ended an emergency call, and the device is in a configured time
     *    window after the end of that call.
     * 3. If the device is in a emergency callback state, this is provided by querying
     *    TelephonyManager.
     * 4. If the user has recently sent an Emergency SMS and telephony reports that it is in
     *    emergency SMS mode, this is provided by querying TelephonyManager.
     * @return true if is considered in user initiated emergency mode for NI purposes
     */
    public boolean getInEmergency() {
        boolean isInEmergencyExtension =
                (mCallEndElapsedRealtimeMillis > 0)
                && ((SystemClock.elapsedRealtime() - mCallEndElapsedRealtimeMillis)
                        < mEmergencyExtensionMillis);
        boolean isInEmergencyCallback = mTelephonyManager.getEmergencyCallbackMode();
        boolean isInEmergencySmsMode = mTelephonyManager.isInEmergencySmsMode();
        boolean isInEmergencyCall = mTelecom.isInEmergencyCall();
        return isInEmergencyCall || isInEmergencyCallback || isInEmergencyExtension
                || isInEmergencySmsMode;
    }

    public void setEmergencyExtensionSeconds(int emergencyExtensionSeconds) {
        mEmergencyExtensionMillis = TimeUnit.SECONDS.toMillis(emergencyExtensionSeconds);
    }

    // Handles NI events from HAL
    public void handleNiNotification(GpsNiNotification notif) {
        if (DEBUG) Log.d(TAG, "in handleNiNotification () :"
                        + " notificationId: " + notif.notificationId
                        + " requestorId: " + notif.requestorId
                        + " text: " + notif.text
                        + " mIsSuplEsEnabled: " + getSuplEsEnabled()
                        + " mIsLocationEnabled: " + getLocationEnabled());

        try {
            if (getSuplEsEnabled()) {
                handleNiInEs(notif);
            } else {
                handleNi(notif);
            }
        } catch (Exception e) {
            Log.d(TAG, "Error encountered on NI handling.", e);
        }

        //////////////////////////////////////////////////////////////////////////
        //   A note about timeout
        //   According to the protocol, in the need_notify and need_verify case,
        //   a default response should be sent when time out.
        //
        //   In some GPS hardware, the GPS driver (under HAL) can handle the timeout case
        //   and this class GpsNetInitiatedHandler does not need to do anything.
        //
        //   However, the UI should at least close the dialog when timeout. Further,
        //   for more general handling, timeout response should be added to the Handler here.
        //
    }

    // handle NI form HAL when SUPL_ES is disabled.
    private void handleNi(GpsNiNotification notif) {
        if (DEBUG) Log.d(TAG, "in handleNi () :"
                        + " needNotify: " + notif.needNotify
                        + " needVerify: " + notif.needVerify
                        + " privacyOverride: " + notif.privacyOverride
                        + " mPopupImmediately: " + mPopupImmediately
                        + " mInEmergency: " + getInEmergency());

        if (!getLocationEnabled() && !getInEmergency()) {
            // Location is currently disabled, ignore all NI requests.
                mNetInitiatedListener.sendNiResponse(notif.notificationId,
                                                     GPS_NI_RESPONSE_IGNORE);
        }
        if (notif.needNotify) {
        // If NI does not need verify or the dialog is not requested
        // to pop up immediately, the dialog box will not pop up.
            if (notif.needVerify && mPopupImmediately) {
                // Popup the dialog box now
                openNiDialog(notif);
            } else {
                // Show the notification
                setNiNotification(notif);
            }
        }
        // ACCEPT cases: 1. Notify, no verify; 2. no notify, no verify;
        // 3. privacy override.
        if (!notif.needVerify || notif.privacyOverride) {
                mNetInitiatedListener.sendNiResponse(notif.notificationId,
                                                     GPS_NI_RESPONSE_ACCEPT);
        }
    }

    // handle NI from HAL when the SUPL_ES is enabled
    private void handleNiInEs(GpsNiNotification notif) {

        if (DEBUG) Log.d(TAG, "in handleNiInEs () :"
                    + " niType: " + notif.niType
                    + " notificationId: " + notif.notificationId);

        // UE is in emergency mode when in emergency call mode or in emergency call back mode
        /*
           1. When SUPL ES bit is off and UE is not in emergency mode:
                  Call handleNi() to do legacy behaviour.
           2. When SUPL ES bit is on and UE is in emergency mode:
                  Call handleNi() to do acceptance behaviour.
           3. When SUPL ES bit is off but UE is in emergency mode:
                  Ignore the emergency SUPL INIT.
           4. When SUPL ES bit is on but UE is not in emergency mode:
                  Ignore the emergency SUPL INIT.
        */
        boolean isNiTypeES = (notif.niType == GPS_NI_TYPE_EMERGENCY_SUPL);
        if (isNiTypeES != getInEmergency()) {
                mNetInitiatedListener.sendNiResponse(notif.notificationId,
                                                     GPS_NI_RESPONSE_IGNORE);
        } else {
            handleNi(notif);
        }
    }

    // Sets the NI notification.
    private synchronized void setNiNotification(GpsNiNotification notif) {
        NotificationManager notificationManager = (NotificationManager) mContext
                .getSystemService(Context.NOTIFICATION_SERVICE);
        if (notificationManager == null) {
            return;
        }

        String title = getNotifTitle(notif, mContext);
        String message = getNotifMessage(notif, mContext);

        if (DEBUG) Log.d(TAG, "setNiNotification, notifyId: " + notif.notificationId +
                ", title: " + title +
                ", message: " + message);

        NotificationChannel channel = new NotificationChannel(
                SystemNotificationChannels.NETWORK_ALERTS,
                "" + notif.notificationId,
                NotificationManager.IMPORTANCE_HIGH);
        notificationManager.createNotificationChannel(channel);

        // Construct Notification
        if (mNiNotificationBuilder == null) {
            mNiNotificationBuilder = new Notification.Builder(mContext,
                SystemNotificationChannels.NETWORK_ALERTS)
                    .setSmallIcon(com.android.internal.R.drawable.stat_sys_gps_on)
                    .setWhen(0)
                    .setOngoing(true)
                    .setAutoCancel(true)
                    .setColor(mContext.getColor(
                            com.android.internal.R.color.system_notification_accent_color));
        }

        if (mPlaySounds) {
            mNiNotificationBuilder.setDefaults(Notification.DEFAULT_SOUND);
        } else {
            mNiNotificationBuilder.setDefaults(0);
        }

        // if not to popup dialog immediately, pending intent will open the dialog
        /// M: Empty intent as pending intent would be a vulnerability.
        /// The empty intent may be filled another data by other 3rd party packages.
        /// We can fill it with a default string to avoid this vulnerability.
        String SHOULD_NOT_RESOLVE = "SHOULD_NOT_RESOLVE";
        Intent identityIntent = new Intent();
        identityIntent.setComponent(new ComponentName(SHOULD_NOT_RESOLVE, SHOULD_NOT_RESOLVE));
        identityIntent.setAction(SHOULD_NOT_RESOLVE);
        identityIntent.addCategory(SHOULD_NOT_RESOLVE);
        PendingIntent pi = PendingIntent.getBroadcast(mContext, 0, identityIntent, 0);

        mNiNotificationBuilder.setTicker(getNotifTicker(notif, mContext))
                .setContentTitle(title)
                .setContentText(message)
                .setContentIntent(pi);

        notificationManager.notifyAsUser(null, notif.notificationId, mNiNotificationBuilder.build(),
                UserHandle.ALL);
    }

    // Opens the notification dialog and waits for user input
    private void openNiDialog(GpsNiNotification notif)
    {
        if (DEBUG) Log.d(TAG, "openNiDialog, notifyId: " + notif.notificationId +
                ", requestorId: " + notif.requestorId +
                ", text: " + notif.text);

        AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
        builder.setTitle(getDialogTitle(notif, mContext));
        builder.setMessage(getDialogMessage(notif, mContext));
        builder.setCancelable(false);

        builder.setPositiveButton(String.format(mContext.getString(R.string.gpsVerifYes)),
            new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
                mNetInitiatedListener.sendNiResponse(mCurNotif.notificationId,
                        GPS_NI_RESPONSE_ACCEPT);
            }
        });

        builder.setNegativeButton(String.format(mContext.getString(R.string.gpsVerifNo)),
            new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
                mNetInitiatedListener.sendNiResponse(mCurNotif.notificationId,
                        GPS_NI_RESPONSE_DENY);
            }
        });

        mAlertDialog = builder.create();
        mAlertDialog.getWindow().setType(WindowManager.LayoutParams.TYPE_SYSTEM_ALERT);
        mAlertDialog.show();
        mCurNotif = notif;
        mHandler.removeMessages(GPS_NO_RESPONSE_TIME_OUT);
        mHandler.sendEmptyMessageDelayed(GPS_NO_RESPONSE_TIME_OUT, (notif.timeout * 1000));
    }

    private final Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case GPS_NO_RESPONSE_TIME_OUT: {
                if (mAlertDialog != null && mAlertDialog.isShowing()) {
                    mAlertDialog.cancel();
                    if (mCurNotif.notificationId != -1) {
                        mNetInitiatedListener.sendNiResponse(mCurNotif.notificationId,
                                mCurNotif.defaultResponse);
                    }
                }
                mAlertDialog = null;
            }
            break;
            default:
            }
        }
    };

    // Converts a string (or Hex string) to a char array
    static byte[] stringToByteArray(String original, boolean isHex)
    {
        int length = isHex ? original.length() / 2 : original.length();
        byte[] output = new byte[length];
        int i;

        if (isHex)
        {
            for (i = 0; i < length; i++)
            {
                output[i] = (byte) Integer.parseInt(original.substring(i*2, i*2+2), 16);
            }
        }
        else {
            for (i = 0; i < length; i++)
            {
                output[i] = (byte) original.charAt(i);
            }
        }

        return output;
    }

    /**
     * Unpacks an byte array containing 7-bit packed characters into a String.
     *
     * @param input a 7-bit packed char array
     * @return the unpacked String
     */
    static String decodeGSMPackedString(byte[] input)
    {
        final char PADDING_CHAR = 0x00;
        int lengthBytes = input.length;
        int lengthSeptets = (lengthBytes * 8) / 7;
        String decoded;

        /* Special case where the last 7 bits in the last byte could hold a valid
         * 7-bit character or a padding character. Drop the last 7-bit character
         * if it is a padding character.
         */
        if (lengthBytes % 7 == 0) {
            if (lengthBytes > 0) {
                if ((input[lengthBytes - 1] >> 1) == PADDING_CHAR) {
                    lengthSeptets = lengthSeptets - 1;
                }
            }
        }

        decoded = GsmAlphabet.gsm7BitPackedToString(input, 0, lengthSeptets);

        // Return "" if decoding of GSM packed string fails
        if (null == decoded) {
            Log.e(TAG, "Decoding of GSM packed string failed");
            decoded = "";
        }

        return decoded;
    }

    static String decodeUTF8String(byte[] input)
    {
        String decoded = "";
        try {
            decoded = new String(input, "UTF-8");
        }
        catch (UnsupportedEncodingException e)
        {
            throw new AssertionError();
        }
        return decoded;
    }

    static String decodeUCS2String(byte[] input)
    {
        String decoded = "";
        try {
            decoded = new String(input, "UTF-16");
        }
        catch (UnsupportedEncodingException e)
        {
            throw new AssertionError();
        }
        return decoded;
    }

    /** Decode NI string
     *
     * @param original   The text string to be decoded
     * @param isHex      Specifies whether the content of the string has been encoded as a Hex string. Encoding
     *                   a string as Hex can allow zeros inside the coded text.
     * @param coding     Specifies the coding scheme of the string, such as GSM, UTF8, UCS2, etc. This coding scheme
     *                      needs to match those used passed to HAL from the native GPS driver. Decoding is done according
     *                   to the <code> coding </code>, after a Hex string is decoded. Generally, if the
     *                   notification strings don't need further decoding, <code> coding </code> encoding can be
     *                   set to -1, and <code> isHex </code> can be false.
     * @return the decoded string
     */
    static private String decodeString(String original, boolean isHex, int coding)
    {
        if (coding == GPS_ENC_NONE || coding == GPS_ENC_UNKNOWN) {
            return original;
        }

        byte[] input = stringToByteArray(original, isHex);

        switch (coding) {
            case GPS_ENC_SUPL_GSM_DEFAULT:
                return decodeGSMPackedString(input);

            case GPS_ENC_SUPL_UTF8:
                return decodeUTF8String(input);

            case GPS_ENC_SUPL_UCS2:
                return decodeUCS2String(input);

            default:
                Log.e(TAG, "Unknown encoding " + coding + " for NI text " + original);
                return original;
        }
    }

    // change this to configure notification display
    static private String getNotifTicker(GpsNiNotification notif, Context context)
    {
        String ticker = String.format(context.getString(R.string.gpsNotifTicker),
                decodeString(notif.requestorId, mIsHexInput, notif.requestorIdEncoding),
                decodeString(notif.text, mIsHexInput, notif.textEncoding));
        return ticker;
    }

    // change this to configure notification display
    static private String getNotifTitle(GpsNiNotification notif, Context context)
    {
        String title = String.format(context.getString(R.string.gpsNotifTitle));
        return title;
    }

    // change this to configure notification display
    static private String getNotifMessage(GpsNiNotification notif, Context context)
    {
        String message = String.format(context.getString(R.string.gpsNotifMessage),
                decodeString(notif.requestorId, mIsHexInput, notif.requestorIdEncoding),
                decodeString(notif.text, mIsHexInput, notif.textEncoding));
        return message;
    }

    // change this to configure dialog display (for verification)
    static public String getDialogTitle(GpsNiNotification notif, Context context)
    {
        return getNotifTitle(notif, context);
    }

    // change this to configure dialog display (for verification)
    static private String getDialogMessage(GpsNiNotification notif, Context context)
    {
        return getNotifMessage(notif, context);
    }

}
