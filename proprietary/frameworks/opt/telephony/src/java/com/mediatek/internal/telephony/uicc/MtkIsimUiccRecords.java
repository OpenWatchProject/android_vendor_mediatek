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

import android.content.Context;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;
import android.os.Registrant;
import android.os.SystemProperties;
import android.telephony.Rlog;
import android.content.Intent;

import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.uicc.IccUtils;
import com.android.internal.telephony.uicc.UiccController;
import com.android.internal.telephony.uicc.UiccCardApplication;
import com.android.internal.telephony.uicc.IsimUiccRecords;
import com.android.internal.telephony.uicc.UiccController;
import com.android.internal.telephony.uicc.UiccCardApplication;
import com.android.internal.telephony.uicc.IccRecords;
import com.android.internal.telephony.uicc.IccRefreshResponse;

import com.android.internal.telephony.gsm.SimTlv;
//import com.android.internal.telephony.gsm.VoiceMailConstants;

import java.io.FileDescriptor;
import java.io.PrintWriter;
import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.Arrays;

import com.mediatek.internal.telephony.uicc.IsimServiceTable;
import com.mediatek.internal.telephony.uicc.MtkIccConstants;

// MTK-START: ISIM
/**
 * {@hide}
 */
public class MtkIsimUiccRecords extends IsimUiccRecords implements MtkIsimRecords {
    protected static final String LOG_TAG = "MtkIsimUiccRecords";

    // MTK-START: SIM GBA
    private static final int EVENT_GET_GBABP_DONE = 200;
    private static final int EVENT_GET_GBANL_DONE = 201;
    // MTK-END
    private static final int EVENT_GET_PSISMSC_DONE = 202;

    protected UiccController mUiccController;
    IsimServiceTable mIsimServiceTable;

    private int mSlotId;
    private int mIsimChannel;

    byte[] mEfPsismsc = null;
    // MTK-START: SIM GBA
    private String mIsimGbabp;
    ArrayList<byte[]> mEfGbanlList;
    // MTK-END
    public MtkIsimUiccRecords(UiccCardApplication app, Context c, CommandsInterface ci) {
        super(app, c, ci);

        if (DBG) log("MtkIsimUiccRecords X ctor this=" + this);
        // MTK-START
        mSlotId = ((MtkUiccCardApplication)app).getPhoneId();
        mUiccController = UiccController.getInstance();
        // MTK-END
    }

    // ***** Overridden from Handler
    public void handleMessage(Message msg) {
        AsyncResult ar;
        // MTK-START
        byte data[];
        boolean isRecordLoadResponse = false;
        // MTK-END
        if (mDestroyed.get()) {
            Rlog.e(LOG_TAG, "Received message " + msg +
                    "[" + msg.what + "] while being destroyed. Ignoring.");
            return;
        }
        loge("IsimUiccRecords: handleMessage " + msg + "[" + msg.what + "] ");

        try {
            switch (msg.what) {
                // MTK-START
                // MTK-START: SIM GBA
                case EVENT_GET_GBABP_DONE:
                    isRecordLoadResponse = true;
                    ar = (AsyncResult) msg.obj;
                    if (ar.exception == null) {
                       data = ((byte[]) ar.result);
                       mIsimGbabp = IccUtils.bytesToHexString(data);
                       if (DUMP_RECORDS) log("EF_ISIM_GBABP=" + mIsimGbabp);
                    } else {
                        loge("Error on GET_ISIM_GBABP with exp " + ar.exception);
                    }
                    break;

                case EVENT_GET_GBANL_DONE:
                    isRecordLoadResponse = true;
                    ar = (AsyncResult) msg.obj;
                    if (ar.exception == null) {
                       mEfGbanlList = ((ArrayList<byte[]>) ar.result);
                        if (DBG) log("GET_ISIM_GBANL record count: " + mEfGbanlList.size());
                    } else {
                        loge("Error on GET_ISIM_GBANL with exp " + ar.exception);
                    }
                    break;
                // MTK-END
                case EVENT_GET_PSISMSC_DONE:
                    isRecordLoadResponse = true;

                    ar = (AsyncResult) msg.obj;
                    data = (byte[]) ar.result;

                    if (ar.exception != null) {
                        break;
                    }

                    log("EF_PSISMSC: " + IccUtils.bytesToHexString(data));

                    if (data != null) {
                        mEfPsismsc = data;
                    }
                    break;
                // MTK-END
                default:
                    super.handleMessage(msg);   // IccRecords handles generic record load responses

            }
        } catch (RuntimeException exc) {
            // I don't want these exceptions to be fatal
            Rlog.w(LOG_TAG, "Exception parsing SIM record", exc);
        // MTK-START
        } finally {
            // Count up record load responses even if they are fails
            if (isRecordLoadResponse) {
                onRecordLoaded();
            }
        }
        // MTK-END
    }

    protected void fetchIsimRecords() {
        mRecordsRequested = true;

        mFh.loadEFTransparent(EF_IMPI, obtainMessage(
                IccRecords.EVENT_GET_ICC_RECORD_DONE, new EfIsimImpiLoaded()));
        mRecordsToLoad++;

        mFh.loadEFLinearFixedAll(EF_IMPU, obtainMessage(
                IccRecords.EVENT_GET_ICC_RECORD_DONE, new EfIsimImpuLoaded()));
        mRecordsToLoad++;

        mFh.loadEFTransparent(EF_DOMAIN, obtainMessage(
                IccRecords.EVENT_GET_ICC_RECORD_DONE, new EfIsimDomainLoaded()));
        mRecordsToLoad++;
        mFh.loadEFTransparent(EF_IST, obtainMessage(
                    IccRecords.EVENT_GET_ICC_RECORD_DONE, new MtkEfIsimIstLoaded()));
        mRecordsToLoad++;

        // MTK-START
        //mFh.loadEFLinearFixedAll(EF_PCSCF, obtainMessage(
        //            IccRecords.EVENT_GET_ICC_RECORD_DONE, new EfIsimPcscfLoaded()));
        //mRecordsToLoad++;
        // MTK-END

        if (DBG) log("fetchIsimRecords " + mRecordsToLoad + " requested: " + mRecordsRequested);
    }

    // MTK-START: SIM GBA
    protected void fetchGbaParam() {
        if (mIsimServiceTable.isAvailable(IsimServiceTable.IsimService.GBA)) {
            mFh.loadEFTransparent(MtkIccConstants.EF_ISIM_GBABP,
                    obtainMessage(EVENT_GET_GBABP_DONE));
            mRecordsToLoad++;

            mFh.loadEFLinearFixedAll(MtkIccConstants.EF_ISIM_GBANL,
                    obtainMessage(EVENT_GET_GBANL_DONE));
            mRecordsToLoad++;
        }
    }
    // MTK-END

    private class MtkEfIsimIstLoaded implements IccRecords.IccRecordLoaded {
        public String getEfName() {
            return "EF_ISIM_IST";
        }
        public void onRecordLoaded(AsyncResult ar) {
            byte[] data = (byte[]) ar.result;
            mIsimIst = IccUtils.bytesToHexString(data);
            if (DUMP_RECORDS) log("EF_IST=" + mIsimIst);

            mIsimServiceTable = new IsimServiceTable(data);
            if (DBG) log("IST: " + mIsimServiceTable);

            if (mIsimServiceTable.isAvailable(IsimServiceTable.IsimService.PCSCF_ADDRESS)
                    || mIsimServiceTable.isAvailable(
                            IsimServiceTable.IsimService.PCSCF_DISCOVERY)) {
                mFh.loadEFLinearFixedAll(EF_PCSCF, obtainMessage(
                        IccRecords.EVENT_GET_ICC_RECORD_DONE, new EfIsimPcscfLoaded()));
                mRecordsToLoad++;
            }

            if (mIsimServiceTable.isAvailable(IsimServiceTable.IsimService.SM_OVER_IP)) {
                mFh.loadEFLinearFixed(MtkIccConstants.EF_PSISMSC, 1,
                        obtainMessage(EVENT_GET_PSISMSC_DONE));
                mRecordsToLoad++;
            }
            // MTK-START: SIM GBA
            fetchGbaParam();
            // MTK-END
        }
    }

    @Override
    public void registerForRecordsLoaded(Handler h, int what, Object obj) {
        if (mDestroyed.get()) {
            return;
        }

        Registrant r = new Registrant(h, what, obj);
        mRecordsLoadedRegistrants.add(r);

        if (mRecordsToLoad == 0 && mRecordsRequested == true) {
            r.notifyRegistrant(new AsyncResult(null, null, null));
        }
    }

    @Override
    public void unregisterForRecordsLoaded(Handler h) {
        mRecordsLoadedRegistrants.remove(h);
    }

    public byte[] getEfPsismsc() {
        log("PSISMSC = " + IccUtils.bytesToHexString(mEfPsismsc));
        return mEfPsismsc;
    }

    // MTK-START: SIM GBA
    /**
     * Returns the GBA bootstrapping parameters (GBABP) that was loaded from the ISIM.
     * @return GBA bootstrapping parameters or null if not present or not loaded
     */
    public String getIsimGbabp() {
        log("ISIM GBABP = " + mIsimGbabp);
        return mIsimGbabp;
    }

    /**
     * Set the GBA bootstrapping parameters (GBABP) value into the ISIM.
     * @param gbabp a GBA bootstrapping parameters value in String type
     * @param onComplete
     *        onComplete.obj will be an AsyncResult
     *        ((AsyncResult)onComplete.obj).exception == null on success
     *        ((AsyncResult)onComplete.obj).exception != null on fail
     */
    public void setIsimGbabp(String gbabp, Message onComplete) {
        byte[] data = IccUtils.hexStringToBytes(gbabp);

        mFh.updateEFTransparent(MtkIccConstants.EF_ISIM_GBABP, data, onComplete);
    }
    // MTK-END

    @Override
    protected void log(String s) {
        if (DBG) Rlog.d(LOG_TAG, "[ISIM] " + s + " (slot " + mSlotId + ")");
    }

    @Override
    protected void loge(String s) {
        if (DBG) Rlog.e(LOG_TAG, "[ISIM] " + s + " (slot " + mSlotId + ")");
    }
}
// MTK-END
