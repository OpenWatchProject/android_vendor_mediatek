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

package com.android.mms.ui;

import android.app.Activity;
import android.app.ActivityManager;
import android.app.ActivityManager.RunningAppProcessInfo;
import android.app.ActivityManager.RunningTaskInfo;
import android.app.AlertDialog;
import android.app.FragmentTransaction;
import android.app.ProgressDialog;
import android.content.ActivityNotFoundException;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.content.res.Resources;
import android.database.Cursor;
import android.database.sqlite.SqliteWrapper;
import android.graphics.Bitmap;
import android.graphics.Bitmap.CompressFormat;
import android.graphics.Bitmap.Config;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.PorterDuff.Mode;
import android.graphics.PorterDuffXfermode;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.Typeface;
import android.location.Country;
import android.location.CountryDetector;
import android.media.CamcorderProfile;
import android.media.RingtoneManager;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Uri;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.StatFs;
import android.os.SystemProperties;
import android.os.storage.StorageManager;
import android.os.storage.StorageVolume;
import android.preference.PreferenceManager;
import android.provider.BaseColumns;
import android.provider.ContactsContract;
import android.provider.ContactsContract.CommonDataKinds.Email;
import android.provider.ContactsContract.Contacts;
import android.provider.MediaStore;
import android.provider.Settings;
import android.provider.Telephony.Mms;
import android.provider.Telephony.Sms;
import android.support.v4.content.FileProvider;
import android.telephony.PhoneNumberUtils;
import android.telephony.ServiceState;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.Spannable;
import android.text.SpannableString;
import android.text.TextUtils;
import android.text.format.DateUtils;
import android.text.format.Time;
import android.text.style.StyleSpan;
import android.text.style.URLSpan;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.WindowManager;
import android.webkit.MimeTypeMap;
import android.widget.TextView;
import android.widget.Toast;

import com.android.i18n.phonenumbers.AsYouTypeFormatter;
import com.android.i18n.phonenumbers.PhoneNumberUtil;
import com.android.internal.telephony.PhoneConstants;
import com.android.mms.LogTag;
import com.android.mms.MmsApp;
import com.android.mms.MmsConfig;
import com.android.mms.R;
import com.android.mms.TempFileProvider;
import com.android.mms.data.Contact;
import com.android.mms.data.Conversation;
import com.android.mms.data.WorkingMessage;
import com.android.mms.model.CarrierContentRestriction;
import com.android.mms.model.MediaModel;
import com.android.mms.model.SlideModel;
import com.android.mms.model.SlideshowModel;
import com.android.mms.transaction.MessagingNotification;
import com.android.mms.ui.DeliveryReportActivity.MmsReportStatus;
import com.android.mms.util.FeatureOption;
import com.android.mms.util.MmsContentType;
import com.android.mms.util.MmsLog;
import com.google.android.mms.MmsException;
import com.google.android.mms.pdu.CharacterSets;
import com.google.android.mms.pdu.EncodedStringValue;
import com.google.android.mms.pdu.MultimediaMessagePdu;
import com.google.android.mms.pdu.NotificationInd;
import com.google.android.mms.pdu.PduBody;
import com.google.android.mms.pdu.PduPart;
import com.google.android.mms.pdu.RetrieveConf;
import com.google.android.mms.pdu.SendReq;
import com.mediatek.android.mms.pdu.MtkMultimediaMessagePdu;
import com.mediatek.android.mms.pdu.MtkPduHeaders;
import com.mediatek.android.mms.pdu.MtkPduPersister;
import com.mediatek.cb.cbmsg.CbMessageListActivity;
import com.mediatek.cb.cbmsg.CbMessagingNotification;
import com.mediatek.internal.telephony.IMtkSms;
import com.mediatek.internal.telephony.IMtkTelephonyEx;
import com.mediatek.ipmsg.util.IpMessageUtils;
import com.mediatek.mms.ext.IOpMessageUtilsExt;
import com.mediatek.mms.util.DrmUtilsEx;
import com.mediatek.omadrm.OmaDrmStore;
import com.mediatek.opmsg.util.OpMessageUtils;
import com.mediatek.setting.NotificationPreferenceActivity;
import com.mediatek.storage.StorageManagerEx;
import com.mediatek.telephony.MtkTelephonyManagerEx;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

import mediatek.telephony.MtkTelephony;
import com.mediatek.android.mms.pdu.MtkSendReq;
import com.mediatek.android.mms.pdu.MtkRetrieveConf;
/* Mem opt*/
import android.graphics.drawable.BitmapDrawable;
import android.widget.ImageView;

/**
 * An utility class for managing messages.
 */
public class MessageUtils {
    interface ResizeImageResultCallback {
        void onResizeResult(PduPart part, boolean append);
    }

    private static final String TAG = LogTag.TAG;

    private static String sLocalNumber;
    private static String[] sNoSubjectStrings;

    // Cache of both groups of space-separated ids to their full
    // comma-separated display names, as well as individual ids to
    // display names.
    // TODO: is it possible for canonical address ID keys to be
    // re-used?  SQLite does reuse IDs on NULL id_ insert, but does
    // anything ever delete from the mmssms.db canonical_addresses
    // table?  Nothing that I could find.
    private static final Map<String, String> sRecipientAddress =
            new ConcurrentHashMap<String, String>(20 /* initial capacity */);


    /**
     * MMS address parsing data structures
     */
    // allowable phone number separators
    private static final char[] NUMERIC_CHARS_SUGAR = {
        '-', '.', ',', '(', ')', ' ', '/', '\\', '*', '#', '+', 'N'
    };

    public static final Uri THREAD_SETTINGS_URI = Uri.parse("content://mms-sms/thread_settings/");

    private static HashMap numericSugarMap = new HashMap(NUMERIC_CHARS_SUGAR.length);

    static {
        for (int i = 0; i < NUMERIC_CHARS_SUGAR.length; i++) {
            numericSugarMap.put(NUMERIC_CHARS_SUGAR[i], NUMERIC_CHARS_SUGAR[i]);
        }
    }

    /// M: Code analyze 006, For fix bug ALPS00234739, draft can't be saved
    // after share the edited picture to the same ricipient.Remove old Mms draft
    // in conversation list instead of compose view. @{
    private static Thread mRemoveOldMmsThread;
    /// @}
    /// M: Code analyze 011, For fix bug ALPS00246535, prompt
    // "Please input SD card" after back from attachment list screen. @{
    public static final String SDCARD_1 = "/mnt/sdcard";
    public static final String SDCARD_2 = "/mnt/sdcard2";
    /// @}
    /// M: Code analyze 021, For fix bug ALPS00279524, The "JE" about "MMS"
    // pops up after we launch "Messaging" again. @{
    public static final int NUMBER_OF_RESIZE_ATTEMPTS = 4;
    /// @}
    /* Mem opt*/
    private static SlideModel slide = null;
    private static String sLocalNumber2;

    private static final String DB_PATH =
        "/data/data/com.android.providers.telephony/databases/mmssms.db";
    /// M: for showing notification when view mms with video player in full screen model.
    private static final String EXTRA_FULLSCREEN_NOTIFICATION =
        "mediatek.intent.extra.FULLSCREEN_NOTIFICATION";
    /// M: Action for contact selection intent
    public static final String ACTION_CONTACT_SELECTION =
        "mediatek.intent.action.contacts.list.PICKMULTIPHONEANDEMAILS";

    public static IOpMessageUtilsExt sOpMessageUtilsExt;

  /// M: fix bug ALPS01523754.set google+ pic as wallpaper.@{
    private static final String TEMP_WALLPAPER = "tempWallpaper";
/// @}
    public static final int SHOW_INVITE_PANEL = 1;
    public static final int UPDATE_SENDBUTTON = 2;

    public static final int[] SubBackgroundLightRes = new int[] {
        com.mediatek.internal.R.drawable.sim_light_blue,
        com.mediatek.internal.R.drawable.sim_light_orange,
        com.mediatek.internal.R.drawable.sim_light_green,
        com.mediatek.internal.R.drawable.sim_light_purple
    };

    public static final String MMS_SHARED_FILES_FOLDER_NAME = "voicemails";
    public static final String MMS_SHARED_FILE_PROVIDER_AUTHORITIES = "com.android.mms.files";
    private static final String TEMP_FILE_TAG = "MmsTemp";

    private MessageUtils() {
        // Forbidden being instantiated.
    }

    public static void init(Context context) {
        sOpMessageUtilsExt = OpMessageUtils.getOpMessagePlugin().getOpMessageUtilsExt();
        sOpMessageUtilsExt.setExtendedAudioType(
                CarrierContentRestriction.getSupportedAudioTypes());
    }

    /** M: google jb.mr1 patch
     * cleanseMmsSubject will take a subject that's says, "<Subject: no subject>", and return
     * a null string. Otherwise it will return the original subject string.
     * @param context a regular context so the function can grab string resources
     * @param subject the raw subject
     * @return
     */
    public static String cleanseMmsSubject(Context context, String subject) {
        if (TextUtils.isEmpty(subject)) {
            return subject;
        }
        if (sNoSubjectStrings == null) {
            sNoSubjectStrings =
                    context.getResources().getStringArray(R.array.empty_subject_strings);

        }
        final int len = sNoSubjectStrings.length;
        for (int i = 0; i < len; i++) {
            if (subject.equalsIgnoreCase(sNoSubjectStrings[i])) {
                return null;
            }
        }
        return subject;
    }

    public static Bitmap getCircularBitmap(Bitmap bitmap) {
        Bitmap output = Bitmap.createBitmap(bitmap.getWidth(),
        bitmap.getHeight(), Config.ARGB_8888);
        Canvas canvas = new Canvas(output);

        final int color = 0xff424242;
        final Paint paint = new Paint();
        final Rect rect = new Rect(0, 0, bitmap.getWidth(), bitmap.getHeight());
        final RectF rectF = new RectF(rect);
        final float roundPx = bitmap.getWidth() / 2;

        paint.setAntiAlias(true);
        canvas.drawARGB(0, 0, 0, 0);
        paint.setColor(color);
        canvas.drawRoundRect(rectF, roundPx, roundPx, paint);

        paint.setXfermode(new PorterDuffXfermode(Mode.SRC_IN));
        canvas.drawBitmap(bitmap, rect, rect, paint);
        return output;
    }

    // check if the sub is active.
    public static boolean isSmsSubIdActive(Context context, int subId) {
        List<SubscriptionInfo> subinfoList =
            SubscriptionManager.from(context).getActiveSubscriptionInfoList();
        if (subinfoList == null || subinfoList.size() == 0) {
            return false;
        }
        for (SubscriptionInfo subInfo : subinfoList) {
            if (subInfo.getSubscriptionId() == subId) {
                return true;
            }
        }
        return false;
    }

    // / M: Code analyze 027, new feature, to improve the performance of Mms. @{
    public static String getMessageDetails(Context context, MessageItem msgItem) {
        if (msgItem == null) {
            return null;
        }

        if ("mms".equals(msgItem.mType)) {
            int type = msgItem.mMessageType;
            switch (type) {
                case MtkPduHeaders.MESSAGE_TYPE_NOTIFICATION_IND:
                    return getNotificationIndDetails(context, msgItem);
                case MtkPduHeaders.MESSAGE_TYPE_RETRIEVE_CONF:
                case MtkPduHeaders.MESSAGE_TYPE_SEND_REQ:
                    return getMultimediaMessageDetails(context, msgItem);
                default:
                    Log.w(TAG, "No details could be retrieved.");
                    return "";
            }
        } else {
            return getTextMessageDetails(context, msgItem);
        }
    }
    /// @}

    // / M: Code analyze 027, new feature, to improve the performance of Mms. @{
    private static String getNotificationIndDetails(Context context, MessageItem msgItem) {
    /// @}
        StringBuilder details = new StringBuilder();
        Resources res = context.getResources();

        // / M: Code analyze 027, new feature, to improve the performance of
        // Mms. @{
        Uri uri = ContentUris.withAppendedId(Mms.CONTENT_URI, msgItem.mMsgId);
        /// @}
        NotificationInd nInd;

        try {
            // / M: Code analyze 027, new feature, to improve the performance of
            // Mms. @{
            nInd = (NotificationInd) MtkPduPersister.getPduPersister(context).load(uri);
            /// @}
        } catch (MmsException e) {
            Log.e(TAG, "Failed to load the message: " + uri, e);
            return context.getResources().getString(R.string.cannot_get_details);
        }

        // Message Type: Mms Notification.
        details.append(res.getString(R.string.message_type_label));
        details.append(res.getString(R.string.multimedia_notification));

        /// M: Code analyze 026, new feature, show the service center number.
        // @{
        details.append('\n');
        details.append(res.getString(R.string.service_center_label));
        details.append(!TextUtils.isEmpty(msgItem.mServiceCenter) ? msgItem.mServiceCenter : "");
        /// @}

        // From: ***
        String from = extractEncStr(context, nInd.getFrom());
        details.append('\n');
        details.append(res.getString(R.string.from_label));
        details.append(!TextUtils.isEmpty(from) ? from :
                                 res.getString(R.string.hidden_sender_address));

        // Date: ***
        details.append('\n');
        details.append(res.getString(
                                R.string.expire_on,
                                MessageUtils.formatTimeStampString(
                                        context, nInd.getExpiry() * 1000L, true)));

        // Subject: ***
        details.append('\n');
        details.append(res.getString(R.string.subject_label));

        EncodedStringValue subject = nInd.getSubject();
        if (subject != null) {
            details.append(subject.getString());
        }

        // Message class: Personal/Advertisement/Infomational/Auto
        details.append('\n');
        details.append(res.getString(R.string.message_class_label));
        details.append(new String(nInd.getMessageClass()));

        // Message size: *** KB
        details.append('\n');
        details.append(res.getString(R.string.message_size_label));
        details.append(String.valueOf((nInd.getMessageSize() + 1023) / 1024));
        details.append(context.getString(R.string.kilobyte));

        return details.toString();
    }
    /// @}

    /// M: @{
    // / M: Code analyze 027, new feature, to improve the performance of Mms. @{
    private static String getMultimediaMessageDetails(Context context, MessageItem msgItem) {
        if (msgItem.mMessageType == MtkPduHeaders.MESSAGE_TYPE_NOTIFICATION_IND) {
            return getNotificationIndDetails(context, msgItem);
        }

        StringBuilder details = new StringBuilder();
        Resources res = context.getResources();

        // / M: Code analyze 027, new feature, to improve the performance of
        // Mms. @{
        Uri uri = ContentUris.withAppendedId(Mms.CONTENT_URI, msgItem.mMsgId);
        /// @}
        /* Handle date sent if exist */
        MultimediaMessagePdu msgPdu;
        try {
            msgPdu = (MultimediaMessagePdu) MtkPduPersister.getPduPersister(
                    context).load(uri);
        } catch (MmsException e) {
            Log.e(TAG, "Failed to load the message: " + uri, e);
            return context.getResources().getString(R.string.cannot_get_details);
        }

        // Message Type: Text message.
        initializeMsgDetails(context, details, res, msgPdu);

        // SentDate: ***
        MtkSendReq sendReq;
        MtkRetrieveConf recvReq;
        long dataSent = 0L;

        if (msgPdu instanceof MtkSendReq) {
            sendReq = (MtkSendReq) msgPdu;
            dataSent = sendReq.getDateSent();
        } else if (msgPdu instanceof MtkRetrieveConf) {
            recvReq = (MtkRetrieveConf) msgPdu;
            dataSent = recvReq.getDateSent();
        }
        if (dataSent > 0
                && msgItem.mBoxId == Mms.MESSAGE_BOX_INBOX) {
            details.append('\n');
            details.append(res.getString(R.string.sent_label));
            String dateStr = formatDateAndTimeStampString(
                    context, 0, dataSent * 1000L, true);
            details.append(dateStr);
        }
        /* Handle date sent ends */

        // Date: ***
        details.append('\n');
        // / M: Code analyze 027, new feature, to improve the performance of
        // Mms. @{
        int msgBox = msgItem.mBoxId;
        /// @}
        if (msgBox == Mms.MESSAGE_BOX_DRAFTS) {
            details.append(res.getString(R.string.saved_label));
        } else if (msgBox == Mms.MESSAGE_BOX_INBOX) {
            details.append(res.getString(R.string.received_label));
        } else {
            details.append(res.getString(R.string.sent_label));
        }
        String dateStr = formatDateAndTimeStampString(context, 0, msgPdu.getDate() * 1000L, true);

        details.append(dateStr);

        // Subject: ***
        details.append('\n');
        details.append(res.getString(R.string.subject_label));

        // / M: Code analyze 027, new feature, to improve the performance of
        // Mms. @{
        int size = msgItem.mMessageSize;
        /// @}
        EncodedStringValue subject = msgPdu.getSubject();
        if (subject != null) {
            String subStr = subject.getString();
            // Message size should include size of subject.
            // Modify ALPS00427926. Because message size has
            // include PDU header, so it should not add this size here
            //size += subStr.length();
            details.append(subStr);
        }

        // Priority: High/Normal/Low
        return formatDetails(details, context, msgPdu, size, res);
    }

    private static void initializeMsgDetails(Context context,
            StringBuilder details, Resources res, MultimediaMessagePdu msg) {
        details.append(res.getString(R.string.message_type_label));
        details.append(res.getString(R.string.multimedia_message));

        if ((MultimediaMessagePdu) msg instanceof RetrieveConf) {
            // From: ***
            String from = extractEncStr(context, ((RetrieveConf) msg).getFrom());
            details.append('\n');
            details.append(res.getString(R.string.from_label));
            details.append(!TextUtils.isEmpty(from) ? from :
                                  res.getString(R.string.hidden_sender_address));
        }

        // To: ***
        details.append('\n');
        details.append(res.getString(R.string.to_address_label));
        EncodedStringValue[] to = msg.getTo();
        if (to != null) {
            details.append(EncodedStringValue.concat(to));
        }
        else {
            Log.w(TAG, "recipient list is empty!");
        }

        // Bcc: ***
        if (msg instanceof SendReq) {
            EncodedStringValue[] values = ((SendReq) msg).getBcc();
            if ((values != null) && (values.length > 0)) {
                details.append('\n');
                details.append(res.getString(R.string.bcc_label));
                details.append(EncodedStringValue.concat(values));
            }
        }
    }

    /// @}

    // / M: Code analyze 027, new feature, to improve the performance of Mms. @{
    private static String getTextMessageDetails(Context context, MessageItem msgItem) {
        StringBuilder details = new StringBuilder();
        Resources res = context.getResources();

        // Message Type: Text message.
        details.append(res.getString(R.string.message_type_label));
        String type = IpMessageUtils.getIpMessagePlugin(context)
            .getIpUtils().getIpTextMessageType(msgItem.mIpMessageItem);
        /// M: modify for ipmessage
        if (type != null) {
            details.append(type);
        } else {
            details.append(res.getString(R.string.text_message));
        }

        // Address: ***
        details.append('\n');
        // / M: Code analyze 027, new feature, to improve the performance of
        // Mms. @{
        int smsType = msgItem.mBoxId;
        /// @}
        if (Sms.isOutgoingFolder(smsType)) {
            details.append(res.getString(R.string.to_address_label));
        } else {
            details.append(res.getString(R.string.from_label));
        }
        // / M: Code analyze 027, new feature, to improve the performance of
        // Mms. @{

        details.append(msgItem.mAddress);

        /// @}

        // / @}

        // SentDate: ***
        if (msgItem.mSmsSentDate > 0 && smsType == Sms.MESSAGE_TYPE_INBOX) {
            details.append('\n');
            details.append(res.getString(R.string.sent_label));
            String dateStr = formatDateAndTimeStampString(context, 0, msgItem.mSmsSentDate, true);
            details.append(dateStr);
        }

        // / M: Code analyze 027, new feature, to improve the performance of
        // Mms. @{
        if (msgItem.mSmsDate > 0L) {
            details.append('\n');
            if (smsType == Sms.MESSAGE_TYPE_DRAFT) {
                details.append(res.getString(R.string.saved_label));
            } else if (smsType == Sms.MESSAGE_TYPE_INBOX) {
                details.append(res.getString(R.string.received_label));
            } else {
                details.append(res.getString(R.string.sent_label));
            }
            String dateStr = formatDateAndTimeStampString(context, 0, msgItem.mSmsDate, true);
            details.append(dateStr);
        }
        /// @}

        /// M: Code analyze 028, new feature, add service center in Sms details. @{
        if (smsType == Sms.MESSAGE_TYPE_INBOX) {
            details.append('\n');
            details.append(res.getString(R.string.service_center_label));
            details.append(msgItem.mServiceCenter == null ? "" : msgItem.mServiceCenter);
        }
        /// @}

        // Delivered: ***
        if (smsType == Sms.MESSAGE_TYPE_SENT) {
            // For sent messages with delivery reports, we stick the delivery time in the
            // date_sent column (see MessageStatusReceiver).
            /// M: long dateDelivered = cursor.getLong(MessageListAdapter.COLUMN_SMS_DATE_SENT);
            // / M: Code analyze 027, new feature, to improve the performance of
            // Mms. @{
            long dateDelivered = -1L;
            if (dateDelivered > 0) {
                details.append('\n');
                details.append(res.getString(R.string.delivered_label));
                details.append(MessageUtils.formatTimeStampString(context, dateDelivered, true));
            }
        }

        // Error code: ***
        int errorCode = msgItem.mErrorCode;
        /// @}
        if (errorCode != 0) {
            details.append('\n')
                .append(res.getString(R.string.error_code_label))
                .append(errorCode);
        }

        return details.toString();
    }
    /// @}

    static private String getPriorityDescription(Context context, int PriorityValue) {
        Resources res = context.getResources();
        switch(PriorityValue) {
            case MtkPduHeaders.PRIORITY_HIGH:
                return res.getString(R.string.priority_high);
            case MtkPduHeaders.PRIORITY_LOW:
                return res.getString(R.string.priority_low);
            case MtkPduHeaders.PRIORITY_NORMAL:
            default:
                return res.getString(R.string.priority_normal);
        }
    }

    public static int getAttachmentType(SlideshowModel model, MultimediaMessagePdu mmp) {
        if (model == null || mmp == null) {
            return MessageItem.ATTACHMENT_TYPE_NOT_LOADED;
        }

        int numberOfSlides = model.size();
        if (numberOfSlides > 1) {
            return WorkingMessage.SLIDESHOW;
        } else if (numberOfSlides == 1) {
            // Only one slide in the slide-show.
            SlideModel slide = model.get(0);
            if (slide.hasVideo()) {
                return WorkingMessage.VIDEO;
            }

            if (slide.hasAudio() && slide.hasImage()) {
                return WorkingMessage.SLIDESHOW;
            }

            if (slide.hasAudio()) {
                return WorkingMessage.AUDIO;
            }

            if (slide.hasImage()) {
                return WorkingMessage.IMAGE;
            }

            /// M: Code analyze 004, For fix bug ALPS00242740, can't see the
            // select all/cut/copy icon. @{
            if (model.sizeOfFilesAttach() > 0) {
                return WorkingMessage.ATTACHMENT;
            }
            /// @}

            if (slide.hasText()) {
                return WorkingMessage.TEXT;
            }

            String subject = mmp.getSubject() != null ? mmp.getSubject().getString() : null;
            if (!TextUtils.isEmpty(subject)) {
                return WorkingMessage.TEXT;
            }
        }

        if (model.sizeOfFilesAttach() > 0) {
            return WorkingMessage.ATTACHMENT;
        }

        return MessageItem.ATTACHMENT_TYPE_NOT_LOADED;
    }

    public static String formatTimeStampString(Context context, long when) {
        return formatTimeStampString(context, when, false);
    }

    public static String formatDateAndTimeStampString(Context context, long msgDate,
            long msgDateSent, boolean fullFormat) {
        String dateAndTime = "";
        if (msgDate != 0) {
            dateAndTime = formatTimeStampString(context, msgDate, fullFormat);
        }
        if (msgDateSent != 0) {
            dateAndTime = formatTimeStampString(context, msgDateSent, fullFormat);
        }
        return sOpMessageUtilsExt.formatDateAndTimeStampString(dateAndTime, context, msgDate,
                msgDateSent, fullFormat);
    }

    public static String formatTimeStampString(Context context, long when, boolean fullFormat) {
        Time then = new Time();
        then.set(when);
        Time now = new Time();
        now.setToNow();

        // Basic settings for formatDateTime() we want for all cases.
        int format_flags = DateUtils.FORMAT_NO_NOON_MIDNIGHT |
        /// M: Fix ALPS00419488 to show 12:00, so mark DateUtils.FORMAT_ABBREV_ALL
                           //DateUtils.FORMAT_ABBREV_ALL |
                           DateUtils.FORMAT_CAP_AMPM;

        // If the message is from a different year, show the date and year.
        if (then.year != now.year) {
            format_flags |= DateUtils.FORMAT_SHOW_YEAR | DateUtils.FORMAT_SHOW_DATE;
        } else if (then.yearDay != now.yearDay) {
            // If it is from a different day than today, show only the date.
            format_flags |= DateUtils.FORMAT_SHOW_DATE;
        } else {
            // Otherwise, if the message is from today, show the time.
            format_flags |= DateUtils.FORMAT_SHOW_TIME;
        }

        // If the caller has asked for full details, make sure to show the date
        // and time no matter what we've determined above (but still make showing
        // the year only happen if it is a different year from today).
        if (fullFormat) {
            format_flags |= (DateUtils.FORMAT_SHOW_DATE | DateUtils.FORMAT_SHOW_TIME);
        }

        String dateTime = sOpMessageUtilsExt.formatTimeStampString(context, when, format_flags);
        if (dateTime != null) {
            return dateTime;
        }
        return DateUtils.formatDateTime(context, when, format_flags);
    }

    public static void selectAudio(Activity activity, int requestCode) {
        // / M: Code analyze 027, new feature, to improve the performance of
        // Mms. @{
        Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
        intent.addCategory(Intent.CATEGORY_DEFAULT);
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        intent.setType(MmsContentType.AUDIO_UNSPECIFIED);
        String[] mimeTypess = new String[] {MmsContentType.AUDIO_UNSPECIFIED,
                MmsContentType.AUDIO_OGG, "application/x-ogg"};
        intent.putExtra(Intent.EXTRA_MIME_TYPES, mimeTypess);
        if (FeatureOption.MTK_DRM_APP) {
            intent.putExtra(OmaDrmStore.DrmIntentExtra.EXTRA_DRM_LEVEL,
                    OmaDrmStore.DrmIntentExtra.LEVEL_SD);
        }
        /// @}
        activity.startActivityForResult(intent, requestCode);
    }

    public static void recordSound(Activity activity, int requestCode, long sizeLimit) {
            Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
            intent.setType(MmsContentType.AUDIO_AMR);
            intent.setClassName("com.android.soundrecorder",
                    "com.android.soundrecorder.SoundRecorder");
            intent.putExtra(android.provider.MediaStore.Audio.Media.EXTRA_MAX_BYTES, sizeLimit);
        activity.startActivityForResult(intent, requestCode);
    }

    public static void recordVideo(Activity activity, int requestCode, long sizeLimit) {
        // The video recorder can sometimes return a file that's larger than the max we
        // say we can handle. Try to handle that overshoot by specifying an 85% limit.
        /// M: media recoder can handle this issue,so mark it.
//        sizeLimit *= .85F;

            int durationLimit = getVideoCaptureDurationLimit();

        if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
            log("recordVideo: durationLimit: " + durationLimit +
                    " sizeLimit: " + sizeLimit);
        }

            Intent intent = new Intent(MediaStore.ACTION_VIDEO_CAPTURE);
            intent.putExtra(MediaStore.EXTRA_VIDEO_QUALITY, 0);
            intent.putExtra("android.intent.extra.sizeLimit", sizeLimit);
            intent.putExtra("android.intent.extra.durationLimit", durationLimit);
            /// M: Code analyze 009, For fix bug ALPS00241707, You can not add
            // capture video to Messaging after you preview it in Gallery. @{
            intent.putExtra(MediaStore.EXTRA_OUTPUT, TempFileProvider.SCRAP_VIDEO_URI);
            /// M: fix bug ALPS01043585
            intent.putExtra("CanShare", false);
            activity.startActivityForResult(intent, requestCode);
    }

    public static void capturePicture(Activity activity, int requestCode) {
        Intent intent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
        intent.putExtra(MediaStore.EXTRA_OUTPUT, TempFileProvider.SCRAP_CONTENT_URI);
        activity.startActivityForResult(intent, requestCode);
    }

    public static int getVideoCaptureDurationLimit() {
        CamcorderProfile camcorder = CamcorderProfile.get(CamcorderProfile.QUALITY_LOW);
        return camcorder == null ? 0 : camcorder.duration;
    }

    public static void selectVideo(Context context, int requestCode) {
        selectMediaByType(context, requestCode, MmsContentType.VIDEO_UNSPECIFIED, true);
    }

    public static void selectImage(Context context, int requestCode) {
        selectMediaByType(context, requestCode, MmsContentType.IMAGE_UNSPECIFIED, false);
    }

    private static void selectMediaByType(
            Context context, int requestCode, String contentType, boolean localFilesOnly) {
         if (context instanceof Activity) {

            Intent innerIntent = new Intent(Intent.ACTION_GET_CONTENT);

            innerIntent.setType(contentType);
            /// M: @{
            if (FeatureOption.MTK_DRM_APP) {
                innerIntent.putExtra(OmaDrmStore.DrmIntentExtra.EXTRA_DRM_LEVEL,
                        OmaDrmStore.DrmIntentExtra.LEVEL_SD);
            }
            /// @}
            if (localFilesOnly) {
                innerIntent.putExtra(Intent.EXTRA_LOCAL_ONLY, true);
            }

            Intent wrapperIntent = Intent.createChooser(innerIntent, null);
            ((Activity) context).startActivityForResult(wrapperIntent, requestCode);
        }
    }

    /*
     * Other APPS cannot access MMS file because of security in N version. Files should be
     * put in FileProvider then pass the Uri to other APPS to read.
     */
    public static File createTempFileExposed(Context context, Uri fromUri, String fileName) {
        MmsLog.ipi(TAG, "createTempFileExposed, file name: " + fileName + " Uri: " + fromUri);
        File file = null;
        try {
            File baseDir = context.getFilesDir();
            File folder = new File(baseDir.toString() + "/" + MMS_SHARED_FILES_FOLDER_NAME);
            if (!folder.exists()) {
                folder.mkdirs();
                folder.setExecutable(true, false);
            }
            file = new File(folder.toString() + "/" + TEMP_FILE_TAG + fileName);
            InputStream in = null;
            FileOutputStream out = null;
            try {
                in = context.getContentResolver().openInputStream(fromUri);
                out = new FileOutputStream(file);
                byte[] buf = new byte[8096];
                int seg = 0;
                while ((seg = in.read(buf)) != -1) {
                    out.write(buf, 0, seg);
                }
            } finally {
                if (in != null) {
                    in.close();
                }
                if (out != null) {
                    out.close();
                }
            }
        } catch (FileNotFoundException e) {
            MmsLog.epi(TAG, "createTempFileExposed, file not found " + fromUri + ", exception ", e);
        } catch (IOException e) {
            MmsLog.epi(TAG, "createTempFileExposed, ioexception " + fromUri + ", exception ", e);
        }
        return file;
    }

    /*
     * Delete all shared files
     */
    public static void deleteAllSharedFiles(Context context, String tag) {
        String fileDir = context.getFilesDir().toString() + "/"
                + MMS_SHARED_FILES_FOLDER_NAME;
        File[] allFiles = new File(fileDir).listFiles();
        if (allFiles == null) {
            return;
        }

        for (int  i = 0; i < allFiles.length; i++) {
            File file = allFiles[i];
            if (file.getName().contains(tag)) {
                MmsLog.dpi(TAG, "getPreviewFileUri, delete old preview file: " + file.getPath());
                file.delete();
            }
        }
    }

    public static Uri getPreviewFileUri(Context context, MediaModel model) {
        deleteAllSharedFiles(context, TEMP_FILE_TAG);
        final File tempPreviewFile =
            createTempFileExposed(context, model.getUri(), model.getSrc());
        if (tempPreviewFile == null
                || !tempPreviewFile.exists()
                || tempPreviewFile.length() <= 0) {
            MmsLog.epi(TAG, "getPreviewFileUri, file is not exists or empty " + tempPreviewFile);
            return null;
        }
        return FileProvider.getUriForFile(
                context, MMS_SHARED_FILE_PROVIDER_AUTHORITIES, tempPreviewFile);
    }

    public static void viewSimpleSlideshow(Context context, SlideshowModel slideshow) {
        if (!slideshow.isSimple()) {
            throw new IllegalArgumentException(
                    "viewSimpleSlideshow() called on a non-simple slideshow");
        }
        SlideModel slide = slideshow.get(0);
        MediaModel mm = null;
        if (slide.hasImage()) {
            mm = slide.getImage();
        } else if (slide.hasVideo()) {
            mm = slide.getVideo();
        } else if (slide.hasAudio()) {
            mm = slide.getAudio();
        }

        Intent intent = new Intent(Intent.ACTION_VIEW);
        intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
        intent.addFlags(Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
        intent.putExtra("SingleItemOnly", true); // So we don't see "surrounding" images in Gallery
        /// M: CanShare: false, Hide the videopalye's share option menu.
        /// M: CanShare: ture, Show the share option menu.
        /// M: Code analyze 010, For fix bug ALPS00244046, The "JE" pops up
        // after you tap the "messaging" icon. @{
        intent.putExtra("CanShare", false);
        /// @}
        /// M: for showing notification when view mms with video player in full screen model @{
        intent.putExtra(EXTRA_FULLSCREEN_NOTIFICATION, true);
        /// @}

        String contentType = "";
        if (mm != null) {
            contentType = mm.getContentType();
            MmsLog.epi(TAG, "viewSimpleSildeshow. Uri:" + mm.getUri());
            MmsLog.e(TAG, "viewSimpleSildeshow. contentType:" + contentType);
            intent.setDataAndType(getPreviewFileUri(context, mm), contentType);
        }
        /// M: Code analyze 013, For fix bug ALPS00250939, Exception/Java(JE)-->com.android.mms.
        try {
            // M: change feature ALPS01751464
            if (mm != null && mm.hasDrmContent()) {
                DrmUtilsEx.showDrmAlertDialog(context);
                return;
            }

            context.startActivity(intent);
        } catch (ActivityNotFoundException e) {
            Message msg = Message.obtain(MmsApp.getToastHandler());
            msg.what = MmsApp.MSG_MMS_CAN_NOT_OPEN;
            msg.obj = contentType;
            msg.sendToTarget();
            /// M: after user click view, and the error toast is shown,
            /// we must make it can press again. tricky code
            if (context instanceof ComposeMessageActivity) {
                ((ComposeMessageActivity) context).mClickCanResponse = true;
            }
        }
        /// @}
    }

    /// M: Code analyze 025, For fix bug ALPS00298363, The "JE" pops up
    // after you launch message again. @{
    public static void showErrorDialog(Activity activity,
            int titleId, int messageId, int mediaTypeIdForTitle, int mediaTypeIdForMsg) {
        /** M:
            the original code is replaced by the following code.
            the original code has a bug, JE may happen.
            the case is when the activity is destoried by some reason(ex:change language),
            when dismiss the dialog, it may be throw a JE. see 298363.
        */
        ErrorDialog errDialog = ErrorDialog.newInstance(titleId, messageId,
                mediaTypeIdForTitle, mediaTypeIdForMsg);
        try {
            errDialog.show(activity.getFragmentManager(), "errDialog");
        } catch (IllegalStateException e) {
            try {
                MmsLog.d(TAG, "showErrorDialog catch IllegalStateException." + e);
                FragmentTransaction transaction = activity.getFragmentManager().beginTransaction();
                transaction.add(errDialog, "errDialog");
                transaction.commitAllowingStateLoss();
            } catch (Exception e2) {
                MmsLog.e(TAG, "showErrorDialog commitAllowingStateLoss catch Exception." + e2);
            }
            return;
        }
    }
    /// @}

    /**
     * The quality parameter which is used to compress JPEG images.
     */
    public static final int IMAGE_COMPRESSION_QUALITY = 95;
    /**
     * The minimum quality parameter which is used to compress JPEG images.
     */
    /// M: Modify for ALPS00778930
    public static final int MINIMUM_IMAGE_COMPRESSION_QUALITY = 10;

    public static final int MAX_COMPRESS_TIMES = 8;

    /**
     * Message overhead that reduces the maximum image byte size.
     * 5000 is a realistic overhead number that allows for user to also include
     * a small MIDI file or a couple pages of text along with the picture.
     */
    public static final int MESSAGE_OVERHEAD = 5000;

    public static void resizeImageAsync(final Context context,
            final Uri imageUri, final Handler handler,
            final ResizeImageResultCallback cb,
            final boolean append) {

        // Show a progress toast if the resize hasn't finished
        // within one second.
        // Stash the runnable for showing it away so we can cancel
        // it later if the resize completes ahead of the deadline.
        /* q0 migration
        final Runnable showProgress = new Runnable() {
            @Override
            public void run() {
                Toast.makeText(context, R.string.compressing, Toast.LENGTH_SHORT).show();
            }
        };
        // Schedule it for one second from now.
        handler.postDelayed(showProgress, 1000);
*/
        new Thread(new Runnable() {
            @Override
            public void run() {
                final PduPart part;
                try {
                    UriImage image = new UriImage(context, imageUri);
                    int widthLimit = MmsConfig.getMaxImageWidth();
                    int heightLimit = MmsConfig.getMaxImageHeight();
                    // In mms_config.xml, the max width always been declared larger than the max
                    // height. Swap the width and height limits if necessary so scale the picture
                    // as little as possible.
                    if (image.getHeight() > image.getWidth()) {
                        int temp = widthLimit;
                        widthLimit = heightLimit;
                        heightLimit = temp;
                    }

                    // / M: Code analyze 027, new feature, to improve the
                    // performance of Mms. @{
                    /*part = image.getResizedImageAsPart(
                        widthLimit,
                        heightLimit,
                        MmsConfig.getMaxMessageSize() - MESSAGE_OVERHEAD);
                    */
                    part = image.getResizedImageAsPart(
                            widthLimit,
                            heightLimit,
                            MmsConfig.getUserSetMmsSizeLimit(true) - MESSAGE_OVERHEAD);
                    handler.post(new Runnable() {
                        @Override
                        public void run() {
                            cb.onResizeResult(part, append);
                        }
                    });
                } catch (IllegalArgumentException e) {
                    MmsLog.e(TAG, "Unexpected IllegalArgumentException.", e);
                } finally {
                    /// M: Cancel pending show of the progress toast if necessary.
                    // q0 migration handler.removeCallbacks(showProgress);
                }
                /// @}
            }
        }, "MessageUtils.resizeImageAsync").start();
    }

    /// M: Code analyze 003, For fix bug ALPS00113244, It adds a blank slide
    // when share multi folders from Gallery. @{
    public static void resizeImage(final Context context,
            final Uri imageUri, final Handler handler,
            final ResizeImageResultCallback cb, final boolean append, boolean showToast) {

        /** M: Show a progress toast if the resize hasn't finished
         * within one second.
         * Stash the runnable for showing it away so we can cancel
         * it later if the resize completes ahead of the deadline.
        */
        /* q0 migration
        final Runnable showProgress = new Runnable() {
            public void run() {
                Toast.makeText(context, R.string.compressing, Toast.LENGTH_SHORT).show();
            }
        };
        if (showToast) {
            handler.post(showProgress);
//          handler.postDelayed(showProgress, 1000);
        }
        */
        final PduPart part;
        try {
            UriImage image = new UriImage(context, imageUri);
            part = image.getResizedImageAsPart(MmsConfig.getMaxImageWidth(),
                    MmsConfig.getMaxImageHeight(), MmsConfig.getUserSetMmsSizeLimit(true)
                    - MESSAGE_OVERHEAD);
            cb.onResizeResult(part, append);
        } catch (IllegalArgumentException e) {
            MmsLog.e(TAG, "Unexpected IllegalArgumentException.", e);
        } finally {
            /// M: Cancel pending show of the progress toast if necessary.
            // q0 migration handler.removeCallbacks(showProgress);
        }
    }

    /// @}
    public static AlertDialog showDiscardDraftConfirmDialog(Context context,
            OnClickListener listener) {
        return new AlertDialog.Builder(context)
                /// M: Code analyze 008, new feature, Android4.1 has moved this
                // icon and title. @{
                .setIconAttribute(android.R.attr.alertDialogIcon)
                .setTitle(R.string.discard_message)
                /// @}
                .setMessage(R.string.discard_message_reason)
                .setPositiveButton(R.string.yes, listener)
                .setNegativeButton(R.string.no, null)
                .show();
    }

    /// M: we[mtk] do not support reply read report when deleteing without read.
    public static void handleReadReport(final Context context,
            final Collection<Long> threadIds,
            final int status,
            final Runnable callback) {
//        StringBuilder selectionBuilder = new StringBuilder(Mms.MESSAGE_TYPE + " = "
//                + MtkPduHeaders.MESSAGE_TYPE_RETRIEVE_CONF
//                + " AND " + Mms.READ + " = 0"
//                + " AND " + Mms.READ_REPORT + " = " + MtkPduHeaders.VALUE_YES);
//
//        String[] selectionArgs = null;
//        if (threadIds != null) {
//            String threadIdSelection = null;
//            StringBuilder buf = new StringBuilder();
//            selectionArgs = new String[threadIds.size()];
//            int i = 0;
//
//            for (long threadId : threadIds) {
//                if (i > 0) {
//                    buf.append(" OR ");
//                }
//                buf.append(Mms.THREAD_ID).append("=?");
//                selectionArgs[i++] = Long.toString(threadId);
//            }
//            threadIdSelection = buf.toString();
//
//            selectionBuilder.append(" AND (" + threadIdSelection + ")");
//        }
//
//        final Cursor c = SqliteWrapper.query(context, context.getContentResolver(),
//                        Mms.Inbox.CONTENT_URI, new String[] {Mms._ID, Mms.MESSAGE_ID},
//                        selectionBuilder.toString(), selectionArgs, null);
//
//        if (c == null) {
//            return;
//        }
//
//        final Map<String, String> map = new HashMap<String, String>();
//        try {
//            if (c.getCount() == 0) {
//                if (callback != null) {
//                    callback.run();
//                }
//                return;
//            }
//
//            while (c.moveToNext()) {
//                Uri uri = ContentUris.withAppendedId(Mms.CONTENT_URI, c.getLong(0));
//                map.put(c.getString(1), AddressUtils.getFrom(context, uri));
//            }
//        } finally {
//            c.close();
//        }
        // we[mtk] do not support reply read report when deleteing without read.
        // the underlayer code[ReadRecTransaction.java] is modified to support another branch.
        if (callback != null) {
            callback.run();
        }
        /// M: Code analyze 016, For fix bug ALPS00274375, remove the entry of
        // sending read report when deleting mms without read. @{
        /** M:
        OnClickListener positiveListener = new OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                for (final Map.Entry<String, String> entry : map.entrySet()) {
                    MmsMessageSender.sendReadRec(context, entry.getValue(),
                                                 entry.getKey(), status);
                }

                if (callback != null) {
                    callback.run();
                }
                dialog.dismiss();
            }
        };

        OnClickListener negativeListener = new OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                if (callback != null) {
                    callback.run();
                }
                dialog.dismiss();
            }
        };

        OnCancelListener cancelListener = new OnCancelListener() {
            @Override
            public void onCancel(DialogInterface dialog) {
                if (callback != null) {
                    callback.run();
                }
                dialog.dismiss();
            }
        };

        confirmReadReportDialog(context, positiveListener,
                                         negativeListener,
                                         cancelListener);
        */
        /// @}
    }

    private static void confirmReadReportDialog(Context context,
            OnClickListener positiveListener, OnClickListener negativeListener,
            OnCancelListener cancelListener) {
        AlertDialog.Builder builder = new AlertDialog.Builder(context);
        builder.setCancelable(true);
        builder.setTitle(R.string.confirm);
        builder.setMessage(R.string.message_send_read_report);
        builder.setPositiveButton(R.string.yes, positiveListener);
        builder.setNegativeButton(R.string.no, negativeListener);
        builder.setOnCancelListener(cancelListener);
        builder.show();
    }

    public static String extractEncStrFromCursor(Cursor cursor,
            int columnRawBytes, int columnCharset) {
        String rawBytes = cursor.getString(columnRawBytes);
        int charset = cursor.getInt(columnCharset);

        if (TextUtils.isEmpty(rawBytes)) {
            return "";
        } else if (charset == CharacterSets.ANY_CHARSET) {
            return rawBytes;
        } else {
            return new EncodedStringValue(charset, MtkPduPersister.getBytes(rawBytes)).getString();
        }
    }

    private static String extractEncStr(Context context, EncodedStringValue value) {
        if (value != null) {
            return value.getString();
        } else {
            Log.d(TAG, "extractEncStr EncodedStringValue is null");
            return "";
        }
    }

    public static ArrayList<String> extractUris(URLSpan[] spans) {
        int size = spans.length;
        ArrayList<String> accumulator = new ArrayList<String>();

        for (int i = 0; i < size; i++) {
            // / M: Code analyze 027, new feature, to improve the performance of
            // Mms. @{
            if (!accumulator.contains(spans[i].getURL())) {
                accumulator.add(spans[i].getURL());
            }
            /// @
        }
        return accumulator;
    }

    /**
     * Play/view the message attachments.
     * TOOD: We need to save the draft before launching another activity to view the attachments.
     *       This is hacky though since we will do saveDraft twice and slow down the UI.
     *       We should pass the slideshow in intent extra to the view activity instead of
     *       asking it to read attachments from database.
     * @param activity
     * @param msgUri the MMS message URI in database
     * @param slideshow the slideshow to save
     * @param persister the PDU persister for updating the database
     * @param sendReq the MtkSendReq for updating the database
     */
    public static void viewMmsMessageAttachment(Activity activity, Uri msgUri,
            SlideshowModel slideshow, AsyncDialog asyncDialog) {
        viewMmsMessageAttachment(activity, msgUri, slideshow, 0, asyncDialog);
    }


    public static void viewMmsMessageAttachment(final Activity activity, final Uri msgUri,
            final SlideshowModel slideshow, final int requestCode, AsyncDialog asyncDialog) {
        /// M: Code analyze 002, For fix bug ALPS00112553, system-server JE
        // happens and MS reboot when tap play in MMS. @{
        final boolean isSimple = (slideshow == null) ? false : slideshow.isSimple();

        if (isSimple) {
            SlideModel slideTemp = slideshow.get(0);
            // In attachment-editor mode, we only ever have one slide.
            /// M: fix bug ALPS00393187, play in gellay when simple slide only has picture or video
            if (slideTemp != null && !slideTemp.hasAudio()
                    && (!slideTemp.hasText() || slideTemp.getText().getText().length() == 0)) {
                MessageUtils.viewSimpleSlideshow(activity, slideshow);
                return;
            }
        }
        /// @}
        // M: change feature ALPS01751464
        if (isSimple) {
            SlideModel slideOne = slideshow.get(0);
            if (slideOne != null && slideOne.hasAudio()) {
                MediaModel model = slideOne.getAudio();
                if (model != null && model.hasDrmContent()) {
                    DrmUtilsEx.showDrmAlertDialog(activity);
                    return;
                }
            }
        }

        // The user wants to view the slideshow. We have to persist the slideshow parts
        // in a background task. If the task takes longer than a half second, a progress dialog
        // is displayed. Once the PDU persisting is done, another runnable on the UI thread get
        // executed to start the SlideshowActivity.
        asyncDialog.runAsync(new Runnable() {
            @Override
            public void run() {
                // If a slideshow was provided, save it to disk first.
                if (slideshow != null) {
                    MtkPduPersister persister = MtkPduPersister.getPduPersister(activity);
                    try {
                        PduBody pb = slideshow.toPduBody();
                        MessageUtils.updatePartsIfNeeded(slideshow, persister, msgUri, pb, null);
                        //persister.updateParts(msgUri, pb, null);
                        slideshow.sync(pb);
                    } catch (MmsException e) {
                        Log.e(TAG, "Unable to save message for preview");
                        return;
                    }
/* Mem opt*/
                    slide = slideshow.get(0);
                }
            }
        } , new Runnable() {
            @Override
            public void run() {
             // Launch the slideshow activity to play/view.
                Intent intent;
/* Mem opt*/
                //SlideModel slide = slideshow.get(0);
                if ((isSimple && slide.hasAudio())
                        || (requestCode == AttachmentEditor.MSG_PLAY_AUDIO)) {
                    intent = new Intent(activity.getApplicationContext(), SlideshowActivity.class);
                } else {
                    intent = new Intent(activity.getApplicationContext(), MmsPlayerActivity.class);
                }
                intent.setData(msgUri);
                if (requestCode > 0) {
                    activity.startActivityForResult(intent, requestCode);
                } else {
                    activity.startActivity(intent);
                }
                slide = null;
//                    // Once the above background thread is complete, this runnable is run
//                    // on the UI thread to launch the slideshow activity.
//                    launchSlideshowActivity(activity, msgUri, requestCode);

            }
        }, R.string.building_slideshow_title);

    }

    public static void launchSlideshowActivity(Context context, Uri msgUri, int requestCode) {
        // Launch the slideshow activity to play/view.
        Intent intent = new Intent(context, SlideshowActivity.class);
        intent.setData(msgUri);
        if (requestCode > 0 && context instanceof Activity) {
            ((Activity) context).startActivityForResult(intent, requestCode);
        } else {
            context.startActivity(intent);
        }
    }

    /**
     * Debugging
     */
    public static void writeHprofDataToFile() {
        String filename = Environment.getExternalStorageDirectory() + "/mms_oom_hprof_data";
        try {
            android.os.Debug.dumpHprofData(filename);
            Log.i(TAG, "##### written hprof data to " + filename);
        } catch (IOException ex) {
            Log.e(TAG, "writeHprofDataToFile: caught " + ex);
        }
    }

    // An alias (or commonly called "nickname") is:
    // Nickname must begin with a letter.
    // Only letters a-z, numbers 0-9, or . are allowed in Nickname field.
    public static boolean isAlias(String string) {
        if (!MmsConfig.isAliasEnabled()) {
            return false;
        }

        int len = string == null ? 0 : string.length();

        if (len < MmsConfig.getAliasMinChars() || len > MmsConfig.getAliasMaxChars()) {
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

    /**
     * Given a phone number, return the string without syntactic sugar, meaning parens,
     * spaces, slashes, dots, dashes, etc. If the input string contains non-numeric
     * non-punctuation characters, return null.
     */
    private static String parsePhoneNumberForMms(String address) {
        StringBuilder builder = new StringBuilder();
        int len = address.length();

        for (int i = 0; i < len; i++) {
            char c = address.charAt(i);

            // accept the first '+' in the address
            if (c == '+' && builder.length() == 0) {
                builder.append(c);
                continue;
            }

            if (Character.isDigit(c)) {
                builder.append(c);
                continue;
            }

            if (numericSugarMap.get(c) == null) {
                return null;
            }
        }
        return builder.toString();
    }

    /**
     * Returns true if the address passed in is a valid MMS address.
     */
    public static boolean isValidMmsAddress(String address) {
        String retVal = parseMmsAddress(address);
        /// M: @{
        //return (retVal != null);
        return (retVal != null && !retVal.equals(""));
        /// @}
    }

    /**
     * parse the input address to be a valid MMS address.
     * - if the address is an email address, leave it as is.
     * - if the address can be parsed into a valid MMS phone number, return the parsed number.
     * - if the address is a compliant alias address, leave it as is.
     */
    public static String parseMmsAddress(String address) {
        // if it's a valid Email address, use that.
        if (Mms.isEmailAddress(address)) {
            return address;
        }

        // if we are able to parse the address to a MMS compliant phone number, take that.
        String retVal = parsePhoneNumberForMms(address);
        if (retVal != null) {
            return retVal;
        }

        // if it's an alias compliant address, use that.
        if (isAlias(address)) {
            return address;
        }

        // it's not a valid MMS address, return null
        return null;
    }

    private static void log(String msg) {
        Log.d(TAG, "[MsgUtils] " + msg);
    }

    /// M: @{
    public static Uri saveBitmapAsPart(Context context, Uri messageUri, Bitmap bitmap)
        throws MmsException {

        ByteArrayOutputStream os = new ByteArrayOutputStream();
        bitmap.compress(CompressFormat.JPEG, IMAGE_COMPRESSION_QUALITY, os);

        PduPart part = new PduPart();

        part.setContentType("image/jpeg".getBytes());
        String contentId = "Image" + System.currentTimeMillis();
        part.setContentLocation((contentId + ".jpg").getBytes());
        part.setContentId(contentId.getBytes());
        part.setData(os.toByteArray());

        Uri retVal = MtkPduPersister.getPduPersister(context).persistPart(part,
                            ContentUris.parseId(messageUri), null);

        if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
            log("saveBitmapAsPart: persisted part with uri=" + retVal);
        }

        return retVal;
    }

    public static String getLocalNumber() {
        return MmsApp.getApplication().getTelephonyManager().getLine1Number();
    }

    public static String getLocalNumber(int subId) {
        return MmsApp.getApplication().getTelephonyManager().getLine1Number(subId);
    }

    public static boolean isLocalNumber(String number) {
        if (number == null) {
            return false;
        }

        // we don't use Mms.isEmailAddress() because it is too strict for comparing addresses like
        // "foo+caf_=6505551212=tmomail.net@gmail.com",
        // which is the 'from' address from a forwarded email
        // message from Gmail. We don't want to treat "foo+caf
        // =6505551212=tmomail.net@gmail.com" and
        // "6505551212" to be the same.
        if (number.indexOf('@') >= 0) {
            return false;
        }

        List<SubscriptionInfo> subInfoList;
        subInfoList = SubscriptionManager.from(MmsApp.getApplication())
                .getActiveSubscriptionInfoList();
        if (subInfoList == null || subInfoList.size() == 0) {
            MmsLog.d(TAG, "isLocalNumber SIM not insert");
            return false;
        }
        for (SubscriptionInfo subInfoRecord : subInfoList) {
            if (PhoneNumberUtils.compare(number,
                    getLocalNumber(subInfoRecord.getSubscriptionId()))) {
                    return true;
                }
            }
         return false;
    }

    public static void viewMmsMessageAttachmentMini(Context context, Uri msgUri,
            SlideshowModel slideshow) {
        if (msgUri == null) {
            return;
        }
        boolean isSimple = (slideshow == null) ? false : slideshow.isSimple();
        if (slideshow != null) {
            /// M: If a slideshow was provided, save it to disk first.
            MtkPduPersister persister = MtkPduPersister.getPduPersister(context);
            try {
                PduBody pb = slideshow.toPduBody();
                MessageUtils.updatePartsIfNeeded(slideshow, persister, msgUri, pb, null);
                //persister.updateParts(msgUri, pb, null);
                slideshow.sync(pb);
            } catch (MmsException e) {
                MmsLog.e(TAG, "Unable to save message for preview");
                return;
            }
        }

        /// M: Launch the slideshow activity to play/view.
        Intent intent;
        if (isSimple && (slideshow != null) && slideshow.get(0).hasAudio()) {
            intent = new Intent(context, SlideshowActivity.class);
        } else {
            intent = new Intent(context, MmsPlayerActivity.class);
        }
        intent.setData(msgUri);
        context.startActivity(intent);
    }

    /// M:wappush: add this function to handle the url string,
    /// if it does not contain the http or https schema, then add http schema manually.
    public static String checkAndModifyUrl(String url) {
        if (url == null) {
            return null;
        }

        Uri uri = Uri.parse(url);
        if (uri.getScheme() != null) {
            return url;
        }

        return "http://" + url;
    }

    public static void selectRingtone(Context context, int requestCode) {
        if (context instanceof Activity) {
            Intent intent = new Intent(RingtoneManager.ACTION_RINGTONE_PICKER);
            intent.putExtra(RingtoneManager.EXTRA_RINGTONE_SHOW_DEFAULT, false);
            intent.putExtra(RingtoneManager.EXTRA_RINGTONE_SHOW_SILENT, false);
            intent.putExtra(RingtoneManager.EXTRA_RINGTONE_INCLUDE_DRM, false);
            intent.putExtra(RingtoneManager.EXTRA_RINGTONE_TITLE,
                    context.getString(R.string.select_audio));
            intent.putExtra(RingtoneManager.EXTRA_RINGTONE_TYPE, RingtoneManager.TYPE_RINGTONE);
            intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
            if (FeatureOption.MTK_DRM_APP) {
                intent.putExtra(OmaDrmStore.DrmIntentExtra.EXTRA_DRM_LEVEL,
                        OmaDrmStore.DrmIntentExtra.LEVEL_SD);
            }
            ((Activity) context).startActivityForResult(intent, requestCode);
        }
    }

    public static void setSubIconAndLabel(int subId, String subName, TextView subView) {
        // Log.i(TAG, "setSubIconAndLabel subId=" + subId);
        int textColor = 0;
        if (subView == null) {
            return;
        }
        int activeSubCount = SubscriptionManager.from(MmsApp.getApplication())
                .getActiveSubscriptionInfoCount();

        if (subName == null && activeSubCount > 1) {
            SubscriptionInfo subInfo = SubscriptionManager.from(MmsApp.getApplication())
                    .getActiveSubscriptionInfo(subId);
            MmsLog.dpi(TAG, "subInfo=" + subInfo);
            if (null != subInfo) {
                if ((subInfo.getSimSlotIndex() == SubscriptionManager.SIM_NOT_INSERTED)
                        || (subInfo.getSimSlotIndex()
                                == SubscriptionManager.INVALID_SUBSCRIPTION_ID)) {
                    Log.i(TAG, "current not insert sim card");
                } else {
                    subName = subInfo.getDisplayName().toString();
                    textColor = subInfo.getIconTint();
                }
            } else {
                Log.i(TAG, "subInfo is null ");
            }
        }

        if (subName == null || activeSubCount <= 1) {
            subView.setVisibility(View.GONE);
        } else {
            subView.setVisibility(View.VISIBLE);
            subView.setTextColor(textColor);
            subView.setText(subName);
        }
    }

    public static void addNumberOrEmailtoContact(final String numberOrEmail,
            final int REQUEST_CODE,
            final Activity activity) {
        if (!TextUtils.isEmpty(numberOrEmail)) {
            String message = activity.getResources().getString(
                    R.string.add_contact_dialog_message, numberOrEmail);
            AlertDialog.Builder builder =
                new AlertDialog.Builder(activity).setTitle(numberOrEmail).setMessage(message);
            AlertDialog dialog = builder.create();
            dialog.setButton(AlertDialog.BUTTON_POSITIVE, activity.getResources().getString(
                    R.string.add_contact_dialog_existing), new DialogInterface.OnClickListener() {

                public void onClick(DialogInterface dialog, int which) {
                    // TODO Auto-generated method stub
                    Intent intent = new Intent(Intent.ACTION_INSERT_OR_EDIT);
                    intent.setType(Contacts.CONTENT_ITEM_TYPE);
                    if (Mms.isEmailAddress(numberOrEmail)) {
                        intent.putExtra(ContactsContract.Intents.Insert.EMAIL, numberOrEmail);
                    } else {
                        intent.putExtra(ContactsContract.Intents.Insert.PHONE, numberOrEmail);
                    }
                    if (REQUEST_CODE > 0) {
                        activity.startActivityForResult(intent, REQUEST_CODE);
                    } else {
                        activity.startActivity(intent);
                    }
                }
            });

            dialog.setButton(AlertDialog.BUTTON_NEGATIVE, activity.getResources()
                    .getString(R.string.add_contact_dialog_new),
                    new DialogInterface.OnClickListener() {

                public void onClick(DialogInterface dialog, int which) {
                    // TODO Auto-generated method stub
                    final Intent intent = new Intent(Intent.ACTION_INSERT, Contacts.CONTENT_URI);
                    if (Mms.isEmailAddress(numberOrEmail)) {
                        intent.putExtra(ContactsContract.Intents.Insert.EMAIL, numberOrEmail);
                    } else {
                        intent.putExtra(ContactsContract.Intents.Insert.PHONE, numberOrEmail);
                    }
                    if (REQUEST_CODE > 0) {
                        activity.startActivityForResult(intent, REQUEST_CODE);
                    } else {
                        activity.startActivity(intent);
                    }
                }
            });
            dialog.show();
        }
    }

    /** M:
     * Return the current storage status.
     */
    public static String getStorageStatus(Context context) {
        /// M: we need count only
        final String[] PROJECTION = new String[] {
            BaseColumns._ID, Mms.MESSAGE_SIZE
        };
        final ContentResolver cr = context.getContentResolver();
        final Resources res = context.getResources();
        Cursor cursor = null;

        StringBuilder buffer = new StringBuilder();
        // Mms count
        cursor = cr.query(Mms.CONTENT_URI, PROJECTION, null, null, null);
        int mmsCount = 0;
        if (cursor != null) {
            mmsCount = cursor.getCount();
        }
        buffer.append(res.getString(R.string.storage_dialog_mms, mmsCount));
        buffer.append("\n");
        //Mms size
        long size = 0;
        if (cursor != null) {
            if (cursor.moveToFirst()) {
                do {
                    size += cursor.getInt(1);
                } while (cursor.moveToNext());
            }
            cursor.close();
        }
        buffer.append(res.getString(R.string.storage_dialog_mms_size)
                + getHumanReadableSize(size));
        buffer.append("\n");
        // Attachment size
        size = getAttachmentSize(context);
        Log.d(TAG, "mms attachment size = " + size);
        final String sizeTag = getHumanReadableSize(size);
        buffer.append(res.getString(R.string.storage_dialog_attachments) + sizeTag);
        buffer.append("\n");
        // Sms count
        cursor = cr.query(Sms.CONTENT_URI, PROJECTION, null, null, null);
        int smsCount = 0;
        if (cursor != null) {
            smsCount = cursor.getCount();
            cursor.close();
        }
        buffer.append(res.getString(R.string.storage_dialog_sms, smsCount));
        buffer.append("\n");
        // Database size
        final long dbsize = getDatabaseSize(context);
        buffer.append(res.getString(R.string.storage_dialog_database)
                + getHumanReadableSize(dbsize));
        buffer.append("\n");
        // Available space
        final StatFs datafs = new StatFs(Environment.getDataDirectory().getAbsolutePath());
        final long availableSpace = (long) datafs.getAvailableBlocks() * datafs.getBlockSize();
        buffer.append(res.getString(R.string.storage_dialog_available_space)
                + getHumanReadableSize(availableSpace));
        return buffer.toString();
    }

    /// M: Code analyze 017, For fix bug ALPS00275452, the size of Mms
    // attachment is 0B in the settings. @{
    private static long getAttachmentSize(Context context) {
        Uri uri = Uri.parse("content://mms/attachment_size");
        ContentValues insertValues = new ContentValues();
        uri = context.getContentResolver().insert(uri, insertValues);
        String size = uri.getQueryParameter("size");
        return Long.parseLong(size);
    }
    /// @}

    /// M: Get database size, for SELinux enhance, mms can not get the size directly,
    //  should get the size through phone process @{
    private static long getDatabaseSize(Context context) {
        Uri uri = Uri.parse("content://mms-sms/database_size");
        ContentValues insertValues = new ContentValues();
        uri = context.getContentResolver().insert(uri, insertValues);
        String size = uri.getQueryParameter("size");
        return Long.parseLong(size);
    }
    /// @}

    /// M: Code analyze 004, For fix bug ALPS00231349, add new feature vCard
    // support. @{
    public static String getHumanReadableSize(long size) {
        /// @}
        String tag;
        float fsize = (float) size;
        if (size < 1024L) {
            tag = String.valueOf(size) + "B";
        } else if (size < 1024L * 1024L) {
            fsize /= 1024.0f;
            tag = String.format(Locale.ENGLISH, "%.2f", fsize) + "KB";
        } else {
            fsize /= 1024.0f * 1024.0f;
            tag = String.format(Locale.ENGLISH, "%.2f", fsize) + "MB";
        }
        return tag;
    }

    /// M: Code analyze 001, For fix bug ALPS00101270, The address is displayed
    // as contact in the unsent message view,but the add contact icon still
    // displayed in the view. @{
    public static boolean canAddToContacts(Contact contact) {
        // There are some kind of automated messages, like STK messages, that we don't want
        // to add to contacts. These names begin with special characters, like, "*Info".
        final String name = contact.getName();
        if (!TextUtils.isEmpty(contact.getNumber())) {
            char c = contact.getNumber().charAt(0);
            if (isSpecialChar(c)) {
                return false;
            }
        }
        if (!TextUtils.isEmpty(name)) {
            char c = name.charAt(0);
            if (isSpecialChar(c)) {
                return false;
            }
        }
        if (!(Mms.isEmailAddress(name) || (Mms.isPhoneNumber(name) || isPhoneNumber(name)) ||
                MessageUtils.isLocalNumber(contact.getNumber()))) {     // Handle "Me"
            return false;
        }
        return true;
    }

    private static boolean isPhoneNumber(String num) {
        num = num.trim();
        if (TextUtils.isEmpty(num)) {
            return false;
        }
        final char[] digits = num.toCharArray();
        for (char c : digits) {
            if (!Character.isDigit(c)) {
                return false;
            }
        }
        return true;
    }

    private static boolean isSpecialChar(char c) {
        return c == '*' || c == '%' || c == '$';
    }
    /// @}

    /// M: Code analyze 005, For fix bug ALPS00120575, add for cmcc dir ui. @{
    public static void replyMessage(int subId, Context context, String address) {
        Intent intent = new Intent();
        intent.putExtra("address", address);
        intent.putExtra("showinput", true);
        /// M: fix alps01858266. this put sub id here. @{
        intent.putExtra(PhoneConstants.SUBSCRIPTION_KEY, subId);
        /// @}
        intent.setClassName(context, "com.android.mms.ui.ComposeMessageActivity");
        context.startActivity(intent);
    }

    public static void confirmDeleteMessage(final Activity activity, final Uri msgUri) {
        AlertDialog.Builder builder = new AlertDialog.Builder(activity);
        builder.setTitle(R.string.confirm_dialog_title);
        builder.setIconAttribute(android.R.attr.alertDialogIcon);
        builder.setCancelable(true);
        builder.setMessage(R.string.confirm_delete_message);
        builder.setPositiveButton(R.string.delete,
                                new DialogInterface.OnClickListener() {
                                    public void onClick(DialogInterface dialog, int which) {
                                        /// M: fix bug ALPS00351620; for requery searchactivity.
                                        SearchActivity.setNeedRequery();
                                        SqliteWrapper.delete(activity.getApplicationContext(),
                                                                activity.getContentResolver(),
                                                                msgUri, null, null);
                                        dialog.dismiss();
                                        activity.finish();
                                    }
        });
        builder.setNegativeButton(R.string.no, null);
        builder.show();
    }

    public static String getMmsDetail(Context context, Uri uri, int size, int msgBox) {
        Resources res = context.getResources();
        /* Handle date sent if exists */
        MultimediaMessagePdu msgPdu;
        MtkMultimediaMessagePdu msg;
        try {
            msgPdu = (MultimediaMessagePdu) MtkPduPersister.getPduPersister(
                    context).load(uri);
        } catch (MmsException e) {
            Log.e(TAG, "Failed to load the message: " + uri, e);
            return res.getString(R.string.cannot_get_details);
        }

        StringBuilder details = new StringBuilder();
        // Message Type: Text message.
        initializeMsgDetails(context, details, res, msgPdu);

        // SentDate: ***
        /*[MTK MMS FW] Start*/
        MtkSendReq sendReq;
        MtkRetrieveConf recvReq;
        long dataSent = 0L;

        if (msgPdu instanceof MtkSendReq) {
            sendReq = (MtkSendReq) msgPdu;
            dataSent = sendReq.getDateSent();
        } else if (msgPdu instanceof MtkRetrieveConf) {
            recvReq = (MtkRetrieveConf) msgPdu;
            dataSent = recvReq.getDateSent();
        }

        if (dataSent > 0
                && msgBox == Mms.MESSAGE_BOX_INBOX) {
            details.append('\n');
            details.append(res.getString(R.string.sent_label));
            details.append(MessageUtils.formatTimeStampString(context,
                    dataSent * 1000L, true));
        }
        /*  Handle date sent if exists ends */

        // Date: ***
        details.append('\n');
        if (msgBox == Mms.MESSAGE_BOX_DRAFTS) {
            details.append(res.getString(R.string.saved_label));
        } else if (msgBox == Mms.MESSAGE_BOX_INBOX) {
            details.append(res.getString(R.string.received_label));
        } else {
            details.append(res.getString(R.string.sent_label));
        }

        details.append(MessageUtils.formatTimeStampString(
                context, msgPdu.getDate() * 1000L, true));

        // Subject: ***
        details.append('\n');
        details.append(res.getString(R.string.subject_label));

        EncodedStringValue subject = msgPdu.getSubject();
        if (subject != null) {
            String subStr = subject.getString();
            // Message size already include size of subject.
//            size += subStr.length();
            details.append(subStr);
        }
        // Priority: High/Normal/Low
        return formatDetails(details, context, msgPdu, size, res);
    }

    private static String formatDetails(StringBuilder details, Context context,
            MultimediaMessagePdu msg, int size, Resources res) {
        details.append('\n');
        details.append(res.getString(R.string.priority_label));
        details.append(getPriorityDescription(context, msg.getPriority()));

        // Message size: *** KB
        details.append('\n');
        details.append(res.getString(R.string.message_size_label));
        details.append((size - 1) / 1024 + 1);
        details.append(res.getString(R.string.kilobyte));

        return details.toString();
    }

    public static void updateNotification(final Context context) {
        new Thread(new Runnable() {
            public void run() {
                MessagingNotification.nonBlockingUpdateSendFailedNotification(context);
                MessagingNotification.updateDownloadFailedNotification(context);
                CbMessagingNotification.updateNewMessageIndicator(context);
            }
        }).start();
    }
    /// @}

    /// M: Code analyze 006, For fix bug ALPS00234739, draft can't be saved
    // after share the edited picture to the same ricipient.Remove old Mms draft
    // in conversation list instead of compose view. @{
    public static void addRemoveOldMmsThread(Runnable r) {
        mRemoveOldMmsThread = new Thread(r);
    }

    public static void asyncDeleteOldMms() {
        if (mRemoveOldMmsThread != null) {
            mRemoveOldMmsThread.start();
            mRemoveOldMmsThread = null;
        }
    }

    /// @}

    /// M: Code analyze 015, new feature, Unread message number of Mms, Phone,
    // Email and Calendar display in Launcher. @{
    private static int getUnreadMessageNumber(Context context) {
        Cursor cursor = SqliteWrapper.query(context, context.getContentResolver(),
                Uri.parse("content://mms-sms/unread_count"), null, null, null, null);
        if (cursor != null) {
            try {
                if (cursor.moveToFirst()) {
                    int count = cursor.getInt(0);
                    MmsLog.d(MmsApp.TXN_TAG, "unread message count: " + count);
                    return count;

                }
            } finally {
                cursor.close();
            }
        } else {
            MmsLog.d(MmsApp.TXN_TAG, "can not get unread message count.");
        }
        return 0;
    }

    /// @}

    /// M: Code analyze 012, For fix bug ALPS00245433, "Suggested" does not
    // show in select Sub dialog if te recipient number associates with Sub
    // cards. @{
    public static String detectCountry() {
        try {
            CountryDetector detector =
                (CountryDetector) MmsApp.getApplication().getSystemService(
                        Context.COUNTRY_DETECTOR);
            final Country country = detector.detectCountry();
            if (country != null) {
                return country.getCountryIso();
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        return null;
    }

    /// @}

    /// M: Code analyze 023, For fix bug ALPS00284546, The video can not be
    // added and there is no prompt. @{
    public static String formatNumber(String number, Context context) {
        String countryCode = detectCountry();
        AsYouTypeFormatter mFormatter =
            PhoneNumberUtil.getInstance().getAsYouTypeFormatter(countryCode);
        char [] cha = number.toCharArray();
        int ii = cha.length;
        for (int num = 0; num < ii; num++) {
            number = mFormatter.inputDigit(cha[num]);
        }
        return number;
    }

    /// @}

    /// M: Code analyze 014, new feature, add new feature vCalendar @{
    public static boolean isVCalendarAvailable(Context context) {
        final Intent intent = new Intent("mediatek.intent.action.CALENDARCHOICE");
        intent.setType("text/x-vcalendar");
        final PackageManager packageManager = context.getPackageManager();
        List<ResolveInfo> list =
                packageManager.queryIntentActivities(intent,
                        PackageManager.MATCH_DEFAULT_ONLY);
        return list.size() > 0;
    }
    /// @}

    public static boolean isSoundRecorderAvailable(Context context) {
        final Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
        intent.setType(MmsContentType.AUDIO_AMR);
        intent.setClassName("com.android.soundrecorder",
                    "com.android.soundrecorder.SoundRecorder");
        final PackageManager packageManager = context.getPackageManager();
        List<ResolveInfo> list =
                packageManager.queryIntentActivities(intent,
                        PackageManager.MATCH_DEFAULT_ONLY);
        return list.size() > 0;
    }

    /** M: Code analyze 019, For fix bug ALPS00278013, The name of the image is
     * changed to number and the suffix also disappeared when received in the email.
     * Get an unique name . It is different with the existed names.
     *
     * @param names
     * @param fileName
     * @return
     */
    public static String getUniqueName(String names[], String fileName) {
        if (names == null || names.length == 0) {
            return fileName;
        }
        int mIndex = 0;
        String tempName = "";
        String finalName = fileName;
        String extendion = "";
        String fileNamePrefix = "";
        int fileCount = 0;
        while (mIndex < names.length) {
            tempName = names[mIndex];
            if (tempName != null && tempName.equals(finalName)) {
                fileCount++;
                int tempInt = fileName.lastIndexOf(".");
                if (tempInt == -1) {
                    extendion = "";
                    fileNamePrefix = fileName;
                    finalName = fileNamePrefix + "(" + fileCount + ")" + extendion;
                } else {
                    extendion = fileName.substring(tempInt, fileName.length());
                    fileNamePrefix = fileName.substring(0, tempInt);
                    finalName = fileNamePrefix + "(" + fileCount + ")" + extendion;
                }
                mIndex = 0;
            } else {
                mIndex++;
            }
        }
        return finalName;
    }

    /** M: Code analyze 021, For fix bug ALPS00279524, The "JE" about "MMS"
     * pops up after we launch "Messaging" again.
     *
     * @param bitmap
     * @param maxWidth
     * @param maxHeight
     * @return
     */
    public static Bitmap getResizedBitmap(Bitmap bitmap, int maxWidth, int maxHeight) {
        if (bitmap == null) {
            return bitmap;
        }
        int originWidth = bitmap.getWidth();
        int originHeight = bitmap.getHeight();

        if (originWidth < maxWidth && originHeight < maxHeight) {
            return bitmap;
        }

        int width = originWidth;
        int height = originHeight;

        if (originWidth > maxWidth) {
            width = maxWidth;
            double i = originWidth * 1.0 / maxWidth;
            height = (int) Math.floor(originHeight / i);
            Bitmap mBitmap = Bitmap.createScaledBitmap(bitmap, width, height, false);
            return mBitmap;
        }

        if (originHeight > maxHeight) {
            height = maxHeight;
            double i = originHeight * 1.0 / maxHeight;
            width = (int) Math.floor(originWidth / i);
            Bitmap mBitmap = Bitmap.createScaledBitmap(bitmap, width, height, false);
            return mBitmap;
        }

        return bitmap;
    }

    /** M: Code analyze 022, For fix bug ALPS00281094, Can not send and receive Sms.
     * Return true iff the network portion of <code>address</code> is,
     * as far as we can tell on the device, suitable for use as an SMS
     * destination address.
     */
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

    /// @}

    /// M: Code analyze 024, For fix bug ALPS00296775, attachments in slideshow
    // can be saved. Network service Provider will change the file name whick
    // contains unusual characters. @{
    public static String getContentType(String contentType, String fileName) {
        String finalContentType = "";
        if (contentType == null) {
            return contentType;
        }
        if (contentType.equalsIgnoreCase("application/oct-stream")
                || contentType.equalsIgnoreCase("application/octet-stream")) {
            if (fileName != null) {
                String suffix = fileName.contains(".") ?
                        fileName.substring(fileName.lastIndexOf("."), fileName.length()) : "";
                /// M: fix bug ALPS00427229, Ignore suffix Case
                if (suffix.equals("")) {
                    return contentType;
                } else if (suffix.equalsIgnoreCase(".bmp")) {
                    finalContentType = MmsContentType.IMAGE_BMP;
                } else if (suffix.equalsIgnoreCase(".jpg")) {
                    finalContentType = MmsContentType.IMAGE_JPG;
                } else if (suffix.equalsIgnoreCase(".wbmp")) {
                    finalContentType = MmsContentType.IMAGE_WBMP;
                } else if (suffix.equalsIgnoreCase(".gif")) {
                    finalContentType = MmsContentType.IMAGE_GIF;
                } else if (suffix.equalsIgnoreCase(".png")) {
                    finalContentType = MmsContentType.IMAGE_PNG;
                } else if (suffix.equalsIgnoreCase(".jpeg")) {
                    finalContentType = MmsContentType.IMAGE_JPEG;
                } else if (suffix.equalsIgnoreCase(".vcs")) {
                    finalContentType = MmsContentType.TEXT_VCALENDAR;
                } else if (suffix.equalsIgnoreCase(".vcf")) {
                    finalContentType = MmsContentType.TEXT_VCARD;
                } else if (suffix.equalsIgnoreCase(".imy")) {
                    finalContentType = MmsContentType.AUDIO_IMELODY;
                // M: fix bug ALPS00355917
                } else if (suffix.equalsIgnoreCase(".ogg")) {
                    finalContentType = MmsContentType.AUDIO_OGG;
                } else if (suffix.equalsIgnoreCase(".aac")) {
                    finalContentType = MmsContentType.AUDIO_AAC;
                } else if (suffix.equalsIgnoreCase(".mp2")) {
                    finalContentType = MmsContentType.AUDIO_MPEG;
                /// M: fix bug ALPS00444328, 3gp audio contentType will be modified
                /// when CMCC send to CU
                } else if (suffix.equalsIgnoreCase(".3gp")) {
                    finalContentType = MmsContentType.AUDIO_3GPP;
                } else {
                    String extension = fileName.contains(".")
                            ? fileName.substring(fileName.lastIndexOf(".") + 1,
                                    fileName.length()) : "";
                    finalContentType =
                        MimeTypeMap.getSingleton().getMimeTypeFromExtension(extension);
                    if (finalContentType == null) {
                        return contentType;
                    }
                }
                return finalContentType;
            }
        }
        return contentType;
    }

    /// @}
    public static float getPreferenceValueFloat(Context context, String key, float defaultValue) {
        SharedPreferences sp = context.getSharedPreferences("com.android.mms_preferences",
                        Context.MODE_PRIVATE | Context.MODE_MULTI_PROCESS);
        return sp.getFloat(key, defaultValue);
    }

    /// M: Code analyze 025, For fix bug ALPS00298363, The "JE" pops up after
    // you launch message again. @{
    public static long getAvailableBytesInFileSystemAtGivenRoot(String rootFilePath) {
        StatFs stat = new StatFs(rootFilePath);
//        final long totalBlocks = stat.getBlockCount();
        // put a bit of margin (in case creating the file grows the system by a few blocks)
        final long availableBlocks = stat.getAvailableBlocks() - 128;

        // long mTotalSize = totalBlocks * stat.getBlockSize();
        long mAvailSize = availableBlocks * stat.getBlockSize();

        Log.i(TAG, "getAvailableBytesInFileSystemAtGivenRoot(): "
                + "available space (in bytes) in filesystem rooted at: "
                + rootFilePath + " is: " + mAvailSize);
        return mAvailSize;
    }
    /// @}

    /** M: 4.1 has removed this function
    public static void viewMmsMessageAttachment(Context context, WorkingMessage msg,
            int requestCode) {
        SlideshowModel slideshow = msg.getSlideshow();
        if (slideshow == null) {
            throw new IllegalStateException("msg.getSlideshow() == null");
        }

        SlideModel slide = slideshow.get(0);
        if (slideshow.isSimple() && slide!=null && !slide.hasAudio()) {
            MessageUtils.viewSimpleSlideshow(context, slideshow);
        } else {
            Uri uri = msg.saveAsMms(false);
            if (uri != null) {
                // Pass null for the slideshow paramater, otherwise viewMmsMessageAttachment
                // will persist the slideshow to disk again (we just did that above in saveAsMms)
                viewMmsMessageAttachment(context, uri, null, requestCode);
            }
        }
    }*/

     /** M: Move this functiom from compose activity to this class.*/
     public static boolean isRestrictedType(Context context, long msgId) {
        PduBody body = PduBodyCache.getPduBody(context, ContentUris.withAppendedId(Mms.CONTENT_URI,
                msgId));
        if (body == null) {
            return false;
        }

        int partNum = body.getPartsNum();
        for (int i = 0; i < partNum; i++) {
            PduPart part = body.getPart(i);
            int width = 0;
            int height = 0;
            String type = new String(part.getContentType());

            int mediaTypeStringId;
            if (MmsContentType.isVideoType(type)) {
                mediaTypeStringId = R.string.type_video;
            } else if (MmsContentType.isAudioType(type)
                    || "application/ogg".equalsIgnoreCase(type)) {
                mediaTypeStringId = R.string.type_audio;
            } else if (MmsContentType.isImageType(type)) {
                mediaTypeStringId = R.string.type_picture;
                InputStream input = null;
                try {
                    input = context.getContentResolver().openInputStream(part.getDataUri());
                    BitmapFactory.Options opt = new BitmapFactory.Options();
                    opt.inJustDecodeBounds = true;
                    BitmapFactory.decodeStream(input, null, opt);
                    width = opt.outWidth;
                    height = opt.outHeight;
                } catch (FileNotFoundException e) {
                    // Ignore
                    MmsLog.e(TAG, "FileNotFoundException caught while opening stream", e);
                } finally {
                    if (null != input) {
                        try {
                            input.close();
                        } catch (IOException e) {
                            // Ignore
                            MmsLog.e(TAG, "IOException caught while closing stream", e);
                        }
                    }
                }
            } else {
                continue;
            }
            if (!MmsContentType.isUnrestrictedType(type)
                    || width > MmsConfig.getMaxRestrictedImageWidth()
                    || height > MmsConfig.getMaxRestrictedImageHeight()) {
                if (WorkingMessage.sCreationMode == WorkingMessage.RESTRICTED_TYPE) {
                    Resources res = context.getResources();
                    MessageUtils.showErrorDialog((Activity) context,
                            R.string.unsupported_media_format,
                            R.string.select_different_media, mediaTypeStringId, mediaTypeStringId);
                }
                return true;
            }
        }
        return false;
    }

    /// M: new feature, SD card exist or not
    public static boolean existingSD(Context context, boolean isExternal) {
        StorageManager mStorageMamatger;
        mStorageMamatger = (StorageManager) context.getSystemService(Context.STORAGE_SERVICE);
        StorageVolume[] volumes = mStorageMamatger.getVolumeList();
        if (volumes == null) {
            return false;
        }
        String mountPoint = null;
        for (int i = 0; i < volumes.length; i++) {
            if (isExternal) {
                if (volumes[i].isRemovable()) {
                    mountPoint = volumes[i].getPath();
                    break;
                }
            } else {
                if (!volumes[i].isRemovable()) {
                    mountPoint = volumes[i].getPath();
                    break;
                }
            }
        }
        if (mountPoint == null) {
            return false;
        }
        String volumeState = mStorageMamatger.getVolumeState(mountPoint);
        return (volumeState != null && volumeState.equals(Environment.MEDIA_MOUNTED));
    }
    /// @}

    public static Intent createIntentByThreadId(Context context, long threadId, int type) {
        Intent intent = null;
        if (MtkTelephony.MtkThreads.CELL_BROADCAST_THREAD == type) {
           intent = CbMessageListActivity.createIntent(context, threadId);
           intent.putExtra("bFromLaunch", true);
        } else {
           intent = new Intent(context, ComposeMessageActivity.class);
           intent.putExtra("finish", true);
           if (threadId > 0) {
               intent.setAction(Intent.ACTION_SENDTO);
               intent.setData(Uri.parse("smsto:" + TextUtils.join(
                       ",", Conversation.get(context,
                               threadId, true).getRecipients().getNumbers())));
           }
        }
        return intent;
    }
    /// @}

    /**
     * M: copy file from srcFile to destFile.
     *
     * @param srcFile
     * @param destFile
     * @return
     */
    public static boolean copyFile(String srcFile, String destFile) {
        InputStream is = null;
        FileOutputStream os = null;
        try {
            File inFile = new File(srcFile);
            if (!inFile.exists()) {
                return false;
            }
            is = new FileInputStream(inFile);
            File outFile = new File(destFile);
            if (!outFile.exists()) {
                outFile.createNewFile();
            }
            os = new FileOutputStream(outFile);
            byte[] buffer = new byte[2048];
            for (int len = 0; (len = is.read(buffer)) != -1; ) {
                os.write(buffer, 0, len);
            }
            return true;
        } catch (FileNotFoundException e) {
            return false;
        } catch (IOException e) {
            return false;
        } finally {
            try {
                if (null != is) {
                    is.close();
                }
                if (null != os) {
                    os.close();
                }
            } catch (IOException e) {

            }
        }
    }

    /**
     * M:
     * @param path
     * @return
     */
    public static String getFileName(String path) {
        if (path == null || path.equals("")) {
            return path;
        }
        int index = path.lastIndexOf(File.separator);
        if (index > 0) {
            path = path.substring(index + 1, path.length());
        }
        return path;
    }

    /*
     * this method is similar with formatTimeStampString, except that it can show Now/Yesterday
     * if the time is within a minute
     * obviously you need to refresh to update this String after some seconds.
     */
    public static String formatTimeStampStringExtend(Context context, long when) {
        Time then = new Time();
        then.set(when);
        Time now = new Time();
        now.setToNow();

        // Basic settings for formatDateTime() we want for all cases.
        int format_flags = DateUtils.FORMAT_NO_NOON_MIDNIGHT |
                           DateUtils.FORMAT_ABBREV_ALL |
                           DateUtils.FORMAT_CAP_AMPM;

        // If the message is from a different year, show the date and year.
        if (then.year != now.year) {
            format_flags |= DateUtils.FORMAT_SHOW_YEAR | DateUtils.FORMAT_SHOW_DATE;
        } else if (then.yearDay != now.yearDay) {
            // If it is from a different day than today, show only the date.
            if ((now.yearDay - then.yearDay) == 1) {
                return context.getString(R.string.str_ipmsg_yesterday);
            } else {
                format_flags |= DateUtils.FORMAT_SHOW_DATE;
            }
        } else if ((now.toMillis(false) - then.toMillis(false)) < 60000) {
            return context.getString(R.string.time_now);
        } else {
            // Otherwise, if the message is from today, show the time.
            format_flags |= DateUtils.FORMAT_SHOW_TIME;
        }
        sOpMessageUtilsExt.formatTimeStampStringExtend(context, when, format_flags);
        return DateUtils.formatDateTime(context, when, format_flags);
    }

    private static List<String> getHomes(Context context) {
        MmsLog.d(TAG, "DialogModeActivity.getHomes");

        List<String> names = new ArrayList<String>();
        PackageManager packageManager = context.getPackageManager();
        Intent intent = new Intent(Intent.ACTION_MAIN);
        intent.addCategory(Intent.CATEGORY_HOME);
        List<ResolveInfo> resolveInfo = packageManager.queryIntentActivities(intent,
            PackageManager.MATCH_DEFAULT_ONLY);

        for (ResolveInfo ri : resolveInfo) {
            names.add(ri.activityInfo.packageName);
            MmsLog.d(TAG, "package name=" + ri.activityInfo.packageName
                    + " class name=" + ri.activityInfo.name);
        }
        return names;
    }

    public static boolean isHome(Context context) {
        List<String> homePackageNames = getHomes(context);
        String packageName = "";
        String className = "";
        boolean ret = false;

        ActivityManager activityManager =
            (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
        List<RunningTaskInfo> rti = activityManager.getRunningTasks(2);

        if (rti != null && rti.size() > 0) {
            packageName = rti.get(0).topActivity.getPackageName();
            className = rti.get(0).topActivity.getClassName();
        }

        MmsLog.d(TAG, "package0= " + packageName + " class0=" + className);

        ret = homePackageNames.contains(packageName);
        if (!ret) {
            if ("com.mediatek.mms.ui.DialogModeActivity".equals(className)) {
                ret = true;
            }
        }

        /// M: fix bug ALPS00687923, check RunningAppProcessInfo IMPORTANCE_FOREGROUND @{
        if (!ret) {
            List<RunningAppProcessInfo> appProcesses = activityManager.getRunningAppProcesses();
            if (appProcesses == null || appProcesses.size() == 0) {
                MmsLog.d(TAG, "appProcesses == null || appProcesses.size() == 0");
                ret = false;
            } else {
                for (RunningAppProcessInfo appProcess : appProcesses) {
                    if (appProcess.importance == RunningAppProcessInfo.IMPORTANCE_FOREGROUND
                            && appProcess.processName.equals("com.android.launcher")) {
                        MmsLog.d(TAG, "IMPORTANCE_FOREGROUND == com.android.launcher");
                        ret = true;
                    }
                }
            }
        }
        /// @}

        return ret;
    }

    /**
     * M: read app drawable resource and create a new file in /data/data/com.android.mms/files path.
     * @param context
     * @param fileName
     * @return
     */
    public static boolean createFileForResource(Context context,
            String fileName, int fileResourceId) {
        OutputStream os = null;
        InputStream ins = null;
        try {
            os = context.openFileOutput(fileName, Context.MODE_PRIVATE);
            ins = context.getResources().openRawResource(fileResourceId);
            byte[] buffer = new byte[2048];
            for (int len = 0; (len = ins.read(buffer)) != -1; ) {
                os.write(buffer, 0, len);
            }
            return true;
        } catch (FileNotFoundException e) {
            MmsLog.e(TAG, "create file failed.", e);
            return false;
        } catch (IOException e) {
            MmsLog.e(TAG, "create file failed.", e);
            return false;
        } finally {
            try {
                if (null != ins) {
                    ins.close();
                }
                if (null != os) {
                    os.close();
                }
            } catch (IOException e) {
                MmsLog.e(TAG, "createFileForResource:" + e.toString());
            }
        }
    }

    public static int getStatusResourceId(Context context, MessageItem msgItem) {
        if (msgItem == null) {
            MmsLog.e(TAG, "getStatusResourceId(): MsgItem is null!", new Exception());
            return 0;
        }
        if (msgItem.isSubMsg()) {
            MmsLog.d(TAG, "getStatusResourceId(): Sub message.");
            return 0;
        }
        if (msgItem.isMms()) {
            if (msgItem.mHasReadReport) {
                return R.drawable.im_meg_status_read;
            }
            if (msgItem.mHasDeliveryReport) {
                return R.drawable.im_meg_status_reach;
            }

            switch (msgItem.mBoxId) {
            case Mms.MESSAGE_BOX_SENT:
                return R.drawable.im_meg_status_out;

            case Mms.MESSAGE_BOX_OUTBOX:
            case Mms.MESSAGE_BOX_FAILED:
                MmsLog.dpi(MmsApp.TXN_TAG, "mms is sending, uri = " + msgItem.mMessageUri);
                return R.drawable.im_meg_status_sending;

            case Mms.MESSAGE_BOX_INBOX:
            case Mms.MESSAGE_BOX_DRAFTS:
            case Mms.MESSAGE_BOX_ALL:
            default:
                return 0;
            }
        } else if (msgItem.isSms()) {
            switch (msgItem.mBoxId) {
            case Sms.MESSAGE_TYPE_QUEUED:
            case Sms.MESSAGE_TYPE_OUTBOX:
                return R.drawable.im_meg_status_sending;

            case Sms.MESSAGE_TYPE_SENT:
                return R.drawable.im_meg_status_out;

            case Sms.MESSAGE_TYPE_FAILED:
                ///M: this status has been handled by common flow
            case Sms.MESSAGE_TYPE_INBOX:
            case Sms.MESSAGE_TYPE_DRAFT:
            case Sms.MESSAGE_TYPE_ALL:
            default:
                return 0;
            }
        }
        return 0;
    }

    public static List<MmsReportStatus> getMmsReportStatus(Context context, long messageId) {
        MmsLog.d(TAG, "getMmsReportStatus(): messageId = " + messageId);
        Uri uri = Uri.withAppendedPath(Mms.REPORT_STATUS_URI, String.valueOf(messageId));
        Cursor c = SqliteWrapper.query(context, context.getContentResolver(), uri,
            new String[] {Mms.Addr.ADDRESS, "delivery_status", "read_status"}, null, null, null);
        if (c == null) {
            return null;
        }
        try {
            List<MmsReportStatus> mmsReportStatusList = new ArrayList<MmsReportStatus>();
            while (c.moveToNext()) {
                int columnDeliveryStatus = 1;
                int columnReadStatus = 2;
                mmsReportStatusList.add(new MmsReportStatus(c.getInt(columnDeliveryStatus),
                        c.getInt(columnReadStatus)));
            }
            return mmsReportStatusList;
        } finally {
            c.close();
        }
    }

    /**
     * M: For EVDO: check the sim is whether UIM or not.
     * @param subId the sim's sub id.
     * @return true: UIM; false: not UIM.
     */
    public static boolean isUSimType(int subId) {
        String phoneType = MtkTelephonyManagerEx.getDefault().getIccCardType(subId);
        if (phoneType == null) {
            Log.d(TAG, "[isUIMType]: phoneType = null");
            return false;
        }
        Log.d(TAG, "[isUIMType]: phoneType = " + phoneType);
        return phoneType.equalsIgnoreCase("CSIM") || phoneType.equalsIgnoreCase("UIM")
            || phoneType.equalsIgnoreCase("RUIM");
    }

    /**
     * M: for check CSIM in gsm mode or not.
     * @param subId the CSIM's sub id.
     * @return true: in gsm; false: no.
     */
    public static boolean isCSIMInGsmMode(int subId) {
        if (isUSimType(subId)) {
            MtkTelephonyManagerEx tmEx = MtkTelephonyManagerEx.getDefault();
            int vnt = tmEx.getPhoneType(SubscriptionManager.getSlotIndex(subId));
            Log.d(TAG,
                "[isCSIMInGsmMode]:[NO_PHONE = 0; GSM_PHONE = 1; CDMA_PHONE = 2;]; phoneType:"
                    + vnt);
            if (vnt == TelephonyManager.PHONE_TYPE_GSM) {
                return true;
            }
        }
        return false;
    }

    public static CharSequence formatMsgContent(String subject, String body,
            String displayAddress) {
        StringBuilder buf = new StringBuilder(
                displayAddress == null
                ? ""
                : displayAddress.replace('\n', ' ').replace('\r', ' '));
        buf.append(':').append(' ');

        int offset = buf.length();
        if (!TextUtils.isEmpty(subject)) {
            subject = subject.replace('\n', ' ').replace('\r', ' ');
            buf.append(subject);
            buf.append(' ');
        }

        if (!TextUtils.isEmpty(body)) {
            body = body.replace('\n', ' ').replace('\r', ' ');
            buf.append(body);
        }

        SpannableString spanText = new SpannableString(buf.toString());
        spanText.setSpan(new StyleSpan(Typeface.BOLD), 0, offset,
                Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);

        return spanText;
    }

    public static boolean haveEmailContact(String emailAddress, Context context) {
        Cursor cursor = SqliteWrapper.query(context, context.getContentResolver(),
                Uri.withAppendedPath(Email.CONTENT_LOOKUP_URI, Uri.encode(emailAddress)),
                new String[] { Contacts.DISPLAY_NAME }, null, null, null);

        if (cursor != null) {
            try {
                String name;
                while (cursor.moveToNext()) {
                    name = cursor.getString(0);
                    if (!TextUtils.isEmpty(name)) {
                        return true;
                    }
                }
            } finally {
                cursor.close();
            }
        }
        return false;
    }

    public static CharSequence getVisualTextName(Context context,
                                                String enumName,
                                                int choiceNameResId,
                                                int choiceValueResId) {
        CharSequence[] visualNames = context.getResources().getTextArray(choiceNameResId);

        visualNames = sOpMessageUtilsExt.getVisualTextName(
                visualNames, context, choiceNameResId == R.array.pref_sms_save_location_choices);

        CharSequence[] enumNames = context.getResources().getTextArray(choiceValueResId);
        // Sanity check
        if (visualNames.length != enumNames.length) {
            return "";
        }
        for (int i = 0; i < enumNames.length; i++) {
            if (enumNames[i].equals(enumName)) {
                return visualNames[i];
            }
        }
        return "";
    }

    public static File getStorageFile(String filename, Context context) {
        String dir = "";
        String path = StorageManagerEx.getDefaultPath();
        if (path == null) {
            MmsLog.e(TAG, "default path is null");
            return null;
        }
        dir = path + "/" + Environment.DIRECTORY_DOWNLOADS + "/";
        MmsLog.ipi(TAG, "copyPart,  file full path is " + dir + filename);
        File file = getUniqueDestination(dir + filename);

        // make sure the path is valid and directories created for this file.
        File parentFile = file.getParentFile();
        if (!parentFile.exists() && !parentFile.mkdirs()) {
            MmsLog.epi(TAG, "[MMS] copyPart: mkdirs for " + parentFile.getPath()
                    + " failed!");
            return null;
        }
        return file;
    }

    public static String getMainCardDisplayName() {
        String mainSubDisplayName = "";
        Context ct = MmsApp.getApplication();
        int mainSubId = (int) MmsConfig.DEFAULT_SIM_NOT_SET;
        if (mainSubId != MmsConfig.DEFAULT_SIM_SETTING_ALWAYS_ASK
                && mainSubId != MmsConfig.DEFAULT_SIM_NOT_SET) {
            SubscriptionInfo info = SubscriptionManager.from(MmsApp.getApplication())
                    .getActiveSubscriptionInfo(mainSubId);
            mainSubDisplayName = info.getDisplayName().toString();
        } else {
            SubscriptionInfo info = SubscriptionManager.from(MmsApp.getApplication())
                    .getActiveSubscriptionInfo(mainSubId);
            if (info != null && info.getSubscriptionId() > 0) {
                mainSubDisplayName = info.getDisplayName().toString();
            } else {
                info = SubscriptionManager.from(MmsApp.getApplication()).getActiveSubscriptionInfo(
                        mainSubId);
                if (info != null && info.getSubscriptionId() > 0) {
                    mainSubDisplayName = info.getDisplayName().toString();
                } else {
                    MmsLog.e(TAG, "error to get main sub display name");
                }
            }
        }
        return mainSubDisplayName;
    }

  //Modify ALPS00445952, if this attachment didn't include extension(index < 0),
  //fileName.substring(0, index) will throw exception, and save attachment fail.
  //So we should deal with this situation.

    public static File getUniqueDestination(String fileName) {
        File file ;
        final int index = fileName.indexOf(".");
        if (index > 0) {
            final String extension = fileName.substring(index + 1, fileName.length());
            final String base = fileName.substring(0, index);
            file = new File(base + "." + extension);
            for (int i = 2; file.exists(); i++) {
                file = new File(base + "_" + i + "." + extension);
            }
        } else {
            file = new File(fileName);
            for (int i = 2; file.exists(); i++) {
                file = new File(fileName + "_" + i);
            }
        }
        return file;
  }

    /* remove NotificationPlus notiPlus
    public static void handleNewNotification(Context context, int messageCount) {
        Intent clickIntent = new Intent(Intent.ACTION_MAIN);
        //clickIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
        //        | Intent.FLAG_ACTIVITY_SINGLE_TOP
        //        | Intent.FLAG_ACTIVITY_CLEAR_TOP);
        clickIntent.setClassName("com.android.mms", "com.android.mms.ui.BootActivity");
        // Make a startActivity() PendingIntent for the notification.
        PendingIntent pendingIntent = PendingIntent.getActivity(context, 0, clickIntent,
                PendingIntent.FLAG_UPDATE_CURRENT);

        EncapsulatedNotificationPlus notiPlus =
                new EncapsulatedNotificationPlus.EncapsulatedBuilder(context)
                .setTitle(context.getString(R.string.new_message))
                .setMessage(context.getResources().getQuantityString(
                    R.plurals.notification_multiple, messageCount, messageCount))
                .setPositiveButton(context.getString(R.string.view), pendingIntent)
                .create();
        EncapsulatedNotificationManagerPlus.notify(1, notiPlus);
    }
    */

    public static void setMmsLimitSize(Context context) {
        Context otherAppContext = null;
        SharedPreferences sp = null;
        try {
            otherAppContext = context.createPackageContext("com.android.mms",
                    Context.CONTEXT_IGNORE_SECURITY);

        } catch (Exception e) {
            MmsLog.e(TAG, "ConversationList NotFoundContext");
        }
        if (otherAppContext != null) {
           sp = otherAppContext.
                    getSharedPreferences("com.android.mms_preferences", Context.MODE_PRIVATE);
        }
        String mSizeLimitTemp = null;
        int mMmsSizeLimit = 0;
        if (sp != null) {
            mSizeLimitTemp = sp.getString("pref_key_mms_size_limit", "300");
        }
        if (mSizeLimitTemp != null && 0 == mSizeLimitTemp.compareTo("100")) {
            mMmsSizeLimit = 100;
        } else if (mSizeLimitTemp != null && 0 == mSizeLimitTemp.compareTo("200")) {
            mMmsSizeLimit = 200;
        } else {
            mMmsSizeLimit = 300;
        }
        MmsConfig.setUserSetMmsSizeLimit(mMmsSizeLimit);
    }

    /// M: fix bug ALPS604911, change MmsContentType when share multi-file from FileManager @{
    public static String getContentType(Uri uri) {
        String path = uri.getPath();

        MimeTypeMap mimeTypeMap = MimeTypeMap.getSingleton();
        String extension = MimeTypeMap.getFileExtensionFromUrl(path).toLowerCase();
        if (TextUtils.isEmpty(extension)) {
            int dotPos = path.lastIndexOf('.');
            if (0 <= dotPos) {
                extension = path.substring(dotPos + 1);
                extension = extension.toLowerCase();
            }
        }

        String type = mimeTypeMap.getMimeTypeFromExtension(extension);
        return type;
    }
    /// @}

    /// M: fix bug ALPS01379798, support multi-file share from KK Download @{
    public static String queryContentType(Context context, Uri uri) {
        String type = context.getContentResolver().getType(uri);
        return type;
    }
    /// @}

    /*
     * M: If image can display return true, else return false.
     */
    public static boolean checkImageOK(Context context, Uri imageUri) {
        try {
            Bitmap bitmap = null;
            InputStream mInputStream = null;
            try {
                mInputStream = context.getContentResolver().openInputStream(imageUri);
                if (mInputStream != null) {
                    bitmap = BitmapFactory.decodeStream(mInputStream);
                }
                if (bitmap == null) {
                    return false;
                } else {
                    return true;
                }
            } catch (FileNotFoundException e) {
                bitmap = null;
            } finally {
                if (mInputStream != null) {
                    mInputStream.close();
                }
            }
        } catch (java.lang.OutOfMemoryError e) {
            Log.e(TAG, "checkImageOK(Uri): out of memory: ", e);
        } catch (IOException e) {
            Log.e(TAG, "checkImageOK(Uri): IOException: ", e);
        } catch (Exception e) {
            Log.e(TAG, "checkImageOK(Uri): Exception: ", e);
        }
        return false;
    }

    /*
     * M: If image can not display, show default broken image.
     */
    public static Bitmap getDefaultBrokenImage(Context context) {
        try {
            Bitmap bitmap = BitmapFactory.decodeResource(context.getResources(),
                    R.drawable.ic_missing_thumbnail_picture);
            return bitmap;
        } catch (java.lang.OutOfMemoryError e) {
            Log.e(TAG, "getDefaultBrokenImage: out of memory: ", e);
        }
        return null;
    }

    public static void updatePartsIfNeeded(SlideshowModel slideshow, MtkPduPersister persister,
            Uri uri, PduBody pb, HashMap<Uri, InputStream> preOpenedFiles) throws MmsException {
        try {
            if (!slideshow.needUpdate()) {
                return;
            }
        } catch (ArrayIndexOutOfBoundsException e) {
            Log.e(TAG, "updatePartsIfNeeded: array index out of bounds: ", e);
            return;
        }
        persister.updateParts(uri, pb, preOpenedFiles);
        slideshow.resetUpdateState();
    }

    public static int calculateWallpaperSize(Context context, int height, int width) {
        WindowManager windowManager =
            (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
        int currentMaxHeight = windowManager.getDefaultDisplay().getHeight();
        int currentMaxWidth = windowManager.getDefaultDisplay().getWidth();
        MmsLog.d(TAG, "CurrentMaxHeight = "
                + currentMaxHeight + " CurrentMaxWidth = " + currentMaxWidth);
        int ratio = 1;
        while ((height / ratio) > currentMaxHeight || (width / ratio) > currentMaxWidth) {
            ratio *= 2;
        }
        return ratio;
    }

    /// M: fix bug ALPS01523754.set google+ pic as wallpaper.@{
    /**
     * @param uri The Uri to check.
     * @return Whether the Uri authority is Google Photos.
     */
    public static boolean isGooglePhotosUri(Uri uri) {
        return "com.google.android.apps.photos.content".equals(uri.getAuthority());
    }

    public static boolean isSimMessageAccessable(Context context, int... subId) {
        // First, forbid to access SIM message if this is not default MMS.
        boolean isSmsEnable = MmsConfig.isSmsEnabled(context);
        if (!isSmsEnable) {
            MmsLog.d(TAG, "isSimMessageAccessable Sms not enabled");
            return false;
        }

        // Second, check airplane mode
        boolean airplaneOn = Settings.System.getInt(context.getContentResolver(),
                Settings.System.AIRPLANE_MODE_ON, 0) == 1;
        if (airplaneOn) {
            MmsLog.d(TAG, "isSimMessageAccessable airplane is On");
            return false;
        }

        // Third, check whether has inserted SIM
        List<SubscriptionInfo> subInfoList = SubscriptionManager.from(MmsApp.getApplication())
                .getActiveSubscriptionInfoList();
        if (subInfoList == null || subInfoList.size() == 0) {
            MmsLog.d(TAG, "isSimMessageAccessable SIM not insert");
            return false;
        }

        // Forth, check SIM ON/OFF state
        boolean simOnOffEnabled = MtkTelephonyManagerEx.getDefault().isSimOnOffEnabled();
        MmsLog.d(TAG, "simOnOffEnabled = " + simOnOffEnabled + " subId = " + subId);

        if (simOnOffEnabled == true && subId != null) {
            int subIdLen = subId.length;
            if (subIdLen == 1) {
                MmsLog.d(TAG, "just 1 slotId = " + subId[0]);
                SubscriptionInfo subInfo = SubscriptionManager.from(MmsApp.getApplication()).
                    getActiveSubscriptionInfo(subId[0]);
                if (subInfo != null) {
                    int slotId = subInfo.getSimSlotIndex();
                    int slotState = MtkTelephonyManagerEx.getDefault().getSimOnOffState(slotId);
                    if (slotState != MtkTelephonyManagerEx.SIM_POWER_STATE_SIM_ON) {
                        return false;
                    }
                } else {
                    return false;
                }
            } else if (subIdLen == 0) {
                MmsLog.d(TAG, "no slotId passed");
                boolean anyOn = false;
                for (SubscriptionInfo subInfo : subInfoList) {
                    if (subInfo != null) {
                        int slotId = subInfo.getSimSlotIndex();
                        int slotState = MtkTelephonyManagerEx.getDefault().getSimOnOffState(slotId);
                        MmsLog.d(TAG, "slotState = " + slotState);
                        if (slotState == MtkTelephonyManagerEx.SIM_POWER_STATE_SIM_ON) {
                            anyOn = true;
                        }
                    }
                }
                if (anyOn == false) {
                    return false;
                }
            }
        }

        // Fifth, check sms ready
        IMtkSms mSmsManager = IMtkSms.Stub.asInterface(ServiceManager.getService("imtksms"));
        //ISms.Stub.asInterface(ServiceManager.getService("isms"));
        if (mSmsManager == null) {
            MmsLog.d(TAG, "isSimMessageAccessable mSmsManager is null");
            return false;
        }
        boolean isSimReady = false;
        try {
            if (subId.length == 1) {
                isSimReady = mSmsManager.isSmsReadyForSubscriber(subId[0]);
            } else {
                for (SubscriptionInfo subInfoRecord : subInfoList) {
                    isSimReady =
                        mSmsManager.isSmsReadyForSubscriber(subInfoRecord.getSubscriptionId());
                    if (isSimReady) {
                        break;
                    }
                }
            }
        } catch (RemoteException e) {
            MmsLog.e(TAG, "isSimMessageAccessable failed to get sms state");
            isSimReady = false;
        }

        MmsLog.d(TAG, "isSimMessageAccessable" + isSimReady);
        return isSimReady;
    }


    public static String getTempWallpaper(Context context, Uri uri) {
        InputStream is = null;
        FileOutputStream fos = null;
        String tempPath = TempFileProvider.getScrapPath(context, TEMP_WALLPAPER);
        try {
            is = context.getContentResolver().openInputStream(uri);
            File tempFile = new File(tempPath);
            if (is != null) {
                if (tempFile.exists()) {
                    tempFile.delete();
                }
                File parentFile = tempFile.getParentFile();
                if (!parentFile.exists() && !parentFile.mkdirs()) {
                    MmsLog.w(TAG, "getTempWallpaper parentFile.mkdirs fail");
                    return null;
                }
                tempFile.createNewFile();
            }
            fos = new FileOutputStream(tempFile);
            byte[] buffer = new byte[8192];
            for (int len = 0; (len = is.read(buffer)) != -1; ) {
                fos.write(buffer, 0, len);
            }
            return tempPath;
        } catch (FileNotFoundException e) {
            MmsLog.e(TAG, "getTempWallpaper FileNotFoundException", e);
        } catch (IOException e) {
            MmsLog.d(TAG, "getTempWallpaper IOException", e);
        } finally {
            if (fos != null) {
                try {
                    fos.close();
                } catch (IOException e) {
                    MmsLog.epi(TAG, "getTempWallpaper IOException while closing: " + fos, e);
                }
            }
            if (is != null) {
                try {
                    is.close();
                } catch (IOException e) {
                    MmsLog.epi(TAG, "getTempWallpaper IOException while closing: " + is, e);
                }
            }
        }
        return null;
    }
    /// @}

    /**
     * @return Whether use Sim Simulator Toll or not.
     */
    public static boolean isUseSubSimulator() {
        /* P0 migration*/
        String enableNowSMS = ""; //SystemProperties.get("net.ENABLE_NOWSMS");
        //String enableSimulator = SystemProperties.get("net.Enable_Simulator_Tool");
        if (enableNowSMS.equals("true")) {
            return true;
        }
        return false;
    }

    public static boolean shouldShowTimeDivider(long curTime, long nextTime) {
        Date curDate = new Date(curTime);
        Date nextDate = new Date(nextTime);
        Date cur = new Date(curDate.getYear(), curDate.getMonth(), curDate.getDate(), 0, 0, 0);
        Date next = new Date(nextDate.getYear(), nextDate.getMonth(), nextDate.getDate(), 0, 0, 0);
        return (cur.getTime() != next.getTime());
    }

    public static String getTimeDividerString(Context context, long when) {
        Time then = new Time();
        then.set(when);
        Time now = new Time();
        now.setToNow();

        // Basic settings for formatDateTime() we want for all cases.
        int formatFlags = DateUtils.FORMAT_NO_NOON_MIDNIGHT | DateUtils.FORMAT_ABBREV_ALL
                | DateUtils.FORMAT_CAP_AMPM;

        // If the message is from a different year, show the date and year.
        if (then.year != now.year) {
            formatFlags |= DateUtils.FORMAT_SHOW_YEAR | DateUtils.FORMAT_SHOW_DATE;
        } else if (then.yearDay != now.yearDay) {
            // If it is from a different day than today, show only the date.
            formatFlags |= DateUtils.FORMAT_SHOW_DATE;
            Date curDate = new Date();
            Date cur = new Date(curDate.getYear(), curDate.getMonth(), curDate.getDate(), 0, 0, 0);
            long oneDay = 24 * 60 * 60 * 1000;
            long elapsedTime = cur.getTime() - when;
            if (elapsedTime < oneDay && elapsedTime > 0) {
                return context.getResources().getString(R.string.str_ipmsg_yesterday);
            }
        } else {
            return context.getString(R.string.str_ipmsg_today);
        }
        return DateUtils.formatDateTime(context, when, formatFlags);
    }

    public static String getShortTimeString(Context context, long time) {
        int formatFlags = DateUtils.FORMAT_NO_NOON_MIDNIGHT | DateUtils.FORMAT_CAP_AMPM;
        formatFlags |= DateUtils.FORMAT_SHOW_TIME;
        return DateUtils.formatDateTime(context, time, formatFlags);

    }

    public static String unescapeXML(String str) {
        return str.replaceAll("&lt;", "<")
                .replaceAll("&gt;", ">")
                .replaceAll("&quot;", "\"")
                .replaceAll("&apos;", "'")
                .replaceAll("&amp;", "&");
    }

    public static boolean allowSafeDraft(final Activity activity, boolean deviceStorageIsFull,
            boolean isNofityUser, int toastType) {
        return sOpMessageUtilsExt.allowSafeDraft(activity, deviceStorageIsFull, isNofityUser,
                toastType);
    }

    ///M: WFC: utility api @ {
    public static boolean isSimPresent(Context context) {
        int[] subs = SubscriptionManager.from(context).getActiveSubscriptionIdList();
        if (subs.length == 0) {
            MmsLog.d(TAG, "Sim not present");
            return false;
        } else {
            MmsLog.d(TAG, "Sim present");
            return true;
        }
    }

    public static boolean isWifiConnected(Context context) {
        WifiManager wifiManager = (WifiManager) context.getSystemService(Context.WIFI_SERVICE);
        ConnectivityManager cm =
            (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo wifi = cm.getNetworkInfo(ConnectivityManager.TYPE_WIFI);
        MmsLog.d(TAG, "wifi state:"
                + wifiManager.getWifiState() + "wifi connected:" + wifi.isConnected());
        if (wifiManager.getWifiState() == WifiManager.WIFI_STATE_ENABLED && wifi.isConnected()) {
            MmsLog.d(TAG, "Wifi connected");
            return true;
        }
        MmsLog.d(TAG, "Wifi off or not connected");
        return false;
    }

    public static int getCellularState(int selectedSubId, Context context) {
        MmsLog.d(TAG, "getCellularState() subid: " + selectedSubId);
        IMtkTelephonyEx telephonyEx = IMtkTelephonyEx.Stub.asInterface(
            ServiceManager.getService("phoneEx"));
        Bundle bundle = null;
        ServiceState serviceState = null;

            serviceState = TelephonyManager.getDefault().
                        getServiceStateForSubscriber(selectedSubId);
//            bundle = telephonyEx.getServiceState(selectedSubId);

        if (serviceState != null) {
            return serviceState.getState();
        } else {
            return ServiceState.STATE_OUT_OF_SERVICE;
        }
    }
    /// @}

    /**
     * check whether the card inserted into the slot is a CDMA card
     */
    public static boolean isCdmaCard(int slotId) {
        boolean isCdma = false;
        MtkTelephonyManagerEx telephonyMgrEx = MtkTelephonyManagerEx.getDefault();
        String[] types = telephonyMgrEx.getSupportCardType(slotId);

        if (types == null) {
            Log.d(TAG, "isCdmaCard, types = null");
            return isCdma;
        }

        for (String type : types) {
            if ("RUIM".equals(type) || "CSIM".equals(type)) {
                isCdma = true;
            } else if ("SIM".equals(type) && telephonyMgrEx.isCt3gDualMode(slotId)) {
                isCdma = true;
            }
        }

        Log.d(TAG, "isCdmaCard, slotId = " + slotId + ", types = " + types
                + ", isCdma = " + isCdma);
        return isCdma;
    }

    private static void recycleBitmap(ImageView view) {
        MmsLog.d(TAG, "recycleBitmap");
        if (view == null) {
            MmsLog.d(TAG, "recycleBitmap1");
            return;
        }
        MmsLog.d(TAG, "recycleBitmap2");
        BitmapDrawable bd = (BitmapDrawable) view.getDrawable();
        view.setImageBitmap(null);
        if (bd == null) {
            MmsLog.d(TAG, "recycleBitmap3");
            return;
        }
        MmsLog.d(TAG, "recycleBitmap4");
        Bitmap bm = bd.getBitmap();
        MmsLog.d(TAG, "bm = " + bm);

        if (bm != null && !bm.isRecycled()) {
            MmsLog.d(TAG, "recycleBitmap5");
            bm.recycle();
            bm = null;
        }
        MmsLog.d(TAG, "recycleBitmap6");
    }
}

/**
 * The common code about delete progress dialogs.
 */
/// M: Code analyze 018, For fix bug ALPS00278539, ANR occured when clicking
// the deleting message. @{
class DeleteProgressDialogUtil {
    /**
     * Gets a delete progress dialog.
     * @param context the activity context.
     * @return the delete progress dialog.
     */
    public static NewProgressDialog getProgressDialog(Context context) {
        NewProgressDialog dialog = new NewProgressDialog(context);
        dialog.setCancelable(false);
        dialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
        dialog.setMessage(context.getString(R.string.deleting));
        dialog.setMax(1); /* default is one complete */
        // ignore the search key, when deleting we do not want the search bar come out.
        dialog.setOnKeyListener(new DialogInterface.OnKeyListener() {
            public boolean onKey(DialogInterface dialog, int keyCode, KeyEvent event) {
                return (keyCode == KeyEvent.KEYCODE_SEARCH);
            }
        });
        return dialog;
    }
}

// M: fix bug ALPS00351620
class SearchProgressDialogUtil {
    /**
     * Gets a search progress dialog.
     * @param context the activity context.
     * @return the search progress dialog.
     */
    public static NewProgressDialog getProgressDialog(Context context) {
        NewProgressDialog dialog = new NewProgressDialog(context);
        dialog.setCancelable(false);
        dialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
        dialog.setMessage(context.getString(R.string.refreshing));
        dialog.setMax(1); /* default is one complete */
        // ignore the search key, when searching we do not want the search bar come out.
        dialog.setOnKeyListener(new DialogInterface.OnKeyListener() {
            public boolean onKey(DialogInterface dialog, int keyCode, KeyEvent event) {
                return (keyCode == KeyEvent.KEYCODE_SEARCH);
            }
        });
        return dialog;
    }
}

class NewProgressDialog extends ProgressDialog {
    private boolean mIsDismiss = false;
    public NewProgressDialog(Context context) {
        super(context);
    }

    public void dismiss() {
       if (isDismiss()) {
           super.dismiss();
       }
    }

    public synchronized void setDismiss(boolean isDismiss) {
        this.mIsDismiss = isDismiss;
    }

    public synchronized boolean isDismiss() {
        return mIsDismiss;
    }
}
/// @}
