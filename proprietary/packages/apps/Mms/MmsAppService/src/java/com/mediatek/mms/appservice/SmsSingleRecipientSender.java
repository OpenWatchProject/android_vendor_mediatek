package com.mediatek.mms.appservice;

import android.app.Activity;
import android.app.PendingIntent;
import android.content.ContentUris;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.database.sqlite.SqliteWrapper;
import android.net.Uri;
import android.os.UserHandle;
import android.os.SystemProperties;
import android.os.PersistableBundle;
import android.provider.Telephony.Mms;
import android.provider.Telephony.Sms;
import android.provider.Telephony.Threads;
import android.telephony.CarrierConfigManager;
import android.telephony.PhoneNumberUtils;
import android.telephony.SmsManager;
import android.telephony.SmsMessage;
import android.text.TextUtils;
import android.util.Log;

import com.android.internal.telephony.PhoneConstants;
import com.mediatek.mms.appservice.LogTag;
import com.mediatek.mms.appservice.MmsConfig;

import com.google.android.mms.MmsException;

//import com.mediatek.mms.ext.IOpSmsSingleRecipientSenderExt;
//import com.mediatek.opmsg.util.OpMessageUtils;

import java.util.ArrayList;

import mediatek.telephony.MtkSmsManager;

public class SmsSingleRecipientSender extends SmsMessageSender {

    private final boolean mRequestDeliveryReport;
    private String mDest;
    private Uri mUri;
    private String mEmailAddress = null;
    private static final String TAG = "SmsSingleRecipientSender";

//    private IOpSmsSingleRecipientSenderExt mOpSmsSenderExt;

    private static final String MESSAGE_STATUS_RECEIVED_ACTION =
        "com.android.mms.transaction.MessageStatusReceiver.MESSAGE_STATUS_RECEIVED";

    // Indicates next message can be picked up and sent out.
    public static final String EXTRA_MESSAGE_SENT_SEND_NEXT = "SendNextMsg";
    public static final String EXTRA_MESSAGE_CONCATENATION = "ConcatenationMsg";
    private static Uri sSingleCanonicalAddressUri =
        Uri.parse("content://mms-sms/canonical-address");

    private static final Uri mAllThreadsUri =
        Threads.CONTENT_URI.buildUpon().appendQueryParameter("simple", "true").build();

    private static final String[] THREADS_PROJECTION = {
        Threads._ID, Threads.RECIPIENT_IDS
    };


    public SmsSingleRecipientSender(Context context, String dest, String msgText, long threadId,
            boolean requestDeliveryReport, Uri uri, int subId) {
        super(context, null, msgText, threadId, subId);
        mRequestDeliveryReport = requestDeliveryReport;
        mDest = dest;
        mUri = uri;
//        mOpSmsSenderExt = OpMessageUtils.getOpMessagePlugin().getOpSmsSingleRecipientSenderExt();
    }

    private boolean isMultiPartMmsNotSupported() {
        CarrierConfigManager configMgr = (CarrierConfigManager) mContext.
                                                getSystemService(Context.CARRIER_CONFIG_SERVICE);
        PersistableBundle carrierConfig =
                configMgr.getConfigForSubId(mSubId);
        boolean isMultiPartNotSupported =
                carrierConfig.getBoolean
                        (CarrierConfigManager.KEY_MMS_SEND_MULTIPART_SMS_AS_SEPARATE_MESSAGES_BOOL);
        Log.d(TAG, "isMultiPartNotSupported" + isMultiPartNotSupported);
        return isMultiPartNotSupported;
    }

    public boolean sendMessage() throws MmsException {
        if (mMessageText == null) {
            // Don't try to send an empty message, and destination should be just
            // one.
            throw new MmsException("Null message body or have multiple destinations.");
        }

        /// M:Code analyze 002,add a variable to caculate the length of sms @{
        int codingType = MmsConfig.getEncodingType();

        MtkSmsManager smsManager = MtkSmsManager.getSmsManagerForSubscriptionId(mSubId);
        ArrayList<String> messages = null;
        if ((MmsConfig.getEmailGateway() != null) &&
                (Mms.isEmailAddress(mDest) || MmsConfig.isAlias(mDest))) {
            String msgText;
            msgText = mDest + " " + mMessageText;
            mDest = MmsConfig.getEmailGateway();
            /// M:Code analyze 003,add a parameter codingType to caculate length of sms @{
            messages = smsManager.divideMessage(msgText, codingType);
            /// @}
        } else {
            /// M:Code analyze 003,add a parameter codingType to caculate length of sms @{
            messages = smsManager.divideMessage(mMessageText, codingType);
            /// @}
            // remove spaces and dashes from destination number
            // (e.g. "801 555 1212" -> "8015551212")
            // (e.g. "+8211-123-4567" -> "+82111234567")
            /// M:Code analyze 004, comment the line,using customized striping pattern to mDest @{
            //mDest = PhoneNumberUtils.stripSeparators(mDest);
            /** M: remove space from des number (e.g. "801 555 1212" -> "8015551212") @{ */
            mDest = mDest.replaceAll(" ", "");
            mDest = mDest.replaceAll("-", "");
            /// @}
            mDest = verifySingleRecipient(mContext, mThreadId, mDest);
        }
        int messageCount = messages.size();
        /// M:
        Log.d(LogTag.TXN_TAG, "SmsSingleRecipientSender: sendMessage(), Message Count="
                + messageCount);

        if (messageCount == 0) {
            // Don't try to send an empty message.
            throw new MmsException("SmsMessageSender.sendMessage: divideMessage returned " +
                    "empty messages. Original message is \""); // + mMessageText + "\"");
        }

        boolean moved = Sms.moveMessageToFolder(mContext, mUri, Sms.MESSAGE_TYPE_OUTBOX, 0);
        if (!moved) {
            throw new MmsException("SmsMessageSender.sendMessage: couldn't move message " +
                    "to outbox: " + mUri);
        }
        if (LogTag.DEBUG_SEND) {
            Log.i(TAG, "sendMessage mDest: " + mDest + " mRequestDeliveryReport: " +
                    mRequestDeliveryReport);
        }

        ArrayList<PendingIntent> deliveryIntents =  new ArrayList<PendingIntent>(messageCount);
        ArrayList<PendingIntent> sentIntents = new ArrayList<PendingIntent>(messageCount);

        for (int i = 0; i < messageCount; i++) {
            if (mRequestDeliveryReport && (i == (messageCount - 1))) {
                deliveryIntents.add(PendingIntent.getBroadcastAsUser(
                    mContext, 0,
                    new Intent(
                            MESSAGE_STATUS_RECEIVED_ACTION,mUri)
                            .setClassName("com.android.mms",
                            "com.android.mms.transaction.MessageStatusReceiver")
                            .putExtra(PhoneConstants.SUBSCRIPTION_KEY, mSubId),
                            0, UserHandle.CURRENT));
//                }
            } else {
                deliveryIntents.add(null);
            }

            Intent intent  = new Intent(SmsReceiverService.MESSAGE_SENT_ACTION, mUri);
            intent.setClassName("com.android.mms", "com.android.mms.transaction.SmsReceiver");
            /// M:Code analyze 007, comment the line,using different
            /// requestCode for every sub_message @{
            //int requestCode = 0;
            /// @}
            if (i == messageCount - 1) {
                // Changing the requestCode so that a different pending intent
                // is created for the last fragment with
                // EXTRA_MESSAGE_SENT_SEND_NEXT set to true.
                /// M:Code analyze 007, comment the line,using different
                /// requestCode for every sub_message @{
                // requestCode = 1;
                /// @}
                intent.putExtra(EXTRA_MESSAGE_SENT_SEND_NEXT, true);
            }

            /// M:Code analyze 008, add for concatenation msg @{
            if (messageCount > 1) {
                intent.putExtra(EXTRA_MESSAGE_CONCATENATION, true);
            }
            /// @}
            if (LogTag.DEBUG_SEND) {
                Log.v(TAG, "sendMessage sendIntent: " + intent);
            }
            intent.putExtra(PhoneConstants.SUBSCRIPTION_KEY, mSubId);
            /// M:Code analyze 007, comment the line,using different
            /// requestCode for every sub_message @{
            intent.setFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
            sentIntents.add(PendingIntent.getBroadcastAsUser(mContext, i, intent, 0,
                    UserHandle.CURRENT));
            /// @}
        }
        try {
            /// M:Code analyze 008, print log @{
            MmsLog.ipi(LogTag.TXN_TAG, "\t Destination\t= " + mDest
                    + "\t ServiceCenter\t= " + mServiceCenter
                    //+ "\t Message\t= " + messages
                    + "\t uri\t= " + mUri
                    + "\t subId\t= " + mSubId
                    + "\t CodingType\t= " + codingType);
            /// @}

            boolean isMultiPartNotSupported = isMultiPartMmsNotSupported();
            Context appContext = mContext.getApplicationContext();
            if (isMultiPartNotSupported == true) {
                //In case multipart SMS is not supported
                for (int msgSegemtSent = 0; msgSegemtSent < messageCount; msgSegemtSent++) {
                     ArrayList<String> msg = new ArrayList<String>();
                     msg.add(messages.get(msgSegemtSent));
                     ArrayList<PendingIntent> sIntents = new ArrayList<PendingIntent>();
                     sIntents.add(sentIntents.get(msgSegemtSent));
                     ArrayList<PendingIntent> dIntents =  new ArrayList<PendingIntent>();
                     dIntents.add(deliveryIntents.get(msgSegemtSent));
                     MmsAppCallback.getInstance(appContext).appCallbackSendSms(appContext, mSubId,
                         mDest, mServiceCenter, msg, codingType, sIntents, dIntents);
                 }
            } else {
                MmsAppCallback.getInstance(appContext).appCallbackSendSms(appContext, mSubId, mDest,
                    mServiceCenter, messages, codingType, sentIntents, deliveryIntents);
            }

            /// M:add this for Sms emulator update sms status @{
            /* P0 migration*/
            String enableNowSMS = ""; //SystemProperties.get("net.ENABLE_NOWSMS");
            if (enableNowSMS.equals("true")) {
                Sms.moveMessageToFolder(mContext, mUri, Sms.MESSAGE_TYPE_SENT, 0);
            }
            /// @}
        } catch (Exception ex) {
            Log.e(TAG, "SmsMessageSender.sendMessage: caught", ex);
            throw new MmsException("SmsMessageSender.sendMessage: caught " + ex +
                    " from SmsManager.sendTextMessage()");
        }
        if (Log.isLoggable(LogTag.TRANSACTION, Log.VERBOSE) || LogTag.DEBUG_SEND) {
            Log.i(LogTag.TAG, "sendMessage: address=" + mDest + ", threadId=" + mThreadId +
                    ", uri=" + mUri + ", msgs.count=" + messageCount);
        }
        return false;
    }

    private void log(String msg) {
        Log.d(LogTag.TAG, "[SmsSingleRecipientSender] " + msg);
    }

    /**
     * verifySingleRecipient takes a threadId and a string recipient [phone number or email
     * address]. It uses that threadId to lookup the row in the threads table and grab the
     * recipient ids column. The recipient ids column contains a space-separated list of
     * recipient ids. These ids are keys in the canonical_addresses table. The recipient is
     * compared against what's stored in the mmssms.db, but only if the recipient id list has
     * a single address.
     * @param context is used for getting a ContentResolver
     * @param threadId of the thread we're sending to
     * @param recipientStr is a phone number or email address
     * @return the verified number or email of the recipient
     */
    private String verifySingleRecipient(final Context context,
            final long threadId, final String recipientStr) {
        if (threadId <= 0) {
            Log.i(TAG, "verifySingleRecipient threadId is ZERO, recipient: " + recipientStr);
            return recipientStr;
        }
        Cursor c = context.getContentResolver().query(mAllThreadsUri, THREADS_PROJECTION,
                "_id=" + Long.toString(threadId), null, null);
        if (c == null) {
            Log.i(TAG, "verifySingleRecipient threadId: " + threadId +
                    " resulted in NULL cursor , recipient: " + recipientStr);
            return recipientStr;
        }
        String address = recipientStr;
        String recipientIds;
        try {
            if (!c.moveToFirst()) {
                Log.i(TAG, "verifySingleRecipient threadId: " + threadId +
                        " can't moveToFirst , recipient: " + recipientStr);
                return recipientStr;
            }
            recipientIds = c.getString(1);
        } finally {
            c.close();
        }
        String[] ids = recipientIds.split(" ");

        if (ids.length != 1) {
            // We're only verifying the situation where we have a single recipient input against
            // a thread with a single recipient. If the thread has multiple recipients, just
            // assume the input number is correct and return it.
            return recipientStr;
        }

        // Get the actual number from the canonical_addresses table for this recipientId
        address = getSingleAddressFromCanonicalAddressInDb(context, ids[0]);

        if (TextUtils.isEmpty(address)) {
            Log.i(TAG, "verifySingleRecipient threadId: " + threadId +
                    " getSingleNumberFromCanonicalAddresses returned empty number for: " +
                    ids[0] + " recipientIds: " + recipientIds);
            return recipientStr;
        }
        if (PhoneNumberUtils.compareLoosely(recipientStr, address)) {
            // Bingo, we've got a match. We're returning the input number because of area
            // codes. We could have a number in the canonical_address name of "232-1012" and
            // assume the user's phone's area code is 650. If the user sends a message to
            // "(415) 232-1012", it will loosely match "232-1202". If we returned the value
            // from the table (232-1012), the message would go to the wrong person (to the
            // person in the 650 area code rather than in the 415 area code).
            return recipientStr;
        }

        return address;
    }

    /**
     * getSingleNumberFromCanonicalAddresses looks up the recipientId in the canonical_addresses
     * table and returns the associated number or email address.
     * @param context needed for the ContentResolver
     * @param recipientId of the contact to look up
     * @return phone number or email address of the recipientId
     */
    private String getSingleAddressFromCanonicalAddressInDb(final Context context,
            final String recipientId) {
        Cursor c = SqliteWrapper.query(context, context.getContentResolver(),
                ContentUris.withAppendedId(sSingleCanonicalAddressUri, Long.parseLong(recipientId)),
                null, null, null, null);
        if (c == null) {
            Log.i(TAG, "null Cursor looking up recipient: " + recipientId);
            return null;
        }
        try {
            if (c.moveToFirst()) {
                String number = c.getString(0);
                return number;
            }
        } finally {
            c.close();
        }
        return null;
    }
}
