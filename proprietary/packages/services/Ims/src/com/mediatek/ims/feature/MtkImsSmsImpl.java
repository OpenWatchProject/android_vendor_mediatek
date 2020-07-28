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

package com.mediatek.ims.feature;

import android.os.Build;
import android.os.SystemProperties;

import android.annotation.SystemApi;
import android.content.Context;
import android.provider.Telephony.Sms.Intents;
import android.telephony.ims.aidl.IImsSmsListener;
import android.telephony.ims.stub.ImsSmsImplBase;
import android.telephony.Rlog;
import android.telephony.ServiceState;
import android.telephony.SmsManager;
import android.telephony.SmsMessage;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;

import com.mediatek.ims.OperatorUtils;
import com.mediatek.ims.ImsService;
import com.mediatek.ims.internal.IMtkImsService;
import com.android.internal.telephony.ims.MmTelInterfaceAdapter;
import com.android.internal.telephony.uicc.IccUtils;
import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.SmsMessageBase;

import java.util.HashMap;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * Base implementation for SMS over IMS.
 *
 * Any service wishing to provide SMS over IMS should extend this class and implement all methods
 * that the service supports.
 *
 * @hide
 */
@SystemApi
public class MtkImsSmsImpl extends ImsSmsImplBase {
    private static final String LOG_TAG = "MtkImsSmsImpl";
    private static final boolean ENG = "eng".equals(Build.TYPE);

    private ImsService mImsServiceImpl = null;
    private int mPhoneId = -1;
    private Context mContext;
    private boolean mIsReady = false;
    private static HashMap<Integer, MtkImsSmsImpl> sMtkImsSmsImpltances =
            new HashMap<Integer, MtkImsSmsImpl>();
    private ConcurrentHashMap<Integer, Integer> mToken = new ConcurrentHashMap<Integer, Integer>();
    private AtomicInteger mNextToken = new AtomicInteger();
    private ConcurrentHashMap<Integer, String> mInboundSmsFormat =
            new ConcurrentHashMap<Integer, String>();
    public static MtkImsSmsImpl getInstance(Context context, int phoneId,
            ImsService service) {
        if (sMtkImsSmsImpltances.containsKey(phoneId)) {
            MtkImsSmsImpl m = sMtkImsSmsImpltances.get(phoneId);
            return m;
        } else {
            sMtkImsSmsImpltances.put(phoneId, new MtkImsSmsImpl(context, phoneId, service));
            return sMtkImsSmsImpltances.get(phoneId);
        }
    }

    public MtkImsSmsImpl(Context context, int phoneId, ImsService service) {
        configure(context, phoneId, service);
    }

    public MtkImsSmsImpl(int phoneId) {
        configure(null, phoneId, null);
    }

    public void configure(Context context, int phoneId, ImsService service) {
        mImsServiceImpl = service;
        mPhoneId = phoneId;
        mContext = context;
        log("configure phone " + mPhoneId);
    }

    /**
     * This method will be triggered by the platform when the user attempts to send an SMS. This
     * method should be implemented by the IMS providers to provide implementation of sending an SMS
     * over IMS.
     *
     * @param token unique token generated by the platform that should be used when triggering
     *             callbacks for this specific message.
     * @param messageRef the message reference.
     * @param format the format of the message. Valid values are {@link SmsMessage#FORMAT_3GPP} and
     *               {@link SmsMessage#FORMAT_3GPP2}.
     * @param smsc the Short Message Service Center address.
     * @param isRetry whether it is a retry of an already attempted message or not.
     * @param pdu PDUs representing the contents of the message.
     */
    public void sendSms(int token, int messageRef, String format, String smsc, boolean isRetry,
            byte[] pdu) {
        if (!mIsReady) {
            throw new RuntimeException("onReady is not called yet");
        }

        mImsServiceImpl.sendSms(mPhoneId, token, messageRef, format, smsc, isRetry, pdu);
    }

    public void sendSmsRsp(int token, int messageRef,  @SendStatusResult int status,
            int reason) throws RuntimeException {
        log("sendSmsRsp toke=" + token + ",messageRef=" + messageRef + ",status=" + status
            + ",reason=" + reason);
        // Add the new one
        if (status == SEND_STATUS_OK) {
            mToken.put(messageRef, token);
        }

        onSendSmsResult(token, messageRef, status, reason);
    }

    public void newStatusReportInd(byte[] pdu, String format) {
        SmsMessageBase sms = null;
        if (SmsMessage.FORMAT_3GPP.equals(format)) {
            sms = (SmsMessageBase)
                    com.android.internal.telephony.gsm.SmsMessage.newFromCDS(pdu);
        } else if (SmsMessage.FORMAT_3GPP2.equals(format)) {
            sms = (SmsMessageBase)
                    com.android.internal.telephony.cdma.SmsMessage.createFromPdu(pdu);
        }
        boolean mayAckHere = true;
        if (sms != null) {
            int messageRef = sms.mMessageRef;
            // Get the token for the messageRef
            int token = mToken.getOrDefault(messageRef, -1);
            log("newStatusReportInd token=" + token + ", messageRef=" + messageRef
                    + ", pdu: " + IccUtils.bytesToHexString(pdu));
            if (token >= 0) {
                mayAckHere = false;
                mInboundSmsFormat.put(token, format);
                onSmsStatusReportReceived(token, messageRef, format, pdu);
            } else {
                loge("newStatusReportInd, token < 0, shouldn't be here");
            }
        } else {
            loge("newStatusReportInd, sms is null, shouldn't be here");
        }
        if (mayAckHere) {
            if (SmsMessage.FORMAT_3GPP.equals(format)) {
                mImsServiceImpl.acknowledgeLastIncomingGsmSms(mPhoneId, false, 1);
            } else if (SmsMessage.FORMAT_3GPP2.equals(format)) {
                mImsServiceImpl.acknowledgeLastIncomingCdmaSms
                        (mPhoneId, false, Intents.RESULT_SMS_GENERIC_ERROR);
            } else {
                loge("SMS format error.");
            }
        }
    }

    public void newImsSmsInd(byte[] pdu, String format) {
        int token = mNextToken.incrementAndGet();
        mInboundSmsFormat.put(token, format);
        onSmsReceived(token, format, pdu);
    }

    /**
     * This method will be triggered by the platform after
     * {@link #onSmsStatusReportReceived(int, int, String, byte[])} has been called to provide the
     * result to the IMS provider.
     *
     * @param token token provided in {@link #sendSms(int, int, String, String, boolean, byte[])}
     * @param result result of delivering the message. Valid values are:
     *  {@link #STATUS_REPORT_STATUS_OK},
     *  {@link #STATUS_REPORT_STATUS_ERROR}
     * @param messageRef the message reference
     */
    public void acknowledgeSmsReport(int token, int messageRef,
                                   @StatusReportResult int result) {
        if (!mIsReady) {
            throw new RuntimeException("onReady is not called yet");
        }

        log("acknowledgeSmsReport toke=" + token + ",messageRef=" + messageRef
            + ",result=" + result);
        // Remove the old one
        mToken.remove(token, messageRef);
        acknowledgeSms(token, messageRef, result);
    }

    @Override
    public void acknowledgeSms(int token, @DeliverStatusResult int messageRef, int result) {
        if (!mIsReady) {
            throw new RuntimeException("onReady is not called yet");
        }
        String format = mInboundSmsFormat.remove(token);
        boolean ok = (result == STATUS_REPORT_STATUS_OK);
        if (SmsMessage.FORMAT_3GPP.equals(format)) {
            int cause = resultToCauseForGsm(result);
            mImsServiceImpl.acknowledgeLastIncomingGsmSms(mPhoneId, ok, cause);
        } else if (SmsMessage.FORMAT_3GPP2.equals(format)) {
            int cause = resultToCauseForCdma(result);
            mImsServiceImpl.acknowledgeLastIncomingCdmaSms(
                    mPhoneId, ok, cause);
        } else {
            loge("SMS format error.");
        }
    }

    /**
     * Returns the SMS format. Default is {@link SmsMessage#FORMAT_3GPP} unless overridden by IMS
     * Provider.
     *
     * @return  the format of the message. Valid values are {@link SmsMessage#FORMAT_3GPP} and
     * {@link SmsMessage#FORMAT_3GPP2}.
     */
    public String getSmsFormat() {
        if (!mIsReady) {
            throw new RuntimeException("onReady is not called yet");
        }
        boolean is3GPP2Format = OperatorUtils.isMatched(OperatorUtils.OPID.OP236, mPhoneId);
        String smsFormat = SystemProperties.get("persist.vendor.radio.smsformat", "");
        log("mPhoneId:" + mPhoneId + ",is3GPP2Format:" + is3GPP2Format + ",format:" + smsFormat);
        if (smsFormat.equals("3gpp2")) {
            smsFormat = SmsMessage.FORMAT_3GPP2;
        } else if (smsFormat.equals("3gpp")) {
            smsFormat = SmsMessage.FORMAT_3GPP;
        } else {
            if (true == is3GPP2Format) {
                smsFormat = SmsMessage.FORMAT_3GPP2;
            } else {
                smsFormat = SmsMessage.FORMAT_3GPP;
            }
        }
        return smsFormat;
    }

    /**
     * Called when ImsSmsImpl has been initialized and communication with the framework is set up.
     * Any attempt by this class to access the framework before this method is called will return
     * with a {@link RuntimeException}.
     */
    public void onReady() {
        // Base Implementation - Should be overridden
        mIsReady = true;
        log("onReady");
    }

    private void log(String msg) {
        Rlog.d(LOG_TAG, "[" + mPhoneId + "] " + msg);
    }

    private void loge(String msg) {
        Rlog.e(LOG_TAG, "[" + mPhoneId + "] " + msg);
    }

    private static int resultToCauseForCdma(int rc) {
        switch (rc) {
        case STATUS_REPORT_STATUS_OK:
            // Cause code is ignored on success.
            return 0;
        case DELIVER_STATUS_ERROR_NO_MEMORY:
            return CommandsInterface.CDMA_SMS_FAIL_CAUSE_RESOURCE_SHORTAGE;
        case DELIVER_STATUS_ERROR_REQUEST_NOT_SUPPORTED:
            return CommandsInterface.CDMA_SMS_FAIL_CAUSE_INVALID_TELESERVICE_ID;
        case DELIVER_STATUS_ERROR_GENERIC:
        default:
            return CommandsInterface.CDMA_SMS_FAIL_CAUSE_OTHER_TERMINAL_PROBLEM;
        }
    }

    private static int resultToCauseForGsm(int rc) {
        switch (rc) {
            case STATUS_REPORT_STATUS_OK:
                // Cause code is ignored on success.
                return 0;
            case DELIVER_STATUS_ERROR_NO_MEMORY:
                return CommandsInterface.GSM_SMS_FAIL_CAUSE_MEMORY_CAPACITY_EXCEEDED;
            case DELIVER_STATUS_ERROR_GENERIC:
            default:
                return CommandsInterface.GSM_SMS_FAIL_CAUSE_UNSPECIFIED_ERROR;
        }
    }
}
