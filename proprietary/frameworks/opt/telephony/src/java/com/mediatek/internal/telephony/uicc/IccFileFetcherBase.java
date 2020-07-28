/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
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

import android.content.Context;
import android.content.Intent;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;
import android.telephony.Rlog;

import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.uicc.IccFileHandler;
import com.android.internal.telephony.uicc.UiccController;

import com.mediatek.internal.telephony.MtkIccUtils;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;

/**
 * Sim info request.
 */
class IccFileRequest {
    public int mEfid;
    public int mEfType;
    public int mAppType;
    public int mRecordNum;
    public String mEfPath;
    public String mPin2;
    public byte[] mData;
    public String mKey = null;

    /**
     * Constructor.
     */
    public IccFileRequest(int efid, int eftype, int apptype, String path, byte[] data,
            int recordnum, String pin2) {
        mEfid = efid;
        mEfType = eftype;
        mAppType = apptype;
        mEfPath = path;
        mData = data;
        mRecordNum = recordnum;
        mPin2 = pin2;
    }
}

/**
 * Base Sim info fetcher.
 */
public abstract class IccFileFetcherBase extends Handler {
    private static final String TAG = "IccFileFetcherBase";
    protected Phone mPhone;
    protected int mPhoneId;
    protected Context mContext;
    protected IccFileHandler mFh = null;
    protected UiccController mUiccController;
    protected HashMap<String, Object> mData = new HashMap<String, Object>();

    //File type
    protected static final int EF_TYPE_LINEARFIXED = 0;
    protected static final int EF_TYPE_TRANSPARENT = 1;
    //App type
    protected static final int APP_TYPE_ACTIVE = 0;
    protected static final int APP_TYPE_3GPP = 1;
    protected static final int APP_TYPE_3GPP2 = 2;
    protected static final int APP_TYPE_IMS = 3;
    //For invalid input
    protected static final int INVALID_INDEX = -1;

    //Event
    protected static final int EVENT_GET_LINEARFIXED_RECORD_SIZE_DONE = 0;
    protected static final int EVENT_LOAD_LINEARFIXED_ALL_DONE = 1;
    protected static final int EVENT_LOAD_TRANSPARENT_DONE = 2;
    protected static final int EVENT_UPDATE_LINEARFIXED_DONE = 3;
    protected static final int EVENT_UPDATE_TRANSPARENT_DONE = 4;

    protected IccFileFetcherBase(Context c, Phone phone) {
        log("IccFileFetcherBase Creating!");
        mPhone = phone;
        mPhoneId = mPhone.getPhoneId();
        mContext = c;
    }

    //Override
    /**
     * Get file key lists.
     * @return ArrayList<String> file key
     */
    public abstract ArrayList<String> onGetKeys();
    /**
     * Get file parameter.
     * @param key file index
     * @return IccFileRequest file info
     */
    public abstract IccFileRequest onGetFilePara(String key);
    /**
     * Parse the result.
     * @param key file index
     * @param transparent  file content
     * @param linearfixed  file content
     */
    public abstract void onParseResult(String key, byte[] transparent,
            ArrayList<byte[]> linearfixed);

    /**
     * Handle the intent.
     * @param intent message
     */
    public void onHandleIntent(Intent intent) {
        if (intent == null) {
            return;
        }
        String action = intent.getAction();

        if (TelephonyIntents.ACTION_SIM_STATE_CHANGED.equals(action)) {
            int phoneId = intent.getIntExtra(PhoneConstants.PHONE_KEY, -1);
            if ((mPhoneId != phoneId) ||
                    (mPhone.getPhoneType() != PhoneConstants.PHONE_TYPE_CDMA)) {
                // log("IccFileFetcherBase ACTION_SIM_STATE_CHANGED phoneId: " + phoneId
                //        + "phonetype: " + mPhone.getPhoneType() +
                //        ", No need to read omh file, return.");
                return;
            }
            String simStatus = intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);
            if (IccCardConstants.INTENT_VALUE_ICC_LOADED.equals(simStatus)) {
                // log("IccFileFetcherBase ACTION_SIM_STATE_CHANGED simStatus: " + simStatus);
                new Thread() {
                    public void run() {
                        exchangeSimInfo();
                    }
                } .start();
            }
        } else if (TelephonyIntents.ACTION_RADIO_TECHNOLOGY_CHANGED.equals(action)) {
            // log("IccFileFetcherBase Receive action: " + action);
            if (mData != null) {
                mData.clear();
                log("IccFileFetcherBase hashmap is cleared!");
            }
        }
    }

    protected void exchangeSimInfo() {
        IccFileRequest mRq;
        mUiccController = UiccController.getInstance();
        ArrayList<String> mKey = onGetKeys();
        Iterator<String> it = mKey.iterator();
        String key;
        while (it.hasNext()) {
            key = it.next();
            mRq = onGetFilePara(key);
            if (mRq == null) {
                loge("exchangeSimInfo mPhoneId:" + mPhoneId + "  key: " + it
                        + "  get Para failed!");
                break;
            }
            log("exchangeSimInfo key:" + key + " mEfid:" + mRq.mEfid + " mEfType:" + mRq.mEfType
                    + " mAppType :" + mRq.mAppType + " mEfPath:" + mRq.mEfPath + " mData:"
                    + MtkIccUtils.bytesToHexString(mRq.mData) + " mRecordNum:"
                    + mRq.mRecordNum + " mPin2:" + mRq.mPin2);
            if (mRq.mAppType == APP_TYPE_ACTIVE) {
                mFh = mPhone.getIccFileHandler();
            } else {
                mFh = mUiccController.getIccFileHandler(mPhoneId, mRq.mAppType);
            }
            if (mFh != null) {
                mRq.mKey = key;
                if (("".equals(mRq.mEfPath)) || (mRq.mEfPath == null)) {
                    log("exchangeSimInfo path is null, it may get an invalid reponse!");
                }
                if (mRq.mData == null) {
                    loadSimInfo(mRq);
                } else {
                    updateSimInfo(mRq);
                }
            } else {
                log("exchangeSimInfo mFh[" + mPhoneId + "] is null, read failed!");
            }
        }
    }

    protected void loadSimInfo(IccFileRequest req) {
        if (req.mEfType == EF_TYPE_LINEARFIXED) {
            mFh.loadEFLinearFixedAll(req.mEfid, req.mEfPath,
                    obtainMessage(EVENT_LOAD_LINEARFIXED_ALL_DONE, req));
        } else if (req.mEfType == EF_TYPE_TRANSPARENT) {
            mFh.loadEFTransparent(req.mEfid, req.mEfPath,
                    obtainMessage(EVENT_LOAD_TRANSPARENT_DONE, req));
        } else {
            loge("loadSimInfo req.mEfType = " + req.mEfType + " is invalid!");
        }
    }

    protected void updateSimInfo(IccFileRequest req) {
        if (mFh == null) {
            log("updateSimInfo mFh[" + mPhoneId + "] is null, updateSimInfo failed!");
            return;
        }
        if (req.mEfType == EF_TYPE_LINEARFIXED) {
            mFh.updateEFLinearFixed(req.mEfid, req.mEfPath, req.mRecordNum, req.mData, req.mPin2,
                    obtainMessage(EVENT_UPDATE_LINEARFIXED_DONE, req));
        } else if (req.mEfType == EF_TYPE_TRANSPARENT) {
            mFh.updateEFTransparent(req.mEfid, req.mEfPath, req.mData,
                    obtainMessage(EVENT_UPDATE_TRANSPARENT_DONE, req));
        } else {
            loge("updateSimInfo req.mEfType = " + req.mEfType + " is invalid!");
        }
    }

    @Override
    public void handleMessage(Message msg) {
        AsyncResult ar;
        // log("handleMessage : " + msg.what);
        byte[] data;
        ArrayList<byte[]> datas;
        int[] recordsize;
        IccFileRequest sr;

        try {
            switch (msg.what) {
                case EVENT_GET_LINEARFIXED_RECORD_SIZE_DONE:
                    ar = (AsyncResult) msg.obj;
                    if (ar.exception != null) {
                        log("EVENT_GET_LINEARFIXED_RECORD_SIZE_DONE Exception: "
                                + ar.exception);
                        break;
                    }
                    sr = (IccFileRequest) ar.userObj;
                    recordsize = (int[]) ar.result;
                    break;
                case EVENT_LOAD_LINEARFIXED_ALL_DONE:
                    ar = (AsyncResult) msg.obj;
                    sr = (IccFileRequest) ar.userObj;
                    if (ar.exception != null) {
                        loge("EVENT_LOAD_LINEARFIXED_ALL_DONE Exception: " + ar.exception);
                        onParseResult(sr.mKey, null, null);
                        break;
                    }
                    datas = (ArrayList<byte[]>) (ar.result);
                    log("EVENT_LOAD_LINEARFIXED_ALL_DONE key: " + sr.mKey + "  datas: " + datas);
                    onParseResult(sr.mKey, null, datas);
                    break;
                case EVENT_LOAD_TRANSPARENT_DONE:
                    ar = (AsyncResult) msg.obj;
                    sr = (IccFileRequest) ar.userObj;
                    if (ar.exception != null) {
                        loge("EVENT_LOAD_TRANSPARENT_DONE Exception: " + ar.exception);
                        onParseResult(sr.mKey, null, null);
                        break;
                    }
                    data = (byte[]) ar.result;
                    log("EVENT_LOAD_TRANSPARENT_DONE key: " + sr.mKey + "  data: "
                            + MtkIccUtils.bytesToHexString(data));
                    onParseResult(sr.mKey, data, null);
                    break;
                case EVENT_UPDATE_LINEARFIXED_DONE:
                    ar = (AsyncResult) msg.obj;
                    if (ar.exception != null) {
                        loge("EVENT_UPDATE_LINEARFIXED_DONE Exception: " + ar.exception);
                        break;
                    }
                    sr = (IccFileRequest) ar.userObj;
                    log("EVENT_UPDATE_LINEARFIXED_DONE key: " + sr.mKey + "  data: "
                            + MtkIccUtils.bytesToHexString(sr.mData));
                    break;
                case EVENT_UPDATE_TRANSPARENT_DONE:
                    ar = (AsyncResult) msg.obj;
                    if (ar.exception != null) {
                        loge("EVENT_UPDATE_TRANSPARENT_DONE Exception: " + ar.exception);
                        break;
                    }
                    sr = (IccFileRequest) ar.userObj;
                    log("EVENT_UPDATE_TRANSPARENT_DONE key: " + sr.mKey + "  data: "
                            + MtkIccUtils.bytesToHexString(sr.mData));
                    break;
                default:
                    super.handleMessage(msg);
                    break;
            }
        } catch (IllegalArgumentException exc) {
            loge("Exception parsing file record" + exc);
        }
    }

    protected void log(String msg) {
        Rlog.d(TAG, msg  + " (phoneId " + mPhoneId + ")");
    }
    protected void loge(String msg) {
        Rlog.e(TAG, msg + " (phoneId " + mPhoneId + ")");
    }
}

