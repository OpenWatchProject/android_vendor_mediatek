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

package com.mediatek.rcse.plugin.message;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.ContentUris;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.util.AndroidException;
import android.util.Log;
import android.database.Cursor;
import android.database.sqlite.SqliteWrapper;
import android.graphics.Bitmap;
import android.graphics.Bitmap.CompressFormat;
import android.graphics.BitmapFactory;
import android.graphics.BitmapFactory.Options;
import android.graphics.drawable.Drawable;
import android.graphics.Matrix;
import android.graphics.Rect;
import android.media.CamcorderProfile;
import android.media.ExifInterface;
import android.media.RingtoneManager;
import android.net.Uri;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.os.StatFs;
import android.provider.Settings;
import android.provider.Telephony.Sms;
import android.text.format.DateFormat;
import android.text.format.DateUtils;
import android.text.format.Time;
import android.text.method.LinkMovementMethod;
import android.text.Spannable;
import android.text.SpannableString;
import android.text.SpannableStringBuilder;
import android.text.Spanned;
import android.text.style.ClickableSpan;
import android.text.style.URLSpan;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.ImageView;
import android.widget.SimpleAdapter;
import android.widget.TextView;
import android.widget.Toast;
import mediatek.telephony.MtkTelephony;
import android.provider.Telephony;
import android.telephony.SubscriptionManager;


//import com.mediatek.common.MPlugin;
import com.mediatek.rcse.plugin.message.IpMessageConsts.*;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcs.R;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
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
import java.util.Map;

import com.android.i18n.phonenumbers.AsYouTypeFormatter;
import com.android.i18n.phonenumbers.PhoneNumberUtil;

import android.location.Country;
import android.location.CountryDetector;

import com.google.android.mms.ContentType;

import com.mediatek.mms.ipmessage.DefaultIpUtilsExt;
import com.mediatek.mms.callback.IUtilsCallback;
import mediatek.telephony.MtkTelephony.MtkSms;

public class IpMessageUtils extends DefaultIpUtilsExt {
    private static final String TAG = "Mms/ipmsg/utils";

    private static final String[] SMS_BODY_PROJECTION_WITH_IPMSG_ID
                        = { Sms._ID, MtkTelephony.MtkSms.IPMSG_ID };
    private static final String SMS_DRAFT_WHERE = Sms.TYPE + "=" + Sms.MESSAGE_TYPE_DRAFT;
    private static final int SMS_ID_INDEX = 0;
    private static final int SMS_IPMSG_ID_INDEX = 1;

    public static final int SDCARD_SIZE_RESERVED = 5 * 1024;

    public static final String SELECTION_CONTACT_RESULT = "contactId";
    public static final String IPMSG_NOTIFICATION_TAG = "Mms/noti";
    public static final String WALLPAPER_PATH = "/data/data/com.android.providers.telephony/app_wallpaper";
    private static final Uri THREAD_SETTINGS_URI = Uri.parse("content://mms-sms/thread_settings/");

    public static void startRemoteActivity(Context context, Intent intent) {
        IpMessageActivitiesManager.getInstance(context).startRemoteActivity(context, intent);
    }

    public static void startRemoteActivityForResult(Context context, Intent intent) {
        IpMessageActivitiesManager.getInstance(context).startRemoteActivity(context, intent);
    }

    public static final int IPMSG_ERROR_ACTION                  = 0;
    public static final int IPMSG_NEW_MESSAGE_ACTION            = 1;
    public static final int IPMSG_REFRESH_CONTACT_LIST_ACTION   = 2;
    public static final int IPMSG_REFRESH_GROUP_LIST_ACTION     = 3;
    public static final int IPMSG_SERCIVE_STATUS_ACTION         = 4;
    public static final int IPMSG_IM_STATUS_ACTION              = 5;
    public static final int IPMSG_SAVE_HISTORY_ACTION           = 6;
    public static final int IPMSG_ACTIVATION_STATUS_ACTION      = 7;
    public static final int IPMSG_IP_MESSAGE_STATUS_ACTION      = 8;
    public static final int IPMSG_DOWNLOAD_ATTACH_STATUS_ACTION = 9;
    public static final int IPMSG_SET_PROFILE_RESULT_ACTION     = 10;
    public static final int IPMSG_BACKUP_MSG_STATUS_ACTION      = 11;
    public static final int IPMSG_RESTORE_MSG_STATUS_ACTION     = 12;
    public static final int IPMSG_UPDATE_GROUP_INFO             = 13;
    public static final int IPMSG_IPMESSAGE_CONTACT_UPDATE      = 14;
    public static final int IPMSG_SIM_INFO_ACTION               = 15;
    // Add for Joyn
    public static final int IPMSG_DISABLE_SERVICE_STATUS_ACTION = 16;
    /// M: add for ipmessage @{
    public static final int IPMSG_REG_STATUS_ACTION = 17;
    public static final int IPMSG_FALLBACK_MMS = 18;
    /// @}

    public static final String PLUGIN_VERSION = "2.0.0";
    public static final String PLUGIN_METANAME = "class";

    private static IUtilsCallback mCallback;

    @Override
    public boolean initIpUtils(IUtilsCallback callback) {
        mCallback = callback;
        return true;
    }

    public static String formatIpTimeStampString(long when, boolean fullFormat, Context context) {
        if (mCallback != null) {
            return mCallback.formatIpTimeStampString(context, when, fullFormat);
        }
        return null;
    }
    
    @Override
    public boolean onIpBootCompleted(Context context) {
        IpMessageServiceMananger.getInstance(context).startIpService();
        return true;
    }

    @Override
    public void onIpDeleteMessage(Context context, Collection<Long> threadIds, int maxSmsId, boolean deleteLockedMessages) {
      deleteIpMessage(context, threadIds, maxSmsId, deleteLockedMessages);     
    }

    @Override
    public boolean onIpMmsCreate(Context context) {
        IpMessageServiceMananger.getInstance(context).startIpService();
        return true;
    }

    public static void blockingIpUpdateNewMessageIndicator(Context context, long newMsgThreadId,
            boolean isStatusMessage, Uri statusMessageUri) {
        mCallback.blockingIpUpdateNewMessageIndicator(context, newMsgThreadId, isStatusMessage,
                statusMessageUri);
    }

    public static void notifyIpWidgetDatasetChanged(Context context) {
        mCallback.notifyIpWidgetDatasetChanged(context);
    }

    public static boolean isIpHome(Context context) {
        return mCallback.isIpHome(context);
    }

    public static int getActionTypeByAction(String action) {
        if (TextUtils.isEmpty(action)) {
            return IPMSG_ERROR_ACTION;
        }
        if (action.equals(NewMessageAction.ACTION_NEW_MESSAGE)) {
            return IPMSG_NEW_MESSAGE_ACTION;
        }
        if (action.equals(RefreshContactList.ACTION_REFRESH_CONTACTS_LIST)) {
            return IPMSG_REFRESH_CONTACT_LIST_ACTION;
        }
        if (action.equals(RefreshGroupList.ACTION_REFRESH_GROUP_LIST)) {
            return IPMSG_REFRESH_GROUP_LIST_ACTION;
        }
        if (action.equals(ServiceStatus.ACTION_SERVICE_STATUS)) {
            return IPMSG_SERCIVE_STATUS_ACTION;
        }
        if (action.equals(ImStatus.ACTION_IM_STATUS)) {
            return IPMSG_IM_STATUS_ACTION;
        }
        if (action.equals(IpMessageStatus.ACTION_MESSAGE_STATUS)) {
            return IPMSG_IP_MESSAGE_STATUS_ACTION;
        }
        if (action.equals(DownloadAttachStatus.ACTION_DOWNLOAD_ATTACH_STATUS)) {
            return IPMSG_DOWNLOAD_ATTACH_STATUS_ACTION;
        }
        if (action.equals(SetProfileResult.ACTION_SET_PROFILE_RESULT)) {
            return IPMSG_SET_PROFILE_RESULT_ACTION;
        }
        if (action.equals(BackupMsgStatus.ACTION_BACKUP_MSG_STATUS)) {
            return IPMSG_BACKUP_MSG_STATUS_ACTION;
        }
        if (action.equals(RestoreMsgStatus.ACTION_RESTORE_MSG_STATUS)) {
            return IPMSG_RESTORE_MSG_STATUS_ACTION;
        }
        if (action.equals(IpMessageConsts.UpdateGroup.UPDATE_GROUP_ACTION)) {
            return IPMSG_UPDATE_GROUP_INFO;
        }
        if (action.equals(IpMessageConsts.ContactStatus.CONTACT_UPDATE)) {
            return IPMSG_IPMESSAGE_CONTACT_UPDATE;
        }
        if (action.equals(IpMessageConsts.SimInfoChanged.SIM_INFO_ACTION)) {
            return IPMSG_SIM_INFO_ACTION;
        }
        if (action.equals(IpMessageConsts.DisableServiceStatus.ACTION_DISABLE_SERVICE_STATUS)) {
            return IPMSG_DISABLE_SERVICE_STATUS_ACTION;
        }
        /// M: add for ipmessage @{
        if (action.equals(IpMessageConsts.RegStatus.ACTION_REG_STATUS)) {
            return IPMSG_REG_STATUS_ACTION;
        }
        if (action.equals(IpMessageConsts.IpMessageStatus.IP_MESSAGE_FALLBACK_MMS)) {
            return IPMSG_FALLBACK_MMS;
        }
        /// @}

        Logger.w(TAG, "getActionTypeByAction(): Unknown ipmessage action.");
        return IPMSG_ERROR_ACTION;
    }

    public static void addIpMsgNotificationListeners(Context context, INotificationsListener notiListener) {
        IpNotificationsManager.getInstance(context).registerNotificationsListener(notiListener);
    }

    public static void removeIpMsgNotificationListeners(Context context, INotificationsListener notiListener) {
        IpNotificationsManager.getInstance(context).unregisterNotificationsListener(notiListener);
    }

    public static String getThreadListString(Collection<Long> threads) {
        StringBuilder threadList = new StringBuilder();
        for (long thread : threads) {
            threadList.append(thread);
            threadList.append(",");
        }
        Logger.d(TAG, "threadList:" + threadList);
        return threadList.substring(0, threadList.length() - 1);
    }

    public static void deleteIpMessage(Context ct, Collection<Long> threads, int maxSmsId, boolean deleteLockedMessages) {
    	Logger.d(TAG, "deleteIpMessage:" + threads + "maxSmsId:" + maxSmsId);
        long[] ids;
        String threadList;
        String selection = "";
        if(deleteLockedMessages)
            selection = MtkTelephony.MtkSms.IPMSG_ID + " > 0 AND " + Sms._ID + " <= " + maxSmsId;
        else
            selection = MtkTelephony.MtkSms.IPMSG_ID + " > 0 AND " + Sms._ID + " <= " + maxSmsId + " AND " + Sms.LOCKED + " = 0";
        if (threads != null) {
            if (threads.size() < 1) {
                Logger.w(TAG, "threads list is empty!");
                return;
            }
            threadList = getThreadListString(threads);
            Logger.d(TAG, "threadList:" + threadList);
            selection += " AND " + Sms.THREAD_ID + " IN (" + threadList + ")";
        }
        Cursor cursor = null;
        cursor = SqliteWrapper.query(ct, ct.getContentResolver(),
                Sms.CONTENT_URI, new String[]{Sms._ID}, selection, null, null);
        if (cursor != null) {
            ids = new long[cursor.getCount()];
            int i = 0;
            while (cursor.moveToNext()) {
                ids[i++] = cursor.getLong(0);
                Logger.d(TAG, "id" + (i - 1) + ":" + ids[i - 1]);
            }
            cursor.close();
        } else {
            Logger.w(TAG, "delete ipmessage query get cursor null!");
            return;
        }
        IpMessageManager.getInstance(ct).deleteIpMsg(ids, false);
    }

    public static boolean isValidAttach(String path, boolean inspectSize) {
        if (!isExistsFile(path) || getFileSize(path) == 0) {
            Logger.e(TAG, "isValidAttach: file is not exist, or size is 0");
            return false;
        }
        return true;
    }

    public static void createLoseSDCardNotice(Context context, String content) {
        new AlertDialog.Builder(context)
                .setTitle(
                        IpMessageResourceMananger.getInstance(context).getSingleString(
                                IpMessageConsts.string.ipmsg_no_sdcard))
                .setMessage(content)
                .setPositiveButton(
                        IpMessageResourceMananger.getInstance(context).getSingleString(
                                IpMessageConsts.string.ipmsg_cancel), null)
                .create().show();

    }

    public static String formatFileSize(int size) {
        String result = "";
        int oneMb = 1024 * 1024;
        int oneKb = 1024;
        if (size > oneMb) {
            int s = size % oneMb / 100;
            if (s == 0) {
                result = size / oneMb + "MB";
            } else {
                result = size / oneMb + "." + s + "MB";
            }
        } else if (size > oneKb) {
            int s = size % oneKb / 100;
            if (s == 0) {
                result = size / oneKb + "KB";
            } else {
                result = size / oneKb + "." + s + "KB";
            }
        } else if (size > 0) {
            result = size + "B";
        } else {
            result = "invalid size";
        }
        return result;
    }

    public static String formatAudioTime(int duration) {
        String result = "";
        if (duration > 60) {
            if (duration % 60 == 0) {
                result = duration / 60 + "'";
            } else {
                result = duration / 60 + "'" + duration % 60 + "\"";
            }
        } else if (duration > 0) {
            result = duration + "\"";
        } else {
            result = "no duration";
        }
        return result;
    }

    public static String getShortTimeString(Context context, long time) {
        int formatFlags = DateUtils.FORMAT_NO_NOON_MIDNIGHT
                | DateUtils.FORMAT_CAP_AMPM;
        formatFlags |= DateUtils.FORMAT_SHOW_TIME;
        return DateUtils.formatDateTime(context, time, formatFlags);

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

    private static String getDefaultFM(Context context, boolean bThisYear) {
        String fm;
        char[] order = DateFormat.getDateFormatOrder(context);
        if (order != null) {
            if (bThisYear) {
                if (order[0] == 'y' || order[0] == 'Y') {
                    fm = "" + order[1] + order[1] + "/" + order[2] + order[2];
                } else {
                    fm = "" + order[0] + order[0] + "/" + order[1] + order[1];
                }
            } else {
                fm = "" + order[0] + order[0] + "/" + order[1] + order[1] + "/" + order[2]
                        + order[2];
            }
        } else {
            fm = "MM/DD";
        }
        return fm;
    }


    public static boolean isFileStatusOk(Context context, String path) {
        if (TextUtils.isEmpty(path)) {
            Toast.makeText(
                    context,
                    IpMessageResourceMananger.getInstance(context)
                .getSingleString(IpMessageConsts.string.ipmsg_no_such_file), Toast.LENGTH_SHORT).show();
            return false;
        }
        if (!isExistsFile(path)) {
            Toast.makeText(
                    context,
                    IpMessageResourceMananger.getInstance(context)
                .getSingleString(IpMessageConsts.string.ipmsg_no_such_file), Toast.LENGTH_SHORT).show();
            return false;
        }
        /*if (getFileSize(path) > (2 * 1024 * 1024)) {
            Toast.makeText(
                    context,
                    IpMessageResourceMananger.getInstance(context)
                .getSingleString(IpMessageConsts.string.ipmsg_over_file_limit), Toast.LENGTH_SHORT).show();
            return false;
        }*/
        return true;
    }

    public static boolean isPic(String name) {
        if (TextUtils.isEmpty(name)) {
            return false;
        }
        String path = name.toLowerCase();
        if (path.endsWith(".png") || path.endsWith(".jpg") || path.endsWith(".jpeg")
                || path.endsWith(".bmp") || path.endsWith(".gif")) {
            return true;
        } else {
            return false;
        }
    }

    public static boolean isVideo(String name) {
        Logger.d(TAG, "isVideo(): name = " + name);
        if (TextUtils.isEmpty(name)) {
            return false;
        }
        String path = name.toLowerCase();
        if (path.endsWith(".mp4") || path.endsWith(".3gp")) {
            return true;
        }
        // Support all types of video files
        return true;
    }

    public static boolean isAudio(String name) {
        if (TextUtils.isEmpty(name)) {
            return false;
        }
        ///M: add 3gpp audio file{@
        String extArrayString[] = {".amr", ".ogg", ".mp3", ".aac", ".ape", ".flac", ".wma", ".wav", ".mp2", ".mid", ".3gpp"};
        ///@}
        String path = name.toLowerCase();
        for (String ext : extArrayString) {
            if (path.endsWith(ext)) {
                return true;
            }
        }
        return false;
    }

    public static IpMessage readIpMessageDraft(Context context, long threadId) {
        try {
        Logger.d(TAG, "readIpMessageDraft(): threadId = " + threadId);
        // If it's an invalid thread or we know there's no draft, don't bother.
        if (threadId <= 0) {
            Logger.d(TAG, "readDraftIpMessage(): no draft, threadId = " + threadId);
            return null;
        }

        Uri threadUri = ContentUris.withAppendedId(Sms.Conversations.CONTENT_URI, threadId);
        String body = "";

        Cursor c = SqliteWrapper.query(context, context.getContentResolver(), threadUri,
            SMS_BODY_PROJECTION_WITH_IPMSG_ID, SMS_DRAFT_WHERE, null, null);
        long msgId = 0L;
        long ipMsgId = 0L;
        if (c != null) {
            try {
                if (c.moveToFirst()) {
                    ipMsgId = c.getLong(SMS_IPMSG_ID_INDEX);
                    if (ipMsgId > 0) {
                        msgId = c.getLong(SMS_ID_INDEX);
                    }
                }
            } finally {
                c.close();
            }
        }

        if (msgId > 0 && ipMsgId > 0) {
            IpMessage ipMessage = IpMessageManager.getInstance(context).getIpMsgInfo(msgId);
            /// M: a draft sms must be deleted after loaded. a new record will created when save.
            // rcse Todo
            // workingMessage.asyncDeleteDraftSmsMessage(conv);
            //getChatManager(context).deleteDraftMessageInThread(conv.getThreadId());
            if (ipMessage != null) {
                ipMessage.setStatus(IpMessageStatus.OUTBOX);
                // rcse Todo
                // workingMessage.clearConversation(conv, true);
                Logger.d(TAG, "readIpMessageDraft(): Get IP message draft, msgId = " + msgId);
                return ipMessage;
            }
        }
        Logger.d(TAG, "readIpMessageDraft(): No IP message draft, msgId = " + msgId);
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        return null;
    }

    public static boolean deleteIpMessageDraft(Context context, long threadId) {
        Logger.d(TAG, "deleteIpMessageDraft(): threadId = " + threadId);

        // If it's an invalid thread or we know there's no draft, don't bother.
        if (threadId <= 0) {
            Logger.d(TAG, "deleteIpMessageDraft(): no draft, threadId = " + threadId);
            return false;
        }

        Uri threadUri = ContentUris.withAppendedId(Sms.Conversations.CONTENT_URI, threadId);
        Cursor c = SqliteWrapper.query(context, context.getContentResolver(), threadUri, SMS_BODY_PROJECTION_WITH_IPMSG_ID,
            SMS_DRAFT_WHERE, null, null);
        long msgId = 0L;
        long ipMsgId = 0L;
        if (c != null) {
            try {
                if (c.moveToFirst()) {
                    ipMsgId = c.getLong(SMS_IPMSG_ID_INDEX);
                    if (ipMsgId > 0) {
                        msgId = c.getLong(SMS_ID_INDEX);
                    }
                }
            } finally {
                c.close();
            }
        }

        if (msgId > 0) {
            if (ipMsgId > 0) {
                IpMessageManager.getInstance(context).deleteIpMsg(new long[] {
                    msgId
                }, true);
                Logger.d(TAG, "deleteIpMessageDraft(): Delete IP message draft, msgId = " + msgId);
            }
            /* rcse Todo
             * else { workingMessage.asyncDeleteDraftSmsMessage(conv); }
             */
            return true;
        }
        Logger.d(TAG, "deleteIpMessageDraft(): No IP message draft, msgId = " + msgId);
        return false;
    }

    public static String getIpMessageCaption(IpMessage ipMessage) {
        Logger.d(TAG, "getIpMessageCaption()");
        String caption = "";
        int type = ipMessage.getType();
        switch (type) {
        case IpMessageType.PICTURE:
            caption = ((IpImageMessage) ipMessage).getCaption();
                Logger.d(TAG, "getIpMessageCaption(): Get pic caption, caption = " + caption);
            break;
        case IpMessageType.VOICE:
            caption = ((IpVoiceMessage) ipMessage).getCaption();
                Logger.d(TAG, "getIpMessageCaption(): Get audio caption, caption = " + caption);
            break;
        case IpMessageType.VIDEO:
            caption = ((IpVideoMessage) ipMessage).getCaption();
                Logger.d(TAG, "getIpMessageCaption(): Get video caption, caption = " + caption);
            break;
        case IpMessageType.TEXT:
        case IpMessageType.VCARD:
        case IpMessageType.CALENDAR:
        case IpMessageType.GROUP_CREATE_CFG:
        case IpMessageType.GROUP_ADD_CFG:
        case IpMessageType.GROUP_QUIT_CFG:
        case IpMessageType.UNKNOWN_FILE:
        case IpMessageType.COUNT:
            break;
        default:
            break;
        }
        return caption;
    }

    public static int getIpMessageStatusResourceId(int status) {
        int id = 0;
        if (status == IpMessageStatus.OUTBOX) {
            id = R.drawable.im_meg_status_sending;
        } else if (status == IpMessageStatus.SENT) {
            id = R.drawable.im_meg_status_out;
        } else if (status == IpMessageStatus.DELIVERED) {
            id = R.drawable.im_meg_status_reach;
        } else if (status == IpMessageStatus.FAILED) {
            id = R.drawable.ic_list_alert_sms_failed;
        } else if (status == IpMessageStatus.VIEWED) {
            id = R.drawable.im_meg_status_read;
        } else if (status == IpMessageStatus.NOT_DELIVERED) {
            id = R.drawable.ic_list_alert_sms_failed;
        }

        return id;
    }

    public static final int UNCONSTRAINED = -1;
    public static final String IP_MESSAGE_FILE_PATH = File.separator + "Rcse" + File.separator;
    public static final String CACHE_PATH = File.separator + "Rcse" + "/Cache/";
    public static String getCachePath(Context c) {
        String path = null;
        String sdCardPath = getSDCardPath(c);
        if (!TextUtils.isEmpty(sdCardPath)) {
            path = sdCardPath + CACHE_PATH;
        }
        return path;
    }

    public static String getMemPath(Context c) {
        return c.getFilesDir().getAbsolutePath();
    }

    public static String getSDCardPath(Context c) {
        File sdDir = null;
        String sdStatus = Environment.getExternalStorageState();

        if (TextUtils.isEmpty(sdStatus)) {
            return c.getFilesDir().getAbsolutePath();
        }

        boolean sdCardExist = sdStatus.equals(android.os.Environment.MEDIA_MOUNTED);

        if (sdCardExist) {
            sdDir = c.getExternalFilesDir(null);
            return sdDir.toString();
        }

        return c.getFilesDir().getAbsolutePath();
    }

    public static boolean isPopupenabledForThread(Context context, long threadId) {
        Logger.d(TAG, "isPopupenabledforthread: " + threadId);
        boolean threadNotificationEnabled = true;
        if (threadId > 0) {
            Uri threadSettingsUri = ContentUris.withAppendedId(
                    THREAD_SETTINGS_URI, (int) threadId);
            Cursor c = context
                    .getContentResolver()
                    .query(threadSettingsUri,
                            new String[] { MtkTelephony.MtkThreadSettings.NOTIFICATION_ENABLE },
                            null, null, null);

            if (c == null) {
                Logger.d(TAG, "cursor is null.");
                return threadNotificationEnabled;
            }
            try {
                if (c.getCount() == 0) {
                    Logger.d(TAG, "cursor count is 0");
                } else {
                    c.moveToFirst();
                    threadNotificationEnabled = c.getInt(0) == 0 ? false : true;
                    Logger.d(TAG,
                            "isPopupenabledforthread: threadNotificationEnabled = "
                                    + threadNotificationEnabled);
                }
            } finally {
                if (c != null) {
                    c.close();
                }
            }
        }
        if (threadId > 0 && !threadNotificationEnabled) {
            Logger.d(TAG, "thread notification set disabled!" + threadId);
        }
        return threadNotificationEnabled;
    }

    public static boolean getSDCardStatus() {
        boolean ret = false;
        String sdStatus = Environment.getExternalStorageState();
        Logger.d(TAG, "getSDCardStatus(): sdStatus = " + sdStatus);
        if (sdStatus.equals(Environment.MEDIA_MOUNTED)) {
            ret = true;
        }
        return ret;
    }

    public static long getSDcardAvailableSpace() {
        File path = Environment.getExternalStorageDirectory();
        StatFs stat = new StatFs(path.getPath());
        long blockSize = stat.getBlockSize();
        long availableBlocks = stat.getAvailableBlocks();

        return availableBlocks * blockSize;
    }

    public static int getExifOrientation(String filepath) {
        int degree = 0;
        ExifInterface exif = null;

        try {
            exif = new ExifInterface(filepath);
        } catch (IOException ex) {
            Logger.e(TAG, "getExifOrientation(): IOException");
        }

        if (exif != null) {
            int orientation = exif.getAttributeInt(ExifInterface.TAG_ORIENTATION, -1);
            if (orientation != -1) {
                // We only recognize a subset of orientation tag values.
                switch (orientation) {
                case ExifInterface.ORIENTATION_ROTATE_90:
                    degree = 90;
                    break;

                case ExifInterface.ORIENTATION_ROTATE_180:
                    degree = 180;
                    break;

                case ExifInterface.ORIENTATION_ROTATE_270:
                    degree = 270;
                    break;
                default:
                    break;
                }
            }
        }

        return degree;
    }

    public static Bitmap rotate(Bitmap b, int degrees) {
        if (degrees != 0 && b != null) {
            Matrix m = new Matrix();
            m.setRotate(degrees, (float) b.getWidth() / 2, (float) b.getHeight() / 2);
            try {
                Bitmap b2 = Bitmap.createBitmap(b, 0, 0, b.getWidth(), b.getHeight(), m, true);
                if (b != b2) {
                    b.recycle();
                    b = b2;
                }
            } catch (OutOfMemoryError ex) {
                // We have no memory to rotate. Return the original bitmap.
                Logger.w(TAG, "OutOfMemoryError.");
            }
        }

        return b;
    }

    public static Bitmap resizeImage(Bitmap bitmap, int w, int h, boolean needRecycle) {
        if (null == bitmap) {
            return null;
        }

        Bitmap bitmapOrg = bitmap;
        int width = bitmapOrg.getWidth();
        int height = bitmapOrg.getHeight();
        int newWidth = w;
        int newHeight = h;

        float scaleWidth = ((float) newWidth) / width;
        float scaleHeight = ((float) newHeight) / height;
        Matrix matrix = new Matrix();
        matrix.postScale(scaleWidth, scaleHeight);

        Bitmap resizedBitmap = Bitmap.createBitmap(bitmapOrg, 0, 0, width, height, matrix, true);
        if (needRecycle && !bitmapOrg.isRecycled() && bitmapOrg != resizedBitmap) {
            bitmapOrg.recycle();
        }
        return resizedBitmap;
    }

    public static byte[] resizeImg(String path, float maxLength) {
        int d = getExifOrientation(path);
        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inJustDecodeBounds = true;
        Bitmap bitmap = BitmapFactory.decodeFile(path, options);
        options.inJustDecodeBounds = false;

        int l = Math.max(options.outHeight, options.outWidth);
        int be = (int) (l / maxLength);
        if (be <= 0) {
            be = 1;
        }
        options.inSampleSize = be;

        bitmap = BitmapFactory.decodeFile(path, options);
        if (null == bitmap) {
            return null;
        }
        if (d != 0) {
            bitmap = rotate(bitmap, d);
        }

        String[] tempStrArry = path.split("\\.");
        String filePostfix = tempStrArry[tempStrArry.length - 1];
        CompressFormat formatType = null;
        if (filePostfix.equalsIgnoreCase("PNG")) {
            formatType = Bitmap.CompressFormat.PNG;
        } else if (filePostfix.equalsIgnoreCase("JPG") || filePostfix.equalsIgnoreCase("JPEG")) {
            formatType = Bitmap.CompressFormat.JPEG;
        } else if (filePostfix.equalsIgnoreCase("GIF")) {
            formatType = Bitmap.CompressFormat.PNG;
        } else if (filePostfix.equalsIgnoreCase("BMP")) {
            formatType = Bitmap.CompressFormat.PNG;
        } else {
            Logger.e(TAG, "resizeImg(): Can't compress the image,because can't support the format:"
                    + filePostfix);
            return null;
        }

        int quality = 100;
        if (be == 1) {
            if (getFileSize(path) > 50 * 1024) {
                quality = 30;
            }
        } else {
            quality = 30;
        }

        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        bitmap.compress(formatType, quality, baos);
        final byte[] tempArry = baos.toByteArray();
        if (baos != null) {
            try {
                baos.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
            baos = null;
        }

        return tempArry;
    }

    public static boolean isExistsFile(String filepath) {
        try {
            if (TextUtils.isEmpty(filepath)) {
                return false;
            }
            File file = new File(filepath);
            return file.exists();
        } catch (Exception e) {
            Logger.e(TAG, "isExistsFile(): Exception");
            return false;
        }
    }

    public static int getFileSize(String filepath) {
        try {
            if (TextUtils.isEmpty(filepath)) {
                return -1;
            }
            File file = new File(filepath);
            return (int) file.length();
        } catch (Exception e) {
            Logger.e(TAG, "getFileSize(): Exception");
            return -1;
        }
    }

    public static void copy(String src, String dest) {
        InputStream is = null;
        OutputStream os = null;

        File out = new File(dest);
        if (!out.getParentFile().exists()) {
            out.getParentFile().mkdirs();
        }

        try {
            is = new BufferedInputStream(new FileInputStream(src));
            os = new BufferedOutputStream(new FileOutputStream(dest));

            byte[] b = new byte[256];
            int len = 0;
            try {
                while ((len = is.read(b)) != -1) {
                    os.write(b, 0, len);

                }
                os.flush();
            } catch (IOException e) {
                Logger.e(TAG, "IOException");
            } finally {
                if (is != null) {
                    try {
                        is.close();
                    } catch (IOException e) {
                        Logger.e(TAG, "IOException");
                    }
                }
            }
        } catch (FileNotFoundException e) {
            Logger.e(TAG, "FileNotFoundException");
        } finally {
            if (os != null) {
                try {
                    os.close();
                } catch (IOException e) {
                    Logger.e(TAG, "IOException");
                }
            }
        }
    }

    /**
     *
     *
     * @param stream
     * @return
     */
    public static void nmsStream2File(byte[] stream, String filepath) throws Exception {
        FileOutputStream outStream = null;
        try {
            File f = new File(filepath);
            if (!f.getParentFile().exists()) {
                f.getParentFile().mkdirs();
            }
            if (f.exists()) {
                f.delete();
            }
            f.createNewFile();
            outStream = new FileOutputStream(f);
            outStream.write(stream);
            outStream.flush();
        } catch (IOException e) {
            Logger.e(TAG, "nmsStream2File(): IOException");
            throw new RuntimeException(e.getMessage());
        } finally {
            if (outStream != null) {
                try {
                    outStream.close();
                    outStream = null;
                } catch (IOException e) {
                    Logger.e(TAG, "nmsStream2File(): IOException");
                    throw new RuntimeException(e.getMessage());
                }
            }
        }
    }

    private static Rect getScreenRegion(int width, int height) {
        return new Rect(0, 0, width, height);
    }

    /**
     * get inSampleSize.
     *
     * @param options
     * @param minSideLength
     * @param maxNumOfPixels
     * @return
     */
    public static int computeSampleSize(BitmapFactory.Options options, int minSideLength,
            int maxNumOfPixels) {
        int initialSize = computeInitialSampleSize(options, minSideLength, maxNumOfPixels);

        int roundedSize;
        if (initialSize <= 8) {
            roundedSize = 1;
            while (roundedSize < initialSize) {
                roundedSize <<= 1;
            }
        } else {
            roundedSize = (initialSize + 7) / 8 * 8;
        }

        return roundedSize;
    }

    private static int computeInitialSampleSize(BitmapFactory.Options options, int minSideLength,
            int maxNumOfPixels) {
        double w = options.outWidth;
        double h = options.outHeight;

        int lowerBound = (maxNumOfPixels == UNCONSTRAINED) ? 1 : (int) Math.ceil(Math.sqrt(w * h
                / maxNumOfPixels));
        int upperBound = (minSideLength == UNCONSTRAINED) ? 128 : (int) Math.min(
                Math.floor(w / minSideLength), Math.floor(h / minSideLength));

        if (upperBound < lowerBound) {
            return lowerBound;
        }

        if ((maxNumOfPixels == UNCONSTRAINED) && (minSideLength == UNCONSTRAINED)) {
            return 1;
        } else if (minSideLength == UNCONSTRAINED) {
            return lowerBound;
        } else {
            return upperBound;
        }
    }

    /**
     * Get bitmap
     *
     * @param path
     * @param options
     * @return
     */
    public static Bitmap getBitmapByPath(String path, Options options, int width, int height) {
        if (TextUtils.isEmpty(path) || width <= 0 || height <= 0) {
            Logger.w(TAG, "parm is error.");
            return null;
        }

        File file = new File(path);
        if (!file.exists()) {
            Logger.w(TAG, "file not exist!");
            return null;
        }
        FileInputStream in = null;
        try {
            in = new FileInputStream(file);
        } catch (FileNotFoundException e) {
            Logger.e(TAG, "FileNotFoundException:" + e.toString());
        }
        if (options != null) {
            Rect r = getScreenRegion(width, height);
            int w = r.width();
            int h = r.height();
            int maxSize = w > h ? w : h;
            int inSimpleSize = computeSampleSize(options, maxSize, w * h);
            options.inSampleSize = inSimpleSize;
            options.inJustDecodeBounds = false;
        }
        Bitmap bm = null;
        try {
            bm = BitmapFactory.decodeStream(in, null, options);
        } catch (java.lang.OutOfMemoryError e) {
            Logger.e(TAG, "bitmap decode failed, catch outmemery error");
        }
        try {
            in.close();
        } catch (IOException e) {
            Logger.e(TAG, "IOException:" + e.toString());
        }
        return bm;
    }

    public static Options getOptions(String path) {
        Options options = new Options();
        options.inJustDecodeBounds = true;
        BitmapFactory.decodeFile(path, options);
        return options;
    }

    public static long getAvailableBytesInFileSystemAtGivenRoot(String rootFilePath) {
        StatFs stat = new StatFs(rootFilePath);
        final long availableBlocks = stat.getAvailableBlocks() - 128;
        long mAvailSize = availableBlocks * stat.getBlockSize();
        Log.i(TAG, "getAvailableBytesInFileSystemAtGivenRoot(): "
                + "available space (in bytes) in filesystem rooted at: " + rootFilePath + " is: "
                + mAvailSize);
        return mAvailSize;
    }

    public static String formatNumber(String number, Context context) {
        String countryCode = detectCountry(context);
        AsYouTypeFormatter mFormatter = PhoneNumberUtil.getInstance().getAsYouTypeFormatter(countryCode);
        char [] cha = number.toCharArray();
        int ii = cha.length;
        for (int num = 0; num < ii; num++) {
            number = mFormatter.inputDigit(cha[num]);
        }
        return number;
    }

    public static int getVideoCaptureDurationLimit() {
        CamcorderProfile camcorder = CamcorderProfile.get(CamcorderProfile.QUALITY_LOW);
        return camcorder == null ? 0 : camcorder.duration;
    }

    public static void selectRingtone(Context context, int requestCode) {
        if (context instanceof Activity) {
            Intent intent = new Intent(RingtoneManager.ACTION_RINGTONE_PICKER);
            intent.putExtra(RingtoneManager.EXTRA_RINGTONE_SHOW_DEFAULT, false);
            intent.putExtra(RingtoneManager.EXTRA_RINGTONE_SHOW_SILENT, false);
            intent.putExtra(RingtoneManager.EXTRA_RINGTONE_INCLUDE_DRM, false);
            intent.putExtra(RingtoneManager.EXTRA_RINGTONE_TITLE,
                    "Select Audio");
            ((Activity) context).startActivityForResult(intent, requestCode);
        }
    }

    public static void selectAudio(Activity activity, int requestCode) {
        Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
        intent.addCategory(Intent.CATEGORY_DEFAULT);
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        intent.setType(ContentType.AUDIO_UNSPECIFIED);
        String[] mimeTypess = new String[] {
                ContentType.AUDIO_UNSPECIFIED, ContentType.AUDIO_OGG, "application/x-ogg"
        };
        intent.putExtra(Intent.EXTRA_MIME_TYPES, mimeTypess);
        activity.startActivityForResult(intent, requestCode);
    }

    public static String detectCountry(Context context) {
        try {
            CountryDetector detector = (CountryDetector) context
                    .getSystemService(Context.COUNTRY_DETECTOR);
            final Country country = detector.detectCountry();
            if (country != null) {
                return country.getCountryIso();
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        return null;
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
        return DateUtils.formatDateTime(context, when, format_flags);
    }
}
