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

 import com.google.android.mms.pdu.PduHeaders;
 import com.google.android.mms.InvalidHeaderValueException;

import java.util.ArrayList;
import android.util.Log;

public class MtkPduHeaders extends PduHeaders {
    private static final String TAG = "MtkPduHeaders";

    /*
     * M: TransactionService will skip downloading Mms if auto-download is off
     * @internal
     */
    public static final int STATE_SKIP_RETRYING             = 0x89;

    /*
     * M: add for saving sent time of received messages.
     * @internal
     */
    public static final int DATE_SENT                   = 0xC9;

     public MtkPduHeaders() {
          super();
     }

      /**
     * Set octet value to pdu header by header field.
     *
     * @param value the value
     * @param field the field
     * @throws InvalidHeaderValueException if the value is invalid.
     */
    protected void setOctet(int value, int field)
            throws InvalidHeaderValueException {
        /**
         * Check whether this field can be set for specific
         * header and check validity of the field.
         */
        switch (field) {
            case REPORT_ALLOWED:
            case ADAPTATION_ALLOWED:
            case DELIVERY_REPORT:
            case DRM_CONTENT:
            case DISTRIBUTION_INDICATOR:
            case QUOTAS:
            case READ_REPORT:
            case STORE:
            case STORED:
            case TOTALS:
            case SENDER_VISIBILITY:
                if ((VALUE_YES != value) && (VALUE_NO != value)) {
                    // Invalid value.
                    throw new InvalidHeaderValueException("Invalid Octet value!");
                }
                break;
            case READ_STATUS:
                if ((READ_STATUS_READ != value) &&
                        (READ_STATUS__DELETED_WITHOUT_BEING_READ != value)) {
                    // Invalid value.
                    throw new InvalidHeaderValueException("Invalid Octet value!");
                }
                break;
            case CANCEL_STATUS:
                if ((CANCEL_STATUS_REQUEST_SUCCESSFULLY_RECEIVED != value) &&
                        (CANCEL_STATUS_REQUEST_CORRUPTED != value)) {
                    // Invalid value.
                    throw new InvalidHeaderValueException("Invalid Octet value!");
                }
                break;
            case PRIORITY:
                if ((value < PRIORITY_LOW) || (value > PRIORITY_HIGH)) {
                    // Invalid value.
                    throw new InvalidHeaderValueException("Invalid Octet value!");
                }
                break;
            case STATUS:
                if ((value < STATUS_EXPIRED) || (value > STATE_SKIP_RETRYING)) {
                    // Invalid value.
                    throw new InvalidHeaderValueException("Invalid Octet value!");
                }
                break;
            case REPLY_CHARGING:
                if ((value < REPLY_CHARGING_REQUESTED)
                        || (value > REPLY_CHARGING_ACCEPTED_TEXT_ONLY)) {
                    // Invalid value.
                    throw new InvalidHeaderValueException("Invalid Octet value!");
                }
                break;
            case MM_STATE:
                if ((value < MM_STATE_DRAFT) || (value > MM_STATE_FORWARDED)) {
                    // Invalid value.
                    throw new InvalidHeaderValueException("Invalid Octet value!");
                }
                break;
            case RECOMMENDED_RETRIEVAL_MODE:
                if (RECOMMENDED_RETRIEVAL_MODE_MANUAL != value) {
                    // Invalid value.
                    throw new InvalidHeaderValueException("Invalid Octet value!");
                }
                break;
            case CONTENT_CLASS:
                if ((value < CONTENT_CLASS_TEXT)
                        || (value > CONTENT_CLASS_CONTENT_RICH)) {
                    // Invalid value.
                    throw new InvalidHeaderValueException("Invalid Octet value!");
                }
                break;
            case RETRIEVE_STATUS:
                // According to oma-ts-mms-enc-v1_3, section 7.3.50, we modify the invalid value.
                if ((value > RETRIEVE_STATUS_ERROR_TRANSIENT_NETWORK_PROBLEM) &&
                        (value < RETRIEVE_STATUS_ERROR_PERMANENT_FAILURE)) {
                    value = RETRIEVE_STATUS_ERROR_TRANSIENT_FAILURE;
                } else if ((value > RETRIEVE_STATUS_ERROR_PERMANENT_CONTENT_UNSUPPORTED) &&
                        (value <= RETRIEVE_STATUS_ERROR_END)) {
                    value = RETRIEVE_STATUS_ERROR_PERMANENT_FAILURE;
                } else if ((value < RETRIEVE_STATUS_OK) ||
                        ((value > RETRIEVE_STATUS_OK) &&
                                (value < RETRIEVE_STATUS_ERROR_TRANSIENT_FAILURE)) ||
                                (value > RETRIEVE_STATUS_ERROR_END)) {
                    value = RETRIEVE_STATUS_ERROR_PERMANENT_FAILURE;
                }
                break;
            case STORE_STATUS:
                // According to oma-ts-mms-enc-v1_3, section 7.3.58, we modify the invalid value.
                if ((value > STORE_STATUS_ERROR_TRANSIENT_NETWORK_PROBLEM) &&
                        (value < STORE_STATUS_ERROR_PERMANENT_FAILURE)) {
                    value = STORE_STATUS_ERROR_TRANSIENT_FAILURE;
                } else if ((value > STORE_STATUS_ERROR_PERMANENT_MMBOX_FULL) &&
                        (value <= STORE_STATUS_ERROR_END)) {
                    value = STORE_STATUS_ERROR_PERMANENT_FAILURE;
                } else if ((value < STORE_STATUS_SUCCESS) ||
                        ((value > STORE_STATUS_SUCCESS) &&
                                (value < STORE_STATUS_ERROR_TRANSIENT_FAILURE)) ||
                                (value > STORE_STATUS_ERROR_END)) {
                    value = STORE_STATUS_ERROR_PERMANENT_FAILURE;
                }
                break;
            case RESPONSE_STATUS:
                // According to oma-ts-mms-enc-v1_3, section 7.3.48, we modify the invalid value.
                if ((value > RESPONSE_STATUS_ERROR_TRANSIENT_PARTIAL_SUCCESS) &&
                        (value < RESPONSE_STATUS_ERROR_PERMANENT_FAILURE)) {
                    value = RESPONSE_STATUS_ERROR_TRANSIENT_FAILURE;
                } else if (((value > RESPONSE_STATUS_ERROR_PERMANENT_LACK_OF_PREPAID) &&
                        (value <= RESPONSE_STATUS_ERROR_PERMANENT_END)) ||
                        (value < RESPONSE_STATUS_OK) ||
                        ((value > RESPONSE_STATUS_ERROR_UNSUPPORTED_MESSAGE) &&
                                (value < RESPONSE_STATUS_ERROR_TRANSIENT_FAILURE)) ||
                                (value > RESPONSE_STATUS_ERROR_PERMANENT_END)) {
                    value = RESPONSE_STATUS_ERROR_PERMANENT_FAILURE;
                }
                break;
            case MMS_VERSION:
                if ((value < MMS_VERSION_1_0) || (value > MMS_VERSION_1_3)) {
                    value = CURRENT_MMS_VERSION; // Current version is the default value.
                }
                break;
            case MESSAGE_TYPE:
                if ((value < MESSAGE_TYPE_SEND_REQ) || (value > MESSAGE_TYPE_CANCEL_CONF)) {
                    // Invalid value.
                    throw new InvalidHeaderValueException("Invalid Octet value!");
                }
                break;
            default:
                // This header value should not be Octect.
                throw new RuntimeException("Invalid header field!");
        }
        mHeaderMap.put(field, value);
    }
   /**
     * Get TO, CC or BCC header value.
     *
     * @param field the field
     * @return the EncodeStringValue array of the pdu header
     *          with specified header field
     */
    public MtkEncodedStringValue[] getEncodedStringValuesEx(int field) {
        ArrayList<MtkEncodedStringValue> list =
                (ArrayList<MtkEncodedStringValue>) mHeaderMap.get(field);
        if (null == list) {
            return null;
        }
        MtkEncodedStringValue[] values = new MtkEncodedStringValue[list.size()];
        return list.toArray(values);
    }

      /**
     * Get EncodedStringValue value by header field.
     *
     * @param field the field
     * @return the EncodedStringValue value of the pdu header
     *          with specified header field
     */
    public MtkEncodedStringValue getEncodedStringValueEx(int field) {
        return (MtkEncodedStringValue) mHeaderMap.get(field);
    }

    /*[MTK MMS FW] OP01*/
    /**
     * Set LongInteger value to pdu header by header field.
     *
     * @param value the value
     * @param field the field
     */
    public void setLongInteger(long value, int field) {
        //Log.d(TAG, "setLongInteger(" + value + ", " + field + ")");
        switch (field) {
            case DATE_SENT:
                Log.d(TAG, "DATE_SENT");
                mHeaderMap.put(field, value);
                break;

            default:
                super.setLongInteger(value, field);
                break;
        }
    }
    /*[MTK MMS FW] OP01 end*/
}
