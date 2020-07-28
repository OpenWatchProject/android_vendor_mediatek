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

import android.content.ContentUris;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.Telephony.Mms;
import android.provider.Telephony.MmsSms;
import android.provider.Telephony.Sms;
import android.telephony.SmsManager;
import android.text.TextUtils;
import android.util.Log;

import com.android.mms.LogTag;
import com.android.mms.MmsApp;
import com.android.mms.data.Contact;
import com.android.mms.model.SlideModel;
import com.android.mms.model.SlideshowModel;
import com.android.mms.model.TextModel;
import com.android.mms.R;
import com.android.mms.ui.DeliveryReportActivity.MmsReportStatus;
import com.android.mms.util.AddressUtils;
import com.android.mms.util.DownloadManager;
import com.android.mms.util.ItemLoadedCallback;
import com.android.mms.util.ItemLoadedFuture;
import com.android.mms.util.MmsLog;
import com.android.mms.util.PduLoaderManager;
import com.google.android.mms.MmsException;
import com.google.android.mms.pdu.EncodedStringValue;
import com.google.android.mms.pdu.MultimediaMessagePdu;
import com.google.android.mms.pdu.NotificationInd;
import com.google.android.mms.pdu.RetrieveConf;
import com.google.android.mms.pdu.SendReq;
import com.mediatek.android.mms.pdu.MtkMultimediaMessagePdu;
import com.mediatek.android.mms.pdu.MtkPduHeaders;
import com.mediatek.android.mms.pdu.MtkPduPersister;
import com.mediatek.ipmsg.util.IpMessageUtils;
import com.mediatek.mms.callback.IMessageItemCallback;
import com.mediatek.mms.ipmessage.IIpMessageItemExt;
import com.mediatek.opmsg.util.OpMessageUtils;

import java.util.List;
import java.util.regex.Pattern;



/**
 * Mostly immutable model for an SMS/MMS message.
 *
 * <p>The only mutable field is the cached formatted message member,
 * the formatting of which is done outside this model in MessageListItem.
 */
public class MessageItem implements IMessageItemCallback {
    private static String TAG = "MessageItem";
    private static String IPMSG_TAG = "Mms/ipmsg/MessageItem";

    public enum DeliveryStatus  { NONE, INFO, FAILED, PENDING, RECEIVED }

    public static int ATTACHMENT_TYPE_NOT_LOADED = -1;

    final Context mContext;
    final String mType;
    final long mMsgId;
    final int mBoxId;

    DeliveryStatus mDeliveryStatus = DeliveryStatus.NONE;
    boolean mReadReport;
    boolean mLocked;            // locked to prevent auto-deletion

    String mTimestamp;
    String mAddress;
    String mContact;
    /// M: fix bug ALPS00439894, MTK MR1 new feature: Group Mms
    Contact mGroupContact;
    String mBody; // Body of SMS, first text of MMS.
    String mTextContentType; // ContentType of text of MMS.
    Pattern mHighlight; // portion of message to highlight (from search)

    // The only non-immutable field.  Not synchronized, as access will
    // only be from the main GUI thread.  Worst case if accessed from
    // another thread is it'll return null and be set again from that
    // thread.
    CharSequence mCachedFormattedMessage;

    // The last message is cached above in mCachedFormattedMessage. In the latest design, we
    // show "Sending..." in place of the timestamp when a message is being sent. mLastSendingState
    // is used to keep track of the last sending state so that if the current sending state is
    // different, we can clear the message cache so it will get rebuilt and recached.
    boolean mLastSendingState;

    // Fields for MMS only.
    Uri mMessageUri;
    int mMessageType;
    int mAttachmentType;
    String mSubject;
    SlideshowModel mSlideshow;
    int mMessageSize;
    int mErrorType;
    int mErrorCode;
    int mMmsStatus;
    int mMmsSize;
    long mSmsStatus;
    boolean mHasReadReport = false;
    boolean mHasDeliveryReport = false;
    Cursor mCursor;
    private PduLoadedCallback mPduLoadedCallback;
    private ItemLoadedFuture mItemLoadedFuture;

    /// M:
    boolean mSubMsg = false;
    /// M: add for gemini
    int mSubId;
    CharSequence mCachedFormattedTimestamp;
    CharSequence mCachedFormattedSubStatus;

    long mSmsDate = 0;
    long mSmsSentDate = 0;
    String mServiceCenter = null;

    private boolean mItemSelected = false;

    /// M: add for ipmessage
    long mIpMessageId = 0;

    boolean mIsDrawTimeDivider = false;
    String mTimeDividerString = "";

    boolean mIsDrawOnlineDivider = false;
    String mOnlineString = "";

    public IIpMessageItemExt mIpMessageItem;

    // M:for ALPS01065027,just for compose sms messagelist in scrolling
    MessageItem(Context context, Cursor cursor) throws MmsException {
        mContext = context;
        // / M: @{
        if (cursor == null) {
            throw new MmsException("Get the null cursor");
        }
        // / @}
        mMsgId = cursor.getLong(MessageListAdapter.COLUMN_ID);
        mBoxId = cursor.getInt(MessageListAdapter.COLUMN_SMS_TYPE);
        String type = cursor.getString(MessageListAdapter.COLUMN_MSG_TYPE);
        mType = type;
        mCursor = cursor;
        if ("sms".equals(type)) {
            mBody = cursor.getString(MessageListAdapter.COLUMN_SMS_BODY);
        }

        mIpMessageItem = IpMessageUtils.getIpMessagePlugin(context).getIpMessageItem();
        if (mIpMessageItem != null) {
            mIpMessageItem.onIpCreateMessageItem(context, cursor, null);
        }
    }

    MessageItem(Context context, int boxId, int messageType, int subId, int errorType, int locked,
            int charset, long msgId, String type, String subject,
            String deliveryReport, String readReport, Pattern highlight, boolean isDrawTimeDivider,
            long indDate, int mmsStatus,
            int mmsSize) throws MmsException {
        mContext = context;
        mBoxId = boxId;
        mMessageType = messageType;
        mSubId = subId;
        mErrorType = errorType;
        mLocked = locked != 0;
        if (!TextUtils.isEmpty(subject)) {
            EncodedStringValue v = new EncodedStringValue(charset,
                    MtkPduPersister.getBytes(subject));
            mSubject = v.getString();
        }
        mMsgId = msgId;
        mType = type;
        mHighlight = highlight;
        /// M: fix bug ALPS00406912
        mMmsStatus = mmsStatus;
        mMmsSize = mmsSize;
        mMessageUri = ContentUris.withAppendedId(Mms.CONTENT_URI, mMsgId);
        long timestamp = 0L;
        MtkPduPersister p = MtkPduPersister.getPduPersister(mContext.getApplicationContext());
        if (MtkPduHeaders.MESSAGE_TYPE_NOTIFICATION_IND == mMessageType) {
            mDeliveryStatus = DeliveryStatus.NONE;
            NotificationInd notifInd = (NotificationInd) p.load(mMessageUri);
            interpretFrom(notifInd.getFrom(), mMessageUri);
            /// M: Borrow the mBody to hold the URL of the message.
            if (notifInd.getContentLocation() != null) {
                mBody = new String(notifInd.getContentLocation());
            }
            mMessageSize = (int) notifInd.getMessageSize();
            timestamp = notifInd.getExpiry() * 1000L;
        } else {
            MultimediaMessagePdu msg = (MultimediaMessagePdu) p.load(mMessageUri);
            mSlideshow = SlideshowModel.createFromPduBody(context, msg.getBody());
            mAttachmentType = MessageUtils.getAttachmentType(mSlideshow, msg);

            if (mMessageType == MtkPduHeaders.MESSAGE_TYPE_RETRIEVE_CONF ||
                msg instanceof RetrieveConf) {
                RetrieveConf retrieveConf = (RetrieveConf) ((MultimediaMessagePdu) msg);
                interpretFrom(retrieveConf.getFrom(), mMessageUri);
                timestamp = retrieveConf.getDate() * 1000L;
            } else {
                /// M: Use constant string for outgoing messages
                mAddress = context.getString(R.string.messagelist_sender_self);
                mContact = mAddress;
                timestamp = ((SendReq) ((MultimediaMessagePdu) msg)).getDate() * 1000L;
            }

            if ((deliveryReport == null) || !mAddress.equals(context.getString(
                    R.string.messagelist_sender_self))) {
                mDeliveryStatus = DeliveryStatus.NONE;
            } else {
                int reportInt;
                try {
                    reportInt = Integer.parseInt(deliveryReport);
                    if (reportInt == MtkPduHeaders.VALUE_YES) {
                        mDeliveryStatus = DeliveryStatus.RECEIVED;
                    } else {
                        mDeliveryStatus = DeliveryStatus.NONE;
                    }
                } catch (NumberFormatException nfe) {
                    Log.e(TAG, "Value for delivery report was invalid.");
                    mDeliveryStatus = DeliveryStatus.NONE;
                }
            }

            if ((readReport == null) || !mAddress.equals(context.getString(
                    R.string.messagelist_sender_self))) {
                mReadReport = false;
            } else {
                int reportInt;
                try {
                    reportInt = Integer.parseInt(readReport);
                    mReadReport = (reportInt == MtkPduHeaders.VALUE_YES);
                } catch (NumberFormatException nfe) {
                    Log.e(TAG, "Value for read report was invalid.");
                    mReadReport = false;
                }
            }
            if (mBoxId == Mms.MESSAGE_BOX_SENT
                    && (mReadReport || mDeliveryStatus == MessageItem.DeliveryStatus.RECEIVED)) {
                List<MmsReportStatus> mmsReportStatusList =
                    MessageUtils.getMmsReportStatus(context, mMsgId);
                if (mmsReportStatusList != null && mmsReportStatusList.size() > 0) {
                    for (MmsReportStatus mmsReportStatus : mmsReportStatusList) {
                        MmsLog.d(TAG, "MessageItem.init(): readStatus = "
                                + mmsReportStatus.readStatus);
                        if (mmsReportStatus.readStatus == MtkPduHeaders.READ_STATUS_READ) {
                            mHasReadReport = true;
                            break;
                        }
                        MmsLog.d(TAG, "MessageItem.init(): deliveryStatus = "
                                + mmsReportStatus.deliveryStatus);
                        if (mmsReportStatus.deliveryStatus == MtkPduHeaders.STATUS_RETRIEVED) {
                            mHasDeliveryReport = true;
                        }
                    }
                }
            }

            SlideModel slide = mSlideshow.get(0);
            if ((slide != null) && slide.hasText()) {
                TextModel tm = slide.getText();
                mBody = tm.getText();
                mTextContentType = tm.getContentType();
            }

            mMessageSize = mSlideshow.getCurrentSlideshowSize();
        }

        if (!isOutgoingMessage()) {
            if (MtkPduHeaders.MESSAGE_TYPE_NOTIFICATION_IND == mMessageType) {
                mTimestamp = context.getString(getTimestampStrId(), MessageUtils
                        .formatTimeStampString(context, timestamp));
            } else {
                mTimestamp = MessageUtils.getShortTimeString(context, timestamp);
            }
        }

        mIsDrawTimeDivider = isDrawTimeDivider;
        if (mIsDrawTimeDivider) {
            if (MtkPduHeaders.MESSAGE_TYPE_NOTIFICATION_IND == mMessageType) {
                mTimeDividerString = MessageUtils.getTimeDividerString(context, indDate * 1000L);
            } else {
                mTimeDividerString = MessageUtils.getTimeDividerString(context, timestamp);
            }
        }
        mIpMessageItem = IpMessageUtils.getIpMessagePlugin(context).getIpMessageItem();
        if (!mIpMessageItem.onIpCreateMessageItem(context, mMsgId, mType, mSubId, mBoxId, mSubject,
            mMessageType, mAddress, timestamp)) {
            if (mIpMessageId > 0) {
                mIpMessageId = 0;
            }
        }
    }

   // SIM message Cursor column index, refer SmsProvider.java
    public static final int COLUMN_SIM_SERVICE_CENTER_ADDRESS  = 0;
    public static final int COLUMN_SIM_ADDRESS                 = 1;
    public static final int COLUMN_SIM_MESSAGE_CLASS           = 2;
    public static final int COLUMN_SIM_BODY                    = 3;
    public static final int COLUMN_SIM_DATE                    = 4;
    public static final int COLUMN_SIM_STATUS                  = 5;
    public static final int COLUMN_SIM_INDEX_ON_ICC            = 6;
    public static final int COLUMN_SIM_IS_STATUS_REPORT        = 7;
    public static final int COLUMN_SIM_TRANSPORT_TYPE          = 8;   // always "sms"
    public static final int COLUMN_SIM_TYPE                    = 9;   // always MESSAGE_TYPE_ALL
    public static final int COLUMN_SIM_LOCKED                  = 10;  // always 0
    public static final int COLUMN_SIM_ERROR_CODE              = 11;  // always 0
    public static final int COLUMN_SIM_ID                      = 12;
    public static final int COLUMN_SIM_SUBID                   = 13;

    // add for sim message
    MessageItem(Context context, Cursor cursor, boolean isDrawTimeDivider) throws MmsException {
        mContext = context;
        mCursor = cursor;
        mSubMsg = true;

        mMsgId = cursor.getLong(COLUMN_SIM_ID);
        mType = cursor.getString(COLUMN_SIM_TRANSPORT_TYPE);
        mServiceCenter = cursor.getString(COLUMN_SIM_SERVICE_CENTER_ADDRESS);
        mSubId = cursor.getInt(COLUMN_SIM_SUBID);
        mReadReport = false; // No read reports in sms

        long status = cursor.getLong(COLUMN_SIM_STATUS);
        if (status == SmsManager.STATUS_ON_ICC_SENT
                || status == SmsManager.STATUS_ON_ICC_UNSENT) {
            mBoxId = Sms.MESSAGE_TYPE_SENT;
        } else {
            mBoxId = Sms.MESSAGE_TYPE_INBOX;
        }

        mMessageUri = ContentUris.withAppendedId(Sms.CONTENT_URI, mMsgId);
        mAddress = cursor.getString(COLUMN_SIM_ADDRESS);

        // For incoming messages, the ADDRESS field contains the sender.
        mGroupContact = Contact.get(mAddress, true);
        mContact = mGroupContact.getName();
        /// @}

        /// M: if the sms's body is null, just do not show it.for 547338 @{
        String tempBody = cursor.getString(COLUMN_SIM_BODY);
        mBody = mContact + " : " + (tempBody != null ? tempBody : "");

        // Unless the message is currently in the progress of being sent, it gets a time stamp.
        long date = cursor.getLong(COLUMN_SIM_DATE);
        if (!isOutgoingMessage()) {
            mSmsDate = date;
            if (date != 0) {
                if (isReceivedMessage()) {
                    mTimestamp = String.format(context.getString(R.string.received_on),
                                MessageUtils.formatTimeStampString(context, date));
                    if (mSmsSentDate == 0) {
                        mSmsSentDate = date;
                    }
                } else {
                    mTimestamp = MessageUtils.getShortTimeString(context, date);
                }
            } else {
                mTimestamp = "";
            }
        }
    }

    MessageItem(Context context, String type, Cursor cursor,
            Pattern highlight, boolean isDrawTimeDivider, long indDate,
            boolean isGroupConversation) throws MmsException {
        mContext = context;
        /// M: @{
        if (cursor == null) {
            throw new MmsException("Get the null cursor");
        }
        /// @}

        mMsgId = cursor.getLong(MessageListAdapter.COLUMN_ID);
        mHighlight = highlight;
        mType = type;
        mCursor = cursor;

        /// M: add for ipmessage
        mIpMessageId = cursor.getLong(MessageListAdapter.COLUMN_SMS_IP_MESSAGE_ID);

        mIsDrawTimeDivider = isDrawTimeDivider;

        /// M: @{
        mServiceCenter = cursor.getString(MessageListAdapter.COLUMN_SMS_SERVICE_CENTER);

        //to filter SIM Message
        mSubId = -1;
        /// @}
        if ("sms".equals(type)) {
            mReadReport = false; // No read reports in sms

            long status = cursor.getLong(MessageListAdapter.COLUMN_SMS_STATUS);
            mSmsStatus = status;
            /// M: @{
            if (status == SmsManager.STATUS_ON_ICC_READ
                    || status == SmsManager.STATUS_ON_ICC_UNREAD
                    || status == SmsManager.STATUS_ON_ICC_SENT
                    || status == SmsManager.STATUS_ON_ICC_UNSENT) {
                mSubMsg = true;
            }
            /// @}

            /// M: @{
            /*
            if (status == Sms.STATUS_NONE) {
                // No delivery report requested
                mDeliveryStatus = DeliveryStatus.NONE;
            } else if (status >= Sms.STATUS_FAILED) {
                // Failure
                mDeliveryStatus = DeliveryStatus.FAILED;
            } else if (status >= Sms.STATUS_PENDING) {
                // Pending
                mDeliveryStatus = DeliveryStatus.PENDING;
            } else {
                // Success
                mDeliveryStatus = DeliveryStatus.RECEIVED;
            }*/
            if (status >= Sms.STATUS_FAILED) {
                // Failure
                mDeliveryStatus = DeliveryStatus.FAILED;
            } else if (status >= Sms.STATUS_PENDING) {
                // Pending
                mDeliveryStatus = DeliveryStatus.PENDING;
            } else if (status >= Sms.STATUS_COMPLETE && !mSubMsg) {
                // Success
                mDeliveryStatus = DeliveryStatus.RECEIVED;
            } else {
                mDeliveryStatus = DeliveryStatus.NONE;
            }
            /// @}

            mMessageUri = ContentUris.withAppendedId(Sms.CONTENT_URI, mMsgId);
            // Set contact and message body
            /// M: @{
            //mBoxId = cursor.getInt(columnsMap.mColumnSmsType);
            if (mSubMsg) {
                if (status == SmsManager.STATUS_ON_ICC_SENT
                        || status == SmsManager.STATUS_ON_ICC_UNSENT) {
                    mBoxId = Sms.MESSAGE_TYPE_SENT;
                } else {
                    mBoxId = Sms.MESSAGE_TYPE_INBOX;
                }
            } else {
                mBoxId = cursor.getInt(MessageListAdapter.COLUMN_SMS_TYPE);
            }
            /// @}
            mAddress = cursor.getString(MessageListAdapter.COLUMN_SMS_ADDRESS);

            /// M: @{
            //add for gemini and td single
            mSubId = cursor.getInt(MessageListAdapter.COLUMN_SUBID);
            /// @}

            /// M: @{
            /*if (Sms.isOutgoingFolder(mBoxId)) {
                String meString = context.getString(
                        R.string.messagelist_sender_self);

                mContact = meString;
            } else {
                // For incoming messages, the ADDRESS field contains the sender.
                mContact = Contact.get(mAddress, false).getName();
            }*/
            if (Sms.isOutgoingFolder(mBoxId) && !mSubMsg) {
                String meString = context.getString(
                        R.string.messagelist_sender_self);

                mContact = meString;
            } else {
                // For incoming messages, the ADDRESS field contains the sender.
                if (!TextUtils.isEmpty(mAddress) && (isGroupConversation || mSubMsg)) {
                    mGroupContact = Contact.get(mAddress, true);
                    mContact = mGroupContact.getName();
                } else {
                    mContact = context.getString(android.R.string.unknownName);
                }
            }
            /// @}

            /// M: @{
            //mBody = cursor.getString(columnsMap.mColumnSmsBody);
            if (mSubMsg) {
                /// M: if the sms's body is null, just do not show it.for 547338 @{
                String tempBody = cursor.getString(MessageListAdapter.COLUMN_SMS_BODY);
                mBody = mContact + " : " + (tempBody != null ? tempBody : "");
                /// @}
            } else {
                mBody = cursor.getString(MessageListAdapter.COLUMN_SMS_BODY);
            }
            /// @}

            // Unless the message is currently in the progress of being sent, it gets a time stamp.
            long date = cursor.getLong(MessageListAdapter.COLUMN_DATE);
            if (!isOutgoingMessage()) {
                // Set "received" or "sent" time stamp
                /// M: @{
                //mTimestamp = MessageUtils.formatTimeStampString(context, date);
                mSmsDate = date;
                if (date != 0) {
                    if (isReceivedMessage()) {
                        if (mSubMsg) {
                            mTimestamp = String.format(context.getString(R.string.received_on),
                                    MessageUtils.formatTimeStampString(context, date));
                        } else {
                            mTimestamp = MessageUtils.getShortTimeString(context, date);
                        }
                        if (mSmsSentDate == 0) {
                            mSmsSentDate = date;
                        }
                    } else {
//                        mTimestamp = String.format(context.getString(R.string.sent_on),
//                                MessageUtils.formatTimeStampString(context, date));
                        mTimestamp = MessageUtils.getShortTimeString(context, date);
                    }
                } else {
                    mTimestamp = "";
                }
            }
            if (mIsDrawTimeDivider) {
                mTimeDividerString = MessageUtils.getTimeDividerString(context, date);
            }
            mLocked = cursor.getInt(MessageListAdapter.COLUMN_LOCKED) != 0;
            mErrorCode = cursor.getInt(MessageListAdapter.COLUMN_SMS_ERROR_CODE);

            // add for ipmessage
            mIpMessageItem = IpMessageUtils.getIpMessagePlugin(context).getIpMessageItem();
            if (!mIpMessageItem.onIpCreateMessageItem(context, cursor,  mMsgId, mType, mSubId,
                    null, this, isDrawTimeDivider)) {
                if (mIpMessageId > 0) {
                    mIpMessageId = 0;
                }
            }
        } else if ("mms".equals(type)) {
            mMessageUri = ContentUris.withAppendedId(Mms.CONTENT_URI, mMsgId);
            mBoxId = cursor.getInt(MessageListAdapter.COLUMN_MMS_MESSAGE_BOX);
            mMessageType = cursor.getInt(MessageListAdapter.COLUMN_MMS_MESSAGE_TYPE);
            mErrorType = cursor.getInt(MessageListAdapter.COLUMN_MMS_ERROR_TYPE);
            String subject = cursor.getString(MessageListAdapter.COLUMN_MMS_SUBJECT);
            if (!TextUtils.isEmpty(subject)) {
                EncodedStringValue v = new EncodedStringValue(
                        cursor.getInt(MessageListAdapter.COLUMN_MMS_SUBJECT_CHARSET),
                        MtkPduPersister.getBytes(subject));
                /// M: google jb.mr1 patch, group mms
                mSubject = MessageUtils.cleanseMmsSubject(context, v.getString());
            }
            mLocked = cursor.getInt(MessageListAdapter.COLUMN_LOCKED) != 0;

            /// M: @{
            //add for gemini
            mSubId = cursor.getInt(MessageListAdapter.COLUMN_ID);
            /// @}

            mSlideshow = null;
            mAttachmentType = ATTACHMENT_TYPE_NOT_LOADED;
            mDeliveryStatus = DeliveryStatus.NONE;
            mReadReport = false;
            mBody = null;
            mMessageSize = 0;
            mTextContentType = null;
            /// google JB.MR1 patch, Initialize the time stamp to "" instead of null @{
            mTimestamp = "";
            /// @}
            mMmsStatus = cursor.getInt(MessageListAdapter.COLUMN_MMS_STATUS);

            // Start an async load of the pdu. If the pdu is already loaded, the callback
            // will get called immediately
            boolean loadSlideshow = mMessageType != MtkPduHeaders.MESSAGE_TYPE_NOTIFICATION_IND;

            mItemLoadedFuture = MmsApp.getApplication().getPduLoaderManager()
                    .getPdu(mMessageUri, loadSlideshow,
                    new PduLoadedMessageItemCallback());

        } else {
            // add for ip system message
            mIpMessageItem = IpMessageUtils.getIpMessagePlugin(context).getIpMessageItem();
            mBoxId = Sms.MESSAGE_TYPE_ALL;
            if (!mIpMessageItem.onIpCreateMessageItem(context, cursor, mMsgId, mType, -1,
                    null, this, isDrawTimeDivider)) {
                throw new MmsException("Unknown type of the message: " + type);
            }
        }
    }

    private void interpretFrom(EncodedStringValue from, Uri messageUri) {
        if (from != null) {
            mAddress = from.getString();
        } else {
            // In the rare case when getting the "from" address from the pdu fails,
            // (e.g. from == null) fall back to a slower, yet more reliable method of
            // getting the address from the "addr" table. This is what the Messaging
            // notification system uses.
            mAddress = AddressUtils.getFrom(mContext, messageUri);
        }
        /// M: fix bug ALPS00439894, MTK MR1 new feature: Group Mms
        mGroupContact = Contact.get(mAddress, false);
        mContact = TextUtils.isEmpty(mAddress) ? mContext.getString(android.R.string.unknownName)
                : mGroupContact.getName();
    }

    public boolean isMms() {
        return mType.equals("mms");
    }

    public boolean isSms() {
        return mType.equals("sms");
    }

    public boolean isDownloaded() {
        return (mMessageType != MtkPduHeaders.MESSAGE_TYPE_NOTIFICATION_IND);
    }

    /// M: google JB.MR1 patch, group mms
    public boolean isMe() {
        // Logic matches MessageListAdapter.getItemViewType which is used to decide which
        // type of MessageListItem to create: a left or right justified item depending on whether
        // the message is incoming or outgoing.
        boolean isIncomingMms = isMms()
                                    && (mBoxId == Mms.MESSAGE_BOX_INBOX
                                            || mBoxId == Mms.MESSAGE_BOX_ALL);
        boolean isIncomingSms = isSms()
                                    && (mBoxId == Sms.MESSAGE_TYPE_INBOX
                                            || mBoxId == Sms.MESSAGE_TYPE_ALL);
        return !(isIncomingMms || isIncomingSms);
    }

    public boolean isOutgoingMessage() {
        boolean isOutgoingMms = isMms()
                && (mBoxId == Mms.MESSAGE_BOX_OUTBOX || mBoxId == Mms.MESSAGE_BOX_FAILED);
        boolean isOutgoingSms = isSms()
                                    && ((mBoxId == Sms.MESSAGE_TYPE_FAILED)
                                            || (mBoxId == Sms.MESSAGE_TYPE_OUTBOX)
                                            || (mBoxId == Sms.MESSAGE_TYPE_QUEUED));
        boolean isOutgoingIpMsg = mIpMessageItem == null ? false :
                                                            mIpMessageItem.isOutgoingMessage();
        return isOutgoingMms || isOutgoingSms || isOutgoingIpMsg;
    }

    public boolean isSending() {
        return !isFailedMessage() && isOutgoingMessage();
    }

    public boolean isFailedMessage() {
        boolean isFailedMms = isMms()
                            && mErrorType >= MmsSms.ERR_TYPE_GENERIC_PERMANENT;
        boolean isFailedSms = isSms()
                            && (mBoxId == Sms.MESSAGE_TYPE_FAILED);
        boolean isFailedIpMsg = mIpMessageItem == null ? false : mIpMessageItem.isFailedMessage();
        return isFailedMms || isFailedSms || isFailedIpMsg;
    }

    // Note: This is the only mutable field in this class.  Think of
    // mCachedFormattedMessage as a C++ 'mutable' field on a const
    // object, with this being a lazy accessor whose logic to set it
    // is outside the class for model/view separation reasons.  In any
    // case, please keep this class conceptually immutable.
    public void setCachedFormattedMessage(CharSequence formattedMessage) {
        mCachedFormattedMessage = formattedMessage;
    }

    public CharSequence getCachedFormattedMessage() {
        boolean isSending = isSending();
        if (isSending != mLastSendingState) {
            mLastSendingState = isSending;
            mCachedFormattedMessage = null;         // clear cache so we'll rebuild the message
                                                    // to show "Sending..." or the sent date.
        }
        return mCachedFormattedMessage;
    }

    public int getBoxId() {
        return mBoxId;
    }

    public long getMessageId() {
        return mMsgId;
    }

    public int getMmsDownloadStatus() {
        return mMmsStatus & ~DownloadManager.DEFERRED_MASK;
    }

    @Override
    public String toString() {
        //add for gemini
        return "type: " + mType + " box: " + mBoxId + " sub: " + mSubId + " uri: " + mMessageUri
                + " address: " + mAddress + " contact: " + mContact + " read: " + mReadReport
                + " delivery status: " + mDeliveryStatus
                + " subject:" + mSubject
                + " mBody: " + mBody;
    }


    public class PduLoadedMessageItemCallback implements ItemLoadedCallback {
        public void onItemLoaded(Object result, Throwable exception) {
            if (exception != null) {
                Log.e(TAG, "PduLoadedMessageItemCallback PDU couldn't be loaded: ", exception);
                return;
            }
            /// M: google jb.mr1 pathc, remove and fully reloaded the next time
            /// When a pdu or image is canceled during loading @{
            if (mItemLoadedFuture != null) {
                synchronized (mItemLoadedFuture) {
                    mItemLoadedFuture.setIsDone(true);
                }
            }
            /// @}
            PduLoaderManager.PduLoaded pduLoaded = (PduLoaderManager.PduLoaded) result;
            long timestamp = 0L;
            long indDate = 0L;
            if (MtkPduHeaders.MESSAGE_TYPE_NOTIFICATION_IND == mMessageType) {
                mDeliveryStatus = DeliveryStatus.NONE;
                NotificationInd notifInd = (NotificationInd) pduLoaded.mPdu;
                interpretFrom(notifInd.getFrom(), mMessageUri);
                // Borrow the mBody to hold the URL of the message.
                mBody = new String(notifInd.getContentLocation());
                mMessageSize = (int) notifInd.getMessageSize();
                timestamp = notifInd.getExpiry() * 1000L;
                indDate = mCursor.getLong(MessageListAdapter.COLUMN_DATE);
            } else {
                if (mCursor.isClosed()) {
                    return;
                }
                MtkMultimediaMessagePdu msg = (MtkMultimediaMessagePdu) pduLoaded.mPdu;
                mSlideshow = pduLoaded.mSlideshow;
                mAttachmentType = MessageUtils.getAttachmentType(mSlideshow, msg);

                if (mMessageType == MtkPduHeaders.MESSAGE_TYPE_RETRIEVE_CONF) {
                    if (msg == null) {
                        interpretFrom(null, mMessageUri);
                    } else {
                        RetrieveConf retrieveConf = (RetrieveConf) ((MultimediaMessagePdu) msg);
                        interpretFrom(retrieveConf.getFrom(), mMessageUri);
                        timestamp = retrieveConf.getDate() * 1000L;
                    }
                } else {
                    // Use constant string for outgoing messages
                    mContact = mAddress =
                            mContext.getString(R.string.messagelist_sender_self);
                    timestamp = msg == null ? 0 : ((SendReq)
                        ((MultimediaMessagePdu) msg)).getDate() * 1000L;
                }

                SlideModel slide = mSlideshow == null ? null : mSlideshow.get(0);
                if ((slide != null) && slide.hasText()) {
                    TextModel tm = slide.getText();
                    mBody = tm.getText();
                    mTextContentType = tm.getContentType();
                }

                mMessageSize = mSlideshow == null ? 0 : mSlideshow.getCurrentSlideshowSize();

                String report = mCursor.getString(MessageListAdapter.COLUMN_MMS_DELIVERY_REPORT);
                if ((report == null) || !mAddress.equals(mContext.getString(
                        R.string.messagelist_sender_self))) {
                    mDeliveryStatus = DeliveryStatus.NONE;
                } else {
                    int reportInt;
                    try {
                        reportInt = Integer.parseInt(report);
                        if (reportInt == MtkPduHeaders.VALUE_YES) {
                            mDeliveryStatus = DeliveryStatus.RECEIVED;
                        } else {
                            mDeliveryStatus = DeliveryStatus.NONE;
                        }
                    } catch (NumberFormatException nfe) {
                        Log.e(TAG, "Value for delivery report was invalid.");
                        mDeliveryStatus = DeliveryStatus.NONE;
                    }
                }

                report = mCursor.getString(MessageListAdapter.COLUMN_MMS_READ_REPORT);
                if ((report == null) || !mAddress.equals(mContext.getString(
                        R.string.messagelist_sender_self))) {
                    mReadReport = false;
                } else {
                    int reportInt;
                    try {
                        reportInt = Integer.parseInt(report);
                        mReadReport = (reportInt == MtkPduHeaders.VALUE_YES);
                    } catch (NumberFormatException nfe) {
                        Log.e(TAG, "Value for read report was invalid.");
                        mReadReport = false;
                    }
                }
            }
            if (!isOutgoingMessage()) {
                if (MtkPduHeaders.MESSAGE_TYPE_NOTIFICATION_IND == mMessageType) {
                    mTimestamp = mContext.getString(getTimestampStrId(), MessageUtils
                            .formatTimeStampString(mContext, timestamp));
                } else {
                    mTimestamp = mContext.getString(getTimestampStrId(),
                            MessageUtils.getShortTimeString(mContext, timestamp));
                }
            }

            if (mIsDrawTimeDivider) {
                if (MtkPduHeaders.MESSAGE_TYPE_NOTIFICATION_IND == mMessageType) {
                    mTimeDividerString =
                        MessageUtils.getTimeDividerString(mContext, indDate * 1000L);
                } else {
                    mTimeDividerString = MessageUtils.getTimeDividerString(mContext, timestamp);
                }
            }

            if (mPduLoadedCallback != null) {
                mPduLoadedCallback.onPduLoaded(MessageItem.this);
            }
        }
    }

    public void setOnPduLoaded(PduLoadedCallback pduLoadedCallback) {
        mPduLoadedCallback = pduLoadedCallback;
    }

    public void cancelPduLoading() {
        /// M: google jb.mr1 pathc, remove and fully reloaded the next time
        /// When a pdu or image is canceled during loading @{
        if (mItemLoadedFuture != null && !mItemLoadedFuture.isDone()) {
            if (Log.isLoggable(LogTag.APP, Log.DEBUG)) {
                Log.v(TAG, "cancelPduLoading for: " + this);
            }
            mItemLoadedFuture.cancel(mMessageUri);
            mItemLoadedFuture = null;
        }
        /// @}
    }

    public interface PduLoadedCallback {
        /**
         * Called when this item's pdu and slideshow are finished loading.
         *
         * @param messageItem the MessageItem that finished loading.
         */
        void onPduLoaded(MessageItem messageItem);
    }

    public SlideshowModel getSlideshow() {
        return mSlideshow;
    }

    /// M: @{
    public boolean hasDrmMedia() {
        return mSlideshow != null ? mSlideshow.hasDrmMedia() : false;
    }

    public boolean isSubMsg() {
        return mSubMsg;
    }

    public boolean isReceivedMessage() {
        boolean isReceivedMms = isMms() && (mBoxId == Mms.MESSAGE_BOX_INBOX);
        boolean isReceivedSms = isSms() && (mBoxId == Sms.MESSAGE_TYPE_INBOX);
        return isReceivedMms || isReceivedSms || (mBoxId == 0 && isSms());
    }

    public boolean isSentMessage() {
        boolean isSentMms = isMms() && (mBoxId == Mms.MESSAGE_BOX_SENT);
        boolean isSentSms = isSms() && (mBoxId == Sms.MESSAGE_TYPE_SENT);
        return isSentMms || isSentSms;
    }

    public void setCachedFormattedTimestamp(CharSequence formattedTimestamp) {
        mCachedFormattedTimestamp = formattedTimestamp;
    }

    public CharSequence getCachedFormattedTimestamp() {
        boolean isSending = isSending();
        if (isSending != mLastSendingState) {
            mLastSendingState = isSending;
            mCachedFormattedTimestamp = null;
        }
        return mCachedFormattedTimestamp;
    }

    public void setCachedFormattedSubStatus(CharSequence formattedSimStatus) {
        mCachedFormattedSubStatus = formattedSimStatus;
    }

    public CharSequence getCachedFormattedSubStatus() {
        boolean isSending = isSending();
        if (isSending != mLastSendingState) {
            mLastSendingState = isSending;
            mCachedFormattedSubStatus = null;
        }
        return mCachedFormattedSubStatus;
    }

    /// M: add for gemini
    public int getSubId() {
        return mSubId;
    }

    public boolean isSelected() {
        return mItemSelected;
    }

    public void setSelectedState(boolean isSelected) {
        mItemSelected = isSelected;
    }

    /// M: Add for vCard begin
    public int getFileAttachmentCount() {
        if (mSlideshow != null) {
            return mSlideshow.sizeOfFilesAttach();
        }
        return 0;
    }
    /// M: Add for vCard end

    public String getServiceCenter() {
        return mServiceCenter;
    }

    private int getTimestampStrId() {
        if (MtkPduHeaders.MESSAGE_TYPE_NOTIFICATION_IND == mMessageType) {
            return R.string.expire_on;
        } else {
            if (isReceivedMessage()) {
                return R.string.received_on;
            } else {
                return R.string.sent_on;
            }
        }
    }
    /// @}

    /// M: IOpMessageItemCallback @{
    public boolean isMmsCallback() {
        return isMms();
    }

    public boolean isDownloadedCallback() {
        return isDownloaded();
    }

    public String getType() {
        return mType;
    }

    public String getBody() {
        return mBody;
    }

    public int getBoxIdCallback() {
        return getBoxId();
    }

    public String getAddress() {
        return mAddress;
    }

    public int getMmsStatus() {
        return mMmsStatus;
    }

    public int getMessageType() {
        return mMessageType;
    }

    public boolean getIsSubMessage() {
        return mSubMsg;
    }

    public long getMessageDate() {
        return mSmsDate;
    }

    public Uri getMessageUri() {
        return mMessageUri;
    }

    public void setAddress(String address) {
        mAddress = address;
    }

    public void setSubId(int subId) {
        mSubId = subId;
    }

    public void setBody(String body) {
        mBody = body;
    }

    public void setUri(Uri uri) {
        mMessageUri = uri;
    }

    public void setTimeStamp(String timeStamp) {
        mTimestamp = timeStamp;
    }

    public void setLocked(boolean locked) {
        mLocked = locked;
    }

    public void setDeliveryStatus(Object status) {
        mDeliveryStatus = (DeliveryStatus) status;
    }

    public void setTimeDivider(String dividerString) {
        mTimeDividerString = dividerString;
    }

    // use the current message item and no need load form cursor again, then return true
    public boolean updateSmsItem(final Cursor c) {
        String type = c.getString(MessageListAdapter.COLUMN_MSG_TYPE);
        if (!type.equals("sms")) {
            return false;
        }
        if (mLocked != (c.getInt(MessageListAdapter.COLUMN_LOCKED) != 0)) {
            mLocked = (c.getInt(MessageListAdapter.COLUMN_LOCKED) != 0);
            return true;
        }
        if ((mSmsStatus == c.getInt(MessageListAdapter.COLUMN_SMS_STATUS)) &&
                (mSmsDate == c.getInt(MessageListAdapter.COLUMN_DATE)) &&
                (mErrorType == c.getInt(MessageListAdapter.COLUMN_SMS_ERROR_CODE))) {
            return true;
        }
        return false;
    }

    // use the current message item and no need load form cursor again, then return true
    public boolean updateMmsItem(final Cursor c) {
        String type = c.getString(MessageListAdapter.COLUMN_MSG_TYPE);
        if (!type.equals("mms")) {
            return false;
        }

        if (mLocked != (c.getInt(MessageListAdapter.COLUMN_LOCKED) != 0)) {
            mLocked = (c.getInt(MessageListAdapter.COLUMN_LOCKED) != 0);
            return true;
        }

        if ((mSubId != c.getInt(MessageListAdapter.COLUMN_SUBID)) ||
                (mMmsStatus != c.getInt(MessageListAdapter.COLUMN_MMS_STATUS)) ||
                (mBoxId != c.getInt(MessageListAdapter.COLUMN_MMS_MESSAGE_BOX)) ||
                (mErrorType != c.getInt(MessageListAdapter.COLUMN_MMS_ERROR_TYPE)) ||
                isDeliveryReportChanged(c) ||
                isSubjectChanged(c) ||
                isMmsSizeChanged(c)) {
            return false;
        }

        return true;
    }

    // mHasDeliveryReport: False --> True, then return true
    private boolean isDeliveryReportChanged(final Cursor c) {
        if (mBoxId == Mms.MESSAGE_BOX_SENT
                && (mReadReport || mDeliveryStatus == MessageItem.DeliveryStatus.RECEIVED)) {
            if (mHasDeliveryReport) {
                return false;
            }
            List<MmsReportStatus> mmsReportStatusList =
                    MessageUtils.getMmsReportStatus(mContext, mMsgId);
            if (mmsReportStatusList != null && mmsReportStatusList.size() > 0) {
                for (MmsReportStatus mmsReportStatus : mmsReportStatusList) {
                    if (mmsReportStatus.readStatus == MtkPduHeaders.READ_STATUS_READ) {
                        return true;
                    }
                    if (mmsReportStatus.deliveryStatus == MtkPduHeaders.STATUS_RETRIEVED) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    // mSubject: null --> unNull, then return true
    private boolean isSubjectChanged(final Cursor c) {
        String subject = c.getString(MessageListAdapter.COLUMN_MMS_SUBJECT);
        if (!TextUtils.isEmpty(subject) && TextUtils.isEmpty(mSubject)) {
            return true;
        }
        return false;
    }

    private boolean isMmsSizeChanged(final Cursor c) {
        if (mMmsSize != c.getInt(MessageListAdapter.COLUMN_MESSAGE_SIZE)) {
            return true;
        }
        return false;
    }
    /// end IOpMessageItemCallback @}
}
