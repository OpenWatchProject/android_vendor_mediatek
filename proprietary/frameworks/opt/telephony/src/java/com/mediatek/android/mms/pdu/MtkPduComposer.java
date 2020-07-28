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
import android.content.Context;
import android.util.Log;
import com.google.android.mms.pdu.PduComposer;
import com.google.android.mms.pdu.RetrieveConf;
import com.google.android.mms.pdu.SendReq;
import com.google.android.mms.pdu.PduBody;
import com.google.android.mms.pdu.PduPart;
import com.google.android.mms.pdu.EncodedStringValue;
import java.io.ByteArrayOutputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import com.google.android.mms.pdu.GenericPdu;
import com.google.android.mms.pdu.NotificationInd;


public class MtkPduComposer extends PduComposer {
    private static final String LOG_TAG = "MtkPduComposer";
    public MtkPduComposer(Context context, GenericPdu pdu) {
        super(context, pdu);
    }

     /**
     * Make the message. No need to check whether mandatory fields are set,
     * because the constructors of outgoing pdus are taking care of this.
     *
     * @return OutputStream of maked message. Return null if
     *         the PDU is invalid.
     */
    public byte[] make() {
        // Get Message-type.
        int type = mPdu.getMessageType();

        Log.d(LOG_TAG, "make, type = " + type);

        /* make the message */
        switch (type) {
            case MtkPduHeaders.MESSAGE_TYPE_SEND_REQ:
            case MtkPduHeaders.MESSAGE_TYPE_RETRIEVE_CONF:
                /* Q0 migration*/
                if (makeSendRetrievePdu(type) != PduComposer.PDU_COMPOSE_SUCCESS) {
                    return null;
                }
                break;
            /// M:Code analyze 003,add a new branch for composing notifyInd pdu @{
            case MtkPduHeaders.MESSAGE_TYPE_NOTIFICATION_IND:
                if (makeNotifyIndEx() != PduComposer.PDU_COMPOSE_SUCCESS) {
                    return null;
                }
                break;
            /// @}
            case MtkPduHeaders.MESSAGE_TYPE_NOTIFYRESP_IND:
                if (makeNotifyRespEx() != PduComposer.PDU_COMPOSE_SUCCESS) {
                    return null;
                }
                break;
            case MtkPduHeaders.MESSAGE_TYPE_ACKNOWLEDGE_IND:
                if (makeAckInd() != PduComposer.PDU_COMPOSE_SUCCESS) {
                    return null;
                }
                break;
            case MtkPduHeaders.MESSAGE_TYPE_READ_REC_IND:
                if (makeReadRecInd() != PduComposer.PDU_COMPOSE_SUCCESS) {
                    return null;
                }
                break;
            /* Q0 migration*/
            /*
            /// M:Code analyze 004,add a new branch for composing retrieve conf pdu @{
            case MtkPduHeaders.MESSAGE_TYPE_RETRIEVE_CONF:
                if (makeRetrievePduEx() != PduComposer.PDU_COMPOSE_SUCCESS) {
                    return null;
                }
                break;
                */
            /// @}
            default:
                return null;
        }

        return mMessage.toByteArray();
    }

     /**
     * Make Send.req.
     */
    private int makeSendReqPduEx() {
        if (mMessage == null) {
            mMessage = new ByteArrayOutputStream();
            mPosition = 0;
        }

        // X-Mms-Message-Type
        appendOctet(MtkPduHeaders.MESSAGE_TYPE);
        appendOctet(MtkPduHeaders.MESSAGE_TYPE_SEND_REQ);

        // X-Mms-Transaction-ID
        appendOctet(MtkPduHeaders.TRANSACTION_ID);

        byte[] trid = mPduHeader.getTextString(MtkPduHeaders.TRANSACTION_ID);
        if (trid == null) {
            // Transaction-ID should be set(by Transaction) before make().
            throw new IllegalArgumentException("Transaction-ID is null.");
        }
        appendTextString(trid);

        //  X-Mms-MMS-Version
        if (appendHeader(MtkPduHeaders.MMS_VERSION) != PduComposer.PDU_COMPOSE_SUCCESS) {
            return PduComposer.PDU_COMPOSE_CONTENT_ERROR;
        }

        // Date Date-value Optional.
        appendHeader(MtkPduHeaders.DATE);

        // From
        if (appendHeader(MtkPduHeaders.FROM) != PduComposer.PDU_COMPOSE_SUCCESS) {
            return PduComposer.PDU_COMPOSE_CONTENT_ERROR;
        }

        boolean recipient = false;

        // To
        if (appendHeader(MtkPduHeaders.TO) != PduComposer.PDU_COMPOSE_CONTENT_ERROR) {
            recipient = true;
        }

        // Cc
        if (appendHeader(MtkPduHeaders.CC) != PduComposer.PDU_COMPOSE_CONTENT_ERROR) {
            recipient = true;
        }

        // Bcc
        if (appendHeader(MtkPduHeaders.BCC) != PduComposer.PDU_COMPOSE_CONTENT_ERROR) {
            recipient = true;
        }

        // Need at least one of "cc", "bcc" and "to".
        if (false == recipient) {
            return PduComposer.PDU_COMPOSE_CONTENT_ERROR;
        }

        // Subject Optional
        appendHeader(MtkPduHeaders.SUBJECT);

        // X-Mms-Message-Class Optional
        // Message-class-value = Class-identifier | Token-text
        appendHeader(MtkPduHeaders.MESSAGE_CLASS);

        // X-Mms-Expiry Optional
        appendHeader(MtkPduHeaders.EXPIRY);

        // X-Mms-Priority Optional
        appendHeader(MtkPduHeaders.PRIORITY);

        // X-Mms-Delivery-Report Optional
        appendHeader(MtkPduHeaders.DELIVERY_REPORT);

        // X-Mms-Read-Report Optional
        appendHeader(MtkPduHeaders.READ_REPORT);

        //    Content-Type
        appendOctet(MtkPduHeaders.CONTENT_TYPE);

        //  Message body
        makeMessageBodyEx(2);

        return PDU_COMPOSE_SUCCESS;  // Composing the message is OK
    }

     /// M:Code analyze 006,add a new method for composing notifyInd pdu @{
    /**
     * Make NotifyInd.Ind.
     */
    private int makeNotifyIndEx() {
        if (mMessage == null) {
            mMessage = new ByteArrayOutputStream();
            mPosition = 0;
        }

        //    X-Mms-Message-Type
        appendOctet(MtkPduHeaders.MESSAGE_TYPE);
        appendOctet(MtkPduHeaders.MESSAGE_TYPE_NOTIFICATION_IND);

        // X-Mms-Transaction-ID
        if (appendHeader(MtkPduHeaders.TRANSACTION_ID) != PduComposer.PDU_COMPOSE_SUCCESS) {
            return PduComposer.PDU_COMPOSE_CONTENT_ERROR;
        }

        // X-Mms-MMS-Version
        if (appendHeader(MtkPduHeaders.MMS_VERSION) != PduComposer.PDU_COMPOSE_SUCCESS) {
            return PduComposer.PDU_COMPOSE_CONTENT_ERROR;
        }
        //  X-Mms-Class
        // Message-class-value = personal
        if (appendHeader(MtkPduHeaders.MESSAGE_CLASS) != PduComposer.PDU_COMPOSE_SUCCESS) {
            return PduComposer.PDU_COMPOSE_CONTENT_ERROR;
        }
        // X-Mms-Message-Size
        appendOctet(MtkPduHeaders.MESSAGE_SIZE);
        long size = ((NotificationInd) mPdu).getMessageSize();
        appendLongInteger(size);
        // X-Mms-Expiry Optional
        if (appendHeader(MtkPduHeaders.EXPIRY) != PduComposer.PDU_COMPOSE_SUCCESS) {
            return PduComposer.PDU_COMPOSE_CONTENT_ERROR;
        }
        // X-Mms-Content-location
        appendOctet(MtkPduHeaders.CONTENT_LOCATION);
        byte[] contentLocation = ((NotificationInd) mPdu).getContentLocation();
        if (contentLocation != null) {
        Log.d(LOG_TAG, "makeNotifyIndEx contentLocation != null");
           appendTextString(contentLocation);
        } else {
           Log.d(LOG_TAG, "makeNotifyIndEx contentLocation  = null");
        }
        //  X-Mms-Subject
        EncodedStringValue subject =
                       ((NotificationInd) mPdu).getSubject();
        if (subject != null) {
           Log.d(LOG_TAG, "makeNotifyIndEx subject != null");
           appendOctet(MtkPduHeaders.SUBJECT);
           appendEncodedString(subject);
        } else {
           Log.d(LOG_TAG, "makeNotifyIndEx subject  = null");
        }

        // Date Date-value Optional.
        appendHeader(MtkPduHeaders.DATE);

        //  X-Mms-From
        if (appendHeader(MtkPduHeaders.FROM) != PduComposer.PDU_COMPOSE_SUCCESS) {
            return PduComposer.PDU_COMPOSE_CONTENT_ERROR;
        }
        //  X-Mms-Status
        if (appendHeader(MtkPduHeaders.STATUS) != PduComposer.PDU_COMPOSE_SUCCESS) {
            return PduComposer.PDU_COMPOSE_CONTENT_ERROR;
        }

        return PduComposer.PDU_COMPOSE_SUCCESS;
    }

        /// M:Code analyze 008,add a new method for composing retrieve pdu @{
    private int makeRetrievePduEx() {
        Log.d(LOG_TAG, "makeRetrievePduEx begin");
        if (mMessage == null) {
            mMessage = new ByteArrayOutputStream();
            mPosition = 0;
        }

        // X-Mms-Message-Type
        appendOctet(MtkPduHeaders.MESSAGE_TYPE);
        appendOctet(MtkPduHeaders.MESSAGE_TYPE_RETRIEVE_CONF);

        byte[] trid = mPduHeader.getTextString(MtkPduHeaders.TRANSACTION_ID);
        if (trid == null) {
            // Transaction-ID should be set(by Transaction) before make().
            Log.d(LOG_TAG, "Transaction ID is null");
        } else {
            // X-Mms-Transaction-ID
            appendOctet(MtkPduHeaders.TRANSACTION_ID);
            appendTextString(trid);
        }

        //  X-Mms-MMS-Version
        if (appendHeader(MtkPduHeaders.MMS_VERSION) != PDU_COMPOSE_SUCCESS) {
            return PduComposer.PDU_COMPOSE_CONTENT_ERROR;
        }

        // Date Date-value Optional.
        appendHeader(MtkPduHeaders.DATE);

        // From
        if (appendHeader(MtkPduHeaders.FROM) != PduComposer.PDU_COMPOSE_SUCCESS) {
            return PduComposer.PDU_COMPOSE_CONTENT_ERROR;
        }

        boolean recipient = false;

        // To
        if (appendHeader(MtkPduHeaders.TO) != PduComposer.PDU_COMPOSE_CONTENT_ERROR) {
            recipient = true;
        }

        // Cc
        if (appendHeader(MtkPduHeaders.CC) != PduComposer.PDU_COMPOSE_CONTENT_ERROR) {
            recipient = true;
        }

        // Bcc
        if (appendHeader(MtkPduHeaders.BCC) != PduComposer.PDU_COMPOSE_CONTENT_ERROR) {
            recipient = true;
        }

        // Need at least one of "cc", "bcc" and "to".
        if (false == recipient) {
            return PduComposer.PDU_COMPOSE_CONTENT_ERROR;
        }

        // Subject Optional
        appendHeader(MtkPduHeaders.SUBJECT);

        // X-Mms-Message-Class Optional
        // Message-class-value = Class-identifier | Token-text
        appendHeader(MtkPduHeaders.MESSAGE_CLASS);

        // X-Mms-Expiry Optional
        appendHeader(MtkPduHeaders.EXPIRY);

        // X-Mms-Priority Optional
        appendHeader(MtkPduHeaders.PRIORITY);

        // X-Mms-Delivery-Report Optional
        appendHeader(MtkPduHeaders.DELIVERY_REPORT);

        // X-Mms-Read-Report Optional
        appendHeader(MtkPduHeaders.READ_REPORT);

        /*[MTK MMS FW] OP01*/
        /* Method for BackupRestore, for this application,
         * 1.RetrieveConf's DATE should use the value from pdu instead of
         *   setting system time directly
         * 2.RetrivevConf's read status should use column "READ" in table pdu,
         *   instead of "READ_STATUS" which is null in db*/
        /* Q0 migration*/
        /*
        if (mForBackup == true) {
            appendHeader(MtkPduHeaders.READ_STATUS);
            Log.d(LOG_TAG, "set DATE_SENT");
            appendHeader(MtkPduHeaders.DATE_SENT);
        }
        */
        //    Content-Type
        appendOctet(MtkPduHeaders.CONTENT_TYPE);

        //  Message body
        makeMessageBodyEx(1);
        Log.d(LOG_TAG, "makeRetrievePduEx end");
        return PduComposer.PDU_COMPOSE_SUCCESS;  // Composing the message is OK
    }

    /*
     * M:Code analyze 009,add a parameter into the method
     * for distiguish send pdu or retrieve pdu @{
     */
    /**
     * Make message body.
     */
    private int makeMessageBodyEx(int type) {
        // 1. add body informations
        mStack.newbuf();  // Switching buffer because we need to

        PositionMarker ctStart = mStack.mark();

        // This contentTypeIdentifier should be used for type of attachment...
        String contentType = new String(mPduHeader.getTextString(MtkPduHeaders.CONTENT_TYPE));
        Integer contentTypeIdentifier = mContentTypeMap.get(contentType);
        if (contentTypeIdentifier == null) {
            // content type is mandatory
            return PduComposer.PDU_COMPOSE_CONTENT_ERROR;
        }

        appendShortInteger(contentTypeIdentifier.intValue());

        /// M:Code analyze 009,add a parameter into the method
        /// for distiguish send pdu or retrieve pdu @{
        PduBody body = null;

        if (type == MtkPduHeaders.MESSAGE_TYPE_RETRIEVE_CONF) {
            body = ((RetrieveConf) mPdu).getBody();
        } else {
            body = ((SendReq) mPdu).getBody();
        }
        /// @}

        if (null == body || body.getPartsNum() == 0) {
            Log.d(LOG_TAG, "makeMessageBodyEx body == null");
            // empty message
            appendUintvarInteger(0);
            mStack.pop();
            mStack.copy();
            return PduComposer.PDU_COMPOSE_SUCCESS;
        }

        PduPart part;
        try {
            part = (PduPart) body.getPart(0);

            byte[] start = part.getContentId();
            if (start != null) {
                appendOctet(MtkPduPart.P_DEP_START);
                if (('<' == start[0]) && ('>' == start[start.length - 1])) {
                    appendTextString(start);
                } else {
                    appendTextString("<" + new String(start) + ">");
                }
            }

            // content-type parameter: type
            appendOctet(MtkPduPart.P_CT_MR_TYPE);
            appendTextString(part.getContentType());
        }
        catch (ArrayIndexOutOfBoundsException e) {
            e.printStackTrace();
        }

        int ctLength = ctStart.getLength();
        mStack.pop();
        appendValueLength(ctLength);
        mStack.copy();

        // 3. add content
        int partNum = body.getPartsNum();
        appendUintvarInteger(partNum);
        for (int i = 0; i < partNum; i++) {
            part = (MtkPduPart) body.getPart(i);
            mStack.newbuf();  // Leaving space for header lengh and data length
            PositionMarker attachment = mStack.mark();

            mStack.newbuf();  // Leaving space for Content-Type length
            PositionMarker contentTypeBegin = mStack.mark();

            byte[] partContentType = part.getContentType();

            if (partContentType == null) {
                // content type is mandatory
                return PDU_COMPOSE_CONTENT_ERROR;
            }

            // content-type value
            Integer partContentTypeIdentifier =
                mContentTypeMap.get(new String(partContentType));
            if (partContentTypeIdentifier == null) {
                appendTextString(partContentType);
            } else {
                appendShortInteger(partContentTypeIdentifier.intValue());
            }

            /* Content-type parameter : name.
             * The value of name, filename, content-location is the same.
             * Just one of them is enough for this PDU.
             */
            byte[] name = part.getName();

            /// M:Code analyze 010,add a extra judgement if it is null @{
            if (null == name || name.length == 0) {
                name = part.getFilename();

                if (null == name || name.length == 0) {
                    name = part.getContentLocation();
                    if (null == name || name.length == 0) {
                         name = part.getContentId();
                         if (name != null && name.length != 0) {
                             Log.d(LOG_TAG, "makeMessageBodyEx name 1= " + name.toString());
                         } else {
                              /* at lease one of name, filename, Content-location
                               * should be available.
                               */
                              return MtkPduComposer.PDU_COMPOSE_CONTENT_ERROR;
                         }
                    }
                }
            }
            /// @}
            if (name != null && name.length != 0) {
                Log.d(LOG_TAG, "makeMessageBodyEx name 2= " + name.toString());
            }
            appendOctet(MtkPduPart.P_DEP_NAME);
            appendTextString(name);

            // content-type parameter : charset
            int charset = part.getCharset();
            if (charset != 0) {
                appendOctet(MtkPduPart.P_CHARSET);
                appendShortInteger(charset);
            }

            int contentTypeLength = contentTypeBegin.getLength();
            mStack.pop();
            appendValueLength(contentTypeLength);
            mStack.copy();

            // content id
            byte[] contentId = part.getContentId();

            /// M:Code analyze 010,add a extra judgement if it is null
            if (null != contentId && contentId.length != 0) {
                appendOctet(MtkPduPart.P_CONTENT_ID);
                if (('<' == contentId[0]) && ('>' == contentId[contentId.length - 1])) {
                    appendQuotedString(contentId);
                } else {
                    appendQuotedString("<" + new String(contentId) + ">");
                }
            }

            // content-location
            byte[] contentLocation = part.getContentLocation();
            /// M:Code analyze 010,add a extra judgement if it is null
            if (null != contentLocation && contentLocation.length != 0) {
                appendOctet(MtkPduPart.P_CONTENT_LOCATION);
                appendTextString(contentLocation);
            }

            // content
            int headerLength = attachment.getLength();

            int dataLength = 0; // Just for safety...
            byte[] partData = part.getData();

            if (partData != null) {
                arraycopy(partData, 0, partData.length);
                dataLength = partData.length;
            } else {
                InputStream cr = null;
                try {
                    byte[] buffer = new byte[PduComposer.PDU_COMPOSER_BLOCK_SIZE];
                    cr = mResolver.openInputStream(part.getDataUri());
                    int len = 0;
                    while ((len = cr.read(buffer)) != -1) {
                        mMessage.write(buffer, 0, len);
                        mPosition += len;
                        dataLength += len;
                    }
                } catch (FileNotFoundException e) {
                    return PduComposer.PDU_COMPOSE_CONTENT_ERROR;
                } catch (IOException e) {
                    return PduComposer.PDU_COMPOSE_CONTENT_ERROR;
                } catch (RuntimeException e) {
                    return PduComposer.PDU_COMPOSE_CONTENT_ERROR;
                } finally {
                    if (cr != null) {
                        try {
                            cr.close();
                        } catch (IOException e) {
                        }
                    }
                }
            }

            if (dataLength != (attachment.getLength() - headerLength)) {
                throw new RuntimeException("BUG: Length sanity check failed");
            }

            mStack.pop();
            appendUintvarInteger(headerLength);
            appendUintvarInteger(dataLength);
            mStack.copy();
        }

        return PDU_COMPOSE_SUCCESS;
    }
    /**
     * Make NotifyResp.Ind.
     */
    private int makeNotifyRespEx() {
        if (mMessage == null) {
            mMessage = new ByteArrayOutputStream();
            mPosition = 0;
        }

        //    X-Mms-Message-Type
        appendOctet(MtkPduHeaders.MESSAGE_TYPE);
        appendOctet(MtkPduHeaders.MESSAGE_TYPE_NOTIFYRESP_IND);

        // X-Mms-Transaction-ID
        if (appendHeader(MtkPduHeaders.TRANSACTION_ID) != PDU_COMPOSE_SUCCESS) {
            return PDU_COMPOSE_CONTENT_ERROR;
        }

        // X-Mms-MMS-Version
        if (appendHeader(MtkPduHeaders.MMS_VERSION) != PDU_COMPOSE_SUCCESS) {
            return PDU_COMPOSE_CONTENT_ERROR;
        }

        //  X-Mms-Status
        if (appendHeader(MtkPduHeaders.STATUS) != PDU_COMPOSE_SUCCESS) {
            return PDU_COMPOSE_CONTENT_ERROR;
        }

        // X-Mms-Report-Allowed Optional (not support)
        /// M:Code analyze 007,add report allowed to support report @{
        appendHeader(MtkPduHeaders.REPORT_ALLOWED);
        /// @}
        return PDU_COMPOSE_SUCCESS;
    }


    /*[MTK MMS FW] OP01*/
    protected int appendHeader(int field) {
        //Log.d(LOG_TAG, "appendHeader: " + field);
        switch (field) {
            case MtkPduHeaders.EXPIRY:
                Log.d(LOG_TAG, "EXPIRY");
                long expiry = mPduHeader.getLongInteger(field);
                if (-1 == expiry) {
                    return PDU_COMPOSE_FIELD_NOT_SET;
                }
                appendOctet(field);

                mStack.newbuf();
                PositionMarker expiryStart = mStack.mark();

                /// M:Code analyze 005,change logic for BackupRestore @{
                /* Q0 migration*/
                /*
                if (mForBackup) {
                    Log.e(LOG_TAG, "absolute token");
                    append(MtkPduHeaders.VALUE_ABSOLUTE_TOKEN);
                } else {
                    Log.e(LOG_TAG, "relative token");
                    append(MtkPduHeaders.VALUE_RELATIVE_TOKEN);
                }*/
                append(MtkPduHeaders.VALUE_RELATIVE_TOKEN);
                /// @}
                appendLongInteger(expiry);

                int expiryLength = expiryStart.getLength();
                mStack.pop();
                appendValueLength(expiryLength);
                mStack.copy();
                break;
            /* Q0 migration*/
            /*
            case MtkPduHeaders.DATE_SENT:
                Log.d(LOG_TAG, "DATE_SENT");
                long date = mPduHeader.getLongInteger(field);
                Log.d(LOG_TAG, "date_sent = " + date);
                if (-1 == date) {
                    return PDU_COMPOSE_FIELD_NOT_SET;
                }

                appendOctet(field);
                appendDateValue(date);
                break;
*/
            default:
                return super.appendHeader(field);
        }
        return PDU_COMPOSE_SUCCESS;
    }

    /// M:Code analyze 002,Method for BackupRestore, for this application @{
    /* 1.RetrieveConf's DATE should use the value from pdu instead of
     *   setting system time directly
     * 2.RetrivevConf's read status should use column "READ" in table pdu,
     *   instead of "READ_STATUS" which is null in db*/
    /* Q0 migration*/
    /*
    private boolean mForBackup = false;
    public byte[] make(boolean forBackup) {
        mForBackup = forBackup;
        return make();
    }
    */
    /// @}
    /*[MTK MMS FW] OP01 end*/

    /* Q0 migration*/
    private int makeSendRetrievePdu(int type) {
        if (mMessage == null) {
            mMessage = new ByteArrayOutputStream();
            mPosition = 0;
        }

        // X-Mms-Message-Type
        appendOctet(MtkPduHeaders.MESSAGE_TYPE);
        appendOctet(type);

        // X-Mms-Transaction-ID
        appendOctet(MtkPduHeaders.TRANSACTION_ID);

        byte[] trid = mPduHeader.getTextString(MtkPduHeaders.TRANSACTION_ID);
        if (trid == null) {
            // Transaction-ID should be set(by Transaction) before make().
            Log.d(LOG_TAG, "Transaction ID is null");
        } else {
            // X-Mms-Transaction-ID
            appendTextString(trid);
        }

        //  X-Mms-MMS-Version
        if (appendHeader(MtkPduHeaders.MMS_VERSION) != PDU_COMPOSE_SUCCESS) {
            return PDU_COMPOSE_CONTENT_ERROR;
        }

        // Date Date-value Optional.
        appendHeader(MtkPduHeaders.DATE);

        // From
        if (appendHeader(MtkPduHeaders.FROM) != PDU_COMPOSE_SUCCESS) {
            return PDU_COMPOSE_CONTENT_ERROR;
        }

        boolean recipient = false;

        // To
        if (appendHeader(MtkPduHeaders.TO) != PDU_COMPOSE_CONTENT_ERROR) {
            recipient = true;
        }

        // Cc
        if (appendHeader(MtkPduHeaders.CC) != PDU_COMPOSE_CONTENT_ERROR) {
            recipient = true;
        }

        // Bcc
        if (appendHeader(MtkPduHeaders.BCC) != PDU_COMPOSE_CONTENT_ERROR) {
            recipient = true;
        }

        // Need at least one of "cc", "bcc" and "to".
        if (false == recipient) {
            return PDU_COMPOSE_CONTENT_ERROR;
        }

        // Subject Optional
        appendHeader(MtkPduHeaders.SUBJECT);

        // X-Mms-Message-Class Optional
        // Message-class-value = Class-identifier | Token-text
        appendHeader(MtkPduHeaders.MESSAGE_CLASS);

        // X-Mms-Expiry Optional
        appendHeader(MtkPduHeaders.EXPIRY);

        // X-Mms-Priority Optional
        appendHeader(MtkPduHeaders.PRIORITY);

        // X-Mms-Delivery-Report Optional
        appendHeader(MtkPduHeaders.DELIVERY_REPORT);

        // X-Mms-Read-Report Optional
        appendHeader(MtkPduHeaders.READ_REPORT);

        if (type == MtkPduHeaders.MESSAGE_TYPE_RETRIEVE_CONF) {
            // X-Mms-Retrieve-Status Optional
            appendHeader(MtkPduHeaders.RETRIEVE_STATUS);
            // X-Mms-Retrieve-Text Optional
            appendHeader(MtkPduHeaders.RETRIEVE_TEXT);
        }


        //    Content-Type
        appendOctet(MtkPduHeaders.CONTENT_TYPE);

        //  Message body
        return makeMessageBodyEx(type);
    }
}
