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

package com.android.mms;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.XmlResourceParser;
import android.database.Cursor;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.provider.Telephony;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.android.internal.telephony.TelephonyProperties;
import com.android.mms.util.DownloadManager;
import com.android.mms.util.FeatureOption;
import com.android.mms.util.Recycler;
import com.mediatek.custom.CustomProperties;
import com.mediatek.ipmsg.util.IpMessageUtils;
import com.mediatek.mms.ext.IOpMmsConfigExt;
import com.mediatek.mms.ipmessage.IMmsConfigExt;
import com.mediatek.opmsg.util.OpMessageUtils;
import com.mediatek.provider.MtkSettingsExt;
import com.mediatek.setting.MmsPreferenceActivity;
import com.mediatek.telephony.MtkTelephonyManagerEx;
import mediatek.telephony.MtkTelephony;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

public class MmsConfig {
    private static final String TAG = "MmsConfig";
    private static final boolean DEBUG = true;
    private static final boolean LOCAL_LOGV = false;

    private static final String DEFAULT_HTTP_KEY_X_WAP_PROFILE = "x-wap-profile";
    private static final String DEFAULT_USER_AGENT = "Android-Mms/2.0";

    private static final String MMS_APP_PACKAGE = "com.android.mms";

    private static final String SMS_PROMO_DISMISSED_KEY = "sms_promo_dismissed_key";

    private static final int MAX_IMAGE_HEIGHT = 480;
    private static final int MAX_IMAGE_WIDTH = 640;
    private static final int MAX_TEXT_LENGTH = 2000;

    /// add for appservice
    public static final String MMS_APP_SERVICE_PACKAGE  = "com.mediatek.mms.appservice";
    public static final String TRANSACTION_SERVICE      = "com.mediatek.mms.appservice.TransactionService";
    public static final String WAPPUSH_RECEIVER_SERVICE = "com.mediatek.mms.appservice.wappush.WapPushReceiverService";
    public static final String SMS_RECEIVER_SERVICE     = "com.mediatek.mms.appservice.SmsReceiverService";
    public static final String MESSAGE_STATUS_SERVICE   = "com.mediatek.mms.appservice.MessageStatusService";
    public static final String CB_RECEIVER_SERVICE      = "com.mediatek.mms.appservice.CbMessageReceiverService";
    public static final String MWI_RECEIVER_SERVICE     = "com.mediatek.mms.appservice.mwi.MwiReceiverService";
    public static final String MMS_PUSH_RECEIVER_SERVICE     = "com.mediatek.mms.appservice.MmsPushReceiveService";

    /**
     * Whether to hide MMS functionality from the user (i.e. SMS only).
     */
    private static boolean mTransIdEnabled = false;
    private static int mMmsEnabled = 1;                         // default to true
    private static int mMaxMessageSize = 300 * 1024;            // default to 300k max size
    private static String mUserAgent = DEFAULT_USER_AGENT;
    private static String mUaProfTagName = DEFAULT_HTTP_KEY_X_WAP_PROFILE;
    private static String mUaProfUrl = "http://www.google.com/oha/rdf/ua-profile-kila.xml";
    private static String mHttpParams = null;
    private static String mHttpParamsLine1Key = null;
    private static String mEmailGateway = null;
    private static int mMaxImageHeight = MAX_IMAGE_HEIGHT;      // default value
    private static int mMaxImageWidth = MAX_IMAGE_WIDTH;        // default value
    private static int mRecipientLimit = 100;                   // default value
    private static int mSmsRecipientLimit = mRecipientLimit;
    private static int mMmsRecipientLimit = mRecipientLimit;
    private static int mDefaultSMSMessagesPerThread = 2000;    // default value
    private static int mDefaultMMSMessagesPerThread = 1000;     // default value
    private static int mMinMessageCountPerThread = 10;           // default value
    private static int mMaxMessageCountPerThread = 2000;        // default value
    private static int mHttpSocketTimeout = 60*1000;            // default to 1 min
    private static int mMinimumSlideElementDuration = 7;        // default to 7 sec
    private static boolean mNotifyWapMMSC = false;
    private static boolean mAllowAttachAudio = true;

    // If mEnableMultipartSMS is true, long sms messages are always sent as multi-part sms
    // messages, with no checked limit on the number of segments.
    // If mEnableMultipartSMS is false, then mSmsToMmsTextThreshold is used to determine the
    // limit of the number of sms segments before turning the long sms message into an mms message
    // For example, if mSmsToMmsTextThreshold is 4, then a long sms message with three
    // or fewer segments will be sent as a multi-part sms. When the user types more characters
    // to cause the message to be 4 segments or more, the send button will show the MMS tag to
    // indicate the message will be sent as an mms.
    private static boolean mEnableMultipartSMS = true;

    // If mEnableMultipartSMS is true and mSmsToMmsTextThreshold > 1, then multi-part SMS messages
    // will be converted into a single mms message. For example, if the mms_config.xml file
    // specifies <int name="smsToMmsTextThreshold">4</int>, then on the 5th sms segment, the
    // message will be converted to an mms.
    private static int mSmsToMmsTextThreshold = 4;
    private static int sSmsToMmsTextThresholdForCT = 7;
    private static int sSmsToMmsTextThresholdForJapan = 11;

    private static boolean mEnableSlideDuration = true;
    private static boolean mEnableMMSReadReports = true;        // key: "enableMMSReadReports"
    private static boolean mEnableSMSDeliveryReports = true;    // key: "enableSMSDeliveryReports"
    private static boolean mEnableMMSDeliveryReports = true;    // key: "enableMMSDeliveryReports"
    private static int mMaxTextLength = -1;

    // This is the max amount of storage multiplied by mMaxMessageSize that we
    // allow of unsent messages before blocking the user from sending any more
    // MMS's.
    private static int mMaxSizeScaleForPendingMmsAllowed = 4;       // default value

    // Email gateway alias support, including the master switch and different rules
    private static boolean mAliasEnabled = false;
    private static int mAliasRuleMinChars = 2;
    private static int mAliasRuleMaxChars = 48;

    private static int mMaxSubjectLength = 40;  // maximum number of characters allowed for mms
                                                // subject

    // If mEnableGroupMms is true, a message with multiple recipients, regardless of contents,
    // will be sent as a single MMS message with multiple "TO" fields set for each recipient.
    // If mEnableGroupMms is false, the group MMS setting/preference will be hidden in the settings
    // activity.
    private static boolean mEnableGroupMms = true;

    /// M: Mms size limit, default 300K.
    private static int mUserSetMmsSizeLimit = 300;

    /// M: default value
    private static int mMaxRestrictedImageHeight = 1200;
    private static int mMaxRestrictedImageWidth = 1600;

    private static boolean mDeviceStorageFull = false;

    private static IOpMmsConfigExt mOpConfigExt = null;

    private static boolean mSIMSmsAtSettingEnabled = false;

    // / M: Add for get max text size from ip message
    private static Context mContext;

    private static List<Integer> allQuickTextIds = new ArrayList<Integer>();
    private static List<String> allQuickTexts = new ArrayList<String>();

    public static IMmsConfigExt mIpConfig;

    public static final long DEFAULT_SIM_NOT_SET = MtkSettingsExt.System.DEFAULT_SIM_NOT_SET;
    public static final long DEFAULT_SIM_SETTING_ALWAYS_ASK = MtkSettingsExt.System.DEFAULT_SIM_SETTING_ALWAYS_ASK;
    public static final String SMS_SIM_SETTING = MtkSettingsExt.System.SMS_SIM_SETTING;

    private static void initPlugin(Context context) {
        mOpConfigExt = OpMessageUtils.getOpMessagePlugin().getOpMmsConfigExt();
        mIpConfig = IpMessageUtils.getIpMessagePlugin(context).getIpConfig();
    }

    public static void init(Context context) {
        if (LOCAL_LOGV) {
            Log.v(TAG, "MmsConfig.init()");
        }
        // Always put the mnc/mcc in the log so we can tell which mms_config.xml was loaded.
        Log.d(TAG, "mnc/mcc: " +
                android.os.SystemProperties.get(TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC));
        mContext = context;
        // initialize the operator plugin
        initPlugin(context);

        loadMmsSettings(context);

        // add for ipmessage
        mIpConfig.onIpInit(context);
    }

    public static boolean isSmsEnabled(Context context) {
        String defaultSmsApplication = Telephony.Sms.getDefaultSmsPackage(context);

        if (defaultSmsApplication != null && defaultSmsApplication.equals(MMS_APP_PACKAGE)) {
            return true;
        }
        return false;
    }

    public static boolean isSmsPromoDismissed(Context context) {
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(context);
        return preferences.getBoolean(SMS_PROMO_DISMISSED_KEY, false);
    }

    public static void setSmsPromoDismissed(Context context) {
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(context);
        SharedPreferences.Editor editor = preferences.edit();
        editor.putBoolean(SMS_PROMO_DISMISSED_KEY, true);
        editor.apply();
    }

    public static Intent getRequestDefaultSmsAppActivity() {
        final Intent intent = new Intent(Telephony.Sms.Intents.ACTION_CHANGE_DEFAULT);
        intent.putExtra(Telephony.Sms.Intents.EXTRA_PACKAGE_NAME, MMS_APP_PACKAGE);
        return intent;
    }

    /// M: this method is changed to use plugin
    public static int getSmsToMmsTextThreshold() {
        int limitSmsCount = mOpConfigExt.getSmsToMmsTextThreshold();
        if (limitSmsCount > 0) {
            return limitSmsCount;
        }

        TelephonyManager phone = (TelephonyManager)
            mContext.getSystemService(Context.TELEPHONY_SERVICE);
        String plmn = phone.getNetworkOperator();
        if ((plmn != null) && (plmn.length() >= 3)) {
            String mcc = plmn.substring(0,3);
            if (mcc.equals("440")) {
                return sSmsToMmsTextThresholdForJapan;
            }
        }

        if (FeatureOption.MTK_C2K_SUPPORT) {
            if (hasUSIMInserted(mContext)) {
                return sSmsToMmsTextThresholdForCT;
            }
        }

        return mSmsToMmsTextThreshold;
    }

    /**
     * M: For OM Version: check whethor has usim card.
     * @param context the context.
     * @return ture: has usim; false: not.
     */
    private static boolean hasUSIMInserted(Context context) {
        if (context == null) {
            return false;
        }
        int[] ids = SubscriptionManager.from(context).getActiveSubscriptionIdList();
        if (ids != null && ids.length > 0) {
            for (int subId : ids) {
                if (isUSimType(subId)) {
                    return true;
                }
            }
        }
        return false;
    }

    /**
     * M: For EVDO: check the sim is whether UIM or not.
     *
     * @param subId the sim's sub id.
     * @return true: UIM; false: not UIM.
     */
    private static boolean isUSimType(int subId) {
        String phoneType = MtkTelephonyManagerEx.getDefault().getIccCardType(subId);
        if (phoneType == null) {
            // Log.d(TAG, "[isUIMType]: phoneType = null");
            return false;
        }
        // Log.d(TAG, "[isUIMType]: phoneType = " + phoneType);
        return phoneType.equalsIgnoreCase("CSIM") || phoneType.equalsIgnoreCase("UIM")
                || phoneType.equalsIgnoreCase("RUIM");
    }

    public static boolean getMmsEnabled() {
        return mMmsEnabled == 1 ? true : false;
    }

    public static int getMaxMessageSize() {
        if (LOCAL_LOGV) {
            Log.v(TAG, "MmsConfig.getMaxMessageSize(): " + mMaxMessageSize);
        }
        return mMaxMessageSize;
    }

    /**
     * This function returns the value of "enabledTransID" present in mms_config file.
     * In case of single segment wap push message, this "enabledTransID" indicates whether
     * TransactionID should be appended to URI or not.
     */
    public static boolean getTransIdEnabled() {
        return mTransIdEnabled;
    }

    public static String getUserAgent() {
        /// M: @{
        String value = CustomProperties.getString(
                CustomProperties.MODULE_MMS,
                CustomProperties.USER_AGENT,
                mUserAgent);
        /// @}
        return value;
    }

    public static String getUaProfTagName() {
        return mUaProfTagName;
    }

    public static String getUaProfUrl() {
        /// M: @{
        String value = CustomProperties.getString(
                CustomProperties.MODULE_MMS,
                CustomProperties.UAPROF_URL,
                mUaProfUrl);
        /// @}
        return value;
    }

    public static String getHttpParams() {
        return mHttpParams;
    }

    public static String getHttpParamsLine1Key() {
        return mHttpParamsLine1Key;
    }

    public static String getEmailGateway() {
        return mEmailGateway;
    }

    public static int getMaxImageHeight() {
        return mMaxImageHeight;
    }

    public static int getMaxImageWidth() {
        return mMaxImageWidth;
    }

    public static int getRecipientLimit() {
        return mRecipientLimit;
    }

    // not used right now, if customer want get this value, need add config in mms_config.xml
    public static int getSmsRecipientLimit() {
        return mSmsRecipientLimit;
    }

    /// M: change this method to plugin
    public static int getMmsRecipientLimit() {
        return mOpConfigExt.getMmsRecipientLimit();
    }


    /// M: Add MmsService configure param @{
    public static Bundle getMmsServiceConfig() {
        // Operator Plugin
        return mOpConfigExt.getMmsServiceConfig();
    }
    /// @}

    public static int getMmsRetryPromptIndex() {
        return mOpConfigExt.getMmsRetryPromptIndex();
    }

    /// M: change this to plugin
    public static int getMaxTextLimit() {
        // add for ipmessage
        if (mIpConfig.getMaxTextLimit(mContext) != 0) {
            return mIpConfig.getMaxTextLimit(mContext);
        }
        return mOpConfigExt.getMaxTextLimit();
    }

    public static int getDefaultSMSMessagesPerThread() {
        return mDefaultSMSMessagesPerThread;
    }

    public static int getDefaultMMSMessagesPerThread() {
        return mDefaultMMSMessagesPerThread;
    }

    public static int getMinMessageCountPerThread() {
        return mMinMessageCountPerThread;
    }

    public static int getMaxMessageCountPerThread() {
        return mMaxMessageCountPerThread;
    }

    public static int getHttpSocketTimeout() {
        return mHttpSocketTimeout;
    }

    public static int getMinimumSlideElementDuration() {
        return mMinimumSlideElementDuration;
    }

    public static boolean getMultipartSmsEnabled() {
        return mEnableMultipartSMS;
    }

    public static boolean getSlideDurationEnabled() {
        return mEnableSlideDuration;
    }

    public static boolean getMMSReadReportsEnabled() {
        return mEnableMMSReadReports;
    }

    public static boolean getSMSDeliveryReportsEnabled() {
        return mEnableSMSDeliveryReports;
    }

    public static boolean getMMSDeliveryReportsEnabled() {
        return mEnableMMSDeliveryReports;
    }

    public static boolean getNotifyWapMMSC() {
        return mNotifyWapMMSC;
    }

    public static int getMaxSizeScaleForPendingMmsAllowed() {
        return mMaxSizeScaleForPendingMmsAllowed;
    }

    public static boolean isAliasEnabled() {
        return mAliasEnabled;
    }

    public static int getAliasMinChars() {
        return mAliasRuleMinChars;
    }

    public static int getAliasMaxChars() {
        return mAliasRuleMaxChars;
    }

    public static boolean getAllowAttachAudio() {
        return mAllowAttachAudio;
    }

    public static int getMaxSubjectLength() {
        return mMaxSubjectLength;
    }

    public static boolean getGroupMmsEnabled() {
        return mEnableGroupMms;
    }

    public static final void beginDocument(XmlPullParser parser, String firstElementName)
            throws XmlPullParserException, IOException
    {
        int type;
        while ((type = parser.next()) != XmlPullParser.START_TAG
                   && type != XmlPullParser.END_DOCUMENT) {
            ;
        }

        if (type != XmlPullParser.START_TAG) {
            throw new XmlPullParserException("No start tag found");
        }

        if (!parser.getName().equals(firstElementName)) {
            throw new XmlPullParserException("Unexpected start tag: found " + parser.getName() +
                    ", expected " + firstElementName);
        }
    }

    public static final void nextElement(XmlPullParser parser)
            throws XmlPullParserException, IOException
    {
        int type;
        while ((type = parser.next()) != XmlPullParser.START_TAG
                   && type != XmlPullParser.END_DOCUMENT) {
            ;
        }
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

    public static void setDeviceStorageFullStatus(boolean bFull) {
        mDeviceStorageFull = bFull;
    }

    public static boolean getDeviceStorageFullStatus() {
        return mDeviceStorageFull;
    }

    /// M: new feature, init defualt quick text @{
    public static void setInitQuickText(boolean init) {
        SharedPreferences sp =
            PreferenceManager.getDefaultSharedPreferences(MmsApp.getApplication());
        SharedPreferences.Editor editor = sp.edit();
        editor.putBoolean("InitQuickText", init);
        editor.commit();
    }

    public static boolean getInitQuickText() {
        SharedPreferences sp =
            PreferenceManager.getDefaultSharedPreferences(MmsApp.getApplication());
        boolean isFristInit = sp.getBoolean("InitQuickText", true);
        return isFristInit;
    }
    /// @}

    public static List<String> getQuicktexts() {
        return allQuickTexts;
    }

    public static List<Integer> getQuicktextsId() {
        return allQuickTextIds;
    }

    public static int updateAllQuicktexts() {
        Cursor cursor = mContext.getContentResolver().query(
                         MtkTelephony.MtkMmsSms.CONTENT_URI_QUICKTEXT, null, null, null, null);
        allQuickTextIds.clear();
        allQuickTexts.clear();
        String[] defaultTexts = mContext.getResources().getStringArray(
                                            R.array.default_quick_texts);
        int maxId = defaultTexts.length;
        if (cursor != null) {
            try {
                while (cursor.moveToNext()) {
                    int qtId = cursor.getInt(0);
                    allQuickTextIds.add(qtId);
                    String qtText = cursor.getString(1);
                    if (qtText != null && !qtText.isEmpty()) {
                        allQuickTexts.add(qtText);
                    } else {
                        allQuickTexts.add(defaultTexts[qtId - 1]);
                    }
                    if (qtId > maxId) {
                        maxId = qtId;
                    }
                }
            } finally {
                cursor.close();
            }
        }
        return maxId;
    }

    public static int getUserSetMmsSizeLimit(boolean isBytes) {
        if (true == isBytes) {
            return mUserSetMmsSizeLimit * 1024;
        } else {
            return mUserSetMmsSizeLimit;
        }
    }

    public static void setUserSetMmsSizeLimit(int limit) {
        mUserSetMmsSizeLimit = limit;
    }

    public static int getMaxRestrictedImageHeight() {
        return mMaxRestrictedImageHeight;
    }

    public static int getMaxRestrictedImageWidth() {
        return mMaxRestrictedImageWidth;
    }

    public static boolean getSIMSmsAtSettingEnabled() {
        return mSIMSmsAtSettingEnabled;
    }

    private static void loadMmsSettings(Context context) {
        XmlResourceParser parser = context.getResources().getXml(R.xml.mms_config);

        try {
            beginDocument(parser, "mms_config");

            while (true) {
                nextElement(parser);
                String tag = parser.getName();
                if (tag == null) {
                    break;
                }
                String name = parser.getAttributeName(0);
                String value = parser.getAttributeValue(0);
                String text = null;
                if (parser.next() == XmlPullParser.TEXT) {
                    text = parser.getText();
                }

                if (DEBUG) {
                    Log.v(TAG, "tag: " + tag + " value: " + value + " - " +
                            text);
                }
                if ("name".equalsIgnoreCase(name)) {
                    if ("bool".equals(tag)) {
                        // bool config tags go here
                        if ("enabledMMS".equalsIgnoreCase(value)) {
                            mMmsEnabled = "true".equalsIgnoreCase(text) ? 1 : 0;
                        } else if ("enabledTransID".equalsIgnoreCase(value)) {
                            mTransIdEnabled = "true".equalsIgnoreCase(text);
                        } else if ("enabledNotifyWapMMSC".equalsIgnoreCase(value)) {
                            mNotifyWapMMSC = "true".equalsIgnoreCase(text);
                        } else if ("aliasEnabled".equalsIgnoreCase(value)) {
                            mAliasEnabled = "true".equalsIgnoreCase(text);
                        } else if ("allowAttachAudio".equalsIgnoreCase(value)) {
                            mAllowAttachAudio = "true".equalsIgnoreCase(text);
                        } else if ("enableMultipartSMS".equalsIgnoreCase(value)) {
                            mEnableMultipartSMS = "true".equalsIgnoreCase(text);
                        } else if ("enableSlideDuration".equalsIgnoreCase(value)) {
                            mEnableSlideDuration = "true".equalsIgnoreCase(text);
                        } else if ("enableMMSReadReports".equalsIgnoreCase(value)) {
                            mEnableMMSReadReports = "true".equalsIgnoreCase(text);
                        } else if ("enableSMSDeliveryReports".equalsIgnoreCase(value)) {
                            mEnableSMSDeliveryReports = "true".equalsIgnoreCase(text);
                        } else if ("enableMMSDeliveryReports".equalsIgnoreCase(value)) {
                            mEnableMMSDeliveryReports = "true".equalsIgnoreCase(text);
                        } else if ("enableGroupMms".equalsIgnoreCase(value)) {
                            mEnableGroupMms = "true".equalsIgnoreCase(text);
                        } else if ("simSmsAtSettingEnabled".equalsIgnoreCase(value)) {
                            mSIMSmsAtSettingEnabled = "true".equalsIgnoreCase(text);
                        }
                    } else if ("int".equals(tag)) {
                        // int config tags go here
                        if ("maxMessageSize".equalsIgnoreCase(value)) {
                            mMaxMessageSize = Integer.parseInt(text);
                        } else if ("maxImageHeight".equalsIgnoreCase(value)) {
                            mMaxImageHeight = Integer.parseInt(text);
                        } else if ("maxImageWidth".equalsIgnoreCase(value)) {
                            mMaxImageWidth = Integer.parseInt(text);
                        } else if ("maxRestrictedImageHeight".equalsIgnoreCase(value)) {
                            mMaxRestrictedImageHeight = Integer.parseInt(text);
                        } else if ("maxRestrictedImageWidth".equalsIgnoreCase(value)) {
                            mMaxRestrictedImageWidth = Integer.parseInt(text);
                        } else if ("defaultSMSMessagesPerThread".equalsIgnoreCase(value)) {
                            mDefaultSMSMessagesPerThread = Integer.parseInt(text);
                        } else if ("defaultMMSMessagesPerThread".equalsIgnoreCase(value)) {
                            mDefaultMMSMessagesPerThread = Integer.parseInt(text);
                        } else if ("minMessageCountPerThread".equalsIgnoreCase(value)) {
                            mMinMessageCountPerThread = Integer.parseInt(text);
                        } else if ("maxMessageCountPerThread".equalsIgnoreCase(value)) {
                            mMaxMessageCountPerThread = Integer.parseInt(text);
                        } else if ("recipientLimit".equalsIgnoreCase(value)) {
                            /// M: Operator Plugin
                            mOpConfigExt.setMmsRecipientLimit(Integer.parseInt(text));
                        } else if ("smsRecipientLimit".equalsIgnoreCase(value)) {
                            mSmsRecipientLimit = Integer.parseInt(text);
                            if (mSmsRecipientLimit < 0) {
                                mSmsRecipientLimit = Integer.MAX_VALUE;
                            }
                        } else if ("httpSocketTimeout".equalsIgnoreCase(value)) {
                            mOpConfigExt.setHttpSocketTimeout(Integer.parseInt(text));
                        } else if ("minimumSlideElementDuration".equalsIgnoreCase(value)) {
                            mMinimumSlideElementDuration = Integer.parseInt(text);
                        } else if ("maxSizeScaleForPendingMmsAllowed".equalsIgnoreCase(value)) {
                            mMaxSizeScaleForPendingMmsAllowed = Integer.parseInt(text);
                        } else if ("aliasMinChars".equalsIgnoreCase(value)) {
                            mAliasRuleMinChars = Integer.parseInt(text);
                        } else if ("aliasMaxChars".equalsIgnoreCase(value)) {
                            mAliasRuleMaxChars = Integer.parseInt(text);
                        } else if ("smsToMmsTextThreshold".equalsIgnoreCase(value)) {
                            /// M: Operator Plugin
                            mOpConfigExt.setSmsToMmsTextThreshold(Integer.parseInt(text));
                        } else if ("maxMessageTextSize".equalsIgnoreCase(value)) {
                            /// M: Operator Plugin
                            mOpConfigExt.setMaxTextLimit(Integer.parseInt(text));
                        } else if ("maxSubjectLength".equalsIgnoreCase(value)) {
                            mMaxSubjectLength = Integer.parseInt(text);
                        }
                    } else if ("string".equals(tag)) {
                        // string config tags go here
                        if ("userAgent".equalsIgnoreCase(value)) {
                            mUserAgent = text;
                        } else if ("uaProfTagName".equalsIgnoreCase(value)) {
                            mUaProfTagName = text;
                        } else if ("uaProfUrl".equalsIgnoreCase(value)) {
                            mUaProfUrl = text;
                        } else if ("httpParams".equalsIgnoreCase(value)) {
                            mHttpParams = text;
                        } else if ("httpParamsLine1Key".equalsIgnoreCase(value)) {
                            mHttpParamsLine1Key = text;
                        } else if ("emailGatewayNumber".equalsIgnoreCase(value)) {
                            mEmailGateway = text;
                        }
                    }
                }
            }
        } catch (XmlPullParserException e) {
            Log.e(TAG, "loadMmsSettings caught ", e);
        } catch (NumberFormatException e) {
            Log.e(TAG, "loadMmsSettings caught ", e);
        } catch (IOException e) {
            Log.e(TAG, "loadMmsSettings caught ", e);
        } finally {
            parser.close();
        }

        String errorStr = null;

        if (getMmsEnabled() && mUaProfUrl == null) {
            errorStr = "uaProfUrl";
        }

        if (errorStr != null) {
            String err =
                String.format("MmsConfig.loadMmsSettings mms_config.xml missing %s setting",
                        errorStr);
            Log.e(TAG, err);
        }
    }

    public static final String MMS_ENABLE_TO_SEND_DELIVERY_REPORT_KEY
            = "pref_key_mms_enable_to_send_delivery_reports";

    public static void setMmsConfig(Intent intent, int subId) {
        Log.i(TAG, "setMmsConfig intent: " + intent + ", subId:" + subId);
        // mms auto download
        if (DownloadManager.getInstance().isAuto(subId)) {
            intent.putExtra("auto_retrieve", 1);
        } else {
            intent.putExtra("auto_retrieve", 0);
        }

        // mms/sms auto delete
        if (Recycler.isAutoDeleteEnabled(mContext)) {
            int smsLimit = Recycler.getSmsRecycler().getMessageLimit(mContext);
            int mmsLimit = Recycler.getMmsRecycler().getMessageLimit(mContext);
            intent.putExtra("auto_delete", 1);
            intent.putExtra("sms_limit", smsLimit);
            intent.putExtra("mms_limit", mmsLimit);
        } else {
            intent.putExtra("auto_delete", 0);
        }

        // group mms
        if (MmsPreferenceActivity.getIsGroupMmsEnabled(mContext)) {
            intent.putExtra("group_mms_enable", 1);
        } else {
            intent.putExtra("group_mms_enable", 0);
        }

        // mms service config
        intent.putExtra("mms_service_config", getMmsServiceConfig());

        // group wap mmsc
        if (getNotifyWapMMSC()) {
            intent.putExtra("notify_wap_mmsc", 1);
        } else {
            intent.putExtra("notify_wap_mmsc", 0);
        }

        // delivery report
        SharedPreferences prefs =
                PreferenceManager.getDefaultSharedPreferences(mContext);
        boolean enable = prefs.getBoolean(Integer.toString(subId) + "_" +
                MMS_ENABLE_TO_SEND_DELIVERY_REPORT_KEY, true);
        if (enable) {
            intent.putExtra("delivery_report", 1);
        } else {
            intent.putExtra("delivery_report", 0);
        }
    }

    public static void setSmsConfig(Intent intent) {
        // sms auto delete
        if (Recycler.isAutoDeleteEnabled(mContext)) {
            int smsLimit = Recycler.getSmsRecycler().getMessageLimit(mContext);
            intent.putExtra("auto_delete", 1);
            intent.putExtra("sms_limit", smsLimit);
        } else {
            intent.putExtra("auto_delete", 0);
        }
    }
}
