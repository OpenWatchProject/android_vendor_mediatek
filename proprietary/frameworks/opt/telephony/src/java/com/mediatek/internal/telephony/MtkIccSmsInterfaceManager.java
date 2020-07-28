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

import android.Manifest;
import android.app.ActivityThread;
import android.app.AppOpsManager;
import android.app.PendingIntent;
import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteException;
import android.net.Uri;
import android.os.AsyncResult;
import android.os.Binder;
import android.os.Handler;
import android.os.Message;
import android.os.UserManager;
import android.provider.Telephony;
import android.telephony.Rlog;
import android.telephony.SmsManager;
import android.telephony.SmsMessage;
import android.telephony.PhoneNumberUtils;
import android.util.Log;

import com.android.internal.telephony.gsm.SmsBroadcastConfigInfo;
import com.android.internal.telephony.cdma.CdmaSmsBroadcastConfigInfo;
import com.android.internal.telephony.uicc.IccConstants;
import com.android.internal.telephony.uicc.IccFileHandler;
import com.android.internal.telephony.uicc.IccUtils;
import com.android.internal.telephony.uicc.UiccController;
import com.android.internal.telephony.IccSmsInterfaceManager;
import com.android.internal.telephony.SmsRawData;
import com.android.internal.telephony.CommandException;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.SmsNumberUtils;
import com.android.internal.util.HexDump;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import static android.telephony.SmsManager.STATUS_ON_ICC_FREE;
import static android.telephony.SmsManager.STATUS_ON_ICC_READ;
import static android.telephony.SmsManager.STATUS_ON_ICC_UNREAD;

import android.telephony.TelephonyManager;

import android.os.Process;
import android.os.SystemProperties;
import android.content.BroadcastReceiver;
import android.content.IntentFilter;
import android.content.Intent;
import com.android.internal.telephony.GsmAlphabet.TextEncodingDetails;
import com.mediatek.internal.telephony.MtkIccSmsStorageStatus;
import com.mediatek.internal.telephony.MtkSmsHeader;
import com.mediatek.internal.telephony.MtkLteDataOnlyController;
import com.mediatek.internal.telephony.uicc.MtkSIMFileHandler;
import mediatek.telephony.MtkSimSmsInsertStatus;
import mediatek.telephony.MtkSmsParameters;
import mediatek.telephony.MtkSmsManager;
import static android.telephony.SmsManager.RESULT_ERROR_GENERIC_FAILURE;
import static mediatek.telephony.MtkSmsManager.RESULT_ERROR_SUCCESS;
import static mediatek.telephony.MtkSmsManager.RESULT_ERROR_SIM_MEM_FULL;
import static mediatek.telephony.MtkSmsManager.RESULT_ERROR_INVALID_ADDRESS;
import static mediatek.telephony.MtkSmsManager.EXTRA_PARAMS_VALIDITY_PERIOD;
import static android.telephony.SmsManager.STATUS_ON_ICC_SENT;
import static android.telephony.SmsManager.STATUS_ON_ICC_UNSENT;

// Mobile Manager Service
import android.os.Bundle;
import android.text.TextUtils;
import android.app.PendingIntent.CanceledException;

/**
 * MtkIccSmsInterfaceManager to provide an inter-process communication to
 * access Sms in Icc.
 */
public class MtkIccSmsInterfaceManager extends IccSmsInterfaceManager {
    static final String LOG_TAG = "MtkIccSmsInterfaceManager";
    static final boolean DBG = true;

    protected boolean mMtkSuccess;
    protected boolean mSmsWipedRsp;
    private List<SmsRawData> mSms;

    protected final Object mMtkLock = new Object();
    protected final Object mMtkLoadLock = new Object();

    private static final int EVENT_SIM_SMS_DELETE_DONE = 100;
    private static final int EVENT_GET_SMS_SIM_MEM_STATUS_DONE = 101;
    private static final int EVENT_INSERT_TEXT_MESSAGE_TO_ICC_DONE = 102;
    private static final int EVENT_GET_SMS_PARAMS = 103;
    private static final int EVENT_SET_SMS_PARAMS = 104;
    private static final int EVENT_LOAD_ONE_RECORD_DONE = 105;
    private static final int EVENT_GET_BROADCAST_ACTIVATION_DONE = 106;
    private static final int EVENT_REMOVE_BROADCAST_MSG_DONE = 107;
    private static final int EVENT_GET_BROADCAST_CONFIG_CHANNEL_DONE = 108;
    private static final int EVENT_SET_BROADCAST_CONFIG_LANGUAGE_DONE = 109;
    private static final int EVENT_GET_BROADCAST_CONFIG_LANGUAGE_DONE = 110;
    private static final int EVENT_SET_ETWS_CONFIG_DONE = 111;
    private static final int EVENT_GET_SMSC_ADDRESS_DONE = 112;
    private static final int EVENT_GET_SMSC_ADDRESS_BUNDLE_DONE = 113;
    private static final int EVENT_SET_SMSC_ADDRESS_DONE = 114;
    private static final int EVENT_MTK_LOAD_DONE = 115;
    private static final int EVENT_MTK_UPDATE_DONE =116;
    private static final int EVENT_SMS_WIPE_DONE =117;

    // MTK-START
    private MtkIccSmsStorageStatus mSimMemStatus;
    // Text message inserting
    private boolean mInsertMessageSuccess;
    private boolean mInserted;
    private final Object mSimInsertLock = new Object();
    private MtkSimSmsInsertStatus smsInsertRet =
            new MtkSimSmsInsertStatus(RESULT_ERROR_SUCCESS, "");
    private static int sConcatenatedRef = 456;
    private static final String INDEXT_SPLITOR = ",";
    // Raw message inserting
    private MtkSimSmsInsertStatus smsInsertRet2 =
            new MtkSimSmsInsertStatus(RESULT_ERROR_SUCCESS, "");
    // EFsmsp read/write
    private MtkSmsParameters mSmsParams = null;
    private boolean mSmsParamsSuccess = false;
    // Single sms record loading
    private SmsRawData mSmsRawData = null;
    // Record the CB config
    private SmsBroadcastConfigInfo[] mSmsCBConfig = null;
    private String mSmsCbChannelConfig = "";
    private String mSmsCbLanguageConfig = "";
    // SMSC address
    private String mSmscAddress = "";
    private Bundle mSmscAddressBundle = new Bundle();

    protected Handler mMtkHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            AsyncResult ar;

            switch (msg.what) {
                case EVENT_MTK_UPDATE_DONE:
                    ar = (AsyncResult) msg.obj;
                    synchronized (mMtkLock) {
                        mMtkSuccess = (ar.exception == null);
                        // MTK-START
                        if (mMtkSuccess == true) {
                            try {
                                int index = ((int[]) ar.result)[0];
                                smsInsertRet2.indexInIcc += (index + INDEXT_SPLITOR);
                                log("[insertRaw save one pdu in index " + index);
                            } catch (ClassCastException e) {
                                e.printStackTrace();
                            } catch (Exception ex) {
                                ex.printStackTrace();
                            }
                        } else {
                            log("[insertRaw fail to insert raw into ICC");
                            smsInsertRet2.indexInIcc += ("-1" + INDEXT_SPLITOR);
                        }
                        // MTK-END
                        mMtkLock.notifyAll();
                    }

                    // MTK-START
                    if (ar.exception != null) {
                        CommandException e = (CommandException) ar.exception;
                        if (DBG) log("Cannot update SMS " + e.getCommandError());

                        if (e.getCommandError() == CommandException.Error.SIM_FULL) {
                            ((MtkSmsDispatchersController)mDispatchersController).handleIccFull();
                        }
                    }
                    // MTK-END
                    break;
                case EVENT_SMS_WIPE_DONE:
                    ar = (AsyncResult) msg.obj;
                    synchronized (mMtkLock) {
                        // MTK-START
                        if (ar.exception == null) {
                            try {
                                int index = ((int[]) ar.result)[0];
                                smsInsertRet2.indexInIcc += (index + INDEXT_SPLITOR);
                                log("[insertRaw save one pdu in index " + index);
                            } catch (ClassCastException e) {
                                e.printStackTrace();
                            } catch (Exception ex) {
                                ex.printStackTrace();
                            }
                        } else {
                            log("[insertRaw fail to insert raw into ICC");
                            smsInsertRet2.indexInIcc += ("-1" + INDEXT_SPLITOR);
                        }
                        mSmsWipedRsp = true;
                        // MTK-END
                        mMtkLock.notifyAll();
                    }

                    // MTK-START
                    if (ar.exception != null) {
                        CommandException e = (CommandException) ar.exception;
                        if (DBG) log("Cannot update SMS " + e.getCommandError());

                        if (e.getCommandError() == CommandException.Error.SIM_FULL) {
                            ((MtkSmsDispatchersController)mDispatchersController).handleIccFull();
                        }
                    }
                    // MTK-END
                    break;
                case EVENT_MTK_LOAD_DONE:
                    ar = (AsyncResult)msg.obj;
                    // MTK-START
                    synchronized (mMtkLoadLock) {
                    // MTK-END
                        if (ar.exception == null) {
                            mSms = buildValidRawData((ArrayList<byte[]>) ar.result);
                            //Mark SMS as read after importing it from card.
                            markMessagesAsRead((ArrayList<byte[]>) ar.result);
                        } else {
                            if (Rlog.isLoggable("SMS", Log.DEBUG)) {
                                log("Cannot load Sms records");
                            }
                            mSms = null;
                        }
                        // MTK-START
                        mMtkLoadLock.notifyAll();
                        // MTK-END
                    }
                    break;
                case EVENT_GET_SMS_SIM_MEM_STATUS_DONE:
                    ar = (AsyncResult) msg.obj;

                    synchronized (mMtkLock) {
                        if (ar.exception == null) {
                            mMtkSuccess = true;

                            if (mSimMemStatus == null) {
                                mSimMemStatus = new MtkIccSmsStorageStatus();
                            }

                            MtkIccSmsStorageStatus tmpStatus = (MtkIccSmsStorageStatus) ar.result;

                            mSimMemStatus.mUsed = tmpStatus.mUsed;
                            mSimMemStatus.mTotal = tmpStatus.mTotal;
                        } else {
                            if (DBG)
                                log("Cannot Get Sms SIM Memory Status from SIM");
                        }
                        mMtkLock.notifyAll();
                    }
                    break;
                case EVENT_INSERT_TEXT_MESSAGE_TO_ICC_DONE:
                    ar = (AsyncResult) msg.obj;
                    synchronized (mSimInsertLock) {
                        mInsertMessageSuccess = (ar.exception == null);
                        if (mInsertMessageSuccess == true) {
                            try {
                                int index = ((int[]) ar.result)[0];
                                smsInsertRet.indexInIcc += (index + INDEXT_SPLITOR);
                                log("insertText save one pdu in index " + index);
                            } catch (ClassCastException e) {
                                e.printStackTrace();
                            } catch (Exception ex) {
                                ex.printStackTrace();
                            }
                        } else {
                            log("insertText fail to insert sms into ICC");
                            smsInsertRet.indexInIcc += ("-1" + INDEXT_SPLITOR);
                        }
                        mInserted = true;
                        mSimInsertLock.notifyAll();
                    }
                    break;
                case EVENT_GET_SMS_PARAMS:
                    ar = (AsyncResult) msg.obj;
                    synchronized (mMtkLock) {
                        if (ar.exception == null) {
                            try {
                                mSmsParams = (MtkSmsParameters) ar.result;
                            } catch (ClassCastException e) {
                                log("[EFsmsp fail to get sms params ClassCastException");
                                e.printStackTrace();
                            } catch (Exception ex) {
                                log("[EFsmsp fail to get sms params Exception");
                                ex.printStackTrace();
                            }
                        } else {
                            log("[EFsmsp fail to get sms params");
                            mSmsParams = null;
                        }

                        mMtkLock.notifyAll();
                    }
                    break;
                case EVENT_SET_SMS_PARAMS:
                    ar = (AsyncResult) msg.obj;
                    synchronized (mMtkLock) {
                        if (ar.exception == null) {
                            mSmsParamsSuccess = true;
                        } else {
                            log("[EFsmsp fail to set sms params");
                            mSmsParamsSuccess = false;
                        }

                        mMtkLock.notifyAll();
                    }
                    break;
                case EVENT_LOAD_ONE_RECORD_DONE:
                    ar = (AsyncResult) msg.obj;
                    synchronized (mMtkLock) {
                        if (ar.exception == null) {
                            try {
                                // mSmsRawData = (SmsRawData)ar.result;
                                byte[] rawData = (byte[]) ar.result;
                                if (rawData[0] == STATUS_ON_ICC_FREE) {
                                    log("sms raw data status is FREE");
                                    mSmsRawData = null;
                                } else {
                                    mSmsRawData = new SmsRawData(rawData);
                                }
                            } catch (ClassCastException e) {
                                log("fail to get sms raw data ClassCastException");
                                e.printStackTrace();
                                mSmsRawData = null;
                            }
                        } else {
                            log("fail to get sms raw data rild");
                            mSmsRawData = null;
                        }
                        mMtkLock.notifyAll();
                    }
                    break;
                case EVENT_SET_ETWS_CONFIG_DONE:
                case EVENT_SET_BROADCAST_CONFIG_LANGUAGE_DONE:
                case EVENT_SET_SMSC_ADDRESS_DONE:
                    ar = (AsyncResult) msg.obj;
                    synchronized (mMtkLock) {
                        mMtkSuccess = (ar.exception == null);
                        mMtkLock.notifyAll();
                    }
                    break;
                case EVENT_GET_BROADCAST_ACTIVATION_DONE:
                    ar = (AsyncResult) msg.obj;
                    synchronized (mMtkLock) {
                        if (ar.exception == null) {
                            int[] activation = (int[])(ar.result);
                            mMtkSuccess = ((activation[0] == 1)? true : false);
                        }

                        log("queryCbActivation: " + mMtkSuccess);
                        mMtkLock.notifyAll();
                    }
                    break;
                case EVENT_REMOVE_BROADCAST_MSG_DONE:
                    ar = (AsyncResult) msg.obj;
                    synchronized (mMtkLock) {
                        mMtkSuccess = (ar.exception == null);
                        mMtkLock.notifyAll();
                    }
                    break;
                case EVENT_GET_BROADCAST_CONFIG_CHANNEL_DONE:
                    ar = (AsyncResult) msg.obj;
                    synchronized (mMtkLock) {
                        if (ar.exception == null) {
                            SmsBroadcastConfigInfo cbConfig = null;

                            ArrayList<SmsBroadcastConfigInfo> mList =
                                    (ArrayList<SmsBroadcastConfigInfo>) ar.result;

                            for (int i = 0; i < mList.size(); i++) {
                                // Convert SmsBroadcastConfigInfo to String
                                cbConfig = mList.get(i);
                                if (cbConfig.getFromServiceId() == cbConfig.getToServiceId()) {
                                    mSmsCbChannelConfig = mSmsCbChannelConfig +
                                            cbConfig.getFromServiceId();
                                } else {
                                    mSmsCbChannelConfig = mSmsCbChannelConfig +
                                            cbConfig.getFromServiceId() + "-" +
                                            cbConfig.getToServiceId();
                                }

                                if ((i + 1) != mList.size()) {
                                    mSmsCbChannelConfig = mSmsCbChannelConfig + ",";
                                }
                            }
                            log("Channel configuration " + mSmsCbChannelConfig);
                        } else {
                            log("Cannot Get CB configs");
                        }
                        mMtkLock.notifyAll();
                    }
                    break;
                case EVENT_GET_BROADCAST_CONFIG_LANGUAGE_DONE:
                    ar = (AsyncResult) msg.obj;
                    synchronized (mMtkLock) {
                        if (ar.exception == null) {
                            mSmsCbLanguageConfig = (String) ar.result;
                            mSmsCbLanguageConfig =
                                    ((mSmsCbLanguageConfig != null)? mSmsCbLanguageConfig : "");
                            log("Language configuration " + mSmsCbLanguageConfig);
                        } else {
                            log("Cannot Get CB configs");
                        }
                        mMtkLock.notifyAll();
                    }
                    break;
                case EVENT_GET_SMSC_ADDRESS_DONE:
                    ar = (AsyncResult) msg.obj;
                    synchronized (mMtkLock) {
                        if (ar.exception == null) {
                            mSmscAddress = (String) ar.result;
                        } else {
                            log("Cannot Get SMSC address");
                        }
                        mMtkLock.notifyAll();
                    }
                    break;
                case EVENT_GET_SMSC_ADDRESS_BUNDLE_DONE:
                    ar = (AsyncResult) msg.obj;
                    synchronized (mMtkLock) {
                        mSmscAddressBundle.clear();
                        if (ar.exception == null) {
                            mSmscAddressBundle.putByte(MtkSmsManager.GET_SC_ADDRESS_KEY_RESULT,
                                    MtkSmsManager.ERROR_CODE_NO_ERROR);
                            mSmscAddressBundle.putCharSequence(
                                    MtkSmsManager.GET_SC_ADDRESS_KEY_ADDRESS, (String) ar.result);
                        } else {
                            log("Cannot Get SMSC address");
                            byte error = MtkSmsManager.ERROR_CODE_GENERIC_ERROR;
                            if (ar.exception instanceof CommandException) {
                                CommandException ce = (CommandException) ar.exception;
                                if (ce.getCommandError() ==
                                        CommandException.Error.REQUEST_NOT_SUPPORTED) {
                                    error = MtkSmsManager.ERROR_CODE_NO_SUPPORT_SC_ADDR;
                                }
                            }
                            log("Fail to get sc address, error = " + error);
                            mSmscAddressBundle.putByte(
                                    MtkSmsManager.GET_SC_ADDRESS_KEY_RESULT, error);
                            mSmscAddressBundle.putCharSequence(
                                    MtkSmsManager.GET_SC_ADDRESS_KEY_ADDRESS, "");
                        }
                        mMtkLock.notifyAll();
                    }
                    break;
            }
        }
    };

    protected MtkIccSmsInterfaceManager(Phone phone) {
        super(phone);

        IntentFilter filter = new IntentFilter();
        filter.addAction("com.mediatek.dm.LAWMO_WIPE");
        mContext.registerReceiver(mSmsWipeReceiver, filter);
    }

    /**
     * Send a text based SMS.
     *
     * @param destAddr the address to send the message to
     * @param scAddr is the service center address or null to use
     *  the current default SMSC
     * @param text the body of the message to send
     * @param sentIntent if not NULL this <code>PendingIntent</code> is
     *  broadcast when the message is successfully sent, or failed.
     *  The result code will be <code>Activity.RESULT_OK<code> for success,
     *  or one of these errors:<br>
     *  <code>RESULT_ERROR_GENERIC_FAILURE</code><br>
     *  <code>RESULT_ERROR_RADIO_OFF</code><br>
     *  <code>RESULT_ERROR_NULL_PDU</code><br>
     *  For <code>RESULT_ERROR_GENERIC_FAILURE</code> the sentIntent may include
     *  the extra "errorCode" containing a radio technology specific value,
     *  generally only useful for troubleshooting.<br>
     *  The per-application based SMS control checks sentIntent. If sentIntent
     *  is NULL the caller will be checked against all unknown applications,
     *  which cause smaller number of SMS to be sent in checking period.
     * @param deliveryIntent if not NULL this <code>PendingIntent</code> is
     *  broadcast when the message is delivered to the recipient.  The
     *  raw pdu of the status report is in the extended data ("pdu").
     */

    protected void sendTextInternal(String callingPackage, String destAddr, String scAddr,
            String text, PendingIntent sentIntent, PendingIntent deliveryIntent,
            boolean persistMessageForNonDefaultSmsApp, int priority, boolean expectMore,
            int validityPeriod, boolean isForVvm) {
        log("sendTextMessage");

        if (!isValidParameters(destAddr, text, sentIntent)) {
            return;
        }

        if (!checkTddDataOnlyPermission(sentIntent)) {
            log("TDD data only and w/o permission!");
            return;
        }

        Context context = ActivityThread.currentApplication().getApplicationContext();

        super.sendTextInternal(callingPackage, destAddr, scAddr, text, sentIntent,
                deliveryIntent, persistMessageForNonDefaultSmsApp, priority, expectMore,
                validityPeriod, isForVvm);
    }

    /**
     * Send a data based SMS to a specific application port.
     *
     * @param destAddr the address to send the message to
     * @param scAddr is the service center address or null to use
     *  the current default SMSC
     * @param destPort the port to deliver the message to
     * @param data the body of the message to send
     * @param sentIntent if not NULL this <code>PendingIntent</code> is
     *  broadcast when the message is successfully sent, or failed.
     *  The result code will be <code>Activity.RESULT_OK<code> for success,
     *  or one of these errors:<br>
     *  <code>RESULT_ERROR_GENERIC_FAILURE</code><br>
     *  <code>RESULT_ERROR_RADIO_OFF</code><br>
     *  <code>RESULT_ERROR_NULL_PDU</code><br>
     *  For <code>RESULT_ERROR_GENERIC_FAILURE</code> the sentIntent may include
     *  the extra "errorCode" containing a radio technology specific value,
     *  generally only useful for troubleshooting.<br>
     *  The per-application based SMS control checks sentIntent. If sentIntent
     *  is NULL the caller will be checked against all unknown applications,
     *  which cause smaller number of SMS to be sent in checking period.
     * @param deliveryIntent if not NULL this <code>PendingIntent</code> is
     *  broadcast when the message is delivered to the recipient.  The
     *  raw pdu of the status report is in the extended data ("pdu").
     */

    protected void sendDataInternal(String callingPackage, String destAddr, String scAddr,
            int destPort, byte[] data, PendingIntent sentIntent, PendingIntent deliveryIntent,
            boolean isForVvm) {
        log("sendDataMessage");

        if (!isValidParameters(destAddr, "send_data", sentIntent)) {
            return;
        }

        if (!checkTddDataOnlyPermission(sentIntent)) {
            log("TDD data only and w/o permission!");
            return;
        }

        Context context = ActivityThread.currentApplication().getApplicationContext();

        super.sendDataInternal(callingPackage, destAddr, scAddr, destPort, data, sentIntent,
                deliveryIntent, isForVvm);
    }

    public void sendStoredText(String callingPkg, Uri messageUri, String scAddress,
            PendingIntent sentIntent, PendingIntent deliveryIntent) {
        Context context = ActivityThread.currentApplication().getApplicationContext();

        if (!checkTddDataOnlyPermission(sentIntent)) {
            log("TDD data only and w/o permission!");
            return;
        }

        super.sendStoredText(callingPkg, messageUri, scAddress, sentIntent, deliveryIntent);
    }

    /**
     * Update the specified message on the Icc.
     *
     * @param index record index of message to update
     * @param status new message status (STATUS_ON_ICC_READ,
     *                  STATUS_ON_ICC_UNREAD, STATUS_ON_ICC_SENT,
     *                  STATUS_ON_ICC_UNSENT, STATUS_ON_ICC_FREE)
     * @param pdu the raw PDU to store
     * @return success or not
     *
     */

    public void sendStoredMultipartText(String callingPkg, Uri messageUri, String scAddress,
            List<PendingIntent> sentIntents, List<PendingIntent> deliveryIntents) {
        Context context = ActivityThread.currentApplication().getApplicationContext();

        if (!checkTddDataOnlyPermission(sentIntents)) {
            log("TDD data only and w/o permission!");
            return;
        }

        super.sendStoredMultipartText(callingPkg, messageUri, scAddress, sentIntents,
                deliveryIntents);
    }

    /**
     * Retrieves all messages currently stored on Icc.
     *
     * @return list of SmsRawData of all sms on Icc
     */

    public List<SmsRawData> getAllMessagesFromIccEf(String callingPackage) {
        // MTK-START
        if (DBG) log("getAllMessagesFromEF " + callingPackage);
        // MTK-END

        mContext.enforceCallingOrSelfPermission(
                Manifest.permission.RECEIVE_SMS,
                "Reading messages from Icc");
        if (mAppOps.noteOp(AppOpsManager.OP_READ_ICC_SMS, Binder.getCallingUid(),
                callingPackage) != AppOpsManager.MODE_ALLOWED) {
            return new ArrayList<SmsRawData>();
        }
        // MTK-START
        synchronized (mMtkLoadLock) {
        // MTK-END

            IccFileHandler fh = mPhone.getIccFileHandler();
            if (fh == null) {
                Rlog.e(LOG_TAG, "Cannot load Sms records. No icc card?");
                mSms = null;
                return mSms;
            }

            Message response = mMtkHandler.obtainMessage(EVENT_MTK_LOAD_DONE);
            fh.loadEFLinearFixedAll(IccConstants.EF_SMS, response);

            try {
                // MTK-START
                mMtkLoadLock.wait();
                // MTK-END
            } catch (InterruptedException e) {
                log("interrupted while trying to load from the Icc");
            }
        }
        return mSms;
    }

    /**
     * create SmsRawData lists from all sms record byte[]
     * Use null to indicate "free" record
     *
     * @param messages List of message records from EF_SMS.
     * @return SmsRawData list of all in-used records
     */
    protected ArrayList<SmsRawData> buildValidRawData(ArrayList<byte[]> messages) {
        int count = messages.size();
        ArrayList<SmsRawData> ret;

        ret = new ArrayList<SmsRawData>(count);

        // MTK-START/
        int validSmsCount = 0;
        // MTK-END/
        for (int i = 0; i < count; i++) {
            byte[] ba = messages.get(i);
            if (ba[0] == STATUS_ON_ICC_FREE) {
                ret.add(null);
            } else {
                // MTK-START
                validSmsCount++;
                // MTK-END
                ret.add(new SmsRawData(messages.get(i)));
            }
        }
        // MTK-START
        log("validSmsCount = " + validSmsCount);
        // MTK-END

        return ret;
    }

    /**
     * Generates an EF_SMS record from status and raw PDU.
     *
     * @param status Message status.  See TS 51.011 10.5.3.
     * @param pdu Raw message PDU.
     * @return byte array for the record.
     */
    protected byte[] makeSmsRecordData(int status, byte[] pdu) {
        byte[] data;
        if (PhoneConstants.PHONE_TYPE_GSM == mPhone.getPhoneType()) {
            data = new byte[SmsManager.SMS_RECORD_LENGTH];
        } else {
            data = new byte[SmsManager.CDMA_SMS_RECORD_LENGTH];
        }

        // Status bits for this record.  See TS 51.011 10.5.3
        data[0] = (byte)(status & 7);
        // MTK-START
        log("ISIM-makeSmsRecordData: pdu size = " + pdu.length);
        if (pdu.length == SmsManager.SMS_RECORD_LENGTH) {
            log("ISIM-makeSmsRecordData: sim pdu");
            try {
                System.arraycopy(pdu, 1, data, 1, pdu.length - 1);
            } catch (ArrayIndexOutOfBoundsException e) {
                log("ISIM-makeSmsRecordData: out of bounds, sim pdu");
            }
        } else {
            log("ISIM-makeSmsRecordData: normal pdu");
            try {
                System.arraycopy(pdu, 0, data, 1, pdu.length);
            } catch (ArrayIndexOutOfBoundsException e) {
                log("ISIM-makeSmsRecordData: out of bounds, normal pdu");
            }
        }
        // MTK-END

        // Pad out with 0xFF's.
        for (int j = pdu.length+1; j < data.length; j++) {
            data[j] = -1;
        }

        return data;
    }

    protected void log(String msg) {
        // MTK-START, Print log to radio in order to easy debug
        Rlog.d(LOG_TAG, msg);
        // MTK-END
    }

    private void loge(String msg) {
        Rlog.e(LOG_TAG, msg);
    }

    // MTK-START
    /**
     * Send a data based SMS to a specific application port with original port.
     *
     * @param destAddr the address to send the message to
     * @param scAddr is the service center address or null to use
     *  the current default SMSC
     * @param destPort the port to deliver the message to
     * @param originalPort the port to deliver the message from
     * @param data the body of the message to send
     * @param sentIntent if not NULL this <code>PendingIntent</code> is
     *  broadcast when the message is successfully sent, or failed.
     *  The result code will be <code>Activity.RESULT_OK<code> for success,
     *  or one of these errors:<br>
     *  <code>RESULT_ERROR_GENERIC_FAILURE</code><br>
     *  <code>RESULT_ERROR_RADIO_OFF</code><br>
     *  <code>RESULT_ERROR_NULL_PDU</code><br>
     *  For <code>RESULT_ERROR_GENERIC_FAILURE</code> the sentIntent may include
     *  the extra "errorCode" containing a radio technology specific value,
     *  generally only useful for troubleshooting.<br>
     *  The per-application based SMS control checks sentIntent. If sentIntent
     *  is NULL the caller will be checked against all unknown applications,
     *  which cause smaller number of SMS to be sent in checking period.
     * @param deliveryIntent if not NULL this <code>PendingIntent</code> is
     *  broadcast when the message is delivered to the recipient.  The
     *  raw pdu of the status report is in the extended data ("pdu").
     * @param checkPermission check permission or not
     */
    public void sendDataWithOriginalPort(String callingPackage, String destAddr, String scAddr,
            int destPort, int originalPort, byte[] data, PendingIntent sentIntent,
            PendingIntent deliveryIntent, boolean checkPermission) {
        Rlog.d(LOG_TAG, "Enter IccSmsInterfaceManager.sendDataWithOriginalPort");
        if (checkPermission) {
            mPhone.getContext().enforceCallingPermission(
                    Manifest.permission.SEND_SMS,
                    "Sending SMS message");

            if (Rlog.isLoggable("SMS", Log.VERBOSE)) {
                log("sendData: data='" + HexDump.toHexString(data) + "' sentIntent=" + sentIntent
                        + " deliveryIntent=" + deliveryIntent);
            }
            if (mAppOps.noteOp(AppOpsManager.OP_SEND_SMS, Binder.getCallingUid(),
                    callingPackage) != AppOpsManager.MODE_ALLOWED) {
                return;
            }
        }
        ((MtkSmsDispatchersController)mDispatchersController).sendData(callingPackage,
                destAddr, scAddr, destPort, originalPort, data, sentIntent, deliveryIntent);
    }

    /**
     * Send a multi-part text based SMS.
     *
     * @param destAddr the address to send the message to
     * @param scAddr is the service center address or null to use
     *   the current default SMSC
     * @param parts an <code>ArrayList</code> of strings that, in order,
     *   comprise the original message
     * @param sentIntents if not null, an <code>ArrayList</code> of
     *   <code>PendingIntent</code>s (one for each message part) that is
     *   broadcast when the corresponding message part has been sent.
     *   The result code will be <code>Activity.RESULT_OK<code> for success,
     *   or one of these errors:
     *   <code>RESULT_ERROR_GENERIC_FAILURE</code>
     *   <code>RESULT_ERROR_RADIO_OFF</code>
     *   <code>RESULT_ERROR_NULL_PDU</code>.
     *  The per-application based SMS control checks sentIntent. If sentIntent
     *  is NULL the caller will be checked against all unknown applications,
     *  which cause smaller number of SMS to be sent in checking period.
     * @param deliveryIntents if not null, an <code>ArrayList</code> of
     *   <code>PendingIntent</code>s (one for each message part) that is
     *   broadcast when the corresponding message part has been delivered
     *   to the recipient.  The raw pdu of the status report is in the
     *   extended data ("pdu").
     */

    public void sendMultipartText(String callingPackage, String destAddr, String scAddr,
            List<String> parts, List<PendingIntent> sentIntents,
            List<PendingIntent> deliveryIntents, boolean persistMessageForNonDefaultSmsApp) {
        mPhone.getContext().enforceCallingPermission(
                Manifest.permission.SEND_SMS,
                "Sending SMS message");

        // Support empty content

        log("sendMultipartTextMessage");

        if (!isValidParameters(destAddr, parts, sentIntents)) {
            return;
        }

        if (!checkTddDataOnlyPermission(sentIntents)) {
            log("TDD data only and w/o permission!");
            return;
        }

        Context context = ActivityThread.currentApplication().getApplicationContext();

        super.sendMultipartText(callingPackage, destAddr, scAddr, parts, sentIntents,
                deliveryIntents, persistMessageForNonDefaultSmsApp);
    }

    /**
     * Send a multi-part data based SMS.
     *
     * @param destinationAddress the address to send the message to
     * @param scAddress is the service center address or null to use
     *   the current default SMSC
     * @param data an <code>ArrayList</code> of strings that, in order,
     *   comprise the original message
     * @param destPort the port to deliver the message to
     * @param data an array of data messages in order,
     *   comprise the original message
     * @param sentIntents if not null, an <code>ArrayList</code> of
     *   <code>PendingIntent</code>s (one for each message part) that is
     *   broadcast when the corresponding message part has been sent.
     *   The result code will be <code>Activity.RESULT_OK<code> for success,
     *   or one of these errors:
     *   <code>RESULT_ERROR_GENERIC_FAILURE</code>
     *   <code>RESULT_ERROR_RADIO_OFF</code>
     *   <code>RESULT_ERROR_NULL_PDU</code>.
     * @param deliveryIntents if not null, an <code>ArrayList</code> of
     *   <code>PendingIntent</code>s (one for each message part) that is
     *   broadcast when the corresponding message part has been delivered
     *   to the recipient.  The raw pdu of the status report is in the
     *   extended data ("pdu").
     */
    public void sendMultipartData(
            String callingPackage,
            String destAddr,
            String scAddr,
            int destPort,
            List<SmsRawData> data,
            List<PendingIntent> sentIntents,
            List<PendingIntent> deliveryIntents) {

        mPhone.getContext().enforceCallingPermission(
                Manifest.permission.SEND_SMS,
                "Sending SMS message");

        if (Rlog.isLoggable("SMS", Log.VERBOSE)) {
            for (SmsRawData rData : data) {
                log("sendMultipartData:data='" + HexDump.toHexString(rData.getBytes()));
            }
        }
        if (mAppOps.noteOp(AppOpsManager.OP_SEND_SMS, Binder.getCallingUid(),
                callingPackage) != AppOpsManager.MODE_ALLOWED) {
            return;
        }
        ((MtkSmsDispatchersController)mDispatchersController).sendMultipartData(callingPackage,
                destAddr, scAddr, destPort, (ArrayList<SmsRawData>) data,
                (ArrayList<PendingIntent>) sentIntents, (ArrayList<PendingIntent>) deliveryIntents);
    }

    /**
     * Set the memory storage status of the SMS This function is used for FTA
     * test only
     *
     * @param status false for storage full, true for storage available
     */
    public void setSmsMemoryStatus(boolean status) {
        log("setSmsMemoryStatus: set storage status -> " + status);
        ((MtkSmsDispatchersController)mDispatchersController).setSmsMemoryStatus(status);
    }

    /**
     * Judge if SMS subsystem is ready or not
     *
     * @return true for success
     */
    public boolean isSmsReady() {
        boolean isReady = ((MtkSmsDispatchersController)mDispatchersController).isSmsReady();

        log("isSmsReady: " + isReady);
        return isReady;
    }

    /**
     * Send an SMS with specified encoding type.
     *
     * @param smsc the SMSC to send the message through, or NULL for the
     *  default SMSC
     * @param text the body of the message to send
     * @param encodingType the encoding type of content of message(GSM 7-bit, Unicode or Automatic)
     * @param sentIntent if not NULL this <code>PendingIntent</code> is
     *  broadcast when the message is sucessfully sent, or failed.
     *  The result code will be <code>Activity.RESULT_OK<code> for success,
     *  or one of these errors:<br>
     *  <code>RESULT_ERROR_GENERIC_FAILURE</code><br>
     *  <code>RESULT_ERROR_RADIO_OFF</code><br>
     *  <code>RESULT_ERROR_NULL_PDU</code><br>
     *  For <code>RESULT_ERROR_GENERIC_FAILURE</code> the sentIntent may include
     *  the extra "errorCode" containing a radio technology specific value,
     *  generally only useful for troubleshooting.<br>
     *  The per-application based SMS control checks sentIntent. If sentIntent
     *  is NULL the caller will be checked against all unknown applications,
     *  which cause smaller number of SMS to be sent in checking period.
     * @param deliveryIntent if not NULL this <code>PendingIntent</code> is
     *  broadcast when the message is delivered to the recipient.  The
     *  raw pdu of the status report is in the extended data ("pdu").
     * @param persistMessage whether to save the sent message into SMS DB for a
     *   non-default SMS app.
     */
    public void sendTextWithEncodingType(String callingPackage, String destAddr, String scAddr,
            String text, int encodingType, PendingIntent sentIntent, PendingIntent deliveryIntent,
            boolean persistMessageForNonDefaultSmsApp) {
        sendTextWithOptions(callingPackage, destAddr, scAddr, text, sentIntent, deliveryIntent,
                persistMessageForNonDefaultSmsApp, SMS_MESSAGE_PRIORITY_NOT_SPECIFIED,
                false /*isExpectMore*/, SMS_MESSAGE_PERIOD_NOT_SPECIFIED, encodingType);
    }

    /**
     * Send a multi-part text based SMS with specified encoding type.
     *
     * @param destinationAddress the address to send the message to
     * @param scAddr is the service center address or null to use
     *   the current default SMSC
     * @param parts an <code>ArrayList</code> of strings that, in order,
     *   comprise the original message
     * @param encodingType the encoding type of content of message(GSM 7-bit, Unicode or Automatic)
     * @param sentIntents if not null, an <code>ArrayList</code> of
     *   <code>PendingIntent</code>s (one for each message part) that is
     *   broadcast when the corresponding message part has been sent.
     *   The result code will be <code>Activity.RESULT_OK<code> for success,
     *   or one of these errors:
     *   <code>RESULT_ERROR_GENERIC_FAILURE</code>
     *   <code>RESULT_ERROR_RADIO_OFF</code>
     *   <code>RESULT_ERROR_NULL_PDU</code>.
     * @param deliveryIntents if not null, an <code>ArrayList</code> of
     *   <code>PendingIntent</code>s (one for each message part) that is
     *   broadcast when the corresponding message part has been delivered
     *   to the recipient.  The raw pdu of the status report is in the
     *   extended data ("pdu").
     * @param persistMessageForNonDefaultSmsApp whether to save the sent message into SMS DB for a
     *   non-default SMS app.
     */
    public void sendMultipartTextWithEncodingType(String callingPackage, String destAddr,
            String scAddr, List<String> parts, int encodingType, List<PendingIntent> sentIntents,
            List<PendingIntent> deliveryIntents, boolean persistMessageForNonDefaultSmsApp) {
        sendMultipartTextWithOptions(callingPackage, destAddr, scAddr, parts, sentIntents,
                deliveryIntents, persistMessageForNonDefaultSmsApp,
                SMS_MESSAGE_PRIORITY_NOT_SPECIFIED, false /*isExpectMore*/,
                SMS_MESSAGE_PERIOD_NOT_SPECIFIED, encodingType);
    }

    /**
     * Send an SMS with specified encoding type.
     *
     * @param destAddr the address to send the message to
     * @param scAddr the SMSC to send the message through, or NULL for the
     *  default SMSC
     * @param text the body of the message to send
     * @param extraParams extra parameters, such as validity period, encoding type
     * @param sentIntent if not NULL this <code>PendingIntent</code> is
     *  broadcast when the message is sucessfully sent, or failed.
     * @param deliveryIntent if not NULL this <code>PendingIntent</code> is
     *  broadcast when the message is delivered to the recipient.  The
     *  raw pdu of the status report is in the extended data ("pdu").
     * @param persistMessage whether to save the sent message into SMS DB for a
     *   non-default SMS app.
     */
    public void sendTextWithExtraParams(String callingPackage, String destAddr, String scAddr,
            String text, Bundle extraParams, PendingIntent sentIntent,
            PendingIntent deliveryIntent, boolean persistMessageForNonDefaultSmsApp) {

        mPhone.getContext().enforceCallingPermission(
                Manifest.permission.SEND_SMS,
                "Sending SMS message");

        if (mAppOps.noteOp(AppOpsManager.OP_SEND_SMS, Binder.getCallingUid(),
                callingPackage) != AppOpsManager.MODE_ALLOWED) {
            return;
        }

        int validityPeriod = extraParams.getInt(MtkSmsManager.EXTRA_PARAMS_VALIDITY_PERIOD, -1);
        sendTextInternal(callingPackage, destAddr, scAddr, text, sentIntent, deliveryIntent,
                persistMessageForNonDefaultSmsApp, SMS_MESSAGE_PRIORITY_NOT_SPECIFIED, false,
                validityPeriod, false);
    }

    /**
     * Send a multi-part text based SMS with specified encoding type.
     *
     * @param destAddr the address to send the message to
     * @param scAddr is the service center address or null to use
     *   the current default SMSC
     * @param parts an <code>ArrayList</code> of strings that, in order,
     *   comprise the original message
     * @param extraParams extra parameters, such as validity period, encoding type
     * @param sentIntents if not null, an <code>ArrayList</code> of
     *   <code>PendingIntent</code>s (one for each message part) that is
     *   broadcast when the corresponding message part has been sent.
     * @param deliveryIntents if not null, an <code>ArrayList</code> of
     *   <code>PendingIntent</code>s (one for each message part) that is
     *   broadcast when the corresponding message part has been delivered
     *   to the recipient.  The raw pdu of the status report is in the
     *   extended data ("pdu").
     * @param persistMessageForNonDefaultSmsApp whether to save the sent message into SMS DB for a
     *   non-default SMS app.
     */
    public void sendMultipartTextWithExtraParams(String callingPackage, String destAddr,
            String scAddr, List<String> parts, Bundle extraParams, List<PendingIntent> sentIntents,
            List<PendingIntent> deliveryIntents, boolean persistMessageForNonDefaultSmsApp) {

        mPhone.getContext().enforceCallingPermission(
                Manifest.permission.SEND_SMS,
                "Sending SMS message");

        if (mAppOps.noteOp(AppOpsManager.OP_SEND_SMS, Binder.getCallingUid(),
                callingPackage) != AppOpsManager.MODE_ALLOWED) {
            return;
        }

        if (!checkTddDataOnlyPermission(sentIntents)) {
            log("TDD data only and w/o permission!");
            return;
        }

        destAddr = filterDestAddress(destAddr);
        int validityPeriod = extraParams.getInt(MtkSmsManager.EXTRA_PARAMS_VALIDITY_PERIOD, -1);

        if (parts.size() > 1 && parts.size() < 10 && !SmsMessage.hasEmsSupport()) {
            for (int i = 0; i < parts.size(); i++) {
                // If EMS is not supported, we have to break down EMS into single segment SMS
                // and add page info " x/y".
                String singlePart = parts.get(i);
                if (SmsMessage.shouldAppendPageNumberAsPrefix()) {
                    singlePart = String.valueOf(i + 1) + '/' + parts.size() + ' ' + singlePart;
                } else {
                    singlePart = singlePart.concat(' ' + String.valueOf(i + 1) + '/' +
                            parts.size());
                }

                PendingIntent singleSentIntent = null;
                if (sentIntents != null && sentIntents.size() > i) {
                    singleSentIntent = sentIntents.get(i);
                }

                PendingIntent singleDeliveryIntent = null;
                if (deliveryIntents != null && deliveryIntents.size() > i) {
                    singleDeliveryIntent = deliveryIntents.get(i);
                }


                sendTextWithOptions(callingPackage, destAddr, scAddr, singlePart, singleSentIntent,
                        singleDeliveryIntent, persistMessageForNonDefaultSmsApp,
                        SMS_MESSAGE_PRIORITY_NOT_SPECIFIED, false, validityPeriod);
            }
            return;
        }

        sendMultipartTextWithOptions(callingPackage, destAddr, scAddr, parts, sentIntents,
                deliveryIntents, persistMessageForNonDefaultSmsApp,
                SMS_MESSAGE_PRIORITY_NOT_SPECIFIED, false, validityPeriod);
    }


    /**
     * Receive the WIPE intent
     */
    private BroadcastReceiver mSmsWipeReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            log("Receive intent");
            if (intent.getAction().equals("com.mediatek.dm.LAWMO_WIPE")) {
                log("Receive wipe intent");
                Thread t = new Thread() {
                    public void run() {
                        synchronized (mMtkLock) {
                            log("Delete message on sub " + mPhone.getSubId());
                            mSmsWipedRsp = false;
                            Message response = mMtkHandler.obtainMessage(EVENT_SMS_WIPE_DONE);
                            mPhone.mCi.deleteSmsOnSim(-1, response);
                            try {
                                while (!mSmsWipedRsp) {
                                    mMtkLock.wait();
                                }
                            } catch (InterruptedException e) {
                                log("insertRaw interrupted while trying to update by index");
                            }
                        }
                    }
                };
                t.start();
            }
        }
    };

    /**
     * Retrieves message currently stored on ICC by index.
     *
     * @return SmsRawData of sms on ICC
     */
    public SmsRawData getMessageFromIccEf(String callingPackage, int index) {
        log("getMessageFromIccEf");

        mPhone.getContext().enforceCallingPermission(
                "android.permission.RECEIVE_SMS",
                "Reading messages from SIM");
        if (mAppOps.noteOp(AppOpsManager.OP_READ_ICC_SMS, Binder.getCallingUid(),
                callingPackage) != AppOpsManager.MODE_ALLOWED) {
            return null;
        }

        mSmsRawData = null;
        synchronized (mMtkLock) {
            /* icc file handler will be null while plug-out sim card */
            IccFileHandler fh = mPhone.getIccFileHandler();
            if (fh != null) {
                Message response = mMtkHandler.obtainMessage(EVENT_LOAD_ONE_RECORD_DONE);
                mPhone.getIccFileHandler().loadEFLinearFixed(IccConstants.EF_SMS, index, response);

                try {
                    mMtkLock.wait();
                } catch (InterruptedException e) {
                    log("interrupted while trying to load from the SIM");
                }
            }
        }

        return mSmsRawData;
    }

    public List<SmsRawData> getAllMessagesFromIccEfByMode(String callingPackage, int mode) {
        if (DBG) log("getAllMessagesFromIccEfByMode, mode=" + mode);
        if (mode < PhoneConstants.PHONE_TYPE_GSM || mode > PhoneConstants.PHONE_TYPE_CDMA) {
            log("getAllMessagesFromIccEfByMode wrong mode=" + mode);
            return null;
        }

        // MTK-START, SIM related operation will be called by non-phone process user
        mContext.enforceCallingOrSelfPermission(
                "android.permission.RECEIVE_SMS",
                "Reading messages from Icc");
        // MTK-END
        if (mAppOps.noteOp(AppOpsManager.OP_READ_ICC_SMS, Binder.getCallingUid(),
                callingPackage) != AppOpsManager.MODE_ALLOWED) {
            return new ArrayList<SmsRawData>();
        }
        // MTK-START
        synchronized (mMtkLoadLock) {
        // MTK-END
            IccFileHandler fh = mPhone.getIccFileHandler();
            if (fh == null) {
                Rlog.e(LOG_TAG, "Cannot load Sms records. No icc card?");
                if (mSms != null) {
                    mSms.clear();
                    return mSms;
                }
                return null;
            }

            Message response = mMtkHandler.obtainMessage(EVENT_MTK_LOAD_DONE);
            if (PhoneConstants.PHONE_TYPE_GSM == mPhone.getPhoneType()) {
                Rlog.e(LOG_TAG, "getAllMessagesFromIccEfByMode. In the case of GSM phone");
                MtkSIMFileHandler sfh = (MtkSIMFileHandler)fh;
                sfh.loadEFLinearFixedAll(IccConstants.EF_SMS, mode, response);

                try {
                    // MTK-START
                    mMtkLoadLock.wait();
                    // MTK-END
                } catch (InterruptedException e) {
                    log("interrupted while trying to load from the SIM");
                }
            }

            return mSms;
        }
    }

    public MtkSmsParameters getSmsParameters(String callingPackage) {
        log("getSmsParameters");
        enforceReceiveAndSend("Get SMS parametner on SIM");
        if (mAppOps.noteOp(AppOpsManager.OP_READ_ICC_SMS, Binder.getCallingUid(),
                callingPackage) != AppOpsManager.MODE_ALLOWED) {
            return null;
        }
        synchronized (mMtkLock) {
            Message response = mMtkHandler.obtainMessage(EVENT_GET_SMS_PARAMS);
            MtkRIL ci = (MtkRIL)mPhone.mCi;
            ci.getSmsParameters(response);

            try {
                mMtkLock.wait();
            } catch (InterruptedException e) {
                log("interrupted while trying to get sms params");
            }

            return mSmsParams;
        }
    }

    public boolean setSmsParameters(String callingPackage, MtkSmsParameters params) {
        log("setSmsParameters");
        enforceReceiveAndSend("Set SMS parametner on SIM");
        if (mAppOps.noteOp(AppOpsManager.OP_WRITE_ICC_SMS, Binder.getCallingUid(),
                callingPackage) != AppOpsManager.MODE_ALLOWED) {
            return false;
        }
        mSmsParamsSuccess = false;
        synchronized (mMtkLock) {
            Message response = mMtkHandler.obtainMessage(EVENT_SET_SMS_PARAMS);
            MtkRIL ci = (MtkRIL)mPhone.mCi;
            ci.setSmsParameters(params, response);

            try {
                mMtkLock.wait();
            } catch (InterruptedException e) {
                log("interrupted while trying to get sms params");
            }
        }
        return mSmsParamsSuccess;
    }

    public int copyTextMessageToIccCard(String callingPkg, String scAddress, String address,
            List<String> text, int status, long timestamp) {
        if (DBG) {
            log("copyTextMessageToIccCard, message count: "
                    + text.size() + " status: " + status);
        }
        enforceReceiveAndSend("Copying message to USIM/SIM");
        if (mAppOps.noteOp(AppOpsManager.OP_WRITE_ICC_SMS, Binder.getCallingUid(),
                callingPkg) != AppOpsManager.MODE_ALLOWED) {
            return RESULT_ERROR_GENERIC_FAILURE;
        }

        MtkIccSmsStorageStatus memStatus;

        memStatus = getSmsSimMemoryStatus(callingPkg);

        if (memStatus == null) {
            log("Fail to get SIM memory status");
            return RESULT_ERROR_GENERIC_FAILURE;
        } else {
            if (memStatus.getUnused() < text.size()) {
                log("SIM memory is not enough");
                return RESULT_ERROR_SIM_MEM_FULL;
            }
        }


        return ((MtkSmsDispatchersController)mDispatchersController).copyTextMessageToIccCard(
                scAddress, address, text, status, timestamp);
    }

    public MtkSimSmsInsertStatus insertTextMessageToIccCard(String callingPackage,
            String scAddress, String address, List<String> text, int status, long timestamp) {
        log("insertTextMessageToIccCard");
        enforceReceiveAndSend("insertText insert message into SIM");
        if (mAppOps.noteOp(AppOpsManager.OP_WRITE_ICC_SMS, Binder.getCallingUid(),
                callingPackage) != AppOpsManager.MODE_ALLOWED) {
            smsInsertRet.insertStatus = RESULT_ERROR_GENERIC_FAILURE;
            return smsInsertRet;
        }

        int msgCount = text.size();
        boolean isDeliverPdu = true;

        log("insertText msgCount=" + msgCount + ", status=" + status);

        smsInsertRet.indexInIcc = "";

        MtkIccSmsStorageStatus memStatus = getSmsSimMemoryStatus(callingPackage);
        if (memStatus != null) {
            int unused = memStatus.getUnused();
            if (unused < msgCount) {
                log("insertText SIM mem is not enough [" + unused + "/" + msgCount + "]");
                smsInsertRet.insertStatus = RESULT_ERROR_SIM_MEM_FULL;
                return smsInsertRet;
            }
        } else {
            log("insertText fail to get SIM mem status");
            smsInsertRet.insertStatus = RESULT_ERROR_GENERIC_FAILURE;
            return smsInsertRet;
        }

        if (checkPhoneNumberInternal(scAddress) == false) {
            log("insertText invalid sc address");
            scAddress = null;
        }

        if (checkPhoneNumberInternal(address) == false) {
            log("insertText invalid address");
            smsInsertRet.insertStatus = RESULT_ERROR_INVALID_ADDRESS;
            return smsInsertRet;
        }

        if (status == STATUS_ON_ICC_READ || status == STATUS_ON_ICC_UNREAD) {
            log("insertText to encode delivery pdu");
            isDeliverPdu = true;
        } else if (status == STATUS_ON_ICC_SENT || status == STATUS_ON_ICC_UNSENT) {
            log("insertText to encode submit pdu");
            isDeliverPdu = false;
        } else {
            log("insertText invalid status " + status);
            smsInsertRet.insertStatus = RESULT_ERROR_GENERIC_FAILURE;
            return smsInsertRet;
        }
        log("insertText params check pass");
        // MTK-START
        if (PhoneConstants.PHONE_TYPE_CDMA == mPhone.getPhoneType()) {
            return writeTextMessageToRuim(address, text, status, timestamp);
        }
        // MTK-END

        int encoding = SmsMessage.ENCODING_UNKNOWN;
        TextEncodingDetails details[] = new TextEncodingDetails[msgCount];
        for (int i = 0; i < msgCount; ++i) {
            details[i] = com.android.internal.telephony.gsm.SmsMessage.calculateLength(text.get(i),
                    false);
            if (encoding != details[i].codeUnitSize &&
                (encoding == SmsMessage.ENCODING_UNKNOWN || encoding == SmsMessage.ENCODING_7BIT)) {
                // use the USC2 if only one message is that coding style
                encoding = details[i].codeUnitSize;
            }
        }

        log("insertText create & insert pdu start...");
        for (int i = 0; i < msgCount; ++i) {
            if (mInsertMessageSuccess == false && i > 0) {
                log("insertText last message insert fail");
                smsInsertRet.insertStatus = RESULT_ERROR_GENERIC_FAILURE;
                return smsInsertRet;
            }

            int singleShiftId = -1;
            int lockingShiftId = -1;
            int language = details[i].shiftLangId;
            int encoding_detail = encoding;

            if (encoding == SmsMessage.ENCODING_7BIT) {
                if (details[i].languageTable > 0 && details[i].languageShiftTable > 0) {
                    singleShiftId = details[i].languageTable;
                    lockingShiftId = details[i].languageShiftTable;
                    encoding_detail =
                            com.mediatek.internal.telephony.gsm.
                            MtkSmsMessage.ENCODING_7BIT_LOCKING_SINGLE;
                } else if (details[i].languageShiftTable > 0) {
                    lockingShiftId = details[i].languageShiftTable;
                    encoding_detail =
                            com.mediatek.internal.telephony.gsm.MtkSmsMessage.ENCODING_7BIT_LOCKING;
                } else if (details[i].languageTable > 0) {
                    singleShiftId = details[i].languageTable;
                    encoding_detail =
                            com.mediatek.internal.telephony.gsm.MtkSmsMessage.ENCODING_7BIT_SINGLE;
                }
            }

            byte[] smsHeader = null;
            if (msgCount > 1) {
                log("insertText create pdu header for concat-message");
                smsHeader = MtkSmsHeader.getSubmitPduHeaderWithLang(-1, (getNextConcatRef() & 0xff),
                        (i + 1), msgCount, singleShiftId, lockingShiftId);
            }

            if (isDeliverPdu) {
                com.mediatek.internal.telephony.gsm.MtkSmsMessage.DeliverPdu pdu =
                        com.mediatek.internal.telephony.gsm.MtkSmsMessage.getDeliverPduWithLang(
                        scAddress, address, text.get(i), smsHeader, timestamp, encoding_detail,
                        language);
                if (pdu != null) {
                    synchronized (mSimInsertLock) {
                        mPhone.mCi.writeSmsToSim(status,
                                IccUtils.bytesToHexString(pdu.encodedScAddress),
                                IccUtils.bytesToHexString(pdu.encodedMessage),
                                mMtkHandler.obtainMessage(EVENT_INSERT_TEXT_MESSAGE_TO_ICC_DONE));

                        try {
                            log("insertText wait until the pdu be wrote into the SIM");
                            mSimInsertLock.wait();
                        } catch (InterruptedException e) {
                            log("insertText fail to insert pdu");
                            smsInsertRet.insertStatus = RESULT_ERROR_GENERIC_FAILURE;
                            return smsInsertRet;
                        }
                    }
                } else {
                    log("insertText fail to create deliver pdu");
                    smsInsertRet.insertStatus = RESULT_ERROR_GENERIC_FAILURE;
                    return smsInsertRet;
                }
            } else {
                com.mediatek.internal.telephony.gsm.MtkSmsMessage.SubmitPdu pdu =
                        com.mediatek.internal.telephony.gsm.MtkSmsMessage.getSubmitPduWithLang(
                        scAddress, address, text.get(i), false, smsHeader, encoding_detail,
                        language, -1);
                if (pdu != null) {
                    synchronized (mSimInsertLock) {
                        mPhone.mCi.writeSmsToSim(status,
                                IccUtils.bytesToHexString(pdu.encodedScAddress),
                                IccUtils.bytesToHexString(pdu.encodedMessage),
                                mMtkHandler.obtainMessage(EVENT_INSERT_TEXT_MESSAGE_TO_ICC_DONE));

                        try {
                            log("insertText wait until the pdu be wrote into the SIM");
                            mSimInsertLock.wait();
                        } catch (InterruptedException e) {
                            log("insertText fail to insert pdu");
                            smsInsertRet.insertStatus = RESULT_ERROR_GENERIC_FAILURE;
                            return smsInsertRet;
                        }
                    }
                } else {
                    log("insertText fail to create submit pdu");
                    smsInsertRet.insertStatus = RESULT_ERROR_GENERIC_FAILURE;
                    return smsInsertRet;
                }
            }
        } // end loop for pdu creation & insertion
        log("insertText create & insert pdu end");

        if (mInsertMessageSuccess == true) {
            log("insertText all messages inserted");
            smsInsertRet.insertStatus = RESULT_ERROR_GENERIC_FAILURE;
            return smsInsertRet;
        }

        log("insertText pdu insert fail");
        smsInsertRet.insertStatus = RESULT_ERROR_GENERIC_FAILURE;
        return smsInsertRet;
    }

    public MtkSimSmsInsertStatus insertRawMessageToIccCard(String callingPackage, int status,
            byte[] pdu, byte[] smsc) {
        if (DBG) log("insertRawMessageToIccCard");
        enforceReceiveAndSend("insertRaw insert message into SIM");
        if (mAppOps.noteOp(AppOpsManager.OP_WRITE_ICC_SMS, Binder.getCallingUid(),
                callingPackage) != AppOpsManager.MODE_ALLOWED) {
            smsInsertRet2.insertStatus = RESULT_ERROR_GENERIC_FAILURE;
            return smsInsertRet2;
        }
        synchronized (mMtkLock) {
            mMtkSuccess = false;
            smsInsertRet2.insertStatus = RESULT_ERROR_GENERIC_FAILURE;
            smsInsertRet2.indexInIcc = "";
            Message response = mMtkHandler.obtainMessage(EVENT_MTK_UPDATE_DONE);

            //RIL_REQUEST_WRITE_SMS_TO_SIM vs RIL_REQUEST_CDMA_WRITE_SMS_TO_RUIM
            if (PhoneConstants.PHONE_TYPE_CDMA != mPhone.getPhoneType()) {
                mPhone.mCi.writeSmsToSim(status, IccUtils.bytesToHexString(smsc),
                        IccUtils.bytesToHexString(pdu), response);
            } else {
                mPhone.mCi.writeSmsToRuim(status, IccUtils.bytesToHexString(pdu), response);
            }

            try {
                mMtkLock.wait();
            } catch (InterruptedException e) {
                log("insertRaw interrupted while trying to update by index");
            }
        }

        if (mMtkSuccess == true) {
            log("insertRaw message inserted");
            smsInsertRet2.insertStatus = RESULT_ERROR_SUCCESS;
            return smsInsertRet2;
        }

        log("insertRaw pdu insert fail");
        smsInsertRet2.insertStatus = RESULT_ERROR_GENERIC_FAILURE;
        return smsInsertRet2;
    }

    public MtkIccSmsStorageStatus getSmsSimMemoryStatus(String callingPackage) {
        if (DBG) log("getSmsSimMemoryStatus");
        enforceReceiveAndSend("Get SMS SIM Card Memory Status from RUIM");
        if (mAppOps.noteOp(AppOpsManager.OP_READ_ICC_SMS, Binder.getCallingUid(),
                callingPackage) != AppOpsManager.MODE_ALLOWED) {
            return null;
        }
        synchronized (mMtkLock) {
            mMtkSuccess = false;

            Message response = mMtkHandler.obtainMessage(EVENT_GET_SMS_SIM_MEM_STATUS_DONE);
            MtkRIL ci = (MtkRIL)mPhone.mCi;
            if (mPhone.getPhoneType() == PhoneConstants.PHONE_TYPE_CDMA) {
                ci.getSmsRuimMemoryStatus(response);
            } else {
                ci.getSmsSimMemoryStatus(response);
            }

            try {
                mMtkLock.wait();
            } catch (InterruptedException e) {
                log("interrupted while trying to get SMS SIM Card Memory Status from SIM");
            }
        }

        if (mMtkSuccess) {
            return mSimMemStatus;
        }

        return null;
    }

    private static int getNextConcatRef() {
        return sConcatenatedRef++;
    }

    private static boolean checkPhoneNumberCharacter(char c) {
        return (c >= '0' && c <= '9') || (c == '*') || (c == '+')
                || (c == '#') || (c == 'N') || (c == ' ') || (c == '-');
    }

    private static boolean checkPhoneNumberInternal(String number) {
        if (number == null) {
            return true;
        }

        for (int i = 0, n = number.length(); i < n; ++i) {
            if (checkPhoneNumberCharacter(number.charAt(i))) {
                continue;
            } else {
                return false;
            }
        }

        return true;
    }

    /**
     * This function write one text message to uim card,
     * may be long text message.
     */
    protected MtkSimSmsInsertStatus writeTextMessageToRuim(String address,
            List<String> text, int status, long timestamp) {
        MtkSimSmsInsertStatus insertRet = new MtkSimSmsInsertStatus(RESULT_ERROR_SUCCESS, "");
        mMtkSuccess = true;

        for (int i = 0; i < text.size(); ++i) {
            if (mMtkSuccess == false) {
                log("[copyText Exception happened when copy message");
                insertRet.insertStatus = RESULT_ERROR_GENERIC_FAILURE;
                return insertRet;
            }

            com.mediatek.internal.telephony.cdma.MtkSmsMessage.SubmitPdu pdu
                    = com.mediatek.internal.telephony.cdma.MtkSmsMessage.createEfPdu(
                    address, text.get(i), timestamp);

            if (pdu != null) {
                synchronized (mSimInsertLock) {
                    Message response = mMtkHandler.obtainMessage(
                            EVENT_INSERT_TEXT_MESSAGE_TO_ICC_DONE);
                    mPhone.mCi.writeSmsToRuim(
                            status, IccUtils.bytesToHexString(pdu.encodedMessage), response);
                    mInserted = false;
                    try {
                        mSimInsertLock.wait();
                    } catch (InterruptedException e) {
                        log("InterruptedException " + e);
                        insertRet.insertStatus = RESULT_ERROR_GENERIC_FAILURE;
                        return insertRet;
                    }
                }
            } else {
                log("writeTextMessageToRuim: pdu == null");
                insertRet.insertStatus = RESULT_ERROR_GENERIC_FAILURE;
                return insertRet;
            }
        }

        log("writeTextMessageToRuim: done");
        insertRet.insertStatus = RESULT_ERROR_SUCCESS;
        return insertRet;
    }

    private String filterDestAddress(String destAddr) {
        String result  = null;
        result = SmsNumberUtils.filterDestAddr(mPhone, destAddr);
        return result != null ? result : destAddr;
    }

    /**
     * Judge if the destination address is a valid SMS address or not, and if
     * the text is null or not
     *
     * @destinationAddress the destination address to which the message be sent
     * @text the content of shorm message
     * @sentIntent will be broadcast if the address or the text is invalid
     * @return true for valid parameters
     */
    private static boolean isValidParameters(String destinationAddress, String text,
            PendingIntent sentIntent) {
        List<PendingIntent> sentIntents =
                new ArrayList<PendingIntent>();
        List<String> parts =
                new ArrayList<String>();

        sentIntents.add(sentIntent);
        parts.add(text);

        // if (TextUtils.isEmpty(text)) {
        // throw new IllegalArgumentException("Invalid message body");
        // }

        return isValidParameters(destinationAddress, parts, sentIntents);
    }

    /**
     * Judges if the destination address is a valid SMS address or not, and if
     * the text is null or not.
     *
     * @param destinationAddress The destination address to which the message be sent
     * @param parts The content of shorm message
     * @param sentIntent will be broadcast if the address or the text is invalid
     * @return True for valid parameters
     */
    private static boolean isValidParameters(String destinationAddress, List<String> parts,
            List<PendingIntent> sentIntents) {
        if (parts == null || parts.size() == 0) {
            return true;
        }

        if (!isValidSmsDestinationAddress(destinationAddress)) {
            for (int i = 0; i < sentIntents.size(); i++) {
                PendingIntent sentIntent = sentIntents.get(i);
                if (sentIntent != null) {
                    try {
                        sentIntent.send(SmsManager.RESULT_ERROR_GENERIC_FAILURE);
                    } catch (CanceledException ex) { }
                }
            }

            Rlog.d("IccSmsInterfaceManagerEx", "Invalid destinationAddress: " + destinationAddress);
            return false;
        }

        if (TextUtils.isEmpty(destinationAddress)) {
            Rlog.e("IccSmsInterfaceManagerEx", "Invalid destinationAddress");
            return false;
        }
        if (parts == null || parts.size() < 1) {
            Rlog.e("IccSmsInterfaceManagerEx", "Invalid message body");
            return false;
        }

        return true;
    }

    /**
     * judge if the input destination address is a valid SMS address or not
     *
     * @param da the input destination address
     * @return true for success
     *
     */
    private static boolean isValidSmsDestinationAddress(String da) {
        String encodeAddress = PhoneNumberUtils.extractNetworkPortion(da);
        if (encodeAddress == null)
            return true;

        return !(encodeAddress.isEmpty());
    }

    public boolean activateCellBroadcastSms(boolean activate) {
        log("activateCellBroadcastSms activate : " + activate);

        return setCellBroadcastActivation(activate);
    }

    public boolean queryCellBroadcastSmsActivation() {
        log("queryCellBroadcastSmsActivation");

        synchronized (mMtkLock) {
            Message response = mMtkHandler.obtainMessage(EVENT_GET_BROADCAST_ACTIVATION_DONE);

            mMtkSuccess = false;
            MtkRIL ci = (MtkRIL)mPhone.mCi;
            ci.getGsmBroadcastActivation(response);

            try {
                mMtkLock.wait();
            } catch (InterruptedException e) {
                log("interrupted while trying to get CB activation");
            }
        }

        return mMtkSuccess;
    }

    public String getCellBroadcastRanges() {
        log("getCellBroadcastChannels");

        synchronized (mMtkLock) {
            Message response = mMtkHandler.obtainMessage(EVENT_GET_BROADCAST_CONFIG_CHANNEL_DONE);

            mSmsCbChannelConfig = "";
            mPhone.mCi.getGsmBroadcastConfig(response);

            try {
                mMtkLock.wait();
            } catch (InterruptedException e) {
                log("interrupted while trying to get CB config");
            }
        }

        return mSmsCbChannelConfig;
    }

    public boolean setCellBroadcastLangs(String lang) {
        log("setCellBroadcastLangs");

        synchronized (mMtkLock) {
            Message response = mMtkHandler.obtainMessage(EVENT_SET_BROADCAST_CONFIG_LANGUAGE_DONE);

            mMtkSuccess = false;
            MtkRIL ci = (MtkRIL)(mPhone.mCi);
            ci.setGsmBroadcastLangs(lang, response);

            try {
                mMtkLock.wait();
            } catch (InterruptedException e) {
                log("interrupted while trying to get CB config");
            }
        }

        return mMtkSuccess;
    }

    public String getCellBroadcastLangs() {
        log("getCellBroadcastLangs");

        synchronized (mMtkLock) {
            Message response = mMtkHandler.obtainMessage(EVENT_GET_BROADCAST_CONFIG_LANGUAGE_DONE);

            mSmsCbLanguageConfig = "";
            MtkRIL ci = (MtkRIL)(mPhone.mCi);
            ci.getGsmBroadcastLangs(response);

            try {
                mMtkLock.wait();
            } catch (InterruptedException e) {
                log("interrupted while trying to get CB config");
            }

            return mSmsCbLanguageConfig;
        }
    }

    public boolean removeCellBroadcastMsg(int channelId, int serialId) {
        if (DBG) log("removeCellBroadcastMsg(" + channelId + " , " + serialId + ")");

        synchronized (mMtkLock) {
            Message response = mMtkHandler.obtainMessage(EVENT_REMOVE_BROADCAST_MSG_DONE);

            mMtkSuccess = false;
            MtkRIL ci = (MtkRIL)(mPhone.mCi);
            ci.removeCellBroadcastMsg(channelId, serialId, response);

            try {
                mMtkLock.wait();
            } catch (InterruptedException e) {
                log("interrupted while trying to remove CB msg");
            }
        }

        return mMtkSuccess;
    }

    public boolean setEtwsConfig(int mode) {
        if (DBG) log("Calling setEtwsConfig(" + mode + ')');

        synchronized (mMtkLock) {
            Message response = mMtkHandler.obtainMessage(EVENT_SET_ETWS_CONFIG_DONE);

            mMtkSuccess = false;
            MtkRIL ci = (MtkRIL)(mPhone.mCi);
            ci.setEtws(mode, response);

            try {
                mMtkLock.wait();
            } catch (InterruptedException e) {
                log("interrupted while trying to set ETWS config");
            }
        }

        return mMtkSuccess;
    }

    /**
     * Get service center address
     *
     * @return service message center address
     */
    public String getScAddress() {
        if (DBG) log("getScAddress");
        synchronized (mMtkLock) {
            Message response = mMtkHandler.obtainMessage(EVENT_GET_SMSC_ADDRESS_DONE);

            mPhone.getSmscAddress(response);

            try {
                mMtkLock.wait();
            } catch (InterruptedException e) {
                log("interrupted while trying to get SMSC address");
            }
        }

        log("getScAddress: exit" );
        return mSmscAddress;
    }

    /**
     * Get service center address
     *
     * @return service message center address and error code
     */
    public Bundle getScAddressWithErrorCode() {
        if (DBG) log("getScAddressWithErrorCode");
        synchronized (mMtkLock) {
            Message response = mMtkHandler.obtainMessage(EVENT_GET_SMSC_ADDRESS_BUNDLE_DONE);

            mPhone.getSmscAddress(response);

            try {
                mMtkLock.wait();
            } catch (InterruptedException e) {
                log("interrupted while trying to get SMSC address and error code");
            }
        }

        log("getScAddressWithErrorCode error code done");
        return mSmscAddressBundle;
    }

    /**
     * Set service message center address
     *
     * @param address service message center addressto be set
     *
     * @return true for success, false for failure
     */
    public boolean setScAddress(String address) {
        if (DBG) log("setScAddressUsingSubId");

        synchronized (mMtkLock) {
            Message response = mMtkHandler.obtainMessage(EVENT_SET_SMSC_ADDRESS_DONE);

            mMtkSuccess = false;
            mPhone.setSmscAddress(address, response);

            try {
                mMtkLock.wait();
            } catch (InterruptedException e) {
                log("interrupted while trying to set SMSC address");
            }
        }
        if (DBG) log("setScAddressUsingSubId result " + mMtkSuccess);
        return mMtkSuccess;
    }

    private boolean checkTddDataOnlyPermission(PendingIntent sentIntent) {
        MtkLteDataOnlyController dataOnlyCtrl = new MtkLteDataOnlyController(mContext);
        if (!dataOnlyCtrl.checkPermission(mPhone.getSubId())) {
            // Notify caller
            log("checkTddDataOnlyPermission, w/o permission, sentIntent = " + sentIntent);
            if (sentIntent == null) {
                log("checkTddDataOnlyPermission, can not notify APP");

            } else {
                try {
                    sentIntent.send(SmsManager.RESULT_ERROR_GENERIC_FAILURE);
                } catch (CanceledException ex) {
                    loge("checkTddDataOnlyPermission, CanceledException happened " +
                            "when send sms fail with sentIntent");
                }
            }
            return false;
        }
        return true;
    }

    private boolean checkTddDataOnlyPermission(List<PendingIntent> sentIntents) {
        MtkLteDataOnlyController dataOnlyCtrl = new MtkLteDataOnlyController(mContext);
        if (!dataOnlyCtrl.checkPermission(mPhone.getSubId())) {
            // Notify caller
            log("checkTddDataOnlyPermission, w/o permission, sentIntents = " + sentIntents);
            if (sentIntents == null) {
                log("checkTddDataOnlyPermission, can not notify APP");
            } else {
                try {
                    PendingIntent si = null;
                    int size = sentIntents.size();
                    for (int i = 0; i < size; i++) {
                        si = sentIntents.get(i);
                        if (si == null) {
                            log("checkTddDataOnlyPermission, can not notify APP for i = " + i);
                        } else {
                            si.send(SmsManager.RESULT_ERROR_GENERIC_FAILURE);
                        }
                    }
                } catch (CanceledException ex) {
                    loge("checkTddDataOnlyPermission, CanceledException happened " +
                            "when send sms fail with sentIntent");
                }
            }
            return false;
        }
        return true;
    }

    private static final int CB_ACTIVATION_UNKNOWN = -1;
    private static final int CB_ACTIVATION_OFF = 0;
    private static final int CB_ACTIVATION_ON = 1;
    private int mCurrentCellBroadcastActivation = CB_ACTIVATION_UNKNOWN;

    protected boolean setCellBroadcastActivation(boolean activate) {
        if (DBG)
            log("Calling proprietary setCellBroadcastActivation(" + activate + ')');

        int newActivationState = (activate) ? CB_ACTIVATION_ON: CB_ACTIVATION_OFF;

        // In order to prevent frequently request from AT command conflict, we only send
        // the request when the setting is different
        if (mCurrentCellBroadcastActivation != newActivationState) {
            super.setCellBroadcastActivation(activate);
        } else {
            mSuccess = true;
        }

        if (mSuccess && (mCurrentCellBroadcastActivation != newActivationState)) {
            mCurrentCellBroadcastActivation = newActivationState;
            log("mCurrentCellBroadcastActivation change to " + mCurrentCellBroadcastActivation);
        }

        return mSuccess;
    }
    /**
     * Send a text based SMS with Messaging Options.
     *
     * @param destAddr the address to send the message to
     * @param scAddr is the service center address or null to use
     *  the current default SMSC
     * @param text the body of the message to send
     * @param sentIntent if not NULL this <code>PendingIntent</code> is
     *  broadcast when the message is successfully sent, or failed.
     *  The result code will be <code>Activity.RESULT_OK<code> for success,
     *  or one of these errors:<br>
     *  <code>RESULT_ERROR_GENERIC_FAILURE</code><br>
     *  <code>RESULT_ERROR_RADIO_OFF</code><br>
     *  <code>RESULT_ERROR_NULL_PDU</code><br>
     *  For <code>RESULT_ERROR_GENERIC_FAILURE</code> the sentIntent may include
     *  the extra "errorCode" containing a radio technology specific value,
     *  generally only useful for troubleshooting.<br>
     *  The per-application based SMS control checks sentIntent. If sentIntent
     *  is NULL the caller will be checked against all unknown applications,
     *  which cause smaller number of SMS to be sent in checking period.
     * @param deliveryIntent if not NULL this <code>PendingIntent</code> is
     *  broadcast when the message is delivered to the recipient.  The
     *  raw pdu of the status report is in the extended data ("pdu").
     * @param persistMessageForNonDefaultSmsApp whether the sent message should
     *  be automatically persisted in the SMS db. It only affects messages sent
     *  by a non-default SMS app. Currently only the carrier app can set this
     *  parameter to false to skip auto message persistence.
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
     *  Any Other values including negative considered as Invalid Priority Indicator of the message.
     * @param expectMore is a boolean to indicate the sending messages through same link or not.
     * @param validityPeriod Validity Period of the message in mins.
     *  Refer specification 3GPP TS 23.040 V6.8.1 section 9.2.3.12.1.
     *  Validity Period(Minimum) -> 5 mins
     *  Validity Period(Maximum) -> 635040 mins(i.e.63 weeks).
     *  Any Other values including negative considered as Invalid Validity Period of the message.
     * @param encodingType the encoding type of content of message(GSM 7-bit, Unicode or Automatic)
     */

    public void sendTextWithOptions(String callingPackage, String destAddr, String scAddr,
            String text, PendingIntent sentIntent, PendingIntent deliveryIntent,
            boolean persistMessageForNonDefaultSmsApp, int priority, boolean expectMore,
            int validityPeriod, int encodingType) {
        mPhone.getContext().enforceCallingPermission(
                Manifest.permission.SEND_SMS,
                "Sending SMS message");

        if (mAppOps.noteOp(AppOpsManager.OP_SEND_SMS, Binder.getCallingUid(),
                callingPackage) != AppOpsManager.MODE_ALLOWED) {
            return;
        }

        if (!checkTddDataOnlyPermission(sentIntent)) {
            log("TDD data only and w/o permission!");
            return;
        }

        ((MtkSmsDispatchersController)mDispatchersController).sendTextWithEncodingType(destAddr,
                scAddr, text, encodingType, sentIntent, deliveryIntent, null/*messageUri*/,
                callingPackage, persistMessageForNonDefaultSmsApp, priority, expectMore,
                validityPeriod);
    }

    /**
     * Send a multi-part text based SMS with Messaging Options.
     *
     * @param destAddr the address to send the message to
     * @param scAddr is the service center address or null to use
     *   the current default SMSC
     * @param parts an <code>ArrayList</code> of strings that, in order,
     *   comprise the original message
     * @param sentIntents if not null, an <code>ArrayList</code> of
     *   <code>PendingIntent</code>s (one for each message part) that is
     *   broadcast when the corresponding message part has been sent.
     *   The result code will be <code>Activity.RESULT_OK<code> for success,
     *   or one of these errors:
     *   <code>RESULT_ERROR_GENERIC_FAILURE</code>
     *   <code>RESULT_ERROR_RADIO_OFF</code>
     *   <code>RESULT_ERROR_NULL_PDU</code>.
     *  The per-application based SMS control checks sentIntent. If sentIntent
     *  is NULL the caller will be checked against all unknown applications,
     *  which cause smaller number of SMS to be sent in checking period.
     * @param deliveryIntents if not null, an <code>ArrayList</code> of
     *   <code>PendingIntent</code>s (one for each message part) that is
     *   broadcast when the corresponding message part has been delivered
     *   to the recipient.  The raw pdu of the status report is in the
     *   extended data ("pdu").
     * @param persistMessageForNonDefaultSmsApp whether the sent message should
     *   be automatically persisted in the SMS db. It only affects messages sent
     *   by a non-default SMS app. Currently only the carrier app can set this
     *   parameter to false to skip auto message persistence.
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
     *  Any Other values including negative considered as Invalid Priority Indicator of the message.
     * @param expectMore is a boolean to indicate the sending messages through same link or not.
     * @param validityPeriod Validity Period of the message in mins.
     *  Refer specification 3GPP TS 23.040 V6.8.1 section 9.2.3.12.1.
     *  Validity Period(Minimum) -> 5 mins
     *  Validity Period(Maximum) -> 635040 mins(i.e.63 weeks).
     *  Any Other values including negative considered as Invalid Validity Period of the message.
     * @param encodingType the encoding type of content of message(GSM 7-bit, Unicode or Automatic)
     */
    public void sendMultipartTextWithOptions(String callingPackage, String destAddr,
            String scAddr, List<String> parts, List<PendingIntent> sentIntents,
            List<PendingIntent> deliveryIntents, boolean persistMessageForNonDefaultSmsApp,
            int priority, boolean expectMore, int validityPeriod, int encodingType) {
        mPhone.getContext().enforceCallingPermission(
                Manifest.permission.SEND_SMS,
                "Sending SMS message");

        if (mAppOps.noteOp(AppOpsManager.OP_SEND_SMS, Binder.getCallingUid(),
                callingPackage) != AppOpsManager.MODE_ALLOWED) {
            return;
        }

        if (!checkTddDataOnlyPermission(sentIntents)) {
            log("TDD data only and w/o permission!");
            return;
        }

        destAddr = filterDestAddress(destAddr);

        if (parts.size() > 1 && parts.size() < 10 && !SmsMessage.hasEmsSupport()) {
            for (int i = 0; i < parts.size(); i++) {
                // If EMS is not supported, we have to break down EMS into single segment SMS
                // and add page info " x/y".
                String singlePart = parts.get(i);
                if (SmsMessage.shouldAppendPageNumberAsPrefix()) {
                    singlePart = String.valueOf(i + 1) + '/' + parts.size() + ' ' + singlePart;
                } else {
                    singlePart = singlePart.concat(' ' + String.valueOf(i + 1) + '/' +
                            parts.size());
                }

                PendingIntent singleSentIntent = null;
                if (sentIntents != null && sentIntents.size() > i) {
                    singleSentIntent = sentIntents.get(i);
                }

                PendingIntent singleDeliveryIntent = null;
                if (deliveryIntents != null && deliveryIntents.size() > i) {
                    singleDeliveryIntent = deliveryIntents.get(i);
                }

                ((MtkSmsDispatchersController)mDispatchersController).sendTextWithEncodingType(
                        destAddr, scAddr, singlePart, encodingType, singleSentIntent,
                        singleDeliveryIntent, null/*messageUri*/, callingPackage,
                        persistMessageForNonDefaultSmsApp, priority, expectMore, validityPeriod);
            }
            return;
        }

        ((MtkSmsDispatchersController)mDispatchersController).sendMultipartTextWithEncodingType(
                destAddr, scAddr, (ArrayList<String>) parts, encodingType,
                (ArrayList<PendingIntent>) sentIntents, (ArrayList<PendingIntent>) deliveryIntents,
                null/*messageUri*/, callingPackage,
                persistMessageForNonDefaultSmsApp, priority, expectMore, validityPeriod);
    }
}
