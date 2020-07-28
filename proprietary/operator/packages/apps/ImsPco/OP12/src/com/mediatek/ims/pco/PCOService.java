/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.ims.pco;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.wifi.WifiManager;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.Messenger;
import android.os.Process;
import android.os.SystemProperties;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;
import com.mediatek.ims.pco.ui.PCOStartUPWizardMainActivity;
import com.mediatek.internal.telephony.MtkPhoneConstants;

import static com.mediatek.ims.pco.PCOConfig.DEBUG;

/**
 * Service to switch ON or OFF data connection.
 * Invoked by PCOReceiver.
 */

public class PCOService extends Service {

    private static final String TAG = "PCOService";

    /* Subscription or IMS PCO status */
    private static final int STATE_PCO_UNKNOWN = -1;
    private static final int STATE_PCO_NOT_ACTIVATED = 1;
    private static final int STATE_PCO_ACTIVATING_PCO_VALUE_3 = 2;
    private static final int STATE_PCO_ACTIVATING_PCO_VALUE_5 = 3;
    private static final int STATE_PCO_ACTIVATING_NW_REJECT_EMM_8 = 4;
    private static final int STATE_PCO_ACTIVATED = 5;

    /* User Operations*/
    private static final int OPSUB_BROWSER_NOT_STARTED = 11;
    private static final int OPSUB_BROWSER_OPENNING = 12;
    private static final int OPSUB_BROWSER_DISABLE_CHECKSTATUS_UNTIL_REBOOT = 13;
    private static final int OPSUB_CALL_NOT_STARTED = 14;
    private static final int OPSUB_CALL_OPENNING = 15;
    private static final int OPSUB_CALL_DISABLE_CHECKSTATUS_UNTIL_REBOOT = 16;


    private static int sStateImsPCO = STATE_PCO_NOT_ACTIVATED;
    private static int sSubOPCall = OPSUB_BROWSER_NOT_STARTED;
    private static int sSubOPBrowser = OPSUB_CALL_NOT_STARTED;

    private static Context sContext;
    private static Handler sHandler;
    private static Looper sServiceLooper;
    private boolean mSimLoaded;
    private static PCOSharedPreference sPCOInfoPref;
    private SubscriptionManager mSubscriptionManager;
    private TelephonyManager mTelephonyManager;
    private String mImei;

    private static final String ACTION_CARRIER_SIGNAL_PCO_VALUE_DEBUG =
            "com.mediatek.ims.pco.ACTION_CARRIER_SIGNAL_PCO_VALUE_DEBUG";

    private static final String ACTION_NETWORK_REJECT_CAUSE_DEBUG =
            "com.mediatek.ims.pco.ACTION_NETWORK_REJECT_CAUSE_DEBUG";

    /**
     *
     */
    @Override
    public void onCreate() {
        log("[v]onCreate.........");
        super.onCreate();
        sContext = getApplicationContext();

        if (sContext == null) {
            log("sContext == null");
            return;
        }
        PCONwUtils.setAppContext(getApplicationContext());
        mSubscriptionManager = SubscriptionManager.from(sContext);
        mTelephonyManager = (TelephonyManager) sContext.getSystemService(Context.TELEPHONY_SERVICE);
        registerForIntents();

        restorePCOState();
    }

    private void restorePCOState() {
        sPCOInfoPref = PCOSharedPreference.getInstance(getApplicationContext());
        if (sPCOInfoPref != null) {
            int mHasPCOState = sPCOInfoPref.getSavedPCOState();
            log("restorePCOState(), Saved IMSPCO value =" + mHasPCOState);
            if (mHasPCOState < 0) {
                sPCOInfoPref.savePCOState(STATE_PCO_UNKNOWN);
            } else {
                updateImsPcoState(mHasPCOState);
                checkPCOActivationStatus();
            }
        }
    }

    private synchronized void resetImsPcoState() {
        updateImsPcoState(STATE_PCO_NOT_ACTIVATED);
        /**
         * Reset IMS PCO state, FirstBoot,
         * and activation status.
         */
        sPCOInfoPref.saveFirstBoot(true);
        sPCOInfoPref.saveActivated(false);
    }

    private int getImsPcoState() {
        log("[?]getImsPcoState() = " + stateToString(sStateImsPCO));
        return sStateImsPCO;
    }

    private void registerForIntents() {

        log("[+]Register- NW Broadcast receivers intents");
         final IntentFilter mIntentFilter = new IntentFilter();
        // Dynamic SIM Switch
        mIntentFilter.addAction(TelephonyIntents.ACTION_SIM_STATE_CHANGED);
        mIntentFilter.addAction(TelephonyIntents.ACTION_RADIO_STATE_CHANGED);
        // For IMS PCO values
        mIntentFilter.addAction(TelephonyIntents.ACTION_CARRIER_SIGNAL_PCO_VALUE);
        mIntentFilter.addAction(TelephonyIntents.ACTION_NETWORK_REJECT_CAUSE);
        mIntentFilter.addAction(WifiManager.WIFI_STATE_CHANGED_ACTION);

        mIntentFilter.addAction(Constants.ACTION_QUERY_IMS_PCO_STATUS_FRM_NOTIFICATION);
        mIntentFilter.addAction(
                Constants.ACTION_QUERY_IMS_PCO_STATUS_FRM_BROWSER);
        mIntentFilter.addAction(
                Constants.ACTION_QUERY_IMS_PCO_STATUS_FRM_CALL);
        mIntentFilter.addAction(
                WifiManager.SUPPLICANT_CONNECTION_CHANGE_ACTION);

        mIntentFilter.addAction(ACTION_CARRIER_SIGNAL_PCO_VALUE_DEBUG);
        mIntentFilter.addAction(ACTION_NETWORK_REJECT_CAUSE_DEBUG);

        /**
         * Start up the thread running the service. Note that we create a
         * separate thread because the service normally runs in the process's
         * main thread, which we don't want to block receiver's thread.
         * */
        HandlerThread thread = new HandlerThread("PCOServiceBRThread",
                Process.THREAD_PRIORITY_BACKGROUND);
        thread.start();
        sServiceLooper = thread.getLooper();
        sHandler = new Handler(sServiceLooper);

        sContext.registerReceiver(mBroadcastReceiver, mIntentFilter, null, sHandler);
        createNotification();
    }

    /**
     * It is not used. onStartCommant to start the service
     * @param intent intent
     * @param flags flags
     * @param startId startid
     * @return
     */
    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        log("[v]onStartCommand: " + intent + " " + startId);
        handlePCOcase(sContext, intent);
        return Service.START_REDELIVER_INTENT;
    }

    private void createNotification() {
        log("[+]adding the notification ");
        NotificationChannel chPCONotification = new NotificationChannel("pco",
                "pco.service",
                NotificationManager.IMPORTANCE_NONE);
        chPCONotification.setDescription("PCO check");
        chPCONotification.enableLights(false);
        chPCONotification.enableVibration(false);
        NotificationManager notificationManager = (NotificationManager) sContext
                .getSystemService(Context.NOTIFICATION_SERVICE);
        notificationManager.createNotificationChannel(chPCONotification);
        Notification.Builder builder = new Notification.Builder(this)
                .setSmallIcon(android.R.drawable.ic_dialog_info)
                .setContentTitle("PCO").setContentText("Checking PCO status.")
                .setChannelId("pco").setAutoCancel(true);
        Notification notification = builder.build();
        startForeground(1, notification);
        stopForeground(STOP_FOREGROUND_REMOVE);
    }

    private void unregisterIntents() {
        log("[-]unregister: NW Broadcast receivers");
        if (mBroadcastReceiver != null) {
            sContext.unregisterReceiver(mBroadcastReceiver);
        }

        if (sServiceLooper != null) {
            sServiceLooper.quit();
        }
    }

    /**
     *
     */
    @Override
    public void onDestroy() {
        log("[x]onDestroy");
        unregisterIntents();

    }

    static final String ACTION_PCO_CHECK = "pco-check";

    private final BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {

        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            log("[<==]Received Broadcast with action = " + action);

//            switch (action) {
//            case Intent.ACTION_BOOT_COMPLETED:
//                break;
//            case TelephonyIntents.ACTION_SIM_STATE_CHANGED:
//                handleSimStateChange(context, intent);
//                break;
//            case TelephonyIntents.ACTION_RADIO_STATE_CHANGED:
//                int intRadioState = intent.getIntExtra("radioState", TelephonyManager.RADIO_POWER_ON);
//                boolean isradioON = PCONwUtils.getRadioStateFromInt(intRadioState);
//                log("[?]isradioON=" + isradioON);
//                if (isradioON) {
//                    checkPCOActivationStatus();
//                }
//                break;
//            case WifiManager.WIFI_STATE_CHANGED_ACTION:
//                int wifiState = intent.getIntExtra(
//                        WifiManager.EXTRA_WIFI_STATE, 0);
//                    log("[?]wifiState=" + wifiState);
//                    if (wifiState == WifiManager.WIFI_STATE_ENABLED) {
//                        checkPCOActivationStatus();
//                    }
//                break;
//            case ACTION_CARRIER_SIGNAL_PCO_VALUE_DEBUG:
//            case TelephonyIntents.ACTION_CARRIER_SIGNAL_PCO_VALUE:
//                handlePCOcase(context, intent);
//                break;
//            case ACTION_NETWORK_REJECT_CAUSE_DEBUG:
//            case TelephonyIntents.ACTION_NETWORK_REJECT_CAUSE:
//                handlePCOEMMcase(context, intent);
//                break;
//            case Constants.ACTION_QUERY_IMS_PCO_STATUS_FRM_BROWSER:
//             // Skips the loop and disable further retry
//                if (sSubOPBrowser == OPSUB_BROWSER_OPENNING) {
//                    sSubOPBrowser = OPSUB_BROWSER_DISABLE_CHECKSTATUS_UNTIL_REBOOT;
//                    return;
//                }
//                checkPCOActivationStatus();
//                break;
//            case Constants.ACTION_QUERY_IMS_PCO_STATUS_FRM_CALL:
//                // Skips the loop and disable further retry
//                if (sSubOPCall == OPSUB_CALL_OPENNING) {
//                    sSubOPBrowser = OPSUB_CALL_DISABLE_CHECKSTATUS_UNTIL_REBOOT;
//                    return;
//                }
//                checkPCOActivationStatus();
//                break;
//            case Constants.ACTION_QUERY_IMS_PCO_STATUS_FRM_NOTIFICATION: {
//                checkPCOActivationStatus();
//            }
//            break;
//            default:
//                break;
//            }
        }
    };

      private void checkPCOActivationStatus() {
          switch (getImsPcoState()) {
          case STATE_PCO_ACTIVATING_PCO_VALUE_3:
              handlePCO3case();
              break;
          case STATE_PCO_ACTIVATING_PCO_VALUE_5:
              handlePCO5case();
              break;
          case STATE_PCO_ACTIVATING_NW_REJECT_EMM_8:
              handlePCOEMM8case();
              break;
          case STATE_PCO_ACTIVATED:
              handleActivatedcase();
              break;
          default:
              break;
          }
      }

      private synchronized void updateImsPcoState(int mUpdatePCOState) {
          if (sStateImsPCO != mUpdatePCOState) {
            log("updateState: from " + stateToString(sStateImsPCO)
                + " to " + stateToString(mUpdatePCOState));
            sStateImsPCO = mUpdatePCOState;
            sPCOInfoPref.savePCOState(sStateImsPCO);
          }
    }

    private void handleSimStateChange(Context context, Intent intent) {
        String simState = intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);
        log("[?]simState:" + simState);
        switch (simState) {
        case IccCardConstants.INTENT_VALUE_ICC_ABSENT:
            /* reset PCO state to default */
            break;
        case IccCardConstants.INTENT_VALUE_ICC_LOADED:
            log("[?]simState:" + simState);
            String newIccid = getIccId();
            String iccid = sPCOInfoPref.getIccid();
            log("[<=>]iccid:" + iccid + ", newIccid:" + newIccid);
            /**
             *  SIM card is changed and reset activation status.
             */
            if (newIccid  != iccid) {
                sPCOInfoPref.saveIccid(newIccid);
                resetImsPcoState();
            }
            break;
        default:
            break;
        }

    }

    public String getImei() {

        if (mImei != null) {
            return mImei;
        }
        String mImei = mTelephonyManager.getImei();
        return mImei;
    }

    private String getIccId() {
        if (mSubscriptionManager == null) {
            log("getIccId(), mSubscriptionManager is null");
            return null;
        }

        int mainCapabilityPhoneId = getMainCapabilityPhoneId();
        SubscriptionInfo subInfo = mSubscriptionManager.getActiveSubscriptionInfoForSimSlotIndex(
                mainCapabilityPhoneId);

        if (subInfo != null) {
            log("ICCID = " + subInfo.getIccId());
            return subInfo.getIccId();
        } else {
            log("getIccId(), subInfo is null");
            return null;
        }
    }

    private int getMainCapabilityPhoneId() {
       int phoneId = SystemProperties.getInt(MtkPhoneConstants.PROPERTY_CAPABILITY_SWITCH, 1) - 1;
       if (phoneId < 0 || phoneId >= TelephonyManager.getDefault().getPhoneCount()) {
           phoneId = SubscriptionManager.INVALID_PHONE_INDEX;
       }
       log("getMainCapabilityPhoneId = " + phoneId);
       return phoneId;
    }

    private void handlePCOEMMcase(Context context, Intent intent) {

        int mNwErrorEMMValue = intent.getIntExtra(TelephonyIntents.EXTRA_EMM_CAUSE, 0);
        //int errorCode = intent.getIntExtra(TelephonyIntents.EXTRA_REJECT_EVENT_TYPE, 0);
        log("Network rejected, emmCause," + "ErrorValue =" + mNwErrorEMMValue);
        if (mNwErrorEMMValue == 8) {
            updateImsPcoState(STATE_PCO_ACTIVATING_NW_REJECT_EMM_8);
            handlePCOEMM8case();
        }
    }

    private void handlePCOEMM8case() {
        log("Handling NW rejected, CauseEMM = 8");
        sPCOInfoPref.saveActivated(false);
            PCONwUtils.turnOffCellularRadio();
          sendMessagetoUI(Constants.ACTION_SIGN_UP,
                Constants.NOT_ACTIVATED,
                Constants.USE_WIFI_NW_ONLY,
                Constants.SCREEN_SHOW_WIFI_DIALOGUE);
    }

    public void handlePCOcase(Context context, Intent intent) {
        Log.i(TAG, "handlePCOcase start");
        String apnType =
                intent.getStringExtra(TelephonyIntents.EXTRA_APN_TYPE_KEY);
        String protoKeyAPN = intent.getStringExtra(TelephonyIntents.EXTRA_APN_PROTO_KEY);
        int pcoIDKey = intent.getIntExtra(TelephonyIntents.EXTRA_PCO_ID_KEY, 0);
        byte[] pcoContent = intent.getByteArrayExtra(TelephonyIntents.EXTRA_PCO_VALUE_KEY);
        if (pcoContent == null || apnType == null) {
            Log.i(TAG, "handlePCOcase pcoContent == null || apnType == null");
            return;
        }

        byte pcoVal = (byte) pcoContent[0];
        int pcoValInt = pcoVal - '0';
        log("apn=" + apnType + "protoKeyAPN=" + protoKeyAPN
                + "pcoVal =" + pcoVal + " pcoValInt = " + pcoValInt);
        switch (apnType) {
        case PhoneConstants.APN_TYPE_IMS: {
            if (pcoValInt == 5) {
                updateImsPcoState(STATE_PCO_ACTIVATING_PCO_VALUE_5);
                handlePCO5case();
            } else if (pcoValInt == 0) {
                handlePCO3case();

            }
        } break;
        default:
            break;
        }

    }

    private void handlePCO3case() {
        log("Handling IMS PCO error case = 3");
        if (sPCOInfoPref.isFirstBoot()) {

            if ((getImsPcoState() == STATE_PCO_UNKNOWN) ||
                    (getImsPcoState() == STATE_PCO_NOT_ACTIVATED)) {
                updateImsPcoState(STATE_PCO_ACTIVATING_PCO_VALUE_3);
            } else {
                handleActivatedcase();
            }

        } else {
            handleActivatedcase();
        }
        return;
        /*
         * sPCOInfoPref.saveActivated(false); PCONwUtils.turnOffCellularRadio();
         * sendMessagetoUI(Constants.ACTION_SIGN_UP, Constants.NOT_ACTIVATED,
         * Constants.USE_MOBILE_NW_ONLY, Constants.SCREEN_SHOW_URL_DIALOGUE); return;
         */
    }

    private void handlePCO5case() {
        log("Handling IMS PCO error case = 5");
        sPCOInfoPref.saveActivated(false);
//        PCONwUtils.turnOnCellularRadio();
        sendMessagetoUI(Constants.ACTION_SIGN_UP,
                Constants.NOT_ACTIVATED,
                Constants.USE_MOBILE_NW_ONLY,
                Constants.SCREEN_SHOW_URL_DIALOGUE);
    }

    private void handleActivatedcase() {
        log("[V]IMS PCO is activated");
        updateImsPcoState(STATE_PCO_ACTIVATED);
        sPCOInfoPref.saveFirstBoot(false);
        sPCOInfoPref.saveActivated(true);
        /**
         *  When SIM card is activated , network  and screen details are
         *  not needed for activity.
         */
        sendMessagetoUI(Constants.ACTION_SIGN_UP_EXIT,
                Constants.ACTIVATED,
                "",
                "");
    }

    private void sendMessagetoUI(String mAction,
            String mActStatus,
            String mNWToUse,
            String mScreentype) {

        log("[==>]sendMessagetoUI" + ", Action=" + mAction
                + ", mNWToUse =" + mNWToUse + ", Screentype=" + mScreentype);
        Intent i = new Intent(sContext, PCOStartUPWizardMainActivity.class);
        i.setAction(mAction);
        i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TASK);
        i.putExtra(Constants.EXTRA_ACTIVATION_STATUS, mActStatus);
        i.putExtra(Constants.EXTRA_NETWORK_TYPE_TO_USE, mNWToUse);
        i.putExtra(Constants.EXTRA_SCREEN_TYPE, mScreentype);
        startActivity(i);
    }

    class IncomingHandler extends Handler {

        /**
         * Receives the messages from activity and process them.
         * @param msg is from activity.
         */
        @Override
        public void handleMessage(Message msg) {

            log("Request from UI, msgid = " + msg.what);

            switch (msg.what) {
            case Constants.MSG_ID_REQ_TRUN_ON_WIFI:
                PCONwUtils.enableWifi();
                break;
            case Constants.MSG_ID_REQ_TRUN_OFF_WIFI:
                break;
            case Constants.MSG_ID_REQ_TRUN_ON_MOBILE_DATA:
                PCONwUtils.turnOnCellularRadio();
                PCONwUtils.enableWirelessConnectivity();
                break;
            case Constants.MSG_ID_REQ_TRUN_OFF_MOBILE_DATA:
                PCONwUtils.turnOffCellularRadio();
                break;
            case Constants.MSG_ID_REQ_CALL_911 :
                sSubOPCall = OPSUB_CALL_OPENNING;
                PCONwUtils.call911();
                break;
            case Constants.MSG_ID_REQ_OPEN_END_POINT_URL :
                sSubOPBrowser = OPSUB_BROWSER_OPENNING;
                String url = PCOConfig.getServerPostPaidUrl();
                PCONwUtils.launchBrowserURL(url,
                        getImei(), getIccId());
                break;
            case Constants.MSG_ID_REQ_OPEN_END_POINT_URL_DISAGREE:
                PCONwUtils.turnOffCellularRadio();
                break;
            case Constants.MSG_ID_REQ_HTTP_PUT :
                break;
            case Constants.MSG_ID_REQ_CHECK_IMS_PCO_STATUS :
                checkPCOActivationStatus();
                break;
            default:
                throw new IllegalStateException("Unknow msg ID from activity");
            }
            super.handleMessage(msg);
        }
    }

    final Messenger mMessenger = new Messenger(new IncomingHandler());

    /**
     *
     * @param intent
     * @return
     */
    @Override
    public IBinder onBind(Intent intent) {
        return mMessenger.getBinder();
    }


    private static void log(String s) {
        if (DEBUG) {
            Log.d(TAG, s);
        }
    }

    private static String stateToString(int state) {
        switch (state) {
            case STATE_PCO_UNKNOWN:
                return "STATE_UNKNOWN_PCO_VALUE";
            case STATE_PCO_NOT_ACTIVATED:
                return "STATE_PCO_NOT_ACTIVATED";
            case STATE_PCO_ACTIVATING_PCO_VALUE_3:
                return "STATE_PCO_ACTIVATING_PCO_VALUE_3";
            case STATE_PCO_ACTIVATING_PCO_VALUE_5:
                return "STATE_PCO_ACTIVATING_PCO_VALUE_5";
            case STATE_PCO_ACTIVATING_NW_REJECT_EMM_8:
                return "STATE_PCO_ACTIVATING_NW_REJECT_EMM_8";
            case STATE_PCO_ACTIVATED:
                return "STATE_PCO_ACTIVATED";
            default:
                return "UNKNOWN state";
        }
    }
}
