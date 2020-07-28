/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2008 Esmertec AG.
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

package com.android.mms;

import android.app.Application;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.res.Configuration;
import android.drm.DrmManagerClient;
import android.location.Country;
import android.location.CountryDetector;
import android.location.CountryListener;
import android.net.Uri;
import android.os.Build;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.os.StrictMode;
import android.preference.PreferenceManager;
import android.provider.SearchRecentSuggestions;
import android.provider.Telephony;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;
import android.widget.Toast;

import com.android.mms.data.Contact;
import com.android.mms.data.Conversation;
import com.android.mms.layout.LayoutManager;
import com.android.mms.transaction.MessagingNotification;
import com.android.mms.transaction.MmsSystemEventReceiver;
import com.android.mms.transaction.SmsRejectedReceiver;
import com.android.mms.transaction.SmsReceiver;
import com.android.mms.ui.MessageUtils;
//import com.android.mms.ui.SmsStorageMonitor;
import com.android.mms.util.DownloadManager;
import com.android.mms.util.MmsLog;
import com.android.mms.util.PduLoaderManager;
//import com.android.mms.util.RateController;
import com.android.mms.util.ThumbnailManager;

import com.android.internal.telephony.TelephonyIntents;

import android.support.v4.content.LocalBroadcastManager;

import com.mediatek.cb.cbmsg.CbMessagingNotification;
import com.mediatek.cb.cbmsg.CbMessageReceiver;
import com.mediatek.contacts.common.util.GlobalEnv;
import com.mediatek.ipmsg.util.IpMessageUtils;
import com.mediatek.mms.util.PermissionCheckUtil;
import com.mediatek.mwi.MwiMessagingNotification;
import com.mediatek.mwi.MwiReceiver;
import com.mediatek.opmsg.util.OpMessageUtils;
import com.mediatek.simmessage.SimFullReceiver;
import com.mediatek.wappush.WapPushMessagingNotification;

import java.util.HashMap;
import java.util.Locale;

import mediatek.telephony.MtkSmsManager;

public class MmsApp extends Application {
    public static final String LOG_TAG = "Mms";

    private SearchRecentSuggestions mRecentSuggestions;
    private TelephonyManager mTelephonyManager;
    private CountryDetector mCountryDetector;
    private CountryListener mCountryListener;
    private String mCountryIso;
    private static MmsApp sMmsApp = null;
    private PduLoaderManager mPduLoaderManager;
    private ThumbnailManager mThumbnailManager;
    private DrmManagerClient mDrmManagerClient;
    /// M: fix bug ALPS00987075, Optimize first launch time @{
    //memslim private Context mContext;
    /// @}

    /// M: for toast thread
    public static final String TXN_TAG = "Mms/Txn";
    /* memslim
    public static final int MSG_RETRIEVE_FAILURE_DEVICE_MEMORY_FULL = 2;
    public static final int MSG_SHOW_TRANSIENTLY_FAILED_NOTIFICATION = 4;
    public static final int MSG_MMS_TOO_BIG_TO_DOWNLOAD = 6;
    */
    public static final int MSG_MMS_CAN_NOT_SAVE = 8;
    public static final int MSG_MMS_CAN_NOT_OPEN = 10;
    public static final int MSG_DONE = 12;
    public static final int EVENT_QUIT = 100;
    //memslim private static HandlerThread mToastthread = null;
    private static ToastHandler mToastHandler = null;

    // intent action send from mms app service to show notification and toast
    public static final String ACTION_MMS_APP_SERVICE_NOTIFICATION =
            "com.mediatek.mms.appservice.notification";
    public static final String ACTION_MMS_APP_SERVICE_TOAST = "com.mediatek.mms.appservice.toast";
    public static final String ACTION_MMS_APP_SERVICE_STOP_SERVICE =
            "com.mediatek.mms.appservice.stopservice";

    public static final String ACTION_MMS_TOAST = "com.mediatek.mms.toast";

    // type of notifications send from mms app service
    public static final int RECEIEVE_CB  = 1;
    public static final int RECEIEVE_NEW = 2;
    public static final int SEND_FAILED  = 3;
    public static final int DOWNLOAD_FAILED = 4;
    public static final int STATUS_MESSAGE = 5;
    public static final int CANCEL_DOWNLOAD_FAILED = 6;
    public static final int CANCEL_SEND_FAILED = 7;
    public static final int CLASS_0 = 8;
    public static final int RECEIEVE_WAPPUSH  = 9;
    public static final int RECEIEVE_WAPPUSH_AUTOLAUNCH  = 10;
    public static final int RECEIEVE_MWI  = 11;

    // toast type send from mms app service
    public static final int EXCEPTION_ERROR = -1;
    public static final int MESSAGE_QUEUED = 1;
    public static final int SERVICE_MESSAGE_NOT_FOUND = 2; // mms expiry passed
    public static final int INVALID_DESTINATION = 3;
    public static final int SERVICE_NOT_ACTIVATED = 4;
    public static final int SERVICE_NETWORK_PROBLEM = 5;
    public static final int DOWNLOAD_LATER = 6;
    public static final int FDN_CHECK_FAILURE = 7;
    public static final int WAPPUSH_UNSUPPORTED_SCHEME = 8;

    @Override
    public void onCreate() {
        super.onCreate();
        MmsLog.d(MmsApp.TXN_TAG, "MmsApp.onCreate");

        if (Log.isLoggable(LogTag.STRICT_MODE_TAG, Log.DEBUG)) {
            // Log tag for enabling/disabling StrictMode violation log. This will dump a stack
            // in the log that shows the StrictMode violator.
            // To enable: adb shell setprop log.tag.Mms:strictmode DEBUG
            StrictMode.setThreadPolicy(
                    new StrictMode.ThreadPolicy.Builder().detectAll().penaltyLog().build());
        }

        if ("eng".equals(Build.TYPE)) {
            StrictMode.setVmPolicy(
                    new StrictMode.VmPolicy.Builder().detectAll().penaltyLog().build());
        }

        sMmsApp = this;
        // add for ipmessage
        IpMessageUtils.onIpMmsCreate(this);

        // Load the default preference values
     //   PreferenceManager.setDefaultValues(this, R.xml.preferences, false);

        // Figure out the country *before* loading contacts and formatting numbers
        mCountryDetector = (CountryDetector) getSystemService(Context.COUNTRY_DETECTOR);
        mCountryListener = new CountryListener() {
            @Override
            public synchronized void onCountryDetected(Country country) {
                mCountryIso = country.getCountryIso();
            }
        };
        mCountryDetector.addCountryListener(mCountryListener, getMainLooper());
        /// M: fix bug ALPS01017776, Optimize first launch time @{
        //memslim mContext = getApplicationContext();
        /// @}

        /// M:init GlobalEnv for mediatek ContactsCommon @}
        //memslim GlobalEnv.setApplicationContext(mContext);
        GlobalEnv.setApplicationContext(getApplicationContext());
        /// @}
        OpMessageUtils.init(this);
        MmsConfig.init(this);
        MessageUtils.init(this);
        /// M: comment this
        Conversation.init(this);
        DownloadManager.init(this);
        //memslim RateController.init(this);
        LayoutManager.init(this);
        MessagingNotification.init(this);
        /// M: @{
        InitToastThread();
        /// @}

        //memslim registerSmsRejectedReceiver(this);
        registerSimFullReceiver(this);
        registerLocalSimFullReceiver(this);
        registerDeviceStorageReceiver(this);
        //registerImsReceiver(this);
        //memslim registerCbReceiver(this);
        registerMmsAppServiceReceiver(this);
        registerMmsToastReceiver(this);
        MmsLog.d(MmsApp.TXN_TAG, "MmsApp.onCreate end");
    }

    synchronized public static MmsApp getApplication() {
        return sMmsApp;
    }

    @Override
    public void onTerminate() {
        MmsLog.d(LOG_TAG, "MmsApp#onTerminate");
        mCountryDetector.removeCountryListener(mCountryListener);
    }

    @Override
    public void onLowMemory() {
        MmsLog.d(LOG_TAG, "MmsApp#onLowMemory");
        super.onLowMemory();
        /// M: fix bug ALPS01017776, Optimize first launch time @{
        if (mPduLoaderManager != null) {
            mPduLoaderManager.onLowMemory();
        }
        if (mThumbnailManager != null) {
            mThumbnailManager.onLowMemory();
        }
        /// @}
    }

    public PduLoaderManager getPduLoaderManager() {
        /// M: fix bug ALPS01017776, Optimize first launch time @{
        if (mPduLoaderManager == null) {
            //memslim mPduLoaderManager = new PduLoaderManager(mContext);
            mPduLoaderManager = new PduLoaderManager(getApplicationContext());
        }
        /// @}
        return mPduLoaderManager;
    }

    public ThumbnailManager getThumbnailManager() {
        /// M: fix bug ALPS01017776, Optimize first launch time @{
        if (mThumbnailManager == null) {
            //memslim mThumbnailManager = new ThumbnailManager(mContext);
            mThumbnailManager = new ThumbnailManager(getApplicationContext());
        }
        /// @}
        return mThumbnailManager;
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        LayoutManager.getInstance().onConfigurationChanged(newConfig);
    }

    /**
     * @return Returns the TelephonyManager.
     */
    public TelephonyManager getTelephonyManager() {
        if (mTelephonyManager == null) {
            mTelephonyManager = (TelephonyManager) getApplicationContext()
                    .getSystemService(Context.TELEPHONY_SERVICE);
        }
        return mTelephonyManager;
    }

    /**
     * Returns the content provider wrapper that allows access to recent searches.
     * @return Returns the content provider wrapper that allows access to recent searches.
     */
    public SearchRecentSuggestions getRecentSuggestions() {
        /*
        if (mRecentSuggestions == null) {
            mRecentSuggestions = new SearchRecentSuggestions(this,
                    SuggestionsProvider.AUTHORITY, SuggestionsProvider.MODE);
        }
        */
        return mRecentSuggestions;
    }

    /// Google JB MR1.1 patch. This function CAN return null.
    public String getCurrentCountryIso() {
        if (mCountryIso == null) {
            Country country = mCountryDetector.detectCountry();

            if (country == null) {
                // Fallback to Locale if there are issues with CountryDetector
                return Locale.getDefault().getCountry();
            }

            mCountryIso = country.getCountryIso();
        }
        return mCountryIso;
    }

    public DrmManagerClient getDrmManagerClient() {
        if (mDrmManagerClient == null) {
            mDrmManagerClient = new DrmManagerClient(getApplicationContext());
        }
        return mDrmManagerClient;
    }

    /// M: a handler belong to UI thread.
    private void InitToastThread() {
        if (null == mToastHandler) {
            mToastHandler = new ToastHandler();
        }
    }

    private BroadcastReceiver mMmsAppReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action != null && action.equals(ACTION_MMS_TOAST)) {
                int toastString = intent.getIntExtra("toast_type", EXCEPTION_ERROR);
                String str = getToastStr(toastString);
                Toast.makeText(MmsApp.this, str, Toast.LENGTH_LONG).show();
            }
        }
    };

    private void registerMmsToastReceiver (Context context) {
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(ACTION_MMS_TOAST);
        context.registerReceiver(mMmsAppReceiver, intentFilter);
    }

    public static ToastHandler getToastHandler() {
        return mToastHandler;
    }

    public final class ToastHandler extends Handler {
        public ToastHandler() {
            super();
        }

        @Override
        public void handleMessage(Message msg) {
            MmsLog.d(MmsApp.TXN_TAG, "Toast Handler handleMessage :" + msg);

            switch (msg.what) {
                case EVENT_QUIT: {
                    MmsLog.d(MmsApp.TXN_TAG, "EVENT_QUIT");
                    getLooper().quit();
                    return;
                }
                /*mem slim
                case MSG_RETRIEVE_FAILURE_DEVICE_MEMORY_FULL: {
                    Toast.makeText(sMmsApp,
                            R.string.download_failed_due_to_full_memory, Toast.LENGTH_LONG).show();
                    break;
                }

                case MSG_SHOW_TRANSIENTLY_FAILED_NOTIFICATION: {
                    Toast.makeText(sMmsApp,
                            R.string.transmission_transiently_failed, Toast.LENGTH_LONG).show();
                    break;
                }

                case MSG_MMS_TOO_BIG_TO_DOWNLOAD: {
                    Toast.makeText(sMmsApp,
                            R.string.mms_too_big_to_download, Toast.LENGTH_LONG).show();
                    break;
                }
                */
                case MSG_MMS_CAN_NOT_SAVE: {
                    Toast.makeText(sMmsApp,
                            R.string.cannot_save_message, Toast.LENGTH_LONG).show();
                    break;
                }
                case MSG_MMS_CAN_NOT_OPEN: {
                    String str = sMmsApp.getResources()
                        .getString(R.string.unsupported_media_format, (String) msg.obj);
                    Toast.makeText(sMmsApp, str, Toast.LENGTH_LONG).show();
                    break;
                }

                case MSG_DONE: {
                    Toast.makeText(sMmsApp, R.string.finish, Toast.LENGTH_SHORT).show();
                    break;
                }
            }
        }
    }

    /* memslim
    private void registerSmsRejectedReceiver (Context context) {
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(Telephony.Sms.Intents.SMS_REJECTED_ACTION);
        context.registerReceiver(new SmsRejectedReceiver(), intentFilter);
    }
    */
    public static final String SIM_FULL_VIEWED_ACTION = "com.android.mms.ui.SIM_FULL_VIEWED";
    private void registerSimFullReceiver (Context context) {
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED);
        context.registerReceiver(new SimFullReceiver(), intentFilter);
    }

    private void registerLocalSimFullReceiver (Context context) {
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(SimFullReceiver.SIM_FULL_VIEWED_ACTION);
        LocalBroadcastManager lbm = LocalBroadcastManager.getInstance(context);
        lbm.registerReceiver(new SimFullReceiver(), intentFilter);
    }

    private void registerDeviceStorageReceiver (Context context) {
        IntentFilter mIntentFilter1 = new IntentFilter();
        mIntentFilter1.addAction(Intent.ACTION_DEVICE_STORAGE_FULL);
        mIntentFilter1.addAction(Intent.ACTION_DEVICE_STORAGE_NOT_FULL);

        context.registerReceiver(new MmsSystemEventReceiver(), mIntentFilter1);
    }

    // <action android:name="mediatek.intent.action.lte.mwi" />
    private void registerImsReceiver (Context context) {
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction("mediatek.intent.action.lte.mwi" );
        context.registerReceiver(new MwiReceiver(), intentFilter);
    }

    /* memslim
    private void registerCbReceiver (Context context) {
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction("android.provider.Telephony.SMS_CB_RECEIVED" );
        context.registerReceiver(new CbMessageReceiver(), intentFilter);
    }
    */
    private BroadcastReceiver mMmsAppServiceReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (!PermissionCheckUtil.checkRequiredPermissions(context)) {
                MmsLog.d(MmsApp.TXN_TAG, "mMmsAppServiceReceiver:onReceive() no permission return");
                return;
            }
            String action = intent.getAction();
            if (action != null && action.equals(ACTION_MMS_APP_SERVICE_NOTIFICATION)) {
                updateNotification(context, intent);
            } else if (action != null && action.equals(ACTION_MMS_APP_SERVICE_TOAST)) {
                int toastString = intent.getIntExtra("toast_type", EXCEPTION_ERROR);
                String str = getToastStr(toastString);
                Toast.makeText(MmsApp.this, str, Toast.LENGTH_LONG).show();
            } else if (action != null && action.equals(ACTION_MMS_APP_SERVICE_STOP_SERVICE)) {
                stopWakeLock(intent);
            }
        }
    };

    private void registerMmsAppServiceReceiver (Context context) {
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(ACTION_MMS_APP_SERVICE_NOTIFICATION);
        intentFilter.addAction(ACTION_MMS_APP_SERVICE_TOAST);
        intentFilter.addAction(ACTION_MMS_APP_SERVICE_STOP_SERVICE);
        context.registerReceiver(mMmsAppServiceReceiver, intentFilter);
    }

    private void updateNotification(Context context, Intent intent) {
        int type = intent.getIntExtra("notification_type", RECEIEVE_NEW);
        long threadId = intent.getLongExtra("thread_id", 0);
        String msgUri = intent.getStringExtra("msgUri");
        boolean isStatusMessage = intent.getBooleanExtra("isStatusMessage", false);
        MmsLog.d(LOG_TAG, "updateNotification, type=" + type + ", threadId:" + threadId
                + ", msgUri:" + msgUri);
        switch (type) {
          case RECEIEVE_CB:
              int subId = intent.getIntExtra("subId",
                      SubscriptionManager.INVALID_SUBSCRIPTION_ID);
              CbMessagingNotification.updateNewMessageIndicator(subId, context, true);
              break;
          case RECEIEVE_NEW:
              MessagingNotification.nonBlockingUpdateNewMessageIndicator(context,
                      threadId, false);
              break;
          case SEND_FAILED:
              MessagingNotification.notifySendFailed(context, true);
              break;
          case DOWNLOAD_FAILED:
              MessagingNotification.notifyDownloadFailed(context, threadId);
              break;
          case STATUS_MESSAGE:
              MessagingNotification.blockingUpdateNewMessageIndicator(context,
                      threadId, isStatusMessage, Uri.parse(msgUri));
              break;
          case CANCEL_DOWNLOAD_FAILED:
              MessagingNotification.updateDownloadFailedNotification(context);
              break;
          case CANCEL_SEND_FAILED:
              MessagingNotification.nonBlockingUpdateSendFailedNotification(context);
              break;
          case CLASS_0:
              String address = intent.getStringExtra("address");
              MessagingNotification.notifyClassZeroMessage(context, address);
              break;
          case RECEIEVE_WAPPUSH:
              String wappushUri = intent.getStringExtra("wap_uri");
              MmsLog.d(LOG_TAG, "wappushUri:" + wappushUri);
              if (wappushUri.equals("all")) {
                  WapPushMessagingNotification.blockingUpdateNewMessageIndicator(
                      context, WapPushMessagingNotification.THREAD_ALL);
              } else if (wappushUri.equals("none")) {
                  WapPushMessagingNotification.blockingUpdateNewMessageIndicator(context,
                      WapPushMessagingNotification.THREAD_NONE);
              } else {
                  long wapThreadId = WapPushMessagingNotification.getWapPushThreadId(context, Uri.parse(wappushUri));
                  Log.d(LOG_TAG, "new SL message, threaId:" + wapThreadId);
                  WapPushMessagingNotification.blockingUpdateNewMessageIndicator(context, wapThreadId);
              }
              break;
          case RECEIEVE_WAPPUSH_AUTOLAUNCH:
              String wappushUrl = intent.getStringExtra("wap_url");
              MmsLog.d(LOG_TAG, "wappushUrl:" + wappushUrl);
              WapPushMessagingNotification.notifySlAutoLanuchMessage(context, wappushUrl);
              break;
          case RECEIEVE_MWI:
              boolean isNew = intent.getBooleanExtra("new", false);
              MmsLog.d(LOG_TAG, "isNew:" + isNew);
              MwiMessagingNotification.nonBlockingUpdateNewMessageIndicator(this, true);
              break;
          default:
              break;
        }
    }

    private String getToastStr(int type) {
        String str = null;
        switch(type) {
        case MESSAGE_QUEUED:
            str = getString(R.string.message_queued);
            break;
        case SERVICE_MESSAGE_NOT_FOUND:
            str = getString(R.string.service_message_not_found);
            break;
        case INVALID_DESTINATION:
            str = getString(R.string.invalid_destination);
            break;
        case SERVICE_NOT_ACTIVATED:
            str = getString(R.string.service_not_activated);
            break;
        case SERVICE_NETWORK_PROBLEM:
            str = getString(R.string.service_network_problem);
            break;
        case DOWNLOAD_LATER:
            str = getString(R.string.download_later);
            break;
        case FDN_CHECK_FAILURE:
            str = getString(R.string.fdn_check_failure);
            break;
        case WAPPUSH_UNSUPPORTED_SCHEME:
            str = getString(R.string.error_unsupported_scheme);
            break;
        default:
            str = getString(R.string.exception_error);
            break;
        }
        return str;
    }

    private void stopWakeLock(Intent intent){
        String type = intent.getStringExtra("finish_type");
        if (type != null) {
            if (type.equals("sms")) {
                MmsLog.dpi(TXN_TAG, "release sms Wake Lock");
                SmsReceiver.releaseWakeLock();
            }
//            else if (type.equals("cb")) {
//
//            } else if (type.equals("wappush")) {
//
//            }
        }
    }

}
