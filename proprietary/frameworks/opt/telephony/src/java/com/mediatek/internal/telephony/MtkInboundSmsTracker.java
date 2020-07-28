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

import android.content.ContentValues;
import android.database.Cursor;
import android.util.Pair;

import com.android.internal.util.HexDump;
import com.android.internal.telephony.InboundSmsHandler;
import com.android.internal.telephony.InboundSmsTracker;

import com.mediatek.internal.telephony.util.MtkSmsCommonUtil;

import java.util.Arrays;
import java.util.ArrayList;
import java.util.List;

/**
 * Tracker for an incoming SMS message ready to broadcast to listeners.
 * This is similar to {@link com.android.internal.telephony.SMSDispatcher.SmsTracker} used for
 * outgoing messages.
 */
public class MtkInboundSmsTracker extends InboundSmsTracker {
    // The number must be the same as the order of PDU_PENDING_MESSAGE_PROJECTION in
    // the class SmsBroadcastUndelivered
    public static final int SUB_ID_COLUMN = 10;

    /** Field for subscription */
    private int mSubId;

    /** Receiving time */
    private long mRecvTime;

    /** upload flag for CT requirement */
    private int mUploadFlag;

    /**
     * Create a tracker for a single-part SMS.
     *
     * @param pdu the message PDU
     * @param timestamp the message timestamp
     * @param destPort the destination port
     * @param is3gpp2 true for 3GPP2 format; false for 3GPP format
     * @param is3gpp2WapPdu true for 3GPP2 format WAP PDU; false otherwise
     * @param address originating address
     * @param displayAddress email address if this message was from an email gateway, otherwise same
     *                       as originating address
     */
    public MtkInboundSmsTracker(byte[] pdu, long timestamp, int destPort, boolean is3gpp2,
            boolean is3gpp2WapPdu, String address, String displayAddress, String messageBody,
            boolean isClass0) {
        super(pdu, timestamp, destPort, is3gpp2, is3gpp2WapPdu, address, displayAddress,
                messageBody, isClass0);
    }

    /**
     * Create a tracker for a multi-part SMS. Sequence numbers start at 1 for 3GPP and regular
     * concatenated 3GPP2 messages, but CDMA WAP push sequence numbers start at 0. The caller will
     * subtract 1 if necessary so that the sequence number is always 0-based. When loading and
     * saving to the raw table, the sequence number is adjusted if necessary for backwards
     * compatibility.
     *
     * @param pdu the message PDU
     * @param timestamp the message timestamp
     * @param destPort the destination port
     * @param is3gpp2 true for 3GPP2 format; false for 3GPP format
     * @param address originating address, or email if this message was from an email gateway
     * @param displayAddress email address if this message was from an email gateway, otherwise same
     *                       as originating address
     * @param referenceNumber the concatenated reference number
     * @param sequenceNumber the sequence number of this segment (0-based)
     * @param messageCount the total number of segments
     * @param is3gpp2WapPdu true for 3GPP2 format WAP PDU; false otherwise
     */
    public MtkInboundSmsTracker(byte[] pdu, long timestamp, int destPort, boolean is3gpp2,
            String address, String displayAddress, int referenceNumber, int sequenceNumber,
            int messageCount, boolean is3gpp2WapPdu, String messageBody, boolean isClass0) {
        super(pdu, timestamp, destPort, is3gpp2, address, displayAddress, referenceNumber,
                sequenceNumber, messageCount, is3gpp2WapPdu, messageBody, isClass0);
    }

    /**
     * Create a new tracker from the row of the raw table pointed to by Cursor.
     * Since this constructor is used only for recovery during startup, the Dispatcher is null.
     * @param cursor a Cursor pointing to the row to construct this SmsTracker for
     */
    public MtkInboundSmsTracker(Cursor cursor, boolean isCurrentFormat3gpp2) {
        super(cursor, isCurrentFormat3gpp2);
        mSubId = cursor.getInt(SUB_ID_COLUMN);
        if (cursor.getInt(InboundSmsHandler.COUNT_COLUMN) != 1) {
            setDeleteWhere(mDeleteWhere, mDeleteWhereArgs);
        }
    }

    @Override
    public ContentValues getContentValues() {
        ContentValues values = super.getContentValues();

        values.put("sub_id", mSubId);

        return values;
    }

    public boolean is3gpp2WapPdu() {
        return mIs3gpp2WapPdu;
    }

    public int getSubId() {
        return mSubId;
    }

    public void setSubId(int subId) {
        mSubId = subId;
    }

    public int getDestPort() {
        return mDestPort;
    }

    @Override
    public String getQueryForSegments() {
        return super.getQueryForSegments() + " AND sub_id=?"
                    + (is3gpp2() ? MtkSmsCommonUtil.SQL_3GPP2_SMS :
                    MtkSmsCommonUtil.SQL_3GPP_SMS);
    }

    @Override
    public void setDeleteWhere(String deleteWhere, String[] deleteWhereArgs) {
        if (getMessageCount() == 1) {
            super.setDeleteWhere(deleteWhere, deleteWhereArgs);
        } else {
            Pair<String, String[]> pair = appendSubIdInQuery(null, deleteWhereArgs);
            super.setDeleteWhere(deleteWhere, pair.second);
        }
    }


    @Override
    public Pair<String, String[]> getExactMatchDupDetectQuery() {
        return appendSubIdInQuery(super.getExactMatchDupDetectQuery());
    }

    @Override
    public Pair<String, String[]> getInexactMatchDupDetectQuery() {
        return appendSubIdInQuery(super.getInexactMatchDupDetectQuery());
    }

    private Pair<String, String[]> appendSubIdInQuery(Pair<String, String[]> base) {
        if (base == null) {
            return null;
        }
        return appendSubIdInQuery(base.first, base.second);
    }

    private Pair<String, String[]> appendSubIdInQuery(final String where,
            final String[] whereArgs) {
        String newWhere = (where == null) ? null: (where +  " AND sub_id=?"
                    + (is3gpp2() ? MtkSmsCommonUtil.SQL_3GPP2_SMS :
                    MtkSmsCommonUtil.SQL_3GPP_SMS));
        List<String > baseWhereArgs = new ArrayList<String>(Arrays.asList(whereArgs));
        baseWhereArgs.add(Integer.toString(mSubId));
        String[] newWhereArgs =  new String[baseWhereArgs.size()];
        baseWhereArgs.toArray(newWhereArgs);
        return new Pair<>(newWhere, newWhereArgs);
    }
}
