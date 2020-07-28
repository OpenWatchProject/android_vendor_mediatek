/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2009 The Android Open Source Project
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

import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.os.UserHandle;
import android.provider.Telephony;
import android.util.Log;
import android.telephony.PhoneNumberUtils;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;

import com.android.internal.telephony.PhoneConstants;

import java.util.HashMap;
import java.util.List;

public class MmsConfig {
    public static final String TAG = "MmsAppService/MmsConfig";
    Context mContext;

    public static int DEFAULT_SMS_LIMIT = 10000;
    public static int DEFAULT_MMS_LIMIT = 1000;

    public static final long THREAD_ALL = -1;
    public static final long THREAD_NONE = -2;

    public static final String ACTION_UPDATE_NOTIFICATION =
            "com.mediatek.mms.appservice.notification";
    public static final String ACTION_SHOW_TOAST = "com.mediatek.mms.appservice.toast";
    public static final String ACTION_STOP_SERVICE =
        "com.mediatek.mms.appservice.stopservice";

    // type of notifications
    public static int RECEIEVE_CB  = 1;
    public static int RECEIEVE_NEW = 2;
    public static int SEND_FAILED  = 3;
    public static int DOWNLOAD_FAILED = 4;
    public static int STATUS_MESSAGE = 5;
    public static int CANCEL_DOWNLOAD_FAILED = 6;
    public static int CANCEL_SEND_FAILED = 7;
    public static int CLASS_0 = 8;
    public static final int RECEIEVE_WAPPUSH  = 9;
    public static final int RECEIEVE_WAPPUSH_AUTOLAUNCH  = 10;
    public static final int RECEIEVE_MWI  = 11;
    private static final String MMS_APP_PACKAGE = "com.android.mms";
    // tyep of toast
    public static final int MESSAGE_QUEUED = 1;
    public static final int SERVICE_MESSAGE_NOT_FOUND = 2; // mms expiry passed
    public static final int INVALID_DESTINATION = 3;
    public static final int SERVICE_NOT_ACTIVATED = 4;
    public static final int SERVICE_NETWORK_PROBLEM = 5;
    public static final int DOWNLOAD_LATER = 6;
    public static final int FDN_CHECK_FAILURE = 7;
    public static final int WAPPUSH_UNSUPPORTED_SCHEME = 8;

    // Email gateway alias support, including the master switch and different rules
    private static boolean mAliasEnabled = false;
    private static int mAliasRuleMinChars = 2;
    private static int mAliasRuleMaxChars = 48;
    private static String mEmailGateway = null;
    private static int mEncodingType = 0;

    // mms config
    private static boolean mGroupMmsEnable = false;
    private static boolean mNotifyWapMmsc = false;
    private static Bundle mMmsServiceConfig = null;
    private static long mAutoDownloadState = 0;
    private static long mSendDeliveryReport = 0;
    // common config
    private static boolean mIsAutoDelete = false;
    private static int mSmsLimit = DEFAULT_SMS_LIMIT;
    private static int mMmsLimit = DEFAULT_MMS_LIMIT;


    public void MmsConfig(Context context) {
        mContext = context;
    }

    public static void print() {
        Log.i(TAG, "mGroupMmsEnable:" + mGroupMmsEnable
            + ", mNotifyWapMmsc:" + mNotifyWapMmsc
            + ", mMmsServiceConfig:" + mMmsServiceConfig
            + ", mAutoDownloadState:" + mAutoDownloadState
            + ", mSendDeliveryReport:" + mSendDeliveryReport
            + ", mIsAutoDelete:" + mIsAutoDelete
            + ", mSmsLimit:" + mSmsLimit
            + ", mMmsLimit:" + mMmsLimit);
    }

    public static boolean getIsGroupMmsEnabled() {
        return mGroupMmsEnable;
    }

    public static void setAutoDownload(Intent intent) {
        int sub = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY,
                            SubscriptionManager.INVALID_SUBSCRIPTION_ID);
        int auto = intent.getIntExtra("auto_retrieve", -1);
        if (auto != -1 && sub != SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
            if (auto == 1) {
                setAutoDownload(sub, true);
            } else if (auto == 0) {
                setAutoDownload(sub, false);
            }
        }
    }

    public static boolean isSmsEnabled(Context context) {

        String defaultSmsApplication = Telephony.Sms.getDefaultSmsPackage(context);

        if (defaultSmsApplication != null && defaultSmsApplication.equals(MMS_APP_PACKAGE)) {
            return true;
        }
        return false;
    }

    private static void setAutoDownload(int subId, boolean isAuto) {
        // subId should be in [1, 31]
        Log.i(TAG, "setAutoDownload, subId: " + subId + ", isAuto:" + isAuto);
        if (subId < 0 || subId >= 32) {
            return;
        }
        if (isAuto) {
           mAutoDownloadState = mAutoDownloadState | (1 << subId);
        } else {
           mAutoDownloadState = mAutoDownloadState &~(1 << subId);
        }
        Log.i(TAG, "new mAutoDownloadState:" + mAutoDownloadState);
    }

    public static boolean isAutoDownload(int subId) {
        boolean auto = ((mAutoDownloadState >> subId) % 2 == 1);
        Log.i(TAG, "isAutoDownload. subId:" + subId + ", isAuto:" + auto);
        return auto;
    }

    public static void setEnableSendDeliveryReport(Intent intent) {
        int subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY,
                            SubscriptionManager.INVALID_SUBSCRIPTION_ID);
        int deliveryReport = intent.getIntExtra("delivery_report", -1);
        if (deliveryReport != -1 &&
                subId != SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
            if (deliveryReport == 1) {
                mSendDeliveryReport = mSendDeliveryReport | (1 << subId);
            } else if (deliveryReport == 0) {
                mSendDeliveryReport = mSendDeliveryReport &~(1 << subId);
            }
            Log.i(TAG, "setEnableSendDeliveryReport, mSendDeliveryReport=" + mSendDeliveryReport);
        }
    }

    public static boolean isEnableSendDeliveryReport(int subId) {
        boolean deliveryReport = ((mSendDeliveryReport >> subId) % 2 == 1);
        Log.i(TAG, "isEnableSendDeliveryReport. subId:" + subId
                + ", deliveryReport:" + deliveryReport);
        return deliveryReport;
    }

    public static Bundle getMmsServiceConfig() {
         return mMmsServiceConfig;
    }

    public static boolean getNotifyWapMMSC() {
        return mNotifyWapMmsc;
    }

    public static void setMmsConfig(Intent intent) {
        Log.i(TAG, "setMmsConfig. intent:" + intent);

        int groupMms = intent.getIntExtra("group_mms_enable", -1);
        if (groupMms == 1) {
            mGroupMmsEnable = true;
        } else if (groupMms == 0) {
            mGroupMmsEnable = false;
        }
        setEnableSendDeliveryReport(intent);
        setAutoDownload(intent);

        int notify = intent.getIntExtra("notify_wap_mmsc", -1);
        if (notify == 1) {
            mNotifyWapMmsc = true;
        } else if (notify == 0) {
            mNotifyWapMmsc = false;
        }

        Bundle config = intent.getBundleExtra("mms_service_config");
        if (config != null) {
            mMmsServiceConfig = config;
        }

        setAutoDeleteConfig(intent);
    }

    public static void setAutoDeleteConfig(Intent intent) {
        int autoDelete = intent.getIntExtra("auto_delete", -1);
        if (autoDelete == 1) {
            mIsAutoDelete = true;
        } else if (autoDelete == 0) {
            mIsAutoDelete = false;
        }
        int smsLimit = intent.getIntExtra("sms_limit", -1);
        if (smsLimit != -1) {
           mSmsLimit = smsLimit;
        }
        int mmsLimit = intent.getIntExtra("mms_limit", -1);
        if (mmsLimit != -1) {
           mMmsLimit = mmsLimit;
        }
    }

    public static boolean isAutoDeleteEnabled() {
        return mIsAutoDelete;
    }

    public static int getSmsLimitPerThread() {
        return mSmsLimit;
    }

    public static int getMmsLimitPerThread() {
        return mMmsLimit;
    }

    /// M:
    /**
     * Notes:for CMCC customization,whether to enable SL automatically lanuch.
     * default set false
     */
    private static boolean mSlAutoLanuchEnabled = false;
    public static boolean getSlAutoLanuchEnabled() {
        return mSlAutoLanuchEnabled;
    }

    public static String getSmsSaveLocation(int subId) {
      // TODO:

        return "Phone";
    }

    public static void updateMessagingNotification(Context context, int notificationType,
            long threadId, boolean isStatusMessage, Uri statusMessageUri ) {
        Intent intent = new Intent();
        intent.setAction(ACTION_UPDATE_NOTIFICATION);
        intent.putExtra("notification_type", notificationType);
        intent.putExtra("thread_id", threadId);
        intent.putExtra("isStatusMessage", isStatusMessage);
        if (statusMessageUri != null) {
            intent.putExtra("msgUri", statusMessageUri.toString());
        }
        Log.d(TAG, "updateMessagingNotification, notificationType=" + notificationType);
        intent.setPackage("com.android.mms");
        context.sendBroadcastAsUser(intent, UserHandle.CURRENT);
    }

    public static void updateFailedNotification(Context context, boolean isDownload,
                long threadId, boolean noisy) {
        Intent intent = new Intent();
        intent.setAction(ACTION_UPDATE_NOTIFICATION);
        if (isDownload) {
            intent.putExtra("notification_type", DOWNLOAD_FAILED);
        } else {
            intent.putExtra("notification_type", SEND_FAILED);
        }
        intent.putExtra("thread_id", threadId);
        intent.putExtra("noisy", noisy);
        intent.setPackage("com.android.mms");
        context.sendBroadcastAsUser(intent, UserHandle.CURRENT);
    }

    public static void updateClassZeroNotification(Context context, String address) {
        Intent intent = new Intent();
        intent.setAction(ACTION_UPDATE_NOTIFICATION);
        intent.putExtra("notification_type", CLASS_0);
        intent.putExtra("address", address);
        intent.setPackage("com.android.mms");
        context.sendBroadcastAsUser(intent, UserHandle.CURRENT);
    }

    public static void updateCbNotification(Context context, int subId) {
        Intent intent = new Intent();
        intent.setAction(ACTION_UPDATE_NOTIFICATION);
        intent.putExtra("notification_type", RECEIEVE_CB);
        intent.putExtra("subId", subId);
        intent.setPackage("com.android.mms");
        context.sendBroadcastAsUser(intent, UserHandle.CURRENT);
    }

    public static void updateWappushNotification(Context context, String wapUri) {
        Intent intent = new Intent();
        intent.setAction(ACTION_UPDATE_NOTIFICATION);
        intent.putExtra("notification_type", RECEIEVE_WAPPUSH);
        intent.putExtra("wap_uri", wapUri);
        intent.setPackage("com.android.mms");
        context.sendBroadcastAsUser(intent, UserHandle.CURRENT);
    }

    public static void updateWappushSlAutoLanuch(Context context, String wapUrl) {
        Intent intent = new Intent();
        intent.setAction(ACTION_UPDATE_NOTIFICATION);
        intent.putExtra("notification_type", RECEIEVE_WAPPUSH_AUTOLAUNCH);
        intent.putExtra("wap_url", wapUrl);
        intent.setPackage("com.android.mms");
        context.sendBroadcastAsUser(intent, UserHandle.CURRENT);
    }

    public static void updateMwiNotification(Context context, boolean isNew) {
        Intent intent = new Intent();
        intent.setAction(ACTION_UPDATE_NOTIFICATION);
        intent.putExtra("notification_type", RECEIEVE_MWI);
        intent.putExtra("new", isNew);
        intent.setPackage("com.android.mms");
        context.sendBroadcastAsUser(intent, UserHandle.CURRENT);
    }

    public static void showToast(Context context, int type) {
        Log.i(TAG, "showToast. type:" + type);
        Intent intent = new Intent();
        intent.setAction(ACTION_SHOW_TOAST);
        intent.putExtra("toast_type", type);
        intent.setPackage("com.android.mms");
        context.getApplicationContext().sendBroadcastAsUser(intent, UserHandle.CURRENT);
    }

    public static void finishStartingService(Context context,String type) {
        Log.i(LogTag.TXN_TAG, "finishStartingService type:" + type);
        Intent intent = new Intent();
        intent.setAction(ACTION_STOP_SERVICE);
        intent.putExtra("finish_type", type);
        intent.setPackage("com.android.mms");
        context.sendBroadcastAsUser(intent, UserHandle.CURRENT);
    }

    public static void setEmailGateway(String emailGateway) {
        mEmailGateway = emailGateway;
    }

    public static String getEmailGateway() {
        return mEmailGateway;
    }

    public static void setAliasEnabled(boolean aliasEnabled) {
        mAliasEnabled = aliasEnabled;
    }

    public static void setAliasMinChars(int minChars) {
        mAliasRuleMinChars = minChars;
    }

    public static void setAliasMaxChars(int maxChars) {
        mAliasRuleMaxChars = maxChars;
    }

    // An alias (or commonly called "nickname") is:
    // Nickname must begin with a letter.
    // Only letters a-z, numbers 0-9, or . are allowed in Nickname field.
    public static boolean isAlias(String string) {
        if (!mAliasEnabled) {
            return false;
        }

        int len = string == null ? 0 : string.length();

        if (len < mAliasRuleMinChars || len > mAliasRuleMaxChars) {
            return false;
        }

        if (!Character.isLetter(string.charAt(0))) {    // Nickname begins with a letter
            return false;
        }
        for (int i = 1; i < len; i++) {
            char c = string.charAt(i);
            if (!(Character.isLetterOrDigit(c) || c == '.')) {
                return false;
            }
        }

        return true;
    }

    public static boolean isWellFormedSmsAddress(String address) {
        // MTK-START [mtk04070][120104][ALPS00109412]Solve
        // "can't send MMS with MSISDN in international format"
        // Merge from ALPS00089029
        if (!isDialable(address)) {
            return false;
        }
        // MTK-END [mtk04070][120104][ALPS00109412]Solve
        // "can't send MMS with MSISDN in international format"

        String networkPortion =
                PhoneNumberUtils.extractNetworkPortion(address);

        return (!(networkPortion.equals("+")
                  || TextUtils.isEmpty(networkPortion)))
               && isDialable(networkPortion);
    }

    private static boolean isDialable(String address) {
        for (int i = 0, count = address.length(); i < count; i++) {
            if (!isDialable(address.charAt(i))) {
                return false;
            }
        }
        return true;
    }

    /** M: True if c is ISO-LATIN characters 0-9, *, # , +, WILD  */
    private static boolean isDialable(char c) {
        return (c >= '0' && c <= '9') || c == '*'
            || c == '#' || c == '+' || c == 'N' || c == '(' || c == ')';
    }

    public static void setEncodingType(int encodingType) {
        mEncodingType = encodingType;
    }

    public static int getEncodingType() {
        return mEncodingType;
    }

}
