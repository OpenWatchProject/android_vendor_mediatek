/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2019. All rights reserved.
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

package com.mediatek.internal.telephony.util;

import android.content.Context;

import com.android.internal.telephony.ImsSmsDispatcher;
import com.android.internal.telephony.cdma.CdmaSMSDispatcher;
import com.android.internal.telephony.GsmAlphabet.TextEncodingDetails;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.SmsMessageBase;
import com.android.internal.telephony.SmsHeader;

import android.app.ActivityManager;

import android.os.Build;
import android.os.Binder;

import android.telephony.Rlog;
import java.util.Iterator;
import java.util.List;

/**
 * Utilities used by {@link com.android.internal.telephony.SMSDispatcher}'s subclasses.
 *
 * These methods can not be moved to {@link CdmaSMSDispatcher} and {@link GsmSMSDispatcher} because
 * they also need to be called from {@link ImsSmsDispatcher} and the utilities will invoke the cdma
 * or gsm version of the method based on the format.
 */
public final class MtkSMSDispatcherUtil {
    static final String TAG = "MtkSMSDispatcherUtil";

    private static final boolean ENG = "eng".equals(Build.TYPE);

    // Prevent instantiation.
    private MtkSMSDispatcherUtil() {}

    /**
     * Trigger the proper implementation for getting submit pdu for text sms based on format.
     *
     * @param isCdma true if cdma format should be used.
     * @param scAddr is the service center address or null to use the current default SMSC
     * @param destAddr the address to send the message to
     * @param message the body of the message.
     * @param statusReportRequested whether or not a status report is requested.
     * @param smsHeader message header.
     * @return the submit pdu.
     */
    public static SmsMessageBase.SubmitPduBase getSubmitPdu(boolean isCdma, String scAddr,
            String destAddr, String message, boolean statusReportRequested, SmsHeader smsHeader) {
        if (isCdma) {
            return getSubmitPduCdma(scAddr, destAddr, message, statusReportRequested, smsHeader);
        } else {
            return getSubmitPduGsm(scAddr, destAddr, message, statusReportRequested);
        }
    }

    /**
     * Trigger the proper implementation for getting submit pdu for text sms based on format.
     *
     * @param isCdma true if cdma format should be used.
     * @param scAddr is the service center address or null to use the current default SMSC
     * @param destAddr the address to send the message to
     * @param message the body of the message.
     * @param statusReportRequested whether or not a status report is requested.
     * @param smsHeader message header.
     * @param priority Priority level of the message
     *  Refer specification See 3GPP2 C.S0015-B, v2.0, table 4.5.9-1
     *  ---------------------------------
     *  PRIORITY      | Level of Priority
     *  ---------------------------------
     *      '00'      |     Normal
     *      '01'      |     Interactive
     *      '10'      |     Urgent
     *      '11'      |     Emergency
     *  ----------------------------------
     *  Any Other values included Negative considered as Invalid Priority Indicator of the message.
     * @param validityPeriod Validity Period of the message in mins.
     *  Refer specification 3GPP TS 23.040 V6.8.1 section 9.2.3.12.1.
     *  Validity Period(Minimum) -> 5 mins
     *  Validity Period(Maximum) -> 635040 mins(i.e.63 weeks).
     *  Any Other values included Negative considered as Invalid Validity Period of the message.
     * @return the submit pdu.
     */
    public static SmsMessageBase.SubmitPduBase getSubmitPdu(boolean isCdma, String scAddr,
            String destAddr, String message, boolean statusReportRequested, SmsHeader smsHeader,
            int priority, int validityPeriod) {
        if (isCdma) {
            return getSubmitPduCdma(scAddr, destAddr, message, statusReportRequested, smsHeader,
                    priority);
        } else {
            return getSubmitPduGsm(scAddr, destAddr, message, statusReportRequested,
                    validityPeriod);
        }
    }

    /**
     * Gsm implementation for
     * {@link #getSubmitPdu(boolean, String, String, String, boolean)}
     *
     * @param scAddr is the service center address or null to use the current default SMSC
     * @param destAddr the address to send the message to
     * @param message the body of the message.
     * @param statusReportRequested whether or not a status report is requested.
     * @return the submit pdu.
     */
    public static SmsMessageBase.SubmitPduBase getSubmitPduGsm(String scAddr, String destAddr,
            String message, boolean statusReportRequested) {
        return com.mediatek.internal.telephony.gsm.MtkSmsMessage.getSubmitPdu(scAddr, destAddr,
                message, statusReportRequested);
    }

    /**
     * Gsm implementation for
     * {@link #getSubmitPdu(boolean, String, String, String, boolean, int)}
     *
     * @param scAddr is the service center address or null to use the current default SMSC
     * @param destAddr the address to send the message to
     * @param message the body of the message.
     * @param statusReportRequested whether or not a status report is requested.
     * @param validityPeriod Validity Period of the message in mins.
     *  Refer specification 3GPP TS 23.040 V6.8.1 section 9.2.3.12.1.
     *  Validity Period(Minimum) -> 5 mins
     *  Validity Period(Maximum) -> 635040 mins(i.e.63 weeks).
     *  Any Other values included Negative considered as Invalid Validity Period of the message.
     * @return the submit pdu.
     */
    public static SmsMessageBase.SubmitPduBase getSubmitPduGsm(String scAddr, String destAddr,
            String message, boolean statusReportRequested, int validityPeriod) {
        return com.mediatek.internal.telephony.gsm.MtkSmsMessage.getSubmitPdu(scAddr, destAddr,
                message, statusReportRequested, validityPeriod);
    }

    /**
     * Cdma implementation for
     * {@link #getSubmitPdu(boolean, String, String, String, boolean, SmsHeader)}
     *
     *  @param scAddr is the service center address or null to use the current default SMSC
     * @param destAddr the address to send the message to
     * @param message the body of the message.
     * @param statusReportRequested whether or not a status report is requested.
     * @param smsHeader message header.
     * @return the submit pdu.
     */
    public static SmsMessageBase.SubmitPduBase getSubmitPduCdma(String scAddr, String destAddr,
            String message, boolean statusReportRequested, SmsHeader smsHeader) {
        return com.android.internal.telephony.cdma.SmsMessage.getSubmitPdu(scAddr, destAddr,
                message, statusReportRequested, smsHeader);
    }

    /**
     * Cdma implementation for
     * {@link #getSubmitPdu(boolean, String, String, String, boolean, SmsHeader)}
     *
     *  @param scAddr is the service center address or null to use the current default SMSC
     * @param destAddr the address to send the message to
     * @param message the body of the message.
     * @param statusReportRequested whether or not a status report is requested.
     * @param smsHeader message header.
     * @param priority Priority level of the message
     *  Refer specification See 3GPP2 C.S0015-B, v2.0, table 4.5.9-1
     *  ---------------------------------
     *  PRIORITY      | Level of Priority
     *  ---------------------------------
     *      '00'      |     Normal
     *      '01'      |     Interactive
     *      '10'      |     Urgent
     *      '11'      |     Emergency
     *  ----------------------------------
     *  Any Other values included Negative considered as Invalid Priority Indicator of the message.
     * @return the submit pdu.
     */
    public static SmsMessageBase.SubmitPduBase getSubmitPduCdma(String scAddr, String destAddr,
            String message, boolean statusReportRequested, SmsHeader smsHeader, int priority) {
        return com.android.internal.telephony.cdma.SmsMessage.getSubmitPdu(scAddr, destAddr,
                message, statusReportRequested, smsHeader, priority);
    }

    /**
     * Trigger the proper implementation for getting submit pdu for data sms based on format.
     *
     * @param isCdma true if cdma format should be used.
     * @param destAddr the address to send the message to
     * @param scAddr is the service center address or null to use the current default SMSC
     * @param destPort the port to deliver the message to
     * @param message the body of the message to send
     * @param statusReportRequested whether or not a status report is requested.
     * @return the submit pdu.
     */
    public static SmsMessageBase.SubmitPduBase getSubmitPdu(boolean isCdma, String scAddr,
            String destAddr, int destPort, byte[] message, boolean statusReportRequested) {
        if (isCdma) {
            return getSubmitPduCdma(scAddr, destAddr, destPort, message, statusReportRequested);
        } else {
            return getSubmitPduGsm(scAddr, destAddr, destPort, message, statusReportRequested);
        }
    }

    /**
     * Cdma implementation of {@link #getSubmitPdu(boolean, String, String, int, byte[], boolean)}

     * @param destAddr the address to send the message to
     * @param scAddr is the service center address or null to use the current default SMSC
     * @param destPort the port to deliver the message to
     * @param message the body of the message to send
     * @param statusReportRequested whether or not a status report is requested.
     * @return the submit pdu.
     */
    public static SmsMessageBase.SubmitPduBase getSubmitPduCdma(String scAddr, String destAddr,
            int destPort, byte[] message, boolean statusReportRequested) {
        return com.android.internal.telephony.cdma.SmsMessage.getSubmitPdu(scAddr, destAddr,
                destPort, message, statusReportRequested);
    }

    /**
     * Gsm implementation of {@link #getSubmitPdu(boolean, String, String, int, byte[], boolean)}
     *
     * @param destAddr the address to send the message to
     * @param scAddr is the service center address or null to use the current default SMSC
     * @param destPort the port to deliver the message to
     * @param message the body of the message to send
     * @param statusReportRequested whether or not a status report is requested.
     * @return the submit pdu.
     */
    public static SmsMessageBase.SubmitPduBase getSubmitPduGsm(String scAddr, String destAddr,
            int destPort, byte[] message, boolean statusReportRequested) {
        return com.mediatek.internal.telephony.gsm.MtkSmsMessage.getSubmitPdu(scAddr, destAddr,
                destPort, message, statusReportRequested);
    }

    /**
     * Calculate the number of septets needed to encode the message. This function should only be
     * called for individual segments of multipart message.
     *
     * @param isCdma  true if cdma format should be used.
     * @param messageBody the message to encode
     * @param use7bitOnly ignore (but still count) illegal characters if true
     * @return TextEncodingDetails
     */
    public static TextEncodingDetails calculateLength(boolean isCdma, CharSequence messageBody,
            boolean use7bitOnly) {
        if (isCdma) {
            return calculateLengthCdma(messageBody, use7bitOnly);
        } else {
            return calculateLengthGsm(messageBody, use7bitOnly);
        }
    }

    /**
     * Gsm implementation for {@link #calculateLength(boolean, CharSequence, boolean)}
     *
     * @param messageBody the message to encode
     * @param use7bitOnly ignore (but still count) illegal characters if true
     * @return TextEncodingDetails
     */
    public static TextEncodingDetails calculateLengthGsm(CharSequence messageBody,
            boolean use7bitOnly) {
        return com.mediatek.internal.telephony.gsm.MtkSmsMessage.calculateLength(messageBody,
                use7bitOnly);

    }

    /**
     * Cdma implementation for {@link #calculateLength(boolean, CharSequence, boolean)}
     *
     * @param messageBody the message to encode
     * @param use7bitOnly ignore (but still count) illegal characters if true
     * @return TextEncodingDetails
     */
    public static TextEncodingDetails calculateLengthCdma(CharSequence messageBody,
            boolean use7bitOnly) {
        return com.android.internal.telephony.cdma.SmsMessage.calculateLength(messageBody,
                use7bitOnly, false);
    }

    /**
     * Get an SMS-SUBMIT PDU for a data message to a destination address with original port
     *
     * @param isCdma true if cdma format should be used.
     * @param scAddress Service Centre address. null == use default
     * @param destinationAddress the address of the destination for the message
     * @param destinationPort the port to deliver the message to at the
     *        destination
     * @param originalPort the port to deliver the message from
     * @param data the data for the message
     * @return a <code>SubmitPdu</code> containing the encoded SC
     *         address, if applicable, and the encoded message.
     *         Returns null on encode error.
     *
     */
    public static SmsMessageBase.SubmitPduBase getSubmitPdu(boolean isCdma, String scAddr,
            String destAddr, int destinationPort, int originalPort, byte[] data,
            boolean statusReportRequested) {
        if (isCdma) {
            // TODO: Wait for CDMA part is ready
            return null;
        } else {
            return getSubmitPduGsm(scAddr, destAddr, destinationPort, originalPort, data,
                    statusReportRequested);
        }
    }

    /**
     * Gsm implementation of {@link #getSubmitPdu(boolean, String, String, int, byte[], boolean)}
     *
     * @param destAddr the address to send the message to
     * @param scAddr is the service center address or null to use the current default SMSC
     * @param destPort the port to deliver the message to
     * @param message the body of the message to send
     * @param statusReportRequested whether or not a status report is requested.
     * @return the submit pdu.
     */
    public static SmsMessageBase.SubmitPduBase getSubmitPduGsm(String scAddr,
            String destAddr, int destinationPort, int originalPort, byte[] data,
            boolean statusReportRequested) {
        return com.mediatek.internal.telephony.gsm.MtkSmsMessage.getSubmitPdu(scAddr, destAddr,
                destinationPort, originalPort, data, statusReportRequested);
    }

    public static String getPackageNameViaProcessId(
            Context context, String[] packageNames) {
        String packageName = null;
        String rsp = null;

        if (packageNames.length == 1) {
            packageName = packageNames[0];
        } else if (packageNames.length > 1) {
            int callingPid = Binder.getCallingPid();
            Iterator index = null;

            ActivityManager am = (ActivityManager) context.getSystemService(
                    Context.ACTIVITY_SERVICE);
            List processList = am.getRunningAppProcesses();
            if (processList != null) {
                index = processList.iterator();
                while (index.hasNext()) {
                    ActivityManager.RunningAppProcessInfo processInfo =
                            (ActivityManager.RunningAppProcessInfo) (index.next());
                    if (callingPid == processInfo.pid) {
                        for (String pkgInProcess : processInfo.pkgList) {
                            for (String pkg : packageNames) {
                                if (pkg.equals(pkgInProcess)) {
                                    packageName = pkg;
                                    break;
                                }
                            }
                            if (packageName != null) {
                                break;
                            }
                        }
                        break;
                    }
                }
            }
        }

        if (packageName != null) {
            rsp = packageName;
        } else if (packageNames != null && packageNames.length > 0) {
            rsp = packageNames[0];
        }

        if (ENG) {
            Rlog.d(TAG, "getPackageNameViaProcessId: " + rsp);
        }
        return rsp;
    }

    /**
     * Get an SMS-SUBMIT PDU for a data message with data header
     * to a destination address
     *
     * @param scAddress Service Centre address. null == use default
     * @param destinationAddress the address of the destination for the message
     * @param data the data for the message
     * @param header the pdu header for the message
     * @return a <code>SubmitPdu</code> containing the encoded SC
     *         address, if applicable, and the encoded message.
     *         Returns null on encode error.
     */
    public static SmsMessageBase.SubmitPduBase getSubmitPdu(boolean isCdma, String scAddress,
            String destinationAddress, byte[] data, byte[] smsHeaderData,
            boolean statusReportRequested) {
        if (!isCdma) {
            return com.mediatek.internal.telephony.gsm.MtkSmsMessage.getSubmitPdu(scAddress,
                    destinationAddress, data, smsHeaderData, statusReportRequested);
        } else {
            // TODO: Wait for CDMA part is ready
            return null;
        }
    }

    /**
     * Get an SMS-SUBMIT PDU for a data message with data header
     * to a destination address
     *
     * @param scAddress Service Centre address. null == use default
     * @param destinationAddress the address of the destination for the message
     * @param data the data for the message
     * @param header the pdu header for the message
     * @return a <code>SubmitPdu</code> containing the encoded SC
     *         address, if applicable, and the encoded message.
     *         Returns null on encode error.
     */
    public static SmsMessageBase.SubmitPduBase getSubmitPduGsm(String scAddress,
            String destinationAddress, byte[] data, byte[] smsHeaderData,
            boolean statusReportRequested) {
        return com.mediatek.internal.telephony.gsm.MtkSmsMessage.getSubmitPdu(scAddress,
                destinationAddress, data, smsHeaderData, statusReportRequested);
    }

    /**
     * Get an SMS-SUBMIT PDU for a destination address and a message using the
     * specified encoding.
     *
     * @param scAddress Service Centre address.  Null means use default.
     * @param encoding Encoding defined by constants in android.telephony.SmsMessage.ENCODING_*
     * @param languageTable
     * @param languageShiftTable
     * @return a <code>SubmitPdu</code> containing the encoded SC
     *         address, if applicable, and the encoded message.
     *         Returns null on encode error.
     * @hide
     */
    public static SmsMessageBase.SubmitPduBase getSubmitPdu(boolean isCdma, String scAddress,
            String destinationAddress, String message,
            boolean statusReportRequested, byte[] header, int encoding,
            int languageTable, int languageShiftTable, int validityPeriod) {
        if (!isCdma) {
            return getSubmitPduGsm(scAddress, destinationAddress, message, statusReportRequested,
                    header, encoding, languageTable, languageShiftTable, validityPeriod);
        } else {
            // TODO: Wait for CDMA part is ready
            return null;
        }
    }

    public static SmsMessageBase.SubmitPduBase getSubmitPduGsm(String scAddress,
            String destinationAddress, String message,
            boolean statusReportRequested, byte[] header, int encoding,
            int languageTable, int languageShiftTable, int validityPeriod) {
        return com.mediatek.internal.telephony.gsm.MtkSmsMessage.getSubmitPdu(scAddress,
                destinationAddress, message, statusReportRequested, header,
                encoding, languageTable, languageShiftTable, validityPeriod);
    }
}
