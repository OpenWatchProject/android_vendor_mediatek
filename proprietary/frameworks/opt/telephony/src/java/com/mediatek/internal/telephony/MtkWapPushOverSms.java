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

import static com.google.android.mms.pdu.PduHeaders.MESSAGE_TYPE_DELIVERY_IND;
import static com.google.android.mms.pdu.PduHeaders.MESSAGE_TYPE_NOTIFICATION_IND;
import static com.google.android.mms.pdu.PduHeaders.MESSAGE_TYPE_READ_ORIG_IND;
import android.app.Activity;
import android.app.AppOpsManager;
import android.app.BroadcastOptions;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.database.Cursor;
import android.database.DatabaseUtils;
import android.database.sqlite.SQLiteException;
import android.database.sqlite.SqliteWrapper;
import android.net.Uri;
import android.os.Bundle;
import android.os.IBinder;
import android.os.IDeviceIdleController;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.UserHandle;
import android.os.Build;
import android.provider.Telephony;
import android.provider.Telephony.Sms.Intents;
import android.telephony.Rlog;
import android.telephony.SmsManager;
import android.telephony.SubscriptionManager;
import android.util.Log;

import com.android.internal.telephony.uicc.IccUtils;
import com.android.internal.telephony.WapPushOverSms;
import com.android.internal.telephony.InboundSmsHandler;
import com.android.internal.telephony.WspTypeDecoder;
import com.android.internal.telephony.TelephonyComponentFactory;
import com.android.internal.telephony.BlockChecker;
import com.android.internal.telephony.IWapPushManager;
import com.android.internal.telephony.WapPushManagerParams;
import com.android.internal.telephony.SmsApplication;


import java.util.HashMap;

import com.google.android.mms.MmsException;
import com.google.android.mms.pdu.DeliveryInd;
import com.google.android.mms.pdu.GenericPdu;
import com.google.android.mms.pdu.NotificationInd;
import com.google.android.mms.pdu.PduHeaders;
import com.google.android.mms.pdu.PduParser;
import com.google.android.mms.pdu.PduPersister;
import com.google.android.mms.pdu.ReadOrigInd;

import mediatek.telephony.MtkTelephony;

/**
 * WAP push handler class.
 *
 * @hide
 */
public class MtkWapPushOverSms extends WapPushOverSms {
    private static final String TAG = "Mtk_WAP_PUSH";
    private static final boolean ENG = "eng".equals(Build.TYPE);

    public MtkWapPushOverSms(Context context) {
        super(context);
    }

    /**
     * Decodes the wap push pdu. The decoded result is wrapped inside the {@link DecodedResult}
     * object. The caller of this method should check {@link DecodedResult#statusCode} for the
     * decoding status. It  can have the following values.
     *
     * Activity.RESULT_OK - the wap push pdu is successfully decoded and should be further processed
     * Intents.RESULT_SMS_HANDLED - the wap push pdu should be ignored.
     * Intents.RESULT_SMS_GENERIC_ERROR - the pdu is invalid.
     */
    private DecodedResult decodeWapPdu(byte[] pdu, InboundSmsHandler handler) {
        DecodedResult result = new DecodedResult();
        if (ENG) Rlog.d(TAG, "Rx: " + IccUtils.bytesToHexString(pdu));

        try {
            int index = 0;
            int transactionId = pdu[index++] & 0xFF;
            int pduType = pdu[index++] & 0xFF;

            // Should we "abort" if no subId for now just no supplying extra param below
            int phoneId = handler.getPhone().getPhoneId();

            if ((pduType != WspTypeDecoder.PDU_TYPE_PUSH) &&
                    (pduType != WspTypeDecoder.PDU_TYPE_CONFIRMED_PUSH)) {
                index = mContext.getResources().getInteger(
                        com.android.internal.R.integer.config_valid_wappush_index);
                if (index != -1) {
                    transactionId = pdu[index++] & 0xff;
                    pduType = pdu[index++] & 0xff;
                    if (ENG)
                        Rlog.d(TAG, "index = " + index + " PDU Type = " + pduType +
                                " transactionID = " + transactionId);

                    // recheck wap push pduType
                    if ((pduType != WspTypeDecoder.PDU_TYPE_PUSH)
                            && (pduType != WspTypeDecoder.PDU_TYPE_CONFIRMED_PUSH)) {
                        if (ENG) Rlog.w(TAG, "Received non-PUSH WAP PDU. Type = " + pduType);
                        result.statusCode = Intents.RESULT_SMS_HANDLED;
                        return result;
                    }
                } else {
                    if (ENG) Rlog.w(TAG, "Received non-PUSH WAP PDU. Type = " + pduType);
                    result.statusCode = Intents.RESULT_SMS_HANDLED;
                    return result;
                }
            }

            TelephonyComponentFactory telephonyComponentFactory = TelephonyComponentFactory
                    .getInstance().inject(TelephonyComponentFactory.class.getName());
            MtkWspTypeDecoder pduDecoder = (MtkWspTypeDecoder)
                    telephonyComponentFactory.makeWspTypeDecoder(pdu);

            /**
             * Parse HeaderLen(unsigned integer).
             * From wap-230-wsp-20010705-a section 8.1.2
             * The maximum size of a uintvar is 32 bits.
             * So it will be encoded in no more than 5 octets.
             */
            if (pduDecoder.decodeUintvarInteger(index) == false) {
                if (ENG) Rlog.w(TAG, "Received PDU. Header Length error.");
                result.statusCode = Intents.RESULT_SMS_GENERIC_ERROR;
                return result;
            }
            int headerLength = (int) pduDecoder.getValue32();
            index += pduDecoder.getDecodedDataLength();

            int headerStartIndex = index;

            /**
             * Parse Content-Type.
             * From wap-230-wsp-20010705-a section 8.4.2.24
             *
             * Content-type-value = Constrained-media | Content-general-form
             * Content-general-form = Value-length Media-type
             * Media-type = (Well-known-media | Extension-Media) *(Parameter)
             * Value-length = Short-length | (Length-quote Length)
             * Short-length = <Any octet 0-30>   (octet <= WAP_PDU_SHORT_LENGTH_MAX)
             * Length-quote = <Octet 31>         (WAP_PDU_LENGTH_QUOTE)
             * Length = Uintvar-integer
             */
            if (pduDecoder.decodeContentType(index) == false) {
                if (ENG) Rlog.w(TAG, "Received PDU. Header Content-Type error.");
                result.statusCode = Intents.RESULT_SMS_GENERIC_ERROR;
                return result;
            }

            String mimeType = pduDecoder.getValueString();
            long binaryContentType = pduDecoder.getValue32();
            index += pduDecoder.getDecodedDataLength();

            byte[] header = new byte[headerLength];
            System.arraycopy(pdu, headerStartIndex, header, 0, header.length);
            // MTK-START
            pduDecoder.decodeHeaders(index, headerLength - index + headerStartIndex);
            // MTK-END

            byte[] intentData;

            if (mimeType != null && mimeType.equals(WspTypeDecoder.CONTENT_TYPE_B_PUSH_CO)) {
                intentData = pdu;
            } else {
                int dataIndex = headerStartIndex + headerLength;
                intentData = new byte[pdu.length - dataIndex];
                System.arraycopy(pdu, dataIndex, intentData, 0, intentData.length);
            }

            int[] subIds = SubscriptionManager.getSubId(phoneId);
            int subId = (subIds != null) && (subIds.length > 0) ? subIds[0]
                    : SmsManager.getDefaultSmsSubscriptionId();

            // Continue if PDU parsing fails: the default messaging app may successfully parse the
            // same PDU.
            GenericPdu parsedPdu = null;
            try {
                parsedPdu = new PduParser(intentData, shouldParseContentDisposition(subId)).parse();
            } catch (Exception e) {
                Rlog.e(TAG, "Unable to parse PDU: " + e.toString());
            }

            if (parsedPdu != null && parsedPdu.getMessageType() == MESSAGE_TYPE_NOTIFICATION_IND) {
                final NotificationInd nInd = (NotificationInd) parsedPdu;
                if (nInd.getFrom() != null
                        && BlockChecker.isBlocked(mContext, nInd.getFrom().getString())) {
                    result.statusCode = Intents.RESULT_SMS_HANDLED;
                    return result;
                }
            }

            /**
             * Seek for application ID field in WSP header.
             * If application ID is found, WapPushManager substitute the message
             * processing. Since WapPushManager is optional module, if WapPushManager
             * is not found, legacy message processing will be continued.
             */
            if (pduDecoder.seekXWapApplicationId(index, index + headerLength - 1)) {
                index = (int) pduDecoder.getValue32();
                pduDecoder.decodeXWapApplicationId(index);
                String wapAppId = pduDecoder.getValueString();
                if (wapAppId == null) {
                    wapAppId = Integer.toString((int) pduDecoder.getValue32());
                }
                result.wapAppId = wapAppId;
                String contentType = ((mimeType == null) ?
                        Long.toString(binaryContentType) : mimeType);
                result.contentType = contentType;
                if (ENG) Rlog.v(TAG, "appid found: " + wapAppId + ":" + contentType);
            }

            result.subId = subId;
            result.phoneId = phoneId;
            result.parsedPdu = parsedPdu;
            result.mimeType = mimeType;
            result.transactionId = transactionId;
            result.pduType = pduType;
            result.header = header;
            result.intentData = intentData;
            result.contentTypeParameters = pduDecoder.getContentParameters();
            result.statusCode = Activity.RESULT_OK;
            result.headerList = pduDecoder.getHeaders();
        } catch (ArrayIndexOutOfBoundsException aie) {
            // 0-byte WAP PDU or other unexpected WAP PDU contents can easily throw this;
            // log exception string without stack trace and return false.
            Rlog.e(TAG, "ignoring dispatchWapPdu() array index exception: " + aie);
            result.statusCode = Intents.RESULT_SMS_GENERIC_ERROR;
        }
        return result;
    }

    /**
     * Dispatches inbound messages that are in the WAP PDU format. See
     * wap-230-wsp-20010705-a section 8 for details on the WAP PDU format.
     *
     * @param pdu The WAP PDU, made up of one or more SMS PDUs
     * @return a result code from {@link android.provider.Telephony.Sms.Intents}, or
     *         {@link Activity#RESULT_OK} if the message has been broadcast
     *         to applications
     */
    @Override
    public int dispatchWapPdu(byte[] pdu, BroadcastReceiver receiver, InboundSmsHandler handler) {
        DecodedResult result = decodeWapPdu(pdu, handler);
        if (result.statusCode != Activity.RESULT_OK) {
            return result.statusCode;
        }

        if (SmsManager.getDefault().getAutoPersisting()) {
            // Store the wap push data in telephony
            writeInboxMessage(result.subId, result.parsedPdu);
        }

        /**
         * If the pdu has application ID, WapPushManager substitute the message
         * processing. Since WapPushManager is optional module, if WapPushManager
         * is not found, legacy message processing will be continued.
         */
        if (result.wapAppId != null) {
            try {
                boolean processFurther = true;
                IWapPushManager wapPushMan = mWapPushManager;

                if (wapPushMan == null) {
                    if (ENG) Rlog.d(TAG, "wap push manager not found!");
                } else {
                    if (ENG) Rlog.w(TAG, "addPowerSaveTempWhitelistAppForMms - start");
                    synchronized (this) {
                        mDeviceIdleController.addPowerSaveTempWhitelistAppForMms(
                                mWapPushManagerPackage, 0, "mms-mgr");
                    }
                    if (ENG) Rlog.d(TAG, "addPowerSaveTempWhitelistAppForMms - end");

                    Intent intent = new Intent();
                    intent.putExtra("transactionId", result.transactionId);
                    intent.putExtra("pduType", result.pduType);
                    intent.putExtra("header", result.header);
                    intent.putExtra("data", result.intentData);
                    intent.putExtra("contentTypeParameters", result.contentTypeParameters);
                    SubscriptionManager.putPhoneIdAndSubIdExtra(intent, result.phoneId);
                    // MTK-START
                    intent.putExtra("wspHeaders", result.headerList);

                    if (bundle != null) {
                        Rlog.d(TAG, "put addr info into intent 1");
                        intent.putExtra(MtkTelephony.WapPush.ADDR, bundle.getString(
                                MtkTelephony.WapPush.ADDR));
                        intent.putExtra(MtkTelephony.WapPush.SERVICE_ADDR, bundle.getString(
                                MtkTelephony.WapPush.SERVICE_ADDR));
                    }
                    // MTK-END

                    int procRet = wapPushMan.processMessage(
                        result.wapAppId, result.contentType, intent);
                    if (ENG) Rlog.v(TAG, "procRet:" + procRet);
                    if ((procRet & WapPushManagerParams.MESSAGE_HANDLED) > 0
                            && (procRet & WapPushManagerParams.FURTHER_PROCESSING) == 0) {
                        processFurther = false;
                    }
                }
                if (!processFurther) {
                    return Intents.RESULT_SMS_HANDLED;
                }
            } catch (RemoteException e) {
                if (ENG) Rlog.w(TAG, "remote func failed...");
            }
        }
        if (ENG) Rlog.v(TAG, "fall back to existing handler");

        if (result.mimeType == null) {
            if (ENG) Rlog.w(TAG, "Header Content-Type error.");
            return Intents.RESULT_SMS_GENERIC_ERROR;
        }

        Intent intent = new Intent(Intents.WAP_PUSH_DELIVER_ACTION);
        intent.setType(result.mimeType);
        intent.putExtra("transactionId", result.transactionId);
        intent.putExtra("pduType", result.pduType);
        intent.putExtra("header", result.header);
        intent.putExtra("data", result.intentData);
        intent.putExtra("contentTypeParameters", result.contentTypeParameters);
        SubscriptionManager.putPhoneIdAndSubIdExtra(intent, result.phoneId);
        // MTK-START
        intent.putExtra("wspHeaders", result.headerList);

        if (bundle != null) {
            Rlog.d(TAG, "put addr info into intent 2");
            intent.putExtra(MtkTelephony.WapPush.ADDR, bundle.getString(MtkTelephony.WapPush.ADDR));
            intent.putExtra(MtkTelephony.WapPush.SERVICE_ADDR, bundle.getString(
                    MtkTelephony.WapPush.SERVICE_ADDR));
        }
        // MTK-END

        // Direct the intent to only the default MMS app. If we can't find a default MMS app
        // then sent it to all broadcast receivers.
        ComponentName componentName = SmsApplication.getDefaultMmsApplication(mContext, true);
        Bundle options = null;
        if (componentName != null) {
            // Deliver MMS message only to this receiver
            intent.setComponent(componentName);
            if (ENG) Rlog.v(TAG, "Delivering MMS to: " + componentName.getPackageName() +
                    " " + componentName.getClassName());
            try {
                long duration = mDeviceIdleController.addPowerSaveTempWhitelistAppForMms(
                        componentName.getPackageName(), 0, "mms-app");
                BroadcastOptions bopts = BroadcastOptions.makeBasic();
                bopts.setTemporaryAppWhitelistDuration(duration);
                options = bopts.toBundle();
            } catch (RemoteException e) {
            }
        }

        handler.dispatchIntent(intent, getPermissionForType(result.mimeType),
                getAppOpsPermissionForIntent(result.mimeType), options, receiver,
                UserHandle.SYSTEM);
        return Activity.RESULT_OK;
    }

    /**
     * Place holder for decoded Wap pdu data.
     */
    private final class DecodedResult {
        String mimeType;
        String contentType;
        int transactionId;
        int pduType;
        int phoneId;
        int subId;
        byte[] header;
        String wapAppId;
        byte[] intentData;
        HashMap<String, String> contentTypeParameters;
        GenericPdu parsedPdu;
        int statusCode;
        // MTK-START
        HashMap<String, String> headerList;
        // MTK-END
    }

    // MTK-START
    /*
     * Add for wappush to get address and service address.
     * Address and service address will be stored in bundle
     * dispatchWapPdu(byte[] pdu, Bundle extra) will be called by framework
     */
    private Bundle bundle;
    public int dispatchWapPdu(byte[] pdu,  BroadcastReceiver receiver, InboundSmsHandler handler,
            Bundle extra) {
        if (ENG) Rlog.i(TAG, "dispathchWapPdu!");

        bundle = extra;
        return dispatchWapPdu(pdu, receiver, handler);
    }
    // MTK-END
}
