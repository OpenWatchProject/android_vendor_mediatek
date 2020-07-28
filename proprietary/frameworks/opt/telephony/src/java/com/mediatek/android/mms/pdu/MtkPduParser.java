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

import android.util.Log;
import com.google.android.mms.ContentType;
import java.lang.reflect.InvocationTargetException;
import java.io.UnsupportedEncodingException;
import java.util.Arrays;
import com.google.android.mms.pdu.PduParser;
import com.google.android.mms.pdu.PduBody;
import com.google.android.mms.pdu.RetrieveConf;
import com.google.android.mms.pdu.SendReq;

import com.google.android.mms.pdu.NotifyRespInd;
import com.google.android.mms.pdu.DeliveryInd;
import com.google.android.mms.pdu.AcknowledgeInd;
import com.google.android.mms.pdu.ReadOrigInd;
import com.google.android.mms.pdu.PduPart;
import com.google.android.mms.pdu.ReadRecInd;
import com.google.android.mms.pdu.GenericPdu;
import com.google.android.mms.pdu.NotificationInd;
import com.google.android.mms.pdu.SendConf;
import com.google.android.mms.pdu.PduContentTypes;
import java.io.ByteArrayInputStream;
import java.util.HashMap;
/*[MTK MMS FW] OP01*/
import com.google.android.mms.InvalidHeaderValueException;
import com.google.android.mms.pdu.EncodedStringValue;
import com.google.android.mms.pdu.PduHeaders;

public class MtkPduParser extends PduParser {
    private static final String LOG_TAG = "MtkPduParser";
     /// M: For fix bug, parse read report failed.
    protected static final int UNSIGNED_INT_LIMIT = 2;
    /*[MTK MMS FW] OP01*/
    /* Q0 migration*/
//    private boolean mForRestore = false;

    public MtkPduParser(byte[] pduDataStream, boolean parseContentDisposition) {
        super(pduDataStream,parseContentDisposition);
    }

     /**
     * Parse the pdu.
     *
     * @return the pdu structure if parsing successfully.
     *         null if parsing error happened or mandatory fields are not set.
     */
    @Override
    public GenericPdu parse(){
        if (mPduDataStream == null) {
            Log.i(LOG_TAG, "Input parse stream is null");
            return null;
        }

        /* parse headers */
        mHeaders = parseHeaders(mPduDataStream);
        if (null == mHeaders) {
            // Parse headers failed.
            Log.i(LOG_TAG, "Parse PduHeader Failed");
            return null;
        }

        /* get the message type */
        int messageType = mHeaders.getOctet(MtkPduHeaders.MESSAGE_TYPE);

        /* check mandatory header fields */
        if (false == checkMandatoryHeader(mHeaders)) {
            Log.d(LOG_TAG, "check mandatory headers failed!");
            return null;
        }
        /// M:Code analyze 003,parse mPduDataStream,if the unsigned integer is more than 2
        /// reconstruct retrieveConf @{
        mPduDataStream.mark(1);
        int count = parseUnsignedInt(mPduDataStream);
        mPduDataStream.reset();
        if (MtkPduHeaders.MESSAGE_TYPE_RETRIEVE_CONF ==
            messageType && count >= UNSIGNED_INT_LIMIT) {
            byte[] contentType = mHeaders.getTextString(MtkPduHeaders.CONTENT_TYPE);
            if (null == contentType) {
                Log.i(LOG_TAG, "Parse MESSAGE_TYPE_RETRIEVE_CONF Failed: content Type is null _0");
                return null;
            }
            String contentTypeStr = new String(contentType);
            contentTypeStr = contentTypeStr.toLowerCase();
            if (!contentTypeStr.equals(ContentType.MULTIPART_MIXED)
                    && !contentTypeStr.equals(ContentType.MULTIPART_RELATED)
                    && !contentTypeStr.equals(ContentType.MULTIPART_ALTERNATIVE)) {

                if (contentTypeStr.equals(ContentType.TEXT_PLAIN)) {
                    Log.i(LOG_TAG, "Content Type is text/plain");

                    PduPart theOnlyPart = new PduPart();
                    theOnlyPart.setContentType(contentType);
                    theOnlyPart.setContentLocation(Long.toOctalString(
                            System.currentTimeMillis()).getBytes());
                    theOnlyPart.setContentId("<part1>".getBytes());

                    mPduDataStream.mark(1);

                    int partDataLen = 0;
                    while (mPduDataStream.read() != -1) {
                        partDataLen++;
                    }

                    byte[] partData = new byte[partDataLen];
                    Log.i(LOG_TAG, "got part length: " + partDataLen);
                    mPduDataStream.reset();
                    mPduDataStream.read(partData, 0, partDataLen);
                    String showData = new String(partData);
                    Log.i(LOG_TAG, "show data: " + showData);

                    theOnlyPart.setData(partData);
                    Log.i(LOG_TAG, "setData finish");
                    PduBody onlyPartBody = new PduBody();
                    onlyPartBody.addPart(theOnlyPart);
                    RetrieveConf retrieveConf = null;
                    try {
                        retrieveConf = new RetrieveConf(mHeaders, onlyPartBody);
                    } catch (Exception e) {
                        Log.i(LOG_TAG, "new RetrieveConf has exception");
                    }
                    if (retrieveConf == null) {
                        Log.i(LOG_TAG, "retrieveConf is null");
                    }
                    return retrieveConf;
                }

                // Here maybe we can add other content type support.
            }
        }
        /// @}

        if ((MtkPduHeaders.MESSAGE_TYPE_SEND_REQ == messageType) ||
                (MtkPduHeaders.MESSAGE_TYPE_RETRIEVE_CONF == messageType)) {
            /* need to parse the parts */
            mBody = parseParts(mPduDataStream);
            if (null == mBody) {
                // Parse parts failed.
                Log.i(LOG_TAG, "Parse parts Failed");
                return null;
            }
        }

        switch (messageType) {
            case MtkPduHeaders.MESSAGE_TYPE_SEND_REQ:
                if (LOCAL_LOGV) {
                    Log.v(LOG_TAG, "parse: MESSAGE_TYPE_SEND_REQ");
                }
                SendReq sendReq = new SendReq(mHeaders, mBody);
                return sendReq;
            case MtkPduHeaders.MESSAGE_TYPE_SEND_CONF:
                if (LOCAL_LOGV) {
                    Log.v(LOG_TAG, "parse: MESSAGE_TYPE_SEND_CONF");
                }
                MtkSendConf sendConf = new MtkSendConf(mHeaders);
                return sendConf;
            case MtkPduHeaders.MESSAGE_TYPE_NOTIFICATION_IND:
                if (LOCAL_LOGV) {
                    Log.v(LOG_TAG, "parse: MESSAGE_TYPE_NOTIFICATION_IND");
                }
                NotificationInd notificationInd =
                    new NotificationInd(mHeaders);
                return notificationInd;
            case MtkPduHeaders.MESSAGE_TYPE_NOTIFYRESP_IND:
                if (LOCAL_LOGV) {
                    Log.v(LOG_TAG, "parse: MESSAGE_TYPE_NOTIFYRESP_IND");
                }
                NotifyRespInd notifyRespInd =
                    new NotifyRespInd(mHeaders);
                return notifyRespInd;
            case MtkPduHeaders.MESSAGE_TYPE_RETRIEVE_CONF:
                if (LOCAL_LOGV) {
                    Log.v(LOG_TAG, "parse: MESSAGE_TYPE_RETRIEVE_CONF");
                }
                RetrieveConf retrieveConf =
                    new RetrieveConf(mHeaders, mBody);

                byte[] contentType = retrieveConf.getContentType();
                if (null == contentType) {
                    Log.i(LOG_TAG, "Parse MESSAGE_TYPE_RETRIEVE_CONF Failed: content Type is null");
                    return null;
                }
                String ctTypeStr = new String(contentType);
                /// M:Code analyze 004,make the string into lowercase,matching the content type @{
                ctTypeStr = ctTypeStr.toLowerCase();
                /// @}
                if (ctTypeStr.equals(ContentType.MULTIPART_MIXED)
                        || ctTypeStr.equals(ContentType.MULTIPART_RELATED)
                        || ctTypeStr.equals(ContentType.MULTIPART_ALTERNATIVE)
                        /// M:Code analyze 005,add a new content type text/plain
                        || ctTypeStr.equals(ContentType.TEXT_PLAIN)) {
                    // The MMS content type must be "application/vnd.wap.multipart.mixed"
                    // or "application/vnd.wap.multipart.related"
                    // or "application/vnd.wap.multipart.alternative"
                    return retrieveConf;
                } else if (ctTypeStr.equals(ContentType.MULTIPART_ALTERNATIVE)) {
                    // "application/vnd.wap.multipart.alternative"
                    // should take only the first part.
                    PduPart firstPart = mBody.getPart(0);
                    mBody.removeAll();
                    mBody.addPart(0, firstPart);
                    return retrieveConf;
                }
                Log.i(LOG_TAG, "Parse MESSAGE_TYPE_RETRIEVE_CONF Failed: content Type is null _2");
                return null;
            case MtkPduHeaders.MESSAGE_TYPE_DELIVERY_IND:
                if (LOCAL_LOGV) {
                    Log.v(LOG_TAG, "parse: MESSAGE_TYPE_DELIVERY_IND");
                }
                DeliveryInd deliveryInd =
                    new DeliveryInd(mHeaders);
                return deliveryInd;
            case MtkPduHeaders.MESSAGE_TYPE_ACKNOWLEDGE_IND:
                if (LOCAL_LOGV) {
                    Log.v(LOG_TAG, "parse: MESSAGE_TYPE_ACKNOWLEDGE_IND");
                }
                AcknowledgeInd acknowledgeInd =
                    new AcknowledgeInd(mHeaders);
                return acknowledgeInd;
            case MtkPduHeaders.MESSAGE_TYPE_READ_ORIG_IND:
                if (LOCAL_LOGV) {
                    Log.v(LOG_TAG, "parse: MESSAGE_TYPE_READ_ORIG_IND");
                }
                ReadOrigInd readOrigInd =
                    new ReadOrigInd(mHeaders);
                return readOrigInd;
            case MtkPduHeaders.MESSAGE_TYPE_READ_REC_IND:
                if (LOCAL_LOGV) {
                    Log.v(LOG_TAG, "parse: MESSAGE_TYPE_READ_REC_IND");
                }
                ReadRecInd readRecInd =
                    new ReadRecInd(mHeaders);
                return readRecInd;
            default:
                Log.d(LOG_TAG, "Parser doesn't support this message type in this version!");
            return null;
       }
    }
@Override
  /**
     * Parse content type parameters. For now we just support
     * four parameters used in mms: "type", "start", "name", "charset".
     *
     * @param pduDataStream pdu data input stream
     * @param map to store parameters of Content-Type field
     * @param length length of all the parameters
     */
    protected void parseContentTypeParams(ByteArrayInputStream pduDataStream,
            HashMap<Integer, Object> map, Integer length) {
        /**
         * From wap-230-wsp-20010705-a.pdf
         * Parameter = Typed-parameter | Untyped-parameter
         * Typed-parameter = Well-known-parameter-token Typed-value
         * the actual expected type of the value is implied by the well-known parameter
         * Well-known-parameter-token = Integer-value
         * the code values used for parameters are specified in the Assigned Numbers appendix
         * Typed-value = Compact-value | Text-value
         * In addition to the expected type, there may be no value.
         * If the value cannot be encoded using the expected type, it shall be encoded as text.
         * Compact-value = Integer-value |
         * Date-value | Delta-seconds-value | Q-value | Version-value |
         * Uri-value
         * Untyped-parameter = Token-text Untyped-value
         * the type of the value is unknown, but it shall be encoded as an integer,
         * if that is possible.
         * Untyped-value = Integer-value | Text-value
         */
        assert(null != pduDataStream);
        assert(length > 0);

        int startPos = pduDataStream.available();
        int tempPos = 0;
        int lastLen = length;
        while(0 < lastLen) {
            int param = pduDataStream.read();
            assert(-1 != param);
            lastLen--;

            switch (param) {
                /**
                 * From rfc2387, chapter 3.1
                 * The type parameter must be specified and its value is the MIME media
                 * type of the "root" body part. It permits a MIME user agent to
                 * determine the content-type without reference to the enclosed body
                 * part. If the value of the type parameter and the root body part's
                 * content-type differ then the User Agent's behavior is undefined.
                 *
                 * From wap-230-wsp-20010705-a.pdf
                 * type = Constrained-encoding
                 * Constrained-encoding = Extension-Media | Short-integer
                 * Extension-media = *TEXT End-of-string
                 */
                case PduPart.P_TYPE:
                case PduPart.P_CT_MR_TYPE:
                    pduDataStream.mark(1);
                    int first = extractByteValue(pduDataStream);
                    pduDataStream.reset();
                    if (first > TEXT_MAX) {
                        // Short-integer (well-known type)
                        int index = parseShortInteger(pduDataStream);

                        if (index < PduContentTypes.contentTypes.length) {
                            byte[] type = (PduContentTypes.contentTypes[index]).getBytes();
                            if ((null != type) && (null != map)) {
                              map.put(PduPart.P_TYPE, type);
                            }
                        } else {
                            //not support this type, ignore it.
                        }
                    } else {
                        // Text-String (extension-media)
                        byte[] type = parseWapString(pduDataStream, TYPE_TEXT_STRING);
                        if ((null != type) && (null != map)) {
                            map.put(PduPart.P_TYPE, type);
                        }
                    }

                    tempPos = pduDataStream.available();
                    lastLen = length - (startPos - tempPos);
                    break;

                    /**
                     * From oma-ts-mms-conf-v1_3.pdf, chapter 10.2.3.
                     * Start Parameter Referring to Presentation
                     *
                     * From rfc2387, chapter 3.2
                     * The start parameter, if given, is the content-ID of the compound
                     * object's "root". If not present the "root" is the first body part in
                     * the Multipart/Related entity. The "root" is the element the
                     * applications processes first.
                     *
                     * From wap-230-wsp-20010705-a.pdf
                     * start = Text-String
                     */
                case PduPart.P_START:
                case PduPart.P_DEP_START:
                    byte[] start = parseWapString(pduDataStream, TYPE_TEXT_STRING);
                    if ((null != start) && (null != map)) {
                        map.put(PduPart.P_START, start);
                    }

                    tempPos = pduDataStream.available();
                    lastLen = length - (startPos - tempPos);
                    break;

                    /**
                     * From oma-ts-mms-conf-v1_3.pdf
                     * In creation, the character set SHALL be either us-ascii
                     * (IANA MIBenum 3) or utf-8 (IANA MIBenum 106)[Unicode].
                     * In retrieval, both us-ascii and utf-8 SHALL be supported.
                     *
                     * From wap-230-wsp-20010705-a.pdf
                     * charset = Well-known-charset|Text-String
                     * Well-known-charset = Any-charset | Integer-value
                     * Both are encoded using values from Character Set
                     * Assignments table in Assigned Numbers
                     * Any-charset = <Octet 128>
                     * Equivalent to the special RFC2616 charset value "*"
                     */
                case PduPart.P_CHARSET:
                    pduDataStream.mark(1);
                    int firstValue = extractByteValue(pduDataStream);
                    pduDataStream.reset();
                    //Check first char
                    if (((firstValue > TEXT_MIN) && (firstValue < TEXT_MAX)) ||
                            (END_STRING_FLAG == firstValue)) {
                        //Text-String (extension-charset)
                        byte[] charsetStr = parseWapString(pduDataStream, TYPE_TEXT_STRING);
                        try {
                            int charsetInt = MtkCharacterSets.getMibEnumValue(
                                    new String(charsetStr));
                            Log.i(LOG_TAG, "Parse CharacterSets: charsetStr");
                            if (null != map) {
                              map.put(PduPart.P_CHARSET, charsetInt);
                            }
                        } catch (UnsupportedEncodingException e) {
                            // Not a well-known charset, use "*".
                            Log.e(LOG_TAG, Arrays.toString(charsetStr), e);
                            if (null != map) {
                              map.put(PduPart.P_CHARSET, MtkCharacterSets.ANY_CHARSET);
                            }
                        }
                    } else {
                        //Well-known-charset
                        int charset = (int) parseIntegerValue(pduDataStream);
                        if (map != null) {
                            Log.i(LOG_TAG, "Parse Well-known-charset: charset");
                            map.put(PduPart.P_CHARSET, charset);
                        }
                    }

                    tempPos = pduDataStream.available();
                    lastLen = length - (startPos - tempPos);
                    break;

                    /// M:Code analyze 006,add for new feature @{
                    /* From oma-ts-mms-conf-v1_3.pdf
                     * A name for multipart object SHALL be encoded using name-parameter
                     * for Content-Type header in WSP multipart headers.
                     *
                     * From wap-230-wsp-20010705-a.pdf
                     * name, filename, comment, domain, path = Text-String
                     */
                case PduPart.P_DEP_NAME:
                case PduPart.P_NAME:
                    byte[] name = parseWapString(pduDataStream, TYPE_TEXT_STRING);
                    if ((null != name) && (null != map)) {
                        map.put(PduPart.P_NAME, name);
                    }

                    tempPos = pduDataStream.available();
                    lastLen = length - (startPos - tempPos);
                    break;
                case PduPart.P_DEP_FILENAME:
                case PduPart.P_FILENAME:
                    byte[] fileName = parseWapString(pduDataStream, TYPE_TEXT_STRING);
                    if ((null != fileName) && (null != map)) {
                        map.put(PduPart.P_FILENAME, fileName);
                    }

                    tempPos = pduDataStream.available();
                    lastLen = length - (startPos - tempPos);
                    break;
                case PduPart.P_PATH:
                case PduPart.P_DEP_PATH:
                    byte[] path = parseWapString(pduDataStream, TYPE_TEXT_STRING);
                    if ((null != path) && (null != map)) {
                        map.put(PduPart.P_PATH, path);
                    }

                    tempPos = pduDataStream.available();
                    lastLen = length - (startPos - tempPos);
                    break;
                case PduPart.P_DEP_COMMENT:
                case PduPart.P_COMMENT:
                    byte[] comment = parseWapString(pduDataStream, TYPE_TEXT_STRING);
                    if ((null != comment) && (null != map)) {
                        map.put(PduPart.P_COMMENT, comment);
                    }

                    tempPos = pduDataStream.available();
                    lastLen = length - (startPos - tempPos);
                    break;
                case PduPart.P_DEP_DOMAIN:
                case PduPart.P_DOMAIN:
                    byte[] domain = parseWapString(pduDataStream, TYPE_TEXT_STRING);
                    if ((null != domain) && (null != map)) {
                        map.put(PduPart.P_DOMAIN, domain);
                    }

                    tempPos = pduDataStream.available();
                    lastLen = length - (startPos - tempPos);
                    break;
                    /// @}


                default:
                    if (LOCAL_LOGV) {
                        Log.v(LOG_TAG, "Not supported Content-Type parameter");
                    }
                if (-1 == skipWapValue(pduDataStream, lastLen)) {
                    Log.e(LOG_TAG, "Corrupt Content-Type");
                } else {
                    lastLen = 0;
                }
                break;
            }
        }

        if (0 != lastLen) {
            Log.e(LOG_TAG, "Corrupt Content-Type");
        }
    }

    /*[MTK MMS FW] OP01*/
    /**
     * Parse pdu headers.
     *
     * @param pduDataStream pdu data input stream
     * @return headers in PduHeaders structure, null when parse fail
     */
    protected PduHeaders parseHeaders(ByteArrayInputStream pduDataStream){
        if (pduDataStream == null) {
            return null;
        }
        boolean keepParsing = true;
        MtkPduHeaders headers = new MtkPduHeaders();

        while (keepParsing && (pduDataStream.available() > 0)) {
            pduDataStream.mark(1);
            int headerField = extractByteValue(pduDataStream);
            /* parse custom text header */
            if ((headerField >= TEXT_MIN) && (headerField <= TEXT_MAX)) {
                pduDataStream.reset();
                byte [] bVal = parseWapString(pduDataStream, TYPE_TEXT_STRING);
                if (LOCAL_LOGV) {
                    Log.v(LOG_TAG, "TextHeader: " + new String(bVal));
                }
                /* we should ignore it at the moment */
                continue;
            }
            switch (headerField) {
                case MtkPduHeaders.MESSAGE_TYPE:
                {
                    int messageType = extractByteValue(pduDataStream);
                    //if (LOCAL_LOGV) {
                        Log.d(LOG_TAG, "parseHeaders: messageType: " + messageType);
                    //}
                    switch (messageType) {
                        // We don't support these kind of messages now.
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
                            Log.i(LOG_TAG, "PduParser: parseHeaders: "
                                    + "We don't support these kind of messages now.");
                            return null;
                    }
                    try {
                        headers.setOctet(messageType, headerField);
                    } catch(InvalidHeaderValueException e) {
                        log("Set invalid Octet value: " + messageType +
                                " into the header filed: " + headerField);
                        return null;
                    } catch(RuntimeException e) {
                        log(headerField + "is not Octet header field!");
                        return null;
                    }
                    break;
                }
                /* Octect value */
                case MtkPduHeaders.REPORT_ALLOWED:
                case MtkPduHeaders.ADAPTATION_ALLOWED:
                case MtkPduHeaders.DELIVERY_REPORT:
                case MtkPduHeaders.DRM_CONTENT:
                case MtkPduHeaders.DISTRIBUTION_INDICATOR:
                case MtkPduHeaders.QUOTAS:
                case MtkPduHeaders.READ_REPORT:
                case MtkPduHeaders.STORE:
                case MtkPduHeaders.STORED:
                case MtkPduHeaders.TOTALS:
                case MtkPduHeaders.SENDER_VISIBILITY:
                case MtkPduHeaders.READ_STATUS:
                case MtkPduHeaders.CANCEL_STATUS:
                case MtkPduHeaders.PRIORITY:
                case MtkPduHeaders.STATUS:
                case MtkPduHeaders.REPLY_CHARGING:
                case MtkPduHeaders.MM_STATE:
                case MtkPduHeaders.RECOMMENDED_RETRIEVAL_MODE:
                case MtkPduHeaders.CONTENT_CLASS:
                case MtkPduHeaders.RETRIEVE_STATUS:
                case MtkPduHeaders.STORE_STATUS:
                    /**
                     * The following field has a different value when
                     * used in the M-Mbox-Delete.conf and M-Delete.conf PDU.
                     * For now we ignore this fact, since we do not support these PDUs
                     */
                case MtkPduHeaders.RESPONSE_STATUS:
                {
                    int value = extractByteValue(pduDataStream);
                    if (LOCAL_LOGV) {
                        Log.v(LOG_TAG, "parseHeaders: byte: " + headerField + " value: " +
                                value);
                    }

                    try {
                        headers.setOctet(value, headerField);
                    } catch(InvalidHeaderValueException e) {
                        log("Set invalid Octet value: " + value +
                                " into the header filed: " + headerField);
                        return null;
                    } catch(RuntimeException e) {
                        log(headerField + "is not Octet header field!");
                        return null;
                    }
                    break;
                }

                /* Long-Integer */
                case MtkPduHeaders.DATE:
                case MtkPduHeaders.REPLY_CHARGING_SIZE:
                case MtkPduHeaders.MESSAGE_SIZE:
                case MtkPduHeaders.DATE_SENT:
                {
                    Log.d(LOG_TAG, "parseHeaders " + headerField);
                    try {
                        long value = parseLongInteger(pduDataStream);
                        Log.d(LOG_TAG, "value = " + value);
                         /// M:Code analyze 002,Method for BackupRestore, for this application
                         /* 1.RetrieveConf's DATE should use the value from pdu instead of
                          *   setting system time directly
                          * 2.RetrivevConf's read status should use column "READ" in table pdu,
                          *   instead of "READ_STATUS" which is null in db*/
                        /// M: add for saving sent time of received messages,
                        /// no matter that mForRestore is true or false. @{
                        if (headerField == MtkPduHeaders.DATE) {
                            headers.setLongInteger(value, MtkPduHeaders.DATE_SENT);
                            /* Q0 migration*/
                            //if (mForRestore == false) {
                                //change date to local time
                            value = System.currentTimeMillis() / 1000;
                            //}
                        }
                        /// @}
                        headers.setLongInteger(value, headerField);
                    } catch(RuntimeException e) {
                        log(headerField + "is not Long-Integer header field!");
                        return null;
                    }
                    break;
                }

                /* Integer-Value */
                case MtkPduHeaders.MESSAGE_COUNT:
                case MtkPduHeaders.START:
                case MtkPduHeaders.LIMIT:
                {
                    try {
                        long value = parseIntegerValue(pduDataStream);
                        if (LOCAL_LOGV) {
                            Log.v(LOG_TAG, "parseHeaders: int: " + headerField + " value: " +
                                    value);
                        }
                        headers.setLongInteger(value, headerField);
                    } catch(RuntimeException e) {
                        log(headerField + "is not Long-Integer header field!");
                        return null;
                    }
                    break;
                }

                /* Text-String */
                case MtkPduHeaders.TRANSACTION_ID:
                case MtkPduHeaders.REPLY_CHARGING_ID:
                case MtkPduHeaders.AUX_APPLIC_ID:
                case MtkPduHeaders.APPLIC_ID:
                case MtkPduHeaders.REPLY_APPLIC_ID:
                    /**
                     * The next three header fields are email addresses
                     * as defined in RFC2822,
                     * not including the characters "<" and ">"
                     */
                case MtkPduHeaders.MESSAGE_ID:
                case MtkPduHeaders.REPLACE_ID:
                case MtkPduHeaders.CANCEL_ID:
                    /**
                     * The following field has a different value when
                     * used in the M-Mbox-Delete.conf and M-Delete.conf PDU.
                     * For now we ignore this fact, since we do not support these PDUs
                     */
                case MtkPduHeaders.CONTENT_LOCATION:
                {
                    byte[] value = parseWapString(pduDataStream, TYPE_TEXT_STRING);
                    if (null != value) {
                        try {
                            if (LOCAL_LOGV) {
                                Log.v(LOG_TAG, "parseHeaders: string: " + headerField + " value: " +
                                        new String(value));
                            }
                            headers.setTextString(value, headerField);
                        } catch(NullPointerException e) {
                            log("null pointer error!");
                        } catch(RuntimeException e) {
                            log(headerField + "is not Text-String header field!");
                            return null;
                        }
                    }
                    break;
                }

                /* Encoded-string-value */
                case MtkPduHeaders.SUBJECT:
                case MtkPduHeaders.RECOMMENDED_RETRIEVAL_MODE_TEXT:
                case MtkPduHeaders.RETRIEVE_TEXT:
                case MtkPduHeaders.STATUS_TEXT:
                case MtkPduHeaders.STORE_STATUS_TEXT:
                    /* the next one is not support
                     * M-Mbox-Delete.conf and M-Delete.conf now */
                case MtkPduHeaders.RESPONSE_TEXT:
                {
                    EncodedStringValue value =
                        parseEncodedStringValue(pduDataStream);
                    if (null != value) {
                        try {
                            if (LOCAL_LOGV) {
                                Log.v(LOG_TAG, "parseHeaders: encoded string: " + headerField
                                        + " value: " + value.getString());
                            }
                            headers.setEncodedStringValue(value, headerField);
                        } catch(NullPointerException e) {
                            log("null pointer error!");
                        } catch (RuntimeException e) {
                            log(headerField + "is not Encoded-String-Value header field!");
                            return null;
                        }
                    }
                    break;
                }

                /* Addressing model */
                case MtkPduHeaders.BCC:
                case MtkPduHeaders.CC:
                case MtkPduHeaders.TO:
                {
                    EncodedStringValue value =
                        parseEncodedStringValue(pduDataStream);
                    if (null != value) {
                        byte[] address = value.getTextString();
                        if (null != address) {
                            String str = new String(address);
                            if (LOCAL_LOGV) {
                                Log.v(LOG_TAG, "parseHeaders: (to/cc/bcc) address: " + headerField
                                        + " value: " + str);
                            }
                            int endIndex = str.indexOf("/");
                            if (endIndex > 0) {
                                str = str.substring(0, endIndex);
                            }
                            try {
                                value.setTextString(str.getBytes());
                            } catch(NullPointerException e) {
                                log("null pointer error!");
                                return null;
                            }
                        }

                        try {
                            headers.appendEncodedStringValue(value, headerField);
                        } catch(NullPointerException e) {
                            log("null pointer error!");
                        } catch(RuntimeException e) {
                            log(headerField + "is not Encoded-String-Value header field!");
                            return null;
                        }
                    }
                    break;
                }

                /* Value-length
                 * (Absolute-token Date-value | Relative-token Delta-seconds-value) */
                case MtkPduHeaders.DELIVERY_TIME:
                case MtkPduHeaders.EXPIRY:
                case MtkPduHeaders.REPLY_CHARGING_DEADLINE:
                {
                    /* parse Value-length */
                    parseValueLength(pduDataStream);

                    /* Absolute-token or Relative-token */
                    int token = extractByteValue(pduDataStream);

                    /* Date-value or Delta-seconds-value */
                    long timeValue;
                    try {
                        timeValue = parseLongInteger(pduDataStream);
                    } catch(RuntimeException e) {
                        log(headerField + "is not Long-Integer header field!");
                        return null;
                    }
                    if (MtkPduHeaders.VALUE_RELATIVE_TOKEN == token) {
                        /* need to convert the Delta-seconds-value
                         * into Date-value */
                        timeValue = System.currentTimeMillis()/1000 + timeValue;
                    }

                    try {
                        if (LOCAL_LOGV) {
                            Log.v(LOG_TAG, "parseHeaders: time value: " + headerField
                                    + " value: " + timeValue);
                        }
                        headers.setLongInteger(timeValue, headerField);
                    } catch(RuntimeException e) {
                        log(headerField + "is not Long-Integer header field!");
                        return null;
                    }
                    break;
                }

                case MtkPduHeaders.FROM: {
                    /* From-value =
                     * Value-length
                     * (Address-present-token Encoded-string-value | Insert-address-token)
                     */
                    EncodedStringValue from = null;
                    parseValueLength(pduDataStream); /* parse value-length */

                    /* Address-present-token or Insert-address-token */
                    int fromToken = extractByteValue(pduDataStream);

                    /* Address-present-token or Insert-address-token */
                    if (MtkPduHeaders.FROM_ADDRESS_PRESENT_TOKEN == fromToken) {
                        /* Encoded-string-value */
                        from = parseEncodedStringValue(pduDataStream);
                        if (null != from) {
                            byte[] address = from.getTextString();
                            if (null != address) {
                                String str = new String(address);
                                int endIndex = str.indexOf("/");
                                if (endIndex > 0) {
                                    str = str.substring(0, endIndex);
                                }
                                try {
                                    from.setTextString(str.getBytes());
                                } catch(NullPointerException e) {
                                    log("null pointer error!");
                                    return null;
                                }
                            }
                        }
                    } else {
                        try {
                            from = new EncodedStringValue(
                                    MtkPduHeaders.FROM_INSERT_ADDRESS_TOKEN_STR.getBytes());
                        } catch(NullPointerException e) {
                            log(headerField + "is not Encoded-String-Value header field!");
                            return null;
                        }
                    }

                    try {
                        if (LOCAL_LOGV) {
                            Log.v(LOG_TAG, "parseHeaders: from address: " + headerField
                                    + " value: " + from.getString());
                        }
                        headers.setEncodedStringValue(from, MtkPduHeaders.FROM);
                    } catch(NullPointerException e) {
                        log("null pointer error!");
                    } catch(RuntimeException e) {
                        log(headerField + "is not Encoded-String-Value header field!");
                        return null;
                    }
                    break;
                }

                case MtkPduHeaders.MESSAGE_CLASS: {
                    /* Message-class-value = Class-identifier | Token-text */
                    pduDataStream.mark(1);
                    int messageClass = extractByteValue(pduDataStream);
                    if (LOCAL_LOGV) {
                        Log.v(LOG_TAG, "parseHeaders: MESSAGE_CLASS: " + headerField
                                + " value: " + messageClass);
                    }

                    if (messageClass >= MtkPduHeaders.MESSAGE_CLASS_PERSONAL) {
                        /* Class-identifier */
                        try {
                            if (MtkPduHeaders.MESSAGE_CLASS_PERSONAL == messageClass) {
                                headers.setTextString(
                                        MtkPduHeaders.MESSAGE_CLASS_PERSONAL_STR.getBytes(),
                                        MtkPduHeaders.MESSAGE_CLASS);
                            } else if (MtkPduHeaders.MESSAGE_CLASS_ADVERTISEMENT == messageClass) {
                                headers.setTextString(
                                        MtkPduHeaders.MESSAGE_CLASS_ADVERTISEMENT_STR.getBytes(),
                                        MtkPduHeaders.MESSAGE_CLASS);
                            } else if (MtkPduHeaders.MESSAGE_CLASS_INFORMATIONAL == messageClass) {
                                headers.setTextString(
                                        MtkPduHeaders.MESSAGE_CLASS_INFORMATIONAL_STR.getBytes(),
                                        MtkPduHeaders.MESSAGE_CLASS);
                            } else if (MtkPduHeaders.MESSAGE_CLASS_AUTO == messageClass) {
                                headers.setTextString(
                                        MtkPduHeaders.MESSAGE_CLASS_AUTO_STR.getBytes(),
                                        MtkPduHeaders.MESSAGE_CLASS);
                            }
                        } catch(NullPointerException e) {
                            log("null pointer error!");
                        } catch(RuntimeException e) {
                            log(headerField + "is not Text-String header field!");
                            return null;
                        }
                    } else {
                        /* Token-text */
                        pduDataStream.reset();
                        byte[] messageClassString = parseWapString(pduDataStream, TYPE_TEXT_STRING);
                        if (null != messageClassString) {
                            try {
                                headers.setTextString(messageClassString,
                                        MtkPduHeaders.MESSAGE_CLASS);
                            } catch(NullPointerException e) {
                                log("null pointer error!");
                            } catch(RuntimeException e) {
                                log(headerField + "is not Text-String header field!");
                                return null;
                            }
                        }
                    }
                    break;
                }

                case MtkPduHeaders.MMS_VERSION: {
                    int version = parseShortInteger(pduDataStream);

                    try {
                        if (LOCAL_LOGV) {
                            Log.v(LOG_TAG, "parseHeaders: MMS_VERSION: " + headerField
                                    + " value: " + version);
                        }
                        headers.setOctet(version, MtkPduHeaders.MMS_VERSION);
                    } catch(InvalidHeaderValueException e) {
                        log("Set invalid Octet value: " + version +
                                " into the header filed: " + headerField);
                        return null;
                    } catch(RuntimeException e) {
                        log(headerField + "is not Octet header field!");
                        return null;
                    }
                    break;
                }

                case MtkPduHeaders.PREVIOUSLY_SENT_BY: {
                    /* Previously-sent-by-value =
                     * Value-length Forwarded-count-value Encoded-string-value */
                    /* parse value-length */
                    parseValueLength(pduDataStream);

                    /* parse Forwarded-count-value */
                    try {
                        parseIntegerValue(pduDataStream);
                    } catch(RuntimeException e) {
                        log(headerField + " is not Integer-Value");
                        return null;
                    }

                    /* parse Encoded-string-value */
                    EncodedStringValue previouslySentBy =
                        parseEncodedStringValue(pduDataStream);
                    if (null != previouslySentBy) {
                        try {
                            if (LOCAL_LOGV) {
                                Log.v(LOG_TAG, "parseHeaders: PREVIOUSLY_SENT_BY: " + headerField
                                        + " value: " + previouslySentBy.getString());
                            }
                            headers.setEncodedStringValue(previouslySentBy,
                                    MtkPduHeaders.PREVIOUSLY_SENT_BY);
                        } catch(NullPointerException e) {
                            log("null pointer error!");
                        } catch(RuntimeException e) {
                            log(headerField + "is not Encoded-String-Value header field!");
                            return null;
                        }
                    }
                    break;
                }

                case MtkPduHeaders.PREVIOUSLY_SENT_DATE: {
                    /* Previously-sent-date-value =
                     * Value-length Forwarded-count-value Date-value */
                    /* parse value-length */
                    parseValueLength(pduDataStream);

                    /* parse Forwarded-count-value */
                    try {
                        parseIntegerValue(pduDataStream);
                    } catch(RuntimeException e) {
                        log(headerField + " is not Integer-Value");
                        return null;
                    }

                    /* Date-value */
                    try {
                        long perviouslySentDate = parseLongInteger(pduDataStream);
                        if (LOCAL_LOGV) {
                            Log.v(LOG_TAG, "parseHeaders: PREVIOUSLY_SENT_DATE: " + headerField
                                    + " value: " + perviouslySentDate);
                        }
                        headers.setLongInteger(perviouslySentDate,
                                MtkPduHeaders.PREVIOUSLY_SENT_DATE);
                    } catch(RuntimeException e) {
                        log(headerField + "is not Long-Integer header field!");
                        return null;
                    }
                    break;
                }

                case MtkPduHeaders.MM_FLAGS: {
                    /* MM-flags-value =
                     * Value-length
                     * ( Add-token | Remove-token | Filter-token )
                     * Encoded-string-value
                     */
                    if (LOCAL_LOGV) {
                        Log.v(LOG_TAG, "parseHeaders: MM_FLAGS: " + headerField
                                + " NOT REALLY SUPPORTED");
                    }

                    /* parse Value-length */
                    parseValueLength(pduDataStream);

                    /* Add-token | Remove-token | Filter-token */
                    extractByteValue(pduDataStream);

                    /* Encoded-string-value */
                    parseEncodedStringValue(pduDataStream);

                    /* not store this header filed in "headers",
                     * because now MtkPduHeaders doesn't support it */
                    break;
                }

                /* Value-length
                 * (Message-total-token | Size-total-token) Integer-Value */
                case MtkPduHeaders.MBOX_TOTALS:
                case MtkPduHeaders.MBOX_QUOTAS:
                {
                    if (LOCAL_LOGV) {
                        Log.v(LOG_TAG, "parseHeaders: MBOX_TOTALS: " + headerField);
                    }
                    /* Value-length */
                    parseValueLength(pduDataStream);

                    /* Message-total-token | Size-total-token */
                    extractByteValue(pduDataStream);

                    /*Integer-Value*/
                    try {
                        parseIntegerValue(pduDataStream);
                    } catch(RuntimeException e) {
                        log(headerField + " is not Integer-Value");
                        return null;
                    }

                    /* not store these headers filed in "headers",
                    because now PduHeaders doesn't support them */
                    break;
                }

                case MtkPduHeaders.ELEMENT_DESCRIPTOR: {
                    if (LOCAL_LOGV) {
                        Log.v(LOG_TAG, "parseHeaders: ELEMENT_DESCRIPTOR: " + headerField);
                    }
                    parseContentType(pduDataStream, null);

                    /* not store this header filed in "headers",
                    because now PduHeaders doesn't support it */
                    break;
                }

                case MtkPduHeaders.CONTENT_TYPE: {
                    HashMap<Integer, Object> map =
                        new HashMap<Integer, Object>();
                    byte[] contentType =
                        parseContentType(pduDataStream, map);

                    if (null != contentType) {
                        try {
                            if (LOCAL_LOGV) {
                                Log.v(LOG_TAG, "parseHeaders: CONTENT_TYPE: " + headerField +
                                        contentType.toString());
                            }
                            headers.setTextString(contentType, MtkPduHeaders.CONTENT_TYPE);
                        } catch(NullPointerException e) {
                            log("null pointer error!");
                        } catch(RuntimeException e) {
                            log(headerField + "is not Text-String header field!");
                            return null;
                        }
                    }

                    /* get start parameter */
                    mStartParam = (byte[]) map.get(PduPart.P_START);

                    /* get charset parameter */
                    mTypeParam= (byte[]) map.get(PduPart.P_TYPE);

                    keepParsing = false;
                    break;
                }

                case MtkPduHeaders.CONTENT:
                case MtkPduHeaders.ADDITIONAL_HEADERS:
                case MtkPduHeaders.ATTRIBUTES:
                default: {
                    if (LOCAL_LOGV) {
                        Log.v(LOG_TAG, "parseHeaders: Unknown header: " + headerField);
                    }
                    log("Unknown header");
                }
            }
        }

        return headers;
    }

    /// M:Code analyze 002,Method for BackupRestore, for this application @{
     /* 1.RetrieveConf's DATE should use the value from pdu instead of
     *   setting system time directly
     * 2.RetrivevConf's read status should use column "READ" in table pdu,
     *   instead of "READ_STATUS" which is null in db*/
    /* Q0 migration*/
    /*
    public GenericPdu parse(boolean forRestore) {
        mForRestore = forRestore;
        return parse();
    }
    */
    /// @}
    /*[MTK MMS FW] OP01 end*/

  /**
     * Parse part's headers.
     *
     * @param pduDataStream pdu data input stream
     * @param part to store the header informations of the part
     * @param length length of the headers
     * @return true if parse successfully, false otherwise
     */
    protected boolean parsePartHeaders(ByteArrayInputStream pduDataStream,
            PduPart part, int length) {
        assert(null != pduDataStream);
        assert(null != part);
        assert(length > 0);

        /**
         * From oma-ts-mms-conf-v1_3.pdf, chapter 10.2.
         * A name for multipart object SHALL be encoded using name-parameter
         * for Content-Type header in WSP multipart headers.
         * In decoding, name-parameter of Content-Type SHALL be used if available.
         * If name-parameter of Content-Type is not available,
         * filename parameter of Content-Disposition header SHALL be used if available.
         * If neither name-parameter of Content-Type header nor filename parameter
         * of Content-Disposition header is available,
         * Content-Location header SHALL be used if available.
         *
         * Within SMIL part the reference to the media object parts SHALL use
         * either Content-ID or Content-Location mechanism [RFC2557]
         * and the corresponding WSP part headers in media object parts
         * contain the corresponding definitions.
         */
        int startPos = pduDataStream.available();
        int tempPos = 0;
        int lastLen = length;
        while(0 < lastLen) {
            int header = pduDataStream.read();
            assert(-1 != header);
            lastLen--;
            Log.v(LOG_TAG, "Part headers: " + header);
            if (header > TEXT_MAX) {
                // Number assigned headers.
                switch (header) {
                    case PduPart.P_CONTENT_LOCATION:
                        /**
                         * From wap-230-wsp-20010705-a.pdf, chapter 8.4.2.21
                         * Content-location-value = Uri-value
                         */
                        byte[] contentLocation = parseWapString(pduDataStream, TYPE_TEXT_STRING);
                        if (null != contentLocation) {
                            part.setContentLocation(contentLocation);
                        }

                        tempPos = pduDataStream.available();
                        lastLen = length - (startPos - tempPos);
                        break;
                    case PduPart.P_CONTENT_ID:
                        /**
                         * From wap-230-wsp-20010705-a.pdf, chapter 8.4.2.21
                         * Content-ID-value = Quoted-string
                         */
                        byte[] contentId = parseWapString(pduDataStream, TYPE_QUOTED_STRING);
                        if (null != contentId) {
                            part.setContentId(contentId);
                        }

                        tempPos = pduDataStream.available();
                        lastLen = length - (startPos - tempPos);
                        break;
                    case MtkPduPart.P_TRANSFER_ENCODING:
                        byte[] transferEncoding = parseWapString(pduDataStream, TYPE_TEXT_STRING);
                        if (null != transferEncoding) {
                            part.setContentTransferEncoding(transferEncoding);
                        }
                        tempPos = pduDataStream.available();
                        lastLen = length - (startPos - tempPos);
                        break;
                    case PduPart.P_DEP_CONTENT_DISPOSITION:
                    case PduPart.P_CONTENT_DISPOSITION:
                        /**
                         * From wap-230-wsp-20010705-a.pdf, chapter 8.4.2.21
                         * Content-disposition-value = Value-length Disposition *(Parameter)
                         * Disposition = Form-data | Attachment | Inline | Token-text
                         * Form-data = <Octet 128>
                         * Attachment = <Octet 129>
                         * Inline = <Octet 130>
                         */

                        /*
                         * some carrier mmsc servers do not support content_disposition
                         * field correctly
                         */
                        if (mParseContentDisposition) {
                            int len = parseValueLength(pduDataStream);
                            pduDataStream.mark(1);
                            int thisStartPos = pduDataStream.available();
                            int thisEndPos = 0;
                            int value = pduDataStream.read();

                            if (value == PduPart.P_DISPOSITION_FROM_DATA ) {
                                part.setContentDisposition(PduPart.DISPOSITION_FROM_DATA);
                            } else if (value == PduPart.P_DISPOSITION_ATTACHMENT) {
                                part.setContentDisposition(PduPart.DISPOSITION_ATTACHMENT);
                            } else if (value == PduPart.P_DISPOSITION_INLINE) {
                                part.setContentDisposition(PduPart.DISPOSITION_INLINE);
                            } else {
                                pduDataStream.reset();
                                /* Token-text */
                                part.setContentDisposition(parseWapString(pduDataStream
                                        , TYPE_TEXT_STRING));
                            }

                            /* get filename parameter and skip other parameters */
                            thisEndPos = pduDataStream.available();
                            if (thisStartPos - thisEndPos < len) {
                                value = pduDataStream.read();
                                if (value == PduPart.P_FILENAME) { //filename is text-string
                                    part.setFilename(parseWapString(pduDataStream
                                            , TYPE_TEXT_STRING));
                                }

                                /* skip other parameters */
                                thisEndPos = pduDataStream.available();
                                if (thisStartPos - thisEndPos < len) {
                                    int last = len - (thisStartPos - thisEndPos);
                                    byte[] temp = new byte[last];
                                    pduDataStream.read(temp, 0, last);
                                }
                            }

                            tempPos = pduDataStream.available();
                            lastLen = length - (startPos - tempPos);
                        }
                        break;
                    default:
                        if (LOCAL_LOGV) {
                            Log.v(LOG_TAG, "Not supported Part headers: " + header);
                        }
                    if (-1 == skipWapValue(pduDataStream, lastLen)) {
                        Log.e(LOG_TAG, "Corrupt Part headers");
                        return false;
                    }
                    lastLen = 0;
                    break;
                }
            } else if ((header >= TEXT_MIN) && (header <= TEXT_MAX)) {
                // Not assigned header.
                byte[] tempHeader = parseWapString(pduDataStream, TYPE_TEXT_STRING);
                byte[] tempValue = parseWapString(pduDataStream, TYPE_TEXT_STRING);

                // Check the header whether it is "Content-Transfer-Encoding".
                if (true ==
                    PduPart.CONTENT_TRANSFER_ENCODING.equalsIgnoreCase(new String(tempHeader))) {
                    part.setContentTransferEncoding(tempValue);
                }

                tempPos = pduDataStream.available();
                lastLen = length - (startPos - tempPos);
            } else {
                if (LOCAL_LOGV) {
                    Log.v(LOG_TAG, "Not supported Part headers: " + header);
                }
                // Skip all headers of this part.
                if (-1 == skipWapValue(pduDataStream, lastLen)) {
                    Log.e(LOG_TAG, "Corrupt Part headers");
                    return false;
                }
                lastLen = 0;
            }
        }

        if (0 != lastLen) {
            Log.e(LOG_TAG, "Corrupt Part headers");
            return false;
        }

        return true;
    }
}
