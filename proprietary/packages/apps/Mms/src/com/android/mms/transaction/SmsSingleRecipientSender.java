package com.android.mms.transaction;

import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.SystemProperties;
import android.os.PersistableBundle;
import android.provider.Telephony.Mms;
import android.provider.Telephony.Sms;
import android.telephony.CarrierConfigManager;
import android.telephony.SmsManager;
import android.telephony.SmsMessage;
import android.util.Log;

import com.android.internal.telephony.PhoneConstants;
import com.android.mms.LogTag;
import com.android.mms.MmsApp;
import com.android.mms.MmsConfig;
import com.android.mms.data.Conversation;
import com.android.mms.ui.MessageUtils;
import com.android.mms.util.MmsLog;
import com.google.android.mms.MmsException;

import com.mediatek.mms.ext.IOpSmsSingleRecipientSenderExt;
import com.mediatek.opmsg.util.OpMessageUtils;

import java.util.ArrayList;

import mediatek.telephony.MtkSmsManager;

public class SmsSingleRecipientSender extends SmsMessageSender {

    private final boolean mRequestDeliveryReport;
    private String mDest;
    private Uri mUri;
    private static final String TAG = "SmsSingleRecipientSender";

    private IOpSmsSingleRecipientSenderExt mOpSmsSenderExt;
    
    public static final String MESSAGE_SENT_ACTION =
        "com.android.mms.transaction.MESSAGE_SENT";
    // Indicates next message can be picked up and sent out.
    public static final String EXTRA_MESSAGE_SENT_SEND_NEXT = "SendNextMsg";
    public static final String EXTRA_MESSAGE_CONCATENATION = "ConcatenationMsg";

    public SmsSingleRecipientSender(Context context, String dest, String msgText, long threadId,
            boolean requestDeliveryReport, Uri uri, int subId) {
        super(context, null, msgText, threadId, subId);
        mRequestDeliveryReport = requestDeliveryReport;
        mDest = dest;
        mUri = uri;
        mOpSmsSenderExt = OpMessageUtils.getOpMessagePlugin().getOpSmsSingleRecipientSenderExt();
    }

    private boolean isMultiPartMmsNotSupported() {
        CarrierConfigManager configMgr = (CarrierConfigManager) mContext.
                                                getSystemService(Context.CARRIER_CONFIG_SERVICE);
        PersistableBundle carrierConfig =
                configMgr.getConfigForSubId(mSubId);
        boolean isMultiPartNotSupported =
                carrierConfig.getBoolean
                        (CarrierConfigManager.KEY_MMS_SEND_MULTIPART_SMS_AS_SEPARATE_MESSAGES_BOOL);
        MmsLog.d(TAG, "isMultiPartNotSupported" + isMultiPartNotSupported);
        return isMultiPartNotSupported;
    }

    public boolean sendMessage(long token) throws MmsException {
        if (LogTag.DEBUG_SEND) {
            Log.v(TAG, "sendMessage token: " + token);
        }
        if (mMessageText == null) {
            // Don't try to send an empty message, and destination should be just
            // one.
            throw new MmsException("Null message body or have multiple destinations.");
        }

        /// M:Code analyze 002,add a variable to caculate the length of sms @{
        int codingType = SmsMessage.ENCODING_UNKNOWN;
        codingType = mOpSmsSenderExt.sendMessagePrepare(mContext, codingType);

        // if (!PhoneUtils.isValidSubId(mContext, mSubId)) {
        // // Make last check of the validity of current SIM. It is possible
        // that
        // // it is removed.
        // throw new MmsException("Current selected SIM is not valid");
        // }
        MtkSmsManager smsManager = MtkSmsManager.getSmsManagerForSubscriptionId(mSubId);
        ArrayList<String> messages = null;
        if ((MmsConfig.getEmailGateway() != null) &&
                (Mms.isEmailAddress(mDest) || MessageUtils.isAlias(mDest))) {
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
            mDest = Conversation.verifySingleRecipient(mContext, mThreadId, mDest);
        }
        int messageCount = messages.size();
        /// M:
        MmsLog.d(MmsApp.TXN_TAG, "SmsSingleRecipientSender: sendMessage(), Message Count="
                + messageCount);

        if (messageCount == 0) {
            // Don't try to send an empty message.
            throw new MmsException("SmsMessageSender.sendMessage: divideMessage returned " +
                    "empty messages. Original message is \"" + mMessageText + "\"");
        }

        boolean moved = Sms.moveMessageToFolder(mContext, mUri, Sms.MESSAGE_TYPE_OUTBOX, 0);
        if (!moved) {
            throw new MmsException("SmsMessageSender.sendMessage: couldn't move message " +
                    "to outbox: " + mUri);
        }
        if (LogTag.DEBUG_SEND) {
            MmsLog.vpi(TAG, "sendMessage mDest: " + mDest + " mRequestDeliveryReport: " +
                    mRequestDeliveryReport);
        }

        ArrayList<PendingIntent> deliveryIntents =  new ArrayList<PendingIntent>(messageCount);
        ArrayList<PendingIntent> sentIntents = new ArrayList<PendingIntent>(messageCount);

        for (int i = 0; i < messageCount; i++) {
            if (mRequestDeliveryReport && (i == (messageCount - 1))) {
                if (!mOpSmsSenderExt.sendOpMessage(mContext, mSubId, deliveryIntents)) {
                    // TODO: Fix: It should not be necessary to
                    // specify the class in this intent.  Doing that
                    // unnecessarily limits customizability.
                    deliveryIntents.add(PendingIntent.getBroadcast(
                            mContext, 0,
                            new Intent(
                                    MessageStatusReceiver.MESSAGE_STATUS_RECEIVED_ACTION,
                                    mUri,
                                    mContext,
                                    MessageStatusReceiver.class)
                                            .putExtra(PhoneConstants.SUBSCRIPTION_KEY, mSubId),
                                    0));
                }
            } else {
                deliveryIntents.add(null);
            }
            Intent intent  = new Intent(MESSAGE_SENT_ACTION,
                    mUri,
                    mContext,
                    SmsReceiver.class);
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
            sentIntents.add(PendingIntent.getBroadcast(mContext, i, intent, 0));
            /// @}
        }
        try {
            /// M:Code analyze 008, print log @{
            MmsLog.dpi(MmsApp.TXN_TAG, "\t Destination\t= " + mDest
                    + "\t ServiceCenter\t= " + mServiceCenter
                    //+ "\t Message\t= " + messages
                    + "\t uri\t= " + mUri
                    + "\t subId\t= " + mSubId
                    + "\t CodingType\t= " + codingType);
            /// @}

            boolean isMultiPartNotSupported = isMultiPartMmsNotSupported();
            if (isMultiPartNotSupported == true) {
                //In case multipart SMS is not supported
                for (int msgSegemtSent = 0; msgSegemtSent < messageCount; msgSegemtSent++) {
                     smsManager.sendTextMessage(mDest,
                        mServiceCenter,
                        messages.get(msgSegemtSent),
                        sentIntents.get(msgSegemtSent),
                        deliveryIntents.get(msgSegemtSent));
                 }
            } else  if (!mOpSmsSenderExt.sendMessage(mContext,
                          mSubId, mDest, mServiceCenter, messages, sentIntents, deliveryIntents)) {
                smsManager.sendMultipartTextMessageWithEncodingType(mDest, mServiceCenter,
                        messages, codingType, sentIntents, deliveryIntents);
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
            MmsLog.dpi(LogTag.TAG, "sendMessage: address=" + mDest + ", threadId=" + mThreadId +
                    ", uri=" + mUri + ", msgs.count=" + messageCount);
        }
        return false;
    }

    private void log(String msg) {
        Log.d(LogTag.TAG, "[SmsSingleRecipientSender] " + msg);
    }
}
