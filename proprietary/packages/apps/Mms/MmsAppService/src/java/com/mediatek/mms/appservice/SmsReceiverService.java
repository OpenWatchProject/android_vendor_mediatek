/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2007-2008 Esmertec AG.
 * Copyright (C) 2007-2008 The Android Open Source Project
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

package com.mediatek.mms.appservice;

import static android.content.Intent.ACTION_BOOT_COMPLETED;
import static android.provider.Telephony.Sms.Intents.SMS_DELIVER_ACTION;

import android.app.Activity;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.database.sqlite.SqliteWrapper;
import android.net.Uri;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.Process;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.provider.Telephony;
import android.provider.Telephony.Mms;
import android.provider.Telephony.Sms;
import android.provider.Telephony.Sms.Inbox;
import android.provider.Telephony.Sms.Intents;
import android.provider.Telephony.Sms.Outbox;
import android.telephony.PhoneNumberUtils;
import android.telephony.ServiceState;
import android.telephony.SmsManager;
import android.telephony.SmsMessage;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;
import android.widget.Toast;

import com.android.ims.ImsConfig;
import com.android.ims.ImsManager;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.TelephonyProperties;
import com.mediatek.mms.appservice.MmsConfig;
import com.mediatek.ims.internal.MtkImsManager;
import com.mediatek.internal.telephony.IMtkTelephonyEx;
import com.mediatek.internal.telephony.MtkTelephonyProperties;
import com.mediatek.telephony.MtkTelephonyManagerEx;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.HashMap;
import java.util.Iterator;

import mediatek.telephony.MtkSmsManager;
import mediatek.telephony.MtkTelephony.MtkSms;
import mediatek.telephony.MtkSmsMessage;

import com.mediatek.mmsappservice.ext.IMmsAppServiceSmsReceiverServiceExt;
import com.mediatek.mmsappservice.ext.OpMmsAppServiceCustomizationUtils;

/**
 * This service essentially plays the role of a "worker thread", allowing us to store
 * incoming messages to the database, update notifications, etc. without blocking the
 * main thread that SmsReceiver runs on.
 */
public class SmsReceiverService extends Service { //implements ISmsReceiverServiceCallback {
    private static final String TAG = "SmsReceiverService";

    private ServiceHandler mServiceHandler;
    //private Looper mServiceLooper;
    static HandlerThread sSmsTHandler = null;

    private boolean mSending;

    public static final String MESSAGE_SENT_ACTION =
        "com.android.mms.transaction.MESSAGE_SENT";

    // Indicates next message can be picked up and sent out.
    public static final String EXTRA_MESSAGE_SENT_SEND_NEXT = "SendNextMsg";

    public static final String ACTION_SEND_MESSAGE =
        "com.android.mms.transaction.SEND_MESSAGE";

    public static final String ACTION_SEND_NEXT_MESSAGE =
        "com.mediatek.mmsappservice.SEND_MESSAGE";

    private static final String[] SMS_THREAD_ID_PROJECTION = new String[] { Sms.THREAD_ID };

    // This must match the column IDs below.
    private static final String[] SEND_PROJECTION = new String[] {
        Sms._ID,        //0
        Sms.THREAD_ID,  //1
        Sms.ADDRESS,    //2
        Sms.BODY,       //3
        Sms.STATUS,     //4
        Sms.SUBSCRIPTION_ID,     //5
    };


    // This must match SEND_PROJECTION.
    private static final int SEND_COLUMN_ID         = 0;
    private static final int SEND_COLUMN_THREAD_ID  = 1;
    private static final int SEND_COLUMN_ADDRESS    = 2;
    private static final int SEND_COLUMN_BODY       = 3;
    private static final int SEND_COLUMN_STATUS     = 4;
    private static final int SEND_COLUMN_SUB_ID     = 5;

    private int mResultCode;
  //  private String mSaveLocation = "Phone";
    private HashMap mSaveLocation = new HashMap();
    private static HashSet<Integer> sNoServiceSimSet = new HashSet<Integer>();
    private static int lastIncomingSmsSubId = -1;

    private IMmsAppServiceSmsReceiverServiceExt mOpSmsReceiverServiceExt = null;

    @Override
    public void onCreate() {
        // Temporarily removed for this duplicate message track down.
        Log.v(TAG, "onCreate");
        // Start up the thread running the service.  Note that we create a
        // separate thread because the service normally runs in the process's
        // main thread, which we don't want to block.
        if (sSmsTHandler == null) {
            sSmsTHandler = new HandlerThread(TAG, Process.THREAD_PRIORITY_BACKGROUND);
            sSmsTHandler.start();
        }
        //mServiceLooper = thread.getLooper();
        mServiceHandler = new ServiceHandler(sSmsTHandler.getLooper());
        /// M:
        mOpSmsReceiverServiceExt = OpMmsAppServiceCustomizationUtils.getOpFactory(getApplicationContext())
                .makeOpMmsAppServiceSmsReceiverServicePluginExt();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
    // mark by hongduo,can be move to app side
   //     if (!isSmsEnabled(this)) {
   //         Log.d(TAG, "SmsReceiverService: is not the default sms app");
            // NOTE: We MUST not call stopSelf() directly, since we need to
            // make sure the wake lock acquired by AlertReceiver is released.
     //       SmsReceiver.finishStartingService(SmsReceiverService.this, startId);
  //          return Service.START_NOT_STICKY;
  //      }
        // Temporarily removed for this duplicate message track down.
        Log.d(LogTag.TXN_TAG, "onStartCommand SmsReceiverService");
        Context appContext = getApplicationContext();
        if (intent != null && ACTION_SEND_MESSAGE.endsWith(intent.getAction())) {
            MmsAppCallback.getInstance(appContext).bindAppCallbackService(appContext);//(this);
        }
        // KK migration, for default MMS function.
        // The changes in this project update the in-box SMS/MMS app to use the new
        // intents for SMS/MMS delivery.
        if (intent != null && SMS_DELIVER_ACTION.equals(intent.getAction())) {
            Log.d(LogTag.TXN_TAG, "Sms recevied and need not care mResultCode.");
        } else if (intent != null &&
                TelephonyIntents.ACTION_SERVICE_STATE_CHANGED.equals(intent.getAction())) {
            Log.d(LogTag.TXN_TAG, "Sms service state changed and need not care mResultCode.");
            boolean precheck = serviceStateChangedPrecheck(intent);
            Log.d(LogTag.TXN_TAG, "Sms service state changed, precheck = " + precheck);
            if (precheck == true) {
                MmsAppCallback.getInstance(appContext).bindAppCallbackService(appContext);//(this);
            }
            intent.putExtra("service_state_precheck", precheck);
        }
        /// M: WFC: Handle Ims capability change @{
        else if (ImsManager.isWfcEnabledByPlatform(getApplicationContext())
                && intent != null) {
            Log.d(LogTag.TXN_TAG, "Ims state changed and need not care mResultCode.");
        }
        /// @}
        else {
            mResultCode = intent != null ? intent.getIntExtra("result", 0) : 0;
        }
        /// M:Code analyze 007, print the log @{
        if (intent != null && MESSAGE_SENT_ACTION.equals(intent.getAction())) {
            Log.d(LogTag.TXN_TAG, "Message Sent Result Code = " + mResultCode);
            MmsAppCallback.getInstance(appContext).bindAppCallbackService(appContext); //(this);
        }
        /// @}
        int subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY,
                SubscriptionManager.INVALID_SUBSCRIPTION_ID);

        mSaveLocation.put(subId, intent.getStringExtra("save_location"));

        if (mResultCode != 0) {
            Log.i(TAG, "onStart: #" + startId + " mResultCode: " + mResultCode +
                    " = " + translateResultCode(mResultCode));
        }

        Message msg = mServiceHandler.obtainMessage();
        msg.arg1 = startId;
        msg.obj = intent;
        mServiceHandler.sendMessage(msg);
        return Service.START_NOT_STICKY;
    }

    private static String translateResultCode(int resultCode) {
        switch (resultCode) {
            case Activity.RESULT_OK:
                return "Activity.RESULT_OK";
            case SmsManager.RESULT_ERROR_GENERIC_FAILURE:
                return "SmsManager.RESULT_ERROR_GENERIC_FAILURE";
            case SmsManager.RESULT_ERROR_RADIO_OFF:
                return "SmsManager.RESULT_ERROR_RADIO_OFF";
            case SmsManager.RESULT_ERROR_NULL_PDU:
                return "SmsManager.RESULT_ERROR_NULL_PDU";
            case SmsManager.RESULT_ERROR_NO_SERVICE:
                return "SmsManager.RESULT_ERROR_NO_SERVICE";
            case SmsManager.RESULT_ERROR_LIMIT_EXCEEDED:
                return "SmsManager.RESULT_ERROR_LIMIT_EXCEEDED";
            case SmsManager.RESULT_ERROR_FDN_CHECK_FAILURE:
                return "SmsManager.RESULT_ERROR_FDN_CHECK_FAILURE";
            default:
                return "Unknown error code";
        }
    }

    @Override
    public void onDestroy() {
        // Temporarily removed for this duplicate message track down.
//        if (Log.isLoggable(LogTag.TRANSACTION, Log.VERBOSE) ) {
//            Log.v(TAG, "onDestroy");
//        }
        //mServiceLooper.quit();
        Log.d(LogTag.TXN_TAG, "onDestroy SmsReceiverService");
        Context context = getApplicationContext();
        MmsAppCallback.getInstance(context).unbindAppCallbackService(context);
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    private final class ServiceHandler extends Handler {
        public ServiceHandler(Looper looper) {
            super(looper);
        }

        /**
         * Handle incoming transaction requests.
         * The incoming requests are initiated by the MMSC Server or by the MMS Client itself.
         */
        @Override
        public void handleMessage(Message msg) {
            /// M:
            Log.i(LogTag.TXN_TAG, "Sms handleMessage :" + msg);
            int serviceId = msg.arg1;
            Intent intent = (Intent) msg.obj;
            if (Log.isLoggable(LogTag.TRANSACTION, Log.VERBOSE)) {
                Log.v(TAG, "handleMessage serviceId: " + serviceId + " intent: " + intent);
            }
            // KK migration, for default MMS function.  //Hongduo
            if (intent != null /*&& MmsConfig.isSmsEnabled(getApplicationContext()) */) {
                String action = intent.getAction();

                int error = intent.getIntExtra("errorCode", 0);

                if (Log.isLoggable(LogTag.TRANSACTION, Log.VERBOSE)) {
                    Log.v(TAG, "handleMessage action: " + action + " error: " + error);
                }

                if (MESSAGE_SENT_ACTION.equals(intent.getAction())) {
                    handleSmsSent(intent, error);
                } else if (SMS_DELIVER_ACTION.equals(action)) {
                    handleSmsReceived(intent, error);
                } else if (ACTION_BOOT_COMPLETED.equals(action)) {
                    handleBootCompleted();
                } else if (TelephonyIntents.ACTION_SERVICE_STATE_CHANGED.equals(action)) {
                  //fix me, the intent will add two other extra value
                    handleServiceStateChanged(intent);
                } else if (ACTION_SEND_MESSAGE.endsWith(action)) {
                    handleSendMessage(intent);
                } else if (ImsManager.isWfcEnabledByPlatform(getApplicationContext())) {
                    handleImsCapabilityChange(intent);
                }
                /// @}
            }
            // NOTE: We MUST not call stopSelf() directly, since we need to
            // make sure the wake lock acquired by AlertReceiver is released.
            stopSelf(serviceId);
            MmsConfig.finishStartingService(getApplicationContext(),"sms");
        }
    }

    /// M: WFC: Handle Ims capability change @{
    /*
    private void handleImsCapabilityChange(Intent intent) {
        int[] enabledFeatures = intent.getIntArrayExtra(MtkImsManager.EXTRA_IMS_ENABLE_CAP_KEY);
        Log.d(LogTag.TXN_TAG, "WFC capability:" + enabledFeatures[ImsConfig.FeatureConstants
                .FEATURE_TYPE_VOICE_OVER_WIFI]);
        if (enabledFeatures[ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_WIFI] ==
                ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_WIFI) {
            if (!mSending) {
                sendFirstQueuedMessage();
            }
        }
    }
    */
    /// @}

    private void handleImsCapabilityChange(Intent intent) {
        ServiceState serviceState = ServiceState.newFromBundle(intent.getExtras());
        int subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY,
            SubscriptionManager.INVALID_SUBSCRIPTION_ID);
        Log.d(LogTag.TXN_TAG, "handleImsCapabilityChange subId" + subId);
        int stateData = ServiceState.STATE_OUT_OF_SERVICE ;
        boolean checkIms = MtkTelephonyManagerEx.getDefault().isImsRegistered(subId);

        if (checkIms == true) {
            stateData = serviceState.getDataRegState();
            Log.d(LogTag.TXN_TAG, "serviceState.getDataRegState" + stateData);

            if (stateData == ServiceState.STATE_IN_SERVICE) {
              Log.d(LogTag.TXN_TAG, "handleImsCapabilityChange STATE_IN_SERVICE");
              if (!mSending) {
                   sendFirstQueuedMessage();
                }
            }
        }
    }

    private void handleServiceStateChanged(Intent intent) {
        /// M:
        Log.i(LogTag.TXN_TAG, "Sms handleServiceStateChanged");
        boolean precheck = intent.getBooleanExtra("service_state_precheck", false);
        Log.d(LogTag.TXN_TAG, "service_state_precheck" + precheck);
        if (precheck == false) {
            return;
        }
        // If service just returned, start sending out the queued messages
        ServiceState serviceState = ServiceState.newFromBundle(intent.getExtras());
        int subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY,
                SubscriptionManager.INVALID_SUBSCRIPTION_ID);
        if (!SubscriptionManager.isValidSubscriptionId(subId)) {
            Log.d(TAG, "subId in handleServiceStateChanged() is invalid!" + subId);
            return;
        }

        int stateVoice = serviceState.getVoiceRegState();
        Log.d(LogTag.TXN_TAG, "serviceState.getVoiceRegState" + stateVoice);
        int stateData = ServiceState.STATE_OUT_OF_SERVICE ;
        boolean checkIms = MtkTelephonyManagerEx.getDefault().isImsRegistered(subId);

        if (checkIms == true) {
            stateData = serviceState.getDataRegState();
            Log.d(LogTag.TXN_TAG, "serviceState.getDataRegState" + stateData);
        }

        if ((stateVoice == ServiceState.STATE_IN_SERVICE ||
                stateData == ServiceState.STATE_IN_SERVICE)
                && sNoServiceSimSet.contains(subId)) {
            sNoServiceSimSet.remove(subId);
            if (!mSending) {
                sendFirstQueuedMessage();
            }
        }
    }

    private boolean serviceStateChangedPrecheck(Intent intent) {
        Log.i(LogTag.TXN_TAG, "Sms serviceStateChangedPrecheck");
        // If service just returned, start sending out the queued messages
        ServiceState serviceState = ServiceState.newFromBundle(intent.getExtras());
        int subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY,
                SubscriptionManager.INVALID_SUBSCRIPTION_ID);
        if (!SubscriptionManager.isValidSubscriptionId(subId)) {
            Log.d(TAG, "subId in serviceStateChangedPrecheck() is invalid!" + subId);
            return false;
        }
        int stateVoice = serviceState.getVoiceRegState();
        Log.d(LogTag.TXN_TAG, "serviceState.getVoiceRegState" + stateVoice);
        int stateData = ServiceState.STATE_OUT_OF_SERVICE ;
        boolean checkIms = MtkTelephonyManagerEx.getDefault().isImsRegistered(subId);

        if (checkIms == true) {
            stateData = serviceState.getDataRegState();
            Log.d(LogTag.TXN_TAG, "serviceState.getDataRegState" + stateData);
        }

        if ((stateVoice == ServiceState.STATE_IN_SERVICE ||
                stateData == ServiceState.STATE_IN_SERVICE)
                && sNoServiceSimSet.contains(subId)) {
            Log.d(LogTag.TXN_TAG, "Sms serviceStateChangedPrecheck mSending = " + mSending);
            //sNoServiceSimSet.remove(subId);
            if (!mSending) {
                //sendFirstQueuedMessage();
                return true;
            }
        }
        Log.d(LogTag.TXN_TAG, "Sms serviceStateChangedPrecheck mSending return false");
        return false;
    }

    private void handleSendMessage(Intent intent) {
        Log.d(LogTag.TXN_TAG, "handleSendMessage() mSending = " + mSending);
        MmsConfig.setEmailGateway(intent.getStringExtra("email_address"));
        MmsConfig.setAliasEnabled(intent.getBooleanExtra("alias_enabled", false));
        MmsConfig.setAliasMinChars(intent.getIntExtra("alias_minchar", 2));
        MmsConfig.setAliasMaxChars(intent.getIntExtra("alias_maxchar", 48));
        MmsConfig.setEncodingType(intent.getIntExtra("encoding_type", 0));
        if (!mSending) {
            sendFirstQueuedMessage();
        }
    }

    private void handleSendInactiveMessage() {
        // Inactive messages includes all messages in outbox and queued box.
        moveOutboxMessagesToQueuedBox();
        sendFirstQueuedMessage();
    }

    public synchronized void sendFirstQueuedMessage() {
        /// M:
    //    boolean isSmsEnabled = TransactionService.isSmsEnabled(this);
    //    Log.d(LogTag.TXN_TAG, "sendFirstQueuedMessage(), isSmsEnabled = " + isSmsEnabled);
    //    if (!isSmsEnabled) {
    //        return;
    //    }
        boolean success = true;
        // get all the queued messages from the database
        final Uri uri = Uri.parse("content://sms/queued");

        String selection = null;
        selection = Sms.SUBSCRIPTION_ID + " NOT IN " + "(" + getNoServiceSimString() + ")";
/*
        /// M: WFC: Apply this selection if WFC disabled by platform or user
        if (!(ImsManager.isWfcEnabledByPlatform(getApplicationContext())
                && ImsManager.isWfcEnabledByUser(getApplicationContext()))) {
            //Add for avoiding to send message on No Service Sim card
            selection = Sms.SUBSCRIPTION_ID + " NOT IN " + "(" + getNoServiceSimString() + ")";
        }
*/
        ContentResolver resolver = getContentResolver();
        Cursor c = SqliteWrapper.query(this, resolver, uri,
                        SEND_PROJECTION, selection, null, "date ASC");// date ASC so we send out in
                                                                    // same order the user tried
                                                                    // to send messages.
        if (c != null) {
            try {
                Log.d(LogTag.TXN_TAG, "sendFirstQueuedMessage cursor count = " + c.getCount());
                if (c.moveToFirst()) {
                    String msgText = c.getString(SEND_COLUMN_BODY);
                    String address = c.getString(SEND_COLUMN_ADDRESS);
                    int threadId = c.getInt(SEND_COLUMN_THREAD_ID);
                    int status = c.getInt(SEND_COLUMN_STATUS);
                    int subId = c.getInt(SEND_COLUMN_SUB_ID);

                    int msgId = c.getInt(SEND_COLUMN_ID);
                    Uri msgUri = ContentUris.withAppendedId(Sms.CONTENT_URI, msgId);

                    SmsSingleRecipientSender sender = new SmsSingleRecipientSender(this,
                            address, msgText, threadId, status == Sms.STATUS_PENDING,
                            msgUri, subId);
                    if (Log.isLoggable(LogTag.TRANSACTION, Log.VERBOSE)) {
                        Log.v(TAG, "sendFirstQueuedMessage " + msgUri +
                                ", address: " + address +
                                ", threadId: " + threadId +
                                /// M: print more info
                                ", body: " + msgText);
                    }

                    try {
                        sender.sendMessage();
                        mSending = true;
                    } catch (Exception e) {
                        Log.e(TAG, "sendFirstQueuedMessage: failed to send message " + msgUri
                                + ", caught ", e);
                        mSending = false;
                        messageFailedToSend(msgUri, SmsManager.RESULT_ERROR_GENERIC_FAILURE);
                        success = false;
                        // Sending current message fails. Try to send more pending messages
                        // if there is any.
                        sendBroadcast(new Intent(ACTION_SEND_NEXT_MESSAGE,
                                null).setClassName("com.android.mms",
                                "com.android.mms.transaction.SmsReceiver"));
                    }
                }
            } finally {
                c.close();
            }
        }
        if (success) {
            // We successfully sent all the messages in the queue. We don't need to
            // be notified of any service changes any longer.
            unRegisterForServiceStateChanges();
        }
    }

    private void handleSmsSent(Intent intent, int error) {
        /// M:
        int resultCode = intent != null ? intent.getIntExtra("result", 0) : 0;
        Log.d(LogTag.TXN_TAG, "handleSmsSent(), errorcode= "
                + error + "resultCode=" + resultCode);
        Uri uri = intent.getData();
        mSending = false;
        boolean sendNextMsg = intent.getBooleanExtra(EXTRA_MESSAGE_SENT_SEND_NEXT, false);

        if (Log.isLoggable(LogTag.TRANSACTION, Log.VERBOSE)) {
            Log.v(TAG, "handleSmsSent uri: " + uri + " sendNextMsg: " + sendNextMsg +
                    " resultCode: " + resultCode +
                    " = " + translateResultCode(resultCode) + " error: " + error);
        }

        int subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY,
                SubscriptionManager.INVALID_SUBSCRIPTION_ID);
        if (!SubscriptionManager.isValidSubscriptionId(subId)) {
            Log.e(TAG, "subId in handleSmsSent() is invalid!");
            return;
        }

        int phoneId = SubscriptionManager.getPhoneId(subId);
        String inEcm = TelephonyManager.getTelephonyProperty(phoneId,
                MtkTelephonyProperties.PROPERTY_INECM_MODE_BY_SLOT, null);

        if (resultCode == Activity.RESULT_OK) {
            /// M:
            Log.d(LogTag.TXN_TAG, "handleSmsSent(), result is RESULT_OK");
            if (Log.isLoggable(LogTag.TRANSACTION, Log.VERBOSE)) {
                Log.v(TAG, "handleSmsSent move message to sent folder uri: " + uri);
            }
            /// M:Code analyze 016,change logic for just only
            /// checking one part of long sms is sent failed
            /// or not,if yes,means this long sms is sent failed,no need move it to other boxes @{
            if (sendNextMsg) { //this is the last part of a sms.a long sms's part is sent ordered.
                Cursor cursor = SqliteWrapper.query(this, getContentResolver(),
                                                    uri, new String[] {Sms.TYPE}, null, null, null);
                if (cursor != null) {
                    try {
                        if ((cursor.getCount() == 1) && cursor.moveToFirst()) {
                            int smsType = 0;
                            smsType = cursor.getInt(0);
                            // if smsType is failed, that means at least one
                            // part of this long sms is sent failed.
                            // then this long sms is sent failed.
                            // so we shouldn't move it to other boxes.just keep it in failed box.
                            if (smsType != Sms.MESSAGE_TYPE_FAILED) {
                                //move sms from out box to sent box
                                if (!Sms.moveMessageToFolder(this,
                                        uri, Sms.MESSAGE_TYPE_SENT, error)) {
                                    Log.e(TAG, "handleSmsSent: failed to move message "
                                            + uri + " to sent folder");
                                }
                            }
                        }
                    } finally {
                        cursor.close();
                    }
                }
            }
            /// @}
            if (sendNextMsg) {
                sendFirstQueuedMessage();
            }

            // Update the notification for failed messages since they may be deleted.
           // mark by hongduo
          //  MessagingNotification.nonBlockingUpdateSendFailedNotification(this);
          MmsConfig.updateMessagingNotification(this, MmsConfig.CANCEL_SEND_FAILED, 0, false, null);
        } else if ((!Boolean.parseBoolean(inEcm)) &&
                ((resultCode == SmsManager.RESULT_ERROR_RADIO_OFF)
                || (resultCode == SmsManager.RESULT_ERROR_NO_SERVICE))) {
            /// M:Code analyze 018,print error type @{
            if (resultCode == SmsManager.RESULT_ERROR_RADIO_OFF) {
                Log.d(LogTag.TXN_TAG, "handleSmsSent(), result is RESULT_ERROR_RADIO_OFF");
            } else if (resultCode == SmsManager.RESULT_ERROR_NO_SERVICE) {
                Log.d(LogTag.TXN_TAG, "handleSmsSent(), result is RESULT_ERROR_NO_SERVICE");
            } else {
                Log.d(LogTag.TXN_TAG, "handleSmsSent(), radio state is start");
            }
            /// @}
            if (Log.isLoggable(LogTag.TRANSACTION, Log.VERBOSE)) {
                Log.v(TAG, "handleSmsSent: no service, queuing message w/ uri: " + uri);
            }

            registerForServiceStateChanges(subId);
            // We couldn't send the message, put in the queue to retry later.
            /// M:Code analyze 019, if smsType is failed, that means at least one part of this long
            /// sms is sent failed.then this long sms is sent failed. @{
            Cursor cursor = SqliteWrapper.query(this, getContentResolver(),
                                                    uri, new String[] {Sms.TYPE}, null, null, null);
            if (cursor != null) {
                try {
                    if ((cursor.getCount() == 1) && cursor.moveToFirst()) {
                        int smsType = 0;
                        smsType = cursor.getInt(0);
                        Log.d(LogTag.TXN_TAG, "This SMS " + uri + " = " + smsType);
                        // if smsType is failed, that means at least one
                        // part of this long sms is sent failed.
                        // then this long sms is sent failed.
                        // so we shouldn't move it to other boxes.just keep it in failed box.
                        /* If SMS succeeded, just ignore this but not put it into queued box. */
                        if (smsType == Sms.MESSAGE_TYPE_SENT) {
                            Log.d(LogTag.TXN_TAG, "This SMS " + uri + " is sent not, no need queue it again.");
                        } else if (smsType != Sms.MESSAGE_TYPE_FAILED) {
                            Sms.moveMessageToFolder(this, uri, Sms.MESSAGE_TYPE_QUEUED, error);
                            Log.d(LogTag.TXN_TAG, "move message " + uri + " to queue folder");
                            MmsConfig.showToast(this, MmsConfig.MESSAGE_QUEUED);
                        } else {
                            Log.d(LogTag.TXN_TAG, "One or more part was failed," +
                                " should not move to queue folder.");
                        }
                    }
                } finally {
                    cursor.close();
                }
            }
            /// @}

        } else {
            //ALPS03198359/ALPS03200348 : Update status only when it is in outbox.
            //Since msg-Id of deleted message will be taken by new message .
            //So on update from URI wrong message(new) will be updated
            Cursor cursor = SqliteWrapper.query(this, getContentResolver(),
                                                    uri, new String[] {Sms.TYPE}, null, null, null);
            if (cursor != null) {
                try {
                    if ((cursor.getCount() == 1) && cursor.moveToFirst()) {
                        int smsType = 0;
                        smsType = cursor.getInt(0);
                        if (smsType == Sms.MESSAGE_TYPE_OUTBOX) {
                            messageFailedToSend(uri, error);
                            if (resultCode == SmsManager.RESULT_ERROR_FDN_CHECK_FAILURE) {
                                MmsConfig.showToast(this, MmsConfig.FDN_CHECK_FAILURE);
                            }
                        }
                     }
                 } finally {
                        cursor.close();
                 }
            }

            if (sendNextMsg) {
                sendFirstQueuedMessage();
            }
        }
    }

    private void messageFailedToSend(Uri uri, int error) {
        /// M:
        Log.d(LogTag.TXN_TAG, "messageFailedToSend(),uri=" + uri + "\terror=" + error);
        if (Log.isLoggable(LogTag.TRANSACTION, Log.VERBOSE)) {
            Log.v(TAG, "messageFailedToSend msg failed uri: " + uri + " error: " + error);
        }
        Sms.moveMessageToFolder(this, uri, Sms.MESSAGE_TYPE_FAILED, error);
        /// M:Code analyze 021, update sms status when failed. this Sms.STATUS is used for
        /// delivery report. @{
        ContentValues contentValues = new ContentValues(1);
        contentValues.put(Sms.STATUS, Sms.STATUS_FAILED);
        /// M: BUGFIX:560953; if user has not request delivery report,
        /// then will not update the status;
        SqliteWrapper.update(this, this.getContentResolver(),
                uri, contentValues, Sms.STATUS + " = ? ",
                new String[] {Sms.STATUS_PENDING + ""});
        /// @}
        MmsConfig.updateFailedNotification(getApplicationContext(), false, 0, true);
   //     MessagingNotification.notifySendFailed(getApplicationContext(), true);
    //    MmsWidgetProvider.notifyDatasetChanged(getApplicationContext());
    }

    private void handleSmsReceived(Intent intent, int error) {
        //SmsMessage[] msgs = Intents.getMessagesFromIntent(intent);
        MtkSmsMessage[] msgs = MtkSms.Intents.getMessagesFromIntent(intent);
        MmsConfig.setAutoDeleteConfig(intent);
        /// M:Code analyze 022, check null @{
        if (msgs == null) {
            Log.e(LogTag.TXN_TAG, "getMessagesFromIntent return null.");
            return;
        }
        //Log.d(LogTag.TXN_TAG, "handleSmsReceived SmsReceiverService");
        /// @}

        String format = intent.getStringExtra("format");
        /// M:Code analyze 023, the second argument is change for passing simId info @{
        Uri messageUri = null;
   //     messageUri = mOpSmsReceiverServiceExt.handleSmsReceived(getApplicationContext(), msgs,
   //             intent, error);

        //if (messageUri == null) {
        try {
            messageUri = insertMessage(this, msgs, error, format);
        } catch (IllegalArgumentException e) {
            Log.e(TAG, "Save message fail:" + e.getMessage(), e);
            return;
        }
        //}

        if (Log.isLoggable(LogTag.TRANSACTION, Log.VERBOSE)) {
            SmsMessage sms = (SmsMessage) msgs[0];
            Log.v(TAG, "handleSmsReceived" + (sms.isReplace() ? "(replace)" : "") +
                    " messageUri: " + messageUri +
                    ", address: " + sms.getOriginatingAddress() +
                    ", body: " + sms.getMessageBody());
        }
        /// M:Code analyze 024, print log @{
        SmsMessage tmpsms = (SmsMessage) msgs[0];
        MmsLog.ipi(LogTag.TXN_TAG, "handleSmsReceived" + (tmpsms.isReplace() ? "(replace)" : "")
            + " messageUri: " + messageUri
            + ", address: " + tmpsms.getOriginatingAddress());
        /// @}

        if (messageUri != null) {
            long threadId = getSmsThreadId(this, messageUri);
            MmsConfig.updateMessagingNotification(this, MmsConfig.RECEIEVE_NEW, threadId, false, messageUri);
            /// @}
        /// M:Code analyze 026, add else branch,that means this message is not saved successfully,
        /// because its type is class 0,just show on phone and no need to save @{
        } else {
            SmsMessage sms = (SmsMessage) msgs[0];
            if (sms == null) {
                Log.e(LogTag.TXN_TAG, "msgs[0] null");
                return;
            }
            SmsMessage msg = SmsMessage.createFromPdu(sms.getPdu(), format);
            if (msg == null) {
                Log.e(LogTag.TXN_TAG, "createFromPdu return null.");
                return;
            }
            CharSequence messageChars = msg.getMessageBody();
            if (messageChars == null) {
                Log.e(LogTag.TXN_TAG, "getMessageBody return null.");
                return;
            }
            String message = messageChars.toString();
//            if (!TextUtils.isEmpty(message)) {
//                MmsConfig.updateClassZeroNotification(this, msgs[0].getOriginatingAddress());
//            }
        }
        /// @}
    }

// should not be called
    private void handleBootCompleted() {
        Log.i(LogTag.TXN_TAG, "handleBootCompleted");
        // fix first launch performance issue ALPS01760483, start a service to avoid mms killed.
    // mark by hongduo,can be delete
    //    Intent intent = new Intent();
    //    intent.setClass(SmsReceiverService.this, NoneService.class);
   //     startService(intent);

        // Some messages may get stuck in the outbox or queued.
        // At this point, they're probably irrelevant
        // to the user, so mark them as failed and notify the user, who can then decide whether to
        // resend them manually.
        int numMoved = moveOutboxMessagesToFailedBox();
        numMoved = numMoved + moveQueuedMessagesToFailedBox();
        if (numMoved > 0) {
      //      MessagingNotification.notifySendFailed(getApplicationContext(), true);
           MmsConfig.updateFailedNotification(getApplicationContext(), false, 0, true);
        }

        // Send any queued messages that were waiting from before the reboot.
        sendFirstQueuedMessage();

        // Called off of the UI thread so ok to block.
        MmsConfig.updateMessagingNotification(this, MmsConfig.RECEIEVE_NEW, MmsConfig.THREAD_ALL, false, null);
    }

    /**
     * Move all messages that are in the outbox to the queued state
     * @return The number of messages that were actually moved
     */
    private int moveOutboxMessagesToQueuedBox() {
        ContentValues values = new ContentValues(1);

        values.put(Sms.TYPE, Sms.MESSAGE_TYPE_QUEUED);

        int messageCount = SqliteWrapper.update(
                getApplicationContext(), getContentResolver(), Outbox.CONTENT_URI,
                values, "type = " + Sms.MESSAGE_TYPE_OUTBOX, null);
        if (Log.isLoggable(LogTag.TRANSACTION, Log.VERBOSE)) {
            Log.v(TAG, "moveOutboxMessagesToQueuedBox messageCount: " + messageCount);
        }
        return messageCount;
    }

    /**
     * Move all messages that are in the outbox to the failed state and set them to unread.
     * @return The number of messages that were actually moved
     */
    private int moveOutboxMessagesToFailedBox() {
        ContentValues values = new ContentValues(3);

        values.put(Sms.TYPE, Sms.MESSAGE_TYPE_FAILED);
        values.put(Sms.ERROR_CODE, SmsManager.RESULT_ERROR_GENERIC_FAILURE);
        values.put(Sms.READ, Integer.valueOf(0));

        int messageCount = SqliteWrapper.update(
                getApplicationContext(), getContentResolver(), Outbox.CONTENT_URI,
                values, "type = " + Sms.MESSAGE_TYPE_OUTBOX, null);
        if (Log.isLoggable(LogTag.TRANSACTION, Log.VERBOSE)) {
            Log.v(TAG, "moveOutboxMessagesToFailedBox messageCount: " + messageCount);
        }
        return messageCount;
    }

    public static final String CLASS_ZERO_BODY_KEY = "CLASS_ZERO_BODY";

    // This must match the column IDs below.
    private final static String[] REPLACE_PROJECTION = new String[] {
        Sms._ID,
        Sms.ADDRESS,
        Sms.PROTOCOL
    };

    // This must match REPLACE_PROJECTION.
    private static final int REPLACE_COLUMN_ID = 0;

    /// M:Code analyze 023, the second argument is changed for passing simId info @{
    /**
     * If the message is a class-zero message, display it immediately
     * and return null.  Otherwise, store it using the
     * <code>ContentResolver</code> and return the
     * <code>Uri</code> of the thread containing this message
     * so that we can use it for notification.
     */
    private Uri insertMessage(Context context, SmsMessage[] msgs, int error, String format) {
        // Build the helper classes to parse the messages.
        if (msgs == null) {
            Log.e(LogTag.TXN_TAG, "insertMessage:getMessagesFromIntent return null.");
            return null;
        }
        /// @}
        SmsMessage sms = msgs[0];
        if (sms == null) {
            return null;
        }

        if (sms.getMessageClass() == SmsMessage.MessageClass.CLASS_0) {
            Log.d(LogTag.TXN_TAG, "insertMessage: display class 0 message!");
            displayClassZeroMessage(context, msgs, format);
            return null;
        } else if (sms.isReplace()) {
            Log.d(LogTag.TXN_TAG, "insertMessage: is replace message!");
            return replaceMessage(context, msgs, error);
        } else {
            Log.d(LogTag.TXN_TAG, "insertMessage: stored directly!");
            return storeMessage(context, msgs, error);
        }
    }

    /**
     * If the message is a class-zero message, display it immediately
     * and return null.  Otherwise, store it using the
     * <code>ContentResolver</code> and return the
     * <code>Uri</code> of the thread containing this message
     * so that we can use it for notification.
     * This is the version to handle MtkSmsMessages
     */
    private Uri insertMessage(Context context, MtkSmsMessage[] msgs, int error, String format) {
        // Build the helper classes to parse the messages.
        if (msgs == null) {
            Log.e(LogTag.TXN_TAG, "insertMessage:getMessagesFromIntent return null.");
            return null;
        }
        /// @}
        MtkSmsMessage sms = msgs[0];
        if (sms == null) {
            return null;
        }

        if (sms.getMessageClass() == SmsMessage.MessageClass.CLASS_0) {
            Log.d(LogTag.TXN_TAG, "insertMessage: display class 0 message!");
            displayClassZeroMessage(context, (SmsMessage[]) msgs, format);
            return null;
        } else if (sms.isReplace()) {
            Log.d(LogTag.TXN_TAG, "insertMessage: is replace message!");
            return replaceMessage(context, msgs, error);
        } else {
            Log.d(LogTag.TXN_TAG, "insertMessage: stored directly!");
            return storeMessage(context, msgs, error);
        }
    }

    /**
     * This method is used if this is a "replace short message" SMS.
     * We find any existing message that matches the incoming
     * message's originating address and protocol identifier.  If
     * there is one, we replace its fields with those of the new
     * message.  Otherwise, we store the new message as usual.
     *
     * See TS 23.040 9.2.3.9.
     */
    private Uri replaceMessage(Context context, SmsMessage[] msgs, int error) {
        /// M:
        Log.i(LogTag.TXN_TAG, "Sms replaceMessage");
        SmsMessage sms = msgs[0];
        if (sms == null) {
            return null;
        }

        ContentValues values = extractContentValues(sms);
        values.put(Sms.ERROR_CODE, error);
        int pduCount = msgs.length;

        if (pduCount == 1) {
            // There is only one part, so grab the body directly.
            values.put(Inbox.BODY, replaceFormFeeds(sms.getDisplayMessageBody()));
        } else {
            // Build up the body from the parts.
            StringBuilder body = new StringBuilder();
            for (int i = 0; i < pduCount; i++) {
                sms = msgs[i];
                if (sms.mWrappedSmsMessage != null) {
                    body.append(sms.getDisplayMessageBody());
                }
            }
            values.put(Inbox.BODY, replaceFormFeeds(body.toString()));
        }

        ContentResolver resolver = context.getContentResolver();
        String originatingAddress = sms.getOriginatingAddress();
        int protocolIdentifier = sms.getProtocolIdentifier();

        int subId = sms.getSubId();
        if (!SubscriptionManager.isValidSubscriptionId(subId)) {
            Log.e(TAG, "subId is invalid in replaceMessage()!");
            return null;
        }

        String selection =
                Sms.ADDRESS + " = ? AND " +
                Sms.PROTOCOL + " = ? AND " +
                Sms.SUBSCRIPTION_ID + " = ?";
        String[] selectionArgs = new String[] {
            originatingAddress, Integer.toString(protocolIdentifier),
            Long.toString(subId)
        };

        Cursor cursor = SqliteWrapper.query(context, resolver, Inbox.CONTENT_URI,
                            REPLACE_PROJECTION, selection, selectionArgs, null);

        if (cursor != null) {
            try {
                if (cursor.moveToFirst()) {
                    long messageId = cursor.getLong(REPLACE_COLUMN_ID);
                    Uri messageUri = ContentUris.withAppendedId(
                            Sms.CONTENT_URI, messageId);

                    SqliteWrapper.update(context, resolver, messageUri,
                                        values, null, null);
                    return messageUri;
                }
            } finally {
                cursor.close();
            }
        }
        return storeMessage(context, msgs, error);
    }

    /**
     * This method is used if this is a "replace short message" SMS.
     * We find any existing message that matches the incoming
     * message's originating address and protocol identifier.  If
     * there is one, we replace its fields with those of the new
     * message.  Otherwise, we store the new message as usual.
     *
     * See TS 23.040 9.2.3.9.
     *
     * This is the version to handle MtkSmsMessages
     */
    private Uri replaceMessage(Context context, MtkSmsMessage[] msgs, int error) {
        /// M:
        Log.i(LogTag.TXN_TAG, "Sms replaceMessage");
        MtkSmsMessage sms = msgs[0];
        if (sms == null) {
            return null;
        }

        ContentValues values = extractContentValues(sms);
        values.put(Sms.ERROR_CODE, error);
        int pduCount = msgs.length;

        if (pduCount == 1) {
            // There is only one part, so grab the body directly.
            values.put(Inbox.BODY, replaceFormFeeds(sms.getDisplayMessageBody()));
        } else {
            // Build up the body from the parts.
            StringBuilder body = new StringBuilder();
            for (int i = 0; i < pduCount; i++) {
                sms = msgs[i];
                if (sms.mWrappedSmsMessage != null) {
                    body.append(sms.getDisplayMessageBody());
                }
            }
            values.put(Inbox.BODY, replaceFormFeeds(body.toString()));
        }

        ContentResolver resolver = context.getContentResolver();
        String originatingAddress = sms.getOriginatingAddress();
        int protocolIdentifier = sms.getProtocolIdentifier();

        int subId = sms.getSubId();
        if (!SubscriptionManager.isValidSubscriptionId(subId)) {
            Log.e(TAG, "subId is invalid in replaceMessage()!");
            return null;
        }

        String selection =
                Sms.ADDRESS + " = ? AND " +
                Sms.PROTOCOL + " = ? AND " +
                Sms.SUBSCRIPTION_ID + " = ?";
        String[] selectionArgs = new String[] {
            originatingAddress, Integer.toString(protocolIdentifier),
            Long.toString(subId)
        };

        Cursor cursor = SqliteWrapper.query(context, resolver, Inbox.CONTENT_URI,
                            REPLACE_PROJECTION, selection, selectionArgs, null);

        if (cursor != null) {
            try {
                if (cursor.moveToFirst()) {
                    long messageId = cursor.getLong(REPLACE_COLUMN_ID);
                    Uri messageUri = ContentUris.withAppendedId(
                            Sms.CONTENT_URI, messageId);

                    SqliteWrapper.update(context, resolver, messageUri,
                                        values, null, null);
                    return messageUri;
                }
            } finally {
                cursor.close();
            }
        }
        return storeMessage(context, msgs, error);
    }

    public static String replaceFormFeeds(String s) {
        // Some providers send formfeeds in their messages. Convert those formfeeds to newlines.
        return s == null ? "" : s.replace('\f', '\n');
    }

//    private static int count = 0;
    public static final String SMS_SAVE_LOCATION = "pref_key_sms_save_location";

    private Uri storeMessage(Context context, SmsMessage[] msgs, int error) {
        /// M:
        Log.i(LogTag.TXN_TAG, "Sms storeMessage");
        SmsMessage sms = msgs[0];

        // Store the message in the content provider.
        ContentValues values = extractContentValues(sms);
        values.put(Sms.ERROR_CODE, error);
        int pduCount = msgs.length;
//        if (!mOpSmsReceiverServiceExt.storeMessage(msgs, sms, values)) {
            extractSmsBody(msgs, sms, values);
//        }

        // Make sure we've got a thread id so after the insert we'll be able to delete
        // excess messages.
        Long threadId = values.getAsLong(Sms.THREAD_ID);
        String address = values.getAsString(Sms.ADDRESS);

        if (!TextUtils.isEmpty(address)) {
            address = internalGet(address);
        } else {
            address = "unkonwn";
            values.put(Sms.ADDRESS, address);
        }

        int subId = sms.getSubId();
        if (!SubscriptionManager.isValidSubscriptionId(subId)) {
            Log.e(TAG, "subId in storeMessage() is invalid!");
            return null;
        }
        values.put(Sms.SUBSCRIPTION_ID, subId);

           /* MTK note: for FTA test in the msim phone
            * We need to tell SmsManager where the last incoming SMS comes from.
            * This is because the mms APP and Phone APP runs in two different process
            * and mms will use setSmsMemoryStatus to tell modem that the ME storage is full or not.
            * Since We need to dispatch the information about ME storage to current SIM
            * so we should use setLastIncomingSmsSimId here
            * to tell SmsManager this to let it dispatch the info.
            */
        // mark by hongduo
        lastIncomingSmsSubId = subId;
        /// @}

//        if (((threadId == null) || (threadId == 0)) && (address != null)) {
//            threadId = Conversation.getOrCreateThreadId(context, address);
//            values.put(Sms.THREAD_ID, threadId);
//        }

        ContentResolver resolver = context.getContentResolver();

        Uri insertedUri = SqliteWrapper.insert(context, resolver, Inbox.CONTENT_URI, values);


    //    String storeLocation = MmsConfig.getSmsSaveLocation(subId);
    /*    final String saveLocationKey = Long.toString(subId) + "_"
                + SMS_SAVE_LOCATION;
        SharedPreferences prefs = MmsConfig.getMmsSharedPreferences();
        if (prefs != null) {
            storeLocation = prefs.getString(saveLocationKey, "Phone");
            if (storeLocation == null) {
                storeLocation = prefs.getString(SMS_SAVE_LOCATION, "Phone");
            }
        }*/
        if (mSaveLocation.get(subId).equals("Sim")) {
    //    if (storeLocation.equals("Sim")) {
            String sc =
                (null == sms.getServiceCenterAddress()) ? "" : sms.getServiceCenterAddress();
            String smsBody = values.getAsString(Inbox.BODY);
            MtkSmsManager smsManager = MtkSmsManager .getSmsManagerForSubscriptionId(subId);

            ArrayList<String> messages = smsManager.divideMessage(smsBody);
            int result = smsManager.copyTextMessageToIccCard(
                    sms.getServiceCenterAddress(), values.getAsString(Sms.ADDRESS), messages,
                    SmsManager.STATUS_ON_ICC_READ, System.currentTimeMillis());
            Log.d(LogTag.TXN_TAG,
                    "save sms on SIM. part result:" + result + "; sc:" + sc + "; subId:" + subId);
            /// M: fix for bug ALPS01468873 @{
            /*
            if(bSucceed){
                sendBroadcast(new Intent(ManageSimMessages.ACTION_NOTIFY_SIMMESSAGE_UPDATE));
            }
            */
        }

        /// M:Code analyze 037, set sms size @{
        if (null != insertedUri) {
            int messageSize = 0;
            if (pduCount == 1) {
                messageSize = sms.getPdu().length;
            } else {
                for (int i = 0; i < pduCount; i++) {
                    sms = msgs[i];
                    if (sms.mWrappedSmsMessage != null && sms.getPdu() != null) {
                        messageSize += sms.getPdu().length;
                    }
                }
            }
            ContentValues sizeValue = new ContentValues();
            sizeValue.put(Mms.MESSAGE_SIZE, messageSize);
            SqliteWrapper.update(this, getContentResolver(), insertedUri, sizeValue, null, null);

        /// @}

            Cursor cur = SqliteWrapper.query(this, resolver, insertedUri,
                    new String[] {"thread_id"}, null, null, null);
            try {
                if (cur != null && cur.moveToFirst()) {
                    threadId = cur.getLong(0);
                }
            } finally {
                if (cur != null) {
                    cur.close();
                }
            }
        }
// mark by hongduo
    //    ThreadCountManager.getInstance().isFull(threadId, context,
    //            ThreadCountManager.OP_FLAG_INCREASE);
        isFull(threadId, context, OP_FLAG_INCREASE);

        // Now make sure we're not over the limit in stored messages
        if (threadId != null) {
            Recycler.getSmsRecycler().deleteOldMessagesByThreadId(context, threadId);
        }
// mark by hongduo
   //     MmsWidgetProvider.notifyDatasetChanged(context);

        return insertedUri;
    }

    /* Version to handle MtkSmsMessages*/
    private Uri storeMessage(Context context, MtkSmsMessage[] msgs, int error) {
        /// M:
        Log.i(LogTag.TXN_TAG, "Sms storeMessage");
        MtkSmsMessage sms = msgs[0];

        // Store the message in the content provider.
        ContentValues values = extractContentValues(sms);
        values.put(Sms.ERROR_CODE, error);
        int pduCount = msgs.length;

        if (!mOpSmsReceiverServiceExt.storeMessage(msgs, sms, values)) {
            extractSmsBody((SmsMessage[]) msgs, (SmsMessage) sms, values);
        }

        // Make sure we've got a thread id so after the insert we'll be able to delete
        // excess messages.
        Long threadId = values.getAsLong(Sms.THREAD_ID);
        String address = values.getAsString(Sms.ADDRESS);

        // Code for debugging and easy injection of short codes, non email addresses, etc.
        // See Contact.isAlphaNumber() for further comments and results.
//        switch (count++ % 8) {
//            case 0: address = "AB12"; break;
//            case 1: address = "12"; break;
//            case 2: address = "Jello123"; break;
//            case 3: address = "T-Mobile"; break;
//            case 4: address = "Mobile1"; break;
//            case 5: address = "Dogs77"; break;
//            case 6: address = "****1"; break;
//            case 7: address = "#4#5#6#"; break;
//        }

        if (!TextUtils.isEmpty(address)) {
            address = internalGet(address);
        } else {
            address = "unkonwn"; // getString(R.string.unknown_sender);
            values.put(Sms.ADDRESS, address);
        }

        int subId = sms.getSubId();
        if (!SubscriptionManager.isValidSubscriptionId(subId)) {
            Log.e(TAG, "subId in storeMessage() is invalid!");
            return null;
        }
        values.put(Sms.SUBSCRIPTION_ID, subId);

           /* MTK note: for FTA test in the msim phone
            * We need to tell SmsManager where the last incoming SMS comes from.
            * This is because the mms APP and Phone APP runs in two different process
            * and mms will use setSmsMemoryStatus to tell modem that the ME storage is full or not.
            * Since We need to dispatch the information about ME storage to current SIM
            * so we should use setLastIncomingSmsSimId here
            * to tell SmsManager this to let it dispatch the info.
            */
        lastIncomingSmsSubId = subId;/* sms.getSubscriptionId() */;
        /// @}

//        if (((threadId == null) || (threadId == 0)) && (address != null)) {
//            threadId = Conversation.getOrCreateThreadId(context, address);
//            values.put(Sms.THREAD_ID, threadId);
//        }

        ContentResolver resolver = context.getContentResolver();

        Uri insertedUri = SqliteWrapper.insert(context, resolver, Inbox.CONTENT_URI, values);

//        String storeLocation = MmsConfig.getSmsSaveLocation(subId);

     /*   final String saveLocationKey = Long.toString(subId) + "_"
                + SMS_SAVE_LOCATION;
        storeLocation = prefs.getString(saveLocationKey, "Phone");

        if (storeLocation == null) {
            storeLocation = prefs.getString(SMS_SAVE_LOCATION, "Phone");
        }*/
        if (mSaveLocation != null && ("Sim").equals(mSaveLocation.get(subId))) {
            String sc =
                (null == sms.getServiceCenterAddress()) ? "" : sms.getServiceCenterAddress();
            String smsBody = values.getAsString(Inbox.BODY);

            MtkSmsManager smsManager = MtkSmsManager .getSmsManagerForSubscriptionId(subId);
            ArrayList<String> messages = smsManager.divideMessage(smsBody);

            int result = smsManager.copyTextMessageToIccCard(
                    sms.getServiceCenterAddress(), values.getAsString(Sms.ADDRESS), messages,
                    SmsManager.STATUS_ON_ICC_READ, System.currentTimeMillis());
            Log.d(LogTag.TXN_TAG,
                    "save sms on SIM. part result:" + result + "; sc:" + sc + "; subId:" + subId);
            /// M: fix for bug ALPS01468873 @{
            /*
            if(bSucceed){
                sendBroadcast(new Intent(ManageSimMessages.ACTION_NOTIFY_SIMMESSAGE_UPDATE));
            }
            */
        }

        /// M:Code analyze 037, set sms size @{
        if (null != insertedUri) {
            int messageSize = 0;
            if (pduCount == 1) {
                messageSize = sms.getPdu().length;
            } else {
                for (int i = 0; i < pduCount; i++) {
                    sms = msgs[i];
                    if (sms.mWrappedSmsMessage != null && sms.getPdu() != null) {
                        messageSize += sms.getPdu().length;
                    }
                }
            }
            ContentValues sizeValue = new ContentValues();
            sizeValue.put(Mms.MESSAGE_SIZE, messageSize);
            SqliteWrapper.update(this, getContentResolver(), insertedUri, sizeValue, null, null);

        /// @}

            Cursor cur = SqliteWrapper.query(this, resolver, insertedUri,
                    new String[] {"thread_id"}, null, null, null);
            try {
                if (cur != null && cur.moveToFirst()) {
                    threadId = cur.getLong(0);
                }
            } finally {
                if (cur != null) {
                    cur.close();
                }
            }
        }
// mark by hongduo
     //   ThreadCountManager.getInstance().isFull(threadId, context,
     //           ThreadCountManager.OP_FLAG_INCREASE);
        isFull(threadId, context, OP_FLAG_INCREASE);

        // Now make sure we're not over the limit in stored messages
        if (threadId != null) {
            Recycler.getSmsRecycler().deleteOldMessagesByThreadId(context, threadId);
        }
// mark by hongduo
   //     MmsWidgetProvider.notifyDatasetChanged(context);

        return insertedUri;
    }

    /**
     * Extract all the content values except the body from an SMS
     * message.
     */
    private ContentValues extractContentValues(SmsMessage sms) {
        // Store the message in the content provider.
        ContentValues values = new ContentValues();

        values.put(Inbox.ADDRESS, sms.getDisplayOriginatingAddress());
        /// M:Code analyze 039, comment the code,just using local time @{
        /*
        // Use now for the timestamp to avoid confusion with clock
        // drift between the handset and the SMSC.
        // Check to make sure the system is giving us a non-bogus time.
        Calendar buildDate = new GregorianCalendar(2011, 8, 18);    // 18 Sep 2011
        Calendar nowDate = new GregorianCalendar();
        long now = System.currentTimeMillis();
        nowDate.setTimeInMillis(now);

        if (nowDate.before(buildDate)) {
            // It looks like our system clock isn't set yet because the current time right now
            // is before an arbitrary time we made this build. Instead of inserting a bogus
            // receive time in this case, use the timestamp of when the message was sent.
            now = sms.getTimestampMillis();
        }

        values.put(Inbox.DATE, new Long(now));
        */
        values.put(Inbox.DATE, Long.valueOf(System.currentTimeMillis()));
        /// @}
        values.put(Inbox.DATE_SENT, Long.valueOf(sms.getTimestampMillis()));
        values.put(Inbox.PROTOCOL, sms.getProtocolIdentifier());
        values.put(Inbox.READ, 0);
        values.put(Inbox.SEEN, 0);
        if (sms.getPseudoSubject().length() > 0) {
            values.put(Inbox.SUBJECT, sms.getPseudoSubject());
        }
        values.put(Inbox.REPLY_PATH_PRESENT, sms.isReplyPathPresent() ? 1 : 0);
        values.put(Inbox.SERVICE_CENTER, sms.getServiceCenterAddress());
        return values;
    }

    /// M:Code analyze 030, the second argument is changed,if message type is class 0
    /// just only show it on phone but not save @{
    /**
     * Displays a class-zero message immediately in a pop-up window
     * with the number from where it received the Notification with
     * the body of the message
     *
     */
    private void displayClassZeroMessage(Context context, SmsMessage[] msgs, String format) {
        // Using NEW_TASK here is necessary because we're calling
        // startActivity from outside an activity.
        int subId = msgs[0].getSubId();
        if (!SubscriptionManager.isValidSubscriptionId(subId)) {
            Log.e(TAG, "subId is invalid in displayClassZeroMessage()");
            return;
        }
        int pduCount = msgs.length;
        Object[] pdus = new Object[pduCount];
        for (int i = 0; i < pduCount; i++) {
            pdus[i] = (Object) msgs[i].getPdu();
        }

        Intent smsDialogIntent = new Intent()
                .putExtra("pdus", pdus)
                .putExtra("format", format)
                .putExtra(PhoneConstants.SUBSCRIPTION_KEY, subId)
                .setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                          | Intent.FLAG_ACTIVITY_MULTIPLE_TASK);
                smsDialogIntent.setClassName("com.android.mms",
                "com.android.mms.ui.ClassZeroActivity");
        context.startActivity(smsDialogIntent);
    }
    /// @}

    private void registerForServiceStateChanges(int subId) {
        if (sNoServiceSimSet.isEmpty()) {
            Context context = getApplicationContext();
            unRegisterForServiceStateChanges();

            IntentFilter intentFilter = new IntentFilter();
            intentFilter.addAction(TelephonyIntents.ACTION_SERVICE_STATE_CHANGED);
            //if (Log.isLoggable(LogTag.TRANSACTION, Log.VERBOSE)) {
                Log.v(TAG, "registerForServiceStateChanges");
            //}

            context.registerReceiver(mSmsRetryReceiver, intentFilter);
        }
        sNoServiceSimSet.add(subId);
    }

    private void unRegisterForServiceStateChanges() {
        //if (Log.isLoggable(LogTag.TRANSACTION, Log.VERBOSE)) {
            Log.v(TAG, "unRegisterForServiceStateChanges");
        //}
        try {
            if (sNoServiceSimSet.isEmpty()) {
                Context context = getApplicationContext();
                context.unregisterReceiver(mSmsRetryReceiver);
            }
        } catch (IllegalArgumentException e) {
            // Allow un-matched register-unregister calls
        }
    }

    private String getNoServiceSimString() {
        StringBuilder stringBuilder = new StringBuilder();
        Iterator<Integer> noServiceIterator = sNoServiceSimSet.iterator();
        while (noServiceIterator.hasNext()) {
            Integer subId = noServiceIterator.next();
            Log.d(TAG, "getNoServiceSimString :: subID = " + subId);
            boolean checkIms = MtkTelephonyManagerEx.getDefault().isImsRegistered(subId);
            Log.d(TAG, "getNoServiceSimString :: checkIms = " + checkIms);

            if (checkIms == true ||
                (MtkTelephonyManagerEx.getDefault().isWifiCallingEnabled(subId))) {
                Log.d(TAG, "Wifi calling registered :: subID = " + subId);
                continue;
            }

            if (stringBuilder.length() != 0) {
                stringBuilder.append(",");
            }
            stringBuilder.append(subId);
            Log.d(TAG, "Added to no service string ");
        }
        String result = stringBuilder.toString();
        return result;
    }

    /// M:Code analyze 003, Indicates this is a concatenation sms @{
    public static final String EXTRA_MESSAGE_CONCATENATION = "ConcatenationMsg";
    /// @}

    /**
     * Move all messages that are in the queued to the failed state and set them to unread.
     * @return The number of messages that were actually moved
     */
    private int moveQueuedMessagesToFailedBox() {
        ContentValues values = new ContentValues(3);

        values.put(Sms.TYPE, Sms.MESSAGE_TYPE_FAILED);
        values.put(Sms.ERROR_CODE, SmsManager.RESULT_ERROR_GENERIC_FAILURE);
        values.put(Sms.READ, Integer.valueOf(0));

        final Uri uri = Uri.parse("content://sms/queued");
        int messageCount = SqliteWrapper.update(
                getApplicationContext(), getContentResolver(), uri,
                values, "type = " + Sms.MESSAGE_TYPE_QUEUED , null);

        if (Log.isLoggable(LogTag.TRANSACTION, Log.VERBOSE)) {
            Log.v(TAG, "moveQueuedMessagesToFailedBox messageCount: " + messageCount);
        }
        return messageCount;
    }

    /// M: For receive missing segment of long SMS.
    public Uri callStoreMessage(Context context, SmsMessage[] msgs, int error) {
        return storeMessage(context, msgs, error);
    }

    /// M: For receive missing segment of long SMS. MtkSmsMessage version
    public Uri callStoreMessage(Context context, MtkSmsMessage[] msgs, int error) {
        return storeMessage(context, msgs, error);
    }

    public void extractSmsBody(SmsMessage[] msgs, SmsMessage sms, ContentValues values) {
        int pduCount = msgs.length;

        Log.d(TAG, "SmsReceiverImpl.extractSmsBody, pduCount=" + pduCount);

        if (pduCount == 1) {
            // There is only one part, so grab the body directly.
            values.put(Inbox.BODY, replaceFormFeeds(sms.getDisplayMessageBody()));
        } else {
            // Build up the body from the parts.
            StringBuilder body = new StringBuilder();
            for (int i = 0; i < pduCount; i++) {
                sms = msgs[i];
                if (sms != null && sms.mWrappedSmsMessage != null) {
                    body.append(sms.getDisplayMessageBody());
                }
            }
            values.put(Inbox.BODY, replaceFormFeeds(body.toString()));
        }
    }

    public static final int OP_FLAG_INCREASE = 0;
    public static final int OP_FLAG_DECREASE = 1;
    private static final int THREAD_MAX_SIZE = 100;
    public void isFull(Long threadId, Context context, int flag) {
        try {
                IMtkTelephonyEx iTelEx = IMtkTelephonyEx.Stub.asInterface(
                        ServiceManager.getService("phoneEx"));
                if (iTelEx.isTestIccCard(0) ||
                        (Integer.parseInt(SystemProperties.get("vendor.gsm.gcf.testmode", "0")) == 2)) {
                    Log.d(TAG, "Now using test icc card...");
                    int lastSubId = lastIncomingSmsSubId;
                    if (flag == OP_FLAG_INCREASE) {
                        if (get(threadId, context) >= THREAD_MAX_SIZE) {
                            Log.d(TAG, "Storage is full. send notification...");
                        MtkSmsManager.getSmsManagerForSubscriptionId(lastSubId)
                                .setSmsMemoryStatus(false);
                        }
                    } else if (flag == OP_FLAG_DECREASE) {
                        if (get(threadId, context) < THREAD_MAX_SIZE) {
                            Log.d(TAG, "Storage is available. send notification...");
                        MtkSmsManager.getSmsManagerForSubscriptionId(lastSubId)
                                .setSmsMemoryStatus(true);
                        }
                    }
                }
        } catch (Exception ex) {
            Log.e(TAG, " " + ex.getMessage());
        }
    }

   /**
     * get the thread count
     *
     * @param threadId
     * @param context
     * @return
     */
    private int get(Long threadId, Context context) {
        ContentResolver resolver = context.getContentResolver();
        String where = Telephony.Mms.THREAD_ID + "=" + threadId;
        String[] projection = new String[] { Sms.Inbox._ID };

        Cursor cursor = null;

        try {
            cursor = Sms.query(resolver, projection, where, null);
            if (cursor == null) {
                return 0;
            } else {
                int cnt = cursor.getCount();
                Log.d(TAG, "sms count is :" + cnt);
                return cnt;
            }
        } finally {
            if (cursor != null) {
                Log.d(TAG, "close cursor");
                cursor.close();
            }
        }
    }

    /**
       * Get the thread ID of the SMS message with the given URI
       * @param context The context
       * @param uri The URI of the SMS message
       * @return The thread ID, or THREAD_NONE if the URI contains no entries
       */
      public static long getSmsThreadId(Context context, Uri uri) {
          Cursor cursor = SqliteWrapper.query(
              context,
              context.getContentResolver(),
              uri,
              SMS_THREAD_ID_PROJECTION,
              null,
              null,
              null);

          if (cursor == null) {
              return MmsConfig.THREAD_NONE;
          }

          try {
              if (cursor.moveToFirst()) {
                  int columnIndex = cursor.getColumnIndex(Sms.THREAD_ID);
                  if (columnIndex < 0) {
                      return MmsConfig.THREAD_NONE;
                  }
                  long threadId = cursor.getLong(columnIndex);
                  return threadId;
              } else {
                  return MmsConfig.THREAD_NONE;
              }
          } finally {
              cursor.close();
          }
      }

      private String internalGet(String numberOrEmail) {
          String workingNumberOrEmail = numberOrEmail;
          workingNumberOrEmail = workingNumberOrEmail.replaceAll(" ", "")
              .replaceAll("-", "").replaceAll("[()]", "");

          final boolean isNotRegularPhoneNumber =
                  Mms.isEmailAddress(numberOrEmail)
                  || MmsConfig.isAlias(numberOrEmail);
          /// M: @{
          if (isNotRegularPhoneNumber) {
              // Log.d(M_TAG, isMe ? "Contact.internalGet(): It's me!" :
              // "Contact.internalGet(): It's a Email!");
              return numberOrEmail;
          } else if (MmsConfig.isWellFormedSmsAddress(workingNumberOrEmail)) {
              // Log.d(M_TAG, "Contact.internalGet(): Number without space
              // and '-' is a well-formed number for sending sms.");
              return workingNumberOrEmail;
          } else {
              Log.d(TAG, "Contact.internalGet(): Unknown formed number.");
              workingNumberOrEmail = PhoneNumberUtils.stripSeparators(workingNumberOrEmail);
              workingNumberOrEmail = PhoneNumberUtils.formatNumber(workingNumberOrEmail);
              if (numberOrEmail.equals(workingNumberOrEmail)) {
                  Log.d(TAG,
                      "Contact.internalGet(): Unknown formed number," +
                      "but the number without local number" +
                      " formatting is a well-formed number.");
                  return PhoneNumberUtils.stripSeparators(workingNumberOrEmail);
              } else {
                  Log.d(TAG, "Contact.internalGet(): Bad number.");
                  return numberOrEmail;
              }
          }
     }

      private static BroadcastReceiver mSmsRetryReceiver = new BroadcastReceiver() {
          @Override
          public void onReceive(Context context, Intent intent) {
              Log.d(LogTag.TXN_TAG, "SmsRetryReceiver : onReceive(). subId = "
                  + intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY,
                          SubscriptionManager.INVALID_SUBSCRIPTION_ID)
                  + ", action = " + intent.getAction());
              if (serviceStateChangedPrecheck(intent) == true) {
                  intent.setClass(context, SmsReceiverService.class);
                  context.startService(intent);
              }
          }

          private boolean serviceStateChangedPrecheck(Intent intent) {
              Log.d(LogTag.TXN_TAG, "mSmsRetryReceiver serviceStateChangedPrecheck");

              // If service just returned, start sending out the queued messages
              ServiceState serviceState = ServiceState.newFromBundle(intent.getExtras());
              int subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY,
                      SubscriptionManager.INVALID_SUBSCRIPTION_ID);

              Log.d(LogTag.TXN_TAG, "subId in serviceStateChangedPrecheck() subId = " + subId);

              if (!SubscriptionManager.isValidSubscriptionId(subId)) {
                  Log.d(LogTag.TXN_TAG, "subId in serviceStateChangedPrecheck() is invalid!" + subId);
                  return false;
              }
              int stateVoice = serviceState.getVoiceRegState();

              Log.d(LogTag.TXN_TAG, "serviceState.getVoiceRegState" + stateVoice);

              int stateData = ServiceState.STATE_OUT_OF_SERVICE ;
              boolean checkIms = MtkTelephonyManagerEx.getDefault().isImsRegistered(subId);

              if (checkIms == true) {
                  stateData = serviceState.getDataRegState();
                  Log.d(LogTag.TXN_TAG, "serviceState.getDataRegState" + stateData);
              }

              if ((stateVoice == ServiceState.STATE_IN_SERVICE ||
                      stateData == ServiceState.STATE_IN_SERVICE)
                      && (SmsReceiverService.sNoServiceSimSet != null)
                      && SmsReceiverService.sNoServiceSimSet.contains(subId)) {
                  Log.d(LogTag.TXN_TAG, "mSmsRetryReceiver serviceStateChangedPrecheck contains " + subId);
                  return true;
              }
              Log.d(LogTag.TXN_TAG, "mSmsRetryReceiver serviceStateChangedPrecheck return false");
              return false;
          }
      };
}
