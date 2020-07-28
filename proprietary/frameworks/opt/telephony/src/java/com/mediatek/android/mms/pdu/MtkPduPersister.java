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
package com.mediatek.android.mms.pdu;

import com.google.android.mms.pdu.PduPersister;
import android.content.Context;
import android.net.Uri;
import java.util.Map;
import com.google.android.mms.pdu.PduBody;
import com.google.android.mms.pdu.PduPart;
import com.google.android.mms.pdu.PduHeaders;
import com.google.android.mms.util.PduCacheEntry;
import com.google.android.mms.pdu.MultimediaMessagePdu;
import java.util.HashMap;
import java.io.InputStream;
import com.google.android.mms.MmsException;
import com.google.android.mms.util.SqliteWrapper;
import android.util.Log;
import java.util.ArrayList;
import android.provider.Telephony.Mms.Part;
import android.database.DatabaseUtils;
import android.content.ContentUris;
import android.provider.Telephony.Mms;
import android.provider.Telephony.MmsSms;
import android.database.Cursor;
import android.content.ContentValues;
import java.util.HashSet;
import android.provider.Telephony.Threads;
import android.telephony.SubscriptionManager;
import android.telephony.PhoneNumberUtils;
import android.provider.Telephony.MmsSms.PendingMessages;
import com.google.android.mms.pdu.EncodedStringValue;
import com.google.android.mms.pdu.SendReq;
import com.google.android.mms.pdu.GenericPdu;

import com.google.android.mms.ContentType;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
/*[MTK MMS FW] OP01*/
import com.google.android.mms.pdu.AcknowledgeInd;
import com.google.android.mms.pdu.DeliveryInd;
import com.google.android.mms.pdu.NotificationInd;
import com.google.android.mms.pdu.NotifyRespInd;
import com.google.android.mms.pdu.PduHeaders;
import com.google.android.mms.pdu.PduPart;
import com.google.android.mms.pdu.ReadOrigInd;
import com.google.android.mms.pdu.ReadRecInd;
import mediatek.telephony.MtkTelephony.MtkThreads;
import java.util.Map.Entry;
import java.util.Set;

public class MtkPduPersister extends PduPersister {
    private static final String TAG = "MtkPduPersister";
    protected static MtkPduPersister sPersister;

    /*[MTK MMS FW] OP01*/
    //private static PduPersister sPersister;
    private static final String[] PDU_PROJECTION = new String[] {
        Mms._ID,
        Mms.MESSAGE_BOX,
        Mms.THREAD_ID,
        Mms.RETRIEVE_TEXT,
        Mms.SUBJECT,
        Mms.CONTENT_LOCATION,
        Mms.CONTENT_TYPE,
        Mms.MESSAGE_CLASS,
        Mms.MESSAGE_ID,
        Mms.RESPONSE_TEXT,
        Mms.TRANSACTION_ID,
        Mms.CONTENT_CLASS,
        Mms.DELIVERY_REPORT,
        Mms.MESSAGE_TYPE,
        Mms.MMS_VERSION,
        Mms.PRIORITY,
        Mms.READ_REPORT,
        Mms.READ_STATUS,
        Mms.REPORT_ALLOWED,
        Mms.RETRIEVE_STATUS,
        Mms.STATUS,
        Mms.DATE,
        Mms.DELIVERY_TIME,
        Mms.EXPIRY,
        Mms.MESSAGE_SIZE,
        Mms.SUBJECT_CHARSET,
        Mms.RETRIEVE_TEXT_CHARSET,
        /// M:Code analyze 001,add a new read field for query @{
        Mms.READ,
        /// @}
        /// M: add for saving sent time of received messages.
        Mms.DATE_SENT
    };

    private static final int PDU_COLUMN_READ                  = 27;
    /// M: add for saving sent time of received messages.
    private static final int PDU_COLUMN_DATE_SENT             = 28;
    /*[MTK MMS FW] OP01 end*/

    protected MtkPduPersister(Context context){
        super(context);
        /*[MTK MMS FW] OP01*/
        if (LONG_COLUMN_INDEX_MAP != null) {
            //Log.v(TAG, "LONG_COLUMN_INDEX_MAP add DATE_SENT (" + MtkPduHeaders.DATE_SENT + ", "
            //      + PDU_COLUMN_DATE_SENT + ")");
            LONG_COLUMN_INDEX_MAP.put(MtkPduHeaders.DATE_SENT, PDU_COLUMN_DATE_SENT);
        }
        if (LONG_COLUMN_NAME_MAP != null) {
            //Log.v(TAG, "LONG_COLUMN_NAME_MAP add DATE_SENT (" + MtkPduHeaders.DATE_SENT + ", "
            //      + Mms.DATE_SENT + ")");
            LONG_COLUMN_NAME_MAP.put(MtkPduHeaders.DATE_SENT, Mms.DATE_SENT);
        }
        /*[MTK MMS FW] OP01 end*/
    }

    /** Get(or create if not exist) an instance of PduPersister */
    public static synchronized MtkPduPersister getPduPersister(Context context) {
        Log.d(TAG, "getPduPersister context = " + context);
        if ((sPersister == null)) {
            sPersister = new MtkPduPersister(context);
        } else if (!context.equals(sPersister.mContext)) {
            sPersister.release();
            sPersister = new MtkPduPersister(context);
        }
        Log.d(TAG, "releaseOwnPduPersister return " + sPersister);
        return (MtkPduPersister)sPersister;
    }

    public static synchronized void releaseOwnPduPersister(Context context) {
        Log.d(TAG, "releaseOwnPduPersister context = " + context +
            ", sPersister = " + sPersister);
        if ((sPersister != null) && (sPersister.mContext != null) &&
            context.equals(sPersister.mContext)) {
            Log.d(TAG, "releaseOwnPduPersister it is my own");
            sPersister.release();
            sPersister = null;
        }
        Log.d(TAG, "releaseOwnPduPersister leave syncObj");
    }

//   @Override
   /**
     * Update all parts of a PDU.
     *
     * @param uri The PDU which need to be updated.
     * @param body New message body of the PDU.
     * @param preOpenedFiles if not null, a map of preopened InputStreams for the parts.
     * @throws MmsException Bad URI or updating failed.
     */
     /*
    public void updateParts(Uri uri, PduBody body, HashMap<Uri, InputStream> preOpenedFiles)
            throws MmsException {
        try {
            PduCacheEntry cacheEntry;
            synchronized(PDU_CACHE_INSTANCE) {
                if (PDU_CACHE_INSTANCE.isUpdating(uri)) {
                    if (LOCAL_LOGV) {
                        Log.v(TAG, "MtkupdateParts: " + uri + " blocked by isUpdating()");
                    }
                    try {
                        PDU_CACHE_INSTANCE.wait();
                    } catch (InterruptedException e) {
                        Log.e(TAG, "MtkupdateParts: ", e);
                    }
                    cacheEntry = PDU_CACHE_INSTANCE.get(uri);
                    if (cacheEntry != null) {
                        ((MultimediaMessagePdu) cacheEntry.getPdu()).setBody(body);
                    }
                }
                // Tell the cache to indicate to other callers that this item
                // is currently being updated.
                PDU_CACHE_INSTANCE.setUpdating(uri, true);
            }

            ArrayList<? extends PduPart> toBeCreated = new ArrayList<MtkPduPart>();
            HashMap<Uri,PduPart> toBeUpdated = new HashMap<Uri, PduPart>();

            int partsNum = body.getPartsNum();
            StringBuilder filter = new StringBuilder().append('(');
            int skippedCount = 0;
            int updateCount = 0;
            for (int i = 0; i < partsNum; i++) {
                PduPart part = (PduPart)body.getPart(i);
                Uri partUri = part.getDataUri();
                if ((partUri == null) || !partUri.getAuthority().startsWith("mms")) {
                    toBeCreated.add(part);
                    updateCount++;
                } else {
                    MtkPduPart mtkPart = (MtkPduPart)part;
                    /// M: If part is no need to update, then continue. @{
                    if (mtkPart.needUpdate()) {
                        toBeUpdated.put(partUri, part);
                        updateCount++;
                    } else {
                        skippedCount++;
                    }
                    /// @}

                    // Don't use 'i > 0' to determine whether we should append
                    // 'AND' since 'i = 0' may be skipped in another branch.
                    if (filter.length() > 1) {
                        filter.append(" AND ");
                    }

                    filter.append(Part._ID);
                    filter.append("!=");
                    DatabaseUtils.appendEscapedSQLString(filter, partUri.getLastPathSegment());
                }
            }
            filter.append(')');
            //Log.d(TAG, "skipped count: " + skippedCount + ", updateCount: " + updateCount);

            long msgId = ContentUris.parseId(uri);

            // Remove the parts which doesn't exist anymore.
            SqliteWrapper.delete(mContext, mContentResolver,
                    Uri.parse(Mms.CONTENT_URI + "/" + msgId + "/part"),
                    filter.length() > 2 ? filter.toString() : null, null);

            // Create new parts which didn't exist before.
            for (PduPart part : toBeCreated) {
                 MtkPduPart partmtk = (MtkPduPart)part;
                persistPart(partmtk, msgId, preOpenedFiles);
            }
            // Update the modified parts.
            for (Map.Entry<Uri,PduPart> e : toBeUpdated.entrySet()) {
                updatePart(e.getKey(), e.getValue(), preOpenedFiles);
            }
        } finally {
            synchronized(PDU_CACHE_INSTANCE) {
                PDU_CACHE_INSTANCE.setUpdating(uri, false);
                PDU_CACHE_INSTANCE.notifyAll();
            }
        }
    }
    */

     /// M: Add for a bug
    // Function getByteArrayFromPartColumn will call getBytes to return a byte[],
    // but in getBytes, it will encode a string into ISO_8859_1 charset.
    // However, "filename" and "name" will save to database in utf-8 charset, so
    // if loading it to PDU, we should encode it into utf-8 but not ISO_8859_1.

    private byte[] getByteArrayFromPartColumn2(Cursor c, int columnIndex) {
        if (!c.isNull(columnIndex)) {
            return c.getString(columnIndex).getBytes();
        }
        return null;
    }

  @Override
  protected void persistAddress(
            long msgId, int type, EncodedStringValue[] array) {

        /// M fix bug:ALPS00598507 The text content will disappear after enter the draft again @{
        ArrayList<String> strValues = new ArrayList<String>();
        ContentValues values = new ContentValues();
        Uri uri = Uri.parse("content://mms/" + msgId + "/addr");
        if (array == null) {
            return;
        }
        for (EncodedStringValue addrOrg : array) {
            EncodedStringValue addr =  addrOrg;
            strValues.add(toIsoString(addr.getTextString()));
            strValues.add(String.valueOf(addr.getCharacterSet()));
            strValues.add(String.valueOf(type));
        }
        values.putStringArrayList("addresses", strValues);
        SqliteWrapper.insert(mContext, mContentResolver, uri, values);

        /// @}
    }




 @Override
  protected PduPart[] loadParts(long msgId) throws MmsException {
        Cursor c = SqliteWrapper.query(mContext, mContentResolver,
                Uri.parse("content://mms/" + msgId + "/part"),
                PART_PROJECTION, null, null, null);

        MtkPduPart[] parts = null;

        try {
            if ((c == null) || (c.getCount() == 0)) {
                if (LOCAL_LOGV) {
                    Log.v(TAG, "loadParts(" + msgId + "): no part to load.");
                }
                return null;
            }

            int partCount = c.getCount();
            int partIdx = 0;
            parts = new MtkPduPart[partCount];
            while (c.moveToNext()) {
                MtkPduPart part = new MtkPduPart();
                Integer charset = getIntegerFromPartColumn(
                        c, PART_COLUMN_CHARSET);
                if (charset != null) {
                    part.setCharset(charset);
                }

                byte[] contentDisposition = getByteArrayFromPartColumn(
                        c, PART_COLUMN_CONTENT_DISPOSITION);
                if (contentDisposition != null) {
                    part.setContentDisposition(contentDisposition);
                }

                byte[] contentId = getByteArrayFromPartColumn(
                        c, PART_COLUMN_CONTENT_ID);
                if (contentId != null) {
                    part.setContentId(contentId);
                }

                byte[] contentLocation = getByteArrayFromPartColumn(
                        c, PART_COLUMN_CONTENT_LOCATION);
                if (contentLocation != null) {
                    part.setContentLocation(contentLocation);
                }

                byte[] contentType = getByteArrayFromPartColumn(
                        c, PART_COLUMN_CONTENT_TYPE);
                if (contentType != null) {
                    part.setContentType(contentType);
                } else {
                    throw new MmsException("Content-Type must be set.");
                }

                byte[] fileName = getByteArrayFromPartColumn2(
                        c, PART_COLUMN_FILENAME);
                if (fileName != null) {
                    part.setFilename(fileName);
                }

                byte[] name = getByteArrayFromPartColumn2(
                        c, PART_COLUMN_NAME);
                if (name != null) {
                    part.setName(name);
                }

                // Construct a Uri for this part.
                long partId = c.getLong(PART_COLUMN_ID);
                Uri partURI = Uri.parse("content://mms/part/" + partId);
                part.setDataUri(partURI);

                // For images/audio/video, we won't keep their data in Part
                // because their renderer accept Uri as source.
                String type = toIsoString(contentType);
                if (!ContentType.isImageType(type)
                        && !ContentType.isAudioType(type)
                        && !ContentType.isVideoType(type)) {
                    ByteArrayOutputStream baos = new ByteArrayOutputStream();
                    InputStream is = null;

                    // Store simple string values directly in the database instead of an
                    // external file.  This makes the text searchable and retrieval slightly
                    // faster.
                    if (ContentType.TEXT_PLAIN.equals(type) || ContentType.APP_SMIL.equals(type)
                            || ContentType.TEXT_HTML.equals(type)) {
                        String text = c.getString(PART_COLUMN_TEXT);
                        /// M:Code analyze 003,change logic with a new function
                        /// to construct a byte array for fix the bug ALPS00074082,
                        /// When forward MMS have UTF-16,the UTF-16 are garbled @{
                        byte [] blob;
                        if (charset != null) {
                            blob = new MtkEncodedStringValue(charset, text != null ? text : "")
                            .getTextString();
                        } else {
                            blob = new MtkEncodedStringValue(text != null ? text : "")
                            .getTextString();
                        }
                        /// @}
                        baos.write(blob, 0, blob.length);
                    } else {

                        try {
                            is = mContentResolver.openInputStream(partURI);

                            byte[] buffer = new byte[256];
                            int len = is.read(buffer);
                            while (len >= 0) {
                                baos.write(buffer, 0, len);
                                len = is.read(buffer);
                            }
                        } catch (IOException e) {
                            Log.e(TAG, "Failed to load part data", e);
                            c.close();
                            throw new MmsException(e);
                        } finally {
                            if (is != null) {
                                try {
                                    is.close();
                                } catch (IOException e) {
                                    Log.e(TAG, "Failed to close stream", e);
                                } // Ignore
                            }
                        }
                    }
                    part.setData(baos.toByteArray());
                }
                parts[partIdx++] = part;
            }
        } finally {
            if (c != null) {
                c.close();
            }
        }

        return parts;
    }

 @Override
 public Uri persistPart(PduPart part, long msgId, HashMap<Uri, InputStream> preOpenedFiles)
            throws MmsException {
        Uri uri = Uri.parse("content://mms/" + msgId + "/part");
        ContentValues values = new ContentValues(8);
        /* Google original code
        int charset = part.getCharset();
        if (charset != 0 ) {
            values.put(Part.CHARSET, charset);
        }
        */
        String contentType = getPartContentType(part);
        if (contentType != null) {
            // There is no "image/jpg" in Android (and it's an invalid mimetype).
            // Change it to "image/jpeg"
            if (ContentType.IMAGE_JPG.equals(contentType)) {
                contentType = ContentType.IMAGE_JPEG;
            }

            values.put(Part.CONTENT_TYPE, contentType);
            // To ensure the SMIL part is always the first part.
            if (ContentType.APP_SMIL.equals(contentType)) {
                values.put(Part.SEQ, -1);
            }
        } else {
            throw new MmsException("MIME type of the part must be set.");
        }

        /// M:Code analyze 006,if this part is SMIL, we must ignore the charset, otherwise would be
        /// wrong charset when we got the smil data in DB.more details please check IOT CR:105247 @{
        int charset = part.getCharset();
        if (charset != 0) {
            if (!ContentType.APP_SMIL.equals(contentType)) {
                values.put(Part.CHARSET, charset);
            }
        }
        /// @}

        if (part.getFilename() != null) {
            String fileName = new String(part.getFilename());
            values.put(Part.FILENAME, fileName);
        }

        if (part.getName() != null) {
            String name = new String(part.getName());
            values.put(Part.NAME, name);
        }

        Object value = null;
        if (part.getContentDisposition() != null) {
            value = toIsoString(part.getContentDisposition());
            values.put(Part.CONTENT_DISPOSITION, (String) value);
        }

        if (part.getContentId() != null) {
            value = toIsoString(part.getContentId());
            values.put(Part.CONTENT_ID, (String) value);
        }

        if (part.getContentLocation() != null) {
            value = toIsoString(part.getContentLocation());
            values.put(Part.CONTENT_LOCATION, (String) value);
        }

        Uri res = SqliteWrapper.insert(mContext, mContentResolver, uri, values);
        if (res == null) {
            throw new MmsException("Failed to persist part, return null.");
        }

        persistData(part, res, contentType, preOpenedFiles);
        // After successfully store the data, we should update
        // the dataUri of the part.
        part.setDataUri(res);

        return res;
    }

     /**
     * Update headers of a SendReq.
     *
     * @param uri The PDU which need to be updated.
     * @param pdu New headers.
     * @throws MmsException Bad URI or updating failed.
     */
/*
    public void updateHeaders(Uri uri, SendReq sendReq) {
        synchronized (PDU_CACHE_INSTANCE) {
            // If the cache item is getting updated, wait until it's done updating before
            // purging it.
            if (PDU_CACHE_INSTANCE.isUpdating(uri)) {
                if (LOCAL_LOGV) {
                    Log.v(TAG, "updateHeaders: " + uri + " blocked by isUpdating()");
                }
                try {
                    PDU_CACHE_INSTANCE.wait();
                } catch (InterruptedException e) {
                    Log.e(TAG, "updateHeaders: ", e);
                }
            }
        }
        PDU_CACHE_INSTANCE.purge(uri);

        ContentValues values = new ContentValues(10);
        byte[] contentType = sendReq.getContentType();
        if (contentType != null) {
            values.put(Mms.CONTENT_TYPE, toIsoString(contentType));
        }

        long date = sendReq.getDate();
        if (date != -1) {
            values.put(Mms.DATE, date);
        }

        int deliveryReport = sendReq.getDeliveryReport();
        if (deliveryReport != 0) {
            values.put(Mms.DELIVERY_REPORT, deliveryReport);
        }

        long expiry = sendReq.getExpiry();
        if (expiry != -1) {
            values.put(Mms.EXPIRY, expiry);
        }

        byte[] msgClass = sendReq.getMessageClass();
        if (msgClass != null) {
            values.put(Mms.MESSAGE_CLASS, toIsoString(msgClass));
        }

        int priority = sendReq.getPriority();
        if (priority != 0) {
            values.put(Mms.PRIORITY, priority);
        }

        int readReport = sendReq.getReadReport();
        if (readReport != 0) {
            values.put(Mms.READ_REPORT, readReport);
        }

        byte[] transId = sendReq.getTransactionId();
        if (transId != null) {
            values.put(Mms.TRANSACTION_ID, toIsoString(transId));
        }

        MtkEncodedStringValue subject = (MtkEncodedStringValue) sendReq.getSubject();
        if (subject != null) {
            values.put(Mms.SUBJECT, toIsoString(subject.getTextString()));
            values.put(Mms.SUBJECT_CHARSET, subject.getCharacterSet());
        } else {
            values.put(Mms.SUBJECT, "");
        }

        long messageSize = sendReq.getMessageSize();
        if (messageSize > 0) {
            values.put(Mms.MESSAGE_SIZE, messageSize);
        }

        MtkPduHeaders headers = (MtkPduHeaders) sendReq.getPduHeaders();
        HashSet<String> recipients = new HashSet<String>();
        for (int addrType : ADDRESS_FIELDS) {
            MtkEncodedStringValue[] array = null;
            if (addrType == MtkPduHeaders.FROM) {
                MtkEncodedStringValue v = headers.getEncodedStringValueEx(addrType);
                if (v != null) {
                    array = new MtkEncodedStringValue[1];
                    array[0] = v;
                }
            } else {
                array = headers.getEncodedStringValuesEx(addrType);
            }

            if (array != null) {
                long msgId = ContentUris.parseId(uri);
                updateAddress(msgId, addrType, array);
                if (addrType == MtkPduHeaders.TO) {
                    for (MtkEncodedStringValue v : array) {
                        if (v != null) {
                            recipients.add(v.getString());
                        }
                    }
                }
            }
        }
        if (!recipients.isEmpty()) {
            long threadId = Threads.getOrCreateThreadId(mContext, recipients);
            values.put(Mms.THREAD_ID, threadId);
        }

        SqliteWrapper.update(mContext, mContentResolver, uri, values, null, null);
    }
*/
    //This code shall be removed from AOSP @Override
    public Uri persist(GenericPdu pdu, Uri uri, boolean createThreadId,
                     boolean groupMmsEnabled)
            throws MmsException {
           return persist(pdu, uri, createThreadId, groupMmsEnabled, null);
    }

    //This code shall be removed from AOSP @Override
    public Uri persist(GenericPdu pdu, Uri uri)
            throws MmsException {
           return persist(pdu, uri, true, false);
    }

    @Override
     /**
     * For a given address type, extract the recipients from the headers.
     *
     * @param addressType can be PduHeaders.FROM, PduHeaders.TO or PduHeaders.CC
     * @param recipients a HashSet that is loaded with the recipients from the FROM,
     * TO or CC headers
     * @param addressMap a HashMap of the addresses from the ADDRESS_FIELDS header
     * @param excludeMyNumber if true, the number of this phone will be excluded from recipients
     */
    protected void loadRecipients(int addressType, HashSet<String> recipients,
            HashMap<Integer, EncodedStringValue[]> addressMap, boolean excludeMyNumber) {
        EncodedStringValue[] array = addressMap.get(addressType);
        if (array == null) {
            return;
        }
        // If the TO recipients is only a single address, then we can skip loadRecipients when
        // we're excluding our own number because we know that address is our own.
        if (excludeMyNumber && array.length == 1) {
            return;
        }

        int [] SubIdList = SubscriptionManager.from(mContext).getActiveSubscriptionIdList();
        for (EncodedStringValue vOrg : array) {
            EncodedStringValue v = (EncodedStringValue) vOrg;
            if (v != null) {
                String number = v.getString();
                //Log.d(TAG, "number = " + number);
                Log.d(TAG, "length = " + SubIdList.length);
                if (SubIdList.length == 0) {
                    //Log.d(TAG, "recipients add number = " + number);
                    recipients.add(number);
                    continue;
                }
                for (int subid : SubIdList) {
                    Log.d(TAG, "subid = " + subid);
                    String myNumber = excludeMyNumber ? mTelephonyManager
                            .getLine1Number(subid) : null;
                    if ((myNumber == null || !PhoneNumberUtils.compare(number, myNumber))
                            && !recipients.contains(number)) {
                        // Only add numbers which aren't my own number.
                        recipients.add(number);
                    }
                }
            }
        }
    }

    @Override
    /**
     * Find all messages to be sent or downloaded before certain time.
     */
    public Cursor getPendingMessages(long dueTime) {
        Uri.Builder uriBuilder = PendingMessages.CONTENT_URI.buildUpon();
        uriBuilder.appendQueryParameter("protocol", "mms");

      /*
         ALPS03076282:Received repeated MMS
         msg_box<>2 : This will neglect all sent messages and m_type<>132 : will neglect all that
         are already downloaded
      */

       String selection = PendingMessages.ERROR_TYPE + " < ?"
                + " AND " + PendingMessages.DUE_TIME + " <= ?"
                + " AND " + PendingMessages.MSG_ID + " in ( SELECT "
                + PendingMessages.MSG_ID + " FROM pdu "
                + " WHERE " + Mms.MESSAGE_BOX + " <>2 " + " AND "
                + Mms.MESSAGE_TYPE + " <>132 " + " )";


        String[] selectionArgs = new String[] {
                String.valueOf(MmsSms.ERR_TYPE_GENERIC_PERMANENT),
                String.valueOf(dueTime)
        };

        return SqliteWrapper.query(mContext, mContentResolver,
                uriBuilder.build(), null, selection, selectionArgs,
                PendingMessages.DUE_TIME);
    }

    /*[MTK MMS FW] OP01*/
    /**
     * Load a PDU from storage by given Uri.
     *
     * @param uri The Uri of the PDU to be loaded.
     * @return A generic PDU object, it may be cast to dedicated PDU.
     * @throws MmsException Failed to load some fields of a PDU.
     */
    /* Q0 migration*/
    /*
    public GenericPdu load(Uri uri) throws MmsException {
        GenericPdu pdu = null;
        PduCacheEntry cacheEntry = null;
        int msgBox = 0;
        long threadId = -1;
        try {
            synchronized(PDU_CACHE_INSTANCE) {
                if (PDU_CACHE_INSTANCE.isUpdating(uri)) {
                    if (LOCAL_LOGV) {
                        Log.v(TAG, "load: " + uri + " blocked by isUpdating()");
                    }
                    try {
                        PDU_CACHE_INSTANCE.wait();
                    } catch (InterruptedException e) {
                        Log.e(TAG, "load: ", e);
                    }
                    cacheEntry = PDU_CACHE_INSTANCE.get(uri);
                    if (cacheEntry != null) {
                        return cacheEntry.getPdu();
                    }
                }
                // Tell the cache to indicate to other callers that this item
                // is currently being updated.
                PDU_CACHE_INSTANCE.setUpdating(uri, true);
            }

            Cursor c = SqliteWrapper.query(mContext, mContentResolver, uri,
                    PDU_PROJECTION, null, null, null);
            MtkPduHeaders headers = new MtkPduHeaders();
            Set<Entry<Integer, Integer>> set;
            long msgId = ContentUris.parseId(uri);

            try {
                if ((c == null) || (c.getCount() != 1) || !c.moveToFirst()) {
                    throw new MmsException("Bad uri: " + uri);
                }

                msgBox = c.getInt(PDU_COLUMN_MESSAGE_BOX);
                threadId = c.getLong(PDU_COLUMN_THREAD_ID);

                set = ENCODED_STRING_COLUMN_INDEX_MAP.entrySet();
                for (Entry<Integer, Integer> e : set) {
                    setEncodedStringValueToHeaders(
                            c, e.getValue(), headers, e.getKey());
                }

                set = TEXT_STRING_COLUMN_INDEX_MAP.entrySet();
                for (Entry<Integer, Integer> e : set) {
                    setTextStringToHeaders(
                            c, e.getValue(), headers, e.getKey());
                }

                set = OCTET_COLUMN_INDEX_MAP.entrySet();
                for (Entry<Integer, Integer> e : set) {
                    setOctetToHeaders(
                            c, e.getValue(), headers, e.getKey());
                }

                set = LONG_COLUMN_INDEX_MAP.entrySet();
                for (Entry<Integer, Integer> e : set) {
                    setLongToHeaders(
                            c, e.getValue(), headers, e.getKey());
                }

            /// M:Code analyze 005,Method for BackupRestore, for this application @{
            if (mBackupRestore) {
                Log.i(TAG, "load for backuprestore");
                if (!c.isNull(PDU_COLUMN_READ)) {
                    int b = c.getInt(PDU_COLUMN_READ);
                    Log.i(TAG, "read value=" + b);
                    if (b == 1) {
                        headers.setOctet(MtkPduHeaders.READ_STATUS_READ, MtkPduHeaders.READ_STATUS);
                    }
                }
            }
            /// @}
            } finally {
                if (c != null) {
                    c.close();
                }
            }

            // Check whether 'msgId' has been assigned a valid value.
            if (msgId == -1L) {
                throw new MmsException("Error! ID of the message: -1.");
            }

            // Load address information of the MM.
            loadAddress(msgId, headers);

            int msgType = headers.getOctet(MtkPduHeaders.MESSAGE_TYPE);
            PduBody body = new PduBody();

            // For PDU which type is M_retrieve.conf or Send.req, we should
            // load multiparts and put them into the body of the PDU.
            if ((msgType == MtkPduHeaders.MESSAGE_TYPE_RETRIEVE_CONF)
                    || (msgType == MtkPduHeaders.MESSAGE_TYPE_SEND_REQ)) {
                PduPart[] parts = loadParts(msgId);
                if (parts != null) {
                    int partsNum = parts.length;
                    for (int i = 0; i < partsNum; i++) {
                        body.addPart(parts[i]);
                    }
                }
            }

            switch (msgType) {
            case MtkPduHeaders.MESSAGE_TYPE_NOTIFICATION_IND:
                pdu = new NotificationInd(headers);
                break;
            case MtkPduHeaders.MESSAGE_TYPE_DELIVERY_IND:
                pdu = new DeliveryInd(headers);
                break;
            case MtkPduHeaders.MESSAGE_TYPE_READ_ORIG_IND:
                pdu = new ReadOrigInd(headers);
                break;
            case MtkPduHeaders.MESSAGE_TYPE_RETRIEVE_CONF:
                pdu = new MtkRetrieveConf(headers, body);
                break;
            case MtkPduHeaders.MESSAGE_TYPE_SEND_REQ:
                pdu = new MtkSendReq(headers, body);
                break;
            case MtkPduHeaders.MESSAGE_TYPE_ACKNOWLEDGE_IND:
                pdu = new AcknowledgeInd(headers);
                break;
            case MtkPduHeaders.MESSAGE_TYPE_NOTIFYRESP_IND:
                pdu = new NotifyRespInd(headers);
                break;
            case MtkPduHeaders.MESSAGE_TYPE_READ_REC_IND:
                pdu = new ReadRecInd(headers);
                break;
            case MtkPduHeaders.MESSAGE_TYPE_SEND_CONF:
            case MtkPduHeaders.MESSAGE_TYPE_FORWARD_REQ:
            case MtkPduHeaders.MESSAGE_TYPE_FORWARD_CONF:
            case MtkPduHeaders.MESSAGE_TYPE_MBOX_STORE_REQ:
            case MtkPduHeaders.MESSAGE_TYPE_MBOX_STORE_CONF:
            case MtkPduHeaders.MESSAGE_TYPE_MBOX_VIEW_REQ:
            case MtkPduHeaders.MESSAGE_TYPE_MBOX_VIEW_CONF:
            case MtkPduHeaders.MESSAGE_TYPE_MBOX_UPLOAD_REQ:
            case MtkPduHeaders.MESSAGE_TYPE_MBOX_UPLOAD_CONF:
            case MtkPduHeaders.MESSAGE_TYPE_MBOX_DELETE_REQ:
            case MtkPduHeaders.MESSAGE_TYPE_MBOX_DELETE_CONF:
            case MtkPduHeaders.MESSAGE_TYPE_MBOX_DESCR:
            case MtkPduHeaders.MESSAGE_TYPE_DELETE_REQ:
            case MtkPduHeaders.MESSAGE_TYPE_DELETE_CONF:
            case MtkPduHeaders.MESSAGE_TYPE_CANCEL_REQ:
            case MtkPduHeaders.MESSAGE_TYPE_CANCEL_CONF:
                throw new MmsException(
                        "Unsupported PDU type: " + Integer.toHexString(msgType));

            default:
                throw new MmsException(
                        "Unrecognized PDU type: " + Integer.toHexString(msgType));
            }
        } finally {
            synchronized(PDU_CACHE_INSTANCE) {
                if (pdu != null) {
                    assert(PDU_CACHE_INSTANCE.get(uri) == null);
                    // Update the cache entry with the real info
                    cacheEntry = new PduCacheEntry(pdu, msgBox, threadId);
                    PDU_CACHE_INSTANCE.put(uri, cacheEntry);
                }
                PDU_CACHE_INSTANCE.setUpdating(uri, false);
                PDU_CACHE_INSTANCE.notifyAll(); // tell anybody waiting on this entry to go ahead
            }
        }
        return pdu;
    }
    */

    public Uri persist(GenericPdu pdu, Uri uri, boolean createThreadId, boolean groupMmsEnabled,
            HashMap<Uri, InputStream> preOpenedFiles) throws MmsException {
        return persist(pdu, uri, createThreadId, groupMmsEnabled, preOpenedFiles, null);
    }

    /**
     * Persist a PDU object to specific location in the storage.
     *
     * @param pdu The PDU object to be stored.
     * @param uri Where to store the given PDU object.
     * @param createThreadId if true, this function may create a thread id for the recipients
     * @param groupMmsEnabled if true, all of the recipients addressed in the PDU will be used
     *  to create the associated thread. When false, only the sender will be used in finding or
     *  creating the appropriate thread or conversation.
     * @param preOpenedFiles if not null, a map of preopened InputStreams for the parts.
     * @return A Uri which can be used to access the stored PDU.
     */

    public Uri persist(GenericPdu pdu, Uri uri, boolean createThreadId, boolean groupMmsEnabled,
            HashMap<Uri, InputStream> preOpenedFiles, String contentLocation)
            throws MmsException {
        if (uri == null) {
            throw new MmsException("Uri may not be null.");
        }
        long msgId = -1;
        try {
            msgId = ContentUris.parseId(uri);
        } catch (NumberFormatException e) {
            // the uri ends with "inbox" or something else like that
        }
        boolean existingUri = msgId != -1;

        if (!existingUri && MESSAGE_BOX_MAP.get(uri) == null) {
            throw new MmsException(
                    "Bad destination, must be one of "
                    + "content://mms/inbox, content://mms/sent, "
                    + "content://mms/drafts, content://mms/outbox, "
                    + "content://mms/temp.");
        }
        synchronized(PDU_CACHE_INSTANCE) {
            // If the cache item is getting updated, wait until it's done updating before
            // purging it.
            if (PDU_CACHE_INSTANCE.isUpdating(uri)) {
                if (LOCAL_LOGV) {
                    Log.v(TAG, "persist: " + uri + " blocked by isUpdating()");
                }
                try {
                    PDU_CACHE_INSTANCE.wait();
                } catch (InterruptedException e) {
                    Log.e(TAG, "persist1: ", e);
                }
            }
        }
        PDU_CACHE_INSTANCE.purge(uri);
        Log.d(TAG, "persist uri " + uri);
        PduHeaders header = pdu.getPduHeaders();
        PduBody body = null;
        ContentValues values = new ContentValues();
        Set<Entry<Integer, String>> set;

        set = ENCODED_STRING_COLUMN_NAME_MAP.entrySet();
        for (Entry<Integer, String> e : set) {
            int field = e.getKey();
            EncodedStringValue encodedString = header.getEncodedStringValue(field);
            if (encodedString != null) {
                String charsetColumn = CHARSET_COLUMN_NAME_MAP.get(field);
                values.put(e.getValue(), toIsoString(encodedString.getTextString()));
                values.put(charsetColumn, encodedString.getCharacterSet());
            }
        }

        set = TEXT_STRING_COLUMN_NAME_MAP.entrySet();
        for (Entry<Integer, String> e : set){
            byte[] text = header.getTextString(e.getKey());
            if (text != null) {
                values.put(e.getValue(), toIsoString(text));
            }
        }

        set = OCTET_COLUMN_NAME_MAP.entrySet();
        for (Entry<Integer, String> e : set){
            int b = header.getOctet(e.getKey());
            if (b != 0) {
                values.put(e.getValue(), b);
            }
        }

        /// M:Code analyze 005,Method for BackupRestore, for this application @{
        /* Q0 migration*/
        /*
        if (mBackupRestore) {
            Log.i(TAG, "add READ");
            int b = header.getOctet(MtkPduHeaders.READ_STATUS);
            Log.i(TAG, "READ=" + b);
            if (b == 0) {
                values.put(Mms.READ, b);
            } else if (b == MtkPduHeaders.READ_STATUS_READ) {
                values.put(Mms.READ, 1);
            } else {
                values.put(Mms.READ, 0);
            }
        }
        */
        /// @}

        set = LONG_COLUMN_NAME_MAP.entrySet();
        for (Entry<Integer, String> e : set){
            long l = header.getLongInteger(e.getKey());
            if (l != -1L) {
                values.put(e.getValue(), l);
            }
        }

        HashMap<Integer, EncodedStringValue[]> addressMap =
                new HashMap<Integer, EncodedStringValue[]>(ADDRESS_FIELDS.length);
        // Save address information.
        for (int addrType : ADDRESS_FIELDS) {
            EncodedStringValue[] array = null;
            if (addrType == MtkPduHeaders.FROM) {
                EncodedStringValue v = header.getEncodedStringValue(addrType);
                if (v != null) {
                    array = new EncodedStringValue[1];
                    array[0] = v;
                }
            } else {
                array = header.getEncodedStringValues(addrType);
            }
            addressMap.put(addrType, array);
        }

        HashSet<String> recipients = new HashSet<String>();
        int msgType = pdu.getMessageType();
        // Here we only allocate thread ID for M-Notification.ind,
        // M-Retrieve.conf and M-Send.req.
        // Some of other PDU types may be allocated a thread ID outside
        // this scope.
        if ((msgType == MtkPduHeaders.MESSAGE_TYPE_NOTIFICATION_IND)
                || (msgType == MtkPduHeaders.MESSAGE_TYPE_RETRIEVE_CONF)
                || (msgType == MtkPduHeaders.MESSAGE_TYPE_SEND_REQ)) {
            /* Q0 migration*/
            //EncodedStringValue[] array = null;
            switch (msgType) {
                case MtkPduHeaders.MESSAGE_TYPE_NOTIFICATION_IND:
                case MtkPduHeaders.MESSAGE_TYPE_RETRIEVE_CONF:
                    loadRecipients(MtkPduHeaders.FROM, recipients, addressMap, false);

                    // For received messages when group MMS is enabled, we want to associate this
                    // message with the thread composed of all the recipients -- all but our own
                    // number, that is. This includes the person who sent the
                    // message or the FROM field (above) in addition to the other people the message
                    // was addressed to or the TO field. Our own number is in that TO field and
                    // we have to ignore it in loadRecipients.
                    if (groupMmsEnabled) {
                        loadRecipients(MtkPduHeaders.TO, recipients, addressMap, true);

                        // Also load any numbers in the CC field to address group messaging
                        // compatibility issues with devices that place numbers in this field
                        // for group messages.
                        loadRecipients(MtkPduHeaders.CC, recipients, addressMap, true);
                    }
                    break;
                case MtkPduHeaders.MESSAGE_TYPE_SEND_REQ:
                    loadRecipients(MtkPduHeaders.TO, recipients, addressMap, false);
                    break;
            }
            long threadId = 0;
            if (createThreadId && !recipients.isEmpty()) {
                // Given all the recipients associated with this message, find (or create) the
                // correct thread.
                threadId = Threads.getOrCreateThreadId(mContext, recipients);
            }
            values.put(Mms.THREAD_ID, threadId);
        }
        Log.d(TAG, "persist part begin ");
        // Save parts first to avoid inconsistent message is loaded
        // while saving the parts.
        long dummyId = System.currentTimeMillis(); // Dummy ID of the msg.

        // Figure out if this PDU is a text-only message
        boolean textOnly = true;

        // Sum up the total message size
        int messageSize = 0;

        // Get body if the PDU is a RetrieveConf or SendReq.
        if (pdu instanceof MultimediaMessagePdu) {
            body = ((MultimediaMessagePdu) pdu).getBody();
            // Start saving parts if necessary.
            if (body != null) {
                int partsNum = body.getPartsNum();
                if (partsNum > 2) {
                    // For a text-only message there will be two parts: 1-the SMIL, 2-the text.
                    // Down a few lines below we're checking to make sure we've only got SMIL or
                    // text. We also have to check then we don't have more than two parts.
                    // Otherwise, a slideshow with two text slides would be marked as textOnly.
                    textOnly = false;
                }
                for (int i = 0; i < partsNum; i++) {
                    PduPart part = body.getPart(i);
                    messageSize += part.getDataLength();
                    persistPart(part, dummyId, preOpenedFiles);

                    // If we've got anything besides text/plain or SMIL part, then we've got
                    // an mms message with some other type of attachment.
                    String contentType = getPartContentType(part);
                    if (contentType != null && !ContentType.APP_SMIL.equals(contentType)
                            && !ContentType.TEXT_PLAIN.equals(contentType)) {
                        textOnly = false;
                    }
                }
            }
        }
        // Record whether this mms message is a simple plain text or not. This is a hint for the
        // UI.
        values.put(Mms.TEXT_ONLY, textOnly ? 1 : 0);
        // The message-size might already have been inserted when parsing the
        // PDU header. If not, then we insert the message size as well.
        if (values.getAsInteger(Mms.MESSAGE_SIZE) == null) {
            values.put(Mms.MESSAGE_SIZE, messageSize);
        }
        // insert mms content location into pdu table
        if (contentLocation != null) {
            values.put(Mms.CONTENT_LOCATION, contentLocation);
        }
        /// M:Code analyze 010,add a new key value to store into database @{
        Log.d(TAG, "persist pdu begin ");
        values.put("need_notify", false);
        /// @}

        /// M:Code analyze 011,if need backupRestore,need to store one more key value
        /// "seen" into database @{
        /* Q0 migration*/
        /*
        if (mBackupRestore) {
            values.put("seen", 1);
        }
        */
        /// @}

        Uri res = null;
        if (existingUri) {
            res = uri;
            SqliteWrapper.update(mContext, mContentResolver, res, values, null, null);
        } else {
            res = SqliteWrapper.insert(mContext, mContentResolver, uri, values);
            if (res == null) {
                throw new MmsException("persist() failed: return null.");
            }
            // Get the real ID of the PDU and update all parts which were
            // saved with the dummy ID.
            msgId = ContentUris.parseId(res);
        }

        /// M:Code analyze 012,move this paragraph here for bug ALPS00249113,the mms received has no
        /// content,and it is showing normal after received next one @{
        Log.d(TAG, "persist address begin ");
        // Save address information.
        for (int addrType : ADDRESS_FIELDS) {
            EncodedStringValue[] array = addressMap.get(addrType);
            if (array != null) {
                persistAddress(msgId, addrType, array);
            }
        }
        /// @}

        Log.d(TAG, "persist update part begin ");
        values = new ContentValues(1);
        values.put(Part.MSG_ID, msgId);
        /// M:Code analyze 013,add for bug ALPS00249113, a new key value to store into database @{
        if (contentLocation == null) {
            values.put("need_notify", true);
        }
        /// @}
        SqliteWrapper.update(mContext, mContentResolver,
                             Uri.parse("content://mms/" + dummyId + "/part"),
                             values, null, null);
        if (contentLocation != null) {
            // Delete the corresponding NotificationInd
            String LOCATION_SELECTION =
                    Mms.MESSAGE_TYPE + "=? AND " + Mms.CONTENT_LOCATION + " =?";
            SqliteWrapper.delete(mContext,
                    mContentResolver,
                    Mms.CONTENT_URI,
                    LOCATION_SELECTION,
                    new String[]{
                            Integer.toString(MtkPduHeaders.MESSAGE_TYPE_NOTIFICATION_IND),
                            contentLocation
                    });
        }
        /// M:Code analyze 014,add for bug ALPS00259371,update the cache again to fix the problem
        /// to receive mms body lag behind @{
        PDU_CACHE_INSTANCE.purge(uri);
        Log.d(TAG, "persist purge end ");
        /// @}
        // We should return the longest URI of the persisted PDU, for
        // example, if input URI is "content://mms/inbox" and the _ID of
        // persisted PDU is '8', we should return "content://mms/inbox/8"
        // instead of "content://mms/8".
        // FIXME: Should the MmsProvider be responsible for this???
        if (!existingUri) {
            res = Uri.parse(uri + "/" + msgId);
        }

        // Save address information.
        /* Google original code
        for (int addrType : ADDRESS_FIELDS) {
            EncodedStringValue[] array = addressMap.get(addrType);
            if (array != null) {
                persistAddress(msgId, addrType, array);
            }
        }
        */
        return res;
    }

    /// new variable and methods
    /// M:Code analyze 005,Method for BackupRestore, for this application @{
    /* Q0 migration*//* Rollback Backup Restore*/
    private boolean mBackupRestore = false;
    public GenericPdu load(Uri uri, boolean backupRestore) throws MmsException {
        Log.i("MMSLog", "load for backuprestore");
        mBackupRestore = backupRestore;
        return load(uri);
    }
    /// @}

    /// M:Code analyze 009, add a new function for checking if need backupRestore @{
    /* Q0 migration*//* Rollback Backup Restore*/
    public Uri persist(GenericPdu pdu, Uri uri, boolean backupRestore) throws MmsException {
        Log.i("MMSLog", "persist for backuprestore");
        mBackupRestore = backupRestore;
        return persist(pdu, uri, true, false);
    }
    /// @}

    // Add for Backup&Restore enhance
    // Now Restore will call this fuction to persist mms
    // This function will change squence of persist and reduce update database
    /**
     * Comments
     * @internal
     */
    /* Q0 migration*//* Rollback Backup Restore*/
    public Uri persistEx(GenericPdu pdu, Uri uri,
            HashMap<String, String> attach) throws MmsException {
        Log.i("MMSLog", "Call persist_ex 1");
        return persistForBackupRestore(pdu, uri, attach);
    }

    /**
     * Comments
     * @internal
     */
    /* Q0 migration*//* Rollback Backup Restore*/
    public Uri persistEx(GenericPdu pdu, Uri uri,
            boolean backupRestore, HashMap<String, String> attach) throws MmsException {
        Log.i("MMSLog", "Call persist_ex 2");
        mBackupRestore = backupRestore;
        return persistForBackupRestore(pdu, uri, attach);
    }

    //Change persist process
    /* Q0 migration*//* Rollback Backup Restore*/
    private Uri persistForBackupRestore(GenericPdu pdu,
            Uri uri, HashMap<String, String> attach) throws MmsException {
        if (uri == null) {
            throw new MmsException("Uri may not be null.");
        }
        long msgId = -1;
        try {
            msgId = ContentUris.parseId(uri);
        } catch (NumberFormatException e) {
            // the uri ends with "inbox" or something else like that
        }
        boolean existingUri = msgId != -1;

        if (!existingUri && MESSAGE_BOX_MAP.get(uri) == null) {
            throw new MmsException(
                    "Bad destination, must be one of "
                    + "content://mms/inbox, content://mms/sent, "
                    + "content://mms/drafts, content://mms/outbox, "
                    + "content://mms/temp.");
        }
        synchronized (PDU_CACHE_INSTANCE) {
            // If the cache item is getting updated, wait until it's done updating before
            // purging it.
            if (PDU_CACHE_INSTANCE.isUpdating(uri)) {
                if (LOCAL_LOGV) {
                    Log.v(TAG, "persist: " + uri + " blocked by isUpdating()");
                }
                try {
                    PDU_CACHE_INSTANCE.wait();
                } catch (InterruptedException e) {
                    Log.e(TAG, "persist1: ", e);
                }
            }
        }
        PDU_CACHE_INSTANCE.purge(uri);
        Log.d(TAG, "persist uri " + uri);
        PduHeaders header = pdu.getPduHeaders();
        PduBody body = null;
        ContentValues values = new ContentValues();
        Set<Entry<Integer, String>> set;

        set = ENCODED_STRING_COLUMN_NAME_MAP.entrySet();
        for (Entry<Integer, String> e : set) {
            int field = e.getKey();
            EncodedStringValue encodedString = header.getEncodedStringValue(field);
            if (encodedString != null) {
                String charsetColumn = CHARSET_COLUMN_NAME_MAP.get(field);
                values.put(e.getValue(), toIsoString(encodedString.getTextString()));
                values.put(charsetColumn, encodedString.getCharacterSet());
            }
        }

        set = TEXT_STRING_COLUMN_NAME_MAP.entrySet();
        for (Entry<Integer, String> e : set) {
            byte[] text = header.getTextString(e.getKey());
            if (text != null) {
                values.put(e.getValue(), toIsoString(text));
            }
        }

        set = OCTET_COLUMN_NAME_MAP.entrySet();
        for (Entry<Integer, String> e : set) {
            int b = header.getOctet(e.getKey());
            if (b != 0) {
                values.put(e.getValue(), b);
            }
        }

        /// M:Code analyze 005,Method for BackupRestore, for this application @{
        if (mBackupRestore) {
            int read = 0;
            if (attach != null) {
                read = Integer.parseInt(attach.get("read"));
            }

            values.put(Mms.READ, read);

            long size = 0;
            if (attach != null && attach.get("m_size") != null) {
                size = Long.parseLong(attach.get("m_size"));
            }
            values.put(Mms.MESSAGE_SIZE, size);
        }
        /// @}

        set = LONG_COLUMN_NAME_MAP.entrySet();
        for (Entry<Integer, String> e : set) {
            long l = header.getLongInteger(e.getKey());
            if (l != -1L) {
                values.put(e.getValue(), l);
            }
        }

        HashMap<Integer, EncodedStringValue[]> addressMap =
                new HashMap<Integer, EncodedStringValue[]>(ADDRESS_FIELDS.length);
        // Save address information.
        for (int addrType : ADDRESS_FIELDS) {
            EncodedStringValue[] array = null;
            if (addrType == MtkPduHeaders.FROM) {
                EncodedStringValue v = header.getEncodedStringValue(addrType);
                if (v != null) {
                    array = new EncodedStringValue[1];
                    array[0] = v;
                }
            } else {
                array = header.getEncodedStringValues(addrType);
            }
            addressMap.put(addrType, array);
        }

        HashSet<String> recipients = new HashSet<String>();
        int msgType = pdu.getMessageType();
        // Here we only allocate thread ID for M-Notification.ind,
        // M-Retrieve.conf and M-Send.req.
        // Some of other PDU types may be allocated a thread ID outside
        // this scope.
        if ((msgType == MtkPduHeaders.MESSAGE_TYPE_NOTIFICATION_IND)
                || (msgType == MtkPduHeaders.MESSAGE_TYPE_RETRIEVE_CONF)
                || (msgType == MtkPduHeaders.MESSAGE_TYPE_SEND_REQ)) {
            EncodedStringValue[] array = null;
            switch (msgType) {
                case MtkPduHeaders.MESSAGE_TYPE_NOTIFICATION_IND:
                case MtkPduHeaders.MESSAGE_TYPE_RETRIEVE_CONF:
                    array = addressMap.get(MtkPduHeaders.FROM);
                    break;
                case MtkPduHeaders.MESSAGE_TYPE_SEND_REQ:
                    array = addressMap.get(MtkPduHeaders.TO);
                    break;
            }

            if (array != null) {
                for (EncodedStringValue v : array) {
                    if (v != null) {
                        recipients.add(v.getString());
                    }
                }
            }

            long time_1 = System.currentTimeMillis();

            //add for backup&restore enhance
            String backupRestore = null;
            if (attach != null) {
                backupRestore = attach.get("index");
            }

            // Call new api of getOrCreateThreadId for enhance
            if (!recipients.isEmpty()) {
                long threadId = MtkThreads.getOrCreateThreadId(mContext, recipients, backupRestore);
                values.put(Mms.THREAD_ID, threadId);
            }

            long time_2 = System.currentTimeMillis();

            long getThreadId_t = time_2 - time_1;

            Log.d("MMSLog", "BR_TEST: getThreadId=" + getThreadId_t);
        }

        long time_1 = System.currentTimeMillis();
        Log.d(TAG, "persist pdu begin ");
        values.put("need_notify", true);
        /// @}

        /// M:Code analyze 011,if need backupRestore,need to store more key values
        /// "seen" into database @{
        if (mBackupRestore) {
            values.put("seen", 1);
        }

        int sub_id = -1;
        int locked = 0;

        if (attach != null) {
            sub_id = Integer.parseInt(attach.get("sub_id"));
            locked = Integer.parseInt(attach.get("locked"));
        }

        values.put(Mms.SUBSCRIPTION_ID, sub_id);
        values.put(Mms.LOCKED, locked);
        /// @}

        Uri res = null;
        if (existingUri) {
            res = uri;
            SqliteWrapper.update(mContext, mContentResolver, res, values, null, null);
        } else {
            res = SqliteWrapper.insert(mContext, mContentResolver, uri, values);
            if (res == null) {
                throw new MmsException("persist() failed: return null.");
            }
            // Get the real ID of the PDU and update all parts which were
            // saved with the dummy ID.
            msgId = ContentUris.parseId(res);
        }

        long time_2 = System.currentTimeMillis();

        long persistPdu_t = time_2 - time_1;

        Log.d("MMSLog", "BR_TEST: parse time persistPdu=" + persistPdu_t);

        /// M:Code analyze 012,move this paragraph here for bug ALPS00249113,the mms received has no
        /// content,and it is showing normal after received next one @{
        Log.d(TAG, "persist address begin ");
        // Save address information.

        time_1 = System.currentTimeMillis();

        for (int addrType : ADDRESS_FIELDS) {
            EncodedStringValue[] array = addressMap.get(addrType);
            if (array != null) {
                persistAddress(msgId, addrType, array);
            }
        }

        time_2 = System.currentTimeMillis();

        long persistAddress_t = time_2 - time_1;

        Log.d("MMSLog", "BR_TEST: parse time persistAddress=" + persistAddress_t);
        /// @}

         Log.d(TAG, "persist part begin ");
          // Save parts first to avoid inconsistent message is loaded
          // while saving the parts.
          //long dummyId = System.currentTimeMillis(); // Dummy ID of the msg.
          // Get body if the PDU is a RetrieveConf or SendReq.
          if (pdu instanceof MultimediaMessagePdu) {
              body = ((MultimediaMessagePdu) pdu).getBody();
              // Start saving parts if necessary.
              if (body != null) {
                  int partsNum = body.getPartsNum();

                  time_1 = System.currentTimeMillis();

                  for (int i = 0; i < partsNum; i++) {
                      PduPart part = body.getPart(i);
                      persistPart(part, msgId, null);
                  }

                  time_2 = System.currentTimeMillis();
                  long persistPart_t = time_2 - time_1;
                 Log.d("MMSLog", "BR_TEST: parse time PersistPart=" + persistPart_t);
              }
          }

        // We should return the longest URI of the persisted PDU, for
        // example, if input URI is "content://mms/inbox" and the _ID of
        // persisted PDU is '8', we should return "content://mms/inbox/8"
        // instead of "content://mms/8".
        // FIXME: Should the MmsProvider be responsible for this???
        if (!existingUri) {
            res = Uri.parse(uri + "/" + msgId);
        }
        return res;
    }
    /*[MTK MMS FW] OP01 end*/
}
