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
package com.mediatek.internal.telephony.cdma;

import android.telephony.Rlog;
import android.text.format.Time;

import com.android.internal.telephony.GsmAlphabet.TextEncodingDetails;
import com.android.internal.telephony.SmsConstants;
import com.android.internal.telephony.SmsHeader;
import com.android.internal.telephony.cdma.sms.BearerData;
import com.android.internal.telephony.cdma.sms.UserData;
import com.android.internal.telephony.uicc.IccUtils;
import com.android.internal.util.BitwiseInputStream;
import com.android.internal.util.BitwiseOutputStream;

import mediatek.telephony.MtkSmsCbCmasInfo;

/**
 * Sub class to enhance the AOSP class BearerData.
 */
public class MtkBearerData extends BearerData {

    private final static String LOG_TAG = "MtkBearerData";
    private final static byte SUBPARAM_USER_DATA                        = 0x01;
    private final static byte SUBPARAM_MESSAGE_CENTER_TIME_STAMP        = 0x03;
    private final static byte UNENCODABLE_7_BIT_CHAR = 0x20;

    /**
     * Create BearerData object from serialized representation.
     * (See 3GPP2 C.R1001-F, v1.0, section 4.5 for layout details)
     *
     * @param smsData byte array of raw encoded SMS bearer data.
     * @param serviceCategory the envelope service category (for CMAS alert handling)
     * @return an instance of BearerData.
     */
    public static BearerData decode(byte[] smsData, int serviceCategory) {
        BearerData bData = BearerData.decode(smsData, serviceCategory);
        if ((bData != null) &&
                (bData.userData != null) &&
                BearerData.isCmasAlertCategory(serviceCategory)) {
            try {
                BearerData temp = reGetUserData(smsData, serviceCategory);
                UserData backup = bData.userData;
                bData.userData = temp.userData;
                decodeCmasUserData(bData, serviceCategory);
                bData.userData = backup;
            } catch (BitwiseInputStream.AccessException ex) {
                Rlog.e(LOG_TAG, "BearerData decode failed: " + ex);
            } catch (BearerData.CodingException ex) {
                Rlog.e(LOG_TAG, "BearerData decode failed: " + ex);
                ex.printStackTrace();
            }
        }
        return bData;
    }

    /**
     * Create serialized representation for BearerData object.
     * (See 3GPP2 C.R1001-F, v1.0, section 4.5 for layout details)
     *
     * @param bData an instance of BearerData.
     *
     * @return byte array of raw encoded SMS bearer data.
     */
    public static byte[] encode(BearerData bData) {
        byte[] main = BearerData.encode(bData);
        if (main == null) {
            return null;
        }
        /* Support to save time in SIM */
        byte[] append = encodeMsgCenterTimeStamp(bData);
        byte[] result = new byte[main.length + append.length];
        System.arraycopy(main, 0, result, 0, main.length);
        System.arraycopy(append, 0, result, main.length, append.length);
        return result;
    }


    /**
     * Calculate the message text encoding length, fragmentation, and other details.
     *
     * @param msg message text
     * @param force7BitEncoding ignore (but still count) illegal characters if true
     * @param encodingType the encoding type of content of message(GSM 7-bit, Unicode or Automatic)
     * @return septet count, or -1 on failure
     */
    public static TextEncodingDetails calcTextEncodingDetails(CharSequence msg,
            boolean force7BitEncoding, int encodingType) {
        TextEncodingDetails ted;
        int septets = BearerData.countAsciiSeptets(msg, force7BitEncoding);
        if (encodingType == SmsConstants.ENCODING_16BIT) {
            Rlog.d(LOG_TAG, "16bit in cdma");
            septets = -1;
        }
        if (septets != -1 && septets <= SmsConstants.MAX_USER_DATA_SEPTETS) {
            ted = new TextEncodingDetails();
            ted.msgCount = 1;
            ted.codeUnitCount = septets;
            ted.codeUnitsRemaining = SmsConstants.MAX_USER_DATA_SEPTETS - septets;
            ted.codeUnitSize = SmsConstants.ENCODING_7BIT;
        } else {
            //Rlog.d(LOG_TAG, "(divide message use this)sync with send method, when send, use " +
            //        "sSmsInterfaces.calcTextEncodingDetails(msg, force7BitEncoding) to" +
            //        " get encode type and send with the encode type, search XXXXXX for send");
            /*ted = com.android.internal.telephony.gsm.SmsMessage.calculateLength(
                    msg, force7BitEncoding, encodingType);*/
            Rlog.d(LOG_TAG, "gsm can understand the control character, but cdma ignore it(<0x20)");
            ted = BearerData.calcTextEncodingDetails(msg, force7BitEncoding, true);
            if (ted.msgCount == 1 && ted.codeUnitSize == SmsConstants.ENCODING_7BIT) {
                // We don't support single-segment EMS, so calculate for 16-bit
                // TODO: Consider supporting single-segment EMS
                ted.codeUnitCount = msg.length();
                int octets = ted.codeUnitCount * 2;
                if (octets > SmsConstants.MAX_USER_DATA_BYTES) {
                    ted.msgCount = (octets + (SmsConstants.MAX_USER_DATA_BYTES_WITH_HEADER - 1)) /
                            SmsConstants.MAX_USER_DATA_BYTES_WITH_HEADER;
                    ted.codeUnitsRemaining = ((ted.msgCount *
                            SmsConstants.MAX_USER_DATA_BYTES_WITH_HEADER) - octets) / 2;
                } else {
                    ted.msgCount = 1;
                    ted.codeUnitsRemaining = (SmsConstants.MAX_USER_DATA_BYTES - octets) / 2;
                }
                ted.codeUnitSize = SmsConstants.ENCODING_16BIT;
            }
        }
        return ted;
    }

    /**
     * Convert the int to the BCD byte.
     * @param value The int need to be converted.
     * @return the BCD byte for value.
     */
    private static byte cdmaIntToBcdByte(int value) {
        byte ret = 0;
        value = value % 100;
        ret = (byte) (((value / 10) << 4) | (value % 10));
        return ret;
    }

    /**
     * Encode the message centre time into the BearerData.
     * @param bData The bearer data.
     * @param outStream the output stream for PDU.
     * @throws BitwiseOutputStream.AccessException
     */
    private static void encodeMsgCenterTimeStamp(BearerData bData, BitwiseOutputStream outStream)
            throws BitwiseOutputStream.AccessException {
        outStream.write(8, 6);
        int year = bData.msgCenterTimeStamp.year - 2000;
        if (year < 0) {
            year = bData.msgCenterTimeStamp.year - 1900;
        }
        outStream.write(8, cdmaIntToBcdByte(year));
        outStream.write(8, cdmaIntToBcdByte(bData.msgCenterTimeStamp.month + 1));
        outStream.write(8, cdmaIntToBcdByte(bData.msgCenterTimeStamp.monthDay));
        outStream.write(8, cdmaIntToBcdByte(bData.msgCenterTimeStamp.hour));
        outStream.write(8, cdmaIntToBcdByte(bData.msgCenterTimeStamp.minute));
        outStream.write(8, cdmaIntToBcdByte(bData.msgCenterTimeStamp.second));
    }

    private static byte[] encodeMsgCenterTimeStamp(BearerData bData) {
        try {
            BitwiseOutputStream outStream = new BitwiseOutputStream(200);
            // AOSP's MT message PDU can't be wrote to RUIM directly,
            // in order to encode the smsc time to the PDU, we check the time and
            // handle it at here.
            // We support copyTextMessageToIccCard and let application to specify the
            // timestamp, so we need to encode the message center time here.
            if (bData.msgCenterTimeStamp != null) {
                outStream.write(8, SUBPARAM_MESSAGE_CENTER_TIME_STAMP);
                encodeMsgCenterTimeStamp(bData, outStream);
                return outStream.toByteArray();
            }
        } catch (BitwiseOutputStream.AccessException ex) {
            Rlog.e(LOG_TAG, "BearerData encode failed: " + ex);
        }
        return new byte[0];
    }

    /**
     * CMAS message decoding.
     * (See TIA-1149-0-1, CMAS over CDMA)
     *
     * @param bData the BearerData instance
     * @param serviceCategory is the service category from the SMS envelope
     * @throws BitwiseInputStream.AccessException if stream access fail
     * @throws CodingException if decoding fail
     */
    public static void decodeCmasUserData(BearerData bData, int serviceCategory)
            throws BitwiseInputStream.AccessException, CodingException {
        BitwiseInputStream inStream = new BitwiseInputStream(bData.userData.payload);
        int protocolVersion = inStream.read(8);
        if (protocolVersion != 0) {
            throw new CodingException("unsupported CMAE_protocol_version " + protocolVersion);
        }
        long expiration = MtkSmsCbCmasInfo.CMAS_EXPIRATION_UNKNOWN;
        while (inStream.available() >= 16) {
            int recordType = inStream.read(8);
            int recordLen = inStream.read(8);
            switch (recordType) {
                case 2:
                    int identifier = inStream.read(8) << 8;
                    identifier |= inStream.read(8);
                    int alertHandling = inStream.read(8);
                    expiration = getCmasExpireTime(inStream.readByteArray(6 * 8));
                    int language = inStream.read(8);
                    break;

                default:
                    Rlog.w(LOG_TAG, "skipping CMAS record type " + recordType);
                    inStream.skip(recordLen * 8);
                    break;
            }
        }

        bData.cmasWarningInfo = new MtkSmsCbCmasInfo(
                bData.cmasWarningInfo.getMessageClass(),
                bData.cmasWarningInfo.getCategory(),
                bData.cmasWarningInfo.getResponseType(),
                bData.cmasWarningInfo.getSeverity(),
                bData.cmasWarningInfo.getUrgency(),
                bData.cmasWarningInfo.getCertainty(),
                expiration);
        Rlog.w(LOG_TAG, "MtkSmsCbCmasInfo " + bData.cmasWarningInfo);
    }

    private static BearerData reGetUserData(byte[] smsData, int serviceCategory)
        throws BitwiseInputStream.AccessException {
            BitwiseInputStream inStream = new BitwiseInputStream(smsData);
            BearerData bData = new BearerData();
            while (inStream.available() > 0) {
                int subparamId = inStream.read(8);
                switch (subparamId) {
                case SUBPARAM_USER_DATA:
                    decodeUserData(bData, inStream);
                    break;
                default:
                    decodeReserved(bData, inStream, subparamId);
                }
            }
            return bData;
    }

    private static boolean decodeUserData(BearerData bData, BitwiseInputStream inStream)
        throws BitwiseInputStream.AccessException {
        int paramBits = inStream.read(8) * 8;
        bData.userData = new UserData();
        bData.userData.msgEncoding = inStream.read(5);
        bData.userData.msgEncodingSet = true;
        bData.userData.msgType = 0;
        int consumedBits = 5;
        if ((bData.userData.msgEncoding == UserData.ENCODING_IS91_EXTENDED_PROTOCOL) ||
            (bData.userData.msgEncoding == UserData.ENCODING_GSM_DCS)) {
            bData.userData.msgType = inStream.read(8);
            consumedBits += 8;
        }
        bData.userData.numFields = inStream.read(8);
        consumedBits += 8;
        int dataBits = paramBits - consumedBits;
        bData.userData.payload = inStream.readByteArray(dataBits);
        return true;
    }

    private static boolean decodeReserved(
            BearerData bData, BitwiseInputStream inStream, int subparamId)
        throws BitwiseInputStream.AccessException {
        boolean decodeSuccess = false;
        int subparamLen = inStream.read(8); // SUBPARAM_LEN
        int paramBits = subparamLen * 8;
        if (paramBits <= inStream.available()) {
            decodeSuccess = true;
            inStream.skip(paramBits);
        }

        return decodeSuccess;
    }

    private static long getCmasExpireTime(byte[] data) {
        Time ts = new Time(Time.TIMEZONE_UTC);
        int year = data[0];
        if (year > 99 || year < 0) {
            return 0;
        }
        ts.year = year >= 96 ? year + 1900 : year + 2000;
        int month = data[1];
        if (month < 1 || month > 12) {
            return 0;
        }
        ts.month = month - 1;
        int day = data[2];
        if (day < 1 || day > 31) {
            return 0;
        }
        ts.monthDay = day;
        int hour = data[3];
        if (hour < 0 || hour > 23) {
            return 0;
        }
        ts.hour = hour;
        int minute = data[4];
        if (minute < 0 || minute > 59) {
            return 0;
        }
        ts.minute = minute;
        int second = data[5];
        if (second < 0 || second > 59) {
            return 0;
        }
        ts.second = second;
        return ts.toMillis(true);
    }
}
