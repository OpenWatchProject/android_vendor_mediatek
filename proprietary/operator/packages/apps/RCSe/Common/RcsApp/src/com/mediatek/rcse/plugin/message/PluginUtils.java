/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
package com.mediatek.rcse.plugin.message;

import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.res.Resources;
import android.database.Cursor;
import android.database.SQLException;
import android.media.MediaFile;
import android.net.Uri;
import android.os.Environment;
import android.os.ParcelUuid;
import android.provider.MediaStore;
import android.provider.Telephony.Sms;
import android.text.TextUtils;
import android.util.LruCache;
import android.webkit.MimeTypeMap;

import com.mediatek.rcse.plugin.message.IpMessageConsts;
import com.mediatek.rcse.activities.RcsContact;
import com.mediatek.rcse.activities.widgets.AsyncGalleryView;
import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.interfaces.ChatController;
import com.mediatek.rcse.plugin.message.PluginChatWindowManager.WindowTagGetter;
import com.mediatek.rcse.provider.UnregMessageProvider;
import com.mediatek.rcse.service.CoreApplication;
import com.mediatek.rcse.service.PluginApiManager;
import com.mediatek.rcse.service.Utils;
import com.mediatek.rcse.service.binder.FileStructForBinder;
import com.mediatek.rcse.service.binder.ThreadTranslater;
import android.telephony.SubscriptionManager;
//import android.telephony.SubInfoRecord;
//import com.mediatek.telephony.SimInfoManager;
//import com.mediatek.telephony.SimInfoManager.SimInfoRecord;

import com.mediatek.rcs.R;
import com.orangelabs.rcs.core.ims.service.im.chat.ChatUtils;
import com.mediatek.rcse.service.MediatekFactory;
import com.mediatek.rcse.settings.RcsSettings;
import com.gsma.services.rcs.ft.FileTransferLog;
import mediatek.telephony.MtkTelephony.MtkSms;
//import com.orangelabs.rcs.utils.PhoneUtils;


import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.util.ArrayList;
import java.util.List;
import java.util.TreeSet;
import java.util.UUID;
import java.util.Map.Entry;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

import com.gsma.services.rcs.ft.FileTransferLog;
import com.gsma.services.rcs.JoynServiceConfiguration;

/**
 * Plugin Utils.
 */
public class PluginUtils {
    /**
     * The Constant TAG.
     */
    private static final String TAG = "PluginUtils";
    /**
     * The Constant IP_GROUP_INVITATION.
     */
    public static final String IP_GROUP_INVITATION =
            "com.mediatek.plugin.groupinvitation";
    /**
     * The Constant ACTION_FILE_SEND.
     */
    public static final String ACTION_FILE_SEND =
            "com.mediatek.mms.ipmessage.fileTransfer";
    /**
     * The Constant ACTION_MODE_CHANGE.
     */
    public static final String ACTION_MODE_CHANGE =
            "com.mediatek.mms.ipmessage.modeChange";
    /**
     * The Constant ACTION_DB_CHANGE.
     */
    public static final String ACTION_DB_CHANGE =
            "com.mediatek.mms.ipmessage.dbChange";
    /**
     * The Constant ACTION_DB_CHANGE_RELOAD.
     */
    public static final String ACTION_DB_CHANGE_RELOAD =
            "com.mediatek.mms.ipmessage.dbChange.reload";
    /**
     * The Constant ACTION_REALOD_FAILED.
     */
    public static final String ACTION_REALOD_FAILED =
            "com.mediatek.mms.ipmessage.reloadFailed";
    /**
     * The Constant ACTION_SEND_URL.
     */
    public static final String ACTION_SEND_URL =
            "com.mediatek.mms.ipmessage.sendUrl";
    /**
     * The Constant ACTION_SEND_RESIZED_FILE.
     */
    public static final String ACTION_SEND_RESIZED_FILE =
            "com.mediatek.mms.ipmessage.sendResized";

    /**
     * The Constant ACTION_SEND_CANCELLED_COMPRESSED_VIDEO.
     */
    public static final String ACTION_SEND_CANCELLED_COMPRESSED_VIDEO =
            "com.mediatek.mms.ipmessage.sendCancelledVideo";
    /**
     * The Constant RCS_CONTENT_URI.
     */
    public static final Uri RCS_CONTENT_URI = Uri
            .parse("content://com.orangelabs.rcs.chat/message");
    /**
     * The Constant RCS_FT_CONTENT_URI.
     */
    public static final Uri RCS_FT_CONTENT_URI = Uri
            .parse("content://com.orangelabs.rcs.filetransfer/filetransfer");
    /**
     * The Constant TYPE.
     */
    private static final String TYPE = "type";
    /**
     * The Constant KEY_EVENT_ROW_ID.
     */
    public static final String KEY_EVENT_ROW_ID = "_id";
    /**
     * The Constant KEY_EVENT_MESSAGE_ID.
     */
    public static final String KEY_EVENT_MESSAGE_ID = "msg_id";
    /**
     * The Constant OUTBOX_URI.
     */
    private static final String OUTBOX_URI = "content://sms/outbox";
    /**
     * The Constant INBOX_URI.
     */
    private static final String INBOX_URI = "content://sms/inbox";
    /**
     * The Constant CONTENT_URI.
     */
    private static final String CONTENT_URI = "content://sms/";
    /**
     * The Constant OUTBOX_URI.
     */
    private static final String OUTBOX_OUTPUT_URI = "content://sms/outbox/";
    /**
     * The Constant INBOX__URI.
     */
    private static final String INBOX_OUTPUT_URI = "content://sms/inbox/";
    /**
     * The Constant FILE_SCHEMA.
     */
    private static final String FILE_SCHEMA = "file://";
    /**
     * The Constant PROJECTION_WITH_THREAD.
     */
    private static final String[] PROJECTION_WITH_THREAD = { KEY_EVENT_ROW_ID,
            Sms.THREAD_ID, Sms.ADDRESS, Sms.BODY };
    /**
     * The Constant PROJECTION_ONLY_ID.
     */
    private static final String[] PROJECTION_ONLY_ID = { KEY_EVENT_ROW_ID };
    /**
     * The Constant PROJECTION_MESSAGE_ID.
     */
    private static final String[] PROJECTION_MESSAGE_ID = { KEY_EVENT_MESSAGE_ID };
    /**
     * The Constant PROJECTION_FILETRANSFER_ID.
     */
    private static final String[] PROJECTION_FILETRANSFER_ID = { FileTransferLog.FT_ID };
    /**
     * The Constant SELECTION_RELOAD_IP_MSG.
     */
    private static final String SELECTION_RELOAD_IP_MSG = MtkSms.IPMSG_ID + ">?";
    /**
     * The Constant SELECTION_IP_MSG_ID.
     */
    private static final String SELECTION_IP_MSG_ID = MtkSms.IPMSG_ID + "=?";
    /**
     * The Constant SELECTION_MMS_THREAD_ID.
     */
    private static final String SELECTION_MMS_THREAD_ID = Sms.THREAD_ID + "=?";
    /**
     * The Constant INBOX_MESSAGE.
     */
    public static final int INBOX_MESSAGE = 1;
    /**
     * The Constant OUTBOX_MESSAGE.
     */
    public static final int OUTBOX_MESSAGE = 2;
    /**
     * The Constant DEFAULT_MESSAGE_ID.
     */
    public static final int DEFAULT_MESSAGE_ID = 0x100;
    /**
     * The Constant DUMMY_SIM_ID.
     */
    public static final long DUMMY_SIM_ID = 1L;
    /**
     * The s sim id.
     */
    private static long sSimId = DUMMY_SIM_ID;
    /**
     * The Constant STATUS_IS_READ.
     */
    public static final int STATUS_IS_READ = 1;
    /**
     * The Constant STATUS_IS_NOT_READ.
     */
    public static final int STATUS_IS_NOT_READ = 0;
    /**
     * The Constant SIZE_K.
     */
    public static final int SIZE_K = 1024;
    /**
     * The m messaging ux.
     */
    public static int sMessagingUx = -1;
    /**
     * The m max text limit.
     */
    private static int sMaxTextLimit = -1;

    /**
     * The m max text limit.
     */
    private static int sLocalMaxTextLimit = 2048;
    /**
     * The reloading messages.
     */
    public static boolean sReloadingMessages = false;
    /**
     * The Constant MAX_CACHE_SIZE.
     */
    private static final int MAX_CACHE_SIZE = 200;
    /**
     * The Constant SMS_CONTENT_URI.
     */
    public static final Uri SMS_CONTENT_URI = Uri.parse("content://sms");
    /**
     * The Constant THREAD_SETTINGS.
     */
    public static final Uri THREAD_SETTINGS = Uri
            .parse("content://mms-sms/thread_settings/");
    /**
     * The Constant SUBJECT_CACHE.
     */
    private static final LruCache<String, String> SUBJECT_CACHE =
            new LruCache<String, String>(
            MAX_CACHE_SIZE);
    /**
     * The Constant SEMI_COLON.
     */
    private static final String SEMI_COLON = ";";

    public static final int IPMSG_SENT_MESSAGE = 100;

    public static int sJOYN_SERVICE_STATUS = 0;

    private static int UNSENT_MESSAGE_ID_THREASHOLD  = 24246;

    /**
     * The Constant FAILED_MESSAGE_CACHE.
     */
    public static final LruCache<String, Integer> FAILED_MESSAGE_CACHE =
            new LruCache<String, Integer>(
            MAX_CACHE_SIZE);
    /**
     * The group map.
     */
    public static ConcurrentHashMap<String, ArrayList<Integer>> sGroupMap = null;
    /**
     * The O2 o map.
     */
    public static ConcurrentHashMap<String, ArrayList<Integer>> sO2OMap = null;
    /**
     * The is rc se configured.
     */
    public static boolean sIsRCSeConfigured = false;

    /**
     * Checks if is RC se configured.
     *
     * @return true, if is RC se configured
     */
    public static boolean isRCSeConfigured() {
        return sIsRCSeConfigured;
    }
    /**
     * Sets the RC se configured.
     *
     * @param isRCSeConfigured the new RC se configured
     */
    public static void setRCSeConfigured(boolean isRCSeConfigured) {
        Logger.d(TAG, "setRCSeConfigured " + isRCSeConfigured);
        PluginUtils.sIsRCSeConfigured = isRCSeConfigured;
    }
    /**
     * Save threadand tag.
     *
     * @param failed the failed
     * @param contact the contact
     */
    public static void saveThreadandTag(Integer failed, String contact) {
        Logger.d(TAG, "save failed message() entry failed is " + failed
                + " contact is " + contact);
        synchronized (FAILED_MESSAGE_CACHE) {
            FAILED_MESSAGE_CACHE.put(contact, failed);
        }
    }
    /**
     * Translate thread id.
     *
     * @param contact the contact
     * @return the integer
     */
    public static Integer translateThreadId(String contact) {
        Logger.d(TAG, "getfailed message() entry contact is " + contact);
        synchronized (FAILED_MESSAGE_CACHE) {
            Integer result = FAILED_MESSAGE_CACHE.get(contact);
            Logger.d(TAG, "getfailed() result is " + result);
            if (result == null) {
                return 0;
            }
            return result;
        }
    }
    /**
     * Insert a new item into Mms DB.
     *
     * @param body The body of this item
     * @param contact The address of this item
     * @param ipMsgId The ipmsg_id of this item
     * @param messageType The type of this item
     * @return The id in Mms DB
     */
    public static Long insertDatabase(String body, String contact, int ipMsgId,
            int messageType) {
        return insertDatabase(body, contact, ipMsgId, messageType, 0);
    }
    /**
     * Insert database.
     *
     * @param body the body
     * @param contact the contact
     * @param ipMsgId the ip msg id
     * @param messageType the message type
     * @param targetThreadId the target thread id
     * @return the long
     */
    public static Long insertDatabase(String body, String contact, int ipMsgId,
            int messageType, long targetThreadId) {
        Logger.d(TAG, "InsertDatabase(), body = " + body + "contact is "
                + contact + " , messageType: " + messageType + " , threadId: "
                + targetThreadId);
        Long messageIdInMms = Long.MIN_VALUE;
        String messageUri = null;
        ContentValues cv = new ContentValues();
        if (getMessagingMode() == 0) {
            if (!contact.startsWith(IpMessageConsts.JOYN_START)
                    && !contact
                            .startsWith(PluginGroupChatWindow.GROUP_CONTACT_STRING_BEGINNER) &&
                            !contact.startsWith(PluginMultiChatWindow.MULTI_CONTACT_STRING_BEGINNER)) {
                contact = IpMessageConsts.JOYN_START + contact;
            }
        }
        cv.put(Sms.ADDRESS, contact);
        cv.put(Sms.BODY, body);
        cv.put(MtkSms.IPMSG_ID, ipMsgId);
        //cv.put(Sms.SIM_ID, sSimId);
        if (targetThreadId > 0) {
            cv.put(Sms.THREAD_ID, targetThreadId);
            ThreadTranslater.saveThreadandTag(targetThreadId, contact);
        }
        ContentResolver contentResolver = MediatekFactory
                .getApplicationContext().getContentResolver();
        if (messageType == INBOX_MESSAGE) {
            cv.put(TYPE, INBOX_MESSAGE);
            Uri inboxUri = Uri.parse(INBOX_URI);
            messageUri = contentResolver.insert(inboxUri, cv).toString();
        } else if (messageType == OUTBOX_MESSAGE) {
            cv.put(TYPE, OUTBOX_MESSAGE);
            Uri outboxUri = Uri.parse(OUTBOX_URI);
            messageUri = contentResolver.insert(outboxUri, cv).toString();
        }
        if (null == messageUri) {
            Logger.w(TAG, "InsertDatabase() messageUri is null");
            return -1L;
        }
        if (!ThreadTranslater.tagExistInCache(contact)) {
            cacheThreadIdForGroupChat(contact, contentResolver);
        } else {
            Logger.d(TAG,
                    "InsertDatabase() contact not start group identify or not cached");
        }
        String messageId = "";
        if (messageUri.contains(INBOX_OUTPUT_URI)) {
            Logger.d(TAG, "InsertDatabase(), inbox messageUri = " + messageUri);
            messageId = messageUri.replace(INBOX_OUTPUT_URI, "");
        } else if (messageUri.contains(OUTBOX_OUTPUT_URI)) {
            Logger.d(TAG, "InsertDatabase(), outbox messageUri = " + messageUri);
            messageId = messageUri.replace(OUTBOX_OUTPUT_URI, "");
        } else {
            Logger.d(TAG, "InsertDatabase(), else messageUri = " + messageUri);
            messageId = messageUri.replace(CONTENT_URI, "");
        }
        Logger.d(TAG, "InsertDatabase(), messageId = " + messageId);
        messageIdInMms = Long.valueOf(messageId);
        Logger.d(TAG, "InsertDatabase(), messageIdInMms = " + messageIdInMms);
        return messageIdInMms;
    }
    /**
     * Save tagand subject.
     *
     * @param tag the tag
     * @param subject the subject
     */
    public static void saveTagandSubject(String tag, String subject) {
        Logger.d(TAG, "saveThreadandSubject() entry tag is " + tag
                + " subject is " + subject);
        SUBJECT_CACHE.put(tag, subject);
    }
    /**
     * Translate tag.
     *
     * @param tag the tag
     * @return the string
     */
    public static String translateTag(String tag) {
        Logger.d(TAG, "translateThreadId() entry threadId is " + tag);
        String result = SUBJECT_CACHE.get(tag);
        Logger.d(TAG, "translateThreadId() result is " + result);
        return result;
    }
    /**
     * Removes the messages from mms database.
     */
    public static void removeMessagesFromMMSDatabase() {
        Logger.d(TAG, "removeMessagesFromdatabase(),");
        ContentResolver contentResolver = MediatekFactory
                .getApplicationContext().getContentResolver();
        contentResolver.delete(Uri.parse("content://sms/"), "ipmsg_id>0", null);
    }
    /**
     * Cache thread id for group chat.
     *
     * @param contact the contact
     * @param contentResolver the content resolver
     * @return true, if successful
     */
    private static boolean cacheThreadIdForGroupChat(String contact,
            ContentResolver contentResolver) {
        Cursor cursor = null;
        try {
            String[] args = { contact };
            String[] projection = { Sms.THREAD_ID };
            cursor = contentResolver.query(SMS_CONTENT_URI, projection,
                    Sms.ADDRESS + "=?", args, null);
            if (cursor.moveToFirst()) {
                Long threadId = cursor.getLong(cursor
                        .getColumnIndex(Sms.THREAD_ID));
                Logger.d(TAG, "InsertDatabase() the contact is " + contact
                        + ", threadId is " + +threadId);
                String tag = ThreadTranslater.translateThreadId(threadId);
                if (tag == null || tag.equals("")) {
                    ThreadTranslater.saveThreadandTag(threadId, contact);
                    if (!contact
                            .startsWith(PluginGroupChatWindow.GROUP_CONTACT_STRING_BEGINNER) &&
                            !contact
                            .startsWith(PluginMultiChatWindow.MULTI_CONTACT_STRING_BEGINNER)) {
                        insertThreadIDInDB(threadId, getNameByNumber(contact));
                    }
                    return true;
                } else {
                    Logger.d(TAG, "InsertDatabase() the thread to tag exist "
                            + tag);
                }
            } else {
                Logger.e(TAG,
                        "InsertDatabase() the cursor.moveToFirst() is false");
            }
        } finally {
            if (null != cursor) {
                cursor.close();
            }
        }
        return false;
    }
    /**
     * Removes the message from mms database.
     *
     * @param id the id
     */
    public static void removeMessageFromMMSDatabase(long id) {
        Logger.d(TAG, "removeMessageFromdatabase() id is ," + id);
        ContentResolver contentResolver = MediatekFactory
                .getApplicationContext().getContentResolver();
        contentResolver.delete(Uri.parse("content://sms/"), "_id" + "=" + id,
                null);
    }
    /**
     * Gets the name by number.
     *
     * @param number the number
     * @return the name by number
     */
    public static String getNameByNumber(String number) {
        Logger.d(TAG, "getNameByNumber() entry number is " + number);
        String name = number;
        if (PluginUtils.getMessagingMode() == 0) {
            if (!number.startsWith(IpMessageConsts.JOYN_START)) {
                return name;
            } else {
                number = number.substring(4);
            }
        }
        if (null != number) {
            if (number
                    .startsWith(PluginGroupChatWindow.GROUP_CONTACT_STRING_BEGINNER) ||
                    number.startsWith(PluginMultiChatWindow.MULTI_CONTACT_STRING_BEGINNER)) {
                WindowTagGetter window = PluginChatWindowManager
                        .findWindowTagIndex(number);
                if (window instanceof PluginGroupChatWindow) {
                    Logger.d(TAG,
                            "getNameByNumber() window is PluginGroupChatWindow ");
                    name = ((PluginGroupChatWindow) window).getDisplayName();
                } else  if (window instanceof PluginMultiChatWindow) {
                    Logger.d(TAG,
                            "getNameByNumber() window is PluginMultiChatWindow ");
                    name = ((PluginMultiChatWindow) window).getDisplayName();
                }
                else {
                    Logger.d(TAG,
                            "getNameByNumber() window is not PluginGroupChatWindow "
                                    + window);
                }
            } else {
                if (ContactsListManager.getInstance() == null) {
                    Logger.d(TAG,
                            "getNameByNumber() ContactsListManager is null ");
                    ContactsListManager.initialize(MediatekFactory
                            .getApplicationContext());
                }
                List<RcsContact> currentList = new ArrayList<RcsContact>(
                        ContactsListManager.getInstance().CONTACTS_LIST);
                name = "";
                if (currentList != null) {
                    for (int i = 0; i < currentList.size(); i++) {
                        RcsContact rcsContactList = currentList.get(i);
                        String sNumber = Utils.formatNumberToInternational(rcsContactList.mNumber);
                        Logger.d(TAG,
                                "getNameByNumber() number formatNumberToInternational"
                                        + sNumber);
                        if (sNumber.equals(number)) {
                            Logger.e(TAG,
                                    "getNameByNumber() number is present in CONTACT_LIST"
                                            + currentList.get(i).mDisplayName);
                            if (name.equals("")) {
                                name = currentList.get(i).mDisplayName;
                            } else {
                                name = name + SEMI_COLON
                                        + currentList.get(i).mDisplayName;
                            }
                        }
                    }
                }
                if (name.equals("")) {
                    name = JoynServiceConfiguration.getAliasName(MediatekFactory
                            .getApplicationContext(),number);
                    if (name != null) {
                        Logger.e(TAG, "getNameByNumber() number is present"
                                + " in Richmessaging URI = " + name);
                        //name = number;
                    } else if (name == null) {
                        name = number;
                    }
                    Logger.e(TAG,
                            "getNameByNumber() number is not present in CONTACT_LIST Alias = "
                                    + name);
                }
                Logger.e(TAG,
                        "getNameByNumber() number not have group beginner");
            }
        } else {
            Logger.e(TAG, "getNameByNumber() number is null");
        }
        Logger.d(TAG, "getNameByNumber() exit name is " + name);
        return name;
    }
    /**
     * Insert thread id in db.
     *
     * @param threadId the thread id
     * @param groupSubject the group subject
     */
    public static void insertThreadIDInDB(long threadId, String groupSubject) {
        Logger.d(TAG, "insertThreadIDInDB() with threadid = " + threadId
                + "subject=" + groupSubject);
        ContentValues values = new ContentValues();
        values.put(IntegratedMessagingData.KEY_INTEGRATED_MODE_THREAD_ID,
                threadId);
        values.put(IntegratedMessagingData.KEY_INTEGRATED_MODE_GROUP_SUBJECT,
                groupSubject);
        try {
            Uri uri = MediatekFactory
                    .getApplicationContext()
                    .getContentResolver()
                    .insert(IntegratedMessagingData.CONTENT_URI_INTEGRATED,
                            values);
        } catch (SQLException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }
    /*
     * Store a message into mms database
     */
    /**
     * Store message in database.
     *
     * @param messageId the message id
     * @param text the text
     * @param remote the remote
     * @param messageType the message type
     * @return the long
     */
    public static Long storeMessageInDatabase(String messageId, String text,
            String remote, int messageType) {
        Logger.d(TAG, "storeMessageInDatabase() with messageId = " + messageId
                + " ,text = " + text + " ,remote = " + remote
                + " ,and messageType = " + messageType);
        int ipMsgId = -1;
        if (text.startsWith("File Transfer")
                || text.startsWith("File transfer")) {
            ipMsgId = findFTIdInRcseDb(messageId);
        } else {
            ipMsgId = findIdInRcseDb(messageId);
        }
        if (-1 < ipMsgId) {
            return storeMessageInMmsDb(ipMsgId, text, remote, messageType, 0);
        } else {
            Logger.w(TAG,
                    "storeMessageInDatabase() message not found in Rcse DB");
            return -1L;
        }
    }
    /**
     * Store message in database.
     *
     * @param messageId the message id
     * @param text the text
     * @param remote the remote
     * @param messageType the message type
     * @param threadId the thread id
     * @return the long
     */
    public static Long storeMessageInDatabase(String messageId, String text,
            String remote, int messageType, long threadId, int messageTag) {
        Logger.d(TAG, "storeMessageInDatabase() with messageId = " + messageId
                + " ,text = " + text + " ,remote = " + remote
                + " ,and messageType = " + messageType);
        int ipMsgId = -1;
        if (text.startsWith("File Transfer")
                || text.startsWith("File transfer")) {
            ipMsgId = findFTIdInRcseDb(messageId);
        } else {
            ipMsgId = findIdInRcseDb(messageId);
        }
        if (-1 < ipMsgId) {
            return storeMessageInMmsDb(ipMsgId, text, remote, messageType,
                    threadId);
        } else {
            Logger.w(TAG,
                    "storeMessageInDatabase() message not found in Rcse DB");
            return storeMessageInMmsDb(messageTag, text, remote, messageType, threadId);
        }
    }
    /**
     * Store message in mms db.
     *
     * @param ipMsgId the ip msg id
     * @param text the text
     * @param remote the remote
     * @param messageType the message type
     * @param threadId the thread id
     * @return the long
     */
    static long storeMessageInMmsDb(int ipMsgId, String text, String remote,
            int messageType, long threadId) {
        long idInMmsDb = -1;
        if (text.startsWith("File Transfer")
                || text.startsWith("File transfer")) {
            idInMmsDb = getFTIdInMmsDb(ipMsgId);
        } else {
            idInMmsDb = getIdInMmsDb(ipMsgId);
        }

        if (-1 != idInMmsDb) {
            Logger.d(TAG,
                    "storeMessageInDatabase() message found in Mms DB, id: "
                            + idInMmsDb);
            return idInMmsDb;
        } else {
            Logger.d(TAG,
                    "storeMessageInDatabase() message not found in Mms DB");
            if (TextUtils.isEmpty(remote)) {
                Logger.w(TAG, "storeMessageInDatabase() invalid remote: "
                        + remote);
                return -1L;
            }
            if (remote
                    .startsWith(PluginGroupChatWindow.GROUP_CONTACT_STRING_BEGINNER) || remote
                    .startsWith(PluginMultiChatWindow.MULTI_CONTACT_STRING_BEGINNER)) {
                return insertDatabase(text, remote, ipMsgId, messageType,
                        threadId);
            } else {
                final String contact = Utils.extractNumberFromUri(remote);
                return insertDatabase(text, contact, ipMsgId, messageType,
                        threadId);
            }
        }
    }
    /**
     * Update pre sent message in mms db.
     *
     * @param messageId the message id
     * @param messageTag the message tag
     * @return true, if successful
     */
    static boolean updatePreSentMessageInMmsDb(String messageId, int messageTag) {
        int ipMsgId = findIdInRcseDb(messageId);
        if (-1 < ipMsgId) {
            ContentResolver contentResolver = MediatekFactory
                    .getApplicationContext().getContentResolver();
            ContentValues values = new ContentValues();
            values.put(MtkSms.IPMSG_ID, ipMsgId);
            int count = contentResolver.update(SMS_CONTENT_URI, values,
                    MtkSms.IPMSG_ID + "=?",
                    new String[] { Integer.toString(messageTag) });
            Logger.d(TAG, "updatePreSentMessageInMmsDb() messageId: "
                    + messageId + " ,messageTag: " + messageTag + " ,count: "
                    + count);
            return count == 1;
        } else {
            Logger.w(TAG, "updatePreSentMessageInMmsDb() ");
            return false;
        }
    }
    /**
     * Gets the id in mms db.
     *
     * @param ipMsgId the ip msg id
     * @return the id in mms db
     */
    static long getIdInMmsDb(int ipMsgId) {
        Logger.d(TAG, "getIdInMmsDb() entry, ipMsgId: " + ipMsgId);
        ContentResolver contentResolver = MediatekFactory
                .getApplicationContext().getContentResolver();
        Cursor cursor = null;
        try {
            final String[] args = { Integer.toString(ipMsgId) };
            cursor = contentResolver.query(SMS_CONTENT_URI,
                    PROJECTION_WITH_THREAD, SELECTION_IP_MSG_ID, args, null);
            Logger.v(TAG, "getIdInMmsDb() cursor count: " + cursor.getCount());
            if (cursor.moveToFirst()) {
                long mmsDbId = cursor.getLong(cursor.getColumnIndex(Sms._ID));
                long threadId = cursor.getLong(cursor
                        .getColumnIndex(Sms.THREAD_ID));
                String contact = cursor.getString(cursor
                        .getColumnIndex(Sms.ADDRESS));
                String body = cursor.getString(cursor
                        .getColumnIndex(Sms.BODY));
                Logger.v(TAG, "getIdInMmsDb() contact is " + contact
                        + " threadId is " + threadId);
                if (contact != null
                        && contact
                                .startsWith(PluginGroupChatWindow.GROUP_CONTACT_STRING_BEGINNER)) {
                    String tag = ThreadTranslater.translateThreadId(threadId);
                    if (tag == null) {
                        Logger.v(TAG,
                                "getIdInMmsDb() the thread to tag not exist ");
                        ThreadTranslater.saveThreadandTag(threadId, contact);
                    } else {
                        Logger.v(TAG, "getIdInMmsDb() the thread to tag exist "
                                + tag);
                    }
                } else if (contact != null
                        && contact
                                .startsWith(PluginMultiChatWindow.MULTI_CONTACT_STRING_BEGINNER)) {
                    String tag = ThreadTranslater.translateThreadId(threadId);
                    if (tag == null) {
                        Logger.v(TAG,
                                "getIdInMmsDb() the thread to tag not exist ");
                        ThreadTranslater.saveThreadandTag(threadId, contact);
                    } else {
                        Logger.v(TAG, "getIdInMmsDb() the thread to tag exist "
                                + tag);
                    }
                }
                if(cursor.getCount() > 1 && body.contains("File transfer")) {
                    Logger.v(TAG, "getIdInMmsDb() body deosn't ,atch text: " + body);
                    if(cursor.moveToNext()) {
                        mmsDbId = cursor.getLong(cursor.getColumnIndex(Sms._ID));
                    }
                }
                Logger.v(TAG, "getIdInMmsDb() mmsDbId: " + mmsDbId);
                return mmsDbId;
            } else {
                Logger.v(TAG, "getIdInMmsDb() empty cursor");
                return -1L;
            }
        } finally {
            if (null != cursor) {
                cursor.close();
            }
        }
    }

    /**
     * Gets the id in mms db.
     *
     * @param ipMsgId the ip msg id
     * @return the id in mms db
     */
    static long getFTIdInMmsDb(int ipMsgId) {
        Logger.d(TAG, "getFTIdInMmsDb() entry, ipMsgId: " + ipMsgId);
        ContentResolver contentResolver = MediatekFactory
                .getApplicationContext().getContentResolver();
        Cursor cursor = null;
        try {
            final String[] args = { Integer.toString(ipMsgId) };
            cursor = contentResolver.query(SMS_CONTENT_URI,
                    PROJECTION_WITH_THREAD, SELECTION_IP_MSG_ID, args, null);
            Logger.d(TAG, "getIdInMmsDb() cursor count: " + cursor.getCount());
            if (cursor.moveToFirst()) {
                long mmsDbId = cursor.getLong(cursor.getColumnIndex(Sms._ID));
                long threadId = cursor.getLong(cursor
                        .getColumnIndex(Sms.THREAD_ID));
                String contact = cursor.getString(cursor
                        .getColumnIndex(Sms.ADDRESS));
                String body = cursor.getString(cursor
                        .getColumnIndex(Sms.BODY));
                Logger.d(TAG, "getIdInMmsDb() contact is " + contact
                        + " threadId is " + threadId + "body is" + body);
                if (contact != null
                        && contact
                                .startsWith(PluginGroupChatWindow.GROUP_CONTACT_STRING_BEGINNER)) {
                    String tag = ThreadTranslater.translateThreadId(threadId);
                    if (tag == null) {
                        Logger.d(TAG,
                                "getIdInMmsDb() the thread to tag not exist ");
                        ThreadTranslater.saveThreadandTag(threadId, contact);
                    } else {
                        Logger.d(TAG, "getIdInMmsDb() the thread to tag exist "
                                + tag);
                    }
                } else if (contact != null
                        && contact
                                .startsWith(PluginMultiChatWindow.MULTI_CONTACT_STRING_BEGINNER)) {
                    String tag = ThreadTranslater.translateThreadId(threadId);
                    if (tag == null) {
                        Logger.d(TAG,
                                "getIdInMmsDb() the thread to tag not exist ");
                        ThreadTranslater.saveThreadandTag(threadId, contact);
                    } else {
                        Logger.d(TAG, "getIdInMmsDb() the thread to tag exist "
                                + tag);
                    }
                }
                if(cursor.getCount() > 1 && !body.contains("File transfer")) {
                    Logger.d(TAG, "getFTIdInMmsDb() body deosn't ,atch text: " + body);
                    if(cursor.moveToNext()) {
                        mmsDbId = cursor.getLong(cursor.getColumnIndex(Sms._ID));
                    }
                }
                Logger.d(TAG, "getIdInMmsDb() mmsDbId: " + mmsDbId);
                return mmsDbId;
            } else {
                Logger.d(TAG, "getIdInMmsDb() empty cursor");
                return -1L;
            }
        } finally {
            if (null != cursor) {
                cursor.close();
            }
        }
    }

    /**
     * Initiate export complete chat text for one 2one chat.
     *
     * @param threadId Thread id for the chat in MMS DB
     */
    public static void initiateExportChat(long threadId) {
        Logger.d(TAG, "initiateExportChat() entry, threadId: " + threadId);
        String contact = "";
        ContentResolver contentResolver = MediatekFactory
                .getApplicationContext().getContentResolver();
        String messageList = "";
        Cursor cursor = null;
        try {
            final String[] args = { Long.toString(threadId) };
            cursor = contentResolver
                    .query(SMS_CONTENT_URI, PROJECTION_WITH_THREAD,
                            SELECTION_MMS_THREAD_ID, args, null);
            Logger.d(
                    TAG,
                    "getIdInMmsDb() nonEmpty cursor count = "
                            + cursor.getCount());
            if (cursor != null && cursor.moveToFirst()) {
                do {
                    String message = cursor.getString(cursor
                            .getColumnIndex(Sms.BODY));
                    contact = cursor.getString(cursor
                            .getColumnIndex(Sms.ADDRESS));
                    Logger.d(TAG, "getIdInMmsDb() contact is " + contact
                            + " message is " + message);
                    if (!message
                            .equals(getStringInRcse(R.string.file_transfer_title))) {
                        messageList = messageList + contact + " : " + message
                                + "\n";
                    }
                } while (cursor.moveToNext());
            } else {
                Logger.d(TAG, "getIdInMmsDb() empty cursor");
            }
        } finally {
            if (null != cursor) {
                cursor.close();
            }
        }
        if (getMessagingMode() == 0) {
            if (contact.startsWith(IpMessageConsts.JOYN_START)) {
                contact = contact.substring(4);
            }
        }
        if (cursor != null && !messageList.equals("")) {
            String fileName = Environment.getExternalStorageDirectory()
                    .getPath()
                    + "/"
                    + "Joyn"
                    + "/"
                    + contact
                    + "_chat"
                    + ".txt";
            File chatContentFile = null;
            try {
                chatContentFile = new File(fileName);
                chatContentFile.createNewFile();
                FileOutputStream fOut = new FileOutputStream(chatContentFile);
                OutputStreamWriter myOutWriter = new OutputStreamWriter(fOut);
                myOutWriter.append(messageList);
                myOutWriter.close();
                fOut.close();
            } catch (FileNotFoundException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            } catch (IOException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
            if (chatContentFile != null) {
                Uri contentUri = getUriFromPath(fileName);
                Logger.d(TAG, "initiateExportChat2() contentUri" + contentUri);
                Intent shareIntent = new Intent();
                shareIntent.setAction(Intent.ACTION_SEND);
                shareIntent.putExtra(Intent.EXTRA_STREAM, contentUri);
                shareIntent.setType("text/plain");
                shareIntent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
                shareIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                MediatekFactory.getApplicationContext().startActivity(
                        shareIntent);
            }
        }
    }
    /**
     * Find id in rcse db.
     *
     * @param msgId the msg id
     * @return the int
     */
    static int findIdInRcseDb(String msgId) {
        Logger.d(TAG, "findIdInRcseDb() entry, msgId: " + msgId);
        ContentResolver contentResolver = MediatekFactory
                .getApplicationContext().getContentResolver();
        if (TextUtils.isEmpty(msgId)) {
            Logger.e(TAG, "findIdInRcseDb(), invalid msgId: " + msgId);
            return DEFAULT_MESSAGE_ID;
        }
        String[] argument = { msgId };
        Cursor cursor = null;
        try {
            cursor = contentResolver.query(RCS_CONTENT_URI, PROJECTION_ONLY_ID,
                    KEY_EVENT_MESSAGE_ID + "=?", argument, null);
            if (null != cursor && cursor.moveToFirst()) {
                int rowId = cursor.getInt(cursor
                        .getColumnIndex(KEY_EVENT_ROW_ID));
                Logger.d(TAG, "findIdInRcseDb() row id for message: " + msgId
                        + ", is " + rowId);
                return rowId;
            } else {
                Logger.w(TAG, "findIdInRcseDb() invalid cursor: " + cursor);
            }
        } finally {
            if (null != cursor) {
                cursor.close();
            }
        }
        return -1;
    }
    /**
     * Find ft id in rcse db.
     *
     * @param msgId the msg id
     * @return the int
     */
    static int findFTIdInRcseDb(String msgId) {
        Logger.d(TAG, "findFTIdInRcseDb() entry, msgId: " + msgId);
        ContentResolver contentResolver = MediatekFactory
                .getApplicationContext().getContentResolver();
        if (TextUtils.isEmpty(msgId)) {
            Logger.e(TAG, "findFTIdInRcseDb(), invalid msgId: " + msgId);
            return DEFAULT_MESSAGE_ID;
        }
        String[] argument = { msgId };
        Cursor cursor = null;
        try {
            cursor = contentResolver.query(FileTransferLog.CONTENT_URI,
                    PROJECTION_ONLY_ID, FileTransferLog.FT_ID + "=?", argument,
                    null);
            if (null != cursor && cursor.moveToFirst()) {
                int rowId = cursor.getInt(cursor
                        .getColumnIndex(KEY_EVENT_ROW_ID));
                Logger.d(TAG, "findFTIdInRcseDb() row id for file: " + msgId
                        + ", is " + rowId);
                return rowId;
            } else {
                Logger.w(TAG, "findFTIdInRcseDb() invalid cursor: " + cursor);
            }
        } finally {
            if (null != cursor) {
                cursor.close();
            }
        }
        return -1;
    }

    private static boolean isPresentInUnRegProvider(String msgId, ContentResolver contentResolver) {
        Logger.i(TAG, "isPresentInUnRegProvider() msgId: " + msgId );
        Cursor cursor = null;
        int emptyCursor = 0;
        String[] selectionArg = { msgId};
        Logger.i(TAG, "loadO2OMessageFromLocal() MsgTag: " + msgId);
        try {
            cursor = contentResolver.query(UnregMessageProvider.CONTENT_URI,
                    null, UnregMessageProvider.KEY_MESSAGE_TAG + "=?",
                    selectionArg, null);
        } catch (Exception e) {

        }
        if(cursor != null & cursor.getCount() > 0) {
            Logger.i(TAG, "isPresentInUnRegProvider() return: " + true );
            return true;
        }
        Logger.i(TAG, "isPresentInUnRegProvider() return: " + false );
        return false;
    }

    public static void removeMultiMessagesFromMMSDatabase(long[] ids)
    {
        String[] string_list_ids = new String[ids.length];
        Logger.d(TAG, "removeMultiMessagesFromMMSDatabase()," + string_list_ids);
        for(int i = 0; i < ids.length; i++){
            string_list_ids[i] = String.valueOf(ids[i]);
        }
        Logger.d(TAG, "removeMessagesFromdatabase(),");
        ContentResolver contentResolver = MediatekFactory.getApplicationContext().getContentResolver();
        contentResolver
                    .delete(Uri.parse("content://sms/"), "_id" + " IN (" + new String(new char[ids.length - 1]).replace("\0", "?,") + "?)", string_list_ids);
     }
    /**
     * Find message tag in rcse db.
     *
     * @param msgId the msg id
     * @return the string
     */
    static String findMessageTagInRcseDb(String msgId) {
        Logger.d(TAG, "findMessageTagInRcseDb() entry, msgId: " + msgId);
        ContentResolver contentResolver = MediatekFactory
                .getApplicationContext().getContentResolver();
        String[] argument = { msgId };
        Cursor cursor = null;
        try {
            cursor = contentResolver.query(RCS_CONTENT_URI,
                    PROJECTION_MESSAGE_ID, KEY_EVENT_ROW_ID + "=?", argument,
                    null);
            if (null != cursor && cursor.moveToFirst()) {
                String messageTag = cursor.getString(cursor
                        .getColumnIndex(KEY_EVENT_MESSAGE_ID));
                Logger.d(TAG, "findMessageTagInRcseDb() row id for message: "
                        + msgId + ", is " + messageTag);
                return messageTag;
            } else {
                Logger.w(TAG, "findMessageTagInRcseDb() invalid cursor: "
                        + cursor);
            }
        } finally {
            if (null != cursor) {
                cursor.close();
            }
        }
        return null;
    }
    /*package*//**
                * Reload rcse messages.
                */
    static void reloadRcseMessages() {
        Logger.d(TAG, "reloadRcseMessages() entry");
        sReloadingMessages = true;
        PluginGroupChatWindow.removeAllGroupChatInvitationsInMms();
        ArrayList<Integer> messageIdArray = new ArrayList<Integer>();
        ContentResolver contentResolver = MediatekFactory
                .getApplicationContext().getContentResolver();
        Cursor cursor = null;
        sGroupMap = new ConcurrentHashMap<String, ArrayList<Integer>>();
        sO2OMap = new ConcurrentHashMap<String, ArrayList<Integer>>();
        try {
            final String[] args = { "0" };
            cursor = contentResolver.query(SMS_CONTENT_URI, null,
                    SELECTION_RELOAD_IP_MSG, args, null);
            if (cursor.moveToFirst()) {
                do {
                    int id = cursor.getInt(cursor.getColumnIndex(MtkSms.IPMSG_ID));
                    String address = cursor.getString(cursor
                            .getColumnIndex(Sms.ADDRESS));
                    String body = cursor.getString(cursor
                            .getColumnIndex(Sms.BODY));
                    Logger.d(TAG, "reloadRcseMessages() ipMsgId: " + id
                            + " , address: " + address);
                    if (address != null
                            && address.startsWith(PluginGroupChatWindow.
                                    GROUP_CONTACT_STRING_BEGINNER)) {
                        if (sGroupMap.containsKey(address)) {
                            ArrayList<Integer> messageList = sGroupMap
                                    .get(address);
                            messageList.add(id);
                        } else {
                            ArrayList<Integer> messageList = new ArrayList<Integer>();
                            messageList.add(id);
                            sGroupMap.put(address, messageList);
                        }
                    } else {
                            Logger.d(TAG,
                                    "reloadRcseMessages() O2O, need to rebuild!");
                            if (!messageIdArray.contains(id)) {
                                if (sO2OMap.containsKey(address)) {
                                    ArrayList<Integer> messageList = sO2OMap
                                            .get(address);
                                    messageList.add(id);
                                } else {
                                    ArrayList<Integer> messageList =
                                            new ArrayList<Integer>();
                                    messageList.add(id);
                                    sO2OMap.put(address, messageList);
                                }
                                messageIdArray.add(id);
                            } else {
                                Logger.d(TAG,
                                        "reloadRcseMessages() O2O duplicate Id, not to rebuild!");
                            }
                            if(id > UNSENT_MESSAGE_ID_THREASHOLD && !isPresentInUnRegProvider(Integer.toString(id),contentResolver)) {
                                Logger.d(TAG,
                                        "reloadRcseMessages() id is not in rcs db!");
                                String contact = address;
                                if (address.startsWith(IpMessageConsts.JOYN_START)) {
                                    contact = address.substring(4);
                                }
                                if(!body.contains("File transfer")) {
                                    Logger.d(TAG,
                                            "reloadRcseMessages() resent failed message");
                                IpMessageManager.getInstance(MediatekFactory.getApplicationContext())
                                    .sentMessageViaRCSe(body, contact, id);
                                }
                        }
                    }
                } while (cursor.moveToNext());
            } else {
                Logger.d(TAG, "reloadRcseMessages() cursor is empty");
            }
        } finally {
            if (null != cursor) {
                cursor.close();
            }
        }
        /* Logger.i(TAG, "reloadRcseMessages() " + messageIdArray.size() + " Rcse message found: "
                 + messageIdArray);
         PluginController.obtainMessage(ChatController.EVENT_RELOAD_MESSAGE, messageIdArray)
                 .sendToTarget();*/
        // Reload OneOne Messages
        messageIdArray.clear();
        Set<Entry<String, ArrayList<Integer>>> o2OtagSet = sO2OMap.entrySet();
        for (Entry<String, ArrayList<Integer>> tag : o2OtagSet) {
            ArrayList<Integer> messageList = tag.getValue();
            if (messageList.size() > 20) {
                ArrayList<Integer> trimMessageList = new ArrayList<Integer>();
                trimMessageList.addAll(messageList.subList(0, 20));
                Logger.i(TAG, "reloadRcseMessages()[size > 20] O2Otag is  "
                        + tag + " trimMessageList: " + trimMessageList);
                PluginController.obtainMessage(
                        ChatController.EVENT_RELOAD_MESSAGE, trimMessageList)
                        .sendToTarget();
                //messageList = (ArrayList<Integer>) messageList.subList(30, messageList.size()-1);
                messageList.removeAll(trimMessageList);
                sO2OMap.put(tag.getKey(), messageList);
                Logger.i(TAG, "New Message List List " + tag + " messageList: "
                        + messageList);
            } else {
                ArrayList<Integer> trimMessageList = new ArrayList<Integer>();
                Logger.i(TAG, "reloadRcseMessages() O2Otag is [size <= 20]  "
                        + tag + " trimMessageList: " + trimMessageList);
                PluginController.obtainMessage(
                        ChatController.EVENT_RELOAD_MESSAGE, messageList)
                        .sendToTarget();
                sO2OMap.remove(tag.getKey());
            }
        }
        // Reload Group Messages
        Set<Entry<String, ArrayList<Integer>>> tagSet = sGroupMap.entrySet();
        for (Entry<String, ArrayList<Integer>> tag : tagSet) {
            ArrayList<Integer> messageList = tag.getValue();
            ArrayList<Integer> trimMessageList = new ArrayList<Integer>();
            if (messageList.size() > 20) {
                trimMessageList.addAll(messageList.subList(0, 20));
            } else {
                trimMessageList.addAll(messageList);
            }
            Logger.i(TAG, "reloadRcseMessages() GroupTag is  " + tag
                    + " trimMessageList: " + trimMessageList);
            PluginController.obtainMessage(ChatController.EVENT_RELOAD_MESSAGE,
                    tag.getKey(), trimMessageList).sendToTarget();
        }
    }
    /**
     * Update message id in mms db.
     *
     * @param oldId the old id
     * @param newId the new id
     * @return the int
     */
    static int updateMessageIdInMmsDb(String oldId, String newId) {
        Logger.d(TAG, "updateMessageIdInMmsDb() entry");
        int idInRcse = findFTIdInRcseDb(newId);
        long idInMms = IpMessageManager.getMessageId(oldId);
        ContentResolver contentResolver = MediatekFactory
                .getApplicationContext().getContentResolver();
        if (contentResolver != null && idInMms != -1) {
            Uri uri = Uri.parse(SMS_CONTENT_URI + "/" + idInMms);
            ContentValues contentValues = new ContentValues();
            contentValues.put(MtkSms.IPMSG_ID, idInRcse);
            contentResolver.update(uri, contentValues, null, null);
        } else {
            Logger.e(TAG, "getIdInMmsDb(), cr is null!");
        }
        return idInRcse;
    }
    /**
     * On view file detials.
     *
     * @param filePath the file path
     * @param context the context
     * @return true, if successful
     */
    static boolean onViewFileDetials(String filePath, Context context) {
        Intent intent = generateFileDetailsIntent(filePath);
        if (intent != null) {
            context.startActivity(intent);
            return true;
        } else {
            Logger.w(TAG, "onViewFileDetials() intent is null");
            return false;
        }
    }

    /**
     * Returns the Uri which can be used to delete/work with images in the photo gallery.
     * @param filePath Path to IMAGE on SD card
     * @return Uri in the format of... content://media/external/images/media/[NUMBER]
     */
    public static Uri getUriFromPath(String filePath) {
        Uri photoUri = null;
        try {
            Logger.d(TAG, "getUriFromPath() entry, filePath: "
                    + filePath);
            long photoId = 0;
            photoUri = MediaStore.Files.getContentUri("external");
            String[] projection = {MediaStore.Files.FileColumns._ID};
            // TODO This will break if we have no matching item in the MediaStore.
            Cursor cursor = MediatekFactory.getApplicationContext().getContentResolver().query(photoUri, projection, MediaStore.MediaColumns.DATA + " LIKE ?", new String[] { filePath }, null);
            if (cursor != null && cursor.moveToFirst()) {
                int columnIndex = cursor.getColumnIndex(projection[0]);
                photoId = cursor.getLong(columnIndex);
                cursor.close();
                return Uri.parse(photoUri.toString() + "/" + photoId);
            } else {
                ContentValues values = new ContentValues();
                values.put(MediaStore.MediaColumns.DATA, filePath);
                Uri uri = MediatekFactory.getApplicationContext().getContentResolver().insert(photoUri, values);
                Logger.d(TAG, "getUriFromPath insert DB, media uri  = " + uri);
                return uri;
            }

        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
            Logger.d(TAG, "getUriFromPath() exception, filePath: "
                    + filePath);
            return null;
        }
    }
    /**
     * Generate file details intent.
     *
     * @param filePath the file path
     * @return the intent
     */
    private static Intent generateFileDetailsIntent(String filePath) {
        Logger.d(TAG, "generateFileDetailsIntent() entry, filePath: "
                + filePath);
        if (!TextUtils.isEmpty(filePath)) {
            Uri fileUri = null;
            File openFile  = new File(filePath);
            fileUri = getUriFromPath(filePath);
            Logger.d(TAG, "generateFileDetailsIntent() entry, fileUri: "
                    + fileUri);
            String mimeType = AsyncGalleryView.getMimeType(filePath);
            if (null != mimeType) {
                Intent intent = new Intent(Intent.ACTION_VIEW);
                intent.setDataAndType(fileUri,
                        mimeType);
                intent.setFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
                return intent;
            }
        }
        return null;
    }
    /**
     * Initialize sim id from Telephony.
     *
     * @param context Any context will be ok
     */
    public static void initializeSimIdFromTelephony(Context context) {/*
        Logger.d(TAG, "initializeSimIdFromTelephony() entry");
        int slotCount = getSimSlotCount(context);
        if (slotCount > 0) {
            List<SubInfoRecord> simInfos = SubscriptionManager
                    .getAllSubInfoList();
            int currentSlot = 0;
            while (currentSlot < slotCount) {
                //SubInfoRecord simInfo = SubscriptionManager.getSubInfoUsingSlotId(
                         //currentSlot);
                 if (simInfo != null) {
                    sSimId = simInfo.mSimInfoId;
                    Logger.d(TAG, "initializeSimIdFromTelephony() slot "
                            + currentSlot + " simId is " + sSimId);
                    break;
                } else {
                    Logger.d(TAG, "initializeSimIdFromTelephony() slot "
                            + currentSlot + " simInfo is null");
                 }
                currentSlot += 1;
            }
        } else {
            Logger.e(TAG,
                    "initializeSimIdFromTelephony() simCount must more than 0, now is "
                            + slotCount);
        }
        // Error handle if get wrong sim id
        if (sSimId <= 0) {
            Logger.e(TAG, "initializeSimIdFromTelephony() get wrong simId");
            sSimId = DUMMY_SIM_ID;
        }
        Logger.d(TAG, "initializeSimIdFromTelephony() exit simId is " + sSimId);
    */}
    /**
     * Get simcard slot count.
     *
     * @param context any context will be ok
     * @return the number of slot count
     */
    private static int getSimSlotCount(Context context) {/*
        int simCount = SubscriptionManager.getAllSubInfoCount();
        Logger.d(TAG, "getSimSlotCount() simCount is " + simCount);
        return simCount;
    */return 0;
    }
    /**
     * Return the string value associated with a particular resource ID in RCSe.
     *
     * @param resourceId The desired resource identifier, as generated by the
     *            aapt tool.
     * @return The string data associated with the resource, stripped of styled
     *         text information.
     */
    public static String getStringInRcse(int resourceId) {
        Resources resource = null;
        String string = null;
        try {
            resource = MediatekFactory.getApplicationContext()
                    .getPackageManager()
                    .getResourcesForApplication(CoreApplication.APP_NAME);
        } catch (android.content.pm.PackageManager.NameNotFoundException e) {
            e.printStackTrace();
        }
        if (null != resource) {
            string = resource.getString(resourceId);
        } else {
            Logger.e(TAG, "getStringInRcse(), resource is null!");
        }
        return string;
    }
    /**
     * Return whether file transfer is supported.
     *
     * @param contact the contact
     * @return True if file transfer is supported, else false.
     */
    public static boolean isFtSupportedInRcse(String contact) {
        Logger.d(TAG, "isFtSupportedInRcse() entry contact is " + contact);
        boolean support = false;
        PluginApiManager apiManager = PluginApiManager.getInstance();
        if (null != apiManager) {
            if (apiManager.isFtSupported(contact)) {
                support = true;
            } else {
                support = false;
            }
        } else {
            support = false;
        }
        Logger.d(TAG, "isFtSupportedInRcse() exit support is " + support);
        return support;
    }
    /**
     * Return the file transfer IpMessage.
     *
     * @param remote remote user
     * @param fileTransfer the file transfer
     * @return The file transfer IpMessage
     */
    public static IpMessage analysisFileType(String remote,
            FileStructForBinder fileTransfer) {
        String fileName = fileTransfer.mFileName;
        if (fileName != null) {
            String mimeType = MediaFile.getMimeTypeForFile(fileName);
            if (mimeType == null) {
                mimeType = MimeTypeMap.getSingleton().getMimeTypeFromExtension(
                        Utils.getFileExtension(fileName));
            }
            if (mimeType != null) {
                if (mimeType.contains(Utils.FILE_TYPE_IMAGE)) {
                    return new PluginIpImageMessage(fileTransfer, remote);
                } else if (mimeType.contains(Utils.FILE_TYPE_AUDIO)
                        || mimeType.contains("application/ogg")) {
                    return new PluginIpVoiceMessage(fileTransfer, remote);
                } else if (mimeType.contains(Utils.FILE_TYPE_VIDEO) && fileName.startsWith("record")) {
                    return new PluginIpVoiceMessage(fileTransfer, remote);
                } else if (mimeType.contains(Utils.FILE_TYPE_VIDEO)) {
                    return new PluginIpVideoMessage(fileTransfer, remote);
                } else if (fileName.toLowerCase().endsWith(".vcf")) {
                    return new PluginIpVcardMessage(fileTransfer, remote);
                } else if (fileName.toLowerCase().endsWith(".vcs")) {
                    return new PluginIpVCalendarMessage(fileTransfer, remote);
                } else {
                    // Todo
                    Logger.d(TAG, "analysisFileType() other type add here!");
                }
            }
        } else {
            Logger.w(TAG, "analysisFileType(), file name is null!");
        }
        return new PluginIpAttachMessage(fileTransfer, remote);
    }
    /**
     * Gets the messaging mode.
     *
     * @return the messaging mode
     */
    public static int getMessagingMode() {
        if (sMessagingUx == -1) {
            sMessagingUx = RcsSettings.getInstance().getMessagingUx();
        }
        return sMessagingUx;
    }
    /**
     * Gets the maximum text limit.
     *
     * @return the maximum text limit
     */
    public static int getMaximumTextLimit() {
        if (sMaxTextLimit == -1) {
            sMaxTextLimit = (int) (RcsSettings.getInstance()
                    .getMaxChatMessageLength());
        }
        if(sMaxTextLimit > sLocalMaxTextLimit) {
            sMaxTextLimit = sLocalMaxTextLimit;
        }
        return sMaxTextLimit;
    }
    /**
     * Return the file transfer IpMessage.
     *
     * @param messageType the message type
     * @param fileStruct the file struct
     * @param remote the remote
     * @return IpMessage Ipmessage in plugin defined
     */
    public static IpMessage exchangeIpMessage(int messageType,
            FileStructForBinder fileStruct, String remote) {
        String newTag = fileStruct.mFileTransferTag;
        Logger.d(TAG, "exchangeIpMessage() entry, newTag = " + newTag);
        switch (messageType) {
        case IpMessageConsts.IpMessageType.PICTURE:
            PluginIpImageMessage ipImageMessage = new PluginIpImageMessage(
                    fileStruct, remote);
            ipImageMessage.setTag(newTag);
            return ipImageMessage;
        case IpMessageConsts.IpMessageType.VOICE:
            PluginIpVoiceMessage ipVoiceMessage = new PluginIpVoiceMessage(
                    fileStruct, remote);
            ipVoiceMessage.setTag(newTag);
            return ipVoiceMessage;
        case IpMessageConsts.IpMessageType.VIDEO:
            PluginIpVideoMessage ipVideoMessage = new PluginIpVideoMessage(
                    fileStruct, remote);
            ipVideoMessage.setTag(newTag);
            return ipVideoMessage;
        case IpMessageConsts.IpMessageType.VCARD:
            PluginIpVcardMessage ipVcardMessage = new PluginIpVcardMessage(
                    fileStruct, remote);
            ipVcardMessage.setTag(newTag);
            return ipVcardMessage;
        case IpMessageConsts.IpMessageType.CALENDAR:
            PluginIpVCalendarMessage ipVCalenderMessage = new PluginIpVCalendarMessage(
                    fileStruct, remote);
            ipVCalenderMessage.setTag(newTag);
            return ipVCalenderMessage;
        default:
            PluginIpAttachMessage ipAttachMessage = new PluginIpAttachMessage(
                    fileStruct, remote);
            ipAttachMessage.setTag(newTag);
            return ipAttachMessage;
        }
    }
}
