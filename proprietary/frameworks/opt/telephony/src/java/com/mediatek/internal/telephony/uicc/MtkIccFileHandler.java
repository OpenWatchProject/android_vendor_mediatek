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

package com.mediatek.internal.telephony.uicc;

import android.telephony.Rlog;

import android.os.Message;
import android.os.AsyncResult;

import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.uicc.IccFileHandler;
import com.android.internal.telephony.uicc.UiccCardApplication;
import com.android.internal.telephony.uicc.IccIoResult;
import com.android.internal.telephony.uicc.IccFileTypeMismatch;
import com.android.internal.telephony.uicc.IccConstants;
import com.android.internal.telephony.uicc.IccUtils;
import com.android.internal.telephony.PhoneConstants;

import com.mediatek.internal.telephony.uicc.EFResponseData;

import java.util.ArrayList;

/**
 * {@hide}
 */
public final class MtkIccFileHandler extends IccFileHandler {
    static final String LOG_TAG = "MtkIccFileHandler";

    // PHB START
    static protected final int EVENT_SELECT_EF_FILE = 100;
    // PHB END
    /** Finished retrieving size of transparent EF; start loading. */
    static protected final int EVENT_GET_BINARY_SIZE_DONE_EX = 101;
    /** Finished retrieving size of records for linear-fixed EF; now load. */
    static protected final int EVENT_GET_RECORD_SIZE_DONE_EX = 102;
    /** Finished loading single record from a linear-fixed EF; post result. */
    static protected final int EVENT_READ_RECORD_DONE_EX = 103;

    //***** Instance Variables

    //***** Constructor

    public MtkIccFileHandler(UiccCardApplication app, String aid, CommandsInterface ci) {
        super(app, aid, ci);
        logd("SelectFileHandlerEx constructor");
    }


    static class MtkLoadLinearFixedContext /*extends LoadLinearFixedContext*/ {

        int mEfid;
        int mRecordNum, mRecordSize, mCountRecords;
        boolean mLoadAll;
        String mPath;
        // MTK-START
        int mMode;
        // MTK-END

        Message mOnLoaded;

        ArrayList<byte[]> results;

        MtkLoadLinearFixedContext(int efid, Message onLoaded) {
            mEfid = efid;
            mRecordNum = 1;
            mLoadAll = true;
            mOnLoaded = onLoaded;
            mPath = null;
            // MTK-START
            mMode = -1;
            // MTK-END
        }

    }

    // MTK-START
    /**
     * Transparent context class.
     */
    static class MtkLoadTransparentContext {
        int mEfid;
        String mPath;
        Message mOnLoaded;

        MtkLoadTransparentContext(int efid, String path, Message onLoaded) {
            mEfid = efid;
            mPath = path;
            mOnLoaded = onLoaded;
        }
    }
    // MTK-END

    //***** Overridden from IccFileHandler

    @Override
    public void handleMessage(Message msg) {
        AsyncResult ar;
        IccIoResult result;
        Message response = null;
        byte data[];
        int size;
        MtkLoadLinearFixedContext lc;
        String path = null;

        try {
            switch (msg.what) {
                case EVENT_GET_BINARY_SIZE_DONE_EX:
                    ar = (AsyncResult) msg.obj;
                    MtkLoadTransparentContext tc = (MtkLoadTransparentContext) ar.userObj;
                    result = (IccIoResult) ar.result;
                    response = tc.mOnLoaded;
                    path = tc.mPath;
                    if (processException(response, (AsyncResult) msg.obj)) {
                        break;
                    }

                    data = result.payload;
                    if (TYPE_EF != data[RESPONSE_DATA_FILE_TYPE]) {
                        throw new IccFileTypeMismatch();
                    }
                    if (EF_TYPE_TRANSPARENT != data[RESPONSE_DATA_STRUCTURE]) {
                        throw new IccFileTypeMismatch();
                    }
                    size = ((data[RESPONSE_DATA_FILE_SIZE_1] & 0xff) << 8)
                           + (data[RESPONSE_DATA_FILE_SIZE_2] & 0xff);

                    if (path == null) {
                        path = getEFPath(tc.mEfid);
                    }
                    mCi.iccIOForApp(COMMAND_READ_BINARY, tc.mEfid, path, 0, 0, size, null, null,
                            mAid, obtainMessage(EVENT_READ_BINARY_DONE, tc.mEfid, 0, response));
                break;
                case EVENT_GET_RECORD_SIZE_DONE_EX:
                   ar = (AsyncResult)msg.obj;
                   lc = (MtkLoadLinearFixedContext) ar.userObj;
                   result = (IccIoResult) ar.result;
                   response = lc.mOnLoaded;

                   if (processException(response, (AsyncResult) msg.obj)) {
                       break;
                   }

                   data = result.payload;
                   path = lc.mPath;

                   if (TYPE_EF != data[RESPONSE_DATA_FILE_TYPE]) {
                       throw new IccFileTypeMismatch();
                   }

                   if (EF_TYPE_LINEAR_FIXED != data[RESPONSE_DATA_STRUCTURE]) {
                       throw new IccFileTypeMismatch();
                   }

                   lc.mRecordSize = data[RESPONSE_DATA_RECORD_LENGTH] & 0xFF;

                   size = ((data[RESPONSE_DATA_FILE_SIZE_1] & 0xff) << 8)
                          + (data[RESPONSE_DATA_FILE_SIZE_2] & 0xff);

                   lc.mCountRecords = size / lc.mRecordSize;

                   if (lc.mLoadAll) {
                       lc.results = new ArrayList<byte[]>(lc.mCountRecords);
                   }

                   // MTK-START
                   if (lc.mMode != -1) {
                       mCi.iccIOForApp(COMMAND_READ_RECORD, lc.mEfid, getSmsEFPath(lc.mMode),
                               lc.mRecordNum,
                               READ_RECORD_MODE_ABSOLUTE,
                               lc.mRecordSize, null, null, mAid,
                               obtainMessage(EVENT_READ_RECORD_DONE_EX, lc));
                   } else {
                   // MTK-END
                        if (path == null) {
                            path = getEFPath(lc.mEfid);
                        }
                        mCi.iccIOForApp(COMMAND_READ_RECORD, lc.mEfid, path,
                                lc.mRecordNum,
                                READ_RECORD_MODE_ABSOLUTE,
                                lc.mRecordSize, null, null, mAid,
                                obtainMessage(EVENT_READ_RECORD_DONE_EX, lc));
                   // MTK-START
                   }
                   // MTK-END
                   break;
                case EVENT_READ_RECORD_DONE_EX:

                    ar = (AsyncResult)msg.obj;
                    lc = (MtkLoadLinearFixedContext) ar.userObj;
                    result = (IccIoResult) ar.result;
                    response = lc.mOnLoaded;
                    path = lc.mPath;

                    if (processException(response, (AsyncResult) msg.obj)) {
                        break;
                    }

                    if (!lc.mLoadAll) {
                        sendResult(response, result.payload, null);
                    } else {
                        lc.results.add(result.payload);
                        lc.mRecordNum++;

                        if (lc.mRecordNum > lc.mCountRecords) {
                            sendResult(response, lc.results, null);
                        } else {
                            // MTK-START
                            if (lc.mMode != -1) {
                                mCi.iccIOForApp(COMMAND_READ_RECORD, lc.mEfid,
                                            getSmsEFPath(lc.mMode),
                                            lc.mRecordNum,
                                            READ_RECORD_MODE_ABSOLUTE,
                                            lc.mRecordSize, null, null, mAid,
                                            obtainMessage(EVENT_READ_RECORD_DONE_EX, lc));
                            } else {
                            // MTK-END
                                if (path == null) {
                                    path = getEFPath(lc.mEfid);
                                }

                                mCi.iccIOForApp(COMMAND_READ_RECORD, lc.mEfid, path,
                                            lc.mRecordNum,
                                            READ_RECORD_MODE_ABSOLUTE,
                                            lc.mRecordSize, null, null, mAid,
                                            obtainMessage(EVENT_READ_RECORD_DONE_EX, lc));
                            // MTK-START
                            }
                            // MTK-END
                        }
                    }
                break;
                // PHB START
                case EVENT_SELECT_EF_FILE:
                    ar = (AsyncResult) msg.obj;
                    response = (Message) ar.userObj;
                    result = (IccIoResult) ar.result;

                    if (processException(response, (AsyncResult) msg.obj)) {
                        loge("EVENT_SELECT_EF_FILE exception");
                        break;
                    }

                    data = result.payload;

                    if (TYPE_EF != data[RESPONSE_DATA_FILE_TYPE]) {
                        throw new IccFileTypeMismatch();
                    }

                    EFResponseData efData = new EFResponseData(data);
                    sendResult(response, efData, null);
                break;
                // PHB END
                default:
                    super.handleMessage(msg);
        }} catch (Exception exc) {
            if (response != null) {
                loge("caught exception:" + exc);
                sendResult(response, null, exc);
            } else {
                loge("uncaught exception" + exc);
            }
        }
    }


    // MTK-START
    public void loadEFLinearFixedAllByPath(int fileid, Message onLoaded, boolean is7FFF) {
        Message response = obtainMessage(EVENT_GET_RECORD_SIZE_DONE_EX,
                        new MtkLoadLinearFixedContext(fileid, onLoaded));

        mCi.iccIOForApp(COMMAND_GET_RESPONSE, fileid, getEFPath(fileid),
                        0, 0, GET_RESPONSE_EF_SIZE_BYTES, null, null, mAid, response);
    }

    public void loadEFLinearFixedAllByMode(int fileid, int mode , Message onLoaded) {
        MtkLoadLinearFixedContext lc = new MtkLoadLinearFixedContext(fileid, onLoaded);
        lc.mMode = mode;
        Message response = obtainMessage(EVENT_GET_RECORD_SIZE_DONE_EX, lc);

        mCi.iccIOForApp(COMMAND_GET_RESPONSE, fileid, getSmsEFPath(mode),
                    0, 0, GET_RESPONSE_EF_SIZE_BYTES, null, null, mAid, response);
    }

    protected String getSmsEFPath(int mode)
    {
        String efpath = "";

        if (mode == PhoneConstants.PHONE_TYPE_GSM)
            efpath = IccConstants.MF_SIM + IccConstants.DF_TELECOM;
        else if (mode == PhoneConstants.PHONE_TYPE_CDMA)
            efpath = IccConstants.MF_SIM + IccConstants.DF_CDMA;

        return efpath;
    }

    /**
     * Load a SIM Transparent EF with path specified
     *
     * @param fileid EF id
     * @param path EF path
     * @param onLoaded
     *
     * ((AsyncResult)(onLoaded.obj)).result is the byte[]
     *
     */

    public void loadEFTransparent(int fileid, String path, Message onLoaded) {
        String efPath = (path == null) ? getEFPath(fileid) : path;
        Message response = obtainMessage(EVENT_GET_BINARY_SIZE_DONE_EX,
                new MtkLoadTransparentContext(fileid, efPath, onLoaded));

        mCi.iccIOForApp(COMMAND_GET_RESPONSE, fileid, efPath,
                0, 0, GET_RESPONSE_EF_SIZE_BYTES, null, null, mAid, response);
    }

    /**
     * Update a transparent EF.
     * @param fileid EF id
     * @param path Path of the EF on the card
     * @param data must be exactly as long as the EF
     * @param onComplete onComplete.obj will be an AsyncResult
     */
    public void updateEFTransparent(int fileid, String path, byte[] data, Message onComplete) {
        String efPath = (path == null) ? getEFPath(fileid) : path;
        mCi.iccIOForApp(COMMAND_UPDATE_BINARY, fileid, efPath, 0, 0, data.length,
                IccUtils.bytesToHexString(data), null, mAid, onComplete);
    }

    // PHB START
    /**
     * Read a record from a SIM Linear Fixed EF
     *
     * @param fileid EF id
     * @param recordNum 1-based (not 0-based) record number
     * @param onLoaded
     *
     * ((AsyncResult)(onLoaded.obj)).result is the byte[]
     *
     */
    public void readEFLinearFixed(int fileid, int recordNum, int recordSize, Message onLoaded) {
        mCi.iccIOForApp(COMMAND_READ_RECORD, fileid, getEFPath(fileid),
                recordNum, READ_RECORD_MODE_ABSOLUTE, recordSize, null, null, mAid, onLoaded);
    }

     /**
     * select an EF file and get response
     *
     * @param fileid EF id
     * @param onLoaded (EFResponseData)efData
     *
     */
    public void selectEFFile(int fileid, Message onLoaded) {
        Message response = obtainMessage(EVENT_SELECT_EF_FILE,  fileid, 0, onLoaded);

        mCi.iccIOForApp(COMMAND_GET_RESPONSE, fileid, getEFPath(fileid),
                    0, 0, GET_RESPONSE_EF_SIZE_BYTES, null, null, mAid, response);
    }
    // PHB END
    // MTK-END

    @Override
    protected String getEFPath(int efid) {
        String path = getCommonIccEFPath(efid);
        if (path == null) {
            Rlog.e(LOG_TAG, "Error: EF Path being returned in null");
        }
        return path;
    }

    @Override
    protected void logd(String msg) {
        Rlog.i(LOG_TAG, msg);
    }

    @Override
    protected void loge(String msg) {
        Rlog.e(LOG_TAG, msg);
    }
}

