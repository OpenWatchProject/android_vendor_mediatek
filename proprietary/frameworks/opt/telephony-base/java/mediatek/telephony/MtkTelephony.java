/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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


package mediatek.telephony;

import android.annotation.IntDef;
import android.annotation.SdkConstant;
import android.annotation.SdkConstant.SdkConstantType;
import android.annotation.SystemApi;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Intent;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SqliteWrapper;
import android.provider.Telephony;
import android.provider.BaseColumns;
import android.net.Uri;
import android.telephony.SmsCbMessage;
import android.telephony.SubscriptionManager;
import android.telephony.Rlog;
import android.os.Parcelable;

import com.android.internal.telephony.PhoneConstants;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.util.Set;

public final class MtkTelephony {
    private static final String TAG = "MtkTelephony";

    /**
     * Not instantiable.
     */
    private MtkTelephony() {
    }

    /**
     * Broadcast Action: The SMS sub-system in the modem is ready.
     * The intent is sent to inform the APP if the SMS sub-system
     * is ready or not. The intent will have the following extra value:</p>
     *
     * <ul>
     *   <li><em>ready</em> - An boolean result code, true for ready</li>
     * </ul>
     */
    @SdkConstant(SdkConstantType.BROADCAST_INTENT_ACTION)
    public static final String SMS_STATE_CHANGED_ACTION =
            "android.provider.Telephony.SMS_STATE_CHANGED";

    /**
     * WapPush table columns
     */
    public static final class WapPush implements BaseColumns {

        /**
         * The content:// style URL for wap push table.
         */
        public static final Uri CONTENT_URI = Uri.parse("content://wappush");

        /**
         * The content:// style URL for wap push SI table.
         */
        public static final Uri CONTENT_URI_SI = Uri.parse("content://wappush/si");

        /**
         * The content:// style URL for wap push SL table.
         */
        public static final Uri CONTENT_URI_SL = Uri.parse("content://wappush/sl");

        /**
         * The content:// style URL for thread id table.
         */
        public static final Uri CONTENT_URI_THREAD = Uri.parse("content://wappush/thread_id");

        // Database Columns
        /**
         * Database column name of thread id.
         */
        public static final String THREAD_ID = "thread_id";

        /**
         * Database column name of sort order.
         */
        public static final String DEFAULT_SORT_ORDER = "date ASC";

        /**
         * Database column name of address.
         */
        public static final String ADDR = "address";

        /**
         * Database column name of service address.
         */
        public static final String SERVICE_ADDR = "service_center";

        /**
         * Database column name of read.
         */
        public static final String READ = "read";

        /**
         * Database column name of seen.
         */
        public static final String SEEN = "seen";

        /**
         * Database column name of lock status.
         */
        public static final String LOCKED = "locked";

        /**
         * Database column name of error.
         */
        public static final String ERROR = "error";

        /**
         * Database column name of date.
         */
        public static final String DATE = "date";

        /**
         * Database column name of wap push type.
         */
        public static final String TYPE = "type";

        /**
         * Database column name of SIID.
         */
        public static final String SIID = "siid";

        /**
         * Database column name of URL.
         */
        public static final String URL = "url";

        /**
         * Database column name of create.
         */
        public static final String CREATE = "created";

        /**
         * Database column name of expiration.
         */
        public static final String EXPIRATION = "expiration";

        /**
         * Database column name of read.
         */
        public static final String ACTION = "action";

        /**
         * Database column name of text.
         */
        public static final String TEXT = "text";

        /**
         * Database column name of subscription id.
         */
        public static final String SUBSCRIPTION_ID = "sub_id";

        /**
         * Database value of column SI.
         */
        public static final int TYPE_SI = 0;

        /**
         * Database value of column SI.
         */
        public static final int TYPE_SL = 1;

        /**
         * Database read value of column STATUS.
         */
        public static final int STATUS_SEEN = 1;

        /**
         * Database read value of column STATUS.
         */
        public static final int STATUS_UNSEEN = 0;

        /**
         * Database read value of column READ.
         */
        public static final int STATUS_READ = 1;

        /**
         * Database unread value of column READ.
         */
        public static final int STATUS_UNREAD = 0;

        public static final int STATUS_LOCKED = 1;
        public static final int STATUS_UNLOCKED = 0;
    }

    /**
     * Base columns for tables that contain text based SMSCbs.
     */
    public interface TextBasedSmsCbColumns {

        /**
         * The SUB ID which indicated which Subscription the SMSCb comes from
         * <P>Type: LONG</P>
         */
        public static final String SUBSCRIPTION_ID = "sub_id";

        /**
         * The channel ID of the message
         * which is the message identifier defined in the Spec. 3GPP TS 23.041
         * <P>Type: INTEGER</P>
         */
        public static final String CHANNEL_ID = "channel_id";

        /**
         * The date the message was sent
         * <P>Type: INTEGER (long)</P>
         */
        public static final String DATE = "date";

        /**
         * Has the message been read
         * <P>Type: INTEGER (boolean)</P>
         */
        public static final String READ = "read";

        /**
         * The body of the message
         * <P>Type: TEXT</P>
         */
        public static final String BODY = "body";

        /**
         * The thread id of the message
         * <P>Type: INTEGER</P>
         */
        public static final String THREAD_ID = "thread_id";

        /**
         * Indicates whether this message has been seen by the user. The "seen" flag will be
         * used to figure out whether we need to throw up a statusbar notification or not.
         */
        public static final String SEEN = "seen";

        /**
         * Has the message been locked?
         * <P>Type: INTEGER (boolean)</P>
         */
        public static final String LOCKED = "locked";
    }

    /**
     * Contains all cell broadcast messages in the cell broadcast app.
     */
    public static final class SmsCb implements BaseColumns, TextBasedSmsCbColumns {
        public static final Cursor query(ContentResolver cr, String[] projection) {
            return cr.query(CONTENT_URI, projection, null, null, DEFAULT_SORT_ORDER);
        }

        public static final Cursor query(ContentResolver cr, String[] projection,
                String where, String orderBy) {
            return cr.query(CONTENT_URI, projection, where,
                    null, orderBy == null ? DEFAULT_SORT_ORDER : orderBy);
        }

        /**
         * The content:// style URL for cellbroadcast message table.
         */
        public static final Uri CONTENT_URI = Uri.parse("content://cb/messages");

        /**
         * The content:// style URL for "canonical_addresses" table
         */
        public static final Uri ADDRESS_URI = Uri.parse("content://cb/addresses");

        /**
         * The default sort order for this table
         */
        public static final String DEFAULT_SORT_ORDER = "date DESC";

        /**
         * Add an SMS to the given URI with thread_id specified.
         *
         * @param subId the subscription which the message belongs to
         * @param resolver the content resolver to use
         * @param uri the URI to add the message to
         * @param sim_id the id of the SIM card
         * @param channel_id the message identifier of the CB message
         * @param date the timestamp for the message
         * @param read true if the message has been read, false if not
         * @param body the body of the message
         * @return the URI for the new message
         */
        public static Uri addMessageToUri(int subId, ContentResolver resolver,
                Uri uri, int channel_id, long date, boolean read, String body) {
            ContentValues values = new ContentValues(5);

            values.put(SUBSCRIPTION_ID, Integer.valueOf(subId));
            values.put(DATE, Long.valueOf(date));
            values.put(READ, read ? Integer.valueOf(1) : Integer.valueOf(0));
            values.put(BODY, body);
            values.put(CHANNEL_ID, Integer.valueOf(channel_id));

            return resolver.insert(uri, values);
        }

        /**
         * Contains all received SMSCb messages in the SMS app's.
         */
        public static final class Conversations
                implements BaseColumns, TextBasedSmsCbColumns {
            /**
             * The content:// style URL for cell broadcast thread table.
             */
            public static final Uri CONTENT_URI =
                    Uri.parse("content://cb/threads");

            /**
             * The default sort order for this table
             */
            public static final String DEFAULT_SORT_ORDER = "date DESC";

            /**
             * The first 45 characters of the body of the message
             * <P>Type: TEXT</P>
             */
            public static final String SNIPPET = "snippet";

            /**
             * The number of messages in the conversation
             * <P>Type: INTEGER</P>
             */
            public static final String MESSAGE_COUNT = "msg_count";

            /**
             * The _id of address table in the conversation
             * <P>Type: INTEGER</P>
             */
            public static final String ADDRESS_ID = "address_id";
        }

        /**
         * Columns for the "canonical_addresses" table used by CB-SMS
         */
        public interface CanonicalAddressesColumns extends BaseColumns {
            /**
             * An address used in CB-SMS. Just a channel number
             * <P>Type: TEXT</P>
             */
            public static final String ADDRESS = "address";
        }

        /**
         * Columns for the "canonical_addresses" table used by CB-SMS
         */
        public static final class CbChannel implements BaseColumns {
            /**
             * The content:// style URL for this table
             */
            public static final Uri CONTENT_URI =
                    Uri.parse("content://cb/channel");

            public static final String NAME = "name";

            public static final String NUMBER = "number";

            public static final String ENABLE = "enable";

        }

        // TODO open when using CB Message
        /**
         * Read the PDUs out of an {@link #SMS_CB_RECEIVED_ACTION} intent.
         */
        public static final class Intents {

            /**
             * Read the PDUs out of an {@link #SMS_CB_RECEIVED_ACTION}.
             *
             * @param intent the intent to read from
             * @return an array of SmsCbMessages for the PDUs
             */
            public static final SmsCbMessage[] getMessagesFromIntent(
                    Intent intent) {
                Parcelable[] messages = intent.getParcelableArrayExtra("message");
                if (messages == null) {
                    return null;
                }

                SmsCbMessage[] msgs = new SmsCbMessage[messages.length];

                for (int i = 0; i < messages.length; i++) {
                    msgs[i] = (SmsCbMessage) messages[i];
                }
                return msgs;
            }
        }
    }

    public static class MtkCellBroadcasts implements BaseColumns {
        /**
         * Not instantiable.
         */
        private MtkCellBroadcasts() {}

        /**
         * Sepcify subscription identity for cell braodcast message
         * The sub id the messge come from.
         * <p>Type: LONG</p>
         */
        public static final String SUBSCRIPTION_ID = "sub_id";

        /**
         * Warning Area Coordinates
         * <p>Type: TEXT</p>
         */
        public static final String WAC = "wac";

        /**
         * Query columns for instantiating {@link android.telephony.CellBroadcastMessage} objects.
         */
        public static final String[] QUERY_COLUMNS = {
                _ID,
                Telephony.CellBroadcasts.GEOGRAPHICAL_SCOPE,
                Telephony.CellBroadcasts.PLMN,
                Telephony.CellBroadcasts.LAC,
                Telephony.CellBroadcasts.CID,
                Telephony.CellBroadcasts.SERIAL_NUMBER,
                Telephony.CellBroadcasts.SERVICE_CATEGORY,
                Telephony.CellBroadcasts.LANGUAGE_CODE,
                Telephony.CellBroadcasts.MESSAGE_BODY,
                Telephony.CellBroadcasts.DELIVERY_TIME,
                Telephony.CellBroadcasts.MESSAGE_READ,
                Telephony.CellBroadcasts.MESSAGE_FORMAT,
                Telephony.CellBroadcasts.MESSAGE_PRIORITY,
                Telephony.CellBroadcasts.ETWS_WARNING_TYPE,
                Telephony.CellBroadcasts.CMAS_MESSAGE_CLASS,
                Telephony.CellBroadcasts.CMAS_CATEGORY,
                Telephony.CellBroadcasts.CMAS_RESPONSE_TYPE,
                Telephony.CellBroadcasts.CMAS_SEVERITY,
                Telephony.CellBroadcasts.CMAS_URGENCY,
                Telephony.CellBroadcasts.CMAS_CERTAINTY,
                SUBSCRIPTION_ID,
                WAC
        };
    }

    public static class MtkSms implements BaseColumns, Telephony.TextBasedSmsColumns {
        /**
         * IP message field.
         */
        public static final String IPMSG_ID = "ipmsg_id";

        /**
         * CT feature for concatenated message.
         * Specify the reference id for part of SMS.
         */
        public static final String REFERENCE_ID = "ref_id";

        /**
         * CT feature for concatenated message.
         * Specify the receive length for concatenated SMS.
         */
        public static final String TOTAL_LENGTH = "total_len";

        /**
         * CT feature for concatenated message.
         * Specify the receive length for part of SMS.
         */
        public static final String RECEIVED_LENGTH = "rec_len";

        /**
         * CT feature for concatenated message.
         * Specify the received time for part of SMS.
         */
        public static final String RECEIVED_TIME = "recv_time";

        /**
         * CT feature for concatenated message.
         * Specify the upload flag for part of SMS.
         */
        public static final String UPLOAD_FLAG = "upload_flag";

        /**
         * TP-Status: CDMA card request deliver report
         * @hide
         */
        public static final int STATUS_REPLACED_BY_SC = 2;

        /**
         * Add an SMS to the given URI with thread_id specified.
         *
         * @param subId the sub_id which the message belongs to
         * @param resolver the content resolver to use
         * @param uri the URI to add the message to
         * @param address the address of the sender
         * @param body the body of the message
         * @param subject the psuedo-subject of the message
         * @param sc the service center of the message
         * @param date the timestamp for the message
         * @param read true if the message has been read, false if not
         * @param deliveryReport true if a delivery report was requested, false if not
         * @param threadId the thread_id of the message
         * @return the URI for the new message
         */
        public static Uri addMessageToUri(int subId, ContentResolver resolver,
                Uri uri, String address, String body, String subject, String sc,
                Long date, boolean read, boolean deliveryReport, long threadId) {
            ContentValues values = new ContentValues(9);
            Rlog.v(TAG,"Telephony addMessageToUri sub id: " + subId);

            values.put(Telephony.TextBasedSmsColumns.SUBSCRIPTION_ID, subId);
            values.put(Telephony.TextBasedSmsColumns.ADDRESS, address);
            if (date != null) {
                values.put(Telephony.TextBasedSmsColumns.DATE, date);
            }
            if (sc != null) {
                values.put(Telephony.TextBasedSmsColumns.SERVICE_CENTER, sc);
            }
            values.put(Telephony.TextBasedSmsColumns.READ,
                    read ? Integer.valueOf(1) : Integer.valueOf(0));
            values.put(Telephony.TextBasedSmsColumns.SUBJECT, subject);
            values.put(Telephony.TextBasedSmsColumns.BODY, body);
            if (deliveryReport) {
                values.put(Telephony.TextBasedSmsColumns.STATUS,
                        Telephony.TextBasedSmsColumns.STATUS_PENDING);
            }
            if (threadId != -1L) {
                values.put(Telephony.TextBasedSmsColumns.THREAD_ID, threadId);
            }
            return resolver.insert(uri, values);
        }

        /**
         * Contains all text-based SMS messages in the SMS app inbox.
         */
        public static final class Inbox implements BaseColumns, Telephony.TextBasedSmsColumns {

            /**
             * Not instantiable.
             */
            private Inbox() {
            }



            // MTK-START
            /**
             * Add an SMS to the Inbox.
             *
             * @param subId the sub Id for specified message
             * @param resolver the content resolver to use
             * @param address the address of the sender
             * @param body the body of the message
             * @param subject the pseudo-subject of the message
             * @param sc the service center address of the message
             * @param date the timestamp for the message
             * @param read true if the message has been read, false if not
             * @return the URI for the new message
             */
            public static Uri addMessage(int subId, ContentResolver resolver,
                    String address, String body, String subject, String sc, Long date,
                    boolean read) {
                return addMessageToUri(subId, resolver, Telephony.Sms.Inbox.CONTENT_URI, address,
                        body, subject, sc, date, read, false, -1L);
            }
            //MTK-END
        }


        /**
         * Contains all sent text-based SMS messages in the SMS app.
         */
        public static final class Sent implements BaseColumns, Telephony.TextBasedSmsColumns {

            /**
             * Not instantiable.
             */
            private Sent() {
            }

            /**
             * Add an SMS to the Sent box.
             *
             * @param subId the sub id for specified message
             * @param resolver the content resolver to use
             * @param address the address of the sender
             * @param body the body of the message
             * @param subject the pseudo-subject of the message
             * @param sc the service center address of the message
             * @param date the timestamp for the message
             * @return the URI for the new message
             */
            public static Uri addMessage(int subId, ContentResolver resolver,
                    String address, String body, String subject, String sc, Long date) {
                return addMessageToUri(subId, resolver, Telephony.Sms.Sent.CONTENT_URI, address,
                        body, subject, sc, date, true, false, -1L);
            }
        }
        /**
         * Contains constants for SMS related Intents that are broadcast.
         */
        public static final class Intents {

            /**
             * Not instantiable.
             */
            private Intents() {
            }

            /**
             * Read the PDUs out of an {@link #SMS_RECEIVED_ACTION} or a
             * {@link #DATA_SMS_RECEIVED_ACTION} intent.
             *
             * @param intent the intent to read from
             * @return an array of SmsMessages for the PDUs
             */
            public static MtkSmsMessage[] getMessagesFromIntent(Intent intent) {
                Object[] messages;
                try {
                    messages = (Object[]) intent.getSerializableExtra("pdus");
                }
                catch (ClassCastException e) {
                    Rlog.e(TAG, "getMessagesFromIntent: " + e);
                    return null;
                }

                if (messages == null) {
                    Rlog.e(TAG, "pdus does not exist in the intent");
                    return null;
                }

                String format = intent.getStringExtra("format");
                int subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY,
                        SubscriptionManager.getDefaultSmsSubscriptionId());

                Rlog.v(TAG, " getMessagesFromIntent sub_id : " + subId);

                int pduCount = messages.length;
                MtkSmsMessage[] msgs = new MtkSmsMessage[pduCount];

                for (int i = 0; i < pduCount; i++) {
                    byte[] pdu = (byte[]) messages[i];
                    msgs[i] = MtkSmsMessage.createFromPdu(pdu, format);
                    if (msgs[i] != null) msgs[i].setSubId(subId);
                }
                return msgs;
            }
        }
    }

    /**
     * Helper functions for the "threads" table used by MMS and SMS.
     */
    public static final class MtkThreads implements Telephony.ThreadsColumns {
        /**
         * The read message count of the thread.
         * <P>Type: INTEGER</P>
         */
        public static final String READ_COUNT = "readcount";

        /**
         * Wap push thread.
         */
        public static final int WAPPUSH_THREAD = 2;

        /**
         * Cell broadcast thread.
         */
        public static final int CELL_BROADCAST_THREAD = 3;

        /**
         * IP message thread.
         */
        public static final int IP_MESSAGE_GUIDE_THREAD = 10;

        /**
         * Whether a thread is being writen or not
         * 0: normal 1: being writen
         * <P>Type: INTEGER (boolean)</P>
         */
        public static final String STATUS = "status";

        /**
         * CT feature for date sent.
         */
        public static final String DATE_SENT = "date_sent";

        private static final String[] ID_PROJECTION = { BaseColumns._ID };

        /**
         * Private {@code content://} style URL for this table. Used by
         * {@link #getOrCreateThreadId(android.content.Context, java.util.Set)}.
         */
        private static final Uri THREAD_ID_CONTENT_URI = Uri.parse(
                "content://mms-sms/threadID");

        /**
         * Not instantiable.
         */
        private MtkThreads() {
        }

        /**
         * Only for BackupRestore
         * Given the recipients list and subject of an unsaved message,
         * return its thread ID.  If the message starts a new thread,
         * allocate a new thread ID.  Otherwise, use the appropriate
         * existing thread ID.
         *
         * Find the thread ID of the same set of recipients (in
         * any order, without any additions). If one
         * is found, return it.  Otherwise, return a unique thread ID.
         */
        public static long getOrCreateThreadId(
                Context context, Set<String> recipients, String backupRestoreIndex) {
            Uri.Builder uriBuilder = THREAD_ID_CONTENT_URI.buildUpon();

            if (backupRestoreIndex != null && backupRestoreIndex.length() > 0) {
                uriBuilder.appendQueryParameter("backupRestoreIndex", backupRestoreIndex);
            }

            for (String recipient : recipients) {
                if (Telephony.Mms.isEmailAddress(recipient)) {
                    recipient = Telephony.Mms.extractAddrSpec(recipient);
                }
                uriBuilder.appendQueryParameter("recipient", recipient);
            }
            Uri uri = uriBuilder.build();
            Cursor cursor = SqliteWrapper.query(context, context.getContentResolver(),
                    uri, ID_PROJECTION, null, null, null);
            if (cursor != null) {
                try {
                    if (cursor.moveToFirst()) {
                        Rlog.d(TAG, "getOrCreateThreadId for BackupRestore threadId = "
                                + cursor.getLong(0));
                        return cursor.getLong(0);
                    } else {
                        Rlog.e(TAG, "getOrCreateThreadId for BackupRestore returned no rows!");
                    }
                } finally {
                    cursor.close();
                }
            }
            Rlog.e(TAG, "getOrCreateThreadId for BackupRestore failed with uri " + uri.toString());
            throw new IllegalArgumentException("Unable to find or allocate a thread ID.");
        }

        /**
         * Get thread id by use recipient, without create.
         * It's convenient for use with Contacts APP.
         *
         * @param context the context object to use.
         * @param recipient the recipient to send to.
         * @return the thread id for the recipient
         */
        public static long getThreadId(Context context, String recipient) {
            Uri.Builder uriBuilder = Uri.parse("content://sms/thread_id").buildUpon();

            if (Telephony.Mms.isEmailAddress(recipient)) {
                recipient = Telephony.Mms.extractAddrSpec(recipient);
            }

            uriBuilder.appendQueryParameter("recipient", recipient);

            Uri uri = uriBuilder.build();

            Cursor cursor = SqliteWrapper.query(context, context.getContentResolver(), uri,
                    ID_PROJECTION, null, null, null);
            if (cursor != null) {
                try {
                    if (cursor.moveToFirst()) {
                        return cursor.getLong(0);
                    } else {
                        Rlog.e(TAG, "getThreadId returned no rows!");
                    }
                } finally {
                    cursor.close();
                }
            }

            Rlog.e(TAG, "getThreadId failed with uri " + uri.toString());
            throw new IllegalArgumentException("Unable to find a thread ID.");
        }
    }

    /**
     * Mwi table columns.
     */
    public static final class MtkMwi implements BaseColumns {
        /**
         * The content:// style URL for message waiting message table.
         */
        public static final Uri CONTENT_URI = Uri.parse("content://mwimsg");

        /**
         * Database column name of message account.
         */
        public static final String MSG_ACCOUNT = "msg_account";

        /**
         * Database column name of to account.
         */
        public static final String TO = "to_account";

        /**
         * Database column name of from account.
         */
        public static final String FROM = "from_account";

        /**
         * Database column name of subject.
         */
        public static final String SUBJECT = "subject";

        /**
         * Database column name of date.
         */
        public static final String MSG_DATE = "msg_date";

        /**
         * Database column name of wap push priority.
         */
        public static final String PRIORITY = "priority";

        /**
         * Database column name of id.
         */
        public static final String MSG_ID = "msg_id";

        /**
         * Database column name of context.
         */
        public static final String MSG_CONTEXT = "msg_context";

        /**
         * Database column name of read status.
         */
        public static final String READ = "read";

        /**
         * Database column name of seen status.
         */
        public static final String SEEN = "seen";

        /**
         * Database column name of content.
         */
        public static final String GOT_CONTENT = "got_content";
    }

        public static final class MtkThreadSettings implements BaseColumns {

            /**
             * Whether a thread is set notification enabled.
             * <P>Type: INTEGER (boolean)</P>
             */
            public static final String NOTIFICATION_ENABLE = "notification_enable";

            /**
             * Which thread does this settings belongs to
             * <P>Type: INTEGER </P>
             */
            public static final String THREAD_ID = "thread_id";

            /**
             * Whether a thread is set spam
             * 0: normal 1: spam
             * <P>Type: INTEGER (boolean)</P>
             */
            public static final String SPAM = "spam";

            /**
             * Whether a thread is set mute
             * 0: normal >1: mute duration
             * <P>Type: INTEGER (boolean)</P>
             */
            public static final String MUTE = "mute";

            /**
             * when does a thread be set mute
             * 0: normal >1: mute start time
             * <P>Type: INTEGER (boolean)</P>
             */
            public static final String MUTE_START = "mute_start";

            /**
             * Whether a thread is set vibrate
             * 0: normal 1: vibrate
             * <P>Type: INTEGER (boolean)</P>
             */
            public static final String VIBRATE = "vibrate";

            /**
             * Ringtone for a thread
             * <P>Type: STRING</P>
             */
            public static final String RINGTONE = "ringtone";

            /**
             * Wallpaper for a thread
             * <P>Type: STRING</P>
             */
            public static final String WALLPAPER = "_data";

            /**
             * Top Info for a thread
             * <P>Type: INTEGER</P>
             */
            public static final String TOP = "top";
        }
        // MTK-END


        /**
         * Contains MTK MMS and SMS messages.
         */
        public static final class MtkMmsSms implements BaseColumns {

            /**
             * Not instantiable.
             */
            private MtkMmsSms() {
            }

           // MTK-START
           /**
            * The style URL for quick text table.
            */
            public static final Uri CONTENT_URI_QUICKTEXT = Uri.parse(
                    "content://mms-sms/quicktext");
            // MTK-END

        }

        /**
         * Contains MTK MMS messages.
         */
        public static final class MtkMms {

              /**
               * Not instantiable.
               */
              private MtkMms() {
              }

              // MTK-START
              /**
               * The service center (SC) through which to send the message, if present
               *
               * <P>Type: TEXT</P>
               */
              public static final String SERVICE_CENTER = "service_center";

              /**
               * The status of the message
               *
               * <P>Type: INTEGER</P>
               */
              public static final String STATUS_EXT = "st_ext";
              // MTK-END
        }

    /**
     * Carriers class contains information about APNs, including MMSC information.
     */
    public static final class Carriers implements BaseColumns {

        /**
         * Not instantiable.
         * @hide
         */
        private Carriers() {}

        /**
         * The {@code content://} style URL for this table.
         * For MSIM, this will return APNs for the default subscription
         * {@link SubscriptionManager#getDefaultSubscriptionId()}. To specify subId for MSIM,
         * use {@link Uri#withAppendedPath(Uri, String)} to append with subscription id.
         */
        public static final Uri CONTENT_URI = Uri.parse("content://telephony/carriers");

        /**
         * The {@code content://} style URL for this table. Used for APN query based on current
         * subscription. Instead of specifying carrier matching information in the selection,
         * this API will return all matching APNs from current subscription carrier and queries
         * will be applied on top of that. If there is no match for MVNO (Mobile Virtual Network
         * Operator) APNs, return APNs from its MNO (based on mccmnc) instead. For MSIM, this will
         * return APNs for the default subscription
         * {@link SubscriptionManager#getDefaultSubscriptionId()}. To specify subId for MSIM,
         * use {@link Uri#withAppendedPath(Uri, String)} to append with subscription id.
         */
        public static final Uri SIM_APN_URI = Uri.parse(
                "content://telephony/carriers/sim_apn_list");

        /**
         * The {@code content://} style URL to be called from DevicePolicyManagerService,
         * can manage DPC-owned APNs.
         * @hide
         */
        public static final Uri DPC_URI = Uri.parse("content://telephony/carriers/dpc");

        /**
         * The {@code content://} style URL to be called from Telephony to query APNs.
         * When DPC-owned APNs are enforced, only DPC-owned APNs are returned, otherwise only
         * non-DPC-owned APNs are returned. For MSIM, this will return APNs for the default
         * subscription {@link SubscriptionManager#getDefaultSubscriptionId()}. To specify subId
         * for MSIM, use {@link Uri#withAppendedPath(Uri, String)} to append with subscription id.
         * @hide
         */
        public static final Uri FILTERED_URI = Uri.parse("content://telephony/carriers/filtered");

        /**
         * The {@code content://} style URL to be called from DevicePolicyManagerService
         * or Telephony to manage whether DPC-owned APNs are enforced.
         * @hide
         */
        public static final Uri ENFORCE_MANAGED_URI = Uri.parse(
                "content://telephony/carriers/enforce_managed");

        /**
         * The column name for ENFORCE_MANAGED_URI, indicates whether DPC-owned APNs are enforced.
         * @hide
         */
        public static final String ENFORCE_KEY = "enforced";

        /**
         * The default sort order for this table.
         */
        public static final String DEFAULT_SORT_ORDER = "name ASC";

        /**
         * Entry name.
         * <P>Type: TEXT</P>
         */
        public static final String NAME = "name";

        /**
         * APN name.
         * <P>Type: TEXT</P>
         */
        public static final String APN = "apn";

        /**
         * Proxy address.
         * <P>Type: TEXT</P>
         */
        public static final String PROXY = "proxy";

        /**
         * Proxy port.
         * <P>Type: TEXT</P>
         */
        public static final String PORT = "port";

        /**
         * MMS proxy address.
         * <P>Type: TEXT</P>
         */
        public static final String MMSPROXY = "mmsproxy";

        /**
         * MMS proxy port.
         * <P>Type: TEXT</P>
         */
        public static final String MMSPORT = "mmsport";

        /**
         * Server address.
         * <P>Type: TEXT</P>
         */
        public static final String SERVER = "server";

        /**
         * APN username.
         * <P>Type: TEXT</P>
         */
        public static final String USER = "user";

        /**
         * APN password.
         * <P>Type: TEXT</P>
         */
        public static final String PASSWORD = "password";

        /**
         * MMSC URL.
         * <P>Type: TEXT</P>
         */
        public static final String MMSC = "mmsc";

        /**
         * Mobile Country Code (MCC).
         * <P>Type: TEXT</P>
         * @deprecated Use {@link #SIM_APN_URI} to query APN instead, this API will return
         * matching APNs based on current subscription carrier, thus no need to specify MCC and
         * other carrier matching information. In the future, Android will not support MCC for
         * APN query.
         */
        public static final String MCC = "mcc";

        /**
         * Mobile Network Code (MNC).
         * <P>Type: TEXT</P>
         * @deprecated Use {@link #SIM_APN_URI} to query APN instead, this API will return
         * matching APNs based on current subscription carrier, thus no need to specify MNC and
         * other carrier matching information. In the future, Android will not support MNC for
         * APN query.
         */
        public static final String MNC = "mnc";

        /**
         * Numeric operator ID (as String). Usually {@code MCC + MNC}.
         * <P>Type: TEXT</P>
         * @deprecated Use {@link #SIM_APN_URI} to query APN instead, this API will return
         * matching APNs based on current subscription carrier, thus no need to specify Numeric
         * and other carrier matching information. In the future, Android will not support Numeric
         * for APN query.
         */
        public static final String NUMERIC = "numeric";

        /**
         * Authentication type.
         * <P>Type:  INTEGER</P>
         */
        public static final String AUTH_TYPE = "authtype";

        /**
         * Comma-delimited list of APN types.
         * <P>Type: TEXT</P>
         */
        public static final String TYPE = "type";

        /**
         * The protocol to use to connect to this APN.
         *
         * One of the {@code PDP_type} values in TS 27.007 section 10.1.1.
         * For example: {@code IP}, {@code IPV6}, {@code IPV4V6}, or {@code PPP}.
         * <P>Type: TEXT</P>
         */
        public static final String PROTOCOL = "protocol";

        /**
         * The protocol to use to connect to this APN when roaming.
         * The syntax is the same as protocol.
         * <P>Type: TEXT</P>
         */
        public static final String ROAMING_PROTOCOL = "roaming_protocol";

        /**
         * Is this the current APN?
         * <P>Type: INTEGER (boolean)</P>
         */
        public static final String CURRENT = "current";

        /**
         * Is this APN enabled?
         * <P>Type: INTEGER (boolean)</P>
         */
        public static final String CARRIER_ENABLED = "carrier_enabled";

        /**
         * Radio Access Technology info.
         * To check what values are allowed, refer to {@link android.telephony.ServiceState}.
         * This should be spread to other technologies,
         * but is currently only used for LTE (14) and eHRPD (13).
         * <P>Type: INTEGER</P>
         * @deprecated this column is no longer supported, use {@link #NETWORK_TYPE_BITMASK} instead
         */
        @Deprecated
        public static final String BEARER = "bearer";

        /**
         * Radio Access Technology bitmask.
         * To check what values can be contained, refer to {@link android.telephony.ServiceState}.
         * 0 indicates all techs otherwise first bit refers to RAT/bearer 1, second bit refers to
         * RAT/bearer 2 and so on.
         * Bitmask for a radio tech R is (1 << (R - 1))
         * <P>Type: INTEGER</P>
         * @hide
         * @deprecated this column is no longer supported, use {@link #NETWORK_TYPE_BITMASK} instead
         */
        @Deprecated
        public static final String BEARER_BITMASK = "bearer_bitmask";

        /**
         * Radio technology (network type) bitmask.
         * To check what values can be contained, refer to the NETWORK_TYPE_ constants in
         * {@link android.telephony.TelephonyManager}.
         * Bitmask for a radio tech R is (1 << (R - 1))
         * <P>Type: INTEGER</P>
         */
        public static final String NETWORK_TYPE_BITMASK = "network_type_bitmask";

        /**
         * MVNO type:
         * {@code SPN (Service Provider Name), IMSI, GID (Group Identifier Level 1)}.
         * <P>Type: TEXT</P>
         * @deprecated Use {@link #SIM_APN_URI} to query APN instead, this API will return
         * matching APNs based on current subscription carrier, thus no need to specify MVNO_TYPE
         * and other carrier matching information. In the future, Android will not support MVNO_TYPE
         * for APN query.
         */
        public static final String MVNO_TYPE = "mvno_type";

        /**
         * MVNO data.
         * Use the following examples.
         * <ul>
         *     <li>SPN: A MOBILE, BEN NL, ...</li>
         *     <li>IMSI: 302720x94, 2060188, ...</li>
         *     <li>GID: 4E, 33, ...</li>
         * </ul>
         * <P>Type: TEXT</P>
         * @deprecated Use {@link #SIM_APN_URI} to query APN instead, this API will return
         * matching APNs based on current subscription carrier, thus no need to specify
         * MVNO_MATCH_DATA and other carrier matching information. In the future, Android will not
         * support MVNO_MATCH_DATA for APN query.
         */
        public static final String MVNO_MATCH_DATA = "mvno_match_data";

        /**
         * The subscription to which the APN belongs to
         * <p>Type: INTEGER (long) </p>
         */
        public static final String SUBSCRIPTION_ID = "sub_id";

        /**
         * The profile_id to which the APN saved in modem.
         * <p>Type: INTEGER</p>
         *@hide
         */
        public static final String PROFILE_ID = "profile_id";

        /**
         * If set to {@code true}, then the APN setting will persist to the modem.
         * <p>Type: INTEGER (boolean)</p>
         *@hide
         */
        @SystemApi
        public static final String MODEM_PERSIST = "modem_cognitive";

        /**
         * The max number of connections of this APN.
         * <p>Type: INTEGER</p>
         *@hide
         */
        @SystemApi
        public static final String MAX_CONNECTIONS = "max_conns";

        /**
         * The wait time for retrying the APN, in milliseconds.
         * <p>Type: INTEGER</p>
         *@hide
         */
        @SystemApi
        public static final String WAIT_TIME_RETRY = "wait_time";

        /**
         * The max number of seconds this APN will support its maximum number of connections
         * as defined in {@link #MAX_CONNECTIONS}.
         * <p>Type: INTEGER</p>
         *@hide
         */
        @SystemApi
        public static final String TIME_LIMIT_FOR_MAX_CONNECTIONS = "max_conns_time";

        /**
         * The MTU (maximum transmit unit) size of the mobile interface to which the APN is
         * connected, in bytes.
         * <p>Type: INTEGER </p>
         * @hide
         */
        @SystemApi
        public static final String MTU = "mtu";

        /**
         * APN edit status. APN could be added/edited/deleted by a user or carrier.
         * see all possible returned APN edit status.
         * <ul>
         *     <li>{@link #UNEDITED}</li>
         *     <li>{@link #USER_EDITED}</li>
         *     <li>{@link #USER_DELETED}</li>
         *     <li>{@link #CARRIER_EDITED}</li>
         *     <li>{@link #CARRIER_DELETED}</li>
         * </ul>
         * <p>Type: INTEGER </p>
         * @hide
         */
        @SystemApi
        public static final String EDITED_STATUS = "edited";

        /**
         * {@code true} if this APN visible to the user, {@code false} otherwise.
         * <p>Type: INTEGER (boolean)</p>
         * @hide
         */
        @SystemApi
        public static final String USER_VISIBLE = "user_visible";

        /**
         * {@code true} if the user allowed to edit this APN, {@code false} otherwise.
         * <p>Type: INTEGER (boolean)</p>
         * @hide
         */
        @SystemApi
        public static final String USER_EDITABLE = "user_editable";

        /**
         * {@link #EDITED_STATUS APN edit status} indicates that this APN has not been edited or
         * fails to edit.
         * <p>Type: INTEGER </p>
         * @hide
         */
        @SystemApi
        public static final @EditStatus int UNEDITED = 0;

        /**
         * {@link #EDITED_STATUS APN edit status} indicates that this APN has been edited by users.
         * <p>Type: INTEGER </p>
         * @hide
         */
        @SystemApi
        public static final @EditStatus int USER_EDITED = 1;

        /**
         * {@link #EDITED_STATUS APN edit status} indicates that this APN has been deleted by users.
         * <p>Type: INTEGER </p>
         * @hide
         */
        @SystemApi
        public static final @EditStatus int USER_DELETED = 2;

        /**
         * {@link #EDITED_STATUS APN edit status} is an intermediate value used to indicate that an
         * entry deleted by the user is still present in the new APN database and therefore must
         * remain tagged as user deleted rather than completely removed from the database.
         * @hide
         */
        public static final int USER_DELETED_BUT_PRESENT_IN_XML = 3;

        /**
         * {@link #EDITED_STATUS APN edit status} indicates that this APN has been edited by
         * carriers.
         * <p>Type: INTEGER </p>
         * @hide
         */
        @SystemApi
        public static final @EditStatus int CARRIER_EDITED = 4;

        /**
         * {@link #EDITED_STATUS APN edit status} indicates that this APN has been deleted by
         * carriers. CARRIER_DELETED values are currently not used as there is no use case.
         * If they are used, delete() will have to change accordingly. Currently it is hardcoded to
         * USER_DELETED.
         * <p>Type: INTEGER </p>
         * @hide
         */
        public static final @EditStatus int CARRIER_DELETED = 5;

        /**
         * {@link #EDITED_STATUS APN edit status} is an intermediate value used to indicate that an
         * entry deleted by the carrier is still present in the new APN database and therefore must
         * remain tagged as user deleted rather than completely removed from the database.
         * @hide
         */
        public static final int CARRIER_DELETED_BUT_PRESENT_IN_XML = 6;

        /**
         * The owner of the APN.
         * <p>Type: INTEGER</p>
         * @hide
         */
        public static final String OWNED_BY = "owned_by";

        /**
         * Possible value for the OWNED_BY field.
         * APN is owned by DPC.
         * @hide
         */
        public static final int OWNED_BY_DPC = 0;

        /**
         * Possible value for the OWNED_BY field.
         * APN is owned by other sources.
         * @hide
         */
        public static final int OWNED_BY_OTHERS = 1;

        /**
         * The APN set id. When the user manually selects an APN or the framework sets an APN as
         * preferred, all APNs with the same set id as the selected APN should be prioritized over
         * APNs in other sets.
         * <p>Type: INTEGER</p>
         * @hide
         */
        @SystemApi
        public static final String APN_SET_ID = "apn_set_id";

        /**
         * Possible value for the {@link #APN_SET_ID} field. By default APNs will not belong to a
         * set. If the user manually selects an APN without apn set id, there is no need to
         * prioritize any specific APN set ids.
         * <p>Type: INTEGER</p>
         * @hide
         */
        @SystemApi
        public static final int NO_APN_SET_ID = 0;

        /**
         * A unique carrier id associated with this APN
         * {@see TelephonyManager#getSimCarrierId()}
         * <p>Type: STRING</p>
         */
        public static final String CARRIER_ID = "carrier_id";

        /**
         * The skip 464xlat flag. Flag works as follows.
         * {@link #SKIP_464XLAT_DEFAULT}: the APN will skip only APN is IMS and no internet.
         * {@link #SKIP_464XLAT_DISABLE}: the APN will NOT skip 464xlat
         * {@link #SKIP_464XLAT_ENABLE}: the APN will skip 464xlat
         * <p>Type: INTEGER</p>
         *
         * @hide
         */
        public static final String SKIP_464XLAT = "skip_464xlat";

        /**
         * Possible value for the {@link #SKIP_464XLAT} field.
         * <p>Type: INTEGER</p>
         *
         * @hide
         */
        public static final int SKIP_464XLAT_DEFAULT = -1;

        /**
         * Possible value for the {@link #SKIP_464XLAT} field.
         * <p>Type: INTEGER</p>
         *
         * @hide
         */
        public static final int SKIP_464XLAT_DISABLE = 0;

        /**
         * Possible value for the {@link #SKIP_464XLAT} field.
         * <p>Type: INTEGER</p>
         *
         * @hide
         */
        public static final int SKIP_464XLAT_ENABLE = 1;


        /** @hide */
        @IntDef({
                UNEDITED,
                USER_EDITED,
                USER_DELETED,
                CARRIER_DELETED,
                CARRIER_EDITED,
        })
        @Retention(RetentionPolicy.SOURCE)
        public @interface EditStatus {}

        /** @hide */
        @IntDef({
                SKIP_464XLAT_DEFAULT,
                SKIP_464XLAT_DISABLE,
                SKIP_464XLAT_ENABLE,
        })
        @Retention(RetentionPolicy.SOURCE)
        public @interface Skip464XlatStatus {}

        /**
         * The OMACP ID for the OMACP APN.
         */
        public static final String OMACP_ID = "omacpid";

        /**
         * The NAP address for the OMACP APN.
         */
        public static final String NAP_ID = "napid";

        /**
         * The proxy id for the OMACP APN.
         */
        public static final String PROXY_ID = "proxyid";

        /**
         * The source type of this APN.
         * Indicates the APN is preset or added by user.
         * When APN restore happened, the apn added by user will be erased.
         */
        public static final String SOURCE_TYPE = "sourcetype";

        /**
         * The CSD number of this APN.
         */
        public static final String CSD_NUM = "csdnum";

        /**
         * The SPN value for the MVNO type.
         */
        public static final String SPN = "spn";

        /**
         * The IMSI value for the MVNO type.
         */
        public static final String IMSI = "imsi";

        /**
         * The PNN value for the MVNO type.
         */
        public static final String PNN = "pnn";

        /**
         * The PPP dial number of this APN.
         */
        public static final String PPP = "ppp";

        /**
         * The content style URL for this DM table.
         */
        public static final Uri CONTENT_URI_DM = Uri.parse("content://telephony/carriers_dm");
    }

}
