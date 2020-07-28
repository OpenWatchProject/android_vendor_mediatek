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

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.os.Parcel;
import android.os.Parcelable;
import android.provider.Telephony;
import android.text.format.DateUtils;

import android.telephony.SmsCbMessage;
import android.telephony.CellBroadcastMessage;
import android.telephony.SmsCbLocation;
import android.telephony.SmsCbEtwsInfo;
import android.telephony.SmsCbCmasInfo;

import com.mediatek.internal.telephony.gsm.cbutil.Shape;
import com.mediatek.internal.telephony.gsm.cbutil.WhamTuple;
import com.mediatek.internal.telephony.gsm.MtkGsmSmsCbMessage;
import com.mediatek.internal.telephony.MtkSmsCbMessage;

import java.util.ArrayList;

import mediatek.telephony.MtkTelephony;

/**
 * Application wrapper for {@link SmsCbMessage}. This is Parcelable so that
 * decoded broadcast message objects can be passed between running Services.
 * New broadcasts are received by the CellBroadcastReceiver app, which exports
 * the database of previously received broadcasts at "content://cellbroadcasts/".
 * The "android.permission.READ_CELL_BROADCASTS" permission is required to read
 * from the ContentProvider, and writes to the database are not allowed.<p>
 *
 * Use {@link #createFromCursor} to create CellBroadcastMessage objects from rows
 * in the database cursor returned by the ContentProvider.
 *
 * {@hide}
 */
public class MtkCellBroadcastMessage extends CellBroadcastMessage {
    private MtkCellBroadcastMessage(int subId, SmsCbMessage message, long deliveryTime,
            boolean isRead) {
        super();
        mSubId = subId;
        mSmsCbMessage = message;
        mDeliveryTime = deliveryTime;
        mIsRead = isRead;
    }

    public MtkCellBroadcastMessage(SmsCbMessage message) {
        super(message);
    }

    private MtkCellBroadcastMessage(Parcel in) {
        mSmsCbMessage = new MtkSmsCbMessage(in);
        mDeliveryTime = in.readLong();
        mIsRead = (in.readInt() != 0);
        mSubId = in.readInt();
    }

    public static final Parcelable.Creator<MtkCellBroadcastMessage> CREATOR
            = new Parcelable.Creator<MtkCellBroadcastMessage>() {
        @Override
        public MtkCellBroadcastMessage createFromParcel(Parcel in) {
            return new MtkCellBroadcastMessage(in);
          }

        @Override
        public MtkCellBroadcastMessage[] newArray(int size) {
            return new MtkCellBroadcastMessage[size];
        }
    };

    /**
     * Create a CellBroadcastMessage from a row in the database.
     * @param cursor an open SQLite cursor pointing to the row to read
     * @return the new CellBroadcastMessage
     * @throws IllegalArgumentException if one of the required columns is missing
     */
    public static MtkCellBroadcastMessage createFromCursor(Cursor cursor) {
        int geoScope = cursor.getInt(
                cursor.getColumnIndexOrThrow(Telephony.CellBroadcasts.GEOGRAPHICAL_SCOPE));
        int serialNum = cursor.getInt(
                cursor.getColumnIndexOrThrow(Telephony.CellBroadcasts.SERIAL_NUMBER));
        int category = cursor.getInt(
                cursor.getColumnIndexOrThrow(Telephony.CellBroadcasts.SERVICE_CATEGORY));
        String language = cursor.getString(
                cursor.getColumnIndexOrThrow(Telephony.CellBroadcasts.LANGUAGE_CODE));
        String body = cursor.getString(
                cursor.getColumnIndexOrThrow(Telephony.CellBroadcasts.MESSAGE_BODY));
        int format = cursor.getInt(
                cursor.getColumnIndexOrThrow(Telephony.CellBroadcasts.MESSAGE_FORMAT));
        int priority = cursor.getInt(
                cursor.getColumnIndexOrThrow(Telephony.CellBroadcasts.MESSAGE_PRIORITY));

        String plmn;
        int plmnColumn = cursor.getColumnIndex(Telephony.CellBroadcasts.PLMN);
        if (plmnColumn != -1 && !cursor.isNull(plmnColumn)) {
            plmn = cursor.getString(plmnColumn);
        } else {
            plmn = null;
        }

        int lac;
        int lacColumn = cursor.getColumnIndex(Telephony.CellBroadcasts.LAC);
        if (lacColumn != -1 && !cursor.isNull(lacColumn)) {
            lac = cursor.getInt(lacColumn);
        } else {
            lac = -1;
        }

        int cid;
        int cidColumn = cursor.getColumnIndex(Telephony.CellBroadcasts.CID);
        if (cidColumn != -1 && !cursor.isNull(cidColumn)) {
            cid = cursor.getInt(cidColumn);
        } else {
            cid = -1;
        }

        SmsCbLocation location = new SmsCbLocation(plmn, lac, cid);

        SmsCbEtwsInfo etwsInfo;
        int etwsWarningTypeColumn = cursor.getColumnIndex(
                Telephony.CellBroadcasts.ETWS_WARNING_TYPE);
        if (etwsWarningTypeColumn != -1 && !cursor.isNull(etwsWarningTypeColumn)) {
            int warningType = cursor.getInt(etwsWarningTypeColumn);
            etwsInfo = new SmsCbEtwsInfo(warningType, false, false, false, null);
        } else {
            etwsInfo = null;
        }

        SmsCbCmasInfo cmasInfo;
        int cmasMessageClassColumn = cursor.getColumnIndex(
                Telephony.CellBroadcasts.CMAS_MESSAGE_CLASS);
        if (cmasMessageClassColumn != -1 && !cursor.isNull(cmasMessageClassColumn)) {
            int messageClass = cursor.getInt(cmasMessageClassColumn);

            int cmasCategory;
            int cmasCategoryColumn = cursor.getColumnIndex(
                    Telephony.CellBroadcasts.CMAS_CATEGORY);
            if (cmasCategoryColumn != -1 && !cursor.isNull(cmasCategoryColumn)) {
                cmasCategory = cursor.getInt(cmasCategoryColumn);
            } else {
                cmasCategory = SmsCbCmasInfo.CMAS_CATEGORY_UNKNOWN;
            }

            int responseType;
            int cmasResponseTypeColumn = cursor.getColumnIndex(
                    Telephony.CellBroadcasts.CMAS_RESPONSE_TYPE);
            if (cmasResponseTypeColumn != -1 && !cursor.isNull(cmasResponseTypeColumn)) {
                responseType = cursor.getInt(cmasResponseTypeColumn);
            } else {
                responseType = SmsCbCmasInfo.CMAS_RESPONSE_TYPE_UNKNOWN;
            }

            int severity;
            int cmasSeverityColumn = cursor.getColumnIndex(
                    Telephony.CellBroadcasts.CMAS_SEVERITY);
            if (cmasSeverityColumn != -1 && !cursor.isNull(cmasSeverityColumn)) {
                severity = cursor.getInt(cmasSeverityColumn);
            } else {
                severity = SmsCbCmasInfo.CMAS_SEVERITY_UNKNOWN;
            }

            int urgency;
            int cmasUrgencyColumn = cursor.getColumnIndex(
                    Telephony.CellBroadcasts.CMAS_URGENCY);
            if (cmasUrgencyColumn != -1 && !cursor.isNull(cmasUrgencyColumn)) {
                urgency = cursor.getInt(cmasUrgencyColumn);
            } else {
                urgency = SmsCbCmasInfo.CMAS_URGENCY_UNKNOWN;
            }

            int certainty;
            int cmasCertaintyColumn = cursor.getColumnIndex(
                    Telephony.CellBroadcasts.CMAS_CERTAINTY);
            if (cmasCertaintyColumn != -1 && !cursor.isNull(cmasCertaintyColumn)) {
                certainty = cursor.getInt(cmasCertaintyColumn);
            } else {
                certainty = SmsCbCmasInfo.CMAS_CERTAINTY_UNKNOWN;
            }

            cmasInfo = new MtkSmsCbCmasInfo(messageClass, cmasCategory, responseType, severity,
                    urgency, certainty, 0L);
        } else {
            cmasInfo = null;
        }

        byte[] wac = cursor.getBlob(
                cursor.getColumnIndexOrThrow(MtkTelephony.MtkCellBroadcasts.WAC));

        MtkSmsCbMessage msg = new MtkSmsCbMessage(format, geoScope, serialNum, location, category,
                language, body, priority, etwsInfo, cmasInfo, wac);

        long deliveryTime = cursor.getLong(cursor.getColumnIndexOrThrow(
                Telephony.CellBroadcasts.DELIVERY_TIME));
        boolean isRead = (cursor.getInt(cursor.getColumnIndexOrThrow(
                Telephony.CellBroadcasts.MESSAGE_READ)) != 0);

        int subId = cursor.getInt(
                cursor.getColumnIndexOrThrow(MtkTelephony.MtkCellBroadcasts.SUBSCRIPTION_ID));

        return new MtkCellBroadcastMessage(subId, msg, deliveryTime, isRead);
    }

    /**
     * Return a ContentValues object for insertion into the database.
     * @return a new ContentValues object containing this object's data
     */
    public ContentValues getContentValues() {
        // MTK-START
        ContentValues cv = new ContentValues(17);
        // MTK-END
        MtkSmsCbMessage msg = (MtkSmsCbMessage)mSmsCbMessage;
        cv.put(Telephony.CellBroadcasts.GEOGRAPHICAL_SCOPE, msg.getGeographicalScope());
        SmsCbLocation location = msg.getLocation();
        if (location.getPlmn() != null) {
            cv.put(Telephony.CellBroadcasts.PLMN, location.getPlmn());
        }
        if (location.getLac() != -1) {
            cv.put(Telephony.CellBroadcasts.LAC, location.getLac());
        }
        if (location.getCid() != -1) {
            cv.put(Telephony.CellBroadcasts.CID, location.getCid());
        }
        cv.put(Telephony.CellBroadcasts.SERIAL_NUMBER, msg.getSerialNumber());
        cv.put(Telephony.CellBroadcasts.SERVICE_CATEGORY, msg.getServiceCategory());
        cv.put(Telephony.CellBroadcasts.LANGUAGE_CODE, msg.getLanguageCode());
        cv.put(Telephony.CellBroadcasts.MESSAGE_BODY, msg.getMessageBody());
        cv.put(Telephony.CellBroadcasts.DELIVERY_TIME, mDeliveryTime);
        cv.put(Telephony.CellBroadcasts.MESSAGE_READ, mIsRead);
        cv.put(Telephony.CellBroadcasts.MESSAGE_FORMAT, msg.getMessageFormat());
        cv.put(Telephony.CellBroadcasts.MESSAGE_PRIORITY, msg.getMessagePriority());

        SmsCbEtwsInfo etwsInfo = mSmsCbMessage.getEtwsWarningInfo();
        if (etwsInfo != null) {
            cv.put(Telephony.CellBroadcasts.ETWS_WARNING_TYPE, etwsInfo.getWarningType());
        }

        SmsCbCmasInfo cmasInfo = mSmsCbMessage.getCmasWarningInfo();
        if (cmasInfo != null) {
            cv.put(Telephony.CellBroadcasts.CMAS_MESSAGE_CLASS, cmasInfo.getMessageClass());
            cv.put(Telephony.CellBroadcasts.CMAS_CATEGORY, cmasInfo.getCategory());
            cv.put(Telephony.CellBroadcasts.CMAS_RESPONSE_TYPE, cmasInfo.getResponseType());
            cv.put(Telephony.CellBroadcasts.CMAS_SEVERITY, cmasInfo.getSeverity());
            cv.put(Telephony.CellBroadcasts.CMAS_URGENCY, cmasInfo.getUrgency());
            cv.put(Telephony.CellBroadcasts.CMAS_CERTAINTY, cmasInfo.getCertainty());
        }

        cv.put(MtkTelephony.MtkCellBroadcasts.SUBSCRIPTION_ID, mSubId);

        cv.put(MtkTelephony.MtkCellBroadcasts.WAC, msg.getWac());

        return cv;
    }

    public ArrayList<Shape> getWacResult() {
        MtkSmsCbMessage msg = (MtkSmsCbMessage)mSmsCbMessage;
        return MtkGsmSmsCbMessage.parseWac(msg);
    }

    public int getMaxWaitTime() {
        MtkSmsCbMessage msg = (MtkSmsCbMessage)mSmsCbMessage;
        return msg.getMaxWaitTime();
    }

    public ArrayList<ArrayList<WhamTuple>> getWHAMTupleList() {
        MtkSmsCbMessage msg = (MtkSmsCbMessage)mSmsCbMessage;
        return MtkGsmSmsCbMessage.parseWHAMTupleList(msg);
    }
}
