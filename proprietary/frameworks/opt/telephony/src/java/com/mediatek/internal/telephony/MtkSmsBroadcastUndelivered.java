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


package com.mediatek.internal.telephony;

import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.database.SQLException;
import android.os.UserHandle;
import android.os.UserManager;
import android.telephony.Rlog;

import com.android.internal.telephony.SmsBroadcastUndelivered;
import com.android.internal.telephony.cdma.CdmaInboundSmsHandler;
import com.android.internal.telephony.gsm.GsmInboundSmsHandler;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.InboundSmsHandler;
import com.android.internal.telephony.InboundSmsTracker;
import com.android.internal.telephony.TelephonyComponentFactory;
import com.mediatek.internal.telephony.gsm.MtkGsmInboundSmsHandler;
import com.mediatek.internal.telephony.MtkInboundSmsTracker;
import com.mediatek.internal.telephony.util.MtkSmsCommonUtil;

import java.util.HashMap;
import java.util.HashSet;

// MTK-START
import android.telephony.TelephonyManager;
// MTK-END

/**
 * Called when the credential-encrypted storage is unlocked, collecting all acknowledged messages
 * and deleting any partial message segments older than 30 days. Called from a worker thread to
 * avoid delaying phone app startup. The last step is to broadcast the first pending message from
 * the main thread, then the remaining pending messages will be broadcast after the previous
 * ordered broadcast completes.
 */
public class MtkSmsBroadcastUndelivered extends SmsBroadcastUndelivered {
    private static final String TAG = "MtkSmsBroadcastUndelivered";

    /**
     * Query projection for dispatching pending messages at boot time.
     * Column order must match the {@code *_COLUMN} constants in {@link InboundSmsHandler}.
     */
    // MTK-START
    private static final String[] PDU_PENDING_MESSAGE_PROJECTION = {
            "pdu",
            "sequence",
            "destination_port",
            "date",
            "reference_number",
            "count",
            "address",
            "_id",
            "message_body",
            "display_originating_addr",
            "sub_id"
    };

    // Modify here for MSIM
    private static MtkSmsBroadcastUndelivered instance[] =
            new MtkSmsBroadcastUndelivered[TelephonyManager.getDefault().getPhoneCount()];
    // MTK-END

    // MTK-START
    private final Phone mPhone;
    // MTK-END

    public static void initialize(Context context, GsmInboundSmsHandler gsmInboundSmsHandler,
        CdmaInboundSmsHandler cdmaInboundSmsHandler) {
        // MTK-START
        // Modification here for MSIM
        int phoneId = gsmInboundSmsHandler.getPhone().getPhoneId();
        if (instance[phoneId] == null) {
            if (DBG) Rlog.d(TAG, "Phone " + phoneId + " call initialize");
            instance[phoneId] = new MtkSmsBroadcastUndelivered(
                    context, gsmInboundSmsHandler, cdmaInboundSmsHandler);
        }
        // MTK-END

        // Tell handlers to start processing new messages and transit from the startup state to the
        // idle state. This method may be called multiple times for multi-sim devices. We must make
        // sure the state transition happen to all inbound sms handlers.
        if (gsmInboundSmsHandler != null) {
            gsmInboundSmsHandler.sendMessage(InboundSmsHandler.EVENT_START_ACCEPTING_SMS);
        }
        if (cdmaInboundSmsHandler != null) {
            cdmaInboundSmsHandler.sendMessage(InboundSmsHandler.EVENT_START_ACCEPTING_SMS);
        }
    }

    private MtkSmsBroadcastUndelivered(Context context, GsmInboundSmsHandler gsmInboundSmsHandler,
            CdmaInboundSmsHandler cdmaInboundSmsHandler) {
        super(context, gsmInboundSmsHandler, cdmaInboundSmsHandler);

        // MTK-START
        mPhone = gsmInboundSmsHandler.getPhone();
        // MTK-END

        UserManager userManager = (UserManager) context.getSystemService(Context.USER_SERVICE);

        if (!userManager.isUserUnlocked()) {
            if (DBG) Rlog.d(TAG, "Phone " + mPhone.getPhoneId() + " register user unlock event");
        }
    }

    /**
     * Scan the raw table for complete SMS messages to broadcast, and old PDUs to delete.
     */
    public static void scanRawTable(Context context, CdmaInboundSmsHandler cdmaInboundSmsHandler,
            GsmInboundSmsHandler gsmInboundSmsHandler, long oldMessageTimestamp) {
        scanRawTable(context, false, cdmaInboundSmsHandler, gsmInboundSmsHandler,
                oldMessageTimestamp);
        scanRawTable(context, true, cdmaInboundSmsHandler, gsmInboundSmsHandler,
                oldMessageTimestamp);
    }

    private static void scanRawTable(Context context, boolean isCurrentFormat3gpp2,
            CdmaInboundSmsHandler cdmaInboundSmsHandler,
            GsmInboundSmsHandler gsmInboundSmsHandler,
            long oldMessageTimestamp) {
        if (DBG) Rlog.d(TAG, "scanning raw table for undelivered messages");
        long startTime = System.nanoTime();
        ContentResolver contentResolver = context.getContentResolver();
        HashMap<SmsReferenceKey, Integer> multiPartReceivedCount =
                new HashMap<SmsReferenceKey, Integer>(4);
        HashSet<SmsReferenceKey> oldMultiPartMessages = new HashSet<SmsReferenceKey>(4);
        Cursor cursor = null;
        try {
            // query only non-deleted ones
            String nonDeleteWhere = "deleted = 0"
                    + (isCurrentFormat3gpp2 ? MtkSmsCommonUtil.SQL_3GPP2_SMS :
                    MtkSmsCommonUtil.SQL_3GPP_SMS);
            cursor = contentResolver.query(InboundSmsHandler.sRawUri,
                    PDU_PENDING_MESSAGE_PROJECTION, nonDeleteWhere, null, null);
            if (cursor == null) {
                Rlog.e(TAG, "error getting pending message cursor");
                return;
            }

            while (cursor.moveToNext()) {
                MtkInboundSmsTracker tracker;
                try {
                    TelephonyComponentFactory telephonyComponentFactory = TelephonyComponentFactory
                            .getInstance().inject(TelephonyComponentFactory.class.getName());
                    tracker = (MtkInboundSmsTracker)telephonyComponentFactory.makeInboundSmsTracker(
                            cursor, isCurrentFormat3gpp2);
                } catch (IllegalArgumentException e) {
                    Rlog.e(TAG, "error loading SmsTracker: " + e);
                    continue;
                }

                if (tracker.getMessageCount() == 1) {
                    // deliver single-part message
                    // MTK-START
                    // Check the Sub id if the same
                    // TODO: Wait for SmsBroadcastUndeliveredEx
                    if (tracker.getSubId() == gsmInboundSmsHandler.getPhone().getSubId()) {
                        if (DBG) Rlog.d(TAG, "New sms on raw table, subId: " + tracker.getSubId());
                        broadcastSms(tracker, cdmaInboundSmsHandler, gsmInboundSmsHandler);
                    }
                    // MTK-END
                } else {
                    SmsReferenceKey reference = new SmsReferenceKey(tracker);
                    Integer receivedCount = multiPartReceivedCount.get(reference);
                    if (receivedCount == null) {
                        multiPartReceivedCount.put(reference, 1);    // first segment seen
                        if (tracker.getTimestamp() < oldMessageTimestamp) {
                            // older than oldMessageTimestamp; delete if we don't find all the
                            // segments
                            oldMultiPartMessages.add(reference);
                        }
                    } else {
                        int newCount = receivedCount + 1;
                        if (newCount == tracker.getMessageCount()) {
                            // looks like we've got all the pieces; send a single tracker
                            // to state machine which will find the other pieces to broadcast
                            if (DBG) Rlog.d(TAG, "found complete multi-part message");
                            // MTK-START
                            // Check the Sub id if the same
                            if (tracker.getSubId() == gsmInboundSmsHandler.getPhone().getSubId()) {
                                if (DBG) Rlog.d(TAG, "New sms on raw table, subId: " +
                                        tracker.getSubId());
                                broadcastSms(tracker, cdmaInboundSmsHandler, gsmInboundSmsHandler);
                            }
                            // MTK-END
                            // don't delete this old message until after we broadcast it
                            oldMultiPartMessages.remove(reference);
                        } else {
                            multiPartReceivedCount.put(reference, newCount);
                        }
                    }
                }
            }
            // Delete old incomplete message segments
            for (SmsReferenceKey message : oldMultiPartMessages) {
                // delete permanently
                String where = MtkSmsCommonUtil.SELECT_BY_REFERENCE
                        + (isCurrentFormat3gpp2 ? MtkSmsCommonUtil.SQL_3GPP2_SMS :
                        MtkSmsCommonUtil.SQL_3GPP_SMS);
                int rows = contentResolver.delete(InboundSmsHandler.sRawUriPermanentDelete,
                        where, message.getDeleteWhereArgs());
                if (rows == 0) {
                    Rlog.e(TAG, "No rows were deleted from raw table!");
                } else if (DBG) {
                    Rlog.d(TAG, "Deleted " + rows + " rows from raw table for incomplete "
                            + message.mMessageCount + " part message");
                }
            }
        } catch (SQLException e) {
            Rlog.e(TAG, "error reading pending SMS messages", e);
        } finally {
            if (cursor != null) {
                cursor.close();
            }
            if (DBG) Rlog.d(TAG, "finished scanning raw table in "
                    + ((System.nanoTime() - startTime) / 1000000) + " ms");
        }
    }

    /**
     * Send tracker to appropriate (3GPP or 3GPP2) inbound SMS handler for broadcast.
     */
    private static void broadcastSms(InboundSmsTracker tracker,
            CdmaInboundSmsHandler cdmaInboundSmsHandler,
            GsmInboundSmsHandler gsmInboundSmsHandler) {
        InboundSmsHandler handler;
        if (tracker.is3gpp2()) {
            handler = cdmaInboundSmsHandler;
        } else {
            handler = gsmInboundSmsHandler;
        }
        if (handler != null) {
            handler.sendMessage(InboundSmsHandler.EVENT_BROADCAST_SMS, tracker);
        } else {
            Rlog.e(TAG, "null handler for " + tracker.getFormat() + " format, can't deliver.");
        }
    }

    /**
     * Used as the HashMap key for matching concatenated message segments.
     */
    private static class SmsReferenceKey {
        final String mAddress;
        final int mReferenceNumber;
        final int mMessageCount;
        // MTK-START
        final long mSubId;
        // MTK-END

        SmsReferenceKey(MtkInboundSmsTracker tracker) {
            mAddress = tracker.getAddress();
            mReferenceNumber = tracker.getReferenceNumber();
            mMessageCount = tracker.getMessageCount();
            // MTK-START
            mSubId = tracker.getSubId();
            // MTK-END
        }

        String[] getDeleteWhereArgs() {
            // MTK-START
            return new String[]{mAddress, Integer.toString(mReferenceNumber),
                    Integer.toString(mMessageCount), Long.toString(mSubId)};
            // MTK-END
        }

        @Override
        public int hashCode() {
            // MTK-START
            return ((((int) mSubId * 63) + mReferenceNumber * 31) + mMessageCount) * 31 +
                    mAddress.hashCode();
            // MTK-END
        }

        @Override
        public boolean equals(Object o) {
            if (o instanceof SmsReferenceKey) {
                SmsReferenceKey other = (SmsReferenceKey) o;
                // MTK-START
                return other.mAddress.equals(mAddress)
                        && (other.mReferenceNumber == mReferenceNumber)
                        && (other.mMessageCount == mMessageCount)
                        && (other.mSubId == mSubId);
                // MTK-END
            }
            return false;
        }
    }
}
